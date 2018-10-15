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

#include <gtest/gtest.h>
#include <iostream>
#include <inttypes.h>
#include <math.h>

extern "C"
{
#include "opcua_manager.h"
#include "opcua_common.h"
#include "edge_identifier.h"
#include "edge_utils.h"
#include "edge_logger.h"
#include "edge_malloc.h"
#include "open62541.h"
#include "test_common.h"
#include "browse.h"
}

#define TAG "TC"

#define COLOR_YELLOW      "\x1b[33m"
#define COLOR_RESET         "\x1b[0m"

#define LOCALHOST "localhost"
#define ENDPOINT_URI  "opc:tcp://%s:12686/edge-opc-server"
#define IPADDRESS "opc.tcp://localhost:12686"

#define TEST_STR1_R "test1"
#define TEST_STR2_R "test2"
#define TEST_STR3_R "test3"
#define TEST_DOUBLE_R 50.4
#define TEST_INT32_R 40
#define TEST_UINT16_R 30
#define TEST_METHOD_SR 256

#define TEST_STR1_W "apple"
#define TEST_STR2_W "banana"
#define TEST_STR3_W "mango"
#define TEST_DOUBLE_W 33.9
#define TEST_INT32_W 44
#define TEST_UINT16_W 77

#define PRINT(str) std::cout<<str<<std::endl
#define PRINT_ARG(str, arg) std::cout<<str<<" "<<arg<<std::endl

#define VERIFY_NON_NULL(arg, retVal) { if (!(arg)) { printf( \
             #arg " is NULL"); return (retVal); } }
#define VERIFY_NON_NULL_NR(arg) { if (!(arg)) { printf( \
             #arg " is NULL"); return; } }

static char ipAddress[128];
static char endpointUri[512];
static EdgeEndPointInfo *epInfo = NULL;
static EdgeConfigure *config = NULL;

static uint8_t supportedApplicationTypes = EDGE_APPLICATIONTYPE_SERVER | EDGE_APPLICATIONTYPE_DISCOVERYSERVER |
    EDGE_APPLICATIONTYPE_CLIENTANDSERVER | EDGE_APPLICATIONTYPE_CLIENT;

static bool startServerFlag = false;
static bool startClientFlag = false;
static bool readNodeFlag = true;
static bool browseNodeFlag = false;
static bool methodCallFlag = false;
static bool errorCallFlag = false;

char node_arr[46][30] =
{
    "{2;S;v=12}String1", "{2;S;v=12}String2", "{2;S;v=12}String3", "{2;S;v=11}Double", "{2;S;v=6}Int32",
    "{2;S;v=5}UInt16", "{2;S;v=15}ByteString", "{2;S;v=3}Byte", "{2;S;v=12}Error", "{2;S;v=14}Guid",
    "{2;S;v=11}DoubleArray", "{2;S;v=12}CharArray", "{2;S;v=15}ByteStringArray", "{2;S;v=14}GuidArray",
    "{2;S;v=21}LocalizedText", "{2;S;v=20}QualifiedName", "{2;S;v=17}NodeId1", "{2;S;v=17}NodeId2",
    "{2;S;v=17}NodeId3", "{2;S;v=17}NodeId4", "{2;S;v=10}Float", "{2;S;v=16}XmlElement",
    "{2;S;v=7}UInt32", "{2;S;v=9}UInt64", "{2;S;v=4}Int16", "{2;S;v=8}Int64", "{2;S;v=7}UInt32writeonly",
    "{2;S;v=9}UInt64readonly", "{2;S;v=1}Boolean", "{2;S;v=13}DateTime", "{2;S;v=2}SByte",
    "{2;S;v=1}BoolArray", "{2;S;v=2}SByteArray", "{2;S;v=3}ByteArray", "{2;S;v=8}Int64Array",
    "{2;S;v=5}UInt16Array", "{2;S;v=4}Int16Array", "{2;S;v=7}UInt32Array", "{2;S;v=6}Int32Array",
    "{2;S;v=9}UInt64Array", "{2;S;v=10}FloatArray", "{2;S;v=13}DateTimeArray", "{2;S;v=16}XmlElementArray",
    "{2;S;v=17}NodeIdArray", "{2;S;v=20}QualifiedNameArray", "{2;S;v=21}LocalizedTextArray"
};

static int method_arr[5] =
{ 15, 25, 35, 45, 55 };

extern void testRead_P1(char *endpointUri);
extern void testRead_P2(char *endpointUri);
extern void testRead_P3(char *endpointUri);
extern void testRead_P4(char *endpointUri);
extern void testRead_P5(char *endpointUri);
extern void testReadWithoutEndpoint();
extern void testReadWithoutCommand();
extern void testReadWithoutValueAlias(char *endpointUri);
extern void testReadWithoutMessage();

extern void testWrite_P1(char *endpointUri);
extern void testWrite_P2(char *endpointUri);
extern void testWrite_P3(char *endpointUri);
extern void testWrite_P4(char *endpointUri);
extern void testWriteWithoutCommand();
extern void testWriteWithoutEndpoint();
extern void testWriteWithoutValueAlias(char *endpointUri);
extern void testWriteWithoutMessage();

extern void testMethod_P1(char *endpointUri);
extern void testMethod_P2(char *endpointUri);
extern void testMethod_P3(char *endpointUri);
extern void testMethod_P4(char *endpointUri);
extern void testMethodWithoutCommand();
extern void testMethodWithoutParam();
extern void testMethodWithoutEndpoint();
extern void testMethodWithoutValueAlias(char *endpointUri);
extern void testMethodWithoutMessage();

extern void testSubscription_P1(char *endpointUri);
extern void testSubscription_P2(char *endpointUri);
extern void testSubscription_P3(char *endpointUri);
extern void testSubscriptionWithoutCommand(char *endpointUri);
extern void testSubscriptionWithoutEndpoint();
extern void testSubscriptionWithoutValueAlias(char *endpointUri);
extern void testSubscriptionWithoutMessage();
extern void testSubscriptionWithoutSubReq(char *endpointUri);

extern "C"
{

    static void startClient(char *addr, int port, char *securityPolicyUri);

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
                printf("Byte String: %s\n", (char *) id->identifier.byteString.data);
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

    static void response_msg_cb(EdgeMessage *data)
    {
        if (data->type == GENERAL_RESPONSE)
        {
            int len = data->responseLength;
            int idx = 0;
            for (idx = 0; idx < len; idx++)
            {
                if (data->responses[idx]->message != NULL)
                {
                    if (data->command == CMD_READ || data->command == CMD_METHOD)
                    {
                        if (data->responses[idx]->message->isArray)
                        {
                            // Handle Output array
                            int arrayLen = data->responses[idx]->message->arrayLength;
                            if (data->responses[idx]->type == EDGE_NODEID_BOOLEAN)
                            {
                                /* Handle Boolean output array */
                                PRINT_ARG("Boolean output array length :: ", arrayLen);
                                for (int arrayIdx = 0; arrayIdx < arrayLen; arrayIdx++)
                                {
                                    PRINT_ARG("  ",
                                            ((bool * ) data->responses[idx]->message->value)[arrayIdx]);
                                }
                            }
                            else if (data->responses[idx]->type == EDGE_NODEID_BYTE)
                            {
                                /* Handle Byte output array */
                                PRINT_ARG("Byte output array length ::", arrayLen);
                                for (int arrayIdx = 0; arrayIdx < arrayLen; arrayIdx++)
                                {
                                    PRINT(
                                            ((int8_t * ) data->responses[idx]->message->value)[arrayIdx]);
                                }
                            }
                            else if (data->responses[idx]->type == EDGE_NODEID_SBYTE)
                            {
                                /* Handle SByte output array */
                                PRINT_ARG("SByte output array length :: ", arrayLen);
                                for (int arrayIdx = 0; arrayIdx < arrayLen; arrayIdx++)
                                {
                                    PRINT(
                                            ((int8_t * ) data->responses[idx]->message->value)[arrayIdx]);
                                }
                            }
                            else if (data->responses[idx]->type == EDGE_NODEID_INT16)
                            {
                                /* Handle int16 output array */
                                PRINT_ARG("Int16 output array length :: ", arrayLen);
                                for (int arrayIdx = 0; arrayIdx < arrayLen; arrayIdx++)
                                {
                                    PRINT(
                                            ((int16_t * ) data->responses[idx]->message->value)[arrayIdx]);
                                }
                            }
                            else if (data->responses[idx]->type == EDGE_NODEID_UINT16)
                            {
                                /* Handle UInt16 output array */
                                PRINT_ARG("UInt16 output array length ::", arrayLen);
                                for (int arrayIdx = 0; arrayIdx < arrayLen; arrayIdx++)
                                {
                                    PRINT(
                                            ((int16_t * ) data->responses[idx]->message->value)[arrayIdx]);
                                }
                            }
                            else if (data->responses[idx]->type == EDGE_NODEID_INT32)
                            {
                                /* Handle Int32 output array */
                                PRINT_ARG("Int32 output array length :: ", arrayLen);
                                if (methodCallFlag)
                                    EXPECT_EQ(arrayLen, 5);
                                for (int arrayIdx = 0; arrayIdx < arrayLen; arrayIdx++)
                                {
                                    PRINT(
                                            ((int32_t * ) data->responses[idx]->message->value)[arrayIdx]);
                                    if (methodCallFlag)
                                        EXPECT_EQ(
                                                ((int32_t * ) data->responses[idx]->message->value)[arrayIdx],
                                                method_arr[arrayIdx]);
                                }
                                methodCallFlag = false;
                            }
                            else if (data->responses[idx]->type == EDGE_NODEID_UINT32)
                            {
                                /* Handle UInt32 output array */
                                PRINT_ARG("UInt32 output array length :: ", arrayLen);
                                for (int arrayIdx = 0; arrayIdx < arrayLen; arrayIdx++)
                                {
                                    PRINT(
                                            ((int32_t * ) data->responses[idx]->message->value)[arrayIdx]);
                                }
                            }
                            else if (data->responses[idx]->type == EDGE_NODEID_INT64)
                            {
                                /* Handle Int64 output array */
                                PRINT_ARG("Int64 output array length :: ", arrayLen);
                                for (int arrayIdx = 0; arrayIdx < arrayLen; arrayIdx++)
                                {
                                    PRINT(
                                            ((long int * ) data->responses[idx]->message->value)[arrayIdx]);
                                }
                            }
                            else if (data->responses[idx]->type == EDGE_NODEID_UINT64)
                            {
                                /* Handle UInt64 output array */
                                PRINT_ARG("UInt64 output array length :: ", arrayLen);
                                for (int arrayIdx = 0; arrayIdx < arrayLen; arrayIdx++)
                                {
                                    PRINT(
                                            ((long int * ) data->responses[idx]->message->value)[arrayIdx]);
                                }
                            }
                            else if (data->responses[idx]->type == EDGE_NODEID_FLOAT)
                            {
                                /* Handle Float output array */
                                PRINT_ARG("Float output array length :: ", arrayLen);
                                for (int arrayIdx = 0; arrayIdx < arrayLen; arrayIdx++)
                                {
                                    PRINT(
                                            ((float * ) data->responses[idx]->message->value)[arrayIdx]);
                                }
                            }
                            else if (data->responses[idx]->type == EDGE_NODEID_DOUBLE)
                            {
                                /* Handle Double output array */
                                PRINT_ARG("Double output array length :: ", arrayLen);
                                for (int arrayIdx = 0; arrayIdx < arrayLen; arrayIdx++)
                                {
                                    PRINT(
                                            ((double * ) data->responses[idx]->message->value)[arrayIdx]);
                                }
                            }
                            else if (data->responses[idx]->type == EDGE_NODEID_STRING
                                    || data->responses[idx]->type == EDGE_NODEID_BYTESTRING
                                    || data->responses[idx]->type == EDGE_NODEID_XMLELEMENT
                                    || data->responses[idx]->type == EDGE_NODEID_GUID)
                            {
                                /* Handle String/ByteString/Guid output array */
                                PRINT_ARG("String/ByteString/Guid/XmlElement output array length :: ",
                                        arrayLen);
                                char **values = ((char **) data->responses[idx]->message->value);
                                for (int arrayIdx = 0; arrayIdx < arrayLen; arrayIdx++)
                                {
                                    PRINT(values[arrayIdx]);
                                }
                            }
                            else if (data->responses[idx]->type == EDGE_NODEID_DATETIME)
                            {
                                /* Handle DateTime output array */
                                PRINT_ARG("DateTime output array length :: ", arrayLen);
                                for (int arrayIdx = 0; arrayIdx < arrayLen; arrayIdx++)
                                {
                                    PRINT(
                                            ((int64_t * ) data->responses[idx]->message->value)[arrayIdx]);
                                }
                            }
                            else if (data->responses[idx]->type == EDGE_NODEID_NODEID)
                            {
                                /* Handle NodeId output array */
                                PRINT_ARG("NodeId output array length :: ", arrayLen);
                                Edge_NodeId **nodeId = (Edge_NodeId **) data->responses[idx]->message->value;
                                for (int arrayIdx = 0; arrayIdx < arrayLen; arrayIdx++)
                                    showNodeId(nodeId[arrayIdx]);
                            }
                            else if (data->responses[idx]->type == EDGE_NODEID_QUALIFIEDNAME)
                            {
                                /* Handle QualifiedName output array */
                                PRINT_ARG("QualifiedName output array length :: ", arrayLen);
                                Edge_QualifiedName **qn = (Edge_QualifiedName **) data->responses[idx]->message->value;
                                for (int arrayIdx = 0; arrayIdx < arrayLen; arrayIdx++)
                                    printf("[NameSpace Index: %" PRIu16 ", Name: %s]\n", qn[arrayIdx]->namespaceIndex, qn[arrayIdx]->name.data);
                            }
                            else if (data->responses[idx]->type == EDGE_NODEID_LOCALIZEDTEXT)
                            {
                                /* Handle LocalizedText output array */
                                PRINT_ARG("LocalizedText output array length :: ", arrayLen);
                                Edge_LocalizedText **lt = (Edge_LocalizedText **) data->responses[idx]->message->value;
                                for (int arrayIdx = 0; arrayIdx < arrayLen; arrayIdx++)
                                    printf("[Locale: %s, Text: %s]\n", (uint8_t*)lt[arrayIdx]->locale.data, (uint8_t*)lt[arrayIdx]->text.data);
                            }
                        }
                        else
                        {
                            if (data->responses[idx]->type == EDGE_NODEID_BOOLEAN)
                                PRINT_ARG(
                                        "[Application response Callback] Data read from node ===>> ",
                                        *((int * )data->responses[idx]->message->value));
                            else if (data->responses[idx]->type == EDGE_NODEID_BYTE)
                                PRINT_ARG(
                                        "[Application response Callback] Data read from node ===>> ",
                                        *((int8_t * )data->responses[idx]->message->value));
                            else if (data->responses[idx]->type == EDGE_NODEID_SBYTE)
                                PRINT_ARG(
                                        "[Application response Callback] Data read from node ===>> ",
                                        *((int8_t * )data->responses[idx]->message->value));
                            else if (data->responses[idx]->type == EDGE_NODEID_BYTESTRING)
                                PRINT_ARG(
                                        "[Application response Callback] Data read from node ===>> ",
                                        (char * )data->responses[idx]->message->value);
                            else if (data->responses[idx]->type == EDGE_NODEID_DATETIME)
                                PRINT_ARG(
                                        "[Application response Callback] Data read from node ===>> ",
                                        *((int64_t * )data->responses[idx]->message->value));
                            else if (data->responses[idx]->type == EDGE_NODEID_DOUBLE)
                            {
                                PRINT_ARG(
                                        "[Application response Callback] Data read from node ===>>  ",
                                        *((double * )data->responses[idx]->message->value));
                            }
                            else if (data->responses[idx]->type == EDGE_NODEID_FLOAT)
                                PRINT_ARG(
                                        "[Application response Callback] Data read from node ===>>  ",
                                        *((float * )data->responses[idx]->message->value));
                            else if (data->responses[idx]->type == EDGE_NODEID_INT16)
                                PRINT_ARG(
                                        "[Application response Callback] Data read from node ===>> ",
                                        *((int16_t * )data->responses[idx]->message->value));
                            else if (data->responses[idx]->type == EDGE_NODEID_UINT16)
                            {
                                PRINT_ARG(
                                        "[Application response Callback] Data read from node ===>> ",
                                        *((int16_t * )data->responses[idx]->message->value));
                            }
                            else if (data->responses[idx]->type == EDGE_NODEID_INT32)
                            {
                                PRINT_ARG(
                                        "[Application response Callback] Data read from node ===>>  ",
                                        *((int32_t * )data->responses[idx]->message->value));
                            }
                            else if (data->responses[idx]->type == EDGE_NODEID_UINT32)
                                PRINT_ARG(
                                        "[Application response Callback] Data read from node ===>>  ",
                                        *((int32_t * )data->responses[idx]->message->value));
                            else if (data->responses[idx]->type == EDGE_NODEID_INT64)
                                PRINT_ARG(
                                        "[Application response Callback] Data read from node ===>>  ",
                                        *((int64_t * )data->responses[idx]->message->value));
                            else if (data->responses[idx]->type == EDGE_NODEID_UINT64)
                                PRINT_ARG(
                                        "[Application response Callback] Data read from node ===>>  ",
                                        *((int64_t * )data->responses[idx]->message->value));
                            else if (data->responses[idx]->type == EDGE_NODEID_STRING || data->responses[idx]->type == EDGE_NODEID_GUID
                                || data->responses[idx]->type == EDGE_NODEID_XMLELEMENT)
                            {
                                PRINT_ARG(
                                        "[Application response Callback] Data read from node ===>>  ",
                                        (char * )data->responses[idx]->message->value);
                            }
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
                        EXPECT_EQ(strcmp("Good", (char * ) data->responses[idx]->message->value),
                                0);
                        PRINT_ARG("[Application response Callback] Write response :: ",
                                (char * ) data->responses[idx]->message->value);
                    }
                }
                PRINT("=========");
            }
        }
    }

    static void monitored_msg_cb(EdgeMessage *data)
    {
        if (data->type == REPORT)
        {
            printf("[Application response Callback] Monitored Item Response received\n");
            int len = data->responseLength;
            int idx = 0;
            for (idx = 0; idx < len; idx++)
            {
                printf("Msg id : [%" PRIu32 "] , [Node Name] : %s\n", data->message_id, data->responses[idx]->nodeInfo->valueAlias);
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
                        /* Handle Byte output array */
                        for (int arrayIdx = 0; arrayIdx < arrayLen; arrayIdx++)
                        {
                            printf("%" PRIu8 " ", ((uint8_t *) data->responses[idx]->message->value)[arrayIdx]);
                        }
                    }
                    else if (data->responses[idx]->type == EDGE_NODEID_SBYTE)
                    {
                        /* Handle SByte output array */
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
                        /* Handle UInt16 output array */
                        for (int arrayIdx = 0; arrayIdx < arrayLen; arrayIdx++)
                        {
                            printf("%" PRIu16 "  ", ((uint16_t *) data->responses[idx]->message->value)[arrayIdx]);
                        }
                    }
                    else if (data->responses[idx]->type == EDGE_NODEID_INT32)
                    {
                        /* Handle Int32 output array */
                        for (int arrayIdx = 0; arrayIdx < arrayLen; arrayIdx++)
                        {
                            printf("%d  ", ((int32_t *) data->responses[idx]->message->value)[arrayIdx]);
                        }
                    }
                    else if (data->responses[idx]->type == EDGE_NODEID_UINT32)
                    {
                        /* Handle UInt32 output array */
                        for (int arrayIdx = 0; arrayIdx < arrayLen; arrayIdx++)
                        {
                            printf("%u  ", ((uint *) data->responses[idx]->message->value)[arrayIdx]);
                        }
                    }
                    else if (data->responses[idx]->type == EDGE_NODEID_INT64)
                    {
                        /* Handle Int64 output array */
                        for (int arrayIdx = 0; arrayIdx < arrayLen; arrayIdx++)
                        {
                            printf("%ld  ", ((long int *) data->responses[idx]->message->value)[arrayIdx]);
                        }
                    }
                    else if (data->responses[idx]->type == EDGE_NODEID_UINT64)
                    {
                        /* Handle UInt64 output array */
                        for (int arrayIdx = 0; arrayIdx < arrayLen; arrayIdx++)
                        {
                            printf("%lu  ", ((ulong *) data->responses[idx]->message->value)[arrayIdx]);
                        }
                    }
                    else if (data->responses[idx]->type == EDGE_NODEID_FLOAT)
                    {
                        /* Handle Float output array */
                        for (int arrayIdx = 0; arrayIdx < arrayLen; arrayIdx++)
                        {
                            printf("%g  ", ((float *) data->responses[idx]->message->value)[arrayIdx]);
                        }
                    }
                    else if (data->responses[idx]->type == EDGE_NODEID_DOUBLE)
                    {
                        /* Handle Double output array */
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
                    if (data->responses[idx]->message != NULL)
                    {
                        if (data->responses[idx]->type == EDGE_NODEID_INT16)
                            printf("[MonitoredItem DataChange callback] Monitored Change Value read from node ===>> [%d]\n",
                                   *((int *)data->responses[idx]->message->value));
                        else if (data->responses[idx]->type == EDGE_NODEID_BYTE)
                            printf("[MonitoredItem DataChange callback] Monitored Change Value read from node ===>> [%d]\n",
                                   *((uint8_t *)data->responses[idx]->message->value));
                        else if (data->responses[idx]->type == EDGE_NODEID_BYTESTRING)
                            printf("[MonitoredItem DataChange callback] Monitored Change Value read from node ===>> [%s]\n",
                                   (char *)data->responses[idx]->message->value);
                        else if (data->responses[idx]->type == EDGE_NODEID_UINT16)
                            printf("[MonitoredItem DataChange callback] Monitored Change Value read from node ===>> [%d]\n",
                                   *((int *)data->responses[idx]->message->value));
                        else if (data->responses[idx]->type == EDGE_NODEID_INT32)
                            printf("[MonitoredItem DataChange callback] Monitored Change Value read from node ===>>  [%d]\n",
                                   *((int *)data->responses[idx]->message->value));
                        else if (data->responses[idx]->type == EDGE_NODEID_UINT32)
                            printf("[MonitoredItem DataChange callback] Monitored Change Value read from node ===>>  [%d]\n",
                                   *((int *)data->responses[idx]->message->value));
                        else if (data->responses[idx]->type == EDGE_NODEID_INT64)
                            printf("[MonitoredItem DataChange callback] Monitored Change Value read from node ===>>  [%ld]\n",
                                   *((long *)data->responses[idx]->message->value));
                        else if (data->responses[idx]->type == EDGE_NODEID_UINT64)
                            printf("[MonitoredItem DataChange callback] Monitored Change Value read from node ===>>  [%ld]\n",
                                   *((long *)data->responses[idx]->message->value));
                        else if (data->responses[idx]->type == EDGE_NODEID_FLOAT)
                            printf("[MonitoredItem DataChange callback] Monitored Change Value read from node  ===>>  [%f]\n",
                                   *((float *)data->responses[idx]->message->value));
                        else if (data->responses[idx]->type == EDGE_NODEID_DOUBLE)
                            printf("[MonitoredItem DataChange callback] Monitored Change Value read from node  ===>>  [%f]\n",
                                   *((double *)data->responses[idx]->message->value));
                        else if (data->responses[idx]->type == EDGE_NODEID_STRING || data->responses[idx]->type == EDGE_NODEID_GUID)
                            printf("[MonitoredItem DataChange callback] Monitored Change Value read from node ===>>  [%s]\n",
                                   ((char *)data->responses[idx]->message->value));
                    }
                }
            }
            printf("\n\n");
        }
    }

    static void error_msg_cb(EdgeMessage *data)
    {
        errorCallFlag = true;
        PRINT_ARG("[error_msg_cb] EdgeStatusCode: ", data->result->code);
    }
    static void browse_msg_cb (EdgeMessage *data)
    {
        browseNodeFlag = true;
        if (data->browseResult)
        {
            if(data->responses[0]->message != NULL)
            {
                PRINT_ARG("\n", (unsigned char *)data->responses[0]->message->value);
            }
        }
    }

    /* status callbacks */
    static void status_start_cb(EdgeEndPointInfo *epInfo, EdgeStatusCode status)
    {
        if (status == STATUS_SERVER_STARTED)
        {
            PRINT("\n[Application Callback] Server started");
            startServerFlag = true;
        }
        if (status == STATUS_CLIENT_STARTED)
        {
            PRINT("[Application Callback] Client connected\n");
            startClientFlag = true;
        }
    }

    static void status_stop_cb(EdgeEndPointInfo *epInfo, EdgeStatusCode status)
    {
        if (status == STATUS_STOP_SERVER)
        {
            PRINT("\n[Application Callback] Server stopped ");
            startServerFlag = false;
        }
        if (status == STATUS_STOP_CLIENT)
        {
            PRINT("[Application Callback] Client disconnected \n\n");
            startClientFlag = false;
        }
    }

    static void status_network_cb(EdgeEndPointInfo *epInfo, EdgeStatusCode status)
    {

    }

    /* discovery callback */
    static void endpoint_found_cb(EdgeDevice *device)
    {
        if (device)
        {
            int num_endpoints = device->num_endpoints;
            int idx = 0;
            for (idx = 0; idx < num_endpoints; idx++)
                startClient(device->address, device->port,
                        device->endpointsInfo[idx]->securityPolicyUri);
        }
    }

    static void device_found_cb(EdgeDevice *device)
    {

    }
}

