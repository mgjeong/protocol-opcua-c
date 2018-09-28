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

#include "opcua_manager.h"
#include "edge_opcua_server.h"
#include "edge_opcua_client.h"
#include "message_dispatcher.h"
#include "edge_logger.h"
#include "edge_utils.h"
#include "edge_open62541.h"
#include "edge_malloc.h"
#include "edge_random.h"

#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#ifndef _WIN32
#include <regex.h>
#endif

#define TAG "opcua_manager"

static ReceivedMessageCallback *receivedMsgCb;
static StatusCallback *statusCb;
static DiscoveryCallback *discoveryCb;

static bool b_serverInitialized = false;

void showNodeList(void)
{
    printNodeListInServer();
}

static void registerRecvCallback(ReceivedMessageCallback *callback)
{
    receivedMsgCb = callback;
}

static void registerStatusCallback(StatusCallback *callback)
{
    statusCb = callback;
}

static void registerDiscoveryCallback(DiscoveryCallback *callback)
{
    discoveryCb = callback;
}

void configure(EdgeConfigure *config)
{
    registerRecvCallback(config->recvCallback);
    registerStatusCallback(config->statusCallback);
    registerDiscoveryCallback(config->discoveryCallback);

    /* Set the application types supported by the client.
        As supportedApplicationTypes configuration parameter is only for clients,
        only client applications are supposed to set this.
        Server responses which come with an application type which is not supported
        by the client will be filtered.
        In case if server application sets this parameter, it will not be used anywhere in the stack. */
    setSupportedApplicationTypes(config->supportedApplicationTypes);

    registerClientCallback(onResponseMessage, onStatusCallback, onDiscoveryCallback);
    registerServerCallback(onStatusCallback);
    registerMQCallback(onResponseMessage, onSendMessage);
}

EdgeResult createNamespace(const char *name, const char *rootNodeId, const char *rootBrowseName,
		const char *rootDisplayName)
{
    return createNamespaceInServer(name, rootNodeId, rootBrowseName, rootDisplayName);
}

EdgeResult createNode(const char *namespaceUri, EdgeNodeItem *item)
{
    // add Nodes in server
    return addNodesInServer(namespaceUri, item);
}

EdgeResult modifyVariableNode(const char *namespaceUri, const char *nodeUri, EdgeVersatility *value)
{
    // modify variable nodes
    return modifyNodeInServer(namespaceUri, nodeUri, value);
}

EdgeResult addReference(EdgeReference *reference)
{
    return addReferenceInServer(reference);
}

EdgeResult createMethodNode(const char *namespaceUri, EdgeNodeItem *item, EdgeMethod *method)
{
    return addMethodNodeInServer(namespaceUri, item, method);
}

EdgeResult createServer(EdgeEndPointInfo *epInfo)
{
    EDGE_LOG(TAG, "[Received command] :: Server start.");
    EdgeResult result = {STATUS_PARAM_INVALID };
    VERIFY_NON_NULL_MSG(epInfo, "", result);
    VERIFY_NON_NULL_MSG(epInfo->endpointConfig, "", result);
    COND_CHECK((epInfo->endpointConfig->bindPort < 1), result);
    COND_CHECK((epInfo->endpointConfig->bindPort > 65535), result);
		
    result.code = STATUS_ALREADY_INIT;
    VERIFY_NON_NULL_MSG(!b_serverInitialized, "Server already initialized.\n", result);
    result = start_server(epInfo);
    COND_CHECK((result.code != STATUS_OK), result);
    b_serverInitialized = true;
    return result;
}

void closeServer(EdgeEndPointInfo *epInfo)
{
    VERIFY_NON_NULL_NR_MSG(epInfo, "NULL param epINfo in closeServer\n");
    COND_CHECK_NR_MSG((!b_serverInitialized), "No server initialized.\n");
    stop_server(epInfo);
    b_serverInitialized = false;
}

EdgeResult getEndpointInfo(EdgeMessage *msg)
{
    EdgeResult ret;
    ret.code = STATUS_PARAM_INVALID;
    VERIFY_NON_NULL_MSG(msg, "msg is null\n", ret);
    VERIFY_NON_NULL_MSG(msg->endpointInfo, "msg endpointInfo is null\n", ret);
    EDGE_LOG_V(TAG, "[Received command] :: Get endpoint info for [%s].\n", msg->endpointInfo->endpointUri);
    return client_getEndpoints(msg->endpointInfo->endpointUri);
}

