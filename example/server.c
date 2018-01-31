#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>

#include <pthread.h>
#include <unistd.h>

#include <opcua_manager.h>
#include "opcua_common.h"
#include "edge_identifier.h"
#include "edge_utils.h"
#include "open62541.h"

#define COLOR_GREEN        "\x1b[32m"
#define COLOR_YELLOW      "\x1b[33m"
#define COLOR_PURPLE      "\x1b[35m"
#define COLOR_RESET         "\x1b[0m"

#define MAX_TEST_NUMBER 10000
#define SAMPLE_STRING_1 "test_1"
#define SAMPLE_STRING_2 "test_2"

static bool startFlag = false;
static bool stopFlag = false;

static char ipAddress[128];
static char endpointUri[512];

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

// TODO: Remove this function later when sdk expose it.

static void response_msg_cb(EdgeMessage *data)
{

}

static void monitored_msg_cb(EdgeMessage *data)
{

}

static void error_msg_cb(EdgeMessage *data)
{
    printf("[error_msg_cb] EdgeStatusCode: %d\n", data->result->code);
}

static void browse_msg_cb(EdgeMessage *data)
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

static void status_network_cb(EdgeEndPointInfo *epInfo, EdgeStatusCode status)
{

}

/* discovery callback */
static void endpoint_found_cb(EdgeDevice *device)
{

}

static void device_found_cb(EdgeDevice *device)
{

}

static void init()
{
    config = (EdgeConfigure *) malloc(sizeof(EdgeConfigure));
    VERIFY_NON_NULL_NR(config);
    config->recvCallback = (ReceivedMessageCallback *) malloc(sizeof(ReceivedMessageCallback));
    if (IS_NULL(config->recvCallback))
    {
        printf("Error :: calloc failed for config->recvCallback in init server\n");
        FREE(config);
        return;
    }
    config->recvCallback->resp_msg_cb = response_msg_cb;
    config->recvCallback->monitored_msg_cb = monitored_msg_cb;
    config->recvCallback->error_msg_cb = error_msg_cb;
    config->recvCallback->browse_msg_cb = browse_msg_cb;

    config->statusCallback = (StatusCallback *) malloc(sizeof(StatusCallback));
    if (IS_NULL(config->statusCallback))
    {
        printf("Error :: calloc failed for config->statusCallback in init server\n");
        if (IS_NOT_NULL(config))
        {
            FREE(config->recvCallback);
            FREE(config);
        }
        return;
    }
    config->statusCallback->start_cb = status_start_cb;
    config->statusCallback->stop_cb = status_stop_cb;
    config->statusCallback->network_cb = status_network_cb;

    config->discoveryCallback = (DiscoveryCallback *) malloc(sizeof(DiscoveryCallback));
    if (IS_NULL(config->recvCallback))
    {
        printf("Error :: calloc failed for config->recvCallback in init server\n");
        if (IS_NOT_NULL(config))
        {
            FREE(config->recvCallback);
            FREE(config->statusCallback);
            FREE(config);
        }
        return;
    }
    config->discoveryCallback->endpoint_found_cb = endpoint_found_cb;
    config->discoveryCallback->device_found_cb = device_found_cb;

    registerCallbacks(config);
}

static void startServer()
{
    EdgeEndpointConfig *endpointConfig = (EdgeEndpointConfig *) calloc(1,
            sizeof(EdgeEndpointConfig));
    VERIFY_NON_NULL_NR(endpointConfig);
    endpointConfig->bindAddress = ipAddress;
    endpointConfig->bindPort = 12686;
    endpointConfig->serverName = DEFAULT_SERVER_NAME_VALUE;
    //endpointConfig->requestTimeout = 60000;

    printf(COLOR_GREEN "[Endpoint Configuration]\n" COLOR_RESET);
    printf("\nBind Address : %s", endpointConfig->bindAddress);
    printf("\nBind Port : %d\n", endpointConfig->bindPort);

    EdgeApplicationConfig *appConfig = (EdgeApplicationConfig *) calloc(1,
            sizeof(EdgeApplicationConfig));
    if (IS_NULL(appConfig))
    {
        printf("Error :: calloc failed for appConfig in start server\n");
        FREE(endpointConfig);
        return;
    }
    appConfig->applicationName = DEFAULT_SERVER_APP_NAME_VALUE;
    appConfig->applicationUri = DEFAULT_SERVER_APP_URI_VALUE;
    appConfig->productUri = DEFAULT_PRODUCT_URI_VALUE;

    printf(COLOR_GREEN "\n[Application Configuration]\n" COLOR_RESET);
    printf("\nApplication Name : %s", appConfig->applicationName);
    printf("\nApplication Uri : %s", appConfig->applicationUri);
    printf("\nProudct Uri  : %s\n", appConfig->productUri);

    epInfo->endpointConfig = endpointConfig;
    epInfo->appConfig = appConfig;
    epInfo->securityPolicyUri = NULL;

    // Commented - For future message queue handling
    /*EdgeMessage *msg = (EdgeMessage *) malloc(sizeof(EdgeMessage));
     msg->endpointInfo = ep;
     msg->command = CMD_START_SERVER;
     msg->type = SEND_REQUEST;*/

    //send(msg);
    createServer(epInfo);
}

