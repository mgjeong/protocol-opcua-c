#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>
#include <inttypes.h>

#include <opcua_manager.h>
#include <opcua_common.h>
#include <edge_logger.h>
#include <edge_malloc.h>

#define TEST_WITH_REFERENCE_SERVER 0

#define COLOR_GREEN        "\x1b[32m"
#define COLOR_YELLOW      "\x1b[33m"
#define COLOR_PURPLE      "\x1b[35m"
#define COLOR_RESET         "\x1b[0m"

#define MAX_CHAR_SIZE (512)
#define MAX_ADDRESS_SIZE (128)
#define NODE_COUNT (6)

static bool startFlag = false;
static bool stopFlag = false;

static char ipAddress[MAX_ADDRESS_SIZE];
static char endpointUri[MAX_CHAR_SIZE];
static int endpointCount = 0;

static EdgeConfigure *config = NULL;

#define TAG "SAMPLE_CLIENT"

typedef struct EndPointList {
    char *endpoint;
    struct EndPointList* next;
} EndPointList;

static EndPointList* epList = NULL;

typedef struct BrowseNextData
{
    EdgeBrowseParameter browseParam;
    int count;
    int last_used;
    EdgeContinuationPoint *cp; // Continuation point List. Size of list = last_used.
    EdgeNodeId **srcNodeId; // Id of source node of every continuation point. Size of list = last_used.
} BrowseNextData;

BrowseNextData *browseNextData = NULL;

#define MAX_CP_LIST_COUNT 1000

int maxReferencesPerNode = 0;

static void add_to_endpoint_list(char *endpoint);
static EndPointList *remove_from_endpoint_list(char *endpoint);

static void startClient(char *addr, int port, char *securityPolicyUri, char *endpoint);

// TODO: Remove this function later when sdk expose it.
char *cloneString(const char *str)
{
    if (!str)
    {
        return NULL;
    }
    int len = strlen(str);
    if (len < 1)
    {
        return NULL;
    }
    char *clone = (char *)calloc(len + 1,  sizeof(char));
    if (!clone)
    {
        return NULL;
    }
    strncpy(clone, str, len);
    clone[len] = '\0';
    return clone;
}

// TODO: Remove this function later when sdk expose it.
EdgeNodeId *cloneEdgeNodeId(EdgeNodeId *nodeId)
{
    if (!nodeId)
    {
        return NULL;
    }

    EdgeNodeId *clone = (EdgeNodeId *) calloc(1, sizeof(EdgeNodeId));
    if (!clone)
    {
        return NULL;
    }

    clone->nameSpace = nodeId->nameSpace;
    if (nodeId->nodeUri)
    {
        clone->nodeUri = cloneString(nodeId->nodeUri);
        if (!clone->nodeUri)
        {
            FREE(clone);
            return NULL;
        }
    }
    clone->nodeIdentifier = nodeId->nodeIdentifier;
    clone->type = nodeId->type;
    if (nodeId->nodeId)
    {
        clone->nodeId = cloneString(nodeId->nodeId);
        if (!clone->nodeId)
        {
            FREE(clone->nodeUri);
            FREE(clone);
            return NULL;
        }
    }
    clone->integerNodeId = nodeId->integerNodeId;

    return clone;
}

// TODO: Remove this function later when sdk expose it.
void freeEdgeNodeId(EdgeNodeId *nodeId)
{
    if (!nodeId)
    {
        return;
    }
    FREE(nodeId->nodeUri);
    FREE(nodeId->nodeId);
    FREE(nodeId);
}

bool addBrowseNextData(BrowseNextData *data, EdgeContinuationPoint *cp, EdgeNodeId *nodeId)
{
    if (data->last_used >= data->count)
    {
        printf("BrowseNextData limit(%d) reached. Cannot add this data.\n", data->count);
        return false;
    }

    int index = ++data->last_used;
    data->cp[index].length = cp->length;
    data->cp[index].continuationPoint = (unsigned char *)malloc(cp->length * sizeof(unsigned char));
    if(IS_NULL(data->cp[index].continuationPoint))
    {
        printf("Error : Malloc failed for data->cp[index].continuationPoint in addBrowseNextData\n");
        return false;
    }
    for (int i = 0; i < cp->length; i++)
    {
        data->cp[index].continuationPoint[i] = cp->continuationPoint[i];
    }

    data->srcNodeId[index] = cloneEdgeNodeId(nodeId);
    return true;
}

void destroyBrowseNextDataElements(BrowseNextData *data)
{
    if (!data)
        return;

    for (int i = 0; i <= data->last_used; ++i)
    {
        if(IS_NOT_NULL(data->cp))
        {
            FREE(data->cp[i].continuationPoint);
        }
        freeEdgeNodeId(data->srcNodeId[i]);
    }
}

void destroyBrowseNextData(BrowseNextData *data)
{
    if (!data)
        return;

    destroyBrowseNextDataElements(data);
    FREE(data->cp);
    FREE(data->srcNodeId);
    FREE(data);
}

void initBrowseNextData(EdgeBrowseParameter *browseParam)
{
    destroyBrowseNextData(browseNextData);
    browseNextData = (BrowseNextData *)calloc(1, sizeof(BrowseNextData));
    VERIFY_NON_NULL_NR(browseNextData);
    if(browseParam)
        browseNextData->browseParam = *browseParam;
    browseNextData->count = MAX_CP_LIST_COUNT;
    browseNextData->last_used = -1;
    browseNextData->cp = (EdgeContinuationPoint *)calloc(browseNextData->count,
                         sizeof(EdgeContinuationPoint));
    VERIFY_NON_NULL_NR(browseNextData->cp);
    browseNextData->srcNodeId = (EdgeNodeId **)calloc(browseNextData->count, sizeof(EdgeNodeId *));
    VERIFY_NON_NULL_NR(browseNextData->srcNodeId);
}

BrowseNextData *cloneBrowseNextData(BrowseNextData *data)
{
    if (!data)
        return NULL;

    BrowseNextData *clone = (BrowseNextData *)calloc(1, sizeof(BrowseNextData));
    VERIFY_NON_NULL(clone, NULL);
    clone->browseParam = browseNextData->browseParam;
    clone->count = browseNextData->count;
    clone->last_used = -1;
    clone->cp = (EdgeContinuationPoint *)calloc(clone->count, sizeof(EdgeContinuationPoint));
    if(IS_NULL(clone->cp))
    {
        printf("Error :: Calloc Failed for lone->cp in cloneBrowseNextData \n");
        FREE(clone);
        return NULL;
    }
    clone->srcNodeId = (EdgeNodeId **)calloc(clone->count, sizeof(EdgeNodeId *));
    if(IS_NULL(clone->srcNodeId))
    {
        printf("Error :: Calloc Failed for clone->srcNodeId in cloneBrowseNextData \n");
        FREE(clone->cp);
        FREE(clone);
        return NULL;
    }
    for (int i = 0; i <= browseNextData->last_used; ++i)
    {
        addBrowseNextData(clone, &browseNextData->cp[i], browseNextData->srcNodeId[i]);
    }

    return clone;
}

static void response_msg_cb (EdgeMessage *data)
{
    if (data->type == GENERAL_RESPONSE)
    {
        int len = data->responseLength;
        if (0 == len)
            printf("Response Received ::  \n");
        int idx = 0;
        for (idx = 0; idx < len; idx++)
        {
            if (data->responses[idx]->message != NULL)
            {
                if (data->command == CMD_READ || data->command == CMD_METHOD)
                {
                    printf("Response Received ::  ");
                    if (data->responses[idx]->message->isArray)
                    {
                        // Handle Output array
                        int arrayLen = data->responses[idx]->message->arrayLength;
                        if (data->responses[idx]->type == Boolean)
                        {
                            /* Handle Boolean output array */
                            for (int arrayIdx = 0; arrayIdx < arrayLen; arrayIdx++)
                            {
                                printf("%d  ", ((bool *) data->responses[idx]->message->value)[arrayIdx]);
                            }
                        }
                        else if (data->responses[idx]->type == Byte)
                        {
                            /* Handle Byte output array */
                            for (int arrayIdx = 0; arrayIdx < arrayLen; arrayIdx++)
                            {
                                printf("%" PRIu8 " ", ((uint8_t *) data->responses[idx]->message->value)[arrayIdx]);
                            }
                        }
                        else if (data->responses[idx]->type == SByte)
                        {
                            /* Handle SByte output array */
                            for (int arrayIdx = 0; arrayIdx < arrayLen; arrayIdx++)
                            {
                                printf("%" PRId8 " ", ((int8_t *) data->responses[idx]->message->value)[arrayIdx]);
                            }
                        }
                        else if (data->responses[idx]->type == Int16)
                        {
                            /* Handle int16 output array */
                            for (int arrayIdx = 0; arrayIdx < arrayLen; arrayIdx++)
                            {
                                printf("%" PRId16 "  ", ((int16_t *) data->responses[idx]->message->value)[arrayIdx]);
                            }
                        }
                        else if (data->responses[idx]->type == UInt16)
                        {
                            /* Handle UInt16 output array */
                            for (int arrayIdx = 0; arrayIdx < arrayLen; arrayIdx++)
                            {
                                printf("%" PRIu16 "  ", ((uint16_t *) data->responses[idx]->message->value)[arrayIdx]);
                            }
                        }
                        else if (data->responses[idx]->type == Int32)
                        {
                            /* Handle Int32 output array */
                            for (int arrayIdx = 0; arrayIdx < arrayLen; arrayIdx++)
                            {
                                printf("%d  ", ((int32_t *) data->responses[idx]->message->value)[arrayIdx]);
                            }
                        }
                        else if (data->responses[idx]->type == UInt32)
                        {
                            /* Handle UInt32 output array */
                            for (int arrayIdx = 0; arrayIdx < arrayLen; arrayIdx++)
                            {
                                printf("%u  ", ((uint *) data->responses[idx]->message->value)[arrayIdx]);
                            }
                        }
                        else if (data->responses[idx]->type == Int64)
                        {
                            /* Handle Int64 output array */
                            for (int arrayIdx = 0; arrayIdx < arrayLen; arrayIdx++)
                            {
                                printf("%ld  ", ((long int *) data->responses[idx]->message->value)[arrayIdx]);
                            }
                        }
                        else if (data->responses[idx]->type == UInt64)
                        {
                            /* Handle UInt64 output array */
                            for (int arrayIdx = 0; arrayIdx < arrayLen; arrayIdx++)
                            {
                                printf("%lu  ", ((ulong *) data->responses[idx]->message->value)[arrayIdx]);
                            }
                        }
                        else if (data->responses[idx]->type == Float)
                        {
                            /* Handle Float output array */
                            for (int arrayIdx = 0; arrayIdx < arrayLen; arrayIdx++)
                            {
                                printf("%g  ", ((float *) data->responses[idx]->message->value)[arrayIdx]);
                            }
                        }
                        else if (data->responses[idx]->type == Double)
                        {
                            /* Handle Double output array */
                            for (int arrayIdx = 0; arrayIdx < arrayLen; arrayIdx++)
                            {
                                printf("%g  ", ((double *) data->responses[idx]->message->value)[arrayIdx]);
                            }
                        }
                        else if (data->responses[idx]->type == String || data->responses[idx]->type == ByteString
                                 || data->responses[idx]->type == Guid)
                        {
                            /* Handle String/ByteString/Guid output array */
                            char **values = ((char **) data->responses[idx]->message->value);
                            for (int arrayIdx = 0; arrayIdx < arrayLen; arrayIdx++)
                            {
                                printf("%s  ", values[arrayIdx]);
                            }
                        }
                        else if (data->responses[idx]->type == DateTime)
                        {
                            /* Handle DateTime output array */
                            for (int arrayIdx = 0; arrayIdx < arrayLen; arrayIdx++)
                            {
                                printf("%" PRId64 "  ", ((int64_t *) data->responses[idx]->message->value)[arrayIdx]);
                            }
                        }
                        printf("\n");
                    }
                    else
                    {
                        if (data->responses[idx]->type == Boolean)
                            printf("[%d]\n",
                                   *((int *)data->responses[idx]->message->value));
                        else if (data->responses[idx]->type == Byte)
                            printf("[%" PRIu8 "]\n",
                                   *((uint8_t *)data->responses[idx]->message->value));
                        else if (data->responses[idx]->type == SByte)
                            printf("[%" PRId8 "]\n",
                                   *((int8_t *)data->responses[idx]->message->value));
                        else if (data->responses[idx]->type == ByteString)
                            printf("[%s]\n",
                                   (char *)data->responses[idx]->message->value);
                        else if (data->responses[idx]->type == DateTime)
                            printf("[%" PRId64 "]\n",
                                   *((int64_t *)data->responses[idx]->message->value));
                        else if (data->responses[idx]->type == Double)
                            printf(" [%g]\n",
                                   *((double *)data->responses[idx]->message->value));
                        else if (data->responses[idx]->type == Float)
                            printf("[%g]\n",
                                   *((float *)data->responses[idx]->message->value));
                        else if (data->responses[idx]->type == Int16)
                            printf("[%" PRId16 "]\n",
                                   *((int16_t *)data->responses[idx]->message->value));
                        else if (data->responses[idx]->type == UInt16)
                            printf("[%" PRIu16 "]\n",
                                   *((uint16_t *)data->responses[idx]->message->value));
                        else if (data->responses[idx]->type == Int32)
                            printf("[%" PRId32 "]\n",
                                   *((int32_t *)data->responses[idx]->message->value));
                        else if (data->responses[idx]->type == UInt32)
                            printf("[%" PRIu32 "]\n",
                                   *((uint32_t *)data->responses[idx]->message->value));
                        else if (data->responses[idx]->type == Int64)
                            printf("[%" PRId64 "]\n",
                                   *((int64_t *)data->responses[idx]->message->value));
                        else if (data->responses[idx]->type == UInt64)
                            printf("[%" PRIu64 "]\n",
                                   *((uint64_t *)data->responses[idx]->message->value));
                        else if (data->responses[idx]->type == String)
                            printf("[%s]\n",
                                   (char *)data->responses[idx]->message->value);
                    }
                }
                else if (data->command == CMD_WRITE)
                {
                    printf("\nWrite response :: %s\n",
                           (char *) data->responses[idx]->message->value);
                }
                // Diagnostics information
                if (data->responses[idx]->m_diagnosticInfo)
                {
                    printf("Diagnostics information\n");
                    printf("symbolicId :: %d, localizedText : %d, additionalInfo : %s , msg :: %s\n" ,
                           data->responses[idx]->m_diagnosticInfo->symbolicId,
                           data->responses[idx]->m_diagnosticInfo->localizedText,
                           data->responses[idx]->m_diagnosticInfo->additionalInfo,
                           data->responses[idx]->m_diagnosticInfo->msg);
                }
            }
        }
    }
}

