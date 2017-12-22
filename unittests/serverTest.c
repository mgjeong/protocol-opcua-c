#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>

#include <pthread.h>

#include <opcua_manager.h>
#include "opcua_common.h"
#include "edge_identifier.h"

#define PRINT(str) printf(str)

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

static void monitored_msg_cb (void* data) {

}

static void error_msg_cb (void* data) {

}

static void browse_msg_cb (EdgeMessage* data) {

}

/* status callbacks */
static void status_start_cb (EdgeEndPointInfo* epInfo, EdgeStatusCode status) {
  if (status == STATUS_SERVER_STARTED) {
    PRINT("[Application Callback] Server started");
    startFlag = true;

    testCreateNamespace();
    testCreateNodes();
  }
}

static void status_stop_cb (EdgeEndPointInfo* epInfo, EdgeStatusCode status) {
  if (status == STATUS_STOP_SERVER) {
      PRINT("[Application Callback] Server stopped \n");
      //exit(0);
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
  int i;
  for (i = 0; i < 5; i++) {
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

    PRINT("\n Start Server");

    strcpy(ipAddress, "localhost");
    snprintf(endpointUri, sizeof(endpointUri), "opc:tcp://%s:12686/edge-opc-server", ipAddress);

    epInfo = (EdgeEndPointInfo*) malloc(sizeof(EdgeEndPointInfo));
    int len = strlen(endpointUri);
    epInfo->endpointUri = (char*) malloc(len +1);
    strncpy(epInfo->endpointUri, endpointUri, len);
    epInfo->endpointUri[len] = '\0';

    init();

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
  createNamespace(DEFAULT_NAMESPACE_VALUE,
                              DEFAULT_ROOT_NODE_INFO_VALUE,
                              DEFAULT_ROOT_NODE_INFO_VALUE,
                              DEFAULT_ROOT_NODE_INFO_VALUE);
}

static void testCreateNodes() {
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

  int value = 30;
  item->browseName = "UInt16";
  item->variableItemName = "Location";
  item->variableIdentifier = UInt16;
  item->variableData = (void*) &value;
  createNode(DEFAULT_NAMESPACE_VALUE, item);

  value = 40;
  item->browseName = "Int32";
  item->variableItemName = "Location";
  item->variableIdentifier = Int32;
  item->variableData = (void*) &value;
  createNode(DEFAULT_NAMESPACE_VALUE, item);

  double d_val = 50.4;
  item->browseName = "Double";
  item->variableItemName = "Location";
  item->variableIdentifier = Double;
  item->variableData = (void*) &d_val;
  createNode(DEFAULT_NAMESPACE_VALUE, item);

  bool flag = true;
  item->browseName = "Boolean";
  item->variableItemName = "Boolean";
  item->variableIdentifier = Boolean;
  item->variableData = (void*) &flag;
  createNode(DEFAULT_NAMESPACE_VALUE, item);

  double* data = (double*) malloc(sizeof(double) * 2);
  data[0] = 10.2;
  data[1] = 20.2;
  item->browseName = "IntArray";
  item->nodeType = ARRAY_NODE;
  item->variableItemName = "IntArray";
  item->variableIdentifier = Double;
  item->arrayLength = 2;
  item->variableData = (void*) data;
  createNode(DEFAULT_NAMESPACE_VALUE, item);

  char** data1 = (char**) malloc(sizeof(char*) * 2);
  data1[0] = (char*) malloc(10);
  strcpy(data1[0], "string");
  data1[1] = (char*) malloc(10);
  strcpy(data1[1], "array");
  item->browseName = "CharArray";
  item->nodeType = ARRAY_NODE;
  item->variableItemName = "CharArray";
  item->variableIdentifier = String;
  item->arrayLength = 2;
  item->variableData = (void*) (data1);
  createNode(DEFAULT_NAMESPACE_VALUE, item);

  /******************* Object Node *********************/
  item->nodeType = OBJECT_NODE;
  item->browseName = "Object1";
  item->sourceNodeId = (EdgeNodeId*) malloc (sizeof(EdgeNodeId));
  item->sourceNodeId->nodeId = NULL;    // no source node
  createNode(DEFAULT_NAMESPACE_VALUE, item);
  
  item->nodeType = OBJECT_NODE;
  item->browseName = "Object2";
  item->sourceNodeId = (EdgeNodeId*) malloc (sizeof(EdgeNodeId));
  item->sourceNodeId->nodeId = "Object1";
  createNode(DEFAULT_NAMESPACE_VALUE, item);

  /******************* Object Type Node *********************/
  item->nodeType = OBJECT_TYPE_NODE;
  item->browseName = "ObjectType1";
  item->sourceNodeId = (EdgeNodeId*) malloc (sizeof(EdgeNodeId));
  item->sourceNodeId->nodeId = NULL;    // no source node
  createNode(DEFAULT_NAMESPACE_VALUE, item);

  item->nodeType = OBJECT_TYPE_NODE;
  item->browseName = "ObjectType2";
  item->sourceNodeId = (EdgeNodeId*) malloc (sizeof(EdgeNodeId));
  item->sourceNodeId->nodeId = "ObjectType1";    // no source node
  createNode(DEFAULT_NAMESPACE_VALUE, item);

  /******************* Variable Type Node *********************/
  double d[2] = { 10.2, 20.2 };
  item->browseName = "DoubleVariableType";
  item->nodeType = VARIABLE_TYPE_NODE;
  item->variableItemName = "DoubleVariableType";
  item->variableIdentifier = Double;
  item->arrayLength = 2;
  item->variableData = (void*) (d);
  createNode(DEFAULT_NAMESPACE_VALUE, item);

  /******************* Data Type Node *********************/
  item->nodeType = DATA_TYPE_NODE;
  item->browseName = "DataType1";
  item->sourceNodeId = (EdgeNodeId*) malloc (sizeof(EdgeNodeId));
  item->sourceNodeId->nodeId = NULL;    // no source node
  createNode(DEFAULT_NAMESPACE_VALUE, item);

  item->nodeType = DATA_TYPE_NODE;
  item->browseName = "DataType2";
  item->sourceNodeId = (EdgeNodeId*) malloc (sizeof(EdgeNodeId));
  item->sourceNodeId->nodeId = "DataType1";
  createNode(DEFAULT_NAMESPACE_VALUE, item);

  /******************* View Node *********************/
  item->nodeType = VIEW_NODE;
  item->browseName = "ViewNode1";
  item->sourceNodeId = (EdgeNodeId*) malloc (sizeof(EdgeNodeId));
  item->sourceNodeId->nodeId = NULL;    // no source node
  createNode(DEFAULT_NAMESPACE_VALUE, item);

  item->nodeType = VIEW_NODE;
  item->browseName = "ViewNode2";
  item->sourceNodeId = (EdgeNodeId*) malloc (sizeof(EdgeNodeId));
  item->sourceNodeId->nodeId = "ViewNode1";    // no source node
  createNode(DEFAULT_NAMESPACE_VALUE, item);


  /******************* Reference Node *********************/
  EdgeReference* reference = (EdgeReference*) malloc(sizeof(EdgeReference));
  reference->forward=  false;
  reference->sourceNamespace = DEFAULT_NAMESPACE_VALUE;
  reference->sourcePath = "ObjectType1";
  reference->targetNamespace = DEFAULT_NAMESPACE_VALUE;
  reference->targetPath = "ObjectType2";
  addReference(reference);

  reference->forward=  true;
  reference->sourceNamespace = DEFAULT_NAMESPACE_VALUE;
  reference->sourcePath = "ViewNode1";
  reference->targetNamespace = DEFAULT_NAMESPACE_VALUE;
  reference->targetPath = "ObjectType2";
  addReference(reference);

  /******************* Reference Type Node *********************/
  item->nodeType = REFERENCE_TYPE_NODE;
  item->browseName = "ReferenceTypeNode1";
  item->sourceNodeId = (EdgeNodeId*) malloc (sizeof(EdgeNodeId));
  item->sourceNodeId->nodeId = NULL;    // no source node
  createNode(DEFAULT_NAMESPACE_VALUE, item);
  
  item->nodeType = REFERENCE_TYPE_NODE;
  item->browseName = "ReferenceTypeNode2";
  item->sourceNodeId = (EdgeNodeId*) malloc (sizeof(EdgeNodeId));
  item->sourceNodeId->nodeId = "ReferenceTypeNode1";
  createNode(DEFAULT_NAMESPACE_VALUE, item);

  /******************* Method Node *********************/
  EdgeNodeItem *methodNodeItem = (EdgeNodeItem*) malloc(sizeof(EdgeNodeItem));
  methodNodeItem->browseName = "square_root";
  methodNodeItem->sourceNodeId = NULL;

  EdgeMethod* method = (EdgeMethod*) malloc(sizeof(EdgeMethod));
  method->description = "Calculate square root";
  method->methodNodeName = "square_root";
  method->method_fn = sqrt_method;
  method->num_inpArgs = 1;
  method->inpArg = (EdgeArgument**) malloc(sizeof(EdgeArgument*) * method->num_inpArgs);
  int idx;
  for (idx = 0; idx < method->num_inpArgs; idx++) {
    method->inpArg[idx] = (EdgeArgument*) malloc(sizeof(EdgeArgument));
    method->inpArg[idx]->argType = Double;
    method->inpArg[idx]->valType = SCALAR;
  }

  method->num_outArgs = 1;
  method->outArg = (EdgeArgument**) malloc(sizeof(EdgeArgument*) * method->num_outArgs);
  for (idx = 0; idx < method->num_outArgs; idx++) {
    method->outArg[idx] = (EdgeArgument*) malloc(sizeof(EdgeArgument));
    method->outArg[idx]->argType = Double;
    method->inpArg[idx]->valType = SCALAR;
  }
  createMethodNode(DEFAULT_NAMESPACE_VALUE, methodNodeItem, method);

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
  for (idx = 0; idx < method1->num_outArgs; idx++) {
    method1->outArg[idx] = (EdgeArgument*) malloc(sizeof(EdgeArgument));
    method1->outArg[idx]->argType = Int32;
    method1->outArg[idx]->valType = ARRAY_1D;
    method1->outArg[idx]->arrayLength = 5;
  }
  createMethodNode(DEFAULT_NAMESPACE_VALUE, methodNodeItem1, method1);
  
}

static void deinit() {
  if (startFlag) {
    stopServer();
    startFlag = false;
  }
}

static void print_menu() {
  PRINT("=============== OPC UA =======================\n\n");

  PRINT("restart : Re start opcua server\n");
  PRINT("quit : terminate/stop opcua server/client and then quit\n");
  PRINT("help : print menu\n");

  PRINT("\n=============== OPC UA =======================\n\n");
}

int main() {
    char command[128];

    startServer();

    print_menu();

    while (!stopFlag) {
    PRINT("[INPUT Command] : ");
    scanf("%s", command);

    if (stopFlag) {
        break;
    } else if(!strcmp(command, "restart")) {

        PRINT("\n Start Server");

        deinit();

        startServer();

    } else if(!strcmp(command, "quit")) {
        deinit();
        stopFlag = true;
    } else if(!strcmp(command, "help")) {
        print_menu();
    }
  }
  return 0;
}