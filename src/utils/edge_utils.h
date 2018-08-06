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
 * @brief This file contains the utilities APIs for use in OPCUA module be implemented.
 */

#ifndef EDGE_UTILS_H_
#define EDGE_UTILS_H_

#include "edge_logger.h"
#include "opcua_common.h"

#ifdef __cplusplus
extern "C"
{
#endif

#define FREE(arg) if(arg) {free(arg); arg=NULL; }

#define IS_NULL(arg) ((arg == NULL) ? true : false)
#define IS_NOT_NULL(arg) ((arg != NULL) ? true : false)

#define VERIFY_NON_NULL_MSG(arg, msg, retVal) { if (!(arg)) { EDGE_LOG(TAG, \
            msg); return (retVal); } }
#define VERIFY_NON_NULL_NR_MSG(arg, msg) { if (!(arg)) { EDGE_LOG(TAG, \
            msg); return; } }
#define COND_CHECK(arg, retVal) { if (arg) { return (retVal); } }
#define COND_CHECK_MSG(arg, msg, retVal) { if (arg) { EDGE_LOG(TAG, \
            msg); return (retVal); } }
#define COND_CHECK_NR_MSG(arg, msg) { if (arg) { EDGE_LOG(TAG, \
            msg); return; } }

#define GUID_LENGTH (36)

#define CHECKING_ENDPOINT_URI_PATTERN ("^(opc)[.]{1}(tcp:)[/]{2}[A-Za-z0-9.-]{1,30}:[0-9]{1,6}([A-Za-z0-9_/-]{0,100})$")

#ifdef _WIN32
/**
 * @brief Gets the current time.(equivalent of gettimeofday() linux)
 * @remarks 
 */
int getTimeofDay(struct timeval *tp, struct timezone *tzp);
#endif

/**
 * @brief Prints the current system time in "MM/DD HH:MM:SS.sss" format.
 * @remarks Works only if the stack is built in debug mode.
 */
void logCurrentTimeStamp();

/**
 * @brief Clones a given string.
 * @remarks Allocated memory should be freed by the caller.
 * @param[in]  str String to be cloned.
 * @return Cloned string on success. Otherwise null.
 */
char *cloneString(const char *str);

/**
 * @brief Clones data of a specified length.
 * @remarks Allocated memory should be freed by the caller.
 * @param[in]  src Data to be cloned.
 * @param[in]  lenInbytes Length of the data to be cloned.
 * @return Cloned data on success. Otherwise null.
 */
void *cloneData(const void *src, int lenInbytes);

/**
 * @brief De-allocates the memory consumed by Edge_NodeId and its members.
 * @remarks Both Edge_NodeId and its members should have been allocated dynamically.
 * @param[in]  id Pointer to Edge_NodeId which needs to be freed.
 */
void freeEdgeNodeIdType(Edge_NodeId *id);

/**
 * @brief De-allocates the memory consumed by Edge_QualifiedName and its members.
 * @remarks Both Edge_QualifiedName and its members should have been allocated dynamically.
 * @param[in]  qn Pointer to Edge_QualifiedName which needs to be freed.
 */
void freeEdgeQualifiedName(Edge_QualifiedName *qn);

/**
 * @brief De-allocates the memory consumed by Edge_LocalizedText and its members.
 * @remarks Both Edge_LocalizedText and its members should have been allocated dynamically.
 * @param[in]  lt Pointer to Edge_LocalizedText which needs to be freed.
 */
void freeEdgeLocalizedText(Edge_LocalizedText *lt);

/**
 * @brief De-allocates the memory consumed by EdgeEndpointConfig and its members.
 * @remarks Both EdgeEndpointConfig and its members should have been allocated dynamically.
 * @param[in]  config Pointer to EdgeEndpointConfig which needs to be freed.
 */
void freeEdgeEndpointConfig(EdgeEndpointConfig *config);

