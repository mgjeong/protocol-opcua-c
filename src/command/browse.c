#include "browse.h"
#include "edge_utils.h"
#include "edge_node_type.h"

#include <stdio.h>

#define DEBUG 0
#if DEBUG
#define LOG(param) printf("[Browse] %s\n", param)
#else
#define LOG(param)
#endif

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

static BrowseMap *map;
static int mapSize;

static bool initMap(int size)
{
    map = (BrowseMap *)calloc(size, sizeof(BrowseMap));
    if(!map)
    {
        return false;
    }
    mapSize = size;
    return true;
}

static void destroyMap()
{
    for(int i = 0; i < mapSize; ++i)
    {
        BrowseMapListNode *listPtr = map[i].listHead;
        BrowseMapListNode *next = NULL;
        while(listPtr)
        {
            next = listPtr->next;
            free(listPtr->browseName);
            free(listPtr);
            listPtr = next;
        }
    }
    free(map);
    map = NULL;
    mapSize = 0;
}

static char *convertUAStringToString(UA_String *browseName)
{
    if(!browseName || browseName->length <= 0)
    {
        return NULL;
    }

    char *str = (char *)calloc(browseName->length+1, sizeof(char));
    if(!str)
    {
        return NULL;
    }

    for(int i = 0; i < browseName->length; ++i)
    {
        str[i] = browseName->data[i];
    }
    return str;
}

static BrowseMapListNode *createBrowseMapListNode(UA_NodeId nodeId, UA_String *browseName)
{
    BrowseMapListNode *node = (BrowseMapListNode *)malloc(sizeof(BrowseMapListNode));
    if(!node)
    {
        return NULL;
    }
    if(UA_STATUSCODE_GOOD != UA_NodeId_copy(&nodeId, &node->nodeId))
    {
        free(node);
        return NULL;
    }
    node->browseName = convertUAStringToString(browseName);
    node->next = NULL;
    return node;
}

static bool addToMap(int index, UA_NodeId nodeId, UA_String *browseName)
{
    if(index >= mapSize)
    {
        return false;
    }

    BrowseMapListNode *newNode = createBrowseMapListNode(nodeId, browseName);
    if(!newNode)
    {
        return false;
    }

    newNode->next = map[index].listHead;
    map[index].listHead = newNode;
    map[index].listSize++;
    return true;
}

static bool hasNode(int index, UA_NodeId nodeId)
{
    if(mapSize <= 0 || index >= mapSize)
        return false;

    BrowseMapListNode *ptr = map[index].listHead;
    while(ptr)
    {
        if(UA_NodeId_equal(&nodeId, &ptr->nodeId))
        {
            return true;
        }
        ptr = ptr->next;
    }

    return false;
}

static const int BROWSE_DESCRIPTION_NODECLASS_MASK =
    UA_NODECLASS_OBJECT | UA_NODECLASS_VARIABLE |
    UA_NODECLASS_METHOD | UA_NODECLASS_REFERENCETYPE;

static UA_NodeId *getNodeId(EdgeRequest *req)
{
    if(!req)
    {
        return NULL;
    }
    UA_NodeId *node = (UA_NodeId *)calloc(1, sizeof(UA_NodeId));
    if(!node)
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
    if(!msg)
    {
        return NULL;
    }
    return getNodeId(msg->requests[reqId]);
}

static EdgeNodeInfo *getEndpoint(EdgeMessage *msg, int msgId)
{
    if(msg->type == SEND_REQUEST)
    {
        return msg->request->nodeInfo;
    }
    return msg->requests[msgId]->nodeInfo;
}

static bool checkStatusGood(UA_StatusCode status)
{
     return (UA_STATUSCODE_GOOD == status) ? true : false;
}

