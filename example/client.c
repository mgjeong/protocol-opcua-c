#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>
#include <inttypes.h>

#include "opcua_manager.h"
#include "opcua_common.h"
#include "edge_malloc.h"
#include "sample_common.h"

#define TAG "SAMPLE_CLIENT"

#define IS_NULL(arg) ((arg == NULL) ? true : false)
#define VERIFY_NON_NULL(arg, retVal) { if (!(arg)) { printf( \
             #arg " is NULL"); return (retVal); } }
#define VERIFY_NON_NULL_NR(arg) { if (!(arg)) { printf( \
             #arg " is NULL"); return; } }

#define TEST_WITH_REFERENCE_SERVER 0

#define COLOR_GREEN        "\x1b[32m"
#define COLOR_YELLOW      "\x1b[33m"
#define COLOR_PURPLE      "\x1b[35m"
#define COLOR_RESET         "\x1b[0m"

#define MAX_CHAR_SIZE (512)
#define MAX_ADDRESS_SIZE (128)
#define MAX_CP_LIST_COUNT (1000)
#define NODE_COUNT (6)

static bool startFlag = false;
static bool stopFlag = false;

static char endpointUri[MAX_CHAR_SIZE];
static int endpointCount = 0;
static bool connect = false;

static uint8_t supportedApplicationTypes = EDGE_APPLICATIONTYPE_SERVER | EDGE_APPLICATIONTYPE_DISCOVERYSERVER |
    EDGE_APPLICATIONTYPE_CLIENTANDSERVER;


typedef struct EndPointList {
    char *endpoint;
    struct EndPointList* next;
} EndPointList;

static EndPointList* epList = NULL;
static EdgeConfigure *config = NULL;

EdgeBrowseNextData *browseNextData = NULL;
int maxReferencesPerNode = 0;

static void add_to_endpoint_list(char *endpoint);
static EndPointList *remove_from_endpoint_list(char *endpoint);

static void startClient(char *addr, int port, char *securityPolicyUri, char *endpoint);
static void *getNewValuetoWrite(int type, int num_values);

