/******************************************************************
 *
 * Copyright 2017 Samsung Electronics All Rights Reserved.
 *
 *
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *      http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 *
 ******************************************************************/

#ifndef EDGE_OPCUA_MANAGER_H
#define EDGE_OPCUA_MANAGER_H

#include <stdio.h>
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
typedef struct EdgeMethodRequestParams EdgeMethodRequestParams;
typedef struct EdgeContinuationPoint EdgeContinuationPoint;
typedef struct EdgeContinuationPointList EdgeContinuationPointList;
typedef struct EdgeRequest EdgeRequest;
typedef struct EdgeEndpointConfig EdgeEndpointConfig;
typedef struct EdgeApplicationConfig EdgeApplicationConfig;

/* Recevied Message callbacks */
typedef void (*response_msg_cb_t) (EdgeMessage *data);
typedef void (*monitored_msg_cb_t) (EdgeMessage *data);
typedef void (*error_msg_cb_t) (EdgeMessage *data);
typedef void (*browse_msg_cb_t) (EdgeMessage *data);

/* status callbacks */
typedef void (*status_start_cb_t) (EdgeEndPointInfo *epInfo, EdgeStatusCode status);
typedef void (*status_stop_cb_t) (EdgeEndPointInfo *epInfo, EdgeStatusCode status);
typedef void (*status_network_cb_t) (EdgeEndPointInfo *epInfo, EdgeStatusCode status);

/* discovery callback */
typedef void (*endpoint_found_cb_t) (EdgeDevice *device);
typedef void (*device_found_cb_t) (EdgeDevice *device);


typedef struct ReceivedMessageCallback
{
    response_msg_cb_t resp_msg_cb;
    monitored_msg_cb_t monitored_msg_cb;
    error_msg_cb_t error_msg_cb;
    browse_msg_cb_t browse_msg_cb;
} ReceivedMessageCallback;

typedef struct StatusCallback
{
    status_start_cb_t start_cb;
    status_stop_cb_t stop_cb;
    status_network_cb_t network_cb;
} StatusCallback;

