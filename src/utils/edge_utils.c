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
#include <sys/time.h>
#include <time.h>

#include "edge_open62541.h"
#include "edge_logger.h"
#include "edge_malloc.h"

#define TAG "edge_utils"

void logCurrentTimeStamp()
{
#if DEBUG
    struct timeval curTime;
    gettimeofday(&curTime, NULL);

    char buffer[15];
    strftime(buffer, sizeof(buffer), "%m/%d %H:%M:%S", localtime(&curTime.tv_sec));
    EDGE_LOG_V(TAG, "Current time: %s.%06d\n", buffer, (int)(curTime.tv_usec));
#endif
}

char *cloneString(const char *str)
{
    VERIFY_NON_NULL_MSG(str, "NULL str param in clonseString\n", NULL);
    size_t len = strlen(str);
    char *clone = (char *) EdgeMalloc(len + 1);
    VERIFY_NON_NULL_MSG(clone, "EdgeMalloc FAILED for cloneString\n", NULL);
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
    VERIFY_NON_NULL_MSG(cloned, "EdgeMalloc failed for cloneData\n", NULL);
    memcpy(cloned, src, lenInbytes);
    return cloned;
}

void freeEdgeNodeIdType(Edge_NodeId *id)
{
    VERIFY_NON_NULL_NR_MSG(id, "Input param is NULL");
    if(id->identifierType == STRING)
    {
        EdgeFree(id->identifier.string.data);
    }
    else if(id->identifierType == BYTESTRING)
    {
        EdgeFree(id->identifier.byteString.data);
    }
    EdgeFree(id);
}

void freeEdgeEndpointConfig(EdgeEndpointConfig *config)
{
    VERIFY_NON_NULL_NR_MSG(config, "NULL config param in freeEdgeEndpointConfig\n");
    EdgeFree(config->serverName);
    EdgeFree(config->bindAddress);
    EdgeFree(config);
}

void freeEdgeApplicationConfigMembers(EdgeApplicationConfig *config)
{
    VERIFY_NON_NULL_NR_MSG(config, "NULL config param in freeEdgeApplicationConfigMembers\n");
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
    VERIFY_NON_NULL_NR_MSG(config, "NULL config param in freeEdgeApplicationConfig\n");
    freeEdgeApplicationConfigMembers(config);
    EdgeFree(config);
}