/**
 * @brief De-allocates the memory consumed by the members of EdgeApplicationConfig.
 * @remarks EdgeApplicationConfig's members should have been allocated dynamically.
 * @param[in]  config Pointer to EdgeApplicationConfig whose members need to be freed.
 */
void freeEdgeApplicationConfigMembers(EdgeApplicationConfig *config);

/**
 * @brief De-allocates the memory consumed by EdgeApplicationConfig and its members.
 * @remarks Both EdgeApplicationConfig and its members should have been allocated dynamically.
 * @param[in]  config Pointer to EdgeApplicationConfig which needs to be freed.
 */
void freeEdgeApplicationConfig(EdgeApplicationConfig *config);

/**
 * @brief De-allocates the memory consumed by EdgeEndPointInfo and its members.
 * @remarks Both EdgeEndPointInfo and its members should have been allocated dynamically.
 * @param[in]  endpointInfo Pointer to EdgeEndPointInfo which needs to be freed.
 */
void freeEdgeEndpointInfo(EdgeEndPointInfo *endpointInfo);

/**
 * @brief De-allocates the memory consumed by EdgeBrowseResult and its members of the given size.
 * @remarks Both EdgeBrowseResult and its members should have been allocated dynamically.
 * @param[in]  browseResult Pointer to EdgeBrowseResult which needs to be freed.
 * @param[in]  browseResultLength Number of elements in the given EdgeBrowseResult.
 */
void freeEdgeBrowseResult(EdgeBrowseResult *browseResult, int browseResultLength);

/**
 * @brief De-allocates the memory consumed by EdgeResult.
 * @remarks EdgeResult should have been allocated dynamically.
 * @param[in]  res Pointer to EdgeResult which needs to be freed.
 */
void freeEdgeResult(EdgeResult *res);

/**
 * @brief De-allocates the memory consumed by EdgeNodeId and its members.
 * @remarks Both EdgeNodeId and its members should have been allocated dynamically.
 * @param[in]  nodeId Pointer to EdgeNodeId which needs to be freed.
 */
void freeEdgeNodeId(EdgeNodeId *nodeId);

/**
 * @brief De-allocates the memory consumed by EdgeNodeInfo and its members.
 * @remarks Both EdgeNodeInfo and its members should have been allocated dynamically.
 * @param[in]  nodeInfo Pointer to EdgeNodeInfo which needs to be freed.
 */
void freeEdgeNodeInfo(EdgeNodeInfo *nodeInfo);

/**
 * @brief De-allocates the memory consumed by EdgeMethodRequestParams and its members.
 * @remarks Both EdgeMethodRequestParams and its members should have been allocated dynamically.
 * @param[in]  methodParams Pointer to EdgeMethodRequestParams which needs to be freed.
 */
void freeEdgeMethodRequestParams(EdgeMethodRequestParams *methodParams);

/**
 * @brief De-allocates the memory consumed by EdgeRequest and its members.
 * @remarks Both EdgeRequest and its members should have been allocated dynamically.
 * @param[in]  req Pointer to EdgeRequest which needs to be freed.
 */
void freeEdgeRequest(EdgeRequest *req);

/**
 * @brief De-allocates the memory consumed by EdgeRequest and its members of the given size.
 * @remarks Both EdgeRequest and its members should have been allocated dynamically.
 * @param[in]  requests Pointer to EdgeRequest which needs to be freed.
 * @param[in]  requestLength Number of elements in the given request.
 */
void freeEdgeRequests(EdgeRequest **requests, int requestLength);

/**
 * @brief De-allocates the memory consumed by EdgeVersatility and its members.
 * @remarks Both EdgeVersatility and its members should have been allocated dynamically.
 * @param[in]  versatileValue Pointer to EdgeVersatility which needs to be freed.
 */
void freeEdgeVersatility(EdgeVersatility *versatileValue);

/**
 * @brief De-allocates the memory consumed by EdgeResponse and its members.
 * @remarks Both EdgeResponse and its members should have been allocated dynamically.
 * @param[in]  response Pointer to EdgeResponse which needs to be freed.
 */
