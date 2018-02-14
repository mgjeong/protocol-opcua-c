#include <gtest/gtest.h>
#include <iostream>
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
}

#define TAG "TC"

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

static char ipAddress[128];
static char endpointUri[512];
static EdgeEndPointInfo *epInfo = NULL;
static EdgeConfigure *config = NULL;

static uint8_t supportedApplicationTypes = EDGE_APPLICATIONTYPE_SERVER | EDGE_APPLICATIONTYPE_DISCOVERYSERVER |
    EDGE_APPLICATIONTYPE_CLIENTANDSERVER;

static bool startServerFlag = false;
static bool startClientFlag = false;
static bool readNodeFlag = true;
static bool browseNodeFlag = false;
static bool methodCallFlag = false;

char node_arr[13][30] =
{ "{2;S;v=12}String1", "{2;S;v=12}String2", "{2;S;v=12}String3", "{2;S;v=11}Double", "{2;S;v=6}Int32",
        "{2;S;v=5}UInt16", "{2;S;v=15}ByteString", "{2;S;v=3}Byte", "{2;S;v=12}Error", "{2;S;v=14}Guid",
        "{2;S;v=11}DoubleArray", "{2;S;v=12}CharArray", "{2;S;v=15}ByteStringArray" };

static int method_arr[5] =
{ 15, 25, 35, 45, 55 };

typedef struct BrowseNextData
{
    EdgeBrowseParameter browseParam;
    int count;
    int last_used;
    EdgeContinuationPoint *cp; // Continuation point List. Size of list = last_used.
    EdgeNodeId **srcNodeId; // Id of source node of every continuation point. Size of list = last_used.
} BrowseNextData;

BrowseNextData *browseNextData = NULL;

extern void testRead_P1(char *endpointUri);
extern void testRead_P2(char *endpointUri);
extern void testRead_P3(char *endpointUri);
extern void testRead_P4(char *endpointUri);
extern void testReadWithoutEndpoint();
extern void testReadWithoutValueAlias(char *endpointUri);
extern void testReadWithoutMessage();

extern void testWrite_P1(char *endpointUri);
extern void testWrite_P2(char *endpointUri);
extern void testWrite_P3(char *endpointUri);
extern void testWriteWithoutEndpoint();
extern void testWriteWithoutValueAlias(char *endpointUri);
extern void testWriteWithoutMessage();

extern void testMethod_P1(char *endpointUri);
extern void testMethod_P2(char *endpointUri);
extern void testMethod_P3(char *endpointUri);
extern void testMethodWithoutEndpoint();
extern void testMethodWithoutValueAlias(char *endpointUri);
extern void testMethodWithoutMessage();


