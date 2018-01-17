#ifndef EDGE_OPCUA_MANAGER_H
#define EDGE_OPCUA_MANAGER_H

#include "opcua_common.h"

#ifdef __cplusplus
extern "C" {
#endif

typedef struct EdgeConfigure EdgeConfigure;
typedef struct EdgeResult EdgeResult;
typedef struct EdgeMessage EdgeMessage;
typedef struct EdgeDevice EdgeDevice;
typedef struct EdgeEndPointInfo EdgeEndPointInfo;
typedef struct EdgeReference EdgeReference;

/* Recevied Message callbacks */
typedef void (*response_msg_cb_t) (EdgeMessage* data);
typedef void (*monitored_msg_cb_t) (EdgeMessage* data);
typedef void (*error_msg_cb_t) (EdgeMessage* data);
typedef void (*browse_msg_cb_t) (EdgeMessage* data);

/* status callbacks */
typedef void (*status_start_cb_t) (EdgeEndPointInfo* epInfo, EdgeStatusCode status);
typedef void (*status_stop_cb_t) (EdgeEndPointInfo* epInfo, EdgeStatusCode status);
typedef void (*status_network_cb_t) (EdgeEndPointInfo* epInfo, EdgeStatusCode status);

/* discovery callback */
typedef void (*endpoint_found_cb_t) (EdgeDevice* device);
typedef void (*device_found_cb_t) (EdgeDevice* device);


typedef struct ReceivedMessageCallback {
  response_msg_cb_t resp_msg_cb;
  monitored_msg_cb_t monitored_msg_cb;
  error_msg_cb_t error_msg_cb;
  browse_msg_cb_t browse_msg_cb;
} ReceivedMessageCallback;

typedef struct StatusCallback {
  status_start_cb_t start_cb;
  status_stop_cb_t stop_cb;
  status_network_cb_t network_cb;
} StatusCallback;

typedef struct DiscoveryCallback {
  endpoint_found_cb_t endpoint_found_cb;
  device_found_cb_t device_found_cb;
} DiscoveryCallback;

//typedef struct {
//  ReceivedMessageCallback* recvCallback;
//  StatusCallback* statusCallback;
//  DiscoveryCallback* discoveryCallback;
//} EdgeConfigure;

//void onSendMessage(EdgeMessage* msg);
void onResponseMessage(EdgeMessage *msg);
void onStatusCallback(EdgeEndPointInfo* epInfo, EdgeStatusCode status);
void onDiscoveryCallback(EdgeDevice *device);


// Server
__attribute__((visibility("default"))) void createServer(EdgeEndPointInfo *epInfo);
__attribute__((visibility("default"))) void closeServer(EdgeEndPointInfo *epInfo);

// Client
__attribute__((visibility("default"))) void getEndpointInfo(EdgeEndPointInfo* epInfo);
__attribute__((visibility("default"))) void connectClient(EdgeEndPointInfo *epInfo);
__attribute__((visibility("default"))) void disconnectClient(EdgeEndPointInfo *epInfo);

__attribute__((visibility("default"))) void registerCallbacks(EdgeConfigure *config);
//__attribute__((visibility("default"))) EdgeResult* send(EdgeMessage* msg);
__attribute__((visibility("default"))) EdgeResult createNamespace(char* name, char* rootNodeId,
                                                                   char* rootBrowseName, char* rootDisplayName);
__attribute__((visibility("default"))) EdgeResult createNode(char* namespaceUri, EdgeNodeItem* item);
__attribute__((visibility("default"))) EdgeResult modifyVariableNode(char* namespaceUri, char* nodeUri, EdgeVersatility *value);
__attribute__((visibility("default"))) EdgeResult createMethodNode(char *namespaceUri, EdgeNodeItem *item, EdgeMethod *method);
__attribute__((visibility("default"))) EdgeResult addReference(EdgeReference *reference);
__attribute__((visibility("default"))) EdgeResult readNode(EdgeMessage *msg);
__attribute__((visibility("default"))) EdgeResult writeNode(EdgeMessage *msg);
__attribute__((visibility("default"))) EdgeResult browseNode(EdgeMessage *msg);
__attribute__((visibility("default"))) EdgeResult browseNext(EdgeMessage *msg);
__attribute__((visibility("default"))) EdgeResult callMethod(EdgeMessage *msg);
__attribute__((visibility("default"))) EdgeResult handleSubscription(EdgeMessage *msg);




#ifdef __cplusplus
}
#endif



#endif  // EDGE_OPCUA_MANAGER_H
