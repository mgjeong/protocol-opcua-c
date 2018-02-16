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
#include "edge_node_type.h"
#include "edge_logger.h"
#include "edge_malloc.h"
#include "message_dispatcher.h"
#include "command_adapter.h"

#include <inttypes.h>
#include <string.h>

#define TAG "browse"
#define GUID_LENGTH (36)

static const int BROWSE_NODECLASS_MASK = UA_NODECLASS_UNSPECIFIED;
static const int VIEW_NODECLASS_MASK = UA_NODECLASS_OBJECT | UA_NODECLASS_VIEW;
static const int SHOW_SPECIFIC_NODECLASS_MASK = UA_NODECLASS_VARIABLE | UA_NODECLASS_VIEW | UA_NODECLASS_METHOD;
static const int SHOW_SPECIFIC_NODECLASS = false;

static response_cb_t g_responseCallback = NULL;

typedef struct ViewNodeInfo
{
    UA_NodeId *nodeId;
    unsigned char *browseName;
} ViewNodeInfo_t;

typedef struct NodesToBrowse
{
    UA_NodeId *nodeId;
    unsigned char **browseName;
    size_t size;
} NodesToBrowse_t;

static NodesToBrowse_t *initNodesToBrowse(size_t size)
{
    NodesToBrowse_t *browseNodesInfo = (NodesToBrowse_t *) EdgeCalloc(1, sizeof(NodesToBrowse_t));
    if (IS_NULL(browseNodesInfo))
    {
        EDGE_LOG(TAG, "Memory allocation failed for browseNodesInfo.");
        return NULL;
    }
    browseNodesInfo->size = size;

    if(size > 0)
    {
        browseNodesInfo->nodeId = (UA_NodeId *) EdgeCalloc(size, sizeof(UA_NodeId));
        if (IS_NULL(browseNodesInfo->nodeId))
        {
            EDGE_LOG(TAG, "Memory allocation failed for browseNodesInfo->nodeId.");
            EdgeFree(browseNodesInfo);
            return NULL;
        }

        browseNodesInfo->browseName = (unsigned char **) calloc(size, sizeof(unsigned char *));
        if (IS_NULL(browseNodesInfo->browseName))
        {
            EDGE_LOG(TAG, "Memory allocation failed for browseNodesInfo->browseName.");
            EdgeFree(browseNodesInfo->nodeId);
            EdgeFree(browseNodesInfo);
            return NULL;
        }
    }
    return browseNodesInfo;
}

static void destroyNodesToBrowse(NodesToBrowse_t *ptr, bool deleteNodeId)
{
    if(IS_NULL(ptr))
    {
        return;
    }

    for(int i = 0; i < ptr->size; ++i)
    {
        if(deleteNodeId)
            UA_NodeId_deleteMembers(ptr->nodeId + i);
        EdgeFree(ptr->browseName[i]);
    }
    EdgeFree(ptr->nodeId);
    EdgeFree(ptr->browseName);
    EdgeFree(ptr);
}

