#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include <pthread.h>

#include <opcua_manager.h>
#include "opcua_common.h"
#include "edge_identifier.h"

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

static void monitored_msg_cb (void* data) {

}

static void error_msg_cb (void* data) {

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

  printf("\n[%d]) Variable node with int variant ", ++index);
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

  printf("\n[%d]) Variable node with boolean variant ", ++index);
  bool flag = true;
  item->browseName = "Boolean";
  item->variableItemName = "Boolean";
  item->variableIdentifier = Boolean;
  item->variableData = (void*) &flag;
  createNode(DEFAULT_NAMESPACE_VALUE, item);

  /******************* Array *********************/
  printf("\n-------------------------------------------------------");
  printf("\n[%d]) Array node with double values ", ++index);
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

  printf("\n[%d]) Array node with string values ", ++index);
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

  printf("\n[%d]) Reference Node", ++index);
  reference->forward=  true;
  reference->sourceNamespace = DEFAULT_NAMESPACE_VALUE;
  reference->sourcePath = "ViewNode1";
  reference->targetNamespace = DEFAULT_NAMESPACE_VALUE;
  reference->targetPath = "ObjectType2";
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



  printf("\n-------------------------------------------------------");
  printf("\n\n");
}

static void deinit() {
  if (startFlag) {
    stopServer();
    startFlag = false;
  }
}

static void print_menu() {
  printf("=============== OPC UA =======================\n\n");

  printf("start : start opcua server\n");
//  printf("start CNC : start CNC server cycle\n");
//  printf("getnode : get node information\n");
//  printf("getnode2 : get node information with browseName\n");
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

    } else if(!strcmp(command, "start CNC")) {

    } else if(!strcmp(command, "getnode")) {

    } else if(!strcmp(command, "getnode2")) {

    } else if(!strcmp(command, "quit")) {
      deinit();
    } else if(!strcmp(command, "help")) {
      print_menu();
    }
  }
  return 0;
}