static void response_msg_cb (EdgeMessage *data)
{
    if (data->type == GENERAL_RESPONSE)
    {
        int len = data->responseLength;
        if (0 == len)
            printf("Msg id : [%" PRIu32 "] , Response Received ::  \n", data->message_id);
        int idx = 0;
        for (idx = 0; idx < len; idx++)
        {
            if (data->responses[idx]->message != NULL)
            {
                if (data->command == CMD_READ || data->command == CMD_METHOD)
                {
                    printf("Msg id : [%" PRIu32 "] , Response Received ::  ", data->message_id);
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
                        else if (data->responses[idx]->type == String || data->responses[idx]->type == XmlElement)
                            printf("[%s]\n",
                                   (char *)data->responses[idx]->message->value);
                    }
                }
                else if (data->command == CMD_WRITE)
                {
                    printf("\nMsg id : : [%" PRIu32 "] ,  Write response :: %s\n", data->message_id,
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
            printf("Msg id : [%" PRIu32 "] , [Node Name] : %s\n", data->message_id, data->responses[idx]->nodeInfo->valueAlias);
            if (data->responses[idx]->message == NULL)
            {
                printf("data->responses[%d]->message is NULL\n", idx);
                continue;
            }

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
                if (data->responses[idx]->type == Int16)
                    printf("[MonitoredItem DataChange callback] Monitored Change Value read from node ===>> [%d]\n",
                           *((int *)data->responses[idx]->message->value));
                else if (data->responses[idx]->type == Byte)
                    printf("[MonitoredItem DataChange callback] Monitored Change Value read from node ===>> [%d]\n",
                           *((uint8_t *)data->responses[idx]->message->value));
                else if (data->responses[idx]->type == ByteString)
                    printf("[MonitoredItem DataChange callback] Monitored Change Value read from node ===>> [%s]\n",
                           (char *)data->responses[idx]->message->value);
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
        printf("\n\n");
    }
}

static void error_msg_cb (EdgeMessage *data)
{
    printf("\n[Application error response callback]\n");
    printf("================================================\n");
    printf("EdgeStatusCode: %d\n", data->result->code);
    int responseLength = data->responseLength;
    printf("ResponseLength: %d\n", responseLength);
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

static void browse_msg_cb (EdgeMessage *data)
{
    if (data->browseResult)
    {
        if(data->responses[0]->message != NULL){
            printf("%s\n", (unsigned char *)data->responses[0]->message->value);
        }
    }
    else
    {
        if (data->cpList && data->cpList->count > 0)
        {
            printf("Total number of continuation points: %zu\n", data->cpList->count);
            for (size_t i = 0; i < data->cpList->count; ++i)
            {
                EdgeNodeId *nodeId = data->responses[i]->nodeInfo->nodeId;
                printf("Node ID of Continuation point[%zu]: ", i + 1);
                (nodeId->type == INTEGER) ? printf("%d\n", nodeId->integerNodeId) : printf("%s\n", nodeId->nodeId);

                int length = data->cpList->cp[i]->length;
                unsigned char *cp = data->cpList->cp[i]->continuationPoint;
                printf("Length: %d\n", length);
                for (int j = 0; j < length; ++j)
                {
                    printf("%02X", cp[j]);
                }
                printf("\n");

                EdgeResult ret = addBrowseNextData(&browseNextData, data->cpList->cp[i], nodeId);
                if (STATUS_OK != ret.code)
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
            EdgeFree (list->endpoint);
            EdgeFree (list);
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

            if(connect)
            {
                startClient(device->address, device->port, device->endpointsInfo[idx]->securityPolicyUri, device->endpointsInfo[idx]->endpointUri);
            }
        }
    }
}

static void device_found_cb (EdgeDevice *device)
{

}

////////////////////////// START : helper function for endpoint list //////////////////////////////

static EndPointList *create_endpoint_list(char *endpoint) {
    EndPointList *ep = (EndPointList*) EdgeMalloc(sizeof(EndPointList));
    VERIFY_NON_NULL(ep, NULL);
    ep->endpoint = (char*) EdgeMalloc(sizeof(char) * (strlen(endpoint) + 1));
    if(IS_NULL(ep->endpoint))
    {
        printf("Error : Malloc failed for endpoint in create_endpoint_list\n");
        EdgeFree(ep);
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
    config = (EdgeConfigure *) EdgeMalloc(sizeof(EdgeConfigure));
    VERIFY_NON_NULL_NR(config);
    config->recvCallback = (ReceivedMessageCallback *) EdgeMalloc(sizeof(ReceivedMessageCallback));
    VERIFY_NON_NULL_NR(config->recvCallback);
    config->recvCallback->resp_msg_cb = response_msg_cb;
    config->recvCallback->monitored_msg_cb = monitored_msg_cb;
    config->recvCallback->error_msg_cb = error_msg_cb;
    config->recvCallback->browse_msg_cb = browse_msg_cb;

    config->statusCallback = (StatusCallback *) EdgeMalloc(sizeof(StatusCallback));
    VERIFY_NON_NULL_NR(config->statusCallback);
    config->statusCallback->start_cb = status_start_cb;
    config->statusCallback->stop_cb = status_stop_cb;
    config->statusCallback->network_cb = status_network_cb;

    config->discoveryCallback = (DiscoveryCallback *) EdgeMalloc(sizeof(DiscoveryCallback));
    VERIFY_NON_NULL_NR(config->discoveryCallback);
    config->discoveryCallback->endpoint_found_cb = endpoint_found_cb;
    config->discoveryCallback->device_found_cb = device_found_cb;

    config->supportedApplicationTypes = supportedApplicationTypes;

    configure(config);
}

static void testFindServers()
{
    char endpointUri[MAX_ADDRESS_SIZE];
    printf("[Please input server endpoint uri (Ex: opc.tcp://hostname:port/path)] : ");
    scanf("%s", endpointUri);

#if 0
    size_t serverUrisSize = 2;
    unsigned char *serverUris[2] = {NULL};
    serverUris[0] = (unsigned char *)"urn:digitalpetri:opcua:client";
    serverUris[1] = (unsigned char *)"urn:digitalpetri:opcua:invalid_client";
#else
    size_t serverUrisSize = 0;
    unsigned char **serverUris = NULL;
#endif

#if 0
    size_t localeIdsSize = 1;
    unsigned char *localeIds[1] = {NULL};
    localeIds[0] = (unsigned char *)"en-US";
#else
    size_t localeIdsSize = 0;
    unsigned char **localeIds = NULL;
#endif

    EdgeApplicationConfig *registeredServers = NULL;
    size_t registeredServersSize = 0;
    EdgeResult res = findServers(endpointUri, serverUrisSize, serverUris, localeIdsSize, localeIds, &registeredServersSize, &registeredServers);
    if(res.code != STATUS_OK)
    {
        printf("findServers() failed. Status Code: %d\n", res.code);
    }
    else
    {
        printf("\nTotal number of registered servers at the given server: %zu\n", registeredServersSize);
        for(size_t idx = 0; idx < registeredServersSize ; ++idx)
        {
            printf("Server[%zu] -> Application URI: %s\n", idx+1, registeredServers[idx].applicationUri);
            printf("Server[%zu] -> Product URI: %s\n", idx+1, registeredServers[idx].productUri);
            printf("Server[%zu] -> Application Name: %s\n", idx+1, registeredServers[idx].applicationName);
            printf("Server[%zu] -> Application Type: %u\n", idx+1, registeredServers[idx].applicationType);
            printf("Server[%zu] -> Gateway Server URI: %s\n", idx+1, registeredServers[idx].gatewayServerUri);
            printf("Server[%zu] -> Discovery Profile URI: %s\n", idx+1, registeredServers[idx].discoveryProfileUri);
            for(size_t i = 0; i < registeredServers[idx].discoveryUrlsSize; ++i)
            {
                printf("Server[%zu] -> Discovery URL[%zu]: %s\n", idx+1, i+1, registeredServers[idx].discoveryUrls[i]);
            }
        }

        // Application has to deallocate the memory for EdgeApplicationConfig array and its members.
        for(size_t idx = 0; idx < registeredServersSize ; ++idx)
        {
            destroyEdgeApplicationConfigMembers(&registeredServers[idx]);
        }
        EdgeFree(registeredServers);
    }
}

static void testGetEndpoints(const char *endpointUri)
{
    printf("\n" COLOR_YELLOW "------------------------------------------------------" COLOR_RESET);
    printf("\n" COLOR_YELLOW "                  Client get endpoints             " COLOR_RESET);
    printf("\n" COLOR_YELLOW "------------------------------------------------------" COLOR_RESET
           "\n\n");

    EdgeMessage *msg = createEdgeMessage(endpointUri, 1, CMD_GET_ENDPOINTS);
    if(IS_NULL(msg))
    {
        printf("Error : Malloc failed for EdgeMessage in test Method\n");
        return;
    }

    EdgeResult res = getEndpointInfo(msg);
    if(res.code != STATUS_OK)
    {
        printf("getEndpointInfo() failed. Status Code: %d\n", res.code);
    }

    destroyEdgeMessage(msg);
}

static void startClient(char *addr, int port, char *securityPolicyUri, char *endpoint)
{
    printf("\n" COLOR_YELLOW "------------------------------------------------------" COLOR_RESET);
    printf("\n" COLOR_YELLOW "                       Client connect            "COLOR_RESET);
    printf("\n" COLOR_YELLOW "------------------------------------------------------" COLOR_RESET
           "\n\n");

    EdgeMessage *msg = createEdgeMessage(endpointUri, 0, CMD_START_CLIENT);
    if(IS_NULL(msg))
    {
        printf("Error : Malloc failed for EdgeMessage in test Method\n");
        return;
    }

    msg->endpointInfo->endpointConfig = (EdgeEndpointConfig *) EdgeCalloc(1, sizeof(EdgeEndpointConfig));
    if(IS_NULL(msg->endpointInfo->endpointConfig))
    {
        printf("Error : Malloc failed for EdgeEndpointConfig in test subscription delete\n");
        goto EXIT_START;
    }
    msg->endpointInfo->endpointConfig->requestTimeout = 60000;
    msg->endpointInfo->endpointConfig->serverName = copyString(DEFAULT_SERVER_NAME_VALUE);
    msg->endpointInfo->endpointConfig->bindAddress = copyString(addr);
    msg->endpointInfo->endpointConfig->bindPort = port;
    msg->endpointInfo->securityPolicyUri = copyString(securityPolicyUri);

    sendRequest(msg);

    EXIT_START:
    destroyEdgeMessage(msg);
}

static void stopClient()
{
    EndPointList *next = NULL;
    EndPointList *temp = epList;
    while (temp)
    {
        next = temp->next;
        EdgeMessage *msg = createEdgeMessage(temp->endpoint, 1, CMD_STOP_CLIENT);
        if(IS_NULL(msg))
        {
            printf("Error : Malloc failed for EdgeMessage in test Method\n");
            return;
        }
        printf("\n" COLOR_YELLOW "********************** stop client **********************"
           COLOR_RESET"\n");
        disconnectClient(msg->endpointInfo);
        destroyEdgeMessage(msg);
        temp = next;
    }
}

static void deinit()
{
    stopClient();
    if (config)
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
    }
}

static void testBrowseNext()
{
    printf("\n" COLOR_YELLOW "------------------------------------------------------" COLOR_RESET);
    printf("\n" COLOR_YELLOW "                       Browse Next            "COLOR_RESET);
    printf("\n" COLOR_YELLOW "------------------------------------------------------" COLOR_RESET
           "\n\n");

    if (!browseNextData || browseNextData->next_free < 1)
    {
        printf("Invalid data for browse next service.\n");
        return;
    }

    EdgeBrowseNextData *clone = cloneBrowseNextData(browseNextData);
    if (!clone)
    {
        printf("Failed to clone the BrowseNextData.\n");
        return;
    }

    browseNextData = initBrowseNextData(browseNextData, &browseNextData->browseParam, MAX_CP_LIST_COUNT);
    printf("Total number of continuation points: %zu.\n", clone->next_free);

    // SEND_REQUESTS : There can be one or more continuation points.
    // CMD_BROWSENEXT : Using the same existing command for browse next operation as well.
    size_t requestLength = clone->next_free;
    EdgeMessage *msg = createEdgeMessage(endpointUri, requestLength, CMD_BROWSENEXT);
    if(IS_NULL(msg))
    {
        printf("Error : Malloc failed for EdgeMessage in test Method\n");
        return;
    }

    if(requestLength == 1)
    {
        msg->request->nodeInfo = (EdgeNodeInfo *) EdgeCalloc(1, sizeof(EdgeNodeInfo));
        if(IS_NULL(msg->request->nodeInfo))
        {
            printf("Error : Malloc failed for nodeInfo in testBrowseNext()\n");
            goto EXIT_BROWSENEXT;
        }
        msg->request->nodeInfo->nodeId = clone->srcNodeId[0];
    }
    else
    {
        for (size_t i = 0; i < requestLength; i++)
        {
            msg->requests[i] = (EdgeRequest *) EdgeCalloc(1, sizeof(EdgeRequest));
            if(IS_NULL(msg->requests[i]))
            {
                printf("Error : Malloc failed for requests[%zu] in testBrowseNext()\n", i);
                goto EXIT_BROWSENEXT;
            }

            msg->requests[i]->nodeInfo = (EdgeNodeInfo *) EdgeCalloc(1, sizeof(EdgeNodeInfo));
            if(IS_NULL(msg->requests[i]->nodeInfo))
            {
                printf("Error : Malloc failed for nodeInfo in testBrowseNext()\n");
                goto EXIT_BROWSENEXT;
            }
            msg->requests[i]->nodeInfo->nodeId = clone->srcNodeId[i];
        }
    }

    msg->requestLength = requestLength;
    msg->browseParam = &clone->browseParam;

    msg->cpList = (EdgeContinuationPointList *)EdgeCalloc(1, sizeof(EdgeContinuationPointList));
    if(IS_NULL(msg->cpList))
    {
        printf("Error : Malloc failed for msg->cpList in testBrowseNext()\n");
        goto EXIT_BROWSENEXT;
    }
    msg->cpList->count = requestLength;
    msg->cpList->cp = (EdgeContinuationPoint **)calloc(requestLength, sizeof(EdgeContinuationPoint *));
    if(IS_NULL(msg->cpList->cp))
    {
        printf("Error : Malloc failed for msg->cpList->cp in testBrowseNext()\n");
        goto EXIT_BROWSENEXT;
    }
    for (size_t i = 0; i < requestLength; i++)
    {
        msg->cpList->cp[i] = &clone->cp[i];
    }

    sendRequest(msg);

    EXIT_BROWSENEXT:

    // Free request or requests based on the request length.
    if(requestLength == 1)
    {
        msg->request->nodeInfo->nodeId = NULL;
    }
    else
    {
        for (size_t i = 0; i < requestLength; i++)
            msg->requests[i]->nodeInfo->nodeId = NULL;
    }

    // Free continuation point list
    for (size_t i = 0; i < requestLength; i++)
    {
        msg->cpList->cp[i] = NULL;
    }

    msg->browseParam = NULL;
    destroyEdgeMessage(msg);
    destroyBrowseNextData(clone);
}

static void testBrowseViews(char* endpointUri)
{
    printf("\n" COLOR_YELLOW "------------------------------------------------------" COLOR_RESET);
    printf("\n" COLOR_YELLOW "                       Browse Views            "COLOR_RESET);
    printf("\n" COLOR_YELLOW "------------------------------------------------------" COLOR_RESET
           "\n");

    EdgeMessage *msg = createEdgeMessage(endpointUri, 0, CMD_BROWSE_VIEW);
    if(IS_NULL(msg))
    {
        printf("Error : Malloc failed for EdgeMessage in test Method\n");
        return;
    }

    printf("\n" COLOR_YELLOW "********** Browse Views under RootFolder node in system namespace **********"
           COLOR_RESET "\n");

    browseNextData = initBrowseNextData(browseNextData, msg->browseParam, MAX_CP_LIST_COUNT);

    sendRequest(msg);

    destroyEdgeMessage(msg);
}

static void testBrowse(char* endpointUri)
{
    printf("\n" COLOR_YELLOW "------------------------------------------------------" COLOR_RESET);
    printf("\n" COLOR_YELLOW "                       Browse Node            "COLOR_RESET);
    printf("\n" COLOR_YELLOW "------------------------------------------------------" COLOR_RESET
           "\n\n");

    EdgeMessage *msg = createEdgeMessage(endpointUri, 1, CMD_BROWSE);
    if(IS_NULL(msg))
    {
        printf("Error : Malloc failed for EdgeMessage in test Method\n");
        return;
    }

    EdgeNodeInfo* nodeInfo = createEdgeNodeInfoForNodeId(INTEGER, EDGE_NODEID_ROOTFOLDER, SYSTEM_NAMESPACE_INDEX);
    EdgeBrowseParameter param = {DIRECTION_FORWARD, maxReferencesPerNode};
    insertBrowseParameter(&msg, nodeInfo, param);
    printf("\n\n" COLOR_YELLOW "********** Browse RootFolder node in system namespace **********"
           COLOR_RESET "\n");

    browseNextData = initBrowseNextData(browseNextData, msg->browseParam, MAX_CP_LIST_COUNT);

    sendRequest(msg);

    destroyEdgeMessage(msg);
}

static void testBrowses(char* endpointUri)
{
    printf("\n" COLOR_YELLOW "------------------------------------------------------" COLOR_RESET);
    printf("\n" COLOR_YELLOW "                       Browse Multiple Nodes            "COLOR_RESET);
    printf("\n" COLOR_YELLOW "------------------------------------------------------" COLOR_RESET
           "\n\n");

    int requestLength = 3;
    EdgeMessage *msg = createEdgeMessage(endpointUri, requestLength, CMD_BROWSE);
    if(IS_NULL(msg))
    {
        printf("Error : Malloc failed for EdgeMessage in test Method\n");
        return;
    }

    EdgeNodeInfo* nodeInfo = createEdgeNodeInfoForNodeId(INTEGER, EDGE_NODEID_ROOTFOLDER, SYSTEM_NAMESPACE_INDEX);
    EdgeBrowseParameter param = {DIRECTION_FORWARD, maxReferencesPerNode};
    insertBrowseParameter(&msg, nodeInfo, param);
    nodeInfo = createEdgeNodeInfoForNodeId(INTEGER, EDGE_NODEID_OBJECTSFOLDER, SYSTEM_NAMESPACE_INDEX);
    insertBrowseParameter(&msg, nodeInfo, param);
    nodeInfo = createEdgeNodeInfo("{2;S;v=0}Object1");
    insertBrowseParameter(&msg, nodeInfo, param);

    printf("\n\n" COLOR_YELLOW
           "********** Browse RootFolder, ObjectsFolder nodes in system namespace and Object1 in namespace 1 **********"
           COLOR_RESET "\n");

    browseNextData = initBrowseNextData(browseNextData, msg->browseParam, MAX_CP_LIST_COUNT);

    sendRequest(msg);

    destroyEdgeMessage(msg);
}

static void readHelper(int num_requests, char *ep)
{
    char nodeName[MAX_CHAR_SIZE];
    EdgeMessage *msg = createEdgeAttributeMessage(ep, num_requests, CMD_READ);
    if(IS_NULL(msg))
    {
        printf("Error : Malloc failed for attribute message");
        return;
    }

    for (int i = 0; i < num_requests; i++)
    {
        printf("\nEnter the node #%d name to read :: ", (i + 1));
        scanf("%s", nodeName);
        insertReadAccessNode(&msg, nodeName);
    }

    sendRequest(msg);
    destroyEdgeMessage(msg);
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
    testBrowseViews(ep);

    // Perform read operation.
    int num_requests = 1;
    readHelper(num_requests, ep);
}

static void testReadGroup()
{
    char *ep = getEndPoint_input();
    if (ep == NULL)
    {
        printf("Client not connected to any endpoints\n\n");
        return ;
    }

    // Get the list of browse names and display them to user.
    testBrowseViews(ep);

    // Get the number of requests.
    int num_requests;
    printf("\nEnter number of nodes to read (less than 10) :: ");
    scanf("%d", &num_requests);
    if(num_requests < 1 || num_requests > 9)
    {
        printf("Error : Invalid input for number of nodes to read\n");
        return ;
    }

    // Perform read.
    readHelper(num_requests, ep);
}

static void writeHelper(int num_requests, char *ep)
{
    char nodeName[MAX_CHAR_SIZE];
    int valueLen;

    EdgeMessage *msg = createEdgeAttributeMessage(ep, num_requests, CMD_WRITE);
    if(IS_NULL(msg))
    {
        printf("Error : Malloc failed for attribute message");
        return;
    }

    for (int i = 0; i < num_requests; i++)
    {
        printf("\nEnter the node #%d name to write :: ", (i + 1));
        scanf("%s", nodeName);

        int nodeType = getValueType(nodeName);
        printf("Enter number of elements to write (1 for scalar, > 1 for Array) : ");
        scanf("%d", &valueLen);
        void *value = getNewValuetoWrite(nodeType, valueLen);
        if(IS_NULL(value))
        {
            printf("Value ptr is NULL. So write() can't be performed.\n.");
            destroyEdgeMessage(msg);
            return;
        }

        EdgeResult res = insertWriteAccessNode(&msg, nodeName, value, valueLen);
        if(STATUS_OK != res.code)
        {
            printf("Error : insertWriteAccessNode() failed (Status Code: %d).\n", res.code);
            destroyEdgeMessage(msg);
            return;
        }
    }

    printf("write node \n");
    sendRequest(msg);
    printf("write node call success \n");
    destroyEdgeMessage(msg);
}

static void testWrite()
{
    char *ep = getEndPoint_input();
    if (ep == NULL)
    {
        printf("Client not connected to any endpoints\n\n");
        return ;
    }

    // Get the list of browse names and display them to user.
    testBrowseViews(ep);

    // Perform write.
    int num_requests = 1;
    writeHelper(num_requests, ep);
}

static void testWriteGroup()
{
    char *ep = getEndPoint_input();
    if (ep == NULL)
    {
        printf("Client not connected to any endpoints\n\n");
        return ;
    }

    // Get the list of browse names and display them to user.
    testBrowseViews(ep);

    int num_requests;
    printf("Enter number of nodes to write (less than 10) :: ");
    scanf("%d", &num_requests);
    if(num_requests < 1 || num_requests > 9)
    {
        printf("Error : Invalid input for number of nodes to Write\n");
        return ;
    }

    // Perform write.
    writeHelper(num_requests, ep);
}

static void testMethod()
{
    printf("\n" COLOR_YELLOW "------------------------------------------------------" COLOR_RESET);
    printf("\n" COLOR_YELLOW "                       Method Call            "COLOR_RESET);
    printf("\n" COLOR_YELLOW "------------------------------------------------------" COLOR_RESET
           "\n\n");

    printf("\n|------------------- [Method Call] - sqrt(x) \n");

    EdgeMessage *msg = createEdgeMessage(endpointUri, 1, CMD_METHOD);
    if(IS_NULL(msg))
    {
        printf("Error : Malloc failed for EdgeMessage in test Method\n");
        return;
    }
    double input = 16.0;
    EdgeResult ret = insertEdgeMethodParameter(&msg, "{2;S;v=0}sqrt(x)", 1, Double, SCALAR, (void *) &input, NULL, 0);
    if (ret.code != STATUS_OK) {
        printf("Error : insertEdgeMethodParameter has failed\n");
        goto EXIT_METHOD;
    }
    printf("input(x) :: [%.2f]\n", input);
    sendRequest(msg);

    EXIT_METHOD:
    msg->request->methodParams->inpArg[0]->scalarValue = NULL; // Setting NULL to prevent destroy() from trying to deallocate it.
    destroyEdgeMessage(msg);

    printf("\n|------------------- [Method Call ] - incrementInc32Array(x,delta) \n");

    msg = createEdgeMessage(endpointUri, 1, CMD_METHOD);
    if(IS_NULL(msg))
    {
        printf("Error : Malloc failed for EdgeMessage in test Method\n");
        return;
    }

    int32_t array[5] = {10, 20, 30, 40, 50};
    ret = insertEdgeMethodParameter(&msg, "{2;S;v=0}incrementInc32Array(x,delta)", 2,
            Int32, ARRAY_1D, NULL, (void *) array, 5);
    if (ret.code != STATUS_OK) {
        printf("Error : insertEdgeMethodParameter has failed\n");
        goto EXIT_METHOD1;
    }
    int delta = 5;
    ret = insertEdgeMethodParameter(&msg, "{2;S;v=0}incrementInc32Array(x,delta)", 2,
            Int32, SCALAR, (void *) &delta, NULL, 0);
    if (ret.code != STATUS_OK) {
        printf("Error : insertEdgeMethodParameter has failed\n");
        goto EXIT_METHOD1;
    }
    sendRequest(msg);

    EXIT_METHOD1:
    // Setting NULL to prevent destroy() from trying to deallocate it.
    msg->request->methodParams->inpArg[0]->arrayData = NULL;
    msg->request->methodParams->inpArg[1]->scalarValue = NULL;
    destroyEdgeMessage(msg);

    printf("\n|------------------- [Method Call ] - print(x) \n");

    msg = createEdgeMessage(endpointUri, 1, CMD_METHOD);
    if(IS_NULL(msg))
    {
        printf("Error : Malloc failed for EdgeMessage in test Method\n");
        return;
    }

    int32_t val = 100;
    ret = insertEdgeMethodParameter(&msg, "{2;S;v=0}print(x)", 1,
                Int32, SCALAR, (void *)&val, NULL, 0);
    if (ret.code != STATUS_OK) {
        printf("Error : insertEdgeMethodParameter has failed\n");
        goto EXIT_METHOD2;
    }
    printf("input(x) :: [%d]\n", val);
    sendRequest(msg);

    EXIT_METHOD2:
    msg->request->methodParams->inpArg[0]->scalarValue = NULL;
    destroyEdgeMessage (msg);

    printf("\n|------------------- [Method Call ] - shutdown() \n");

    msg = createEdgeMessage(endpointUri, 1, CMD_METHOD);
    if(IS_NULL(msg))
    {
        printf("Error : Malloc failed for EdgeMessage in test Method\n");
        return;
    }

    ret = insertEdgeMethodParameter(&msg, "{2;S;v=0}shutdown()", 0,
                    0, 0, NULL, NULL, 0);
    if (ret.code != STATUS_OK) {
        printf("Error : insertEdgeMethodParameter has failed\n");
        goto EXIT_METHOD3;
    }
    sendRequest(msg);

    EXIT_METHOD3:
    destroyEdgeMessage(msg);

    printf("\n|------------------- [Method Call ] - version() \n");

    msg = createEdgeMessage(endpointUri, 1, CMD_METHOD);
    if(IS_NULL(msg))
    {
        printf("Error : Malloc failed for EdgeMessage in test Method\n");
        return;
    }

    ret = insertEdgeMethodParameter(&msg, "{2;S;v=0}version()", 0,
                    0, 0, NULL, NULL, 0);
    if (ret.code != STATUS_OK) {
        printf("Error : insertEdgeMethodParameter has failed\n");
        goto EXIT_METHOD4;
    }
    sendRequest(msg);

    EXIT_METHOD4:
    destroyEdgeMessage(msg);
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
        return;
    }

    // Get the list of browse names and display them to user.
    testBrowseViews(ep);
    char nodeName[MAX_CHAR_SIZE];
    int num_requests;
    printf("\nEnter number of nodes to Subscribe (less than 10) :: ");
    scanf("%d", &num_requests);
    if(num_requests > 10 || num_requests < 1)
    {
        printf("Error :: Invalid input. Try Again\n ");
        return;
    }

    EdgeMessage* msg = createEdgeSubMessage(ep, nodeName, num_requests, Edge_Create_Sub);
    if(IS_NULL(msg))
    {
        printf("Error : EdgeMalloc failed for msg in test subscription\n");
        return;
    }

    for (int i = 0; i < num_requests; i++)
    {
        printf("\nEnter the node #%d name to subscribe :: ", (i + 1));
        scanf("%s", nodeName);

        double samplingInterval;
        printf("\nEnter number of sampling interval[millisecond] (minimum : 100ms) :: ");
        scanf("%lf", &samplingInterval);
        int keepalivetime = (1 > (int) (ceil(10000.0 / 0.0))) ? 1 : (int) ceil(10000.0 / 0.0);
        insertSubParameter(&msg, nodeName, Edge_Create_Sub, samplingInterval, 0.0, keepalivetime, 10000, 1, true, 0, 50);
    }


    EdgeResult result = sendRequest(msg);
    if (result.code == STATUS_OK)
    {
        printf(COLOR_GREEN "\nSUBSCRPTION CREATE SUCCESSFULLY\n" COLOR_RESET);
    } else {
        printf("CREATE RESULT : %d\n",  result.code);
    }

    destroyEdgeMessage (msg);
}

static void testSubModify()
{
    char *ep = getEndPoint_input();
    if (ep == NULL)
    {
        printf("Client not connected to any endpoints\n\n");
        return ;
    }

    testBrowseViews(ep);
    char nodeName[MAX_CHAR_SIZE];

    printf("\nEnter the node name to modify Subscribe :: ");
    scanf("%s", nodeName);

    EdgeMessage* msg = createEdgeSubMessage(ep, nodeName, 0, Edge_Modify_Sub);
    if(IS_NULL(msg))
    {
        printf("Error : EdgeMalloc failed for msg in test subscription\n");
        return;
    }

    double samplingInterval;
    printf("\nEnter number of sampling interval[millisecond] (minimum : 100ms) :: ");
    scanf("%lf", &samplingInterval);
    int keepalivetime = (1 > (int) (ceil(10000.0 / 0.0))) ? 1 : (int) ceil(10000.0 / 0.0);
    insertSubParameter(&msg, nodeName, Edge_Modify_Sub, samplingInterval, 0.0, keepalivetime, 10000, 1, true, 0, 50);

    EdgeResult result = sendRequest(msg);
    if (result.code == STATUS_OK)
    {
        printf("SUBSCRPTION MODIFY SUCCESSFULL\n");
    }

    destroyEdgeMessage (msg);
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

    EdgeMessage* msg = createEdgeSubMessage(ep, nodeName, 0, Edge_Republish_Sub);
    if(IS_NULL(msg))
    {
        printf("Error : EdgeMalloc failed for msg in test subscription\n");
        return;
    }
    EdgeResult result = sendRequest(msg);
    printf("REPUBLISH RESULT : %d\n",  result.code);
    if (result.code == STATUS_OK)
    {
        printf("REPUBLISH SUCCESSFULL\n");
    }

    destroyEdgeMessage(msg);
}

static void testSubDelete()
{
    char *ep = getEndPoint_input();
    if (ep == NULL)
    {
        printf("Client not connected to any endpoints\n\n");
        return ;
    }

    // Get the list of browse names and display them to user.
    testBrowseViews(ep);

    char nodeName[MAX_CHAR_SIZE];
    printf("\nEnter the node name to delete Subscribe :: ");
    scanf("%s", nodeName);

    EdgeMessage* msg = createEdgeSubMessage(ep, nodeName, 0, Edge_Delete_Sub);
    if(IS_NULL(msg))
    {
        printf("Error : EdgeMalloc failed for msg in test subscription\n");
        return;
    }

    EdgeResult result = sendRequest(msg);
    printf("DELETE RESULT : %d\n",  result.code);
    if (result.code == STATUS_OK)
    {
        printf("SUBSCRPTION DELETED SUCCESSFULL\n");
    }

    destroyEdgeMessage(msg);
}

static void *getNewValuetoWrite(int type, int num_values)
{
    if(num_values < 1)
    {
        printf("Number of elements cannot be less than 1.\n");
        return NULL;
    }

    printf("Enter the new value to write :: ");
    switch (type)
    {
        case Boolean:
            {
                int *val = (int *) EdgeMalloc(sizeof(int) * num_values);
                if(IS_NULL(val))
                    return NULL;
                for (int i = 0; i < num_values; i++)
                    scanf("%d", &val[i]);
                return (void *) val;
            }
            break;
        case SByte:
            {
                int8_t *val = (int8_t *) EdgeMalloc(sizeof(int8_t) * num_values);
                if(IS_NULL(val))
                    return NULL;
                for (int i = 0; i < num_values; i++)
                    scanf("%" SCNd8, &val[i]);
                return (void *) val;
            }
        case Byte:
            {
                uint8_t *val = (uint8_t *) EdgeMalloc(sizeof(uint8_t) * num_values);
                if(IS_NULL(val))
                    return NULL;
                for (int i = 0; i < num_values; i++)
                    scanf("%" SCNu8, &val[i]);
                return (void *) val;
            }
            break;
        case Int16:
            {
                int16_t *val = (int16_t *) EdgeMalloc(sizeof(int16_t) * num_values);
                if(IS_NULL(val))
                    return NULL;
                for (int i = 0; i < num_values; i++)
                    scanf("%" SCNd16, &val[i]);
                return (void *) val;
            }
        case UInt16:
            {
                uint16_t *val = (uint16_t *) EdgeMalloc(sizeof(uint16_t) * num_values);
                if(IS_NULL(val))
                    return NULL;
                for (int i = 0; i < num_values; i++)
                    scanf("%" SCNu16, &val[i]);
                return (void *) val;
            }
            break;
        case Int32:
            {
                int32_t *val = (int32_t *) EdgeMalloc(sizeof(int32_t) * num_values);
                if(IS_NULL(val))
                    return NULL;
                for (int i = 0; i < num_values; i++)
                    scanf("%" SCNd32, &val[i]);
                return (void *) val;
            }
            break;
        case UInt32:
            {
                uint32_t *val = (uint32_t *) EdgeMalloc(sizeof(uint32_t) * num_values);
                if(IS_NULL(val))
                    return NULL;
                for (int i = 0; i < num_values; i++)
                    scanf("%" SCNu32, &val[i]);
                return (void *) val;
            }
            break;
        case Int64:
            {
                int64_t *val = (int64_t *) EdgeMalloc(sizeof(int64_t) * num_values);
                if(IS_NULL(val))
                    return NULL;
                for (int i = 0; i < num_values; i++)
                    scanf("%" SCNd64, &val[i]);
                return (void *) val;
            }
            break;
        case UInt64:
            {
                uint64_t *val = (uint64_t *) EdgeMalloc(sizeof(uint64_t) * num_values);
                if(IS_NULL(val))
                    return NULL;
                for (int i = 0; i < num_values; i++)
                    scanf("%" SCNu64, &val[i]);
                return (void *) val;
            }
            break;
        case Float:
            {
                float *val = (float *) EdgeMalloc(sizeof(float) * num_values);
                if(IS_NULL(val))
                    return NULL;
                for (int i = 0; i < num_values; i++)
                    scanf("%g", &val[i]);
                return (void *) val;
            }
            break;
        case Double:
            {
                double *val = (double *) EdgeMalloc(sizeof(double) * num_values);
                if(IS_NULL(val))
                    return NULL;
                for (int i = 0; i < num_values; i++)
                scanf("%lf", &val[i]);
                return (void *) val;
            }
            break;
        case String:
            {
                if(num_values > 1)
                {
                    char **retStr = (char**) malloc(sizeof(char*) * num_values);
                    if(IS_NULL(retStr))
                        return NULL;
                    int len;
                    char val[MAX_CHAR_SIZE];
                    for (int i = 0; i < num_values; i++)
                    {
                        scanf("%s", val);
                        len  = strlen(val);
                        retStr[i] = (char *) EdgeMalloc(len + 1);
                        if(IS_NULL(retStr[i]))
                        {
                            EdgeFree(retStr);
                            return NULL;
                        }
                        strncpy(retStr[i], val, len+1);
                    }
                    return (void *) retStr;
                }
                else
                {
                    char *retStr = NULL;
                    char val[MAX_CHAR_SIZE];
                    scanf("%s", val);
                    size_t len  = strlen(val);
                    retStr = (char *) EdgeMalloc(len + 1);
                    if(IS_NULL(retStr))
                        return NULL;
                    strncpy(retStr, val, len+1);
                    return (void *) retStr;
                }
            }
            break;
        default:
            break;
    }

    return NULL;
}

static void print_menu()
{
    printf("\n=============== OPC UA =======================\n\n");

    printf("find_servers : Get a list of all registered servers at the given server. \n");
    printf("get_endpoints : Get a list of all endpoints supported by a server. \n");
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
        else if (!strcmp(command, "find_servers"))
        {
            testFindServers();
        }
        else if (!strcmp(command, "get_endpoints"))
        {
            char endpointUri[MAX_ADDRESS_SIZE];
            printf("[Please input server endpoint uri (Ex: opc.tcp://hostname:port/path)]: ");
            scanf("%s", endpointUri);

            connect = false;
            testGetEndpoints(endpointUri);
        }
        else if (!strcmp(command, "start"))
        {
            printf("[Please input server endpoint uri (Ex: opc.tcp://hostname:port/path)] : ");
            scanf("%s", endpointUri);

            connect = true;
            testGetEndpoints(endpointUri);

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
            char *ep = getEndPoint_input();
            if (ep == NULL)
            {
                printf("Client not connected to any endpoints\n\n");
                break;
            }
            testBrowses(ep);
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
            stopFlag = true;
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
