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

#define TEST_WITH_REFERENCE_SERVER 0

static void startClient(char* addr, int port, char *securityPolicyUri);

static void response_msg_cb (EdgeMessage* data) {
  if (data->type == GENERAL_RESPONSE) {
    printf("\n[Application response Callback] General response received\n");
    int len = data->responseLength;
    int idx = 0;
    for (idx = 0; idx < len; idx++) {
      if (data->responses[idx]->message != NULL) {
        if (data->command == CMD_READ || data->command == CMD_METHOD) {
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
            printf("\n\n");
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
        } else if (data->command == CMD_WRITE) {
            printf("[Application response Callback] Write response :: %s\n", (char*) data->responses[idx]->message->value);
        }

        // Diagnostics information
        if (data->responses[idx]->m_diagnosticInfo) {
          printf("[Application response Callback] Diagnostics information\n");
          printf("symbolicId :: %d, localizedText : %d, additionalInfo : %s , msg :: %s\n" , data->responses[idx]->m_diagnosticInfo->symbolicId,
               data->responses[idx]->m_diagnosticInfo->localizedText, data->responses[idx]->m_diagnosticInfo->additionalInfo, data->responses[idx]->m_diagnosticInfo->msg);
        }
      }

      printf("\n=========\n");
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
        else if(data->responses[idx]->type == Byte)
          printf("[MonitoredItem DataChange callback] Monitored Change Value read from node ===>> [%d]\n", *((uint8_t*)data->responses[idx]->message->value));
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
  printf("\n[Application error response callback]\n");
  printf("================================================\n");
  printf("EdgeStatusCode: %d\n", data->result->code);
  int responseLength = data->responseLength;
  for(int i = 0; i < responseLength; ++i)
  {
    EdgeResponse *response = data->responses[i];
    if(!response)
    {
        printf("EdgeResponse[%d] is null\n", i);
        continue;
    }
    if(response->message)
    {
        printf("Response[%d]->message: %s\n", i, (char *)response->message->value);
    }
    else
    {
        printf("Response[%d]->message is NULL\n", i);
    }
    if(response->nodeInfo)
    {
        EdgeNodeId *nodeId = response->nodeInfo->nodeId;
        if(nodeId)
        {
            if(nodeId->type == INTEGER)
                printf("Response[%d]->NodeId(Integer): %d\n", i, nodeId->integerNodeId);
            else
                printf("Response[%d]->NodeId(String): %s\n", i, nodeId->nodeId);
        }
        else
        {
            printf("Response[%d]->nodeInfo->nodeId is NULL\n", i);
        }
    }
    else
    {
        printf("Response[%d]->nodeInfo is NULL\n", i);
    }
  }
  printf("================================================\n");
}

#if 0
static void browse_msg_cb (EdgeMessage* data) {
  if (data->type == BROWSE_RESPONSE) {
      EdgeBrowseResult *browseResult = data->browseResult;
      int idx = 0;
      printf("\n[Application browse response callback] List of Browse Names for request(%d)\n",
        data->responses[0]->requestId);
      printf("================================================\n");
      for (idx = 0; idx < data->browseResultLength; idx++) {
        printf("[%d] %s\n", idx+1, browseResult[idx].browseName);
      }
      printf("================================================\n");
    }
}
#endif
static void browse_msg_cb (EdgeMessage* data) {
  if (data->type == BROWSE_RESPONSE) {
      EdgeBrowseResult *browseResult = data->browseResult;
      int idx = 0;
      printf("\n[Application browse response callback] [Request(%d)]\n",
        data->responses[0]->requestId);
      printf("BrowseName(s): ");
      for (idx = 0; idx < data->browseResultLength; idx++) {
        if(idx != 0) printf(", ");
        printf("%s", browseResult[idx].browseName);
      }
      printf("\n");
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

  EdgeEndPointInfo* ep = (EdgeEndPointInfo*) calloc(1, sizeof(EdgeEndPointInfo));
  ep->endpointUri = endpointUri;
  ep->config = NULL;

  EdgeMessage *msg = (EdgeMessage*) calloc(1, sizeof(EdgeMessage));
  msg->type = SEND_REQUEST;
  msg->endpointInfo = ep;
  msg->command = CMD_BROWSE;

#if TEST_WITH_REFERENCE_SERVER
    EdgeNodeInfo *nodeInfo = (EdgeNodeInfo*) calloc(1, sizeof(EdgeNodeInfo));
    nodeInfo->nodeId = (EdgeNodeId*) calloc(1, sizeof(EdgeNodeId));
    nodeInfo->nodeId->type = STRING;
    nodeInfo->nodeId->nodeId = "DataAccess_DataItem_Int16";
    nodeInfo->nodeId->nameSpace = 2;

    printf("\n\n" COLOR_YELLOW "********** Browse Int16 node in namespace 2 **********" COLOR_RESET "\n");

#else
    EdgeNodeInfo *nodeInfo = (EdgeNodeInfo*) calloc(1, sizeof(EdgeNodeInfo));
    nodeInfo->nodeId = (EdgeNodeId*) calloc(1, sizeof(EdgeNodeId));
    nodeInfo->nodeId->type = INTEGER;
    nodeInfo->nodeId->integerNodeId = RootFolder;
    nodeInfo->nodeId->nameSpace = SYSTEM_NAMESPACE_INDEX;

    printf("\n\n" COLOR_YELLOW "********** Browse RootFolder node in system namespace **********" COLOR_RESET "\n");
#endif
  EdgeRequest *request = (EdgeRequest*) calloc(1, sizeof(EdgeRequest));
  request->nodeInfo = nodeInfo;

  msg->request = request;
  msg->requestLength = 0;
  msg->browseParam = (EdgeBrowseParameter *)calloc(1, sizeof(EdgeBrowseParameter));
  msg->browseParam->direction = DIRECTION_FORWARD;
  msg->browseParam->maxReferencesPerNode = 0;

  browseNode(msg);

  free(msg->browseParam);
  free(request);
  free(nodeInfo->nodeId);
  free(nodeInfo);
  free(msg);
  free(ep);
}

static void testBrowses() {
  printf("\n" COLOR_YELLOW "------------------------------------------------------" COLOR_RESET);
  printf("\n" COLOR_YELLOW "                       Browse Multiple Nodes            "COLOR_RESET);
  printf("\n" COLOR_YELLOW "------------------------------------------------------" COLOR_RESET "\n\n");
  EdgeEndPointInfo* ep = (EdgeEndPointInfo*) calloc(1, sizeof(EdgeEndPointInfo));
  ep->endpointUri = endpointUri;
  ep->config = NULL;

  EdgeMessage *msg = (EdgeMessage*) calloc(1, sizeof(EdgeMessage));
  msg->type = SEND_REQUESTS;
  msg->endpointInfo = ep;
  msg->command = CMD_BROWSE;

#if TEST_WITH_REFERENCE_SERVER
  int requestLength = 2;
  EdgeRequest **requests = (EdgeRequest**) calloc(requestLength, sizeof(EdgeRequest *));

  EdgeNodeInfo *nodeInfo1 = (EdgeNodeInfo*) calloc(1, sizeof(EdgeNodeInfo));
  nodeInfo1->nodeId = (EdgeNodeId*) calloc(1, sizeof(EdgeNodeId));
  nodeInfo1->nodeId->type = STRING;
  nodeInfo1->nodeId->nodeId = "DataAccess_DataItem_Int16";
  nodeInfo1->nodeId->nameSpace = 2;

  EdgeNodeInfo *nodeInfo2 = (EdgeNodeInfo*) calloc(1, sizeof(EdgeNodeInfo));
  nodeInfo2->nodeId = (EdgeNodeId*) calloc(1, sizeof(EdgeNodeId));
  nodeInfo2->nodeId->type = STRING;
  nodeInfo2->nodeId->nodeId = "DataAccess_AnalogType_SByte";
  nodeInfo2->nodeId->nameSpace = 2;

  requests[0] = (EdgeRequest*) calloc(1, sizeof(EdgeRequest));
  requests[0]->nodeInfo = nodeInfo1;
  requests[1] = (EdgeRequest*) calloc(1, sizeof(EdgeRequest));
  requests[1]->nodeInfo = nodeInfo2;

  printf("\n\n" COLOR_YELLOW "********** Browse Int16 & SByte nodes in namespace 2 **********" COLOR_RESET "\n");
#else
  int requestLength = 3;
  EdgeRequest **requests = (EdgeRequest**) calloc(requestLength, sizeof(EdgeRequest *));

  EdgeNodeInfo *nodeInfo1 = (EdgeNodeInfo*) calloc(1, sizeof(EdgeNodeInfo));
  nodeInfo1->nodeId = (EdgeNodeId*) calloc(1, sizeof(EdgeNodeId));
  nodeInfo1->nodeId->type = INTEGER;
  nodeInfo1->nodeId->integerNodeId = RootFolder;
  nodeInfo1->nodeId->nameSpace = SYSTEM_NAMESPACE_INDEX;

  EdgeNodeInfo *nodeInfo2 = (EdgeNodeInfo*) calloc(1, sizeof(EdgeNodeInfo));
  nodeInfo2->nodeId = (EdgeNodeId*) calloc(1, sizeof(EdgeNodeId));
  nodeInfo2->nodeId->type = INTEGER;
  nodeInfo2->nodeId->integerNodeId = ObjectsFolder;
  nodeInfo2->nodeId->nameSpace = SYSTEM_NAMESPACE_INDEX;

  EdgeNodeInfo *nodeInfo3 = (EdgeNodeInfo*) calloc(1, sizeof(EdgeNodeInfo));
  nodeInfo3->nodeId = (EdgeNodeId*) calloc(1, sizeof(EdgeNodeId));
  nodeInfo3->nodeId->type = STRING;
  nodeInfo3->nodeId->nodeId = "Object1";
  nodeInfo3->nodeId->nameSpace = 1;

  requests[0] = (EdgeRequest*) calloc(1, sizeof(EdgeRequest));
  requests[0]->nodeInfo = nodeInfo1;
  requests[1] = (EdgeRequest*) calloc(1, sizeof(EdgeRequest));
  requests[1]->nodeInfo = nodeInfo2;
  requests[2] = (EdgeRequest*) calloc(1, sizeof(EdgeRequest));
  requests[2]->nodeInfo = nodeInfo3;

  printf("\n\n" COLOR_YELLOW "********** Browse RootFolder, ObjectsFolder nodes in system namespace and Object1 in namespace 1 **********" COLOR_RESET "\n");
#endif

  msg->requests = requests;
  msg->requestLength = requestLength;
  msg->browseParam = (EdgeBrowseParameter *)calloc(1, sizeof(EdgeBrowseParameter));
  msg->browseParam->direction = DIRECTION_FORWARD;
  msg->browseParam->maxReferencesPerNode = 0;

  browseNode(msg);

  free(msg->browseParam);
  free(nodeInfo1->nodeId); free(nodeInfo1);
  free(nodeInfo2->nodeId); free(nodeInfo2);
  free(requests[0]); free(requests[1]);
#if !TEST_WITH_REFERENCE_SERVER
  free(nodeInfo3->nodeId); free(nodeInfo3);
  free(requests[2]);
#endif
  free(requests);
  free(ep); ep = NULL;
  free(msg); msg = NULL;
}

static void testRead() {
  // Get the list of browse names and display them to user.
  testBrowse();

  char nodeName[MAX_CHAR_SIZE];
   int num_requests = 1;

  EdgeEndPointInfo* ep = (EdgeEndPointInfo*) malloc(sizeof(EdgeEndPointInfo));
  ep->endpointUri = endpointUri;
  ep->config = NULL;

  EdgeNodeInfo **nodeInfo= (EdgeNodeInfo**) malloc(sizeof(EdgeNodeInfo*) * num_requests);
  for (int i = 0; i < num_requests; i++) {
    printf("\nEnter the node #%d name to read :: ", (i+1));
    nodeInfo[i]= (EdgeNodeInfo*) malloc(sizeof(EdgeNodeInfo));
    scanf("%s", nodeName);
    nodeInfo[i]->valueAlias = (char*) malloc(strlen(nodeName) + 1);
    strcpy(nodeInfo[i]->valueAlias, nodeName);
    nodeInfo[i]->valueAlias[strlen(nodeName)] = '\0';
  }

  EdgeRequest **requests = (EdgeRequest**) malloc(sizeof(EdgeRequest*) * num_requests);
  for (int i = 0; i < num_requests; i++) {
    requests[i] = (EdgeRequest*) malloc(sizeof(EdgeRequest));
    requests[i]->nodeInfo = nodeInfo[i];
  }

  EdgeMessage *msg = (EdgeMessage*) malloc(sizeof(EdgeMessage));
  msg->endpointInfo = ep;
  msg->command = CMD_READ;
  msg->type = SEND_REQUESTS;
  msg->requests = requests;
  msg->requestLength = num_requests;

  readNode(msg);

  for (int i = 0; i < num_requests; i++) {
    free(nodeInfo[i]->valueAlias); nodeInfo[i]->valueAlias = NULL;
    free (nodeInfo[i]); nodeInfo[i] = NULL;
  }
  free (nodeInfo); nodeInfo = NULL;
  free(ep); ep = NULL;
  for (int i = 0; i < num_requests; i++) {
    free (requests[i]); requests[i] = NULL;
  }
  free(requests); requests = NULL;
  free(msg); msg = NULL;
}

static void testReadGroup() {
  char nodeName[MAX_CHAR_SIZE];
  int num_requests;
  printf("Enter number of nodes to read (less than 10) :: ");
  scanf("%d", &num_requests);

  EdgeEndPointInfo* ep = (EdgeEndPointInfo*) malloc(sizeof(EdgeEndPointInfo));
  ep->endpointUri = endpointUri;
  ep->config = NULL;

  EdgeNodeInfo **nodeInfo= (EdgeNodeInfo**) malloc(sizeof(EdgeNodeInfo*) * num_requests);
  EdgeRequest **requests = (EdgeRequest**) malloc(sizeof(EdgeRequest*) * num_requests);
  for (int i = 0; i < num_requests; i++) {
    printf("\nEnter the node #%d  name to read :: ", (i+1));
    nodeInfo[i]= (EdgeNodeInfo*) malloc(sizeof(EdgeNodeInfo));
    scanf("%s", nodeName);
    nodeInfo[i]->valueAlias = (char*) malloc(strlen(nodeName) + 1);
    strcpy(nodeInfo[i]->valueAlias, nodeName);
    nodeInfo[i]->valueAlias[strlen(nodeName)] = '\0';

    requests[i] = (EdgeRequest*) malloc(sizeof(EdgeRequest));
    requests[i]->nodeInfo = nodeInfo[i];
  }

  EdgeMessage *msg = (EdgeMessage*) malloc(sizeof(EdgeMessage));
  msg->endpointInfo = ep;
  msg->command = CMD_READ;
  msg->type = SEND_REQUESTS;
  msg->requests = requests;
  msg->requestLength = num_requests;

  readNode(msg);

  for (int i = 0; i < num_requests; i++) {
    free(nodeInfo[i]->valueAlias); nodeInfo[i]->valueAlias = NULL;
    free (nodeInfo[i]); nodeInfo[i] = NULL;
  }
  free (nodeInfo); nodeInfo = NULL;
  free(ep); ep = NULL;
  for (int i = 0; i < num_requests; i++) {
    free (requests[i]); requests[i] = NULL;
  }
  free(requests); requests = NULL;
  free(msg); msg = NULL;
}

static int getInputType() {
  int type;

  printf("\n1) Boolean\n");
  printf("2) SByte\n");
  printf("3) Byte\n");
  printf("4) Int16\n");
  printf("5) UInt16\n");
  printf("6) Int32\n");
  printf("7) UInt32\n");
  printf("8) Int64\n");
  printf("9) UInt64\n");
  printf("10) Float\n");
  printf("11) Double\n");
  printf("12) String\n\n");

  printf("Enter the type of the data to write (integer option only) :: ");
  scanf("%d", &type);

  return type;
}

static void* getNewValuetoWrite(EdgeNodeIdentifier type) {
  printf("Enter the new value to write :: ");
  switch(type) {
   case Boolean:
   {
   	int *val = (int*) malloc(sizeof(int));
	scanf("%d", val);
	return (void*) val;
   }
   	break;
   case SByte:
   {
        int8_t* val = (int8_t*) malloc(sizeof(int8_t));
        scanf("%" SCNd8, val);
        return (void*) val;
   }
   case Byte:
   {
        uint8_t* val = (uint8_t*) malloc(sizeof(uint8_t));
        scanf("%" SCNu8, val);
        return (void*) val;
   }
        break;
   case Int16:
   {
        int16_t* val = (int16_t*) malloc(sizeof(int16_t));
        scanf("%" SCNd16, val);
        return (void*) val;
   }
   case UInt16:
   {
   	uint16_t *val = (uint16_t*) malloc(sizeof(uint16_t));
	scanf("%" SCNu16, val);
	return (void*) val;
   }
   	break;
   case Int32:
   {
   	int32_t *val = (int32_t*) malloc(sizeof(int32_t));
	scanf("%" SCNd32, val);
	return (void*) val;
   }
   	break;
   case UInt32:
   {
   	uint32_t *val = (uint32_t*) malloc(sizeof(uint32_t));
	scanf("%" SCNu32, val);
	return (void*) val;
   }
   	break;
   case Int64:
   {
   	int64_t *val = (int64_t*) malloc(sizeof(int64_t));
	scanf("%" SCNd64, val);
	return (void*) val;
   }
   	break;
   case UInt64:
   {
   	uint64_t *val = (uint64_t*) malloc(sizeof(uint64_t));
	scanf("%" SCNu64, val);
	return (void*) val;
   }
   	break;
   case Float:
   {
   	float* val = (float*) malloc(sizeof(float));
	scanf("%g", val);
	return (void*) val;
   }
   	break;
   case Double:
   {
   	double *val = (double*) malloc(sizeof(double));
	scanf("%lf", val);
	return (void*) val;
   }
   	break;
   case String:
   {
   	char val[MAX_CHAR_SIZE];
	scanf("%s", val);
	char *retStr = (char*) malloc(strlen(val) + 1);
	strcpy(retStr, val);
	retStr[strlen(val)] = '\0';
	return (void*) retStr;
   }
   	break;
   default:
        break;
  }

  return NULL;
}

static void testWrite() {
  char nodeName[MAX_CHAR_SIZE];
  int num_requests = 1;

  EdgeEndPointInfo* ep = (EdgeEndPointInfo*) malloc(sizeof(EdgeEndPointInfo));
  ep->endpointUri = endpointUri;
  ep->config = NULL;

  EdgeNodeInfo **nodeInfo= (EdgeNodeInfo**) malloc(sizeof(EdgeNodeInfo*));
  EdgeRequest **requests = (EdgeRequest**) malloc(sizeof(EdgeRequest*) * num_requests);
  for (int i = 0; i < num_requests; i++) {
    printf("\nEnter the node #%d name to write :: ", (i+1));
    nodeInfo[i]= (EdgeNodeInfo*) malloc(sizeof(EdgeNodeInfo));
    scanf("%s", nodeName);
    nodeInfo[i]->valueAlias = (char*) malloc(strlen(nodeName) + 1);
    strcpy(nodeInfo[i]->valueAlias, nodeName);
    nodeInfo[i]->valueAlias[strlen(nodeName)] = '\0';

    requests[i] = (EdgeRequest*) malloc(sizeof(EdgeRequest));
    requests[i]->nodeInfo = nodeInfo[i];
    requests[i]->type = getInputType();
    requests[i]->value = getNewValuetoWrite(requests[i]->type);
  }

  EdgeMessage *msg = (EdgeMessage*) malloc(sizeof(EdgeMessage));
  msg->endpointInfo = ep;
  msg->command = CMD_WRITE;
  msg->type = SEND_REQUESTS;
  msg->requests = requests;
  msg->requestLength = num_requests;

printf("write node \n");
  writeNode(msg);
  printf("write node call success \n");

  for (int i = 0; i < num_requests; i++) {
    free(nodeInfo[i]->valueAlias); nodeInfo[i]->valueAlias = NULL;
    free (nodeInfo[i]); nodeInfo[i] = NULL;
  }
  free (nodeInfo); nodeInfo = NULL;
  free(ep); ep = NULL;
  for (int i = 0; i < num_requests; i++) {
    free (requests[i]->value); requests[i]->value = NULL;
    free (requests[i]); requests[i] = NULL;
  }
  free(requests); requests = NULL;
  free(msg); msg = NULL;
}

static void testWriteGroup() {
  char nodeName[MAX_CHAR_SIZE];
  int num_requests;
  printf("Enter number of nodes to write (less than 10) :: ");
  scanf("%d", &num_requests);

  EdgeEndPointInfo* ep = (EdgeEndPointInfo*) malloc(sizeof(EdgeEndPointInfo));
  ep->endpointUri = endpointUri;
  ep->config = NULL;

  EdgeNodeInfo **nodeInfo= (EdgeNodeInfo**) malloc(sizeof(EdgeNodeInfo*) * num_requests);
  EdgeRequest **requests = (EdgeRequest**) malloc(sizeof(EdgeRequest*) * num_requests);
  for (int i = 0; i < num_requests; i++) {
    printf("\nEnter the node #%d name to write :: ", (i+1));
    nodeInfo[i]= (EdgeNodeInfo*) malloc(sizeof(EdgeNodeInfo));
    scanf("%s", nodeName);
    nodeInfo[i]->valueAlias = (char*) malloc(strlen(nodeName) + 1);
    strcpy(nodeInfo[i]->valueAlias, nodeName);
    nodeInfo[i]->valueAlias[strlen(nodeName)] = '\0';

    requests[i] = (EdgeRequest*) malloc(sizeof(EdgeRequest));
    requests[i]->nodeInfo = nodeInfo[i];
    requests[i]->type = getInputType();
    requests[i]->value = getNewValuetoWrite(requests[i]->type);
  }

  EdgeMessage *msg = (EdgeMessage*) malloc(sizeof(EdgeMessage));
  msg->endpointInfo = ep;
  msg->command = CMD_WRITE;
  msg->type = SEND_REQUESTS;
  msg->requests = requests;
  msg->requestLength = num_requests;

  writeNode(msg);

  for (int i = 0; i < num_requests; i++) {
    free(nodeInfo[i]->valueAlias); nodeInfo[i]->valueAlias = NULL;
    free (nodeInfo[i]); nodeInfo[i] = NULL;
  }
  free (nodeInfo); nodeInfo = NULL;
  free(ep); ep = NULL;
  for (int i = 0; i < num_requests; i++) {
    free (requests[i]->value); requests[i]->value = NULL;
    free (requests[i]); requests[i] = NULL;
  }
  free(requests); requests = NULL;
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

    // Get the list of browse names and display them to user.
    testBrowse();
    char nodeName[MAX_CHAR_SIZE];

    printf("\nEnter the node name to create Subscribe :: ");
    scanf("%s", nodeName);

  EdgeEndPointInfo* ep = (EdgeEndPointInfo*) malloc(sizeof(EdgeEndPointInfo));
  ep->endpointUri = endpointUri;
  ep->config = NULL;

  EdgeSubRequest* subReq = (EdgeSubRequest*) malloc(sizeof(EdgeSubRequest));
  subReq->subType = Edge_Create_Sub;
  subReq->samplingInterval = 1000.0;
  subReq->publishingInterval = 0.0;
  subReq->maxKeepAliveCount = (1 > (int) (
    ceil(10000.0 / subReq->publishingInterval))) ? 1 : (int) ceil(10000.0 / subReq->publishingInterval);
  subReq->lifetimeCount = 10000;  //subReq->maxKeepAliveCount * 6;
  subReq->maxNotificationsPerPublish = 1;
  subReq->publishingEnabled = true;
  subReq->priority = 0;
  subReq->queueSize = 50;

  EdgeNodeInfo* nodeInfo = (EdgeNodeInfo*) malloc(sizeof(EdgeNodeInfo));
  nodeInfo->valueAlias = (char*) malloc(strlen(nodeName) + 1);
  strcpy(nodeInfo->valueAlias, nodeName);

  EdgeRequest *request = (EdgeRequest*) malloc(sizeof(EdgeRequest));
  request->nodeInfo = nodeInfo;
  request->subMsg = subReq;

  EdgeMessage* msg = (EdgeMessage*) malloc(sizeof(EdgeMessage));
  msg->endpointInfo = ep;
  msg->command = CMD_SUB;
  msg->request = request;

  EdgeResult result = handleSubscription(msg);
  printf("CREATE RESULT : %d\n",  result.code);
  if (result.code == STATUS_OK) {
    printf("SUBSCRPTION CREATE SUCCESSFULL\n");
  }

  free(nodeInfo->valueAlias); nodeInfo->valueAlias = NULL;
  free(nodeInfo); nodeInfo = NULL;
  free(subReq); subReq = NULL;
  free(request); request = NULL;
  free(ep); ep = NULL;
  free (msg); msg = NULL;
}

static void testSubModify() {

    testBrowse();
    char nodeName[MAX_CHAR_SIZE];

    printf("\nEnter the node name to modify Subscribe :: ");
    scanf("%s", nodeName);

  EdgeEndPointInfo* ep = (EdgeEndPointInfo*) malloc(sizeof(EdgeEndPointInfo));
  ep->endpointUri = endpointUri;
  ep->config = NULL;

  EdgeSubRequest* subReq = (EdgeSubRequest*) malloc(sizeof(EdgeSubRequest));
  subReq->subType = Edge_Modify_Sub;
  subReq->samplingInterval = 3000.0;
  subReq->publishingInterval = 0.0;
  subReq->maxKeepAliveCount = (1 > (int) (ceil(10000.0 / subReq->publishingInterval))) ? 1 : (int) ceil(
    10000.0 / subReq->publishingInterval);
  printf("keepalive count :: %d\n", subReq->maxKeepAliveCount);
  subReq->lifetimeCount = 10000;  //subReq->maxKeepAliveCount * 6;
  printf("lifetimecount :: %d\n", subReq->lifetimeCount);
  subReq->maxNotificationsPerPublish = 1;
  subReq->publishingEnabled = true;
  subReq->priority = 0;
  subReq->queueSize = 50;

  EdgeNodeInfo* nodeInfo = (EdgeNodeInfo*) malloc(sizeof(EdgeNodeInfo));
  nodeInfo->valueAlias = (char*) malloc(strlen(nodeName) + 1);
  strcpy(nodeInfo->valueAlias, nodeName);

  EdgeRequest *request = (EdgeRequest*) malloc(sizeof(EdgeRequest));
  request->nodeInfo = nodeInfo;
  request->subMsg = subReq;

  EdgeMessage* msg = (EdgeMessage*) malloc(sizeof(EdgeMessage));
  msg->endpointInfo = ep;
  msg->command = CMD_SUB;
  msg->request = request;

  EdgeResult result = handleSubscription(msg);
  if (result.code == STATUS_OK) {
    printf("SUBSCRPTION MODIFY SUCCESSFULL\n");
  }

  free(nodeInfo->valueAlias); nodeInfo->valueAlias = NULL;
  free(nodeInfo); nodeInfo = NULL;
  free(subReq); subReq = NULL;
  free(request); request = NULL;
  free(ep); ep = NULL;
  free (msg); msg = NULL;
}

static void testRePublish() {
    
    char nodeName[MAX_CHAR_SIZE];
    printf("\nEnter the node name to Re publish :: ");
    scanf("%s", nodeName);

  EdgeEndPointInfo* ep = (EdgeEndPointInfo*) malloc(sizeof(EdgeEndPointInfo));
  ep->endpointUri = endpointUri;
  ep->config = NULL;

  EdgeSubRequest* subReq = (EdgeSubRequest*) malloc(sizeof(EdgeSubRequest));
  subReq->subType = Edge_Republish_Sub;  

  EdgeNodeInfo* nodeInfo = (EdgeNodeInfo*) malloc(sizeof(EdgeNodeInfo));
  nodeInfo->valueAlias = (char*) malloc(strlen(nodeName) + 1);
  strcpy(nodeInfo->valueAlias, nodeName);

  EdgeRequest *request = (EdgeRequest*) malloc(sizeof(EdgeRequest));
  request->nodeInfo = nodeInfo;
  request->subMsg = subReq;

  EdgeMessage* msg = (EdgeMessage*) malloc(sizeof(EdgeMessage));
  msg->endpointInfo = ep;
  msg->command = CMD_SUB;
  msg->request = request;

  EdgeResult result = handleSubscription(msg);
  printf("REPUBLISH RESULT : %d\n",  result.code);
  if (result.code == STATUS_OK) {
    printf("REPUBLISH SUCCESSFULL\n");
  }

  free(nodeInfo->valueAlias); nodeInfo->valueAlias = NULL;
  free(nodeInfo); nodeInfo = NULL;
  free(subReq); subReq = NULL;
  free(request); request = NULL;
  free(ep); ep = NULL;
  free (msg); msg = NULL;
}

static void testSubDelete() {

    testBrowse();
    char nodeName[MAX_CHAR_SIZE];

    printf("\nEnter the node name to delete Subscribe :: ");
    scanf("%s", nodeName);

  EdgeEndPointInfo* ep = (EdgeEndPointInfo*) malloc(sizeof(EdgeEndPointInfo));
  ep->endpointUri = endpointUri;
  ep->config = NULL;

  EdgeSubRequest* subReq = (EdgeSubRequest*) malloc(sizeof(EdgeSubRequest));
  subReq->subType = Edge_Delete_Sub;

  EdgeNodeInfo* nodeInfo = (EdgeNodeInfo*) malloc(sizeof(EdgeNodeInfo));
  nodeInfo->valueAlias = (char*) malloc(strlen(nodeName) + 1);
  strcpy(nodeInfo->valueAlias, nodeName);

  EdgeRequest *request = (EdgeRequest*) malloc(sizeof(EdgeRequest));
  request->nodeInfo = nodeInfo;
  request->subMsg = subReq;

  EdgeMessage* msg = (EdgeMessage*) malloc(sizeof(EdgeMessage));
  msg->endpointInfo = ep;
  msg->command = CMD_SUB;
  msg->request = request;

  EdgeResult result = handleSubscription(msg);
  printf("DELETE RESULT : %d\n",  result.code);
  if (result.code == STATUS_OK) {
    printf("SUBSCRPTION DELETED SUCCESSFULL\n");
  }

  free(nodeInfo->valueAlias); nodeInfo->valueAlias = NULL;
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
  printf("read_group : group read attributes from nodes\n");
  printf("write : write attribute into nodes\n");
  printf("write_group : group write attributes from nodes\n");
  printf("browse : browse nodes\n");
  printf("browse_m : browse multiple nodes\n");
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
      strcpy(endpointUri, ipAddress);

      testGetEndpoints();
      //startFlag = true;

    } else if(!strcmp(command, "read")) {
      testRead();
    } else if(!strcmp(command, "read_group")) {
      testReadGroup();
    } else if(!strcmp(command, "write")) {
      testWrite();
    } else if(!strcmp(command, "write_group")) {
      testWriteGroup();
    } else if(!strcmp(command, "browse")) {
      testBrowse();
    } else if(!strcmp(command, "browse_m")) {
      testBrowses();
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
    } else if(!strcmp(command, "republish")) {
      testRePublish();
    }
  }

  return 0;
}