static void monitored_msg_cb (EdgeMessage *data)
{
    if (data->type == REPORT)
    {
        printf("[Application response Callback] Monitored Item Response received\n");
        int len = data->responseLength;
        int idx = 0;
        for (idx = 0; idx < len; idx++)
        {
            printf("[Node Name] : %s\n", data->responses[idx]->nodeInfo->valueAlias);
            if (data->responses[idx]->message->isArray)
            {
                // Handle Output array
                int arrayLen = data->responses[idx]->message->arrayLength;
                if (data->responses[idx]->type == Boolean)
                {
                    /* Handle Boolean output array */
                    for (int arrayIdx = 0; arrayIdx < arrayLen; arrayIdx++)
                    {
                        printf("%d  ", ((bool *) data->responses[idx]->message->value)[arrayIdx]);
                    }
                }
                else if (data->responses[idx]->type == Byte)
                {
                    /* Handle Byte output array */
                    for (int arrayIdx = 0; arrayIdx < arrayLen; arrayIdx++)
                    {
                        printf("%" PRIu8 " ", ((uint8_t *) data->responses[idx]->message->value)[arrayIdx]);
                    }
                }
                else if (data->responses[idx]->type == SByte)
                {
                    /* Handle SByte output array */
                    for (int arrayIdx = 0; arrayIdx < arrayLen; arrayIdx++)
                    {
                        printf("%" PRId8 " ", ((int8_t *) data->responses[idx]->message->value)[arrayIdx]);
                    }
                }
                else if (data->responses[idx]->type == Int16)
                {
                    /* Handle int16 output array */
                    for (int arrayIdx = 0; arrayIdx < arrayLen; arrayIdx++)
                    {
                        printf("%" PRId16 "  ", ((int16_t *) data->responses[idx]->message->value)[arrayIdx]);
                    }
                }
                else if (data->responses[idx]->type == UInt16)
                {
                    /* Handle UInt16 output array */
                    for (int arrayIdx = 0; arrayIdx < arrayLen; arrayIdx++)
                    {
                        printf("%" PRIu16 "  ", ((uint16_t *) data->responses[idx]->message->value)[arrayIdx]);
                    }
                }
                else if (data->responses[idx]->type == Int32)
                {
                    /* Handle Int32 output array */
                    for (int arrayIdx = 0; arrayIdx < arrayLen; arrayIdx++)
                    {
                        printf("%d  ", ((int32_t *) data->responses[idx]->message->value)[arrayIdx]);
                    }
                }
                else if (data->responses[idx]->type == UInt32)
                {
                    /* Handle UInt32 output array */
                    for (int arrayIdx = 0; arrayIdx < arrayLen; arrayIdx++)
                    {
                        printf("%u  ", ((uint *) data->responses[idx]->message->value)[arrayIdx]);
                    }
                }
                else if (data->responses[idx]->type == Int64)
                {
                    /* Handle Int64 output array */
                    for (int arrayIdx = 0; arrayIdx < arrayLen; arrayIdx++)
                    {
                        printf("%ld  ", ((long int *) data->responses[idx]->message->value)[arrayIdx]);
                    }
                }
                else if (data->responses[idx]->type == UInt64)
                {
                    /* Handle UInt64 output array */
                    for (int arrayIdx = 0; arrayIdx < arrayLen; arrayIdx++)
                    {
                        printf("%lu  ", ((ulong *) data->responses[idx]->message->value)[arrayIdx]);
                    }
                }
                else if (data->responses[idx]->type == Float)
                {
                    /* Handle Float output array */
                    for (int arrayIdx = 0; arrayIdx < arrayLen; arrayIdx++)
                    {
                        printf("%g  ", ((float *) data->responses[idx]->message->value)[arrayIdx]);
                    }
                }
                else if (data->responses[idx]->type == Double)
                {
                    /* Handle Double output array */
                    for (int arrayIdx = 0; arrayIdx < arrayLen; arrayIdx++)
                    {
                        printf("%g  ", ((double *) data->responses[idx]->message->value)[arrayIdx]);
                    }
                }
                else if (data->responses[idx]->type == String || data->responses[idx]->type == ByteString
                         || data->responses[idx]->type == Guid)
                {
                    /* Handle String/ByteString/Guid output array */
                    char **values = ((char **) data->responses[idx]->message->value);
                    for (int arrayIdx = 0; arrayIdx < arrayLen; arrayIdx++)
                    {
                        printf("%s  ", values[arrayIdx]);
                    }
                }
                else if (data->responses[idx]->type == DateTime)
                {
                    /* Handle DateTime output array */
                    for (int arrayIdx = 0; arrayIdx < arrayLen; arrayIdx++)
                    {
                        printf("%" PRId64 "  ", ((int64_t *) data->responses[idx]->message->value)[arrayIdx]);
                    }
                }
                printf("\n");
            }
            else
            {
                if (data->responses[idx]->message != NULL)
                {
                    if (data->responses[idx]->type == Int16)
                        printf("[MonitoredItem DataChange callback] Monitored Change Value read from node ===>> [%d]\n",
                               *((int *)data->responses[idx]->message->value));
                    else if (data->responses[idx]->type == Byte)
                        printf("[MonitoredItem DataChange callback] Monitored Change Value read from node ===>> [%d]\n",
                               *((uint8_t *)data->responses[idx]->message->value));
                    else if (data->responses[idx]->type == UInt16)
                        printf("[MonitoredItem DataChange callback] Monitored Change Value read from node ===>> [%d]\n",
                               *((int *)data->responses[idx]->message->value));
                    else if (data->responses[idx]->type == Int32)
                        printf("[MonitoredItem DataChange callback] Monitored Change Value read from node ===>>  [%d]\n",
                               *((int *)data->responses[idx]->message->value));
                    else if (data->responses[idx]->type == UInt32)
                        printf("[MonitoredItem DataChange callback] Monitored Change Value read from node ===>>  [%d]\n",
                               *((int *)data->responses[idx]->message->value));
                    else if (data->responses[idx]->type == Int64)
                        printf("[MonitoredItem DataChange callback] Monitored Change Value read from node ===>>  [%ld]\n",
                               *((long *)data->responses[idx]->message->value));
                    else if (data->responses[idx]->type == UInt64)
                        printf("[MonitoredItem DataChange callback] Monitored Change Value read from node ===>>  [%ld]\n",
                               *((long *)data->responses[idx]->message->value));
                    else if (data->responses[idx]->type == Float)
                        printf("[MonitoredItem DataChange callback] Monitored Change Value read from node  ===>>  [%f]\n",
                               *((float *)data->responses[idx]->message->value));
                    else if (data->responses[idx]->type == Double)
                        printf("[MonitoredItem DataChange callback] Monitored Change Value read from node  ===>>  [%f]\n",
                               *((double *)data->responses[idx]->message->value));
                    else if (data->responses[idx]->type == String)
                        printf("[MonitoredItem DataChange callback] Monitored Change Value read from node ===>>  [%s]\n",
                               ((char *)data->responses[idx]->message->value));
                }
            }
        }
        printf("\n\n");
    }
}

static void error_msg_cb (EdgeMessage *data)
{
    printf("\n[Application error response callback]\n");
    printf("================================================\n");
    printf("EdgeStatusCode: %d\n", data->result->code);
    int responseLength = data->responseLength;
    for (int i = 0; i < responseLength; ++i)
    {
        EdgeResponse *response = data->responses[i];
        if (!response)
        {
            printf("EdgeResponse[%d] is null\n", i);
            continue;
        }
        if (response->message)
        {
            printf("Response[%d]->message: %s\n", i, (char *)response->message->value);
        }
        else
        {
            printf("Response[%d]->message is NULL\n", i);
        }
        if (response->nodeInfo)
        {
            EdgeNodeId *nodeId = response->nodeInfo->nodeId;
            if (nodeId)
            {
                if (nodeId->type == INTEGER)
                    printf("Response[%d]->NodeId(Integer): %d\n", i, nodeId->integerNodeId);
                else
                    printf("Response[%d]->NodeId(String): %s\n", i, nodeId->nodeId);
            }
            else
            {
                printf("Response[%d]->nodeInfo->nodeId is NULL\n", i);
            }
        }
        else
        {
            printf("Response[%d]->nodeInfo is NULL\n", i);
        }
    }
    printf("================================================\n");
}

#if 0
static void browse_msg_cb (EdgeMessage *data)
{
    if (data->type == BROWSE_RESPONSE)
    {
        EdgeBrowseResult *browseResult = data->browseResult;
        int idx = 0;
        printf("\n[Application browse response callback] List of Browse Names for request(%d)\n",
               data->responses[0]->requestId);
        printf("================================================\n");
        for (idx = 0; idx < data->browseResultLength; idx++)
        {
            printf("[%d] %s\n", idx + 1, browseResult[idx].browseName);
        }
        printf("================================================\n");
    }
}
#endif
static void browse_msg_cb (EdgeMessage *data)
{
    if (data->browseResult)
    {
        printf("%s\n", (unsigned char *)data->responses[0]->message->value);
        /*EdgeBrowseResult *browseResult = data->browseResult;
        //EdgeNodeId *nodeId = data->responses[0]->nodeInfo->nodeId;
        //printf("Source Node ID: ");
        //(nodeId->type == INTEGER) ? printf("%d\n", nodeId->integerNodeId) : printf("%s\n", nodeId->nodeId);

        if (data->browseResultLength > 0)
        {
            //printf("BrowseName(s): ");
            for (int idx = 0; idx < data->browseResultLength; idx++)
            {
                if (idx != 0) printf(", ");
                printf("Browse Name: %s\n", browseResult[idx].browseName);
            }
            //printf("\n\n");
        }*/
    }
    else
    {
        if (data->cpList && data->cpList->count > 0)
        {
            printf("Total number of continuation points: %d\n", data->cpList->count);
            for (int i = 0; i < data->cpList->count; ++i)
            {
                EdgeNodeId *nodeId = data->responses[i]->nodeInfo->nodeId;
                printf("Node ID of Continuation point[%d]: ", i + 1);
                (nodeId->type == INTEGER) ? printf("%d\n", nodeId->integerNodeId) : printf("%s\n", nodeId->nodeId);

                int length = data->cpList->cp[i]->length;
                unsigned char *cp = data->cpList->cp[i]->continuationPoint;
                printf("Length: %d\n", length);
                for (int j = 0; j < length; ++j)
                {
                    printf("%02X", cp[j]);
                }
                printf("\n");

                if (!addBrowseNextData(browseNextData, data->cpList->cp[i], nodeId))
                    break;
            }
            printf("\n\n");
        }
    }
}

/* status callbacks */
static void status_start_cb (EdgeEndPointInfo *epInfo, EdgeStatusCode status)
{
    if (status == STATUS_CLIENT_STARTED)
    {
        printf(COLOR_GREEN "[Application Callback] Client connected\n" COLOR_RESET);
        startFlag = true;
        add_to_endpoint_list(epInfo->endpointUri);
        endpointCount++;
    }
}

static void status_stop_cb (EdgeEndPointInfo *epInfo, EdgeStatusCode status)
{
    if (status == STATUS_STOP_CLIENT)
    {
        printf(COLOR_GREEN "[Application Callback] Client disconnected \n\n" COLOR_RESET);
        EndPointList *list = remove_from_endpoint_list(epInfo->endpointUri);
        if (list)
        {
            FREE (list->endpoint);
            FREE (list);
            endpointCount--;
        }
        if (0 == endpointCount)
        {
            stopFlag = true;
        }
    }
}

