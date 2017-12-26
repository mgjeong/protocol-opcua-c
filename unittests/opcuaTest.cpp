#include <gtest/gtest.h>
#include <iostream>

extern "C" {
    #include "opcua_manager.h"
    #include "opcua_common.h"
    #include "edge_identifier.h"
    #include "edge_utils.h"
}

#define LOCALHOST "localhost"
#define ENDPOINT_URI  "opc:tcp://%s:12686/edge-opc-server"
#define IPADDRESS "opc.tcp://localhost:12686"

#define TEST_STR1_R "test1"
#define TEST_STR2_R "test2"
#define TEST_STR3_R "test3"
#define TEST_DOUBLE_R 50.4
#define TEST_INT32_R 40
#define TEST_UINT16_R 30

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
static EdgeEndPointInfo* epInfo = NULL;
static EdgeConfigure *config = NULL;

static bool startServerFlag = false;
static bool startClientFlag = false;
static bool readNodeFlag = true;

static char node_arr[6][10] = {
  "String1",
  "String2",
  "String3",
  "Double",
  "Int32",
  "UInt16"
};

extern "C" 
{
    static void response_msg_cb (EdgeMessage* data) {
        if (data->type == GENERAL_RESPONSE) {
            PRINT("[Application response Callback] General response received\n");
            int len = data->responseLength;
            int idx = 0;
            for (idx = 0; idx < len; idx++) {
            if (data->responses[idx]->message != NULL) {
                if (data->responses[idx]->message->isArray) {
                  // Handle Output array
                  int arrayLen = data->responses[idx]->message->arrayLength;

                  if(data->responses[idx]->type == Int16) {
                    /* Handle int16 output array */
                  } else if(data->responses[idx]->type == UInt16)  {
                    /* Handle UInt16 output array */
                  } else if(data->responses[idx]->type == Int32) {
                    /* Handle Int32 output array */
                    PRINT_ARG("Int32 output array length :: ", arrayLen);
                    for (int arrayIdx = 0; arrayIdx < arrayLen; arrayIdx++) {
                      PRINT(((int32_t*) data->responses[idx]->message->value)[arrayIdx]);
                    }
                  } else if(data->responses[idx]->type == UInt32) {
                    /* Handle UInt32 output array */
                  } else if(data->responses[idx]->type == Int64) {
                    /* Handle Int64 output array */
                  } else if(data->responses[idx]->type == UInt64) {
                    /* Handle UInt64 output array */
                  } else if(data->responses[idx]->type == Float) {
                    /* Handle Float output array */
                  } else if(data->responses[idx]->type == Double) {
                    /* Handle Double output array */
                  } else if(data->responses[idx]->type == String) {
                    /* Handle String output array */
                  }
                } else {
                  if(data->responses[idx]->type == UInt16) {
                    PRINT_ARG("[Application response Callback] Data read from node ===>> ",
                            *((int*)data->responses[idx]->message->value));
                    int temp = *((int*)data->responses[idx]->message->value);
                    if(readNodeFlag)
                        EXPECT_EQ(temp, TEST_UINT16_R);
                    else
                        EXPECT_EQ(temp, TEST_UINT16_W);
                  } else if(data->responses[idx]->type == Int32) {
                    PRINT_ARG("[Application response Callback] Data read from node ===>>  ",
                            *((int*)data->responses[idx]->message->value));
                    
                    int temp = *((int*)data->responses[idx]->message->value);
                    if(readNodeFlag)
                        EXPECT_EQ(temp, TEST_INT32_R);
                    else
                        EXPECT_EQ(temp, TEST_INT32_W);
                  } else if(data->responses[idx]->type == Double) {
                    PRINT_ARG("[Application response Callback] Data read from node ===>>  ",
                            *((double*)data->responses[idx]->message->value));

                    double temp = *((double*)data->responses[idx]->message->value);
                    if(readNodeFlag)
                        EXPECT_EQ(temp, TEST_DOUBLE_R);
                    else
                        EXPECT_EQ(temp, TEST_DOUBLE_W);
                  } else if(data->responses[idx]->type == String) {
                    PRINT_ARG("[Application response Callback] Data read from node ===>>  ",
                            ((char*)data->responses[idx]->message->value));
                    char* temp = ((char*)data->responses[idx]->message->value);
                    if(readNodeFlag)
                        EXPECT_EQ((strcmp(temp, TEST_STR1_R) && strcmp(temp, TEST_STR2_R) && 
                            strcmp(temp, TEST_STR3_R)), 0);
                    else
                        EXPECT_EQ((strcmp(temp, TEST_STR1_W) && strcmp(temp, TEST_STR2_W) &&
                                strcmp(temp, TEST_STR3_W)), 0);
                  }
                }
            }
        }
    }
   }

    static void monitored_msg_cb (EdgeMessage* data) {
      if (data->type == REPORT) {
        PRINT("[Application response Callback] Monitored Item Response received");
        int len = data->responseLength;
        int idx = 0;
        for (idx = 0; idx < len; idx++) {
          if (data->responses[idx]->message != NULL) {
            if(data->responses[idx]->type == Int16)
              PRINT_ARG("[MonitoredItem DataChange callback] Monitored Change Value read from node ===>> ",
                        *((int*)data->responses[idx]->message->value));
            else if(data->responses[idx]->type == UInt16)
              PRINT_ARG("[MonitoredItem DataChange callback] Monitored Change Value read from node ===>> ",
                        *((int*)data->responses[idx]->message->value));
            else if(data->responses[idx]->type == Int32)
              PRINT_ARG("[MonitoredItem DataChange callback] Monitored Change Value read from node ===>>  ",
                        *((int*)data->responses[idx]->message->value));
            else if(data->responses[idx]->type == UInt32)
              PRINT_ARG("[MonitoredItem DataChange callbackk] Monitored Change Value read from node ===>> ",
                        *((int*)data->responses[idx]->message->value));
            else if(data->responses[idx]->type == Int64)
              PRINT_ARG("[MonitoredItem DataChange callbackk] Monitored Change Value read from node ===>>  ",
                        *((long*)data->responses[idx]->message->value));
            else if(data->responses[idx]->type == UInt64)
              PRINT_ARG("[MonitoredItem DataChange callbackk] Monitored Change Value read from node ===>>  ",
                        *((long*)data->responses[idx]->message->value));
            else if(data->responses[idx]->type == Float)
              PRINT_ARG("[MonitoredItem DataChange callback] Monitored Change Value read from node  ===>>  ",
                        *((float*)data->responses[idx]->message->value));
            else if(data->responses[idx]->type == Double)
              PRINT_ARG("[MonitoredItem DataChange callback] Monitored Change Value read from node  ===>>  ",
                        *((double*)data->responses[idx]->message->value));
            else if(data->responses[idx]->type == String)
              PRINT_ARG("[MonitoredItem DataChange callback] Monitored Change Value read from node ===>>  ",
                        ((char*)data->responses[idx]->message->value));
          }
        }
      }
    }

    static void error_msg_cb (void* data) {

    }

    static void browse_msg_cb (EdgeMessage* data) {

    }

    /* status callbacks */
    static void status_start_cb (EdgeEndPointInfo* epInfo, EdgeStatusCode status) {
      if (status == STATUS_SERVER_STARTED) {
        PRINT("\n[Application Callback] Server started");
        startServerFlag = true;
      }
      if (status == STATUS_CLIENT_STARTED) {
        printf("[Application Callback] Client connected\n");
        startClientFlag = true;
      }
    }

    static void status_stop_cb (EdgeEndPointInfo* epInfo, EdgeStatusCode status) {
      if (status == STATUS_STOP_SERVER) {
            PRINT("\n[Application Callback] Server stopped ");
            startServerFlag = false;
        }
      if (status == STATUS_STOP_CLIENT) {
        printf("[Application Callback] Client disconnected \n\n");
        startClientFlag = false;
      }
    }

    static void status_network_cb (EdgeEndPointInfo* epInfo, EdgeStatusCode status) {

    }

    /* discovery callback */
    static void endpoint_found_cb (EdgeDevice* device) {

    }

    static void device_found_cb (EdgeDevice* device) {

    }
}

