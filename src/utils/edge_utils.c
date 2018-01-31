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

#include "edge_logger.h"
#include "edge_malloc.h"

#define TAG "edge_utils"

edgeMap *createMap()
{
    edgeMap *map = (edgeMap *) malloc(sizeof(edgeMap));
    VERIFY_NON_NULL(map, NULL);
    map->head = NULL;
    return map;
}

void insertMapElement(edgeMap *map, keyValue key, keyValue value)
{
    edgeMapNode *node = (edgeMapNode *) malloc(sizeof(edgeMapNode));
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
        FREE(temp);
        temp = xtemp;
    }

    map->head = NULL;
}

static List *createListNode(void *data)
{
    List *node = (List *)calloc(1, sizeof(List));
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
    if(IS_NULL(ptr))
    {
        return 0;
    }

    int size = 0;
    while(ptr)
    {
        size++;
        ptr=ptr->link;
    }
    return size;
}

void deleteListNode(List **head, void *data)
{
    if (!head || !data)
    {
        return;
    }

    List *ptr = *head, *prev = NULL;
    while (ptr && ptr->data != data)
    {
        prev = ptr;
        ptr = ptr->link;
    }

    if (!ptr)
    {
        return;
    }

    if (prev)
    {
        prev->link = ptr->link;
    }
    else
    {
        *head = ptr->link;
    }

    ptr->link = NULL;
    FREE(ptr);
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
        FREE(ptr);
        ptr = next;
    }
    *head = NULL;
}

char *cloneString(const char *str)
{
    VERIFY_NON_NULL(str, NULL);
    int len = strlen(str);
    char *clone = (char *)malloc(len + 1);
    VERIFY_NON_NULL(clone, NULL);
    memcpy(clone, str, len + 1);
    return clone;
}

void *cloneData(const void *src, int lenInbytes)
{
    if(!src || lenInbytes < 1)
    {
        return NULL;
    }

    void *cloned = malloc(lenInbytes);
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

    char *str = (char *)malloc(uaStr->length + 1);
    VERIFY_NON_NULL(str, NULL);
    memcpy(str, uaStr->data, uaStr->length);
    str[uaStr->length] = '\0';
    return str;
}

void freeEdgeEndpointConfig(EdgeEndpointConfig *config)
{
    VERIFY_NON_NULL_NR(config);
    FREE(config->serverName);
    FREE(config->bindAddress);
    FREE(config);
}

void freeEdgeApplicationConfig(EdgeApplicationConfig *config)
{
    VERIFY_NON_NULL_NR(config);
    FREE(config->applicationUri);
    FREE(config->productUri);
    FREE(config->applicationName);
    FREE(config->gatewayServerUri);
    FREE(config->discoveryProfileUri);
    for(int i = 0; i < config->discoveryUrlsSize; ++i)
    {
        FREE(config->discoveryUrls[i]);
    }
    FREE(config->discoveryUrls);
    FREE(config);
}

void freeEdgeContinuationPoint(EdgeContinuationPoint *cp)
{
    VERIFY_NON_NULL_NR(cp);
    FREE(cp->continuationPoint);
    FREE(cp);
}

void freeEdgeContinuationPointList(EdgeContinuationPointList *cpList)
{
    VERIFY_NON_NULL_NR(cpList);
    for (int i = 0; i < cpList->count; ++i)
    {
        freeEdgeContinuationPoint(cpList->cp[i]);
    }

    FREE(cpList->cp);
    FREE(cpList);
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
    FREE(dev->endpointsInfo);
    FREE(dev->address);
    FREE(dev->serverName);
    FREE(dev);
}

EdgeEndpointConfig *cloneEdgeEndpointConfig(EdgeEndpointConfig *config)
{
    VERIFY_NON_NULL(config, NULL);
    EdgeEndpointConfig *clone = (EdgeEndpointConfig *)calloc(1, sizeof(EdgeEndpointConfig));
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

ERROR:
    freeEdgeEndpointConfig(clone);
    return NULL;
}

EdgeApplicationConfig *cloneEdgeApplicationConfig(EdgeApplicationConfig *config)
{
    VERIFY_NON_NULL(config, NULL);
    EdgeApplicationConfig *clone = (EdgeApplicationConfig *)calloc(1, sizeof(EdgeApplicationConfig));
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
    clone->discoveryUrls = (char **)calloc(config->discoveryUrlsSize, sizeof(char *));
    if(!clone->discoveryUrls)
    {
        goto ERROR;
    }

    for(int i = 0; i < clone->discoveryUrlsSize; ++i)
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

ERROR:
    freeEdgeApplicationConfig(clone);
    return NULL;
}

void freeEdgeEndpointInfo(EdgeEndPointInfo *endpointInfo)
{
    VERIFY_NON_NULL_NR(endpointInfo);
    FREE(endpointInfo->endpointUri);
    freeEdgeEndpointConfig(endpointInfo->endpointConfig);
    freeEdgeApplicationConfig(endpointInfo->appConfig);
    FREE(endpointInfo->securityPolicyUri);
    FREE(endpointInfo->transportProfileUri);
    FREE(endpointInfo);
}

EdgeEndPointInfo *cloneEdgeEndpointInfo(EdgeEndPointInfo *endpointInfo)
{
    VERIFY_NON_NULL(endpointInfo, NULL);
    EdgeEndPointInfo *clone = (EdgeEndPointInfo *)calloc(1, sizeof(EdgeEndPointInfo));
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

ERROR:
    freeEdgeEndpointInfo(clone);
    return NULL;
}

