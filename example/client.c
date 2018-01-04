#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>
#include <inttypes.h>

#include <opcua_manager.h>
#include <opcua_common.h>

#define COLOR_GREEN        "\x1b[32m"
#define COLOR_YELLOW      "\x1b[33m"
#define COLOR_PURPLE      "\x1b[35m"
#define COLOR_RESET         "\x1b[0m"

#define MAX_CHAR_SIZE   512
#define NODE_COUNT  6

static bool startFlag = false;
static bool stopFlag = false;

static char ipAddress[MAX_CHAR_SIZE];
static char endpointUri[MAX_CHAR_SIZE];

static EdgeConfigure *config = NULL;

static char node_arr[NODE_COUNT][MAX_CHAR_SIZE] = {
  "String1",
  "String2",
  "String3",
  "Double",
  "Int32",
  "UInt16"
};

static bool subscription_done[NODE_COUNT] = {
  false,
  false,
  false,
  false,
  false,
  false
};

static void startClient(char* addr, int port, char *securityPolicyUri);

static void response_msg_cb (EdgeMessage* data) {
  if (data->type == GENERAL_RESPONSE) {
    printf("[Application response Callback] General response received\n");
    int len = data->responseLength;
    int idx = 0;
    for (idx = 0; idx < len; idx++) {
      if (data->responses[idx]->message != NULL) {
        if (data->responses[idx]->message->isArray) {
          // Handle Output array
          int arrayLen = data->responses[idx]->message->arrayLength;
          if(data->responses[idx]->type == Boolean) {
            /* Handle Boolean output array */
            printf("Boolean output array length :: [%d]\n\n", arrayLen);
            for (int arrayIdx = 0; arrayIdx < arrayLen; arrayIdx++) {
              printf("%d  ", ((bool*) data->responses[idx]->message->value)[arrayIdx]);
            }
          } else if(data->responses[idx]->type == Byte) {
            /* Handle Byte output array */
            printf("Byte output array length :: [%d]\n\n", arrayLen);
            for (int arrayIdx = 0; arrayIdx < arrayLen; arrayIdx++) {
              printf("%" PRIu8 " ", ((uint8_t*) data->responses[idx]->message->value)[arrayIdx]);
            }
          } else if(data->responses[idx]->type == SByte) {
            /* Handle SByte output array */
            printf("SByte output array length :: [%d]\n\n", arrayLen);
            for (int arrayIdx = 0; arrayIdx < arrayLen; arrayIdx++) {
              printf("%" PRId8 " ", ((int8_t*) data->responses[idx]->message->value)[arrayIdx]);
            }
          } else if(data->responses[idx]->type == Int16) {
            /* Handle int16 output array */
            printf("Int16 output array length :: [%d]\n\n", arrayLen);
            for (int arrayIdx = 0; arrayIdx < arrayLen; arrayIdx++) {
              printf("%" PRId16 "  ", ((int16_t*) data->responses[idx]->message->value)[arrayIdx]);
            }
          } else if(data->responses[idx]->type == UInt16)  {
            /* Handle UInt16 output array */
            printf("UInt16 output array length :: [%d]\n\n", arrayLen);
            for (int arrayIdx = 0; arrayIdx < arrayLen; arrayIdx++) {
              printf("%" PRIu16 "  ", ((uint16_t*) data->responses[idx]->message->value)[arrayIdx]);
            }
          } else if(data->responses[idx]->type == Int32) {
            /* Handle Int32 output array */
            printf("Int32 output array length :: [%d]\n\n", arrayLen);
            for (int arrayIdx = 0; arrayIdx < arrayLen; arrayIdx++) {
              printf("%d  ", ((int32_t*) data->responses[idx]->message->value)[arrayIdx]);
            }
          } else if(data->responses[idx]->type == UInt32) {
            /* Handle UInt32 output array */
            printf("UInt32 output array length :: [%d]\n\n", arrayLen);
            for (int arrayIdx = 0; arrayIdx < arrayLen; arrayIdx++) {
              printf("%u  ", ((uint*) data->responses[idx]->message->value)[arrayIdx]);
            }
          } else if(data->responses[idx]->type == Int64) {
            /* Handle Int64 output array */
            printf("Int64 output array length :: [%d]\n\n", arrayLen);
            for (int arrayIdx = 0; arrayIdx < arrayLen; arrayIdx++) {
              printf("%ld  ", ((long int*) data->responses[idx]->message->value)[arrayIdx]);
            }
          } else if(data->responses[idx]->type == UInt64) {
            /* Handle UInt64 output array */
            printf("UInt64 output array length :: [%d]\n\n", arrayLen);
            for (int arrayIdx = 0; arrayIdx < arrayLen; arrayIdx++) {
              printf("%lu  ", ((ulong*) data->responses[idx]->message->value)[arrayIdx]);
            }
          } else if(data->responses[idx]->type == Float) {
            /* Handle Float output array */
            printf("Float output array length :: [%d]\n\n", arrayLen);
            for (int arrayIdx = 0; arrayIdx < arrayLen; arrayIdx++) {
              printf("%g  ", ((float*) data->responses[idx]->message->value)[arrayIdx]);
            }
          } else if(data->responses[idx]->type == Double) {
            /* Handle Double output array */
            printf("Double output array length :: [%d]\n\n", arrayLen);
            for (int arrayIdx = 0; arrayIdx < arrayLen; arrayIdx++) {
              printf("%g  ", ((double*) data->responses[idx]->message->value)[arrayIdx]);
            }
          } else if(data->responses[idx]->type == String || data->responses[idx]->type == ByteString
                    || data->responses[idx]->type == Guid) {
            /* Handle String/ByteString/Guid output array */
            printf("String/ByteString/Guid output array length :: [%d]\n\n", arrayLen);
            char **values = ((char**) data->responses[idx]->message->value);
            for (int arrayIdx = 0; arrayIdx < arrayLen; arrayIdx++) {
              printf("%s  ", values[arrayIdx]);
            }
          } else if(data->responses[idx]->type == DateTime) {
            /* Handle DateTime output array */
            printf("DateTime output array length :: [%d]\n\n", arrayLen);
            for (int arrayIdx = 0; arrayIdx < arrayLen; arrayIdx++) {
              printf("%" PRId64 "  ", ((int64_t*) data->responses[idx]->message->value)[arrayIdx]);
            }
          }
        } else {
          if(data->responses[idx]->type == Boolean)
            printf("[Application response Callback] Data read from node ===>> [%d]\n", *((int*)data->responses[idx]->message->value));
          else if(data->responses[idx]->type == Byte)
            printf("[Application response Callback] Data read from node ===>> [%" PRIu8 "]\n", *((uint8_t*)data->responses[idx]->message->value));
          else if(data->responses[idx]->type == SByte)
            printf("[Application response Callback] Data read from node ===>> [%" PRId8 "]\n", *((int8_t*)data->responses[idx]->message->value));
          else if(data->responses[idx]->type == ByteString)
            printf("[Application response Callback] Data read from node ===>> [%s]\n", (char*)data->responses[idx]->message->value);
          else if(data->responses[idx]->type == DateTime)
            printf("[Application response Callback] Data read from node ===>> [%" PRId64 "]\n", *((int64_t*)data->responses[idx]->message->value));
          else if(data->responses[idx]->type == Double)
            printf("[Application response Callback] Data read from node ===>>  [%g]\n", *((double*)data->responses[idx]->message->value));
          else if(data->responses[idx]->type == Float)
            printf("[Application response Callback] Data read from node ===>>  [%g]\n", *((float*)data->responses[idx]->message->value));
          else if(data->responses[idx]->type == Int16)
            printf("[Application response Callback] Data read from node ===>> [%" PRId16 "]\n", *((int16_t*)data->responses[idx]->message->value));
          else if(data->responses[idx]->type == UInt16)
            printf("[Application response Callback] Data read from node ===>> [%" PRIu16 "]\n", *((uint16_t*)data->responses[idx]->message->value));
          else if(data->responses[idx]->type == Int32)
            printf("[Application response Callback] Data read from node ===>>  [%" PRId32 "]\n", *((int32_t*)data->responses[idx]->message->value));
          else if(data->responses[idx]->type == UInt32)
            printf("[Application response Callback] Data read from node ===>>  [%" PRIu32 "]\n", *((uint32_t*)data->responses[idx]->message->value));
          else if(data->responses[idx]->type == Int64)
            printf("[Application response Callback] Data read from node ===>>  [%" PRId64 "]\n", *((int64_t*)data->responses[idx]->message->value));
          else if(data->responses[idx]->type == UInt64)
            printf("[Application response Callback] Data read from node ===>>  [%" PRIu64 "]\n", *((uint64_t*)data->responses[idx]->message->value));
          else if(data->responses[idx]->type == String)
            printf("[Application response Callback] Data read from node ===>>  [%s]\n", (char*)data->responses[idx]->message->value);
        }
      }
    }
    printf("\n\n");
  }
}