static unsigned char *convertUAStringToUnsignedChar(UA_String *uaStr)
{
    if (!uaStr || uaStr->length <= 0)
    {
        return NULL;
    }

    unsigned char *str = (unsigned char *) EdgeCalloc(uaStr->length+1, sizeof(unsigned char));
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

    EdgeContinuationPointList *cpList = (EdgeContinuationPointList *) EdgeCalloc(1,
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

    cpList->cp[0] = (EdgeContinuationPoint *) EdgeCalloc(1, sizeof(EdgeContinuationPoint));
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
    if (IS_NULL(cp) || cp->length < 1)
    {
        return NULL;
    }

    UA_ByteString *byteStr = (UA_ByteString *) EdgeMalloc(sizeof(UA_ByteString));
    VERIFY_NON_NULL(byteStr, NULL);

    byteStr->length = cp->length;
    byteStr->data = (UA_Byte *) EdgeMalloc(byteStr->length * sizeof(UA_Byte));
    if (IS_NULL(byteStr->data))
    {
        EdgeFree(byteStr);
        return NULL;
    }

    for (int i = 0; i < byteStr->length; ++i)
    {
        byteStr->data[i] = cp->continuationPoint[i];
    }

    return byteStr;
}

static UA_NodeId *getNodeId(EdgeRequest *req)
{
    if (IS_NULL(req) || !req->nodeInfo || !req->nodeInfo->nodeId)
    {
        return NULL;
    }
    UA_NodeId *node = (UA_NodeId *) EdgeCalloc(1, sizeof(UA_NodeId));
    VERIFY_NON_NULL(node, NULL);
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
    VERIFY_NON_NULL(msg, NULL);
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

static UA_BrowseDescription *getBrowseDescriptions(NodesToBrowse_t *browseNodesInfo,
        EdgeMessage *msg, UA_UInt32 nodeClassMask)
{
    VERIFY_NON_NULL(msg->browseParam, NULL);
    VERIFY_NON_NULL(browseNodesInfo, NULL);

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

    UA_BrowseDescription *browseDesc = (UA_BrowseDescription *) UA_calloc(browseNodesInfo->size,
            sizeof(UA_BrowseDescription));
    VERIFY_NON_NULL(browseDesc, NULL);

    for (size_t idx = 0; idx < browseNodesInfo->size; ++idx)
    {
        browseDesc[idx].nodeId = browseNodesInfo->nodeId[idx];
        browseDesc[idx].browseDirection = directionParam;
        browseDesc[idx].referenceTypeId = UA_NODEID_NUMERIC(SYSTEM_NAMESPACE_INDEX,
        UA_NS0ID_REFERENCES);
        browseDesc[idx].includeSubtypes = true;
        browseDesc[idx].nodeClassMask = nodeClassMask;
        browseDesc[idx].resultMask = UA_BROWSERESULTMASK_ALL;
    }
    return browseDesc;
}

void invokeErrorCb(EdgeNodeId *srcNodeId, EdgeStatusCode edgeResult, const char *versatileValue)
{
    EdgeMessage *resultMsg = (EdgeMessage *) EdgeCalloc(1, sizeof(EdgeMessage));
    if (IS_NULL(resultMsg))
    {
        return;
    }

    resultMsg->endpointInfo = (EdgeEndPointInfo *) EdgeCalloc(1, sizeof(EdgeEndPointInfo));
    if (IS_NULL(resultMsg->endpointInfo))
    {
        goto EXIT;
    }

    resultMsg->endpointInfo->endpointUri = cloneString(WELL_KNOWN_LOCALHOST_URI_VALUE);
    if(IS_NULL(resultMsg->endpointInfo->endpointUri))
    {
        goto EXIT;
    }

    resultMsg->type = ERROR;
    resultMsg->result = createEdgeResult(edgeResult);
    if (IS_NULL(resultMsg->result))
    {
        goto EXIT;
    }

    resultMsg->responses = (EdgeResponse **) EdgeCalloc(1, sizeof(EdgeResponse *));
    if (IS_NULL(resultMsg->responses))
    {
        goto EXIT;
    }

    resultMsg->responses[0] = (EdgeResponse *) EdgeCalloc(1, sizeof(EdgeResponse));
    if (IS_NULL(resultMsg->responses[0]))
    {
        goto EXIT;
    }

    resultMsg->responses[0]->message = (EdgeVersatility *) EdgeCalloc(1, sizeof(EdgeVersatility));
    if (IS_NULL(resultMsg->responses[0]->message))
    {
        goto EXIT;
    }
    resultMsg->responses[0]->message->isArray = false;
    resultMsg->responses[0]->message->value = cloneString(versatileValue);

    if (srcNodeId)
    {
        resultMsg->responses[0]->nodeInfo = (EdgeNodeInfo *) EdgeCalloc(1, sizeof(EdgeNodeInfo));
        if (IS_NULL(resultMsg->responses[0]->nodeInfo))
        {
            goto EXIT;
        }
        resultMsg->responses[0]->nodeInfo->nodeId = srcNodeId;
    }

    resultMsg->responseLength = 1;

    if (IS_NOT_NULL(g_responseCallback))
    {
        g_responseCallback(resultMsg);
    }

    resultMsg->responses[0]->nodeInfo->nodeId = NULL;

EXIT:
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
    else if (UA_NODECLASS_UNSPECIFIED != BROWSE_NODECLASS_MASK &&
        (nodeClass & BROWSE_NODECLASS_MASK) == 0)
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
        EdgeBrowseResult *browseResult, size_t size, const unsigned char *browsePath)
{
    if(IS_NULL(browseResult) || IS_NULL(browseResult->browseName))
    {
        EDGE_LOG(TAG, "Browse result is empty.");
        return;
    }

    EdgeMessage *resultMsg = (EdgeMessage *) EdgeCalloc(1, sizeof(EdgeMessage));
    if (IS_NULL(resultMsg))
    {
        EDGE_LOG(TAG, "Memory allocation failed.");
        return;
    }

    resultMsg->type = BROWSE_RESPONSE;
    resultMsg->message_id = msg->message_id;
    resultMsg->endpointInfo = cloneEdgeEndpointInfo(msg->endpointInfo);
    if (IS_NULL(resultMsg->endpointInfo))
    {
        EDGE_LOG(TAG, "Failed to clone the EdgeEndpointInfo.");
        goto ERROR;
    }

    resultMsg->responses = (EdgeResponse **) EdgeCalloc (1, sizeof(EdgeResponse *));
    if (IS_NULL(resultMsg->responses))
    {
        EDGE_LOG(TAG, "Memory allocation failed.");
        goto ERROR;
    }
    resultMsg->responseLength = 1;
    resultMsg->responses[0] = (EdgeResponse *) EdgeCalloc(1, sizeof(EdgeResponse));
    if (IS_NULL(resultMsg->responses[0]))
    {
        EDGE_LOG(TAG, "Memory allocation failed.");
        goto ERROR;
    }

    if(IS_NOT_NULL(browsePath))
    {
        resultMsg->responses[0]->message = (EdgeVersatility *) EdgeCalloc(1, sizeof(EdgeVersatility));
        if (IS_NULL(resultMsg->responses[0]->message))
        {
            EDGE_LOG(TAG, "Memory allocation failed.");
            goto ERROR;
        }
        resultMsg->responses[0]->message->isArray = false;
        resultMsg->responses[0]->message->value = (unsigned char *)cloneData(browsePath, strlen((char *)browsePath)+1);
        if(IS_NULL(resultMsg->responses[0]->message->value))
        {
            EDGE_LOG(TAG, "Memory allocation failed.");
            goto ERROR;
        }
    }

    resultMsg->responses[0]->nodeInfo = (EdgeNodeInfo *) EdgeCalloc(1, sizeof(EdgeNodeInfo));
    if (IS_NULL(resultMsg->responses[0]->nodeInfo))
    {
        EDGE_LOG(TAG, "Memory allocation failed.");
        goto ERROR;
    }
    resultMsg->responses[0]->nodeInfo->nodeId = cloneEdgeNodeId(srcNodeId);               //srcNodeId;
    resultMsg->responses[0]->requestId = msgId; // Response for msgId'th request.
    resultMsg->browseResult = (EdgeBrowseResult *) EdgeCalloc(1, sizeof(EdgeBrowseResult));               //browseResult;
    if (IS_NULL(resultMsg->browseResult))
    {
        EDGE_LOG(TAG, "Memory allocation failed.");
        goto ERROR;
    }
    resultMsg->browseResult->browseName = cloneString(browseResult->browseName);
    if(IS_NULL(resultMsg->browseResult->browseName))
    {
        EDGE_LOG(TAG, "Memory allocation failed.");
        goto ERROR;
    }

    resultMsg->browseResultLength = size;

    add_to_recvQ(resultMsg);
    return;

ERROR:
    freeEdgeMessage(resultMsg);
}

static void invokeResponseCbForContinuationPoint(EdgeMessage *msg, int msgId, EdgeNodeId *srcNodeId,
        UA_ByteString *continuationPoint)
{
    if (!continuationPoint || continuationPoint->length < 1)
    {
        return;
    }

    EdgeMessage *resultMsg = (EdgeMessage *) EdgeCalloc(1, sizeof(EdgeMessage));
    if (IS_NULL(resultMsg))
    {
        EDGE_LOG(TAG, "Memory allocation failed.");
        return;
    }

    resultMsg->type = BROWSE_RESPONSE;

    resultMsg->cpList = getContinuationPointList(continuationPoint);
    if (!resultMsg->cpList)
    {
        EDGE_LOG(TAG, "Failed to form the continuation point.");
        EdgeFree(resultMsg);
        return;
    }

    resultMsg->endpointInfo = cloneEdgeEndpointInfo(msg->endpointInfo);
    if (!resultMsg->endpointInfo)
    {
        EDGE_LOG(TAG, "Failed to clone the EdgeEndpointInfo.");
        freeEdgeMessage(resultMsg);
        return;
    }

    EdgeResponse *response = (EdgeResponse *) EdgeCalloc(1, sizeof(EdgeResponse));
    if (IS_NULL(response))
    {
        EDGE_LOG(TAG, "Memory allocation failed.");
        freeEdgeMessage(resultMsg);
        return;
    }
    response->nodeInfo = (EdgeNodeInfo *) EdgeCalloc(1, sizeof(EdgeNodeInfo));
    if (IS_NULL(response->nodeInfo))
    {
        EDGE_LOG(TAG, "Memory allocation failed.");
        freeEdgeResponse(response);
        freeEdgeMessage(resultMsg);
        return;
    }
    response->nodeInfo->nodeId = cloneEdgeNodeId(srcNodeId);
    response->requestId = msgId; // Response for msgId'th request.
    EdgeResponse **responses = (EdgeResponse **) calloc(1, sizeof(EdgeResponse *));
    if (IS_NULL(responses))
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
    resultMsg->message_id = msg->message_id;

    add_to_recvQ(resultMsg);
}

static EdgeNodeId *getEdgeNodeId(UA_NodeId *node)
{
    if (!node)
    {
        return NULL;
    }

    EdgeNodeId *edgeNodeId = (EdgeNodeId *) EdgeCalloc(1, sizeof(EdgeNodeId));
    VERIFY_NON_NULL(edgeNodeId, NULL);

    edgeNodeId->nameSpace = node->namespaceIndex;
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
            char *value = (char *) EdgeMalloc(GUID_LENGTH + 1);
            if (IS_NULL(value))
            {
                EDGE_LOG(TAG, "Memory allocation failed.");
                EdgeFree(edgeNodeId);
                edgeNodeId = NULL;
                break;
            }

            snprintf(value, GUID_LENGTH + 1, "%08x-%04x-%04x-%02x%02x-%02x%02x%02x%02x%02x%02x",
                    guid.data1, guid.data2, guid.data3, guid.data4[0], guid.data4[1], guid.data4[2],
                    guid.data4[3], guid.data4[4], guid.data4[5], guid.data4[6], guid.data4[7]);
            edgeNodeId->nodeId = value;
            break;
        default:
            break;
    }

    return edgeNodeId;
}

void printNodeId(UA_NodeId n1)
{
    char *str = NULL;
    switch (n1.identifierType)
    {
        case UA_NODEIDTYPE_NUMERIC:
            EDGE_LOG_V(TAG, "Numeric: %d\t", n1.identifier.numeric);
            break;
        case UA_NODEIDTYPE_STRING:
            str = convertUAStringToString(&n1.identifier.string);
            EDGE_LOG_V(TAG, "String: %s\t", str);
            break;
        case UA_NODEIDTYPE_BYTESTRING:
            str = convertUAStringToString(&n1.identifier.byteString);
            EDGE_LOG_V(TAG, "Byte String: %s\t", str);
            break;
        case UA_NODEIDTYPE_GUID:
            EDGE_LOG(TAG, "GUID\n");
            break;
    }
    EdgeFree(str);
}

typedef struct _browsePathNode{
    EdgeNodeId *edgeNodeId;
    unsigned char *browseName;
    struct _browsePathNode *pre;
    struct _browsePathNode *next;
}browsePathNode;

browsePathNode *browsePathNodeListHead = NULL, *browsePathNodeListTail = NULL;

void DestroyBrowsePathNodeList() {
    browsePathNode *ptr = browsePathNodeListHead;
    while(ptr != NULL){
        browsePathNode *nextNode = ptr->next;
        EdgeFree(ptr);
        ptr = nextNode;
    }
    browsePathNodeListHead = NULL;
    browsePathNodeListTail = NULL;
}

browsePathNode *InitBrowsePathNodeList(){
    if (browsePathNodeListHead != NULL){
        DestroyBrowsePathNodeList();
    }
    browsePathNodeListHead = NULL;
    browsePathNodeListHead = (browsePathNode*)EdgeMalloc(sizeof(browsePathNode));
    if(browsePathNodeListHead == NULL){
        EDGE_LOG(TAG, "Memory allocation failed.");
        return NULL;
    }
    browsePathNodeListHead->edgeNodeId = NULL;
    browsePathNodeListHead->browseName = NULL;
    browsePathNodeListHead->next = NULL;
    browsePathNodeListHead->pre = NULL;
    browsePathNodeListTail = browsePathNodeListHead;
    return browsePathNodeListHead;
}

browsePathNode* PushBrowsePathNode(EdgeNodeId *edgeNodeId, unsigned char *browseName){
    if(browsePathNodeListTail == NULL || browsePathNodeListHead == NULL){
            return NULL;
    }
    browsePathNode *newNode = (browsePathNode*)EdgeMalloc(sizeof(browsePathNode));
    if(newNode == NULL){
            EDGE_LOG(TAG, "Memory allocation failed.");
            return NULL;
    }
    newNode->browseName = browseName;
    newNode->edgeNodeId = edgeNodeId;
    newNode->pre = browsePathNodeListTail;
    newNode->next = NULL;
    browsePathNodeListTail->next = newNode;
    browsePathNodeListTail = newNode;
    return newNode;
}

void PopBrowsePathNode(){
    if(browsePathNodeListTail == NULL || browsePathNodeListHead == NULL
            || browsePathNodeListTail == browsePathNodeListHead){
        EDGE_LOG(TAG, "Browse Path Node Pop Error");
        return;
    }
    browsePathNode *deleteNode = browsePathNodeListTail;
    browsePathNodeListTail = browsePathNodeListTail->pre;
    browsePathNodeListTail->next = NULL;
    EdgeFree(deleteNode);
}

static unsigned char *getCurrentBrowsePath()
{
    if(browsePathNodeListTail == NULL || browsePathNodeListHead == NULL)
    {
        return NULL;
    }

    const size_t blockSize = 100;
    size_t curSize = blockSize;
    int lastUsed = -1;
    unsigned char *browsePath = (unsigned char *)EdgeMalloc(curSize * sizeof(unsigned char));
    if(IS_NULL(browsePath))
    {
        EDGE_LOG(TAG, "Memory allocation failed.");
        return NULL;
    }

    for(browsePathNode *ptr = browsePathNodeListHead->next; ptr != NULL ; ptr = ptr->next)
    {
        /*EdgeNodeTypeCommon type = ptr->edgeNodeId->type;
        if(type == INTEGER){
        printf("/%d",ptr->edgeNodeId->integerNodeId);
        }else if( type == STRING){
        printf("/%s",ptr->edgeNodeId->nodeId);
        }*/
        if(IS_NULL(ptr->browseName))
        {
            continue;
        }
        size_t strLen = strlen((char *)ptr->browseName);
        if(lastUsed+strLen+2 >= curSize)
        {
            curSize += blockSize;
            unsigned char *newLoc = (unsigned char *)EdgeRealloc(browsePath, curSize);
            if(IS_NULL(newLoc))
            {
                EDGE_LOG(TAG, "EdgeRealloc Memory allocation failed.");
                EdgeFree(browsePath);
                return NULL;
            }
            browsePath = newLoc;
        }
        browsePath[++lastUsed] = '/';
        memcpy(browsePath+(++lastUsed), ptr->browseName, strLen);
        lastUsed += strLen-1;
    }
    if(lastUsed < 0)
    {
        EdgeFree(browsePath);
        return NULL;
    }
    browsePath[lastUsed+1] = '\0';
    return browsePath;
}

unsigned char *getCompleteBrowsePath(char *browseName, UA_NodeId* nodeId, UA_LocalizedText description)
{
    char *nodeIdInfo = NULL;
    int browseNameLen = 0;
    int nodeIdInfoLen = 0;
    if(IS_NOT_NULL(browseName))
    {
        const int bufferSize = 20;
        browseNameLen = strlen(browseName);
        nodeIdInfo = (char *)EdgeCalloc(bufferSize, sizeof(char));
        if(IS_NULL(nodeIdInfo))
        {
            EDGE_LOG(TAG, "Memory allocation failed.");
            return NULL;
        }

        char curType = getCharacterNodeIdType(nodeId->identifierType);
        if (UA_NODEIDTYPE_STRING == nodeId->identifierType)
        {
            unsigned char *valueType = convertUAStringToUnsignedChar(&description.text);
            if(IS_NOT_NULL(valueType))
            {
                if (0 == strncmp((const char*)valueType, "v=", 2))
                {
                    snprintf(nodeIdInfo, bufferSize*sizeof(char), "{%d;%c;%s}", nodeId->namespaceIndex, curType, valueType);
                }
                else
                {
                    snprintf(nodeIdInfo, bufferSize*sizeof(char), "{%d;%c}", nodeId->namespaceIndex, curType);
                }
                EdgeFree(valueType);
            }
        }
        else
        {
            snprintf(nodeIdInfo, bufferSize*sizeof(char), "{%d;%c}", nodeId->namespaceIndex, curType);
        }

        if(IS_NOT_NULL(nodeIdInfo))
        {
            nodeIdInfoLen = strlen(nodeIdInfo);
        }
    }

    unsigned char *browsePath = getCurrentBrowsePath();
    int pathLen = IS_NOT_NULL(browsePath) ? strlen((char *)browsePath) : 0;
    unsigned char *completePath = (unsigned char *)EdgeCalloc(pathLen+nodeIdInfoLen+browseNameLen + 2, sizeof(unsigned char));
    if(IS_NULL(completePath))
    {
        EDGE_LOG(TAG, "Memory allocation failed.");
        EdgeFree(nodeIdInfo);
        EdgeFree(browsePath);
        return NULL;
    }

    if(pathLen > 0)
    {
        memcpy(completePath, browsePath, pathLen);
    }

    if(browseNameLen > 0)
    {
        completePath[pathLen++] = '/';
        if(nodeIdInfoLen > 0)
        {
            memcpy(completePath + pathLen, nodeIdInfo, nodeIdInfoLen);
            pathLen += nodeIdInfoLen;
        }
        memcpy(completePath + pathLen, browseName, browseNameLen);
        pathLen += browseNameLen;
    }
    completePath[pathLen] = '\0';
    EdgeFree(nodeIdInfo);
    EdgeFree(browsePath);
    return completePath;
}

bool hasNode(UA_String *browseName)
{
    unsigned char *browseNameCharStr = convertUAStringToUnsignedChar(browseName);
    if(IS_NULL(browseNameCharStr))
    {
        return false;
    }

    bool found = false;
    for(browsePathNode *ptr = browsePathNodeListHead->next; ptr != NULL ; ptr = ptr->next)
    {
        if(IS_NOT_NULL(ptr->browseName) &&
            !memcmp(ptr->browseName, browseNameCharStr, strlen((char *)ptr->browseName)+1))
        {
            found = true;
            break;
        }
    }

    FREE(browseNameCharStr);
    return found;
}

unsigned char *convertNodeIdToString(UA_NodeId *nodeId)
{
    if(!nodeId)
    {
        return NULL;
    }

    unsigned char *browseName = NULL;
    if(UA_NODEIDTYPE_STRING == nodeId->identifierType)
    {
        browseName = convertUAStringToUnsignedChar(&nodeId->identifier.string);
    }
    /*else if(UA_NODEIDTYPE_NUMERIC == nodeId->identifierType)
    {
        int maxDigits = 10;
        browseName = (unsigned char *)EdgeCalloc(maxDigits+1, sizeof(unsigned char));
        VERIFY_NON_NULL(browseName, NULL);
        snprintf((char *)browseName, maxDigits, "%" PRIu32, nodeId->identifier.numeric);
    } */
    // TODO: Handle GUID and ByteString
    return browseName;
}

static EdgeRequest *getEdgeRequestForViewBrowse()
{
    EdgeNodeInfo *nodeInfo = (EdgeNodeInfo *) EdgeCalloc(1, sizeof(EdgeNodeInfo));
    if(IS_NULL(nodeInfo))
    {
    EDGE_LOG(TAG, "Memory allocation failed.");
    return NULL;
    }

    nodeInfo->nodeId = (EdgeNodeId *) EdgeCalloc(1, sizeof(EdgeNodeId));
    if(IS_NULL(nodeInfo->nodeId))
    {
    EDGE_LOG(TAG, "Memory allocation failed.");
    EdgeFree(nodeInfo);
    return NULL;
    }
    nodeInfo->nodeId->type = INTEGER;
    nodeInfo->nodeId->integerNodeId = ViewsFolder;
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

ViewNodeInfo_t *getNodeInfo(UA_NodeId *nodeId, UA_String *browseName)
{
    if(IS_NULL(nodeId) || IS_NULL(browseName))
    {
        return NULL;
    }

    ViewNodeInfo_t *nodeInfo = (ViewNodeInfo_t *)EdgeCalloc(1, sizeof(ViewNodeInfo_t));
    if(IS_NULL(nodeInfo))
    {
        EDGE_LOG(TAG, "Memory allocation failed.");
        return NULL;
    }

    if(browseName->length > 0)
    {
        nodeInfo->browseName = convertUAStringToUnsignedChar(browseName);
        if(IS_NULL(nodeInfo->browseName))
        {
            EDGE_LOG(TAG, "Failed to convert UA_String to unsigned char string.");
            EdgeFree(nodeInfo);
            return NULL;
        }
    }

    nodeInfo->nodeId = (UA_NodeId *)EdgeCalloc(1, sizeof(UA_NodeId));
    if(IS_NULL(nodeInfo->nodeId))
    {
        EDGE_LOG(TAG, "Memory allocation failed.");
        EdgeFree(nodeInfo->browseName);
        EdgeFree(nodeInfo);
        return NULL;
    }

    if(UA_STATUSCODE_GOOD != UA_NodeId_copy(nodeId, nodeInfo->nodeId))
    {
        EDGE_LOG(TAG, "Failed to copy the node id.");
        EdgeFree(nodeInfo->nodeId);
        EdgeFree(nodeInfo->browseName);
        EdgeFree(nodeInfo);
        return NULL;
    }

    return nodeInfo;
}

void destroyViewListMembers(List *ptr)
{
    if(IS_NULL(ptr))
    {
        return;
    }

    ViewNodeInfo_t *nodeInfo;
    while(ptr)
    {
        nodeInfo = ptr->data;
        EdgeFree(nodeInfo->browseName);
        UA_NodeId_delete(nodeInfo->nodeId);
        EdgeFree(nodeInfo);
        ptr=ptr->link;
    }
}

static EdgeMessage *prepareEdgeMessageForBrowseView(EdgeMessage *msg, List *viewNodeList)
{
    if(IS_NULL(msg) || IS_NULL(viewNodeList))
    {
        return NULL;
    }
    EdgeMessage *browseViewMsg = (EdgeMessage *)EdgeCalloc(1, sizeof(EdgeMessage));
    if(IS_NULL(browseViewMsg))
    {
        return NULL;
    }

    browseViewMsg->endpointInfo = (EdgeEndPointInfo *)EdgeCalloc(1, sizeof(EdgeEndPointInfo));
    if(IS_NULL(browseViewMsg->endpointInfo))
    {
        EdgeFree(browseViewMsg);
        return NULL;
    }
    browseViewMsg->endpointInfo->endpointUri = cloneString(msg->endpointInfo->endpointUri);
    browseViewMsg->type = SEND_REQUESTS;
    browseViewMsg->command = CMD_BROWSE;

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

EdgeStatusCode browse(UA_Client *client, EdgeMessage *msg, bool browseNext,
        NodesToBrowse_t *browseNodesInfo, int *msgIdList, size_t msgCount, List **viewList)
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
        bReq.continuationPoints = (UA_ByteString *) EdgeMalloc(
                bReq.continuationPointsSize * sizeof(UA_ByteString));
        VERIFY_NON_NULL(bReq.continuationPoints, STATUS_INTERNAL_ERROR);

        for (int i = 0; i < bReq.continuationPointsSize; ++i)
        {
            UA_ByteString *byteStr;
            bReq.continuationPoints[i] =
                    (byteStr = getUAStringFromEdgeContinuationPoint(msg->cpList->cp[i])) ?
                            *byteStr : UA_BYTESTRING_NULL;
            EdgeFree(byteStr);
        }
        browseNextResp = UA_Client_Service_browseNext(client, bReq);
        resp = (UA_BrowseResponse *) &browseNextResp;
        UA_BrowseNextRequest_deleteMembers(&bReq);
        EdgeFree(bReq.continuationPoints);
    }
    else
    {
        nodesToBrowse = getBrowseDescriptions(browseNodesInfo, msg,
            IS_NULL(viewList) ? BROWSE_NODECLASS_MASK : VIEW_NODECLASS_MASK);
        if (!nodesToBrowse)
        {
            EDGE_LOG(TAG, "Failed to create browse descriptions.");
            return STATUS_ERROR;
        }

        UA_BrowseRequest bReq;
        UA_BrowseRequest_init(&bReq);
        bReq.requestedMaxReferencesPerNode = msg->browseParam->maxReferencesPerNode;
        bReq.nodesToBrowse = nodesToBrowse;
        bReq.nodesToBrowseSize = browseNodesInfo->size;
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
        EdgeFree(nodesToBrowse);
        UA_BrowseResponse_deleteMembers(resp);
        return statusCode;
    }

    int *nextMsgIdList = NULL;
    NodesToBrowse_t *nextBrowseNodesInfo = NULL;
    EdgeStatusCode statusCode;
    int nodeIdUnknownCount = 0;
    EdgeNodeId *srcNodeId = NULL;
    unsigned char *srcBrowseName = NULL;
    for (size_t i = 0; i < resp->resultsSize; ++i)
    {
        freeEdgeNodeId(srcNodeId);
        srcNodeId = getEdgeNodeId(&browseNodesInfo->nodeId[i]);
        srcBrowseName = browseNodesInfo->browseName[i];
        if(IS_NULL(PushBrowsePathNode(srcNodeId, srcBrowseName)))
        {
            EDGE_LOG(TAG, "Push Node of Browse Path Error.");
            statusCode = STATUS_INTERNAL_ERROR;
            goto EXIT;
        }

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
        if (IS_NULL(nextMsgIdList))
        {
            EDGE_LOG(TAG, "Memory allocation failed.");
            statusCode = STATUS_INTERNAL_ERROR;
            goto EXIT;
        }

        nextBrowseNodesInfo = initNodesToBrowse(resp->results[i].referencesSize);
        if (IS_NULL(nextBrowseNodesInfo))
        {
            EDGE_LOG(TAG, "Memory allocation failed.");
            statusCode = STATUS_INTERNAL_ERROR;
            goto EXIT;
        }

        int nextMsgListCount = 0;
        size_t nextNodeListCount = 0;
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
#if DEBUG
                printNodeId(ref->nodeId.nodeId);
#endif
                if (!hasNode(&ref->browseName.name))
                {
                    if(IS_NULL(viewList))
                    {
                        size_t size = 1;
                        EdgeBrowseResult *browseResult = (EdgeBrowseResult *) EdgeCalloc(size,
                                sizeof(EdgeBrowseResult));
                        if (IS_NULL(browseResult))
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
                            EdgeFree(browseResult);
                            goto EXIT;
                        }

                        // EdgeVersatility in EdgeResponse will have the complete path to browse name (Including the browse name).
                        unsigned char *completePath = NULL;
                        if((!SHOW_SPECIFIC_NODECLASS) || (ref->nodeClass & SHOW_SPECIFIC_NODECLASS_MASK)){
                            completePath = getCompleteBrowsePath(browseResult->browseName, &(ref->nodeId.nodeId), ref->displayName);
                        }

                        invokeResponseCb(msg, msgId, srcNodeId, browseResult, size, completePath);
                        EdgeFree(completePath);
                        EdgeFree(browseResult->browseName);
                        EdgeFree(browseResult);
                    }
                    else if(UA_NODECLASS_VIEW == ref->nodeClass)
                    {
                        // This browse() is for views. If the current reference is a view node, then it will be added in the viewList.
                        // Application callback will not be invoked.
                        ViewNodeInfo_t *info = getNodeInfo(&ref->nodeId.nodeId, &ref->browseName.name);
                        if(IS_NULL(info))
                        {
                            EDGE_LOG(TAG, "Failed to copy node info from ReferenceDescription.");
                            statusCode = STATUS_INTERNAL_ERROR;
                            goto EXIT;
                        }
                        if(!addListNode(viewList, info))
                        {
                            EDGE_LOG(TAG, "Adding view node to list failed.");
                            statusCode = STATUS_INTERNAL_ERROR;
                            goto EXIT;
                        }
                    }

                    if(UA_NODECLASS_VARIABLE != ref->nodeClass)
                    {
                        nextBrowseNodesInfo->nodeId[nextNodeListCount] = ref->nodeId.nodeId;
                        nextBrowseNodesInfo->browseName[nextNodeListCount] = convertUAStringToUnsignedChar(&ref->browseName.name);
                        nextMsgIdList[nextMsgListCount] = msgId;
                        nextNodeListCount++;
                        nextMsgListCount++;
                    }
                }
                else
                {
                    EDGE_LOG(TAG, "Already visited this node in the current browse path.");
                }
            }
        }

        nextBrowseNodesInfo->size = nextNodeListCount;

        // Pass the continuation point for this result to the application.
        if (resp->results[i].continuationPoint.length > 0)
        {
            EDGE_LOG(TAG, "Passing continuation point to application.");
            invokeResponseCbForContinuationPoint(msg, msgId, srcNodeId,
                    &resp->results[i].continuationPoint);
        }

        if (nextNodeListCount > 0)
        {
            browse(client, msg, false, nextBrowseNodesInfo, nextMsgIdList, nextMsgListCount, viewList);
        }
        PopBrowsePathNode();
        freeEdgeNodeId(srcNodeId);
        srcNodeId = NULL;
        FREE(nextMsgIdList); // P
        destroyNodesToBrowse(nextBrowseNodesInfo, false);
        nextBrowseNodesInfo = NULL;
    }

    statusCode = STATUS_OK;

    EXIT: EdgeFree(nextMsgIdList);
    destroyNodesToBrowse(nextBrowseNodesInfo, false);
    EdgeFree(nodesToBrowse);
    freeEdgeNodeId(srcNodeId);
    UA_BrowseResponse_deleteMembers(resp);
    return statusCode;
}