EdgeResult findServers(const char *endpointUri, size_t serverUrisSize, unsigned char **serverUris,
        size_t localeIdsSize, unsigned char **localeIds, size_t *registeredServersSize,
        EdgeApplicationConfig **registeredServers)
{
    return client_findServers(endpointUri, serverUrisSize, serverUris, localeIdsSize, localeIds,
            registeredServersSize, registeredServers);
}

void disconnectClient(EdgeEndPointInfo *epInfo)
{
    VERIFY_NON_NULL_NR_MSG(epInfo, "NULL param epINfo in dosconnect client\n");
    EDGE_LOG(TAG, "[Received command] :: Client disconnect.");
    disconnect_client(epInfo);
}

EdgeNodeItem* createVariableNodeItem(const char* name, int type, void* data,
        EdgeIdentifier nodeType, double minimumInterval)
{
    return createVariableNodeItemImpl(name, type, data, nodeType, minimumInterval);
}

EdgeNodeItem* createNodeItem(const char* name, EdgeIdentifier nodeType, EdgeNodeId *sourceNodeId)
{
    return createNodeItemImpl(name, nodeType, sourceNodeId);
}

EdgeResult deleteNodeItem(EdgeNodeItem* item)
{
    return deleteNodeItemImpl(item);
}

void destroyEdgeResult(EdgeResult *res)
{
    freeEdgeResult(res);
}

void destroyEdgeEndpointConfig(EdgeEndpointConfig *epConfig)
{
    freeEdgeEndpointConfig(epConfig);
}

void destroyEdgeApplicationConfigMembers(EdgeApplicationConfig *config)
{
    freeEdgeApplicationConfigMembers(config);
}

void destroyEdgeVersatility(EdgeVersatility *versatileValue)
{
    freeEdgeVersatility(versatileValue);
}

void destroyEdgeNodeId(EdgeNodeId *nodeId)
{
    freeEdgeNodeId(nodeId);
}

void destroyEdgeArgument(EdgeArgument *arg)
{
    freeEdgeArgument(arg);
}

void destroyEdgeMethodRequestParams(EdgeMethodRequestParams *reqParams)
{
    freeEdgeMethodRequestParams(reqParams);
}

void destroyEdgeNodeInfo(EdgeNodeInfo *nodeInfo)
{
    freeEdgeNodeInfo(nodeInfo);
}

void destroyEdgeEndpointInfo(EdgeEndPointInfo *endpointInfo)
{
    freeEdgeEndpointInfo(endpointInfo);
}

void destroyEdgeRequest(EdgeRequest *req)
{
    freeEdgeRequest(req);
}

void destroyEdgeResponse(EdgeResponse *resp)
{
    freeEdgeResponse(resp);
}

void destroyEdgeMessage(EdgeMessage *msg)
{
    freeEdgeMessage(msg);
}

char *copyString(const char *str)
{
    return cloneString(str);
}

bool checkEndpointURI(char *endpoint) {
#ifndef _WIN32
    regex_t regex;
    bool result = false;
    int retComp = regcomp(&regex, CHECKING_ENDPOINT_URI_PATTERN, REG_EXTENDED);
    COND_CHECK_MSG((retComp), "Error in compiling regex\n", result);

    int retRegex = regexec(&regex, endpoint, 0, NULL, 0);
    regfree(&regex);

    COND_CHECK_MSG((REG_NOERROR == retRegex), "Endpoint URI has port number\n", true);
    COND_CHECK_MSG((REG_NOMATCH == retRegex), "Endpoint URI has no port number\n", result);
    EDGE_LOG(TAG, "Error in regex\n");
    return result;
#else
    return true;
#endif	
}

