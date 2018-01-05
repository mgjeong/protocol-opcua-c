#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>

#include <pthread.h>

#include <opcua_manager.h>
#include "opcua_common.h"
#include "edge_identifier.h"
#include "open62541.h"

#define COLOR_GREEN        "\x1b[32m"
#define COLOR_YELLOW      "\x1b[33m"
#define COLOR_PURPLE      "\x1b[35m"
#define COLOR_RESET         "\x1b[0m"

static bool startFlag = false;
static bool stopFlag = false;

static char ipAddress[128];
static char endpointUri[512];

static EdgeEndPointInfo* epInfo;
static EdgeConfigure *config = NULL;

static void testCreateNamespace();
static void testCreateNodes();

static void response_msg_cb (EdgeMessage* data) {

}

static void monitored_msg_cb (EdgeMessage* data) {

}

static void error_msg_cb (EdgeMessage* data) {
  printf("[error_msg_cb] EdgeStatusCode: %d\n", data->result->code);
}

static void browse_msg_cb (EdgeMessage* data) {

}

/* status callbacks */
static void status_start_cb (EdgeEndPointInfo* epInfo, EdgeStatusCode status) {
  if (status == STATUS_SERVER_STARTED) {
    printf("[Application Callback] Server started\n");
    startFlag = true;

    testCreateNamespace();
    testCreateNodes();
  }
}

static void status_stop_cb (EdgeEndPointInfo* epInfo, EdgeStatusCode status) {
  if (status == STATUS_STOP_SERVER) {
      printf("[Application Callback] Server stopped \n");
      exit(0);
    }
}

static void status_network_cb (EdgeEndPointInfo* epInfo, EdgeStatusCode status) {

}

/* discovery callback */
static void endpoint_found_cb (EdgeDevice* device) {

}

static void device_found_cb (EdgeDevice* device) {

}


/**************************************************************************************************/
// Method callbacks
// Method callbacks

static void arg_method(int inpSize, void **input, int outSize, void **output) {

}

static void sqrt_method(int inpSize, void **input, int outSize, void **output) {
  double *inp = (double*) input[0];
  double *sq_root = (double*) malloc(sizeof(double));
  *sq_root = sqrt(*inp);
  output[0] = (void*) sq_root;
}

static void increment_int32Array_method(int inpSize, void **input, int outSize, void **output) {
  int32_t* inputArray = (int32_t*) input[0];
  int* delta = (int*) input[1];

  int32_t* outputArray = (int32_t*) malloc(sizeof(int32_t) * 5);
  for (int i = 0; i < 5; i++) {
    outputArray[i] = inputArray[i] + *delta;
  }
  output[0] = (void*) outputArray;
}


/**************************************************************************************************/

static void init() {
  config = (EdgeConfigure*) malloc(sizeof(EdgeConfigure));
  config->recvCallback = (ReceivedMessageCallback*) malloc(sizeof(ReceivedMessageCallback));
  config->recvCallback->resp_msg_cb = response_msg_cb;
  config->recvCallback->monitored_msg_cb = monitored_msg_cb;
  config->recvCallback->error_msg_cb = error_msg_cb;
  config->recvCallback->browse_msg_cb = browse_msg_cb;

  config->statusCallback = (StatusCallback*) malloc(sizeof(StatusCallback));
  config->statusCallback->start_cb = status_start_cb;
  config->statusCallback->stop_cb = status_stop_cb;
  config->statusCallback->network_cb = status_network_cb;

  config->discoveryCallback = (DiscoveryCallback*) malloc(sizeof(DiscoveryCallback));
  config->discoveryCallback->endpoint_found_cb = endpoint_found_cb;
  config->discoveryCallback->device_found_cb = device_found_cb;

  registerCallbacks(config);
}