EdgeResult executeBrowse(UA_Client *client, EdgeMessage *msg, bool browseNext)
{
    EdgeResult result;
    if (IS_NULL(client))
    {
        EDGE_LOG(TAG, "Invalid client handle parameter.");
        result.code = STATUS_PARAM_INVALID;
        return result;
    }

    if (IS_NULL(msg))
    {
        EDGE_LOG(TAG, "Invalid edge message parameter.");
        result.code = STATUS_PARAM_INVALID;
        return result;
    }

    if (browseNext && (IS_NULL(msg->cpList) || msg->cpList->count < 1))
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
    size_t nodesToBrowseSize;
    NodesToBrowse_t *browseNodesInfo = NULL;
    if (browseNext)
    {
        nodesToBrowseSize = msg->cpList->count;
    }
    else
    {
        nodesToBrowseSize = (msg->requestLength) ? msg->requestLength : 1;
    }

    msgIdList = (int *) EdgeCalloc(nodesToBrowseSize, sizeof(int));
    if (IS_NULL(msgIdList))
    {
        EDGE_LOG(TAG, "Memory allocation failed.");
        result.code = STATUS_INTERNAL_ERROR;
        return result;
    }

    browseNodesInfo = initNodesToBrowse(nodesToBrowseSize);
    if (IS_NULL(browseNodesInfo))
    {
        EDGE_LOG(TAG, "Memory allocation failed.");
        EdgeFree(msgIdList);
        result.code = STATUS_INTERNAL_ERROR;
        return result;
    }

    if (msg->type == SEND_REQUEST)
    {
        EDGE_LOG(TAG, "Message Type: " SEND_REQUEST_DESC);
        UA_NodeId *nodeId;
        browseNodesInfo->nodeId[0] = (nodeId = getNodeId(msg->request)) ? *nodeId : UA_NODEID_NULL;
        browseNodesInfo->browseName[0] = convertNodeIdToString(nodeId);
        msgIdList[0] = 0;
        EdgeFree(nodeId);
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
            destroyNodesToBrowse(browseNodesInfo, true);
            EdgeFree(msgIdList);
            return result;
        }

        for (size_t i = 0; i < nodesToBrowseSize; ++i)
        {
            UA_NodeId *nodeId;
            browseNodesInfo->nodeId[i] = (nodeId = getNodeIdMultiReq(msg, i)) ? *nodeId : UA_NODEID_NULL;
            browseNodesInfo->browseName[i] = convertNodeIdToString(nodeId);
            EdgeFree(nodeId);
            msgIdList[i] = i;
        }
    }

    if(IS_NULL(InitBrowsePathNodeList()))
    {
        EDGE_LOG(TAG, "Init Browse Path List Error.");
        result.code = STATUS_INTERNAL_ERROR;
    }
    else
    {
        EdgeStatusCode statusCode = browse(client, msg, browseNext, browseNodesInfo,
        msgIdList, nodesToBrowseSize, NULL);
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

    destroyNodesToBrowse(browseNodesInfo, true);
    EdgeFree(msgIdList);
    return result;
}