static void status_network_cb (EdgeEndPointInfo *epInfo, EdgeStatusCode status)
{

}

/* discovery callback */
static void endpoint_found_cb (EdgeDevice *device)
{
    if (device)
    {
        int num_endpoints = device->num_endpoints;
        printf("\n[Application Callback] Total number of endpoints: %d\n", num_endpoints);
        for (int idx = 0; idx < num_endpoints; idx++)
        {
            printf("\n[Application Callback] EndpointUri :: %s\n", device->endpointsInfo[idx]->endpointUri);
            printf("[Application Callback] Address :: %s, Port : %d, ServerName :: %s\n", device->address,
                   device->port, device->serverName);
            printf("[Application Callback] SecurityPolicyUri :: %s\n\n",
                   device->endpointsInfo[idx]->securityPolicyUri);

            startClient(device->address, device->port, device->endpointsInfo[idx]->securityPolicyUri, device->endpointsInfo[idx]->endpointUri);
        }
    }
}

static void device_found_cb (EdgeDevice *device)
{

}

////////////////////////// START : helper function for endpoint list //////////////////////////////

static EndPointList *create_endpoint_list(char *endpoint) {
    EndPointList *ep = (EndPointList*) malloc(sizeof(EndPointList));
    VERIFY_NON_NULL(ep, NULL);
    ep->endpoint = (char*) malloc(sizeof(char) * (strlen(endpoint) + 1));
    if(IS_NULL(ep->endpoint))
    {
        printf("Error : Malloc failed for endpoint in create_endpoint_list\n");
        FREE(ep);
        return NULL;
    }
    strncpy(ep->endpoint, endpoint, strlen(endpoint));
    ep->endpoint[strlen(endpoint)] = '\0';
    ep->next = NULL;
    return ep;
}

static void add_to_endpoint_list(char *endpoint)
{
    printf("Adding new endpoint list : %s\n\n", endpoint);
    if (NULL == epList)
    {
        epList = create_endpoint_list(endpoint);
        return ;
    }

    EndPointList *temp = epList;
    while (temp)
    {
        if (temp->next == NULL)
        {
            // add to the end of the list
            EndPointList *list = create_endpoint_list(endpoint);
            temp->next = list;
            break;
        }
        temp = temp->next;
    }
}

static char* get_endpoint_from_list(int option) {

    int cnt = 1;
    EndPointList *temp = epList;
    while (temp)
    {
        if (cnt == option)
        {
            return temp->endpoint;
        }
        temp = temp->next;
        cnt++;
    }
    return NULL;
}

static int print_endpoint_list() {
    int cnt = 0;
    EndPointList *temp = epList;
    while (temp)
    {
        printf("\n(%d)   %s", cnt+1, temp->endpoint);
        temp = temp->next;
        cnt++;
    }
    printf("\n\n");
    return cnt;
}

static EndPointList *remove_from_endpoint_list(char *endpoint)
{
    EndPointList *temp = epList;
    EndPointList *prev = NULL;
    while (temp != NULL)
    {
        if (!strcmp(temp->endpoint, endpoint))
        {
            if (prev == NULL)
            {
                epList = temp->next;
            }
            else
            {
                prev->next = temp->next;
            }
//      FREE (temp); temp = NULL;
            return temp;
        }
        prev = temp;
        temp = temp->next;
    }
    return NULL;
}

////////////////////////// END : helper function for endpoint list //////////////////////////////


////////////////////////// START : Get endpoint info from user //////////////////////////////

static char *getEndPoint_input()
{
    int cnt = print_endpoint_list();
    int inp;
    printf("Enter the endpoint (integer option) to connect to ::  ");
    scanf("%d", &inp);

    if (inp < 0 || inp > cnt)
    {
        printf("invalid option.\n");
        return NULL;
    }

    char *ep = get_endpoint_from_list(inp);
    return ep;
}

////////////////////////// END : Get endpoint info from user ///////////////////////////////


static void init()
{
    config = (EdgeConfigure *) malloc(sizeof(EdgeConfigure));
    VERIFY_NON_NULL_NR(config);
    config->recvCallback = (ReceivedMessageCallback *) malloc(sizeof(ReceivedMessageCallback));
    VERIFY_NON_NULL_NR(config->recvCallback);
    config->recvCallback->resp_msg_cb = response_msg_cb;
    config->recvCallback->monitored_msg_cb = monitored_msg_cb;
    config->recvCallback->error_msg_cb = error_msg_cb;
    config->recvCallback->browse_msg_cb = browse_msg_cb;

    config->statusCallback = (StatusCallback *) malloc(sizeof(StatusCallback));
    VERIFY_NON_NULL_NR(config->statusCallback);
    config->statusCallback->start_cb = status_start_cb;
    config->statusCallback->stop_cb = status_stop_cb;
    config->statusCallback->network_cb = status_network_cb;

    config->discoveryCallback = (DiscoveryCallback *) malloc(sizeof(DiscoveryCallback));
    VERIFY_NON_NULL_NR(config->discoveryCallback);
    config->discoveryCallback->endpoint_found_cb = endpoint_found_cb;
    config->discoveryCallback->device_found_cb = device_found_cb;

    registerCallbacks(config);
}

static void testGetEndpoints()
{
    EdgeEndPointInfo *ep = (EdgeEndPointInfo *) calloc(1, sizeof(EdgeEndPointInfo));
    if(IS_NULL(ep))
    {
        printf("Error : Malloc failed for EdgeEndPointInfo in test GetEndpoints\n");
        goto EXIT_EP;
    }
    ep->endpointUri = endpointUri;

    EdgeMessage *msg = (EdgeMessage *) malloc(sizeof(EdgeMessage));
    if(IS_NULL(msg))
    {
        printf("Error : Malloc failed for EdgeMessage in test GetEndpoints\n");
        goto EXIT_EP;
    }
    msg->endpointInfo = ep;
    msg->command = CMD_GET_ENDPOINTS;
    msg->type = SEND_REQUEST;

    EdgeResult res = getEndpointInfo(ep);
    if(res.code != STATUS_OK)
    {
        printf("getEndpointInfo() failed.\n");
    }

    EXIT_EP:
    FREE(ep);
    FREE(msg);
}

static void startClient(char *addr, int port, char *securityPolicyUri, char *endpoint)
{
    printf("\n" COLOR_YELLOW "------------------------------------------------------" COLOR_RESET);
    printf("\n" COLOR_YELLOW "                       Client connect            "COLOR_RESET);
    printf("\n" COLOR_YELLOW "------------------------------------------------------" COLOR_RESET
           "\n\n");

    EdgeEndpointConfig *endpointConfig = (EdgeEndpointConfig *) calloc(1, sizeof(EdgeEndpointConfig));
    if(IS_NULL(endpointConfig))
    {
        printf("Error : Malloc failed for EdgeEndpointConfig in test subscription delete\n");
        goto EXIT_START;
    }
    endpointConfig->requestTimeout = 60000;
    endpointConfig->serverName = DEFAULT_SERVER_NAME_VALUE;
    endpointConfig->bindAddress = addr;
    endpointConfig->bindPort = port;


    EdgeEndPointInfo *ep = (EdgeEndPointInfo *) calloc(1, sizeof(EdgeEndPointInfo));
    if(IS_NULL(ep))
    {
        printf("Error : Malloc failed for EdgeEndPointInfo in Start Client\n");
        goto EXIT_START;
    }
    ep->endpointUri = endpointUri;
    ep->endpointConfig = endpointConfig;
//    ep->appConfig = appConfig;
    ep->securityPolicyUri = securityPolicyUri;

    EdgeMessage *msg = (EdgeMessage *) malloc(sizeof(EdgeMessage));
    if(IS_NULL(msg))
    {
        printf("Error : Malloc failed for EdgeMessage in Start Client\n");
        goto EXIT_START;
    }
    msg->endpointInfo = ep;
    msg->command = CMD_START_CLIENT;
    msg->type = SEND_REQUEST;

    connectClient(ep);

    EXIT_START:
    FREE(endpointConfig);
    FREE(ep);
    FREE(msg);
}

static void stopClient()
{
    EdgeEndPointInfo *epInfo = (EdgeEndPointInfo *) calloc(1, sizeof(EdgeEndPointInfo));
    if(IS_NULL(epInfo))
    {
        printf("Error : Malloc failed for EdgeEndPointInfo in stop client\n");
        return;
    }
    EdgeMessage *msg = (EdgeMessage *) malloc(sizeof(EdgeMessage));
    if(IS_NULL(msg))
    {
        printf("Error : Malloc failed for EdgeMessage in stop client\n");
        FREE(epInfo);
        return;
    }
    msg->endpointInfo = epInfo;
    msg->command = CMD_STOP_CLIENT;

    EndPointList *temp = epList;
    while (temp)
    {
        epInfo->endpointUri = temp->endpoint;
        printf("\n" COLOR_YELLOW "********************** stop client **********************"
           COLOR_RESET"\n");
        disconnectClient(epInfo);
        temp = temp->next;
    }

    FREE(epInfo);
    FREE(msg);
}

static void deinit()
{
    stopClient();
/*    if (config)
    {
        if (config->recvCallback)
        {
            free (config->recvCallback);
            config->recvCallback = NULL;
        }
        if (config->statusCallback)
        {
            free (config->statusCallback);
            config->statusCallback = NULL;
        }
        if (config->discoveryCallback)
        {
            free (config->discoveryCallback);
            config->discoveryCallback = NULL;
        }
        free (config); config = NULL;
    }*/
}

static void testBrowseNext()
{
    printf("\n" COLOR_YELLOW "------------------------------------------------------" COLOR_RESET);
    printf("\n" COLOR_YELLOW "                       Browse Next            "COLOR_RESET);
    printf("\n" COLOR_YELLOW "------------------------------------------------------" COLOR_RESET
           "\n\n");

    if (!browseNextData || browseNextData->last_used < 0)
    {
        printf("Invalid data for browse next service.\n");
        return;
    }

    BrowseNextData *clone = cloneBrowseNextData(browseNextData);
    if (!clone)
    {
        printf("Failed to clone the BrowseNextData.\n");
        return;
    }

    initBrowseNextData(&browseNextData->browseParam);
    printf("Total number of continuation points: %d.\n", clone->last_used + 1);

    EdgeEndPointInfo *ep = (EdgeEndPointInfo *) calloc(1, sizeof(EdgeEndPointInfo));
    if(IS_NULL(ep))
    {
        printf("Error : Malloc failed for EdgeEndPointInfo in test browse Next\n");
        goto EXIT_BROWSENEXT;
    }
    ep->endpointUri = endpointUri;

    EdgeMessage *msg = (EdgeMessage *) calloc(1, sizeof(EdgeMessage));
    if(IS_NULL(msg))
    {
        printf("Error : Malloc failed for EdgeMessage in test browse Next\n");
        goto EXIT_BROWSENEXT;
    }
    msg->type = SEND_REQUESTS; // There can be one or more continuation points.
    msg->endpointInfo = ep;
    msg->command = CMD_BROWSE; // Using the same existing command for browse next operation as well.

    int requestLength = clone->last_used + 1;
    EdgeRequest **requests = (EdgeRequest **) calloc(requestLength, sizeof(EdgeRequest *));
    if(IS_NULL(requests))
    {
        printf("Error : Malloc failed for requests in test browse Next\n");
        goto EXIT_BROWSENEXT;
    }

    for (int i = 0; i < requestLength; i++)
    {
        EdgeNodeInfo *nodeInfo = (EdgeNodeInfo *) calloc(1, sizeof(EdgeNodeInfo));
        if(IS_NULL(nodeInfo))
        {
            printf("Error : Malloc failed for nodeInfo in test browse Next\n");
            goto EXIT_BROWSENEXT;
        }
        nodeInfo->nodeId = clone->srcNodeId[i];
        requests[i] = (EdgeRequest *) calloc(1, sizeof(EdgeRequest));
        if(IS_NULL(requests[i]))
        {
            printf("Error : Malloc failed for requests[%d] in test browse Next\n", i);
            FREE(nodeInfo);
            goto EXIT_BROWSENEXT;
        }
        requests[i]->nodeInfo = nodeInfo;
    }

    msg->requests = requests;
    msg->requestLength = requestLength;
    msg->browseParam = &clone->browseParam;

    msg->cpList = (EdgeContinuationPointList *)calloc(1, sizeof(EdgeContinuationPointList));
    if(IS_NULL(msg->cpList))
    {
        printf("Error : Malloc failed for msg->cpList in test browse Next\n");
        goto EXIT_BROWSENEXT;
    }
    msg->cpList->count = requestLength;
    msg->cpList->cp = (EdgeContinuationPoint **)calloc(requestLength, sizeof(EdgeContinuationPoint *));
    if(IS_NULL(msg->cpList->cp))
    {
        printf("Error : Malloc failed for msg->cpList->cp in test browse Next\n");
        goto EXIT_BROWSENEXT;
    }
    for (int i = 0; i < requestLength; i++)
    {
        msg->cpList->cp[i] = &clone->cp[i];
    }

    browseNext(msg);

    EXIT_BROWSENEXT:
    if(IS_NOT_NULL(requests))
    {
            for (int i = 0; i < requestLength; i++)
            {
                if(IS_NOT_NULL(requests[i]))
                {
                    FREE(requests[i]->nodeInfo);
                    FREE(requests[i]);
                }
            }
        FREE(requests);
    }
    if(IS_NOT_NULL(msg))
    {
        if(IS_NOT_NULL(msg->cpList))
        {
            FREE(msg->cpList->cp);
            FREE(msg->cpList);
        }
    }
    FREE(ep);
    FREE(msg);
    destroyBrowseNextData(clone);
}

