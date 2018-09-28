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
 * @file edge_utils.h
 * @brief This file contains the utility APIs which involves the direct access of open62541 library.
 */

#ifndef EDGE_OPEN62541_H_
#define EDGE_OPEN62541_H_

#include "opcua_common.h"
#include "edge_utils.h"
#include "open62541.h"

#ifdef __cplusplus
extern "C"
{
#endif

/**
 * @brief Create scalar variant .
 * @param[in]  type data type.
 * @param[in]  data Data.
 * @param[out]  out Output Scalar variant.
 * @return GOOD status on success. Otherwise error status
 */
UA_StatusCode createScalarVariant(int type, void *data, UA_Variant *out);

/**
 * @brief Create Array variant.
 * @param[in]  type data type.
 * @param[in]  data Array Data.
 * @param[in]  len  Array Length
 * @param[out]  out Output Array variant.
 * @return GOOD status on success. Otherwise error status
 */
UA_StatusCode createArrayVariant(int type, void *data, int len, UA_Variant *out);

/**
 * @brief Converts string of type UA_String to char string.
 * @remarks Allocated memory should be freed by the caller.
 * @param[in]  uaStr Data to be converted.
 * @return Converted string on success. Otherwise null.
 */
char *convertUAStringToString(UA_String *uaStr);

/**
 * @brief Converts string of type UA_String to Edge_String.
 * @remarks Allocated memory should be freed by the caller.
 * Converted string is NULL terminated.
 * @param[in]  uaStr Data to be converted.
 * @return Converted string on success. Otherwise null.
 */
Edge_String *convertToEdgeString(UA_String *uaStr);

/**
 * @brief Converts UA_ApplicationType to EdgeApplicationType.
 * @param[in]  appType Application type as in open62541 library.
 * @return Converted application type.
 */
EdgeApplicationType convertToEdgeApplicationType(UA_ApplicationType appType);

/**
 * @brief Converts EdgeApplicationType to UA_ApplicationType.
 * @param[in]  appType Application type.
 * @return Converted application type as in open62541 library.
 */
UA_ApplicationType convertEdgeApplicationType(EdgeApplicationType appType);

/**
 * @brief Converts UA_Guid to string.
 * @param[in]  guid UA_Guid.
 * @param[out]  out output guid string.
 */
void convertGuidToString(UA_Guid guid, char **out);

/**
 * @brief Converts UA_NodeId to Edge_NodeId.
 * @remarks Allocated memory should be freed by the caller.
 * String and ByteString type NodeIds are NULL terminated.
 * @param[in]  nodeId NodeId to be converted.
 * @return Converted NodeId on success. Otherwise null.
 */
Edge_NodeId *convertToEdgeNodeIdType(UA_NodeId *nodeId);

/**
 * @brief Converts UA_NodeId to EdgeNodeId.
 * @remarks Allocated memory should be freed by the caller.
 * String, ByteString & Guid type NodeIds are NULL terminated.
 * @param[in]  nodeId NodeId to be converted.
 * @return Converted EdgeNodeId on success. Otherwise null.
 */
EdgeNodeId *getEdgeNodeId(UA_NodeId *node);

/**
 * @brief De-allocates the memory consumed by EdgeArgument and its members.
 * @remarks Both EdgeArgument and its members should have been allocated dynamically.
 * @param[in]  arg Pointer to EdgeArgument which needs to be freed.
 */
void freeEdgeArgument(EdgeArgument *arg);

/**
 * @brief De-allocates the memory consumed by EdgeVersatility and its members.
 * @remarks Both EdgeVersatility and its members should have been allocated dynamically.
 * @param[in]  versatileValue Pointer to EdgeVersatility which needs to be freed.
 * @param[in]  type Type of the value in EdgeVersatility.
 */
void freeEdgeVersatilityByType(EdgeVersatility *versatileValue, int type);

/**
 * @brief Checks whether the given node class is valid & supported by open62541.
 * @param[in]  nodeClass Represents the node class.
 * @return @c true on success, othewise @c false.
 */
bool isNodeClassValid(UA_NodeClass nodeClass);

/**
 * @brief Clones EdgeMessage object.
 * @remarks Allocated memory should be freed by the caller.
 * @param[in]  msg EdgeMessage object to be cloned.
 * @return Cloned EdgeMessage object on success. Otherwise null.
 */
EdgeMessage* cloneEdgeMessage(EdgeMessage *msg);

/**
 * @brief Clones EdgeMethodRequestParams object.
 * @remarks Allocated memory should be freed by the caller.
 * @param[in]  msg EdgeMethodRequestParams object to be cloned.
 * @return Cloned EdgeMethodRequestParams object on success. Otherwise null.
 */
EdgeMethodRequestParams* cloneEdgeMethodRequestParams(EdgeMethodRequestParams *methodParams);

/**
 * @brief To get the size of the node of a given type.
 * @param[in]  type Type of node.
 * @param[in]  isArray Indicates whether size is required for an array or single element.
 * @return Size of the node of a given type.
 */
size_t get_size(int type, bool isArray);

/**
 * @brief To get the char equivalent for the given type.
 * @param[in]  type Type of node id.
 * @return Character equivalent for the given type.
 */
char getCharacterNodeIdType(uint32_t type);

/**
 * @brief Clones UA_NodeId object.
 * @remarks Allocated memory should be freed by the caller.
 * @param[in]  nodeId UA_NodeId object to be cloned.
 * @return Cloned UA_NodeId object on success. Otherwise null.
 */
UA_NodeId *cloneNodeId(UA_NodeId *nodeId);

/**
 * @brief To display the given node id on console.
 * @remarks Works only when the stack is built in debug mode.
 * @param[in]  id Node Id to be logged.
 */
void logNodeId(UA_NodeId id);

#ifdef __cplusplus
}
#endif

#endif /* EDGE_OPEN62541_H_ */
