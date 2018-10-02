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
#include <signal.h>
#include <inttypes.h>

#ifndef _WIN32
#include <pthread.h>
#include <unistd.h>
#else
#include "pthread.h"
#endif
//#include <unistd.h>

#include "opcua_manager.h"
#include "opcua_common.h"
#include "edge_identifier.h"
#include "edge_malloc.h"
#include "sample_common.h"
#include "edge_opcua_common.h"

#define TAG "SAMPLE_SERVER"

#define IS_NULL(arg) ((arg == NULL) ? true : false)
#define IS_NOT_NULL(arg) ((arg != NULL) ? true : false)
#define VERIFY_NON_NULL_NR(arg) { if (!(arg)) { printf( \
             #arg " is NULL"); return; } }

#define COLOR_GREEN        "\x1b[32m"
#define COLOR_YELLOW      "\x1b[33m"
#define COLOR_PURPLE      "\x1b[35m"
#define COLOR_RESET         "\x1b[0m"

#define MAX_TEST_NUMBER 10000
#define SAMPLE_STRING_1 "test_1"
#define SAMPLE_STRING_2 "test_2"
#define DEFAULT_HOST_NAME "localhost"
#define MAX_ENDPOINT_URI_SIZE (512)
#define MAX_ADDRESS_SIZE (128)

#define DATETIME_USEC 10LL
#define DATETIME_MSEC (DATETIME_USEC * 1000LL)
#define DATETIME_SEC (DATETIME_MSEC * 1000LL)
#define DATETIME_UNIX_EPOCH (11644473600LL * DATETIME_SEC)

static bool startFlag = false;
static bool stopFlag = false;

static char ipAddress[MAX_ADDRESS_SIZE];
static char endpointUri[MAX_ENDPOINT_URI_SIZE];

static EdgeEndPointInfo *epInfo;
static EdgeConfigure *config = NULL;

static void testCreateNamespace();
static void testCreateNodes();

/***** Method Callbacks ******/
extern void test_method_shutdown(int inpSize, void **input, int outSize, void **output);
extern void test_method_print(int inpSize, void **input, int outSize, void **output);
extern void test_method_version(int inpSize, void **input, int outSize, void **output);
extern void test_method_sqrt(int inpSize, void **input, int outSize, void **output);
extern void test_method_increment_int32Array(int inpSize, void **input, int outSize, void **output);
void test_method_move(int inpSize, void **input, int outSize, void **output);

/* update node automatically */
static bool b_running = false;
static pthread_t m_serverThread;
static int robot_data_idx = 0;
static int rorot_pos_key = 256;

static int getRandom(int key)
{
    return (rand() % key) + 1;
}

static void *server_sample_loop(void *ptr)
{
    char s_value[MAX_ENDPOINT_URI_SIZE] = {0, };
    char dataNum[1];

    while (b_running)
    {
        sprintf(dataNum, "%d", getRandom(1000));
        strncpy(s_value, "robot data ", sizeof(s_value));
        strcat(s_value, dataNum);
        EdgeVersatility *message = (EdgeVersatility *) EdgeMalloc(sizeof(EdgeVersatility));
        if (IS_NOT_NULL(message))
        {
            message->value = (void *) s_value;
            message->isArray = false;
            modifyVariableNode(DEFAULT_NAMESPACE_VALUE, "accura2300s_xxx.xxx.xxx.xxx_Vab", message);
            EdgeFree(message);
        }
        else
        {
            printf("Error :: EdgeMalloc failed for EdgeVersatility in Test Modify Nodes\n");
        }

        message = (EdgeVersatility *) EdgeMalloc(sizeof(EdgeVersatility));
        if (IS_NOT_NULL(message))
        {
            int posArray[3];
            if (IS_NOT_NULL(posArray))
            {
                posArray[0] = getRandom(rorot_pos_key);
                posArray[1] = getRandom(rorot_pos_key);
                posArray[2] = getRandom(rorot_pos_key);
                message->value = (void *) posArray;
                message->isArray = true;
                message->arrayLength = 3;
                modifyVariableNode(DEFAULT_NAMESPACE_VALUE, "robot_position", message);
                //printf("(%d %d %d)\n", posArray[0], posArray[1], posArray[2]);
            }
            else
            {
                printf("Error :: EdgeMalloc failed for int Array in Test create Nodes\n");
            }
            EdgeFree(message);
        }
        else
        {
            printf("Error :: EdgeMalloc failed for EdgeVersatility in Test Modify Nodes\n");
        }

        #ifndef _WIN32
            usleep(25 *1000);
        #else
            Sleep(25);
        #endif
        robot_data_idx++;
    }
    return NULL;
}

static void status_network_cb(EdgeEndPointInfo *epInfo, EdgeStatusCode status)
{

}

/* status callbacks */
static void status_start_cb(EdgeEndPointInfo *epInfo, EdgeStatusCode status)
{
    if (status == STATUS_SERVER_STARTED)
    {
        printf(COLOR_GREEN "\n[Application Callback] Server started\n" COLOR_RESET);
        startFlag = true;

        testCreateNamespace();
        testCreateNodes();
    }
}

static void status_stop_cb(EdgeEndPointInfo *epInfo, EdgeStatusCode status)
{
    if (status == STATUS_STOP_SERVER)
    {
        printf(COLOR_GREEN "\n[Application Callback] Server stopped \n" COLOR_RESET);
        exit(0);
    }
}

static void init()
{
    config = (EdgeConfigure *) EdgeCalloc(1, sizeof(EdgeConfigure));
    VERIFY_NON_NULL_NR(config);

    config->statusCallback = (StatusCallback *) EdgeMalloc(sizeof(StatusCallback));
    if (IS_NULL(config->statusCallback))
    {
        printf("Error :: EdgeCalloc failed for config->statusCallback in init server\n");
        if (IS_NOT_NULL(config))
        {
            EdgeFree(config);
        }
        return;
    }
    config->statusCallback->start_cb = status_start_cb;
    config->statusCallback->stop_cb = status_stop_cb;
    config->statusCallback->network_cb = status_network_cb;

    config->discoveryCallback = (DiscoveryCallback *) EdgeMalloc(sizeof(DiscoveryCallback));
    if (IS_NULL(config->discoveryCallback))
    {
        printf("Error :: EdgeCalloc failed for config->discoveryCallback in init server\n");
        if (IS_NOT_NULL(config))
        {
            EdgeFree(config->statusCallback);
            EdgeFree(config);
        }
        return;
    }

    configure(config);
}

static void startServer(unsigned int port)
{
    EdgeEndpointConfig *endpointConfig = (EdgeEndpointConfig *) EdgeCalloc(1,
            sizeof(EdgeEndpointConfig));
    VERIFY_NON_NULL_NR(endpointConfig);
    endpointConfig->bindAddress = ipAddress;
    endpointConfig->bindPort = port;
    endpointConfig->serverName = (char *) DEFAULT_SERVER_NAME_VALUE;
    //endpointConfig->requestTimeout = 60000;

    printf(COLOR_GREEN "[Endpoint Configuration]\n" COLOR_RESET);
    printf("\nAddress : %s", endpointConfig->bindAddress);
    printf("\nPort : %u\n", endpointConfig->bindPort);

    EdgeApplicationConfig *appConfig = (EdgeApplicationConfig *) EdgeCalloc(1,
            sizeof(EdgeApplicationConfig));
    if (IS_NULL(appConfig))
    {
        printf("Error :: EdgeCalloc failed for appConfig in start server\n");
        EdgeFree(endpointConfig);
        return;
    }
    appConfig->applicationName = (char *) DEFAULT_SERVER_APP_NAME_VALUE;
    appConfig->applicationUri = (char *) DEFAULT_SERVER_APP_URI_VALUE;
    appConfig->productUri = (char *) DEFAULT_PRODUCT_URI_VALUE;
    appConfig->applicationType = EDGE_APPLICATIONTYPE_SERVER;

    printf(COLOR_GREEN "\n[Application Configuration]\n" COLOR_RESET);
    printf("\nApplication Name : %s", appConfig->applicationName);
    printf("\nApplication Uri : %s", appConfig->applicationUri);
    printf("\nProudct Uri  : %s\n", appConfig->productUri);

    epInfo->endpointConfig = endpointConfig;
    epInfo->appConfig = appConfig;
    epInfo->securityPolicyUri = NULL;

    EdgeResult ret = createServer(epInfo);
    if (STATUS_OK != ret.code) {
        printf("Start has failed\n");
    }
}