static void testBrowseViews(char* endpointUri)
{
    printf("\n" COLOR_YELLOW "------------------------------------------------------" COLOR_RESET);
    printf("\n" COLOR_YELLOW "                       Browse Views            "COLOR_RESET);
    printf("\n" COLOR_YELLOW "------------------------------------------------------" COLOR_RESET
           "\n\n");

    EdgeEndPointInfo *ep = (EdgeEndPointInfo *) calloc(1, sizeof(EdgeEndPointInfo));
    if(IS_NULL(ep))
    {
        printf("Error : Malloc failed for EdgeEndPointInfo in test browse\n");
        goto EXIT_BROWSE;
    }
    ep->endpointUri = endpointUri;

    EdgeMessage *msg = (EdgeMessage *) calloc(1, sizeof(EdgeMessage));
    if(IS_NULL(msg))
    {
        printf("Error : Malloc failed for EdgeMessage in test browse\n");
        goto EXIT_BROWSE;
    }
    msg->type = SEND_REQUEST;
    msg->endpointInfo = ep;
    msg->command = CMD_BROWSE;

    printf("\n\n" COLOR_YELLOW "********** Browse Views under RootFolder node in system namespace **********"
           COLOR_RESET "\n");

    initBrowseNextData(msg->browseParam);

    browseViews(msg);

    EXIT_BROWSE:
    FREE(ep);
    FREE(msg);
}


static void testBrowse(char* endpointUri)
{
    printf("\n" COLOR_YELLOW "------------------------------------------------------" COLOR_RESET);
    printf("\n" COLOR_YELLOW "                       Browse Node            "COLOR_RESET);
    printf("\n" COLOR_YELLOW "------------------------------------------------------" COLOR_RESET
           "\n\n");

    EdgeEndPointInfo *ep = (EdgeEndPointInfo *) calloc(1, sizeof(EdgeEndPointInfo));
    if(IS_NULL(ep))
    {
        printf("Error : Malloc failed for EdgeEndPointInfo in test browse\n");
        goto EXIT_BROWSE;
    }
    ep->endpointUri = endpointUri;

    EdgeMessage *msg = (EdgeMessage *) calloc(1, sizeof(EdgeMessage));
    if(IS_NULL(msg))
    {
        printf("Error : Malloc failed for EdgeMessage in test browse\n");
        goto EXIT_BROWSE;
    }
    msg->type = SEND_REQUEST;
    msg->endpointInfo = ep;
    msg->command = CMD_BROWSE;

#if TEST_WITH_REFERENCE_SERVER
    EdgeNodeInfo *nodeInfo = (EdgeNodeInfo *) calloc(1, sizeof(EdgeNodeInfo));
    if(IS_NULL(nodeInfo))
    {
        printf("Error : Malloc failed for nodeInfo in test browse\n");
        goto EXIT_BROWSE;
    }
    nodeInfo->nodeId = (EdgeNodeId *) calloc(1, sizeof(EdgeNodeId));
    if(IS_NULL(nodeInfo->nodeId))
    {
        printf("Error : Malloc failed for nodeInfo->nodeId in test browse\n");
        goto EXIT_BROWSE;
    }
    nodeInfo->nodeId->type = STRING;
    nodeInfo->nodeId->nodeId = "DataAccess_DataItem_Int16";
    nodeInfo->nodeId->nameSpace = 2;

    printf("\n\n" COLOR_YELLOW "********** Browse Int16 node in namespace 2 **********" COLOR_RESET
           "\n");

#else
    EdgeNodeInfo *nodeInfo = (EdgeNodeInfo *) calloc(1, sizeof(EdgeNodeInfo));
    if(IS_NULL(nodeInfo))
    {
        printf("Error : Malloc failed for nodeInfo in test browse\n");
        goto EXIT_BROWSE;
    }
    nodeInfo->nodeId = (EdgeNodeId *) calloc(1, sizeof(EdgeNodeId));
    if(IS_NULL(nodeInfo->nodeId))
    {
        printf("Error : Malloc failed for nodeInfo->nodeId in test browse\n");
        goto EXIT_BROWSE;
    }
    nodeInfo->nodeId->type = INTEGER;
    nodeInfo->nodeId->integerNodeId = RootFolder;
    nodeInfo->nodeId->nameSpace = SYSTEM_NAMESPACE_INDEX;

    printf("\n\n" COLOR_YELLOW "********** Browse RootFolder node in system namespace **********"
           COLOR_RESET "\n");
#endif
    EdgeRequest *request = (EdgeRequest *) calloc(1, sizeof(EdgeRequest));
    if(IS_NULL(request))
    {
        printf("Error : Malloc failed for request in test browse\n");
        goto EXIT_BROWSE;
    }
    request->nodeInfo = nodeInfo;

    msg->request = request;
    msg->requestLength = 0;
    msg->browseParam = (EdgeBrowseParameter *)calloc(1, sizeof(EdgeBrowseParameter));
    if(IS_NULL(msg->browseParam))
    {
        printf("Error : Malloc failed for msg->browseParam in test browse\n");
        goto EXIT_BROWSE;
    }
    msg->browseParam->direction = DIRECTION_FORWARD;
    msg->browseParam->maxReferencesPerNode = maxReferencesPerNode;

    initBrowseNextData(msg->browseParam);

    browseNode(msg);

    EXIT_BROWSE:
    if(IS_NOT_NULL(msg))
    {
        FREE(msg->browseParam);
    }
    FREE(request);
    if(IS_NOT_NULL(nodeInfo))
    {
        FREE(nodeInfo->nodeId);
        FREE(nodeInfo);
    }
    FREE(msg);
    FREE(ep);
}

static void testBrowses()
{
    printf("\n" COLOR_YELLOW "------------------------------------------------------" COLOR_RESET);
    printf("\n" COLOR_YELLOW "                       Browse Multiple Nodes            "COLOR_RESET);
    printf("\n" COLOR_YELLOW "------------------------------------------------------" COLOR_RESET
           "\n\n");
    EdgeEndPointInfo *ep = (EdgeEndPointInfo *) calloc(1, sizeof(EdgeEndPointInfo));
    if(IS_NULL(ep))
    {
        printf("Error : Malloc failed for EdgeEndPointInfo in test browses\n");
        goto EXIT_BROWSES;
    }
    ep->endpointUri = endpointUri;

    EdgeMessage *msg = (EdgeMessage *) calloc(1, sizeof(EdgeMessage));
    if(IS_NULL(msg))
    {
        printf("Error : Malloc failed for EdgeMessage in test browses\n");
        goto EXIT_BROWSES;
    }
    msg->type = SEND_REQUESTS;
    msg->endpointInfo = ep;
    msg->command = CMD_BROWSE;

#if TEST_WITH_REFERENCE_SERVER
    int requestLength = 2;
    EdgeRequest **requests = (EdgeRequest **) calloc(requestLength, sizeof(EdgeRequest *));
    if(IS_NULL(requests))
    {
        printf("Error : Malloc failed for requests in test browses\n");
        goto EXIT_BROWSES;
    }

    EdgeNodeInfo *nodeInfo1 = (EdgeNodeInfo *) calloc(1, sizeof(EdgeNodeInfo));
    if(IS_NULL(nodeInfo1))
    {
        printf("Error : Malloc failed for nodeInfo1 in test browses\n");
        goto EXIT_BROWSES;
    }
    nodeInfo1->nodeId = (EdgeNodeId *) calloc(1, sizeof(EdgeNodeId));
    if(IS_NULL(nodeInfo1->nodeId))
    {
        printf("Error : Malloc failed for nodeInfo1->nodeId in test browses\n");
        goto EXIT_BROWSES;
    }
    nodeInfo1->nodeId->type = STRING;
    nodeInfo1->nodeId->nodeId = "DataAccess_DataItem_Int16";
    nodeInfo1->nodeId->nameSpace = 2;

    EdgeNodeInfo *nodeInfo2 = (EdgeNodeInfo *) calloc(1, sizeof(EdgeNodeInfo));
    if(IS_NULL(nodeInfo2))
    {
        printf("Error : Malloc failed for nodeInfo2 in test browses\n");
        goto EXIT_BROWSES;
    }
    nodeInfo2->nodeId = (EdgeNodeId *) calloc(1, sizeof(EdgeNodeId));
    if(IS_NULL(nodeInfo2->nodeId))
    {
        printf("Error : Malloc failed for nodeInfo2->nodeId in test browses\n");
        goto EXIT_BROWSES;
    }
    nodeInfo2->nodeId->type = STRING;
    nodeInfo2->nodeId->nodeId = "DataAccess_AnalogType_SByte";
    nodeInfo2->nodeId->nameSpace = 2;

    requests[0] = (EdgeRequest *) calloc(1, sizeof(EdgeRequest));
    if(IS_NULL(requests[0]))
    {
        printf("Error : Malloc failed for requests[0] in test browses\n");
        goto EXIT_BROWSES;
    }
    requests[0]->nodeInfo = nodeInfo1;
    requests[1] = (EdgeRequest *) calloc(1, sizeof(EdgeRequest));
    if(IS_NULL(requests[1]))
    {
        printf("Error : Malloc failed for requests[1] in test browses\n");
        goto EXIT_BROWSES;
    }
    requests[1]->nodeInfo = nodeInfo2;

    printf("\n\n" COLOR_YELLOW "********** Browse Int16 & SByte nodes in namespace 2 **********"
           COLOR_RESET "\n");
#else
    int requestLength = 3;
    EdgeRequest **requests = (EdgeRequest **) calloc(requestLength, sizeof(EdgeRequest *));
    if(IS_NULL(requests))
    {
        printf("Error : Malloc failed for requests in test browses\n");
        goto EXIT_BROWSES;
    }

    EdgeNodeInfo *nodeInfo1 = (EdgeNodeInfo *) calloc(1, sizeof(EdgeNodeInfo));
    if(IS_NULL(nodeInfo1))
    {
        printf("Error : Malloc failed for nodeInfo1 in test browses\n");
        goto EXIT_BROWSES;
    }
    nodeInfo1->nodeId = (EdgeNodeId *) calloc(1, sizeof(EdgeNodeId));
    if(IS_NULL(nodeInfo1->nodeId))
    {
        printf("Error : Malloc failed for nodeInfo1->nodeId in test browses\n");
        goto EXIT_BROWSES;
    }
    nodeInfo1->nodeId->type = INTEGER;
    nodeInfo1->nodeId->integerNodeId = RootFolder;
    nodeInfo1->nodeId->nameSpace = SYSTEM_NAMESPACE_INDEX;

    EdgeNodeInfo *nodeInfo2 = (EdgeNodeInfo *) calloc(1, sizeof(EdgeNodeInfo));
    if(IS_NULL(nodeInfo2))
    {
        printf("Error : Malloc failed for nodeInfo2 in test browses\n");
        goto EXIT_BROWSES;
    }
    nodeInfo2->nodeId = (EdgeNodeId *) calloc(1, sizeof(EdgeNodeId));
    if(IS_NULL(nodeInfo2->nodeId))
    {
        printf("Error : Malloc failed for nodeInfo2->nodeId in test browses\n");
        goto EXIT_BROWSES;
    }
    nodeInfo2->nodeId->type = INTEGER;
    nodeInfo2->nodeId->integerNodeId = ObjectsFolder;
    nodeInfo2->nodeId->nameSpace = SYSTEM_NAMESPACE_INDEX;

    EdgeNodeInfo *nodeInfo3 = (EdgeNodeInfo *) calloc(1, sizeof(EdgeNodeInfo));
    if(IS_NULL(nodeInfo3))
    {
        printf("Error : Malloc failed for nodeInfo3 in test browses\n");
        goto EXIT_BROWSES;
    }
    nodeInfo3->nodeId = (EdgeNodeId *) calloc(1, sizeof(EdgeNodeId));
    if(IS_NULL(nodeInfo3->nodeId))
    {
        printf("Error : Malloc failed for nodeInfo3->nodeId in test browses\n");
        goto EXIT_BROWSES;
    }
    nodeInfo3->nodeId->type = STRING;
    nodeInfo3->nodeId->nodeId = "Object1";
    nodeInfo3->nodeId->nameSpace = 2;

    requests[0] = (EdgeRequest *) calloc(1, sizeof(EdgeRequest));
    if(IS_NULL(requests[0]))
    {
        printf("Error : Malloc failed for requests[0] in test browses\n");
        goto EXIT_BROWSES;
    }
    requests[0]->nodeInfo = nodeInfo1;
    requests[1] = (EdgeRequest *) calloc(1, sizeof(EdgeRequest));
    if(IS_NULL(requests[1]))
    {
        printf("Error : Malloc failed for requests[1] in test browses\n");
        goto EXIT_BROWSES;
    }
    requests[1]->nodeInfo = nodeInfo2;
    requests[2] = (EdgeRequest *) calloc(1, sizeof(EdgeRequest));
    if(IS_NULL(requests[2]))
    {
        printf("Error : Malloc failed for requests[2] in test browses\n");
        goto EXIT_BROWSES;
    }
    requests[2]->nodeInfo = nodeInfo3;

    printf("\n\n" COLOR_YELLOW
           "********** Browse RootFolder, ObjectsFolder nodes in system namespace and Object1 in namespace 1 **********"
           COLOR_RESET "\n");
#endif

    msg->requests = requests;
    msg->requestLength = requestLength;
    msg->browseParam = (EdgeBrowseParameter *)calloc(1, sizeof(EdgeBrowseParameter));
    if(IS_NULL(msg->browseParam))
    {
        printf("Error : Malloc failed for msg->browseParam in test browses\n");
        goto EXIT_BROWSES;
    }
    msg->browseParam->direction = DIRECTION_FORWARD;
    msg->browseParam->maxReferencesPerNode = maxReferencesPerNode;

    initBrowseNextData(msg->browseParam);

    browseNode(msg);

    EXIT_BROWSES:
    if(IS_NOT_NULL(msg))
    {
        FREE(msg->browseParam);
    }
    if(IS_NOT_NULL(nodeInfo1))
    {
        FREE(nodeInfo1->nodeId);
        FREE(nodeInfo1);
    }
    if(IS_NOT_NULL(nodeInfo2))
    {
        FREE(nodeInfo2->nodeId);
        FREE(nodeInfo2);
    }
    if(IS_NOT_NULL(requests))
    {
        FREE(requests[0]);
        FREE(requests[1]);
    }
#if !TEST_WITH_REFERENCE_SERVER
    if(IS_NOT_NULL(nodeInfo3))
    {
        FREE(nodeInfo3->nodeId);
        FREE(nodeInfo3);
    }
    if(IS_NOT_NULL(requests))
    {
        FREE(requests[2]);
    }
#endif
    FREE(requests);
    FREE(ep);
    FREE(msg);
}