static void startServer() {

  EdgeEndpointConfig* endpointConfig = (EdgeEndpointConfig*) malloc(sizeof(EdgeEndpointConfig));
  endpointConfig->bindAddress = ipAddress;
  endpointConfig->bindPort = 12686;
  endpointConfig->applicationName = DEFAULT_SERVER_APP_NAME_VALUE;
  endpointConfig->applicationUri = DEFAULT_SERVER_APP_URI_VALUE;
  endpointConfig->productUri = DEFAULT_PRODUCT_URI_VALUE;
  endpointConfig->securityPolicyUri = NULL;
  endpointConfig->serverName = DEFAULT_SERVER_NAME_VALUE;
  //endpointConfig->requestTimeout = 60000;

  EdgeEndPointInfo* ep = (EdgeEndPointInfo*) malloc(sizeof(EdgeEndPointInfo));
  ep->endpointUri = endpointUri;
  ep->config = endpointConfig;

  EdgeMessage* msg = (EdgeMessage*) malloc(sizeof(EdgeMessage));
  msg->endpointInfo = ep;
  msg->command = CMD_START_SERVER;
  msg->type = SEND_REQUEST;

  printf("start server\n\n");
  //send(msg);
  createServer(ep);
}

static void stopServer() {

  EdgeEndPointInfo* ep = (EdgeEndPointInfo*) malloc(sizeof(EdgeEndPointInfo));
  ep->endpointUri = endpointUri;
  ep->config = NULL;

  EdgeMessage* msg = (EdgeMessage*) malloc(sizeof(EdgeMessage));
  msg->endpointInfo = ep;
  msg->command = CMD_STOP_SERVER;
  msg->type = SEND_REQUEST;

  //send(msg);
  closeServer(ep);
}

static void testCreateNamespace() {
  printf("\n" COLOR_YELLOW "===================== Creating namespace ==================" COLOR_RESET "\n");
  createNamespace(DEFAULT_NAMESPACE_VALUE,
                              DEFAULT_ROOT_NODE_INFO_VALUE,
                              DEFAULT_ROOT_NODE_INFO_VALUE,
                              DEFAULT_ROOT_NODE_INFO_VALUE);
}

