/* ****************************************************************
 *
 * Copyright 2017 Samsung Electronics All Rights Reserved.
 *
 *
 *
 * Licensed under the Apache License, Version 2.0 = the "License";
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

#include "edge_utils.h"

#include <stdlib.h>
#include <string.h>

#include "open62541.h"
#include "edge_logger.h"
#include "edge_malloc.h"

#define TAG "edge_utils"

edgeMap *createMap()
{
    edgeMap *map = (edgeMap *) EdgeMalloc(sizeof(edgeMap));
    VERIFY_NON_NULL(map, NULL);
    map->head = NULL;
    return map;
}

void insertMapElement(edgeMap *map, keyValue key, keyValue value)
{
    edgeMapNode *node = (edgeMapNode *) EdgeMalloc(sizeof(edgeMapNode));
    VERIFY_NON_NULL_NR(node);
    node->key = key;
    node->value = value;
    node->next = NULL;

    edgeMapNode *temp = map->head;

    if (temp == NULL)
    {
        map->head = node;
    }
    else
    {
        while (temp->next != NULL)
            temp = temp->next;

        temp->next = node;
    }
}

keyValue getMapElement(edgeMap *map, keyValue key)
{
    edgeMapNode *temp = map->head;

    while (temp != NULL)
    {
        if (temp->key == key)
        {
            return temp->value;
        }
        temp = temp->next;
    }

    return NULL;
}

void deleteMap(edgeMap *map)
{
    edgeMapNode *temp = map->head;
    edgeMapNode *xtemp;

    while (temp != NULL)
    {
        xtemp = temp->next;
        EdgeFree(temp);
        temp = xtemp;
    }

    map->head = NULL;
}

static List *createListNode(void *data)
{
    List *node = (List *) EdgeCalloc(1, sizeof(List));
    VERIFY_NON_NULL(node, NULL);
    node->data = data;
    return node;
}

bool addListNode(List **head, void *data)
{
    if (IS_NULL(head) || IS_NULL(data))
    {
        return false;
    }

    List *newnode = createListNode(data);
    if (IS_NULL(newnode))
    {
        return false;
    }

    newnode->link = *head;
    *head = newnode;
    return true;
}

unsigned int getListSize(List *ptr)
{
    if (IS_NULL(ptr))
    {
        return 0;
    }

    size_t size = 0;
    while (ptr)
    {
        size++;
        ptr = ptr->link;
    }
    return size;
}

void deleteList(List **head)
{
    if (!head)
    {
        return;
    }

    List *next = NULL;
    List *ptr = *head;
    while (ptr)
    {
        next = ptr->link;
        EdgeFree(ptr);
        ptr = next;
    }
    *head = NULL;
}

char *cloneString(const char *str)
{
    VERIFY_NON_NULL(str, NULL);
    size_t len = strlen(str);
    char *clone = (char *) EdgeMalloc(len + 1);
    VERIFY_NON_NULL(clone, NULL);
    strncpy(clone, str, len+1);
    return clone;
}

void *cloneData(const void *src, int lenInbytes)
{
    if (!src || lenInbytes < 1)
    {
        return NULL;
    }

    void *cloned = EdgeMalloc(lenInbytes);
    VERIFY_NON_NULL(cloned, NULL);
    memcpy(cloned, src, lenInbytes);
    return cloned;
}

char *convertUAStringToString(UA_String *uaStr)
{
    if (!uaStr || uaStr->length <= 0)
    {
        return NULL;
    }

    char *str = (char *) EdgeMalloc(uaStr->length + 1);
    VERIFY_NON_NULL(str, NULL);
    memcpy(str, uaStr->data, uaStr->length);
    str[uaStr->length] = '\0';
    return str;
}

void freeEdgeEndpointConfig(EdgeEndpointConfig *config)
{
    VERIFY_NON_NULL_NR(config);
    EdgeFree(config->serverName);
    EdgeFree(config->bindAddress);
    EdgeFree(config);
}

void freeEdgeApplicationConfigMembers(EdgeApplicationConfig *config)
{
    VERIFY_NON_NULL_NR(config);
    EdgeFree(config->applicationUri);
    EdgeFree(config->productUri);
    EdgeFree(config->applicationName);
    EdgeFree(config->gatewayServerUri);
    EdgeFree(config->discoveryProfileUri);
    for (size_t i = 0; i < config->discoveryUrlsSize; ++i)
    {
        EdgeFree(config->discoveryUrls[i]);
    }
    EdgeFree(config->discoveryUrls);
}

void freeEdgeApplicationConfig(EdgeApplicationConfig *config)
{
    VERIFY_NON_NULL_NR(config);
    freeEdgeApplicationConfigMembers(config);
    EdgeFree(config);
}

void freeEdgeContinuationPoint(EdgeContinuationPoint *cp)
{
    VERIFY_NON_NULL_NR(cp);
    EdgeFree(cp->continuationPoint);
    EdgeFree(cp);
}

void freeEdgeContinuationPointList(EdgeContinuationPointList *cpList)
{
    VERIFY_NON_NULL_NR(cpList);
    for (size_t i = 0; i < cpList->count; ++i)
    {
        freeEdgeContinuationPoint(cpList->cp[i]);
    }

    EdgeFree(cpList->cp);
    EdgeFree(cpList);
}

void freeEdgeDevice(EdgeDevice *dev)
{
    VERIFY_NON_NULL_NR(dev);
    if (dev->endpointsInfo)
    {
        for (int i = 0; i < dev->num_endpoints; ++i)
        {
            freeEdgeEndpointInfo(dev->endpointsInfo[i]);
        }
    }
    EdgeFree(dev->endpointsInfo);
    EdgeFree(dev->address);
    EdgeFree(dev->serverName);
    EdgeFree(dev);
}

EdgeSubRequest* cloneSubRequest(EdgeSubRequest* subReq)
{
    VERIFY_NON_NULL(subReq, NULL);
    EdgeSubRequest *clone = (EdgeSubRequest *)EdgeCalloc(1, sizeof(EdgeSubRequest));
    VERIFY_NON_NULL(clone, NULL);

    clone->subType = subReq->subType;
    clone->samplingInterval = subReq->samplingInterval;
    clone->publishingInterval = subReq->publishingInterval;
    clone->maxKeepAliveCount = subReq->maxKeepAliveCount;
    clone->lifetimeCount = subReq->lifetimeCount;
    clone->maxNotificationsPerPublish = subReq->maxNotificationsPerPublish;
    clone->publishingEnabled = subReq->publishingEnabled;
    clone->priority = subReq->priority;
    clone->queueSize = subReq->queueSize;

    return clone;
}

EdgeMethodRequestParams* cloneEdgeMethodRequestParams(EdgeMethodRequestParams *methodParams)
{
    VERIFY_NON_NULL(methodParams, NULL);
    EdgeMethodRequestParams* clone = (EdgeMethodRequestParams *) EdgeCalloc(1, sizeof(EdgeMethodRequestParams));
    VERIFY_NON_NULL(clone, NULL);

    clone->num_outArgs = methodParams->num_outArgs;

    if(methodParams->num_inpArgs < 1)
    {
        return clone;
    }

    clone->inpArg = (EdgeArgument**) EdgeCalloc(methodParams->num_inpArgs, sizeof(EdgeArgument*));
    if(IS_NULL(clone->inpArg))
    {
        EDGE_LOG(TAG, "Memory allocation failed.");
        goto ERROR;
    }

    clone->num_inpArgs = methodParams->num_inpArgs;
    for (size_t i  = 0; i < clone->num_inpArgs; i++)
    {
        clone->inpArg[i] = (EdgeArgument*) EdgeCalloc(1, sizeof(EdgeArgument));
        if(IS_NULL(clone->inpArg[i]))
        {
            EDGE_LOG(TAG, "Memory allocation failed.");
            goto ERROR;
        }

        clone->inpArg[i]->argType = methodParams->inpArg[i]->argType;
        clone->inpArg[i]->valType = methodParams->inpArg[i]->valType;
        clone->inpArg[i]->arrayLength = 0;
        clone->inpArg[i]->arrayData = NULL;
        if (SCALAR == methodParams->inpArg[i]->valType)
        {
            if (methodParams->inpArg[i]->argType == String)
            {
                clone->inpArg[i]->scalarValue = cloneString((char*) methodParams->inpArg[i]->scalarValue);
                if(IS_NULL(clone->inpArg[i]->scalarValue))
                {
                    EDGE_LOG(TAG, "Memory allocation failed.");
                    goto ERROR;
                }
            }
            else
            {
                size_t size = get_size(methodParams->inpArg[i]->argType, false);
                clone->inpArg[i]->scalarValue = (void *) EdgeCalloc(1, size);
                if(IS_NULL(clone->inpArg[i]->scalarValue))
                {
                    EDGE_LOG(TAG, "Memory allocation failed.");
                    goto ERROR;
                }
                memcpy(clone->inpArg[i]->scalarValue, methodParams->inpArg[i]->scalarValue, size);
            }
        }
        else if (ARRAY_1D == methodParams->inpArg[i]->valType)
        {
            clone->inpArg[i]->arrayLength = methodParams->inpArg[i]->arrayLength;
            if (methodParams->inpArg[i]->argType == String)
            {
                clone->inpArg[i]->arrayData = EdgeCalloc(methodParams->inpArg[i]->arrayLength, sizeof(char*));
                if(IS_NULL(clone->inpArg[i]->arrayData))
                {
                    EDGE_LOG(TAG, "Memory allocation failed.");
                    goto ERROR;
                }
                char **srcVal = (char**) methodParams->inpArg[i]->arrayData;
                char **dstVal = (char **) clone->inpArg[i]->arrayData;
                for (size_t j = 0; j < methodParams->inpArg[i]->arrayLength; j++)
                {
                    size_t len = strlen(srcVal[j]);
                    dstVal[j] = (char*) EdgeCalloc(len+1, sizeof(char));
                    if(IS_NULL(dstVal[j]))
                    {
                        EDGE_LOG(TAG, "Memory allocation failed.");
                        goto ERROR;
                    }
                    strncpy(dstVal[j], srcVal[j], len+1);
                }
            }
            else
            {
                size_t size = get_size(methodParams->inpArg[i]->argType, true);
                clone->inpArg[i]->arrayData = (void *) EdgeCalloc(clone->inpArg[i]->arrayLength, size);
                if(IS_NULL(clone->inpArg[i]->arrayData))
                {
                    EDGE_LOG(TAG, "Memory allocation failed.");
                    goto ERROR;
                }
                memcpy(clone->inpArg[i]->arrayData, methodParams->inpArg[i]->arrayData,
                        get_size(methodParams->inpArg[i]->argType, false) * methodParams->inpArg[i]->arrayLength);
            }

        }
    }
    return clone;

ERROR:
    freeEdgeMethodRequestParams(clone);
    return NULL;
}

EdgeEndpointConfig *cloneEdgeEndpointConfig(EdgeEndpointConfig *config)
{
    VERIFY_NON_NULL(config, NULL);
    EdgeEndpointConfig *clone = (EdgeEndpointConfig *) EdgeCalloc(1, sizeof(EdgeEndpointConfig));
    VERIFY_NON_NULL(clone, NULL);
    clone->requestTimeout = config->requestTimeout;
    clone->bindPort = config->bindPort;
    if (config->serverName)
    {
        clone->serverName = cloneString(config->serverName);
        if (!clone->serverName)
        {
            goto ERROR;
        }
    }

    if (config->bindAddress)
    {
        clone->bindAddress = cloneString(config->bindAddress);
        if (!clone->bindAddress)
        {
            goto ERROR;
        }
    }

    return clone;

    ERROR: freeEdgeEndpointConfig(clone);
    return NULL;
}

EdgeApplicationConfig *cloneEdgeApplicationConfig(EdgeApplicationConfig *config)
{
    VERIFY_NON_NULL(config, NULL);
    EdgeApplicationConfig *clone = (EdgeApplicationConfig *) EdgeCalloc(1,
            sizeof(EdgeApplicationConfig));
    VERIFY_NON_NULL(clone, NULL);
    clone->applicationType = config->applicationType;

    if (config->applicationUri)
    {
        clone->applicationUri = cloneString(config->applicationUri);
        if (!clone->applicationUri)
        {
            goto ERROR;
        }
    }

    if (config->productUri)
    {
        clone->productUri = cloneString(config->productUri);
        if (!clone->productUri)
        {
            goto ERROR;
        }
    }

    if (config->applicationName)
    {
        clone->applicationName = cloneString(config->applicationName);
        if (!clone->applicationName)
        {
            goto ERROR;
        }
    }

    if (config->gatewayServerUri)
    {
        clone->gatewayServerUri = cloneString(config->gatewayServerUri);
        if (!clone->gatewayServerUri)
        {
            goto ERROR;
        }
    }

    if (config->discoveryProfileUri)
    {
        clone->discoveryProfileUri = cloneString(config->discoveryProfileUri);
        if (!clone->discoveryProfileUri)
        {
            goto ERROR;
        }
    }

    clone->discoveryUrlsSize = config->discoveryUrlsSize;
    clone->discoveryUrls = (char **) calloc(config->discoveryUrlsSize, sizeof(char *));
    if (!clone->discoveryUrls)
    {
        goto ERROR;
    }

    for (size_t i = 0; i < clone->discoveryUrlsSize; ++i)
    {
        if (config->discoveryUrls[i])
        {
            clone->discoveryUrls[i] = cloneString(config->discoveryUrls[i]);
            if (!clone->discoveryUrls[i])
            {
                goto ERROR;
            }
        }
    }

    return clone;

    ERROR: freeEdgeApplicationConfig(clone);
    return NULL;
}

void freeEdgeEndpointInfo(EdgeEndPointInfo *endpointInfo)
{
    VERIFY_NON_NULL_NR(endpointInfo);
    EdgeFree(endpointInfo->endpointUri);
    freeEdgeEndpointConfig(endpointInfo->endpointConfig);
    freeEdgeApplicationConfig(endpointInfo->appConfig);
    EdgeFree(endpointInfo->securityPolicyUri);
    EdgeFree(endpointInfo->transportProfileUri);
    EdgeFree(endpointInfo);
}

EdgeEndPointInfo *cloneEdgeEndpointInfo(EdgeEndPointInfo *endpointInfo)
{
    VERIFY_NON_NULL(endpointInfo, NULL);
    EdgeEndPointInfo *clone = (EdgeEndPointInfo *) EdgeCalloc(1, sizeof(EdgeEndPointInfo));
    VERIFY_NON_NULL(clone, NULL);
    clone->securityMode = endpointInfo->securityMode;
    clone->securityLevel = endpointInfo->securityLevel;

    if (endpointInfo->endpointUri)
    {
        clone->endpointUri = cloneString(endpointInfo->endpointUri);
        if (!clone->endpointUri)
        {
            goto ERROR;
        }
    }

    if (endpointInfo->securityPolicyUri)
    {
        clone->securityPolicyUri = cloneString(endpointInfo->securityPolicyUri);
        if (!clone->securityPolicyUri)
        {
            goto ERROR;
        }
    }

    if (endpointInfo->transportProfileUri)
    {
        clone->transportProfileUri = cloneString(endpointInfo->transportProfileUri);
        if (!clone->transportProfileUri)
        {
            goto ERROR;
        }
    }

    if (endpointInfo->endpointConfig)
    {
        clone->endpointConfig = cloneEdgeEndpointConfig(endpointInfo->endpointConfig);
        if (!clone->endpointConfig)
        {
            goto ERROR;
        }
    }

    if (endpointInfo->appConfig)
    {
        clone->appConfig = cloneEdgeApplicationConfig(endpointInfo->appConfig);
        if (!clone->appConfig)
        {
            goto ERROR;
        }
    }

    return clone;

    ERROR: freeEdgeEndpointInfo(clone);
    return NULL;
}

void freeEdgeBrowseResult(EdgeBrowseResult *browseResult, int browseResultLength)
{
    VERIFY_NON_NULL_NR(browseResult);

    for (size_t i = 0; i < browseResultLength; ++i)
    {
        EdgeFree(browseResult[i].browseName);
    }
    EdgeFree(browseResult);
}

void freeEdgeResult(EdgeResult *res)
{
    VERIFY_NON_NULL_NR(res);
    EdgeFree(res);
}

void freeEdgeNodeId(EdgeNodeId *nodeId)
{
    VERIFY_NON_NULL_NR(nodeId);
    EdgeFree(nodeId->nodeUri);
    EdgeFree(nodeId->nodeId);
    EdgeFree(nodeId);
}

void freeEdgeNodeInfo(EdgeNodeInfo *nodeInfo)
{
    VERIFY_NON_NULL_NR(nodeInfo);
    EdgeFree(nodeInfo->methodName);
    freeEdgeNodeId(nodeInfo->nodeId);
    EdgeFree(nodeInfo->valueAlias);
    EdgeFree(nodeInfo);
}

void freeEdgeArgument(EdgeArgument *arg)
{
    VERIFY_NON_NULL_NR(arg);
    if(SCALAR == arg->valType)
    {
        EdgeFree(arg->scalarValue);
    }
    else if(ARRAY_1D == arg->valType)
    {
        if(String == arg->argType)
        {
            char **val = (char **) arg->arrayData;
            for (size_t i = 0; i < arg->arrayLength; ++i)
            {
                EdgeFree(val[i]);
            }
            EdgeFree(val);
        }
        else
        {
            EdgeFree(arg->arrayData);
        }
    }
    EdgeFree(arg);
}

void freeEdgeMethodRequestParams(EdgeMethodRequestParams *methodParams)
{
    VERIFY_NON_NULL_NR(methodParams);
    for (size_t i = 0; i < methodParams->num_inpArgs; ++i)
    {
        freeEdgeArgument(methodParams->inpArg[i]);
    }
    EdgeFree(methodParams->inpArg);

    for (size_t i = 0; i < methodParams->num_outArgs; ++i)
    {
        freeEdgeArgument(methodParams->outArg[i]);
    }
    EdgeFree(methodParams->outArg);
    EdgeFree(methodParams);
}

void freeEdgeRequest(EdgeRequest *req)
{
    VERIFY_NON_NULL_NR(req);
    EdgeFree(req->value);
    EdgeFree(req->subMsg);
    freeEdgeMethodRequestParams(req->methodParams);
    freeEdgeNodeInfo(req->nodeInfo);
    EdgeFree(req);
}

void freeEdgeRequests(EdgeRequest **requests, int requestLength)
{
    VERIFY_NON_NULL_NR(requests);
    for (size_t i = 0; i < requestLength; ++i)
    {
        freeEdgeRequest(requests[i]);
    }
    EdgeFree(requests);
}

void freeEdgeVersatility(EdgeVersatility *versatileValue)
{
    VERIFY_NON_NULL_NR(versatileValue);
    EdgeFree(versatileValue->value);
    EdgeFree(versatileValue);
}

void freeEdgeVersatilityByType(EdgeVersatility *versatileValue, EdgeNodeIdentifier type)
{
    VERIFY_NON_NULL_NR(versatileValue);

    if (versatileValue->isArray && (type == String || type == ByteString || type == Guid))
    {
        // Free String array
        char **values = versatileValue->value;
        if (values)
        {
            for (int j = 0; j < versatileValue->arrayLength; j++)
            {
                EdgeFree(values[j]);
            }
            EdgeFree(values);
        }
    }
    else
    {
        EdgeFree(versatileValue->value);
    }

    EdgeFree(versatileValue);
}

void freeEdgeDiagnosticInfo(EdgeDiagnosticInfo *info)
{
    VERIFY_NON_NULL_NR(info);
    EdgeFree(info->additionalInfo);
    EdgeFree(info->innerDiagnosticInfo);
    EdgeFree(info->msg);
    EdgeFree(info);
}

void freeEdgeResponse(EdgeResponse *response)
{
    VERIFY_NON_NULL_NR(response);
    if(IS_NOT_NULL(response->message))
        freeEdgeVersatilityByType(response->message, response->type);

    if(IS_NOT_NULL(response->value))
        EdgeFree(response->value);

    if(IS_NOT_NULL(response->nodeInfo))
        freeEdgeNodeInfo(response->nodeInfo);

    if(IS_NOT_NULL(response->result))
        EdgeFree(response->result);

    if(IS_NOT_NULL(response->m_diagnosticInfo))
        freeEdgeDiagnosticInfo(response->m_diagnosticInfo);

    EdgeFree(response);
}

void freeEdgeResponses(EdgeResponse **responses, int responseLength)
{
    VERIFY_NON_NULL_NR(responses);
    for (size_t i = 0; i < responseLength; ++i)
    {
        freeEdgeResponse(responses[i]);
    }
    EdgeFree(responses);
}

void freeEdgeMessage(EdgeMessage *msg)
{
    VERIFY_NON_NULL_NR(msg);
    if(IS_NOT_NULL(msg->endpointInfo))
        freeEdgeEndpointInfo(msg->endpointInfo);

    if(IS_NOT_NULL(msg->request))
        freeEdgeRequest(msg->request);

    if(IS_NOT_NULL(msg->requests))
        freeEdgeRequests(msg->requests, msg->requestLength);

    if(IS_NOT_NULL(msg->responses))
        freeEdgeResponses(msg->responses, msg->responseLength);

    if(IS_NOT_NULL(msg->result))
        EdgeFree(msg->result);

    if(IS_NOT_NULL(msg->browseParam))
        EdgeFree(msg->browseParam);

    if(IS_NOT_NULL(msg->browseResult))
        freeEdgeBrowseResult(msg->browseResult, msg->browseResultLength);

    if(IS_NOT_NULL(msg->cpList))
        freeEdgeContinuationPointList(msg->cpList);

    EdgeFree(msg);
}

EdgeResult *createEdgeResult(EdgeStatusCode code)
{
    EdgeResult *result = (EdgeResult *) EdgeCalloc(1, sizeof(EdgeResult));
    if (!result)
    {
        return NULL;
    }
    result->code = code;
    return result;
}

bool isNodeClassValid(UA_NodeClass nodeClass)
{
    bool valid = true;
    switch (nodeClass)
    {
        case UA_NODECLASS_UNSPECIFIED:
        case UA_NODECLASS_OBJECT:
        case UA_NODECLASS_VARIABLE:
        case UA_NODECLASS_METHOD:
        case UA_NODECLASS_OBJECTTYPE:
        case UA_NODECLASS_VARIABLETYPE:
        case UA_NODECLASS_REFERENCETYPE:
        case UA_NODECLASS_DATATYPE:
        case UA_NODECLASS_VIEW:
            valid = true;
            break;
        default:
            valid = false;
            break;
    }
    return valid;
}

EdgeNodeId *cloneEdgeNodeId(EdgeNodeId *nodeId)
{
    if (!nodeId)
    {
        return NULL;
    }

    EdgeNodeId *clone = (EdgeNodeId *) EdgeCalloc(1, sizeof(EdgeNodeId));
    if (!clone)
    {
        return NULL;
    }

    clone->nameSpace = nodeId->nameSpace;
    if (nodeId->nodeUri)
    {
        clone->nodeUri = cloneString(nodeId->nodeUri);
        if (!clone->nodeUri)
        {
            EdgeFree(clone);
            return NULL;
        }
    }
    clone->nodeIdentifier = nodeId->nodeIdentifier;
    clone->type = nodeId->type;
    if (nodeId->nodeId)
    {
        clone->nodeId = cloneString(nodeId->nodeId);
        if (!clone->nodeId)
        {
            EdgeFree(clone->nodeUri);
            EdgeFree(clone);
            return NULL;
        }
    }
    clone->integerNodeId = nodeId->integerNodeId;

    return clone;
}

EdgeNodeInfo *cloneEdgeNodeInfo(EdgeNodeInfo *nodeInfo)
{
    if (!nodeInfo)
    {
        return NULL;
    }

    EdgeNodeInfo *clone = (EdgeNodeInfo *) EdgeCalloc(1, sizeof(EdgeNodeInfo));
    if (!clone)
    {
        return NULL;
    }

    if (nodeInfo->methodName)
    {
        clone->methodName = cloneString(nodeInfo->methodName);
        if (!clone->methodName)
        {
            EdgeFree(clone);
            return NULL;
        }
    }

    if (nodeInfo->nodeId)
    {
        clone->nodeId = cloneEdgeNodeId(nodeInfo->nodeId);
        if (!clone->nodeId)
        {
            EdgeFree(clone->methodName);
            EdgeFree(clone);
            return NULL;
        }
    }

    if (nodeInfo->valueAlias)
    {
        clone->valueAlias = cloneString(nodeInfo->valueAlias);
        if (!clone->valueAlias)
        {
            freeEdgeNodeId(clone->nodeId);
            EdgeFree(clone->methodName);
            EdgeFree(clone);
            return NULL;
        }
    }

    return clone;
}

size_t get_size(EdgeNodeIdentifier type, bool isArray)
{
    size_t size = -1;
    switch (type)
    {
        case Boolean:
            {
                size = (isArray) ? sizeof(bool*) : sizeof(bool);
            }
            break;
        case SByte:
            {
                size = (isArray) ? sizeof(int8_t*) : sizeof(int8_t);
            }
            break;
        case Byte:
            {
                size = (isArray) ? sizeof(uint8_t*) : sizeof(uint8_t);
            }
            break;
        case Int16:
            {
                size = (isArray) ? sizeof(int16_t*) : sizeof(int16_t);
            }
            break;
        case UInt16:
            {
                size = (isArray) ? sizeof(uint16_t*) : sizeof(uint16_t);
            }
            break;
        case Int32:
            {
                size = (isArray) ? sizeof(int32_t*) : sizeof(int32_t);
            }
            break;
        case UInt32:
            {
                size = (isArray) ? sizeof(uint32_t*) : sizeof(uint32_t);
            }
            break;
        case Int64:
            {
                size = (isArray) ? sizeof(int64_t*) : sizeof(int64_t);
            }
            break;
        case UInt64:
            {
                size = (isArray) ? sizeof(uint64_t*) : sizeof(uint64_t);
            }
            break;
        case Float:
            {
                size = (isArray) ? sizeof(float*) : sizeof(float);
            }
            break;
        case Double:
            {
                size = (isArray) ? sizeof(double*) : sizeof(double);
            }
            break;
        case String:
            {
                size = (isArray) ? sizeof(char*) : sizeof(char);
            }
            break;
        default:
            {
                size = -1;
            }
            break;
    }
    return size;
}


EdgeMessage* cloneEdgeMessage(EdgeMessage *msg)
{
    if (!msg)
    {
        return NULL;
    }

    EdgeMessage *clone = (EdgeMessage *)EdgeCalloc(1, sizeof(EdgeMessage));
    if (!clone)
    {
        return NULL;
    }

    clone->type = msg->type;
    clone->command = msg->command;
    if(IS_NOT_NULL(msg->endpointInfo))
    {
        clone->endpointInfo = cloneEdgeEndpointInfo(msg->endpointInfo);
        if(IS_NULL(clone->endpointInfo))
        {
            goto ERROR;
        }
    }

    clone->requestLength = msg->requestLength;
    clone->message_id = msg->message_id;

    if (msg->browseParam)
    {
        clone->browseParam = (EdgeBrowseParameter *) EdgeCalloc(1, sizeof(EdgeBrowseParameter));
        if(IS_NULL(clone->browseParam))
        {
            goto ERROR;
        }
        clone->browseParam->direction = msg->browseParam->direction;
        clone->browseParam->maxReferencesPerNode = msg->browseParam->maxReferencesPerNode;
    }

    if (msg->cpList)
    {
        clone->cpList = (EdgeContinuationPointList *)EdgeCalloc(1, sizeof(EdgeContinuationPointList));
        if(IS_NULL(clone->cpList))
        {
            goto ERROR;
        }

        clone->cpList->count = msg->cpList->count;
        clone->cpList->cp =  (EdgeContinuationPoint **)EdgeCalloc(msg->cpList->count, sizeof(EdgeContinuationPoint *));
        if(IS_NULL(clone->cpList->cp))
        {
            goto ERROR;
        }

        for (size_t  i = 0; i < msg->cpList->count; i++)
        {
            clone->cpList->cp[i] = (EdgeContinuationPoint *)EdgeCalloc(1, sizeof(EdgeContinuationPoint));
            if(IS_NULL(clone->cpList->cp[i]))
            {
                goto ERROR;
            }

            clone->cpList->cp[i]->length = msg->cpList->cp[i]->length;
            clone->cpList->cp[i]->continuationPoint = (unsigned char *) cloneData(msg->cpList->cp[i]->continuationPoint,
                                                                                  strlen((char *) msg->cpList->cp[i]->continuationPoint) + 1);
            if(IS_NULL(clone->cpList->cp[i]->continuationPoint))
            {
                goto ERROR;
            }
        }
    }

    if (msg->type == SEND_REQUEST)
    {
        if (msg->request)
        {
            clone->request = (EdgeRequest*) EdgeCalloc(1, sizeof(EdgeRequest));
            if(IS_NULL(clone->request))
            {
                goto ERROR;
            }

            if (msg->request->nodeInfo)
            {
                clone->request->nodeInfo = cloneEdgeNodeInfo(msg->request->nodeInfo);
                if(IS_NULL(clone->request->nodeInfo))
                {
                    goto ERROR;
                }
            }

            if (msg->request->subMsg)
            {
                clone->request->subMsg = cloneSubRequest(msg->request->subMsg);
                if(IS_NULL(clone->request->subMsg))
                {
                    goto ERROR;
                }
            }

            if (msg->request->methodParams)
            {
                clone->request->methodParams = cloneEdgeMethodRequestParams(msg->request->methodParams);
                if(IS_NULL(clone->request->methodParams))
                {
                    goto ERROR;
                }
            }
        }
    }

    if (msg->type == SEND_REQUESTS)
    {
        clone->requests = (EdgeRequest**) EdgeCalloc(msg->requestLength, sizeof(EdgeRequest*));
        if(IS_NULL(clone->requests))
        {
            goto ERROR;
        }

        for (size_t i = 0; i < msg->requestLength; i++)
        {
            clone->requests[i] = (EdgeRequest*) EdgeCalloc(1, sizeof(EdgeRequest));
            if(IS_NULL(clone->requests[i]))
            {
                goto ERROR;
            }

            if (msg->requests[i]->nodeInfo)
            {
                clone->requests[i]->nodeInfo = cloneEdgeNodeInfo(msg->requests[i]->nodeInfo);
                if(IS_NULL(clone->requests[i]->nodeInfo))
                {
                    goto ERROR;
                }
            }

            if (msg->command == CMD_WRITE)
            {
                clone->requests[i]->type = msg->requests[i]->type;
                // EdgeVersatility
                if (msg->requests[i]->value)
                {
                    EdgeVersatility *srcVersatility = (EdgeVersatility *) msg->requests[i]->value;
                    clone->requests[i]->value = (EdgeVersatility*) EdgeCalloc(1, sizeof(EdgeVersatility));
                    if(IS_NULL(clone->requests[i]->value))
                    {
                        goto ERROR;
                    }

                    EdgeVersatility* cloneVersatility = (EdgeVersatility*) clone->requests[i]->value;
                    if (srcVersatility->isArray == false)
                    {
                        // Scalar
                        void *val = srcVersatility->value;

                        cloneVersatility->arrayLength = srcVersatility->arrayLength;
                        cloneVersatility->isArray = srcVersatility->isArray;
                        size_t size = get_size(msg->requests[i]->type, srcVersatility->isArray);
                        if (msg->requests[i]->type == String)
                        {
                            size_t len = strlen((char *) srcVersatility->value);
                            cloneVersatility->value = (void *) EdgeCalloc(1, len+1);
                            if(IS_NULL(cloneVersatility->value))
                            {
                                goto ERROR;
                            }

                            strncpy(cloneVersatility->value, (char*) srcVersatility->value, len+1);
                        }
                        else
                        {
                            cloneVersatility->value = (void *) EdgeCalloc(1, size);
                            if(IS_NULL(cloneVersatility->value))
                            {
                                goto ERROR;
                            }
                            memcpy(cloneVersatility->value, val, size);
                        }
                    }
                    else
                    {
                        // Array
                        void *val = srcVersatility->value;

                        cloneVersatility->arrayLength = srcVersatility->arrayLength;
                        cloneVersatility->isArray = srcVersatility->isArray;
                        size_t size = get_size(msg->requests[i]->type, srcVersatility->isArray);
                        if (msg->requests[i]->type == String || msg->requests[i]->type == ByteString)
                        {
                            char **srcVal = (char**) srcVersatility->value;
                            cloneVersatility->value = EdgeCalloc(srcVersatility->arrayLength, sizeof(char*));
                            if(IS_NULL(cloneVersatility->value))
                            {
                                goto ERROR;
                            }

                            char **dstVal = (char **) cloneVersatility->value;
                            size_t len;
                            for (int j = 0; j < srcVersatility->arrayLength; j++)
                            {
                                len = strlen(srcVal[j]);
                                dstVal[j] = (char*) EdgeCalloc(1, sizeof(char) * (len+1));
                                if(IS_NULL(dstVal[j]))
                                {
                                    goto ERROR;
                                }
                                strncpy(dstVal[j], srcVal[j], len+1);
                            }
                        }
                        else
                        {
                            cloneVersatility->value = EdgeCalloc(srcVersatility->arrayLength, size);
                            if(IS_NULL(cloneVersatility->value))
                            {
                                goto ERROR;
                            }
                            memcpy(cloneVersatility->value, val, get_size(msg->requests[i]->type, false) * srcVersatility->arrayLength);
                        }
                    }
                }
            }
            else if (msg->command == CMD_SUB)
            {
                if(IS_NOT_NULL(msg->requests[i]->subMsg))
                {
                    clone->requests[i]->subMsg = cloneSubRequest(msg->requests[i]->subMsg);
                    if(IS_NULL(clone->requests[i]->subMsg))
                    {
                        goto ERROR;
                    }
                }
            }
        }
    }

    return clone;

ERROR:
    freeEdgeMessage(clone);
    return NULL;
}

EdgeNodeIdType getEdgeNodeIdType(char type)
{
    EdgeNodeIdType edgeNodeType = INTEGER;
    switch (type)
    {
        case 'N':
            edgeNodeType = INTEGER;
            break;
        case 'S':
            edgeNodeType = STRING;
            break;
        case 'B':
            edgeNodeType = BYTESTRING;
            break;
        case 'G':
            edgeNodeType = UUID;
            break;
        default:
            break;
    }
    return edgeNodeType;
}

char getCharacterNodeIdType(uint32_t type)
{
    char nodeType;
    char nodeTypeArray[4] =
    { 'N', 'S', 'B', 'G' };
    switch (type)
    {
        case UA_NODEIDTYPE_NUMERIC:
            nodeType = nodeTypeArray[0];
            break;
        case UA_NODEIDTYPE_STRING:
            nodeType = nodeTypeArray[1];
            break;
        case UA_NODEIDTYPE_BYTESTRING:
            nodeType = nodeTypeArray[2];
            break;
        case UA_NODEIDTYPE_GUID:
            nodeType = nodeTypeArray[3];
            break;
        default:
            break;
    }
    return nodeType;
}

// USAGE

/*
 edgeMap* X = createMap();

 insert(X, 10, "arya");
 insert(X, 20, "mango");
 insert(X, 25, "apple");

 char* ret = (char *)get(X, 25);

 deleteMap(X);
 */