static void testRead()
{
    char *ep = getEndPoint_input();
    if (ep == NULL)
    {
        printf("Client not connected to any endpoints\n\n");
        return ;
    }

    // Get the list of browse names and display them to user.
    testBrowse(ep);

    char nodeName[MAX_CHAR_SIZE];
    int num_requests = 1;

    EdgeEndPointInfo *epInfo = (EdgeEndPointInfo *) calloc(1, sizeof(EdgeEndPointInfo));
    if(IS_NULL(epInfo))
    {
        printf("Error : Malloc failed for epInfo in test read\n");
        goto EXIT_READ;
    }
    epInfo->endpointUri = ep;

    EdgeNodeInfo **nodeInfo = (EdgeNodeInfo **) malloc(sizeof(EdgeNodeInfo *) * num_requests);
    if(IS_NULL(nodeInfo))
    {
        printf("Error : Malloc failed for nodeInfo in test read\n");
        goto EXIT_READ;
    }
    for (int i = 0; i < num_requests; i++)
    {
        printf("\nEnter the node #%d name to read :: ", (i + 1));
        nodeInfo[i] = (EdgeNodeInfo *) malloc(sizeof(EdgeNodeInfo));
        if(IS_NULL(nodeInfo[i]))
        {
            printf("Error : Malloc failed for nodeInfo[%d] in test read\n", i);
            goto EXIT_READ;
        }
        scanf("%s", nodeName);
        nodeInfo[i]->valueAlias = (char *) malloc(strlen(nodeName) + 1);
        if(IS_NULL(nodeInfo[i]->valueAlias))
        {
            printf("Error : Malloc failed for nodeInfo[%d]->valueAlias in test read\n", i);
            goto EXIT_READ;
        }
        strncpy(nodeInfo[i]->valueAlias, nodeName, strlen(nodeName));
        nodeInfo[i]->valueAlias[strlen(nodeName)] = '\0';
    }

    EdgeRequest **requests = (EdgeRequest **) malloc(sizeof(EdgeRequest *) * num_requests);
    if(IS_NULL(requests))
    {
        printf("Error : Malloc failed for requests in test read\n");
        goto EXIT_READ;
    }
    for (int i = 0; i < num_requests; i++)
    {
        requests[i] = (EdgeRequest *) malloc(sizeof(EdgeRequest));
        if(IS_NULL(requests[i]))
        {
            printf("Error : Malloc failed for requests[%d] in test read\n", i);
            goto EXIT_READ;
        }
        requests[i]->nodeInfo = nodeInfo[i];
    }

    EdgeMessage *msg = (EdgeMessage *) malloc(sizeof(EdgeMessage));
    if(IS_NULL(msg))
    {
        printf("Error : Malloc failed for EdgeMessage in test read\n");
        goto EXIT_READ;
    }
    msg->endpointInfo = epInfo;
    msg->command = CMD_READ;
    msg->type = SEND_REQUESTS;
    msg->requests = requests;
    msg->requestLength = num_requests;

    readNode(msg);

    EXIT_READ:
    if(IS_NOT_NULL(nodeInfo))
    {
        for (int i = 0; i < num_requests; i++)
        {
            if(IS_NOT_NULL(nodeInfo[i]))
            {
                FREE(nodeInfo[i]->valueAlias);
                FREE (nodeInfo[i]);
            }
        }
        FREE (nodeInfo);
    }
    if(IS_NOT_NULL(requests))
    {
        for (int i = 0; i < num_requests; i++)
        {
            FREE (requests[i]);
        }
        FREE(requests);
    }

    FREE(epInfo);
    FREE(msg);
}

static void testReadGroup()
{
    char *ep = getEndPoint_input();
    if (ep == NULL)
    {
        printf("Client not connected to any endpoints\n\n");
        return ;
    }

    char nodeName[MAX_CHAR_SIZE];
    int num_requests;
    printf("Enter number of nodes to read (less than 10) :: ");
    scanf("%d", &num_requests);
    if(num_requests < 1 || num_requests > 9)
    {
        printf("Error : Invalid input for number of nodes to read\n");
        return ;
    }

    EdgeEndPointInfo *epInfo = (EdgeEndPointInfo *) calloc(1, sizeof(EdgeEndPointInfo));
    if(IS_NULL(epInfo))
    {
        printf("Error : Malloc failed for epInfo in test read group\n");
        goto EXIT_READGROUP;
    }
    epInfo->endpointUri = ep;

    EdgeNodeInfo **nodeInfo = (EdgeNodeInfo **) malloc(sizeof(EdgeNodeInfo *) * num_requests);
    if(IS_NULL(epInfo))
    {
        printf("Error : Malloc failed for nodeInfo in test read group\n");
        goto EXIT_READGROUP;
    }
    EdgeRequest **requests = (EdgeRequest **) malloc(sizeof(EdgeRequest *) * num_requests);
    if(IS_NULL(requests))
    {
        printf("Error : Malloc failed for requests in test read group\n");
        goto EXIT_READGROUP;
    }

    for (int i = 0; i < num_requests; i++)
    {
        printf("\nEnter the node #%d  name to read :: ", (i + 1));
        nodeInfo[i] = (EdgeNodeInfo *) malloc(sizeof(EdgeNodeInfo));
        if(IS_NULL(nodeInfo[i]))
        {
            printf("Error : Malloc failed for nodeInfo[%d] in test read group\n", i);
            goto EXIT_READGROUP;
        }
        scanf("%s", nodeName);
        nodeInfo[i]->valueAlias = (char *) malloc(strlen(nodeName) + 1);
        if(IS_NULL(nodeInfo[i]->valueAlias))
        {
            printf("Error : Malloc failed for nodeInfo[%d]->valueAlias in test read group\n", i);
            goto EXIT_READGROUP;
        }
        strncpy(nodeInfo[i]->valueAlias, nodeName, strlen(nodeName));
        nodeInfo[i]->valueAlias[strlen(nodeName)] = '\0';

        requests[i] = (EdgeRequest *) malloc(sizeof(EdgeRequest));
        if(IS_NULL(requests[i]))
        {
            printf("Error : Malloc failed for requests[%d] in test read group\n", i);
            goto EXIT_READGROUP;
        }
        requests[i]->nodeInfo = nodeInfo[i];
    }

    EdgeMessage *msg = (EdgeMessage *) malloc(sizeof(EdgeMessage));
    if(IS_NULL(msg))
    {
        printf("Error : Malloc failed for EdgeMessage in test read group\n");
        goto EXIT_READGROUP;
    }
    msg->endpointInfo = epInfo;
    msg->command = CMD_READ;
    msg->type = SEND_REQUESTS;
    msg->requests = requests;
    msg->requestLength = num_requests;

    readNode(msg);

    EXIT_READGROUP:
    if(IS_NOT_NULL(nodeInfo))
    {
        for (int i = 0; i < num_requests; i++)
        {
            if(IS_NOT_NULL(nodeInfo[i]))
            {
                FREE(nodeInfo[i]->valueAlias);
                FREE (nodeInfo[i]);
            }
        }
        FREE (nodeInfo);
    }
    if(IS_NOT_NULL(requests))
    {
        for (int i = 0; i < num_requests; i++)
        {
            FREE (requests[i]);
        }
        FREE(requests);
    }

    FREE(epInfo);
    FREE(msg);
}

static int getInputType()
{
    int type;

    printf("\n1) Boolean\n");
    printf("2) SByte\n");
    printf("3) Byte\n");
    printf("4) Int16\n");
    printf("5) UInt16\n");
    printf("6) Int32\n");
    printf("7) UInt32\n");
    printf("8) Int64\n");
    printf("9) UInt64\n");
    printf("10) Float\n");
    printf("11) Double\n");
    printf("12) String\n\n");

    printf("Enter the type of the data to write (integer option only) :: ");
    scanf("%d", &type);

    return type;
}

static void *getNewValuetoWrite(EdgeNodeIdentifier type)
{
    printf("Enter the new value to write :: ");
    switch (type)
    {
        case Boolean:
            {
                int *val = (int *) malloc(sizeof(int));
                scanf("%d", val);
                return (void *) val;
            }
            break;
        case SByte:
            {
                int8_t *val = (int8_t *) malloc(sizeof(int8_t));
                scanf("%" SCNd8, val);
                return (void *) val;
            }
        case Byte:
            {
                uint8_t *val = (uint8_t *) malloc(sizeof(uint8_t));
                scanf("%" SCNu8, val);
                return (void *) val;
            }
            break;
        case Int16:
            {
                int16_t *val = (int16_t *) malloc(sizeof(int16_t));
                scanf("%" SCNd16, val);
                return (void *) val;
            }
        case UInt16:
            {
                uint16_t *val = (uint16_t *) malloc(sizeof(uint16_t));
                scanf("%" SCNu16, val);
                return (void *) val;
            }
            break;
        case Int32:
            {
                int32_t *val = (int32_t *) malloc(sizeof(int32_t));
                scanf("%" SCNd32, val);
                return (void *) val;
            }
            break;
        case UInt32:
            {
                uint32_t *val = (uint32_t *) malloc(sizeof(uint32_t));
                scanf("%" SCNu32, val);
                return (void *) val;
            }
            break;
        case Int64:
            {
                int64_t *val = (int64_t *) malloc(sizeof(int64_t));
                scanf("%" SCNd64, val);
                return (void *) val;
            }
            break;
        case UInt64:
            {
                uint64_t *val = (uint64_t *) malloc(sizeof(uint64_t));
                scanf("%" SCNu64, val);
                return (void *) val;
            }
            break;
        case Float:
            {
                float *val = (float *) malloc(sizeof(float));
                scanf("%g", val);
                return (void *) val;
            }
            break;
        case Double:
            {
                double *val = (double *) malloc(sizeof(double));
                scanf("%lf", val);
                return (void *) val;
            }
            break;
        case String:
            {
                char val[MAX_CHAR_SIZE];
                scanf("%s", val);
                char *retStr = (char *) malloc(strlen(val) + 1);
                strncpy(retStr, val, strlen(val));
                retStr[strlen(val)] = '\0';
                return (void *) retStr;
            }
            break;
        default:
            break;
    }

    return NULL;
}