static EdgeResult checkParameterValid(EdgeMessage *msg)
{
    EdgeResult result;
    result.code = STATUS_PARAM_INVALID;
    VERIFY_NON_NULL_MSG(msg, "NULL param EdgeMessage in checkParameterValid\n", result);
    VERIFY_NON_NULL_MSG(msg->endpointInfo, "EdgeMessage endpoint NULL in checkParameterValid\n", result);
    VERIFY_NON_NULL_MSG(msg->endpointInfo->endpointUri, "EndpointURI NULL in checkParameterValid\n", result);

    if(!checkEndpointURI(msg->endpointInfo->endpointUri)) {
        char *m_endpoint = (char*) EdgeCalloc(strlen(msg->endpointInfo->endpointUri) + 1, sizeof(char));
        strncpy(m_endpoint, msg->endpointInfo->endpointUri, strlen(msg->endpointInfo->endpointUri));

        const char *defaultPort = ":4840";
        m_endpoint = (char*) EdgeRealloc(m_endpoint, strlen(m_endpoint) + strlen(defaultPort) + 1);
        strncat(m_endpoint, defaultPort, strlen(defaultPort));

        EDGE_LOG_V(TAG, "modified endpoint uri : %s\n", m_endpoint);
        msg->endpointInfo->endpointUri = m_endpoint;
    }

    if (IS_NOT_NULL(msg->request))
    {
        VERIFY_NON_NULL_MSG(msg->request->nodeInfo, "NodeInfo in EdgeRequest is NULL", result);
    }

    if (IS_NOT_NULL(msg->requests))
    {
        COND_CHECK_MSG((0 == msg->requestLength), "Request Length is 0 but requests in EdgeMessage is not NULL.", result);
    }

    if (IS_NOT_NULL(msg->requests))
    {
        for (size_t indx = 0; indx < msg->requestLength; indx++)
        {
            EdgeRequest *req = msg->requests[indx];
            VERIFY_NON_NULL_MSG(req, "EdgeRequest received is NULL in checkParameterValid\n", result);
            VERIFY_NON_NULL_MSG(req->nodeInfo, "Request Nodeinfo is NULL in checkParameterValid\n", result);

            if(msg->command != CMD_BROWSE && msg->command != CMD_BROWSE_VIEW)
            {
                VERIFY_NON_NULL_MSG(req->nodeInfo->valueAlias, "valuealias NULL in checkParameterValid\n", result);
            }
        }
    }

    if ((msg->type == SEND_REQUEST && IS_NULL(msg->request))
             || (msg->type == SEND_REQUESTS  && IS_NULL(msg->requests)))
    {
        COND_CHECK((msg->command == CMD_READ), result);
        COND_CHECK((msg->command == CMD_WRITE), result);
        COND_CHECK((msg->command == CMD_BROWSE), result);
        COND_CHECK((msg->command == CMD_METHOD), result);
        COND_CHECK((msg->command == CMD_SUB), result);
        COND_CHECK((msg->command == CMD_READ_SAMPLING_INTERVAL), result);
    }

    if (msg->command == CMD_BROWSE)
    {
        VERIFY_NON_NULL_MSG(msg->browseParam, "BrowseParam is NULL in checkParameterValid\n", result);
    }

    if (msg->command == CMD_SUB)
    {
        if (msg->request)
        {
            VERIFY_NON_NULL_MSG(msg->request->subMsg, "Message request subrequest is NULL\n", result);
        }
        else
        {
            for (size_t indx = 0; indx < msg->requestLength; indx++)
            {
                VERIFY_NON_NULL_MSG(msg->requests[indx]->subMsg, "One of the message requests"
                    "submessage in NULL\n", result);
            }
        }
    }

    result.code = STATUS_OK;
    return result;
}

//////////////////////// QUEUE MESSAGE HANDLING /////////////////////////

EdgeResult sendRequest(EdgeMessage* msg)
{
    // Initializes the queueing thread if it is not initialized yet.
    init_queue();

    EdgeResult result = checkParameterValid(msg);
    COND_CHECK((result.code != STATUS_OK), result);
    EdgeMessage *msgCopy = cloneEdgeMessage(msg);
    result.code = STATUS_ERROR;
    VERIFY_NON_NULL_MSG(msgCopy, "NULL messageCopy recevied in send request\n", result);
    bool ret = add_to_sendQ(msgCopy);
    result.code = (ret ? STATUS_OK : STATUS_ENQUEUE_ERROR);
    return result;
}

void onSendMessage(EdgeMessage* msg)
{
    if (CMD_START_SERVER == msg->command)
    {
        EDGE_LOG(TAG, "\n[Received command] :: START SERVER \n");
        VERIFY_NON_NULL_NR_MSG(!b_serverInitialized, "Server already initialized\n");
        EdgeResult result = start_server(msg->endpointInfo);
        COND_CHECK_NR_MSG((result.code != STATUS_OK), "Error in starting server\n");
        b_serverInitialized = true;
    }
    else if (CMD_START_CLIENT == msg->command)
    {
        EDGE_LOG(TAG, "\n[Received command] :: START CLIENT \n");
        bool result = connect_client(msg->endpointInfo->endpointUri);
        VERIFY_NON_NULL_NR_MSG(!result, "");
    }
    else if (CMD_STOP_SERVER == msg->command)
    {
        EDGE_LOG(TAG, "\nReceived command] :: STOP SERVER \n");
        stop_server(msg->endpointInfo);
        b_serverInitialized = false;
    }
    else if (CMD_STOP_CLIENT == msg->command)
    {
        EDGE_LOG(TAG, "\n[Received command] :: STOP CLIENT \n");
        disconnect_client(msg->endpointInfo);
    }
    else if (CMD_READ == msg->command)
    {
        EDGE_LOG(TAG, "\n[Received command] :: READ \n");
        readNodesFromServer(msg);
    }
    else if (CMD_READ_SAMPLING_INTERVAL == msg->command)
    {
        EDGE_LOG(TAG, "\n[Received command] :: READ \n");
        readNodesFromServer(msg);
    }
    else if (CMD_WRITE == msg->command)
    {
        EDGE_LOG(TAG, "\n[Received command] :: WRITE \n");
        writeNodesInServer(msg);
    }
    else if (CMD_METHOD == msg->command)
    {
        EDGE_LOG(TAG, "\n[Received command] :: METHOD CALL \n");
        callMethodInServer(msg);
    }
    else if (CMD_SUB == msg->command)
    {
        EDGE_LOG(TAG, "\n[Received command] :: SUB \n");
        executeSubscriptionInServer(msg);
    }
    else if (CMD_BROWSE == msg->command || CMD_BROWSE_VIEW == msg->command)
    {
        EDGE_LOG(TAG, "\n[Received command] :: BROWSE \n");
        browseNodesInServer(msg);
    }
}

