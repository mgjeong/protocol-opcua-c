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

#include "browse.h"
#include "browse_common.h"
#include "browse_view.h"

#define TAG "browse"

void registerBrowseResponseCallback(response_cb_t callback) {
    setErrorResponseCallback(callback);
}

void executeBrowse(UA_Client *client, EdgeMessage *msg)
{
    if (IS_NULL(msg))
    {
        EDGE_LOG(TAG, "Edge message parameter is NULL.");
        invokeErrorCb(0, NULL, STATUS_PARAM_INVALID, "Edge message parameter is NULL.");
        return;
    }

    if (IS_NULL(client))
    {
        EDGE_LOG(TAG, "Client handle parameter is NULL.");
        invokeErrorCb(msg->message_id, NULL, STATUS_PARAM_INVALID, "Client handle parameter is NULL.");
        return;
    }

    if(msg->type != SEND_REQUEST && msg->type != SEND_REQUESTS)
    {
        EDGE_LOG(TAG, "Invalid message type.");
        invokeErrorCb(msg->message_id, NULL, STATUS_PARAM_INVALID, "Message type is invalid.");
        return;
    }

    if(msg->command==CMD_BROWSE)
    {
        browseNodes(client, msg);
    }
    else if(msg->command==CMD_BROWSE_VIEW)
    {
        browseView(client, msg);
    }
    else
    {
        EDGE_LOG(TAG, "Invalid command in message.");
        invokeErrorCb(msg->message_id, NULL, STATUS_PARAM_INVALID, "Invalid command in message.");
    }
}
