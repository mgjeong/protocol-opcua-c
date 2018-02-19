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

/**
 * @file opcua_manager.h
 *
 * This file contains the top level functions for OPC UA adapter
 */

#ifndef EDGE_OPCUA_MANAGER_H
#define EDGE_OPCUA_MANAGER_H

#include <stdio.h>
#include "opcua_common.h"
#include "opcua_interface.h"

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
typedef struct EdgeBrowseParameter EdgeBrowseParameter;

void onSendMessage(EdgeMessage* msg);
void onResponseMessage(EdgeMessage *msg);
void onStatusCallback(EdgeEndPointInfo *epInfo, EdgeStatusCode status);
void onDiscoveryCallback(EdgeDevice *device);

/**
 * @brief Function for creating the server
 * @param[in]  epInfo End point information for server.
 * @return @c EdgeResult code is 0 on success, otherwise an error value
 * @retval #STATUS_OK Successful
 * @retval #STATUS_PARAM_INVALID Invalid parameter
 * @retval #STATUS_ALREADY_INIT Server already initialized
 */
__attribute__((visibility("default"))) EdgeResult createServer(EdgeEndPointInfo *epInfo);

/**
 * @brief Function for closing the server connection
 * @param[in]  epInfo End point information for server.
 */
__attribute__((visibility("default"))) void closeServer(EdgeEndPointInfo *epInfo);

/**
 * @brief Gets a list of all registered servers at the given server. Application has to free the memory \n
                   allocated for the resultant array of EdgeApplicationConfig objects and its members.
 * @param[in]  endpointUri Endpoint Uri.
 * @param[in]  serverUrisSize Optional filter for specific server uris
 * @param[in]  serverUris Optional filter for specific server uris
 * @param[in]  localeIdsSize Optional indication which locale you prefer
 * @param[in]  localeIds Optional indication which locale you prefer
 * @return @c EdgeResult code is 0 on success, otherwise an error value
 * @retval #STATUS_OK Successful
 * @retval #STATUS_PARAM_INVALID Invalid parameter
 * @retval #STATUS_ERROR Operation failed
 */
__attribute__((visibility("default"))) EdgeResult findServers(const char *endpointUri, size_t serverUrisSize,
        unsigned char **serverUris, size_t localeIdsSize, unsigned char **localeIds,
        size_t *registeredServersSize, EdgeApplicationConfig **registeredServers);

/**
 * @brief Gets a list of endpoints of a server
 * @param[in]  EdgeMessage EdgeMessage containing the endpoint information.
 * @return @c EdgeResult code is 0 on success, otherwise an error value
 * @retval #STATUS_OK Successful
 * @retval #STATUS_PARAM_INVALID Invalid parameter
 * @retval #STATUS_ERROR Operation failed
 */
__attribute__((visibility("default"))) EdgeResult getEndpointInfo(EdgeMessage *msg);

/**
 * @brief Disconnect the client connection
 * @param[in]  epInfo End point information for server.
 */
__attribute__((visibility("default"))) void disconnectClient(EdgeEndPointInfo *epInfo);

/**
 * @brief Set either server/client configuration
 * @param[in]  config Configuration information.
 */
__attribute__((visibility("default"))) void configure(EdgeConfigure *config);

/**
 * @brief Add a new namespace to the server.
 * @param[in]  name Namespace name/URI
 * @param[in]  rootNodeId Root Node identifier
 * @param[in]  rootBrowseName Root Node Browse Name
 * @param[in]  rootDisplayName Root Node Display Name
 * @return @c EdgeResult code is 0 on success, otherwise an error value
 * @retval #STATUS_OK Successful
 * @retval #STATUS_PARAM_INVALID Invalid parameter
 * @retval #STATUS_ERROR Operation failed
 */
__attribute__((visibility("default"))) EdgeResult createNamespace(char *name, char *rootNodeId,
        char *rootBrowseName, char *rootDisplayName);

/**
 * @brief Add the node in the server.
 * @param[in]  namespaceUri Namespace URI
 * @param[in]  item Node information like browse name, display name, access level etc.
 * @return @c EdgeResult code is 0 on success, otherwise an error value
 * @retval #STATUS_OK Successful
 * @retval #STATUS_PARAM_INVALID Invalid parameter
 * @retval #STATUS_ERROR Operation failed
 */
__attribute__((visibility("default"))) EdgeResult createNode(char *namespaceUri,
        EdgeNodeItem *item);