void freeEdgeBrowseResult(EdgeBrowseResult *browseResult, int browseResultLength)
{
    VERIFY_NON_NULL_NR(browseResult);
    
    for (int i = 0; i < browseResultLength; ++i)
    {
        FREE(browseResult[i].browseName);
    }
    FREE(browseResult);
}

void freeEdgeNodeId(EdgeNodeId *nodeId)
{
    VERIFY_NON_NULL_NR(nodeId);
    FREE(nodeId->nodeUri);
    FREE(nodeId->nodeId);
    FREE(nodeId);
}

void freeEdgeNodeInfo(EdgeNodeInfo *nodeInfo)
{
    VERIFY_NON_NULL_NR(nodeInfo);
    FREE(nodeInfo->methodName);
    freeEdgeNodeId(nodeInfo->nodeId);
    FREE(nodeInfo->valueAlias);
    FREE(nodeInfo);
}

void freeEdgeArgument(EdgeArgument *arg)
{
    VERIFY_NON_NULL_NR(arg);
    FREE(arg->scalarValue);
    FREE(arg->arrayData);
    FREE(arg);
}

void freeEdgeMethodRequestParams(EdgeMethodRequestParams *methodParams)
{
    VERIFY_NON_NULL_NR(methodParams);
    for (int i = 0; i < methodParams->num_inpArgs; ++i)
    {
        freeEdgeArgument(methodParams->inpArg[i]);
    }
    FREE(methodParams->inpArg);

    for (int i = 0; i < methodParams->num_outArgs; ++i)
    {
        freeEdgeArgument(methodParams->outArg[i]);
    }
    FREE(methodParams->outArg);
    FREE(methodParams);
}

void freeEdgeRequest(EdgeRequest *req)
{
    VERIFY_NON_NULL_NR(req);
    FREE(req->value);
    FREE(req->subMsg);
    freeEdgeMethodRequestParams(req->methodParams);
    freeEdgeNodeInfo(req->nodeInfo);
    FREE(req);
}

void freeEdgeRequests(EdgeRequest **requests, int requestLength)
{
   VERIFY_NON_NULL_NR(requests);
    for (int i = 0; i < requestLength; ++i)
    {
        freeEdgeRequest(requests[i]);
    }
    FREE(requests);
}

void freeEdgeVersatility(EdgeVersatility *versatileValue)
{
    VERIFY_NON_NULL_NR(versatileValue);
    FREE(versatileValue->value);
    FREE(versatileValue);
}

void freeEdgeDiagnosticInfo(EdgeDiagnosticInfo *info)
{
    VERIFY_NON_NULL_NR(info);
    FREE(info->additionalInfo);
    FREE(info->innerDiagnosticInfo);
    FREE(info->msg);
    FREE(info);
}

void freeEdgeResponse(EdgeResponse *response)
{
    VERIFY_NON_NULL_NR(response);
    freeEdgeVersatility(response->message);
    FREE(response->value);
    freeEdgeNodeInfo(response->nodeInfo);
    FREE(response->result);
    freeEdgeDiagnosticInfo(response->m_diagnosticInfo);
    FREE(response);
}

void freeEdgeResponses(EdgeResponse **responses, int responseLength)
{
    VERIFY_NON_NULL_NR(responses);
    for (int i = 0; i < responseLength; ++i)
    {
        freeEdgeResponse(responses[i]);
    }
    FREE(responses);
}

void freeEdgeMessage(EdgeMessage *msg)
{
    VERIFY_NON_NULL_NR(msg);
    freeEdgeEndpointInfo(msg->endpointInfo);
    freeEdgeRequest(msg->request);
    freeEdgeRequests(msg->requests, msg->requestLength);
    freeEdgeResponses(msg->responses, msg->responseLength);
    FREE(msg->result);
    FREE(msg->browseParam);
    freeEdgeBrowseResult(msg->browseResult, msg->browseResultLength);
    freeEdgeContinuationPointList(msg->cpList);
    FREE(msg);
}

EdgeResult *createEdgeResult(EdgeStatusCode code)
{
    EdgeResult *result = (EdgeResult *)calloc(1, sizeof(EdgeResult));
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

    EdgeNodeId *clone = (EdgeNodeId *) calloc(1, sizeof(EdgeNodeId));
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
            FREE(clone);
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
            FREE(clone->nodeUri);
            FREE(clone);
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

    EdgeNodeInfo *clone = (EdgeNodeInfo *)calloc(1, sizeof(EdgeNodeInfo));
    if (!clone)
    {
        return NULL;
    }

    if (nodeInfo->methodName)
    {
        clone->methodName = cloneString(nodeInfo->methodName);
        if (!clone->methodName)
        {
            FREE(clone);
            return NULL;
        }
    }

    if (nodeInfo->nodeId)
    {
        clone->nodeId = cloneEdgeNodeId(nodeInfo->nodeId);
        if (!clone->nodeId)
        {
            FREE(clone->methodName);
            FREE(clone);
            return NULL;
        }
    }

    if (nodeInfo->valueAlias)
    {
        clone->valueAlias = cloneString(nodeInfo->valueAlias);
        if (!clone->valueAlias)
        {
            freeEdgeNodeId(clone->nodeId);
            FREE(clone->methodName);
            FREE(clone);
            return NULL;
        }
    }

    return clone;
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
