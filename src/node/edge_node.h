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
 * @file edge_node.h
 *
 * @brief This file contains the definition, types and APIs for manage nodes in server.
 */

#ifndef EDGE_NODE_H
#define EDGE_NODE_H

#include "opcua_common.h"

#include <open62541.h>

#ifdef __cplusplus
extern "C"
{
#endif

/**
 * @brief Create/add node in server
 * @param[in]  server Server Handle
 * @param[in]  nsIndex Namespace Index
 * @param[in]  item Node item information
 * @return @c EdgeResult code is 0 on success, otherwise an error value
 * @retval #STATUS_OK Successful
 * @retval #STATUS_PARAM_INVALID Invalid parameter
 * @retval #STATUS_ERROR Operation failed
 */
EdgeResult addNodes(UA_Server *server, uint16_t nsIndex, const EdgeNodeItem *item);

/**
 * @brief Create/add method node in server
 * @param[in]  server Server Handle
 * @param[in]  nsIndex Namespace Index
 * @param[in]  item Node item information
 * @param[in]  method Method and argument data information
 * @return @c EdgeResult code is 0 on success, otherwise an error value
 * @retval #STATUS_OK Successful
 * @retval #STATUS_PARAM_INVALID Invalid parameter
 * @retval #STATUS_ERROR Operation failed
 */
EdgeResult addMethodNode(UA_Server *server, uint16_t nsIndex, const EdgeNodeItem *item, EdgeMethod *method);

/**
 * @brief Modify node in server
 * @param[in]  server Server Handle
 * @param[in]  nsIndex Namespace Index
 * @param[in]  nodeUri Node Uri
 * @param[in]  value New data
 * @return @c EdgeResult code is 0 on success, otherwise an error value
 * @retval #STATUS_OK Successful
 * @retval #STATUS_PARAM_INVALID Invalid parameter
 * @retval #STATUS_ERROR Operation failed
 */
EdgeResult modifyNode(UA_Server *server, uint16_t nsIndex, const char *nodeUri, EdgeVersatility *value);

/**
 * @brief Add node reference in server
 * @param[in]  server Server Handle
 * @param[in]  reference Node reference information
 * @param[in]  src_nsIndex Source Namespace Index
 * @param[in]  target_nsIndex Target Namespace Index
 * @return @c EdgeResult code is 0 on success, otherwise an error value
 * @retval #STATUS_OK Successful
 * @retval #STATUS_PARAM_INVALID Invalid parameter
 * @retval #STATUS_ERROR Operation failed
 */
EdgeResult addReferences(UA_Server *server, EdgeReference *reference, uint16_t src_nsIndex, uint16_t target_nsIndex);

#ifdef __cplusplus
}
#endif

#endif