static UA_BrowseDescription *getBrowseDescriptions(UA_NodeId *nodeIdList, int count, EdgeMessage *msg)
{
    if(!msg->browseParam)
    {
        return NULL;
    }

    int direct = msg->browseParam->direction;
    UA_BrowseDirection directionParam = UA_BROWSEDIRECTION_FORWARD;
    if(DIRECTION_INVERSE == direct)
    {
        directionParam = UA_BROWSEDIRECTION_INVERSE;
    }
    else if(DIRECTION_BOTH == direct)
    {
        directionParam = UA_BROWSEDIRECTION_BOTH;
    }

    UA_BrowseDescription *browseDesc = (UA_BrowseDescription *)UA_calloc(count, sizeof(UA_BrowseDescription));
    if(NULL == browseDesc)
    {
        LOG("Memory allocation failed.");
        return NULL;
    }

    for(int id = 0; id < count; ++id)
    {
        browseDesc[id].nodeId = nodeIdList[id];
        browseDesc[id].browseDirection = directionParam;
        browseDesc[id].referenceTypeId = UA_NODEID_NUMERIC(SYSTEM_NAMESPACE_INDEX, UA_NS0ID_REFERENCES);
        browseDesc[id].includeSubtypes = true;
        browseDesc[id].nodeClassMask = BROWSE_DESCRIPTION_NODECLASS_MASK;
        browseDesc[id].resultMask = UA_BROWSERESULTMASK_ALL;
    }
    return browseDesc;
}

void invokeErrorCb(EdgeNodeInfo *nodeInfo, EdgeStatusCode edgeResult, const char *versatileValue)
{
    EdgeMessage *resultMsg = (EdgeMessage*) calloc(1, sizeof(EdgeMessage));
    if(!resultMsg)
    {
        goto EXIT;
    }

    EdgeEndPointInfo *epInfo = (EdgeEndPointInfo *) calloc(1, sizeof(EdgeEndPointInfo));
    if(!epInfo)
    {
        goto EXIT;
    }

    epInfo->endpointUri = cloneString(WELL_KNOWN_LOCALHOST_URI_VALUE);
    resultMsg->endpointInfo = epInfo;
    resultMsg->type = ERROR;
    resultMsg->result = createEdgeResult(edgeResult);
    if(!resultMsg->result)
    {
        goto EXIT;
    }

    EdgeVersatility *versatileVal = (EdgeVersatility *) calloc(1, sizeof(EdgeVersatility));
    if(!versatileVal)
    {
        goto EXIT;
    }
    versatileVal->isArray = false;
    versatileVal->value = cloneString(versatileValue);

    EdgeResponse *response = (EdgeResponse *) calloc(1, sizeof(EdgeResponse));
    if(!response)
    {
        freeEdgeVersatility(versatileVal);
        goto EXIT;
    }
    response->message = versatileVal;
    response->nodeInfo = cloneEdgeNodeInfo(nodeInfo);

    EdgeResponse **responses = (EdgeResponse **) calloc(1, sizeof(EdgeResponse *));
    if(!responses)
    {
        freeEdgeResponse(response);
        goto EXIT;
    }
    responses[0] = response;
    resultMsg->responses = responses;
    resultMsg->responseLength = 1;

    onResponseMessage(resultMsg);

EXIT:
    freeEdgeMessage(resultMsg);
}

bool checkContinuationPoint(UA_ByteString cp, EdgeNodeInfo *ep)
{
    bool retVal = true;
    /*if(cp.length <= 0)
    {
        LOG("Error: " CONTINUATIONPOINT_EMPTY);
        invokeErrorCb(ep, STATUS_ERROR, CONTINUATIONPOINT_EMPTY);
        retVal = false;
    }
    else*/ if(cp.length >= 1000)
    {
        LOG("Error: " CONTINUATIONPOINT_LONG);
        invokeErrorCb(ep, STATUS_ERROR, CONTINUATIONPOINT_LONG);
        retVal = false;
    }
    return retVal;
}