static void monitored_msg_cb (EdgeMessage* data) {
  if (data->type == REPORT) {
    printf("[Application response Callback] Monitored Item Response received\n");
    int len = data->responseLength;
    int idx = 0;
    for (idx = 0; idx < len; idx++) {
      if (data->responses[idx]->message != NULL) {
        if(data->responses[idx]->type == Int16)
          printf("[MonitoredItem DataChange callback] Monitored Change Value read from node ===>> [%d]\n", *((int*)data->responses[idx]->message->value));
        else if(data->responses[idx]->type == UInt16)
          printf("[MonitoredItem DataChange callback] Monitored Change Value read from node ===>> [%d]\n", *((int*)data->responses[idx]->message->value));
        else if(data->responses[idx]->type == Int32)
          printf("[MonitoredItem DataChange callback] Monitored Change Value read from node ===>>  [%d]\n", *((int*)data->responses[idx]->message->value));
        else if(data->responses[idx]->type == UInt32)
          printf("[MonitoredItem DataChange callbackk] Monitored Change Value read from node ===>>  [%d]\n", *((int*)data->responses[idx]->message->value));
        else if(data->responses[idx]->type == Int64)
          printf("[MonitoredItem DataChange callbackk] Monitored Change Value read from node ===>>  [%ld]\n", *((long*)data->responses[idx]->message->value));
        else if(data->responses[idx]->type == UInt64)
          printf("[MonitoredItem DataChange callbackk] Monitored Change Value read from node ===>>  [%ld]\n", *((long*)data->responses[idx]->message->value));
        else if(data->responses[idx]->type == Float)
          printf("[MonitoredItem DataChange callback] Monitored Change Value read from node  ===>>  [%f]\n", *((float*)data->responses[idx]->message->value));
        else if(data->responses[idx]->type == Double)
          printf("[MonitoredItem DataChange callback] Monitored Change Value read from node  ===>>  [%f]\n", *((double*)data->responses[idx]->message->value));
        else if(data->responses[idx]->type == String)
          printf("[MonitoredItem DataChange callback] Monitored Change Value read from node ===>>  [%s]\n", ((char*)data->responses[idx]->message->value));
      }
    }
    printf("\n\n");
  }
}