void onResponseMessage(EdgeMessage *msg)
{
    VERIFY_NON_NULL_NR_MSG(receivedMsgCb, "NULL receivedMsgCb in onResponseMessage\n");
    VERIFY_NON_NULL_NR_MSG(msg, "NULL Message param in onResponseMessage\n");
  
    if(msg->type == GENERAL_RESPONSE)
        receivedMsgCb->resp_msg_cb(msg);
    if(msg->type == BROWSE_RESPONSE)
        receivedMsgCb->browse_msg_cb(msg);
    if(msg->type == REPORT)
        receivedMsgCb->monitored_msg_cb(msg);
    if(msg->type == ERROR_RESPONSE)
        receivedMsgCb->error_msg_cb(msg);
}

void onDiscoveryCallback(EdgeDevice *device)
{
    VERIFY_NON_NULL_NR_MSG(discoveryCb, "NULL discoveryCb in onDiscoveryCallback\n"); // discovery callback not registered by application.
    discoveryCb->endpoint_found_cb(device);
}

void onStatusCallback(EdgeEndPointInfo *epInfo, EdgeStatusCode status)
{
    VERIFY_NON_NULL_NR_MSG(statusCb, "NULL statusCb in onStatusCallback\n"); // status callback not registered by application.
    if (STATUS_SERVER_STARTED == status || STATUS_CLIENT_STARTED == status)
    {
        statusCb->start_cb(epInfo, status);
    }
    else if (STATUS_STOP_SERVER == status || STATUS_STOP_CLIENT == status)
    {
        statusCb->stop_cb(epInfo, status);
    }
    else if (STATUS_CONNECTED == status || STATUS_DISCONNECTED == status)
    {
        statusCb->network_cb(epInfo, status);
    }
}

int getValueType(const char* nodeName)
{
    int nsIdx = 0, valueType = 0;
    char nodeType;
    char browseName[MAX_BROWSENAME_SIZE+1] = {0};
    sscanf(nodeName, UNIQUE_NODE_PATH, &nsIdx, &nodeType, &valueType, browseName);
    return valueType;
}

EdgeNodeInfo* createEdgeNodeInfoForNodeId(EdgeNodeIdType type, int nodeId, uint16_t nameSpace)
{
    EdgeNodeInfo* nodeInfo = (EdgeNodeInfo *) EdgeCalloc(1, sizeof(EdgeNodeInfo));
    VERIFY_NON_NULL_MSG(nodeInfo, "EdgeCalloc failed in createEdgeNodeInfoForNodeId\n", NULL);

    nodeInfo->nodeId = (EdgeNodeId *) EdgeCalloc(1, sizeof(EdgeNodeId));
    if (IS_NULL(nodeInfo->nodeId))
    {
        EDGE_LOG(TAG, "Error : Malloc failed for nodeInfo->nodeId in test browse");
        freeEdgeNodeInfo(nodeInfo);
        return NULL;
    }
    nodeInfo->nodeId->type = type;
    nodeInfo->nodeId->integerNodeId = nodeId;
    nodeInfo->nodeId->nameSpace = nameSpace;

    return nodeInfo;
}