static void stopServer()
{
    EdgeFree(epInfo->endpointConfig);
    EdgeFree(epInfo->appConfig);

    closeServer(epInfo);
}

static void testCreateNamespace()
{
    printf("\n" COLOR_PURPLE "=================== Creating namespace ================" COLOR_RESET
    "\n");
    printf(COLOR_GREEN "\n[Create Namespace]" COLOR_RESET);
    printf(": %s\n", DEFAULT_NAMESPACE_VALUE);
    createNamespace(DEFAULT_NAMESPACE_VALUE,
    DEFAULT_ROOT_NODE_INFO_VALUE,
    DEFAULT_ROOT_NODE_INFO_VALUE,
    DEFAULT_ROOT_NODE_INFO_VALUE);
}

static void testCreateNodes()
{
    printf(
            "\n" COLOR_PURPLE "==================== Creating nodes ===================" COLOR_RESET "\n");
    int index = 0;

    printf(COLOR_GREEN"\n[Create Variable Node]\n"COLOR_RESET);
    printf("\n[%d] Variable node with string variant\n", ++index);

    EdgeNodeItem* item = NULL;
    item = createVariableNodeItem("accura2300s_xxx.xxx.xxx.xxx_Vab", EDGE_NODEID_STRING, "test1", VARIABLE_NODE, 100);
    VERIFY_NON_NULL_NR(item);
    createNode(DEFAULT_NAMESPACE_VALUE, item);
    printf("\n|------------[Added]  %s\n", item->browseName);
    deleteNodeItem(item);

    item = createVariableNodeItem("String2", EDGE_NODEID_STRING, "test2", VARIABLE_NODE, 100);
    VERIFY_NON_NULL_NR(item);
    createNode(DEFAULT_NAMESPACE_VALUE, item);
    printf("\n|------------[Added] %s\n", item->browseName);
    deleteNodeItem(item);

    item = createVariableNodeItem("String3", EDGE_NODEID_STRING, "test3", VARIABLE_NODE, 100);
    VERIFY_NON_NULL_NR(item);
    createNode(DEFAULT_NAMESPACE_VALUE, item);
    printf("\n|------------[Added] %s\n", item->browseName);
    deleteNodeItem(item);

    VERIFY_NON_NULL_NR(item);
    printf("\n[%d] Variable node with XML ELEMENT variant: \n", ++index);
    Edge_XmlElement *xml_value = (Edge_XmlElement *) EdgeMalloc(sizeof(Edge_XmlElement));
    if (IS_NOT_NULL(xml_value))
    {
        xml_value->length = 2;
        xml_value->data = (Edge_Byte *) "ab";
        item = createVariableNodeItem("xml_value", EDGE_NODEID_XMLELEMENT, (void *) xml_value, VARIABLE_NODE, 100);
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
    char *locale = "COUNTRY", *text = "INDIA";
    Edge_LocalizedText lt_value = {
        { strlen(locale), (uint8_t *)locale },
        { strlen(text), (uint8_t *)text }
    };
    item = createVariableNodeItem("LocalizedText", EDGE_NODEID_LOCALIZEDTEXT, (void *) &lt_value,
            VARIABLE_NODE, 100);
    VERIFY_NON_NULL_NR(item);
    createNode(DEFAULT_NAMESPACE_VALUE, item);
    printf("\n|------------[Added] %s\n", item->browseName);
    deleteNodeItem(item);

    printf("\n[%d] Variable node with byte string variant: \n", ++index);
    item = createVariableNodeItem("ByteString", EDGE_NODEID_BYTESTRING, (void *) "samsung", VARIABLE_NODE, 100);
    VERIFY_NON_NULL_NR(item);
    createNode(DEFAULT_NAMESPACE_VALUE, item);
    printf("\n|------------[Added] %s\n", item->browseName);
    deleteNodeItem(item);

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
    uint16_t uint16_val = 30;
    item = createVariableNodeItem("UInt16", EDGE_NODEID_UINT16, (void *) &uint16_val, VARIABLE_NODE, 100);
    VERIFY_NON_NULL_NR(item);
    createNode(DEFAULT_NAMESPACE_VALUE, item);
    printf("\n|------------[Added] %s\n", item->browseName);
    deleteNodeItem(item);

    printf("\n[%d] Variable node with UInt32 variant: \n", ++index);
    uint32_t uint32_val = 444;
    item = createVariableNodeItem("UInt32", EDGE_NODEID_UINT32, (void *) &uint32_val, VARIABLE_NODE, 100);
    VERIFY_NON_NULL_NR(item);
    createNode(DEFAULT_NAMESPACE_VALUE, item);
    printf("\n|------------[Added] %s\n", item->browseName);
    deleteNodeItem(item);

    printf("\n[%d] Variable node with UInt64 variant: \n", ++index);
    uint64_t uint64_val = 3445516;
    item = createVariableNodeItem("UInt64", EDGE_NODEID_UINT64, (void *) &uint64_val, VARIABLE_NODE, 100);
    VERIFY_NON_NULL_NR(item);
    createNode(DEFAULT_NAMESPACE_VALUE, item);
    printf("\n|------------[Added] %s\n", item->browseName);
    deleteNodeItem(item);

    printf("\n[%d] Variable node with Int16 variant: \n", ++index);
    int16_t int16_val = 4;
    item = createVariableNodeItem("Int16", EDGE_NODEID_INT16, (void *) &int16_val, VARIABLE_NODE, 100);
    VERIFY_NON_NULL_NR(item);
    createNode(DEFAULT_NAMESPACE_VALUE, item);
    printf("\n|------------[Added] %s\n", item->browseName);
    deleteNodeItem(item);

    printf("\n[%d] Variable node with Int32 variant: \n", ++index);
    int32_t int32_val = 40;
    item = createVariableNodeItem("Int32", EDGE_NODEID_INT32, (void *) &int32_val, VARIABLE_NODE, 100);
    VERIFY_NON_NULL_NR(item);
    createNode(DEFAULT_NAMESPACE_VALUE, item);
    printf("\n|------------[Added] %s\n", item->browseName);
    deleteNodeItem(item);

    printf("\n[%d] Variable node with Int64 variant: \n", ++index);
    int64_t int64_val = 32700;
    item = createVariableNodeItem("Int64", EDGE_NODEID_INT64, (void *) &int64_val, VARIABLE_NODE, 100);
    VERIFY_NON_NULL_NR(item);
    createNode(DEFAULT_NAMESPACE_VALUE, item);
    printf("\n|------------[Added] %s\n", item->browseName);
    deleteNodeItem(item);

    printf("\n[%d] Variable node with UInt32 variant: \n", ++index);
    int32_val = 4456;
    item = createVariableNodeItem("UInt32writeonly", EDGE_NODEID_UINT32, (void *) &int32_val, VARIABLE_NODE, 100);
    VERIFY_NON_NULL_NR(item);
    item->accessLevel = WRITE;
    item->userAccessLevel = WRITE;
    createNode(DEFAULT_NAMESPACE_VALUE, item);
    printf("\n|------------[Added] %s\n", item->browseName);
    deleteNodeItem(item);

    printf("\n[%d] Variable node with UInt64 variant: \n", ++index);
    int64_val = 3270000;
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
    struct timeval tv;
    #ifndef _WIN32
        gettimeofday(&tv, NULL);
    #else
        getTimeofDay(&tv, NULL);
    #endif
    Edge_DateTime time = (tv.tv_sec * DATETIME_SEC) + (tv.tv_usec * DATETIME_USEC) + DATETIME_UNIX_EPOCH;
    item = createVariableNodeItem("DateTime", EDGE_NODEID_DATETIME, (void *) &time, VARIABLE_NODE, 100);
    VERIFY_NON_NULL_NR(item);
    createNode(DEFAULT_NAMESPACE_VALUE, item);
    printf("\n|------------[Added] %s [Value: %" PRId64 "]\n", item->browseName, time);
    deleteNodeItem(item);

    printf("\n[%d] Variable node with SByte variant: \n", ++index);
    Edge_SByte sbyte = 2;
    item = createVariableNodeItem("SByte", EDGE_NODEID_SBYTE, (void *) &sbyte, VARIABLE_NODE, 100);
    VERIFY_NON_NULL_NR(item);
    createNode(DEFAULT_NAMESPACE_VALUE, item);
    printf("\n|------------[Added] %s\n", item->browseName);
    deleteNodeItem(item);

    printf("\n[%d] Variable node with GUID variant: \n", ++index);
    Edge_Guid guid =
    { 1, 0, 1,
    { 0, 0, 0, 0, 1, 1, 1, 1 } };
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

    printf("\n[%d] Variable node with NODEID variant: \n", ++index);
    Edge_NodeId node;
    node.namespaceIndex = DEFAULT_NAMESPACE_INDEX;
    node.identifierType = EDGE_INTEGER;
    node.identifier.numeric = EDGE_NODEID_ROOTFOLDER;

    item = createVariableNodeItem("NodeId", EDGE_NODEID_NODEID, &node, VARIABLE_NODE, 100);
    VERIFY_NON_NULL_NR(item);
    createNode(DEFAULT_NAMESPACE_VALUE, item);
    printf("\n|------------[Added] %s\n", item->browseName);
    deleteNodeItem(item);

    /******************* Array *********************/
    printf(COLOR_GREEN "\n[Create Array Node]\n" COLOR_RESET);
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
            deleteNodeItem(item);
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

    printf("\n[%d] Array node with SByte values: \n", ++index);
    Edge_SByte *sbData = (Edge_SByte *) EdgeMalloc(sizeof(Edge_SByte) * 5);
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
        item = createVariableNodeItem("int32Array", EDGE_NODEID_INT32, (void *) intData, VARIABLE_NODE, 100);
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
    int64_t *int64Data = (int64_t *) EdgeMalloc(sizeof(int64_t) * 5);
    if (IS_NOT_NULL(int64Data))
    {
        int64Data[0] = 11111;
        int64Data[1] = 22222;
        int64Data[2] = 33333;
        int64Data[3] = 44444;
        int64Data[4] = 55555;
        item = createVariableNodeItem("int64Array", EDGE_NODEID_INT64, (void *) int64Data, VARIABLE_NODE, 100);
        VERIFY_NON_NULL_NR(item);
        item->nodeType = ARRAY_NODE;
        item->arrayLength = 5;
        createNode(DEFAULT_NAMESPACE_VALUE, item);
        printf("\n|------------[Added] %s\n", item->browseName);
        EdgeFree(int64Data);
        deleteNodeItem(item);
    }
    else
    {
        printf("Error :: EdgeMalloc failed for int64Data Array in Test create Nodes\n");
    }

    printf("\n[%d] Array node with UInt16 values: \n", ++index);
    uint16_t uInt16Arr[] = {0, 100, 10000, 50000, 65535};
    item = createVariableNodeItem("UInt16Array", EDGE_NODEID_UINT16, (void *) uInt16Arr, VARIABLE_NODE, 100);
    VERIFY_NON_NULL_NR(item);
    item->nodeType = ARRAY_NODE;
    item->arrayLength = 5;
    createNode(DEFAULT_NAMESPACE_VALUE, item);
    printf("\n|------------[Added] %s\n", item->browseName);
    deleteNodeItem(item);

    printf("\n[%d] Array node with Int16 values: \n", ++index);
    int16_t int16Arr[] = {-32768, -10000, 0, 10000, 32767};
    item = createVariableNodeItem("Int16Array", EDGE_NODEID_INT16, (void *) int16Arr, VARIABLE_NODE, 100);
    VERIFY_NON_NULL_NR(item);
    item->nodeType = ARRAY_NODE;
    item->arrayLength = 5;
    createNode(DEFAULT_NAMESPACE_VALUE, item);
    printf("\n|------------[Added] %s\n", item->browseName);
    deleteNodeItem(item);

    printf("\n[%d] Array node with UInt32 values: \n", ++index);
    uint32_t uInt32Arr[] = {0, 100, 10000, 1000000, 100000000};
    item = createVariableNodeItem("UInt32Array", EDGE_NODEID_UINT32, (void *) uInt32Arr, VARIABLE_NODE, 100);
    VERIFY_NON_NULL_NR(item);
    item->nodeType = ARRAY_NODE;
    item->arrayLength = 5;
    createNode(DEFAULT_NAMESPACE_VALUE, item);
    printf("\n|------------[Added] %s\n", item->browseName);
    deleteNodeItem(item);

    printf("\n[%d] Array node with UInt64 values: \n", ++index);
    uint64_t uInt64Arr[] = {0, 10000, 1000000, 100000000, 10000000000};
    item = createVariableNodeItem("UInt64Array", EDGE_NODEID_UINT64, (void *) uInt64Arr, VARIABLE_NODE, 100);
    VERIFY_NON_NULL_NR(item);
    item->nodeType = ARRAY_NODE;
    item->arrayLength = 5;
    createNode(DEFAULT_NAMESPACE_VALUE, item);
    printf("\n|------------[Added] %s\n", item->browseName);
    deleteNodeItem(item);

    printf("\n[%d] Array node with float values: \n", ++index);
    float floatArr[] = {-111.11, 0, 111.11, 222.22, 333.33};
    item = createVariableNodeItem("FloatArray", EDGE_NODEID_FLOAT, (void *) floatArr, VARIABLE_NODE, 100);
    VERIFY_NON_NULL_NR(item);
    item->nodeType = ARRAY_NODE;
    item->arrayLength = 5;
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
    Edge_Byte *b_arrvalue = (Edge_Byte *) EdgeCalloc(1, sizeof(Edge_Byte) * 5);
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

    printf("\n[%d] Variable node with guid array variant: \n", ++index);
    Edge_Guid guidArr[5] = {
        { 1, 0, 1, { 0, 0, 0, 0, 1, 1, 1, 1 } },
        { 2, 0, 2, { 0, 0, 0, 0, 2, 2, 2, 2 } },
        { 3, 3, 3, { 0, 3, 0, 3, 3, 3, 3, 3 } },
        { 4, 4, 4, { 0, 4, 4, 4, 4, 0, 0, 0 } },
        { 0, 0, 5, { 5, 0, 0, 5, 0, 0, 5, 0 } }
    };
    item = createVariableNodeItem("GuidArray", EDGE_NODEID_GUID, (void *) guidArr, VARIABLE_NODE, 100);
    VERIFY_NON_NULL_NR(item);
    item->nodeType = ARRAY_NODE;
    item->arrayLength = 5;
    createNode(DEFAULT_NAMESPACE_VALUE, item);
    printf("\n|------------[Added] %s\n", item->browseName);
    deleteNodeItem(item);

    printf("\n[%d] Array node with dateTime variant: \n", ++index);
    Edge_DateTime time_now = 131653195862335600; // Some random value
    Edge_DateTime timeArr[5] = {
        time_now,
        time_now-100000,
        time_now/4,
        time_now/2,
        time_now+10
    };
    item = createVariableNodeItem("DateTimeArray", EDGE_NODEID_DATETIME, (void *) timeArr, VARIABLE_NODE, 100);
    VERIFY_NON_NULL_NR(item);
    item->nodeType = ARRAY_NODE;
    item->arrayLength = 5;
    createNode(DEFAULT_NAMESPACE_VALUE, item);
    printf("\n|------------[Added] %s\n", item->browseName);
    deleteNodeItem(item);

    printf("\n[%d] Array node with XML ELEMENT variant: \n", ++index);
    Edge_XmlElement xmlValueArr[6] = {
        {2, (uint8_t *)"ab"},
        {3, (uint8_t *)"abc"},
        {2, (uint8_t *)"st"},
        {3, (uint8_t *)"mno"},
        {2, (uint8_t *)"cd"},
        {3, (uint8_t *)"xyz"}
    };
    item = createVariableNodeItem("XmlElementArray", EDGE_NODEID_XMLELEMENT, (void *) xmlValueArr, VARIABLE_NODE, 100);
    VERIFY_NON_NULL_NR(item);
    item->nodeType = ARRAY_NODE;
    item->arrayLength = 6;
    createNode(DEFAULT_NAMESPACE_VALUE, item);
    printf("\n|------------[Added] %s\n", item->browseName);
    deleteNodeItem(item);

    printf("\n[%d] Array node with NODEID variant: \n", ++index);
    Edge_NodeId nodeArr[5];
    for(int index  = 0; index < 5; index++)
    {
        nodeArr[index].namespaceIndex = DEFAULT_NAMESPACE_INDEX;
        if(index %2 == 0)
        {
            nodeArr[index].identifierType = EDGE_INTEGER;
            nodeArr[index].identifier.numeric = EDGE_NODEID_ROOTFOLDER;
        }
        else
        {
            nodeArr[index].identifierType = EDGE_STRING;
            nodeArr[index].identifier.string = EdgeStringAlloc("StringNodeId");
        }
    }
    item = createVariableNodeItem("NodeIdArray", EDGE_NODEID_NODEID, nodeArr, VARIABLE_NODE, 100);
    VERIFY_NON_NULL_NR(item);
    item->nodeType = ARRAY_NODE;
    item->arrayLength = 5;
    createNode(DEFAULT_NAMESPACE_VALUE, item);
    printf("\n|------------[Added] %s\n", item->browseName);
    deleteNodeItem(item);

    printf("\n[%d] Array node with qualified name variant: \n", ++index);
    Edge_QualifiedName qnValueArr[5] = {
        {2, {5, (uint8_t *)"qn100"} },
        {2, {6, (uint8_t *)"qn1000"} },
        {2, {6, (uint8_t *)"qn5000"} },
        {2, {7, (uint8_t *)"qn10000"} },
        {2, {7, (uint8_t *)"qn50000"} }
    };
    item = createVariableNodeItem("QualifiedNameArray", EDGE_NODEID_QUALIFIEDNAME, (void *) qnValueArr,
            VARIABLE_NODE, 100);
    VERIFY_NON_NULL_NR(item);
    item->nodeType = ARRAY_NODE;
    item->arrayLength = 5;
    createNode(DEFAULT_NAMESPACE_VALUE, item);
    printf("\n|------------[Added] %s\n", item->browseName);
    deleteNodeItem(item);

    printf("\n[%d] Variable node with localized text variant: \n", ++index);
    Edge_LocalizedText ltValueArr[7] = {
        { {7, (uint8_t *)"localeA"}, {5, (uint8_t *)"textA"} },
        { {7, (uint8_t *)"localeB"}, {5, (uint8_t *)"textB"} },
        { {7, (uint8_t *)"localeC"}, {5, (uint8_t *)"textC"} },
        { {7, (uint8_t *)"localeD"}, {5, (uint8_t *)"textD"} },
        { {7, (uint8_t *)"localeE"}, {5, (uint8_t *)"textE"} },
        { {7, (uint8_t *)"localeF"}, {5, (uint8_t *)"textF"} },
        { {7, (uint8_t *)"localeG"}, {5, (uint8_t *)"textG"} }
    };
    item = createVariableNodeItem("LocalizedTextArray", EDGE_NODEID_LOCALIZEDTEXT, (void *) ltValueArr,
            VARIABLE_NODE, 100);
    item->nodeType = ARRAY_NODE;
    item->arrayLength = 7;
    VERIFY_NON_NULL_NR(item);
    createNode(DEFAULT_NAMESPACE_VALUE, item);
    printf("\n|------------[Added] %s\n", item->browseName);
    deleteNodeItem(item);

    /******************* Object Node *********************/
    printf(COLOR_GREEN"\n[Create Object Node]\n"COLOR_RESET);
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
    printf(COLOR_GREEN"\n[Create View Node]\n"COLOR_RESET);
    printf("\n[%d] View Node with GTC_robot\n", ++index);
    edgeNodeId = (EdgeNodeId *) EdgeCalloc(1, sizeof(EdgeNodeId));
    if (IS_NOT_NULL(edgeNodeId))
    {
        item = createNodeItem("GTC_robot", VIEW_NODE, edgeNodeId);
        VERIFY_NON_NULL_NR(item);
        createNode(DEFAULT_NAMESPACE_VALUE, item);
        printf("\n|------------[Added] %s\n", item->browseName);
        EdgeFree(edgeNodeId);
        deleteNodeItem(item);
    }
    else
    {
        printf("Error :: EdgeMalloc failed for GTC_robot in Test create Nodes\n");
    }

    printf("\n[%d] View Node with property (ViewNode)\n", ++index);
    edgeNodeId = (EdgeNodeId *) EdgeMalloc(sizeof(EdgeNodeId));
    if (IS_NOT_NULL(edgeNodeId))
    {
        edgeNodeId->nodeId = "GTC_robot";
        item = createNodeItem("property", VIEW_NODE, edgeNodeId);
        VERIFY_NON_NULL_NR(item);
        createNode(DEFAULT_NAMESPACE_VALUE, item);
        printf("\n|------------[Added] %s\n", item->browseName);
        EdgeFree(edgeNodeId);
        deleteNodeItem(item);
    }
    else
    {
        printf("Error :: EdgeMalloc failed for property (ViewNode) in Test create Nodes\n");
    }

    /******************* Object Type Node *********************/
    printf(COLOR_GREEN"\n[Create Object Type Node]\n"COLOR_RESET);
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
    printf(COLOR_GREEN"\n[Create Variable Type Node]\n"COLOR_RESET);
    printf("\n[%d] Variable Type Node with Double Variable Type \n", ++index);
    double d[2] = { 10.2, 20.2 };
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
        deleteNodeItem(item);
    }
    else
    {
        printf("Error :: EdgeMalloc failed for DataType2 in Test create Nodes\n");
    }

    /******************* Reference Type Node *********************/
    printf(COLOR_GREEN"\n[Create Reference Type Node]\n"COLOR_RESET);
    printf("\n[%d] Reference Type Node with ReferenceTypeNode1", ++index);
    edgeNodeId = (EdgeNodeId *) EdgeCalloc(1, sizeof(EdgeNodeId));
    if (IS_NOT_NULL(edgeNodeId))
    {
        item = createNodeItem("ReferenceTypeNode1", REFERENCE_TYPE_NODE, edgeNodeId);
        VERIFY_NON_NULL_NR(item);
        createNode(DEFAULT_NAMESPACE_VALUE, item);
        printf("\n|------------[Added] %s\n", item->browseName);
        EdgeFree(edgeNodeId);
        deleteNodeItem(item);
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
        deleteNodeItem(item);
    }
    else
    {
        printf("Error :: EdgeMalloc failed for ReferenceTypeNode2 in Test create Nodes\n");
    }

    /******************* Method Node *********************/
    printf(COLOR_GREEN"\n[Create Method Node]\n"COLOR_RESET);
    printf("\n[%d] Method Node with square_root \n", ++index);
    EdgeNodeItem *methodNodeItem = (EdgeNodeItem *) EdgeCalloc(1, sizeof(EdgeNodeItem));
    VERIFY_NON_NULL_NR(methodNodeItem);
    methodNodeItem->browseName = "sqrt(x)";
    methodNodeItem->sourceNodeId = NULL;

    EdgeMethod *method = (EdgeMethod *) EdgeMalloc(sizeof(EdgeMethod));
    if (IS_NULL(method))
    {
        EdgeFree(methodNodeItem);
        EdgeFree(method);
        printf("Error :: EdgeMalloc failed for method square_root  in Test create Nodes\n");
        return;
    }
    method->description = "Calculate square root";
    method->methodNodeName = "square_root";
    method->method_fn = test_method_sqrt;
    method->num_inpArgs = 1;
    method->inpArg = (EdgeArgument **) EdgeCalloc(method->num_inpArgs, sizeof(EdgeArgument *));
    if (IS_NULL(method->inpArg))
    {
        EdgeFree(methodNodeItem);
        EdgeFree(method);
        printf("Error :: EdgeMalloc failed for method method->inpArg  in Test create Nodes\n");
        return;
    }
    for (int idx = 0; idx < method->num_inpArgs; idx++)
    {
        method->inpArg[idx] = (EdgeArgument *) EdgeMalloc(sizeof(EdgeArgument));
        if (IS_NULL(method->inpArg[idx]))
        {
            for (int j = 0; j < idx; j++)
            {
                if (IS_NOT_NULL(method->inpArg[j]))
                {
                    EdgeFree(method->inpArg[j]);
                }
            }
            EdgeFree(method->inpArg);
            EdgeFree(methodNodeItem);
            EdgeFree(method);
            printf("Error :: EdgeMalloc failed for method method->inpArg[%d]  in Test create Nodes\n",
                    idx);
            return;
        }
        method->inpArg[idx]->argType = EDGE_NODEID_DOUBLE;
        method->inpArg[idx]->valType = SCALAR;
    }

    method->num_outArgs = 1;
    method->outArg = (EdgeArgument **) malloc(sizeof(EdgeArgument *) * method->num_outArgs);
    if (IS_NULL(method->outArg))
    {
        for (int idx = 0; idx < method->num_inpArgs; idx++)
        {
            EdgeFree(method->inpArg[idx]);
        }
        EdgeFree(method->inpArg);
        EdgeFree(methodNodeItem);
        EdgeFree(method);
        printf("Error :: EdgeMalloc failed for method method->outArg  in Test create Nodes\n");
        return;
    }
    for (int idx = 0; idx < method->num_outArgs; idx++)
    {
        method->outArg[idx] = (EdgeArgument *) EdgeMalloc(sizeof(EdgeArgument));
        if (IS_NULL(method->outArg[idx]))
        {
            for (int j = 0; j < idx; j++)
            {
                if (IS_NOT_NULL(method->outArg[j]))
                {
                    EdgeFree(method->outArg[j]);
                }
            }
            EdgeFree(method->outArg);

            for (int inpIdx = 0; inpIdx < method->num_inpArgs; inpIdx++)
            {
                EdgeFree(method->inpArg[idx]);
            }
            EdgeFree(method->inpArg);

            EdgeFree(methodNodeItem);
            EdgeFree(method);
            printf("Error :: EdgeMalloc failed for method method->outArg[%d]  in Test create Nodes\n",
                    idx);
            return;
        }
        method->outArg[idx]->argType = EDGE_NODEID_DOUBLE;
        method->outArg[idx]->valType = SCALAR;
    }
    createMethodNode(DEFAULT_NAMESPACE_VALUE, methodNodeItem, method);
    printf("\n|------------[Added] %s\n", methodNodeItem->browseName);
    EdgeFree(methodNodeItem);

    printf("\n[%d] Method Node with incrementInc32Array \n", ++index);
    EdgeNodeItem *methodNodeItem1 = (EdgeNodeItem *) EdgeCalloc(1, sizeof(EdgeNodeItem));
    VERIFY_NON_NULL_NR(methodNodeItem1);
    methodNodeItem1->browseName = "incrementInc32Array(x,delta)";
    methodNodeItem1->sourceNodeId = NULL;

    EdgeMethod *method1 = (EdgeMethod *) EdgeMalloc(sizeof(EdgeMethod));
    if (IS_NULL(method1))
    {
        EdgeFree(methodNodeItem1);
        EdgeFree(method1);
        printf("Error :: EdgeMalloc failed for method incrementInc32Array  in Test create Nodes\n");
        return;
    }
    method1->description = "Increment int32 array by delta";
    method1->methodNodeName = "incrementInc32Array";
    method1->method_fn = test_method_increment_int32Array;

    method1->num_inpArgs = 2;
    method1->inpArg = (EdgeArgument **) malloc(sizeof(EdgeArgument *) * method1->num_inpArgs);
    if (IS_NULL(method1->inpArg))
    {
        EdgeFree(methodNodeItem1);
        EdgeFree(method1);
        printf("Error :: EdgeMalloc failed for methodmethod1->inpArg  in Test create Nodes\n");
        return;
    }
    method1->inpArg[0] = (EdgeArgument *) EdgeMalloc(sizeof(EdgeArgument));
    if (IS_NULL(method1->inpArg[0]))
    {
        EdgeFree(method1->inpArg);
        EdgeFree(methodNodeItem1);
        EdgeFree(method1);
        printf("Error :: EdgeMalloc failed for method method1->inpArg[0]  in Test create Nodes\n");
        return;
    }
    method1->inpArg[0]->argType = EDGE_NODEID_INT32;
    method1->inpArg[0]->valType = ARRAY_1D;
    method1->inpArg[0]->arrayLength = 5;

    method1->inpArg[1] = (EdgeArgument *) EdgeMalloc(sizeof(EdgeArgument));
    if (IS_NULL(method1->inpArg[1]))
    {
        EdgeFree(method1->inpArg[0]);
        EdgeFree(method1->inpArg);
        EdgeFree(methodNodeItem1);
        EdgeFree(method1);
        printf("Error :: EdgeMalloc failed for method method1->inpArg[1]  in Test create Nodes\n");
        return;
    }
    method1->inpArg[1]->argType = EDGE_NODEID_INT32;
    method1->inpArg[1]->valType = SCALAR;

    method1->num_outArgs = 1;
    method1->outArg = (EdgeArgument **) malloc(sizeof(EdgeArgument *) * method1->num_outArgs);
    if (IS_NULL(method1->outArg))
    {
        EdgeFree(method1->inpArg[0]);
        EdgeFree(method1->inpArg[1]);
        EdgeFree(method1->inpArg);
        EdgeFree(methodNodeItem1);
        EdgeFree(method1);
        printf("Error :: EdgeMalloc failed for method1->outArg  in Test create Nodes\n");
        return;
    }
    for (int idx = 0; idx < method1->num_outArgs; idx++)
    {
        method1->outArg[idx] = (EdgeArgument *) EdgeMalloc(sizeof(EdgeArgument));
        if (IS_NULL(method1->outArg[idx]))
        {
            for (int j = 0; j < idx; j++)
            {
                if (IS_NOT_NULL(method->outArg[j]))
                {
                    EdgeFree(method->outArg[j]);
                }
            }

            EdgeFree(method1->inpArg[0]);
            EdgeFree(method1->inpArg[1]);
            EdgeFree(method1->inpArg);
            EdgeFree(method1->outArg);
            EdgeFree(methodNodeItem1);
            EdgeFree(method1);
            printf("Error :: EdgeMalloc failed for method method1->outArg[%d]  in Test create Nodes\n",
                    idx);
            return;
        }
        method1->outArg[idx]->argType = EDGE_NODEID_INT32;
        method1->outArg[idx]->valType = ARRAY_1D;
        method1->outArg[idx]->arrayLength = 5;
    }
    createMethodNode(DEFAULT_NAMESPACE_VALUE, methodNodeItem1, method1);
    printf("\n|------------[Added] %s\n", methodNodeItem1->browseName);
    EdgeFree(methodNodeItem1);

    printf("\n[%d] Method Node with noArgMethod \n", ++index);
    EdgeNodeItem *methodNodeItem2 = (EdgeNodeItem *) EdgeCalloc(1, sizeof(EdgeNodeItem));
    VERIFY_NON_NULL_NR(methodNodeItem2);
    methodNodeItem2->browseName = "shutdown()";
    methodNodeItem2->sourceNodeId = NULL;

    EdgeMethod *method2 = (EdgeMethod *) EdgeMalloc(sizeof(EdgeMethod));
    if (IS_NULL(method2))
    {
        EdgeFree(methodNodeItem2);
        printf("Error :: EdgeMalloc failed for method shutdown in Test create Nodes\n");
        return;
    }
    method2->description = "shutdown method";
    method2->methodNodeName = "shutdown";
    method2->method_fn = test_method_shutdown;

    method2->num_inpArgs = 0;
    method2->inpArg = NULL;

    method2->num_outArgs = 0;
    method2->outArg = NULL;

    createMethodNode(DEFAULT_NAMESPACE_VALUE, methodNodeItem2, method2);
    printf("\n|------------[Added] %s\n", methodNodeItem2->browseName);
    EdgeFree(methodNodeItem2);

    printf("\n[%d] Method Node with inArgMethod \n", ++index);
    EdgeNodeItem *methodNodeItem3 = (EdgeNodeItem *) EdgeCalloc(1, sizeof(EdgeNodeItem));
    VERIFY_NON_NULL_NR(methodNodeItem3);
    methodNodeItem3->browseName = "move_start_point";
    methodNodeItem3->sourceNodeId = NULL;

    EdgeMethod *method3 = (EdgeMethod *) EdgeMalloc(sizeof(EdgeMethod));
    if (IS_NULL(method3))
    {
        EdgeFree(methodNodeItem3);
        printf("Error :: EdgeMalloc failed for method printx  in Test create Nodes\n");
        return;
    }
    method3->description = "move start point";
    method3->methodNodeName = "move_start_point";
    method3->method_fn = test_method_move;

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
    if (IS_NULL(method3->inpArg[0]))
    {
        EdgeFree(method3->inpArg);
        EdgeFree(methodNodeItem3);
        EdgeFree(method3);
        printf("Error :: EdgeMalloc failed for method method3->inpArg[0]  in Test create Nodes\n");
        return;
    }
    method3->inpArg[0]->argType = EDGE_NODEID_INT32;
    method3->inpArg[0]->valType = SCALAR;

    method3->num_outArgs = 0;
    method3->outArg = NULL;

    createMethodNode(DEFAULT_NAMESPACE_VALUE, methodNodeItem3, method3);
    printf("\n|------------[Added] %s\n", methodNodeItem3->browseName);
    EdgeFree(methodNodeItem3);

    printf("\n[%d] Method Node with outArgMethod \n", ++index);
    EdgeNodeItem *methodNodeItem4 = (EdgeNodeItem *) EdgeCalloc(1, sizeof(EdgeNodeItem));
    VERIFY_NON_NULL_NR(methodNodeItem4);
    methodNodeItem4->browseName = "version()";
    methodNodeItem4->sourceNodeId = NULL;

    EdgeMethod *method4 = (EdgeMethod *) EdgeMalloc(sizeof(EdgeMethod));
    if (IS_NULL(method4))
    {
        EdgeFree(methodNodeItem4);
        printf("Error :: EdgeMalloc failed for method version  in Test create Nodes\n");
        return;
    }
    method4->description = "Get Version Info";
    method4->methodNodeName = "version";
    method4->method_fn = test_method_version;

    method4->num_inpArgs = 0;
    method4->inpArg = NULL;

    method4->num_outArgs = 1;
    method4->outArg = (EdgeArgument **) malloc(sizeof(EdgeArgument *) * method4->num_outArgs);
    if (IS_NULL(method4->outArg))
    {
        EdgeFree(methodNodeItem4);
        EdgeFree(method4);
        printf("Error :: EdgeMalloc failed for method method4->outArg  in Test create Nodes\n");
        return;
    }
    method4->outArg[0] = (EdgeArgument *) EdgeMalloc(sizeof(EdgeArgument));
    if (IS_NULL(method4->outArg[0]))
    {
        EdgeFree(method4->outArg);
        EdgeFree(methodNodeItem4);
        EdgeFree(method4);
        printf("Error :: EdgeMalloc failed for method method4->outArg[0]  in Test create Nodes\n");
        return;
    }
    method4->outArg[0]->argType = EDGE_NODEID_STRING;
    method4->outArg[0]->valType = SCALAR;

    createMethodNode(DEFAULT_NAMESPACE_VALUE, methodNodeItem4, method4);
    printf("\n|------------[Added] %s\n", methodNodeItem4->browseName);
    EdgeFree(methodNodeItem4);

    /******************* Robot Scenario Demo *********************/
    item = createVariableNodeItem("robot_name", EDGE_NODEID_STRING, "GTC-Robot-1", VARIABLE_NODE, 100);
    VERIFY_NON_NULL_NR(item);
    createNode(DEFAULT_NAMESPACE_VALUE, item);
    printf("\n|------------[Added] %s\n", item->browseName);
    deleteNodeItem(item);

    bool val = true;
    item = createVariableNodeItem("robot_status", EDGE_NODEID_BOOLEAN, (void *)&val, VARIABLE_NODE, 100);
    VERIFY_NON_NULL_NR(item);
    createNode(DEFAULT_NAMESPACE_VALUE, item);
    printf("\n|------------[Added] %s\n", item->browseName);
    deleteNodeItem(item);

    item = createVariableNodeItem("robot_id", EDGE_NODEID_STRING, "A31FR-23214-ASFF", VARIABLE_NODE, 25);
    VERIFY_NON_NULL_NR(item);
    createNode(DEFAULT_NAMESPACE_VALUE, item);
    printf("\n|------------[Added] %s\n", item->browseName);
    deleteNodeItem(item);

    int *posArray = (int *) EdgeMalloc(sizeof(int) * 3);
    if (IS_NOT_NULL(posArray))
    {
        posArray[0] = 123;
        posArray[1] = 34;
        posArray[2] = 20;
        item = createVariableNodeItem("robot_position", EDGE_NODEID_INT32, (void *) posArray, VARIABLE_NODE, 25);
        VERIFY_NON_NULL_NR(item);
        item->nodeType = ARRAY_NODE;
        item->arrayLength = 3;
        createNode(DEFAULT_NAMESPACE_VALUE, item);
        printf("\n|------------[Added] %s\n", item->browseName);
        EdgeFree(posArray);
        deleteNodeItem(item);
    }
    else
    {
        printf("Error :: EdgeMalloc failed for int Array in Test create Nodes\n");
    }

    /******************* Add Reference *********************/
    /************ Reference is not NODE CLASS ***************/
    printf(COLOR_GREEN"\n[Create Reference]\n"COLOR_RESET);
    printf("\n[%d] Make Reference that GTC_robot node Organizes with ObjectType1\n", ++index);

    EdgeReference *reference = (EdgeReference *) EdgeCalloc(1, sizeof(EdgeReference));
    if (IS_NOT_NULL(reference))
    {
        reference->forward = true;
        reference->sourceNamespace = (char *) DEFAULT_NAMESPACE_VALUE;
        reference->sourcePath = "GTC_robot";
        reference->targetNamespace = (char *) DEFAULT_NAMESPACE_VALUE;
        reference->targetPath = "robot_name";
        /* default reference ID : Organizes */
        addReference(reference);

        EdgeFree(reference);
    }
    else
    {
        printf("Error :: EdgeMalloc failed for EdgeReference in Test create Nodes\n");
    }

    reference = (EdgeReference *) EdgeCalloc(1, sizeof(EdgeReference));
    if (IS_NOT_NULL(reference))
    {
        reference->forward = true;
        reference->sourceNamespace = (char *) DEFAULT_NAMESPACE_VALUE;
        reference->sourcePath = "GTC_robot";
        reference->targetNamespace = (char *) DEFAULT_NAMESPACE_VALUE;
        reference->targetPath = "robot_id";
        /* default reference ID : Organizes */
        addReference(reference);

        EdgeFree(reference);
    }
    else
    {
        printf("Error :: EdgeMalloc failed for EdgeReference in Test create Nodes\n");
    }

    reference = (EdgeReference *) EdgeCalloc(1, sizeof(EdgeReference));
    if (IS_NOT_NULL(reference))
    {
        reference->forward = true;
        reference->sourceNamespace = (char *) DEFAULT_NAMESPACE_VALUE;
        reference->sourcePath = "GTC_robot";
        reference->targetNamespace = (char *) DEFAULT_NAMESPACE_VALUE;
        reference->targetPath = "robot_status";
        /* default reference ID : Organizes */
        addReference(reference);

        EdgeFree(reference);
    }
    else
    {
        printf("Error :: EdgeMalloc failed for EdgeReference in Test create Nodes\n");
    }

    reference = (EdgeReference *) EdgeCalloc(1, sizeof(EdgeReference));
    if (IS_NOT_NULL(reference))
    {
        reference->forward = true;
        reference->sourceNamespace = (char *) DEFAULT_NAMESPACE_VALUE;
        reference->sourcePath = "GTC_robot";
        reference->targetNamespace = (char *) DEFAULT_NAMESPACE_VALUE;
        reference->targetPath = "robot_position";
        /* default reference ID : Organizes */
        addReference(reference);

        EdgeFree(reference);
    }
    else
    {
        printf("Error :: EdgeMalloc failed for EdgeReference in Test create Nodes\n");
    }

        reference = (EdgeReference *) EdgeCalloc(1, sizeof(EdgeReference));
            if (IS_NOT_NULL(reference))
            {
                reference->forward = true;
                reference->sourceNamespace = (char *) DEFAULT_NAMESPACE_VALUE;
                reference->sourcePath = "GTC_robot";
                reference->targetNamespace = (char *) DEFAULT_NAMESPACE_VALUE;
                reference->targetPath = "move_start_point";
                /* default reference ID : Organizes */
                addReference(reference);

                EdgeFree(reference);
            }
            else
            {
                printf("Error :: EdgeMalloc failed for EdgeReference in Test create Nodes\n");
            }
    printf("\n\n");
}