static void error_msg_cb (EdgeMessage* data) {
  printf("[error_msg_cb] EdgeStatusCode: %d\n", data->result->code);
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

static int getNodeOptions(const char *menu) {
  printf("\n" COLOR_YELLOW "------------------------------------------------------" COLOR_RESET);
  printf("\n" COLOR_YELLOW "                     %s            "COLOR_RESET, menu);
  printf("\n" COLOR_YELLOW "------------------------------------------------------" COLOR_RESET "\n\n");
  int option;
  printf("\n\n" COLOR_YELLOW  "********************** Available nodes to test the '%s' service **********************" COLOR_RESET "\n", menu);
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
    return -1;
  }
  return option;
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

  free(endpointConfig); endpointConfig = NULL;
  free(ep); ep = NULL;
  free(msg); msg = NULL;
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

  printf("\n" COLOR_YELLOW "********************** startClient **********************" COLOR_RESET"\n");
  connectClient(ep);

  free(endpointConfig); endpointConfig = NULL;
  free(ep); ep = NULL;
  free(msg); msg = NULL;
}

static void stopClient() {
  EdgeEndPointInfo* ep = (EdgeEndPointInfo*) malloc(sizeof(EdgeEndPointInfo));
  ep->endpointUri = endpointUri;
  ep->config = NULL;

  EdgeMessage* msg = (EdgeMessage*) malloc(sizeof(EdgeMessage));
  msg->endpointInfo = ep;
  msg->command = CMD_STOP_CLIENT;

  printf("\n" COLOR_YELLOW "********************** stop client **********************" COLOR_RESET"\n");
  disconnectClient(ep);

  free(ep); ep = NULL;
  free(msg); msg = NULL;
}

