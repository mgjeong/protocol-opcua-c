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
#include "edge_utils.h"
#include "edge_node_type.h"
#include "edge_logger.h"

#include <stdio.h>

#define GUID_LENGTH 36
#define TAG "browse"

typedef struct BrowseMapListNode
{
    UA_NodeId nodeId;
    char *browseName;
    struct BrowseMapListNode *next;
} BrowseMapListNode;

typedef struct BrowseMap
{
    BrowseMapListNode *listHead;
    int listSize;
} BrowseMap;

static BrowseMap *initMap(int size)
{
    return calloc(size, sizeof(BrowseMap));
}

static void destroyMap(BrowseMap *map, int mapSize)
{
    for (int i = 0; i < mapSize; ++i)
    {
        BrowseMapListNode *listPtr = map[i].listHead;
        BrowseMapListNode *next = NULL;
        while (listPtr)
        {
            next = listPtr->next;
            free(listPtr->browseName);
            UA_NodeId_deleteMembers(&listPtr->nodeId);
            free(listPtr);
            listPtr = next;
        }
    }
    free(map);
}

static char *convertUAStringToString(UA_String *uaStr)
{
    if (!uaStr || uaStr->length <= 0)
    {
        return NULL;
    }

    char *str = (char *) calloc(uaStr->length + 1, sizeof(char));
    if (!str)
    {
        return NULL;
    }

    for (int i = 0; i < uaStr->length; ++i)
    {
        str[i] = uaStr->data[i];
    }
    return str;
}

static unsigned char *convertUAStringToUnsignedChar(UA_String *uaStr)
{
    if (!uaStr || uaStr->length <= 0)
    {
        return NULL;
    }

    unsigned char *str = (unsigned char *) calloc(uaStr->length, sizeof(unsigned char));
    if (!str)
    {
        return NULL;
    }

    for (int i = 0; i < uaStr->length; ++i)
    {
        str[i] = uaStr->data[i];
    }
    return str;
}

static EdgeContinuationPointList *getContinuationPointList(UA_String *uaStr)
{
    if (!uaStr)
    {
        return NULL;
    }

    EdgeContinuationPointList *cpList = (EdgeContinuationPointList *) calloc(1,
            sizeof(EdgeContinuationPoint));
    if (!cpList)
    {
        return NULL;
    }

    cpList->count = 1;
    cpList->cp = (EdgeContinuationPoint **) calloc(cpList->count, sizeof(EdgeContinuationPoint *));
    if (!cpList->cp)
    {
        freeEdgeContinuationPointList(cpList);
        return NULL;
    }

    cpList->cp[0] = (EdgeContinuationPoint *) calloc(1, sizeof(EdgeContinuationPoint));
    if (!cpList->cp[0])
    {
        freeEdgeContinuationPointList(cpList);
        return NULL;
    }

    cpList->cp[0]->continuationPoint = convertUAStringToUnsignedChar(uaStr);
    if (!cpList->cp[0]->continuationPoint)
    {
        freeEdgeContinuationPointList(cpList);
        return NULL;
    }

    cpList->cp[0]->length = uaStr->length;
    return cpList;
}

UA_ByteString *getUAStringFromEdgeContinuationPoint(EdgeContinuationPoint *cp)
{
    if (!cp || cp->length < 1)
    {
        return NULL;
    }

    UA_ByteString *byteStr = (UA_ByteString *) malloc(sizeof(UA_ByteString));
    if (!byteStr)
    {
        return NULL;
    }

    byteStr->length = cp->length;
    byteStr->data = (UA_Byte *) malloc(byteStr->length * sizeof(UA_Byte));
    if (!byteStr->data)
    {
        free(byteStr);
        return NULL;
    }

    for (int i = 0; i < byteStr->length; ++i)
    {
        byteStr->data[i] = cp->continuationPoint[i];
    }

    return byteStr;
}

static BrowseMapListNode *createBrowseMapListNode(UA_NodeId nodeId, UA_String *browseName)
{
    BrowseMapListNode *node = (BrowseMapListNode *) malloc(sizeof(BrowseMapListNode));
    if (!node)
    {
        return NULL;
    }
    if (UA_STATUSCODE_GOOD != UA_NodeId_copy(&nodeId, &node->nodeId))
    {
        free(node);
        return NULL;
    }
    node->browseName = convertUAStringToString(browseName);
    node->next = NULL;
    return node;
}

static bool addToMap(BrowseMap *map, int mapSize, int index, UA_NodeId nodeId,
        UA_String *browseName)
{
    if (!map || index >= mapSize)
    {
        return false;
    }

    BrowseMapListNode *newNode = createBrowseMapListNode(nodeId, browseName);
    if (!newNode)
    {
        return false;
    }

    newNode->next = map[index].listHead;
    map[index].listHead = newNode;
    map[index].listSize++;
    return true;
}

