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
#include "edge_malloc.h"
#include "edge_random.h"

#include <stdio.h>
#include <stdlib.h>
#include <math.h>

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

    setSupportedApplicationTypes(config->supportedApplicationTypes);

    resgisterClientCallback(onResponseMessage, onStatusCallback, onDiscoveryCallback);
    resgisterServerCallback(onStatusCallback);
    resgisterMQCallback(onResponseMessage, onSendMessage);
}

EdgeResult createNamespace(char *name, char *rootNodeId, char *rootBrowseName,
        char *rootDisplayName)
{
    createNamespaceInServer(name, rootNodeId, rootBrowseName, rootDisplayName);
    EdgeResult result;
    result.code = STATUS_OK;
    return result;
}

EdgeResult createNode(char *namespaceUri, EdgeNodeItem *item)
{
    // add Nodes in server
    EdgeResult result = addNodesInServer(namespaceUri, item);
    return result;
}

EdgeResult modifyVariableNode(char *namespaceUri, char *nodeUri, EdgeVersatility *value)
{
    // modify variable nodes
    EdgeResult result = modifyNodeInServer(namespaceUri, nodeUri, value);
    return result;
}

EdgeResult addReference(EdgeReference *reference)
{
    EdgeResult result = addReferenceInServer(reference);
    return result;
}

EdgeResult createMethodNode(char *namespaceUri, EdgeNodeItem *item, EdgeMethod *method)
{
    EdgeResult result = addMethodNodeInServer(namespaceUri, item, method);
    return result;
}

void createServer(EdgeEndPointInfo *epInfo)
{
    EDGE_LOG(TAG, "[Received command] :: Server start.");
    if (b_serverInitialized)
    {
        EDGE_LOG(TAG, "Server already initialized.");
        return;
    }

    EdgeResult result = start_server(epInfo);
    if (result.code == STATUS_OK)
    {
        b_serverInitialized = true;
    }
}

void closeServer(EdgeEndPointInfo *epInfo)
{
    if (b_serverInitialized)
    {
        stop_server(epInfo);
        b_serverInitialized = false;
    }
}

EdgeResult getEndpointInfo(EdgeMessage *msg)
{
    EdgeResult ret;
    ret.code = STATUS_OK;
    if (NULL == msg || NULL == msg->endpointInfo)
    {
        ret.code = STATUS_PARAM_INVALID;
        return ret;
    }

    EDGE_LOG_V(TAG, "[Received command] :: Get endpoint info for [%s].\n", msg->endpointInfo->endpointUri);
    return getClientEndpoints(msg->endpointInfo->endpointUri);
}

EdgeResult findServers(const char *endpointUri, size_t serverUrisSize, unsigned char **serverUris,
        size_t localeIdsSize, unsigned char **localeIds, size_t *registeredServersSize,
        EdgeApplicationConfig **registeredServers)
{
    return findServersInternal(endpointUri, serverUrisSize, serverUris, localeIdsSize, localeIds,
            registeredServersSize, registeredServers);
}

void disconnectClient(EdgeEndPointInfo *epInfo)
{
    EDGE_LOG(TAG, "[Received command] :: Client disconnect.");
    disconnect_client(epInfo);
}

EdgeNodeItem* createVariableNodeItem(char* name, EdgeNodeIdentifier type, void* data,
        EdgeIdentifier nodeType)
{
    return createVariableNodeItemImpl(name, type, data, nodeType);
}

EdgeNodeItem* createNodeItem(char* name, EdgeIdentifier nodeType, EdgeNodeId *sourceNodeId)
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

void destroyEdgeContinuationPoint(EdgeContinuationPoint *cp)
{
    freeEdgeContinuationPoint(cp);
}