static void stopServer()
{
    FREE(epInfo->endpointConfig);
    FREE(epInfo->appConfig);

    // Commented - For future message queue handling
    /*EdgeMessage *msg = (EdgeMessage *) malloc(sizeof(EdgeMessage));
     msg->endpointInfo = ep;
     msg->command = CMD_STOP_SERVER;
     msg->type = SEND_REQUEST;*/

    //send(msg);
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
    item = createVariableNodeItem("String1", String, "test1", VARIABLE_NODE);
    VERIFY_NON_NULL_NR(item);
    createNode(DEFAULT_NAMESPACE_VALUE, item);
    printf("\n|------------[Added]  %s\n", item->browseName);
    deleteNodeItem(item);

    item = createVariableNodeItem("String2", String, "test2", VARIABLE_NODE);
    VERIFY_NON_NULL_NR(item);
    createNode(DEFAULT_NAMESPACE_VALUE, item);
    printf("\n|------------[Added] %s\n", item->browseName);
    deleteNodeItem(item);

    item = createVariableNodeItem("String3", String, "test3", VARIABLE_NODE);
    VERIFY_NON_NULL_NR(item);
    createNode(DEFAULT_NAMESPACE_VALUE, item);
    printf("\n|------------[Added] %s\n", item->browseName);
    deleteNodeItem(item);

    VERIFY_NON_NULL_NR(item);
    printf("\n[%d] Variable node with XML ELEMENT variant: \n", ++index);
    UA_XmlElement *xml_value = (UA_XmlElement *) malloc(sizeof(UA_XmlElement));
    if (IS_NOT_NULL(xml_value))
    {
        xml_value->length = 2;
        xml_value->data = (UA_Byte *) "ab";
        item = createVariableNodeItem("xml_value", XmlElement, (void *) xml_value, VARIABLE_NODE);
        VERIFY_NON_NULL_NR(item);
        createNode(DEFAULT_NAMESPACE_VALUE, item);
        printf("\n|------------[Added] %s\n", item->browseName);
        FREE(xml_value);
        deleteNodeItem(item);
    }
    else
    {
        printf("Error :: malloc failed for UA_XmlElement in Test create Nodes\n");
    }

    printf("\n[%d] Variable node with localized text variant: \n", ++index);
    UA_LocalizedText *lt_value = (UA_LocalizedText *) malloc(sizeof(UA_LocalizedText));
    if (IS_NOT_NULL(lt_value))
    {
        lt_value->locale = UA_STRING_ALLOC("COUNTRY");
        lt_value->text = UA_STRING_ALLOC("INDIA");
        item = createVariableNodeItem("LocalizedText", LocalizedText, (void *) lt_value, VARIABLE_NODE);
        VERIFY_NON_NULL_NR(item);
        createNode(DEFAULT_NAMESPACE_VALUE, item);
        printf("\n|------------[Added] %s\n", item->browseName);
        FREE(lt_value->locale.data);
        FREE(lt_value->text.data);
        FREE(lt_value);
        deleteNodeItem(item);
    }
    else
    {
        printf("Error :: malloc failed for UA_LocalizedText in Test create Nodes\n");
    }

    printf("\n[%d] Variable node with byte string variant: \n", ++index);
    UA_ByteString *bs_value = (UA_ByteString *) malloc(sizeof(UA_ByteString));
    if (IS_NOT_NULL(bs_value))
    {
        bs_value->length = 7;
        bs_value->data = (UA_Byte *) "samsung";
        item = createVariableNodeItem("ByteString", ByteString, (void *) bs_value, VARIABLE_NODE);
        VERIFY_NON_NULL_NR(item);
        createNode(DEFAULT_NAMESPACE_VALUE, item);
        printf("\n|------------[Added] %s\n", item->browseName);
        FREE(bs_value);
        deleteNodeItem(item);
    }
    else
    {
        printf("Error :: malloc failed for UA_ByteString in Test create Nodes\n");
    }

    printf("\n[%d] Variable node with byte variant: \n", ++index);
    UA_Byte b_value = 2;
    item = createVariableNodeItem("Byte", Byte, (void *) &b_value, VARIABLE_NODE);
    VERIFY_NON_NULL_NR(item);
    createNode(DEFAULT_NAMESPACE_VALUE, item);
    printf("\n|------------[Added] %s\n", item->browseName);
    deleteNodeItem(item);

    printf("\n[%d] Variable node with float variant: \n", ++index);
    float f_value = 4.4;
    item = createVariableNodeItem("Float", Float, (void *) &f_value, VARIABLE_NODE);
    VERIFY_NON_NULL_NR(item);
    createNode(DEFAULT_NAMESPACE_VALUE, item);
    printf("\n|------------[Added] %s\n", item->browseName);
    deleteNodeItem(item);

    printf("\n[%d] Variable node with int variant: \n", ++index);
    int value = 30;
    item = createVariableNodeItem("UInt16", UInt16, (void *) &value, VARIABLE_NODE);
    VERIFY_NON_NULL_NR(item);
    createNode(DEFAULT_NAMESPACE_VALUE, item);
    printf("\n|------------[Added] %s\n", item->browseName);
    deleteNodeItem(item);

    printf("\n[%d] Variable node with UInt32 variant: \n", ++index);
    value = 444;
    item = createVariableNodeItem("UInt32", UInt32, (void *) &value, VARIABLE_NODE);
    VERIFY_NON_NULL_NR(item);
    createNode(DEFAULT_NAMESPACE_VALUE, item);
    printf("\n|------------[Added] %s\n", item->browseName);
    deleteNodeItem(item);

    printf("\n[%d] Variable node with UInt64 variant: \n", ++index);
    value = 3445516;
    item = createVariableNodeItem("UInt64", UInt64, (void *) &value, VARIABLE_NODE);
    VERIFY_NON_NULL_NR(item);
    createNode(DEFAULT_NAMESPACE_VALUE, item);
    printf("\n|------------[Added] %s\n", item->browseName);
    deleteNodeItem(item);

    printf("\n[%d] Variable node with Int16 variant: \n", ++index);
    value = 4;
    item = createVariableNodeItem("Int16", Int16, (void *) &value, VARIABLE_NODE);
    VERIFY_NON_NULL_NR(item);
    createNode(DEFAULT_NAMESPACE_VALUE, item);
    printf("\n|------------[Added] %s\n", item->browseName);
    deleteNodeItem(item);

    printf("\n[%d] Variable node with Int32 variant: \n", ++index);
    value = 40;
    item = createVariableNodeItem("Int32", Int32, (void *) &value, VARIABLE_NODE);
    VERIFY_NON_NULL_NR(item);
    createNode(DEFAULT_NAMESPACE_VALUE, item);
    printf("\n|------------[Added] %s\n", item->browseName);
    deleteNodeItem(item);

    item = (EdgeNodeItem *) calloc(1, sizeof(EdgeNodeItem));
    VERIFY_NON_NULL_NR(item);
    printf("\n[%d] Variable node with Int64 variant: \n", ++index);
    value = 32700;
    item = createVariableNodeItem("Int64", Int64, (void *) &value, VARIABLE_NODE);
    VERIFY_NON_NULL_NR(item);
    createNode(DEFAULT_NAMESPACE_VALUE, item);
    printf("\n|------------[Added] %s\n", item->browseName);
    deleteNodeItem(item);

    printf("\n[%d] Variable node with UInt32 variant: \n", ++index);
    uint32_t int32_val = 4456;
    item = createVariableNodeItem("UInt32writeonly", UInt32, (void *) &int32_val, VARIABLE_NODE);
    VERIFY_NON_NULL_NR(item);
    item->accessLevel = WRITE;
    item->userAccessLevel = WRITE;
    createNode(DEFAULT_NAMESPACE_VALUE, item);
    printf("\n|------------[Added] %s\n", item->browseName);
    deleteNodeItem(item);

    printf("\n[%d] Variable node with UInt64 variant: \n", ++index);
    int64_t int64_val = 3270000;
    item = createVariableNodeItem("UInt64readonly", UInt64, (void *) &int64_val, VARIABLE_NODE);
    VERIFY_NON_NULL_NR(item);
    item->userAccessLevel = READ;
    item->accessLevel = READ;
    createNode(DEFAULT_NAMESPACE_VALUE, item);
    printf("\n|------------[Added] %s\n", item->browseName);
    deleteNodeItem(item);

    printf("\n[%d] Variable node with double variant: \n", ++index);
    double d_val = 50.4;
    item = createVariableNodeItem("Double", Double, (void *) &d_val, VARIABLE_NODE);
    VERIFY_NON_NULL_NR(item);
    createNode(DEFAULT_NAMESPACE_VALUE, item);
    printf("\n|------------[Added] %s\n", item->browseName);
    deleteNodeItem(item);

    printf("\n[%d] Variable node with boolean variant: \n", ++index);
    bool flag = true;
    item = createVariableNodeItem("Boolean", Boolean, (void *) &flag, VARIABLE_NODE);
    VERIFY_NON_NULL_NR(item);
    createNode(DEFAULT_NAMESPACE_VALUE, item);
    printf("\n|------------[Added] %s\n", item->browseName);
    deleteNodeItem(item);

    printf("\n[%d] Variable node with dateTime variant: \n", ++index);
    UA_DateTime time = UA_DateTime_now();
    item = createVariableNodeItem("DateTime", DateTime, (void *) &time, VARIABLE_NODE);
    VERIFY_NON_NULL_NR(item);
    createNode(DEFAULT_NAMESPACE_VALUE, item);
    printf("\n|------------[Added] %s\n", item->browseName);
    deleteNodeItem(item);

    printf("\n[%d] Variable node with SByte variant: \n", ++index);
    UA_SByte sbyte = 2;
    item = createVariableNodeItem("SByte", SByte, (void *) &sbyte, VARIABLE_NODE);
    VERIFY_NON_NULL_NR(item);
    createNode(DEFAULT_NAMESPACE_VALUE, item);
    printf("\n|------------[Added] %s\n", item->browseName);
    deleteNodeItem(item);

    printf("\n[%d] Variable node with GUID variant: \n", ++index);
    UA_Guid guid =
    { 1, 0, 1,
    { 0, 0, 0, 0, 1, 1, 1, 1 } };
    item = createVariableNodeItem("Guid", Guid, (void *) &guid, VARIABLE_NODE);
    VERIFY_NON_NULL_NR(item);
    createNode(DEFAULT_NAMESPACE_VALUE, item);
    printf("\n|------------[Added] %s\n", item->browseName);
    deleteNodeItem(item);

    printf("\n[%d] Variable node with qualified name variant: \n", ++index);
    UA_QualifiedName *qn_value = (UA_QualifiedName *) malloc(sizeof(UA_QualifiedName));
    if (IS_NOT_NULL(qn_value))
    {
        UA_String str = UA_STRING_ALLOC("qualifiedName");
        qn_value->namespaceIndex = 2;
        qn_value->name = str;
        item = createVariableNodeItem("QualifiedName", QualifiedName, (void *) qn_value, VARIABLE_NODE);
        VERIFY_NON_NULL_NR(item);
        createNode(DEFAULT_NAMESPACE_VALUE, item);
        printf("\n|------------[Added] %s\n", item->browseName);
        FREE(str.data);
        FREE(qn_value);
        deleteNodeItem(item);
    }
    else
    {
        printf("Error :: malloc failed for UA_QualifiedName in Test create Nodes\n");
    }

    printf("\n[%d] Variable node with NODEID variant: \n", ++index);
    UA_NodeId node = UA_NODEID_NUMERIC(DEFAULT_NAMESPACE_INDEX, RootFolder);
    item = createVariableNodeItem("NodeId", NodeId, (void *) &node, VARIABLE_NODE);
    VERIFY_NON_NULL_NR(item);
    createNode(DEFAULT_NAMESPACE_VALUE, item);
    printf("\n|------------[Added] %s\n", item->browseName);
    deleteNodeItem(item);

    /******************* Array *********************/
    printf(COLOR_GREEN "\n[Create Array Node]\n" COLOR_RESET);
    printf("\n[%d] Array node with ByteString values: \n", ++index);
    UA_ByteString **dataArray = (UA_ByteString **) malloc(sizeof(UA_ByteString *) * 5);
    if (IS_NOT_NULL(dataArray))
    {
        dataArray[0] = (UA_ByteString *) malloc(sizeof(UA_ByteString));
        dataArray[1] = (UA_ByteString *) malloc(sizeof(UA_ByteString));
        dataArray[2] = (UA_ByteString *) malloc(sizeof(UA_ByteString));
        dataArray[3] = (UA_ByteString *) malloc(sizeof(UA_ByteString));
        dataArray[4] = (UA_ByteString *) malloc(sizeof(UA_ByteString));
        if (IS_NOT_NULL(dataArray[0]) && IS_NOT_NULL(dataArray[1]) && IS_NOT_NULL(dataArray[2])
            && IS_NOT_NULL(dataArray[3]) && IS_NOT_NULL(dataArray[4]))
        {
            *dataArray[0] = UA_BYTESTRING_ALLOC("abcde");
            *dataArray[1] = UA_BYTESTRING_ALLOC("fghij");
            *dataArray[2] = UA_BYTESTRING_ALLOC("klmno");
            *dataArray[3] = UA_BYTESTRING_ALLOC("pqrst");
            *dataArray[4] = UA_BYTESTRING_ALLOC("uvwxyz");
            item = createVariableNodeItem("ByteStringArray", ByteString, (void *) dataArray, VARIABLE_NODE);
            VERIFY_NON_NULL_NR(item);
            item->nodeType = ARRAY_NODE;
            item->arrayLength = 5;
            createNode(DEFAULT_NAMESPACE_VALUE, item);
            printf("\n|------------[Added] %s\n", item->browseName);
        }
        else
        {
            printf(
                    "Error :: malloc failed for UA_ByteString dataArray INDEX in Test create Nodes\n");
        }

        for (int i = 0; i < 5; i++)
        {
            UA_ByteString temp = *dataArray[i];
            FREE(temp.data);
            FREE(dataArray[i]);
        }
        FREE(dataArray);
        deleteNodeItem(item);
    }
    else
    {
        printf("Error :: malloc failed for UA_ByteString dataArray in Test create Nodes\n");
    }

    printf("\n[%d] Array node with Boolean values: \n", ++index);
    bool *arr = (bool *) malloc(sizeof(bool) * 5);
    if (IS_NOT_NULL(arr))
    {
        arr[0] = true;
        arr[1] = false;
        arr[2] = true;
        arr[3] = false;
        arr[4] = true;
        item = createVariableNodeItem("BoolArray", Boolean, (void *) arr, VARIABLE_NODE);
        VERIFY_NON_NULL_NR(item);
        item->nodeType = ARRAY_NODE;
        item->arrayLength = 5;
        createNode(DEFAULT_NAMESPACE_VALUE, item);
        printf("\n|------------[Added] %s\n", item->browseName);
        FREE(arr);
        deleteNodeItem(item);
    }
    else
    {
        printf("Error :: malloc failed for bool array in Test create Nodes\n");
    }

    printf("\n[%d] Array node with SByte values: \n", ++index);
    UA_SByte *sbData = (UA_SByte *) malloc(sizeof(UA_SByte) * 5);
    if (IS_NOT_NULL(sbData))
    {
        sbData[0] = -128;
        sbData[1] = 112;
        sbData[2] = 120;
        sbData[3] = 122;
        sbData[4] = 127;
        item = createVariableNodeItem("SByteArray", SByte, (void *) sbData, VARIABLE_NODE);
        VERIFY_NON_NULL_NR(item);
        item->nodeType = ARRAY_NODE;
        item->arrayLength = 5;
        createNode(DEFAULT_NAMESPACE_VALUE, item);
        printf("\n|------------[Added] %s\n", item->browseName);
        FREE(sbData);
        deleteNodeItem(item);
    }
    else
    {
        printf("Error :: malloc failed for UA_SByte array in Test create Nodes\n");
    }

    printf("\n[%d] Array node with Int32 values: \n", ++index);
    int *intData = (int *) malloc(sizeof(int) * 7);
    if (IS_NOT_NULL(intData))
    {
        intData[0] = 11;
        intData[1] = 22;
        intData[2] = 33;
        intData[3] = 44;
        intData[4] = 55;
        intData[5] = 66;
        intData[6] = 77;
        item = createVariableNodeItem("int32Array", Int32, (void *) intData, VARIABLE_NODE);
        VERIFY_NON_NULL_NR(item);
        item->nodeType = ARRAY_NODE;
        item->arrayLength = 7;
        createNode(DEFAULT_NAMESPACE_VALUE, item);
        printf("\n|------------[Added] %s\n", item->browseName);
        FREE(intData);
        deleteNodeItem(item);
    }
    else
    {
        printf("Error :: malloc failed for int Array in Test create Nodes\n");
    }

    printf("\n[%d] Array node with Int64 values: \n", ++index);
    int *int64Data = (int *) malloc(sizeof(int) * 5);
    if (IS_NOT_NULL(int64Data))
    {
        int64Data[0] = 11111;
        int64Data[1] = 22222;
        int64Data[2] = 33333;
        int64Data[3] = 44444;
        int64Data[4] = 55555;
        item = createVariableNodeItem("int64Array", Int64, (void *) int64Data, VARIABLE_NODE);
        VERIFY_NON_NULL_NR(item);
        item->nodeType = ARRAY_NODE;
        item->arrayLength = 5;
        createNode(DEFAULT_NAMESPACE_VALUE, item);
        printf("\n|------------[Added] %s\n", item->browseName);
        FREE(int64Data);
        deleteNodeItem(item);
    }
    else
    {
        printf("Error :: malloc failed for int64Data Array in Test create Nodes\n");
    }

    printf("\n[%d] Array node with double values: \n", ++index);
    double *data = (double *) malloc(sizeof(double) * 5);
    if (IS_NOT_NULL(data))
    {
        data[0] = 10.2;
        data[1] = 20.2;
        data[2] = 30.2;
        data[3] = 40.2;
        data[4] = 50.2;
        item = createVariableNodeItem("DoubleArray", Double, (void *) data, VARIABLE_NODE);
        VERIFY_NON_NULL_NR(item);
        item->nodeType = ARRAY_NODE;
        item->arrayLength = 5;
        createNode(DEFAULT_NAMESPACE_VALUE, item);
        printf("\n|------------[Added] %s\n", item->browseName);
        FREE(data);
        deleteNodeItem(item);
    }
    else
    {
        printf("Error :: malloc failed for double Array in Test create Nodes\n");
    }

    printf("\n[%d] Array node with string values: \n", ++index);
    char **data1 = (char **) malloc(sizeof(char *) * 5);
    if (IS_NOT_NULL(data1))
    {
        data1[0] = (char *) malloc(10);
        data1[1] = (char *) malloc(10);
        data1[2] = (char *) malloc(10);
        data1[3] = (char *) malloc(10);
        data1[4] = (char *) malloc(10);

        if (IS_NOT_NULL(data1[0]) && IS_NOT_NULL(data1[1]) && IS_NOT_NULL(data1[2])
            && IS_NOT_NULL(data1[3]) && IS_NOT_NULL(data1[4]))
        {
            strncpy(data1[0], "apple", strlen("apple"));
            data1[0][strlen("apple")] = '\0';
            strncpy(data1[1], "ball", strlen("ball"));
            data1[0][strlen("ball")] = '\0';
            strncpy(data1[2], "cats", strlen("cats"));
            data1[0][strlen("cats")] = '\0';
            strncpy(data1[3], "dogs", strlen("dogs"));
            data1[0][strlen("dogs")] = '\0';
            strncpy(data1[4], "elephant", strlen("elephant"));
            data1[0][strlen("elephant")] = '\0';

            item = createVariableNodeItem("CharArray", String, (void *) data1, VARIABLE_NODE);
            VERIFY_NON_NULL_NR(item);
            item->nodeType = ARRAY_NODE;
            item->arrayLength = 5;
            createNode(DEFAULT_NAMESPACE_VALUE, item);
            printf("\n|------------[Added] %s\n", item->browseName);

            for (int i = 0; i < 5; i++)
            {
                FREE(data1[i]);
            }
            FREE(data1);
            deleteNodeItem(item);
        }
        else
        {
            printf("Error :: malloc failed for char dataArray in Test create Nodes\n");
        }
    }
    else
    {
        printf("Error :: malloc failed for char Array in Test create Nodes\n");
    }

    printf("\n[%d] Variable node with byte array variant: \n", ++index);
    UA_Byte *b_arrvalue = (UA_Byte *) calloc(1, sizeof(UA_Byte) * 5);
    if (IS_NOT_NULL(b_arrvalue))
    {
        b_arrvalue[0] = 0x11;
        b_arrvalue[1] = 0x22;
        b_arrvalue[2] = 0x33;
        b_arrvalue[3] = 0x44;
        b_arrvalue[4] = 0x55;
        item = createVariableNodeItem("ByteArray", Byte, (void *) b_arrvalue, VARIABLE_NODE);
        VERIFY_NON_NULL_NR(item);
        item->arrayLength = 5;
        item->nodeType = ARRAY_NODE;
        createNode(DEFAULT_NAMESPACE_VALUE, item);
        printf("\n|------------[Added] %s\n", item->browseName);
        FREE(b_arrvalue);
        deleteNodeItem(item);
    }
    else
    {
        printf("Error :: malloc failed for UA_Byte Array in Test create Nodes\n");
    }

    /******************* Object Node *********************/
    printf(COLOR_GREEN"\n[Create Object Node]\n"COLOR_RESET);
    printf("\n[%d] Object node : \"Object1\"\n", ++index);
    EdgeNodeId *edgeNodeId = (EdgeNodeId *) calloc(1, sizeof(EdgeNodeId));
    if (IS_NOT_NULL(edgeNodeId))
    {
        item = createNodeItem("Object1", OBJECT_NODE, edgeNodeId);
        VERIFY_NON_NULL_NR(item);
        createNode(DEFAULT_NAMESPACE_VALUE, item);
        printf("\n|------------[Added] %s\n", item->browseName);
        FREE(item->sourceNodeId);
        deleteNodeItem(item);
    }
    else
    {
        printf("Error :: malloc failed for Object1 in Test create Nodes\n");
    }

    printf("\n[%d] Object node : \"Object2\" with source Node \"Object1\"\n", ++index);
    edgeNodeId = (EdgeNodeId *) malloc(sizeof(EdgeNodeId));
    if (IS_NOT_NULL(edgeNodeId))
    {
        edgeNodeId->nodeId = "Object1";
        item = createNodeItem("Object2", OBJECT_NODE, edgeNodeId);
        VERIFY_NON_NULL_NR(item);
        createNode(DEFAULT_NAMESPACE_VALUE, item);
        printf("\n|------------[Added] %s\n", item->browseName);
        FREE(edgeNodeId);
        deleteNodeItem(item);
    }
    else
    {
        printf("Error :: malloc failed for Object2 in Test create Nodes\n");
    }

    /******************* View Node *********************/
    printf(COLOR_GREEN"\n[Create View Node]\n"COLOR_RESET);
    printf("\n[%d] View Node with ViewNode1\n", ++index);
    edgeNodeId = (EdgeNodeId *) calloc(1, sizeof(EdgeNodeId));
    if (IS_NOT_NULL(edgeNodeId))
    {
        item = createNodeItem("ViewNode1", VIEW_NODE, edgeNodeId);
        VERIFY_NON_NULL_NR(item);
        createNode(DEFAULT_NAMESPACE_VALUE, item);
        printf("\n|------------[Added] %s\n", item->browseName);
        FREE(edgeNodeId);
        deleteNodeItem(item);
    }
    else
    {
        printf("Error :: malloc failed for ViewNode1 in Test create Nodes\n");
    }

    printf("\n[%d] View Node with ViewNode2\n", ++index);
    edgeNodeId = (EdgeNodeId *) malloc(sizeof(EdgeNodeId));
    if (IS_NOT_NULL(edgeNodeId))
    {
        edgeNodeId->nodeId = "ViewNode1";
        item = createNodeItem("ViewNode2", VIEW_NODE, edgeNodeId);
        VERIFY_NON_NULL_NR(item);
        createNode(DEFAULT_NAMESPACE_VALUE, item);
        printf("\n|------------[Added] %s\n", item->browseName);
        FREE(edgeNodeId);
        deleteNodeItem(item);
    }
    else
    {
        printf("Error :: malloc failed for ViewNode2 in Test create Nodes\n");
    }

    /******************* Object Type Node *********************/
    printf(COLOR_GREEN"\n[Create Object Type Node]\n"COLOR_RESET);
    printf("\n[%d] Object Type node : \"ObjectType1\"\n", ++index);
    edgeNodeId = (EdgeNodeId *) malloc(sizeof(EdgeNodeId));
    if (IS_NOT_NULL(edgeNodeId))
    {
        edgeNodeId->nodeId = NULL; // no source node
        item = createNodeItem("ObjectType1", OBJECT_TYPE_NODE, edgeNodeId);
        VERIFY_NON_NULL_NR(item);
        createNode(DEFAULT_NAMESPACE_VALUE, item);
        printf("\n|------------[Added] %s\n", item->browseName);
        FREE(edgeNodeId);
        deleteNodeItem(item);
    }
    else
    {
        printf("Error :: malloc failed for ObjectType1 in Test create Nodes\n");
    }

    printf("\n[%d] Object Type node : \"ObjectType2\" with source Node \"ObjectType1\"\n", ++index);
    edgeNodeId = (EdgeNodeId *) malloc(sizeof(EdgeNodeId));
    if (IS_NOT_NULL(edgeNodeId))
    {
        edgeNodeId->nodeId = "ObjectType1";
        item = createNodeItem("ObjectType2", OBJECT_TYPE_NODE, edgeNodeId);
        VERIFY_NON_NULL_NR(item);
        createNode(DEFAULT_NAMESPACE_VALUE, item);
        printf("\n|------------[Added] %s\n", item->browseName);
        FREE(edgeNodeId);
        deleteNodeItem(item);
    }
    else
    {
        printf("Error :: malloc failed for ObjectType2 in Test create Nodes\n");
    }

    printf("\n[%d] Object Type node : \"ObjectType3\" with source Node \"ObjectType2\"\n", ++index);
    edgeNodeId = (EdgeNodeId *) malloc(sizeof(EdgeNodeId));
    if (IS_NOT_NULL(edgeNodeId))
    {
        edgeNodeId->nodeId = "ObjectType1";
        item = createNodeItem("ObjectType3", OBJECT_TYPE_NODE, edgeNodeId);
        VERIFY_NON_NULL_NR(item);
        createNode(DEFAULT_NAMESPACE_VALUE, item);
        printf("\n|------------[Added] %s\n", item->browseName);
        FREE(edgeNodeId);
        deleteNodeItem(item);
    }
    else
    {
        printf("Error :: malloc failed for ObjectType3 in Test create Nodes\n");
    }

    printf("\n[%d] Object Type node : \"ObjectType4\" with source Node \"ObjectType3\"\n", ++index);
    edgeNodeId = (EdgeNodeId *) malloc(sizeof(EdgeNodeId));
    if (IS_NOT_NULL(edgeNodeId))
    {
        edgeNodeId->nodeId = "ObjectType1";
        item = createNodeItem("ObjectType4", OBJECT_TYPE_NODE, edgeNodeId);
        VERIFY_NON_NULL_NR(item);
        createNode(DEFAULT_NAMESPACE_VALUE, item);
        printf("\n|------------[Added] %s\n", item->browseName);
        FREE(edgeNodeId);
        deleteNodeItem(item);
    }
    else
    {
        printf("Error :: malloc failed for ObjectType4 in Test create Nodes\n");
    }

    printf("\n[%d] Object Type node : \"ObjectType5\" with source Node \"ObjectType3\"\n", ++index);
    edgeNodeId = (EdgeNodeId *) malloc(sizeof(EdgeNodeId));
    if (IS_NOT_NULL(edgeNodeId))
    {
        edgeNodeId->nodeId = "ObjectType1";
        item = createNodeItem("ObjectType5", OBJECT_TYPE_NODE, edgeNodeId);
        VERIFY_NON_NULL_NR(item);
        createNode(DEFAULT_NAMESPACE_VALUE, item);
        printf("\n|------------[Added] %s\n", item->browseName);
        FREE(edgeNodeId);
        deleteNodeItem(item);
    }
    else
    {
        printf("Error :: malloc failed for ObjectType5 in Test create Nodes\n");
    }

    /******************* Variable Type Node *********************/
    printf(COLOR_GREEN"\n[Create Variable Type Node]\n"COLOR_RESET);
    printf("\n[%d] Variable Type Node with Double Variable Type \n", ++index);
    double d[2] = { 10.2, 20.2 };
    item = createVariableNodeItem("DoubleVariableType", Double, (void *)d, VARIABLE_TYPE_NODE);
    VERIFY_NON_NULL_NR(item);
    item->arrayLength = 2;
    createNode(DEFAULT_NAMESPACE_VALUE, item);
    printf("\n|------------[Added] %s\n", item->browseName);
    deleteNodeItem(item);

    /******************* Data Type Node *********************/
    printf("\n[%d] Data Type Node with DataType1\n", ++index);
    edgeNodeId = (EdgeNodeId *) calloc(1, sizeof(EdgeNodeId));
    if (IS_NOT_NULL(edgeNodeId))
    {
        item = createNodeItem("DataType1", DATA_TYPE_NODE, edgeNodeId);
        VERIFY_NON_NULL_NR(item);
        createNode(DEFAULT_NAMESPACE_VALUE, item);
        printf("\n|------------[Added] %s\n", item->browseName);
        FREE(edgeNodeId);
        deleteNodeItem(item);
    }
    else
    {
        printf("Error :: malloc failed for DataType1 in Test create Nodes\n");
    }

    printf("\n[%d] Data Type Node with DataType2\n", ++index);
    edgeNodeId = (EdgeNodeId *) malloc(sizeof(EdgeNodeId));
    if (IS_NOT_NULL(edgeNodeId))
    {
        edgeNodeId->nodeId = "DataType1";
        item = createNodeItem("DataType2", DATA_TYPE_NODE, edgeNodeId);
        VERIFY_NON_NULL_NR(item);
        createNode(DEFAULT_NAMESPACE_VALUE, item);
        FREE(edgeNodeId);
    }
    else
    {
        printf("Error :: malloc failed for DataType2 in Test create Nodes\n");
    }

    /******************* Reference Type Node *********************/
    printf(COLOR_GREEN"\n[Create Reference Type Node]\n"COLOR_RESET);
    printf("\n[%d] Reference Type Node with ReferenceTypeNode1", ++index);
    edgeNodeId = (EdgeNodeId *) calloc(1, sizeof(EdgeNodeId));
    if (IS_NOT_NULL(edgeNodeId))
    {
        item = createNodeItem("ReferenceTypeNode1", REFERENCE_TYPE_NODE, edgeNodeId);
        VERIFY_NON_NULL_NR(item);
        createNode(DEFAULT_NAMESPACE_VALUE, item);
        printf("\n|------------[Added] %s\n", item->browseName);
        FREE(edgeNodeId);
    }
    else
    {
        printf("Error :: malloc failed for ReferenceTypeNode1 in Test create Nodes\n");
    }

    printf("\n[%d] Reference Type Node with source node\"ReferenceTypeNode1\"\n", ++index);
    edgeNodeId = (EdgeNodeId *) malloc(sizeof(EdgeNodeId));
    if (IS_NOT_NULL(edgeNodeId))
    {
        edgeNodeId->nodeId = "ReferenceTypeNode1";
        item = createNodeItem("ReferenceTypeNode2", REFERENCE_TYPE_NODE, edgeNodeId);
        VERIFY_NON_NULL_NR(item);
        createNode(DEFAULT_NAMESPACE_VALUE, item);
        printf("\n|------------[Added] %s\n", item->browseName);
        FREE(edgeNodeId);
    }
    else
    {
        printf("Error :: malloc failed for ReferenceTypeNode2 in Test create Nodes\n");
    }

    deleteNodeItem(item);

    /******************* Method Node *********************/
    printf(COLOR_GREEN"\n[Create Method Node]\n"COLOR_RESET);
    printf("\n[%d] Method Node with square_root \n", ++index);
    EdgeNodeItem *methodNodeItem = (EdgeNodeItem *) malloc(sizeof(EdgeNodeItem));
    VERIFY_NON_NULL_NR(methodNodeItem);
    methodNodeItem->browseName = "sqrt(x)";
    methodNodeItem->sourceNodeId = NULL;

    EdgeMethod *method = (EdgeMethod *) malloc(sizeof(EdgeMethod));
    if (IS_NULL(method))
    {
        FREE(methodNodeItem);
        FREE(method);
        printf("Error :: malloc failed for method square_root  in Test create Nodes\n");
        return;
    }
    method->description = "Calculate square root";
    method->methodNodeName = "square_root";
    method->method_fn = test_method_sqrt;
    method->num_inpArgs = 1;
    method->inpArg = (EdgeArgument **) malloc(sizeof(EdgeArgument *) * method->num_inpArgs);
    if (IS_NULL(method->inpArg))
    {
        FREE(methodNodeItem);
        FREE(method);
        printf("Error :: malloc failed for method method->inpArg  in Test create Nodes\n");
        return;
    }
    for (int idx = 0; idx < method->num_inpArgs; idx++)
    {
        method->inpArg[idx] = (EdgeArgument *) malloc(sizeof(EdgeArgument));
        if (IS_NULL(method->inpArg[idx]))
        {
            for (int j = 0; j < idx; j++)
            {
                if(IS_NOT_NULL(method->inpArg[j]))
                {
                    FREE(method->inpArg[j]);
                }
            }
            FREE(method->inpArg);
            FREE(methodNodeItem);
            FREE(method);
            printf("Error :: malloc failed for method method->inpArg[%d]  in Test create Nodes\n",
                    idx);
            return;
        }
        method->inpArg[idx]->argType = Double;
        method->inpArg[idx]->valType = SCALAR;
    }

    method->num_outArgs = 1;
    method->outArg = (EdgeArgument **) malloc(sizeof(EdgeArgument *) * method->num_outArgs);
    if (IS_NULL(method->outArg))
    {
        FREE(methodNodeItem);
        FREE(method);
        printf("Error :: malloc failed for method method->outArg  in Test create Nodes\n");
        return;
    }
    for (int idx = 0; idx < method->num_outArgs; idx++)
    {
        method->outArg[idx] = (EdgeArgument *) malloc(sizeof(EdgeArgument));
        if (IS_NULL(method->outArg[idx]))
        {
            for (int j = 0; j < idx; j++)
            {
                if(IS_NOT_NULL(method->outArg[j]))
                {
                    FREE(method->outArg[j]);
                }
            }
            
            FREE(method->outArg);
            FREE(methodNodeItem);
            FREE(method);
            printf("Error :: malloc failed for method method->outArg[%d]  in Test create Nodes\n",
                    idx);
            return;
        }
        method->outArg[idx]->argType = Double;
        method->outArg[idx]->valType = SCALAR;
    }
    createMethodNode(DEFAULT_NAMESPACE_VALUE, methodNodeItem, method);
    printf("\n|------------[Added] %s\n", methodNodeItem->browseName);
    FREE(methodNodeItem);

    printf("\n[%d] Method Node with incrementInc32Array \n", ++index);
    EdgeNodeItem *methodNodeItem1 = (EdgeNodeItem *) malloc(sizeof(EdgeNodeItem));
    VERIFY_NON_NULL_NR(methodNodeItem1);
    methodNodeItem1->browseName = "incrementInc32Array(x,delta)";
    methodNodeItem1->sourceNodeId = NULL;

    EdgeMethod *method1 = (EdgeMethod *) malloc(sizeof(EdgeMethod));
    if (IS_NULL(method1))
    {
        FREE(methodNodeItem1);
        FREE(method1);
        printf("Error :: malloc failed for method incrementInc32Array  in Test create Nodes\n");
        return;
    }
    method1->description = "Increment int32 array by delta";
    method1->methodNodeName = "incrementInc32Array";
    method1->method_fn = test_method_increment_int32Array;

    method1->num_inpArgs = 2;
    method1->inpArg = (EdgeArgument **) malloc(sizeof(EdgeArgument *) * method1->num_inpArgs);
    if (IS_NULL(method1->inpArg))
    {
        FREE(methodNodeItem1);
        FREE(method1);
        printf("Error :: malloc failed for methodmethod1->inpArg  in Test create Nodes\n");
        return;
    }
    method1->inpArg[0] = (EdgeArgument *) malloc(sizeof(EdgeArgument));
    if (IS_NULL(method1->inpArg[0]))
    {
        FREE(method1->inpArg);
        FREE(methodNodeItem1);
        FREE(method1);
        printf("Error :: malloc failed for method method1->inpArg[0]  in Test create Nodes\n");
        return;
    }
    method1->inpArg[0]->argType = Int32;
    method1->inpArg[0]->valType = ARRAY_1D;
    method1->inpArg[0]->arrayLength = 5;

    method1->inpArg[1] = (EdgeArgument *) malloc(sizeof(EdgeArgument));
    if (IS_NULL(method1->inpArg[1]))
    {
        FREE(method1->inpArg[0]);
        FREE(method1->inpArg);
        FREE(methodNodeItem1);
        FREE(method1);
        printf("Error :: malloc failed for method method1->inpArg[1]  in Test create Nodes\n");
        return;
    }
    method1->inpArg[1]->argType = Int32;
    method1->inpArg[1]->valType = SCALAR;

    method1->num_outArgs = 1;
    method1->outArg = (EdgeArgument **) malloc(sizeof(EdgeArgument *) * method1->num_outArgs);
    if (IS_NULL(method1->outArg))
    {
        FREE(method1->inpArg[0]);
        FREE(method1->inpArg[1]);
        FREE(method1->inpArg);
        FREE(methodNodeItem1);
        FREE(method1);
        printf("Error :: malloc failed for method1->outArg  in Test create Nodes\n");
        return;
    }
    for (int idx = 0; idx < method1->num_outArgs; idx++)
    {
        method1->outArg[idx] = (EdgeArgument *) malloc(sizeof(EdgeArgument));
        if (IS_NULL(method1->outArg[idx]))
        {
            for (int j = 0; j < idx; j++)
            {
                if(IS_NOT_NULL(method->outArg[j]))
                {
                    FREE(method->outArg[j]);
                }
            }

            FREE(method1->inpArg[0]);
            FREE(method1->inpArg[1]);
            FREE(method1->inpArg);
            FREE(method1->outArg);
            FREE(methodNodeItem1);
            FREE(method1);
            printf("Error :: malloc failed for method method1->outArg[%d]  in Test create Nodes\n",
                    idx);
            return;
        }
        method1->outArg[idx]->argType = Int32;
        method1->outArg[idx]->valType = ARRAY_1D;
        method1->outArg[idx]->arrayLength = 5;
    }
    createMethodNode(DEFAULT_NAMESPACE_VALUE, methodNodeItem1, method1);
    printf("\n|------------[Added] %s\n", methodNodeItem1->browseName);
    FREE(methodNodeItem1);

    printf("\n[%d] Method Node with noArgMethod \n", ++index);
    EdgeNodeItem *methodNodeItem2 = (EdgeNodeItem *) malloc(sizeof(EdgeNodeItem));
    VERIFY_NON_NULL_NR(methodNodeItem2);
    methodNodeItem2->browseName = "shutdown()";
    methodNodeItem2->sourceNodeId = NULL;

    EdgeMethod *method2 = (EdgeMethod *) malloc(sizeof(EdgeMethod));
    if (IS_NULL(method2))
    {
        FREE(methodNodeItem2);
        printf("Error :: malloc failed for method shutdown in Test create Nodes\n");
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
    FREE(methodNodeItem2);

    printf("\n[%d] Method Node with inArgMethod \n", ++index);
    EdgeNodeItem *methodNodeItem3 = (EdgeNodeItem *) malloc(sizeof(EdgeNodeItem));
    VERIFY_NON_NULL_NR(methodNodeItem3);
    methodNodeItem3->browseName = "print(x)";
    methodNodeItem3->sourceNodeId = NULL;

    EdgeMethod *method3 = (EdgeMethod *) malloc(sizeof(EdgeMethod));
    if (IS_NULL(method3))
    {
        FREE(methodNodeItem3);
        printf("Error :: malloc failed for method printx  in Test create Nodes\n");
        return;
    }
    method3->description = "print x";
    method3->methodNodeName = "print";
    method3->method_fn = test_method_print;

    method3->num_inpArgs = 1;
    method3->inpArg = (EdgeArgument **) malloc(sizeof(EdgeArgument *) * method3->num_inpArgs);
    if (IS_NULL(method3->inpArg))
    {
        FREE(methodNodeItem3);
        FREE(method3);
        printf("Error :: malloc failed for method method3->inpArg  in Test create Nodes\n");
        return;
    }
    method3->inpArg[0] = (EdgeArgument *) malloc(sizeof(EdgeArgument));
    if (IS_NULL(method))
    {
        FREE(method3->inpArg);
        FREE(methodNodeItem3);
        FREE(method3);
        printf("Error :: malloc failed for method method3->inpArg[0]  in Test create Nodes\n");
        return;
    }
    method3->inpArg[0]->argType = Int32;
    method3->inpArg[0]->valType = SCALAR;

    method3->num_outArgs = 0;
    method3->outArg = NULL;

    createMethodNode(DEFAULT_NAMESPACE_VALUE, methodNodeItem3, method3);
    printf("\n|------------[Added] %s\n", methodNodeItem3->browseName);
    FREE(methodNodeItem3);

    printf("\n[%d] Method Node with outArgMethod \n", ++index);
    EdgeNodeItem *methodNodeItem4 = (EdgeNodeItem *) malloc(sizeof(EdgeNodeItem));
    VERIFY_NON_NULL_NR(methodNodeItem4);
    methodNodeItem4->browseName = "version()";
    methodNodeItem4->sourceNodeId = NULL;

    EdgeMethod *method4 = (EdgeMethod *) malloc(sizeof(EdgeMethod));
    if (IS_NULL(method4))
    {
        FREE(methodNodeItem4);
        printf("Error :: malloc failed for method version  in Test create Nodes\n");
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
        FREE(methodNodeItem4);
        FREE(method4);
        printf("Error :: malloc failed for method method4->outArg  in Test create Nodes\n");
        return;
    }
    method4->outArg[0] = (EdgeArgument *) malloc(sizeof(EdgeArgument));
    if (IS_NULL(method))
    {
        FREE(method4->outArg);
        FREE(methodNodeItem4);
        FREE(method4);
        printf("Error :: malloc failed for method method4->outArg[0]  in Test create Nodes\n");
        return;
    }
    method4->outArg[0]->argType = String;
    method4->outArg[0]->valType = SCALAR;

    createMethodNode(DEFAULT_NAMESPACE_VALUE, methodNodeItem4, method4);
    printf("\n|------------[Added] %s\n", methodNodeItem4->browseName);
    FREE(methodNodeItem4);

    /******************* Add Reference *********************/
    /************ Reference is not NODE CLASS ***************/
    printf(COLOR_GREEN"\n[Create Reference]\n"COLOR_RESET);
    printf("\n[%d] Make Reference that ViewNode1 node Organizes with ObjectType1\n", ++index);
    EdgeReference *reference = (EdgeReference *) malloc(sizeof(EdgeReference));
    if (IS_NOT_NULL(reference))
    {
        reference->forward = true;
        reference->sourceNamespace = DEFAULT_NAMESPACE_VALUE;
        reference->sourcePath = "ViewNode1";
        reference->targetNamespace = DEFAULT_NAMESPACE_VALUE;
        reference->targetPath = "ObjectType1";
        /* default reference ID : Organizes */
        addReference(reference);

        FREE(reference);
    }
    else
    {
        printf("Error :: malloc failed for EdgeReference in Test create Nodes\n");
    }

    printf("\n\n");
}

static void testModifyNode()
{

    printf("\n" COLOR_YELLOW "------------------------------------------------------" COLOR_RESET);
    printf("\n" COLOR_YELLOW "                  Modify Variable Node            "COLOR_RESET);
    printf("\n" COLOR_YELLOW "------------------------------------------------------" COLOR_RESET
    "\n\n");
    char s_value[512];
    double d_value;
    unsigned int u_value;
    int i_value;
    int option;
    void *new_value = NULL;
    char name[128];

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
    printf("[7] String1 Changing Thread\n");
    printf("[8] Int32 Increasing Thread\n");
    printf("\nEnter any of the above option :: ");
    scanf("%d", &option);

    if (option < 1 || option > 8)
    {
        printf("Invalid Option!!! \n\n");
        return;
    }

    printf("\nEnter the new value :: ");
    if (option == 1)
    {
        scanf("%s", s_value);
        strcpy(name, "String1");
        new_value = (void *) s_value;
    }
    else if (option == 2)
    {
        scanf("%s", s_value);
        strcpy(name, "String2");
        new_value = (void *) s_value;
    }
    else if (option == 3)
    {
        scanf("%s", s_value);
        strcpy(name, "String3");
        new_value = (void *) s_value;
    }
    else if (option == 4)
    {
        scanf("%lf", &d_value);
        strcpy(name, "Double");
        new_value = (void *) &d_value;
    }
    else if (option == 5)
    {
        scanf("%d", &i_value);
        strcpy(name, "Int32");
        new_value = (void *) &i_value;
    }
    else if (option == 6)
    {
        scanf("%u", &u_value);
        strcpy(name, "UInt16");
        new_value = (void *) &u_value;
    }
    else if (option == 7)
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
            EdgeVersatility *message = (EdgeVersatility *) malloc(sizeof(EdgeVersatility));
            if (IS_NOT_NULL(message))
            {
                message->value = new_value;
                modifyVariableNode(DEFAULT_NAMESPACE_VALUE, name, message);
                FREE(message);
                usleep(1000 * 1000);
            }
            else
            {
                printf("Error :: malloc failed for String1 EdgeVersatility in Test Modify Nodes\n");
            }
        }
        return;
    }
    else if (option == 8)
    {
        strncpy(name, "Int32", strlen("Int32"));
        name[strlen("Int32")] = '\0';
        for (int i = 0; i < MAX_TEST_NUMBER; i++)
        {
            new_value = (void *) &i;
            EdgeVersatility *message = (EdgeVersatility *) malloc(sizeof(EdgeVersatility));
            if (IS_NOT_NULL(message))
            {
                message->value = new_value;
                modifyVariableNode(DEFAULT_NAMESPACE_VALUE, name, message);
                FREE(message);
                usleep(1000 * 1000);
            }
            else
            {
                printf("Error :: malloc failed for Int32 EdgeVersatility in Test Modify Nodes\n");
            }
        }
        return;
    }

    EdgeVersatility *message = (EdgeVersatility *) malloc(sizeof(EdgeVersatility));
    if (IS_NOT_NULL(message))
    {
        message->value = new_value;
        modifyVariableNode(DEFAULT_NAMESPACE_VALUE, name, message);
        FREE(message);
    }
    else
    {
        printf("Error :: malloc failed for EdgeVersatility in Test Modify Nodes\n");
    }
}