static bool hasNode(BrowseMap *map, int mapSize, int index, UA_NodeId nodeId)
{
    if (!map || mapSize <= 0 || index >= mapSize)
        return false;

    BrowseMapListNode *ptr = map[index].listHead;
    while (ptr)
    {
        if (UA_NodeId_equal(&nodeId, &ptr->nodeId))
        {
            return true;
        }
        ptr = ptr->next;
    }

    return false;
}

static const int BROWSE_DESCRIPTION_NODECLASS_MASK = UA_NODECLASS_OBJECT | UA_NODECLASS_VARIABLE
        | UA_NODECLASS_METHOD | UA_NODECLASS_REFERENCETYPE;

static UA_NodeId *getNodeId(EdgeRequest *req)
{
    if (!req || !req->nodeInfo || !req->nodeInfo->nodeId)
    {
        return NULL;
    }
    UA_NodeId *node = (UA_NodeId *) calloc(1, sizeof(UA_NodeId));
    if (!node)
    {
        return NULL;
    }
    if (req->nodeInfo->nodeId->type == INTEGER)
    {
        *node = UA_NODEID_NUMERIC(req->nodeInfo->nodeId->nameSpace,
                req->nodeInfo->nodeId->integerNodeId);
    }
    else if (req->nodeInfo->nodeId->type == STRING)
    {
        *node = UA_NODEID_STRING_ALLOC(req->nodeInfo->nodeId->nameSpace,
                req->nodeInfo->nodeId->nodeId);
    }
    else
    {
        *node = UA_NODEID_NUMERIC(req->nodeInfo->nodeId->nameSpace, UA_NS0ID_ROOTFOLDER);
    }
    return node;
}

static UA_NodeId *getNodeIdMultiReq(EdgeMessage *msg, int reqId)
{
    if (!msg)
    {
        return NULL;
    }
    return getNodeId(msg->requests[reqId]);
}

static EdgeNodeInfo *getEndpoint(EdgeMessage *msg, int msgId)
{
    if (msg->type == SEND_REQUEST)
    {
        return msg->request->nodeInfo;
    }
    return msg->requests[msgId]->nodeInfo;
}

static bool checkStatusGood(UA_StatusCode status)
{
    return (UA_STATUSCODE_GOOD == status) ? true : false;
}

static UA_BrowseDescription *getBrowseDescriptions(UA_NodeId *nodeIdList, int count,
        EdgeMessage *msg)
{
    if (!msg->browseParam)
    {
        return NULL;
    }

    int direct = msg->browseParam->direction;
    UA_BrowseDirection directionParam = UA_BROWSEDIRECTION_FORWARD;
    if (DIRECTION_INVERSE == direct)
    {
        directionParam = UA_BROWSEDIRECTION_INVERSE;
    }
    else if (DIRECTION_BOTH == direct)
    {
        directionParam = UA_BROWSEDIRECTION_BOTH;
    }

    UA_BrowseDescription *browseDesc = (UA_BrowseDescription *) UA_calloc(count,
            sizeof(UA_BrowseDescription));
    if (NULL == browseDesc)
    {
        EDGE_LOG(TAG, "Memory allocation failed.");
        return NULL;
    }

    for (int id = 0; id < count; ++id)
    {
        browseDesc[id].nodeId = nodeIdList[id];
        browseDesc[id].browseDirection = directionParam;
        browseDesc[id].referenceTypeId = UA_NODEID_NUMERIC(SYSTEM_NAMESPACE_INDEX,
        UA_NS0ID_REFERENCES);
        browseDesc[id].includeSubtypes = true;
        browseDesc[id].nodeClassMask = BROWSE_DESCRIPTION_NODECLASS_MASK;
        browseDesc[id].resultMask = UA_BROWSERESULTMASK_ALL;
    }
    return browseDesc;
}