void destroyEdgeContinuationPointList(EdgeContinuationPointList *cpList)
{
    freeEdgeContinuationPointList(cpList);
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

static EdgeResult checkParameterValid(EdgeMessage *msg)
{
    EdgeResult result;
    result.code = STATUS_PARAM_INVALID;
    if (IS_NULL(msg))
    {
        EDGE_LOG(TAG, "EdgeMessage is NULL");
        return result;
    }

    if (IS_NULL(msg->endpointInfo))
    {
        EDGE_LOG(TAG, "EndpointInfo is NULL");
        return result;
    }

    if (IS_NULL(msg->endpointInfo->endpointUri))
    {
        EDGE_LOG(TAG, "EndpointURI in EndpointInfo is NULL");
        return result;
    }

    if (IS_NOT_NULL(msg->request) && IS_NULL(msg->request->nodeInfo))
    {
        EDGE_LOG(TAG, "NodeInfo in EdgeRequest is NULL");
        return result;
    }

    if (IS_NOT_NULL(msg->requests) && 0 == msg->requestLength)
    {
        EDGE_LOG(TAG, "Request Length is 0 but requests in EdgeMessage is not NULL.");
        return result;
    }

    if (IS_NOT_NULL(msg->requests))
    {
        for (size_t indx = 0; indx < msg->requestLength; indx++)
        {
            EdgeRequest *req = msg->requests[indx];
            if (IS_NULL(req) || IS_NULL(req->nodeInfo))
            {
                EDGE_LOG(TAG, "Request or its nodeInfo is NULL in one of the requests");
                return result;
            }

            if(msg->command != CMD_BROWSE && msg->command != CMD_BROWSENEXT &&
                    msg->command != CMD_BROWSE_VIEW)
            {
                if (IS_NULL(req->nodeInfo->valueAlias))
                {
                    EDGE_LOG(TAG, "ValueAlias in NodeInfo is NULL in one of the requests.");
                    return result;
                }
            }
        }
    }

    if ((msg->type == SEND_REQUEST && IS_NULL(msg->request))
             || (msg->type == SEND_REQUESTS  && IS_NULL(msg->requests)))
    {
        if (msg->command == CMD_READ
                || msg->command == CMD_WRITE
                || msg->command == CMD_BROWSE
                || msg->command == CMD_METHOD
                || msg->command == CMD_SUB)
        {
            return result;
        }
    }

    if (msg->command == CMD_BROWSE  && msg->browseParam == NULL)
    {
        EDGE_LOG(TAG, "BrowseParam is NULL");
        return result;
    }

    if (msg->command == CMD_SUB)
    {
        if (msg->request)
        {
            if (IS_NULL(msg->request->subMsg))
            {
                EDGE_LOG(TAG, "SubMessage in request is NULL.");
                return result;
            }
        }
        else
        {
            for (size_t indx = 0; indx < msg->requestLength; indx++)
            {
                if (IS_NULL(msg->requests[indx]->subMsg))
                {
                    EDGE_LOG(TAG, "SubMessage in one of the requests is NULL.");
                    return result;
                }
            }
        }
    }

    result.code = STATUS_OK;
    return result;
}

//////////////////////// QUEUE MESSAGE HANDLING /////////////////////////

EdgeResult sendRequest(EdgeMessage* msg)
{
    EdgeResult result = checkParameterValid(msg);
    if (result.code == STATUS_OK)
    {
        EdgeMessage *msgCopy = cloneEdgeMessage(msg);
        bool ret = add_to_sendQ(msgCopy);
        result.code = (ret ? STATUS_OK : STATUS_ENQUEUE_ERROR);
    }
    return result;
}

void onSendMessage(EdgeMessage* msg)
{
    if (msg->command == CMD_START_SERVER)
    {
        EDGE_LOG(TAG, "\n[Received command] :: START SERVER \n");
        if (b_serverInitialized)
        {
            printf("Server already initialised\n");
            return;
        }
        EdgeResult result = start_server(msg->endpointInfo);
        if (result.code == STATUS_OK)
        {
            b_serverInitialized = true;
        }
    }
    else if (msg->command == CMD_START_CLIENT)
    {
        EDGE_LOG(TAG, "\n[Received command] :: START CLIENT \n");
        bool result = connect_client(msg->endpointInfo->endpointUri);
        if (!result)
            return;
    }
    else if (msg->command == CMD_STOP_SERVER)
    {
        EDGE_LOG(TAG, "\nReceived command] :: STOP SERVER \n");
        stop_server(msg->endpointInfo);
        b_serverInitialized = false;
    }
    else if (msg->command == CMD_STOP_CLIENT)
    {
        EDGE_LOG(TAG, "\n[Received command] :: STOP CLIENT \n");
        disconnect_client(msg->endpointInfo);
    }
    else if (msg->command == CMD_READ)
    {
        EDGE_LOG(TAG, "\n[Received command] :: READ \n");
        readNodesFromServer(msg);
    }
    else if (msg->command == CMD_WRITE)
    {
        EDGE_LOG(TAG, "\n[Received command] :: WRITE \n");
        writeNodesInServer(msg);
    }
    else if (msg->command == CMD_METHOD)
    {
        EDGE_LOG(TAG, "\n[Received command] :: METHOD CALL \n");
        callMethodInServer(msg);
    }
    else if (msg->command == CMD_SUB)
    {
        EDGE_LOG(TAG, "\n[Received command] :: SUB \n");
        executeSubscriptionInServer(msg);
    }
    else if (msg->command == CMD_BROWSE)
    {
        EDGE_LOG(TAG, "\n[Received command] :: BROWSE \n");
        browseNodesInServer(msg);
    }
    else if (msg->command == CMD_BROWSE_VIEW)
    {
        EDGE_LOG(TAG, "\n[Received command] :: BROWSE \n");
        browseViewsInServer(msg);
    }
    else if (msg->command == CMD_BROWSENEXT)
    {
        EDGE_LOG(TAG, "\n[Received command] :: BROWSE \n");
        browseNextInServer(msg);
    }
}

void onResponseMessage(EdgeMessage *msg)
{
    if (NULL == receivedMsgCb || NULL == msg)
    {
        EDGE_LOG(TAG, "parameter is invalid.");
        return;
    }

    switch (msg->type)
    {
        case GENERAL_RESPONSE:
            receivedMsgCb->resp_msg_cb(msg);
            break;
        case BROWSE_RESPONSE:
            receivedMsgCb->browse_msg_cb(msg);
            break;
        case REPORT:
            receivedMsgCb->monitored_msg_cb(msg);
            break;
        case ERROR:
            receivedMsgCb->error_msg_cb(msg);
            break;
        default:
            break;
    }
}

void onDiscoveryCallback(EdgeDevice *device)
{
    if (NULL == discoveryCb)
    {
        // discovery callback not registered by application.
        return;
    }
    discoveryCb->endpoint_found_cb(device);
}

void onStatusCallback(EdgeEndPointInfo *epInfo, EdgeStatusCode status)
{
    if (NULL == statusCb)
    {
        // status callback not registered by application.
        return;
    }
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

EdgeNodeIdentifier getValueType(const char* nodeName)
{
    int nsIdx = 0, valueType = 0;
    char nodeType;
    char browseName[MAX_BROWSENAME_SIZE+1];
    sscanf(nodeName, UNIQUE_NODE_PATH, &nsIdx, &nodeType, &valueType, browseName);
    return valueType;
}

EdgeNodeInfo* createEdgeNodeInfoForNodeId(EdgeNodeIdType type, int nodeId, uint16_t nameSpace)
{
    EdgeNodeInfo* nodeInfo = (EdgeNodeInfo *) EdgeCalloc(1, sizeof(EdgeNodeInfo));
    if (IS_NULL(nodeInfo))
    {
        EDGE_LOG(TAG, "Error : Malloc failed for nodeInfo in test browse");
        return NULL;
    }
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
    char browseName[MAX_BROWSENAME_SIZE+1];
    sscanf(nodeName, UNIQUE_NODE_PATH, &nsIdx, &nodeType, &valueType, browseName);

    EdgeNodeInfo* nodeInfo = (EdgeNodeInfo *) EdgeCalloc(1, sizeof(EdgeNodeInfo));
    if (IS_NULL(nodeInfo))
    {
        return NULL;
    }
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

EdgeResult insertSubParameter(EdgeMessage **msg, const char* nodeName, EdgeNodeIdentifier subType,
        double samplingInterval, double publishingInterval, int maxKeepAliveCount,
        int lifetimeCount, int maxNotificationsPerPublish, bool publishingEnabled, int priority,
        uint32_t queueSize)
{
    EdgeResult result;
    result.code = STATUS_OK;
    if (IS_NULL((*msg)) || IS_NULL(nodeName))
    {
        EDGE_LOG(TAG, "Error : parameter is not valid");
        result.code = STATUS_PARAM_INVALID;
        goto EXIT;
    }

    EdgeSubRequest* subReq = (EdgeSubRequest *) EdgeCalloc(1, sizeof(EdgeSubRequest));
    if (IS_NULL(subReq))
    {
        EDGE_LOG(TAG, "Error : Malloc failed for subReq");
        result.code = STATUS_ERROR;
        goto EXIT;
    }

    if (Edge_Create_Sub == subType || Edge_Modify_Sub == subType)
    {
        subReq->subType = subType;
        subReq->samplingInterval = samplingInterval;
        subReq->publishingInterval = publishingInterval;
        subReq->maxKeepAliveCount = maxKeepAliveCount;
        subReq->lifetimeCount = lifetimeCount;
        subReq->maxNotificationsPerPublish = maxNotificationsPerPublish;
        subReq->publishingEnabled = publishingEnabled;
        subReq->priority = priority;
        subReq->queueSize = queueSize;
    }
    else
    {
        subReq->subType = subType;
    }

    if (Edge_Create_Sub == subType)
    {
        size_t index = (*msg)->requestLength;

        (*msg)->requests[index] = (EdgeRequest *) EdgeCalloc(1, sizeof(EdgeRequest));
        if (IS_NULL((*msg)->requests[index]))
        {
            EDGE_LOG(TAG, "Error : EdgeMalloc failed for requests");
            result.code = STATUS_ERROR;
            goto EXIT;
        }

        (*msg)->requests[index]->nodeInfo = createEdgeNodeInfo(nodeName);
        if (IS_NULL((*msg)->requests[index]->nodeInfo))
        {
            EDGE_LOG(TAG, "Error : Malloc failed for nodeInfo subReq");
            result.code = STATUS_ERROR;
            goto EXIT;
        }
        (*msg)->requests[index]->subMsg = subReq;
        (*msg)->requestLength = ++index;
    }
    else if (Edge_Modify_Sub == subType || Edge_Delete_Sub == subType
            || Edge_Republish_Sub == subType)
    {
        if (NULL == (*msg)->request)
        {
            EDGE_LOG(TAG, "Error : Malloc failed for request");
            result.code = STATUS_ERROR;
            goto EXIT;
        }

        (*msg)->request->nodeInfo = createEdgeNodeInfo(nodeName);
        if (IS_NULL((*msg)->request->nodeInfo))
        {
            EDGE_LOG(TAG, "Error : Malloc failed for nodeInfo modify");
            goto EXIT;
        }
        (*msg)->request->subMsg = subReq;
        (*msg)->requestLength = 1;
    }

    EXIT: return result;
}

EdgeMessage* createEdgeSubMessage(const char *endpointUri, const char* nodeName, size_t requestSize,
        EdgeNodeIdentifier subType)
{
    EdgeMessage *msg = (EdgeMessage *) EdgeCalloc(1, sizeof(EdgeMessage));
    if (IS_NULL(msg))
    {
        EDGE_LOG(TAG, "Error : EdgeMalloc failed for msg");
        return NULL;
    }

    msg->endpointInfo = (EdgeEndPointInfo *) EdgeCalloc(1, sizeof(EdgeEndPointInfo));
    if (IS_NULL(msg->endpointInfo))
    {
        EDGE_LOG(TAG, "Error : Malloc failed for epInfo");
        EdgeFree(msg);
        return NULL;
    }

    msg->endpointInfo->endpointUri = copyString(endpointUri);

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

    msg->command = CMD_SUB;
    msg->message_id = EdgeGetRandom();

    return msg;
}

EdgeMessage* createEdgeAttributeMessage(const char *endpointUri, size_t requestSize,
        EdgeCommand cmd)
{
    EdgeMessage *msg = (EdgeMessage *) EdgeCalloc(1, sizeof(EdgeMessage));
    if (IS_NULL(msg))
    {
        EDGE_LOG(TAG, "Error : EdgeMalloc failed for msg");
        return NULL;
    }

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
    EdgeMessage *msg = (EdgeMessage *) EdgeCalloc(1, sizeof(EdgeMessage));
    if (IS_NULL(msg))
    {
        EDGE_LOG(TAG, "Error : EdgeMalloc failed for msg");
        return NULL;
    }

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
    EdgeResult result;
    result.code = STATUS_OK;
    if (IS_NULL((*msg)) || IS_NULL(nodeName))
    {
        EDGE_LOG(TAG, "Error : parameter is not valid");
        result.code = STATUS_PARAM_INVALID;
        goto EXIT;
    }

    size_t index = (*msg)->requestLength;

    (*msg)->requests[index] = (EdgeRequest *) EdgeCalloc(1, sizeof(EdgeRequest));
    if (IS_NULL((*msg)->requests[index]))
    {
        EDGE_LOG(TAG, "Error : EdgeMalloc failed for requests");
        result.code = STATUS_ERROR;
        goto EXIT;
    }

    (*msg)->requests[index]->nodeInfo = createEdgeNodeInfo(nodeName);
    if (IS_NULL((*msg)->requests[index]->nodeInfo))
    {
        EDGE_LOG(TAG, "Error : Malloc failed for nodeInfo");
        result.code = STATUS_ERROR;
        goto EXIT;
    }
    (*msg)->requestLength = ++index;

    EXIT: return result;
}

EdgeResult insertWriteAccessNode(EdgeMessage **msg, const char* nodeName, void* value,
        size_t valueLen)
{
    EdgeResult result;
    result.code = STATUS_OK;
    if (IS_NULL((*msg)) || IS_NULL(nodeName))
    {
        EDGE_LOG(TAG, "Error : parameter is not valid");
        result.code = STATUS_PARAM_INVALID;
        goto EXIT;
    }

    if(IS_NULL(value) && valueLen != 0)
    {
        EDGE_LOG_V(TAG, "Error : Value length is %zu but value pointer is NULL.", valueLen);
        result.code = STATUS_PARAM_INVALID;
        goto EXIT;
    }

    size_t index = (*msg)->requestLength;

    (*msg)->requests[index] = (EdgeRequest *) EdgeCalloc(1, sizeof(EdgeRequest));
    if (IS_NULL((*msg)->requests[index]))
    {
        EDGE_LOG(TAG, "Error : EdgeMalloc failed for requests");
        result.code = STATUS_ERROR;
        goto EXIT;
    }

    (*msg)->requests[index]->nodeInfo = createEdgeNodeInfo(nodeName);
    if (IS_NULL((*msg)->requests[index]->nodeInfo))
    {
        EDGE_LOG(TAG, "Error : Malloc failed for nodeInfo");
        result.code = STATUS_ERROR;
        goto EXIT;
    }

    (*msg)->requests[index]->type = getValueType(nodeName);

    EdgeVersatility* varient = (EdgeVersatility*) malloc(sizeof(EdgeVersatility));
    if (IS_NULL(varient))
    {
        EDGE_LOG(TAG, "Error : EdgeMalloc failed for varient");
        result.code = STATUS_ERROR;
        goto EXIT;
    }
    varient->value = value;
    varient->arrayLength = 0;
    if (valueLen > 1)
    {
        varient->isArray = true;
        varient->arrayLength = valueLen;
    }
    else
    {
        varient->isArray = false;
    }
    (*msg)->requests[index]->value = varient;

    (*msg)->requestLength = ++index;

    EXIT: return result;
}

EdgeResult insertEdgeMethodParameter(EdgeMessage **msg, const char* nodeName,
        size_t inputParameterSize, EdgeNodeIdentifier argType, EdgeArgValType valType,
        void *scalarValue, void *arrayData, size_t arrayLength)
{
    EdgeResult result;
    result.code = STATUS_OK;

    if (IS_NULL((*msg)) || IS_NULL(nodeName))
    {
        EDGE_LOG(TAG, "Error : parameter is not valid");
        result.code = STATUS_PARAM_INVALID;
        goto EXIT;
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
        goto EXIT;
    }

    if (NULL == request->nodeInfo)
    {
        request->nodeInfo = createEdgeNodeInfo(nodeName);
    }

    if (NULL == request->methodParams)
    {
        request->methodParams = (EdgeMethodRequestParams *) EdgeCalloc(1,
                sizeof(EdgeMethodRequestParams));
        if (IS_NULL(request->methodParams))
        {
            EDGE_LOG(TAG, "Error : EdgeMalloc failed for methodParams");
            result.code = STATUS_ERROR;
            goto EXIT;
        }
    }
    if (inputParameterSize <= 0)
    {
        request->methodParams->num_inpArgs = 0;
        return result;
    }

    if (NULL == request->methodParams->inpArg)
    {
        request->methodParams->inpArg = (EdgeArgument **) EdgeCalloc(inputParameterSize,
                sizeof(EdgeArgument *));
        if (IS_NULL(request->methodParams->inpArg))
        {
            printf("Error : Malloc failed for methodParams->inpArg");
            result.code = STATUS_ERROR;
            goto EXIT;
        }
    }
    size_t num_inpArgs = request->methodParams->num_inpArgs;

    request->methodParams->inpArg[num_inpArgs] = (EdgeArgument *) EdgeCalloc(1,
            sizeof(EdgeArgument));
    if (IS_NULL(request->methodParams->inpArg[num_inpArgs]))
    {
        printf("Error : Malloc failed for methodParams->inpArg[X]");
        result.code = STATUS_ERROR;
        goto EXIT;
    }

    request->methodParams->inpArg[num_inpArgs]->argType = argType;
    request->methodParams->inpArg[num_inpArgs]->valType = valType;
    request->methodParams->inpArg[num_inpArgs]->scalarValue = scalarValue;
    request->methodParams->inpArg[num_inpArgs]->arrayData = arrayData;
    request->methodParams->inpArg[num_inpArgs]->arrayLength = arrayLength;

    request->methodParams->num_inpArgs = ++num_inpArgs;
    EXIT: return result;
}

EdgeResult insertBrowseParameter(EdgeMessage **msg, EdgeNodeInfo* nodeInfo,
        EdgeBrowseParameter parameter)
{
    EdgeResult result;
    result.code = STATUS_OK;

    if (IS_NULL((*msg)) || IS_NULL(nodeInfo))
    {
        EDGE_LOG(TAG, "Error : parameter is not valid");
        result.code = STATUS_PARAM_INVALID;
        goto EXIT;
    }

    if (SEND_REQUESTS == (*msg)->type)
    {
        size_t index = (*msg)->requestLength;

        (*msg)->requests[index] = (EdgeRequest *) EdgeCalloc(1, sizeof(EdgeRequest));
        if (IS_NULL((*msg)->requests[index]))
        {
            EDGE_LOG(TAG, "Error : EdgeMalloc failed for requests");
            result.code = STATUS_ERROR;
            goto EXIT;
        }

        (*msg)->requests[index]->nodeInfo = nodeInfo;
        (*msg)->requestLength = ++index;
    }
    else
    {
        if (NULL == (*msg)->request)
        {
            EDGE_LOG(TAG, "Error : Malloc failed for request");
            result.code = STATUS_ERROR;
            goto EXIT;
        }

        (*msg)->request->nodeInfo = nodeInfo;
        (*msg)->requestLength = 1;
    }

    (*msg)->browseParam = (EdgeBrowseParameter *) EdgeCalloc(1, sizeof(EdgeBrowseParameter));
    if (IS_NULL((*msg)->browseParam))
    {
        EDGE_LOG(TAG, "Error : Malloc failed for msg->browseParam");
        result.code = STATUS_ERROR;
        goto EXIT;
    }
    (*msg)->browseParam->direction = parameter.direction;
    (*msg)->browseParam->maxReferencesPerNode = parameter.maxReferencesPerNode;

    EXIT: return result;
}

void destroyBrowseNextDataElements(EdgeBrowseNextData *data)
{
    if (!data)
        return;

    for (int i = 0; i <= data->last_used; ++i)
    {
        if (IS_NOT_NULL(data->cp))
        {
            EdgeFree(data->cp[i].continuationPoint);
        }
        destroyEdgeNodeId(data->srcNodeId[i]);
    }
}

void destroyBrowseNextData(EdgeBrowseNextData *data)
{
    if (!data)
        return;

    destroyBrowseNextDataElements(data);
    EdgeFree(data->cp);
    EdgeFree(data->srcNodeId);
    EdgeFree(data);
}

EdgeBrowseNextData* initBrowseNextData(EdgeBrowseNextData *browseNextData,
        EdgeBrowseParameter *browseParam, int count, int last_used)
{
    destroyBrowseNextData(browseNextData);
    browseNextData = (EdgeBrowseNextData *) EdgeCalloc(1, sizeof(EdgeBrowseNextData));
    if (NULL == browseNextData)
    {
        return NULL;
    }

    if (browseParam)
    {
        browseNextData->browseParam = *browseParam;
    }
    browseNextData->count = count;
    browseNextData->last_used = last_used;
    browseNextData->cp = (EdgeContinuationPoint *) EdgeCalloc(browseNextData->count,
            sizeof(EdgeContinuationPoint));
    if (NULL == browseNextData->cp)
    {
        EdgeFree(browseNextData);
        return NULL;
    }

    browseNextData->srcNodeId = (EdgeNodeId **) calloc(browseNextData->count, sizeof(EdgeNodeId *));
    if (NULL == browseNextData->srcNodeId)
    {
        EdgeFree(browseNextData->cp);
        EdgeFree(browseNextData);
        return NULL;
    }

    return browseNextData;
}

EdgeResult addBrowseNextData(EdgeBrowseNextData **data, EdgeContinuationPoint *cp,
        EdgeNodeId *nodeId)
{
    EdgeResult result;
    result.code = STATUS_OK;
    if ((*data)->last_used >= (*data)->count)
    {
        printf("BrowseNextData limit(%d) reached. Cannot add this data.\n", (*data)->count);
        result.code = STATUS_ERROR;
        goto EXIT;
    }

    int index = ++(*data)->last_used;
    (*data)->cp[index].length = cp->length;
    (*data)->cp[index].continuationPoint = (unsigned char *) EdgeMalloc(
            cp->length * sizeof(unsigned char));
    if (IS_NULL((*data)->cp[index].continuationPoint))
    {
        printf(
                "Error : Malloc failed for data->cp[index].continuationPoint in addBrowseNextData\n");
        result.code = STATUS_ERROR;
        goto EXIT;
    }
    for (int i = 0; i < cp->length; i++)
    {
        (*data)->cp[index].continuationPoint[i] = cp->continuationPoint[i];
    }

    (*data)->srcNodeId[index] = cloneEdgeNodeId(nodeId);
    EXIT: return result;
}

EdgeBrowseNextData *cloneBrowseNextData(EdgeBrowseNextData* browseNextData)
{
    EdgeBrowseNextData *clone = (EdgeBrowseNextData *)EdgeCalloc(1, sizeof(EdgeBrowseNextData));
    VERIFY_NON_NULL(clone, NULL);
    clone->browseParam = browseNextData->browseParam;
    clone->count = browseNextData->count;
    clone->last_used = -1;
    clone->cp = (EdgeContinuationPoint *)EdgeCalloc(clone->count, sizeof(EdgeContinuationPoint));
    if(IS_NULL(clone->cp))
    {
        printf("Error :: EdgeCalloc Failed for lone->cp in cloneBrowseNextData \n");
        EdgeFree(clone);
        return NULL;
    }
    clone->srcNodeId = (EdgeNodeId **)calloc(clone->count, sizeof(EdgeNodeId *));
    if(IS_NULL(clone->srcNodeId))
    {
        printf("Error :: EdgeCalloc Failed for clone->srcNodeId in cloneBrowseNextData \n");
        EdgeFree(clone->cp);
        EdgeFree(clone);
        return NULL;
    }
    for (int i = 0; i <= browseNextData->last_used; ++i)
    {
        addBrowseNextData(&clone, &browseNextData->cp[i], browseNextData->srcNodeId[i]);
    }

    return clone;
}
