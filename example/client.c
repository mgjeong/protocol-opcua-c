#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include <opcua_manager.h>
#include <opcua_common.h>

#define COLOR_GREEN        "\x1b[32m"
#define COLOR_YELLOW      "\x1b[33m"
#define COLOR_PURPLE      "\x1b[35m"
#define COLOR_RESET         "\x1b[0m"

static bool startFlag = false;
static bool stopFlag = false;

static char ipAddress[512];
static char endpointUri[512];

static EdgeConfigure *config = NULL;

static char node_arr[6][10] = {
  "String1",
  "String2",
  "String3",
  "Double",
  "Int32",
  "UInt16"
};

static void startClient(char* addr, int port, char *securityPolicyUri);

static void response_msg_cb (EdgeMessage* data) {
  if (data->type == GENERAL_RESPONSE) {
    printf("[Application response Callback] General response received\n");
    int len = data->responseLength;
    int idx = 0;
    for (idx = 0; idx < len; idx++) {
      if(data->responses[idx]->type == Int16)
        printf("[Application response Callback] Data read from node ===>> [%d]\n", *((int*)data->responses[idx]->value));
      else if(data->responses[idx]->type == UInt16)
        printf("[Application response Callback] Data read from node ===>> [%d]\n", *((int*)data->responses[idx]->value));
      else if(data->responses[idx]->type == Int32)
        printf("[Application response Callback] Data read from node ===>>  [%d]\n", *((int*)data->responses[idx]->value));
      else if(data->responses[idx]->type == UInt32)
        printf("[Application response Callback] Data read from node ===>>  [%d]\n", *((int*)data->responses[idx]->value));
      else if(data->responses[idx]->type == Int64)
        printf("[Application response Callback] Data read from node ===>>  [%ld]\n", *((long*)data->responses[idx]->value));
      else if(data->responses[idx]->type == UInt64)
        printf("[Application response Callback] Data read from node ===>>  [%ld]\n", *((long*)data->responses[idx]->value));
      else if(data->responses[idx]->type == Float)
        printf("[Application response Callback] Data read from node ===>>  [%f]\n", *((float*)data->responses[idx]->value));
      else if(data->responses[idx]->type == Double)
        printf("[Application response Callback] Data read from node ===>>  [%f]\n", *((double*)data->responses[idx]->value));
      else if(data->responses[idx]->type == String)
        printf("[Application response Callback] Data read from node ===>>  [%s]\n", ((char*)data->responses[idx]->value));
    }
    printf("\n\n");
  }
}

static void monitored_msg_cb (void* data) {

}

static void error_msg_cb (void* data) {

}

static void browse_msg_cb (EdgeMessage* data) {
  if (data->type == BROWSE_RESPONSE) {
      EdgeBrowseResult **browseResult = data->browseResult;
      int idx = 0;
      printf("\n[Application browse response callback] List of Browse Names\n");
      printf("================================================\n");
      for (idx = 0; idx < data->browseResponseLength; idx++) {
        printf("[%d] %s\n", idx+1, browseResult[idx]->browseName);
      }
      printf("================================================\n");
    }
}

/* status callbacks */
static void status_start_cb (EdgeEndPointInfo* epInfo, EdgeStatusCode status) {
  if (status == STATUS_CLIENT_STARTED) {
    printf("[Application Callback] Client connected\n");
    startFlag = true;
  }
}

static void status_stop_cb (EdgeEndPointInfo* epInfo, EdgeStatusCode status) {
  if (status == STATUS_STOP_CLIENT) {
    printf("[Application Callback] Client disconnected \n\n");
    stopFlag = true;
  }
}

static void status_network_cb (EdgeEndPointInfo* epInfo, EdgeStatusCode status) {

}