void invokeErrorCb(EdgeNodeId *srcNodeId, EdgeStatusCode edgeResult, const char *versatileValue)
{
    EdgeMessage *resultMsg = (EdgeMessage *) calloc(1, sizeof(EdgeMessage));
    if (!resultMsg)
    {
        goto EXIT;
    }

    EdgeEndPointInfo *epInfo = (EdgeEndPointInfo *) calloc(1, sizeof(EdgeEndPointInfo));
    if (!epInfo)
    {
        goto EXIT;
    }

    epInfo->endpointUri = cloneString(WELL_KNOWN_LOCALHOST_URI_VALUE);
    resultMsg->endpointInfo = epInfo;
    resultMsg->type = ERROR;
    resultMsg->result = createEdgeResult(edgeResult);
    if (!resultMsg->result)
    {
        goto EXIT;
    }

    EdgeVersatility *versatileVal = (EdgeVersatility *) calloc(1, sizeof(EdgeVersatility));
    if (!versatileVal)
    {
        goto EXIT;
    }
    versatileVal->isArray = false;
    versatileVal->value = cloneString(versatileValue);

    EdgeResponse *response = (EdgeResponse *) calloc(1, sizeof(EdgeResponse));
    if (!response)
    {
        freeEdgeVersatility(versatileVal);
        goto EXIT;
    }
    response->message = versatileVal;

    if (srcNodeId)
    {
        response->nodeInfo = (EdgeNodeInfo *) calloc(1, sizeof(EdgeNodeInfo));
        if (!response->nodeInfo)
        {
            freeEdgeResponse(response);
            goto EXIT;
        }
        response->nodeInfo->nodeId = srcNodeId;
    }

    EdgeResponse **responses = (EdgeResponse **) calloc(1, sizeof(EdgeResponse *));
    if (!responses)
    {
        if (response->nodeInfo)
            response->nodeInfo->nodeId = NULL;

        freeEdgeResponse(response);
        goto EXIT;
    }
    responses[0] = response;
    resultMsg->responses = responses;
    resultMsg->responseLength = 1;

    onResponseMessage(resultMsg);

    EXIT: if (response->nodeInfo)
        response->nodeInfo->nodeId = NULL;

    freeEdgeMessage(resultMsg);
}

bool checkContinuationPoint(UA_BrowseResult browseResult, EdgeNodeId *srcNodeId)
{
    bool retVal = true;
    /*if(browseResult.continuationPoint.length <= 0)
     {
     EDGE_LOG(TAG, "Error: " CONTINUATIONPOINT_EMPTY);
     invokeErrorCb(srcNodeId, STATUS_ERROR, CONTINUATIONPOINT_EMPTY);
     retVal = false;
     }
     else*/if (browseResult.continuationPoint.length >= 1000)
    {
        EDGE_LOG(TAG, "Error: " CONTINUATIONPOINT_LONG);
        invokeErrorCb(srcNodeId, STATUS_ERROR, CONTINUATIONPOINT_LONG);
        retVal = false;
    }
    else if (browseResult.continuationPoint.length > 0
            && (browseResult.referencesSize <= 0 || !browseResult.references))
    {
        EDGE_LOG(TAG, "Error: " STATUS_VIEW_REFERENCE_DATA_INVALID_VALUE);
        invokeErrorCb(srcNodeId, STATUS_ERROR, STATUS_VIEW_REFERENCE_DATA_INVALID_VALUE);
        retVal = false;
    }
    return retVal;
}

bool checkBrowseName(UA_String browseName, EdgeNodeId *srcNodeId)
{
    bool retVal = true;
    if (browseName.length <= 0 || NULL == browseName.data)
    {
        EDGE_LOG(TAG, "Error: " BROWSENAME_EMPTY);
        invokeErrorCb(srcNodeId, STATUS_ERROR, BROWSENAME_EMPTY);
        retVal = false;
    }
    else if (browseName.length >= 1000)
    {
        EDGE_LOG(TAG, "Error: " BROWSENAME_LONG);
        invokeErrorCb(srcNodeId, STATUS_ERROR, BROWSENAME_LONG);
        retVal = false;
    }

    return retVal;
}

bool checkNodeClass(UA_NodeClass nodeClass, EdgeNodeId *srcNodeId)
{
    bool retVal = true;
    if (false == isNodeClassValid(nodeClass))
    {
        EDGE_LOG(TAG, "Error: " NODECLASS_INVALID);
        invokeErrorCb(srcNodeId, STATUS_ERROR, NODECLASS_INVALID);
        retVal = false;
    }
    else if ((nodeClass & BROWSE_DESCRIPTION_NODECLASS_MASK) == 0)
    {
        EDGE_LOG(TAG, "Error: " STATUS_VIEW_NOTINCLUDE_NODECLASS_VALUE);
        invokeErrorCb(srcNodeId, STATUS_ERROR, STATUS_VIEW_NOTINCLUDE_NODECLASS_VALUE);
        retVal = false;
    }
    return retVal;
}

bool checkDisplayName(UA_String displayName, EdgeNodeId *srcNodeId)
{
    bool retVal = true;
    if (displayName.length <= 0 || NULL == displayName.data)
    {
        EDGE_LOG(TAG, "Error: " DISPLAYNAME_EMPTY);
        invokeErrorCb(srcNodeId, STATUS_ERROR, DISPLAYNAME_EMPTY);
        retVal = false;
    }
    else if (displayName.length >= 1000)
    {
        EDGE_LOG(TAG, "Error: " DISPLAYNAME_LONG);
        invokeErrorCb(srcNodeId, STATUS_ERROR, DISPLAYNAME_LONG);
        retVal = false;
    }

    return retVal;
}