static void testCreateNodes() {
  printf("\n" COLOR_YELLOW "==================== Creating nodes ==================" COLOR_RESET "\n");
  int index = 0;

  printf("\n-------------------------------------------------------");
  printf("\n[%d]) Variable node with string variant: ", ++index);
  EdgeNodeItem* item = (EdgeNodeItem*) malloc(sizeof(EdgeNodeItem));
  item->nodeType = VARIABLE_NODE;
  item->accessLevel = READ_WRITE;
  item->userAccessLevel = READ_WRITE;
  item->writeMask = 0;
  item->userWriteMask = 0;
  item->forward = true;
  item->browseName = "String1";
  item->variableItemName = "Location";
  item->variableIdentifier = String;
  item->variableData = (void*) "test1";
  createNode(DEFAULT_NAMESPACE_VALUE, item);

  item->nodeType = VARIABLE_NODE;
  item->accessLevel = READ_WRITE;
  item->userAccessLevel = READ_WRITE;
  item->writeMask = 0;
  item->userWriteMask = 0;
  item->forward = true;
  item->browseName = "String2";
  item->variableItemName = "Location";
  item->variableIdentifier = String;
  item->variableData = (void*) "test2";
  createNode(DEFAULT_NAMESPACE_VALUE, item);

  item->nodeType = VARIABLE_NODE;
  item->accessLevel = READ_WRITE;
  item->userAccessLevel = READ_WRITE;
  item->writeMask = 0;
  item->userWriteMask = 0;
  item->forward = true;
  item->browseName = "String3";
  item->variableItemName = "Location";
  item->variableIdentifier = String;
  item->variableData = (void*) "test3";
  createNode(DEFAULT_NAMESPACE_VALUE, item);

  printf("\n[%d]) Variable node with XML ELEMENT variant ", ++index);
  UA_XmlElement* xml_value = (UA_XmlElement*) malloc(sizeof(UA_XmlElement));
  xml_value->length = 2;
  xml_value->data = (UA_Byte*) "ab";
  item->browseName = "xml_value";
  item->variableItemName = "Location";
  item->variableIdentifier = XmlElement;
  item->variableData = (void*) xml_value;
  createNode(DEFAULT_NAMESPACE_VALUE, item);

  printf("\n[%d]) Variable node with localized text variant ", ++index);
  UA_LocalizedText* lt_value = (UA_LocalizedText*) malloc(sizeof(UA_LocalizedText));
  lt_value->locale= UA_STRING_ALLOC("COUNTRY");
  lt_value->text= UA_STRING_ALLOC("INDIA");
  item->browseName = "LocalizedText";
  item->variableItemName = "Location";
  item->variableIdentifier = LocalizedText;
  item->variableData = (void*) lt_value;
  createNode(DEFAULT_NAMESPACE_VALUE, item);

  printf("\n[%d]) Variable node with byte string variant ", ++index);
  UA_ByteString* bs_value = (UA_ByteString*) malloc(sizeof(UA_ByteString));
  bs_value->length = 7;
  bs_value->data = (UA_Byte*) "samsung";
  item->browseName = "ByteString";
  item->variableItemName = "Location";
  item->variableIdentifier = ByteString;
  item->variableData = (void*) bs_value;
  createNode(DEFAULT_NAMESPACE_VALUE, item);

  printf("\n[%d]) Variable node with byte variant ", ++index);
  UA_Byte b_value = 2;
  item->browseName = "Byte";
  item->variableItemName = "Location";
  item->variableIdentifier = Byte;
  item->variableData = (void*) &b_value;
  createNode(DEFAULT_NAMESPACE_VALUE, item);

  printf("\n[%d]) Variable node with float variant ", ++index);
  float f_value = 4.4;
  item->browseName = "Float";
  item->variableItemName = "Location";
  item->variableIdentifier = Float;
  item->variableData = (void*) &f_value;
  createNode(DEFAULT_NAMESPACE_VALUE, item);

  printf("\n[%d]) Variable node with int variant ", ++index);
  int value = 30;
  item->browseName = "UInt16";
  item->variableItemName = "Location";
  item->variableIdentifier = UInt16;
  item->variableData = (void*) &value;
  createNode(DEFAULT_NAMESPACE_VALUE, item);

  printf("\n[%d]) Variable node with Int32 variant ", ++index);
  value = 40;
  item->browseName = "Int32";
  item->variableItemName = "Location";
  item->variableIdentifier = Int32;
  item->variableData = (void*) &value;
  createNode(DEFAULT_NAMESPACE_VALUE, item);

  printf("\n[%d]) Variable node with int64 variant ", ++index);
  value = 32700;
  item->browseName = "Int64";
  item->variableItemName = "Location";
  item->variableIdentifier = Int64;
  item->variableData = (void*) &value;
  createNode(DEFAULT_NAMESPACE_VALUE, item);
  
  printf("\n[%d]) Variable node with UInt32 variant ", ++index);
  uint32_t int32_val= 4456;
  item->accessLevel= WRITE;
  item->userAccessLevel = WRITE;
  item->browseName = "UInt32";
  item->variableItemName = "Location";
  item->variableIdentifier = UInt32;
  item->variableData = (void*) &int32_val;
  createNode(DEFAULT_NAMESPACE_VALUE, item);

  item->userAccessLevel = READ;
  item->accessLevel= READ;
  printf("\n[%d]) Variable node with UInt64 variant ", ++index);
  int64_t int64_val= 3270000;
  item->browseName = "UInt64";
  item->variableItemName = "Location";
  item->variableIdentifier = UInt64;
  item->variableData = (void*) &int64_val;
  createNode(DEFAULT_NAMESPACE_VALUE, item);

  printf("\n[%d]) Variable node with double variant ", ++index);
  item->userAccessLevel = READ_WRITE;
   item->accessLevel= READ_WRITE;
  double d_val = 50.4;
  item->browseName = "Double";
  item->variableItemName = "Location";
  item->variableIdentifier = Double;
  item->variableData = (void*) &d_val;
  createNode(DEFAULT_NAMESPACE_VALUE, item);

  printf("\n[%d]) Variable node with boolean variant ", ++index);
  bool flag = true;
  item->browseName = "Boolean";
  item->variableItemName = "Boolean";
  item->variableIdentifier = Boolean;
  item->variableData = (void*) &flag;
  createNode(DEFAULT_NAMESPACE_VALUE, item);

  /******************* Array *********************/
  printf("\n-------------------------------------------------------");
  printf("\n[%d]) Array node with ByteString values ", ++index);  
  UA_ByteString** dataArray = (UA_ByteString**) malloc(sizeof(UA_ByteString*) * 5);
  dataArray[0] = (UA_ByteString*) malloc(sizeof(UA_ByteString));
  *dataArray[0] = UA_BYTESTRING_ALLOC("abcde");
  dataArray[1] = (UA_ByteString*) malloc(sizeof(UA_ByteString));
  *dataArray[1] = UA_BYTESTRING_ALLOC("fghij");
  dataArray[2] = (UA_ByteString*) malloc(sizeof(UA_ByteString));
  *dataArray[2] = UA_BYTESTRING_ALLOC("klmno");
  dataArray[3] = (UA_ByteString*) malloc(sizeof(UA_ByteString));
  *dataArray[3] = UA_BYTESTRING_ALLOC("pqrst");
  dataArray[4] = (UA_ByteString*) malloc(sizeof(UA_ByteString));
  *dataArray[4] = UA_BYTESTRING_ALLOC("uvwxyz");
  item->browseName = "ByteStringArray";
  item->nodeType = ARRAY_NODE;
  item->variableItemName = "ByteStringArray";
  item->variableIdentifier = ByteString;
  item->arrayLength = 5;
  item->variableData = (void*) dataArray;
  createNode(DEFAULT_NAMESPACE_VALUE, item);
  
  printf("\n[%d]) Array node with double values ", ++index);
  double* data = (double*) malloc(sizeof(double) * 5);
  data[0] = 10.2;
  data[1] = 20.2;
  data[2] = 30.2;
  data[3] = 40.2;
  data[4] = 50.2;
  item->browseName = "IntArray";
  item->nodeType = ARRAY_NODE;
  item->variableItemName = "IntArray";
  item->variableIdentifier = Double;
  item->arrayLength = 5;
  item->variableData = (void*) data;
  createNode(DEFAULT_NAMESPACE_VALUE, item);

  printf("\n[%d]) Array node with string values ", ++index);
  char** data1 = (char**) malloc(sizeof(char*) * 5);
  data1[0] = (char*) malloc(10);
  strcpy(data1[0], "apple");
  data1[1] = (char*) malloc(10);
  strcpy(data1[1], "ball");
  data1[2] = (char*) malloc(10);
  strcpy(data1[2], "cats");
  data1[3] = (char*) malloc(10);
  strcpy(data1[3], "dogs");
  data1[4] = (char*) malloc(10);
  strcpy(data1[4], "elephant");
  item->browseName = "CharArray";
  item->nodeType = ARRAY_NODE;
  item->variableItemName = "CharArray";
  item->variableIdentifier = String;
  item->arrayLength = 5;
  item->variableData = (void*) (data1);
  createNode(DEFAULT_NAMESPACE_VALUE, item);

  printf("\n[%d]) Variable node with byte array variant ", ++index);
  UA_Byte* b_arrvalue = (UA_Byte*) malloc(sizeof(UA_Byte) * 5);
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
  item->variableData = (void*) b_arrvalue;
  createNode(DEFAULT_NAMESPACE_VALUE, item);

  /******************* Object Node *********************/
  printf("\n-------------------------------------------------------");
  printf("\n[%d]) Object node : \"Object1\"", ++index);
  item->nodeType = OBJECT_NODE;
  item->browseName = "Object1";
  item->sourceNodeId = (EdgeNodeId*) malloc (sizeof(EdgeNodeId));
  item->sourceNodeId->nodeId = NULL;    // no source node
  createNode(DEFAULT_NAMESPACE_VALUE, item);

  printf("\n[%d]) Object node : \"Object2\" with source Node \"Object1\"", ++index);
  item->nodeType = OBJECT_NODE;
  item->browseName = "Object2";
  item->sourceNodeId = (EdgeNodeId*) malloc (sizeof(EdgeNodeId));
  item->sourceNodeId->nodeId = "Object1";
  createNode(DEFAULT_NAMESPACE_VALUE, item);

  /******************* Object Type Node *********************/
  printf("\n-------------------------------------------------------");
  printf("\n[%d]) Object Type node : \"ObjectType1\"", ++index);
  item->nodeType = OBJECT_TYPE_NODE;
  item->browseName = "ObjectType1";
  item->sourceNodeId = (EdgeNodeId*) malloc (sizeof(EdgeNodeId));
  item->sourceNodeId->nodeId = NULL;    // no source node
  createNode(DEFAULT_NAMESPACE_VALUE, item);

  printf("\n[%d]) Object Type node : \"ObjectType2\" with source Node \"ObjectType1\"", ++index);
  item->nodeType = OBJECT_TYPE_NODE;
  item->browseName = "ObjectType2";
  item->sourceNodeId = (EdgeNodeId*) malloc (sizeof(EdgeNodeId));
  item->sourceNodeId->nodeId = "ObjectType1";    // no source node
  createNode(DEFAULT_NAMESPACE_VALUE, item);

  printf("\n[%d]) Object Type node : \"ObjectType3\" with source Node \"ObjectType2\"", ++index);
  item->nodeType = OBJECT_TYPE_NODE;
  item->browseName = "ObjectType3";
  item->sourceNodeId = (EdgeNodeId*) malloc (sizeof(EdgeNodeId));
  item->sourceNodeId->nodeId = "ObjectType1";    // no source node
  createNode(DEFAULT_NAMESPACE_VALUE, item);

  printf("\n[%d]) Object Type node : \"ObjectType4\" with source Node \"ObjectType3\"", ++index);
  item->nodeType = OBJECT_TYPE_NODE;
  item->browseName = "ObjectType4";
  item->sourceNodeId = (EdgeNodeId*) malloc (sizeof(EdgeNodeId));
  item->sourceNodeId->nodeId = "ObjectType1";    // no source node
  createNode(DEFAULT_NAMESPACE_VALUE, item);

  printf("\n[%d]) Object Type node : \"ObjectType5\" with source Node \"ObjectType3\"", ++index);
  item->nodeType = OBJECT_TYPE_NODE;
  item->browseName = "ObjectType5";
  item->sourceNodeId = (EdgeNodeId*) malloc (sizeof(EdgeNodeId));
  item->sourceNodeId->nodeId = "ObjectType1";    // no source node
  createNode(DEFAULT_NAMESPACE_VALUE, item);


  /******************* Variable Type Node *********************/
  printf("\n-------------------------------------------------------");
  printf("\n[%d]) Variable Type Node", ++index);
  double d[2] = { 10.2, 20.2 };
  item->browseName = "DoubleVariableType";
  item->nodeType = VARIABLE_TYPE_NODE;
  item->variableItemName = "DoubleVariableType";
  item->variableIdentifier = Double;
  item->arrayLength = 2;
  item->variableData = (void*) (d);
  createNode(DEFAULT_NAMESPACE_VALUE, item);

  /******************* Data Type Node *********************/
  printf("\n-------------------------------------------------------");
  printf("\n[%d]) Data Type Node", ++index);
  item->nodeType = DATA_TYPE_NODE;
  item->browseName = "DataType1";
  item->sourceNodeId = (EdgeNodeId*) malloc (sizeof(EdgeNodeId));
  item->sourceNodeId->nodeId = NULL;    // no source node
  createNode(DEFAULT_NAMESPACE_VALUE, item);

  printf("\n[%d]) Data Type Node", ++index);
  item->nodeType = DATA_TYPE_NODE;
  item->browseName = "DataType2";
  item->sourceNodeId = (EdgeNodeId*) malloc (sizeof(EdgeNodeId));
  item->sourceNodeId->nodeId = "DataType1";
  createNode(DEFAULT_NAMESPACE_VALUE, item);

  /******************* View Node *********************/
  printf("\n-------------------------------------------------------");
  printf("\n[%d]) View Node", ++index);
  item->nodeType = VIEW_NODE;
  item->browseName = "ViewNode1";
  item->sourceNodeId = (EdgeNodeId*) malloc (sizeof(EdgeNodeId));
  item->sourceNodeId->nodeId = NULL;    // no source node
  createNode(DEFAULT_NAMESPACE_VALUE, item);

  printf("\n[%d]) View Node", ++index);
  item->nodeType = VIEW_NODE;
  item->browseName = "ViewNode2";
  item->sourceNodeId = (EdgeNodeId*) malloc (sizeof(EdgeNodeId));
  item->sourceNodeId->nodeId = "ViewNode1";    // no source node
  createNode(DEFAULT_NAMESPACE_VALUE, item);


  /******************* Reference Node *********************/
  printf("\n-------------------------------------------------------");
  printf("\n[%d]) Reference Node", ++index);
  EdgeReference* reference = (EdgeReference*) malloc(sizeof(EdgeReference));
  reference->forward=  false;
  reference->sourceNamespace = DEFAULT_NAMESPACE_VALUE;
  reference->sourcePath = "ObjectType1";
  reference->targetNamespace = DEFAULT_NAMESPACE_VALUE;
  reference->targetPath = "ObjectType2";
  addReference(reference);

  reference->forward=  true;
  reference->sourceNamespace = DEFAULT_NAMESPACE_VALUE;
  reference->sourcePath = "ObjectType1";
  reference->targetNamespace = DEFAULT_NAMESPACE_VALUE;
  reference->targetPath = "ObjectType3";
  addReference(reference);

  reference->forward=  true;
  reference->sourceNamespace = DEFAULT_NAMESPACE_VALUE;
  reference->sourcePath = "ObjectType1";
  reference->targetNamespace = DEFAULT_NAMESPACE_VALUE;
  reference->targetPath = "ObjectType4";
  addReference(reference);

  reference->forward=  true;
  reference->sourceNamespace = DEFAULT_NAMESPACE_VALUE;
  reference->sourcePath = "ObjectType1";
  reference->targetNamespace = DEFAULT_NAMESPACE_VALUE;
  reference->targetPath = "ObjectType5";
  addReference(reference);

  reference->forward=  true;
  reference->sourceNamespace = DEFAULT_NAMESPACE_VALUE;
  reference->sourcePath = "ObjectType5";
  reference->targetNamespace = DEFAULT_NAMESPACE_VALUE;
  reference->targetPath = "ObjectType1";
  addReference(reference);

  printf("\n[%d]) Reference Node", ++index);
  reference->forward=  true;
  reference->sourceNamespace = DEFAULT_NAMESPACE_VALUE;
  reference->sourcePath = "ViewNode1";
  reference->targetNamespace = DEFAULT_NAMESPACE_VALUE;
  reference->targetPath = "ObjectType1";
  addReference(reference);

  
  /******************* Reference Type Node *********************/
  printf("\n-------------------------------------------------------");
  printf("\n[%d]) Reference Type Node", ++index);
  item->nodeType = REFERENCE_TYPE_NODE;
  item->browseName = "ReferenceTypeNode1";
  item->sourceNodeId = (EdgeNodeId*) malloc (sizeof(EdgeNodeId));
  item->sourceNodeId->nodeId = NULL;    // no source node
  createNode(DEFAULT_NAMESPACE_VALUE, item);

  printf("\n[%d]) Reference Type Node with source node\"ReferenceTypeNode1\"", ++index);
  item->nodeType = REFERENCE_TYPE_NODE;
  item->browseName = "ReferenceTypeNode2";
  item->sourceNodeId = (EdgeNodeId*) malloc (sizeof(EdgeNodeId));
  item->sourceNodeId->nodeId = "ReferenceTypeNode1";
  createNode(DEFAULT_NAMESPACE_VALUE, item);

  /******************* Method Node *********************/
  printf("\n-------------------------------------------------------\n");
  printf("\n[%d]) Method Node \n", ++index);
  EdgeNodeItem *methodNodeItem = (EdgeNodeItem*) malloc(sizeof(EdgeNodeItem));
  methodNodeItem->browseName = "square_root";
  methodNodeItem->sourceNodeId = NULL;

  EdgeMethod* method = (EdgeMethod*) malloc(sizeof(EdgeMethod));
  method->description = "Calculate square root";
  method->methodNodeName = "square_root";
  method->method_fn = sqrt_method;
  method->num_inpArgs = 1;
  method->inpArg = (EdgeArgument**) malloc(sizeof(EdgeArgument*) * method->num_inpArgs);
  for (int idx = 0; idx < method->num_inpArgs; idx++) {
    method->inpArg[idx] = (EdgeArgument*) malloc(sizeof(EdgeArgument));
    method->inpArg[idx]->argType = Double;
    method->inpArg[idx]->valType = SCALAR;
  }

  method->num_outArgs = 1;
  method->outArg = (EdgeArgument**) malloc(sizeof(EdgeArgument*) * method->num_outArgs);
  for (int idx = 0; idx < method->num_outArgs; idx++) {
    method->outArg[idx] = (EdgeArgument*) malloc(sizeof(EdgeArgument));
    method->outArg[idx]->argType = Double;
    method->outArg[idx]->valType = SCALAR;
  }
  createMethodNode(DEFAULT_NAMESPACE_VALUE, methodNodeItem, method);

  printf("\n[%d]) Method Node \n", ++index);
  EdgeNodeItem *methodNodeItem1 = (EdgeNodeItem*) malloc(sizeof(EdgeNodeItem));
  methodNodeItem1->browseName = "incrementInc32Array";
  methodNodeItem1->sourceNodeId = NULL;

  EdgeMethod* method1 = (EdgeMethod*) malloc(sizeof(EdgeMethod));
  method1->description = "Increment int32 array by delta";
  method1->methodNodeName = "incrementInc32Array";
  method1->method_fn = increment_int32Array_method;

  method1->num_inpArgs = 2;
  method1->inpArg = (EdgeArgument**) malloc(sizeof(EdgeArgument*) * method1->num_inpArgs);
  method1->inpArg[0] = (EdgeArgument*) malloc(sizeof(EdgeArgument));
  method1->inpArg[0]->argType = Int32;
  method1->inpArg[0]->valType = ARRAY_1D;
  method1->inpArg[0]->arrayLength = 5;

  method1->inpArg[1] = (EdgeArgument*) malloc(sizeof(EdgeArgument));
  method1->inpArg[1]->argType = Int32;
  method1->inpArg[1]->valType = SCALAR;

  method1->num_outArgs = 1;
  method1->outArg = (EdgeArgument**) malloc(sizeof(EdgeArgument*) * method1->num_outArgs);
  for (int idx = 0; idx < method1->num_outArgs; idx++) {
    method1->outArg[idx] = (EdgeArgument*) malloc(sizeof(EdgeArgument));
    method1->outArg[idx]->argType = Int32;
    method1->outArg[idx]->valType = ARRAY_1D;
    method1->outArg[idx]->arrayLength = 5;
  }
  createMethodNode(DEFAULT_NAMESPACE_VALUE, methodNodeItem1, method1);

  printf("\n[%d]) Method Node \n", ++index);
  EdgeNodeItem *methodNodeItem2 = (EdgeNodeItem*) malloc(sizeof(EdgeNodeItem));
  methodNodeItem2->browseName = "noArgMethod";
  methodNodeItem2->sourceNodeId = NULL;

  EdgeMethod* method2 = (EdgeMethod*) malloc(sizeof(EdgeMethod));
  method2->description = "no arg method";
  method2->methodNodeName = "noArgMethod";
  method2->method_fn = arg_method;

  method2->num_inpArgs = 0;
  method2->inpArg = NULL;

  method2->num_outArgs = 0;
  method2->outArg = NULL;

  createMethodNode(DEFAULT_NAMESPACE_VALUE, methodNodeItem2, method2);

  printf("\n[%d]) Method Node \n", ++index);
  EdgeNodeItem *methodNodeItem3 = (EdgeNodeItem*) malloc(sizeof(EdgeNodeItem));
  methodNodeItem3->browseName = "inArgMethod";
  methodNodeItem3->sourceNodeId = NULL;

  EdgeMethod* method3 = (EdgeMethod*) malloc(sizeof(EdgeMethod));
  method3->description = "only input arg method";
  method3->methodNodeName = "inArgMethod";
  method3->method_fn = arg_method;

  method3->num_inpArgs = 1;
  method3->inpArg = (EdgeArgument**) malloc(sizeof(EdgeArgument*) * method3->num_inpArgs);
  method3->inpArg[0] = (EdgeArgument*) malloc(sizeof(EdgeArgument));
  method3->inpArg[0]->argType = Int32;
  method3->inpArg[0]->valType = SCALAR;

  method3->num_outArgs = 0;
  method3->outArg = NULL;

  createMethodNode(DEFAULT_NAMESPACE_VALUE, methodNodeItem3, method3);

  printf("\n[%d]) Method Node \n", ++index);
  EdgeNodeItem *methodNodeItem4 = (EdgeNodeItem*) malloc(sizeof(EdgeNodeItem));
  methodNodeItem4->browseName = "outArgMethod";
  methodNodeItem4->sourceNodeId = NULL;

  EdgeMethod* method4 = (EdgeMethod*) malloc(sizeof(EdgeMethod));
  method4->description = "only output arg method";
  method4->methodNodeName = "outArgMethod";
  method4->method_fn = arg_method;

  method4->num_inpArgs = 0;
  method4->inpArg = NULL;

  method4->num_outArgs = 1;
  method4->outArg = (EdgeArgument**) malloc(sizeof(EdgeArgument*) * method4->num_outArgs);
  method4->outArg[0] = (EdgeArgument*) malloc(sizeof(EdgeArgument));
  method4->outArg[0]->argType = Double;
  method4->outArg[0]->valType = SCALAR;

  createMethodNode(DEFAULT_NAMESPACE_VALUE, methodNodeItem4, method4);


  printf("\n-------------------------------------------------------");
  printf("\n\n");
}