static void testWrite()
{
    char *ep = getEndPoint_input();
    if (ep == NULL)
    {
        printf("Client not connected to any endpoints\n\n");
        return ;
    }

    char nodeName[MAX_CHAR_SIZE];
    int num_requests = 1;

    EdgeEndPointInfo *epInfo = (EdgeEndPointInfo *) calloc(1, sizeof(EdgeEndPointInfo));
    if(IS_NULL(epInfo))
    {
        printf("Error : Malloc failed for epInfo in test write\n");
        goto EXIT_WRITE;
    }
    epInfo->endpointUri = ep;

    EdgeNodeInfo **nodeInfo = (EdgeNodeInfo **) malloc(sizeof(EdgeNodeInfo *));
    if(IS_NULL(nodeInfo))
    {
        printf("Error : Malloc failed for EdgeNodeInfo in test write\n");
        goto EXIT_WRITE;
    }
    EdgeRequest **requests = (EdgeRequest **) malloc(sizeof(EdgeRequest *) * num_requests);
    if(IS_NULL(requests))
    {
        printf("Error : Malloc failed for EdgeRequest in test write\n");
        goto EXIT_WRITE;
    }
    for (int i = 0; i < num_requests; i++)
    {
        printf("\nEnter the node #%d name to write :: ", (i + 1));
        nodeInfo[i] = (EdgeNodeInfo *) malloc(sizeof(EdgeNodeInfo));
        if(IS_NULL(nodeInfo[i]))
        {
            printf("Error : Malloc failed for nodeInfo[%d] in test write\n", i);
            goto EXIT_WRITE;
        }
        scanf("%s", nodeName);
        nodeInfo[i]->valueAlias = (char *) malloc(strlen(nodeName) + 1);
        if(IS_NULL(nodeInfo[i]->valueAlias))
        {
            printf("Error : Malloc failed for nodeInfo[%d]->valueAlias in test write\n", i);
            goto EXIT_WRITE;
        }
        strncpy(nodeInfo[i]->valueAlias, nodeName, strlen(nodeName));
        nodeInfo[i]->valueAlias[strlen(nodeName)] = '\0';

        requests[i] = (EdgeRequest *) malloc(sizeof(EdgeRequest));
        if(IS_NULL(requests[i]))
        {
            printf("Error : Malloc failed for requests[%d] in test write\n", i);
            goto EXIT_WRITE;
        }
        requests[i]->nodeInfo = nodeInfo[i];
        requests[i]->type = getInputType();
        requests[i]->value = getNewValuetoWrite(requests[i]->type);
    }

    EdgeMessage *msg = (EdgeMessage *) malloc(sizeof(EdgeMessage));
    if(IS_NULL(msg))
    {
        printf("Error : Malloc failed for EdgeMessage in test write\n");
        goto EXIT_WRITE;
    }
    msg->endpointInfo = epInfo;
    msg->command = CMD_WRITE;
    msg->type = SEND_REQUESTS;
    msg->requests = requests;
    msg->requestLength = num_requests;

    printf("write node \n");
    writeNode(msg);
    printf("write node call success \n");

    EXIT_WRITE:
    if(IS_NOT_NULL(nodeInfo))
    {
        for (int i = 0; i < num_requests; i++)
        {
            if(IS_NOT_NULL(nodeInfo[i]))
            {
                FREE(nodeInfo[i]->valueAlias);
            }
            FREE (nodeInfo[i]);
        }
    }
    if(IS_NOT_NULL(requests))
    {
        for (int i = 0; i < num_requests; i++)
        {
            if(IS_NOT_NULL(requests[i]))
            {
                FREE (requests[i]->value);
            }
            FREE (requests[i]);
        }
    }

    FREE (nodeInfo);
    FREE(epInfo);
    FREE(requests);
    FREE(msg);
}

static void testWriteGroup()
{
    char *ep = getEndPoint_input();
    if (ep == NULL)
    {
        printf("Client not connected to any endpoints\n\n");
        return ;
    }

    char nodeName[MAX_CHAR_SIZE];
    int num_requests;
    printf("Enter number of nodes to write (less than 10) :: ");
    scanf("%d", &num_requests);
    if(num_requests < 1 || num_requests > 9)
    {
        printf("Error : Invalid input for number of nodes to Write\n");
        return ;
    }

    EdgeEndPointInfo *epInfo = (EdgeEndPointInfo *) calloc(1, sizeof(EdgeEndPointInfo));
    if(IS_NULL(epInfo))
    {
        printf("Error : Malloc failed for epInfo in test write group\n");
        goto EXIT_WRITEGROUP;
    }
    epInfo->endpointUri = ep;

    EdgeNodeInfo **nodeInfo = (EdgeNodeInfo **) malloc(sizeof(EdgeNodeInfo *) * num_requests);
    if(IS_NULL(nodeInfo))
    {
        printf("Error : Malloc failed for nodeInfo in test write group\n");
        goto EXIT_WRITEGROUP;
    }
    EdgeRequest **requests = (EdgeRequest **) malloc(sizeof(EdgeRequest *) * num_requests);
    if(IS_NULL(requests))
    {
        printf("Error : Malloc failed for requests in test write group\n");
        goto EXIT_WRITEGROUP;
    }
    for (int i = 0; i < num_requests; i++)
    {
        printf("\nEnter the node #%d name to write :: ", (i + 1));
        nodeInfo[i] = (EdgeNodeInfo *) malloc(sizeof(EdgeNodeInfo));
        if(IS_NULL(nodeInfo[i]))
        {
            printf("Error : Malloc failed for nodeInfo[%d] in test write group\n", i);
            goto EXIT_WRITEGROUP;
        }
        scanf("%s", nodeName);
        nodeInfo[i]->valueAlias = (char *) malloc(strlen(nodeName) + 1);
        if(IS_NULL(nodeInfo[i]->valueAlias))
        {
            printf("Error : Malloc failed for nodeInfo[%d]->valueAlias in test write group\n", i);
            goto EXIT_WRITEGROUP;
        }
        strncpy(nodeInfo[i]->valueAlias, nodeName, strlen(nodeName));
        nodeInfo[i]->valueAlias[strlen(nodeName)] = '\0';

        requests[i] = (EdgeRequest *) malloc(sizeof(EdgeRequest));
        if(IS_NULL(requests[i]))
        {
            printf("Error : Malloc failed for requests[%d] in test write group\n", i);
            goto EXIT_WRITEGROUP;
        }
        if(IS_NULL(epInfo))
        {
            printf("Error : Malloc failed for epInfo in test write group\n");
            goto EXIT_WRITEGROUP;
        }
        requests[i]->nodeInfo = nodeInfo[i];
        requests[i]->type = getInputType();
        requests[i]->value = getNewValuetoWrite(requests[i]->type);
    }

    EdgeMessage *msg = (EdgeMessage *) malloc(sizeof(EdgeMessage));
    if(IS_NULL(msg))
    {
        printf("Error : Malloc failed for EdgeMessage in test write group\n");
        goto EXIT_WRITEGROUP;
    }
    msg->endpointInfo = epInfo;
    msg->command = CMD_WRITE;
    msg->type = SEND_REQUESTS;
    msg->requests = requests;
    msg->requestLength = num_requests;

    writeNode(msg);

    EXIT_WRITEGROUP:
    if(IS_NOT_NULL(requests))
    {
        for (int i = 0; i < num_requests; i++)
        {
            if(IS_NOT_NULL(requests[i]))
            {
                FREE (requests[i]->value);
            }
            FREE (requests[i]);
        }
    }
    if(IS_NOT_NULL(nodeInfo))
    {
        for (int i = 0; i < num_requests; i++)
        {
            if(IS_NOT_NULL(nodeInfo[i]))
            {
                FREE(nodeInfo[i]->valueAlias);
            }
            FREE (nodeInfo[i]);
        }
    }

    FREE (nodeInfo);
    FREE(requests);
    FREE(epInfo);
    FREE(msg);
}