bool checkBrowseName(UA_String browseName, EdgeNodeInfo *ep)
{
    bool retVal = true;
    if(browseName.length <= 0 || NULL == browseName.data)
    {
        LOG("Error: " BROWSENAME_EMPTY);
        invokeErrorCb(ep, STATUS_ERROR, BROWSENAME_EMPTY);
        retVal = false;
    }
    else if(browseName.length >= 1000)
    {
        LOG("Error: " BROWSENAME_LONG);
        invokeErrorCb(ep, STATUS_ERROR, BROWSENAME_LONG);
        retVal = false;
    }

    return retVal;
}

bool checkNodeClass(UA_NodeClass nodeClass, EdgeNodeInfo *ep)
{
    bool retVal = true;
    if(false == isNodeClassValid(nodeClass))
    {
        LOG("Error: " NODECLASS_INVALID);
        invokeErrorCb(ep, STATUS_ERROR, NODECLASS_INVALID);
        retVal = false;
    }
    else if((nodeClass & BROWSE_DESCRIPTION_NODECLASS_MASK) == 0)
    {
        LOG("Error: " STATUS_VIEW_NOTINCLUDE_NODECLASS_VALUE);
        invokeErrorCb(ep, STATUS_ERROR, STATUS_VIEW_NOTINCLUDE_NODECLASS_VALUE);
        retVal = false;
    }
    return retVal;
}

bool checkDisplayName(UA_String displayName, EdgeNodeInfo *ep)
{
    bool retVal = true;
    if(displayName.length <= 0 || NULL == displayName.data)
    {
        LOG("Error: " DISPLAYNAME_EMPTY);
        invokeErrorCb(ep, STATUS_ERROR, DISPLAYNAME_EMPTY);
        retVal = false;
    }
    else if(displayName.length >= 1000)
    {
        LOG("Error: " DISPLAYNAME_LONG);
        invokeErrorCb(ep, STATUS_ERROR, DISPLAYNAME_LONG);
        retVal = false;
    }

    return retVal;
}

bool checkNodeId(UA_ExpandedNodeId nodeId, EdgeNodeInfo *ep)
{
    bool retVal = true;
    if(UA_NodeId_isNull(&nodeId.nodeId))
    {
        LOG("Error: " NODEID_NULL);
        invokeErrorCb(ep, STATUS_ERROR, NODEID_NULL);
        retVal = false;
    }
    else if(nodeId.serverIndex != 0)
    {
        LOG("Error: " NODEID_SERVERINDEX);
        invokeErrorCb(ep, STATUS_ERROR, NODEID_SERVERINDEX);
        retVal = false;
    }
    return retVal;
}

bool checkReferenceTypeId(UA_NodeId nodeId, EdgeNodeInfo *ep)
{
    bool retVal = true;
    if(UA_NodeId_isNull(&nodeId))
    {
        LOG("Error: " REFERENCETYPEID_NULL);
        invokeErrorCb(ep, STATUS_ERROR, REFERENCETYPEID_NULL);
        retVal = false;
    }
    return retVal;
}

static void invokeResponseCb(EdgeMessage *msg, int msgId, EdgeBrowseResult *browseResult, int size)
{
    EdgeMessage *resultMsg = (EdgeMessage*) calloc(1, sizeof(EdgeMessage));
    if(!resultMsg)
    {
        LOG("Memory allocation failed.");
        return;
    }

    resultMsg->type = BROWSE_RESPONSE;
    resultMsg->browseResult = browseResult;
    resultMsg->browseResultLength = size;

    resultMsg->endpointInfo = cloneEdgeEndpointInfo(msg->endpointInfo);
    if(!resultMsg->endpointInfo)
    {
        LOG("Failed to clone the EdgeEndpointInfo.");
        freeEdgeMessage(resultMsg);
        return;
    }

    EdgeResponse *response = (EdgeResponse *) calloc(1, sizeof(EdgeResponse));
    if(!response)
    {
        LOG("Memory allocation failed.");
        freeEdgeMessage(resultMsg);
        return;
    }
    response->nodeInfo = cloneEdgeNodeInfo(getEndpoint(msg, msgId));
    response->requestId = msgId; // Response for msgId'th request.
    EdgeResponse **responses = (EdgeResponse **) calloc(1, sizeof(EdgeResponse *));
    if(!responses)
    {
        LOG("Memory allocation failed.");
        freeEdgeResponse(response);
        freeEdgeMessage(resultMsg);
        return;
    }
    responses[0] = response;
    resultMsg->responses = responses;
    resultMsg->responseLength = 1;

    onResponseMessage(resultMsg);
    freeEdgeMessage(resultMsg);
}

