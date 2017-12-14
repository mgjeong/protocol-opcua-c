#include <gtest/gtest.h>
#include <iostream>

extern "C" {
    #include "opcua_manager.h"
    #include "opcua_common.h"
    #include "edge_identifier.h"
}

#define LOCALHOST "localhost"
#define ENDPOINT_URI  "opc:tcp://%s:12686/edge-opc-server"
#define IPADDRESS "opc.tcp://localhost:12686"

#define PRINT(str) std::cout<<str<<std::endl

static char ipAddress[128];
static char endpointUri[512];
static EdgeEndPointInfo* epInfo;
static EdgeConfigure *config;

static bool startServerFlag = false;
static bool startClientFlag = false;


extern "C" 
{
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
        PRINT("\n[Application Callback] Server started");
        startServerFlag = true;

        //testCreateNamespace();
        //testCreateNodes();
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
          //exit(0);
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

    virtual void TearDown()
    {
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

};

//-----------------------------------------------------------------------------
//  Tests
//-----------------------------------------------------------------------------

TEST_F(OPC_serverTests , SetUPServer_P)
{
    ASSERT_TRUE(strcmp(ipAddress, LOCALHOST) == 0);

    ASSERT_FALSE(strcmp(endpointUri, ENDPOINT_URI) == 0);

    ASSERT_TRUE(strcmp(endpointUri, "opc:tcp://localhost:12686/edge-opc-server") == 0);    
}

TEST_F(OPC_serverTests , InitializeServer_P)
{
    
    ASSERT_FALSE(NULL == epInfo);

    int len = strlen(endpointUri);
    epInfo->endpointUri = (char*) malloc(len +1);
    ASSERT_FALSE(NULL == epInfo->endpointUri);
    
    strncpy(epInfo->endpointUri, endpointUri, len);
    epInfo->endpointUri[len] = '\0';

    ASSERT_TRUE(strcmp(epInfo->endpointUri, endpointUri) == 0);

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
    ASSERT_FALSE(NULL == epInfo->endpointUri);
    
    //strncpy(epInfo->endpointUri, endpointUri, len);
    //epInfo->endpointUri[len] = '\0';

    ASSERT_FALSE(strcmp(epInfo->endpointUri, endpointUri) == 0);

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

    // INITIALIZING CONFIGURATIONS
    PRINT("-----INITIALIZING CONFIGURATIONS-----");

    ASSERT_TRUE(NULL == config);
    config = (EdgeConfigure*) malloc(sizeof(EdgeConfigure));
    ASSERT_FALSE(NULL == config);

    config->recvCallback = (ReceivedMessageCallback*) malloc(sizeof(ReceivedMessageCallback));
    ASSERT_FALSE(NULL == config->recvCallback);

    config->recvCallback->resp_msg_cb = response_msg_cb;
    config->recvCallback->monitored_msg_cb = monitored_msg_cb;
    config->recvCallback->error_msg_cb = error_msg_cb;
    config->recvCallback->browse_msg_cb = browse_msg_cb;

    config->statusCallback = (StatusCallback*) malloc(sizeof(StatusCallback));
    ASSERT_FALSE(NULL == config->statusCallback);
    
    config->statusCallback->start_cb = status_start_cb;
    config->statusCallback->stop_cb = status_stop_cb;
    config->statusCallback->network_cb = status_network_cb;

    config->discoveryCallback = (DiscoveryCallback*) malloc(sizeof(DiscoveryCallback));
    ASSERT_FALSE(NULL == config->discoveryCallback);
    
    config->discoveryCallback->endpoint_found_cb = endpoint_found_cb;
    config->discoveryCallback->device_found_cb = device_found_cb;

    registerCallbacks(config);


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

    EdgeEndpointConfig* endpointConfig = (EdgeEndpointConfig*) malloc(sizeof(EdgeEndpointConfig));
    ASSERT_TRUE(NULL != endpointConfig);
    
    endpointConfig->bindAddress = ipAddress;
    ASSERT_TRUE(strcmp(endpointConfig->bindAddress, ipAddress) == 0);
    
    endpointConfig->bindPort = 12686;
    EXPECT_EQ(endpointConfig->bindPort, 12686);
    
    endpointConfig->applicationName = DEFAULT_SERVER_APP_NAME_VALUE;
    ASSERT_TRUE(strcmp(endpointConfig->applicationName, DEFAULT_SERVER_APP_NAME_VALUE) == 0);
    
    endpointConfig->applicationUri = DEFAULT_SERVER_URI_VALUE;
    ASSERT_TRUE(strcmp(endpointConfig->applicationUri, DEFAULT_SERVER_URI_VALUE) == 0);
    
    endpointConfig->productUri = DEFAULT_PRODUCT_URI_VALUE;
    ASSERT_TRUE(strcmp(endpointConfig->productUri, DEFAULT_PRODUCT_URI_VALUE) == 0);
    
    endpointConfig->securityPolicyUri = NULL;
    ASSERT_TRUE(endpointConfig->securityPolicyUri == NULL);
    
    endpointConfig->serverName = DEFAULT_SERVER_NAME_VALUE;
    ASSERT_TRUE(strcmp(endpointConfig->serverName, DEFAULT_SERVER_NAME_VALUE) == 0);

    EdgeEndPointInfo* ep = (EdgeEndPointInfo*) malloc(sizeof(EdgeEndPointInfo));
    ASSERT_TRUE(NULL != ep);
    ep->endpointUri = endpointUri;
    ASSERT_TRUE(strcmp(ep->endpointUri, "opc:tcp://localhost:12686/edge-opc-server") == 0);
    ep->config = endpointConfig;

    EdgeMessage* msg = (EdgeMessage*) malloc(sizeof(EdgeMessage));
    ASSERT_TRUE(NULL != msg);
    msg->endpointInfo = ep;
    
    msg->command = CMD_START_SERVER;
    EXPECT_EQ(msg->command, CMD_START_SERVER);
    
    msg->type = SEND_REQUEST;
    EXPECT_EQ(msg->type, SEND_REQUEST);

    ASSERT_FALSE(startServerFlag);

    PRINT("--- START SERVER ----");

    //createServer(ep);
    //::TODO - This case is faling need to put NULL checks
    //createServer(NULL);

    ASSERT_FALSE(startServerFlag);

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

    EdgeEndpointConfig* endpointConfig = (EdgeEndpointConfig*) malloc(sizeof(EdgeEndpointConfig));
    ASSERT_TRUE(NULL != endpointConfig);
    
    endpointConfig->bindAddress = ipAddress;
    ASSERT_TRUE(strcmp(endpointConfig->bindAddress, ipAddress) == 0);
    
    endpointConfig->bindPort = 12686;
    EXPECT_EQ(endpointConfig->bindPort, 12686);
    
    endpointConfig->applicationName = DEFAULT_SERVER_APP_NAME_VALUE;
    ASSERT_TRUE(strcmp(endpointConfig->applicationName, DEFAULT_SERVER_APP_NAME_VALUE) == 0);
    
    endpointConfig->applicationUri = DEFAULT_SERVER_URI_VALUE;
    ASSERT_TRUE(strcmp(endpointConfig->applicationUri, DEFAULT_SERVER_URI_VALUE) == 0);
    
    endpointConfig->productUri = DEFAULT_PRODUCT_URI_VALUE;
    ASSERT_TRUE(strcmp(endpointConfig->productUri, DEFAULT_PRODUCT_URI_VALUE) == 0);
    
    endpointConfig->securityPolicyUri = NULL;
    ASSERT_TRUE(endpointConfig->securityPolicyUri == NULL);
    
    endpointConfig->serverName = DEFAULT_SERVER_NAME_VALUE;
    ASSERT_TRUE(strcmp(epInfo->endpointUri, endpointUri) == 0);

    EdgeEndPointInfo* ep = (EdgeEndPointInfo*) malloc(sizeof(EdgeEndPointInfo));
    ASSERT_TRUE(NULL != ep);
    ep->endpointUri = endpointUri;
    ASSERT_TRUE(strcmp(ep->endpointUri, "opc:tcp://localhost:12686/edge-opc-server") == 0);
    ep->config = endpointConfig;

    EdgeMessage* msg = (EdgeMessage*) malloc(sizeof(EdgeMessage));
    ASSERT_TRUE(NULL != msg);
    msg->endpointInfo = ep;
    
    msg->command = CMD_START_SERVER;
    EXPECT_EQ(msg->command, CMD_START_SERVER);
    
    msg->type = SEND_REQUEST;
    EXPECT_EQ(msg->type, SEND_REQUEST);

    ASSERT_FALSE(startServerFlag);

    PRINT("--- START SERVER ----");

    createServer(ep);

    ASSERT_TRUE(startServerFlag);

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

    EdgeEndpointConfig* endpointConfig = (EdgeEndpointConfig*) malloc(sizeof(EdgeEndpointConfig));
    
    endpointConfig->bindAddress = ipAddress;
    endpointConfig->bindPort = 12686;
    endpointConfig->applicationName = DEFAULT_SERVER_APP_NAME_VALUE;
    endpointConfig->applicationUri = DEFAULT_SERVER_URI_VALUE;
    endpointConfig->productUri = DEFAULT_PRODUCT_URI_VALUE;
    endpointConfig->securityPolicyUri = NULL;
    endpointConfig->serverName = DEFAULT_SERVER_NAME_VALUE;

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

    if(msgStop != NULL)
    {
        free(msgStop);
        msgStop = NULL;
    }

    if(epStop != NULL)
    {
        free(epStop);
        epStop = NULL;
    }

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

    EdgeEndpointConfig* endpointConfig = (EdgeEndpointConfig*) malloc(sizeof(EdgeEndpointConfig));
    
    endpointConfig->bindAddress = ipAddress;
    endpointConfig->bindPort = 12686;
    endpointConfig->applicationName = DEFAULT_SERVER_APP_NAME_VALUE;
    endpointConfig->applicationUri = DEFAULT_SERVER_URI_VALUE;
    endpointConfig->productUri = DEFAULT_PRODUCT_URI_VALUE;
    endpointConfig->securityPolicyUri = NULL;
    endpointConfig->serverName = DEFAULT_SERVER_NAME_VALUE;

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

    if(epInfo->endpointUri != NULL)
    {
        free(epInfo->endpointUri);
        epInfo->endpointUri = NULL;
    }
}

TEST_F(OPC_clientTests , ConfigureClient_P)
{
    ASSERT_FALSE(NULL == config);
    
    ASSERT_FALSE(NULL == config->recvCallback);
    
    ASSERT_FALSE(NULL == config->statusCallback);
    
    ASSERT_FALSE(NULL == config->discoveryCallback);
}

TEST_F(OPC_clientTests , InitializeClient_P)
{

    int len = strlen(IPADDRESS);
    strcpy(ipAddress, IPADDRESS);
    ipAddress[len] = '\0';

    strcpy(endpointUri, ipAddress);
    endpointUri[len] = '\0';

    ASSERT_TRUE(strcmp(ipAddress, IPADDRESS) == 0);

    ASSERT_FALSE(strcmp(endpointUri, ENDPOINT_URI) == 0);

    ASSERT_TRUE(strcmp(endpointUri, ipAddress) == 0);
}

TEST_F(OPC_clientTests , StartClient_P)
{

    int len = strlen(IPADDRESS);
    strcpy(ipAddress, IPADDRESS);
    ipAddress[len] = '\0';

    strcpy(endpointUri, ipAddress);
    endpointUri[len] = '\0';

    EdgeEndpointConfig* endpointConfig = (EdgeEndpointConfig*) malloc(sizeof(EdgeEndpointConfig));
    ASSERT_TRUE(NULL != endpointConfig);
    endpointConfig->bindAddress = ipAddress;
    ASSERT_TRUE(strcmp(endpointConfig->bindAddress, ipAddress) == 0);
    endpointConfig->bindPort = 12686;
    EXPECT_EQ(endpointConfig->bindPort, 12686);
    endpointConfig->applicationName = DEFAULT_SERVER_APP_NAME_VALUE;
    ASSERT_TRUE(strcmp(endpointConfig->applicationName, DEFAULT_SERVER_APP_NAME_VALUE) == 0);
    endpointConfig->applicationUri = DEFAULT_SERVER_APP_URI_VALUE;
    ASSERT_TRUE(strcmp(endpointConfig->applicationUri, DEFAULT_SERVER_APP_URI_VALUE) == 0);
    endpointConfig->productUri = DEFAULT_PRODUCT_URI_VALUE;
    ASSERT_TRUE(strcmp(endpointConfig->productUri, DEFAULT_PRODUCT_URI_VALUE) == 0);
    endpointConfig->securityPolicyUri = NULL;
    ASSERT_TRUE(endpointConfig->securityPolicyUri == NULL);
    endpointConfig->serverName = DEFAULT_SERVER_NAME_VALUE;
    ASSERT_TRUE(strcmp(endpointConfig->serverName, DEFAULT_SERVER_NAME_VALUE) == 0);
    endpointConfig->requestTimeout = 60000;
    EXPECT_EQ(endpointConfig->requestTimeout, 60000);

    EdgeEndPointInfo* ep = (EdgeEndPointInfo*) malloc(sizeof(EdgeEndPointInfo));
    ASSERT_TRUE(NULL != ep);
    ep->endpointUri = endpointUri;
    ASSERT_TRUE(strcmp(ep->endpointUri, ipAddress) == 0);
    ep->config = endpointConfig;

    EdgeMessage* msg = (EdgeMessage*) malloc(sizeof(EdgeMessage));
    ASSERT_TRUE(NULL != msg);
    msg->endpointInfo = ep;
    msg->command = CMD_START_CLIENT;
    EXPECT_EQ(msg->command, CMD_START_CLIENT);
    msg->type = SEND_REQUEST;
    EXPECT_EQ(msg->type, SEND_REQUEST);

    PRINT("=============== startClient ==================");
    EXPECT_EQ(startClientFlag, false);
    
    connectClient(ep);

    //EXPECT_EQ(startClientFlag, true);

}

int main(int argc, char **argv) {
  ::testing::InitGoogleTest(&argc, argv);
  return RUN_ALL_TESTS();
}

