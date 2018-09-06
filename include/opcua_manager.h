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
 * @brief This file contains the top level functions for OPC UA adapter
 */

#ifndef EDGE_OPCUA_MANAGER_H
#define EDGE_OPCUA_MANAGER_H

#include <stdio.h>
#include "opcua_common.h"
#include "opcua_interface.h"

#ifdef __cplusplus
extern "C" {
#endif

#ifndef _WIN32
#define EXPORT __attribute__((visibility("default")))
#else
#define EXPORT
#endif

typedef struct EdgeConfigure EdgeConfigure;
typedef struct EdgeResult EdgeResult;
typedef struct EdgeMessage EdgeMessage;
typedef struct EdgeDevice EdgeDevice;
typedef struct EdgeEndPointInfo EdgeEndPointInfo;
typedef struct EdgeReference EdgeReference;
typedef struct EdgeMethodRequestParams EdgeMethodRequestParams;
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
EXPORT EdgeResult createServer(EdgeEndPointInfo *epInfo);

/**
 * @brief Function for closing the server connection
 * @param[in]  epInfo End point information for server.
 */
EXPORT void closeServer(EdgeEndPointInfo *epInfo);

/**
 * @brief Gets a list of all registered servers at the given server. Application has to free the memory \n
                   allocated for the resultant array of EdgeApplicationConfig objects and its members.
 * @param[in]  endpointUri Endpoint Uri.
 * @param[in]  serverUrisSize Optional filter for specific server uris
 * @param[in]  serverUris Optional filter for specific server uris
 * @param[in]  localeIdsSize Optional indication which locale you prefer
 * @param[in]  localeIds Optional indication which locale you prefer
 * @param[out]  registeredServersSize Number of registered Servers matching the filter criteria
 * @param[out]  registeredServers Application configuration information of the servers matching the filter criteria
 * @return @c EdgeResult code is 0 on success, otherwise an error value
 * @retval #STATUS_OK Successful
 * @retval #STATUS_PARAM_INVALID Invalid parameter
 * @retval #STATUS_ERROR Operation failed
 */
EXPORT EdgeResult findServers(const char *endpointUri, size_t serverUrisSize,
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
EXPORT EdgeResult getEndpointInfo(EdgeMessage *msg);

/**
 * @brief Disconnect the client connection
 * @param[in]  epInfo End point information for server.
 */
EXPORT void disconnectClient(EdgeEndPointInfo *epInfo);

/**
 * @brief Set either server/client configuration
 * @param[in]  config Configuration information.
 */
EXPORT void configure(EdgeConfigure *config);

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
EXPORT EdgeResult createNamespace(const char *name, const char *rootNodeId,
	    	const char *rootBrowseName, const char *rootDisplayName);

/**
 * @brief Add the node in the server.
 * @param[in]  namespaceUri Namespace URI
 * @param[in]  item Node information like browse name, display name, access level etc.
 * @return @c EdgeResult code is 0 on success, otherwise an error value
 * @retval #STATUS_OK Successful
 * @retval #STATUS_PARAM_INVALID Invalid parameter
 * @retval #STATUS_ERROR Operation failed
 */
EXPORT EdgeResult createNode(const char *namespaceUri,
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
EXPORT EdgeNodeItem* createNodeItem(const char* name, EdgeIdentifier nodeType,
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
EXPORT EdgeNodeItem* createVariableNodeItem(const char* name, int type,
        void* data, EdgeIdentifier nodeType, double minimumInterval);

/**
 * @brief Delete a node item
 * @param[in]  item Node information like browse name etc.
 * @return @c EdgeResult code is 0 on success, otherwise an error value
 * @retval #STATUS_OK Successful
 * @retval #STATUS_PARAM_INVALID Invalid parameter
 * @retval #STATUS_ERROR Operation failed
 */
EXPORT EdgeResult deleteNodeItem(EdgeNodeItem* item);

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
EXPORT EdgeResult modifyVariableNode(const char *namespaceUri,
		    const char *nodeUri, EdgeVersatility *value);

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
EXPORT EdgeResult createMethodNode(const char *namespaceUri,
        EdgeNodeItem *item, EdgeMethod *method);

/**
 * @brief Create node references
 * @param[in]  reference Source and Target node information to create reference
 * @return @c EdgeResult code is 0 on success, otherwise an error value
 * @retval #STATUS_OK Successful
 * @retval #STATUS_PARAM_INVALID Invalid parameter
 * @retval #STATUS_ERROR Operation failed
 */
EXPORT EdgeResult addReference(EdgeReference *reference);

/**
 * @brief Show list of nodes
 */
EXPORT void showNodeList(void);

/**
 * @brief Send the EdgeMessage request to queue for processing
 * @param[in]  msg EdgeMessage request data
 * @return @c EdgeResult code is 0 on success, otherwise an error value
 * @retval #STATUS_OK Successful
 * @retval #STATUS_PARAM_INVALID Invalid parameter
 * @retval #STATUS_ERROR Operation failed
 */
EXPORT EdgeResult sendRequest(EdgeMessage* msg);

/**
 * @brief Deallocates the dynamic memory for EdgeResult. \n
                  Behaviour is undefined if EdgeResult is not dynamically allocated.
 * @param[in]  res EdgeResult data
 */
EXPORT void destroyEdgeResult(EdgeResult *res);

/**
 * @brief Deallocates the dynamic memory for EdgeVersatility. \n
                   Behaviour is undefined if EdgeVersatility is not dynamically allocated.
 * @param[in]  versatileValue EdgeVersatility data
 */
EXPORT void destroyEdgeVersatility(EdgeVersatility *versatileValue);

/**
 * @brief Deallocates the dynamic memory for EdgeArgument. \n
                   Behaviour is undefined if EdgeArgument is not dynamically allocated.
 * @param[in]  res EdgeArgument data
 */
EXPORT void destroyEdgeArgument(EdgeArgument *res);

/**
 * @brief Deallocates the dynamic memory for EdgeMethodRequestParams. \n
                   Behaviour is undefined if EdgeMethodRequestParams is not dynamically allocated.
 * @param[in]  res EdgeMethodRequestParams data
 */
EXPORT void destroyEdgeMethodRequestParams(EdgeMethodRequestParams *res);

/**
 * @brief Deallocates the dynamic memory for EdgeNodeId. \n
                   Behaviour is undefined if EdgeNodeId is not dynamically allocated.
 * @param[in]  nodeId EdgeNodeId data
 */
EXPORT void destroyEdgeNodeId(EdgeNodeId *nodeId);

/**
 * @brief Deallocates the dynamic memory for EdgeEndpointConfig. \n
                   Behaviour is undefined if EdgeEndpointConfig is not dynamically allocated.
 * @param[in]  epConfig EdgeEndpointConfig data
 */
EXPORT void destroyEdgeEndpointConfig(EdgeEndpointConfig *epConfig);

/**
 * @brief Deallocates the dynamic memory for EdgeNodeInfo. \n
                   Behaviour is undefined if EdgeNodeInfo is not dynamically allocated.
 * @param[in]  nodeInfo EdgeNodeInfo data
 */
EXPORT void destroyEdgeNodeInfo(EdgeNodeInfo *nodeInfo);

/**
 * @brief Deallocates the dynamic memory for EdgeEndPointInfo. \n
                   Behaviour is undefined if EdgeEndPointInfo is not dynamically allocated.
 * @param[in]  endpointInfo EdgeEndPointInfo data
 */
EXPORT void destroyEdgeEndpointInfo(EdgeEndPointInfo *endpointInfo);

/**
 * @brief Deallocates the dynamic memory for EdgeApplicationConfig. \n
                   Behaviour is undefined if EdgeApplicationConfig is not dynamically allocated.
 * @param[in]  config EdgeApplicationConfig data
 */
EXPORT void destroyEdgeApplicationConfigMembers(EdgeApplicationConfig *config);

/**
 * @brief Deallocates the dynamic memory for EdgeResponse. \n
 *                 Behaviour is undefined if EdgeResponse is not dynamically allocated.
 * @param[in]  resp EdgeResponse data
 */
EXPORT void destroyEdgeResponse(EdgeResponse *resp);

/**
 * @brief Deallocates the dynamic memory for EdgeRequest. \n
 *                Behaviour is undefined if EdgeRequest is not dynamically allocated.
 * @param[in]  req EdgeRequest data
 */
EXPORT void destroyEdgeRequest(EdgeRequest *req);

/**
 * @brief Deallocates the dynamic memory for EdgeMessage. \n
                   Behaviour is undefined if EdgeMessage is not dynamically allocated.
 * @param[in]  msg EdgeMessage data
 */
EXPORT void destroyEdgeMessage(EdgeMessage *msg);

/* Allocates memory and copies the string. Application has to free the memory. */
/**
 * @brief Allocates memory and copies the string. Application has to free the memory
 * @param[in]  str String to be copied
 * @return @c Copied string value,
 *                          Otherwise NULL if str is not valid
 */
EXPORT char *copyString(const char *str);

/**
 * @brief Create EdgeNodeInfo object
 * @param[in]  nodeName Node name
 * @return EdgeNodeInfo object on success
 *                  NULL in case of error
 */
EXPORT EdgeNodeInfo* createEdgeNodeInfo(const char* nodeName);

/**
 * @brief Create EdgeNodeInfo object
 * @param[in]  type Node type
 * @param[in]  nodeId Node Id
 * @param[in]   nameSpace Namespace index
 * @return EdgeNodeInfo object on success
 *                   NULL in case of error
 */
EXPORT EdgeNodeInfo* createEdgeNodeInfoForNodeId(EdgeNodeIdType type,
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
EXPORT EdgeResult insertSubParameter(EdgeMessage **msg, const char* nodeName,
        EdgeNodeType subType, double samplingInterval, double publishingInterval, int maxKeepAliveCount,
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
EXPORT EdgeMessage* createEdgeSubMessage(const char *endpointUri,
        const char* nodeName, size_t requestSize, EdgeNodeType subType);

/**
 * @brief Create EdgeMessage for Attribute Services such as Read, Write
 * @param[in]  endpointUri Endpoint Uri
 * @param[in]  requestSize request size
 * @param[in]  cmd command type
 * @return EdgeMessage object on success
 *                  NULL in case of error
 */
EXPORT EdgeMessage* createEdgeAttributeMessage(const char *endpointUri,
        size_t requestSize, EdgeCommand cmd);

/**
 * @brief Create EdgeMessage
 * @param[in]  endpointUri Endpoint Uri
 * @param[in]  requestSize request size
 * @param[in]  cmd command type
 * @return EdgeMessage object on success
 *                  NULL in case of error
 */
EXPORT EdgeMessage* createEdgeMessage(const char *endpointUri, size_t requestSize, EdgeCommand cmd);

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
EXPORT EdgeResult insertReadAccessNode(EdgeMessage **msg, const char* nodeName);

/**
 * @brief Insert Write Access to the EdgeMessage request data
 * @param[in]  msg EdgeMessage request
 * @param[in]  nodeName Node name
 * @param[in]  value value
 * @param[in]  value length valueLen
 * @return @c EdgeResult code is 0 on success, otherwise an error value
 * @retval #STATUS_OK Successful
 * @retval #STATUS_PARAM_INVALID Invalid parameter
 * @retval #STATUS_ERROR Operation failed
 */
EXPORT EdgeResult insertWriteAccessNode(EdgeMessage **msg, const char* nodeName,
        void* value, size_t valueLen);

/**
 * @brief Insert Write Access to the EdgeMessage request data
 * @param[in]  msg EdgeMessage request
 * @param[in]  nodeName Node name
 * @param[in]  value value
 * @param[in]  value length valueLen
 * @param[in]  value type valueType
 * @return @c EdgeResult code is 0 on success, otherwise an error value
 * @retval #STATUS_OK Successful
 * @retval #STATUS_PARAM_INVALID Invalid parameter
 * @retval #STATUS_ERROR Operation failed
 */
EXPORT EdgeResult insertWriteAccessNodeWithValueType(EdgeMessage **msg, const char* nodeName, void* value,
        size_t valueCount, int valueType);

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
EXPORT EdgeResult insertEdgeMethodParameter(EdgeMessage **msg, const char* nodeName,
        size_t inputParameterSize, int argType, EdgeArgValType valType,
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
EXPORT EdgeResult insertBrowseParameter(EdgeMessage **msg, EdgeNodeInfo* nodeInfo,
        EdgeBrowseParameter parameter);

/* Get the value type of the variable node */
/**
 * @brief Get the value type of the variable node
 * @param[in]  nodeName Node information
 * @return Node Identifier type
 */
EXPORT int getValueType(const char* nodeName);

#ifdef __cplusplus
}
#endif

#endif  // EDGE_OPCUA_MANAGER_H