EdgeNodeInfo* createEdgeNodeInfo(const char* nodeName)
{
    int nsIdx = 0, valueType = 0;
    char nodeType;
    char browseName[MAX_BROWSENAME_SIZE+1] = {0};
    sscanf(nodeName, UNIQUE_NODE_PATH, &nsIdx, &nodeType, &valueType, browseName);

    EdgeNodeInfo* nodeInfo = (EdgeNodeInfo *) EdgeCalloc(1, sizeof(EdgeNodeInfo));
    VERIFY_NON_NULL_MSG(nodeInfo, "EdgeCalloc FAILED in createEdgeNodeInfo\n", NULL);
    nodeInfo->valueAlias = copyString(browseName);

    nodeInfo->nodeId = (EdgeNodeId *) EdgeCalloc(1, sizeof(EdgeNodeId));
    if (IS_NULL(nodeInfo->nodeId))
    {
        EDGE_LOG(TAG, "Error : Malloc failed for nodeInfo->valueAlias");
        freeEdgeNodeInfo(nodeInfo);
        return NULL;
    }
    nodeInfo->nodeId->nodeUri = copyString(nodeName);
    nodeInfo->nodeId->nodeId = copyString(browseName);
    nodeInfo->nodeId->nameSpace = (uint16_t) nsIdx;
    nodeInfo->nodeId->type = getEdgeNodeIdType(nodeType);

    return nodeInfo;
}

EdgeResult insertSubParameter(EdgeMessage **msg, const char* nodeName, EdgeNodeType subType,
        double samplingInterval, double publishingInterval, int maxKeepAliveCount,
        int lifetimeCount, int maxNotificationsPerPublish, bool publishingEnabled, int priority,
        uint32_t queueSize)
{
    EdgeResult result;
    result.code = STATUS_PARAM_INVALID;
    VERIFY_NON_NULL_MSG((*msg), "Error : Malloc failed for subReq\n", result);
    VERIFY_NON_NULL_MSG(nodeName, "Error : Malloc failed for subReq\n", result);

    COND_CHECK_MSG(((*msg)->command != CMD_SUB), "Error: parameter is not valid", result);

    result.code = STATUS_ERROR;
    EdgeSubRequest* subReq = (EdgeSubRequest *) EdgeCalloc(1, sizeof(EdgeSubRequest));
    VERIFY_NON_NULL_MSG(subReq, "Error : Malloc failed for subReq\n", result);

    result.code = STATUS_OK;
    subReq->subType = subType;
    if (Edge_Create_Sub == subType || Edge_Modify_Sub == subType)
    {
        subReq->samplingInterval = samplingInterval;
        subReq->publishingInterval = publishingInterval;
        subReq->maxKeepAliveCount = maxKeepAliveCount;
        subReq->lifetimeCount = lifetimeCount;
        subReq->maxNotificationsPerPublish = maxNotificationsPerPublish;
        subReq->publishingEnabled = publishingEnabled;
        subReq->priority = priority;
        subReq->queueSize = queueSize;
    }

    if (Edge_Create_Sub == subType)
    {
        size_t index = (*msg)->requestLength;

        result.code = STATUS_ERROR;
        (*msg)->requests[index] = (EdgeRequest *) EdgeCalloc(1, sizeof(EdgeRequest));
        VERIFY_NON_NULL_MSG((*msg)->requests[index], "Error : Malloc failed for requests", result);

        (*msg)->requests[index]->nodeInfo = createEdgeNodeInfo(nodeName);
        VERIFY_NON_NULL_MSG((*msg)->requests[index]->nodeInfo, "Error : Malloc failed for nodeinfo subreq", result);
        (*msg)->requests[index]->subMsg = subReq;
        (*msg)->requestLength = ++index;
    }
    else if (Edge_Modify_Sub == subType || Edge_Delete_Sub == subType
            || Edge_Republish_Sub == subType)
    {
        result.code = STATUS_ERROR;
        VERIFY_NON_NULL_MSG((*msg)->request, "Error : Malloc failed for request\n", result);

        (*msg)->request->nodeInfo = createEdgeNodeInfo(nodeName);
        VERIFY_NON_NULL_MSG((*msg)->request->nodeInfo, "Error : Malloc failed for nodeinfo modify", result);
        (*msg)->request->subMsg = subReq;
        (*msg)->requestLength = 1;
    }

    result.code = STATUS_OK;
    return result;
}

