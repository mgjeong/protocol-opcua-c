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
 * @file edge_opcua_server.h
 *
 * @brief This file contains the definition, types and APIs for server requests
 */

#ifndef EDGE_OPCUA_SERVER_H
#define EDGE_OPCUA_SERVER_H

#include "opcua_common.h"
#include "command_adapter.h"

#ifdef __cplusplus
extern "C"
{
#endif

/**
 * @brief Send the request to server for creating namespace
 * @param[in]  namespaceUri Namespace Uri.
 * @param[in]  rootNodeIdentifier Root Node identifier
 * @param[in]  rootNodeBrowseName Root Node browse name.
 * @param[in]  rootNodeDisplayName Root Node display name.
 * @return @c EdgeResult code is 0 on success, otherwise an error value
 * @retval #STATUS_OK Successful
 * @retval #STATUS_PARAM_INVALID Invalid parameter
 * @retval #STATUS_ERROR Operation failed
 */
EdgeResult createNamespaceInServer(const char *namespaceUri, const char *rootNodeIdentifier,
		    const char *rootNodeBrowseName, const char *rootNodeDisplayName);

/**
 * @brief Send the request to create/add node
 * @param[in]  namespaceUri Namespace Uri to add new node
 * @param[in]  item Node item information
 * @return @c EdgeResult code is 0 on success, otherwise an error value
 * @retval #STATUS_OK Successful
 * @retval #STATUS_PARAM_INVALID Invalid parameter
 * @retval #STATUS_ERROR Operation failed
 */
EdgeResult addNodesInServer(const char *namespaceUri, EdgeNodeItem *item);

/**
 * @brief Send the request for modify node
 * @param[in]  namespaceUri Namespace Uri to add new node
 * @param[in]  nodeUri Node Uri
 * @param[in]  value New data
 * @return @c EdgeResult code is 0 on success, otherwise an error value
 * @retval #STATUS_OK Successful
 * @retval #STATUS_PARAM_INVALID Invalid parameter
 * @retval #STATUS_ERROR Operation failed
 */
EdgeResult modifyNodeInServer(const char *namespaceUri, const char *nodeUri, EdgeVersatility *value);

/**
 * @brief Send the request for adding reference
 * @param[in]  reference Node reference information
 * @return @c EdgeResult code is 0 on success, otherwise an error value
 * @retval #STATUS_OK Successful
 * @retval #STATUS_PARAM_INVALID Invalid parameter
 * @retval #STATUS_ERROR Operation failed
 */
EdgeResult addReferenceInServer(EdgeReference *reference);

/**
 * @brief Send the request to create/add method node
 * @param[in]  namespaceUri Namespace Uri to add new node
 * @param[in]  item Node item information
 * @param[in]  method Method and argument information
 * @return @c EdgeResult code is 0 on success, otherwise an error value
 * @retval #STATUS_OK Successful
 * @retval #STATUS_PARAM_INVALID Invalid parameter
 * @retval #STATUS_ERROR Operation failed
 */
EdgeResult addMethodNodeInServer(const char *namespaceUri, EdgeNodeItem *item, EdgeMethod *method);

/**
 * @brief Send the request to create and start the server
 * @param[in]  epInfo Endpoint information
 * @return @c EdgeResult code is 0 on success, otherwise an error value
 * @retval #STATUS_OK Successful
 * @retval #STATUS_PARAM_INVALID Invalid parameter
 * @retval #STATUS_ERROR Operation failed
 */
EdgeResult start_server(EdgeEndPointInfo *epInfo);

/**
 * @brief Send the request to stop the server
 * @param[in]  epInfo Endpoint information
 */
void stop_server(EdgeEndPointInfo *epInfo);

/**
 * @brief Creates and Initialises the node with default values
 * @param[in]  name Node browse name
 * @param[in]  nodeType Type of node (Variable/Array/Object etc.)
 * @param[in]  sourceNodeId Source Node Id information
 * @return EdgeNodeItem created on success, otherwise return NULL in case of error
 */
EdgeNodeItem* createNodeItemImpl(const char* name, EdgeIdentifier nodeType, EdgeNodeId *sourceNodeId);

/**
 * @brief Creates and Initialises the variable node with default values
 * @param[in]  name Node browse name
 * @param[in]  type data type of node
 * @param[in]  data Data item of the node
 * @param[in]  nodeType Type of node (Variable/Array etc.)
 * @return EdgeNodeItem created on success, otherwise return NULL in case of error
 */
EdgeNodeItem* createVariableNodeItemImpl(const char* name, int type, void* data,
        EdgeIdentifier nodeType, double minimumInterval);

/**
 * @brief Deinitialised and deallocates the node item
 * @param[in]  item Node item to be deleted
 * @return @c EdgeResult code is 0 on success, otherwise an error value
 * @retval #STATUS_OK Successful
 * @retval #STATUS_PARAM_INVALID Invalid parameter
 * @retval #STATUS_ERROR Operation failed
 */
EdgeResult deleteNodeItemImpl(EdgeNodeItem* item);

/**
 * @brief Print node list
 * @param[in]  reference Source and Target node information to create reference
 */
void printNodeListInServer();

/**
 * @brief Register Server Callback
 * @param[in]  statusCallback server status callback
 */
void registerServerCallback(status_cb_t statusCallback);

#ifdef __cplusplus
}
#endif

#endif