static void deinit()
{
    if (startFlag)
    {
        stopServer();
        startFlag = false;

        if (IS_NOT_NULL(config))
        {
            FREE(config->recvCallback);
            FREE(config->statusCallback);
            FREE(config->discoveryCallback);

            FREE(config);
        }

        FREE(epInfo);
    }
}

static void print_menu()
{
    printf("=============== OPC UA =======================\n\n");

    printf("start : start opcua server\n");
    printf("modify_node : modify variable node\n");
    printf("quit : terminate/stop opcua server/client and then quit\n");
    printf("help : print menu\n");

    printf("\n=============== OPC UA =======================\n\n");
}

int main()
{
    char command[128];

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

            strncpy(ipAddress, "localhost", strlen("localhost"));
            ipAddress[strlen("localhost")] = '\0';

            snprintf(endpointUri, sizeof(endpointUri), "opc:tcp://%s:12686/edge-opc-server",
                    ipAddress);

            epInfo = (EdgeEndPointInfo *) calloc(1, sizeof(EdgeEndPointInfo));
            if (IS_NOT_NULL(epInfo))
            {
                epInfo->endpointUri = endpointUri;

                init();
                startServer();

                print_menu();
            }
            else
            {
                printf("Error :: malloc failed for EdgeEndPointInfo in Test create Nodes\n");
            }
        }
        else if (!strcmp(command, "modify_node"))
        {
            testModifyNode();
        }
        else if (!strcmp(command, "quit"))
        {
            deinit();
        }
        else if (!strcmp(command, "help"))
        {
            print_menu();
        }
    }
    return 0;
}