/**
 * @brief Create a node item
 * @param[in]  name Browse name
 * @param[in]  nodeType Type of node (OBJECT/REFERENCE TYPE etc.)
 * @param[in]  sourceNodeId Source/Parent Node information
 * @return @c EdgeResult code is 0 on success, otherwise an error value
 * @retval #STATUS_OK Successful
 * @retval #STATUS_PARAM_INVALID Invalid parameter
 * @retval #STATUS_ERROR Operation failed
 */
__attribute__((visibility("default"))) EdgeNodeItem* createNodeItem(char* name, EdgeIdentifier nodeType,
        EdgeNodeId *sourceNodeId);

/**
 * @brief Create a Variable/Array node
 * @param[in]  name Browse name
 * @param[in]  type Node identifier
 * @param[in]  data Node value
 * @param[in]  nodeType Type of node (VARIABLE/ARRAY)
 * @return @c EdgeResult code is 0 on success, otherwise an error value
 * @retval #STATUS_OK Successful
 * @retval #STATUS_PARAM_INVALID Invalid parameter
 * @retval #STATUS_ERROR Operation failed
 */
__attribute__((visibility("default"))) EdgeNodeItem* createVariableNodeItem(char* name,
        EdgeNodeIdentifier type, void* data, EdgeIdentifier nodeType);

/**
 * @brief Delete a node item
 * @param[in]  item Node information like browse name etc.
 * @return @c EdgeResult code is 0 on success, otherwise an error value
 * @retval #STATUS_OK Successful
 * @retval #STATUS_PARAM_INVALID Invalid parameter
 * @retval #STATUS_ERROR Operation failed
 */
__attribute__((visibility("default"))) EdgeResult deleteNodeItem(EdgeNodeItem* item);

/**
 * @brief Modify a Variable/Array node
 * @param[in]  namespaceUri Namespace uri
 * @param[in]  nodeUri Node browse name
 * @param[in]  value new value to write
 * @return @c EdgeResult code is 0 on success, otherwise an error value
 * @retval #STATUS_OK Successful
 * @retval #STATUS_PARAM_INVALID Invalid parameter
 * @retval #STATUS_ERROR Operation failed
 */
__attribute__((visibility("default"))) EdgeResult modifyVariableNode(char *namespaceUri,
        char *nodeUri, EdgeVersatility *value);

/**
 * @brief Create a Method node
 * @param[in]  namespaceUri Namespace uri
 * @param[in]  item Node information like browse name etc.
 * @param[in]  method Input and Output arguments
 * @return @c EdgeResult code is 0 on success, otherwise an error value
 * @retval #STATUS_OK Successful
 * @retval #STATUS_PARAM_INVALID Invalid parameter
 * @retval #STATUS_ERROR Operation failed
 */
__attribute__((visibility("default"))) EdgeResult createMethodNode(char *namespaceUri,
        EdgeNodeItem *item, EdgeMethod *method);

/**
 * @brief Create node references
 * @param[in]  reference Source and Target node information to create reference
 * @return @c EdgeResult code is 0 on success, otherwise an error value
 * @retval #STATUS_OK Successful
 * @retval #STATUS_PARAM_INVALID Invalid parameter
 * @retval #STATUS_ERROR Operation failed
 */
__attribute__((visibility("default"))) EdgeResult addReference(EdgeReference *reference);

/**
 * @brief Show list of nodes
 */
__attribute__((visibility("default"))) void showNodeList(void);

/**
 * @brief Send the EdgeMessage request to queue for processing
 * @param[in]  msg EdgeMessage request data
 * @return @c EdgeResult code is 0 on success, otherwise an error value
 * @retval #STATUS_OK Successful
 * @retval #STATUS_PARAM_INVALID Invalid parameter
 * @retval #STATUS_ERROR Operation failed
 */
__attribute__((visibility("default"))) EdgeResult sendRequest(EdgeMessage* msg);

/**
 * @brief Deallocates the dynamic memory for EdgeResult. \n
                  Behaviour is undefined if EdgeResult is not dynamically allocated.
 * @param[in]  res EdgeResult data
 */
__attribute__((visibility("default"))) void destroyEdgeResult(EdgeResult *res);

/**
 * @brief Deallocates the dynamic memory for EdgeVersatility. \n
                   Behaviour is undefined if EdgeVersatility is not dynamically allocated.
 * @param[in]  versatileValue EdgeVersatility data
 */
__attribute__((visibility("default"))) void destroyEdgeVersatility(EdgeVersatility *versatileValue);