static void configureCallbacks()
{
    PRINT("-----INITIALIZING CALLBACKS-----");

    EXPECT_EQ(NULL == config, true);
    config = (EdgeConfigure*) malloc(sizeof(EdgeConfigure));
    EXPECT_EQ(NULL == config, false);

    config->recvCallback = (ReceivedMessageCallback*) malloc(sizeof(ReceivedMessageCallback));
    EXPECT_EQ(NULL == config->recvCallback, false);

    config->recvCallback->resp_msg_cb = response_msg_cb;
    config->recvCallback->monitored_msg_cb = monitored_msg_cb;
    config->recvCallback->error_msg_cb = error_msg_cb;
    config->recvCallback->browse_msg_cb = browse_msg_cb;

    config->statusCallback = (StatusCallback*) malloc(sizeof(StatusCallback));
    EXPECT_EQ(NULL == config->statusCallback, false);
    
    config->statusCallback->start_cb = status_start_cb;
    config->statusCallback->stop_cb = status_stop_cb;
    config->statusCallback->network_cb = status_network_cb;

    config->discoveryCallback = (DiscoveryCallback*) malloc(sizeof(DiscoveryCallback));
    EXPECT_EQ(NULL == config->discoveryCallback, false);
    
    config->discoveryCallback->endpoint_found_cb = endpoint_found_cb;
    config->discoveryCallback->device_found_cb = device_found_cb;

    registerCallbacks(config);
}

static void cleanCallbacks()
{
    PRINT("-----CLEANING CALLBACKS-----");

    if(config->recvCallback != NULL)
    {
        free(config->recvCallback);
        config->recvCallback = NULL;
    }

    if(config->statusCallback != NULL)
    {
        free(config->statusCallback);
        config->statusCallback = NULL;
    }

    if(config->discoveryCallback != NULL)
    {
        free(config->discoveryCallback);
        config->discoveryCallback = NULL;
    }

    if(config != NULL)
    {
        free(config);
        config = NULL;
    }
}

static void deleteMessage(EdgeMessage* msg, EdgeEndPointInfo* ep)
{
    if(msg != NULL)
    {
        free(msg);
        msg = NULL;
    }

    if(ep != NULL)
    {
        free(ep);
        ep = NULL;
    }
}