EdgeMessage* createEdgeSubMessage(const char *endpointUri, const char* nodeName, size_t requestSize,
        EdgeNodeType subType)
{
    VERIFY_NON_NULL_MSG(endpointUri, "NULL endpointUri param in createEdgeSubMessage\n", NULL);

    EdgeMessage *msg = (EdgeMessage *) EdgeCalloc(1, sizeof(EdgeMessage));
    VERIFY_NON_NULL_MSG(msg, "EdgeCalloc FAILED for message in createEdgeSubMessage\n", NULL);

    msg->endpointInfo = (EdgeEndPointInfo *) EdgeCalloc(1, sizeof(EdgeEndPointInfo));
    if (IS_NULL(msg->endpointInfo))
    {
        EDGE_LOG(TAG, "Error : Malloc failed for epInfo");
        EdgeFree(msg);
        return NULL;
    }

    msg->endpointInfo->endpointUri = copyString(endpointUri);
    msg->command = CMD_SUB;
    msg->message_id = EdgeGetRandom();

    if (Edge_Create_Sub == subType)
    {
        msg->requests = (EdgeRequest **) EdgeCalloc(requestSize, sizeof(EdgeRequest *));
        if (IS_NULL(msg->requests))
        {
            EDGE_LOG(TAG, "Error : Malloc failed for requests");
            freeEdgeMessage(msg);
            return NULL;
        }
        msg->type = SEND_REQUESTS;
    }
    else if (Edge_Modify_Sub == subType || Edge_Delete_Sub == subType
            || Edge_Republish_Sub == subType)
    {
        msg->request = (EdgeRequest *) EdgeCalloc(1, sizeof(EdgeRequest));
        if (IS_NULL(msg->request))
        {
            EDGE_LOG(TAG, "Error : Malloc failed for request modify");
            freeEdgeMessage(msg);
            return NULL;
        }
        msg->type = SEND_REQUEST;

        if (Edge_Delete_Sub == subType || Edge_Republish_Sub == subType)
        {
            insertSubParameter(&msg, nodeName, subType, 0, 0, 0, 0, 0, false, 0, 0);
        }
    }

    return msg;
}

EdgeMessage* createEdgeAttributeMessage(const char *endpointUri, size_t requestSize,
        EdgeCommand cmd)
{
    VERIFY_NON_NULL_MSG(endpointUri, "NULL endpointUri param in createEdgeAttributeMessage\n", NULL);

    EdgeMessage *msg = (EdgeMessage *) EdgeCalloc(1, sizeof(EdgeMessage));
    VERIFY_NON_NULL_MSG(msg, "EdgeCalloc FAILED for message in createEdgeAttributeMessage\n", NULL);

    msg->endpointInfo = (EdgeEndPointInfo *) EdgeCalloc(1, sizeof(EdgeEndPointInfo));
    if (IS_NULL(msg->endpointInfo))
    {
        EDGE_LOG(TAG, "Error : Malloc failed for epInfo");
        EdgeFree(msg);
        return NULL;
    }

    msg->endpointInfo->endpointUri = copyString(endpointUri);

    msg->requests = (EdgeRequest **) EdgeCalloc(requestSize, sizeof(EdgeRequest *));
    if (IS_NULL(msg->requests))
    {
        EDGE_LOG(TAG, "Error : Malloc failed for requests");
        freeEdgeMessage(msg);
        return NULL;
    }
    msg->type = SEND_REQUESTS;
    msg->command = cmd;
    msg->message_id = EdgeGetRandom();

    return msg;
}

EdgeMessage* createEdgeMessage(const char *endpointUri, size_t requestSize, EdgeCommand cmd)
{
    VERIFY_NON_NULL_MSG(endpointUri, "NULL endpointUri param in createEdgeMessage\n", NULL);

    EdgeMessage *msg = (EdgeMessage *) EdgeCalloc(1, sizeof(EdgeMessage));
    VERIFY_NON_NULL_MSG(msg, "EdgeCalloc failed for message in createEdgeMessage\n", NULL);

    msg->endpointInfo = (EdgeEndPointInfo *) EdgeCalloc(1, sizeof(EdgeEndPointInfo));
    if (IS_NULL(msg->endpointInfo))
    {
        EDGE_LOG(TAG, "Error : Malloc failed for epInfo");
        EdgeFree(msg);
        return NULL;
    }

    msg->endpointInfo->endpointUri = copyString(endpointUri);

    if (requestSize > 1)
    {
        msg->requests = (EdgeRequest **) EdgeCalloc(requestSize, sizeof(EdgeRequest *));
        if (IS_NULL(msg->requests))
        {
            EDGE_LOG(TAG, "Error : Malloc failed for requests");
            freeEdgeMessage(msg);
            return NULL;
        }
        msg->type = SEND_REQUESTS;
    }
    else if (1 == requestSize)
    {
        msg->request = (EdgeRequest *) EdgeCalloc(1, sizeof(EdgeRequest));
        if (IS_NULL(msg->request))
        {
            EDGE_LOG(TAG, "Error : Malloc failed for request");
            freeEdgeMessage(msg);
            return NULL;
        }
        msg->type = SEND_REQUEST;
    }
    else
    {
        msg->type = SEND_REQUEST;
    }

    msg->command = cmd;
    msg->message_id = EdgeGetRandom();

    return msg;
}