static void testMethod()
{
    printf("\n" COLOR_YELLOW "------------------------------------------------------" COLOR_RESET);
    printf("\n" COLOR_YELLOW "                       Method Call            "COLOR_RESET);
    printf("\n" COLOR_YELLOW "------------------------------------------------------" COLOR_RESET
           "\n\n");

    printf("\n|------------------- [Method Call] - sqrt(x) \n");

    EdgeEndPointInfo *ep = (EdgeEndPointInfo *) calloc(1, sizeof(EdgeEndPointInfo));
    if(IS_NULL(ep))
    {
        printf("Error : Malloc failed for ep in test Method\n");
        goto EXIT_METHOD;
    }
    ep->endpointUri = endpointUri;

    EdgeNodeInfo *nodeInfo = (EdgeNodeInfo *) malloc(sizeof(EdgeNodeInfo));
    if(IS_NULL(nodeInfo))
    {
        printf("Error : Malloc failed for nodeInfo in test Method\n");
        goto EXIT_METHOD;
    }
    nodeInfo->valueAlias = "sqrt(x)";

    EdgeMethodRequestParams *methodParams = (EdgeMethodRequestParams *) malloc(sizeof(
            EdgeMethodRequestParams));
    if(IS_NULL(methodParams))
    {
        printf("Error : Malloc failed for methodParams in test Method\n");
        goto EXIT_METHOD;
    }
    methodParams->num_inpArgs = 1;
    methodParams->inpArg = (EdgeArgument **) malloc(sizeof(EdgeArgument *) * methodParams->num_inpArgs);
    if(IS_NULL(methodParams->inpArg))
    {
        printf("Error : Malloc failed for methodParams->inpArg in test Method\n");
        goto EXIT_METHOD;
    }
    methodParams->inpArg[0] = (EdgeArgument *) malloc(sizeof(EdgeArgument));
    if(IS_NULL(methodParams->inpArg[0]))
    {
        printf("Error : Malloc failed for methodParams->inpArg[0] in test Method\n");
        goto EXIT_METHOD;
    }
    methodParams->inpArg[0]->argType = Double;
    methodParams->inpArg[0]->valType = SCALAR;
    double input = 16.0;
    methodParams->inpArg[0]->scalarValue = (void *) &input;

    printf("input(x) :: [%.2f]\n", input);

    EdgeRequest *request = (EdgeRequest *) malloc(sizeof(EdgeRequest));
    if(IS_NULL(request))
    {
        printf("Error : Malloc failed for request in test Method\n");
        goto EXIT_METHOD;
    }
    request->nodeInfo = nodeInfo;
    request->methodParams = methodParams;

    EdgeMessage *msg = (EdgeMessage *) malloc(sizeof(EdgeMessage));
    if(IS_NULL(msg))
    {
        printf("Error : Malloc failed for EdgeMessage in test Method\n");
        goto EXIT_METHOD;
    }
    msg->endpointInfo = ep;
    msg->command = CMD_METHOD;
    msg->request = request;

    callMethod(msg);

    EXIT_METHOD:
    if(IS_NOT_NULL(methodParams))
    {
        if(IS_NOT_NULL(methodParams->inpArg))
        {
            for (int i = 0; i < methodParams->num_inpArgs; i++)
            {
                FREE (methodParams->inpArg[i]);
            }
        }
        FREE (methodParams->inpArg);
        FREE (methodParams);
    }

    FREE(nodeInfo);
    FREE(request);
    FREE(ep);
    FREE (msg);

    printf("\n|------------------- [Method Call ] - incrementInc32Array(x,delta) \n");

    ep = (EdgeEndPointInfo *) calloc(1, sizeof(EdgeEndPointInfo));
    if(IS_NULL(ep))
    {
        printf("Error : Malloc failed for EdgeEndPointInfo in test Method1\n");
        goto EXIT_METHOD1;
    }
    ep->endpointUri = endpointUri;

    nodeInfo = (EdgeNodeInfo *) malloc(sizeof(EdgeNodeInfo));
    if(IS_NULL(nodeInfo))
    {
        printf("Error : Malloc failed for EdgeNodeInfo in test Method1\n");
        goto EXIT_METHOD1;
    }
    nodeInfo->valueAlias = "incrementInc32Array(x,delta)";

    methodParams = (EdgeMethodRequestParams *) malloc(sizeof(EdgeMethodRequestParams));
    if(IS_NULL(methodParams))
    {
        printf("Error : Malloc failed for methodParams in test Method1\n");
        goto EXIT_METHOD1;
    }
    methodParams->num_inpArgs = 2;
    methodParams->inpArg = (EdgeArgument **) malloc(sizeof(EdgeArgument *) * methodParams->num_inpArgs);
    if(IS_NULL(methodParams->inpArg))
    {
        printf("Error : Malloc failed for methodParams->inpArg in test Method1\n");
        goto EXIT_METHOD1;
    }
    methodParams->inpArg[0] = (EdgeArgument *) malloc(sizeof(EdgeArgument));
    if(IS_NULL(methodParams->inpArg[0]))
    {
        printf("Error : Malloc failed for methodParams->inpArg[0] in test Method1\n");
        goto EXIT_METHOD1;
    }
    methodParams->inpArg[0]->argType = Int32;
    methodParams->inpArg[0]->valType = ARRAY_1D;
    int32_t array[5] = {10, 20, 30, 40, 50};
    methodParams->inpArg[0]->arrayData = (void *) array;
    methodParams->inpArg[0]->arrayLength = 5;

    methodParams->inpArg[1] = (EdgeArgument *) malloc(sizeof(EdgeArgument));
    if(IS_NULL(methodParams->inpArg[1]))
    {
        printf("Error : Malloc failed for methodParams->inpArg[1] in test Method1\n");
        goto EXIT_METHOD1;
    }
    methodParams->inpArg[1]->argType = Int32;
    methodParams->inpArg[1]->valType = SCALAR;
    int delta = 5;
    methodParams->inpArg[1]->scalarValue = (void *) &delta;

    printf("input(x, delta) :: [%d %d %d %d %d] [%d]\n",
            array[0], array[1], array[2], array[3], array[4], delta);

    request = (EdgeRequest *) malloc(sizeof(EdgeRequest));
    if(IS_NULL(request))
    {
        printf("Error : Malloc failed for request in test Method1\n");
        goto EXIT_METHOD1;
    }
    request->nodeInfo = nodeInfo;
    request->methodParams = methodParams;

    msg = (EdgeMessage *) malloc(sizeof(EdgeMessage));
    if(IS_NULL(msg))
    {
        printf("Error : Malloc failed for msg in test Method1\n");
        goto EXIT_METHOD1;
    }
    msg->endpointInfo = ep;
    msg->command = CMD_METHOD;
    msg->request = request;

    callMethod(msg);

    EXIT_METHOD1:
    if(IS_NOT_NULL(methodParams))
    {
        if(IS_NOT_NULL(methodParams->inpArg))
        {
            for (int i = 0; i < methodParams->num_inpArgs; i++)
            {
                FREE (methodParams->inpArg[i]);
            }
        }
        FREE (methodParams->inpArg);
        FREE (methodParams);
    }

    FREE(nodeInfo);
    FREE(request);
    FREE(ep);
    FREE (msg);

    printf("\n|------------------- [Method Call ] - print(x) \n");

    ep = (EdgeEndPointInfo *) calloc(1, sizeof(EdgeEndPointInfo));
    if(IS_NULL(ep))
    {
        printf("Error : Malloc failed for EdgeEndPointInfo in test Method1\n");
        goto EXIT_METHOD2;
    }
    ep->endpointUri = endpointUri;

    nodeInfo = (EdgeNodeInfo *) malloc(sizeof(EdgeNodeInfo));
    if(IS_NULL(nodeInfo))
    {
        printf("Error : Malloc failed for EdgeNodeInfo in test Method1\n");
        goto EXIT_METHOD2;
    }
    nodeInfo->valueAlias = "print(x)";

    methodParams = (EdgeMethodRequestParams *) malloc(sizeof(EdgeMethodRequestParams));
    if(IS_NULL(methodParams))
    {
        printf("Error : Malloc failed for methodParams in test Method1\n");
        goto EXIT_METHOD2;
    }
    methodParams->num_inpArgs = 1;
    methodParams->inpArg = (EdgeArgument **) malloc(sizeof(EdgeArgument *) * methodParams->num_inpArgs);
    if(IS_NULL(methodParams->inpArg))
    {
        printf("Error : Malloc failed for methodParams->inpArg in test Method1\n");
        goto EXIT_METHOD2;
    }
    methodParams->inpArg[0] = (EdgeArgument *) malloc(sizeof(EdgeArgument));
    if(IS_NULL(methodParams->inpArg[0]))
    {
        printf("Error : Malloc failed for methodParams->inpArg[0] in test Method1\n");
        goto EXIT_METHOD2;
    }
    methodParams->inpArg[0]->argType = Int32;
    methodParams->inpArg[0]->valType = SCALAR;
    int32_t val = 100;
    methodParams->inpArg[0]->scalarValue = &val;

    printf("input(x) :: [%d]\n", val);

    request = (EdgeRequest *) malloc(sizeof(EdgeRequest));
    if(IS_NULL(request))
    {
        printf("Error : Malloc failed for request in test Method1\n");
        goto EXIT_METHOD2;
    }
    request->nodeInfo = nodeInfo;
    request->methodParams = methodParams;

    msg = (EdgeMessage *) malloc(sizeof(EdgeMessage));
    if(IS_NULL(msg))
    {
        printf("Error : Malloc failed for msg in test Method1\n");
        goto EXIT_METHOD2;
    }
    msg->endpointInfo = ep;
    msg->command = CMD_METHOD;
    msg->request = request;

    callMethod(msg);

    EXIT_METHOD2:
    if(IS_NOT_NULL(methodParams))
    {
        if(IS_NOT_NULL(methodParams->inpArg))
        {
            for (int i = 0; i < methodParams->num_inpArgs; i++)
            {
                FREE (methodParams->inpArg[i]);
            }
        }
        FREE (methodParams->inpArg);
        FREE (methodParams);
    }

    FREE(nodeInfo);
    FREE(request);
    FREE(ep);
    FREE (msg);

    printf("\n|------------------- [Method Call ] - shutdown() \n");

    ep = (EdgeEndPointInfo *) calloc(1, sizeof(EdgeEndPointInfo));
    if(IS_NULL(ep))
    {
        printf("Error : Malloc failed for EdgeEndPointInfo in test Method1\n");
        goto EXIT_METHOD3;
    }
    ep->endpointUri = endpointUri;

    nodeInfo = (EdgeNodeInfo *) malloc(sizeof(EdgeNodeInfo));
    if(IS_NULL(nodeInfo))
    {
        printf("Error : Malloc failed for EdgeNodeInfo in test Method1\n");
        goto EXIT_METHOD3;
    }
    nodeInfo->valueAlias = "shutdown()";

    methodParams = (EdgeMethodRequestParams *) malloc(sizeof(EdgeMethodRequestParams));
    if(IS_NULL(methodParams))
    {
        printf("Error : Malloc failed for methodParams in test Method1\n");
        goto EXIT_METHOD3;
    }
    methodParams->num_inpArgs = 0;

    request = (EdgeRequest *) malloc(sizeof(EdgeRequest));
    if(IS_NULL(request))
    {
        printf("Error : Malloc failed for request in test Method1\n");
        goto EXIT_METHOD3;
    }
    request->nodeInfo = nodeInfo;
    request->methodParams = methodParams;

    msg = (EdgeMessage *) malloc(sizeof(EdgeMessage));
    if(IS_NULL(msg))
    {
        printf("Error : Malloc failed for msg in test Method1\n");
        goto EXIT_METHOD3;
    }
    msg->endpointInfo = ep;
    msg->command = CMD_METHOD;
    msg->request = request;

    callMethod(msg);

    EXIT_METHOD3:
    if(IS_NOT_NULL(methodParams))
    {
        if(IS_NOT_NULL(methodParams->inpArg))
        {
            for (int i = 0; i < methodParams->num_inpArgs; i++)
            {
                FREE (methodParams->inpArg[i]);
            }
        }
        FREE (methodParams->inpArg);
        FREE (methodParams);
    }

    FREE(nodeInfo);
    FREE(request);
    FREE(ep);
    FREE (msg);

    printf("\n|------------------- [Method Call ] - version() \n");

    ep = (EdgeEndPointInfo *) calloc(1, sizeof(EdgeEndPointInfo));
    if(IS_NULL(ep))
    {
        printf("Error : Malloc failed for EdgeEndPointInfo in test Method1\n");
        goto EXIT_METHOD4;
    }
    ep->endpointUri = endpointUri;

    nodeInfo = (EdgeNodeInfo *) malloc(sizeof(EdgeNodeInfo));
    if(IS_NULL(nodeInfo))
    {
        printf("Error : Malloc failed for EdgeNodeInfo in test Method1\n");
        goto EXIT_METHOD4;
    }
    nodeInfo->valueAlias = "version()";

    methodParams = (EdgeMethodRequestParams *) malloc(sizeof(EdgeMethodRequestParams));
    if(IS_NULL(methodParams))
    {
        printf("Error : Malloc failed for methodParams in test Method1\n");
        goto EXIT_METHOD4;
    }
    methodParams->num_inpArgs = 0;

    request = (EdgeRequest *) malloc(sizeof(EdgeRequest));
    if(IS_NULL(request))
    {
        printf("Error : Malloc failed for request in test Method1\n");
        goto EXIT_METHOD4;
    }
    request->nodeInfo = nodeInfo;
    request->methodParams = methodParams;

    msg = (EdgeMessage *) malloc(sizeof(EdgeMessage));
    if(IS_NULL(msg))
    {
        printf("Error : Malloc failed for msg in test Method1\n");
        goto EXIT_METHOD4;
    }
    msg->endpointInfo = ep;
    msg->command = CMD_METHOD;
    msg->request = request;

    callMethod(msg);

    EXIT_METHOD4:
    if(IS_NOT_NULL(methodParams))
    {
        if(IS_NOT_NULL(methodParams->inpArg))
        {
            for (int i = 0; i < methodParams->num_inpArgs; i++)
            {
                FREE (methodParams->inpArg[i]);
            }
        }
        FREE (methodParams->inpArg);
        FREE (methodParams);
    }

    FREE(nodeInfo);
    FREE(request);
    FREE(ep);
    FREE (msg);
}

static void testSub()
{

    printf("\n" COLOR_YELLOW "------------------------------------------------------" COLOR_RESET);
    printf("\n" COLOR_YELLOW "                    Subscribe Node             "COLOR_RESET);
    printf("\n" COLOR_YELLOW "------------------------------------------------------\n" COLOR_RESET);

    char *ep = getEndPoint_input();
    if (ep == NULL)
    {
        printf("Client not connected to any endpoints\n\n");
        return ;
    }

    // Get the list of browse names and display them to user.
    testBrowse(ep);
    char nodeName[MAX_CHAR_SIZE];
    int num_requests;
    printf("Enter number of nodes to Subscribe (less than 10) :: ");
    scanf("%d", &num_requests);
    if(num_requests > 10 || num_requests < 1)
    {
        printf("Error :: Invalid input. Try Again\n ");
        return;
    }


    EdgeEndPointInfo *epInfo = (EdgeEndPointInfo *) calloc(1, sizeof(EdgeEndPointInfo));
    if(IS_NULL(epInfo))
    {
        printf("Error : Malloc failed for epInfo in test subscription\n");
        goto EXIT_SUB;
    }
    epInfo->endpointUri = ep;

    EdgeRequest **requests = (EdgeRequest **) malloc(sizeof(EdgeRequest *) * num_requests);
    if(IS_NULL(requests))
    {
        printf("Error : Malloc failed for requests in test subscription\n");
        goto EXIT_SUB;
    }
    EdgeSubRequest *subReq = (EdgeSubRequest *) malloc(sizeof(EdgeSubRequest));
    if(IS_NULL(subReq))
    {
        printf("Error : Malloc failed for subReq in test subscription\n");
        goto EXIT_SUB;
    }

    double samplingInterval;
    printf("\nEnter number of sampling interval[millisecond] (minimum : 100ms) :: ");
    scanf("%lf", &samplingInterval);

    subReq->subType = Edge_Create_Sub;
    subReq->samplingInterval = samplingInterval;
    subReq->publishingInterval = 0.0;
    subReq->maxKeepAliveCount = (1 > (int) (
                                     ceil(10000.0 / subReq->publishingInterval))) ? 1 : (int) ceil(10000.0 / subReq->publishingInterval);
    subReq->lifetimeCount = 10000;  //subReq->maxKeepAliveCount * 6;
    subReq->maxNotificationsPerPublish = 1;
    subReq->publishingEnabled = true;
    subReq->priority = 0;
    subReq->queueSize = 50;


    for (int i = 0; i < num_requests; i++)
    {
        EdgeNodeInfo *nodeInfo = (EdgeNodeInfo *) malloc(sizeof(EdgeNodeInfo));
        if(IS_NULL(nodeInfo))
        {
            printf("Error : Malloc failed for nodeInfo in test subscription\n");
            goto EXIT_SUB;
        }
        printf("\nEnter the node #%d  name to subscribe :: ", (i + 1));
        scanf("%s", nodeName);
        nodeInfo->valueAlias = (char *) malloc(strlen(nodeName) + 1);
        if(IS_NULL(nodeInfo->valueAlias))
        {
            printf("Error : Malloc failed for nodeInfo->valueAlias in test subscription\n");
            goto EXIT_SUB;
        }
        strncpy(nodeInfo->valueAlias, nodeName, strlen(nodeName));
        nodeInfo->valueAlias[strlen(nodeName)] = '\0';
        requests[i] = (EdgeRequest *) malloc(sizeof(EdgeRequest));
        if(IS_NULL(requests[i]))
        {
            printf("Error : Malloc failed for requests[%d] in test subscription\n", i);
            goto EXIT_SUB;
        }
        requests[i]->nodeInfo = nodeInfo;
        requests[i]->subMsg = subReq;
    }

    EdgeMessage *msg = (EdgeMessage *) malloc(sizeof(EdgeMessage));
    if(IS_NULL(msg))
    {
        printf("Error : Malloc failed for msg in test subscription\n");
        goto EXIT_SUB;
    }
    msg->endpointInfo = epInfo;
    msg->command = CMD_SUB;
    msg->type = SEND_REQUESTS;
    msg->requests = requests;
    msg->requestLength = num_requests;

    EdgeResult result = handleSubscription(msg);
    if (result.code == STATUS_OK)
    {
        printf(COLOR_GREEN "\nSUBSCRPTION CREATE SUCCESSFULLY\n" COLOR_RESET);
    } else {
        printf("CREATE RESULT : %d\n",  result.code);
    }

    EXIT_SUB:
    for (int i = 0; i < num_requests; i++)
    {
        if(IS_NOT_NULL(requests[i]))
        {
            if(IS_NOT_NULL(requests[i]->nodeInfo))
            {
                FREE(requests[i]->nodeInfo->valueAlias);
            }
            FREE(requests[i]->nodeInfo);
            FREE(requests[i]);
        }
    }
    FREE(subReq);
    FREE(requests);
    FREE(epInfo);
    FREE (msg);
}

