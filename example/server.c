#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>

#include <pthread.h>
#include <unistd.h>

#include <opcua_manager.h>
#include "opcua_common.h"
#include "edge_identifier.h"
#include "open62541.h"

#define COLOR_GREEN        "\x1b[32m"
#define COLOR_YELLOW      "\x1b[33m"
#define COLOR_PURPLE      "\x1b[35m"
#define COLOR_RESET         "\x1b[0m"

#define FREE(arg) if(arg) {free(arg); arg=NULL; }

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

static void response_msg_cb (EdgeMessage *data)
{

}

static void monitored_msg_cb (EdgeMessage *data)
{

}

static void error_msg_cb (EdgeMessage *data)
{
    printf("[error_msg_cb] EdgeStatusCode: %d\n", data->result->code);
}

static void browse_msg_cb (EdgeMessage *data)
{

}

/* status callbacks */
static void status_start_cb (EdgeEndPointInfo *epInfo, EdgeStatusCode status)
{
    if (status == STATUS_SERVER_STARTED)
    {
        printf("[Application Callback] Server started\n");
        startFlag = true;

        testCreateNamespace();
        testCreateNodes();
    }
}

static void status_stop_cb (EdgeEndPointInfo *epInfo, EdgeStatusCode status)
{
    if (status == STATUS_STOP_SERVER)
    {
        printf("[Application Callback] Server stopped \n");
        exit(0);
    }
}

static void status_network_cb (EdgeEndPointInfo *epInfo, EdgeStatusCode status)
{

}

/* discovery callback */
static void endpoint_found_cb (EdgeDevice *device)
{

}

static void device_found_cb (EdgeDevice *device)
{

}

static void init()
{
    config = (EdgeConfigure *) malloc(sizeof(EdgeConfigure));
    config->recvCallback = (ReceivedMessageCallback *) malloc(sizeof(ReceivedMessageCallback));
    config->recvCallback->resp_msg_cb = response_msg_cb;
    config->recvCallback->monitored_msg_cb = monitored_msg_cb;
    config->recvCallback->error_msg_cb = error_msg_cb;
    config->recvCallback->browse_msg_cb = browse_msg_cb;

    config->statusCallback = (StatusCallback *) malloc(sizeof(StatusCallback));
    config->statusCallback->start_cb = status_start_cb;
    config->statusCallback->stop_cb = status_stop_cb;
    config->statusCallback->network_cb = status_network_cb;

    config->discoveryCallback = (DiscoveryCallback *) malloc(sizeof(DiscoveryCallback));
    config->discoveryCallback->endpoint_found_cb = endpoint_found_cb;
    config->discoveryCallback->device_found_cb = device_found_cb;

    registerCallbacks(config);
}