void printNodeId(UA_NodeId n1)
{
    switch(n1.identifierType) {
        case UA_NODEIDTYPE_NUMERIC:
            printf("Numeric: %d\n", n1.identifier.numeric);
            break;
        case UA_NODEIDTYPE_STRING:
            printf("String: %s\n", convertUAStringToString(&n1.identifier.string));
            break;
        case UA_NODEIDTYPE_BYTESTRING:
            printf("Byte String: %s\n", convertUAStringToString(&n1.identifier.byteString));
            break;
        case UA_NODEIDTYPE_GUID:
            printf("GUID\n");
            break;
    }
}

EdgeStatusCode browse(UA_Client *client, EdgeMessage *msg, UA_NodeId *nodeIdList,
    int nodeCount, int *msgIdList, int msgCount)
{
    UA_BrowseDescription *nodesToBrowse = getBrowseDescriptions(nodeIdList, nodeCount, msg);
    if(!nodesToBrowse)
    {
        LOG("Failed to create browse descriptions.");
        return STATUS_ERROR;
    }

    UA_BrowseRequest bReq;
    UA_BrowseRequest_init(&bReq);
    bReq.requestedMaxReferencesPerNode = 0;
    //bReq.requestedMaxReferencesPerNode = msg->browseParam->maxReferencesPerNode;
    bReq.nodesToBrowse = nodesToBrowse;
    bReq.nodesToBrowseSize = nodeCount;
    UA_BrowseResponse bResp = UA_Client_Service_browse(client, bReq);
    if(bResp.responseHeader.serviceResult != UA_STATUSCODE_GOOD ||   bResp.resultsSize <= 0)
    {
        char *versatileVal;
        EdgeStatusCode statusCode;
        // Invoke error callback
        if(bResp.resultsSize <= 0)
        {
            statusCode = STATUS_VIEW_BROWSERESULT_EMPTY;
            versatileVal = STATUS_VIEW_BROWSERESULT_EMPTY_VALUE;
            LOG("Error: Empty browse response!!!");
        }
        else
        {
            statusCode = STATUS_SERVICE_RESULT_BAD;
            versatileVal = STATUS_SERVICE_RESULT_BAD_VALUE;
            UA_StatusCode serviceResult = bResp.responseHeader.serviceResult;
            printf("Error in browse :: 0x%08x(%s)\n", serviceResult, UA_StatusCode_name(serviceResult));
        }

        invokeErrorCb(getEndpoint(msg, 0), statusCode, versatileVal);
        free(nodesToBrowse);
        UA_BrowseResponse_deleteMembers(&bResp);
        return statusCode;
    }

    int *nextMsgIdList = NULL;
    UA_NodeId *nextNodeIdList = NULL;
    EdgeStatusCode statusCode;
    int nodeIdUnknownCount = 0;
    for (size_t i = 0; i < bResp.resultsSize; ++i)
    {
        UA_StatusCode status = bResp.results[i].statusCode;
        int msgId = msgIdList[i];
        EdgeNodeInfo *ep = getEndpoint(msg, msgId);
        EdgeBrowseDirection direction = msg->browseParam->direction;
        if(checkStatusGood(status) == false)
        {
            if(UA_STATUSCODE_BADNODEIDUNKNOWN == status)
                nodeIdUnknownCount++;

            if(nodeIdUnknownCount == bResp.resultsSize)
            {
                LOG("Error: " STATUS_VIEW_NODEID_UNKNOWN_ALL_RESULTS_VALUE);
                invokeErrorCb(ep, STATUS_VIEW_NODEID_UNKNOWN_ALL_RESULTS,
                        STATUS_VIEW_NODEID_UNKNOWN_ALL_RESULTS_VALUE);
            }
            else
            {
                const char *statusStr = UA_StatusCode_name(status);
                invokeErrorCb(ep, STATUS_ERROR, statusStr);
            }
            continue;
        }

        checkContinuationPoint(bResp.results[i].continuationPoint, ep);

        nextMsgIdList = (int *)calloc(bResp.results[i].referencesSize, sizeof(int));
        if(!nextMsgIdList)
        {
            LOG("Memory allocation failed.");
            statusCode = STATUS_INTERNAL_ERROR;
            goto EXIT;
        }

        nextNodeIdList = (UA_NodeId *)calloc(bResp.results[i].referencesSize, sizeof(UA_NodeId));
        if(!nextNodeIdList)
        {
            LOG("Memory allocation failed.");
            statusCode = STATUS_INTERNAL_ERROR;
            goto EXIT;
        }

        int nextMsgListCount = 0;
        int nextNodeListCount = 0;
        for (size_t j = 0; j < bResp.results[i].referencesSize; ++j)
        {
            bool isError = false;
            UA_ReferenceDescription *ref = &(bResp.results[i].references[j]);
            if((direction == DIRECTION_FORWARD && ref->isForward == false)
                || (direction == DIRECTION_INVERSE && ref->isForward == true))
            {
                LOG("Error: " STATUS_VIEW_DIRECTION_NOT_MATCH_VALUE);
                invokeErrorCb(ep, STATUS_VIEW_DIRECTION_NOT_MATCH,
                    STATUS_VIEW_DIRECTION_NOT_MATCH_VALUE);
                isError = true;
            }

            if(!checkBrowseName(ref->browseName.name, ep))
                isError = true;
            if(!checkNodeClass(ref->nodeClass, ep))
                isError = true;
            if(!checkDisplayName(ref->displayName.text, ep))
                isError = true;
            if(!checkNodeId(ref->nodeId, ep))
                isError = true;
            if(!checkReferenceTypeId(ref->referenceTypeId, ep))
                isError = true;

            if(!isError)
            {
                int size = 1;
                EdgeBrowseResult *browseResult = (EdgeBrowseResult *)calloc(size, sizeof(EdgeBrowseResult *));
                if(!browseResult)
                {
                    LOG("Memory allocation failed.");
                    statusCode = STATUS_INTERNAL_ERROR;
                    goto EXIT;
                }
                browseResult->browseName = convertUAStringToString(&ref->browseName.name);
                if(!browseResult->browseName)
                {
                    LOG("Memory allocation failed.");
                    statusCode = STATUS_INTERNAL_ERROR;
                    free(browseResult);
                    goto EXIT;
                }
                invokeResponseCb(msg, msgId, browseResult, size);
#if DEBUG
                printNodeId(ref->nodeId.nodeId);
#endif
                if (!hasNode(msgId, ref->nodeId.nodeId))
                {
                    LOG("Adding this NodeId in Map.");
                    if(!addToMap(msgId, ref->nodeId.nodeId, &ref->browseName.name))
                    {
                        LOG("Adding node to map failed.");
                        statusCode = STATUS_INTERNAL_ERROR;
                        goto EXIT;
                    }
                    nextNodeIdList[nextNodeListCount++] = ref->nodeId.nodeId;
                    nextMsgIdList[nextMsgListCount++] = msgId;
                }
                else
                {
                    LOG("Already added this NodeId in Map.");
                }
            }
        }
        if(nextNodeListCount > 0)
        {
            browse(client, msg, nextNodeIdList, nextNodeListCount, nextMsgIdList, nextMsgListCount);
        }
        free(nextMsgIdList); nextMsgIdList = NULL;
        free(nextNodeIdList); nextNodeIdList = NULL;
    }
    statusCode = STATUS_OK;

EXIT:
    free(nextMsgIdList);
    free(nextNodeIdList);
    free(nodesToBrowse);
    UA_BrowseResponse_deleteMembers(&bResp);
    return statusCode;
}