/**
 * @brief Deallocates the dynamic memory for EdgeArgument. \n
                   Behaviour is undefined if EdgeArgument is not dynamically allocated.
 * @param[in]  res EdgeArgument data
 */
__attribute__((visibility("default"))) void destroyEdgeArgument(EdgeArgument *res);

/**
 * @brief Deallocates the dynamic memory for EdgeMethodRequestParams. \n
                   Behaviour is undefined if EdgeMethodRequestParams is not dynamically allocated.
 * @param[in]  res EdgeMethodRequestParams data
 */
__attribute__((visibility("default"))) void destroyEdgeMethodRequestParams(EdgeMethodRequestParams *res);

/**
 * @brief Deallocates the dynamic memory for EdgeNodeId. \n
                   Behaviour is undefined if EdgeNodeId is not dynamically allocated.
 * @param[in]  nodeId EdgeNodeId data
 */
__attribute__((visibility("default"))) void destroyEdgeNodeId(EdgeNodeId *nodeId);

/**
 * @brief Deallocates the dynamic memory for EdgeEndpointConfig. \n
                   Behaviour is undefined if EdgeEndpointConfig is not dynamically allocated.
 * @param[in]  epConfig EdgeEndpointConfig data
 */
__attribute__((visibility("default"))) void destroyEdgeEndpointConfig(EdgeEndpointConfig *epConfig);

/**
 * @brief Deallocates the dynamic memory for EdgeNodeInfo. \n
                   Behaviour is undefined if EdgeNodeInfo is not dynamically allocated.
 * @param[in]  nodeInfo EdgeNodeInfo data
 */
__attribute__((visibility("default"))) void destroyEdgeNodeInfo(EdgeNodeInfo *nodeInfo);

/**
 * @brief Deallocates the dynamic memory for EdgeEndPointInfo. \n
                   Behaviour is undefined if EdgeEndPointInfo is not dynamically allocated.
 * @param[in]  endpointInfo EdgeEndPointInfo data
 */
__attribute__((visibility("default"))) void destroyEdgeEndpointInfo(EdgeEndPointInfo *endpointInfo);

/**
 * @brief Deallocates the dynamic memory for EdgeApplicationConfig. \n
                   Behaviour is undefined if EdgeApplicationConfig is not dynamically allocated.
 * @param[in]  config EdgeApplicationConfig data
 */
__attribute__((visibility("default"))) void destroyEdgeApplicationConfigMembers(EdgeApplicationConfig *config);

/**
 * @brief Deallocates the dynamic memory for EdgeContinuationPoint. \n
 *                Behaviour is undefined if EdgeContinuationPoint is not dynamically allocated.
 * @param[in]  cp EdgeContinuationPoint data
 */
__attribute__((visibility("default"))) void destroyEdgeContinuationPoint(EdgeContinuationPoint *cp);

/**
 * @brief Deallocates the dynamic memory for EdgeContinuationPointList and its members. \n
 *                EdgeContinuationPointList has an array of dynamically allocated EdgeContinuationPoint objects. \n
 *                Array is also dynamically allocated. \n
 *                Behaviour is undefined if EdgeContinuationPoint is not dynamically allocated.
 * @param[in]  cpList EdgeContinuationPointList data
 */
__attribute__((visibility("default"))) void destroyEdgeContinuationPointList(EdgeContinuationPointList *cpList);

/**
 * @brief Deallocates the dynamic memory for EdgeResponse. \n
 *                 Behaviour is undefined if EdgeResponse is not dynamically allocated.
 * @param[in]  resp EdgeResponse data
 */
__attribute__((visibility("default"))) void destroyEdgeResponse(EdgeResponse *resp);

/**
 * @brief Deallocates the dynamic memory for EdgeRequest. \n
 *                Behaviour is undefined if EdgeRequest is not dynamically allocated.
 * @param[in]  req EdgeRequest data
 */
__attribute__((visibility("default"))) void destroyEdgeRequest(EdgeRequest *req);

/**
 * @brief Deallocates the dynamic memory for EdgeMessage. \n
                   Behaviour is undefined if EdgeMessage is not dynamically allocated.
 * @param[in]  msg EdgeMessage data
 */
__attribute__((visibility("default"))) void destroyEdgeMessage(EdgeMessage *msg);

/* Allocates memory and copies the string. Application has to free the memory. */
/**
 * @brief Allocates memory and copies the string. Application has to free the memory
 * @param[in]  str String to be copied
 * @return @c Copied string value,
 *                          Otherwise NULL if str is not valid
 */
__attribute__((visibility("default"))) char *copyString(const char *str);

