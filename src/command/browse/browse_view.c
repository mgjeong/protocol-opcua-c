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

#include "browse_view.h"
#include "browse_common.h"
#include "edge_list.h"
#include "edge_malloc.h"

#define TAG "browse_view"

/**
 * Static function to create an EdgeRequest with 'UA_NS0ID_VIEWSFOLDER' as node id.
 * Created EdgeRequest object can be used to browse all the views under 'UA_NS0ID_VIEWSFOLDER'.
 */
static EdgeRequest *getEdgeRequestForBrowseView()
{
    EdgeNodeInfo *nodeInfo = (EdgeNodeInfo *) EdgeCalloc(1, sizeof(EdgeNodeInfo));
    VERIFY_NON_NULL_MSG(nodeInfo, "EdgeCalloc FAILED for nodeinfo\n", NULL);

    nodeInfo->nodeId = (EdgeNodeId *) EdgeCalloc(1, sizeof(EdgeNodeId));
    if(IS_NULL(nodeInfo->nodeId))
    {
        EDGE_LOG(TAG, "Memory allocation failed.");
        EdgeFree(nodeInfo);
        return NULL;
    }
    nodeInfo->nodeId->type = INTEGER;
    nodeInfo->nodeId->integerNodeId = UA_NS0ID_VIEWSFOLDER;
    nodeInfo->nodeId->nameSpace = SYSTEM_NAMESPACE_INDEX;

    EdgeRequest *request = (EdgeRequest *) EdgeCalloc(1, sizeof(EdgeRequest));
    if(IS_NULL(request))
    {
        EDGE_LOG(TAG, "Memory allocation failed.");
        EdgeFree(nodeInfo->nodeId);
        EdgeFree(nodeInfo);
        return NULL;
    }
    request->nodeInfo = nodeInfo;
    return request;
}

/**
 * Static function which adds all the nodes in the viewNodeList (list containing only view nodes).
 * EdgeRequests for each and every node in the list will be created and added to the given EdgeMessage.
 */
static EdgeMessage *prepareEdgeMessageForBrowseView(EdgeMessage *msg, List *viewNodeList)
{
    VERIFY_NON_NULL_MSG(msg, "NULL EdgeMessage parameter\n", NULL);
    VERIFY_NON_NULL_MSG(viewNodeList, "NULL view node list parameter\n", NULL);

    EdgeMessage *browseViewMsg = (EdgeMessage *)EdgeCalloc(1, sizeof(EdgeMessage));
    VERIFY_NON_NULL_MSG(browseViewMsg, "EdgeCalloc FAILED for browse view message\n", NULL);

    browseViewMsg->endpointInfo = (EdgeEndPointInfo *)EdgeCalloc(1, sizeof(EdgeEndPointInfo));
    if(IS_NULL(browseViewMsg->endpointInfo))
    {
        EdgeFree(browseViewMsg);
        return NULL;
    }
    browseViewMsg->endpointInfo->endpointUri = cloneString(msg->endpointInfo->endpointUri);
    browseViewMsg->type = SEND_REQUESTS;
    browseViewMsg->command = CMD_BROWSE;
    browseViewMsg->message_id = msg->message_id;

    size_t size = getListSize(viewNodeList);

    EdgeRequest **request = (EdgeRequest **)calloc(size, sizeof(EdgeRequest *));
    if(IS_NULL(request))
    {
        goto ERROR;
    }

    List *ptr = viewNodeList;
    size_t idx = 0;
    while(ptr)
    {
        if(IS_NOT_NULL(ptr->data))
        {
            request[idx] = (EdgeRequest *)EdgeCalloc(1, sizeof(EdgeRequest));
            if(IS_NULL(request[idx]))
            {
                goto ERROR;
            }

            request[idx]->nodeInfo = (EdgeNodeInfo *)EdgeCalloc(1, sizeof(EdgeNodeInfo));
            if(IS_NULL(request[idx]->nodeInfo))
            {
                EdgeFree(request[idx]);
                goto ERROR;
            }

            request[idx]->nodeInfo->nodeId = getEdgeNodeId(((ViewNodeInfo_t *)ptr->data)->nodeId);
            if(IS_NULL(request[idx]->nodeInfo->nodeId))
            {
                EdgeFree(request[idx]->nodeInfo);
                EdgeFree(request[idx]);
                goto ERROR;
            }

            idx++;
        }
        ptr = ptr->link;
    }

    browseViewMsg->requests = (EdgeRequest **)calloc(idx, sizeof(EdgeRequest *));
    if(IS_NULL(browseViewMsg->requests))
    {
        goto ERROR;
    }

    for(size_t i = 0; i < idx; ++i)
    {
        browseViewMsg->requests[i] = request[i];
    }

    EdgeFree(request);
    request = NULL;

    browseViewMsg->requestLength = idx;
    browseViewMsg->browseParam = (EdgeBrowseParameter *)EdgeCalloc(1, sizeof(EdgeBrowseParameter));
    if(IS_NULL(browseViewMsg->browseParam))
    {
        goto ERROR;
    }
    browseViewMsg->browseParam->direction = DIRECTION_FORWARD;
    browseViewMsg->browseParam->maxReferencesPerNode = 0;

    return browseViewMsg;

ERROR:
    // Deallocate memory.
    if(IS_NOT_NULL(request))
    {
        for(size_t i = 0; i < idx; ++i)
        {
            freeEdgeRequest(request[i]);
        }
        EdgeFree(request);
    }
    freeEdgeMessage(browseViewMsg);
    return NULL;
}

