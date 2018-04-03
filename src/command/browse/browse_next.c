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

#include "browse_next.h"
#include "browse_common.h"
#include "edge_malloc.h"

#define TAG "browse_next"

void browseNext(UA_Client *client, EdgeMessage *msg)
{
    if (IS_NULL(msg->cpList) || msg->cpList->count < 1)
    {
        EDGE_LOG(TAG, "Continuation point list is either empty or NULL.");
        invokeErrorCb(msg->message_id, NULL, STATUS_PARAM_INVALID, "Continuation point list is either empty or NULL.");
        return;
    }

    size_t nodesToBrowseSize = msg->cpList->count;
    int *reqIdList = (int *) EdgeCalloc(nodesToBrowseSize, sizeof(int));
    if (IS_NULL(reqIdList))
    {
        EDGE_LOG(TAG, "Memory allocation failed.");
        invokeErrorCb(msg->message_id, NULL, STATUS_INTERNAL_ERROR, "Memory allocation failed.");
        return;
    }

    NodesToBrowse_t *browseNodesInfo = initNodesToBrowse(nodesToBrowseSize);
    if (IS_NULL(browseNodesInfo))
    {
        EDGE_LOG(TAG, "Memory allocation failed.");
        invokeErrorCb(msg->message_id, NULL, STATUS_INTERNAL_ERROR, "Memory allocation failed.");
        return;
    }

    if (msg->type == SEND_REQUEST)
    {
        EDGE_LOG(TAG, "Message Type: " SEND_REQUEST_DESC);
        UA_NodeId *nodeId;
        browseNodesInfo->nodeId[0] = (nodeId = getNodeId(msg->request)) ? *nodeId : UA_NODEID_NULL;
        browseNodesInfo->browseName[0] = convertNodeIdToString(nodeId);
        reqIdList[0] = 0;
        EdgeFree(nodeId);
    }
    else
    {
        EDGE_LOG(TAG, "Message Type: " SEND_REQUESTS_DESC);

        for (size_t i = 0; i < nodesToBrowseSize; ++i)
        {
            UA_NodeId *nodeId;
            browseNodesInfo->nodeId[i] = (nodeId = getNodeIdMultiReq(msg, i)) ? *nodeId : UA_NODEID_NULL;
            browseNodesInfo->browseName[i] = convertNodeIdToString(nodeId);
            EdgeFree(nodeId);
            reqIdList[i] = i;
        }
    }

    if(IS_NULL(InitBrowsePathNodeList()))
    {
        EDGE_LOG(TAG, "Failed to initialize a list for browse paths.");
        invokeErrorCb(msg->message_id, NULL, STATUS_INTERNAL_ERROR, "Failed to initialize a list for browse paths.");
    }
    else
    {
        EdgeStatusCode statusCode = browse(client, msg, true, browseNodesInfo, reqIdList, NULL);
        if (statusCode != STATUS_OK)
        {
            EDGE_LOG(TAG, "Browse failed.");
            invokeErrorCb(msg->message_id, NULL, STATUS_ERROR, "Browse failed.");
        }
    }

    destroyNodesToBrowse(browseNodesInfo, true);
    EdgeFree(reqIdList);
}