static void startServer()
{
    EdgeEndpointConfig *endpointConfig = (EdgeEndpointConfig *) calloc(1, sizeof(EdgeEndpointConfig));
    endpointConfig->bindAddress = ipAddress;
    endpointConfig->bindPort = 12686;
    endpointConfig->serverName = DEFAULT_SERVER_NAME_VALUE;
    //endpointConfig->requestTimeout = 60000;

    printf(COLOR_GREEN "[Endpoint Configuration]\n" COLOR_RESET);
    printf("\nBind Address : %s", endpointConfig->bindAddress);
    printf("\nBind Port : %d\n", endpointConfig->bindPort);

    EdgeApplicationConfig *appConfig = (EdgeApplicationConfig *) calloc(1, sizeof(EdgeApplicationConfig));
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
    printf("\n" COLOR_PURPLE "==================== Creating nodes ===================" COLOR_RESET "\n");
    int index = 0;

    printf(COLOR_GREEN"\n[Create Variable Node]\n"COLOR_RESET);
    printf("\n[%d] Variable node with string variant\n", ++index);
    EdgeNodeItem *item = (EdgeNodeItem *) malloc(sizeof(EdgeNodeItem));
    item->nodeType = VARIABLE_NODE;
    item->accessLevel = READ_WRITE;
    item->userAccessLevel = READ_WRITE;
    item->writeMask = 0;
    item->userWriteMask = 0;
    item->forward = true;
    item->browseName = "String1";
    item->variableItemName = "Location";
    item->variableIdentifier = String;
    item->variableData = (void *) "test1";
    createNode(DEFAULT_NAMESPACE_VALUE, item);
    printf("\n|------------[Added]  %s\n", item->browseName);

    item->nodeType = VARIABLE_NODE;
    item->accessLevel = READ_WRITE;
    item->userAccessLevel = READ_WRITE;
    item->writeMask = 0;
    item->userWriteMask = 0;
    item->forward = true;
    item->browseName = "String2";
    item->variableItemName = "Location";
    item->variableIdentifier = String;
    item->variableData = (void *) "test2";
    createNode(DEFAULT_NAMESPACE_VALUE, item);
    printf("\n|------------[Added] %s\n", item->browseName);

    item->nodeType = VARIABLE_NODE;
    item->accessLevel = READ_WRITE;
    item->userAccessLevel = READ_WRITE;
    item->writeMask = 0;
    item->userWriteMask = 0;
    item->forward = true;
    item->browseName = "String3";
    item->variableItemName = "Location";
    item->variableIdentifier = String;
    item->variableData = (void *) "test3";
    createNode(DEFAULT_NAMESPACE_VALUE, item);
    printf("\n|------------[Added] %s\n", item->browseName);

    printf("\n[%d] Variable node with XML ELEMENT variant: \n", ++index);
    UA_XmlElement *xml_value = (UA_XmlElement *) malloc(sizeof(UA_XmlElement));
    xml_value->length = 2;
    xml_value->data = (UA_Byte *) "ab";
    item->browseName = "xml_value";
    item->variableItemName = "Location";
    item->variableIdentifier = XmlElement;
    item->variableData = (void *) xml_value;
    createNode(DEFAULT_NAMESPACE_VALUE, item);
    printf("\n|------------[Added] %s\n", item->browseName);
    FREE(xml_value);

    printf("\n[%d] Variable node with localized text variant: \n", ++index);
    UA_LocalizedText *lt_value = (UA_LocalizedText *) malloc(sizeof(UA_LocalizedText));
    lt_value->locale = UA_STRING_ALLOC("COUNTRY");
    lt_value->text = UA_STRING_ALLOC("INDIA");
    item->browseName = "LocalizedText";
    item->variableItemName = "Location";
    item->variableIdentifier = LocalizedText;
    item->variableData = (void *) lt_value;
    createNode(DEFAULT_NAMESPACE_VALUE, item);
    printf("\n|------------[Added] %s\n", item->browseName);
    FREE(lt_value->locale.data);
    FREE(lt_value->text.data);
    FREE(lt_value);

    printf("\n[%d] Variable node with byte string variant: \n", ++index);
    UA_ByteString *bs_value = (UA_ByteString *) malloc(sizeof(UA_ByteString));
    bs_value->length = 7;
    bs_value->data = (UA_Byte *) "samsung";
    item->browseName = "ByteString";
    item->variableItemName = "Location";
    item->variableIdentifier = ByteString;
    item->variableData = (void *) bs_value;
    createNode(DEFAULT_NAMESPACE_VALUE, item);
    printf("\n|------------[Added] %s\n", item->browseName);
    FREE(bs_value);

    printf("\n[%d] Variable node with byte variant: \n", ++index);
    UA_Byte b_value = 2;
    item->browseName = "Byte";
    item->variableItemName = "Location";
    item->variableIdentifier = Byte;
    item->variableData = (void *) &b_value;
    createNode(DEFAULT_NAMESPACE_VALUE, item);
    printf("\n|------------[Added] %s\n", item->browseName);

    printf("\n[%d] Variable node with float variant: \n", ++index);
    float f_value = 4.4;
    item->browseName = "Float";
    item->variableItemName = "Location";
    item->variableIdentifier = Float;
    item->variableData = (void *) &f_value;
    createNode(DEFAULT_NAMESPACE_VALUE, item);
    printf("\n|------------[Added] %s\n", item->browseName);

    printf("\n[%d] Variable node with int variant: \n", ++index);
    int value = 30;
    item->browseName = "UInt16";
    item->variableItemName = "Location";
    item->variableIdentifier = UInt16;
    item->variableData = (void *) &value;
    createNode(DEFAULT_NAMESPACE_VALUE, item);
    printf("\n|------------[Added] %s\n", item->browseName);

    printf("\n[%d] Variable node with UInt32 variant: \n", ++index);
    value = 444;
    item->browseName = "UInt32";
    item->variableItemName = "Location";
    item->variableIdentifier = UInt32;
    item->variableData = (void *) &value;
    createNode(DEFAULT_NAMESPACE_VALUE, item);
    printf("\n|------------[Added] %s\n", item->browseName);

    printf("\n[%d] Variable node with UInt64 variant: \n", ++index);
    value = 3445516;
    item->browseName = "UInt64";
    item->variableItemName = "Location";
    item->variableIdentifier = UInt64;
    item->variableData = (void *) &value;
    createNode(DEFAULT_NAMESPACE_VALUE, item);
    printf("\n|------------[Added] %s\n", item->browseName);

    printf("\n[%d] Variable node with Int16 variant: \n", ++index);
    value = 4;
    item->browseName = "Int16";
    item->variableItemName = "Location";
    item->variableIdentifier = Int16;
    item->variableData = (void *) &value;
    createNode(DEFAULT_NAMESPACE_VALUE, item);
    printf("\n|------------[Added] %s\n", item->browseName);

    printf("\n[%d] Variable node with Int32 variant: \n", ++index);
    value = 40;
    item->browseName = "Int32";
    item->variableItemName = "Location";
    item->variableIdentifier = Int32;
    item->variableData = (void *) &value;
    createNode(DEFAULT_NAMESPACE_VALUE, item);
    printf("\n|------------[Added] %s\n", item->browseName);

    printf("\n[%d] Variable node with Int64 variant: \n", ++index);
    value = 32700;
    item->browseName = "Int64";
    item->variableItemName = "Location";
    item->variableIdentifier = Int64;
    item->variableData = (void *) &value;
    createNode(DEFAULT_NAMESPACE_VALUE, item);
    printf("\n|------------[Added] %s\n", item->browseName);

    printf("\n[%d] Variable node with UInt32 variant: \n", ++index);
    uint32_t int32_val = 4456;
    item->accessLevel = WRITE;
    item->userAccessLevel = WRITE;
    item->browseName = "UInt32writeonly";
    item->variableItemName = "Location";
    item->variableIdentifier = UInt32;
    item->variableData = (void *) &int32_val;
    createNode(DEFAULT_NAMESPACE_VALUE, item);
    printf("\n|------------[Added] %s\n", item->browseName);

    item->userAccessLevel = READ;
    item->accessLevel = READ;
    printf("\n[%d] Variable node with UInt64 variant: \n", ++index);
    int64_t int64_val = 3270000;
    item->browseName = "UInt64readonly";
    item->variableItemName = "Location";
    item->variableIdentifier = UInt64;
    item->variableData = (void *) &int64_val;
    createNode(DEFAULT_NAMESPACE_VALUE, item);
    printf("\n|------------[Added] %s\n", item->browseName);

    printf("\n[%d] Variable node with double variant: \n", ++index);
    item->userAccessLevel = READ_WRITE;
    item->accessLevel = READ_WRITE;
    double d_val = 50.4;
    item->browseName = "Double";
    item->variableItemName = "Location";
    item->variableIdentifier = Double;
    item->variableData = (void *) &d_val;
    createNode(DEFAULT_NAMESPACE_VALUE, item);
    printf("\n|------------[Added] %s\n", item->browseName);

    printf("\n[%d] Variable node with boolean variant: \n", ++index);
    bool flag = true;
    item->browseName = "Boolean";
    item->variableItemName = "Boolean";
    item->variableIdentifier = Boolean;
    item->variableData = (void *) &flag;
    createNode(DEFAULT_NAMESPACE_VALUE, item);
    printf("\n|------------[Added] %s\n", item->browseName);

    printf("\n[%d] Variable node with dateTime variant: \n", ++index);
    UA_DateTime time = UA_DateTime_now();
    item->browseName = "DateTime";
    item->variableItemName = "DateTime";
    item->variableIdentifier = DateTime;
    item->variableData = (void *) &time;
    createNode(DEFAULT_NAMESPACE_VALUE, item);
    printf("\n|------------[Added] %s\n", item->browseName);

    printf("\n[%d] Variable node with SByte variant: \n", ++index);
    UA_SByte sbyte = 2;
    item->browseName = "SByte";
    item->variableItemName = "SByte";
    item->variableIdentifier = SByte;
    item->variableData = (void *) &sbyte;
    createNode(DEFAULT_NAMESPACE_VALUE, item);
    printf("\n|------------[Added] %s\n", item->browseName);

    printf("\n[%d] Variable node with GUID variant: \n", ++index);
    UA_Guid guid = {1, 0, 1, {0, 0, 0, 0, 1, 1, 1, 1}};
    item->browseName = "Guid";
    item->variableItemName = "Guid";
    item->variableIdentifier = Guid;
    item->variableData = (void *) &guid;
    createNode(DEFAULT_NAMESPACE_VALUE, item);
    printf("\n|------------[Added] %s\n", item->browseName);

    printf("\n[%d] Variable node with qualified name variant: \n", ++index);
    UA_QualifiedName *qn_value = (UA_QualifiedName *) malloc(sizeof(UA_QualifiedName));
    UA_String str = UA_STRING_ALLOC("qualifiedName");
    qn_value->namespaceIndex = 2;
    qn_value->name = str;
    item->browseName = "QualifiedName";
    item->variableItemName = "QualifiedName";
    item->variableIdentifier = QualifiedName;
    item->variableData = (void *) qn_value;
    createNode(DEFAULT_NAMESPACE_VALUE, item);
    printf("\n|------------[Added] %s\n", item->browseName);
    FREE(str.data);
    FREE(qn_value);

    printf("\n[%d] Variable node with NODEID variant: \n", ++index);
    UA_NodeId node = UA_NODEID_NUMERIC(DEFAULT_NAMESPACE_INDEX, RootFolder);
    item->browseName = "NodeId";
    item->variableItemName = "NodeId";
    item->variableIdentifier = NodeId;
    item->variableData = (void *) &node;
    createNode(DEFAULT_NAMESPACE_VALUE, item);
    printf("\n|------------[Added] %s\n", item->browseName);


    /******************* Array *********************/
    printf(COLOR_GREEN "\n[Create Array Node]\n" COLOR_RESET);
    printf("\n[%d] Array node with ByteString values: \n", ++index);
    UA_ByteString **dataArray = (UA_ByteString **) malloc(sizeof(UA_ByteString *) * 5);
    dataArray[0] = (UA_ByteString *) malloc(sizeof(UA_ByteString));
    *dataArray[0] = UA_BYTESTRING_ALLOC("abcde");
    dataArray[1] = (UA_ByteString *) malloc(sizeof(UA_ByteString));
    *dataArray[1] = UA_BYTESTRING_ALLOC("fghij");
    dataArray[2] = (UA_ByteString *) malloc(sizeof(UA_ByteString));
    *dataArray[2] = UA_BYTESTRING_ALLOC("klmno");
    dataArray[3] = (UA_ByteString *) malloc(sizeof(UA_ByteString));
    *dataArray[3] = UA_BYTESTRING_ALLOC("pqrst");
    dataArray[4] = (UA_ByteString *) malloc(sizeof(UA_ByteString));
    *dataArray[4] = UA_BYTESTRING_ALLOC("uvwxyz");
    item->browseName = "ByteStringArray";
    item->nodeType = ARRAY_NODE;
    item->variableItemName = "ByteStringArray";
    item->variableIdentifier = ByteString;
    item->arrayLength = 5;
    item->variableData = (void *) dataArray;
    createNode(DEFAULT_NAMESPACE_VALUE, item);
    printf("\n|------------[Added] %s\n", item->browseName);
    for (int i = 0; i < 5; i++)
    {
        UA_ByteString temp = *dataArray[i];
        free(temp.data); temp.data = NULL;
        free(dataArray[i]); dataArray[i] = NULL;
    }
    FREE(dataArray);

    printf("\n[%d] Array node with Boolean values: \n", ++index);
    bool *arr = (bool *) malloc(sizeof(bool) * 5);
    arr[0] = true;
    arr[1] = false;
    arr[2] = true;
    arr[3] = false;
    arr[4] = true;
    item->browseName = "BoolArray";
    item->nodeType = ARRAY_NODE;
    item->variableItemName = "BoolArray";
    item->variableIdentifier = Boolean;
    item->arrayLength = 5;
    item->variableData = (void *) arr;
    createNode(DEFAULT_NAMESPACE_VALUE, item);
    printf("\n|------------[Added] %s\n", item->browseName);
    FREE(arr);

    printf("\n[%d] Array node with SByte values: \n", ++index);
    UA_SByte *sbData = (UA_SByte *) malloc(sizeof(UA_SByte) * 5);
    sbData[0] = -128;
    sbData[1] = 112;
    sbData[2] = 120;
    sbData[3] = 122;
    sbData[4] = 127;
    item->browseName = "SByteArray";
    item->nodeType = ARRAY_NODE;
    item->variableItemName = "SByteArray";
    item->variableIdentifier = SByte;
    item->arrayLength = 5;
    item->variableData = (void *) sbData;
    createNode(DEFAULT_NAMESPACE_VALUE, item);
    printf("\n|------------[Added] %s\n", item->browseName);
    FREE(sbData);

    printf("\n[%d] Array node with Int32 values: \n", ++index);
    int *intData = (int *) malloc(sizeof(int) * 7);
    intData[0] = 11;
    intData[1] = 22;
    intData[2] = 33;
    intData[3] = 44;
    intData[4] = 55;
    intData[5] = 66;
    intData[6] = 77;
    item->browseName = "int32Array";
    item->nodeType = ARRAY_NODE;
    item->variableItemName = "int32Array";
    item->variableIdentifier = Int32;
    item->arrayLength = 7;
    item->variableData = (void *) intData;
    createNode(DEFAULT_NAMESPACE_VALUE, item);
    printf("\n|------------[Added] %s\n", item->browseName);
    FREE(intData);

    printf("\n[%d] Array node with Int64 values: \n", ++index);
    int *int64Data = (int *) malloc(sizeof(int) * 5);
    int64Data[0] = 11111;
    int64Data[1] = 22222;
    int64Data[2] = 33333;
    int64Data[3] = 44444;
    int64Data[4] = 55555;
    item->browseName = "int64Array";
    item->nodeType = ARRAY_NODE;
    item->variableItemName = "int64Array";
    item->variableIdentifier = Int64;
    item->arrayLength = 5;
    item->variableData = (void *) int64Data;
    createNode(DEFAULT_NAMESPACE_VALUE, item);
    printf("\n|------------[Added] %s\n", item->browseName);
    FREE(int64Data);

    printf("\n[%d] Array node with double values: \n", ++index);
    double *data = (double *) malloc(sizeof(double) * 5);
    data[0] = 10.2;
    data[1] = 20.2;
    data[2] = 30.2;
    data[3] = 40.2;
    data[4] = 50.2;
    item->browseName = "DoubleArray";
    item->nodeType = ARRAY_NODE;
    item->variableItemName = "DoubleArray";
    item->variableIdentifier = Double;
    item->arrayLength = 5;
    item->variableData = (void *) data;
    createNode(DEFAULT_NAMESPACE_VALUE, item);
    printf("\n|------------[Added] %s\n", item->browseName);
    FREE(data);

    printf("\n[%d] Array node with string values: \n", ++index);
    char **data1 = (char **) malloc(sizeof(char *) * 5);
    data1[0] = (char *) malloc(10);
    strcpy(data1[0], "apple");
    data1[1] = (char *) malloc(10);
    strcpy(data1[1], "ball");
    data1[2] = (char *) malloc(10);
    strcpy(data1[2], "cats");
    data1[3] = (char *) malloc(10);
    strcpy(data1[3], "dogs");
    data1[4] = (char *) malloc(10);
    strcpy(data1[4], "elephant");
    item->browseName = "CharArray";
    item->nodeType = ARRAY_NODE;
    item->variableItemName = "CharArray";
    item->variableIdentifier = String;
    item->arrayLength = 5;
    item->variableData = (void *) (data1);
    createNode(DEFAULT_NAMESPACE_VALUE, item);
    printf("\n|------------[Added] %s\n", item->browseName);
    for (int i = 0; i < 5; i++)
    {
        free(data1[i]); data1[i] = NULL;
    }
    FREE(data1);

    printf("\n[%d] Variable node with byte array variant: \n", ++index);
    UA_Byte *b_arrvalue = (UA_Byte *) malloc(sizeof(UA_Byte) * 5);
    b_arrvalue[0] = 0x11;
    b_arrvalue[1] = 0x22;
    b_arrvalue[2] = 0x33;
    b_arrvalue[3] = 0x44;
    b_arrvalue[4] = 0x55;
    item->arrayLength = 5;
    item->browseName = "ByteArray";
    item->nodeType = ARRAY_NODE;
    item->variableItemName = "Location";
    item->variableIdentifier = Byte;
    item->variableData = (void *) b_arrvalue;
    createNode(DEFAULT_NAMESPACE_VALUE, item);
    printf("\n|------------[Added] %s\n", item->browseName);
    FREE(b_arrvalue);

    /******************* Object Node *********************/
    printf(COLOR_GREEN"\n[Create Object Node]\n"COLOR_RESET);
    printf("\n[%d] Object node : \"Object1\"\n", ++index);
    item->nodeType = OBJECT_NODE;
    item->browseName = "Object1";
    item->sourceNodeId = (EdgeNodeId *) malloc (sizeof(EdgeNodeId));
    item->sourceNodeId->nodeId = NULL;    // no source node
    createNode(DEFAULT_NAMESPACE_VALUE, item);
    printf("\n|------------[Added] %s\n", item->browseName);
    FREE(item->sourceNodeId);

    printf("\n[%d] Object node : \"Object2\" with source Node \"Object1\"\n", ++index);
    item->nodeType = OBJECT_NODE;
    item->browseName = "Object2";
    item->sourceNodeId = (EdgeNodeId *) malloc (sizeof(EdgeNodeId));
    item->sourceNodeId->nodeId = "Object1";
    createNode(DEFAULT_NAMESPACE_VALUE, item);
    printf("\n|------------[Added] %s\n", item->browseName);
    FREE(item->sourceNodeId);

    /******************* View Node *********************/
    printf(COLOR_GREEN"\n[Create View Node]\n"COLOR_RESET);
    printf("\n[%d] View Node with ViewNode1\n", ++index);
    item->nodeType = VIEW_NODE;
    item->browseName = "ViewNode1";
    item->sourceNodeId = (EdgeNodeId *) malloc (sizeof(EdgeNodeId));
    item->sourceNodeId->nodeId = NULL;    // no source node
    createNode(DEFAULT_NAMESPACE_VALUE, item);
    printf("\n|------------[Added] %s\n", item->browseName);
    FREE(item->sourceNodeId);

    printf("\n[%d] View Node with ViewNode2\n", ++index);
    item->nodeType = VIEW_NODE;
    item->browseName = "ViewNode2";
    item->sourceNodeId = (EdgeNodeId *) malloc (sizeof(EdgeNodeId));
    item->sourceNodeId->nodeId = "ViewNode1";
    createNode(DEFAULT_NAMESPACE_VALUE, item);
    printf("\n|------------[Added] %s\n", item->browseName);
    FREE(item->sourceNodeId);

    /******************* Object Type Node *********************/
    printf(COLOR_GREEN"\n[Create Object Type Node]\n"COLOR_RESET);
    printf("\n[%d] Object Type node : \"ObjectType1\"\n", ++index);
    item->nodeType = OBJECT_TYPE_NODE;
    item->browseName = "ObjectType1";
    item->sourceNodeId = (EdgeNodeId *) malloc (sizeof(EdgeNodeId));
    item->sourceNodeId->nodeId = NULL;    // no source node
    createNode(DEFAULT_NAMESPACE_VALUE, item);
    printf("\n|------------[Added] %s\n", item->browseName);
    FREE(item->sourceNodeId);

    printf("\n[%d] Object Type node : \"ObjectType2\" with source Node \"ObjectType1\"\n", ++index);
    item->nodeType = OBJECT_TYPE_NODE;
    item->browseName = "ObjectType2";
    item->sourceNodeId = (EdgeNodeId *) malloc (sizeof(EdgeNodeId));
    item->sourceNodeId->nodeId = "ObjectType1";
    createNode(DEFAULT_NAMESPACE_VALUE, item);
    printf("\n|------------[Added] %s\n", item->browseName);
    FREE(item->sourceNodeId);

    printf("\n[%d] Object Type node : \"ObjectType3\" with source Node \"ObjectType2\"\n", ++index);
    item->nodeType = OBJECT_TYPE_NODE;
    item->browseName = "ObjectType3";
    item->sourceNodeId = (EdgeNodeId *) malloc (sizeof(EdgeNodeId));
    item->sourceNodeId->nodeId = "ObjectType1";
    createNode(DEFAULT_NAMESPACE_VALUE, item);
    printf("\n|------------[Added] %s\n", item->browseName);
    FREE(item->sourceNodeId);

    printf("\n[%d] Object Type node : \"ObjectType4\" with source Node \"ObjectType3\"\n", ++index);
    item->nodeType = OBJECT_TYPE_NODE;
    item->browseName = "ObjectType4";
    item->sourceNodeId = (EdgeNodeId *) malloc (sizeof(EdgeNodeId));
    item->sourceNodeId->nodeId = "ObjectType1";
    createNode(DEFAULT_NAMESPACE_VALUE, item);
    printf("\n|------------[Added] %s\n", item->browseName);
    FREE(item->sourceNodeId);

    printf("\n[%d] Object Type node : \"ObjectType5\" with source Node \"ObjectType3\"\n", ++index);
    item->nodeType = OBJECT_TYPE_NODE;
    item->browseName = "ObjectType5";
    item->sourceNodeId = (EdgeNodeId *) malloc (sizeof(EdgeNodeId));
    item->sourceNodeId->nodeId = "ObjectType1";
    createNode(DEFAULT_NAMESPACE_VALUE, item);
    printf("\n|------------[Added] %s\n", item->browseName);
    FREE(item->sourceNodeId);

    /******************* Variable Type Node *********************/
    printf(COLOR_GREEN"\n[Create Variable Type Node]\n"COLOR_RESET);
    printf("\n[%d] Variable Type Node with Double Variable Type \n", ++index);
    double d[2] = { 10.2, 20.2 };
    item->browseName = "DoubleVariableType";
    item->nodeType = VARIABLE_TYPE_NODE;
    item->variableItemName = "DoubleVariableType";
    item->variableIdentifier = Double;
    item->arrayLength = 2;
    item->variableData = (void *) (d);
    createNode(DEFAULT_NAMESPACE_VALUE, item);
    printf("\n|------------[Added] %s\n", item->browseName);

    /******************* Data Type Node *********************/
    printf("\n[%d] Data Type Node with DataType1\n", ++index);
    item->nodeType = DATA_TYPE_NODE;
    item->browseName = "DataType1";
    item->sourceNodeId = (EdgeNodeId *) malloc (sizeof(EdgeNodeId));
    item->sourceNodeId->nodeId = NULL;    // no source node
    createNode(DEFAULT_NAMESPACE_VALUE, item);
    printf("\n|------------[Added] %s\n", item->browseName);
    FREE(item->sourceNodeId);

    printf("\n[%d] Data Type Node with DataType2\n", ++index);
    item->nodeType = DATA_TYPE_NODE;
    item->browseName = "DataType2";
    item->sourceNodeId = (EdgeNodeId *) malloc (sizeof(EdgeNodeId));
    item->sourceNodeId->nodeId = "DataType1";
    createNode(DEFAULT_NAMESPACE_VALUE, item);
    FREE(item->sourceNodeId);

    /******************* Reference Type Node *********************/
    printf(COLOR_GREEN"\n[Create Reference Type Node]\n"COLOR_RESET);
    printf("\n[%d] Reference Type Node with ReferenceTypeNode1", ++index);
    item->nodeType = REFERENCE_TYPE_NODE;
    item->browseName = "ReferenceTypeNode1";
    item->sourceNodeId = (EdgeNodeId *) malloc (sizeof(EdgeNodeId));
    item->sourceNodeId->nodeId = NULL;    // no source node
    createNode(DEFAULT_NAMESPACE_VALUE, item);
    printf("\n|------------[Added] %s\n", item->browseName);
    FREE(item->sourceNodeId);

    printf("\n[%d] Reference Type Node with source node\"ReferenceTypeNode1\"\n", ++index);
    item->nodeType = REFERENCE_TYPE_NODE;
    item->browseName = "ReferenceTypeNode2";
    item->sourceNodeId = (EdgeNodeId *) malloc (sizeof(EdgeNodeId));
    item->sourceNodeId->nodeId = "ReferenceTypeNode1";
    createNode(DEFAULT_NAMESPACE_VALUE, item);
    printf("\n|------------[Added] %s\n", item->browseName);
    FREE(item->sourceNodeId);

    FREE(item);

    /******************* Method Node *********************/
    printf(COLOR_GREEN"\n[Create Method Node]\n"COLOR_RESET);
    printf("\n[%d] Method Node with square_root \n", ++index);
    EdgeNodeItem *methodNodeItem = (EdgeNodeItem *) malloc(sizeof(EdgeNodeItem));
    methodNodeItem->browseName = "sqrt(x)";
    methodNodeItem->sourceNodeId = NULL;

    EdgeMethod *method = (EdgeMethod *) malloc(sizeof(EdgeMethod));
    method->description = "Calculate square root";
    method->methodNodeName = "square_root";
    method->method_fn = test_method_sqrt;
    method->num_inpArgs = 1;
    method->inpArg = (EdgeArgument **) malloc(sizeof(EdgeArgument *) * method->num_inpArgs);
    for (int idx = 0; idx < method->num_inpArgs; idx++)
    {
        method->inpArg[idx] = (EdgeArgument *) malloc(sizeof(EdgeArgument));
        method->inpArg[idx]->argType = Double;
        method->inpArg[idx]->valType = SCALAR;
    }

    method->num_outArgs = 1;
    method->outArg = (EdgeArgument **) malloc(sizeof(EdgeArgument *) * method->num_outArgs);
    for (int idx = 0; idx < method->num_outArgs; idx++)
    {
        method->outArg[idx] = (EdgeArgument *) malloc(sizeof(EdgeArgument));
        method->outArg[idx]->argType = Double;
        method->outArg[idx]->valType = SCALAR;
    }
    createMethodNode(DEFAULT_NAMESPACE_VALUE, methodNodeItem, method);
    printf("\n|------------[Added] %s\n", methodNodeItem->browseName);
    FREE(methodNodeItem);

    printf("\n[%d] Method Node with incrementInc32Array \n", ++index);
    EdgeNodeItem *methodNodeItem1 = (EdgeNodeItem *) malloc(sizeof(EdgeNodeItem));
    methodNodeItem1->browseName = "incrementInc32Array(x,delta)";
    methodNodeItem1->sourceNodeId = NULL;

    EdgeMethod *method1 = (EdgeMethod *) malloc(sizeof(EdgeMethod));
    method1->description = "Increment int32 array by delta";
    method1->methodNodeName = "incrementInc32Array";
    method1->method_fn = test_method_increment_int32Array;

    method1->num_inpArgs = 2;
    method1->inpArg = (EdgeArgument **) malloc(sizeof(EdgeArgument *) * method1->num_inpArgs);
    method1->inpArg[0] = (EdgeArgument *) malloc(sizeof(EdgeArgument));
    method1->inpArg[0]->argType = Int32;
    method1->inpArg[0]->valType = ARRAY_1D;
    method1->inpArg[0]->arrayLength = 5;

    method1->inpArg[1] = (EdgeArgument *) malloc(sizeof(EdgeArgument));
    method1->inpArg[1]->argType = Int32;
    method1->inpArg[1]->valType = SCALAR;

    method1->num_outArgs = 1;
    method1->outArg = (EdgeArgument **) malloc(sizeof(EdgeArgument *) * method1->num_outArgs);
    for (int idx = 0; idx < method1->num_outArgs; idx++)
    {
        method1->outArg[idx] = (EdgeArgument *) malloc(sizeof(EdgeArgument));
        method1->outArg[idx]->argType = Int32;
        method1->outArg[idx]->valType = ARRAY_1D;
        method1->outArg[idx]->arrayLength = 5;
    }
    createMethodNode(DEFAULT_NAMESPACE_VALUE, methodNodeItem1, method1);
    printf("\n|------------[Added] %s\n", methodNodeItem1->browseName);
    FREE(methodNodeItem1);

    printf("\n[%d] Method Node with noArgMethod \n", ++index);
    EdgeNodeItem *methodNodeItem2 = (EdgeNodeItem *) malloc(sizeof(EdgeNodeItem));
    methodNodeItem2->browseName = "shutdown()";
    methodNodeItem2->sourceNodeId = NULL;

    EdgeMethod *method2 = (EdgeMethod *) malloc(sizeof(EdgeMethod));
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
    methodNodeItem3->browseName = "print(x)";
    methodNodeItem3->sourceNodeId = NULL;

    EdgeMethod *method3 = (EdgeMethod *) malloc(sizeof(EdgeMethod));
    method3->description = "print x";
    method3->methodNodeName = "print";
    method3->method_fn = test_method_print;

    method3->num_inpArgs = 1;
    method3->inpArg = (EdgeArgument **) malloc(sizeof(EdgeArgument *) * method3->num_inpArgs);
    method3->inpArg[0] = (EdgeArgument *) malloc(sizeof(EdgeArgument));
    method3->inpArg[0]->argType = Int32;
    method3->inpArg[0]->valType = SCALAR;

    method3->num_outArgs = 0;
    method3->outArg = NULL;

    createMethodNode(DEFAULT_NAMESPACE_VALUE, methodNodeItem3, method3);
    printf("\n|------------[Added] %s\n", methodNodeItem3->browseName);
    FREE(methodNodeItem3);

    printf("\n[%d] Method Node with outArgMethod \n", ++index);
    EdgeNodeItem *methodNodeItem4 = (EdgeNodeItem *) malloc(sizeof(EdgeNodeItem));
    methodNodeItem4->browseName = "version()";
    methodNodeItem4->sourceNodeId = NULL;

    EdgeMethod *method4 = (EdgeMethod *) malloc(sizeof(EdgeMethod));
    method4->description = "Get Version Info";
    method4->methodNodeName = "version";
    method4->method_fn = test_method_version;

    method4->num_inpArgs = 0;
    method4->inpArg = NULL;

    method4->num_outArgs = 1;
    method4->outArg = (EdgeArgument **) malloc(sizeof(EdgeArgument *) * method4->num_outArgs);
    method4->outArg[0] = (EdgeArgument *) malloc(sizeof(EdgeArgument));
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

    reference->forward =  true;
    reference->sourceNamespace = DEFAULT_NAMESPACE_VALUE;
    reference->sourcePath = "ViewNode1";
    reference->targetNamespace = DEFAULT_NAMESPACE_VALUE;
    reference->targetPath = "ObjectType1";
    /* default reference ID : Organizes */
    addReference(reference);

    FREE(reference);

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

    printf("\n\n" COLOR_YELLOW
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
        printf( "Invalid Option!!! \n\n");
        return ;
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
    } else if (option == 7) {
        strcpy(name, "String1");
        for (int i = 0; i < MAX_TEST_NUMBER; i++) {
            if(i % 2 == 0) {
                strcpy(s_value , SAMPLE_STRING_1);
            } else {
                strcpy(s_value, SAMPLE_STRING_2);
            }

            new_value = (void *) s_value;
            EdgeVersatility *message = (EdgeVersatility *) malloc(sizeof(EdgeVersatility));
            message->value = new_value;

            modifyVariableNode(DEFAULT_NAMESPACE_VALUE, name, message);

            FREE(message);
            usleep(1000 * 1000);
        }
        return;
    } else if (option == 8) {
        strcpy(name, "Int32");
        for (int i = 0; i < MAX_TEST_NUMBER; i++) {
            new_value = (void *) &i;

            EdgeVersatility *message = (EdgeVersatility *) malloc(sizeof(EdgeVersatility));
            message->value = new_value;

            modifyVariableNode(DEFAULT_NAMESPACE_VALUE, name, message);

            FREE(message);
            usleep(1000 * 1000);
        }
        return;
    }

    EdgeVersatility *message = (EdgeVersatility *) malloc(sizeof(EdgeVersatility));
    message->value = new_value;

    modifyVariableNode(DEFAULT_NAMESPACE_VALUE, name, message);

    FREE(message);
}

static void deinit()
{
    if (startFlag)
    {
        stopServer();
        startFlag = false;

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

        if(epInfo)
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

            printf("\n" COLOR_YELLOW " ------------------------------------------------------" COLOR_RESET);
            printf("\n" COLOR_YELLOW "                     Start Server            " COLOR_RESET);
            printf("\n" COLOR_YELLOW " ------------------------------------------------------" COLOR_RESET
                   "\n\n");

//            printf("[Please input server address] : ");
//            scanf("%s", ipAddress);

            strcpy(ipAddress, "localhost");

            snprintf(endpointUri, sizeof(endpointUri), "opc:tcp://%s:12686/edge-opc-server", ipAddress);

            epInfo = (EdgeEndPointInfo *) calloc(1, sizeof(EdgeEndPointInfo));
            epInfo->endpointUri = endpointUri;

            init();
            startServer();

            print_menu();
            //startFlag = true;

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

