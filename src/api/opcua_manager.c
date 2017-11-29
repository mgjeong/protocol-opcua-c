#include "opcua_manager.h"
#include "edge_opcua_server.h"
#include "edge_opcua_client.h"
#include "message_dispatcher.h"

#include <stdio.h>
#include <stdlib.h>

static ReceivedMessageCallback* receivedMsgCb;
static StatusCallback* statusCb;
static DiscoveryCallback *discoveryCb;

static bool b_serverInitialized = false;
static bool b_clientInitialized = false;

static void registerRecvCallback(ReceivedMessageCallback* callback) {
  receivedMsgCb = callback;
}

static void registerStatusCallback(StatusCallback* callback) {
  statusCb = callback;
}

static void registerDiscoveryCallback(DiscoveryCallback* callback) {
  discoveryCb = callback;
}

void configure(EdgeConfigure *config) {
  registerRecvCallback(config->recvCallback);
  registerStatusCallback(config->statusCallback);
  registerDiscoveryCallback(config->discoveryCallback);
}

EdgeResult* send(EdgeMessage* msg) {
  printf("send message\n");
  bool ret = add_to_sendQ(msg);

  EdgeResult* result = (EdgeResult*) malloc(sizeof(EdgeResult));
  result->code = (ret  ? STATUS_OK : STATUS_ENQUEUE_ERROR);
  return result;
}

void onSendMessage(EdgeMessage* msg) {
  if (msg->command == CMD_START_SERVER) {
    if (b_serverInitialized) {
      printf( "Server already initialised");
      return ;
    }
    EdgeResult* result = start_server(msg->endpointInfo);
    if (result == NULL)
      return ;
    if (result->code == STATUS_OK) {
      b_serverInitialized = true;
    }
  } else if (msg->command == CMD_START_CLIENT) {
    if (b_clientInitialized) {
      printf( "Client already initialised");
      return ;
    }
    bool result = connect_client(msg->endpointInfo->endpointUri);
    if (!result)
      return ;
   b_clientInitialized = true;
  } else if (msg->command == CMD_STOP_SERVER) {
    stop_server();
    b_serverInitialized = false;
  } else if (msg->command == CMD_STOP_CLIENT) {
    disconnect_client();
    b_clientInitialized = false;
  }
}