static void testModifyNode()
{

    printf("\n" COLOR_YELLOW "------------------------------------------------------" COLOR_RESET);
    printf("\n" COLOR_YELLOW "                  Modify Variable Node            "COLOR_RESET);
    printf("\n" COLOR_YELLOW "------------------------------------------------------" COLOR_RESET
    "\n\n");
    char s_value[MAX_ENDPOINT_URI_SIZE];
    double d_value;
    unsigned int u_value;
    int i_value;
    int option;
    void *new_value = NULL;
    char name[MAX_ADDRESS_SIZE];

    printf(
            "\n\n" COLOR_YELLOW
            "********************** Available nodes to test the 'modifyVariableNode' service **********************"
            COLOR_RESET "\n");

    printf("[1] String1\n");
    printf("[2] String2\n");
    printf("[3] String3\n");
    printf("[4] Double\n");
    printf("[5] Int32\n");
    printf("[6] UInt16\n");
    printf("[7] ByteString\n");
    printf("[8] CharArray\n");
    printf("[9] String1 Changing Thread\n");
    printf("[10] Int32 Increasing Thread\n");
    printf("\nEnter any of the above option :: ");
    scanf("%d", &option);

    if (option < 1 || option > 10)
    {
        printf("Invalid Option!!! \n\n");
        return;
    }

    printf("\nEnter the new value :: ");
    if (option == 1)
    {
        scanf("%s", s_value);
        strncpy(name, "String1", sizeof(name)-1);
        name[sizeof(name)-1] = '\0';
        new_value = (void *) s_value;
    }
    else if (option == 2)
    {
        scanf("%s", s_value);
        strncpy(name, "String2", sizeof(name)-1);
        name[sizeof(name)-1] = '\0';
        new_value = (void *) s_value;
    }
    else if (option == 3)
    {
        scanf("%s", s_value);
        strncpy(name, "String3", sizeof(name)-1);
        name[sizeof(name)-1] = '\0';
        new_value = (void *) s_value;
    }
    else if (option == 4)
    {
        scanf("%lf", &d_value);
        strncpy(name, "Double", sizeof(name)-1);
        name[sizeof(name)-1] = '\0';
        new_value = (void *) &d_value;
    }
    else if (option == 5)
    {
        scanf("%d", &i_value);
        strncpy(name, "Int32", sizeof(name)-1);
        name[sizeof(name)-1] = '\0';
        new_value = (void *) &i_value;
    }
    else if (option == 6)
    {
        scanf("%u", &u_value);
        strncpy(name, "UInt16", sizeof(name)-1);
        name[sizeof(name)-1] = '\0';
        new_value = (void *) &u_value;
    }
    else if (option == 7)
    {
        scanf("%s", s_value);
        strncpy(name, "ByteString", sizeof(name)-1);
        name[sizeof(name)-1] = '\0';
        new_value = (void *) s_value;
    }
    else if (option == 8)
    {
        strncpy(name, "CharArray", sizeof(name)-1);
        name[sizeof(name)-1] = '\0';

        unsigned int num_values;
        printf("Enter number of string elements :  ");
        scanf("%u", &num_values);
        if(num_values < 1)
        {
            printf("Number of elements cannot be less than 1.\n");
            return;
        }

        char **new_str = (char**) EdgeCalloc(num_values, sizeof(char*));
        if(IS_NULL(new_str))
        {
            printf("Memory allocation failed.\n");
            return;
        }

        char val[MAX_ADDRESS_SIZE];
        for (unsigned int i = 0; i < num_values; i++)
        {
            printf("Enter string #%u :  ", (i+1));
            scanf("%s", val);
            size_t len  = strlen(val);
            new_str[i] = (char *) EdgeMalloc(len + 1);
            if(IS_NULL(new_str[i]))
            {
                printf("Memory allocation failed.\n");
                for(unsigned int j=0;j<i;j++)
                {
                    EdgeFree(new_str[j]);
                }
                EdgeFree(new_str);
                return;
            }
            strncpy(new_str[i], val, len);
            new_str[i][len] = '\0';
        }
        new_value = (void*) new_str;
        EdgeVersatility *message = (EdgeVersatility *) EdgeMalloc(sizeof(EdgeVersatility));
        if (IS_NOT_NULL(message))
        {
            message->arrayLength = num_values;
            message->isArray = true;
            message->value = new_value;
            modifyVariableNode(DEFAULT_NAMESPACE_VALUE, name, message);
            EdgeFree(message);

            #ifndef _WIN32
                usleep(1000 * 1000);
            #else
                Sleep(1000);
            #endif
        }
        else
        {
            printf("Error :: EdgeMalloc failed for CharArray EdgeVersatility in Test Modify Nodes\n");
        }
        for (int i = 0; i < num_values; i++)
        {
            EdgeFree(new_str[i]);
        }
        EdgeFree(new_str);
        return ;
    }
    else if (option == 9)
    {
        strncpy(name, "String1", strlen("String1"));
        name[strlen("String1")] = '\0';
        for (int i = 0; i < MAX_TEST_NUMBER; i++)
        {
            if (i % 2 == 0)
            {
                strncpy(s_value, SAMPLE_STRING_1, strlen(SAMPLE_STRING_1));
                s_value[strlen(SAMPLE_STRING_1)] = '\0';
            }
            else
            {
                strncpy(s_value, SAMPLE_STRING_2, strlen(SAMPLE_STRING_2));
                s_value[strlen(SAMPLE_STRING_2)] = '\0';
            }

            new_value = (void *) s_value;
            EdgeVersatility *message = (EdgeVersatility *) EdgeMalloc(sizeof(EdgeVersatility));
            if (IS_NOT_NULL(message))
            {
                message->value = new_value;
                modifyVariableNode(DEFAULT_NAMESPACE_VALUE, name, message);
                EdgeFree(message);
                #ifndef _WIN32
                usleep(1000 * 1000);
            	#else
                Sleep(1000);
				#endif
            }
            else
            {
                printf("Error :: EdgeMalloc failed for String1 EdgeVersatility in Test Modify Nodes\n");
            }
        }
        return;
    }
    else if (option == 10)
    {
        strncpy(name, "Int32", strlen("Int32"));
        name[strlen("Int32")] = '\0';
        for (int i = 0; i < MAX_TEST_NUMBER; i++)
        {
            new_value = (void *) &i;
            EdgeVersatility *message = (EdgeVersatility *) EdgeMalloc(sizeof(EdgeVersatility));
            if (IS_NOT_NULL(message))
            {
                message->value = new_value;
                modifyVariableNode(DEFAULT_NAMESPACE_VALUE, name, message);
                EdgeFree(message);
                #ifndef _WIN32
                usleep(1000 * 1000);
            	#else
                Sleep(1000);
				#endif
            }
            else
            {
                printf("Error :: EdgeMalloc failed for Int32 EdgeVersatility in Test Modify Nodes\n");
            }
        }
        return;
    }

    EdgeVersatility *message = (EdgeVersatility *) EdgeMalloc(sizeof(EdgeVersatility));
    if (IS_NOT_NULL(message))
    {
        message->value = new_value;
        modifyVariableNode(DEFAULT_NAMESPACE_VALUE, name, message);
        EdgeFree(message);
    }
    else
    {
        printf("Error :: EdgeMalloc failed for EdgeVersatility in Test Modify Nodes\n");
    }
}