/* discovery callback */
static void endpoint_found_cb (EdgeDevice* device) {
  if (device) {
    int num_endpoints = device->num_endpoints;
    int idx = 0;
    for (idx = 0; idx < num_endpoints; idx++) {
      printf("\n[Application Callback] EndpointUri :: %s\n", device->endpointsInfo[idx]->endpointUri);
      printf("[Application Callback] Address :: %s, Port : %d, ServerName :: %s\n", device->address, device->port, device->serverName);
      printf("[Application Callback] SecurityPolicyUri :: %s\n\n", device->endpointsInfo[idx]->config->securityPolicyUri);

      startClient(device->address, device->port, device->endpointsInfo[idx]->config->securityPolicyUri);
    }
  }
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

static void testGetEndpoints() {
  EdgeEndpointConfig* endpointConfig = (EdgeEndpointConfig*) malloc(sizeof(EdgeEndpointConfig));
  endpointConfig->applicationName = DEFAULT_SERVER_APP_NAME_VALUE;
  endpointConfig->applicationUri = DEFAULT_SERVER_APP_URI_VALUE;
  endpointConfig->productUri = DEFAULT_PRODUCT_URI_VALUE;
  endpointConfig->serverName = DEFAULT_SERVER_NAME_VALUE;

  EdgeEndPointInfo* ep = (EdgeEndPointInfo*) malloc(sizeof(EdgeEndPointInfo));
  ep->endpointUri = endpointUri;
  ep->config = endpointConfig;

  EdgeMessage* msg = (EdgeMessage*) malloc(sizeof(EdgeMessage));
  msg->endpointInfo = ep;
  msg->command = CMD_START_SERVER;
  msg->type = SEND_REQUEST;

  getEndpointInfo(ep);
}

static void startClient(char* addr, int port, char *securityPolicyUri) {
  printf("\n" COLOR_YELLOW "------------------------------------------------------" COLOR_RESET);
  printf("\n" COLOR_YELLOW "                       Client connect            "COLOR_RESET);
  printf("\n" COLOR_YELLOW "------------------------------------------------------" COLOR_RESET "\n\n");

  EdgeEndpointConfig* endpointConfig = (EdgeEndpointConfig*) malloc(sizeof(EdgeEndpointConfig));
  endpointConfig->bindAddress = addr;
  endpointConfig->bindPort = port;
  endpointConfig->applicationName = DEFAULT_SERVER_APP_NAME_VALUE;
  endpointConfig->applicationUri = DEFAULT_SERVER_APP_URI_VALUE;
  endpointConfig->productUri = DEFAULT_PRODUCT_URI_VALUE;
  endpointConfig->securityPolicyUri = securityPolicyUri;
  endpointConfig->serverName = DEFAULT_SERVER_NAME_VALUE;
  endpointConfig->requestTimeout = 60000;

  EdgeEndPointInfo* ep = (EdgeEndPointInfo*) malloc(sizeof(EdgeEndPointInfo));
  ep->endpointUri = endpointUri;
  ep->config = endpointConfig;

  EdgeMessage* msg = (EdgeMessage*) malloc(sizeof(EdgeMessage));
  msg->endpointInfo = ep;
  msg->command = CMD_START_CLIENT;
  msg->type = SEND_REQUEST;

  printf("\n********************** startClient **********************\n");
  connectClient(ep);
}

static void stopClient() {
  EdgeEndPointInfo* ep = (EdgeEndPointInfo*) malloc(sizeof(EdgeEndPointInfo));
  ep->endpointUri = endpointUri;
  ep->config = NULL;

  EdgeMessage* msg = (EdgeMessage*) malloc(sizeof(EdgeMessage));
  msg->endpointInfo = ep;
  msg->command = CMD_STOP_CLIENT;

  printf("\n********************** stop client **********************\n");
  disconnectClient(ep);
}

static void deinit() {
  stopClient();
}

static void testBrowse() {
  EdgeEndPointInfo* ep = (EdgeEndPointInfo*) malloc(sizeof(EdgeEndPointInfo));
  ep->endpointUri = endpointUri;
  ep->config = NULL;

  EdgeNodeInfo *nodeInfo = (EdgeNodeInfo*) malloc(sizeof(EdgeNodeInfo));
  nodeInfo->nodeId = (EdgeNodeId*) malloc(sizeof(EdgeNodeId));
  nodeInfo->nodeId->type = INTEGER;
  nodeInfo->nodeId->integerNodeId = RootFolder;
  nodeInfo->nodeId->nameSpace = SYSTEM_NAMESPACE_INDEX;

  EdgeRequest *request = (EdgeRequest*) malloc(sizeof(EdgeRequest));
  request->nodeInfo = nodeInfo;

  EdgeMessage *msg = (EdgeMessage*) malloc(sizeof(EdgeMessage));
  msg->endpointInfo = ep;
  msg->command = CMD_BROWSE;
  msg->request = request;

  printf("\n\n" COLOR_YELLOW "********** Browse Nodes in RootFolder in system namespace **********" COLOR_RESET "\n");
  browseNode(msg);

  // ------------------------------------------------- //
  free (nodeInfo);
  nodeInfo = NULL;

  nodeInfo = (EdgeNodeInfo*) malloc(sizeof(EdgeNodeInfo));
  nodeInfo->nodeId = (EdgeNodeId*) malloc(sizeof(EdgeNodeId));

  nodeInfo->nodeId->type = INTEGER;
  nodeInfo->nodeId->integerNodeId = ObjectsFolder;
  nodeInfo->nodeId->nameSpace = SYSTEM_NAMESPACE_INDEX;

  request->nodeInfo = nodeInfo;
  msg->request = request;

  printf("\n" COLOR_YELLOW "********** Browse Nodes in Objects in system namespace **********" COLOR_RESET  "\n");
  browseNode(msg);

  // ------------------------------------------------- //
  free (nodeInfo);
  nodeInfo = NULL;

  nodeInfo = (EdgeNodeInfo*) malloc(sizeof(EdgeNodeInfo));
  nodeInfo->nodeId = (EdgeNodeId*) malloc(sizeof(EdgeNodeId));

  nodeInfo->nodeId->type = STRING;
  nodeInfo->nodeId->nodeId = "Object1";
  nodeInfo->nodeId->nameSpace = 1;

  request->nodeInfo = nodeInfo;
  msg->request = request;

  printf("\n" COLOR_YELLOW "********** Browse \"Object1\" node **********" COLOR_RESET "\n");
  browseNode(msg);

  // ------------------------------------------------- //
  free (nodeInfo);
  nodeInfo = NULL;
  free (request); request = NULL;
  free (ep); ep = NULL;
  free(msg); msg = NULL;
}

static void testRead() {
  int option;
  printf("\n\n" COLOR_YELLOW  "********************** Available nodes to test the read service **********************" COLOR_RESET "\n");
  printf("[1] String1\n");
  printf("[2] String2\n");
  printf("[3] String3\n");
  printf("[4] Double\n");
  printf("[5] Int32\n");
  printf("[6] UInt16\n");
  printf("\nEnter any of the above option to read that node :: ");
  scanf("%d", &option);

  if (option < 1 || option > 6) {
    printf( "Invalid Option!!! \n\n");
    return ;
  }

  printf("\n\n" COLOR_YELLOW  "**********************  Reading the node with browse name \"%s\" **********************" COLOR_RESET "\n\n", node_arr[option-1]);
  EdgeEndPointInfo* ep = (EdgeEndPointInfo*) malloc(sizeof(EdgeEndPointInfo));
  ep->endpointUri = endpointUri;
  ep->config = NULL;

  EdgeNodeInfo *nodeInfo = (EdgeNodeInfo*) malloc(sizeof(EdgeNodeInfo));
  nodeInfo->valueAlias = node_arr[option-1];

  EdgeRequest *request = (EdgeRequest*) malloc(sizeof(EdgeRequest));
  request->nodeInfo = nodeInfo;

  EdgeMessage *msg = (EdgeMessage*) malloc(sizeof(EdgeMessage));
  msg->endpointInfo = ep;
  msg->command = CMD_READ;
  msg->request = request;

  readNode(msg);
}

static void testWrite() {
  char s_value[512];
  double d_value;
  unsigned int u_value;
  int i_value;
  int option;
  EdgeNodeIdentifier type;
  void *new_value = NULL;

  printf("\n\n" COLOR_YELLOW  "********************** Available nodes to test the write service **********************" COLOR_RESET "\n");
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
  if (option == 1 || option == 2 || option == 3) {
    scanf("%s", s_value);
    type = String;
    new_value = (void*) s_value;
  }
  else if (option == 4) {
    scanf("%lf", &d_value);
    type = Double;
    new_value = (void*) &d_value;
  }
  else if (option == 5) {
    scanf("%d", &i_value);
    type = Int32;
    new_value = (void*) &i_value;
  }
  else if (option == 6) {
    scanf("%u", &u_value);
    type = UInt16;
    new_value = (void*) &u_value;
  }

  printf("\n\n" COLOR_YELLOW  "********************** Writing the value to the node \"%s\" **********************" COLOR_RESET  "\n\n", node_arr[option-1]);
  EdgeEndPointInfo* ep = (EdgeEndPointInfo*) malloc(sizeof(EdgeEndPointInfo));
  ep->endpointUri = endpointUri;
  ep->config = NULL;

  EdgeNodeInfo *nodeInfo = (EdgeNodeInfo*) malloc(sizeof(EdgeNodeInfo));
  nodeInfo->valueAlias = node_arr[option-1];

  EdgeRequest *request = (EdgeRequest*) malloc(sizeof(EdgeRequest));
  request->nodeInfo = nodeInfo;
  request->type = type;
  request->value = new_value;

  EdgeMessage *msg = (EdgeMessage*) malloc(sizeof(EdgeMessage));
  msg->endpointInfo = ep;
  msg->command = CMD_WRITE;
  msg->request = request;

  writeNode(msg);
}

static void print_menu() {
  printf("=============== OPC UA =======================\n\n");

  printf("start : Get endpoints and start opcua client \n");
  printf("read : read attribute for target node\n");
  printf("write : write attribute into nodes\n");
  printf("browse : browse nodes\n");
  printf("quit : terminate/stop opcua server/client and then quit\n");
  printf("help : print menu\n");

  printf("\n=============== OPC UA =======================\n\n");
}

int main() {
  char command[128];

  print_menu();
  init();

  while (!stopFlag) {
    printf("\n\n[INPUT Command] : ");
    scanf("%s", command);

    if (stopFlag) {
      break;
    } else if(!strcmp(command, "start")) {

      printf("\n" COLOR_YELLOW "------------------------------------------------------" COLOR_RESET);
      printf("\n" COLOR_YELLOW "                  Client get endpoints             " COLOR_RESET);
      printf("\n" COLOR_YELLOW "------------------------------------------------------" COLOR_RESET "\n\n");

      //printf("[Please input server endpoint uri] : ");
      //scanf("%s", ipAddress);
      int len = strlen("opc.tcp://localhost:12686/edge-opc-server");
      strcpy(ipAddress, "opc.tcp://localhost:12686/edge-opc-server");
      ipAddress[len] = '\0';
      strcpy(endpointUri, ipAddress);
      endpointUri[len] = '\0';

      testGetEndpoints();
      //startFlag = true;

    } else if(!strcmp(command, "read_t")) {
      testRead();
    } else if(!strcmp(command, "write_t")) {
      testWrite();
    } else if(!strcmp(command, "browse")) {
      testBrowse();
    } else if(!strcmp(command, "quit")) {
      deinit();
      //stopFlag = true;
    } else if(!strcmp(command, "help")) {
      print_menu();
    }
  }

  return 0;
}