EdgeResult executeBrowseViews(UA_Client *client, EdgeMessage *msg)
{
    EdgeResult result;
    if (IS_NULL(client))
    {
        EDGE_LOG(TAG, "Invalid client handle parameter.");
        result.code = STATUS_PARAM_INVALID;
        return result;
    }

    if (IS_NULL(msg))
    {
        EDGE_LOG(TAG, "Invalid edge message parameter.");
        result.code = STATUS_PARAM_INVALID;
        return result;
    }

    if (msg->type != SEND_REQUEST && msg->type != SEND_REQUESTS)
    {
        EDGE_LOG(TAG, "Invalid message type.");
        result.code = STATUS_NOT_SUPPORT;
        return result;
    }

    msg->request = getEdgeRequestForViewBrowse();
    if(IS_NULL(msg->request))
    {
        EDGE_LOG(TAG, "Failed to form request for browsing views.");
        result.code = STATUS_PARAM_INVALID;
        return result;
    }

    msg->browseParam = (EdgeBrowseParameter *)EdgeCalloc(1, sizeof(EdgeBrowseParameter));
    if(IS_NULL(msg->browseParam))
    {
        EDGE_LOG(TAG, "Failed to form request for browsing views.");
        freeEdgeRequest(msg->request);
        msg->request = NULL;
        result.code = STATUS_PARAM_INVALID;
        return result;
    }
    msg->browseParam->direction = DIRECTION_FORWARD;
    msg->browseParam->maxReferencesPerNode = 0;

    int nodesToBrowseSize = 1;
    int *msgIdList = (int *) EdgeCalloc(nodesToBrowseSize, sizeof(int));
    if (IS_NULL(msgIdList))
    {
        EDGE_LOG(TAG, "Memory allocation failed.");
        freeEdgeRequest(msg->request);
        msg->request = NULL;
        EdgeFree(msg->browseParam);
        msg->browseParam = NULL;
        result.code = STATUS_INTERNAL_ERROR;
        return result;
    }

    NodesToBrowse_t *browseNodesInfo = initNodesToBrowse(nodesToBrowseSize);
    if (IS_NULL(browseNodesInfo))
    {
        EDGE_LOG(TAG, "Memory allocation failed.");
        EdgeFree(msgIdList);
        freeEdgeRequest(msg->request);
        msg->request = NULL;
        EdgeFree(msg->browseParam);
        msg->browseParam = NULL;
        result.code = STATUS_INTERNAL_ERROR;
        return result;
    }

    UA_NodeId *nodeId;
    browseNodesInfo->nodeId[0] = (nodeId = getNodeId(msg->request)) ? *nodeId : UA_NODEID_NULL;
    browseNodesInfo->browseName[0] = convertNodeIdToString(nodeId);
    msgIdList[0] = 0;
    EdgeFree(nodeId);

    if(IS_NULL(InitBrowsePathNodeList()))
    {
        EDGE_LOG(TAG, "Init Browse Path List Error.");
        result.code = STATUS_INTERNAL_ERROR;
    }
    else
    {
        List *viewList = NULL;
        EdgeStatusCode statusCode = browse(client, msg, false, browseNodesInfo,
        msgIdList, nodesToBrowseSize, &viewList);
        if (statusCode != STATUS_OK)
        {
            EDGE_LOG(TAG, "Browse failed.");
            result.code = STATUS_ERROR;
        }
        else
        {
            EdgeMessage *browseViewMsg = prepareEdgeMessageForBrowseView(msg, viewList);
            if(IS_NULL(browseViewMsg))
            {
                EDGE_LOG(TAG, "Failed to prepare message for browse view request.");
                result.code = STATUS_INTERNAL_ERROR;
            }
            else
            {
                result = executeBrowse(client, browseViewMsg, false);
                if(result.code != STATUS_OK)
                {
                    EDGE_LOG(TAG, "Browse view request failed");
                }
                freeEdgeMessage(browseViewMsg);
            }
        }
        destroyViewListMembers(viewList);
        deleteList(&viewList);
    }

    freeEdgeRequest(msg->request);
    msg->request = NULL;
    EdgeFree(msg->browseParam);
    msg->browseParam = NULL;
    destroyNodesToBrowse(browseNodesInfo, true);
    EdgeFree(msgIdList);
    return result;
}

void resgisterBrowseResponseCallback(response_cb_t callback) {
    g_responseCallback = callback;
}