/**
 * @brief Create EdgeNodeInfo object
 * @param[in]  nodeName Node name
 * @return EdgeNodeInfo object on success
 *                  NULL in case of error
 */
__attribute__((visibility("default"))) EdgeNodeInfo* createEdgeNodeInfo(const char* nodeName);

/**
 * @brief Create EdgeNodeInfo object
 * @param[in]  type Node type
 * @param[in]  nodeId Node Id
 * @param[in]   nameSpace Namespace index
 * @return EdgeNodeInfo object on success
 *                   NULL in case of error
 */
__attribute__((visibility("default"))) EdgeNodeInfo* createEdgeNodeInfoForNodeId(EdgeNodeIdType type,
        int nodeId, uint16_t nameSpace);

/**
 * @brief Insert Monitored Item to the EdgeMessage request
 * @param[in]  nodeName Node name
 * @param[in]  subType Node identifier
 * @param[in]  samplingInterval Sampling interval
 * @param[in]  publishingInterval Publish interval
 * @param[in]  maxKeepAliveCount Max keepAlive count
 * @param[in]  lifetimeCount Life time count
 * @param[in]  maxNotificationsPerPublish Max Notification Per Publish
 * @param[in]  publishingEnabled Enable or Disable publishing
 * @param[in]  priority Priority
 * @param[in]  queueSize Queue size
 * @return @c EdgeResult code is 0 on success, otherwise an error value
 * @retval #STATUS_OK Successful
 * @retval #STATUS_PARAM_INVALID Invalid parameter
 * @retval #STATUS_ERROR Operation failed
 */
__attribute__((visibility("default"))) EdgeResult insertSubParameter(EdgeMessage **msg, const char* nodeName,
        EdgeNodeIdentifier subType, double samplingInterval, double publishingInterval, int maxKeepAliveCount,
        int lifetimeCount, int maxNotificationsPerPublish, bool publishingEnabled, int priority,
        uint32_t queueSize);

/**
 * @brief Create EdgeMessage for Subscription Services
 * @param[in]  endpointUri Endpoint Uri
 * @param[in]  nodeName Node name
 * @param[in]  requestSize request size
 * @param[in]  subType Node identifier type
 * @return EdgeMessage object on success
 *                  NULL in case of error
 */
__attribute__((visibility("default"))) EdgeMessage* createEdgeSubMessage(const char *endpointUri,
        const char* nodeName, size_t requestSize, EdgeNodeIdentifier subType);

/**
 * @brief Create EdgeMessage for Attribute Services such as Read, Write
 * @param[in]  endpointUri Endpoint Uri
 * @param[in]  requestSize request size
 * @param[in]  cmd command type
 * @return EdgeMessage object on success
 *                  NULL in case of error
 */
__attribute__((visibility("default"))) EdgeMessage* createEdgeAttributeMessage(const char *endpointUri,
        size_t requestSize, EdgeCommand cmd);

/**
 * @brief Create EdgeMessage
 * @param[in]  endpointUri Endpoint Uri
 * @param[in]  requestSize request size
 * @param[in]  cmd command type
 * @return EdgeMessage object on success
 *                  NULL in case of error
 */
__attribute__((visibility("default"))) EdgeMessage* createEdgeMessage(const char *endpointUri, size_t requestSize, EdgeCommand cmd);

/**
 * @brief Insert Read Access to the EdgeMessage request data
 * @param[in]  msg EdgeMessage request
 * @param[in]  nodeName Node name
 * @param[out]  msg EdgeMessage request
 * @return @c EdgeResult code is 0 on success, otherwise an error value
 * @retval #STATUS_OK Successful
 * @retval #STATUS_PARAM_INVALID Invalid parameter
 * @retval #STATUS_ERROR Operation failed
 */
__attribute__((visibility("default"))) EdgeResult insertReadAccessNode(EdgeMessage **msg, const char* nodeName);

/**
 * @brief Insert Write Access to the EdgeMessage request data
 * @param[in]  msg EdgeMessage request
 * @param[in]  nodeName Node name
 * @param[out]  msg EdgeMessage request
 * @return @c EdgeResult code is 0 on success, otherwise an error value
 * @retval #STATUS_OK Successful
 * @retval #STATUS_PARAM_INVALID Invalid parameter
 * @retval #STATUS_ERROR Operation failed
 */
__attribute__((visibility("default"))) EdgeResult insertWriteAccessNode(EdgeMessage **msg, const char* nodeName,
        void* value, size_t valueLen);