static void deinit() {
  stopClient();
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

static void testBrowse() {
  printf("\n" COLOR_YELLOW "------------------------------------------------------" COLOR_RESET);
  printf("\n" COLOR_YELLOW "                       Browse Node            "COLOR_RESET);
  printf("\n" COLOR_YELLOW "------------------------------------------------------" COLOR_RESET "\n\n");
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
  free (nodeInfo); nodeInfo = NULL;
  free(ep); ep = NULL;
  free(request); request = NULL;
  free(msg); msg = NULL;
}

static void testRead() {
  // Get the list of browse names and display them to user.
  testBrowse();

  // Get the browse name of the node to be read from the user.
  char browseName[MAX_CHAR_SIZE];
  printf("Enter the browse name of the node to read:");
  scanf("%s", browseName);

  printf("\n\n" COLOR_YELLOW  "**********************  Reading the node with browse name \"%s\" **********************" COLOR_RESET "\n\n", browseName);
  EdgeEndPointInfo* ep = (EdgeEndPointInfo*) malloc(sizeof(EdgeEndPointInfo));
  ep->endpointUri = endpointUri;
  ep->config = NULL;

  EdgeNodeInfo *nodeInfo = (EdgeNodeInfo*) malloc(sizeof(EdgeNodeInfo));
  nodeInfo->valueAlias = browseName;

  EdgeRequest *request = (EdgeRequest*) malloc(sizeof(EdgeRequest));
  request->nodeInfo = nodeInfo;

  EdgeMessage *msg = (EdgeMessage*) malloc(sizeof(EdgeMessage));
  msg->endpointInfo = ep;
  msg->command = CMD_READ;
  msg->request = request;

  readNode(msg);

  free (nodeInfo); nodeInfo = NULL;
  free(ep); ep = NULL;
  free(request); request = NULL;
  free(msg); msg = NULL;
}

static void testWrite() {
  int option = getNodeOptions("Write");
  if (option == -1)
    return ;

  char s_value[MAX_CHAR_SIZE];
  double d_value;
  unsigned int u_value;
  int i_value;
  EdgeNodeIdentifier type;
  void *new_value = NULL;

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

  free (nodeInfo); nodeInfo = NULL;
  free(ep); ep = NULL;
  free(request); request = NULL;
  free(msg); msg = NULL;
}

static void testMethod() {
  printf("\n" COLOR_YELLOW "------------------------------------------------------" COLOR_RESET);
  printf("\n" COLOR_YELLOW "                       Method Call            "COLOR_RESET);
  printf("\n" COLOR_YELLOW "------------------------------------------------------" COLOR_RESET "\n\n");

  EdgeEndPointInfo* ep = (EdgeEndPointInfo*) malloc(sizeof(EdgeEndPointInfo));
  ep->endpointUri = endpointUri;
  ep->config = NULL;

  EdgeNodeInfo *nodeInfo = (EdgeNodeInfo*) malloc(sizeof(EdgeNodeInfo));
  nodeInfo->valueAlias = "square_root";

  EdgeMethodRequestParams *methodParams = (EdgeMethodRequestParams*) malloc(sizeof(EdgeMethodRequestParams));
  methodParams->num_inpArgs = 1;
  methodParams->inpArg = (EdgeArgument**) malloc(sizeof(EdgeArgument*) * methodParams->num_inpArgs);
  methodParams->inpArg[0] = (EdgeArgument*) malloc(sizeof(EdgeArgument));
  methodParams->inpArg[0]->argType = Double;
  methodParams->inpArg[0]->valType = SCALAR;
  double d = 16.0;
  methodParams->inpArg[0]->scalarValue = (void*) &d;

  EdgeRequest *request = (EdgeRequest*) malloc(sizeof(EdgeRequest));
  request->nodeInfo = nodeInfo;
  request->methodParams = methodParams;

  EdgeMessage *msg = (EdgeMessage*) malloc(sizeof(EdgeMessage));
  msg->endpointInfo = ep;
  msg->command = CMD_METHOD;
  msg->request = request;

  callMethod(msg);

  for (int i = 0; i < methodParams->num_inpArgs; i++) {
    free (methodParams->inpArg[i]); methodParams->inpArg[i] = NULL;
  }
  free (methodParams->inpArg); methodParams->inpArg = NULL;
  free (methodParams); methodParams = NULL;

  free(nodeInfo); nodeInfo = NULL;
  free(request); request = NULL;
  free(ep); ep = NULL;
  free (msg); msg = NULL;

  printf("\n=====================================\n\n");

  ep = (EdgeEndPointInfo*) malloc(sizeof(EdgeEndPointInfo));
  ep->endpointUri = endpointUri;
  ep->config = NULL;

  nodeInfo = (EdgeNodeInfo*) malloc(sizeof(EdgeNodeInfo));
  nodeInfo->valueAlias = "incrementInc32Array";

  methodParams = (EdgeMethodRequestParams*) malloc(sizeof(EdgeMethodRequestParams));
  methodParams->num_inpArgs = 2;
  methodParams->inpArg = (EdgeArgument**) malloc(sizeof(EdgeArgument*) * methodParams->num_inpArgs);
  methodParams->inpArg[0] = (EdgeArgument*) malloc(sizeof(EdgeArgument));
  methodParams->inpArg[0]->argType = Int32;
  methodParams->inpArg[0]->valType = ARRAY_1D;
  int32_t array[5] = {10,20,30,40,50};
  methodParams->inpArg[0]->arrayData = (void*) array;
  methodParams->inpArg[0]->arrayLength = 5;

  methodParams->inpArg[1] = (EdgeArgument*) malloc(sizeof(EdgeArgument));
  methodParams->inpArg[1]->argType = Int32;
  methodParams->inpArg[1]->valType = SCALAR;
  int delta = 5;
  methodParams->inpArg[1]->scalarValue = (void*) &delta;

  request = (EdgeRequest*) malloc(sizeof(EdgeRequest));
  request->nodeInfo = nodeInfo;
  request->methodParams = methodParams;

  msg = (EdgeMessage*) malloc(sizeof(EdgeMessage));
  msg->endpointInfo = ep;
  msg->command = CMD_METHOD;
  msg->request = request;

  callMethod(msg);

  for (int i = 0; i < methodParams->num_inpArgs; i++) {
    free (methodParams->inpArg[i]); methodParams->inpArg[i] = NULL;
  }
  free (methodParams->inpArg); methodParams->inpArg = NULL;
  free (methodParams); methodParams = NULL;

  free(nodeInfo); nodeInfo = NULL;
  free(request); request = NULL;
  free(ep); ep = NULL;
  free (msg); msg = NULL;
}

static void testSub() {

  int option = getNodeOptions("Create Subscription");
  if (option == -1)
    return ;

  if (subscription_done[option-1]) {
    printf("subscription already created for this node \n");
    return ;
  }

  EdgeEndPointInfo* ep = (EdgeEndPointInfo*) malloc(sizeof(EdgeEndPointInfo));
  ep->endpointUri = endpointUri;
  ep->config = NULL;

  EdgeSubRequest* subReq = (EdgeSubRequest*) malloc(sizeof(EdgeSubRequest));
  subReq->subType = Edge_Create_Sub;
  subReq->samplingInterval = 1000.0;
  subReq->publishingInterval = 0.0;
  subReq->maxKeepAliveCount = (1 > (int) (ceil(10000.0 / subReq->publishingInterval))) ? 1 : (int) ceil(10000.0 / subReq->publishingInterval);
  subReq->lifetimeCount = 10000;  //subReq->maxKeepAliveCount * 6;
  subReq->maxNotificationsPerPublish = 1;
  subReq->publishingEnabled = true;
  subReq->priority = 0;
  subReq->queueSize = 50;

  EdgeNodeInfo* nodeInfo = (EdgeNodeInfo*) malloc(sizeof(EdgeNodeInfo));
  nodeInfo->valueAlias = (char*) malloc(strlen(node_arr[option-1]));
  strcpy(nodeInfo->valueAlias, node_arr[option-1]);

  EdgeRequest *request = (EdgeRequest*) malloc(sizeof(EdgeRequest));
  request->nodeInfo = nodeInfo;
  request->subMsg = subReq;

  EdgeMessage* msg = (EdgeMessage*) malloc(sizeof(EdgeMessage));
  msg->endpointInfo = ep;
  msg->command = CMD_SUB;
  msg->request = request;

  EdgeResult result = handleSubscription(msg);
  if (result.code == STATUS_OK) {
    subscription_done[option-1] = true;
  }

  free(nodeInfo); nodeInfo = NULL;
  free(subReq); subReq = NULL;
  free(request); request = NULL;
  free(ep); ep = NULL;
  free (msg); msg = NULL;
}

static void testSubModify() {
  int option = getNodeOptions("Modify Subscription");
  if (option == -1)
    return ;

  if (!subscription_done[option-1]) {
    printf("subscription not created yet. First create the subscription and then modify \n");
    return ;
  }

  EdgeEndPointInfo* ep = (EdgeEndPointInfo*) malloc(sizeof(EdgeEndPointInfo));
  ep->endpointUri = endpointUri;
  ep->config = NULL;

  EdgeSubRequest* subReq = (EdgeSubRequest*) malloc(sizeof(EdgeSubRequest));
  subReq->subType = Edge_Modify_Sub;
  subReq->samplingInterval = 3000.0;
  subReq->publishingInterval = 0.0;
  subReq->maxKeepAliveCount = (1 > (int) (ceil(10000.0 / subReq->publishingInterval))) ? 1 : (int) ceil(10000.0 / subReq->publishingInterval);
  printf("keepalive count :: %d\n", subReq->maxKeepAliveCount);
  subReq->lifetimeCount = 10000;  //subReq->maxKeepAliveCount * 6;
  printf("lifetimecount :: %d\n", subReq->lifetimeCount);
  subReq->maxNotificationsPerPublish = 1;
  subReq->publishingEnabled = true;
  subReq->priority = 0;
  subReq->queueSize = 50;

  EdgeNodeInfo* nodeInfo = (EdgeNodeInfo*) malloc(sizeof(EdgeNodeInfo));
  nodeInfo->valueAlias = (char*) malloc(strlen(node_arr[option-1]));
  strcpy(nodeInfo->valueAlias, node_arr[option-1]);

  EdgeRequest *request = (EdgeRequest*) malloc(sizeof(EdgeRequest));
  request->nodeInfo = nodeInfo;
  request->subMsg = subReq;

  EdgeMessage* msg = (EdgeMessage*) malloc(sizeof(EdgeMessage));
  msg->endpointInfo = ep;
  msg->command = CMD_SUB;
  msg->request = request;

  handleSubscription(msg);

  free(nodeInfo); nodeInfo = NULL;
  free(subReq); subReq = NULL;
  free(request); request = NULL;
  free(ep); ep = NULL;
  free (msg); msg = NULL;
}

static void testSubDelete() {
  int option = getNodeOptions("Delete Subscription");
  if (option == -1)
    return ;

  if (!subscription_done[option-1]) {
    printf("subscription not created yet for that node to delete \n");
    return ;
  }

  EdgeEndPointInfo* ep = (EdgeEndPointInfo*) malloc(sizeof(EdgeEndPointInfo));
  ep->endpointUri = endpointUri;
  ep->config = NULL;

  EdgeSubRequest* subReq = (EdgeSubRequest*) malloc(sizeof(EdgeSubRequest));
  subReq->subType = Edge_Delete_Sub;

  EdgeNodeInfo* nodeInfo = (EdgeNodeInfo*) malloc(sizeof(EdgeNodeInfo));
  nodeInfo->valueAlias = (char*) malloc(strlen(node_arr[option-1]));
  strcpy(nodeInfo->valueAlias, node_arr[option-1]);

  EdgeRequest *request = (EdgeRequest*) malloc(sizeof(EdgeRequest));
  request->nodeInfo = nodeInfo;
  request->subMsg = subReq;

  EdgeMessage* msg = (EdgeMessage*) malloc(sizeof(EdgeMessage));
  msg->endpointInfo = ep;
  msg->command = CMD_SUB;
  msg->request = request;

  EdgeResult result = handleSubscription(msg);
  if (result.code == STATUS_OK) {
    subscription_done[option-1] = false;
  }

  free(nodeInfo); nodeInfo = NULL;
  free(subReq); subReq = NULL;
  free(request); request = NULL;
  free(ep); ep = NULL;
  free (msg); msg = NULL;
}

static void print_menu() {
  printf("=============== OPC UA =======================\n\n");

  printf("start : Get endpoints and start opcua client \n");
  printf("read : read attribute for target node\n");
  printf("write : write attribute into nodes\n");
  printf("browse : browse nodes\n");
  printf("method : method call\n");
  printf("create_sub : create subscription\n");
  printf("modify_sub : modify subscription\n");
  printf("delete_sub : delete subscription\n");
  printf("quit : terminate/stop opcua server/client and then quit\n");
  printf("help : print menu\n");

  printf("\n=============== OPC UA =======================\n\n");
}

int main() {
  char command[MAX_CHAR_SIZE];

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

      printf("[Please input server endpoint uri] : ");
      scanf("%s", ipAddress);
      int len = strlen(ipAddress);
      strcpy(endpointUri, ipAddress);
      endpointUri[len] = '\0';

      testGetEndpoints();
      //startFlag = true;

    } else if(!strcmp(command, "read")) {
      testRead();
    } else if(!strcmp(command, "write")) {
      testWrite();
    } else if(!strcmp(command, "browse")) {
      testBrowse();
    } else if(!strcmp(command, "method")) {
      testMethod();
    } else if(!strcmp(command, "create_sub")) {
      testSub();
    } else if(!strcmp(command, "modify_sub")) {
      testSubModify();
    } else if(!strcmp(command, "delete_sub")) {
      testSubDelete();
    } else if(!strcmp(command, "quit")) {
      deinit();
      //stopFlag = true;
    } else if(!strcmp(command, "help")) {
      print_menu();
    }
  }

  return 0;
}