bool checkNodeId(UA_ExpandedNodeId nodeId, EdgeNodeId *srcNodeId)
{
    bool retVal = true;
    if (UA_NodeId_isNull(&nodeId.nodeId))
    {
        EDGE_LOG(TAG, "Error: " NODEID_NULL);
        invokeErrorCb(srcNodeId, STATUS_ERROR, NODEID_NULL);
        retVal = false;
    }
    else if (nodeId.serverIndex != 0)
    {
        EDGE_LOG(TAG, "Error: " NODEID_SERVERINDEX);
        invokeErrorCb(srcNodeId, STATUS_ERROR, NODEID_SERVERINDEX);
        retVal = false;
    }
    return retVal;
}

bool checkReferenceTypeId(UA_NodeId nodeId, EdgeNodeId *srcNodeId)
{
    bool retVal = true;
    if (UA_NodeId_isNull(&nodeId))
    {
        EDGE_LOG(TAG, "Error: " REFERENCETYPEID_NULL);
        invokeErrorCb(srcNodeId, STATUS_ERROR, REFERENCETYPEID_NULL);
        retVal = false;
    }
    return retVal;
}

bool checkTypeDefinition(UA_ReferenceDescription *ref, EdgeNodeId *srcNodeId)
{
    bool retVal = true;
    if ((ref->nodeClass == UA_NODECLASS_OBJECT || ref->nodeClass == UA_NODECLASS_VARIABLE)
            && UA_NodeId_isNull(&ref->typeDefinition.nodeId))
    {
        EDGE_LOG(TAG, "Error: " TYPEDEFINITIONNODEID_NULL);
        invokeErrorCb(srcNodeId, STATUS_ERROR, TYPEDEFINITIONNODEID_NULL);
        retVal = false;
    }
    return retVal;
}

static void invokeResponseCb(EdgeMessage *msg, int msgId, EdgeNodeId *srcNodeId,
        EdgeBrowseResult *browseResult, int size)
{
    EdgeMessage *resultMsg = (EdgeMessage *) calloc(1, sizeof(EdgeMessage));
    if (!resultMsg)
    {
        EDGE_LOG(TAG, "Memory allocation failed.");
        return;
    }

    resultMsg->type = BROWSE_RESPONSE;

    resultMsg->endpointInfo = cloneEdgeEndpointInfo(msg->endpointInfo);
    if (!resultMsg->endpointInfo)
    {
        EDGE_LOG(TAG, "Failed to clone the EdgeEndpointInfo.");
        freeEdgeMessage(resultMsg);
        return;
    }

    EdgeResponse *response = (EdgeResponse *) calloc(1, sizeof(EdgeResponse));
    if (!response)
    {
        EDGE_LOG(TAG, "Memory allocation failed.");
        freeEdgeMessage(resultMsg);
        return;
    }

    response->nodeInfo = (EdgeNodeInfo *) calloc(1, sizeof(EdgeNodeInfo));
    if (!response->nodeInfo)
    {
        EDGE_LOG(TAG, "Memory allocation failed.");
        freeEdgeResponse(response);
        freeEdgeMessage(resultMsg);
        return;
    }
    response->nodeInfo->nodeId = srcNodeId;
    response->requestId = msgId; // Response for msgId'th request.
    EdgeResponse **responses = (EdgeResponse **) calloc(1, sizeof(EdgeResponse *));
    if (!responses)
    {
        EDGE_LOG(TAG, "Memory allocation failed.");
        response->nodeInfo->nodeId = NULL;
        freeEdgeResponse(response);
        freeEdgeMessage(resultMsg);
        return;
    }
    responses[0] = response;
    resultMsg->responses = responses;
    resultMsg->responseLength = 1;

    resultMsg->browseResult = browseResult;
    resultMsg->browseResultLength = size;

    onResponseMessage(resultMsg);

    resultMsg->browseResultLength = 0;
    resultMsg->browseResult = NULL;
    response->nodeInfo->nodeId = NULL;
    freeEdgeMessage(resultMsg);
}