extern "C"
{

    static void startClient(char *addr, int port, char *securityPolicyUri);

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
                            if (data->responses[idx]->type == Boolean)
                            {
                                /* Handle Boolean output array */
                                PRINT_ARG("Boolean output array length :: ", arrayLen);
                                for (int arrayIdx = 0; arrayIdx < arrayLen; arrayIdx++)
                                {
                                    PRINT_ARG("  ",
                                            ((bool * ) data->responses[idx]->message->value)[arrayIdx]);
                                }
                            }
                            else if (data->responses[idx]->type == Byte)
                            {
                                /* Handle Byte output array */
                                PRINT_ARG("Byte output array length ::", arrayLen);
                                for (int arrayIdx = 0; arrayIdx < arrayLen; arrayIdx++)
                                {
                                    PRINT(
                                            ((int8_t * ) data->responses[idx]->message->value)[arrayIdx]);
                                }
                            }
                            else if (data->responses[idx]->type == SByte)
                            {
                                /* Handle SByte output array */
                                PRINT_ARG("SByte output array length :: ", arrayLen);
                                for (int arrayIdx = 0; arrayIdx < arrayLen; arrayIdx++)
                                {
                                    PRINT(
                                            ((int8_t * ) data->responses[idx]->message->value)[arrayIdx]);
                                }
                            }
                            else if (data->responses[idx]->type == Int16)
                            {
                                /* Handle int16 output array */
                                PRINT_ARG("Int16 output array length :: ", arrayLen);
                                for (int arrayIdx = 0; arrayIdx < arrayLen; arrayIdx++)
                                {
                                    PRINT(
                                            ((int16_t * ) data->responses[idx]->message->value)[arrayIdx]);
                                }
                            }
                            else if (data->responses[idx]->type == UInt16)
                            {
                                /* Handle UInt16 output array */
                                PRINT_ARG("UInt16 output array length ::", arrayLen);
                                for (int arrayIdx = 0; arrayIdx < arrayLen; arrayIdx++)
                                {
                                    PRINT(
                                            ((int16_t * ) data->responses[idx]->message->value)[arrayIdx]);
                                }
                            }
                            else if (data->responses[idx]->type == Int32)
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
                            else if (data->responses[idx]->type == UInt32)
                            {
                                /* Handle UInt32 output array */
                                PRINT_ARG("UInt32 output array length :: ", arrayLen);
                                for (int arrayIdx = 0; arrayIdx < arrayLen; arrayIdx++)
                                {
                                    PRINT(
                                            ((int32_t * ) data->responses[idx]->message->value)[arrayIdx]);
                                }
                            }
                            else if (data->responses[idx]->type == Int64)
                            {
                                /* Handle Int64 output array */
                                PRINT_ARG("Int64 output array length :: ", arrayLen);
                                for (int arrayIdx = 0; arrayIdx < arrayLen; arrayIdx++)
                                {
                                    PRINT(
                                            ((long int * ) data->responses[idx]->message->value)[arrayIdx]);
                                }
                            }
                            else if (data->responses[idx]->type == UInt64)
                            {
                                /* Handle UInt64 output array */
                                PRINT_ARG("UInt64 output array length :: ", arrayLen);
                                for (int arrayIdx = 0; arrayIdx < arrayLen; arrayIdx++)
                                {
                                    PRINT(
                                            ((long int * ) data->responses[idx]->message->value)[arrayIdx]);
                                }
                            }
                            else if (data->responses[idx]->type == Float)
                            {
                                /* Handle Float output array */
                                PRINT_ARG("Float output array length :: ", arrayLen);
                                for (int arrayIdx = 0; arrayIdx < arrayLen; arrayIdx++)
                                {
                                    PRINT(
                                            ((float * ) data->responses[idx]->message->value)[arrayIdx]);
                                }
                            }
                            else if (data->responses[idx]->type == Double)
                            {
                                /* Handle Double output array */
                                PRINT_ARG("Double output array length :: ", arrayLen);
                                for (int arrayIdx = 0; arrayIdx < arrayLen; arrayIdx++)
                                {
                                    PRINT(
                                            ((double * ) data->responses[idx]->message->value)[arrayIdx]);
                                }
                            }
                            else if (data->responses[idx]->type == String
                                    || data->responses[idx]->type == ByteString
                                    || data->responses[idx]->type == Guid)
                            {
                                /* Handle String/ByteString/Guid output array */
                                PRINT_ARG("String/ByteString/Guid output array length :: ",
                                        arrayLen);
                                char **values = ((char **) data->responses[idx]->message->value);
                                for (int arrayIdx = 0; arrayIdx < arrayLen; arrayIdx++)
                                {
                                    PRINT(values[arrayIdx]);
                                }
                            }
                            else if (data->responses[idx]->type == DateTime)
                            {
                                /* Handle DateTime output array */
                                PRINT_ARG("DateTime output array length :: ", arrayLen);
                                for (int arrayIdx = 0; arrayIdx < arrayLen; arrayIdx++)
                                {
                                    PRINT(
                                            ((int64_t * ) data->responses[idx]->message->value)[arrayIdx]);
                                }
                            }
                        }
                        else
                        {
                            if (data->responses[idx]->type == Boolean)
                                PRINT_ARG(
                                        "[Application response Callback] Data read from node ===>> ",
                                        *((int * )data->responses[idx]->message->value));
                            else if (data->responses[idx]->type == Byte)
                                PRINT_ARG(
                                        "[Application response Callback] Data read from node ===>> ",
                                        *((int8_t * )data->responses[idx]->message->value));
                            else if (data->responses[idx]->type == SByte)
                                PRINT_ARG(
                                        "[Application response Callback] Data read from node ===>> ",
                                        *((int8_t * )data->responses[idx]->message->value));
                            else if (data->responses[idx]->type == ByteString)
                                PRINT_ARG(
                                        "[Application response Callback] Data read from node ===>> ",
                                        (char * )data->responses[idx]->message->value);
                            else if (data->responses[idx]->type == DateTime)
                                PRINT_ARG(
                                        "[Application response Callback] Data read from node ===>> ",
                                        *((int64_t * )data->responses[idx]->message->value));
                            else if (data->responses[idx]->type == Double)
                            {
                                PRINT_ARG(
                                        "[Application response Callback] Data read from node ===>>  ",
                                        *((double * )data->responses[idx]->message->value));
                            }
                            else if (data->responses[idx]->type == Float)
                                PRINT_ARG(
                                        "[Application response Callback] Data read from node ===>>  ",
                                        *((float * )data->responses[idx]->message->value));
                            else if (data->responses[idx]->type == Int16)
                                PRINT_ARG(
                                        "[Application response Callback] Data read from node ===>> ",
                                        *((int16_t * )data->responses[idx]->message->value));
                            else if (data->responses[idx]->type == UInt16)
                            {
                                PRINT_ARG(
                                        "[Application response Callback] Data read from node ===>> ",
                                        *((int16_t * )data->responses[idx]->message->value));

                                int16_t temp = *((int16_t *) data->responses[idx]->message->value);
                                if (readNodeFlag)
                                    EXPECT_EQ(temp, TEST_UINT16_R);
                                else
                                    EXPECT_EQ(temp, TEST_UINT16_W);
                            }
                            else if (data->responses[idx]->type == Int32)
                            {
                                PRINT_ARG(
                                        "[Application response Callback] Data read from node ===>>  ",
                                        *((int32_t * )data->responses[idx]->message->value));

                                int temp = *((int *) data->responses[idx]->message->value);
                                if (readNodeFlag)
                                    EXPECT_EQ(temp, TEST_INT32_R);
                                else
                                    EXPECT_EQ(temp, TEST_INT32_W);
                            }
                            else if (data->responses[idx]->type == UInt32)
                                PRINT_ARG(
                                        "[Application response Callback] Data read from node ===>>  ",
                                        *((int32_t * )data->responses[idx]->message->value));
                            else if (data->responses[idx]->type == Int64)
                                PRINT_ARG(
                                        "[Application response Callback] Data read from node ===>>  ",
                                        *((int64_t * )data->responses[idx]->message->value));
                            else if (data->responses[idx]->type == UInt64)
                                PRINT_ARG(
                                        "[Application response Callback] Data read from node ===>>  ",
                                        *((int64_t * )data->responses[idx]->message->value));
                            else if (data->responses[idx]->type == String)
                            {
                                PRINT_ARG(
                                        "[Application response Callback] Data read from node ===>>  ",
                                        (char * )data->responses[idx]->message->value);

                                char *temp = ((char *) data->responses[idx]->message->value);
                                if (readNodeFlag)
                                    EXPECT_EQ(
                                            (strcmp(temp, TEST_STR1_R) && strcmp(temp, TEST_STR2_R) && strcmp(temp, TEST_STR3_R)),
                                            0);
                                else
                                    EXPECT_EQ(
                                            (strcmp(temp, TEST_STR1_W) && strcmp(temp, TEST_STR2_W) && strcmp(temp, TEST_STR3_W)),
                                            0);
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
            PRINT("[Application response Callback] Monitored Item Response received");
            int len = data->responseLength;
            int idx = 0;
            for (idx = 0; idx < len; idx++)
            {
                if (data->responses[idx]->message != NULL)
                {
                    if (data->responses[idx]->type == Int16)
                        PRINT_ARG(
                                "[MonitoredItem DataChange callback] Monitored Change Value read from node ===>> ",
                                *((int * )data->responses[idx]->message->value));
                    else if (data->responses[idx]->type == UInt16)
                        PRINT_ARG(
                                "[MonitoredItem DataChange callback] Monitored Change Value read from node ===>> ",
                                *((int * )data->responses[idx]->message->value));
                    else if (data->responses[idx]->type == Int32)
                        PRINT_ARG(
                                "[MonitoredItem DataChange callback] Monitored Change Value read from node ===>>  ",
                                *((int * )data->responses[idx]->message->value));
                    else if (data->responses[idx]->type == UInt32)
                        PRINT_ARG(
                                "[MonitoredItem DataChange callbackk] Monitored Change Value read from node ===>> ",
                                *((int * )data->responses[idx]->message->value));
                    else if (data->responses[idx]->type == Int64)
                        PRINT_ARG(
                                "[MonitoredItem DataChange callbackk] Monitored Change Value read from node ===>>  ",
                                *((long * )data->responses[idx]->message->value));
                    else if (data->responses[idx]->type == UInt64)
                        PRINT_ARG(
                                "[MonitoredItem DataChange callbackk] Monitored Change Value read from node ===>>  ",
                                *((long * )data->responses[idx]->message->value));
                    else if (data->responses[idx]->type == Float)
                        PRINT_ARG(
                                "[MonitoredItem DataChange callback] Monitored Change Value read from node  ===>>  ",
                                *((float * )data->responses[idx]->message->value));
                    else if (data->responses[idx]->type == Double)
                        PRINT_ARG(
                                "[MonitoredItem DataChange callback] Monitored Change Value read from node  ===>>  ",
                                *((double * )data->responses[idx]->message->value));
                    else if (data->responses[idx]->type == String)
                        PRINT_ARG(
                                "[MonitoredItem DataChange callback] Monitored Change Value read from node ===>>  ",
                                ((char * )data->responses[idx]->message->value));
                }
            }
        }
    }

    static void error_msg_cb(EdgeMessage *data)
    {
        PRINT_ARG("[error_msg_cb] EdgeStatusCode: ", data->result->code);
    }
    static void browse_msg_cb (EdgeMessage *data)
    {
        browseNodeFlag = true;
        if (data->browseResult)
        {
            if(data->responses[0]->message != NULL)
            {
                //PRINT_ARG("\n", (unsigned char *)data->responses[0]->message->value);
            }
        }
        else
        {
            if (data->cpList && data->cpList->count > 0)
            {
                // Do somethings
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
        PRINT("endpoint foung\n");
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

static void subscribeAndModifyNodes()
{
    /* Create Subscription */
    EdgeMessage* msg = createEdgeSubMessage(endpointUri, node_arr[0], 1, Edge_Create_Sub);
    EXPECT_EQ(NULL!=msg, true);

    double samplingInterval = 100.0;
    int keepalivetime = (1 > (int) (ceil(10000.0 / 0.0))) ? 1 : (int) ceil(10000.0 / 0.0);
    insertSubParameter(&msg, node_arr[0], Edge_Create_Sub, samplingInterval, 0.0, keepalivetime, 10000, 1, true, 0, 50);

    EdgeResult result = sendRequest(msg);
    EXPECT_EQ(result.code, STATUS_OK);
    destroyEdgeMessage (msg);
    sleep(1);

    /* Modify Subscription */
    msg = createEdgeSubMessage(endpointUri, node_arr[0], 0, Edge_Modify_Sub);
    EXPECT_EQ(NULL!=msg, true);

    samplingInterval = 500.0;
    keepalivetime = (1 > (int) (ceil(10000.0 / 0.0))) ? 1 : (int) ceil(10000.0 / 0.0);
    insertSubParameter(&msg, node_arr[0], Edge_Modify_Sub, samplingInterval, 0.0, keepalivetime, 10000, 1, true, 0, 50);

    result = sendRequest(msg);
    EXPECT_EQ(result.code, STATUS_OK);
    destroyEdgeMessage (msg);
    sleep(1);

    /* Delete Subscription */
    msg = createEdgeSubMessage(endpointUri, node_arr[0], 0, Edge_Delete_Sub);
    EXPECT_EQ(NULL!=msg, true);
    result = sendRequest(msg);
    EXPECT_EQ(result.code, STATUS_OK);
    destroyEdgeMessage(msg);
    sleep(1);
}

static void browseNodes()
{
    int  maxReferencesPerNode = 0;
    EdgeMessage *msg = createEdgeMessage(endpointUri, 1, CMD_BROWSE);
    EXPECT_EQ(NULL != msg, true);

    EdgeNodeInfo* nodeInfo = createEdgeNodeInfoForNodeId(INTEGER, RootFolder, SYSTEM_NAMESPACE_INDEX);
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

static void browseViews()
{
    EdgeMessage *msg = createEdgeMessage(endpointUri, 0, CMD_BROWSE_VIEW);
    EXPECT_EQ(NULL != msg, true);
    EdgeResult result = sendRequest(msg);
    ASSERT_EQ(result.code, STATUS_OK);
    destroyEdgeMessage(msg);
    sleep(1);
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

static void start_server()
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
    appConfig->applicationUri = (char *) DEFAULT_SERVER_URI_VALUE;
    appConfig->productUri = (char *) DEFAULT_PRODUCT_URI_VALUE;

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

static void stop_server()
{
    configureCallbacks();
    EdgeEndPointInfo *epStop = (EdgeEndPointInfo *) EdgeCalloc(1, sizeof(EdgeEndPointInfo));
    epStop->endpointUri = endpointUri;

    closeServer(epStop);

    printf("b4 assert \n");
    //ASSERT_TRUE(startServerFlag == false);
    printf("after assert \n");

    deleteMessage(NULL, epStop);

    printf("clean cbs\n");
    cleanCallbacks();
}

static void stop_client()
{
    EdgeMessage *msg = createEdgeMessage(endpointUri, 1, CMD_STOP_CLIENT);
    EXPECT_EQ(NULL!=msg, true);
    disconnectClient(msg->endpointInfo);
    destroyEdgeMessage(msg);
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
    start_server();

    ASSERT_TRUE(startServerFlag == true);

    if (startServerFlag == true)
        printf("true\n");
    else
        printf("false\n");

    PRINT("=============STOP SERVER===============");

    stop_server();
    if (startServerFlag == true)
        printf("true\n");
    else
        printf("false\n");
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
    appConfig->applicationUri = (char *) DEFAULT_SERVER_URI_VALUE;
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
#ifdef TO_BE_REMOVED

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
    appConfig->applicationUri = (char *) DEFAULT_SERVER_URI_VALUE;
    appConfig->productUri = (char *) DEFAULT_PRODUCT_URI_VALUE;

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

#endif

    EXPECT_EQ(startServerFlag, true);

    EdgeResult result = createNamespace(DEFAULT_NAMESPACE_VALUE,
    DEFAULT_ROOT_NODE_INFO_VALUE,
    DEFAULT_ROOT_NODE_INFO_VALUE,
    DEFAULT_ROOT_NODE_INFO_VALUE);

    EXPECT_EQ(result.code, STATUS_OK);

//    deleteMessage(msg, ep);

//    cleanCallbacks();

//    free(endpointConfig);
//    free(appConfig);

//    if (epInfo->endpointUri != NULL)
//    {
//        free(epInfo->endpointUri);
//        epInfo->endpointUri = NULL;
//    }
}

TEST_F(OPC_serverTests , ServerAddNodes_P)
{
    start_server();

    EXPECT_EQ(startServerFlag, true);

    EdgeNodeItem *item = NULL;

    // VARIABLE NODE with string variant:
    item = createVariableNodeItem("String1", String, (void *)"test1", VARIABLE_NODE);
    EdgeResult result = createNode(DEFAULT_NAMESPACE_VALUE, item);
    EXPECT_EQ(result.code, STATUS_OK);
    deleteNodeItem(item);

    item = createVariableNodeItem("String2", String, (void *)"test2", VARIABLE_NODE);
    result = createNode(DEFAULT_NAMESPACE_VALUE, item);
    EXPECT_EQ(result.code, STATUS_OK);
    deleteNodeItem(item);

    // VARIABLE NODE with Double variant:
    double d_val = 50.4;
    item = createVariableNodeItem("Double", Double, (void *) &d_val, VARIABLE_NODE);
    VERIFY_NON_NULL_NR(item);
    createNode(DEFAULT_NAMESPACE_VALUE, item);
    printf("\n|------------[Added] %s\n", item->browseName);
    deleteNodeItem(item);

    // VARIABLE NODE with UInt16 variant:
    int value = 30;
    item = createVariableNodeItem("UInt16", UInt16, (void *) &value, VARIABLE_NODE);
    result = createNode(DEFAULT_NAMESPACE_VALUE, item);
    EXPECT_EQ(result.code, STATUS_OK);
    deleteNodeItem(item);

    //modify value for this node
    value = 43;
    char *name = "UInt16";
    EdgeVersatility *message = (EdgeVersatility *) EdgeMalloc(sizeof(EdgeVersatility));
    message->value = (void *) &value;
    result = modifyVariableNode(DEFAULT_NAMESPACE_VALUE, name, message);
    EXPECT_EQ(result.code, STATUS_OK);
    EdgeFree(message);

    //Array Nodes with double values
    double *data = (double *) EdgeMalloc(sizeof(double) * 2);
    data[0] = 10.2;
    data[1] = 20.2;
    item = createVariableNodeItem("DoubleArray", Double, (void *) data, VARIABLE_NODE);
    item->nodeType = ARRAY_NODE;
    item->arrayLength = 2;
    result = createNode(DEFAULT_NAMESPACE_VALUE, item);
    EXPECT_EQ(result.code, STATUS_OK);
    EdgeFree(data);
    deleteNodeItem(item);

    // Guid
    UA_Guid guid =
    { 1, 0, 1,
    { 0, 0, 0, 0, 1, 1, 1, 1 } };
    item = createVariableNodeItem("Guid", Guid, (void *) &guid, VARIABLE_NODE);
    VERIFY_NON_NULL_NR(item);
    createNode(DEFAULT_NAMESPACE_VALUE, item);
    printf("\n|------------[Added] %s\n", item->browseName);
    deleteNodeItem(item);

    //Array Nodes with ByteString values
    UA_ByteString **dataArray = (UA_ByteString **) malloc(sizeof(UA_ByteString *) * 5);
    dataArray[0] = (UA_ByteString *) EdgeMalloc(sizeof(UA_ByteString));
    *dataArray[0] = UA_BYTESTRING_ALLOC("abcde");
    dataArray[1] = (UA_ByteString *) EdgeMalloc(sizeof(UA_ByteString));
    *dataArray[1] = UA_BYTESTRING_ALLOC("fghij");
    dataArray[2] = (UA_ByteString *) EdgeMalloc(sizeof(UA_ByteString));
    *dataArray[2] = UA_BYTESTRING_ALLOC("klmno");
    dataArray[3] = (UA_ByteString *) EdgeMalloc(sizeof(UA_ByteString));
    *dataArray[3] = UA_BYTESTRING_ALLOC("pqrst");
    dataArray[4] = (UA_ByteString *) EdgeMalloc(sizeof(UA_ByteString));
    *dataArray[4] = UA_BYTESTRING_ALLOC("uvwxyz");
    item = createVariableNodeItem("ByteStringArray", ByteString, (void *) dataArray, VARIABLE_NODE);
    item->nodeType = ARRAY_NODE;
    item->arrayLength = 5;
    result = createNode(DEFAULT_NAMESPACE_VALUE, item);
    EXPECT_EQ(result.code, STATUS_OK);
    deleteNodeItem(item);

    //Array Nodes with String values
    char **data1 = (char **) malloc(sizeof(char *) * 5);
    data1[0] = (char *) EdgeMalloc(10);
    strcpy(data1[0], "apple");
    data1[1] = (char *) EdgeMalloc(10);
    strcpy(data1[1], "ball");
    data1[2] = (char *) EdgeMalloc(10);
    strcpy(data1[2], "cats");
    data1[3] = (char *) EdgeMalloc(10);
    strcpy(data1[3], "dogs");
    data1[4] = (char *) EdgeMalloc(10);
    strcpy(data1[4], "elephant");
    item = createVariableNodeItem("CharArray", String, (void *) data1, VARIABLE_NODE);
    item->nodeType = ARRAY_NODE;
    item->arrayLength = 5;
    result = createNode(DEFAULT_NAMESPACE_VALUE, item);
    EXPECT_EQ(result.code, STATUS_OK);
    deleteNodeItem(item);

    //OBJECT NODE
    EdgeNodeId *edgeNodeId = (EdgeNodeId *) EdgeCalloc(1, sizeof(EdgeNodeId));
    item = createNodeItem("Object1", OBJECT_NODE, edgeNodeId);
    result = createNode(DEFAULT_NAMESPACE_VALUE, item);
    EXPECT_EQ(result.code, STATUS_OK);
    EdgeFree(edgeNodeId);
    deleteNodeItem(item);

    edgeNodeId = (EdgeNodeId *) EdgeMalloc(sizeof(EdgeNodeId));
    edgeNodeId->nodeId = "Object1";
    item = createNodeItem("Object2", OBJECT_NODE, edgeNodeId);
    result = createNode(DEFAULT_NAMESPACE_VALUE, item);
    EXPECT_EQ(result.code, STATUS_OK);
    EdgeFree(edgeNodeId);
    deleteNodeItem(item);

    //OBJECT TYPE NDOE
    edgeNodeId = (EdgeNodeId *) EdgeMalloc(sizeof(EdgeNodeId));
    edgeNodeId->nodeId = NULL; // no source node
    item = createNodeItem("ObjectType1", OBJECT_TYPE_NODE, edgeNodeId);
    result = createNode(DEFAULT_NAMESPACE_VALUE, item);
    EXPECT_EQ(result.code, STATUS_OK);
    EdgeFree(edgeNodeId);
    deleteNodeItem(item);

    edgeNodeId = (EdgeNodeId *) EdgeMalloc(sizeof(EdgeNodeId));
    edgeNodeId->nodeId = "ObjectType1";
    item = createNodeItem("ObjectType2", OBJECT_TYPE_NODE, edgeNodeId);
    result = createNode(DEFAULT_NAMESPACE_VALUE, item);
    EXPECT_EQ(result.code, STATUS_OK);
    EdgeFree(edgeNodeId);
    deleteNodeItem(item);

    //DATA TYPE NODE
    edgeNodeId = (EdgeNodeId *) EdgeCalloc(1, sizeof(EdgeNodeId));
    item = createNodeItem("DataType1", DATA_TYPE_NODE, edgeNodeId);
    result = createNode(DEFAULT_NAMESPACE_VALUE, item);
    EXPECT_EQ(result.code, STATUS_OK);
    EdgeFree(edgeNodeId);
    deleteNodeItem(item);

    edgeNodeId = (EdgeNodeId *) EdgeMalloc(sizeof(EdgeNodeId));
    edgeNodeId->nodeId = "DataType1";
    item = createNodeItem("DataType2", DATA_TYPE_NODE, edgeNodeId);
    result = createNode(DEFAULT_NAMESPACE_VALUE, item);
    EXPECT_EQ(result.code, STATUS_OK);
    EdgeFree(edgeNodeId);
    deleteNodeItem(item);

    //VIEW NODE
    edgeNodeId = (EdgeNodeId *) EdgeCalloc(1, sizeof(EdgeNodeId));
    item = createNodeItem("ViewNode1", VIEW_NODE, edgeNodeId);
    result = createNode(DEFAULT_NAMESPACE_VALUE, item);
    EXPECT_EQ(result.code, STATUS_OK);
    EdgeFree(edgeNodeId);
    deleteNodeItem(item);

    edgeNodeId = (EdgeNodeId *) EdgeMalloc(sizeof(EdgeNodeId));
    edgeNodeId->nodeId = "ViewNode1";
    item = createNodeItem("ViewNode2", VIEW_NODE, edgeNodeId);
    result = createNode(DEFAULT_NAMESPACE_VALUE, item);
    EXPECT_EQ(result.code, STATUS_OK);
    EdgeFree(edgeNodeId);
    deleteNodeItem(item);

    //REFERENCE TYPE NODE
    edgeNodeId = (EdgeNodeId *) EdgeCalloc(1, sizeof(EdgeNodeId));
    item = createNodeItem("ReferenceTypeNode1", REFERENCE_TYPE_NODE, edgeNodeId);
    result = createNode(DEFAULT_NAMESPACE_VALUE, item);
    EXPECT_EQ(result.code, STATUS_OK);
    EdgeFree(edgeNodeId);

    edgeNodeId = (EdgeNodeId *) EdgeMalloc(sizeof(EdgeNodeId));
    edgeNodeId->nodeId = "ReferenceTypeNode1";
    item = createNodeItem("ReferenceTypeNode2", REFERENCE_TYPE_NODE, edgeNodeId);
    result = createNode(DEFAULT_NAMESPACE_VALUE, item);
    EXPECT_EQ(result.code, STATUS_OK);
    EdgeFree(edgeNodeId);

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
        method->inpArg[idx]->argType = Double;
        method->inpArg[idx]->valType = SCALAR;
    }

    method->num_outArgs = 2;
    method->outArg = (EdgeArgument **) malloc(sizeof(EdgeArgument *) * method->num_outArgs);
    for (int idx = 0; idx < method->num_outArgs; idx++)
    {
        method->outArg[idx] = (EdgeArgument *) EdgeMalloc(sizeof(EdgeArgument));
        method->outArg[idx]->argType = Double;
        method->outArg[idx]->valType = SCALAR;
    }
    result = createMethodNode(DEFAULT_NAMESPACE_VALUE, methodNodeItem, method);
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
    method1->inpArg[0]->argType = Int32;
    method1->inpArg[0]->valType = ARRAY_1D;
    method1->inpArg[0]->arrayLength = 5;

    method1->inpArg[1] = (EdgeArgument *) EdgeMalloc(sizeof(EdgeArgument));
    method1->inpArg[1]->argType = Int32;
    method1->inpArg[1]->valType = SCALAR;

    method1->num_outArgs = 1;
    method1->outArg = (EdgeArgument **) malloc(sizeof(EdgeArgument *) * method1->num_outArgs);
    for (int idx = 0; idx < method1->num_outArgs; idx++)
    {
        method1->outArg[idx] = (EdgeArgument *) EdgeMalloc(sizeof(EdgeArgument));
        method1->outArg[idx]->argType = Int32;
        method1->outArg[idx]->valType = ARRAY_1D;
        method1->outArg[idx]->arrayLength = 5;
    }
    createMethodNode(DEFAULT_NAMESPACE_VALUE, methodNodeItem1, method1);
    EdgeFree(methodNodeItem1);

    /* Method Node */
    EdgeNodeItem *methodNodeItem2 = (EdgeNodeItem *) EdgeMalloc(sizeof(EdgeNodeItem));
    methodNodeItem2->browseName = "string_method(x)";
    methodNodeItem2->sourceNodeId = NULL;

    EdgeMethod *method2 = (EdgeMethod *) EdgeMalloc(sizeof(EdgeMethod));
    method2->description = "string copy method";
    method2->methodNodeName = "string_method";
    method2->method_fn = string_method;

    method2->num_inpArgs = 1;
    method2->inpArg = (EdgeArgument **) malloc(sizeof(EdgeArgument *) * method2->num_inpArgs);
    method2->inpArg[0] = (EdgeArgument *) EdgeMalloc(sizeof(EdgeArgument));
    method2->inpArg[0]->argType = String;
    method2->inpArg[0]->valType = ARRAY_1D;
    method2->inpArg[0]->arrayLength = 5;

    method2->num_outArgs = 1;
    method2->outArg = (EdgeArgument **) malloc(sizeof(EdgeArgument *) * method2->num_outArgs);
    for (int idx = 0; idx < method2->num_outArgs; idx++)
    {
        method2->outArg[idx] = (EdgeArgument *) EdgeMalloc(sizeof(EdgeArgument));
        method2->outArg[idx]->argType = String;
        method2->outArg[idx]->valType = ARRAY_1D;
        method2->outArg[idx]->arrayLength = 5;
    }
    createMethodNode(DEFAULT_NAMESPACE_VALUE, methodNodeItem2, method2);
    EdgeFree(methodNodeItem2);


    //REFERENCE NODE
    /*EdgeReference *reference = (EdgeReference *) EdgeMalloc(sizeof(EdgeReference));
    reference->forward = true;
    reference->sourceNamespace = DEFAULT_NAMESPACE_VALUE;
    reference->sourcePath = "ViewNode1";
    reference->targetNamespace = DEFAULT_NAMESPACE_VALUE;
    reference->targetPath = "ObjectType1";
    result = addReference(reference);
    EXPECT_EQ(result.code, STATUS_OK);
    EdgeFree(reference);*/

//    deleteMessage(msg, ep);

//    cleanCallbacks();

//    free(endpointConfig);
//    free(appConfig);

    if (epInfo->endpointUri != NULL)
    {
        free(epInfo->endpointUri);
        epInfo->endpointUri = NULL;
    }
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

TEST_F(OPC_clientTests , StartClient_P)
{
    PRINT("=============== startClient ==================");
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
    PRINT("=============== startClient ==================");

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
    PRINT("=============== startClient ==================");

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
    PRINT("=============== startClient ==================");

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
    PRINT("=============== startClient ==================");

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

TEST_F(OPC_clientTests , ClientRead_N1)
{
    PRINT("=============== startClient ==================");

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
    PRINT("=============== startClient ==================");

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
    PRINT("=============== startClient ==================");

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

TEST_F(OPC_clientTests , ClientBrowse_P)
{
    PRINT("=============== startClient ==================");

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
    PRINT("=============== startClient ==================");

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

TEST_F(OPC_clientTests , ClientSubscribe_P)
{
    EXPECT_EQ(startClientFlag, false);

    EdgeMessage *msg = createEdgeMessage(endpointUri, 1, CMD_GET_ENDPOINTS);
    EXPECT_EQ(NULL != msg, true);
    EdgeResult res = getEndpointInfo(msg);
    EXPECT_EQ(res.code, STATUS_OK);
    EXPECT_EQ(startClientFlag, true);
    destroyEdgeMessage(msg);

    subscribeAndModifyNodes();

    stop_client();
    EXPECT_EQ(startClientFlag, false);
}

int main(int argc, char **argv)
{
    ::testing::InitGoogleTest(&argc, argv);
    return RUN_ALL_TESTS();
}