static void deinit()
{
    if (startFlag)
    {
        b_running = false;
        pthread_join(m_serverThread, NULL);

        stopServer();
        startFlag = false;

        if (IS_NOT_NULL(config))
        {
            EdgeFree(config->recvCallback);
            EdgeFree(config->statusCallback);
            EdgeFree(config->discoveryCallback);

            EdgeFree(config);
        }

        EdgeFree(epInfo);
    }

}

static void print_menu()
{
    printf("=============== OPC UA =======================\n\n");

    printf("start : start opcua server\n");
    printf("update_node : update variable node\n");
    printf("update_robot : update robot node automatically\n");
    printf("quit : terminate/stop opcua server/client and then quit\n");
    printf("help : print menu\n");
    printf("show_nodelist : show the list of node in server");

    printf("\n=============== OPC UA =======================\n\n");
}

static void stopHandler(int sign)
{
    printf("Force terminate triggerred.\n");
    deinit();
    exit(0);
}

int main()
{
    char command[128];

    signal(SIGINT, stopHandler);

    print_menu();

    while (!stopFlag)
    {
        printf("[INPUT Command] : ");
        scanf("%s", command);

        if (stopFlag)
        {
            break;
        }
        else if (!strcmp(command, "start"))
        {

            printf(
                    "\n" COLOR_YELLOW " ------------------------------------------------------" COLOR_RESET);
            printf("\n" COLOR_YELLOW "                     Start Server            " COLOR_RESET);
            printf(
                    "\n" COLOR_YELLOW " ------------------------------------------------------" COLOR_RESET
                    "\n\n");

            unsigned int port = 0;
            printf("[input port] : ");
            scanf("%u", &port);

            strncpy(ipAddress, DEFAULT_HOST_NAME, strlen(DEFAULT_HOST_NAME));
            ipAddress[strlen(DEFAULT_HOST_NAME)] = '\0';

            snprintf(endpointUri, sizeof(endpointUri), "opc:tcp://%s:%u/edge-opc-server",
                    ipAddress, port);

            epInfo = (EdgeEndPointInfo *) EdgeCalloc(1, sizeof(EdgeEndPointInfo));
            if (IS_NOT_NULL(epInfo))
            {
                epInfo->endpointUri = endpointUri;

                init();
                startServer(port);

                print_menu();
            }
            else
            {
                printf("Error :: EdgeCalloc failed for EdgeEndPointInfo in Test create Nodes\n");
            }
        }
        else if (!strcmp(command, "update_node"))
        {
            testModifyNode();
        }
        else if (!strcmp(command, "update_robot"))
        {
            b_running = true;
            srand((unsigned) time(NULL));
            pthread_create(&m_serverThread, NULL, &server_sample_loop, NULL);
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
        else if (!strcmp(command, "show_nodelist"))
        {
            showNodeList();
        }
    }
    return 0;
}


void test_method_move(int inpSize, void **input, int outSize, void **output)
{
    int *inp = (int*) input[0];
    printf(COLOR_GREEN "\n[method call - robot is moved to start-point]" COLOR_RESET);
    printf(COLOR_PURPLE " (x:%d, y:%d, z:%d)\n" COLOR_RESET, *inp, *inp, *inp );

    EdgeVersatility *message = (EdgeVersatility *) EdgeMalloc(sizeof(EdgeVersatility));
    if (IS_NOT_NULL(message))
    {
        int posArray[3];
        if (IS_NOT_NULL(posArray))
        {
            posArray[0] = *inp;
            posArray[1] = *inp;
            posArray[2] = *inp;
            message->value = (void *) posArray;
            message->isArray = true;
            message->arrayLength = 3;
            modifyVariableNode(DEFAULT_NAMESPACE_VALUE, "robot_position", message);
        }
        else
        {
            printf("Error :: EdgeMalloc failed for int Array in Test create Nodes\n");
        }
        EdgeFree(message);
    }
}