static void invokeResponseCbForContinuationPoint(EdgeMessage *msg, int msgId, EdgeNodeId *srcNodeId,
        UA_ByteString *continuationPoint)
{
    if (!continuationPoint || continuationPoint->length < 1)
    {
        return;
    }

    EdgeMessage *resultMsg = (EdgeMessage *) calloc(1, sizeof(EdgeMessage));
    if (!resultMsg)
    {
        EDGE_LOG(TAG, "Memory allocation failed.");
        return;
    }

    resultMsg->type = BROWSE_RESPONSE;

    resultMsg->cpList = getContinuationPointList(continuationPoint);
    if (!resultMsg->cpList)
    {
        EDGE_LOG(TAG, "Failed to form the continuation point.");
        free(resultMsg);
        return;
    }

    resultMsg->endpointInfo = cloneEdgeEndpointInfo(msg->endpointInfo);
    if (!resultMsg->endpointInfo)
    {
        EDGE_LOG(TAG, "Failed to clone the EdgeEndpointInfo.");
        freeEdgeMessage(resultMsg);
        return;
    }

    EdgeResponse *response = (EdgeResponse *) calloc(1, sizeof(EdgeResponse));
    if (!response)
    {
        EDGE_LOG(TAG, "Memory allocation failed.");
        freeEdgeMessage(resultMsg);
        return;
    }
    response->nodeInfo = (EdgeNodeInfo *) calloc(1, sizeof(EdgeNodeInfo));
    if (!response->nodeInfo)
    {
        EDGE_LOG(TAG, "Memory allocation failed.");
        freeEdgeResponse(response);
        freeEdgeMessage(resultMsg);
        return;
    }
    response->nodeInfo->nodeId = srcNodeId;
    response->requestId = msgId; // Response for msgId'th request.
    EdgeResponse **responses = (EdgeResponse **) calloc(1, sizeof(EdgeResponse *));
    if (!responses)
    {
        EDGE_LOG(TAG, "Memory allocation failed.");
        response->nodeInfo->nodeId = NULL;
        freeEdgeResponse(response);
        freeEdgeMessage(resultMsg);
        return;
    }
    responses[0] = response;
    resultMsg->responses = responses;
    resultMsg->responseLength = 1;

    onResponseMessage(resultMsg);

    response->nodeInfo->nodeId = NULL;

    freeEdgeMessage(resultMsg);
}

EdgeNodeId *getEdgeNodeId(UA_NodeId *node)
{
    if (!node)
    {
        return NULL;
    }

    EdgeNodeId *edgeNodeId = (EdgeNodeId *) calloc(1, sizeof(EdgeNodeId));
    if (!edgeNodeId)
    {
        return NULL;
    }
    edgeNodeId->nameSpace = (int) node->namespaceIndex;
    switch (node->identifierType)
    {
        case UA_NODEIDTYPE_NUMERIC:
            edgeNodeId->type = INTEGER;
            edgeNodeId->integerNodeId = node->identifier.numeric;
            break;
        case UA_NODEIDTYPE_STRING:
            edgeNodeId->type = STRING;
            edgeNodeId->nodeId = convertUAStringToString(&node->identifier.string);
            break;
        case UA_NODEIDTYPE_BYTESTRING:
            edgeNodeId->type = BYTESTRING;
            edgeNodeId->nodeId = convertUAStringToString(&node->identifier.string);
            break;
        case UA_NODEIDTYPE_GUID:
            edgeNodeId->type = UUID;
            UA_Guid guid = node->identifier.guid;
            char *value = (char *) malloc(GUID_LENGTH + 1);
            if (!value)
            {
                EDGE_LOG(TAG, "Memory allocation failed.");
                free(edgeNodeId);
                edgeNodeId = NULL;
                break;
            }

            snprintf(value, (GUID_LENGTH / 2) + 1, "%08x-%04x-%04x", guid.data1, guid.data2,
                    guid.data3);
            sprintf(value, "%s-%02x", value, guid.data4[0]);
            sprintf(value, "%s%02x", value, guid.data4[1]);
            sprintf(value, "%s-", value);
            for (int j = 2; j < 8; j++)
            {
                sprintf(value, "%s%02x", value, guid.data4[j]);
            }
            value[GUID_LENGTH] = '\0';
            edgeNodeId->nodeId = value;
            break;
        default:
            break;
    }

    return edgeNodeId;
}

void printNodeId(UA_NodeId n1)
{
    switch (n1.identifierType)
    {
        case UA_NODEIDTYPE_NUMERIC:
            EDGE_LOG_V(TAG, "Numeric: %d\n", n1.identifier.numeric);
            break;
        case UA_NODEIDTYPE_STRING:
            EDGE_LOG_V(TAG, "String: %s\n", convertUAStringToString(&n1.identifier.string));
            break;
        case UA_NODEIDTYPE_BYTESTRING:
            EDGE_LOG_V(TAG, "Byte String: %s\n", convertUAStringToString(&n1.identifier.byteString));
            break;
        case UA_NODEIDTYPE_GUID:
            EDGE_LOG(TAG, "GUID\n");
            break;
    }
}

