/******************************************************************
 *
 * Copyright 2017 Samsung Electronics All Rights Reserved.
 *
 *
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *      http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 *
 ******************************************************************/

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>
#include <inttypes.h>
#include <signal.h>

#include <time.h>

#ifndef _WIN32
#include <unistd.h>
#endif

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
static bool bIsConnected = false;

static uint8_t supportedApplicationTypes = EDGE_APPLICATIONTYPE_SERVER | EDGE_APPLICATIONTYPE_DISCOVERYSERVER |
    EDGE_APPLICATIONTYPE_CLIENTANDSERVER;


typedef struct EndPointList {
    char *endpoint;
    struct EndPointList* next;
} EndPointList;

static EndPointList* epList = NULL;
static EdgeConfigure *config = NULL;

int maxReferencesPerNode = 0;

static void add_to_endpoint_list(char *endpoint);
static EndPointList *remove_from_endpoint_list(char *endpoint);

static void startClient(char *addr, int port, char *securityPolicyUri, char *endpoint);
static void *getNewValuetoWrite(int type, int num_values);

static void showNodeId(Edge_NodeId *id)
{
    if(IS_NULL(id))
        return;

    switch (id->identifierType)
    {
        case EDGE_INTEGER:
            printf("Numeric: %d\n", id->identifier.numeric);
            break;
        case EDGE_STRING:
            printf("String: %s\n", (char *) id->identifier.string.data);
            break;
        case EDGE_BYTESTRING:
            printf("ByteString: %s\n", (char *) id->identifier.byteString.data);
            break;
        case EDGE_UUID:
            {
                Edge_Guid val = id->identifier.guid;
                char valueStr[37];
                snprintf(valueStr, 37, "%08x-%04x-%04x-%02x%02x-%02x%02x%02x%02x%02x%02x",
                        val.data1, val.data2, val.data3, val.data4[0], val.data4[1], val.data4[2],
                        val.data4[3], val.data4[4], val.data4[5], val.data4[6], val.data4[7]);
                printf("GUID: %s\n", valueStr);
            }
            break;
    }
}

