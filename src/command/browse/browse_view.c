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
#include "edge_open62541.h"
#include "opcua_manager.h"

#define TAG "browse_view"

void browseView(UA_Client *client, EdgeMessage *msg)
{
    EdgeNodeInfo *nodeInfo = createEdgeNodeInfoForNodeId(EDGE_INTEGER, UA_NS0ID_VIEWSFOLDER, SYSTEM_NAMESPACE_INDEX);
    VERIFY_NON_NULL_NR_MSG(nodeInfo, "EdgeCalloc FAILED for nodeinfo\n");
    msg->request = (EdgeRequest *) EdgeCalloc(1, sizeof(EdgeRequest));
    if(IS_NULL(msg->request))
    {
        EDGE_LOG(TAG, "Memory allocation failed.");
        invokeErrorCb(msg->message_id, NULL, STATUS_ERROR, "Failed to form a request for browsing views.");
        EdgeFree(nodeInfo->nodeId);
        EdgeFree(nodeInfo);
        return ;
    }
    msg->request->nodeInfo = nodeInfo;

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

    // First browse all the view nodes under UA_NS0ID_VIEWSFOLDER.
    // Then browse all the nodes under those view nodes.
    browseNodes(client, msg);

    freeEdgeRequest(msg->request);
    msg->request = NULL;
    EdgeFree(msg->browseParam);
    msg->browseParam = NULL;
}