void freeEdgeDevice(EdgeDevice *dev)
{
    VERIFY_NON_NULL_NR_MSG(dev, "NULL edgeDevice in freeEdgeDevice\n");
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
    VERIFY_NON_NULL_MSG(subReq, "NULL sub request param in cloneSubRequest\n", NULL);
    EdgeSubRequest *clone = (EdgeSubRequest *)EdgeCalloc(1, sizeof(EdgeSubRequest));
    VERIFY_NON_NULL_MSG(clone, "EdgeCalloc FAILED in cloneSubRequest\n", NULL);

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

EdgeEndpointConfig *cloneEdgeEndpointConfig(EdgeEndpointConfig *config)
{
    VERIFY_NON_NULL_MSG(config, "NULL config patram in cloneEdgeEndpointConfig\n", NULL);
    EdgeEndpointConfig *clone = (EdgeEndpointConfig *) EdgeCalloc(1, sizeof(EdgeEndpointConfig));
    VERIFY_NON_NULL_MSG(clone, "EdgeCallc failed for clone in cloneEdgeEndpointConfig\n", NULL);
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
    VERIFY_NON_NULL_MSG(config, "NULL cofig patram in cloneEdgeApplicationConfig\n", NULL);
    EdgeApplicationConfig *clone = (EdgeApplicationConfig *) EdgeCalloc(1,
            sizeof(EdgeApplicationConfig));
    VERIFY_NON_NULL_MSG(clone, "EdgeCalloc FAILED for clone in cloneEdgeApplicationConfig\n", NULL);
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
    clone->discoveryUrls = (char **) EdgeCalloc(config->discoveryUrlsSize, sizeof(char *));
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
    VERIFY_NON_NULL_NR_MSG(endpointInfo, "NULL param endpointinfo in cloneEdgeEndpointInfo\n");
    EdgeFree(endpointInfo->endpointUri);
    freeEdgeEndpointConfig(endpointInfo->endpointConfig);
    freeEdgeApplicationConfig(endpointInfo->appConfig);
    EdgeFree(endpointInfo->securityPolicyUri);
    EdgeFree(endpointInfo->transportProfileUri);
    EdgeFree(endpointInfo);
}

EdgeEndPointInfo *cloneEdgeEndpointInfo(EdgeEndPointInfo *endpointInfo)
{
    VERIFY_NON_NULL_MSG(endpointInfo, "NULL param endpointinfo in cloneEdgeEndpointInfo\n", NULL);
    EdgeEndPointInfo *clone = (EdgeEndPointInfo *) EdgeCalloc(1, sizeof(EdgeEndPointInfo));
    VERIFY_NON_NULL_MSG(clone, "EdgeCalloc failed for clone in cloneEdgeEndpointInfo\n", NULL);
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
    VERIFY_NON_NULL_NR_MSG(browseResult, "NULL param browse result in freeEdgeBrowseResult\n");

    for (size_t i = 0; i < browseResultLength; ++i)
    {
        EdgeFree(browseResult[i].browseName);
    }
    EdgeFree(browseResult);
}

void freeEdgeResult(EdgeResult *res)
{
    VERIFY_NON_NULL_NR_MSG(res, "NULL param result in freeEdgeResult\n");
    EdgeFree(res);
}

void freeEdgeNodeId(EdgeNodeId *nodeId)
{
    VERIFY_NON_NULL_NR_MSG(nodeId, "NULL param node id in free edge node id\n");
    EdgeFree(nodeId->nodeUri);
    EdgeFree(nodeId->nodeId);
    EdgeFree(nodeId);
}

void freeEdgeNodeInfo(EdgeNodeInfo *nodeInfo)
{
    VERIFY_NON_NULL_NR_MSG(nodeInfo, "NULL param nodeinfo in freeEdgeNodeInfo\n");
    EdgeFree(nodeInfo->methodName);
    freeEdgeNodeId(nodeInfo->nodeId);
    EdgeFree(nodeInfo->valueAlias);
    EdgeFree(nodeInfo);
}

void freeEdgeMethodRequestParams(EdgeMethodRequestParams *methodParams)
{
    VERIFY_NON_NULL_NR_MSG(methodParams, "NULL method params in freeEdgeMethodRequestParams\n");
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
    VERIFY_NON_NULL_NR_MSG(req, "NULL param request in freeEdgeRequest\n");
    EdgeFree(req->value);
    EdgeFree(req->subMsg);
    freeEdgeMethodRequestParams(req->methodParams);
    freeEdgeNodeInfo(req->nodeInfo);
    EdgeFree(req);
}

void freeEdgeRequests(EdgeRequest **requests, int requestLength)
{
    VERIFY_NON_NULL_NR_MSG(requests, "NULL param requests in freeEdgeRequests\n");
    for (size_t i = 0; i < requestLength; ++i)
    {
        freeEdgeRequest(requests[i]);
    }
    EdgeFree(requests);
}

void freeEdgeVersatility(EdgeVersatility *versatileValue)
{
    VERIFY_NON_NULL_NR_MSG(versatileValue, "NULL param versatileValue in freeEdgeVersatility\n");
    EdgeFree(versatileValue->value);
    EdgeFree(versatileValue);
}

void freeEdgeQualifiedName(Edge_QualifiedName *qn)
{
    VERIFY_NON_NULL_NR_MSG(qn, "Input argument is NULL");
    EdgeFree(qn->name.data);
    EdgeFree(qn);
}

void freeEdgeLocalizedText(Edge_LocalizedText *lt)
{
    VERIFY_NON_NULL_NR_MSG(lt, "Input argument is NULL");
    EdgeFree(lt->locale.data);
    EdgeFree(lt->text.data);
    EdgeFree(lt);
}

void freeEdgeDiagnosticInfo(EdgeDiagnosticInfo *info)
{
    VERIFY_NON_NULL_NR_MSG(info, "NULL param EdgeDiagnosticInfo in freeEdgeDiagnosticInfo\n");
    EdgeFree(info->additionalInfo);
    EdgeFree(info->innerDiagnosticInfo);
    EdgeFree(info->msg);
    EdgeFree(info);
}

void freeEdgeResponse(EdgeResponse *response)
{
    VERIFY_NON_NULL_NR_MSG(response, "NULL param EdgeResponse in freeEdgeResponse\n");
    if(IS_NOT_NULL(response->message))
        freeEdgeVersatilityByType(response->message, response->type);

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
    VERIFY_NON_NULL_NR_MSG(responses, "NULL param responses in freeEdgeResponses\n");
    for (size_t i = 0; i < responseLength; ++i)
    {
        freeEdgeResponse(responses[i]);
    }
    EdgeFree(responses);
}

void freeEdgeMessage(EdgeMessage *msg)
{
    VERIFY_NON_NULL_NR_MSG(msg, "NULL param EdgeMessage in freeEdgeMessage\n");
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

    EdgeFree(msg);
}

EdgeResult *createEdgeResult(EdgeStatusCode code)
{
    EdgeResult *result = (EdgeResult *) EdgeCalloc(1, sizeof(EdgeResult));
    VERIFY_NON_NULL_MSG(result, "EdgeCalloc failed for EdgeResult in createEdgeResult\n", NULL);
    result->code = code;
    return result;
}

EdgeNodeId *cloneEdgeNodeId(EdgeNodeId *nodeId)
{
    VERIFY_NON_NULL_MSG(nodeId, "NULL param nodeID in cloneEdgeNodeId\n", NULL);
    EdgeNodeId *clone = (EdgeNodeId *) EdgeCalloc(1, sizeof(EdgeNodeId));
    VERIFY_NON_NULL_MSG(clone, "EdgeCAlloc FAILED for clone in cloneEdgeNodeId\n", NULL);

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
    VERIFY_NON_NULL_MSG(nodeInfo, "NULL param nodeinfo in cloneEdgeNodeInfo\n", NULL);
    EdgeNodeInfo *clone = (EdgeNodeInfo *) EdgeCalloc(1, sizeof(EdgeNodeInfo));
    VERIFY_NON_NULL_MSG(clone, "EdgeCalloc FAILED for clone in cloneEdgeNodeInfo\n", NULL);

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
            // Default block is empty because of the assignment statement
            // before switch block which treats INTEGER as a default type.
            break;
    }
    return edgeNodeType;
}