void freeEdgeResponse(EdgeResponse *response);

/**
 * @brief De-allocates the memory consumed by EdgeResponse and its members of the given size.
 * @remarks Both EdgeResponse and its members should have been allocated dynamically.
 * @param[in]  responses Pointer to EdgeResponse which needs to be freed.
 * @param[in]  responseLength Number of elements in the given request.
 */
void freeEdgeResponses(EdgeResponse **responses, int responseLength);

/**
 * @brief De-allocates the memory consumed by EdgeMessage and its members.
 * @remarks Both EdgeMessage and its members should have been allocated dynamically.
 * @param[in]  msg Pointer to EdgeMessage which needs to be freed.
 */
void freeEdgeMessage(EdgeMessage *msg);

/**
 * @brief De-allocates the memory consumed by EdgeDevice and its members.
 * @remarks Both EdgeDevice and its members should have been allocated dynamically.
 * @param[in]  dev Pointer to EdgeDevice which needs to be freed.
 */
void freeEdgeDevice(EdgeDevice *dev);

/**
 * @brief Clones EdgeSubRequest object.
 * @remarks Allocated memory should be freed by the caller.
 * @param[in]  config EdgeSubRequest object to be cloned.
 * @return Cloned EdgeSubRequest object on success. Otherwise null.
 */
EdgeSubRequest* cloneSubRequest(EdgeSubRequest* subReq);

/**
 * @brief Clones EdgeEndpointConfig object.
 * @remarks Allocated memory should be freed by the caller.
 * @param[in]  config EdgeEndpointConfig object to be cloned.
 * @return Cloned EdgeEndpointConfig object on success. Otherwise null.
 */
EdgeEndpointConfig *cloneEdgeEndpointConfig(EdgeEndpointConfig *config);

/**
 * @brief Clones EdgeApplicationConfig object.
 * @remarks Allocated memory should be freed by the caller.
 * @param[in]  config EdgeApplicationConfig object to be cloned.
 * @return Cloned EdgeApplicationConfig object on success. Otherwise null.
 */
EdgeApplicationConfig *cloneEdgeApplicationConfig(EdgeApplicationConfig *config);

/**
 * @brief Clones EdgeEndPointInfo object.
 * @remarks Allocated memory should be freed by the caller.
 * @param[in]  endpointInfo EdgeEndPointInfo object to be cloned.
 * @return Cloned EdgeEndPointInfo object on success. Otherwise null.
 */
EdgeEndPointInfo *cloneEdgeEndpointInfo(EdgeEndPointInfo *endpointInfo);

/**
 * @brief Creates an EdgeResult object with the given status code.
 * @remarks Allocated memory should be freed by the caller.
 * @param[in]  code Represents status code.
 * @return EdgeResult object on success. Otherwise null.
 */
EdgeResult *createEdgeResult(EdgeStatusCode code);

/**
 * @brief Clones EdgeNodeId object.
 * @remarks Allocated memory should be freed by the caller.
 * @param[in]  nodeId EdgeNodeId object to be cloned.
 * @return Cloned EdgeNodeId object on success. Otherwise null.
 */
EdgeNodeId *cloneEdgeNodeId(EdgeNodeId *nodeId);

/**
 * @brief Clones EdgeNodeInfo object.
 * @remarks Allocated memory should be freed by the caller.
 * @param[in]  nodeInfo EdgeNodeInfo object to be cloned.
 * @return Cloned EdgeNodeInfo object on success. Otherwise null.
 */
EdgeNodeInfo *cloneEdgeNodeInfo(EdgeNodeInfo *nodeInfo);

/**
 * @brief To get the enum equivalent for the given node type.
 * @param[in]  type Type of node id.
 * @return Enum equivalent for the given type.
 */
EdgeNodeIdType getEdgeNodeIdType(char type);

#ifdef __cplusplus
}
#endif

#endif /* EDGE_UTILS_H_ */