static void response_msg_cb (EdgeMessage *data)
{
    if (data->type == GENERAL_RESPONSE)
    {
        printf("[Application response Callback] General response\n");
        int len = data->responseLength;
        if (0 == len)
            printf("Msg id : [%" PRIu32 "] \n", data->message_id);
        int idx = 0;
        for (idx = 0; idx < len; idx++)
        {
            if (data->responses[idx]->message != NULL)
            {
                if (data->command == CMD_READ || data->command == CMD_READ_SAMPLING_INTERVAL
                    || data->command == CMD_METHOD)
                {
                    printf("Msg id : [%" PRIu32 "] , Response Received ::  ", data->message_id);
                    if (data->responses[idx]->message->isArray)
                    {
                        // Handle Output array
                        int arrayLen = data->responses[idx]->message->arrayLength;
                        if (data->responses[idx]->type == EDGE_NODEID_BOOLEAN)
                        {
                            /* Handle Boolean output array */
                            for (int arrayIdx = 0; arrayIdx < arrayLen; arrayIdx++)
                            {
                                printf("%d  ", ((bool *) data->responses[idx]->message->value)[arrayIdx]);
                            }
                        }
                        else if (data->responses[idx]->type == EDGE_NODEID_BYTE)
                        {
                            /* Handle EDGE_NODEID_BYTE output array */
                            for (int arrayIdx = 0; arrayIdx < arrayLen; arrayIdx++)
                            {
                                printf("%" PRIu8 " ", ((uint8_t *) data->responses[idx]->message->value)[arrayIdx]);
                            }
                        }
                        else if (data->responses[idx]->type == EDGE_NODEID_SBYTE)
                        {
                            /* Handle EDGE_NODEID_SBYTE output array */
                            for (int arrayIdx = 0; arrayIdx < arrayLen; arrayIdx++)
                            {
                                printf("%" PRId8 " ", ((int8_t *) data->responses[idx]->message->value)[arrayIdx]);
                            }
                        }
                        else if (data->responses[idx]->type == EDGE_NODEID_INT16)
                        {
                            /* Handle int16 output array */
                            for (int arrayIdx = 0; arrayIdx < arrayLen; arrayIdx++)
                            {
                                printf("%" PRId16 "  ", ((int16_t *) data->responses[idx]->message->value)[arrayIdx]);
                            }
                        }
                        else if (data->responses[idx]->type == EDGE_NODEID_UINT16)
                        {
                            /* Handle UEDGE_NODEID_INT16 output array */
                            for (int arrayIdx = 0; arrayIdx < arrayLen; arrayIdx++)
                            {
                                printf("%" PRIu16 "  ", ((uint16_t *) data->responses[idx]->message->value)[arrayIdx]);
                            }
                        }
                        else if (data->responses[idx]->type == EDGE_NODEID_INT32)
                        {
                            /* Handle EDGE_NODEID_INT32 output array */
                            for (int arrayIdx = 0; arrayIdx < arrayLen; arrayIdx++)
                            {
                                printf("%d  ", ((int32_t *) data->responses[idx]->message->value)[arrayIdx]);
                            }
                        }
                        else if (data->responses[idx]->type == EDGE_NODEID_UINT32)
                        {
                            /* Handle UEDGE_NODEID_INT32 output array */
                            for (int arrayIdx = 0; arrayIdx < arrayLen; arrayIdx++)
                            {

                                printf("%u  ", ((uint32_t *) data->responses[idx]->message->value)[arrayIdx]);
                            }
                        }
                        else if (data->responses[idx]->type == EDGE_NODEID_INT64)
                        {
                            /* Handle EDGE_NODEID_INT64 output array */
                            for (int arrayIdx = 0; arrayIdx < arrayLen; arrayIdx++)
                            {
                                printf("%ld  ", ((long int *) data->responses[idx]->message->value)[arrayIdx]);
                            }
                        }
                        else if (data->responses[idx]->type == EDGE_NODEID_UINT64)
                        {
                            /* Handle UEDGE_NODEID_INT64 output array */
                            for (int arrayIdx = 0; arrayIdx < arrayLen; arrayIdx++)
                            {
                                printf("%" PRIu64 "  ", ((uint64_t *) data->responses[idx]->message->value)[arrayIdx]);
                            }
                        }
                        else if (data->responses[idx]->type == EDGE_NODEID_FLOAT)
                        {
                            /* Handle EDGE_NODEID_FLOAT output array */
                            for (int arrayIdx = 0; arrayIdx < arrayLen; arrayIdx++)
                            {
                                printf("%g  ", ((float *) data->responses[idx]->message->value)[arrayIdx]);
                            }
                        }
                        else if (data->responses[idx]->type == EDGE_NODEID_DOUBLE)
                        {
                            /* Handle EDGE_NODEID_DOUBLE output array */
                            for (int arrayIdx = 0; arrayIdx < arrayLen; arrayIdx++)
                            {
                                printf("%g  ", ((double *) data->responses[idx]->message->value)[arrayIdx]);
                            }
                        }
                        else if (data->responses[idx]->type == EDGE_NODEID_STRING
                                || data->responses[idx]->type == EDGE_NODEID_BYTESTRING
                                 || data->responses[idx]->type == EDGE_NODEID_GUID
                                 || data->responses[idx]->type == EDGE_NODEID_XMLELEMENT)
                        {
                            /* Handle String/ByteString/Guid/XmlElement output array */
                            char **values = ((char **) data->responses[idx]->message->value);
                            for (int arrayIdx = 0; arrayIdx < arrayLen; arrayIdx++)
                            {
                                printf("%s  ", values[arrayIdx]);
                            }
                        }
                        else if (data->responses[idx]->type == EDGE_NODEID_DATETIME)
                        {
                            /* Handle DateTime output array */
                            for (int arrayIdx = 0; arrayIdx < arrayLen; arrayIdx++)
                            {
                                int64_t value = ((int64_t *) data->responses[idx]->message->value)[arrayIdx];
                                printf("%" PRId64 "  ", value);
                            }
                        }
                        else if (data->responses[idx]->type == EDGE_NODEID_NODEID)
                        {
                            /* Handle NodeId output array */
                            printf("NodeId output array length :: %d\n", arrayLen);
                            Edge_NodeId **nodeId = (Edge_NodeId **) data->responses[idx]->message->value;
                            for (int arrayIdx = 0; arrayIdx < arrayLen; arrayIdx++)
                                showNodeId(nodeId[arrayIdx]);
                        }
                        else if (data->responses[idx]->type == EDGE_NODEID_QUALIFIEDNAME)
                        {
                            /* Handle QualifiedName output array */
                            printf("QualifiedName output array length :: %d\n", arrayLen);
                            Edge_QualifiedName **qn = (Edge_QualifiedName **) data->responses[idx]->message->value;
                            for (int arrayIdx = 0; arrayIdx < arrayLen; arrayIdx++)
                                printf("[NameSpace Index: %" PRIu16 ", Name: %s]\n", qn[arrayIdx]->namespaceIndex, qn[arrayIdx]->name.data);
                        }
                        else if (data->responses[idx]->type == EDGE_NODEID_LOCALIZEDTEXT)
                        {
                            /* Handle LocalizedText output array */
                            printf("LocalizedText output array length :: %d\n", arrayLen);
                            Edge_LocalizedText **lt = (Edge_LocalizedText **) data->responses[idx]->message->value;
                            for (int arrayIdx = 0; arrayIdx < arrayLen; arrayIdx++)
                                printf("[Locale: %s, Text: %s]\n", (uint8_t*)lt[arrayIdx]->locale.data, (uint8_t*)lt[arrayIdx]->text.data);
                        }
                        printf("\n");
                    }
                    else
                    {
                        if (data->responses[idx]->type == EDGE_NODEID_BOOLEAN)
                            printf("[%d]\n",
                                   *((bool *)data->responses[idx]->message->value));
                        else if (data->responses[idx]->type == EDGE_NODEID_BYTE)
                            printf("[%" PRIu8 "]\n",
                                   *((uint8_t *)data->responses[idx]->message->value));
                        else if (data->responses[idx]->type == EDGE_NODEID_SBYTE)
                            printf("[%" PRId8 "]\n",
                                   *((int8_t *)data->responses[idx]->message->value));
                        else if (data->responses[idx]->type == EDGE_NODEID_BYTESTRING)
                            printf("[%s]\n",
                                   (char *)data->responses[idx]->message->value);
                        else if (data->responses[idx]->type == EDGE_NODEID_DATETIME)
                            printf("[%" PRId64 "]\n",
                                   *((int64_t *)data->responses[idx]->message->value));
                        else if (data->responses[idx]->type == EDGE_NODEID_DOUBLE)
                            printf(" [%g]\n",
                                   *((double *)data->responses[idx]->message->value));
                        else if (data->responses[idx]->type == EDGE_NODEID_FLOAT)
                            printf("[%g]\n",
                                   *((float *)data->responses[idx]->message->value));
                        else if (data->responses[idx]->type == EDGE_NODEID_INT16)
                            printf("[%" PRId16 "]\n",
                                   *((int16_t *)data->responses[idx]->message->value));
                        else if (data->responses[idx]->type == EDGE_NODEID_UINT16)
                            printf("[%" PRIu16 "]\n",
                                   *((uint16_t *)data->responses[idx]->message->value));
                        else if (data->responses[idx]->type == EDGE_NODEID_INT32)
                            printf("[%" PRId32 "]\n",
                                   *((int32_t *)data->responses[idx]->message->value));
                        else if (data->responses[idx]->type == EDGE_NODEID_UINT32)
                            printf("[%" PRIu32 "]\n",
                                   *((uint32_t *)data->responses[idx]->message->value));
                        else if (data->responses[idx]->type == EDGE_NODEID_INT64)
                            printf("[%" PRId64 "]\n",
                                   *((int64_t *)data->responses[idx]->message->value));
                        else if (data->responses[idx]->type == EDGE_NODEID_UINT64)
                            printf("[%" PRIu64 "]\n",
                                   *((uint64_t *)data->responses[idx]->message->value));
                        else if (data->responses[idx]->type == EDGE_NODEID_STRING
                                || data->responses[idx]->type == EDGE_NODEID_XMLELEMENT
                                || data->responses[idx]->type == EDGE_NODEID_GUID)
                            printf("[%s]\n",
                                   (char *)data->responses[idx]->message->value);
                        else if (data->responses[idx]->type == EDGE_NODEID_LOCALIZEDTEXT)
                        {
                            Edge_LocalizedText *lt = (Edge_LocalizedText *) data->responses[idx]->message->value;
                            printf("[Locale: %s, Text: %s]\n", (uint8_t*)lt->locale.data, (uint8_t*)lt->text.data);
                        }
                        else if (data->responses[idx]->type == EDGE_NODEID_QUALIFIEDNAME)
                        {
                            Edge_QualifiedName *lt = (Edge_QualifiedName *) data->responses[idx]->message->value;
                            printf("[NameSpace Index: %" PRIu16 ", Name: %s]\n", lt->namespaceIndex, lt->name.data);
                        }
                        else if (data->responses[idx]->type == EDGE_NODEID_NODEID)
                        {
                            Edge_NodeId *nodeId = (Edge_NodeId *) data->responses[idx]->message->value;
                            showNodeId(nodeId);
                        }
                    }
                }
                else if (data->command == CMD_WRITE)
                {
                    printf("\nMsg id : : [%" PRIu32 "] ,  Write response :: %s\n", data->message_id,
                           (char *) data->responses[idx]->message->value);
                }
                // Diagnostics information
//                if (data->responses[idx]->m_diagnosticInfo)
//                {
//                    printf("Diagnostics information\n");
//                    printf("symbolicId :: %d, localizedText : %d, additionalInfo : %s , msg :: %s\n" ,
//                           data->responses[idx]->m_diagnosticInfo->symbolicId,
//                           data->responses[idx]->m_diagnosticInfo->localizedText,
//                           data->responses[idx]->m_diagnosticInfo->additionalInfo,
//                           data->responses[idx]->m_diagnosticInfo->msg);
//                }
            }
        }
    }
}