static void writeNodes(bool defaultFlag)
{
    PRINT("=============== Writting Nodes ==================");
    
    EdgeNodeIdentifier type;
    void *new_value = NULL;

    int id;
    double d_value = TEST_DOUBLE_R;
    int i_value = TEST_INT32_R;
    int id_value = TEST_UINT16_R;
    if(!defaultFlag)
    {
        d_value = TEST_DOUBLE_W;
        i_value = TEST_INT32_W;
        id_value = TEST_UINT16_W;
    }
    
    for(id = 0; id < 6; id++)
    {
        PRINT_ARG("*****  Writting the node with browse name  ", node_arr[id]);

        EdgeEndPointInfo* epWrite = (EdgeEndPointInfo*) malloc(sizeof(EdgeEndPointInfo));
        epWrite->endpointUri = endpointUri;
        epWrite->config = NULL;

        EdgeNodeInfo *nodeInfo = (EdgeNodeInfo*) malloc(sizeof(EdgeNodeInfo));
        nodeInfo->valueAlias = node_arr[id];

        EdgeRequest *request = (EdgeRequest*) malloc(sizeof(EdgeRequest));
        request->nodeInfo = nodeInfo;
        switch(id) { 
            case 0: 
                request->type = String;
                if(defaultFlag)
                    request->value = (void *) TEST_STR1_R;                    
                else
                    request->value = (void *) TEST_STR1_W;                
                break; 
            case 1: 
                request->type = String;
                if(defaultFlag)
                    request->value = (void *) TEST_STR2_R;                    
                else
                    request->value = (void *) TEST_STR2_W;
                break; 
            case 2: 
                request->type = String;
                if(defaultFlag)
                    request->value = (void *) TEST_STR3_R;                   
                else
                    request->value = (void *) TEST_STR3_W;
                break; 
            case 3: 
                request->type = Double;
                request->value = (void *) &d_value;
                break; 
            case 4: 
                request->type = Int32;
                request->value = (void *) &i_value;
                break; 
            case 5: 
                request->type = UInt16;                
                request->value = (void *) &id_value;
                break; 
        }

        EdgeMessage *msgWrite = (EdgeMessage*) malloc(sizeof(EdgeMessage));
        msgWrite->endpointInfo = epWrite;
        msgWrite->command = CMD_READ;
        msgWrite->request = request;

        writeNode(msgWrite);

        deleteMessage(msgWrite, epWrite);

        if(nodeInfo != NULL)
        {
            free(nodeInfo);
            nodeInfo = NULL;
        }

        if(request != NULL)
        {
            free(request);
            request = NULL;
        }
    }
}

static void readNodes()
{
    PRINT("=============== Reading Nodes ==================");

    int idx;
    for(idx = 0; idx < 6; idx++)
    {
        PRINT_ARG("*****  Reading the node with browse name  ", node_arr[idx]);

        EdgeEndPointInfo* epRead = (EdgeEndPointInfo*) malloc(sizeof(EdgeEndPointInfo));
        epRead->endpointUri = endpointUri;
        epRead->config = NULL;

        EdgeNodeInfo *nodeInfo = (EdgeNodeInfo*) malloc(sizeof(EdgeNodeInfo));
        nodeInfo->valueAlias = node_arr[idx];

        EdgeRequest *request = (EdgeRequest*) malloc(sizeof(EdgeRequest));
        request->nodeInfo = nodeInfo;

        EdgeMessage *msgRead = (EdgeMessage*) malloc(sizeof(EdgeMessage));
        msgRead->endpointInfo = epRead;
        msgRead->command = CMD_READ;
        msgRead->request = request;

        readNode(msgRead);

        deleteMessage(msgRead, epRead);

        if(nodeInfo != NULL)
        {
            free(nodeInfo);
            nodeInfo = NULL;
        }

        if(request != NULL)
        {
            free(request);
            request = NULL;
        }
        
    }
}

class OPC_serverTests: public ::testing::Test
{
protected:

    virtual void SetUp()
    {      
        strcpy(ipAddress, LOCALHOST);
        snprintf(endpointUri, sizeof(endpointUri), ENDPOINT_URI, ipAddress);

        epInfo = (EdgeEndPointInfo*) malloc(sizeof(EdgeEndPointInfo));
        
    }