EdgeResult insertReadAccessNode(EdgeMessage **msg, const char* nodeName)
{
    EdgeResult result = { STATUS_PARAM_INVALID };
    VERIFY_NON_NULL_MSG((*msg), "Error : msg is null", result);
    VERIFY_NON_NULL_MSG(nodeName, "Error : nodename is null", result);

    COND_CHECK_MSG(((*msg)->command != CMD_READ && (*msg)->command != CMD_READ_SAMPLING_INTERVAL),
                   "Error: Invalid command", result);

    result.code = STATUS_ERROR;
    size_t index = (*msg)->requestLength;
    (*msg)->requests[index] = (EdgeRequest *) EdgeCalloc(1, sizeof(EdgeRequest));
    VERIFY_NON_NULL_MSG((*msg)->requests[index], "Error : Malloc failed for requests", result);

    (*msg)->requests[index]->nodeInfo = createEdgeNodeInfo(nodeName);
    VERIFY_NON_NULL_MSG((*msg)->requests[index]->nodeInfo, "Error : Malloc failed for nodeinfo", result);
    (*msg)->requestLength = ++index;

    result.code = STATUS_OK;
    return result;
}

EdgeResult insertWriteAccessNode(EdgeMessage **msg, const char* nodeName, void* value,
        size_t valueCount)
{
    EDGE_LOG(TAG, "insertWriteAccessNode");
    return insertWriteAccessNodeWithValueType(msg, nodeName, value, valueCount, 0);
}

EdgeResult insertWriteAccessNodeWithValueType(EdgeMessage **msg, const char* nodeName, void* value,
        size_t valueCount, int valueType)
{
    EDGE_LOG(TAG, "insertWriteAccessNodeWithValueType");
    EdgeResult result = { STATUS_PARAM_INVALID };
    VERIFY_NON_NULL_MSG((*msg), "Error : msg is null", result);
    VERIFY_NON_NULL_MSG(nodeName, "Error : nodename is null", result);
    COND_CHECK_MSG(((*msg)->command != CMD_WRITE), "Error: command is invalid", result);

    if(IS_NULL(value) && valueCount != 0)
    {
        EDGE_LOG_V(TAG, "Error : Value length is %zu but value pointer is NULL.", valueCount);
        result.code = STATUS_PARAM_INVALID;
        return result;
    }

    result.code = STATUS_ERROR;
    size_t index = (*msg)->requestLength;
    (*msg)->requests[index] = (EdgeRequest *) EdgeCalloc(1, sizeof(EdgeRequest));
    VERIFY_NON_NULL_MSG((*msg)->requests[index], "Error : Malloc failed for requests", result);

    (*msg)->requests[index]->nodeInfo = createEdgeNodeInfo(nodeName);
    VERIFY_NON_NULL_MSG((*msg)->requests[index]->nodeInfo, "Error : Malloc failed for nodeinfo", result);
  
    if (valueType <= 0)
    {
        EDGE_LOG(TAG, "valueType will be set by nodeName");
        (*msg)->requests[index]->type = getValueType(nodeName);
    } else {
        EDGE_LOG_V(TAG, "valueType is %d", valueType);
        (*msg)->requests[index]->type = valueType;
    }

    EdgeVersatility* varient = (EdgeVersatility*) malloc(sizeof(EdgeVersatility));
    VERIFY_NON_NULL_MSG(varient, "Error : Malloc failed for Versatility", result);
    varient->value = value;
    varient->arrayLength = 0;
    varient->isArray = false;
    if (valueCount > 1)
    {
        varient->isArray = true;
        varient->arrayLength = valueCount;
    }
    (*msg)->requests[index]->value = varient;
    (*msg)->requestLength = ++index;

    result.code = STATUS_OK;
    return result;
}