static void testModifyNode() {

  printf("\n" COLOR_YELLOW "------------------------------------------------------" COLOR_RESET);
  printf("\n" COLOR_YELLOW "                  Modify Variable Node            "COLOR_RESET);
  printf("\n" COLOR_YELLOW "------------------------------------------------------" COLOR_RESET "\n\n");
  char s_value[512];
  double d_value;
  unsigned int u_value;
  int i_value;
  int option;
  void *new_value = NULL;
  char name[128];

  printf("\n\n" COLOR_YELLOW  "********************** Available nodes to test the 'modifyVariableNode' service **********************" COLOR_RESET "\n");
  printf("[1] String1\n");
  printf("[2] String2\n");
  printf("[3] String3\n");
  printf("[4] Double\n");
  printf("[5] Int32\n");
  printf("[6] UInt16\n");
  printf("\nEnter any of the above option :: ");
  scanf("%d", &option);

  if (option < 1 || option > 6) {
    printf( "Invalid Option!!! \n\n");
    return ;
  }

  printf("\nEnter the new value :: ");
  if (option == 1) {
    scanf("%s", s_value);
    strcpy(name, "String1");
    new_value = (void*) s_value;
  }
  else if (option == 2) {
    scanf("%s", s_value);
    strcpy(name, "String2");
    new_value = (void*) s_value;
  }
  else if (option == 3) {
    scanf("%s", s_value);
    strcpy(name, "String3");
    new_value = (void*) s_value;
  }
  else if (option == 4) {
    scanf("%lf", &d_value);
    strcpy(name, "Double");
    new_value = (void*) &d_value;
  }
  else if (option == 5) {
    scanf("%d", &i_value);
    strcpy(name, "Int32");
    new_value = (void*) &i_value;
  }
  else if (option == 6) {
    scanf("%u", &u_value);
    strcpy(name, "UInt16");
    new_value = (void*) &u_value;
  }

  EdgeVersatility *message = (EdgeVersatility*) malloc(sizeof(EdgeVersatility));
  message->value = new_value;

  modifyVariableNode(DEFAULT_NAMESPACE_VALUE, name, message);

  free(message); message = NULL;
}