static void testSubModify()
{
    char *ep = getEndPoint_input();
    if (ep == NULL)
    {
        printf("Client not connected to any endpoints\n\n");
        return ;
    }

    testBrowse(ep);
    char nodeName[MAX_CHAR_SIZE];

    printf("\nEnter the node name to modify Subscribe :: ");
    scanf("%s", nodeName);

    EdgeEndPointInfo *epInfo = (EdgeEndPointInfo *) calloc(1, sizeof(EdgeEndPointInfo));
    if(IS_NULL(epInfo))
    {
        printf("Error : Malloc failed for epInfo in test subscription modify\n");
        goto EXIT_SUBMODIFY;
    }
    epInfo->endpointUri = ep;

    EdgeSubRequest *subReq = (EdgeSubRequest *) malloc(sizeof(EdgeSubRequest));
    if(IS_NULL(subReq))
    {
        printf("Error : Malloc failed for subReq in test subscription modify\n");
        goto EXIT_SUBMODIFY;
    }
    subReq->subType = Edge_Modify_Sub;
    subReq->samplingInterval = 5000.0;
    subReq->publishingInterval = 0.0;
    subReq->maxKeepAliveCount = (1 > (int) (ceil(10000.0 / subReq->publishingInterval))) ? 1 :
                                (int) ceil(
                                    10000.0 / subReq->publishingInterval);
    printf("keepalive count :: %d\n", subReq->maxKeepAliveCount);
    subReq->lifetimeCount = 10000;  //subReq->maxKeepAliveCount * 6;
    printf("lifetimecount :: %d\n", subReq->lifetimeCount);
    subReq->maxNotificationsPerPublish = 1;
    subReq->publishingEnabled = true;
    subReq->priority = 0;
    subReq->queueSize = 50;

    EdgeNodeInfo *nodeInfo = (EdgeNodeInfo *) malloc(sizeof(EdgeNodeInfo));
    if(IS_NULL(nodeInfo))
    {
        printf("Error : Malloc failed for nodeInfo in test subscription modify\n");
        goto EXIT_SUBMODIFY;
    }
    nodeInfo->valueAlias = (char *) malloc(strlen(nodeName) + 1);
    if(IS_NULL(nodeInfo->valueAlias))
    {
        printf("Error : Malloc failed for nodeInfo->valueAlias in test subscription modify\n");
        goto EXIT_SUBMODIFY;
    }
    strncpy(nodeInfo->valueAlias, nodeName, strlen(nodeName));
    nodeInfo->valueAlias[strlen(nodeName)] = '\0';

    EdgeRequest *request = (EdgeRequest *) malloc(sizeof(EdgeRequest));
    if(IS_NULL(request))
    {
        printf("Error : Malloc failed for request in test subscription modify\n");
        goto EXIT_SUBMODIFY;
    }
    request->nodeInfo = nodeInfo;
    request->subMsg = subReq;

    EdgeMessage *msg = (EdgeMessage *) malloc(sizeof(EdgeMessage));
    if(IS_NULL(msg))
    {
        printf("Error : Malloc failed for EdgeMessage in test subscription modify\n");
        goto EXIT_SUBMODIFY;
    }
    msg->endpointInfo = epInfo;
    msg->command = CMD_SUB;
    msg->request = request;

    EdgeResult result = handleSubscription(msg);
    if (result.code == STATUS_OK)
    {
        printf("SUBSCRPTION MODIFY SUCCESSFULL\n");
    }

    EXIT_SUBMODIFY:
    if(IS_NOT_NULL(nodeInfo))
    {
        FREE(nodeInfo->valueAlias);
        FREE(nodeInfo);
    }

    FREE(subReq);
    FREE(request);
    FREE(epInfo);
    FREE (msg);
}

static void testRePublish()
{
    char *ep = getEndPoint_input();
    if (ep == NULL)
    {
        printf("Client not connected to any endpoints\n\n");
        return ;
    }

    char nodeName[MAX_CHAR_SIZE];
    printf("\nEnter the node name to Re publish :: ");
    scanf("%s", nodeName);

    EdgeEndPointInfo *epInfo = (EdgeEndPointInfo *) calloc(1, sizeof(EdgeEndPointInfo));
    if(IS_NULL(epInfo))
    {
        printf("Error : Malloc failed for epInfo in test republish\n");
        goto EXIT_REPUBLISH;
    }
    epInfo->endpointUri = ep;

    EdgeSubRequest *subReq = (EdgeSubRequest *) malloc(sizeof(EdgeSubRequest));
    if(IS_NULL(subReq))
    {
        printf("Error : Malloc failed for subReq in test republish\n");
        goto EXIT_REPUBLISH;
    }
    subReq->subType = Edge_Republish_Sub;

    EdgeNodeInfo *nodeInfo = (EdgeNodeInfo *) malloc(sizeof(EdgeNodeInfo));
    if(IS_NULL(nodeInfo))
    {
        printf("Error : Malloc failed for nodeInfo in test republish\n");
        goto EXIT_REPUBLISH;
    }
    nodeInfo->valueAlias = (char *) malloc(strlen(nodeName) + 1);
    if(IS_NULL(nodeInfo->valueAlias))
    {
        printf("Error : Malloc failed for nodeInfo->valueAlias in test republish \n");
        goto EXIT_REPUBLISH;
    }
    strncpy(nodeInfo->valueAlias, nodeName, strlen(nodeName));
    nodeInfo->valueAlias[strlen(nodeName)] = '\0';

    EdgeRequest *request = (EdgeRequest *) malloc(sizeof(EdgeRequest));
    if(IS_NULL(request))
    {
        printf("Error : Malloc failed for EdgeRequest in test republish\n");
        goto EXIT_REPUBLISH;
    }
    request->nodeInfo = nodeInfo;
    request->subMsg = subReq;

    EdgeMessage *msg = (EdgeMessage *) malloc(sizeof(EdgeMessage));
    if(IS_NULL(msg))
    {
        printf("Error : Malloc failed for EdgeMessage in test republish\n");
        goto EXIT_REPUBLISH;
    }
    msg->endpointInfo = epInfo;
    msg->command = CMD_SUB;
    msg->request = request;

    EdgeResult result = handleSubscription(msg);
    printf("REPUBLISH RESULT : %d\n",  result.code);
    if (result.code == STATUS_OK)
    {
        printf("REPUBLISH SUCCESSFULL\n");
    }

    EXIT_REPUBLISH:
    if(IS_NOT_NULL(nodeInfo))
    {
        FREE(nodeInfo->valueAlias);
        FREE(nodeInfo);
    }

    FREE(subReq);
    FREE(request);
    FREE(epInfo);
    FREE (msg);
}

static void testSubDelete()
{
    char *ep = getEndPoint_input();
    if (ep == NULL)
    {
        printf("Client not connected to any endpoints\n\n");
        return ;
    }

    //testBrowse();
    char nodeName[MAX_CHAR_SIZE];

    printf("\nEnter the node name to delete Subscribe :: ");
    scanf("%s", nodeName);

    EdgeEndPointInfo *epInfo = (EdgeEndPointInfo *) calloc(1, sizeof(EdgeEndPointInfo));
    if(IS_NULL(epInfo))
    {
        printf("Error : Malloc failed for epInfo in test subscription delete\n");
        goto EXIT_SUBDELETE;
    }
    epInfo->endpointUri = ep;

    EdgeSubRequest *subReq = (EdgeSubRequest *) malloc(sizeof(EdgeSubRequest));
    if(IS_NULL(subReq))
    {
        printf("Error : Malloc failed for subReq in test subscription delete\n");
        goto EXIT_SUBDELETE;
    }
    subReq->subType = Edge_Delete_Sub;

    EdgeNodeInfo *nodeInfo = (EdgeNodeInfo *) malloc(sizeof(EdgeNodeInfo));
    if(IS_NULL(nodeInfo))
    {
        printf("Error : Malloc failed for nodeInfo in test subscription delete\n");
        goto EXIT_SUBDELETE;
    }
    nodeInfo->valueAlias = (char *) malloc(strlen(nodeName) + 1);
    if(IS_NULL(nodeInfo->valueAlias))
    {
        printf("Error : Malloc failed for nodeInfo->valueAlias in test subscription delete\n");
        goto EXIT_SUBDELETE;
    }
    strncpy(nodeInfo->valueAlias, nodeName, strlen(nodeName));
    nodeInfo->valueAlias[strlen(nodeName)] = '\0';

    EdgeRequest *request = (EdgeRequest *) malloc(sizeof(EdgeRequest));
    if(IS_NULL(request))
    {
        printf("Error : Malloc failed for EdgeRequest in test subscription delete\n");
        goto EXIT_SUBDELETE;
    }
    request->nodeInfo = nodeInfo;
    request->subMsg = subReq;

    EdgeMessage *msg = (EdgeMessage *) malloc(sizeof(EdgeMessage));
    if(IS_NULL(msg))
    {
        printf("Error : Malloc failed for EdgeMessage in test subscription delete\n");
        goto EXIT_SUBDELETE;
    }
    msg->endpointInfo = epInfo;
    msg->command = CMD_SUB;
    msg->request = request;

    EdgeResult result = handleSubscription(msg);
    printf("DELETE RESULT : %d\n",  result.code);
    if (result.code == STATUS_OK)
    {
        printf("SUBSCRPTION DELETED SUCCESSFULL\n");
    }

    EXIT_SUBDELETE:
    if(IS_NOT_NULL(nodeInfo))
    {
        FREE(nodeInfo->valueAlias);
        FREE(nodeInfo);
    }
    FREE(subReq);
    FREE(request);
    FREE(epInfo);
    FREE (msg);
}

static void print_menu()
{
    printf("\n=============== OPC UA =======================\n\n");

    printf("start : Get endpoints and start opcua client \n");
    printf("read : read attribute for target node\n");
    printf("read_group : group read attributes from nodes\n");
    printf("write : write attribute into nodes\n");
    printf("write_group : group write attributes from nodes\n");
    printf("browse : browse nodes\n");
    printf("browse_m : browse multiple nodes\n");
    printf("browse_next : browse next nodes\n");
    printf("browse_v : browse views\n");
    printf("method : method call\n");
    printf("create_sub : create subscription\n");
    printf("modify_sub : modify subscription\n");
    printf("delete_sub : delete subscription\n");
    printf("set_max_ref : Set maximum references per node\n");
    printf("quit : terminate/stop opcua server/client and then quit\n");
    printf("help : print menu\n");

    printf("\n=============== OPC UA =======================\n\n");
}

int main()
{
    char command[MAX_CHAR_SIZE];

    print_menu();
    init();

    while (!stopFlag)
    {
        printf("\n\n[INPUT Command] : ");
        scanf("%s", command);

        if (stopFlag)
        {
            break;
        }
        else if (!strcmp(command, "start"))
        {

            printf("\n" COLOR_YELLOW "------------------------------------------------------" COLOR_RESET);
            printf("\n" COLOR_YELLOW "                  Client get endpoints             " COLOR_RESET);
            printf("\n" COLOR_YELLOW "------------------------------------------------------" COLOR_RESET
                   "\n\n");

            printf("[Please input server endpoint uri] : ");
            scanf("%s", ipAddress);
            strncpy(endpointUri, ipAddress, strlen(ipAddress));
            endpointUri[strlen(ipAddress)] = '\0';

            testGetEndpoints();
            //startFlag = true;
            print_menu();
        }
        else if (!strcmp(command, "read"))
        {
            testRead();
        }
        else if (!strcmp(command, "read_group"))
        {
            testReadGroup();
        }
        else if (!strcmp(command, "write"))
        {
            testWrite();
        }
        else if (!strcmp(command, "write_group"))
        {
            testWriteGroup();
        }
        else if (!strcmp(command, "browse"))
        {
            char *ep = getEndPoint_input();
            if (ep == NULL)
            {
                printf("Client not connected to any endpoints\n\n");
                break;
            }
            testBrowse(ep);
        }
        else if (!strcmp(command, "browse_m"))
        {
            testBrowses();
        }
        else if (!strcmp(command, "browse_next"))
        {
            testBrowseNext();
        }
        else if (!strcmp(command, "browse_v"))
        {
            char *ep = getEndPoint_input();
            if (ep == NULL)
            {
                printf("Client not connected to any endpoints\n\n");
                break;
            }
            testBrowseViews(ep);
        }
        else if (!strcmp(command, "method"))
        {
            testMethod();
        }
        else if (!strcmp(command, "create_sub"))
        {
            testSub();
        }
        else if (!strcmp(command, "modify_sub"))
        {
            testSubModify();
        }
        else if (!strcmp(command, "delete_sub"))
        {
            testSubDelete();
        }
        else if (!strcmp(command, "quit"))
        {
            deinit();
            //stopFlag = true;
        }
        else if (!strcmp(command, "help"))
        {
            print_menu();
        }
        else if (!strcmp(command, "republish"))
        {
            testRePublish();
        }
        else if (!strcmp(command, "set_max_ref"))
        {
            printf("Enter a value for maximum references allowed per node:");
            scanf("%d", &maxReferencesPerNode);
        }
    }

    destroyBrowseNextData(browseNextData);
    return 0;
}