    virtual void TearDown()
    {
        if(epInfo != NULL)
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
    epInfo->endpointUri = (char*) malloc(len +1);
    EXPECT_EQ(NULL == epInfo->endpointUri, false);
    
    strncpy(epInfo->endpointUri, endpointUri, len);
    epInfo->endpointUri[len] = '\0';

    EXPECT_EQ(strcmp(epInfo->endpointUri, endpointUri) == 0, true);

    if(epInfo->endpointUri != NULL)
    {
        free(epInfo->endpointUri);
        epInfo->endpointUri = NULL;
    }
}

TEST_F(OPC_serverTests , InitializeServer_N)
{
    int len = strlen(endpointUri);
    epInfo->endpointUri = (char*) malloc(len +1);
    EXPECT_EQ(NULL == epInfo->endpointUri, false);
    
    //strncpy(epInfo->endpointUri, endpointUri, len);
    //epInfo->endpointUri[len] = '\0';

    EXPECT_EQ(strcmp(epInfo->endpointUri, endpointUri) == 0, false);

    if(epInfo->endpointUri != NULL)
    {
        free(epInfo->endpointUri);
        epInfo->endpointUri = NULL;
    }
}

TEST_F(OPC_serverTests , ConfigureServer_P)
{
    int len = strlen(endpointUri);
    epInfo->endpointUri = (char*) malloc(len +1);
    
    strncpy(epInfo->endpointUri, endpointUri, len);
    epInfo->endpointUri[len] = '\0';

    configureCallbacks();

    cleanCallbacks();

    if(epInfo->endpointUri != NULL)
    {
        free(epInfo->endpointUri);
        epInfo->endpointUri = NULL;
    }    
}

TEST_F(OPC_serverTests , StartServer_N)
{
    int len = strlen(endpointUri);
    epInfo->endpointUri = (char*) malloc(len +1);
    
    strncpy(epInfo->endpointUri, endpointUri, len);
    epInfo->endpointUri[len] = '\0';

    configureCallbacks();

    EdgeEndpointConfig* endpointConfig = (EdgeEndpointConfig*) malloc(sizeof(EdgeEndpointConfig));
    EXPECT_EQ(NULL != endpointConfig, true);
    
    endpointConfig->bindAddress = ipAddress;
    EXPECT_EQ(strcmp(endpointConfig->bindAddress, ipAddress) == 0, true);
    
    endpointConfig->bindPort = 12686;
    EXPECT_EQ(endpointConfig->bindPort, 12686);
    
    endpointConfig->applicationName = (char *)DEFAULT_SERVER_APP_NAME_VALUE;
    EXPECT_EQ(strcmp(endpointConfig->applicationName, DEFAULT_SERVER_APP_NAME_VALUE) == 0, true);
    
    endpointConfig->applicationUri = (char *)DEFAULT_SERVER_URI_VALUE;
    EXPECT_EQ(strcmp(endpointConfig->applicationUri, DEFAULT_SERVER_URI_VALUE) == 0, true);
    
    endpointConfig->productUri = (char *)DEFAULT_PRODUCT_URI_VALUE;
    EXPECT_EQ(strcmp(endpointConfig->productUri, DEFAULT_PRODUCT_URI_VALUE) == 0, true);
    
    endpointConfig->securityPolicyUri = NULL;
    EXPECT_EQ(endpointConfig->securityPolicyUri == NULL, true);
    
    endpointConfig->serverName = (char *)DEFAULT_SERVER_NAME_VALUE;
    EXPECT_EQ(strcmp(endpointConfig->serverName, DEFAULT_SERVER_NAME_VALUE) == 0, true);

    EdgeEndPointInfo* ep = (EdgeEndPointInfo*) malloc(sizeof(EdgeEndPointInfo));
    EXPECT_EQ(NULL != ep, true);
    ep->endpointUri = endpointUri;
    ASSERT_TRUE(strcmp(ep->endpointUri, "opc:tcp://localhost:12686/edge-opc-server") == 0);
    ep->config = endpointConfig;

    EdgeMessage* msg = (EdgeMessage*) malloc(sizeof(EdgeMessage));
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
    createServer(NULL);

    EXPECT_EQ(startServerFlag, false);

    deleteMessage(msg, ep);

    cleanCallbacks();

    if(epInfo->endpointUri != NULL)
    {
        free(epInfo->endpointUri);
        epInfo->endpointUri = NULL;
    }
}

TEST_F(OPC_serverTests , StartServer_P)
{
    int len = strlen(endpointUri);
    epInfo->endpointUri = (char*) malloc(len +1);
    
    strncpy(epInfo->endpointUri, endpointUri, len);
    epInfo->endpointUri[len] = '\0';
    configureCallbacks();

    EdgeEndpointConfig* endpointConfig = (EdgeEndpointConfig*) malloc(sizeof(EdgeEndpointConfig));    
    endpointConfig->bindAddress = ipAddress;    
    endpointConfig->bindPort = 12686;    
    endpointConfig->applicationName = (char *)DEFAULT_SERVER_APP_NAME_VALUE;    
    endpointConfig->applicationUri = (char *)DEFAULT_SERVER_URI_VALUE;    
    endpointConfig->productUri = (char *)DEFAULT_PRODUCT_URI_VALUE;    
    endpointConfig->securityPolicyUri = NULL;    
    endpointConfig->serverName = (char *)DEFAULT_SERVER_NAME_VALUE;
    EdgeEndPointInfo* ep = (EdgeEndPointInfo*) malloc(sizeof(EdgeEndPointInfo));
    ep->endpointUri = endpointUri;
    ep->config = endpointConfig;

    EdgeMessage* msg = (EdgeMessage*) malloc(sizeof(EdgeMessage));
    msg->endpointInfo = ep;
    msg->command = CMD_START_SERVER;
    
    msg->type = SEND_REQUEST;

    EXPECT_EQ(startServerFlag, false);

    PRINT("--- START SERVER ----");

    createServer(ep);

    EXPECT_EQ(startServerFlag, true);

    deleteMessage(msg, ep);

    cleanCallbacks();

    if(epInfo->endpointUri != NULL)
    {
        free(epInfo->endpointUri);
        epInfo->endpointUri = NULL;
    }
}

TEST_F(OPC_serverTests , ServerCreateNamespace_P)
{
    int len = strlen(endpointUri);
    epInfo->endpointUri = (char*) malloc(len +1);
    
    strncpy(epInfo->endpointUri, endpointUri, len);
    epInfo->endpointUri[len] = '\0';
    
    configureCallbacks();

    EdgeEndpointConfig* endpointConfig = (EdgeEndpointConfig*) malloc(sizeof(EdgeEndpointConfig));
    
    endpointConfig->bindAddress = ipAddress;
    endpointConfig->bindPort = 12686;
    endpointConfig->applicationName = (char *)DEFAULT_SERVER_APP_NAME_VALUE;
    endpointConfig->applicationUri = (char *)DEFAULT_SERVER_URI_VALUE;
    endpointConfig->productUri = (char *)DEFAULT_PRODUCT_URI_VALUE;
    endpointConfig->securityPolicyUri = NULL;
    endpointConfig->serverName = (char *)DEFAULT_SERVER_NAME_VALUE;

    EdgeEndPointInfo* ep = (EdgeEndPointInfo*) malloc(sizeof(EdgeEndPointInfo));
    ep->endpointUri = endpointUri;
    ep->config = endpointConfig;

    EdgeMessage* msg = (EdgeMessage*) malloc(sizeof(EdgeMessage));
    msg->endpointInfo = ep;
    msg->command = CMD_START_SERVER;
    msg->type = SEND_REQUEST;

    PRINT("--- START SERVER ----");

    createServer(ep);

    EXPECT_EQ(startServerFlag, true);

    EdgeResult result = createNamespace(DEFAULT_NAMESPACE_VALUE,
                              DEFAULT_ROOT_NODE_INFO_VALUE,
                              DEFAULT_ROOT_NODE_INFO_VALUE,
                              DEFAULT_ROOT_NODE_INFO_VALUE);

    EXPECT_EQ(result.code, STATUS_OK);

    deleteMessage(msg, ep);

    cleanCallbacks();

    if(epInfo->endpointUri != NULL)
    {
        free(epInfo->endpointUri);
        epInfo->endpointUri = NULL;
    }
}

TEST_F(OPC_serverTests , ServerAddNodes_P)
{
    int len = strlen(endpointUri);
    epInfo->endpointUri = (char*) malloc(len +1);
    
    strncpy(epInfo->endpointUri, endpointUri, len);
    epInfo->endpointUri[len] = '\0';
    
    configureCallbacks();

    EdgeEndpointConfig* endpointConfig = (EdgeEndpointConfig*) malloc(sizeof(EdgeEndpointConfig));
    endpointConfig->bindAddress = ipAddress;
    endpointConfig->bindPort = 12686;
    endpointConfig->applicationName = (char *)DEFAULT_SERVER_APP_NAME_VALUE;
    endpointConfig->applicationUri = (char *)DEFAULT_SERVER_URI_VALUE;
    endpointConfig->productUri = (char *)DEFAULT_PRODUCT_URI_VALUE;
    endpointConfig->securityPolicyUri = NULL;
    endpointConfig->serverName = (char *)DEFAULT_SERVER_NAME_VALUE;

    EdgeEndPointInfo* ep = (EdgeEndPointInfo*) malloc(sizeof(EdgeEndPointInfo));
    ep->endpointUri = endpointUri;
    ep->config = endpointConfig;

    EdgeMessage* msg = (EdgeMessage*) malloc(sizeof(EdgeMessage));
    msg->endpointInfo = ep;
    msg->command = CMD_START_SERVER;
    msg->type = SEND_REQUEST;

    PRINT("--- START SERVER ----");

    createServer(ep);

    EXPECT_EQ(startServerFlag, true);

    //NULL NODE
    EdgeResult result = createNode(DEFAULT_NAMESPACE_VALUE, NULL);
    EXPECT_EQ(result.code, STATUS_PARAM_INVALID);

    // NULL NODE->ITEM
    EdgeNodeItem* item = (EdgeNodeItem*) malloc(sizeof(EdgeNodeItem));
    item->nodeType = MILTI_FOLDER_NODE_TYPE; // NOT SUPPORTED YET
    result = createNode(DEFAULT_NAMESPACE_VALUE, item);
    EXPECT_EQ(result.code, STATUS_ERROR);

    // VARIABLE NODE with string variant:
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
    result = createNode(DEFAULT_NAMESPACE_VALUE, item);
    EXPECT_EQ(result.code, STATUS_OK);
    int value = 30;
    item->browseName = "UInt16";
    item->variableItemName = "Location";
    item->variableIdentifier = UInt16;
    item->variableData = (void*) &value;
    result = createNode(DEFAULT_NAMESPACE_VALUE, item);
    EXPECT_EQ(result.code, STATUS_OK);

    //Array Nodes with double values
    double* data = (double*) malloc(sizeof(double) * 2);
    data[0] = 10.2;
    data[1] = 20.2;
    item->browseName = "IntArray";
    item->nodeType = ARRAY_NODE;
    item->variableItemName = "IntArray";
    item->variableIdentifier = Double;
    item->arrayLength = 2;
    item->variableData = (void*) data;
    result = createNode(DEFAULT_NAMESPACE_VALUE, item);
    EXPECT_EQ(result.code, STATUS_OK);

    //OBJECT NODE
    item->nodeType = OBJECT_NODE;
    item->browseName = "Object1";
    item->sourceNodeId = (EdgeNodeId*) malloc (sizeof(EdgeNodeId));
    item->sourceNodeId->nodeId = NULL;    // no source node
    result = createNode(DEFAULT_NAMESPACE_VALUE, item);
    EXPECT_EQ(result.code, STATUS_OK);
    item->nodeType = OBJECT_NODE;
    item->browseName = "Object2";
    item->sourceNodeId = (EdgeNodeId*) malloc (sizeof(EdgeNodeId));
    item->sourceNodeId->nodeId = "Object1";
    result = createNode(DEFAULT_NAMESPACE_VALUE, item);
    EXPECT_EQ(result.code, STATUS_OK);

    //OBJECT TYPE NDOE
    item->nodeType = OBJECT_TYPE_NODE;
    item->browseName = "ObjectType1";
    item->sourceNodeId = (EdgeNodeId*) malloc (sizeof(EdgeNodeId));
    item->sourceNodeId->nodeId = NULL;    // no source node
    result = createNode(DEFAULT_NAMESPACE_VALUE, item);
    EXPECT_EQ(result.code, STATUS_OK);
    item->nodeType = OBJECT_TYPE_NODE;
    item->browseName = "ObjectType2";
    item->sourceNodeId = (EdgeNodeId*) malloc (sizeof(EdgeNodeId));
    item->sourceNodeId->nodeId = "ObjectType1";
    result = createNode(DEFAULT_NAMESPACE_VALUE, item);
    EXPECT_EQ(result.code, STATUS_OK);

    //DATA TYPE NODE
    item->nodeType = DATA_TYPE_NODE;
    item->browseName = "DataType1";
    item->sourceNodeId = (EdgeNodeId*) malloc (sizeof(EdgeNodeId));
    item->sourceNodeId->nodeId = NULL;    // no source node
    result = createNode(DEFAULT_NAMESPACE_VALUE, item);
    EXPECT_EQ(result.code, STATUS_OK);

    item->nodeType = DATA_TYPE_NODE;
    item->browseName = "DataType2";
    item->sourceNodeId = (EdgeNodeId*) malloc (sizeof(EdgeNodeId));
    item->sourceNodeId->nodeId = "DataType1";
    result = createNode(DEFAULT_NAMESPACE_VALUE, item);
    EXPECT_EQ(result.code, STATUS_OK);

    //VIEW NODE
    item->nodeType = VIEW_NODE;
    item->browseName = "ViewNode1";
    item->sourceNodeId = (EdgeNodeId*) malloc (sizeof(EdgeNodeId));
    item->sourceNodeId->nodeId = NULL;    // no source node
    result = createNode(DEFAULT_NAMESPACE_VALUE, item);
    EXPECT_EQ(result.code, STATUS_OK);
    item->nodeType = VIEW_NODE;
    item->browseName = "ViewNode2";
    item->sourceNodeId = (EdgeNodeId*) malloc (sizeof(EdgeNodeId));
    item->sourceNodeId->nodeId = "ViewNode1";    // no source node
    result = createNode(DEFAULT_NAMESPACE_VALUE, item);
    EXPECT_EQ(result.code, STATUS_OK);

    //REFERENCE NODE
    EdgeReference* reference = (EdgeReference*) malloc(sizeof(EdgeReference));
    reference->forward=  false;
    reference->sourceNamespace = DEFAULT_NAMESPACE_VALUE;
    reference->sourcePath = "ObjectType1";
    reference->targetNamespace = DEFAULT_NAMESPACE_VALUE;
    reference->targetPath = "ObjectType2";
    result = addReference(reference);
    EXPECT_EQ(result.code, STATUS_OK);
    addReference(reference);
    reference->forward=  true;
    reference->sourceNamespace = DEFAULT_NAMESPACE_VALUE;
    reference->sourcePath = "ViewNode1";
    reference->targetNamespace = DEFAULT_NAMESPACE_VALUE;
    reference->targetPath = "ObjectType2";
    result = addReference(reference);
    EXPECT_EQ(result.code, STATUS_OK);

    //REFERENCE TYPE NODE
    item->nodeType = REFERENCE_TYPE_NODE;
    item->browseName = "ReferenceTypeNode1";
    item->sourceNodeId = (EdgeNodeId*) malloc (sizeof(EdgeNodeId));
    item->sourceNodeId->nodeId = NULL;    // no source node
    result = createNode(DEFAULT_NAMESPACE_VALUE, item);
    EXPECT_EQ(result.code, STATUS_OK);
    item->nodeType = REFERENCE_TYPE_NODE;
    item->browseName = "ReferenceTypeNode2";
    item->sourceNodeId = (EdgeNodeId*) malloc (sizeof(EdgeNodeId));
    item->sourceNodeId->nodeId = "ReferenceTypeNode1";
    result = createNode(DEFAULT_NAMESPACE_VALUE, item);
    EXPECT_EQ(result.code, STATUS_OK);

    deleteMessage(msg, ep);

    cleanCallbacks();

    if(epInfo->endpointUri != NULL)
    {
        free(epInfo->endpointUri);
        epInfo->endpointUri = NULL;
    }
}

TEST_F(OPC_serverTests , StartStopServer_P)
{
    int len = strlen(endpointUri);
    epInfo->endpointUri = (char*) malloc(len +1);
    
    strncpy(epInfo->endpointUri, endpointUri, len);
    epInfo->endpointUri[len] = '\0';
    
    configureCallbacks();

    EdgeEndpointConfig* endpointConfig = (EdgeEndpointConfig*) malloc(sizeof(EdgeEndpointConfig));
    
    endpointConfig->bindAddress = ipAddress;
    endpointConfig->bindPort = 12686;
    endpointConfig->applicationName = (char *)DEFAULT_SERVER_APP_NAME_VALUE;
    endpointConfig->applicationUri = (char *)DEFAULT_SERVER_URI_VALUE;
    endpointConfig->productUri = (char *)DEFAULT_PRODUCT_URI_VALUE;
    endpointConfig->securityPolicyUri = NULL;
    endpointConfig->serverName = (char *)DEFAULT_SERVER_NAME_VALUE;

    EdgeEndPointInfo* ep = (EdgeEndPointInfo*) malloc(sizeof(EdgeEndPointInfo));
    ep->endpointUri = endpointUri;
    ep->config = endpointConfig;

    EdgeMessage* msg = (EdgeMessage*) malloc(sizeof(EdgeMessage));
    msg->endpointInfo = ep;
    msg->command = CMD_START_SERVER;
    msg->type = SEND_REQUEST;

    PRINT("--- START SERVER ----");

    createServer(ep);

    ASSERT_TRUE(startServerFlag);

    deleteMessage(msg, ep);

    PRINT("=============STOP SERVER===============");

    EdgeEndPointInfo* epStop = (EdgeEndPointInfo*) malloc(sizeof(EdgeEndPointInfo));
    epStop->endpointUri = endpointUri;
    epStop->config = NULL;

    EdgeMessage* msgStop = (EdgeMessage*) malloc(sizeof(EdgeMessage));
    ASSERT_TRUE(NULL != msgStop);
    msgStop->endpointInfo = epStop;
    msgStop->command = CMD_STOP_SERVER;
    EXPECT_EQ(msgStop->command, CMD_STOP_SERVER);
    msgStop->type = SEND_REQUEST;
    EXPECT_EQ(msgStop->type, SEND_REQUEST);

    closeServer(epStop);

    ASSERT_FALSE(startServerFlag);

    deleteMessage(msgStop, epStop);

    cleanCallbacks();

    if(epInfo->endpointUri != NULL)
    {
        free(epInfo->endpointUri);
        epInfo->endpointUri = NULL;
    }
}

TEST_F(OPC_serverTests , StartServerNew_P)
{
    int len = strlen(endpointUri);
    epInfo->endpointUri = (char*) malloc(len +1);
    
    strncpy(epInfo->endpointUri, endpointUri, len);
    epInfo->endpointUri[len] = '\0';
    configureCallbacks();

    EdgeEndpointConfig* endpointConfig = (EdgeEndpointConfig*) malloc(sizeof(EdgeEndpointConfig));
    
    endpointConfig->bindAddress = ipAddress;
    endpointConfig->bindPort = 12686;
    endpointConfig->applicationName = (char *)DEFAULT_SERVER_APP_NAME_VALUE;
    endpointConfig->applicationUri = (char *)DEFAULT_SERVER_URI_VALUE;
    endpointConfig->productUri = (char *)DEFAULT_PRODUCT_URI_VALUE;
    endpointConfig->securityPolicyUri = NULL;
    endpointConfig->serverName = (char *)DEFAULT_SERVER_NAME_VALUE;

    EdgeEndPointInfo* ep = (EdgeEndPointInfo*) malloc(sizeof(EdgeEndPointInfo));
    ep->endpointUri = endpointUri;
    ep->config = endpointConfig;

    EdgeMessage* msg = (EdgeMessage*) malloc(sizeof(EdgeMessage));
    msg->endpointInfo = ep;
    msg->command = CMD_START_SERVER;
    msg->type = SEND_REQUEST;

    PRINT("--- START SERVER ----");

    createServer(ep);

    ASSERT_TRUE(startServerFlag);

    deleteMessage(msg, ep);

    cleanCallbacks();

    if(epInfo->endpointUri != NULL)
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
    int len = strlen(IPADDRESS);
    strcpy(ipAddress, IPADDRESS);
    ipAddress[len] = '\0';

    strcpy(endpointUri, ipAddress);
    endpointUri[len] = '\0';

    EXPECT_EQ(strcmp(ipAddress, IPADDRESS) == 0, true);

    EXPECT_EQ(strcmp(endpointUri, ENDPOINT_URI) == 0, false);

    EXPECT_EQ(strcmp(endpointUri, ipAddress) == 0, true);
}

TEST_F(OPC_clientTests , StartClient_P)
{

    int len = strlen(IPADDRESS);
    strcpy(ipAddress, IPADDRESS);
    ipAddress[len] = '\0';

    strcpy(endpointUri, ipAddress);
    endpointUri[len] = '\0';

    EdgeEndpointConfig* endpointConfig = (EdgeEndpointConfig*) malloc(sizeof(EdgeEndpointConfig));
    EXPECT_EQ(NULL != endpointConfig,  true);
    endpointConfig->bindAddress = ipAddress;
    EXPECT_EQ(strcmp(endpointConfig->bindAddress, ipAddress) == 0,  true);
    endpointConfig->bindPort = 12686;
    EXPECT_EQ(endpointConfig->bindPort, 12686);
    endpointConfig->applicationName = (char *)DEFAULT_SERVER_APP_NAME_VALUE;
    EXPECT_EQ(strcmp(endpointConfig->applicationName, DEFAULT_SERVER_APP_NAME_VALUE) == 0,  true);
    endpointConfig->applicationUri = (char *)DEFAULT_SERVER_APP_URI_VALUE;
    EXPECT_EQ(strcmp(endpointConfig->applicationUri, DEFAULT_SERVER_APP_URI_VALUE) == 0,  true);
    endpointConfig->productUri = (char *)DEFAULT_PRODUCT_URI_VALUE;
    EXPECT_EQ(strcmp(endpointConfig->productUri, DEFAULT_PRODUCT_URI_VALUE) == 0,  true);
    endpointConfig->securityPolicyUri = NULL;
    EXPECT_EQ(endpointConfig->securityPolicyUri == NULL,  true);
    endpointConfig->serverName = (char *)DEFAULT_SERVER_NAME_VALUE;
    EXPECT_EQ(strcmp(endpointConfig->serverName, DEFAULT_SERVER_NAME_VALUE) == 0,  true);
    endpointConfig->requestTimeout = 60000;
    EXPECT_EQ(endpointConfig->requestTimeout, 60000);

    EdgeEndPointInfo* ep = (EdgeEndPointInfo*) malloc(sizeof(EdgeEndPointInfo));
    EXPECT_EQ(NULL != ep, true);
    ep->endpointUri = endpointUri;
    EXPECT_EQ(strcmp(ep->endpointUri, ipAddress) == 0, true);
    ep->config = endpointConfig;

    EdgeMessage* msg = (EdgeMessage*) malloc(sizeof(EdgeMessage));
    EXPECT_EQ(NULL != msg, true);
    msg->endpointInfo = ep;
    msg->command = CMD_START_CLIENT;
    EXPECT_EQ(msg->command, CMD_START_CLIENT);
    msg->type = SEND_REQUEST;
    EXPECT_EQ(msg->type, SEND_REQUEST);

    PRINT("=============== startClient ==================");
    EXPECT_EQ(startClientFlag, false);
    
    connectClient(ep);

    EXPECT_EQ(startClientFlag, true);

    deleteMessage(msg, ep);

    EdgeEndPointInfo* ep_t = (EdgeEndPointInfo*) malloc(sizeof(EdgeEndPointInfo));
    ep_t->endpointUri = endpointUri;
    ep_t->config = NULL;

    EdgeMessage* msg_t = (EdgeMessage*) malloc(sizeof(EdgeMessage));
    msg_t->endpointInfo = ep_t;
    msg_t->command = CMD_STOP_CLIENT;

    disconnectClient(ep_t);

    deleteMessage(msg_t, ep_t);

}

TEST_F(OPC_clientTests , ClientRead_P)
{

    int len = strlen(IPADDRESS);
    strcpy(ipAddress, IPADDRESS);
    ipAddress[len] = '\0';

    strcpy(endpointUri, ipAddress);
    endpointUri[len] = '\0';

    EdgeEndpointConfig* endpointConfig = (EdgeEndpointConfig*) malloc(sizeof(EdgeEndpointConfig));
    endpointConfig->bindAddress = ipAddress;
    
    endpointConfig->bindPort = 12686;
    endpointConfig->applicationName = (char *)DEFAULT_SERVER_APP_NAME_VALUE;
    endpointConfig->applicationUri = (char *)DEFAULT_SERVER_APP_URI_VALUE;
    endpointConfig->productUri = (char *)DEFAULT_PRODUCT_URI_VALUE;
    endpointConfig->securityPolicyUri = NULL;
    endpointConfig->serverName = (char *)DEFAULT_SERVER_NAME_VALUE;
    endpointConfig->requestTimeout = 60000;

    EdgeEndPointInfo* ep = (EdgeEndPointInfo*) malloc(sizeof(EdgeEndPointInfo));
    ep->endpointUri = endpointUri;
    ep->config = endpointConfig;

    EdgeMessage* msg = (EdgeMessage*) malloc(sizeof(EdgeMessage));
    msg->endpointInfo = ep;
    msg->command = CMD_START_CLIENT;
    msg->type = SEND_REQUEST;

    PRINT("=============== startClient ==================");
    EXPECT_EQ(startClientFlag, false);
    connectClient(ep);
    EXPECT_EQ(startClientFlag, true);

    readNodes();

    EdgeEndPointInfo* ep_t = (EdgeEndPointInfo*) malloc(sizeof(EdgeEndPointInfo));
    ep_t->endpointUri = endpointUri;
    ep_t->config = NULL;

    EdgeMessage* msg_t = (EdgeMessage*) malloc(sizeof(EdgeMessage));
    msg_t->endpointInfo = ep_t;
    msg_t->command = CMD_STOP_CLIENT;

    disconnectClient(ep_t);

    deleteMessage(msg_t, ep_t);

     deleteMessage(msg, ep);

}

TEST_F(OPC_clientTests , ClientWrite_P)
{

    int len = strlen(IPADDRESS);
    strcpy(ipAddress, IPADDRESS);
    ipAddress[len] = '\0';

    strcpy(endpointUri, ipAddress);
    endpointUri[len] = '\0';

    EdgeEndpointConfig* endpointConfig = (EdgeEndpointConfig*) malloc(sizeof(EdgeEndpointConfig));
    endpointConfig->bindAddress = ipAddress;
    
    endpointConfig->bindPort = 12686;
    endpointConfig->applicationName = (char *)DEFAULT_SERVER_APP_NAME_VALUE;
    endpointConfig->applicationUri = (char *)DEFAULT_SERVER_APP_URI_VALUE;
    endpointConfig->productUri = (char *)DEFAULT_PRODUCT_URI_VALUE;
    endpointConfig->securityPolicyUri = NULL;
    endpointConfig->serverName = (char *)DEFAULT_SERVER_NAME_VALUE;
    endpointConfig->requestTimeout = 60000;

    EdgeEndPointInfo* ep = (EdgeEndPointInfo*) malloc(sizeof(EdgeEndPointInfo));
    ep->endpointUri = endpointUri;
    ep->config = endpointConfig;

    EdgeMessage* msg = (EdgeMessage*) malloc(sizeof(EdgeMessage));
    msg->endpointInfo = ep;
    msg->command = CMD_START_CLIENT;
    msg->type = SEND_REQUEST;

    PRINT("=============== startClient ==================");
    EXPECT_EQ(startClientFlag, false);
    connectClient(ep);
    EXPECT_EQ(startClientFlag, true);

    writeNodes(false);

    readNodeFlag = false;

    readNodes();

     writeNodes(true);

     deleteMessage(msg, ep);

    EdgeEndPointInfo* ep_t = (EdgeEndPointInfo*) malloc(sizeof(EdgeEndPointInfo));
    ep_t->endpointUri = endpointUri;
    ep_t->config = NULL;

    EdgeMessage* msg_t = (EdgeMessage*) malloc(sizeof(EdgeMessage));
    msg_t->endpointInfo = ep_t;
    msg_t->command = CMD_STOP_CLIENT;

    disconnectClient(ep_t);

    deleteMessage(msg_t, ep_t);
        
}

int main(int argc, char **argv) {
  ::testing::InitGoogleTest(&argc, argv);
  return RUN_ALL_TESTS();
}