typedef struct DiscoveryCallback
{
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
void onStatusCallback(EdgeEndPointInfo *epInfo, EdgeStatusCode status);
void onDiscoveryCallback(EdgeDevice *device);


// Server
__attribute__((visibility("default"))) void createServer(EdgeEndPointInfo *epInfo);
__attribute__((visibility("default"))) void closeServer(EdgeEndPointInfo *epInfo);

// Client
/* To get a list of all registered servers at the given server. Application has to free the memory
allocated for the resultant array of EdgeApplicationConfig objects and its members. */
__attribute__((visibility("default"))) EdgeResult findServers(const char *endpointUri, size_t serverUrisSize,
        unsigned char **serverUris, size_t localeIdsSize, unsigned char **localeIds,
        size_t *registeredServersSize, EdgeApplicationConfig **registeredServers);
__attribute__((visibility("default"))) EdgeResult getEndpointInfo(EdgeEndPointInfo *epInfo);
__attribute__((visibility("default"))) void connectClient(EdgeEndPointInfo *epInfo);
__attribute__((visibility("default"))) void disconnectClient(EdgeEndPointInfo *epInfo);

__attribute__((visibility("default"))) void configure(EdgeConfigure *config);
//__attribute__((visibility("default"))) EdgeResult* send(EdgeMessage* msg);
__attribute__((visibility("default"))) EdgeResult createNamespace(char *name, char *rootNodeId,
        char *rootBrowseName, char *rootDisplayName);
__attribute__((visibility("default"))) EdgeResult createNode(char *namespaceUri,
        EdgeNodeItem *item);
__attribute__((visibility("default"))) EdgeNodeItem* createNodeItem(char* name, EdgeIdentifier nodeType,
        EdgeNodeId *sourceNodeId);
__attribute__((visibility("default"))) EdgeNodeItem* createVariableNodeItem(char* name,
        EdgeNodeIdentifier type, void* data, EdgeIdentifier nodeType);
__attribute__((visibility("default"))) EdgeResult deleteNodeItem(EdgeNodeItem* item);

__attribute__((visibility("default"))) EdgeResult modifyVariableNode(char *namespaceUri,
        char *nodeUri, EdgeVersatility *value);
__attribute__((visibility("default"))) EdgeResult createMethodNode(char *namespaceUri,
        EdgeNodeItem *item, EdgeMethod *method);
__attribute__((visibility("default"))) EdgeResult addReference(EdgeReference *reference);
__attribute__((visibility("default"))) EdgeResult readNode(EdgeMessage *msg);
__attribute__((visibility("default"))) EdgeResult writeNode(EdgeMessage *msg);
__attribute__((visibility("default"))) EdgeResult browseNode(EdgeMessage *msg);
__attribute__((visibility("default"))) EdgeResult browseViews(EdgeMessage *msg);
__attribute__((visibility("default"))) EdgeResult browseNext(EdgeMessage *msg);
__attribute__((visibility("default"))) EdgeResult callMethod(EdgeMessage *msg);
__attribute__((visibility("default"))) EdgeResult handleSubscription(EdgeMessage *msg);

__attribute__((visibility("default"))) void showNodeList(void);

// Common

/* Deallocates the dynamic memory for EdgeResult.
Behaviour is undefined if EdgeResult is not dynamically allocated. */
__attribute__((visibility("default"))) void destroyEdgeResult(EdgeResult *res);

/* Deallocates the dynamic memory for EdgeVersatility.
Behaviour is undefined if EdgeVersatility or any of its members not dynamically allocated. */
__attribute__((visibility("default"))) void destroyEdgeVersatility(EdgeVersatility *versatileValue);

/* Deallocates the dynamic memory for EdgeArgument.
Behaviour is undefined if EdgeArgument or any of its members are not dynamically allocated. */
__attribute__((visibility("default"))) void destroyEdgeArgument(EdgeArgument *res);

/* Deallocates the dynamic memory for EdgeMethodRequestParams.
Behaviour is undefined if EdgeMethodRequestParams or any of its members are not dynamically allocated. */
__attribute__((visibility("default"))) void destroyEdgeMethodRequestParams(EdgeMethodRequestParams *res);

/* Deallocates the dynamic memory for EdgeNodeId.
Behaviour is undefined if EdgeNodeId is not dynamically allocated. */
__attribute__((visibility("default"))) void destroyEdgeNodeId(EdgeNodeId *nodeId);

/* Deallocates the dynamic memory for EdgeEndpointConfig.
Behaviour is undefined if EdgeEndpointConfig or any of its members are not dynamically allocated. */
__attribute__((visibility("default"))) void destroyEdgeEndpointConfig(EdgeEndpointConfig *epConfig);

/* Deallocates the dynamic memory for EdgeNodeInfo.
Behaviour is undefined if EdgeNodeInfo is not dynamically allocated. */
__attribute__((visibility("default"))) void destroyEdgeNodeInfo(EdgeNodeInfo *nodeInfo);

/* Deallocates the dynamic memory for EdgeEndpointInfo and its members.
Behaviour is undefined if EdgeEndpointInfo or any of its members are not dynamically allocated. */
__attribute__((visibility("default"))) void destroyEdgeEndpointInfo(EdgeEndPointInfo *endpointInfo);

/* Deallocates the dynamic memory for EdgeApplicationConfig's members.
Behaviour is undefined if any its members are not dynamically allocated. */
__attribute__((visibility("default"))) void destroyEdgeApplicationConfigMembers(EdgeApplicationConfig *config);

/* Deallocates the dynamic memory for EdgeContinuationPoint and its members.
Behaviour is undefined if EdgeContinuationPoint or any of its members are not dynamically allocated. */
__attribute__((visibility("default"))) void destroyEdgeContinuationPoint(EdgeContinuationPoint *cp);

/* Deallocates the dynamic memory for EdgeContinuationPointList and its members.
EdgeContinuationPointList has an array of dynamically allocated EdgeContinuationPoint objects.
Array is also dynamically allocated.
Behaviour is undefined if EdgeContinuationPointList or any of its members are not dynamically allocated. */
__attribute__((visibility("default"))) void destroyEdgeContinuationPointList(EdgeContinuationPointList *cpList);

/* Deallocates the dynamic memory for EdgeResponse and its members.
Behaviour is undefined if EdgeResponse or any of its members are not dynamically allocated. */
__attribute__((visibility("default"))) void destroyEdgeResponse(EdgeResponse *resp);

/* Deallocates the dynamic memory for EdgeRequest and its members.
Behaviour is undefined if EdgeRequest or any of its members are not dynamically allocated. */
__attribute__((visibility("default"))) void destroyEdgeRequest(EdgeRequest *req);

/* Deallocates the dynamic memory for EdgeMessage and its members.
Behaviour is undefined if EdgeMessage or any of its members are not dynamically allocated. */
__attribute__((visibility("default"))) void destroyEdgeMessage(EdgeMessage *msg);

/* Allocates memory and copies the string. Application has to free the memory. */
__attribute__((visibility("default"))) char *copyString(const char *str);

/* Create EdgeNodeInfo object */
__attribute__((visibility("default"))) EdgeNodeInfo* createEdgeNodeInfo(const char* nodeName);
__attribute__((visibility("default"))) EdgeNodeInfo* createEdgeNodeInfoForNodeId(EdgeNodeIdType type,
        int nodeId, uint16_t nameSpace);
__attribute__((visibility("default"))) EdgeResult insertSubParameter(EdgeMessage **msg, const char* nodeName,
        EdgeNodeIdentifier subType, double samplingInterval, double publishingInterval, int maxKeepAliveCount,
        int lifetimeCount, int maxNotificationsPerPublish, bool publishingEnabled, int priority,
        uint32_t queueSize);
__attribute__((visibility("default"))) EdgeMessage* createEdgeSubMessage(const char *endpointUri, size_t requestSize,
        EdgeNodeIdentifier subType);

#ifdef __cplusplus
}
#endif



#endif  // EDGE_OPCUA_MANAGER_H
