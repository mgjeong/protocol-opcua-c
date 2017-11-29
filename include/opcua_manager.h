#ifndef EDGE_OPCUA_MANAGER_H
#define EDGE_OPCUA_MANAGER_H

#include "opcua_common.h"

#ifdef __cplusplus
extern "C" {
#endif

typedef struct EdgeConfigure EdgeConfigure;
typedef struct EdgeResult EdgeResult;
typedef struct EdgeMessage EdgeMessage;

/* Recevied Message callbacks */
typedef void (*response_msg_cb_t) (void* data);
typedef void (*monitored_msg_cb_t) (void* data);
typedef void (*error_msg_cb_t) (void* data);
typedef void (*browse_msg_cb_t) ();

/* status callbacks */
typedef void (*status_start_cb_t) (void* data);
typedef void (*status_stop_cb_t) (void* data);
typedef void (*status_network_cb_t) (void* data);

/* discovery callback */
typedef void (*endpoint_found_cb_t) (void* data);
typedef void (*device_found_cb_t) (void* data);


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

void configure(EdgeConfigure *config);
EdgeResult* send(EdgeMessage* msg);
void onSendMessage(EdgeMessage* msg);

#ifdef __cplusplus
}
#endif



#endif  // EDGE_OPCUA_MANAGER_H