static void square_method(int inpSize, void **input, int outSize, void **output)
{
    double *inp = (double *) input[0];
    double *sq = (double *) EdgeMalloc(sizeof(double));
    *sq = (*inp) * (*inp);
    output[0] = (void *) sq;

    double *check = (double*) EdgeMalloc(sizeof(double));
    *check = (*sq) * (*sq);
    output[1] = (void *) check;
}

void increment_int32Array_method(int inpSize, void **input, int outSize, void **output)
{
    int32_t *inputArray = (int32_t *) input[0];
    int *delta = (int *) input[1];
    int32_t *outputArray = (int32_t *) EdgeMalloc(sizeof(int32_t) * 5);
    for (int i = 0; i < 5; i++)
    {
        outputArray[i] = inputArray[i] + *delta;
    }
    output[0] = (void *) outputArray;
}

void test_method_print_string(int inpSize, void **input, int outSize, void **output)
{
    char *inp = (char*) input[0];
    printf("\n[print() method called] ");
    printf(" %s\n", inp );
    char *out = (char*) EdgeMalloc(sizeof(char) * (strlen(inp)+1));
    strncpy(out, inp, strlen(inp));
    out[strlen(inp)] = '\0';
    output[0] = (void *) out;
}

void string_method(int inpSize, void **input, int outSize, void **output)
{
    char **inputArray = (char **) input[0];
    char **outputArray = (char **) EdgeMalloc(sizeof(char*) * 5);
    for (int i = 0; i < 5; i++)
    {
        outputArray[i] = (char*) EdgeMalloc(sizeof(char) * (strlen(inputArray[i])+1));
        strncpy(outputArray[i], inputArray[i], strlen(inputArray[i]));
        outputArray[i][strlen(inputArray[i])] = '\0';
    }
    output[0] = (void *) outputArray;
}

static void configureCallbacks()
{
    PRINT("-----INITIALIZING CALLBACKS-----");

    EXPECT_EQ(NULL == config, true);
    config = (EdgeConfigure *) EdgeMalloc(sizeof(EdgeConfigure));
    EXPECT_EQ(NULL == config, false);

    config->recvCallback = (ReceivedMessageCallback *) EdgeMalloc(sizeof(ReceivedMessageCallback));
    EXPECT_EQ(NULL == config->recvCallback, false);

    config->recvCallback->resp_msg_cb = response_msg_cb;
    config->recvCallback->monitored_msg_cb = monitored_msg_cb;
    config->recvCallback->error_msg_cb = error_msg_cb;
    config->recvCallback->browse_msg_cb = browse_msg_cb;

    config->statusCallback = (StatusCallback *) EdgeMalloc(sizeof(StatusCallback));
    EXPECT_EQ(NULL == config->statusCallback, false);

    config->statusCallback->start_cb = status_start_cb;
    config->statusCallback->stop_cb = status_stop_cb;
    config->statusCallback->network_cb = status_network_cb;

    config->discoveryCallback = (DiscoveryCallback *) EdgeMalloc(sizeof(DiscoveryCallback));
    EXPECT_EQ(NULL == config->discoveryCallback, false);

    config->discoveryCallback->endpoint_found_cb = endpoint_found_cb;
    config->discoveryCallback->device_found_cb = device_found_cb;

    config->supportedApplicationTypes = supportedApplicationTypes;

    configure(config);
}

static void cleanCallbacks()
{
    PRINT("-----CLEANING CALLBACKS-----");

    if (config->recvCallback != NULL)
    {
        free(config->recvCallback);
        config->recvCallback = NULL;
    }

    if (config->statusCallback != NULL)
    {
        free(config->statusCallback);
        config->statusCallback = NULL;
    }

    if (config->discoveryCallback != NULL)
    {
        free(config->discoveryCallback);
        config->discoveryCallback = NULL;
    }

    if (config != NULL)
    {
        free(config);
        config = NULL;
    }
}

static void deleteMessage(EdgeMessage *msg, EdgeEndPointInfo *ep)
{
    if (msg != NULL)
    {
        free(msg);
        msg = NULL;
    }

    if (ep != NULL)
    {
        free(ep);
        ep = NULL;
    }
}

static void testFindServers()
{

}

static void browseNodeWithoutCommand()
{
    /* Invalid command */
    int  maxReferencesPerNode = 0;
    EdgeMessage *msg = createEdgeMessage(endpointUri, 1, CMD_INVALID);
    EXPECT_EQ(NULL != msg, true);

    EdgeNodeInfo* nodeInfo = createEdgeNodeInfoForNodeId(EDGE_INTEGER, EDGE_NODEID_ROOTFOLDER,
            SYSTEM_NAMESPACE_INDEX);
    EdgeBrowseParameter param = {DIRECTION_FORWARD, maxReferencesPerNode};
    EdgeResult result = insertBrowseParameter(&msg, nodeInfo, param);
    ASSERT_EQ(result.code, STATUS_PARAM_INVALID);
    browseNodeFlag = false;
}

static void browseNodeWithoutEndpoint()
{
    int  maxReferencesPerNode = 0;
    EdgeMessage *msg = createEdgeMessage(NULL, 1, CMD_BROWSE);
    EXPECT_EQ(NULL != msg, false);

    EdgeNodeInfo* nodeInfo = createEdgeNodeInfoForNodeId(EDGE_INTEGER, EDGE_NODEID_ROOTFOLDER,
            SYSTEM_NAMESPACE_INDEX);
    EdgeBrowseParameter param = {DIRECTION_FORWARD, maxReferencesPerNode};
    insertBrowseParameter(&msg, nodeInfo, param);

    EdgeResult result = sendRequest(msg);
    destroyEdgeMessage(msg);
    ASSERT_EQ(result.code, STATUS_PARAM_INVALID);
}

static void browseNodeWithoutValueAlias()
{
    int  maxReferencesPerNode = 0;
    EdgeMessage *msg = createEdgeMessage(endpointUri, 1, CMD_BROWSE);
    EXPECT_EQ(NULL != msg, true);
    EdgeBrowseParameter param = {DIRECTION_FORWARD, maxReferencesPerNode};
    insertBrowseParameter(&msg, NULL, param);
    EdgeResult result = sendRequest(msg);
    destroyEdgeMessage(msg);
    ASSERT_EQ(result.code, STATUS_PARAM_INVALID);
}

static void browseNodeWithoutMessage()
{
    EdgeResult result = sendRequest(NULL);
    ASSERT_EQ(result.code, STATUS_PARAM_INVALID);
}

static void browseNodeWithoutBrowseParam()
{
    EdgeMessage *msg = createEdgeMessage(endpointUri, 1, CMD_BROWSE);
    EXPECT_EQ(NULL != msg, true);
    /*InsertBrowseParameter is not called */
    /* request is sent without browse params */
    EdgeResult result = sendRequest(msg);
    destroyEdgeMessage(msg);
    ASSERT_EQ(result.code, STATUS_PARAM_INVALID);
}

static void browseNode()
{
    int  maxReferencesPerNode = 0;
    EdgeMessage *msg = createEdgeMessage(endpointUri, 1, CMD_BROWSE);
    EXPECT_EQ(NULL != msg, true);

    EdgeNodeInfo* nodeInfo = createEdgeNodeInfoForNodeId(EDGE_INTEGER, EDGE_NODEID_ROOTFOLDER,
    		SYSTEM_NAMESPACE_INDEX);
    EdgeBrowseParameter param = {DIRECTION_FORWARD, maxReferencesPerNode};
    insertBrowseParameter(&msg, nodeInfo, param);

    EXPECT_EQ(browseNodeFlag, false);
    sendRequest(msg);
    destroyEdgeMessage(msg);
    sleep(1);

    /* Wait some time and check whether browse callback is received */
    EXPECT_EQ(browseNodeFlag, true);
    browseNodeFlag = false;
}

static void browseNodes()
{
    int  maxReferencesPerNode = 0;
    EdgeMessage *msg = createEdgeMessage(endpointUri, 3, CMD_BROWSE);
    EXPECT_EQ(NULL != msg, true);

    EdgeNodeInfo* nodeInfo = createEdgeNodeInfoForNodeId(EDGE_INTEGER, EDGE_NODEID_ROOTFOLDER,
            SYSTEM_NAMESPACE_INDEX);
    EdgeBrowseParameter param = {DIRECTION_FORWARD, maxReferencesPerNode};
    insertBrowseParameter(&msg, nodeInfo, param);
    nodeInfo = createEdgeNodeInfoForNodeId(EDGE_INTEGER, EDGE_NODEID_OBJECTSFOLDER, SYSTEM_NAMESPACE_INDEX);
    insertBrowseParameter(&msg, nodeInfo, param);
    nodeInfo = createEdgeNodeInfo("{2;S;v=0}Object1");
    insertBrowseParameter(&msg, nodeInfo, param);

    EXPECT_EQ(browseNodeFlag, false);
    sendRequest(msg);
    destroyEdgeMessage(msg);
    sleep(1);

    /* Wait some time and check whether browse callback is received */
    EXPECT_EQ(browseNodeFlag, true);
    browseNodeFlag = false;
}

static void browseInvalidNode()
{
    int  maxReferencesPerNode = 0;
    EdgeMessage *msg = createEdgeMessage(endpointUri, 1, CMD_BROWSE);
    EXPECT_EQ(NULL != msg, true);

    EdgeNodeInfo* nodeInfo = createEdgeNodeInfo("{2;S;v=0}InvalidNodeXYZ");
    EdgeBrowseParameter param = {DIRECTION_FORWARD, maxReferencesPerNode};
    insertBrowseParameter(&msg, nodeInfo, param);

    errorCallFlag = false;

    sendRequest(msg);
    destroyEdgeMessage(msg);
    sleep(1);

    /* Wait some time and check whether error callback is received */
    EXPECT_EQ(errorCallFlag, true);
}

static void browseViews()
{
    EdgeMessage *msg = createEdgeMessage(endpointUri, 0, CMD_BROWSE_VIEW);
    EXPECT_EQ(NULL != msg, true);
    EdgeResult result = sendRequest(msg);
    ASSERT_EQ(result.code, STATUS_OK);
    destroyEdgeMessage(msg);
    sleep(1);
}

static void browse_next()
{
    int  maxReferencesPerNode = 1;
    EdgeMessage *msg = createEdgeMessage(endpointUri, 1, CMD_BROWSE);
    EXPECT_EQ(NULL != msg, true);

    EdgeNodeInfo* nodeInfo = createEdgeNodeInfoForNodeId(EDGE_INTEGER, EDGE_NODEID_ROOTFOLDER,
            SYSTEM_NAMESPACE_INDEX);
    EdgeBrowseParameter param = {DIRECTION_FORWARD, maxReferencesPerNode};
    insertBrowseParameter(&msg, nodeInfo, param);

    EXPECT_EQ(browseNodeFlag, false);
    sendRequest(msg);
    destroyEdgeMessage(msg);
    sleep(1);

    /* Wait some time and check whether browse callback is received */
    EXPECT_EQ(browseNodeFlag, true);
    browseNodeFlag = false;
}

static void startClient(char *addr, int port, char *securityPolicyUri)
{
    PRINT("                       Client connect            ");
    EdgeMessage *msg = createEdgeMessage(endpointUri, 0, CMD_START_CLIENT);
    EXPECT_EQ(NULL!=msg, true);
    msg->endpointInfo->endpointConfig = (EdgeEndpointConfig *) EdgeCalloc(1, sizeof(EdgeEndpointConfig));
    EXPECT_EQ(NULL!=msg->endpointInfo->endpointConfig, true);
    msg->endpointInfo->endpointConfig->requestTimeout = 60000;
    msg->endpointInfo->endpointConfig->serverName = copyString(DEFAULT_SERVER_NAME_VALUE);
    msg->endpointInfo->endpointConfig->bindAddress = copyString(ipAddress);
    msg->endpointInfo->endpointConfig->bindPort = port;
    msg->endpointInfo->endpointUri = copyString(endpointUri);
    msg->endpointInfo->securityPolicyUri = copyString(securityPolicyUri);

    EdgeResult ret = sendRequest(msg);
    EXPECT_EQ(ret.code, STATUS_OK);
    sleep(1);
    destroyEdgeMessage(msg);
}

static void start_server(uint16_t port, char *appUri, EdgeApplicationType appType)
{
    int len = strlen(endpointUri);
    epInfo->endpointUri = (char *) EdgeMalloc(len + 1);

    strncpy(epInfo->endpointUri, endpointUri, len);
    epInfo->endpointUri[len] = '\0';

    configureCallbacks();

    EdgeEndpointConfig *endpointConfig = (EdgeEndpointConfig *) EdgeCalloc(1, sizeof(EdgeEndpointConfig));
    endpointConfig->bindAddress = ipAddress;
    endpointConfig->bindPort = port;
    endpointConfig->serverName = (char *) DEFAULT_SERVER_NAME_VALUE;

    EdgeApplicationConfig *appConfig = (EdgeApplicationConfig *) EdgeCalloc(1, sizeof(EdgeApplicationConfig));
    appConfig->applicationName = (char *) DEFAULT_SERVER_APP_NAME_VALUE;
    appConfig->applicationUri = appUri;
    appConfig->productUri = (char *) DEFAULT_PRODUCT_URI_VALUE;
    appConfig->applicationType = appType;

    EdgeEndPointInfo *ep = (EdgeEndPointInfo *) EdgeCalloc(1, sizeof(EdgeEndPointInfo));
    ep->endpointUri = endpointUri;
    ep->endpointConfig = endpointConfig;
    ep->appConfig = appConfig;

    EdgeMessage *msg = (EdgeMessage *) EdgeMalloc(sizeof(EdgeMessage));
    msg->endpointInfo = ep;
    msg->command = CMD_START_SERVER;
    msg->type = SEND_REQUEST;

    PRINT("--- START SERVER ----");
    createServer(ep);
    //ASSERT_TRUE(startServerFlag == true);

    printf("start server flag assert\n");

    deleteMessage(msg, ep);
    cleanCallbacks();
    free(endpointConfig);
    free(appConfig);
    if (epInfo->endpointUri != NULL)
    {
        free(epInfo->endpointUri);
        epInfo->endpointUri = NULL;
    }
}

static void stop_server(char *endpoint)
{
    configureCallbacks();
    EdgeEndPointInfo *epStop = (EdgeEndPointInfo *) EdgeCalloc(1, sizeof(EdgeEndPointInfo));
    epStop->endpointUri = endpoint;

    closeServer(epStop);

    //ASSERT_TRUE(startServerFlag == false);

    deleteMessage(NULL, epStop);

    cleanCallbacks();
}

static void stop_client()
{
    EdgeMessage *msg = createEdgeMessage(endpointUri, 1, CMD_STOP_CLIENT);
    EXPECT_EQ(NULL!=msg, true);
    disconnectClient(msg->endpointInfo);
    destroyEdgeMessage(msg);
}