static void deinit() {
  if (startFlag) {
    stopServer();
    startFlag = false;

    if (config) {
      if(config->recvCallback) {
        free (config->recvCallback);
        config->recvCallback = NULL;
      }
      if(config->statusCallback) {
        free (config->statusCallback);
        config->statusCallback = NULL;
      }
      if(config->discoveryCallback) {
        free (config->discoveryCallback);
        config->discoveryCallback = NULL;
      }
      free (config); config = NULL;
    }
  }
}

static void print_menu() {
  printf("=============== OPC UA =======================\n\n");

  printf("start : start opcua server\n");
  printf("modify_node : modify variable node\n");
  printf("quit : terminate/stop opcua server/client and then quit\n");
  printf("help : print menu\n");

  printf("\n=============== OPC UA =======================\n\n");
}

int main() {
  char command[128];

  print_menu();

  while (!stopFlag) {
    printf("[INPUT Command] : ");
    scanf("%s", command);

    if (stopFlag) {
      break;
    } else if(!strcmp(command, "start")) {

      printf("\n" COLOR_YELLOW " ------------------------------------------------------" COLOR_RESET);
      printf("\n" COLOR_YELLOW "                     Start Server            " COLOR_RESET);
      printf("\n" COLOR_YELLOW " ------------------------------------------------------" COLOR_RESET "\n\n");

//      printf("[Please input server address] : ");
//      scanf("%s", ipAddress);
      strcpy(ipAddress, "localhost");
      snprintf(endpointUri, sizeof(endpointUri), "opc:tcp://%s:12686/edge-opc-server", ipAddress);

      epInfo = (EdgeEndPointInfo*) malloc(sizeof(EdgeEndPointInfo));
      int len = strlen(endpointUri);
      epInfo->endpointUri = (char*) malloc(len +1);
      strncpy(epInfo->endpointUri, endpointUri, len);
      epInfo->endpointUri[len] = '\0';

      init();
      startServer();
      //startFlag = true;

    } else if(!strcmp(command, "modify_node")) {
      testModifyNode();
    } else if(!strcmp(command, "quit")) {
      deinit();
    } else if(!strcmp(command, "help")) {
      print_menu();
    }
  }
  return 0;
}

