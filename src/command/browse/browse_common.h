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

typedef struct NodesToBrowse
{
    UA_NodeId *nodeId;
    unsigned char **browseName;
    size_t size;
} NodesToBrowse_t;

typedef struct ViewNodeInfo
{
    UA_NodeId *nodeId;
    unsigned char *browseName;
} ViewNodeInfo_t;

typedef struct _browsePathNode{
    EdgeNodeId *edgeNodeId;
    unsigned char *browseName;
    struct _browsePathNode *pre;
    struct _browsePathNode *next;
} browsePathNode;

/**
 * @brief Register callback for browse response
 * @param[in]  callback response callback
 */
void setErrorResponseCallback(response_cb_t callback);

/**
 * @brief Invokes Browse/BrowseView/BrowseNext APIs in the underlying library and
 * passes the results to the application through callback.
 * @param[in]  client Client Handle.
 * @param[in]  msg EdgeMessage request data.
 * @param[in]  browseNext If true, this function will perform BrowseNext. Otherwise Browse/BrowseView.
 * For browseNext, continuation points will be taken from the given EdgeMessage.
 * @param[in]  browseNodesInfo Nodes to be browsed.
 * @param[in]  reqIdList Array index of the EdgeRequest in the EdgeMessage.
 * If there are 'n' requests in browse request message (EdgeMessage),
 * then the message id of the first request will be 0 and n-1 for the last request.
 * @param[in]  viewList If not null, this function will perform BrowseView. Otherwise Browse/BrowseNext.
 * @return STATUS_OK on success. Otherwise appropriate error code.
 */
EdgeStatusCode browse(UA_Client *client, EdgeMessage *msg, bool browseNext,
    NodesToBrowse_t *browseNodesInfo, int *reqIdList, List **viewList,
    browsePathNode **browsePathListHead, browsePathNode **browsePathListTail);

/**
 * @brief Executes Browse operation.
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
 * @brief Converts the node id in the reqId'th EdgeRequest in the given EdgeMessage to UA_NodeId.
 * @remarks Allocated memory should be freed by the caller.
 * @param[in]  msg EdgeMessage object.
 * @param[in]  reqId Array index of the EdgeRequest in the EdgeMessage.
 * @return UA_NodeId on success. Otherwise NULL.
 */
UA_NodeId *getNodeIdMultiReq(EdgeMessage *msg, int reqId);

/**
 * @brief Converts UA_NodeId to EdgeNodeId.
 * @remarks Allocated memory should be freed by the caller.
 * String, ByteString & Guid type NodeIds are NULL terminated.
 * @param[in]  nodeId NodeId to be converted.
 * @return Converted EdgeNodeId on success. Otherwise null.
 */
EdgeNodeId *getEdgeNodeId(UA_NodeId *node);

/**
 * @brief Creates a NodesToBrowse_t object for the given size.
 * @remarks Allocated memory should be freed by the caller.
 * @param[in]  size Indicates the count of the number of the nodes to be browsed.
 * @return NodesToBrowse_t on success. Otherwise NULL.
 */
NodesToBrowse_t *initNodesToBrowse(size_t size);

/**
 * @brief De-allocates NodesToBrowse_t and its members.
 * @remarks NodesToBrowse_t and its members should have been allocated dynamically.
 * @param[in]  ptr Pointer to valid NodesToBrowse_t.
 * @param[in]  deleteNodeId If true, this function will de-allocated the value of the each and every NodeId.
 */
void destroyNodesToBrowse(NodesToBrowse_t *ptr, bool deleteNodeId);

/**
 * @brief De-allocates each and every node in the list.
 * @remarks List nodes should have been allocated dynamically. Both params will be set to NULL.
 * @param[in]  browsePathListHead Pointer to list's head.
 * @param[in]  browsePathListTail Pointer to list's tail.
 */
void destroyBrowsePathNodeList(browsePathNode **browsePathListHead,
        browsePathNode **browsePathListTail);

/**
 * @brief Converts UA_NodeId to unsigned char string.
 * @remarks Allocated memory should be freed by the caller.
 * @param[in]  nodeId NodeId to be converted.
 * @return An unsigned char string on success. Otherwise NULL.
 */
unsigned char *convertNodeIdToString(UA_NodeId *nodeId);

/**
 * @brief Destroys the existing list and creates a head node for the new list of browse path nodes.
 * @remarks Allocated memory should be freed by the caller.
 * @return Head node pointer on success. Otherwise NULL.
 */
browsePathNode *InitBrowsePathNodeList();

/**
 * @brief Destroys the data of each and every node in the list.
 * @remarks Data of all the nodes in the list should have been allocated dynamically.
 * @param[in]  ptr Pointer to a valid List.
 */
void destroyViewListMembers(List *ptr);

#ifdef __cplusplus
}
#endif

#endif // EDGE_BROWSE_COMMON_H