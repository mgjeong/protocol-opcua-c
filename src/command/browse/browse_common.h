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
 * @file browse_common.h
 *
 * @brief This file contains the common APIs for Browse/BrowseView/BrowseNext commands.
 */

#ifndef EDGE_BROWSE_COMMON_H
#define EDGE_BROWSE_COMMON_H

#include "opcua_common.h"
#include "open62541.h"

#include "edge_utils.h"
#include "edge_list.h"
#include "command_adapter.h"

#ifdef __cplusplus
extern "C"
{
#endif

typedef struct ViewNodeInfo
{
    UA_NodeId *nodeId;
    unsigned char *browseName;
} ViewNodeInfo_t;

/**
 * @brief Register callback for browse response
 * @param[in]  callback response callback
 */
void setErrorResponseCallback(response_cb_t callback);

/**
 * @brief Based on server's capabilities, finds the maximum nodes a client can browse per request.
 * @remarks Performs synchronous read to get the server's capabilities.
 * @param[in]  client Client Handle.
 * @return Value indicating the maximum nodes a client can browse per request on success.
 */
//uint16_t getMaxNodesToBrowse(UA_Client *client);

/**
 * @brief Performs Browse, BrowseNext & BrowseView operations and
 * passes the results to the application through callback.
 * @param[in]  client Client Handle.
 * @param[in]  msg EdgeMessage request data.
 */
void browseNodes(UA_Client *client, EdgeMessage *msg);

/**
 * @brief Invokes application's error callback.
 * @param[in]  srcMsgId Array index of the EdgeRequest in the EdgeMessage.
 * If there are 'n' requests in browse request message (EdgeMessage),
 * then the message id of the first request will be 0 and n-1 for the last request.
 * @param[in]  srcNodeId NodeId of the node which resulted in an error.
 * @param[in]  edgeResult Error status code.
 * @param[in]  versatileValue Error message.
 */
void invokeErrorCb(uint32_t srcMsgId, EdgeNodeId *srcNodeId,
        EdgeStatusCode edgeResult, const char *versatileValue);

/**
 * @brief Converts the node id in the given EdgeRequest to UA_NodeId.
 * @remarks Allocated memory should be freed by the caller.
 * @param[in]  req EdgeRequest object.
 * @return UA_NodeId on success. Otherwise NULL.
 */
UA_NodeId *getNodeId(EdgeRequest *req);

/**
 * @brief Converts UA_NodeId to unsigned char string.
 * @remarks Allocated memory should be freed by the caller.
 * @param[in]  nodeId NodeId to be converted.
 * @return An unsigned char string on success. Otherwise NULL.
 */
unsigned char *convertNodeIdToString(UA_NodeId *nodeId);

#ifdef __cplusplus
}
#endif

#endif // EDGE_BROWSE_COMMON_H