static void monitored_msg_cb (EdgeMessage *data)
{
    if (data->type == REPORT)
    {
        struct tm *lt = data->serverTime.timeInfo;
        struct timeval val = data->serverTime.tv;

        printf("[Application response Callback] Monitored Item Response received\n");
        int len = data->responseLength;
        int idx = 0;
        for (idx = 0; idx < len; idx++)
        {
            printf("Msg id : [%" PRIu32 "] , [Node Name] : %s\n", data->message_id, data->responses[idx]->nodeInfo->valueAlias);
            printf("Monitored Time : [%d-%02d-%02d %02d:%02d:%02d.%06ld], resLength: %d, resIdx: %d\n",
                       lt->tm_year+1900, lt->tm_mon+1, lt->tm_mday, lt->tm_hour, lt->tm_min, lt->tm_sec, val.tv_usec, len, idx);
            if (data->responses[idx]->message == NULL)
            {
                printf("data->responses[%d]->message is NULL\n", idx);
                continue;
            }

            if (data->responses[idx]->message->isArray)
            {
                // Handle Output array
                int arrayLen = data->responses[idx]->message->arrayLength;
                if (data->responses[idx]->type == EDGE_NODEID_BOOLEAN)
                {
                    /* Handle Boolean output array */
                    for (int arrayIdx = 0; arrayIdx < arrayLen; arrayIdx++)
                    {
                        printf("%d  ", ((bool *) data->responses[idx]->message->value)[arrayIdx]);
                    }
                }
                else if (data->responses[idx]->type == EDGE_NODEID_BYTE)
                {
                    /* Handle EDGE_NODEID_BYTE output array */
                    for (int arrayIdx = 0; arrayIdx < arrayLen; arrayIdx++)
                    {
                        printf("%" PRIu8 " ", ((uint8_t *) data->responses[idx]->message->value)[arrayIdx]);
                    }
                }
                else if (data->responses[idx]->type == EDGE_NODEID_SBYTE)
                {
                    /* Handle EDGE_NODEID_SBYTE output array */
                    for (int arrayIdx = 0; arrayIdx < arrayLen; arrayIdx++)
                    {
                        printf("%" PRId8 " ", ((int8_t *) data->responses[idx]->message->value)[arrayIdx]);
                    }
                }
                else if (data->responses[idx]->type == EDGE_NODEID_INT16)
                {
                    /* Handle int16 output array */
                    for (int arrayIdx = 0; arrayIdx < arrayLen; arrayIdx++)
                    {
                        printf("%" PRId16 "  ", ((int16_t *) data->responses[idx]->message->value)[arrayIdx]);
                    }
                }
                else if (data->responses[idx]->type == EDGE_NODEID_INT16)
                {
                    /* Handle UEDGE_NODEID_INT16 output array */
                    for (int arrayIdx = 0; arrayIdx < arrayLen; arrayIdx++)
                    {
                        printf("%" PRIu16 "  ", ((uint16_t *) data->responses[idx]->message->value)[arrayIdx]);
                    }
                }
                else if (data->responses[idx]->type == EDGE_NODEID_INT32)
                {
                    /* Handle EDGE_NODEID_INT32 output array */
                    for (int arrayIdx = 0; arrayIdx < arrayLen; arrayIdx++)
                    {
                        printf("%d  ", ((int32_t *) data->responses[idx]->message->value)[arrayIdx]);
                    }
                }
                else if (data->responses[idx]->type == EDGE_NODEID_INT32)
                {
                    /* Handle UEDGE_NODEID_INT32 output array */
                    for (int arrayIdx = 0; arrayIdx < arrayLen; arrayIdx++)
                    {
                        printf("%u  ", ((uint32_t *) data->responses[idx]->message->value)[arrayIdx]);
                    }
                }
                else if (data->responses[idx]->type == EDGE_NODEID_INT64)
                {
                    /* Handle EDGE_NODEID_INT64 output array */
                    for (int arrayIdx = 0; arrayIdx < arrayLen; arrayIdx++)
                    {
                        printf("%ld  ", ((long int *) data->responses[idx]->message->value)[arrayIdx]);
                    }
                }
                else if (data->responses[idx]->type == EDGE_NODEID_INT64)
                {
                    /* Handle UEDGE_NODEID_INT64 output array */
                    for (int arrayIdx = 0; arrayIdx < arrayLen; arrayIdx++)
                    {
                        printf("%" PRIu64 "   ", ((uint64_t *) data->responses[idx]->message->value)[arrayIdx]);
                    }
                }
                else if (data->responses[idx]->type == EDGE_NODEID_FLOAT)
                {
                    /* Handle EDGE_NODEID_FLOAT output array */
                    for (int arrayIdx = 0; arrayIdx < arrayLen; arrayIdx++)
                    {
                        printf("%g  ", ((float *) data->responses[idx]->message->value)[arrayIdx]);
                    }
                }
                else if (data->responses[idx]->type == EDGE_NODEID_DOUBLE)
                {
                    /* Handle EDGE_NODEID_DOUBLE output array */
                    for (int arrayIdx = 0; arrayIdx < arrayLen; arrayIdx++)
                    {
                        printf("%g  ", ((double *) data->responses[idx]->message->value)[arrayIdx]);
                    }
                }
                else if (data->responses[idx]->type == EDGE_NODEID_STRING
                        || data->responses[idx]->type == EDGE_NODEID_BYTESTRING
                         || data->responses[idx]->type == EDGE_NODEID_GUID)
                {
                    /* Handle String/ByteString/Guid output array */
                    char **values = ((char **) data->responses[idx]->message->value);
                    for (int arrayIdx = 0; arrayIdx < arrayLen; arrayIdx++)
                    {
                        printf("%s  ", values[arrayIdx]);
                    }
                }
                else if (data->responses[idx]->type == EDGE_NODEID_DATETIME)
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
                if (data->responses[idx]->type == EDGE_NODEID_BOOLEAN)
                    printf("[%d]\n",
                           *((int *)data->responses[idx]->message->value));
                else if (data->responses[idx]->type == EDGE_NODEID_INT16)
                    printf("[%d]\n",
                           *((int *)data->responses[idx]->message->value));
                else if (data->responses[idx]->type == EDGE_NODEID_BYTE)
                    printf("[%d]\n",
                           *((uint8_t *)data->responses[idx]->message->value));
                else if (data->responses[idx]->type == EDGE_NODEID_BYTESTRING)
                    printf("[%s]\n",
                           (char *)data->responses[idx]->message->value);
                else if (data->responses[idx]->type == EDGE_NODEID_UINT16)
                    printf("[%d]\n",
                           *((int *)data->responses[idx]->message->value));
                else if (data->responses[idx]->type == EDGE_NODEID_INT32)
                    printf("[%d]\n",
                           *((int *)data->responses[idx]->message->value));
                else if (data->responses[idx]->type == EDGE_NODEID_UINT32)
                    printf("[%d]\n",
                           *((int *)data->responses[idx]->message->value));
                else if (data->responses[idx]->type == EDGE_NODEID_INT64)
                    printf("[%ld]\n",
                           *((long *)data->responses[idx]->message->value));
                else if (data->responses[idx]->type == EDGE_NODEID_UINT64)
                    printf("[%ld]\n",
                           *((long *)data->responses[idx]->message->value));
                else if (data->responses[idx]->type == EDGE_NODEID_FLOAT)
                    printf("[%f]\n",
                           *((float *)data->responses[idx]->message->value));
                else if (data->responses[idx]->type == EDGE_NODEID_DOUBLE)
                    printf("[%f]\n",
                           *((double *)data->responses[idx]->message->value));
                else if (data->responses[idx]->type == EDGE_NODEID_STRING)
                    printf("[%s]\n",
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
                if (nodeId->type == EDGE_INTEGER)
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
//            stopFlag = true;
        }
    }
}

static void status_network_cb (EdgeEndPointInfo *epInfo, EdgeStatusCode status)
{
    if (status == STATUS_DISCONNECTED)
    {
        printf(COLOR_GREEN "[Application Callback] Client disconnected with Server : [ %s ]\n\n" COLOR_RESET,epInfo->endpointUri);
    }
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

            if(bIsConnected)
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
    if(0 == cnt)
    {
        printf("Client not connected to any endpoints\n\n");
        return NULL;
    }
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
        printf("getEndpoint - Status Code: %d\n", res.code);
    }

    destroyEdgeMessage(msg);
}

static void startClient(char *addr, int port, char *securityPolicyUri, char *endpoint)
{
    printf("\n" COLOR_YELLOW "------------------------------------------------------" COLOR_RESET);
    printf("\n" COLOR_YELLOW "                       Client connect            "COLOR_RESET);
    printf("\n" COLOR_YELLOW "------------------------------------------------------" COLOR_RESET
           "\n\n");

    EdgeMessage *msg = createEdgeMessage(endpoint, 0, CMD_START_CLIENT);
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

    EdgeNodeInfo* nodeInfo = createEdgeNodeInfoForNodeId(EDGE_INTEGER, EDGE_NODEID_ROOTFOLDER, SYSTEM_NAMESPACE_INDEX);
    EdgeBrowseParameter param = {DIRECTION_FORWARD, maxReferencesPerNode};
    insertBrowseParameter(&msg, nodeInfo, param);
    printf("\n\n" COLOR_YELLOW "********** Browse RootFolder node in system namespace **********"
           COLOR_RESET "\n");

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

    EdgeNodeInfo* nodeInfo = createEdgeNodeInfoForNodeId(EDGE_INTEGER, EDGE_NODEID_ROOTFOLDER, SYSTEM_NAMESPACE_INDEX);
    EdgeBrowseParameter param = {DIRECTION_FORWARD, maxReferencesPerNode};
    insertBrowseParameter(&msg, nodeInfo, param);
    nodeInfo = createEdgeNodeInfoForNodeId(EDGE_INTEGER, EDGE_NODEID_OBJECTSFOLDER, SYSTEM_NAMESPACE_INDEX);
    insertBrowseParameter(&msg, nodeInfo, param);
    nodeInfo = createEdgeNodeInfo("{2;S;v=0}Object1");
    insertBrowseParameter(&msg, nodeInfo, param);

    printf("\n\n" COLOR_YELLOW
           "********** Browse RootFolder, ObjectsFolder nodes in system namespace and Object1 in namespace 1 **********"
           COLOR_RESET "\n");

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
        #ifndef _WIN32
            usleep(1000);
        #else
	        Sleep(1);
        #endif

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
        return;
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
        return;
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
        return;
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
        return;
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
    EdgeResult ret = insertEdgeMethodParameter(&msg, "{2;S;v=0}sqrt(x)", 1, EDGE_NODEID_DOUBLE, SCALAR, (void *) &input, NULL, 0);
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
            EDGE_NODEID_INT32, ARRAY_1D, NULL, (void *) array, 5);
    if (ret.code != STATUS_OK) {
        printf("Error : insertEdgeMethodParameter has failed\n");
        goto EXIT_METHOD1;
    }
    int delta = 5;
    ret = insertEdgeMethodParameter(&msg, "{2;S;v=0}incrementInc32Array(x,delta)", 2,
            EDGE_NODEID_INT32, SCALAR, (void *) &delta, NULL, 0);
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

    printf("\n|------------------- [Method Call ] - move_start_point \n");

    msg = createEdgeMessage(endpointUri, 1, CMD_METHOD);
    if(IS_NULL(msg))
    {
        printf("Error : Malloc failed for EdgeMessage in test Method\n");
        return;
    }

    int32_t val = 0;
    ret = insertEdgeMethodParameter(&msg, "{2;S;v=0}move_start_point", 1,
                EDGE_NODEID_INT32, SCALAR, (void *)&val, NULL, 0);
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
        printf("\nEnter number of sampling interval[millisecond] (minimum : 25ms) :: ");
        scanf("%lf", &samplingInterval);
        int keepalivetime = 1;	//(1 > (int) (ceil(10000.0 / 0.0))) ? 1 : (int) ceil(10000.0 / 0.0);
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
        return;
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
    printf("\nEnter number of sampling interval[millisecond] (minimum : 25ms) :: ");
    scanf("%lf", &samplingInterval);
    int keepalivetime = 1;	//(1 > (int) (ceil(10000.0 / 0.0))) ? 1 : (int) ceil(10000.0 / 0.0);
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
        return;
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
        return;
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
        case EDGE_NODEID_BOOLEAN:
            {
                int *val = (int *) EdgeMalloc(sizeof(int) * num_values);
                if(IS_NULL(val))
                    return NULL;
                for (int i = 0; i < num_values; i++)
                    scanf("%d", &val[i]);
                return (void *) val;
            }
            break;
        case EDGE_NODEID_SBYTE:
            {
                int8_t *val = (int8_t *) EdgeMalloc(sizeof(int8_t) * num_values);
                if(IS_NULL(val))
                    return NULL;
                for (int i = 0; i < num_values; i++)
                    scanf("%" SCNd8, &val[i]);
                return (void *) val;
            }
        case EDGE_NODEID_BYTE:
            {
                uint8_t *val = (uint8_t *) EdgeMalloc(sizeof(uint8_t) * num_values);
                if(IS_NULL(val))
                    return NULL;
                for (int i = 0; i < num_values; i++)
                    scanf("%" SCNu8, &val[i]);
                return (void *) val;
            }
            break;
        case EDGE_NODEID_INT16:
            {
                int16_t *val = (int16_t *) EdgeMalloc(sizeof(int16_t) * num_values);
                if(IS_NULL(val))
                    return NULL;
                for (int i = 0; i < num_values; i++)
                    scanf("%" SCNd16, &val[i]);
                return (void *) val;
            }
        case EDGE_NODEID_UINT16:
            {
                uint16_t *val = (uint16_t *) EdgeMalloc(sizeof(uint16_t) * num_values);
                if(IS_NULL(val))
                    return NULL;
                for (int i = 0; i < num_values; i++)
                    scanf("%" SCNu16, &val[i]);
                return (void *) val;
            }
            break;
        case EDGE_NODEID_INT32:
            {
                int32_t *val = (int32_t *) EdgeMalloc(sizeof(int32_t) * num_values);
                if(IS_NULL(val))
                    return NULL;
                for (int i = 0; i < num_values; i++)
                    scanf("%" SCNd32, &val[i]);
                return (void *) val;
            }
            break;
        case EDGE_NODEID_UINT32:
            {
                uint32_t *val = (uint32_t *) EdgeMalloc(sizeof(uint32_t) * num_values);
                if(IS_NULL(val))
                    return NULL;
                for (int i = 0; i < num_values; i++)
                    scanf("%" SCNu32, &val[i]);
                return (void *) val;
            }
            break;
        case EDGE_NODEID_INT64:
            {
                int64_t *val = (int64_t *) EdgeMalloc(sizeof(int64_t) * num_values);
                if(IS_NULL(val))
                    return NULL;
                for (int i = 0; i < num_values; i++)
                    scanf("%" SCNd64, &val[i]);
                return (void *) val;
            }
            break;
        case EDGE_NODEID_UINT64:
            {
                uint64_t *val = (uint64_t *) EdgeMalloc(sizeof(uint64_t) * num_values);
                if(IS_NULL(val))
                    return NULL;
                for (int i = 0; i < num_values; i++)
                    scanf("%" SCNu64, &val[i]);
                return (void *) val;
            }
            break;
        case EDGE_NODEID_FLOAT:
            {
                float *val = (float *) EdgeMalloc(sizeof(float) * num_values);
                if(IS_NULL(val))
                    return NULL;
                for (int i = 0; i < num_values; i++)
                    scanf("%g", &val[i]);
                return (void *) val;
            }
            break;
        case EDGE_NODEID_DOUBLE:
            {
                double *val = (double *) EdgeMalloc(sizeof(double) * num_values);
                if(IS_NULL(val))
                    return NULL;
                for (int i = 0; i < num_values; i++)
                scanf("%lf", &val[i]);
                return (void *) val;
            }
            break;
        case EDGE_NODEID_STRING:
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

static void testRobotMethod() {
    printf("\n|------------------- [Method Call ] - move_start_point \n");

    EdgeMessage *msg = createEdgeMessage(endpointUri, 1, CMD_METHOD);
    if(IS_NULL(msg))
    {
        printf("Error : Malloc failed for EdgeMessage in test Method\n");
        return;
    }

    int32_t val = 0;
    EdgeResult ret = insertEdgeMethodParameter(&msg, "{2;S;v=0}move_start_point", 1,
                EDGE_NODEID_INT32, SCALAR, (void *)&val, NULL, 0);
    if (ret.code != STATUS_OK) {
        printf("Error : insertEdgeMethodParameter has failed\n");
        goto EXIT_METHOD2;
    }
    sendRequest(msg);

    EXIT_METHOD2:
    msg->request->methodParams->inpArg[0]->scalarValue = NULL;
    destroyEdgeMessage (msg);
}

static void testRobotSub() {

	  printf("\n" COLOR_YELLOW "------------------------------------------------------" COLOR_RESET);
	  printf("\n" COLOR_YELLOW "                    Subscribe Node             "COLOR_RESET);
	  printf("\n" COLOR_YELLOW "------------------------------------------------------\n" COLOR_RESET);

    char *ep = getEndPoint_input();
    if (ep == NULL)
    {
        return;
    }

    // Get the list of browse names and display them to user.
    testBrowseViews(ep);
    char nodeName[MAX_CHAR_SIZE];
    int num_requests = 2;

    EdgeMessage* msg = createEdgeSubMessage(ep, nodeName, num_requests, Edge_Create_Sub);
    if(IS_NULL(msg))
    {
        printf("Error : EdgeMalloc failed for msg in test subscription\n");
        return;
    }

    for (int i = 0; i < num_requests; i++)
    {
        #ifndef _WIN32
            usleep(1000);
        #else
            Sleep(1);
        #endif
        printf("\nEnter the node #%d name to subscribe :: ", (i + 1));
        scanf("%s", nodeName);

        double samplingInterval;
        printf("\nEnter number of sampling interval[millisecond] (minimum : 25ms) :: ");
        scanf("%lf", &samplingInterval);
        int keepalivetime = 1; //(1 > (int) (ceil(10000.0 / 0.0))) ? 1 : (int) ceil(10000.0 / 0.0);
        insertSubParameter(&msg, nodeName, Edge_Create_Sub, samplingInterval, 0.0, keepalivetime, 10000, 1, true, 0, 50);
    }


    EdgeResult result = sendRequest(msg);
    if (result.code == STATUS_OK)
    {
        printf(COLOR_GREEN "\nSUBSCRPTION IS SUCCESSFULLY\n" COLOR_RESET);
    } else {
        printf("CREATE RESULT : %d\n",  result.code);
    }

    destroyEdgeMessage (msg);
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
    printf("browse_v : browse views\n");
    printf("method : method call\n");
    printf("create_sub : create subscription\n");
    printf("modify_sub : modify subscription\n");
    printf("delete_sub : delete subscription\n");
    printf("set_max_ref : Set maximum references per node\n");
    printf("\n=============== Custom test =======================\n\n");
    printf("call_robot : call robot method \n");
    printf("sub_robot : subscribe robot nodes \n");
    printf("\n=============== Custom test =======================\n\n");
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
        #ifndef _WIN32
            usleep(1000);
        #else
            Sleep(1);
        #endif
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

            bIsConnected = false;
            testGetEndpoints(endpointUri);
        }
        else if (!strcmp(command, "start"))
        {
            printf("[Please input server endpoint uri (Ex: opc.tcp://hostname:port/path)] : ");
            scanf("%s", endpointUri);

            bIsConnected = true;
            testGetEndpoints(endpointUri);

            //startFlag = true;
            print_menu();
        }
        else if (!strcmp(command, "stop"))
        {
            stopClient();
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
            if (ep != NULL)
            {
                testBrowse(ep);
            }
        }
        else if (!strcmp(command, "browse_m"))
        {
            char *ep = getEndPoint_input();
            if (ep != NULL)
            {
                testBrowses(ep);
            }
        }
        else if (!strcmp(command, "browse_v"))
        {
            char *ep = getEndPoint_input();
            if (ep != NULL)
            {
                testBrowseViews(ep);
            }
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
        else if (!strcmp(command, "set_max_ref"))
        {
            printf("Enter a value for maximum references allowed per node:");
            scanf("%d", &maxReferencesPerNode);
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
        else if (!strcmp(command, "call_robot"))
        {
            testRobotMethod();
        }
        else if (!strcmp(command, "sub_robot"))
        {
            testRobotSub();
        }
    }

    return 0;
}