EdgeStatusCode browse(UA_Client *client, EdgeMessage *msg, bool browseNext, UA_NodeId *nodeIdList,
        int nodeCount, int *msgIdList, int msgCount, BrowseMap *map, int mapSize)
{
    UA_BrowseResponse *resp = NULL;
    UA_BrowseResponse browseResp =
    {
    { 0 } };
    UA_BrowseNextResponse browseNextResp =
    {
    { 0 } };
    UA_BrowseDescription *nodesToBrowse = NULL;
    if (browseNext)
    {
        UA_BrowseNextRequest bReq;
        UA_BrowseNextRequest_init(&bReq);
        bReq.releaseContinuationPoints = false;
        bReq.continuationPointsSize = msg->cpList->count;
        bReq.continuationPoints = (UA_ByteString *) malloc(
                bReq.continuationPointsSize * sizeof(UA_ByteString));
        if (!bReq.continuationPoints)
        {
            EDGE_LOG(TAG, "Memory allocation failed.");
            return STATUS_INTERNAL_ERROR;
        }

        for (int i = 0; i < bReq.continuationPointsSize; ++i)
        {
            UA_ByteString *byteStr;
            bReq.continuationPoints[i] =
                    (byteStr = getUAStringFromEdgeContinuationPoint(msg->cpList->cp[i])) ?
                            *byteStr : UA_BYTESTRING_NULL;
            free(byteStr);
        }
        browseNextResp = UA_Client_Service_browseNext(client, bReq);
        resp = (UA_BrowseResponse *) &browseNextResp;
        UA_BrowseNextRequest_deleteMembers(&bReq);
    }
    else
    {
        nodesToBrowse = getBrowseDescriptions(nodeIdList, nodeCount, msg);
        if (!nodesToBrowse)
        {
            EDGE_LOG(TAG, "Failed to create browse descriptions.");
            return STATUS_ERROR;
        }

        UA_BrowseRequest bReq;
        UA_BrowseRequest_init(&bReq);
        bReq.requestedMaxReferencesPerNode = msg->browseParam->maxReferencesPerNode;
        bReq.nodesToBrowse = nodesToBrowse;
        bReq.nodesToBrowseSize = nodeCount;
        browseResp = UA_Client_Service_browse(client, bReq);
        resp = &browseResp;
    }

    if (resp->responseHeader.serviceResult != UA_STATUSCODE_GOOD || resp->resultsSize <= 0)
    {
        char *versatileVal;
        EdgeStatusCode statusCode;
        // Invoke error callback
        if (resp->resultsSize <= 0)
        {
            statusCode = STATUS_VIEW_BROWSERESULT_EMPTY;
            versatileVal = STATUS_VIEW_BROWSERESULT_EMPTY_VALUE;
            EDGE_LOG(TAG, "Error: Empty browse response!!!");
        }
        else
        {
            statusCode = STATUS_SERVICE_RESULT_BAD;
            versatileVal = STATUS_SERVICE_RESULT_BAD_VALUE;
            UA_StatusCode serviceResult = resp->responseHeader.serviceResult;
            (void) serviceResult;
            EDGE_LOG_V(TAG, "Error in browse :: 0x%08x(%s)\n", serviceResult, UA_StatusCode_name(serviceResult));
        }

        EdgeNodeInfo *nodeInfo = getEndpoint(msg, 0);
        invokeErrorCb((nodeInfo ? nodeInfo->nodeId : NULL), statusCode, versatileVal);
        free(nodesToBrowse);
        UA_BrowseResponse_deleteMembers(resp);
        return statusCode;
    }

    int *nextMsgIdList = NULL;
    UA_NodeId *nextNodeIdList = NULL;
    EdgeStatusCode statusCode;
    int nodeIdUnknownCount = 0;
    EdgeNodeId *srcNodeId = NULL;
    for (size_t i = 0; i < resp->resultsSize; ++i)
    {
        freeEdgeNodeId(srcNodeId);
        srcNodeId = getEdgeNodeId(&nodeIdList[i]);
        UA_StatusCode status = resp->results[i].statusCode;
        int msgId = msgIdList[i];
        EdgeBrowseDirection direction = msg->browseParam->direction;
        if (checkStatusGood(status) == false)
        {
            if (UA_STATUSCODE_BADNODEIDUNKNOWN == status)
                nodeIdUnknownCount++;

            if (nodeIdUnknownCount == resp->resultsSize)
            {
                EDGE_LOG(TAG, "Error: " STATUS_VIEW_NODEID_UNKNOWN_ALL_RESULTS_VALUE);
                invokeErrorCb(srcNodeId, STATUS_VIEW_NODEID_UNKNOWN_ALL_RESULTS,
                STATUS_VIEW_NODEID_UNKNOWN_ALL_RESULTS_VALUE);
            }
            else
            {
                const char *statusStr = UA_StatusCode_name(status);
                invokeErrorCb(srcNodeId, STATUS_VIEW_RESULT_STATUS_CODE_BAD, statusStr);
            }
            continue;
        }

        if (!checkContinuationPoint(resp->results[i], srcNodeId))
        {
            continue;
        }

        // If it is a browseNext call,
        // then references should not be empty if statuscode is good.
        if (browseNext && !resp->results[i].referencesSize)
        {
            EDGE_LOG(TAG, "Error: " STATUS_VIEW_REFERENCE_DATA_INVALID_VALUE);
            invokeErrorCb(srcNodeId, STATUS_ERROR, STATUS_VIEW_REFERENCE_DATA_INVALID_VALUE);
            continue;
        }

        nextMsgIdList = (int *) calloc(resp->results[i].referencesSize, sizeof(int));
        if (!nextMsgIdList)
        {
            EDGE_LOG(TAG, "Memory allocation failed.");
            statusCode = STATUS_INTERNAL_ERROR;
            goto EXIT;
        }

        nextNodeIdList = (UA_NodeId *) calloc(resp->results[i].referencesSize, sizeof(UA_NodeId));
        if (!nextNodeIdList)
        {
            EDGE_LOG(TAG, "Memory allocation failed.");
            statusCode = STATUS_INTERNAL_ERROR;
            goto EXIT;
        }

        int nextMsgListCount = 0;
        int nextNodeListCount = 0;
        for (size_t j = 0; j < resp->results[i].referencesSize; ++j)
        {
            bool isError = false;
            UA_ReferenceDescription *ref = &(resp->results[i].references[j]);
            if ((direction == DIRECTION_FORWARD && ref->isForward == false)
                    || (direction == DIRECTION_INVERSE && ref->isForward == true))
            {
                EDGE_LOG(TAG, "Error: " STATUS_VIEW_DIRECTION_NOT_MATCH_VALUE);
                invokeErrorCb(srcNodeId, STATUS_VIEW_DIRECTION_NOT_MATCH,
                STATUS_VIEW_DIRECTION_NOT_MATCH_VALUE);
                isError = true;
            }

            if (!checkBrowseName(ref->browseName.name, srcNodeId))
                isError = true;
            if (!checkNodeClass(ref->nodeClass, srcNodeId))
                isError = true;
            if (!checkDisplayName(ref->displayName.text, srcNodeId))
                isError = true;
            if (!checkNodeId(ref->nodeId, srcNodeId))
                isError = true;
            if (!checkReferenceTypeId(ref->referenceTypeId, srcNodeId))
                isError = true;
            if (!checkTypeDefinition(ref, srcNodeId))
                isError = true;

            if (!isError)
            {
                int size = 1;
                EdgeBrowseResult *browseResult = (EdgeBrowseResult *) calloc(size,
                        sizeof(EdgeBrowseResult *));
                if (!browseResult)
                {
                    EDGE_LOG(TAG, "Memory allocation failed.");
                    statusCode = STATUS_INTERNAL_ERROR;
                    goto EXIT;
                }
                browseResult->browseName = convertUAStringToString(&ref->browseName.name);
                if (!browseResult->browseName)
                {
                    EDGE_LOG(TAG, "Memory allocation failed.");
                    statusCode = STATUS_INTERNAL_ERROR;
                    free(browseResult);
                    goto EXIT;
                }

                invokeResponseCb(msg, msgId, srcNodeId, browseResult, size);
                free(browseResult->browseName);
                free(browseResult);
                browseResult = NULL;
#if DEBUG
                printNodeId(ref->nodeId.nodeId);
#endif
                if (!hasNode(map, mapSize, msgId, ref->nodeId.nodeId))
                {
                    EDGE_LOG(TAG, "Adding this NodeId in Map.");
                    if (!addToMap(map, mapSize, msgId, ref->nodeId.nodeId, &ref->browseName.name))
                    {
                        EDGE_LOG(TAG, "Adding node to map failed.");
                        statusCode = STATUS_INTERNAL_ERROR;
                        goto EXIT;
                    }
                    nextNodeIdList[nextNodeListCount++] = ref->nodeId.nodeId;
                    nextMsgIdList[nextMsgListCount++] = msgId;
                }
                else
                {
                    EDGE_LOG(TAG, "Already added this NodeId in Map.");
                }
            }
        }

        // Pass the continuation point for this result to the application.
        if (resp->results[i].continuationPoint.length > 0)
        {
            EDGE_LOG(TAG, "Passing continuation point to application.");
            invokeResponseCbForContinuationPoint(msg, msgId, srcNodeId,
                    &resp->results[i].continuationPoint);
        }

        if (nextNodeListCount > 0)
        {
            browse(client, msg, false, nextNodeIdList, nextNodeListCount, nextMsgIdList,
                    nextMsgListCount, map, mapSize);
        }
        freeEdgeNodeId(srcNodeId);
        srcNodeId = NULL;
        free(nextMsgIdList);
        nextMsgIdList = NULL;
        free(nextNodeIdList);
        nextNodeIdList = NULL;
    }
    statusCode = STATUS_OK;

    EXIT: free(nextMsgIdList);
    free(nextNodeIdList);
    free(nodesToBrowse);
    freeEdgeNodeId(srcNodeId);
    UA_BrowseResponse_deleteMembers(resp);
    return statusCode;
}