static void findServersHelper()
{
    size_t serverUrisSize = 0;
    unsigned char **serverUris = NULL;
    size_t localeIdsSize = 0;
    unsigned char **localeIds = NULL;
    EdgeApplicationConfig *registeredServers = NULL;
    size_t registeredServersSize = 0;
    EdgeResult res = findServers(endpointUri, serverUrisSize, serverUris, localeIdsSize, localeIds, &registeredServersSize, &registeredServers);
    EXPECT_EQ(res.code,STATUS_OK);

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

class OPC_serverTests: public ::testing::Test
{
protected:

    virtual void SetUp()
    {
        strcpy(ipAddress, LOCALHOST);
        snprintf(endpointUri, sizeof(endpointUri), ENDPOINT_URI, ipAddress);

        epInfo = (EdgeEndPointInfo *) EdgeCalloc(1, sizeof(EdgeEndPointInfo));

    }

    virtual void TearDown()
    {
        if (epInfo != NULL)
        {
            free(epInfo);
            epInfo = NULL;
        }
    }
};

class OPC_clientTests: public ::testing::Test
{
protected:

    virtual void SetUp()
    {
        int len = strlen(IPADDRESS);
        strcpy(ipAddress, IPADDRESS);
        ipAddress[len] = '\0';

        strcpy(endpointUri, ipAddress);
        endpointUri[len] = '\0';

        configureCallbacks();
    }

    virtual void TearDown()
    {
        cleanCallbacks();

        startClientFlag = false;
    }

};

//-----------------------------------------------------------------------------
//  Tests
//-----------------------------------------------------------------------------

TEST_F(OPC_serverTests , SetUPServer_P)
{
    EXPECT_EQ(strcmp(ipAddress, LOCALHOST) == 0, true);

    EXPECT_EQ(strcmp(endpointUri, ENDPOINT_URI) == 0, false);

    EXPECT_EQ(strcmp(endpointUri, "opc:tcp://localhost:12686/edge-opc-server") == 0, true);
}

TEST_F(OPC_serverTests , InitializeServer_P)
{
    EXPECT_EQ(NULL == epInfo, false);

    int len = strlen(endpointUri);
    epInfo->endpointUri = (char *) EdgeMalloc(len + 1);
    EXPECT_EQ(NULL == epInfo->endpointUri, false);

    strncpy(epInfo->endpointUri, endpointUri, len);
    epInfo->endpointUri[len] = '\0';

    EXPECT_EQ(strcmp(epInfo->endpointUri, endpointUri) == 0, true);

    if (epInfo->endpointUri != NULL)
    {
        free(epInfo->endpointUri);
        epInfo->endpointUri = NULL;
    }
}

TEST_F(OPC_serverTests , InitializeServer_N)
{
    int len = strlen(endpointUri);
    epInfo->endpointUri = (char *) EdgeMalloc(len + 1);
    EXPECT_EQ(NULL == epInfo->endpointUri, false);

    //strncpy(epInfo->endpointUri, endpointUri, len);
    //epInfo->endpointUri[len] = '\0';

    EXPECT_EQ(strcmp(epInfo->endpointUri, endpointUri) == 0, false);

    if (epInfo->endpointUri != NULL)
    {
        free(epInfo->endpointUri);
        epInfo->endpointUri = NULL;
    }
}

TEST_F(OPC_serverTests , ConfigureServer_P)
{
    int len = strlen(endpointUri);
    epInfo->endpointUri = (char *) EdgeMalloc(len + 1);

    strncpy(epInfo->endpointUri, endpointUri, len);
    epInfo->endpointUri[len] = '\0';

    configureCallbacks();

    cleanCallbacks();

    if (epInfo->endpointUri != NULL)
    {
        free(epInfo->endpointUri);
        epInfo->endpointUri = NULL;
    }
}

TEST_F(OPC_serverTests , StartServer_N)
{
    int len = strlen(endpointUri);
    epInfo->endpointUri = (char *) EdgeMalloc(len + 1);

    strncpy(epInfo->endpointUri, endpointUri, len);
    epInfo->endpointUri[len] = '\0';

    configureCallbacks();

    EdgeEndpointConfig *endpointConfig = (EdgeEndpointConfig *) EdgeCalloc(1, sizeof(EdgeEndpointConfig));
    EXPECT_EQ(NULL != endpointConfig, true);

    endpointConfig->bindAddress = ipAddress;
    EXPECT_EQ(strcmp(endpointConfig->bindAddress, ipAddress) == 0, true);

    endpointConfig->bindPort = 12686;
    EXPECT_EQ(endpointConfig->bindPort, 12686);

    endpointConfig->serverName = (char *) DEFAULT_SERVER_NAME_VALUE;
    EXPECT_EQ(strcmp(endpointConfig->serverName, DEFAULT_SERVER_NAME_VALUE) == 0, true);

    EdgeApplicationConfig *appConfig = (EdgeApplicationConfig *) EdgeCalloc(1, sizeof(EdgeApplicationConfig));
    EXPECT_EQ(NULL != appConfig, true);

    appConfig->applicationName = (char *) DEFAULT_SERVER_APP_NAME_VALUE;
    EXPECT_EQ(strcmp(appConfig->applicationName, DEFAULT_SERVER_APP_NAME_VALUE) == 0, true);

    appConfig->applicationUri = (char *) DEFAULT_SERVER_URI_VALUE;
    EXPECT_EQ(strcmp(appConfig->applicationUri, DEFAULT_SERVER_URI_VALUE) == 0, true);

    appConfig->productUri = (char *) DEFAULT_PRODUCT_URI_VALUE;
    EXPECT_EQ(strcmp(appConfig->productUri, DEFAULT_PRODUCT_URI_VALUE) == 0, true);

    EdgeEndPointInfo *ep = (EdgeEndPointInfo *) EdgeCalloc(1, sizeof(EdgeEndPointInfo));
    EXPECT_EQ(NULL != ep, true);
    ep->endpointUri = endpointUri;
    ASSERT_TRUE(strcmp(ep->endpointUri, "opc:tcp://localhost:12686/edge-opc-server") == 0);
    ep->securityPolicyUri = NULL;
    EXPECT_EQ(ep->securityPolicyUri == NULL, true);
    ep->endpointConfig = endpointConfig;
    ep->appConfig = appConfig;

    EdgeMessage *msg = (EdgeMessage *) EdgeMalloc(sizeof(EdgeMessage));
    EXPECT_EQ(NULL != msg, true);
    msg->endpointInfo = ep;

    msg->command = CMD_START_SERVER;
    EXPECT_EQ(msg->command, CMD_START_SERVER);

    msg->type = SEND_REQUEST;
    EXPECT_EQ(msg->type, SEND_REQUEST);

    EXPECT_EQ(startServerFlag, false);

    PRINT("--- START SERVER ----");

    //createServer(ep);
    //::TODO - This case is faling need to put NULL checks
    createServer (NULL);

    EXPECT_EQ(startServerFlag, false);

    deleteMessage(msg, ep);

    cleanCallbacks();

    free(endpointConfig);
    free(appConfig);

    if (epInfo->endpointUri != NULL)
    {
        free(epInfo->endpointUri);
        epInfo->endpointUri = NULL;
    }
}

TEST_F(OPC_serverTests , StartStopServer_P)
{
    if (startServerFlag == true)
        printf("true\n");
    else
        printf("false\n");
    start_server(12686, (char *)DEFAULT_SERVER_APP_URI_VALUE, EDGE_APPLICATIONTYPE_SERVER);

    ASSERT_TRUE(startServerFlag == true);

    if (startServerFlag == true)
        printf("true\n");
    else
        printf("false\n");

    PRINT("=============STOP SERVER===============");

    stop_server(endpointUri);
    if (startServerFlag == true)
        printf("true\n");
    else
        printf("false\n");
}

TEST_F(OPC_serverTests, FindServers_With_Different_ServerAppURI)
{
    if (startServerFlag == true)
        printf("true\n");
    else
        printf("false\n");

    char endpointUriCopy[512];
    strcpy(endpointUriCopy, endpointUri);
    strcpy(endpointUri, "opc.tcp://localhost:12687/edge-opc-server");
    char *serverAppUri = "opc.tcp://192.168.0.200:1234";
    start_server(12687, serverAppUri, EDGE_APPLICATIONTYPE_SERVER);

    ASSERT_TRUE(startServerFlag == true);

    if (startServerFlag == true)
        printf("true\n");
    else
        printf("false\n");

    // Find Server
    findServersHelper();

    PRINT("=============STOP SERVER===============");

    stop_server(endpointUri);
    if (startServerFlag == true)
        printf("true\n");
    else
        printf("false\n");

    strcpy(endpointUri, endpointUriCopy);
}

TEST_F(OPC_serverTests, FindServers_With_Invalid_ServerAppURI)
{
    if (startServerFlag == true)
        printf("true\n");
    else
        printf("false\n");

    char endpointUriCopy[512];
    strcpy(endpointUriCopy, endpointUri);
    strcpy(endpointUri, "opc.tcp://localhost:12687/edge-opc-server");
    char *serverAppUri = "opc.tcp://192.168.0.300:1234";
    start_server(12687, serverAppUri, EDGE_APPLICATIONTYPE_SERVER);

    ASSERT_TRUE(startServerFlag == true);

    if (startServerFlag == true)
        printf("true\n");
    else
        printf("false\n");

    // Find Server
    findServersHelper();

    PRINT("=============STOP SERVER===============");

    stop_server(endpointUri);
    if (startServerFlag == true)
        printf("true\n");
    else
        printf("false\n");

    strcpy(endpointUri, endpointUriCopy);
}

TEST_F(OPC_serverTests, ClientAppType_And_FindServers)
{
    if (startServerFlag == true)
        printf("true\n");
    else
        printf("false\n");

    char endpointUriCopy[512];
    strcpy(endpointUriCopy, endpointUri);
    strcpy(endpointUri, "opc.tcp://localhost:12687/edge-opc-server");
    char *serverAppUri = "opc.tcp://192.168.0.200:1234";
    start_server(12687, serverAppUri, EDGE_APPLICATIONTYPE_CLIENT);

    ASSERT_TRUE(startServerFlag == true);

    if (startServerFlag == true)
        printf("true\n");
    else
        printf("false\n");

    // Find Server
    findServersHelper();

    PRINT("=============STOP SERVER===============");

    stop_server(endpointUri);
    if (startServerFlag == true)
        printf("true\n");
    else
        printf("false\n");

    strcpy(endpointUri, endpointUriCopy);
}

TEST_F(OPC_serverTests, ClientAndServerAppType_And_FindServers)
{
    if (startServerFlag == true)
        printf("true\n");
    else
        printf("false\n");

    char endpointUriCopy[512];
    strcpy(endpointUriCopy, endpointUri);
    strcpy(endpointUri, "opc.tcp://localhost:12687/edge-opc-server");
    char *serverAppUri = "opc.tcp://192.168.0.200:1234";
    start_server(12687, serverAppUri, EDGE_APPLICATIONTYPE_CLIENTANDSERVER);

    ASSERT_TRUE(startServerFlag == true);

    if (startServerFlag == true)
        printf("true\n");
    else
        printf("false\n");

    // Find Server
    findServersHelper();

    PRINT("=============STOP SERVER===============");

    stop_server(endpointUri);
    if (startServerFlag == true)
        printf("true\n");
    else
        printf("false\n");

    strcpy(endpointUri, endpointUriCopy);
}

TEST_F(OPC_serverTests, DiscoverServerAppType_And_FindServers)
{
    if (startServerFlag == true)
        printf("true\n");
    else
        printf("false\n");

    char endpointUriCopy[512];
    strcpy(endpointUriCopy, endpointUri);
    strcpy(endpointUri, "opc.tcp://localhost:12687/edge-opc-server");
    char *serverAppUri = "opc.tcp://192.168.0.200:1234";
    start_server(12687, serverAppUri, EDGE_APPLICATIONTYPE_DISCOVERYSERVER);

    ASSERT_TRUE(startServerFlag == true);

    if (startServerFlag == true)
        printf("true\n");
    else
        printf("false\n");

    // Find Server
    findServersHelper();

    PRINT("=============STOP SERVER===============");

    stop_server(endpointUri);
    if (startServerFlag == true)
        printf("true\n");
    else
        printf("false\n");

    strcpy(endpointUri, endpointUriCopy);
}

TEST_F(OPC_serverTests , StartServer_N1)
{
    int len = strlen(endpointUri);
    epInfo->endpointUri = (char *) EdgeMalloc(len + 1);

    strncpy(epInfo->endpointUri, endpointUri, len);
    epInfo->endpointUri[len] = '\0';
    configureCallbacks();

    EdgeApplicationConfig *appConfig = (EdgeApplicationConfig *) EdgeCalloc(1, sizeof(EdgeApplicationConfig));
    appConfig->applicationName = (char *) DEFAULT_SERVER_APP_NAME_VALUE;
    appConfig->applicationUri = (char *) DEFAULT_SERVER_APP_URI_VALUE;
    appConfig->productUri = (char *) DEFAULT_PRODUCT_URI_VALUE;

    EdgeEndPointInfo *ep = (EdgeEndPointInfo *) EdgeCalloc(1, sizeof(EdgeEndPointInfo));
    ep->endpointUri = endpointUri;
    ep->endpointConfig = NULL;
    ep->appConfig = appConfig;

    EdgeMessage *msg = (EdgeMessage *) EdgeMalloc(sizeof(EdgeMessage));
    msg->endpointInfo = ep;
    msg->command = CMD_START_SERVER;
    msg->type = SEND_REQUEST;

    EdgeResult result = createServer(ep);
    ASSERT_EQ(result.code, STATUS_PARAM_INVALID);

    cleanCallbacks();

    free(appConfig);
    if (epInfo->endpointUri != NULL)
    {
        free(epInfo->endpointUri);
        epInfo->endpointUri = NULL;
    }
}

TEST_F(OPC_serverTests , StartServer_N2)
{
    int len = strlen(endpointUri);
    epInfo->endpointUri = (char *) EdgeMalloc(len + 1);

    strncpy(epInfo->endpointUri, endpointUri, len);
    epInfo->endpointUri[len] = '\0';
    configureCallbacks();

    EdgeEndpointConfig *endpointConfig = (EdgeEndpointConfig *) EdgeCalloc(1, sizeof(EdgeEndpointConfig));
    endpointConfig->bindAddress = ipAddress;
    endpointConfig->bindPort = 12686;
    endpointConfig->serverName = (char *) DEFAULT_SERVER_NAME_VALUE;

    EdgeEndPointInfo *ep = (EdgeEndPointInfo *) EdgeCalloc(1, sizeof(EdgeEndPointInfo));
    ep->endpointUri = endpointUri;
    ep->endpointConfig = endpointConfig;
    ep->appConfig = NULL;

    EdgeMessage *msg = (EdgeMessage *) EdgeMalloc(sizeof(EdgeMessage));
    msg->endpointInfo = ep;
    msg->command = CMD_START_SERVER;
    msg->type = SEND_REQUEST;

    EdgeResult result = createServer(ep);
    ASSERT_EQ(result.code, STATUS_PARAM_INVALID);

    cleanCallbacks();

    free(endpointConfig);
    if (epInfo->endpointUri != NULL)
    {
        free(epInfo->endpointUri);
        epInfo->endpointUri = NULL;
    }
}

TEST_F(OPC_serverTests , StartServer_N3)
{
    EdgeResult result = createServer(NULL);
    ASSERT_EQ(result.code, STATUS_PARAM_INVALID);
}

TEST_F(OPC_serverTests , StartServer_P)
{
    int len = strlen(endpointUri);
    epInfo->endpointUri = (char *) EdgeMalloc(len + 1);

    strncpy(epInfo->endpointUri, endpointUri, len);
    epInfo->endpointUri[len] = '\0';
    configureCallbacks();

    EdgeEndpointConfig *endpointConfig = (EdgeEndpointConfig *) EdgeCalloc(1, sizeof(EdgeEndpointConfig));
    endpointConfig->bindAddress = ipAddress;
    endpointConfig->bindPort = 12686;
    endpointConfig->serverName = (char *) DEFAULT_SERVER_NAME_VALUE;

    EdgeApplicationConfig *appConfig = (EdgeApplicationConfig *) EdgeCalloc(1, sizeof(EdgeApplicationConfig));
    appConfig->applicationName = (char *) DEFAULT_SERVER_APP_NAME_VALUE;
    appConfig->applicationUri = (char *) DEFAULT_SERVER_APP_URI_VALUE;
    appConfig->productUri = (char *) DEFAULT_PRODUCT_URI_VALUE;

    EdgeEndPointInfo *ep = (EdgeEndPointInfo *) EdgeCalloc(1, sizeof(EdgeEndPointInfo));
    ep->endpointUri = endpointUri;
    ep->endpointConfig = endpointConfig;
    ep->appConfig = appConfig;

    EdgeMessage *msg = (EdgeMessage *) EdgeMalloc(sizeof(EdgeMessage));
    msg->endpointInfo = ep;
    msg->command = CMD_START_SERVER;
    msg->type = SEND_REQUEST;

    EXPECT_EQ(startServerFlag, false);

    PRINT("--- START SERVER ----");

    createServer(ep);

    EXPECT_EQ(startServerFlag, true);

    deleteMessage(msg, ep);

    cleanCallbacks();

    free(endpointConfig);
    free(appConfig);

    if (epInfo->endpointUri != NULL)
    {
        free(epInfo->endpointUri);
        epInfo->endpointUri = NULL;
    }
}

TEST_F(OPC_serverTests , ServerCreateNamespace_P)
{
    EXPECT_EQ(startServerFlag, true);

    EdgeResult result = createNamespace(DEFAULT_NAMESPACE_VALUE,
    DEFAULT_ROOT_NODE_INFO_VALUE,
    DEFAULT_ROOT_NODE_INFO_VALUE,
    DEFAULT_ROOT_NODE_INFO_VALUE);

    EXPECT_EQ(result.code, STATUS_OK);
}

TEST_F(OPC_serverTests , ServerCreateNamespace_N1)
{
    EXPECT_EQ(startServerFlag, true);

    EdgeResult result = createNamespace(DEFAULT_NAMESPACE_VALUE,
    DEFAULT_ROOT_NODE_INFO_VALUE,
    DEFAULT_ROOT_NODE_INFO_VALUE,
    DEFAULT_ROOT_NODE_INFO_VALUE);

    EXPECT_EQ(result.code, STATUS_PARAM_INVALID);
}

TEST_F(OPC_serverTests , ServerCreateNamespace_N2)
{
    EdgeResult result = createNamespace(NULL,
    DEFAULT_ROOT_NODE_INFO_VALUE,
    DEFAULT_ROOT_NODE_INFO_VALUE,
    DEFAULT_ROOT_NODE_INFO_VALUE);
    ASSERT_EQ(result.code, STATUS_PARAM_INVALID);
}

TEST_F(OPC_serverTests , ServerCreateNamespace_N3)
{
    EdgeResult result = createNamespace(DEFAULT_NAMESPACE_VALUE,
    NULL,
    DEFAULT_ROOT_NODE_INFO_VALUE,
    DEFAULT_ROOT_NODE_INFO_VALUE);
    ASSERT_EQ(result.code, STATUS_PARAM_INVALID);

    result = createNamespace(DEFAULT_NAMESPACE_VALUE,
    DEFAULT_ROOT_NODE_INFO_VALUE,
    NULL,
    DEFAULT_ROOT_NODE_INFO_VALUE);
    ASSERT_EQ(result.code, STATUS_PARAM_INVALID);

    result = createNamespace(DEFAULT_NAMESPACE_VALUE,
    DEFAULT_ROOT_NODE_INFO_VALUE,
    DEFAULT_ROOT_NODE_INFO_VALUE,
    NULL);
    ASSERT_EQ(result.code, STATUS_PARAM_INVALID);
}

TEST_F(OPC_serverTests , ServerCreateNamespace_N4)
{
    EdgeResult result = createNamespace(DEFAULT_NAMESPACE_VALUE,
    NULL,
    NULL,
    NULL);
    ASSERT_EQ(result.code, STATUS_PARAM_INVALID);
}

TEST_F(OPC_serverTests , ServerCreateVariableNodeItem_P)
{
    EdgeNodeItem* item = NULL;
    item = createVariableNodeItem("String1", EDGE_NODEID_STRING, (void*) "test1", VARIABLE_NODE, 100);
    ASSERT_EQ(NULL != item, true);
    EdgeFree(item);
}

TEST_F(OPC_serverTests , ServerCreateVariableNodeItem_N)
{
    EdgeNodeItem* item = NULL;
    item = createVariableNodeItem(NULL, EDGE_NODEID_STRING, (void*) "test1", VARIABLE_NODE, 100);
    ASSERT_EQ(NULL != item, false);
}

TEST_F(OPC_serverTests , ServerCreateNodeItem_P)
{
    EdgeNodeId *edgeNodeId = (EdgeNodeId *) EdgeMalloc(sizeof(EdgeNodeId));
    ASSERT_EQ(NULL != edgeNodeId, true);

    EdgeNodeItem* item = NULL;
    item = createNodeItem("Object1", OBJECT_NODE, edgeNodeId);
    ASSERT_EQ(NULL != item, true);
    EdgeFree(item);

    item = NULL;
    item = createNodeItem("ObjectType1", OBJECT_TYPE_NODE, edgeNodeId);
    ASSERT_EQ(NULL != item, true);
    EdgeFree(item);

    item = NULL;
    item = createNodeItem("DataType1", DATA_TYPE_NODE, edgeNodeId);
    ASSERT_EQ(NULL != item, true);
    EdgeFree(item);

    item = NULL;
    item = createNodeItem("View1", VIEW_NODE, edgeNodeId);
    ASSERT_EQ(NULL != item, true);
    EdgeFree(item);
}

TEST_F(OPC_serverTests , ServerCreateNodeItem_N)
{
    EdgeNodeId *edgeNodeId = (EdgeNodeId *) EdgeMalloc(sizeof(EdgeNodeId));
    ASSERT_EQ(NULL != edgeNodeId, true);

    EdgeNodeItem* item = NULL;
    item = createNodeItem(NULL, OBJECT_NODE, edgeNodeId);
    ASSERT_EQ(NULL != item, false);

    item = createNodeItem(NULL, OBJECT_TYPE_NODE, edgeNodeId);
    ASSERT_EQ(NULL != item, false);

    item = createNodeItem(NULL, DATA_TYPE_NODE, edgeNodeId);
    ASSERT_EQ(NULL != item, false);

    item = createNodeItem(NULL, VIEW_NODE, edgeNodeId);
    ASSERT_EQ(NULL != item, false);
}

TEST_F(OPC_serverTests , ServerAddReference_N1)
{
    EdgeReference *reference = (EdgeReference *) EdgeCalloc(1, sizeof(EdgeReference));
    ASSERT_EQ(NULL != reference, true);
    reference->forward = true;
    reference->sourceNamespace = NULL;
    reference->sourcePath = "ViewNode1";
    reference->targetNamespace = copyString(DEFAULT_NAMESPACE_VALUE);
    reference->targetPath = "robot_name";
    /* default reference ID : Organizes */
    EdgeResult result = addReference(reference);
    ASSERT_EQ(result.code, STATUS_PARAM_INVALID);
    EdgeFree(reference);
}

TEST_F(OPC_serverTests , ServerAddReference_N2)
{
    EdgeReference *reference = (EdgeReference *) EdgeCalloc(1, sizeof(EdgeReference));
    ASSERT_EQ(NULL != reference, true);
    reference->forward = true;
    reference->sourceNamespace = copyString(DEFAULT_NAMESPACE_VALUE);
    reference->sourcePath = "ViewNode1";
    reference->targetNamespace = NULL;
    reference->targetPath = "robot_name";
    /* default reference ID : Organizes */
    EdgeResult result = addReference(reference);
    ASSERT_EQ(result.code, STATUS_PARAM_INVALID);
    EdgeFree(reference);
}

TEST_F(OPC_serverTests , ServerAddReference_N3)
{
    EdgeReference *reference = (EdgeReference *) EdgeCalloc(1, sizeof(EdgeReference));
    ASSERT_EQ(NULL != reference, true);
    reference->forward = true;
    reference->sourceNamespace = NULL;
    reference->sourcePath = "ViewNode1";
    reference->targetNamespace = NULL;
    reference->targetPath = "robot_name";
    /* default reference ID : Organizes */
    EdgeResult result = addReference(reference);
    ASSERT_EQ(result.code, STATUS_PARAM_INVALID);
    EdgeFree(reference);
}

TEST_F(OPC_serverTests , ServerAddReference_N4)
{
    EdgeResult result = addReference(NULL);
    ASSERT_EQ(result.code, STATUS_PARAM_INVALID);
}

TEST_F(OPC_serverTests , ServerAddMethodNode_N1)
{
    EdgeNodeItem *methodNodeItem = (EdgeNodeItem *) EdgeMalloc(sizeof(EdgeNodeItem));
    ASSERT_EQ(NULL != methodNodeItem, true);
    EdgeMethod *method = (EdgeMethod *) EdgeMalloc(sizeof(EdgeMethod));
    ASSERT_EQ(NULL != method, true);
    EdgeResult result = createMethodNode(NULL, methodNodeItem, method);
    ASSERT_EQ(result.code, STATUS_PARAM_INVALID);
    EdgeFree(methodNodeItem);
    EdgeFree(method);
}

TEST_F(OPC_serverTests , ServerAddMethodNode_N2)
{
    EdgeMethod *method = (EdgeMethod *) EdgeMalloc(sizeof(EdgeMethod));
    ASSERT_EQ(NULL != method, true);
    EdgeResult result = createMethodNode(DEFAULT_NAMESPACE_VALUE, NULL, method);
    ASSERT_EQ(result.code, STATUS_PARAM_INVALID);
    EdgeFree(method);
}

TEST_F(OPC_serverTests , ServerAddMethodNode_N3)
{
    EdgeNodeItem *methodNodeItem = (EdgeNodeItem *) EdgeMalloc(sizeof(EdgeNodeItem));
    ASSERT_EQ(NULL != methodNodeItem, true);
    EdgeResult result = createMethodNode(DEFAULT_NAMESPACE_VALUE, methodNodeItem, NULL);
    ASSERT_EQ(result.code, STATUS_PARAM_INVALID);
    EdgeFree(methodNodeItem);
}

TEST_F(OPC_serverTests , ServerAddMethodNode_N4)
{
    EdgeResult result = createMethodNode(NULL, NULL, NULL);
    ASSERT_EQ(result.code, STATUS_PARAM_INVALID);
}

TEST_F(OPC_serverTests , ServerCreateNode_N1)
{
    EdgeResult result = createNode(DEFAULT_NAMESPACE_VALUE, NULL);
    ASSERT_EQ(result.code, STATUS_PARAM_INVALID);
}

TEST_F(OPC_serverTests , ServerCreateNode_N2)
{
    EdgeResult result = createNode(DEFAULT_NAMESPACE_VALUE, NULL);
    ASSERT_EQ(result.code, STATUS_PARAM_INVALID);
}

TEST_F(OPC_serverTests , ServerAddNodes_P)
{
    start_server(12686, (char *) DEFAULT_SERVER_APP_URI_VALUE, EDGE_APPLICATIONTYPE_SERVER);

    EXPECT_EQ(startServerFlag, true);

    int index = 0;
    EdgeNodeItem* item = NULL;
    item = createVariableNodeItem("String1", EDGE_NODEID_STRING, (void*) "test1", VARIABLE_NODE, 100);
    VERIFY_NON_NULL_NR(item);
    createNode(DEFAULT_NAMESPACE_VALUE, item);
    printf("\n|------------[Added]  %s\n", item->browseName);
    deleteNodeItem(item);

    item = createVariableNodeItem("String2", EDGE_NODEID_STRING, (void*) "test2", VARIABLE_NODE, 100);
    VERIFY_NON_NULL_NR(item);
    createNode(DEFAULT_NAMESPACE_VALUE, item);
    printf("\n|------------[Added] %s\n", item->browseName);
    deleteNodeItem(item);

    item = createVariableNodeItem("String3", EDGE_NODEID_STRING, (void*) "test3", VARIABLE_NODE, 100);
    VERIFY_NON_NULL_NR(item);
    createNode(DEFAULT_NAMESPACE_VALUE, item);
    printf("\n|------------[Added] %s\n", item->browseName);
    deleteNodeItem(item);

    printf("\n[%d] Variable node with XML ELEMENT variant: \n", ++index);
    Edge_XmlElement *xml_value = (Edge_XmlElement *) EdgeMalloc(sizeof(Edge_XmlElement));
    if (IS_NOT_NULL(xml_value))
    {
        xml_value->length = 2;
        xml_value->data = (UA_Byte *) "ab";
        item = createVariableNodeItem("XmlElement", EDGE_NODEID_XMLELEMENT, (void *) xml_value, VARIABLE_NODE, 100);
        VERIFY_NON_NULL_NR(item);
        createNode(DEFAULT_NAMESPACE_VALUE, item);
        printf("\n|------------[Added] %s\n", item->browseName);
        EdgeFree(xml_value);
        deleteNodeItem(item);
    }
    else
    {
        printf("Error :: EdgeMalloc failed for Edge_XmlElement in Test create Nodes\n");
    }

    printf("\n[%d] Variable node with localized text variant: \n", ++index);
    Edge_LocalizedText *lt_value = (Edge_LocalizedText *) EdgeMalloc(sizeof(Edge_LocalizedText));
    if (IS_NOT_NULL(lt_value))
    {
        lt_value->locale = EdgeStringAlloc("COUNTRY");
        lt_value->text = EdgeStringAlloc("INDIA");
        item = createVariableNodeItem("LocalizedText", EDGE_NODEID_LOCALIZEDTEXT, (void *) lt_value,
                VARIABLE_NODE, 100);
        VERIFY_NON_NULL_NR(item);
        createNode(DEFAULT_NAMESPACE_VALUE, item);
        printf("\n|------------[Added] %s\n", item->browseName);
        EdgeFree(lt_value->locale.data);
        EdgeFree(lt_value->text.data);
        EdgeFree(lt_value);
        deleteNodeItem(item);
    }
    else
    {
        printf("Error :: EdgeMalloc failed for Edge_LocalizedText in Test create Nodes\n");
    }


    printf("\n[%d] Variable node with byte string variant: \n", ++index);
    char *bs_value = (char *) EdgeMalloc(sizeof(char) * 10);
    if (IS_NOT_NULL(bs_value))
    {
        strncpy(bs_value, "samsung", strlen("samsung"));
        bs_value[strlen("samsung")] = '\0';
        item = createVariableNodeItem("ByteString", EDGE_NODEID_BYTESTRING, (void *) bs_value, VARIABLE_NODE, 100);
        VERIFY_NON_NULL_NR(item);
        createNode(DEFAULT_NAMESPACE_VALUE, item);
        printf("\n|------------[Added] %s\n", item->browseName);
        EdgeFree(bs_value);
        deleteNodeItem(item);
    }
    else
    {
        printf("Error :: EdgeMalloc failed for UA_ByteString in Test create Nodes\n");
    }

    printf("\n[%d] Variable node with byte variant: \n", ++index);
    Edge_Byte b_value = 2;
    item = createVariableNodeItem("Byte", EDGE_NODEID_BYTE, (void *) &b_value, VARIABLE_NODE, 100);
    VERIFY_NON_NULL_NR(item);
    createNode(DEFAULT_NAMESPACE_VALUE, item);
    printf("\n|------------[Added] %s\n", item->browseName);
    deleteNodeItem(item);

    printf("\n[%d] Variable node with float variant: \n", ++index);
    float f_value = 4.4;
    item = createVariableNodeItem("Float", EDGE_NODEID_FLOAT, (void *) &f_value, VARIABLE_NODE, 100);
    VERIFY_NON_NULL_NR(item);
    createNode(DEFAULT_NAMESPACE_VALUE, item);
    printf("\n|------------[Added] %s\n", item->browseName);
    deleteNodeItem(item);

    printf("\n[%d] Variable node with int variant: \n", ++index);
    int value = 30;
    item = createVariableNodeItem("UInt16", EDGE_NODEID_UINT16, (void *) &value, VARIABLE_NODE, 100);
    VERIFY_NON_NULL_NR(item);
    createNode(DEFAULT_NAMESPACE_VALUE, item);
    printf("\n|------------[Added] %s\n", item->browseName);
    deleteNodeItem(item);

    printf("\n[%d] Variable node with UInt32 variant: \n", ++index);
    value = 444;
    item = createVariableNodeItem("UInt32", EDGE_NODEID_UINT32, (void *) &value, VARIABLE_NODE, 100);
    VERIFY_NON_NULL_NR(item);
    createNode(DEFAULT_NAMESPACE_VALUE, item);
    printf("\n|------------[Added] %s\n", item->browseName);
    deleteNodeItem(item);

    printf("\n[%d] Variable node with UInt64 variant: \n", ++index);
    value = 3445516;
    item = createVariableNodeItem("UInt64", EDGE_NODEID_UINT64, (void *) &value, VARIABLE_NODE, 100);
    VERIFY_NON_NULL_NR(item);
    createNode(DEFAULT_NAMESPACE_VALUE, item);
    printf("\n|------------[Added] %s\n", item->browseName);
    deleteNodeItem(item);

    printf("\n[%d] Variable node with Int16 variant: \n", ++index);
    value = 4;
    item = createVariableNodeItem("Int16", EDGE_NODEID_INT16, (void *) &value, VARIABLE_NODE, 100);
    VERIFY_NON_NULL_NR(item);
    createNode(DEFAULT_NAMESPACE_VALUE, item);
    printf("\n|------------[Added] %s\n", item->browseName);
    deleteNodeItem(item);

    printf("\n[%d] Variable node with Int32 variant: \n", ++index);
    value = 40;
    item = createVariableNodeItem("Int32", EDGE_NODEID_INT32, (void *) &value, VARIABLE_NODE, 100);
    VERIFY_NON_NULL_NR(item);
    createNode(DEFAULT_NAMESPACE_VALUE, item);
    printf("\n|------------[Added] %s\n", item->browseName);
    deleteNodeItem(item);

    item = (EdgeNodeItem *) EdgeCalloc(1, sizeof(EdgeNodeItem));
    VERIFY_NON_NULL_NR(item);
    printf("\n[%d] Variable node with Int64 variant: \n", ++index);
    value = 32700;
    item = createVariableNodeItem("Int64", EDGE_NODEID_INT64, (void *) &value, VARIABLE_NODE, 100);
    VERIFY_NON_NULL_NR(item);
    createNode(DEFAULT_NAMESPACE_VALUE, item);
    printf("\n|------------[Added] %s\n", item->browseName);
    deleteNodeItem(item);

    printf("\n[%d] Variable node with UInt32 variant: \n", ++index);
    uint32_t int32_val = 4456;
    item = createVariableNodeItem("UInt32writeonly", EDGE_NODEID_UINT32, (void *) &int32_val, VARIABLE_NODE, 100);
    VERIFY_NON_NULL_NR(item);
    item->accessLevel = WRITE;
    item->userAccessLevel = WRITE;
    createNode(DEFAULT_NAMESPACE_VALUE, item);
    printf("\n|------------[Added] %s\n", item->browseName);
    deleteNodeItem(item);

    printf("\n[%d] Variable node with UInt64 variant: \n", ++index);
    int64_t int64_val = 3270000;
    item = createVariableNodeItem("UInt64readonly", EDGE_NODEID_UINT64, (void *) &int64_val, VARIABLE_NODE, 100);
    VERIFY_NON_NULL_NR(item);
    item->userAccessLevel = READ;
    item->accessLevel = READ;
    createNode(DEFAULT_NAMESPACE_VALUE, item);
    printf("\n|------------[Added] %s\n", item->browseName);
    deleteNodeItem(item);

    printf("\n[%d] Variable node with double variant: \n", ++index);
    double d_val = 50.4;
    item = createVariableNodeItem("Double", EDGE_NODEID_DOUBLE, (void *) &d_val, VARIABLE_NODE, 100);
    VERIFY_NON_NULL_NR(item);
    createNode(DEFAULT_NAMESPACE_VALUE, item);
    printf("\n|------------[Added] %s\n", item->browseName);
    deleteNodeItem(item);

    printf("\n[%d] Variable node with boolean variant: \n", ++index);
    bool flag = true;
    item = createVariableNodeItem("Boolean", EDGE_NODEID_BOOLEAN, (void *) &flag, VARIABLE_NODE, 100);
    VERIFY_NON_NULL_NR(item);
    createNode(DEFAULT_NAMESPACE_VALUE, item);
    printf("\n|------------[Added] %s\n", item->browseName);
    deleteNodeItem(item);

    printf("\n[%d] Variable node with dateTime variant: \n", ++index);
    UA_DateTime time = UA_DateTime_now();
    item = createVariableNodeItem("DateTime", EDGE_NODEID_DATETIME, (void *) &time, VARIABLE_NODE, 100);
    VERIFY_NON_NULL_NR(item);
    createNode(DEFAULT_NAMESPACE_VALUE, item);
    printf("\n|------------[Added] %s\n", item->browseName);
    deleteNodeItem(item);

    printf("\n[%d] Variable node with SByte variant: \n", ++index);
    Edge_SByte sbyte = 2;
    item = createVariableNodeItem("SByte", EDGE_NODEID_SBYTE, (void *) &sbyte, VARIABLE_NODE, 100);
    VERIFY_NON_NULL_NR(item);
    createNode(DEFAULT_NAMESPACE_VALUE, item);
    printf("\n|------------[Added] %s\n", item->browseName);
    deleteNodeItem(item);

    printf("\n[%d] Variable node with GUID variant: \n", ++index);
    Edge_Guid guid ={ 1, 0, 1, { 0, 0, 0, 0, 1, 1, 1, 1 } };
    item = createVariableNodeItem("Guid", EDGE_NODEID_GUID, (void *) &guid, VARIABLE_NODE, 100);
    VERIFY_NON_NULL_NR(item);
    createNode(DEFAULT_NAMESPACE_VALUE, item);
    printf("\n|------------[Added] %s\n", item->browseName);
    deleteNodeItem(item);

    printf("\n[%d] Variable node with qualified name variant: \n", ++index);
    Edge_QualifiedName *qn_value = (Edge_QualifiedName *) EdgeMalloc(sizeof(Edge_QualifiedName));
    if (IS_NOT_NULL(qn_value))
    {
        Edge_String str = EdgeStringAlloc("qualifiedName");
        qn_value->namespaceIndex = 2;
        qn_value->name = str;
        item = createVariableNodeItem("QualifiedName", EDGE_NODEID_QUALIFIEDNAME, (void *) qn_value,
                VARIABLE_NODE, 100);
        VERIFY_NON_NULL_NR(item);
        createNode(DEFAULT_NAMESPACE_VALUE, item);
        printf("\n|------------[Added] %s\n", item->browseName);
        EdgeFree(str.data);
        EdgeFree(qn_value);
        deleteNodeItem(item);
    }
    else
    {
        printf("Error :: EdgeMalloc failed for UA_QualifiedName in Test create Nodes\n");
    }

    printf("\n[%d] Variable node with NODEID (Numeric) variant: \n", ++index);
    Edge_NodeId *node =  (Edge_NodeId *) EdgeMalloc(sizeof(Edge_NodeId));
    node->namespaceIndex = DEFAULT_NAMESPACE_INDEX;
    node->identifierType = EDGE_INTEGER;
    node->identifier.numeric = EDGE_NODEID_ROOTFOLDER;

    item = createVariableNodeItem("NodeId1", EDGE_NODEID_NODEID, node, VARIABLE_NODE, 100);
    VERIFY_NON_NULL_NR(item);
    createNode(DEFAULT_NAMESPACE_VALUE, item);
    printf("\n|------------[Added] %s\n", item->browseName);
    deleteNodeItem(item);
    EdgeFree(node);

    printf("\n[%d] Variable node with NODEID (String) variant: \n", ++index);
    node =  (Edge_NodeId *) EdgeMalloc(sizeof(Edge_NodeId));
    node->namespaceIndex = DEFAULT_NAMESPACE_INDEX;
    node->identifierType = EDGE_STRING;
    node->identifier.string = EdgeStringAlloc("StringNodeId");

    item = createVariableNodeItem("NodeId2", EDGE_NODEID_NODEID, node, VARIABLE_NODE, 100);
    VERIFY_NON_NULL_NR(item);
    createNode(DEFAULT_NAMESPACE_VALUE, item);
    printf("\n|------------[Added] %s\n", item->browseName);
    deleteNodeItem(item);
    EdgeFree(node->identifier.string.data);
    EdgeFree(node);

    printf("\n[%d] Variable node with NODEID (ByteString) variant: \n", ++index);
    node =  (Edge_NodeId *) EdgeMalloc(sizeof(Edge_NodeId));
    node->namespaceIndex = DEFAULT_NAMESPACE_INDEX;
    node->identifierType = EDGE_BYTESTRING;
    node->identifier.byteString = EdgeStringAlloc("ByteStringNodeId");

    item = createVariableNodeItem("NodeId3", EDGE_NODEID_NODEID, node, VARIABLE_NODE, 100);
    VERIFY_NON_NULL_NR(item);
    createNode(DEFAULT_NAMESPACE_VALUE, item);
    printf("\n|------------[Added] %s\n", item->browseName);
    deleteNodeItem(item);
    EdgeFree(node->identifier.byteString.data);
    EdgeFree(node);

    printf("\n[%d] Variable node with NODEID (Guid) variant: \n", ++index);
    node =  (Edge_NodeId *) EdgeMalloc(sizeof(Edge_NodeId));
    node->namespaceIndex = DEFAULT_NAMESPACE_INDEX;
    node->identifierType = EDGE_UUID;
    node->identifier.guid = guid;

    item = createVariableNodeItem("NodeId4", EDGE_NODEID_NODEID, node, VARIABLE_NODE, 100);
    VERIFY_NON_NULL_NR(item);
    createNode(DEFAULT_NAMESPACE_VALUE, item);
    printf("\n|------------[Added] %s\n", item->browseName);
    deleteNodeItem(item);
    EdgeFree(node);

    /******************* Array *********************/
    printf("\n[Create Array Node]\n");
    printf("\n[%d] Array node with ByteString values: \n", ++index);
    char **dataArray = (char **) malloc(sizeof(char *) * 5);
    if (IS_NOT_NULL(dataArray))
    {
        dataArray[0] = (char *) EdgeMalloc(sizeof(char) * 10);
        dataArray[1] = (char *) EdgeMalloc(sizeof(char) * 10);
        dataArray[2] = (char *) EdgeMalloc(sizeof(char) * 10);
        dataArray[3] = (char *) EdgeMalloc(sizeof(char) * 10);
        dataArray[4] = (char *) EdgeMalloc(sizeof(char) * 10);
        if (IS_NOT_NULL(dataArray[0]) && IS_NOT_NULL(dataArray[1]) && IS_NOT_NULL(dataArray[2])
        && IS_NOT_NULL(dataArray[3]) && IS_NOT_NULL(dataArray[4]))
        {
            strncpy(dataArray[0],"abcde", strlen("abcde"));
            dataArray[0][strlen("abcde")] = '\0';
            strncpy(dataArray[1], "fghij", strlen("fghij"));
            dataArray[1][strlen("fghij")] = '\0';
            strncpy(dataArray[2], "klmno", strlen("klmno"));
            dataArray[2][strlen("klmno")] = '\0';
            strncpy(dataArray[3], "pqrst", strlen("pqrst"));
            dataArray[3][strlen("pqrst")] = '\0';
            strncpy(dataArray[4], "uvwxyz", strlen("uvwxyz"));
            dataArray[4][strlen("uvwxyz")] = '\0';
            item = createVariableNodeItem("ByteStringArray", EDGE_NODEID_BYTESTRING, (void *) dataArray,
                    VARIABLE_NODE, 100);
            VERIFY_NON_NULL_NR(item);
            item->nodeType = ARRAY_NODE;
            item->arrayLength = 5;
            createNode(DEFAULT_NAMESPACE_VALUE, item);
            printf("\n|------------[Added] %s\n", item->browseName);
        }
        else
        {
            printf(
                    "Error :: EdgeMalloc failed for UA_ByteString dataArray INDEX in Test create Nodes\n");
        }

        for (int i = 0; i < 5; i++)
        {
            EdgeFree(dataArray[i]);
        }
        EdgeFree(dataArray);
        deleteNodeItem(item);
    }
    else
    {
        printf("Error :: EdgeMalloc failed for UA_ByteString dataArray in Test create Nodes\n");
    }

    printf("\n[%d] Array node with Boolean values: \n", ++index);
    bool *arr = (bool *) EdgeMalloc(sizeof(bool) * 5);
    if (IS_NOT_NULL(arr))
    {
        arr[0] = true;
        arr[1] = false;
        arr[2] = true;
        arr[3] = false;
        arr[4] = true;
        item = createVariableNodeItem("BoolArray", EDGE_NODEID_BOOLEAN, (void *) arr, VARIABLE_NODE, 100);
        VERIFY_NON_NULL_NR(item);
        item->nodeType = ARRAY_NODE;
        item->arrayLength = 5;
        createNode(DEFAULT_NAMESPACE_VALUE, item);
        printf("\n|------------[Added] %s\n", item->browseName);
        EdgeFree(arr);
        deleteNodeItem(item);
    }
    else
    {
        printf("Error :: EdgeMalloc failed for bool array in Test create Nodes\n");
    }

    printf("\n[%d] Array node with dateTime variant: \n", ++index);
    UA_DateTime time_now = UA_DateTime_now();
    UA_DateTime timeArr[2] = {time_now, time_now-1000};
    item = createVariableNodeItem("DateTimeArray", EDGE_NODEID_DATETIME, (void *) timeArr, VARIABLE_NODE, 100);
    VERIFY_NON_NULL_NR(item);
    item->nodeType = ARRAY_NODE;
    item->arrayLength = 2;
    createNode(DEFAULT_NAMESPACE_VALUE, item);
    printf("\n|------------[Added] %s\n", item->browseName);
    deleteNodeItem(item);

    printf("\n[%d] Array node with XML ELEMENT variant: \n", ++index);
    Edge_XmlElement xmlValueArr[2] = { {2, (uint8_t *)"ab"}, {3, (uint8_t *)"abc"} };
    item = createVariableNodeItem("XmlElementArray", EDGE_NODEID_XMLELEMENT, (void *) xmlValueArr, VARIABLE_NODE, 100);
    VERIFY_NON_NULL_NR(item);
    item->nodeType = ARRAY_NODE;
    item->arrayLength = 2;
    createNode(DEFAULT_NAMESPACE_VALUE, item);
    printf("\n|------------[Added] %s\n", item->browseName);
    deleteNodeItem(item);

    printf("\n[%d] Array node with NODEID variant: \n", ++index);
    Edge_NodeId nodeArr[2];
    nodeArr[0].namespaceIndex = DEFAULT_NAMESPACE_INDEX;
    nodeArr[0].identifierType = EDGE_INTEGER;
    nodeArr[0].identifier.numeric = EDGE_NODEID_ROOTFOLDER;

    nodeArr[1].namespaceIndex = DEFAULT_NAMESPACE_INDEX;
    nodeArr[1].identifierType = EDGE_STRING;
    nodeArr[1].identifier.string = EdgeStringAlloc("StringNodeId");

    item = createVariableNodeItem("NodeIdArray", EDGE_NODEID_NODEID, nodeArr, VARIABLE_NODE, 100);
    VERIFY_NON_NULL_NR(item);
    item->nodeType = ARRAY_NODE;
    item->arrayLength = 2;
    createNode(DEFAULT_NAMESPACE_VALUE, item);
    printf("\n|------------[Added] %s\n", item->browseName);
    deleteNodeItem(item);

    printf("\n[%d] Array node with qualified name variant: \n", ++index);
    Edge_QualifiedName qnValueArr[2] = { {2, {5, (uint8_t *)"qn100"} }, {2, {6, (uint8_t *)"qn1000"}} };
    item = createVariableNodeItem("QualifiedNameArray", EDGE_NODEID_QUALIFIEDNAME, (void *) qnValueArr,
            VARIABLE_NODE, 100);
    VERIFY_NON_NULL_NR(item);
    item->nodeType = ARRAY_NODE;
    item->arrayLength = 2;
    createNode(DEFAULT_NAMESPACE_VALUE, item);
    printf("\n|------------[Added] %s\n", item->browseName);
    deleteNodeItem(item);

    printf("\n[%d] Variable node with localized text variant: \n", ++index);
    Edge_LocalizedText ltValueArr[2] = { { {7, (uint8_t *)"localeA"}, {5, (uint8_t *)"textA"}},
            {{7, (uint8_t *)"localeB"}, {5, (uint8_t *)"textB"}} };
    item = createVariableNodeItem("LocalizedTextArray", EDGE_NODEID_LOCALIZEDTEXT, (void *) ltValueArr,
            VARIABLE_NODE, 100);
    item->nodeType = ARRAY_NODE;
    item->arrayLength = 2;
    VERIFY_NON_NULL_NR(item);
    createNode(DEFAULT_NAMESPACE_VALUE, item);
    printf("\n|------------[Added] %s\n", item->browseName);
    deleteNodeItem(item);

    printf("\n[%d] Array node with SByte values: \n", ++index);
    UA_SByte *sbData = (UA_SByte *) EdgeMalloc(sizeof(UA_SByte) * 5);
    if (IS_NOT_NULL(sbData))
    {
        sbData[0] = -128;
        sbData[1] = 112;
        sbData[2] = 120;
        sbData[3] = 122;
        sbData[4] = 127;
        item = createVariableNodeItem("SByteArray", EDGE_NODEID_SBYTE, (void *) sbData, VARIABLE_NODE, 100);
        VERIFY_NON_NULL_NR(item);
        item->nodeType = ARRAY_NODE;
        item->arrayLength = 5;
        createNode(DEFAULT_NAMESPACE_VALUE, item);
        printf("\n|------------[Added] %s\n", item->browseName);
        EdgeFree(sbData);
        deleteNodeItem(item);
    }
    else
    {
        printf("Error :: EdgeMalloc failed for UA_SByte array in Test create Nodes\n");
    }

    printf("\n[%d] Array node with Int32 values: \n", ++index);
    int *intData = (int *) EdgeMalloc(sizeof(int) * 7);
    if (IS_NOT_NULL(intData))
    {
        intData[0] = 11;
        intData[1] = 22;
        intData[2] = 33;
        intData[3] = 44;
        intData[4] = 55;
        intData[5] = 66;
        intData[6] = 77;
        item = createVariableNodeItem("Int32Array", EDGE_NODEID_INT32, (void *) intData, VARIABLE_NODE, 100);
        VERIFY_NON_NULL_NR(item);
        item->nodeType = ARRAY_NODE;
        item->arrayLength = 7;
        createNode(DEFAULT_NAMESPACE_VALUE, item);
        printf("\n|------------[Added] %s\n", item->browseName);
        EdgeFree(intData);
        deleteNodeItem(item);
    }
    else
    {
        printf("Error :: EdgeMalloc failed for int Array in Test create Nodes\n");
    }

    printf("\n[%d] Array node with Int64 values: \n", ++index);
    int64_t int64Data[5];
    int64Data[0] = 11111;
    int64Data[1] = 22222;
    int64Data[2] = 33333;
    int64Data[3] = 44444;
    int64Data[4] = 55555;
    item = createVariableNodeItem("Int64Array", EDGE_NODEID_INT64, (void *) int64Data, VARIABLE_NODE, 100);
    VERIFY_NON_NULL_NR(item);
    item->nodeType = ARRAY_NODE;
    item->arrayLength = 5;
    createNode(DEFAULT_NAMESPACE_VALUE, item);
    printf("\n|------------[Added] %s\n", item->browseName);
    deleteNodeItem(item);

    printf("\n[%d] Array node with UInt16 values: \n", ++index);
    uint16_t uint16Data[3] = {1000, 2000, 3000};
    item = createVariableNodeItem("UInt16Array", EDGE_NODEID_UINT16, (void *) uint16Data, VARIABLE_NODE, 100);
    VERIFY_NON_NULL_NR(item);
    item->nodeType = ARRAY_NODE;
    item->arrayLength = 3;
    createNode(DEFAULT_NAMESPACE_VALUE, item);
    printf("\n|------------[Added] %s\n", item->browseName);
    deleteNodeItem(item);

    printf("\n[%d] Array node with Int16 values: \n", ++index);
    int16_t int16Data[3] = {-1000, 0, 1000};
    item = createVariableNodeItem("Int16Array", EDGE_NODEID_INT16, (void *) int16Data, VARIABLE_NODE, 100);
    VERIFY_NON_NULL_NR(item);
    item->nodeType = ARRAY_NODE;
    item->arrayLength = 3;
    createNode(DEFAULT_NAMESPACE_VALUE, item);
    printf("\n|------------[Added] %s\n", item->browseName);
    deleteNodeItem(item);

    printf("\n[%d] Array node with UInt32 values: \n", ++index);
    uint32_t uint32Data[3] = {1, 500000000, 1000000000};
    item = createVariableNodeItem("UInt32Array", EDGE_NODEID_UINT32, (void *) uint32Data, VARIABLE_NODE, 100);
    VERIFY_NON_NULL_NR(item);
    item->nodeType = ARRAY_NODE;
    item->arrayLength = 3;
    createNode(DEFAULT_NAMESPACE_VALUE, item);
    printf("\n|------------[Added] %s\n", item->browseName);
    deleteNodeItem(item);

    printf("\n[%d] Array node with UInt64 values: \n", ++index);
    uint64_t uint64Data[3] = {1, 50000000000000, 1000000000000000000};
    item = createVariableNodeItem("UInt64Array", EDGE_NODEID_UINT64, (void *) uint64Data, VARIABLE_NODE, 100);
    VERIFY_NON_NULL_NR(item);
    item->nodeType = ARRAY_NODE;
    item->arrayLength = 3;
    createNode(DEFAULT_NAMESPACE_VALUE, item);
    printf("\n|------------[Added] %s\n", item->browseName);
    deleteNodeItem(item);

    printf("\n[%d] Array node with double values: \n", ++index);
    double *data = (double *) EdgeMalloc(sizeof(double) * 5);
    if (IS_NOT_NULL(data))
    {
        data[0] = 10.2;
        data[1] = 20.2;
        data[2] = 30.2;
        data[3] = 40.2;
        data[4] = 50.2;
        item = createVariableNodeItem("DoubleArray", EDGE_NODEID_DOUBLE, (void *) data, VARIABLE_NODE, 100);
        VERIFY_NON_NULL_NR(item);
        item->nodeType = ARRAY_NODE;
        item->arrayLength = 5;
        createNode(DEFAULT_NAMESPACE_VALUE, item);
        printf("\n|------------[Added] %s\n", item->browseName);
        EdgeFree(data);
        deleteNodeItem(item);
    }
    else
    {
        printf("Error :: EdgeMalloc failed for double Array in Test create Nodes\n");
    }

    printf("\n[%d] Array node with double values: \n", ++index);
    double dblArr[5] = {10.2, 20.2, 30.2, 40.2, 50.2};
    item = createVariableNodeItem("FloatArray", EDGE_NODEID_FLOAT, (void *) dblArr, VARIABLE_NODE, 100);
    VERIFY_NON_NULL_NR(item);
    item->nodeType = ARRAY_NODE;
    item->arrayLength = 5;
    createNode(DEFAULT_NAMESPACE_VALUE, item);
    printf("\n|------------[Added] %s\n", item->browseName);
    deleteNodeItem(item);

    printf("\n[%d] Array node with string values: \n", ++index);
    char **data1 = (char **) malloc(sizeof(char *) * 5);
    if (IS_NOT_NULL(data1))
    {
        data1[0] = (char *) EdgeMalloc(10);
        data1[1] = (char *) EdgeMalloc(10);
        data1[2] = (char *) EdgeMalloc(10);
        data1[3] = (char *) EdgeMalloc(10);
        data1[4] = (char *) EdgeMalloc(10);

        if (IS_NOT_NULL(data1[0]) && IS_NOT_NULL(data1[1]) && IS_NOT_NULL(data1[2])
        && IS_NOT_NULL(data1[3]) && IS_NOT_NULL(data1[4]))
        {
            strncpy(data1[0], "apple", strlen("apple"));
            data1[0][strlen("apple")] = '\0';
            strncpy(data1[1], "ball", strlen("ball"));
            data1[1][strlen("ball")] = '\0';
            strncpy(data1[2], "cats", strlen("cats"));
            data1[2][strlen("cats")] = '\0';
            strncpy(data1[3], "dogs", strlen("dogs"));
            data1[3][strlen("dogs")] = '\0';
            strncpy(data1[4], "elephant", strlen("elephant"));
            data1[4][strlen("elephant")] = '\0';

            item = createVariableNodeItem("CharArray", EDGE_NODEID_STRING, (void *) data1, VARIABLE_NODE, 100);
            VERIFY_NON_NULL_NR(item);
            item->nodeType = ARRAY_NODE;
            item->arrayLength = 5;
            createNode(DEFAULT_NAMESPACE_VALUE, item);
            printf("\n|------------[Added] %s\n", item->browseName);

            for (int i = 0; i < 5; i++)
            {
                EdgeFree(data1[i]);
            }
            EdgeFree(data1);
            deleteNodeItem(item);
        }
        else
        {
            printf("Error :: EdgeMalloc failed for char dataArray in Test create Nodes\n");
        }
    }
    else
    {
        printf("Error :: EdgeMalloc failed for char Array in Test create Nodes\n");
    }

    printf("\n[%d] Variable node with byte array variant: \n", ++index);
    UA_Byte *b_arrvalue = (UA_Byte *) EdgeCalloc(1, sizeof(UA_Byte) * 5);
    if (IS_NOT_NULL(b_arrvalue))
    {
        b_arrvalue[0] = 0x11;
        b_arrvalue[1] = 0x22;
        b_arrvalue[2] = 0x33;
        b_arrvalue[3] = 0x44;
        b_arrvalue[4] = 0x55;
        item = createVariableNodeItem("ByteArray", EDGE_NODEID_BYTE, (void *) b_arrvalue, VARIABLE_NODE, 100);
        VERIFY_NON_NULL_NR(item);
        item->arrayLength = 5;
        item->nodeType = ARRAY_NODE;
        createNode(DEFAULT_NAMESPACE_VALUE, item);
        printf("\n|------------[Added] %s\n", item->browseName);
        EdgeFree(b_arrvalue);
        deleteNodeItem(item);
    }
    else
    {
        printf("Error :: EdgeMalloc failed for UA_Byte Array in Test create Nodes\n");
    }

    printf("\n[%d] Array node with Guid values: \n", ++index);
    Edge_Guid guidArr[2] = {
        { 1, 0, 1, { 0, 0, 0, 0, 1, 1, 1, 1 } },
        { 2, 0, 2, { 0, 0, 0, 0, 2, 2, 2, 2 } }
    };
    item = createVariableNodeItem("GuidArray", EDGE_NODEID_GUID, (void *) guidArr, VARIABLE_NODE, 100);
    VERIFY_NON_NULL_NR(item);
    item->nodeType = ARRAY_NODE;
    item->arrayLength = 2;
    createNode(DEFAULT_NAMESPACE_VALUE, item);
    printf("\n|------------[Added] %s\n", item->browseName);
    deleteNodeItem(item);

    /******************* Object Node *********************/
    printf("\n[Create Object Node]\n");
    printf("\n[%d] Object node : \"Object1\"\n", ++index);
    EdgeNodeId *edgeNodeId = (EdgeNodeId *) EdgeCalloc(1, sizeof(EdgeNodeId));
    if (IS_NOT_NULL(edgeNodeId))
    {
        item = createNodeItem("Object1", OBJECT_NODE, edgeNodeId);
        VERIFY_NON_NULL_NR(item);
        createNode(DEFAULT_NAMESPACE_VALUE, item);
        printf("\n|------------[Added] %s\n", item->browseName);
        EdgeFree(item->sourceNodeId);
        deleteNodeItem(item);
    }
    else
    {
        printf("Error :: EdgeMalloc failed for Object1 in Test create Nodes\n");
    }

    printf("\n[%d] Object node : \"Object2\" with source Node \"Object1\"\n", ++index);
    edgeNodeId = (EdgeNodeId *) EdgeMalloc(sizeof(EdgeNodeId));
    if (IS_NOT_NULL(edgeNodeId))
    {
        edgeNodeId->nodeId = "Object1";
        item = createNodeItem("Object2", OBJECT_NODE, edgeNodeId);
        VERIFY_NON_NULL_NR(item);
        createNode(DEFAULT_NAMESPACE_VALUE, item);
        printf("\n|------------[Added] %s\n", item->browseName);
        EdgeFree(edgeNodeId);
        deleteNodeItem(item);
    }
    else
    {
        printf("Error :: EdgeMalloc failed for Object2 in Test create Nodes\n");
    }

    /******************* View Node *********************/
    printf("\n[Create View Node]\n");
    printf("\n[%d] View Node with ViewNode1\n", ++index);
    edgeNodeId = (EdgeNodeId *) EdgeCalloc(1, sizeof(EdgeNodeId));
    if (IS_NOT_NULL(edgeNodeId))
    {
        item = createNodeItem("ViewNode1", VIEW_NODE, edgeNodeId);
        VERIFY_NON_NULL_NR(item);
        createNode(DEFAULT_NAMESPACE_VALUE, item);
        printf("\n|------------[Added] %s\n", item->browseName);
        EdgeFree(edgeNodeId);
        deleteNodeItem(item);
    }
    else
    {
        printf("Error :: EdgeMalloc failed for ViewNode1 in Test create Nodes\n");
    }

    printf("\n[%d] View Node with ViewNode2\n", ++index);
    edgeNodeId = (EdgeNodeId *) EdgeMalloc(sizeof(EdgeNodeId));
    if (IS_NOT_NULL(edgeNodeId))
    {
        edgeNodeId->nodeId = "ViewNode1";
        item = createNodeItem("ViewNode2", VIEW_NODE, edgeNodeId);
        VERIFY_NON_NULL_NR(item);
        createNode(DEFAULT_NAMESPACE_VALUE, item);
        printf("\n|------------[Added] %s\n", item->browseName);
        EdgeFree(edgeNodeId);
        deleteNodeItem(item);
    }
    else
    {
        printf("Error :: EdgeMalloc failed for ViewNode2 in Test create Nodes\n");
    }

    /******************* Object Type Node *********************/
    printf("\n[Create Object Type Node]\n");
    printf("\n[%d] Object Type node : \"ObjectType1\"\n", ++index);
    edgeNodeId = (EdgeNodeId *) EdgeMalloc(sizeof(EdgeNodeId));
    if (IS_NOT_NULL(edgeNodeId))
    {
        edgeNodeId->nodeId = NULL; // no source node
        item = createNodeItem("ObjectType1", OBJECT_TYPE_NODE, edgeNodeId);
        VERIFY_NON_NULL_NR(item);
        createNode(DEFAULT_NAMESPACE_VALUE, item);
        printf("\n|------------[Added] %s\n", item->browseName);
        EdgeFree(edgeNodeId);
        deleteNodeItem(item);
    }
    else
    {
        printf("Error :: EdgeMalloc failed for ObjectType1 in Test create Nodes\n");
    }

    printf("\n[%d] Object Type node : \"ObjectType2\" with source Node \"ObjectType1\"\n", ++index);
    edgeNodeId = (EdgeNodeId *) EdgeMalloc(sizeof(EdgeNodeId));
    if (IS_NOT_NULL(edgeNodeId))
    {
        edgeNodeId->nodeId = "ObjectType1";
        item = createNodeItem("ObjectType2", OBJECT_TYPE_NODE, edgeNodeId);
        VERIFY_NON_NULL_NR(item);
        createNode(DEFAULT_NAMESPACE_VALUE, item);
        printf("\n|------------[Added] %s\n", item->browseName);
        EdgeFree(edgeNodeId);
        deleteNodeItem(item);
    }
    else
    {
        printf("Error :: EdgeMalloc failed for ObjectType2 in Test create Nodes\n");
    }

    printf("\n[%d] Object Type node : \"ObjectType3\" with source Node \"ObjectType2\"\n", ++index);
    edgeNodeId = (EdgeNodeId *) EdgeMalloc(sizeof(EdgeNodeId));
    if (IS_NOT_NULL(edgeNodeId))
    {
        edgeNodeId->nodeId = "ObjectType1";
        item = createNodeItem("ObjectType3", OBJECT_TYPE_NODE, edgeNodeId);
        VERIFY_NON_NULL_NR(item);
        createNode(DEFAULT_NAMESPACE_VALUE, item);
        printf("\n|------------[Added] %s\n", item->browseName);
        EdgeFree(edgeNodeId);
        deleteNodeItem(item);
    }
    else
    {
        printf("Error :: EdgeMalloc failed for ObjectType3 in Test create Nodes\n");
    }

    printf("\n[%d] Object Type node : \"ObjectType4\" with source Node \"ObjectType3\"\n", ++index);
    edgeNodeId = (EdgeNodeId *) EdgeMalloc(sizeof(EdgeNodeId));
    if (IS_NOT_NULL(edgeNodeId))
    {
        edgeNodeId->nodeId = "ObjectType1";
        item = createNodeItem("ObjectType4", OBJECT_TYPE_NODE, edgeNodeId);
        VERIFY_NON_NULL_NR(item);
        createNode(DEFAULT_NAMESPACE_VALUE, item);
        printf("\n|------------[Added] %s\n", item->browseName);
        EdgeFree(edgeNodeId);
        deleteNodeItem(item);
    }
    else
    {
        printf("Error :: EdgeMalloc failed for ObjectType4 in Test create Nodes\n");
    }

    printf("\n[%d] Object Type node : \"ObjectType5\" with source Node \"ObjectType3\"\n", ++index);
    edgeNodeId = (EdgeNodeId *) EdgeMalloc(sizeof(EdgeNodeId));
    if (IS_NOT_NULL(edgeNodeId))
    {
        edgeNodeId->nodeId = "ObjectType1";
        item = createNodeItem("ObjectType5", OBJECT_TYPE_NODE, edgeNodeId);
        VERIFY_NON_NULL_NR(item);
        createNode(DEFAULT_NAMESPACE_VALUE, item);
        printf("\n|------------[Added] %s\n", item->browseName);
        EdgeFree(edgeNodeId);
        deleteNodeItem(item);
    }
    else
    {
        printf("Error :: EdgeMalloc failed for ObjectType5 in Test create Nodes\n");
    }

    /******************* Variable Type Node *********************/
    printf("\n[Create Variable Type Node]\n");
    printf("\n[%d] Variable Type Node with Double Variable Type \n", ++index);
    double d[2] =
    { 10.2, 20.2 };
    item = createVariableNodeItem("DoubleVariableType", EDGE_NODEID_DOUBLE, (void *) d, VARIABLE_TYPE_NODE, 100);
    VERIFY_NON_NULL_NR(item);
    item->arrayLength = 2;
    createNode(DEFAULT_NAMESPACE_VALUE, item);
    printf("\n|------------[Added] %s\n", item->browseName);
    deleteNodeItem(item);

    /******************* Data Type Node *********************/
    printf("\n[%d] Data Type Node with DataType1\n", ++index);
    edgeNodeId = (EdgeNodeId *) EdgeCalloc(1, sizeof(EdgeNodeId));
    if (IS_NOT_NULL(edgeNodeId))
    {
        item = createNodeItem("DataType1", DATA_TYPE_NODE, edgeNodeId);
        VERIFY_NON_NULL_NR(item);
        createNode(DEFAULT_NAMESPACE_VALUE, item);
        printf("\n|------------[Added] %s\n", item->browseName);
        EdgeFree(edgeNodeId);
        deleteNodeItem(item);
    }
    else
    {
        printf("Error :: EdgeMalloc failed for DataType1 in Test create Nodes\n");
    }

    printf("\n[%d] Data Type Node with DataType2\n", ++index);
    edgeNodeId = (EdgeNodeId *) EdgeMalloc(sizeof(EdgeNodeId));
    if (IS_NOT_NULL(edgeNodeId))
    {
        edgeNodeId->nodeId = "DataType1";
        item = createNodeItem("DataType2", DATA_TYPE_NODE, edgeNodeId);
        VERIFY_NON_NULL_NR(item);
        createNode(DEFAULT_NAMESPACE_VALUE, item);
        EdgeFree(edgeNodeId);
    }
    else
    {
        printf("Error :: EdgeMalloc failed for DataType2 in Test create Nodes\n");
    }

    /******************* Reference Type Node *********************/
    printf("\n[Create Reference Type Node]\n");
    printf("\n[%d] Reference Type Node with ReferenceTypeNode1", ++index);
    edgeNodeId = (EdgeNodeId *) EdgeCalloc(1, sizeof(EdgeNodeId));
    if (IS_NOT_NULL(edgeNodeId))
    {
        item = createNodeItem("ReferenceTypeNode1", REFERENCE_TYPE_NODE, edgeNodeId);
        VERIFY_NON_NULL_NR(item);
        createNode(DEFAULT_NAMESPACE_VALUE, item);
        printf("\n|------------[Added] %s\n", item->browseName);
        EdgeFree(edgeNodeId);
    }
    else
    {
        printf("Error :: EdgeMalloc failed for ReferenceTypeNode1 in Test create Nodes\n");
    }

    printf("\n[%d] Reference Type Node with source node\"ReferenceTypeNode1\"\n", ++index);
    edgeNodeId = (EdgeNodeId *) EdgeMalloc(sizeof(EdgeNodeId));
    if (IS_NOT_NULL(edgeNodeId))
    {
        edgeNodeId->nodeId = "ReferenceTypeNode1";
        item = createNodeItem("ReferenceTypeNode2", REFERENCE_TYPE_NODE, edgeNodeId);
        VERIFY_NON_NULL_NR(item);
        createNode(DEFAULT_NAMESPACE_VALUE, item);
        printf("\n|------------[Added] %s\n", item->browseName);
        EdgeFree(edgeNodeId);
    }
    else
    {
        printf("Error :: EdgeMalloc failed for ReferenceTypeNode2 in Test create Nodes\n");
    }

    deleteNodeItem(item);

    //METHOD NODE
    EdgeNodeItem *methodNodeItem = (EdgeNodeItem *) EdgeMalloc(sizeof(EdgeNodeItem));
    methodNodeItem->browseName = "square(x)";
    methodNodeItem->sourceNodeId = NULL;

    EdgeMethod *method = (EdgeMethod *) EdgeMalloc(sizeof(EdgeMethod));
    method->description = "Calculate square";
    method->methodNodeName = "square";
    method->method_fn = square_method;
    method->num_inpArgs = 1;
    method->inpArg = (EdgeArgument **) malloc(sizeof(EdgeArgument *) * method->num_inpArgs);
    for (int idx = 0; idx < method->num_inpArgs; idx++)
    {
        method->inpArg[idx] = (EdgeArgument *) EdgeMalloc(sizeof(EdgeArgument));
        method->inpArg[idx]->argType = EDGE_NODEID_DOUBLE;
        method->inpArg[idx]->valType = SCALAR;
    }

    method->num_outArgs = 2;
    method->outArg = (EdgeArgument **) malloc(sizeof(EdgeArgument *) * method->num_outArgs);
    for (int idx = 0; idx < method->num_outArgs; idx++)
    {
        method->outArg[idx] = (EdgeArgument *) EdgeMalloc(sizeof(EdgeArgument));
        method->outArg[idx]->argType = EDGE_NODEID_DOUBLE;
        method->outArg[idx]->valType = SCALAR;
    }
    EdgeResult result = createMethodNode(DEFAULT_NAMESPACE_VALUE, methodNodeItem, method);
    EXPECT_EQ(result.code, STATUS_OK);
    EdgeFree(methodNodeItem);

    /* Method Node */
    EdgeNodeItem *methodNodeItem1 = (EdgeNodeItem *) EdgeMalloc(sizeof(EdgeNodeItem));
    methodNodeItem1->browseName = "incrementInc32Array(x,delta)";
    methodNodeItem1->sourceNodeId = NULL;

    EdgeMethod *method1 = (EdgeMethod *) EdgeMalloc(sizeof(EdgeMethod));
    method1->description = "Increment int32 array by delta";
    method1->methodNodeName = "incrementInc32Array";
    method1->method_fn = increment_int32Array_method;

    method1->num_inpArgs = 2;
    method1->inpArg = (EdgeArgument **) malloc(sizeof(EdgeArgument *) * method1->num_inpArgs);
    method1->inpArg[0] = (EdgeArgument *) EdgeMalloc(sizeof(EdgeArgument));
    method1->inpArg[0]->argType = EDGE_NODEID_INT32;
    method1->inpArg[0]->valType = ARRAY_1D;
    method1->inpArg[0]->arrayLength = 5;

    method1->inpArg[1] = (EdgeArgument *) EdgeMalloc(sizeof(EdgeArgument));
    method1->inpArg[1]->argType = EDGE_NODEID_INT32;
    method1->inpArg[1]->valType = SCALAR;

    method1->num_outArgs = 1;
    method1->outArg = (EdgeArgument **) malloc(sizeof(EdgeArgument *) * method1->num_outArgs);
    for (int idx = 0; idx < method1->num_outArgs; idx++)
    {
        method1->outArg[idx] = (EdgeArgument *) EdgeMalloc(sizeof(EdgeArgument));
        method1->outArg[idx]->argType = EDGE_NODEID_INT32;
        method1->outArg[idx]->valType = ARRAY_1D;
        method1->outArg[idx]->arrayLength = 5;
    }
    createMethodNode(DEFAULT_NAMESPACE_VALUE, methodNodeItem1, method1);
    EdgeFree(methodNodeItem1);

    /* Method Node */
    EdgeNodeItem *methodNodeItem2 = (EdgeNodeItem *) EdgeMalloc(sizeof(EdgeNodeItem));
    methodNodeItem2->browseName = "print_string_array(x)";
    methodNodeItem2->sourceNodeId = NULL;

    EdgeMethod *method2 = (EdgeMethod *) EdgeMalloc(sizeof(EdgeMethod));
    method2->description = "string copy method";
    method2->methodNodeName = "string_method";
    method2->method_fn = string_method;

    method2->num_inpArgs = 1;
    method2->inpArg = (EdgeArgument **) malloc(sizeof(EdgeArgument *) * method2->num_inpArgs);
    method2->inpArg[0] = (EdgeArgument *) EdgeMalloc(sizeof(EdgeArgument));
    method2->inpArg[0]->argType = EDGE_NODEID_STRING;
    method2->inpArg[0]->valType = ARRAY_1D;
    method2->inpArg[0]->arrayLength = 5;

    method2->num_outArgs = 1;
    method2->outArg = (EdgeArgument **) malloc(sizeof(EdgeArgument *) * method2->num_outArgs);
    for (int idx = 0; idx < method2->num_outArgs; idx++)
    {
        method2->outArg[idx] = (EdgeArgument *) EdgeMalloc(sizeof(EdgeArgument));
        method2->outArg[idx]->argType = EDGE_NODEID_STRING;
        method2->outArg[idx]->valType = ARRAY_1D;
        method2->outArg[idx]->arrayLength = 5;
    }
    createMethodNode(DEFAULT_NAMESPACE_VALUE, methodNodeItem2, method2);
    EdgeFree(methodNodeItem2);

    EdgeNodeItem *methodNodeItem3 = (EdgeNodeItem *) EdgeMalloc(sizeof(EdgeNodeItem));
    VERIFY_NON_NULL_NR(methodNodeItem3);
    methodNodeItem3->browseName = "print_string(x)";
    methodNodeItem3->sourceNodeId = NULL;

    EdgeMethod *method3 = (EdgeMethod *) EdgeMalloc(sizeof(EdgeMethod));
    method3->description = "print str";
    method3->methodNodeName = "print";
    method3->method_fn = test_method_print_string;

    method3->num_inpArgs = 1;
    method3->inpArg = (EdgeArgument **) malloc(sizeof(EdgeArgument *) * method3->num_inpArgs);
    if (IS_NULL(method3->inpArg))
    {
        EdgeFree(methodNodeItem3);
        EdgeFree(method3);
        printf("Error :: EdgeMalloc failed for method method3->inpArg  in Test create Nodes\n");
        return;
    }
    method3->inpArg[0] = (EdgeArgument *) EdgeMalloc(sizeof(EdgeArgument));
    if (IS_NULL(method))
    {
        EdgeFree(method3->inpArg);
        EdgeFree(methodNodeItem3);
        EdgeFree(method3);
        printf("Error :: EdgeMalloc failed for method method3->inpArg[0]  in Test create Nodes\n");
        return;
    }
    method3->inpArg[0]->argType = EDGE_NODEID_STRING;
    method3->inpArg[0]->valType = SCALAR;

    method3->num_outArgs = 1;
    method3->outArg = (EdgeArgument **) malloc(sizeof(EdgeArgument *) * method3->num_outArgs);
    for (int idx = 0; idx < method3->num_outArgs; idx++)
    {
        method3->outArg[idx] = (EdgeArgument *) EdgeMalloc(sizeof(EdgeArgument));
        method3->outArg[idx]->argType = EDGE_NODEID_STRING;
        method3->outArg[idx]->valType = SCALAR;
        method3->outArg[idx]->arrayLength = 0;
    }
    createMethodNode(DEFAULT_NAMESPACE_VALUE, methodNodeItem3, method3);
    printf("\n|------------[Added] %s\n", methodNodeItem3->browseName);
    EdgeFree(methodNodeItem3);

    if (epInfo->endpointUri != NULL)
    {
        free(epInfo->endpointUri);
        epInfo->endpointUri = NULL;
    }

    EdgeReference *reference = (EdgeReference *) EdgeCalloc(1, sizeof(EdgeReference));
    EXPECT_EQ(NULL != reference, true);
    reference->forward = true;
    reference->sourceNamespace = copyString(DEFAULT_NAMESPACE_VALUE);
    reference->sourcePath = "ViewNode1";
    reference->targetNamespace = copyString(DEFAULT_NAMESPACE_VALUE);
    reference->targetPath = "robot_name";
    /* default reference ID : Organizes */
    addReference(reference);
    EdgeFree(reference);

    reference = (EdgeReference *) EdgeCalloc(1, sizeof(EdgeReference));
    EXPECT_EQ(NULL != reference, true);
    reference->forward = true;
    reference->sourceNamespace = copyString(DEFAULT_NAMESPACE_VALUE);
    reference->sourcePath = "ViewNode1";
    reference->targetNamespace = copyString(DEFAULT_NAMESPACE_VALUE);
    reference->targetPath = "robot_id";
    /* default reference ID : Organizes */
    addReference(reference);
    EdgeFree(reference);

    reference = (EdgeReference *) EdgeCalloc(1, sizeof(EdgeReference));
    EXPECT_EQ(NULL != reference, true);
    reference->forward = true;
    reference->sourceNamespace = copyString(DEFAULT_NAMESPACE_VALUE);
    reference->sourcePath = "ViewNode1";
    reference->targetNamespace = copyString(DEFAULT_NAMESPACE_VALUE);
    reference->targetPath = "robot_position";
    /* default reference ID : Organizes */
    addReference(reference);
    EdgeFree(reference);
}

TEST_F(OPC_clientTests , ServerModifyVariableNode_P)
{
    EdgeVersatility *message = (EdgeVersatility *) EdgeMalloc(sizeof(EdgeVersatility));
    EXPECT_EQ(NULL != message, true);

    // Int32
    int32_t val = 60;
    message->value = &val;
    EdgeResult result = modifyVariableNode(DEFAULT_NAMESPACE_VALUE, "Int32", message);
    EXPECT_EQ(result.code, STATUS_OK);
    EdgeFree(message);
    usleep(500 * 1000);

    // Double
    message = (EdgeVersatility *) EdgeMalloc(sizeof(EdgeVersatility));
    EXPECT_EQ(NULL != message, true);
    double d_val = 60.656;
    message->value = &d_val;
    result = modifyVariableNode(DEFAULT_NAMESPACE_VALUE, "Double", message);
    EXPECT_EQ(result.code, STATUS_OK);
    EdgeFree(message);
    usleep(500 * 1000);

    // String1
    message = (EdgeVersatility *) EdgeMalloc(sizeof(EdgeVersatility));
    EXPECT_EQ(NULL != message, true);
    char str[128];
    strncpy(str, "changed_str", strlen("changed_str"));
    str[strlen("changed_str")] = '\0';
    message->value = &str;
    result = modifyVariableNode(DEFAULT_NAMESPACE_VALUE, "String1", message);
    EXPECT_EQ(result.code, STATUS_OK);
    EdgeFree(message);
    usleep(500 * 1000);

    // ByteString
    message = (EdgeVersatility *) EdgeMalloc(sizeof(EdgeVersatility));
    EXPECT_EQ(NULL != message, true);
    strncpy(str, "changed_bytestring", strlen("changed_bytestring"));
    str[strlen("changed_bytestring")] = '\0';
    message->value = &str;
    result = modifyVariableNode(DEFAULT_NAMESPACE_VALUE, "ByteString", message);
    EXPECT_EQ(result.code, STATUS_OK);
    EdgeFree(message);
    usleep(500 * 1000);

    // CharArray
    int num_values = 3;
    char **new_str = (char**) malloc(sizeof(char*) * num_values);
    for (int i = 0; i < num_values; i++)
    {
        new_str[i] = (char*) malloc(sizeof(char) * 15);
        snprintf(new_str[i], 128, "CharArray%d", i+1);
    }
    message = (EdgeVersatility *) EdgeMalloc(sizeof(EdgeVersatility));
    EXPECT_EQ(NULL != message, true);
    message->arrayLength = num_values;
    message->isArray = true;
    message->value = (void *) new_str;
    modifyVariableNode(DEFAULT_NAMESPACE_VALUE, "CharArray", message);
    EdgeFree(message);
    usleep(500 * 1000);
    for (int i = 0; i < num_values; i++)
    {
        EdgeFree(new_str[i]);
    }
    EdgeFree(new_str);

    // ByteString Array
    num_values = 3;
    new_str = (char**) malloc(sizeof(char*) * num_values);
    for (int i = 0; i < num_values; i++)
    {
        new_str[i] = (char*) malloc(sizeof(char) * 15);
        snprintf(new_str[i], 128, "bs%d", i+1);
    }
    message = (EdgeVersatility *) EdgeMalloc(sizeof(EdgeVersatility));
    EXPECT_EQ(NULL != message, true);
    message->arrayLength = num_values;
    message->isArray = true;
    message->value = (void *) new_str;
    modifyVariableNode(DEFAULT_NAMESPACE_VALUE, "ByteStringArray", message);
    EdgeFree(message);
    usleep(500 * 1000);
    for (int i = 0; i < num_values; i++)
    {
        EdgeFree(new_str[i]);
    }
    EdgeFree(new_str);
}

TEST_F(OPC_clientTests , ServerModifyVariableNode_N1)
{
    EdgeVersatility *message = (EdgeVersatility *) EdgeMalloc(sizeof(EdgeVersatility));
    ASSERT_EQ(NULL != message, true);
    EdgeResult result = modifyVariableNode(NULL, "ByteStringArray", message);
    ASSERT_EQ(result.code, STATUS_PARAM_INVALID);
    EdgeFree(message);
}

TEST_F(OPC_clientTests , ServerModifyVariableNode_N2)
{
    EdgeVersatility *message = (EdgeVersatility *) EdgeMalloc(sizeof(EdgeVersatility));
    ASSERT_EQ(NULL != message, true);
    EdgeResult result = modifyVariableNode(DEFAULT_NAMESPACE_VALUE, NULL, message);
    ASSERT_EQ(result.code, STATUS_PARAM_INVALID);
    EdgeFree(message);
}

TEST_F(OPC_clientTests , ServerModifyVariableNode_N3)
{
    EdgeResult result = modifyVariableNode(DEFAULT_NAMESPACE_VALUE, "ByteStringArray", NULL);
    ASSERT_EQ(result.code, STATUS_PARAM_INVALID);
}

TEST_F(OPC_clientTests , ConfigureClient_P)
{
    EXPECT_EQ(NULL == config, false);

    EXPECT_EQ(NULL == config->recvCallback, false);

    EXPECT_EQ(NULL == config->statusCallback, false);

    EXPECT_EQ(NULL == config->discoveryCallback, false);
}

TEST_F(OPC_clientTests , InitializeClient_P)
{
    EXPECT_EQ(strcmp(ipAddress, IPADDRESS) == 0, true);

    EXPECT_EQ(strcmp(endpointUri, ENDPOINT_URI) == 0, false);

    EXPECT_EQ(strcmp(endpointUri, ipAddress) == 0, true);
}

TEST_F(OPC_clientTests , FindServers_P1)
{
    findServersHelper();
}

TEST_F(OPC_clientTests , FindServers_P2)
{
    size_t serverUrisSize = 2;
    unsigned char *serverUris[2] = {NULL};
    serverUris[0] = (unsigned char *)"urn:digitalpetri:opcua:client";
    serverUris[1] = (unsigned char *)"urn:digitalpetri:opcua:invalid_client";
    size_t localeIdsSize = 0;
    unsigned char **localeIds = NULL;
    EdgeApplicationConfig *registeredServers = NULL;
    size_t registeredServersSize = 0;
    EdgeResult res = findServers(endpointUri, serverUrisSize, serverUris, localeIdsSize, localeIds, &registeredServersSize, &registeredServers);
    EXPECT_EQ(res.code,STATUS_OK);

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

TEST_F(OPC_clientTests , FindServers_P3)
{
    size_t serverUrisSize = 0;
   unsigned char **serverUris = NULL;
   size_t localeIdsSize = 1;
   unsigned char *localeIds[1] = {NULL};
   localeIds[0] = (unsigned char *)"en-US";
    EdgeApplicationConfig *registeredServers = NULL;
    size_t registeredServersSize = 0;
    EdgeResult res = findServers(endpointUri, serverUrisSize, serverUris, localeIdsSize, localeIds, &registeredServersSize, &registeredServers);
    EXPECT_EQ(res.code,STATUS_OK);

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

TEST_F(OPC_clientTests , FindServers_N1)
{
    // Invalid Endpoint
    size_t serverUrisSize = 0;
    unsigned char **serverUris = NULL;
    size_t localeIdsSize = 0;
    unsigned char **localeIds = NULL;
    EdgeApplicationConfig *registeredServers = NULL;
    size_t registeredServersSize = 0;
    EdgeResult res = findServers("opc.tcp://localhost:4842", serverUrisSize, serverUris, localeIdsSize, localeIds, &registeredServersSize, &registeredServers);
    EXPECT_EQ(res.code,STATUS_SERVICE_RESULT_BAD);

    // Application has to deallocate the memory for EdgeApplicationConfig array and its members.
    for(size_t idx = 0; idx < registeredServersSize ; ++idx)
    {
        destroyEdgeApplicationConfigMembers(&registeredServers[idx]);
    }
    EdgeFree(registeredServers);;
}

TEST_F(OPC_clientTests , FindServers_N2)
{
    // Invalid Endpoint
    size_t serverUrisSize = 1;
    unsigned char **serverUris = NULL;
    size_t localeIdsSize = 0;
    unsigned char **localeIds = NULL;
    EdgeApplicationConfig *registeredServers = NULL;
    size_t registeredServersSize = 0;
    EdgeResult res = findServers(endpointUri, serverUrisSize, serverUris, localeIdsSize, localeIds, &registeredServersSize, &registeredServers);
    EXPECT_EQ(res.code,STATUS_PARAM_INVALID);

    // Application has to deallocate the memory for EdgeApplicationConfig array and its members.
    for(size_t idx = 0; idx < registeredServersSize ; ++idx)
    {
        destroyEdgeApplicationConfigMembers(&registeredServers[idx]);
    }
    EdgeFree(registeredServers);;
}

TEST_F(OPC_clientTests , FindServers_N3)
{
    // Invalid Endpoint
    size_t serverUrisSize = 0;
    unsigned char **serverUris = NULL;
    size_t localeIdsSize = 1;
    unsigned char **localeIds = NULL;
    EdgeApplicationConfig *registeredServers = NULL;
    size_t registeredServersSize = 0;
    EdgeResult res = findServers(endpointUri, serverUrisSize, serverUris, localeIdsSize, localeIds, &registeredServersSize, &registeredServers);
    EXPECT_EQ(res.code,STATUS_PARAM_INVALID);

    // Application has to deallocate the memory for EdgeApplicationConfig array and its members.
    for(size_t idx = 0; idx < registeredServersSize ; ++idx)
    {
        destroyEdgeApplicationConfigMembers(&registeredServers[idx]);
    }
    EdgeFree(registeredServers);;
}

TEST_F(OPC_clientTests , createEdgeMessage_P)
{
    EdgeMessage *msg = createEdgeMessage(endpointUri, 1, CMD_GET_ENDPOINTS);
    EXPECT_EQ(NULL != msg, true);
    EdgeFree(msg);
}

TEST_F(OPC_clientTests , createEdgeMessage_N)
{
    EdgeMessage *msg = createEdgeMessage(NULL, 1, CMD_GET_ENDPOINTS);
    EXPECT_EQ(NULL != msg, false);
}

TEST_F(OPC_clientTests , createEdgeAttributeMessage_P)
{
    EdgeMessage *msg = createEdgeAttributeMessage(endpointUri, 1, CMD_READ);
    EXPECT_EQ(NULL != msg, true);
    EdgeFree(msg);
}

TEST_F(OPC_clientTests , createEdgeAttributeMessage_N)
{
    EdgeMessage *msg = createEdgeAttributeMessage(NULL, 1, CMD_READ);
    EXPECT_EQ(NULL != msg, false);
}

TEST_F(OPC_clientTests , createEdgeSubMessage_P)
{
    EdgeMessage *msg = createEdgeSubMessage(endpointUri, node_arr[0], 1, Edge_Create_Sub);
    EXPECT_EQ(NULL != msg, true);
    EdgeFree(msg);
}

TEST_F(OPC_clientTests , createEdgeSubMessage_N)
{
    EdgeMessage *msg = createEdgeSubMessage(NULL, node_arr[0], 1, Edge_Create_Sub);
    EXPECT_EQ(NULL != msg, false);
}

TEST_F(OPC_clientTests , getEndpointInfo_N1)
{
    EXPECT_EQ(startClientFlag, false);

    EdgeResult res = getEndpointInfo(NULL);
    EXPECT_EQ(res.code, STATUS_PARAM_INVALID);
}

TEST_F(OPC_clientTests , getEndpointInfo_N2)
{
    EXPECT_EQ(startClientFlag, false);

    EdgeMessage *msg = createEdgeMessage(endpointUri, 1, CMD_GET_ENDPOINTS);
    EXPECT_EQ(NULL != msg, true);
    free(msg->endpointInfo);
    msg->endpointInfo = NULL;

    EdgeResult res = getEndpointInfo(msg);
    EXPECT_EQ(res.code, STATUS_PARAM_INVALID);
    destroyEdgeMessage(msg);
}

TEST_F(OPC_clientTests , StartClient_P)
{
    EXPECT_EQ(startClientFlag, false);

    EdgeMessage *msg = createEdgeMessage(endpointUri, 1, CMD_GET_ENDPOINTS);
    EXPECT_EQ(NULL != msg, true);

    EdgeResult res = getEndpointInfo(msg);
    EXPECT_EQ(res.code, STATUS_OK);

    EXPECT_EQ(startClientFlag, true);

    destroyEdgeMessage(msg);

    stop_client();
    EXPECT_EQ(startClientFlag, false);
}

TEST_F(OPC_clientTests , ClientRead_P1)
{
    EXPECT_EQ(startClientFlag, false);

    EdgeMessage *msg = createEdgeMessage(endpointUri, 1, CMD_GET_ENDPOINTS);
    EXPECT_EQ(NULL != msg, true);

    EdgeResult res = getEndpointInfo(msg);
    EXPECT_EQ(res.code, STATUS_OK);

    EXPECT_EQ(startClientFlag, true);

    destroyEdgeMessage(msg);

    readNodeFlag = true;
    testRead_P1(endpointUri);
    readNodeFlag = false;

    stop_client();
    EXPECT_EQ(startClientFlag, false);
}

TEST_F(OPC_clientTests , ClientRead_P2)
{
    EXPECT_EQ(startClientFlag, false);

    EdgeMessage *msg = createEdgeMessage(endpointUri, 1, CMD_GET_ENDPOINTS);
    EXPECT_EQ(NULL != msg, true);

    EdgeResult res = getEndpointInfo(msg);
    EXPECT_EQ(res.code, STATUS_OK);

    EXPECT_EQ(startClientFlag, true);

    destroyEdgeMessage(msg);

    readNodeFlag = true;
    testRead_P2(endpointUri);
    readNodeFlag = false;

    stop_client();
    EXPECT_EQ(startClientFlag, false);
}

TEST_F(OPC_clientTests , ClientRead_P3)
{
    EXPECT_EQ(startClientFlag, false);

    EdgeMessage *msg = createEdgeMessage(endpointUri, 1, CMD_GET_ENDPOINTS);
    EXPECT_EQ(NULL != msg, true);

    EdgeResult res = getEndpointInfo(msg);
    EXPECT_EQ(res.code, STATUS_OK);

    EXPECT_EQ(startClientFlag, true);

    destroyEdgeMessage(msg);

    readNodeFlag = true;
    testRead_P3(endpointUri);
    readNodeFlag = false;

    stop_client();
    EXPECT_EQ(startClientFlag, false);
}

TEST_F(OPC_clientTests , ClientRead_P4)
{
    EXPECT_EQ(startClientFlag, false);

    EdgeMessage *msg = createEdgeMessage(endpointUri, 1, CMD_GET_ENDPOINTS);
    EXPECT_EQ(NULL != msg, true);

    EdgeResult res = getEndpointInfo(msg);
    EXPECT_EQ(res.code, STATUS_OK);

    EXPECT_EQ(startClientFlag, true);

    destroyEdgeMessage(msg);

    readNodeFlag = true;
    testRead_P4(endpointUri);
    readNodeFlag = false;

    stop_client();
    EXPECT_EQ(startClientFlag, false);
}

TEST_F(OPC_clientTests , ClientRead_P5)
{
    EXPECT_EQ(startClientFlag, false);

    EdgeMessage *msg = createEdgeMessage(endpointUri, 1, CMD_GET_ENDPOINTS);
    EXPECT_EQ(NULL != msg, true);

    EdgeResult res = getEndpointInfo(msg);
    EXPECT_EQ(res.code, STATUS_OK);

    EXPECT_EQ(startClientFlag, true);

    destroyEdgeMessage(msg);

    readNodeFlag = true;
    testRead_P5(endpointUri);
    readNodeFlag = false;

    stop_client();
    EXPECT_EQ(startClientFlag, false);
}

TEST_F(OPC_clientTests , ClientRead_N1)
{
    EXPECT_EQ(startClientFlag, false);

    EdgeMessage *msg = createEdgeMessage(endpointUri, 1, CMD_GET_ENDPOINTS);
    EXPECT_EQ(NULL != msg, true);

    EdgeResult res = getEndpointInfo(msg);
    EXPECT_EQ(res.code, STATUS_OK);

    EXPECT_EQ(startClientFlag, true);

    destroyEdgeMessage(msg);

    readNodeFlag = true;
    testReadWithoutEndpoint();
    readNodeFlag = false;

    stop_client();
    EXPECT_EQ(startClientFlag, false);
}

TEST_F(OPC_clientTests , ClientRead_N2)
{
    EXPECT_EQ(startClientFlag, false);

    EdgeMessage *msg = createEdgeMessage(endpointUri, 1, CMD_GET_ENDPOINTS);
    EXPECT_EQ(NULL != msg, true);

    EdgeResult res = getEndpointInfo(msg);
    EXPECT_EQ(res.code, STATUS_OK);

    EXPECT_EQ(startClientFlag, true);

    destroyEdgeMessage(msg);

    readNodeFlag = true;
    testReadWithoutValueAlias(endpointUri);
    readNodeFlag = false;

    stop_client();
    EXPECT_EQ(startClientFlag, false);
}

TEST_F(OPC_clientTests , ClientRead_N3)
{
    EXPECT_EQ(startClientFlag, false);

    EdgeMessage *msg = createEdgeMessage(endpointUri, 1, CMD_GET_ENDPOINTS);
    EXPECT_EQ(NULL != msg, true);

    EdgeResult res = getEndpointInfo(msg);
    EXPECT_EQ(res.code, STATUS_OK);

    EXPECT_EQ(startClientFlag, true);

    destroyEdgeMessage(msg);

    readNodeFlag = true;
    testReadWithoutMessage();
    readNodeFlag = false;

    stop_client();
    EXPECT_EQ(startClientFlag, false);
}

TEST_F(OPC_clientTests , ClientRead_N4)
{
    EXPECT_EQ(startClientFlag, false);

    EdgeMessage *msg = createEdgeMessage(endpointUri, 1, CMD_GET_ENDPOINTS);
    EXPECT_EQ(NULL != msg, true);

    EdgeResult res = getEndpointInfo(msg);
    EXPECT_EQ(res.code, STATUS_OK);

    EXPECT_EQ(startClientFlag, true);

    destroyEdgeMessage(msg);

    readNodeFlag = true;
    testReadWithoutCommand();
    readNodeFlag = false;

    stop_client();
    EXPECT_EQ(startClientFlag, false);
}

TEST_F(OPC_clientTests , ClientWrite_P1)
{
    EXPECT_EQ(startClientFlag, false);

    EdgeMessage *msg = createEdgeMessage(endpointUri, 1, CMD_GET_ENDPOINTS);
    EXPECT_EQ(NULL != msg, true);

    EdgeResult res = getEndpointInfo(msg);
    EXPECT_EQ(res.code, STATUS_OK);

    EXPECT_EQ(startClientFlag, true);

    destroyEdgeMessage(msg);

    testWrite_P1(endpointUri);

    stop_client();
    EXPECT_EQ(startClientFlag, false);
}

TEST_F(OPC_clientTests , ClientWrite_P2)
{
    EXPECT_EQ(startClientFlag, false);

    EdgeMessage *msg = createEdgeMessage(endpointUri, 1, CMD_GET_ENDPOINTS);
    EXPECT_EQ(NULL != msg, true);

    EdgeResult res = getEndpointInfo(msg);
    EXPECT_EQ(res.code, STATUS_OK);

    EXPECT_EQ(startClientFlag, true);

    destroyEdgeMessage(msg);

    testWrite_P2(endpointUri);

    stop_client();
    EXPECT_EQ(startClientFlag, false);
}

TEST_F(OPC_clientTests , ClientWrite_P3)
{
    EXPECT_EQ(startClientFlag, false);

    EdgeMessage *msg = createEdgeMessage(endpointUri, 1, CMD_GET_ENDPOINTS);
    EXPECT_EQ(NULL != msg, true);

    EdgeResult res = getEndpointInfo(msg);
    EXPECT_EQ(res.code, STATUS_OK);

    EXPECT_EQ(startClientFlag, true);

    destroyEdgeMessage(msg);

    testWrite_P3(endpointUri);

    stop_client();
    EXPECT_EQ(startClientFlag, false);
}

TEST_F(OPC_clientTests , ClientWrite_P4)
{
    EXPECT_EQ(startClientFlag, false);

    EdgeMessage *msg = createEdgeMessage(endpointUri, 1, CMD_GET_ENDPOINTS);
    EXPECT_EQ(NULL != msg, true);

    EdgeResult res = getEndpointInfo(msg);
    EXPECT_EQ(res.code, STATUS_OK);

    EXPECT_EQ(startClientFlag, true);

    destroyEdgeMessage(msg);

    testWrite_P4(endpointUri);

    stop_client();
    EXPECT_EQ(startClientFlag, false);
}

TEST_F(OPC_clientTests , ClientWrite_N1)
{
    EXPECT_EQ(startClientFlag, false);

    EdgeMessage *msg = createEdgeMessage(endpointUri, 1, CMD_GET_ENDPOINTS);
    EXPECT_EQ(NULL != msg, true);

    EdgeResult res = getEndpointInfo(msg);
    EXPECT_EQ(res.code, STATUS_OK);

    EXPECT_EQ(startClientFlag, true);

    destroyEdgeMessage(msg);

    testWriteWithoutEndpoint();

    stop_client();
    EXPECT_EQ(startClientFlag, false);
}

TEST_F(OPC_clientTests , ClientWrite_N2)
{
    EXPECT_EQ(startClientFlag, false);

    EdgeMessage *msg = createEdgeMessage(endpointUri, 1, CMD_GET_ENDPOINTS);
    EXPECT_EQ(NULL != msg, true);

    EdgeResult res = getEndpointInfo(msg);
    EXPECT_EQ(res.code, STATUS_OK);

    EXPECT_EQ(startClientFlag, true);

    destroyEdgeMessage(msg);

    testWriteWithoutValueAlias(endpointUri);

    stop_client();
    EXPECT_EQ(startClientFlag, false);
}

TEST_F(OPC_clientTests , ClientWrite_N3)
{
    EXPECT_EQ(startClientFlag, false);

    EdgeMessage *msg = createEdgeMessage(endpointUri, 1, CMD_GET_ENDPOINTS);
    EXPECT_EQ(NULL != msg, true);

    EdgeResult res = getEndpointInfo(msg);
    EXPECT_EQ(res.code, STATUS_OK);

    EXPECT_EQ(startClientFlag, true);

    destroyEdgeMessage(msg);

    testWriteWithoutMessage();

    stop_client();
    EXPECT_EQ(startClientFlag, false);
}

TEST_F(OPC_clientTests , ClientWrite_N4)
{
    EXPECT_EQ(startClientFlag, false);

    EdgeMessage *msg = createEdgeMessage(endpointUri, 1, CMD_GET_ENDPOINTS);
    EXPECT_EQ(NULL != msg, true);

    EdgeResult res = getEndpointInfo(msg);
    EXPECT_EQ(res.code, STATUS_OK);

    EXPECT_EQ(startClientFlag, true);

    destroyEdgeMessage(msg);

    testWriteWithoutCommand();

    stop_client();
    EXPECT_EQ(startClientFlag, false);
}

TEST_F(OPC_clientTests , ClientBrowse_P)
{
    EXPECT_EQ(startClientFlag, false);

    EdgeMessage *msg = createEdgeMessage(endpointUri, 1, CMD_GET_ENDPOINTS);
    EXPECT_EQ(NULL != msg, true);

    EdgeResult res = getEndpointInfo(msg);
    EXPECT_EQ(res.code, STATUS_OK);

    EXPECT_EQ(startClientFlag, true);

    destroyEdgeMessage(msg);

    browseNode();

    stop_client();
    EXPECT_EQ(startClientFlag, false);
}

TEST_F(OPC_clientTests , ClientBrowseNext_P)
{
    EXPECT_EQ(startClientFlag, false);

    EdgeMessage *msg = createEdgeMessage(endpointUri, 1, CMD_GET_ENDPOINTS);
    EXPECT_EQ(NULL != msg, true);

    EdgeResult res = getEndpointInfo(msg);
    EXPECT_EQ(res.code, STATUS_OK);

    EXPECT_EQ(startClientFlag, true);

    destroyEdgeMessage(msg);

    browse_next();

    stop_client();
    EXPECT_EQ(startClientFlag, false);
}

TEST_F(OPC_clientTests , ClientBrowseGroup_P)
{
    EXPECT_EQ(startClientFlag, false);

    EdgeMessage *msg = createEdgeMessage(endpointUri, 1, CMD_GET_ENDPOINTS);
    EXPECT_EQ(NULL != msg, true);

    EdgeResult res = getEndpointInfo(msg);
    EXPECT_EQ(res.code, STATUS_OK);

    EXPECT_EQ(startClientFlag, true);

    destroyEdgeMessage(msg);

    browseNodes();

    stop_client();
    EXPECT_EQ(startClientFlag, false);
}

TEST_F(OPC_clientTests , ClientBrowseViews_P)
{
    EXPECT_EQ(startClientFlag, false);

    EdgeMessage *msg = createEdgeMessage(endpointUri, 1, CMD_GET_ENDPOINTS);
    EXPECT_EQ(NULL != msg, true);

    EdgeResult res = getEndpointInfo(msg);
    EXPECT_EQ(res.code, STATUS_OK);

    EXPECT_EQ(startClientFlag, true);

    destroyEdgeMessage(msg);

    browseViews();

    stop_client();
    EXPECT_EQ(startClientFlag, false);
}

TEST_F(OPC_clientTests , ClientBrowse_N1)
{
    EXPECT_EQ(startClientFlag, false);

    EdgeMessage *msg = createEdgeMessage(endpointUri, 1, CMD_GET_ENDPOINTS);
    EXPECT_EQ(NULL != msg, true);

    EdgeResult res = getEndpointInfo(msg);
    EXPECT_EQ(res.code, STATUS_OK);

    EXPECT_EQ(startClientFlag, true);

    destroyEdgeMessage(msg);

    browseNodeWithoutEndpoint();

    stop_client();
    EXPECT_EQ(startClientFlag, false);
}

TEST_F(OPC_clientTests , ClientBrowse_N2)
{
    EXPECT_EQ(startClientFlag, false);

    EdgeMessage *msg = createEdgeMessage(endpointUri, 1, CMD_GET_ENDPOINTS);
    EXPECT_EQ(NULL != msg, true);

    EdgeResult res = getEndpointInfo(msg);
    EXPECT_EQ(res.code, STATUS_OK);

    EXPECT_EQ(startClientFlag, true);

    destroyEdgeMessage(msg);

    browseNodeWithoutCommand();

    stop_client();
    EXPECT_EQ(startClientFlag, false);
}

TEST_F(OPC_clientTests , ClientBrowse_N3)
{
    EXPECT_EQ(startClientFlag, false);

    EdgeMessage *msg = createEdgeMessage(endpointUri, 1, CMD_GET_ENDPOINTS);
    EXPECT_EQ(NULL != msg, true);

    EdgeResult res = getEndpointInfo(msg);
    EXPECT_EQ(res.code, STATUS_OK);

    EXPECT_EQ(startClientFlag, true);

    destroyEdgeMessage(msg);

    browseNodeWithoutValueAlias();

    stop_client();
    EXPECT_EQ(startClientFlag, false);
}

TEST_F(OPC_clientTests , ClientBrowse_N4)
{
    EXPECT_EQ(startClientFlag, false);

    EdgeMessage *msg = createEdgeMessage(endpointUri, 1, CMD_GET_ENDPOINTS);
    EXPECT_EQ(NULL != msg, true);

    EdgeResult res = getEndpointInfo(msg);
    EXPECT_EQ(res.code, STATUS_OK);

    EXPECT_EQ(startClientFlag, true);

    destroyEdgeMessage(msg);

    browseNodeWithoutMessage();

    stop_client();
    EXPECT_EQ(startClientFlag, false);
}

TEST_F(OPC_clientTests , ClientBrowse_N5)
{
    EXPECT_EQ(startClientFlag, false);

    EdgeMessage *msg = createEdgeMessage(endpointUri, 1, CMD_GET_ENDPOINTS);
    EXPECT_EQ(NULL != msg, true);

    EdgeResult res = getEndpointInfo(msg);
    EXPECT_EQ(res.code, STATUS_OK);

    EXPECT_EQ(startClientFlag, true);

    destroyEdgeMessage(msg);

    browseNodeWithoutBrowseParam();

    stop_client();
    EXPECT_EQ(startClientFlag, false);
}

TEST_F(OPC_clientTests , ClientBrowse_N6)
{
    EXPECT_EQ(startClientFlag, false);

    EdgeMessage *msg = createEdgeMessage(endpointUri, 1, CMD_GET_ENDPOINTS);
    EXPECT_EQ(NULL != msg, true);

    EdgeResult res = getEndpointInfo(msg);
    EXPECT_EQ(res.code, STATUS_OK);

    EXPECT_EQ(startClientFlag, true);

    destroyEdgeMessage(msg);

    browseInvalidNode();

    stop_client();
    EXPECT_EQ(startClientFlag, false);
}

TEST_F(OPC_clientTests , ClientBrowse_N7)
{
    errorCallFlag = false;

    EdgeMessage msg;
    executeBrowse(NULL, &msg); // No client handle
    sleep(1);

    /* Wait some time and check whether error callback is received */
    EXPECT_EQ(errorCallFlag, true);
}

TEST_F(OPC_clientTests , ClientBrowse_N8)
{
    errorCallFlag = false;

    UA_Client *client;
    executeBrowse(client, NULL); // No msg
    sleep(1);

    /* Wait some time and check whether error callback is received */
    EXPECT_EQ(errorCallFlag, true);
}

TEST_F(OPC_clientTests , ClientBrowse_N9)
{
    errorCallFlag = false;

    UA_Client *client;
    EdgeMessage msg;
    msg.type = REPORT;
    executeBrowse(client, &msg); // Incorrect message type
    sleep(1);

    /* Wait some time and check whether error callback is received */
    EXPECT_EQ(errorCallFlag, true);
}

TEST_F(OPC_clientTests , ClientBrowse_N10)
{
    errorCallFlag = false;

    UA_Client *client;
    EdgeMessage msg;
    msg.type = SEND_REQUEST;
    msg.command = CMD_METHOD;
    executeBrowse(client, &msg); // Incorrect command
    sleep(1);

    /* Wait some time and check whether error callback is received */
    EXPECT_EQ(errorCallFlag, true);
}

TEST_F(OPC_clientTests , ClientMethodCall_P1)
{
    EXPECT_EQ(startClientFlag, false);

    EdgeMessage *msg = createEdgeMessage(endpointUri, 1, CMD_GET_ENDPOINTS);
    EXPECT_EQ(NULL != msg, true);
    EdgeResult res = getEndpointInfo(msg);
    EXPECT_EQ(res.code, STATUS_OK);
    EXPECT_EQ(startClientFlag, true);
    destroyEdgeMessage(msg);

    methodCallFlag = true;
    testMethod_P1(endpointUri);
    methodCallFlag = false;

    stop_client();
    EXPECT_EQ(startClientFlag, false);
}

TEST_F(OPC_clientTests , ClientMethodCall_P2)
{
    EXPECT_EQ(startClientFlag, false);

    EdgeMessage *msg = createEdgeMessage(endpointUri, 1, CMD_GET_ENDPOINTS);
    EXPECT_EQ(NULL != msg, true);
    EdgeResult res = getEndpointInfo(msg);
    EXPECT_EQ(res.code, STATUS_OK);
    EXPECT_EQ(startClientFlag, true);
    destroyEdgeMessage(msg);

    methodCallFlag = true;
    testMethod_P2(endpointUri);
    methodCallFlag = false;

    stop_client();
    EXPECT_EQ(startClientFlag, false);
}

TEST_F(OPC_clientTests , ClientMethodCall_P3)
{
    EXPECT_EQ(startClientFlag, false);

    EdgeMessage *msg = createEdgeMessage(endpointUri, 1, CMD_GET_ENDPOINTS);
    EXPECT_EQ(NULL != msg, true);
    EdgeResult res = getEndpointInfo(msg);
    EXPECT_EQ(res.code, STATUS_OK);
    EXPECT_EQ(startClientFlag, true);
    destroyEdgeMessage(msg);

    methodCallFlag = true;
    testMethod_P3(endpointUri);
    methodCallFlag = false;

    stop_client();
    EXPECT_EQ(startClientFlag, false);
}

TEST_F(OPC_clientTests , ClientMethodCall_P4)
{
    EXPECT_EQ(startClientFlag, false);

    EdgeMessage *msg = createEdgeMessage(endpointUri, 1, CMD_GET_ENDPOINTS);
    EXPECT_EQ(NULL != msg, true);
    EdgeResult res = getEndpointInfo(msg);
    EXPECT_EQ(res.code, STATUS_OK);
    EXPECT_EQ(startClientFlag, true);
    destroyEdgeMessage(msg);

    methodCallFlag = true;
    testMethod_P4(endpointUri);
    methodCallFlag = false;

    stop_client();
    EXPECT_EQ(startClientFlag, false);
}

TEST_F(OPC_clientTests , ClientMethodCall_N1)
{
    EXPECT_EQ(startClientFlag, false);

    EdgeMessage *msg = createEdgeMessage(endpointUri, 1, CMD_GET_ENDPOINTS);
    EXPECT_EQ(NULL != msg, true);
    EdgeResult res = getEndpointInfo(msg);
    EXPECT_EQ(res.code, STATUS_OK);
    EXPECT_EQ(startClientFlag, true);
    destroyEdgeMessage(msg);

    methodCallFlag = true;
    testMethodWithoutEndpoint();
    methodCallFlag = false;

    stop_client();
    EXPECT_EQ(startClientFlag, false);
}

TEST_F(OPC_clientTests , ClientMethodCall_N2)
{
    EXPECT_EQ(startClientFlag, false);

    EdgeMessage *msg = createEdgeMessage(endpointUri, 1, CMD_GET_ENDPOINTS);
    EXPECT_EQ(NULL != msg, true);
    EdgeResult res = getEndpointInfo(msg);
    EXPECT_EQ(res.code, STATUS_OK);
    EXPECT_EQ(startClientFlag, true);
    destroyEdgeMessage(msg);

    methodCallFlag = true;
    testMethodWithoutValueAlias(endpointUri);
    methodCallFlag = false;

    stop_client();
    EXPECT_EQ(startClientFlag, false);
}

TEST_F(OPC_clientTests , ClientMethodCall_N3)
{
    EXPECT_EQ(startClientFlag, false);

    EdgeMessage *msg = createEdgeMessage(endpointUri, 1, CMD_GET_ENDPOINTS);
    EXPECT_EQ(NULL != msg, true);
    EdgeResult res = getEndpointInfo(msg);
    EXPECT_EQ(res.code, STATUS_OK);
    EXPECT_EQ(startClientFlag, true);
    destroyEdgeMessage(msg);

    methodCallFlag = true;
    testMethodWithoutMessage();
    methodCallFlag = false;

    stop_client();
    EXPECT_EQ(startClientFlag, false);
}

TEST_F(OPC_clientTests , ClientMethodCall_N4)
{
    EXPECT_EQ(startClientFlag, false);

    EdgeMessage *msg = createEdgeMessage(endpointUri, 1, CMD_GET_ENDPOINTS);
    EXPECT_EQ(NULL != msg, true);
    EdgeResult res = getEndpointInfo(msg);
    EXPECT_EQ(res.code, STATUS_OK);
    EXPECT_EQ(startClientFlag, true);
    destroyEdgeMessage(msg);

    methodCallFlag = true;
    testMethodWithoutCommand();
    methodCallFlag = false;

    stop_client();
    EXPECT_EQ(startClientFlag, false);
}

TEST_F(OPC_clientTests , ClientMethodCall_N5)
{
    EXPECT_EQ(startClientFlag, false);

    EdgeMessage *msg = createEdgeMessage(endpointUri, 1, CMD_GET_ENDPOINTS);
    EXPECT_EQ(NULL != msg, true);
    EdgeResult res = getEndpointInfo(msg);
    EXPECT_EQ(res.code, STATUS_OK);
    EXPECT_EQ(startClientFlag, true);
    destroyEdgeMessage(msg);

    methodCallFlag = true;
    testMethodWithoutParam();
    methodCallFlag = false;

    stop_client();
    EXPECT_EQ(startClientFlag, false);
}

TEST_F(OPC_clientTests , ClientSubscribe_P1)
{
    EXPECT_EQ(startClientFlag, false);

    EdgeMessage *msg = createEdgeMessage(endpointUri, 1, CMD_GET_ENDPOINTS);
    EXPECT_EQ(NULL != msg, true);
    EdgeResult res = getEndpointInfo(msg);
    EXPECT_EQ(res.code, STATUS_OK);
    EXPECT_EQ(startClientFlag, true);
    destroyEdgeMessage(msg);

    testSubscription_P1(endpointUri);

    stop_client();
    EXPECT_EQ(startClientFlag, false);
}

TEST_F(OPC_clientTests , ClientSubscribe_P2)
{
    EXPECT_EQ(startClientFlag, false);

    EdgeMessage *msg = createEdgeMessage(endpointUri, 1, CMD_GET_ENDPOINTS);
    EXPECT_EQ(NULL != msg, true);
    EdgeResult res = getEndpointInfo(msg);
    EXPECT_EQ(res.code, STATUS_OK);
    EXPECT_EQ(startClientFlag, true);
    destroyEdgeMessage(msg);

    testSubscription_P2(endpointUri);

    stop_client();
    EXPECT_EQ(startClientFlag, false);
}

TEST_F(OPC_clientTests , ClientSubscribe_P3)
{
    EXPECT_EQ(startClientFlag, false);

    EdgeMessage *msg = createEdgeMessage(endpointUri, 1, CMD_GET_ENDPOINTS);
    EXPECT_EQ(NULL != msg, true);
    EdgeResult res = getEndpointInfo(msg);
    EXPECT_EQ(res.code, STATUS_OK);
    EXPECT_EQ(startClientFlag, true);
    destroyEdgeMessage(msg);

    testSubscription_P3(endpointUri);

    stop_client();
    EXPECT_EQ(startClientFlag, false);
}

TEST_F(OPC_clientTests , ClientSubscribe_N1)
{
    EXPECT_EQ(startClientFlag, false);

    EdgeMessage *msg = createEdgeMessage(endpointUri, 1, CMD_GET_ENDPOINTS);
    EXPECT_EQ(NULL != msg, true);
    EdgeResult res = getEndpointInfo(msg);
    EXPECT_EQ(res.code, STATUS_OK);
    EXPECT_EQ(startClientFlag, true);
    destroyEdgeMessage(msg);

    testSubscriptionWithoutEndpoint();

    stop_client();
    EXPECT_EQ(startClientFlag, false);
}

TEST_F(OPC_clientTests , ClientSubscribe_N2)
{
    EXPECT_EQ(startClientFlag, false);

    EdgeMessage *msg = createEdgeMessage(endpointUri, 1, CMD_GET_ENDPOINTS);
    EXPECT_EQ(NULL != msg, true);
    EdgeResult res = getEndpointInfo(msg);
    EXPECT_EQ(res.code, STATUS_OK);
    EXPECT_EQ(startClientFlag, true);
    destroyEdgeMessage(msg);

    testSubscriptionWithoutValueAlias(endpointUri);

    stop_client();
    EXPECT_EQ(startClientFlag, false);
}

TEST_F(OPC_clientTests , ClientSubscribe_N3)
{
    EXPECT_EQ(startClientFlag, false);

    EdgeMessage *msg = createEdgeMessage(endpointUri, 1, CMD_GET_ENDPOINTS);
    EXPECT_EQ(NULL != msg, true);
    EdgeResult res = getEndpointInfo(msg);
    EXPECT_EQ(res.code, STATUS_OK);
    EXPECT_EQ(startClientFlag, true);
    destroyEdgeMessage(msg);

    testSubscriptionWithoutMessage();

    stop_client();
    EXPECT_EQ(startClientFlag, false);
}

TEST_F(OPC_clientTests , ClientSubscribe_N4)
{
    EXPECT_EQ(startClientFlag, false);

    EdgeMessage *msg = createEdgeMessage(endpointUri, 1, CMD_GET_ENDPOINTS);
    EXPECT_EQ(NULL != msg, true);
    EdgeResult res = getEndpointInfo(msg);
    EXPECT_EQ(res.code, STATUS_OK);
    EXPECT_EQ(startClientFlag, true);
    destroyEdgeMessage(msg);

    testSubscriptionWithoutCommand(endpointUri);

    stop_client();
    EXPECT_EQ(startClientFlag, false);
}

TEST_F(OPC_clientTests , ClientSubscribe_N5)
{
    EXPECT_EQ(startClientFlag, false);

    EdgeMessage *msg = createEdgeMessage(endpointUri, 1, CMD_GET_ENDPOINTS);
    EXPECT_EQ(NULL != msg, true);
    EdgeResult res = getEndpointInfo(msg);
    EXPECT_EQ(res.code, STATUS_OK);
    EXPECT_EQ(startClientFlag, true);
    destroyEdgeMessage(msg);

    testSubscriptionWithoutSubReq(endpointUri);

    stop_client();
    EXPECT_EQ(startClientFlag, false);
}

TEST_F(OPC_clientTests , ClientShowNodeList_P)
{
    showNodeList();
}

TEST_F(OPC_clientTests , destroy_N)
{
    destroyEdgeResult(NULL);
    destroyEdgeEndpointConfig(NULL);
    destroyEdgeApplicationConfigMembers(NULL);
    destroyEdgeVersatility(NULL);
    destroyEdgeNodeId(NULL);
    destroyEdgeArgument(NULL);
    destroyEdgeMethodRequestParams(NULL);
    destroyEdgeNodeInfo(NULL);
    //destroyEdgeContinuationPoint(NULL);
    //destroyEdgeContinuationPointList(NULL);
    destroyEdgeEndpointInfo(NULL);
    destroyEdgeRequest(NULL);
    destroyEdgeResponse(NULL);
    destroyEdgeMessage(NULL);
}

int main(int argc, char **argv)
{
    ::testing::InitGoogleTest(&argc, argv);
    return RUN_ALL_TESTS();
}