EdgeResult executeBrowse(UA_Client *client, EdgeMessage *msg)
{
    EdgeResult result;
    if (!client)
    {
        LOG("Invalid client handle parameter.");
        result.code = STATUS_PARAM_INVALID;
        return result;
    }

    if(!msg)
    {
        LOG("Invalid edge message parameter.");
        result.code = STATUS_PARAM_INVALID;
        return result;
    }

    if(msg->type != SEND_REQUEST && msg->type != SEND_REQUESTS)
    {
        LOG("Invalid message type.");
        result.code = STATUS_NOT_SUPPORT;
        return result;
    }

    int *msgIdList = NULL;
    UA_NodeId *nodeIdList = NULL;
    int nodesToBrowseSize = (msg->requestLength) ? msg->requestLength : 1;

    msgIdList = (int *)calloc(nodesToBrowseSize, sizeof(int));
    if(!msgIdList)
    {
        LOG("Memory allocation failed.");
        result.code = STATUS_INTERNAL_ERROR;
        return result;
    }

    nodeIdList = (UA_NodeId *)calloc(nodesToBrowseSize, sizeof(UA_NodeId));
    if(!nodeIdList)
    {
        LOG("Memory allocation failed.");
        free(msgIdList);
        result.code = STATUS_INTERNAL_ERROR;
        return result;
    }

    if(msg->type == SEND_REQUEST)
    {
        LOG("Message Type: " SEND_REQUEST_DESC);
        UA_NodeId *nodeId;
        nodeIdList[0] = (nodeId = getNodeId(msg->request)) ? *nodeId : UA_NODEID_NULL;
        msgIdList[0] = 0;
        free(nodeId);
    }
    else
    {
        LOG("Message Type: " SEND_REQUESTS_DESC);
        if(nodesToBrowseSize > MAX_BROWSEREQUEST_SIZE)
        {
            LOG("Error: " STATUS_VIEW_BROWSEREQUEST_SIZEOVER_VALUE);
            invokeErrorCb(getEndpoint(msg, 0), STATUS_ERROR, STATUS_VIEW_BROWSEREQUEST_SIZEOVER_VALUE);
            result.code = STATUS_ERROR;
            free(nodeIdList);
            free(msgIdList);
            return result;
        }

        for(int i = 0; i < nodesToBrowseSize; ++i)
        {
            UA_NodeId *nodeId;
            nodeIdList[i] = (nodeId = getNodeIdMultiReq(msg, i)) ? *nodeId : UA_NODEID_NULL;
            free(nodeId);
            msgIdList[i] = i;
        }
    }

    if(!initMap(nodesToBrowseSize))
    {
        LOG("Failed to initialize a Map.");
        result.code = STATUS_INTERNAL_ERROR;
        goto EXIT;
    }

    EdgeStatusCode statusCode = browse(client, msg, nodeIdList, nodesToBrowseSize, msgIdList, nodesToBrowseSize);
    if(statusCode != STATUS_OK)
    {
        LOG("Browse failed.");
        result.code = STATUS_ERROR;
        goto EXIT;
    }

    result.code = STATUS_OK;

EXIT:
    free(nodeIdList);
    free(msgIdList);
    destroyMap();
    return result;
}