EdgeResult executeBrowse(UA_Client *client, EdgeMessage *msg, bool browseNext)
{
    EdgeResult result;
    if (!client)
    {
        EDGE_LOG(TAG, "Invalid client handle parameter.");
        result.code = STATUS_PARAM_INVALID;
        return result;
    }

    if (!msg)
    {
        EDGE_LOG(TAG, "Invalid edge message parameter.");
        result.code = STATUS_PARAM_INVALID;
        return result;
    }

    if (browseNext && !msg->cpList && msg->cpList->count < 1)
    {
        EDGE_LOG(TAG, "EdgeContinuationPointList is invalid.");
        result.code = STATUS_PARAM_INVALID;
        return result;
    }

    if (!browseNext && msg->type != SEND_REQUEST && msg->type != SEND_REQUESTS)
    {
        EDGE_LOG(TAG, "Invalid message type.");
        result.code = STATUS_NOT_SUPPORT;
        return result;
    }

    int *msgIdList = NULL;
    UA_NodeId *nodeIdList = NULL;
    int nodesToBrowseSize;
    if (browseNext)
    {
        nodesToBrowseSize = msg->cpList->count;
    }
    else
    {
        nodesToBrowseSize = (msg->requestLength) ? msg->requestLength : 1;
    }

    msgIdList = (int *) calloc(nodesToBrowseSize, sizeof(int));
    if (!msgIdList)
    {
        EDGE_LOG(TAG, "Memory allocation failed.");
        result.code = STATUS_INTERNAL_ERROR;
        return result;
    }

    nodeIdList = (UA_NodeId *) calloc(nodesToBrowseSize, sizeof(UA_NodeId));
    if (!nodeIdList)
    {
        EDGE_LOG(TAG, "Memory allocation failed.");
        free(msgIdList);
        result.code = STATUS_INTERNAL_ERROR;
        return result;
    }

    if (msg->type == SEND_REQUEST)
    {
        EDGE_LOG(TAG, "Message Type: " SEND_REQUEST_DESC);
        UA_NodeId *nodeId;
        nodeIdList[0] = (nodeId = getNodeId(msg->request)) ? *nodeId : UA_NODEID_NULL;
        msgIdList[0] = 0;
        free(nodeId);
    }
    else
    {
        EDGE_LOG(TAG, "Message Type: " SEND_REQUESTS_DESC);
        if (!browseNext && nodesToBrowseSize > MAX_BROWSEREQUEST_SIZE)
        {
            EDGE_LOG(TAG, "Error: " STATUS_VIEW_BROWSEREQUEST_SIZEOVER_VALUE);
            EdgeNodeInfo *nodeInfo = getEndpoint(msg, 0);
            invokeErrorCb((nodeInfo ? nodeInfo->nodeId : NULL), STATUS_ERROR,
            STATUS_VIEW_BROWSEREQUEST_SIZEOVER_VALUE);
            result.code = STATUS_ERROR;
            free(nodeIdList);
            free(msgIdList);
            return result;
        }

        for (int i = 0; i < nodesToBrowseSize; ++i)
        {
            UA_NodeId *nodeId;
            nodeIdList[i] = (nodeId = getNodeIdMultiReq(msg, i)) ? *nodeId : UA_NODEID_NULL;
            free(nodeId);
            msgIdList[i] = i;
        }
    }

    int mapSize = nodesToBrowseSize;
    BrowseMap *map = initMap(nodesToBrowseSize);
    if (!map)
    {
        EDGE_LOG(TAG, "Failed to initialize a Map.");
        result.code = STATUS_INTERNAL_ERROR;
    }
    else
    {
        EdgeStatusCode statusCode = browse(client, msg, browseNext, nodeIdList, nodesToBrowseSize,
                msgIdList, nodesToBrowseSize, map, mapSize);
        if (statusCode != STATUS_OK)
        {
            EDGE_LOG(TAG, "Browse failed.");
            result.code = STATUS_ERROR;
        }
        else
        {
            result.code = STATUS_OK;
        }
    }

    for (int i = 0; i < nodesToBrowseSize; ++i)
    {
        UA_NodeId_deleteMembers(nodeIdList + i);
    }
    free(nodeIdList);
    free(msgIdList);
    destroyMap(map, mapSize);
    return result;
}