EdgeResult insertEdgeMethodParameter(EdgeMessage **msg, const char* nodeName,
        size_t inputParameterSize, int argType, EdgeArgValType valType,
        void *scalarValue, void *arrayData, size_t arrayLength)
{
    EdgeResult result;
    result.code = STATUS_PARAM_INVALID;
    VERIFY_NON_NULL_MSG((*msg), "Error : msg is null", result);
    VERIFY_NON_NULL_MSG(nodeName, "Error : nodeName is null", result);
    COND_CHECK_MSG(((*msg)->command != CMD_METHOD), "Error: command is invalid", result);

    if (inputParameterSize > 0)
    {
        COND_CHECK_MSG(((argType == SCALAR && scalarValue == NULL)), "Error: Invalid command", result);
        COND_CHECK_MSG(((argType == ARRAY_1D && arrayData == NULL)), "Error: Invalid command", result);
    }

    EdgeRequest *request = NULL;
    if (SEND_REQUEST == (*msg)->type)
    {
        request = (*msg)->request;
        (*msg)->requestLength = 1;
    }
    else
    {
        result.code = STATUS_NOT_SUPPORT;
        return result;
    }

    result.code = STATUS_ERROR;
    if (NULL == request->nodeInfo)
    {
        request->nodeInfo = createEdgeNodeInfo(nodeName);
    }

    if (NULL == request->methodParams)
    {
        request->methodParams = (EdgeMethodRequestParams *) EdgeCalloc(1,
                sizeof(EdgeMethodRequestParams));
        VERIFY_NON_NULL_MSG(request->methodParams, "Error : Malloc failed for methodParams", result);
    }

    if (0 == inputParameterSize)
    {
        request->methodParams->num_inpArgs = 0;
        result.code = STATUS_OK;
        return result;
    }

    if (NULL == request->methodParams->inpArg)
    {
        request->methodParams->inpArg = (EdgeArgument **) EdgeCalloc(inputParameterSize,
                sizeof(EdgeArgument *));
        VERIFY_NON_NULL_MSG(request->methodParams->inpArg, "Error : Malloc failed for methodParams->inpArg", result);
    }
    size_t num_inpArgs = request->methodParams->num_inpArgs;

    request->methodParams->inpArg[num_inpArgs] = (EdgeArgument *) EdgeCalloc(1,
            sizeof(EdgeArgument));
    VERIFY_NON_NULL_MSG(request->methodParams->inpArg[num_inpArgs], "Error : Malloc failed for methodParams->inpArg[X]", result);

    request->methodParams->inpArg[num_inpArgs]->argType = argType;
    request->methodParams->inpArg[num_inpArgs]->valType = valType;
    request->methodParams->inpArg[num_inpArgs]->scalarValue = scalarValue;
    request->methodParams->inpArg[num_inpArgs]->arrayData = arrayData;
    request->methodParams->inpArg[num_inpArgs]->arrayLength = arrayLength;

    request->methodParams->num_inpArgs = ++num_inpArgs;

    result.code = STATUS_OK;
    return result;
}

EdgeResult insertBrowseParameter(EdgeMessage **msg, EdgeNodeInfo* nodeInfo,
        EdgeBrowseParameter parameter)
{
    EdgeResult result;
    result.code = STATUS_PARAM_INVALID;
    VERIFY_NON_NULL_MSG((*msg), "Error : msg is null", result);
    VERIFY_NON_NULL_MSG(nodeInfo, "Error : nodeInfo is null", result);

    COND_CHECK_MSG(((*msg)->command != CMD_BROWSE && (*msg)->command != CMD_BROWSE_VIEW),
                   "Error: Invalid command", result);

    result.code = STATUS_ERROR;
    if (SEND_REQUESTS == (*msg)->type)
    {
        size_t index = (*msg)->requestLength;

        (*msg)->requests[index] = (EdgeRequest *) EdgeCalloc(1, sizeof(EdgeRequest));
        VERIFY_NON_NULL_MSG((*msg)->requests[index], "Error : EdgeMalloc failed for requests", result);

        (*msg)->requests[index]->nodeInfo = nodeInfo;
        (*msg)->requestLength = ++index;
    }
    else
    {
        VERIFY_NON_NULL_MSG((*msg)->request, "Error : EdgeMalloc failed for request", result);

        (*msg)->request->nodeInfo = nodeInfo;
        (*msg)->requestLength = 1;
    }

    if (IS_NULL((*msg)->browseParam))
    {
        (*msg)->browseParam = (EdgeBrowseParameter *) EdgeCalloc(1, sizeof(EdgeBrowseParameter));
        if (IS_NULL((*msg)->browseParam))
        {
            EDGE_LOG(TAG, "Error : Malloc failed for msg->browseParam");
            if (SEND_REQUESTS == (*msg)->type)
            {
                int index = --(*msg)->requestLength;
                EdgeFree((*msg)->requests[index]);
                (*msg)->requests[index] = NULL;
            }
            result.code = STATUS_ERROR;
            return result;
        }
    }
    (*msg)->browseParam->direction = parameter.direction;
    (*msg)->browseParam->maxReferencesPerNode = parameter.maxReferencesPerNode;

    result.code = STATUS_OK;
    return result;
}