/**
 * @brief Insert method parameters to the EdgeMessage request
 * @param[in]  msg EdgeMessage Request
 * @param[in]  nodeName Node name
 * @param[in]  inputParameterSize Number of input arguments
 * @param[in]  argType parameter data type
 * @param[in]  valType Scalar or Array data
 * @param[in]  scalarValue Scalar value
 * @param[in]  arrayData Array data
 * @param[in]  arrayLength Array Length
 * @param[out]  msg EdgeMessage Request
 * @return @c EdgeResult code is 0 on success, otherwise an error value
 * @retval #STATUS_OK Successful
 * @retval #STATUS_PARAM_INVALID Invalid parameter
 * @retval #STATUS_ERROR Operation failed
 */
__attribute__((visibility("default"))) EdgeResult insertEdgeMethodParameter(EdgeMessage **msg, const char* nodeName,
        size_t inputParameterSize, EdgeNodeIdentifier argType, EdgeArgValType valType,
        void *scalarValue, void *arrayData, size_t arrayLength);

/**
 * @brief Insert browse parameter to the EdgeMessage request
 * @param[in]  msg EdgeMessage Request
 * @param[in]  nodeInfo Node information
 * @param[in]  parameter Browse parameters such as browse direction, max references per node to browse.
 * @param[out]  msg EdgeMessage Request
 * @return @c EdgeResult code is 0 on success, otherwise an error value
 * @retval #STATUS_OK Successful
 * @retval #STATUS_PARAM_INVALID Invalid parameter
 * @retval #STATUS_ERROR Operation failed
 */
__attribute__((visibility("default"))) EdgeResult insertBrowseParameter(EdgeMessage **msg, EdgeNodeInfo* nodeInfo,
        EdgeBrowseParameter parameter);

/* Get the value type of the variable node */
/**
 * @brief Get the value type of the variable node
 * @param[in]  nodeName Node information
 * @return EdgeNodeIdentifier - Node Identifier type
 */
__attribute__((visibility("default"))) EdgeNodeIdentifier getValueType(const char* nodeName);

/**
 * @brief Deallocates the dynamic memory for EdgeBrowseNextData. \n
                   Behaviour is undefined if EdgeBrowseNextData is not dynamically allocated.
 * @param[in]  data EdgeBrowseNextData data used for BrowseNext
 */
__attribute__((visibility("default"))) void destroyBrowseNextData(EdgeBrowseNextData *data);

/**
 * @brief Deallocates the dynamic memory for EdgeBrowseNextData. \n
                   Behaviour is undefined if EdgeBrowseNextData is not dynamically allocated.
 * @param[in]  data EdgeBrowseNextDataElements data used for BrowseNext
 */
__attribute__((visibility("default"))) void destroyBrowseNextDataElements(EdgeBrowseNextData *data);

/**
 * @brief Initialize the BrowseNextData request to be used in BrowseNext
 * @param[in]  data EdgeBrowseNextData data used for BrowseNext
 * @param[in]  browseParam Browse parameter
 * @param[in]  count Countinuation point list count
 * @param[in]  next_free Next Index to copy the ContinuationPoint to.
 * @return Create EdgeBrowseNextData request
 */
__attribute__((visibility("default"))) EdgeBrowseNextData* initBrowseNextData(EdgeBrowseNextData *browseNextData,
        EdgeBrowseParameter *browseParam, size_t count, size_t next_free);

/**
 * @brief Add the BrowseNext data
 * @param[in]  data EdgeBrowseNextData data used for BrowseNext
 * @param[in]   cp CountinuationPoint data
 * @param[in]   nodeId Node id
 * @param[out] data EdgeBrowseNext request
 * @return @c EdgeResult code is 0 on success, otherwise an error value
 * @retval #STATUS_OK Successful
 * @retval #STATUS_PARAM_INVALID Invalid parameter
 * @retval #STATUS_ERROR Operation failed
 */
__attribute__((visibility("default"))) EdgeResult addBrowseNextData(EdgeBrowseNextData **data, EdgeContinuationPoint *cp,
        EdgeNodeId *nodeId);

/**
 * @brief Clone the BrowseNextData request to be used in BrowseNext
 * @param[in]  browseNextData EdgeBrowseNextData data to be cloned
 * @return Cloned EdgeBrowseNextData request,
 *                   NULL in case of any error
 */
__attribute__((visibility("default"))) EdgeBrowseNextData *cloneBrowseNextData(EdgeBrowseNextData* browseNextData);

#ifdef __cplusplus
}
#endif



#endif  // EDGE_OPCUA_MANAGER_H