void browseView(UA_Client *client, EdgeMessage *msg)
{
    // Updates the given edge message object and reverts them at the end.
    msg->request = getEdgeRequestForBrowseView();
    if(IS_NULL(msg->request))
    {
        EDGE_LOG(TAG, "Failed to form a request for browsing views.");
        invokeErrorCb(msg->message_id, NULL, STATUS_ERROR, "Failed to form a request for browsing views.");
        return;
    }

    msg->browseParam = (EdgeBrowseParameter *)EdgeCalloc(1, sizeof(EdgeBrowseParameter));
    if(IS_NULL(msg->browseParam))
    {
        EDGE_LOG(TAG, "Failed to form request for browsing views.");

        // Clean-up and revert the EdgeMessage parameter usage.
        freeEdgeRequest(msg->request);
        msg->request = NULL;

        invokeErrorCb(msg->message_id, NULL, STATUS_ERROR, "Failed to form a request for browsing views.");
        return;
    }
    msg->browseParam->direction = DIRECTION_FORWARD;
    msg->browseParam->maxReferencesPerNode = 0;

    int nodesToBrowseSize = 1;
    int *reqIdList = (int *) EdgeCalloc(nodesToBrowseSize, sizeof(int));
    if (IS_NULL(reqIdList))
    {
        EDGE_LOG(TAG, "Memory allocation failed.");

        // Clean-up and revert the EdgeMessage parameter usage.
        freeEdgeRequest(msg->request);
        msg->request = NULL;
        EdgeFree(msg->browseParam);
        msg->browseParam = NULL;

        invokeErrorCb(msg->message_id, NULL, STATUS_INTERNAL_ERROR, "Memory allocation failed.");
        return;
    }

    NodesToBrowse_t *browseNodesInfo = initNodesToBrowse(nodesToBrowseSize);
    if (IS_NULL(browseNodesInfo))
    {
        EDGE_LOG(TAG, "Memory allocation failed.");
        EdgeFree(reqIdList);

        // Clean-up and revert the EdgeMessage parameter usage.
        freeEdgeRequest(msg->request);
        msg->request = NULL;
        EdgeFree(msg->browseParam);
        msg->browseParam = NULL;

        invokeErrorCb(msg->message_id, NULL, STATUS_INTERNAL_ERROR, "Memory allocation failed.");
        return;
    }

    UA_NodeId *nodeId;
    browseNodesInfo->nodeId[0] = (nodeId = getNodeId(msg->request)) ? *nodeId : UA_NODEID_NULL;
    browseNodesInfo->browseName[0] = convertNodeIdToString(nodeId);
    reqIdList[0] = 0;
    EdgeFree(nodeId);

    List *viewList = NULL;
    browsePathNode *browsePathListHead = NULL, *browsePathListTail = NULL;
    EdgeStatusCode statusCode = browse(client, msg, false, browseNodesInfo, reqIdList, &viewList,
        &browsePathListHead, &browsePathListTail);
    if (statusCode != STATUS_OK)
    {
        EDGE_LOG(TAG, "Browse failed.");
        invokeErrorCb(msg->message_id, NULL, STATUS_ERROR, "Browse failed.");
    }
    else
    {
        EdgeMessage *browseViewMsg = prepareEdgeMessageForBrowseView(msg, viewList);
        if(IS_NULL(browseViewMsg))
        {
            EDGE_LOG(TAG, "Failed to prepare message for browse view request.");
            invokeErrorCb(msg->message_id, NULL, STATUS_INTERNAL_ERROR, "Failed to prepare message for browse view request.");
        }
        else
        {
            browseNodes(client, browseViewMsg);
            freeEdgeMessage(browseViewMsg);
        }
    }
    destroyViewListMembers(viewList);
    deleteList(&viewList);
    destroyBrowsePathNodeList(&browsePathListHead, &browsePathListTail);
    freeEdgeRequest(msg->request);
    msg->request = NULL;
    EdgeFree(msg->browseParam);
    msg->browseParam = NULL;
    destroyNodesToBrowse(browseNodesInfo, true);
    EdgeFree(reqIdList);
}
