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

#include "browse_common.h"
#include "edge_node_type.h"
#include "edge_logger.h"
#include "edge_malloc.h"
#include "edge_open62541.h"
#include "message_dispatcher.h"
#include "command_adapter.h"
#include "uqueue.h"

#include <inttypes.h>
#include <string.h>

#define TAG "browse_common"
#define BROWSE_PATH_SEPERATOR "/"

static const int BROWSE_NODECLASS_MASK = UA_NODECLASS_OBJECT | UA_NODECLASS_VARIABLE
    | UA_NODECLASS_VIEW | UA_NODECLASS_METHOD;
static const int VIEW_NODECLASS_MASK = UA_NODECLASS_OBJECT | UA_NODECLASS_VIEW;
static const int SHOW_SPECIFIC_NODECLASS_MASK = UA_NODECLASS_VARIABLE | UA_NODECLASS_VIEW | UA_NODECLASS_METHOD;
static const int SHOW_SPECIFIC_NODECLASS = false;
static const char WELL_KNOWN_LOCALHOST_URI_VALUE[] = "opc.tcp://localhost";

static response_cb_t g_responseCallback = NULL;

typedef struct BrowseItem
{
    uint32_t reqId; // If client app rquested browse for N nodes, then a sequential request id from 0 to N-1 will be assigned.
    UA_NodeId *nodeId; // Node ID.
    unsigned char *browseName; // Browse name.
    uint32_t browseNameLen; // Length of browse name.
    unsigned char *browsePath; // Complete path starting from the root till this node. Ex: /a/b/c
    uint32_t browsePathLen; // Length of browse path.
} BrowseItem;

void setErrorResponseCallback(response_cb_t callback) {
    g_responseCallback = callback;
}

static unsigned char *convertUAStringToUnsignedChar(UA_String *uaStr)
{
    VERIFY_NON_NULL_MSG(uaStr, "uaStr is null\n", NULL);
    COND_CHECK_MSG((uaStr->length <= 0), "", NULL);

    unsigned char *str = (unsigned char *) EdgeCalloc(uaStr->length+1, sizeof(unsigned char));
    VERIFY_NON_NULL_MSG(str, "EdgeCalloc FAILED for  unsigned char String\n", NULL);

    for (int i = 0; i < uaStr->length; ++i)
    {
        str[i] = uaStr->data[i];
    }
    return str;
}

UA_NodeId *getNodeId(EdgeRequest *req)
{
    VERIFY_NON_NULL_MSG(req, "EdgeRequest is NULL\n", NULL);
    VERIFY_NON_NULL_MSG(req->nodeInfo, "EdgeRequest NodeInfo is NULL\n", NULL);
    VERIFY_NON_NULL_MSG(req->nodeInfo->nodeId, "EdgeRequest NodeId is NULL\n", NULL);

    UA_NodeId *node = (UA_NodeId *) EdgeCalloc(1, sizeof(UA_NodeId));
    VERIFY_NON_NULL_MSG(node, "EdgeCalloc FAILED for UA Node Id\n", NULL);
    if (req->nodeInfo->nodeId->type == EDGE_INTEGER)
    {
        *node = UA_NODEID_NUMERIC(req->nodeInfo->nodeId->nameSpace,
                req->nodeInfo->nodeId->integerNodeId);
    }
    else if (req->nodeInfo->nodeId->type == EDGE_STRING)
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

void invokeErrorCb(uint32_t srcMsgId, EdgeNodeId *srcNodeId,
        EdgeStatusCode edgeResult, const char *versatileValue)
{
    EdgeMessage *resultMsg = (EdgeMessage *) EdgeCalloc(1, sizeof(EdgeMessage));
    VERIFY_NON_NULL_NR_MSG(resultMsg, "EdgeCalloc FAILED for EdgeMessage in invokeErrorCb\n");

    resultMsg->message_id = srcMsgId; // Error message corresponds to the request message with the given message id.
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

    resultMsg->type = ERROR_RESPONSE;
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

    if (resultMsg->responses[0]->nodeInfo != NULL)
    {
        resultMsg->responses[0]->nodeInfo->nodeId = NULL;
    }

EXIT:
    // Deallocate memory.
    freeEdgeMessage(resultMsg);
}

static void invokeResponseCb(EdgeMessage *msg, int msgId, EdgeNodeId *srcNodeId,
        EdgeBrowseResult *browseResult, size_t size, const unsigned char *browsePath, char *valueAlias)
{
    VERIFY_NON_NULL_NR_MSG(browseResult, "EdgeBrowseResult Param is NULL\n");
    VERIFY_NON_NULL_NR_MSG(browseResult->browseName, "EdgeBrowseResult.BrowseName is NULL\n");

    EdgeMessage *resultMsg = (EdgeMessage *) EdgeCalloc(1, sizeof(EdgeMessage));
    VERIFY_NON_NULL_NR_MSG(resultMsg, "EdgeCalloc Failed for EdgeMessage in invokeResponseCb\n");

    resultMsg->type = BROWSE_RESPONSE;
    resultMsg->message_id = msg->message_id;
    resultMsg->endpointInfo = cloneEdgeEndpointInfo(msg->endpointInfo);
    if (IS_NULL(resultMsg->endpointInfo))
    {
        EDGE_LOG(TAG, "Failed to clone the EdgeEndpointInfo.");
        goto BROWSE_ERROR;
    }

    resultMsg->responses = (EdgeResponse **) EdgeCalloc (1, sizeof(EdgeResponse *));
    if (IS_NULL(resultMsg->responses))
    {
        EDGE_LOG(TAG, "Memory allocation failed.");
        goto BROWSE_ERROR;
    }
    resultMsg->responseLength = 1;
    resultMsg->responses[0] = (EdgeResponse *) EdgeCalloc(1, sizeof(EdgeResponse));
    if (IS_NULL(resultMsg->responses[0]))
    {
        EDGE_LOG(TAG, "Memory allocation failed.");
        goto BROWSE_ERROR;
    }

    if(IS_NOT_NULL(browsePath))
    {
        resultMsg->responses[0]->message = (EdgeVersatility *) EdgeCalloc(1, sizeof(EdgeVersatility));
        if (IS_NULL(resultMsg->responses[0]->message))
        {
            EDGE_LOG(TAG, "Memory allocation failed.");
            goto BROWSE_ERROR;
        }
        resultMsg->responses[0]->message->isArray = false;
        resultMsg->responses[0]->message->value = (unsigned char *)cloneData(browsePath, strlen((char *)browsePath)+1);
        if(IS_NULL(resultMsg->responses[0]->message->value))
        {
            EDGE_LOG(TAG, "Memory allocation failed.");
            goto BROWSE_ERROR;
        }
    }

    resultMsg->responses[0]->nodeInfo = (EdgeNodeInfo *) EdgeCalloc(1, sizeof(EdgeNodeInfo));
    if (IS_NULL(resultMsg->responses[0]->nodeInfo))
    {
        EDGE_LOG(TAG, "Memory allocation failed.");
        goto BROWSE_ERROR;
    }
    resultMsg->responses[0]->nodeInfo->nodeId = cloneEdgeNodeId(srcNodeId);               //srcNodeId;
    resultMsg->responses[0]->nodeInfo->valueAlias = (char *)cloneData(valueAlias, strlen((char *)valueAlias)+1); //valueAlias;
    resultMsg->responses[0]->requestId = msgId; // Response for msgId'th request.
    resultMsg->browseResult = (EdgeBrowseResult *) EdgeCalloc(1, sizeof(EdgeBrowseResult));               //browseResult;
    if (IS_NULL(resultMsg->browseResult))
    {
        EDGE_LOG(TAG, "Memory allocation failed.");
        goto BROWSE_ERROR;
    }
    resultMsg->browseResult->browseName = cloneString(browseResult->browseName);
    if(IS_NULL(resultMsg->browseResult->browseName))
    {
        EDGE_LOG(TAG, "Memory allocation failed.");
        goto BROWSE_ERROR;
    }

    resultMsg->browseResultLength = size;

    add_to_recvQ(resultMsg);
    return;

BROWSE_ERROR:
    // Deallocate memory.
    freeEdgeMessage(resultMsg);
}

static bool checkContinuationPoint(uint32_t msgId, UA_BrowseResult *browseResult,
        EdgeNodeId *srcNodeId)
{
    VERIFY_NON_NULL_MSG(browseResult, "browseResult param is NULL", false);
    VERIFY_NON_NULL_MSG(srcNodeId, "srcNodeId param is NULL", false);

    bool retVal = true;
    /*if(browseResult.continuationPoint.length <= 0)
     {
     EDGE_LOG(TAG, "Error: " CONTINUATIONPOINT_EMPTY);
     invokeErrorCb(srcNodeId, STATUS_ERROR, CONTINUATIONPOINT_EMPTY);
     retVal = false;
     }
     else*/if (browseResult->continuationPoint.length >= 1000)
    {
        EDGE_LOG(TAG, "Error: " CONTINUATIONPOINT_LONG);
        invokeErrorCb(msgId, srcNodeId, STATUS_ERROR, CONTINUATIONPOINT_LONG);
        retVal = false;
    }
    else if (browseResult->continuationPoint.length > 0
            && (browseResult->referencesSize <= 0 || !browseResult->references))
    {
        EDGE_LOG(TAG, "Error: " STATUS_VIEW_REFERENCE_DATA_INVALID_VALUE);
        invokeErrorCb(msgId, srcNodeId, STATUS_ERROR, STATUS_VIEW_REFERENCE_DATA_INVALID_VALUE);
        retVal = false;
    }
    return retVal;
}

static bool checkBrowseName(uint32_t msgId, UA_String browseName, EdgeNodeId *srcNodeId)
{
    bool retVal = true;
    if (browseName.length <= 0 || NULL == browseName.data)
    {
        EDGE_LOG(TAG, "Error: " BROWSENAME_EMPTY);
        invokeErrorCb(msgId, srcNodeId, STATUS_ERROR, BROWSENAME_EMPTY);
        retVal = false;
    }
    else if (browseName.length >= 1000)
    {
        EDGE_LOG(TAG, "Error: " BROWSENAME_LONG);
        invokeErrorCb(msgId, srcNodeId, STATUS_ERROR, BROWSENAME_LONG);
        retVal = false;
    }
    return retVal;
}

static bool checkNodeClass(uint32_t msgId, UA_NodeClass nodeClass, EdgeNodeId *srcNodeId)
{
    bool retVal = true;
    if (false == isNodeClassValid(nodeClass))
    {
        EDGE_LOG(TAG, "Error: " NODECLASS_INVALID);
        invokeErrorCb(msgId, srcNodeId, STATUS_ERROR, NODECLASS_INVALID);
        retVal = false;
    }
    else if (UA_NODECLASS_UNSPECIFIED != BROWSE_NODECLASS_MASK &&
        (nodeClass & BROWSE_NODECLASS_MASK) == 0)
    {
        EDGE_LOG(TAG, "Error: " STATUS_VIEW_NOTINCLUDE_NODECLASS_VALUE);
        invokeErrorCb(msgId, srcNodeId, STATUS_ERROR, STATUS_VIEW_NOTINCLUDE_NODECLASS_VALUE);
        retVal = false;
    }
    return retVal;
}

static bool checkDisplayName(uint32_t msgId, UA_String displayName, EdgeNodeId *srcNodeId)
{
    bool retVal = true;
    if (displayName.length <= 0 || NULL == displayName.data)
    {
        EDGE_LOG(TAG, "Error: " DISPLAYNAME_EMPTY);
        invokeErrorCb(msgId, srcNodeId, STATUS_ERROR, DISPLAYNAME_EMPTY);
        retVal = false;
    }
    else if (displayName.length >= 1000)
    {
        EDGE_LOG(TAG, "Error: " DISPLAYNAME_LONG);
        invokeErrorCb(msgId, srcNodeId, STATUS_ERROR, DISPLAYNAME_LONG);
        retVal = false;
    }
    return retVal;
}

static bool checkNodeId(uint32_t msgId, UA_ExpandedNodeId nodeId, EdgeNodeId *srcNodeId)
{
    bool retVal = true;
    if (UA_NodeId_isNull(&nodeId.nodeId))
    {
        EDGE_LOG(TAG, "Error: " NODEID_NULL);
        invokeErrorCb(msgId, srcNodeId, STATUS_ERROR, NODEID_NULL);
        retVal = false;
    }
    else if (nodeId.serverIndex != 0)
    {
        EDGE_LOG(TAG, "Error: " NODEID_SERVERINDEX);
        invokeErrorCb(msgId, srcNodeId, STATUS_ERROR, NODEID_SERVERINDEX);
        retVal = false;
    }
    return retVal;
}

static bool checkReferenceTypeId(uint32_t msgId, UA_NodeId nodeId, EdgeNodeId *srcNodeId)
{
    bool retVal = true;
    if (UA_NodeId_isNull(&nodeId))
    {
        EDGE_LOG(TAG, "Error: " REFERENCETYPEID_NULL);
        invokeErrorCb(msgId, srcNodeId, STATUS_ERROR, REFERENCETYPEID_NULL);
        retVal = false;
    }
    return retVal;
}

static bool checkTypeDefinition(uint32_t msgId, UA_ReferenceDescription *ref, EdgeNodeId *srcNodeId)
{
    bool retVal = true;
    if ((ref->nodeClass == UA_NODECLASS_OBJECT || ref->nodeClass == UA_NODECLASS_VARIABLE)
            && UA_NodeId_isNull(&ref->typeDefinition.nodeId))
    {
        EDGE_LOG(TAG, "Error: " TYPEDEFINITIONNODEID_NULL);
        invokeErrorCb(msgId, srcNodeId, STATUS_ERROR, TYPEDEFINITIONNODEID_NULL);
        retVal = false;
    }
    return retVal;
}

static char *getValueAlias(char *browseName, UA_NodeId* nodeId, UA_LocalizedText description)
{
    char *nodeInfo = NULL;
    const int bufferSize = 20;
    int browseNameLen = 0;
    nodeInfo = (char *)EdgeCalloc(bufferSize, sizeof(char));
    VERIFY_NON_NULL_MSG(nodeInfo, "EdgeCalloc FAILED for Node info\n", NULL);

    if(IS_NOT_NULL(browseName))
    {
        browseNameLen = strlen(browseName);
    }

    char curType = getCharacterNodeIdType(nodeId->identifierType);
    if (UA_NODEIDTYPE_STRING == nodeId->identifierType)
    {
        unsigned char *valueType = convertUAStringToUnsignedChar(&description.text);
        if(IS_NOT_NULL(valueType))
        {
            if (0 == strncmp((const char*)valueType, "v=", 2))
            {
                snprintf(nodeInfo, bufferSize*sizeof(char), "{%d;%c;%s}", nodeId->namespaceIndex, curType, valueType);
            }
            else
            {
                snprintf(nodeInfo, bufferSize*sizeof(char), "{%d;%c;v=0}", nodeId->namespaceIndex, curType);
            }
            EdgeFree(valueType);
        }
    }
    else
    {
        snprintf(nodeInfo, bufferSize*sizeof(char), "{%d;%c}", nodeId->namespaceIndex, curType);
    }

    int nodeInfoLen = strlen(nodeInfo);
    char *valueAlias = (char *)EdgeCalloc(nodeInfoLen+browseNameLen + 1, sizeof(char));
    if(IS_NULL(valueAlias))
    {
        EDGE_LOG(TAG, "Memory allocation failed.");
        EdgeFree(nodeInfo);
        return NULL;
    }
    if(nodeInfoLen > 0)
    {
        memcpy(valueAlias, nodeInfo, nodeInfoLen);
        if(browseNameLen > 0)
        {
            memcpy(valueAlias + nodeInfoLen, browseName, browseNameLen);
        }
    }
    valueAlias[nodeInfoLen + browseNameLen] = '\0';

    EdgeFree(nodeInfo);
    return valueAlias;
}

unsigned char *convertNodeIdToString(UA_NodeId *nodeId)
{
    VERIFY_NON_NULL_MSG(nodeId, "UA NODE IF parameter is NULL\n", NULL);

    unsigned char *browseName = NULL;
    if(UA_NODEIDTYPE_STRING == nodeId->identifierType)
    {
        browseName = convertUAStringToUnsignedChar(&nodeId->identifier.string);
    }
    /*else if(UA_NODEIDTYPE_NUMERIC == nodeId->identifierType)
    {
        int maxDigits = 10;
        browseName = (unsigned char *)EdgeCalloc(maxDigits+1, sizeof(unsigned char));
         VERIFY_NON_NULL_MSG(browseName, NULL);
        snprintf((char *)browseName, maxDigits, "%" PRIu32, nodeId->identifier.numeric);
    } */
    // TODO: Handle GUID and ByteString
    return browseName;
}

// Ex1: If path1 = '/Objects', path2 = 'Server', then this function returns '/Objects/Server'.
// Ex2: If path1 = '/', path2 = 'Server', then this function returns '/Server'.
// Ex3: If path1 = '', path2 = 'Server', then this function returns 'Server'.
// Ex4: If path1 = '/', path2 = '', then this function returns '/'.
static unsigned char *joinPaths(unsigned char *path1, unsigned char *path2)
{
    size_t len1 = 0, len2 = 0, totLen;
    if(IS_NOT_NULL(path1))
    {
        len1 = strlen((char *)path1);
    }
    if(IS_NOT_NULL(path2))
    {
        len2 = strlen((char *)path2);
    }

    totLen = len1 + len2 + 1;
    COND_CHECK((totLen == 1), NULL); // Both the paths are empty.

    unsigned char *result = (unsigned char *) EdgeCalloc(totLen+1, sizeof(unsigned char));
    VERIFY_NON_NULL_MSG(result, "Memory allocation failed.", NULL);

    if(len1 > 0)
    {
        strncpy((char *)result, (char *)path1, len1);
    }
    size_t offset = len1;
    if(len2 > 0)
    {
        if(len1 > 0 && result[len1-1] != '/')
        {
            result[len1] = '/';
            ++offset;
        }
        strncpy((char *)(result+offset), (char *)path2, len2);
    }
    return result;
}

static uint16_t getMaxNodesToBrowse(UA_Client *client)
{
    /* Read Maximum browse continuation points supported by the server */
    UA_UInt16 maxBrowseContinuationPoints = 1; // Server's mandatory property. Minimum value assumed.
    UA_Variant *val = UA_Variant_new();
    UA_StatusCode retval = UA_Client_readValueAttribute(client,
            UA_NODEID_NUMERIC(0, UA_NS0ID_SERVER_SERVERCAPABILITIES_MAXBROWSECONTINUATIONPOINTS), val);
    if(retval == UA_STATUSCODE_GOOD && UA_Variant_isScalar(val) &&
       val->type == &UA_TYPES[UA_TYPES_UINT16])
    {
        maxBrowseContinuationPoints = *(UA_UInt16*)val->data;
        EDGE_LOG_V(TAG, "Maximum browse continuation points supported by the server is: %u\n",
                maxBrowseContinuationPoints);
    }
    UA_Variant_delete(val);

    /* Read Maximum nodes per browse supported by the server */
    UA_UInt32 maxNodesPerBrowse = 0; // Server's optional property.
    val = UA_Variant_new();
    retval = UA_Client_readValueAttribute(client,
            UA_NODEID_NUMERIC(0, UA_NS0ID_SERVER_SERVERCAPABILITIES_OPERATIONLIMITS_MAXNODESPERBROWSE), val);
    if(retval == UA_STATUSCODE_GOOD && UA_Variant_isScalar(val) &&
       val->type == &UA_TYPES[UA_TYPES_UINT32])
    {
        maxNodesPerBrowse = *(UA_UInt32*)val->data;
        EDGE_LOG_V(TAG, "Maximum nodes per browse request supported by the server is: %u\n",
                maxNodesPerBrowse);
    }
    UA_Variant_delete(val);

    /* Choose the minimum of them */
    uint16_t minimum = maxBrowseContinuationPoints;
    if(maxNodesPerBrowse != 0 && maxNodesPerBrowse < minimum)
    {
        minimum = maxNodesPerBrowse;
    }

    return minimum;
}

static bool isQueueEmpty(u_queue_t *browseQueue)
{
    VERIFY_NON_NULL_MSG(browseQueue, "browseQueue param is NULL", true);
    return (u_queue_get_size(browseQueue)==0);
}

static bool enqueueBrowseItem(u_queue_t *browseQueue, BrowseItem *item)
{
    VERIFY_NON_NULL_MSG(browseQueue, "browseQueue param is NULL", false);
    VERIFY_NON_NULL_MSG(item, "item param is NULL", false);

    u_queue_message_t *msg = (u_queue_message_t *) EdgeMalloc(sizeof(u_queue_message_t));
    VERIFY_NON_NULL_MSG(msg, "Memory allocation failed.", false);

    msg->msg = item;
    msg->size = 0; // Size is not required.
    if(CA_STATUS_OK != u_queue_add_element(browseQueue, msg))
    {
        EdgeFree(msg);
        return false;
    }
    return true;
}

static BrowseItem *dequeueBrowseItem(u_queue_t *browseQueue)
{
    VERIFY_NON_NULL_MSG(browseQueue, "browseQueue param is NULL", NULL);

    u_queue_message_t *msg = u_queue_get_element(browseQueue);
    VERIFY_NON_NULL_MSG(msg, "Item is NULL. Queue is empty or corrupted.", NULL);

    BrowseItem *item = (BrowseItem *) msg->msg;
    EdgeFree(msg);
    return item;
}

static void destroyBrowseItemMembers(BrowseItem *item)
{
    VERIFY_NON_NULL_NR_MSG(item, "browseItem is NULL");
    EdgeFree(item->browseName);
    EdgeFree(item->browsePath);
    VERIFY_NON_NULL_NR_MSG(item->nodeId, "browseItem is NULL");
    UA_NodeId_delete(item->nodeId);
}

static void destroyBrowseItem(BrowseItem *item)
{
    VERIFY_NON_NULL_NR_MSG(item, "browseItem is NULL");
    destroyBrowseItemMembers(item);
    EdgeFree(item);
}

static void destroyBrowseItems(BrowseItem **items, uint32_t size)
{
    VERIFY_NON_NULL_NR_MSG(items, "browseItem is NULL");
    for(uint32_t i = 0; i < size; ++i)
    {
        destroyBrowseItem(items[i]);
    }
}

static void destroyViewListMembers(List *ptr)
{
    VERIFY_NON_NULL_NR_MSG(ptr, "NULL list parameter\n");

    while(ptr)
    {
        BrowseItem *item = ptr->data;
        destroyBrowseItem(item);
        ptr=ptr->link;
    }
}

void destroyViewList(List **ptr)
{
    VERIFY_NON_NULL_NR_MSG(ptr, "NULL list parameter\n");
    destroyViewListMembers(*ptr);
    deleteList(ptr);
}

static void destroyBrowseQueue(u_queue_t **browseQueue)
{
    u_queue_t *queue = *browseQueue;
    VERIFY_NON_NULL_NR_MSG(queue, "browseQueue is NULL\n");

    uint32_t size = u_queue_get_size(queue);
    for(uint32_t i = 0; i < size; ++i)
    {
        u_queue_message_t *msg = u_queue_get_element(queue);
        if(IS_NOT_NULL(msg))
        {
            destroyBrowseItem(msg->msg);
            EdgeFree(msg);
        }
    }
    EdgeFree(queue);
    *browseQueue = NULL;
}

static BrowseItem *parseBrowseNodeFromReference(UA_ReferenceDescription *reference,
        BrowseItem *srcBrowseItem)
{
    VERIFY_NON_NULL_MSG(reference, "reference param is NULL", NULL);
    VERIFY_NON_NULL_MSG(srcBrowseItem, "srcBrowseItem param is NULL", NULL);

    BrowseItem *newItem = (BrowseItem *) EdgeCalloc(1, sizeof(BrowseItem));
    VERIFY_NON_NULL_MSG(newItem, "Memory allocation failed.", NULL);

    newItem->nodeId = cloneNodeId(&reference->nodeId.nodeId);
    if(IS_NULL(newItem->nodeId))
    {
        EdgeFree(newItem);
        EDGE_LOG(TAG, "Memory allocation failed.");
        return NULL;
    }

    newItem->browseName = convertUAStringToUnsignedChar(&reference->browseName.name);
    if(IS_NOT_NULL(newItem->browseName))
    {
        newItem->browseNameLen = strlen((char *)newItem->browseName);
    }

    newItem->browsePath = joinPaths(srcBrowseItem->browsePath, newItem->browseName);
    if(IS_NULL(newItem->browsePath))
    {
        destroyBrowseItem(newItem);
        EDGE_LOG(TAG, "Failed to join browse paths.");
        return NULL;
    }

    newItem->browsePathLen = strlen((char *)newItem->browsePath);
    newItem->reqId = srcBrowseItem->reqId;
    return newItem;
}

static BrowseItem *parseBrowseNodeFromRequest(EdgeRequest *req, uint32_t reqId)
{
    VERIFY_NON_NULL_MSG(req, "req param is NULL", NULL);
    BrowseItem *newItem = (BrowseItem *) EdgeCalloc(1, sizeof(BrowseItem));
    VERIFY_NON_NULL_MSG(newItem, "Memory allocation failed.", NULL);

    newItem->nodeId = getNodeId(req);
    if(IS_NULL(newItem->nodeId))
    {
        EdgeFree(newItem);
        EDGE_LOG(TAG, "Memory allocation failed.");
        return NULL;
    }

    newItem->browseName = convertNodeIdToString(newItem->nodeId);
    if(IS_NOT_NULL(newItem->browseName))
    {
        newItem->browseNameLen = strlen((char *)newItem->browseName);
    }

    unsigned char *rootPath = (unsigned char *)"/";
    newItem->browsePath = joinPaths(rootPath, newItem->browseName);
    if(IS_NULL(newItem->browsePath))
    {
        destroyBrowseItem(newItem);
        EDGE_LOG(TAG, "Failed to join browse paths.");
        return NULL;
    }
    newItem->browsePathLen = strlen((char *)newItem->browsePath);
    newItem->reqId = reqId;
    return newItem;
}

static bool parseBrowseNodesFromRequest(EdgeMessage *msg, u_queue_t *browseQueue)
{
    VERIFY_NON_NULL_MSG(msg, "msg param is NULL", false);
    VERIFY_NON_NULL_MSG(browseQueue, "browseQueue param is NULL", false);

    if (msg->type == SEND_REQUEST)
    {
        EDGE_LOG(TAG, "Message Type: " SEND_REQUEST_DESC);
        BrowseItem *newItem = parseBrowseNodeFromRequest(msg->request, 0);
        VERIFY_NON_NULL_MSG(newItem, "Memory allocation failed.", false);
        if(!enqueueBrowseItem(browseQueue, newItem))
        {
            destroyBrowseItem(newItem);
            EDGE_LOG(TAG, "Failed to enqueue browse item.");
            return false;
        }
    }
    else
    {
        EDGE_LOG(TAG, "Message Type: " SEND_REQUESTS_DESC);
        size_t reqSize = msg->requestLength;
        for (size_t i = 0; i < reqSize; ++i)
        {
            BrowseItem *newItem = parseBrowseNodeFromRequest(msg->requests[i], i);
            VERIFY_NON_NULL_MSG(newItem, "Memory allocation failed.", false);
            if(!enqueueBrowseItem(browseQueue, newItem))
            {
                destroyBrowseItem(newItem);
                EDGE_LOG(TAG, "Failed to enqueue browse item.");
                return false;
            }
        }
    }

    return true;
}

static bool makeBrowseNextRequest(UA_Client *client, EdgeMessage *msg,
        EdgeNodeId *srcNodeId, UA_ByteString *continuationPoint, UA_BrowseNextResponse *bRes)
{
    VERIFY_NON_NULL_MSG(client, "client param is NULL", false);
    VERIFY_NON_NULL_MSG(msg, "msg param is NULL", false);
    VERIFY_NON_NULL_MSG(srcNodeId, "srcNodeId param is NULL", false);
    VERIFY_NON_NULL_MSG(continuationPoint, "continuationPoint param is NULL", false);
    VERIFY_NON_NULL_MSG(bRes, "bRes param is NULL", false);

    UA_BrowseNextRequest bReq;
    UA_BrowseNextRequest_init(&bReq);
    bReq.releaseContinuationPoints = false;
    bReq.continuationPointsSize = 1;
    bReq.continuationPoints = continuationPoint;

    *bRes = UA_Client_Service_browseNext(client, bReq);

    // Check result. Invoke app's error callback if there is an error.
    if (bRes->responseHeader.serviceResult != UA_STATUSCODE_GOOD || bRes->resultsSize != 1)
    {
        char *versatileVal;
        EdgeStatusCode statusCode;
        if (bRes->resultsSize == 0)
        {
            statusCode = STATUS_VIEW_BROWSERESULT_EMPTY;
            versatileVal = STATUS_VIEW_BROWSERESULT_EMPTY_VALUE;
            EDGE_LOG(TAG, "Error: Empty BrowseNext response!!!");
        }
        else
        {
            statusCode = STATUS_SERVICE_RESULT_BAD;
            versatileVal = STATUS_SERVICE_RESULT_BAD_VALUE;
            EDGE_LOG_V(TAG, "Error in BrowseNext :: 0x%08x(%s)\n", bRes->responseHeader.serviceResult,
                    UA_StatusCode_name(bRes->responseHeader.serviceResult));
        }

        invokeErrorCb(msg->message_id, srcNodeId, statusCode, versatileVal);
        UA_BrowseNextResponse_deleteMembers(bRes);
        return false;
    }

    UA_StatusCode status = bRes->results[0].statusCode;
    if (UA_STATUSCODE_GOOD != status)
    {
        const char *statusStr = UA_StatusCode_name(status);
        EDGE_LOG_V(TAG, "Error in BrowseNext :: 0x%08x(%s)\n", status, UA_StatusCode_name(status));
        invokeErrorCb(msg->message_id, srcNodeId, STATUS_VIEW_RESULT_STATUS_CODE_BAD, statusStr);
        UA_BrowseNextResponse_deleteMembers(bRes);
        return false;
    }

    // References should not be empty if statuscode is good.
    if (!bRes->results[0].referencesSize)
    {
        EDGE_LOG(TAG, "Error: " STATUS_VIEW_REFERENCE_DATA_INVALID_VALUE);
        invokeErrorCb(msg->message_id, srcNodeId, STATUS_ERROR, STATUS_VIEW_REFERENCE_DATA_INVALID_VALUE);
        UA_BrowseNextResponse_deleteMembers(bRes);
        return false;
    }

    return true;
}

static bool makeBrowseRequest(UA_Client *client, BrowseItem **currentBrowseItems,
        uint32_t count, EdgeMessage *msg, UA_BrowseResponse *bRes)
{
    VERIFY_NON_NULL_MSG(client, "client param is NULL", false);
    VERIFY_NON_NULL_MSG(currentBrowseItems, "currentBrowseItems param is NULL", false);
    VERIFY_NON_NULL_MSG(msg, "msg param is NULL", false);
    VERIFY_NON_NULL_MSG(bRes, "bRes param is NULL", false);

    // Form browse request.
    int maxReferencesPerNode = 0;
    UA_BrowseDirection directionParam = UA_BROWSEDIRECTION_FORWARD;
    if(IS_NOT_NULL(msg->browseParam))
    {
        int direct = msg->browseParam->direction;
        if (DIRECTION_INVERSE == direct)
        {
            directionParam = UA_BROWSEDIRECTION_INVERSE;
        }
        else if (DIRECTION_BOTH == direct)
        {
            directionParam = UA_BROWSEDIRECTION_BOTH;
        }

        maxReferencesPerNode = msg->browseParam->maxReferencesPerNode;
    }

    UA_BrowseDescription *nodesToBrowse = (UA_BrowseDescription *) UA_calloc(count,
            sizeof(UA_BrowseDescription));
    VERIFY_NON_NULL_MSG(nodesToBrowse, "Failed to allocate memory for browse description.", false);

    UA_UInt32 nodeClassMask = (msg->command != CMD_BROWSE_VIEW) ?
            BROWSE_NODECLASS_MASK : VIEW_NODECLASS_MASK;

    UA_BrowseRequest bReq;
    UA_BrowseRequest_init(&bReq);
    bReq.requestedMaxReferencesPerNode = maxReferencesPerNode;
    bReq.nodesToBrowseSize = count;
    bReq.nodesToBrowse = nodesToBrowse;
    for(uint32_t idx = 0; idx < count; ++idx)
    {
        BrowseItem *item = currentBrowseItems[idx];
        nodesToBrowse[idx].nodeId = *(item->nodeId);
        nodesToBrowse[idx].browseDirection = directionParam;
        nodesToBrowse[idx].referenceTypeId = UA_NODEID_NUMERIC(SYSTEM_NAMESPACE_INDEX,
                UA_NS0ID_REFERENCES);
        nodesToBrowse[idx].includeSubtypes = true;
        nodesToBrowse[idx].nodeClassMask = nodeClassMask;
        nodesToBrowse[idx].resultMask = UA_BROWSERESULTMASK_ALL;
    }

    // Call browse.
    *bRes = UA_Client_Service_browse(client, bReq);

    // Check result. Invoke app's error callback if there is an error.
    if (bRes->responseHeader.serviceResult != UA_STATUSCODE_GOOD || bRes->resultsSize != count)
    {
        char *versatileVal;
        EdgeStatusCode statusCode;
        if (bRes->resultsSize == 0)
        {
            statusCode = STATUS_VIEW_BROWSERESULT_EMPTY;
            versatileVal = STATUS_VIEW_BROWSERESULT_EMPTY_VALUE;
            EDGE_LOG(TAG, "Error: Empty browse response!!!");
        }
        else
        {
            statusCode = STATUS_SERVICE_RESULT_BAD;
            versatileVal = STATUS_SERVICE_RESULT_BAD_VALUE;
            EDGE_LOG_V(TAG, "Error in browse :: 0x%08x(%s)\n", bRes->responseHeader.serviceResult,
                    UA_StatusCode_name(bRes->responseHeader.serviceResult));
        }

        EdgeNodeId *nodeId = (count == 1) ? getEdgeNodeId(currentBrowseItems[0]->nodeId) : NULL;
        invokeErrorCb(msg->message_id, nodeId, statusCode, versatileVal);
        freeEdgeNodeId(nodeId);
        UA_BrowseResponse_deleteMembers(bRes);
        EdgeFree(nodesToBrowse);
        return false;
    }

    EdgeFree(nodesToBrowse);
    return true;
}

static uint32_t dequeueItems(u_queue_t *browseQueue, uint32_t count,
        BrowseItem **currentBrowseItems)
{
    VERIFY_NON_NULL_MSG(browseQueue, "browseQueue param is NULL", 0);
    VERIFY_NON_NULL_MSG(currentBrowseItems, "currentBrowseItems param is NULL", 0);

    for(uint32_t idx = 0; idx < count; ++idx)
    {
        BrowseItem *item = dequeueBrowseItem(browseQueue);
        VERIFY_NON_NULL_MSG(item, "Failed to dequeue a browse item.", idx);
        currentBrowseItems[idx] = item;
    }

    return count;
}

bool hasCycle(unsigned char *srcBrowsePath, UA_String *browseName)
{
    VERIFY_NON_NULL_MSG(srcBrowsePath, "srcBrowsePath param is NULL", false);
    VERIFY_NON_NULL_MSG(browseName, "browseName param is NULL", false);

    size_t tokenLen;
    unsigned char *token = (unsigned char *)strtok((char *)srcBrowsePath, BROWSE_PATH_SEPERATOR);
    while(IS_NOT_NULL(token))
    {
        tokenLen = strlen((char *)token);
        COND_CHECK((tokenLen == browseName->length &&
                    !memcmp(token, browseName->data, browseName->length)), true);
        token = (unsigned char *)strtok(NULL, BROWSE_PATH_SEPERATOR);
    }

    return false;
}

static bool validateReference(UA_ReferenceDescription *reference,
        EdgeMessage *msg, BrowseItem *srcBrowseItem, EdgeNodeId *srcNodeId)
{
    VERIFY_NON_NULL_MSG(reference, "reference param is NULL", false);
    VERIFY_NON_NULL_MSG(msg, "msg param is NULL", false);
    VERIFY_NON_NULL_MSG(srcBrowseItem, "srcBrowseItem param is NULL", false);
    VERIFY_NON_NULL_MSG(srcNodeId, "srcNodeId param is NULL", false);

    bool valid = true;

    int direction = DIRECTION_FORWARD;
    if(IS_NOT_NULL(msg->browseParam))
    {
        direction = msg->browseParam->direction;
    }

    if ((direction == DIRECTION_FORWARD && reference->isForward == false)
            || (direction == DIRECTION_INVERSE && reference->isForward == true))
    {
        EDGE_LOG(TAG, "Error: " STATUS_VIEW_DIRECTION_NOT_MATCH_VALUE);
        invokeErrorCb(msg->message_id, srcNodeId, STATUS_VIEW_DIRECTION_NOT_MATCH,
                STATUS_VIEW_DIRECTION_NOT_MATCH_VALUE);
        valid = false;
    }

    if (!checkBrowseName(msg->message_id, reference->browseName.name, srcNodeId))
        valid = false;
    if (!checkNodeClass(msg->message_id, reference->nodeClass, srcNodeId))
        valid = false;
    if (!checkDisplayName(msg->message_id, reference->displayName.text, srcNodeId))
        valid = false;
    if (!checkNodeId(msg->message_id, reference->nodeId, srcNodeId))
        valid = false;
    if (!checkReferenceTypeId(msg->message_id, reference->referenceTypeId, srcNodeId))
        valid = false;
    if (!checkTypeDefinition(msg->message_id, reference, srcNodeId))
        valid = false;

    // Checking whether the browse name of the reference is there or not in the browse path.
    unsigned char *pathCopy = cloneData(srcBrowseItem->browsePath,
            strlen((char *)srcBrowseItem->browsePath)+1);
    if(hasCycle(pathCopy, &reference->browseName.name))
    {
        EDGE_LOG(TAG, "Found this node in the current browse path. Ignoring this node to avoid cycle.");
        valid = false;
    }

    // Log the NodeId for debugging purpose.
    logNodeId(reference->nodeId.nodeId);

    EdgeFree(pathCopy);
    return valid;
}

static bool passReferenceToApp(UA_ReferenceDescription *reference,
        EdgeMessage *msg, BrowseItem *srcBrowseItem, EdgeNodeId *srcNodeId)
{
    VERIFY_NON_NULL_MSG(reference, "reference param is NULL", false);
    VERIFY_NON_NULL_MSG(msg, "msg param is NULL", false);
    VERIFY_NON_NULL_MSG(srcBrowseItem, "srcBrowseItem param is NULL", false);
    VERIFY_NON_NULL_MSG(srcNodeId, "srcNodeId param is NULL", false);

    size_t size = 1;
    EdgeBrowseResult *browseResult = (EdgeBrowseResult *) EdgeCalloc(size,
            sizeof(EdgeBrowseResult));
    VERIFY_NON_NULL_MSG(browseResult, "Memory allocation failed.", false);

    if (UA_NODEIDTYPE_STRING == reference->nodeId.nodeId.identifierType)
    {
        browseResult->browseName = convertUAStringToString(&reference->nodeId.nodeId.identifier.string);
    }
    else
    {
        browseResult->browseName = convertUAStringToString(&reference->browseName.name);
    }

    if (IS_NULL(browseResult->browseName))
    {
        EDGE_LOG(TAG, "Failed to make browse name from reference's nodeId/browseName.");
        EdgeFree(browseResult);
        return false;
    }

    // EdgeVersatility in EdgeResponse will have the complete path to browse name (Including the browse name).
    unsigned char *completePath = NULL;
    char *valueAlias = NULL;
    if((!SHOW_SPECIFIC_NODECLASS) || (reference->nodeClass & SHOW_SPECIFIC_NODECLASS_MASK)){
        valueAlias = getValueAlias(browseResult->browseName,
                &(reference->nodeId.nodeId), reference->displayName);
        completePath = joinPaths(srcBrowseItem->browsePath, (unsigned char *)valueAlias);
    }

    invokeResponseCb(msg, srcBrowseItem->reqId, srcNodeId, browseResult, size, completePath, valueAlias);
    EdgeFree(completePath);
    EdgeFree(valueAlias);
    EdgeFree(browseResult->browseName);
    EdgeFree(browseResult);
    return true;
}

static bool handleBrowseResult(UA_Client *client, EdgeMessage *msg,
        u_queue_t *browseQueue, BrowseItem *srcBrowseItem,
        EdgeNodeId *srcNodeId, UA_BrowseResult *browseResult,
        List **viewList)
{
    VERIFY_NON_NULL_MSG(client, "client param is NULL", false);
    VERIFY_NON_NULL_MSG(msg, "msg param is NULL", false);
    VERIFY_NON_NULL_MSG(browseQueue, "browseQueue param is NULL", false);
    VERIFY_NON_NULL_MSG(srcBrowseItem, "srcBrowseItem param is NULL", false);
    VERIFY_NON_NULL_MSG(srcNodeId, "srcNodeId param is NULL", false);
    VERIFY_NON_NULL_MSG(browseResult, "browseResult param is NULL", false);

    // If it's a BrowseView request, then view list should be a valid pointer to a list.
    if(CMD_BROWSE_VIEW == msg->command)
    {
        VERIFY_NON_NULL_MSG(viewList, "viewList param is NULL", false);
    }

    // Returning true to avoid processing of other browse results.
    COND_CHECK ((!checkContinuationPoint(msg->message_id, browseResult, srcNodeId)), true);

    // Handle references.
    size_t referencesSize = browseResult->referencesSize;
    for (size_t idx = 0; idx < referencesSize; ++idx)
    {
        UA_ReferenceDescription *reference = &browseResult->references[idx];
        // Verify this reference, check for loop, trigger error callback if it's invalid.
        // Returns true if it's valid.
        if(!validateReference(reference, msg, srcBrowseItem, srcNodeId))
        {
            EDGE_LOG_V(TAG, "Reference(%zu) is invalid.\n", idx);
            continue;
        }

        // Pass the complete browse path of this reference to application.
        // Only for browse requests.
        if(msg->command != CMD_BROWSE_VIEW &&
            !passReferenceToApp(reference, msg, srcBrowseItem, srcNodeId))
        {
            EDGE_LOG_V(TAG, "Failed to pass reference(%zu) to application.\n", idx);
            return false;
        }

        // If the reference is not a variable type, then create a BrowseItem for it and enqueue.
        if(UA_NODECLASS_VARIABLE != reference->nodeClass)
        {
            // Create a BrowseItem.
            BrowseItem *newItem = parseBrowseNodeFromReference(reference, srcBrowseItem);
            VERIFY_NON_NULL_MSG(newItem, "Failed to parse browse item from a reference.", false);

            if(CMD_BROWSE_VIEW == msg->command &&
                UA_NODECLASS_VIEW == reference->nodeClass)
            {
                // Add this BrowseItem in the list represented by viewList parameter.
                if(!addListNode(viewList, newItem))
                {
                    destroyBrowseItem(newItem);
                    EDGE_LOG(TAG, "Failed to add browse item into viewList.");
                    return false;
                }
            }
            else
            {
                // Enqueue this BrowseItem.
                if(!enqueueBrowseItem(browseQueue, newItem))
                {
                    destroyBrowseItem(newItem);
                    EDGE_LOG(TAG, "Failed to enqueue browse item.");
                    return false;
                }
            }
        }
    }

    return true;
}

static bool browseNextNodes(UA_Client *client, EdgeMessage *msg,
        u_queue_t *browseQueue, BrowseItem *srcBrowseItem,
        EdgeNodeId *srcNodeId, UA_ByteString *continuationPoint,
        List **viewList)
{
    VERIFY_NON_NULL_MSG(client, "client param is NULL", false);
    VERIFY_NON_NULL_MSG(msg, "msg param is NULL", false);
    VERIFY_NON_NULL_MSG(browseQueue, "browseQueue param is NULL", false);
    VERIFY_NON_NULL_MSG(srcBrowseItem, "srcBrowseItem param is NULL", false);
    VERIFY_NON_NULL_MSG(srcNodeId, "srcNodeId param is NULL", false);
    VERIFY_NON_NULL_MSG(continuationPoint, "continuationPoint param is NULL", false);

    // If it's a BrowseView request, then view list should be a valid pointer to a list.
    if(CMD_BROWSE_VIEW == msg->command)
    {
        VERIFY_NON_NULL_MSG(viewList, "viewList param is NULL", false);
    }

    bool done = false, deleteCP = false;
    UA_ByteString cp = *continuationPoint;
    do
    {
        // Form BrowseNext request and perform BrowseNext and verify the result status.
        UA_BrowseNextResponse bRes;
        if(!makeBrowseNextRequest(client, msg, srcNodeId, &cp, &bRes))
        {
            EDGE_LOG(TAG, "Failed to make a BrowseNext request.");
            invokeErrorCb(msg->message_id, srcNodeId, STATUS_ERROR, "Failed to make a browse request.");
            if(deleteCP)
                EdgeFree(cp.data);
            return false;
        }

        if(!handleBrowseResult(client, msg, browseQueue, srcBrowseItem, srcNodeId, &bRes.results[0], viewList))
        {
            EDGE_LOG(TAG, "Failed to handle the BrowseNext result.");
            UA_BrowseNextResponse_deleteMembers(&bRes);
            invokeErrorCb(msg->message_id, srcNodeId, STATUS_ERROR, "Failed to handle the BrowseNext result.");
            if(deleteCP)
                EdgeFree(cp.data);
            return false;
        }

        if(deleteCP)
            EdgeFree(cp.data);

        // If there is a continuation point in this browse result, call BrowseNext again.
        if(bRes.results[0].continuationPoint.length > 0)
        {
            // Copy the continuation point.
            cp.length = bRes.results[0].continuationPoint.length;
            cp.data = (UA_Byte *) convertUAStringToUnsignedChar(&bRes.results[0].continuationPoint);
            if(IS_NULL(cp.data))
            {
                EDGE_LOG(TAG, "Failed to convert the continuation point.");
                UA_BrowseNextResponse_deleteMembers(&bRes);
                invokeErrorCb(msg->message_id, srcNodeId, STATUS_ERROR, "Failed to convert the continuation point.");
                return false;
            }
            deleteCP = true;
        }
        else
        {
            done = true;
        }
        UA_BrowseNextResponse_deleteMembers(&bRes);
    } while(!done);

    return true;
}

static bool browseNodesHelper(UA_Client *client, EdgeMessage *msg, u_queue_t *browseQueue,
        uint16_t maxNodesToBrowse, BrowseItem **currentBrowseItems, List **viewList)
{
    VERIFY_NON_NULL_MSG(client, "client param is NULL", false);
    VERIFY_NON_NULL_MSG(msg, "msg param is NULL", false);
    VERIFY_NON_NULL_MSG(browseQueue, "browseQueue param is NULL", false);
    VERIFY_NON_NULL_MSG(currentBrowseItems, "currentBrowseItems param is NULL", false);

    // If it's a BrowseView request, then view list should be a valid pointer to a list.
    if(CMD_BROWSE_VIEW == msg->command)
    {
        VERIFY_NON_NULL_MSG(viewList, "viewList param is NULL", false);
    }

    while(!isQueueEmpty(browseQueue))
    {
         // Adjust the maximum nodes to browse based on the queue size.
        uint32_t count = u_queue_get_size(browseQueue);
        if(count > maxNodesToBrowse)
        {
            count = maxNodesToBrowse;
        }

        // Dequeue 'count' items and put them in currentBrowseItems for further processing.
        // currentBrowseItems needs to be destroyed before re-filling it.
        uint32_t dequeueCount = dequeueItems(browseQueue, count, currentBrowseItems);
        if(dequeueCount != count)
        {
            EDGE_LOG(TAG, "Failed to dequeue required number of browse items.");
            destroyBrowseItems(currentBrowseItems, dequeueCount);
            invokeErrorCb(msg->message_id, NULL, STATUS_ERROR, "Failed to dequeue required number of browse items.");
            return false;
        }

        // Form browse request and perform browse and verify the result status.
        UA_BrowseResponse bRes;
        if(!makeBrowseRequest(client, currentBrowseItems, count, msg, &bRes))
        {
            EDGE_LOG(TAG, "Failed to make a browse request.");
            destroyBrowseItems(currentBrowseItems, count);
            return false;
        }

        uint32_t nodeIdUnknownCount = 0;
        EdgeNodeId *srcNodeId = NULL;
        // Iterate over all the results in the response. Number of results will be same as 'count'.
        for(uint32_t res_idx = 0; res_idx < count; ++res_idx)
        {
            // Get the EdgeNodeId of the corresponding request. This is required to be sent in application callbacks.
            srcNodeId = getEdgeNodeId(currentBrowseItems[res_idx]->nodeId);
            if(IS_NULL(srcNodeId))
            {
                EDGE_LOG(TAG, "Failed to get the edge node id.");
                UA_BrowseResponse_deleteMembers(&bRes);
                destroyBrowseItems(currentBrowseItems, count);
                invokeErrorCb(msg->message_id, NULL, STATUS_ERROR, "Failed to get the edge node id.");
                return false;
            }

            // Check the status code of result and keep track of unknown NodeId cases.
            UA_StatusCode status = bRes.results[res_idx].statusCode;
            if (UA_STATUSCODE_GOOD != status)
            {
                if (UA_STATUSCODE_BADNODEIDUNKNOWN == status)
                    nodeIdUnknownCount++;

                if (nodeIdUnknownCount == bRes.resultsSize)
                {
                    EDGE_LOG(TAG, "Error: " STATUS_VIEW_NODEID_UNKNOWN_ALL_RESULTS_VALUE);
                    invokeErrorCb(msg->message_id, srcNodeId, STATUS_VIEW_NODEID_UNKNOWN_ALL_RESULTS,
                            STATUS_VIEW_NODEID_UNKNOWN_ALL_RESULTS_VALUE);
                }
                else
                {
                    const char *statusStr = UA_StatusCode_name(status);
                    invokeErrorCb(msg->message_id, srcNodeId, STATUS_VIEW_RESULT_STATUS_CODE_BAD, statusStr);
                }
                freeEdgeNodeId(srcNodeId);
                continue;
            }

            // Process all the references in this browse result. Validate them. Detect & avoid cycle in browse path.
            // Create BrowseItem for each reference, Pass it to application and Enqueue.
            if(!handleBrowseResult(client, msg, browseQueue, currentBrowseItems[res_idx], srcNodeId,
                    &bRes.results[res_idx], viewList))
            {
                EDGE_LOG(TAG, "Failed to handle the browse result.");
                freeEdgeNodeId(srcNodeId);
                UA_BrowseResponse_deleteMembers(&bRes);
                destroyBrowseItems(currentBrowseItems, count);
                invokeErrorCb(msg->message_id, srcNodeId, STATUS_ERROR, "Failed to handle the browse result.");
                return false;
            }

            // Handle continution point.
            // If there is a continuation point in this browse result, call BrowseNext.
            if(bRes.results[res_idx].continuationPoint.length > 0)
            {
                // Perform BrowseNext to get next set of pending references.
                // Only one continuation point will be passed per BrowseNext call. So there will be only one result per call.
                // Handle the BrowseNext result - Iterate over all the references, validate them,
                // pass them to app, enqueue them. If the result still has continuation point, call BrowseNext and perform
                // the same operations. Continue till there is no continuation point.
                if(!browseNextNodes(client, msg, browseQueue, currentBrowseItems[res_idx],
                        srcNodeId, &(bRes.results[res_idx].continuationPoint), viewList))
                {
                    EDGE_LOG(TAG, "Failed to perform BrowseNext.\n");
                    freeEdgeNodeId(srcNodeId);
                    UA_BrowseResponse_deleteMembers(&bRes);
                    destroyBrowseItems(currentBrowseItems, count);
                    return false;
                }
            }

            // Destroy items which are created in this loop.
            freeEdgeNodeId(srcNodeId);
        }

        // Destroy items which are created in this loop.
        destroyBrowseItems(currentBrowseItems, count);
        UA_BrowseResponse_deleteMembers(&bRes);
    }
    return true;
}

void browseNodes(UA_Client *client, EdgeMessage *msg)
{
    // Initialize a queue.
    u_queue_t *browseQueue = u_queue_create();
    if (IS_NULL(browseQueue))
    {
        EDGE_LOG(TAG, "Failed to initialize queue.");
        invokeErrorCb(msg->message_id, NULL, STATUS_ERROR, "Failed to initialize queue.");
        return;
    }

    // Iterate over the given requests, create a BrowseItem for each one of them and enqueue.
    if(!parseBrowseNodesFromRequest(msg, browseQueue))
    {
        EDGE_LOG(TAG, "Failed to parse the browse request.");
        destroyBrowseQueue(&browseQueue);
        invokeErrorCb(msg->message_id, NULL, STATUS_ERROR, "Failed to parse the browse request.");
        return;
    }

    // Read server's capability and find out the maximum nodes
    // which can be browsed through a single browse request.
    uint16_t maxNodesToBrowse = getMaxNodesToBrowse(client);

    // List to hold all view nodes (BrowseItem). Only for CMD_BROWSE_VIEW requests.
    List *viewList = NULL;

    // Form an array to hold the nodes to browse.
    // When browse items are dequeued, they will be kept in this array for further processing.
    BrowseItem **currentBrowseItems = (BrowseItem **) EdgeMalloc(maxNodesToBrowse * sizeof(BrowseItem *));
    if(IS_NULL(currentBrowseItems))
    {
        EDGE_LOG(TAG, "Failed to allocate memory for browse request.");
        destroyBrowseQueue(&browseQueue);
        invokeErrorCb(msg->message_id, NULL, STATUS_ERROR, "Failed to allocate memory for browse request.");
        return;
    }

    if(!browseNodesHelper(client, msg, browseQueue, maxNodesToBrowse, currentBrowseItems, &viewList))
    {
        destroyViewList(&viewList);
        EdgeFree(currentBrowseItems);
        destroyBrowseQueue(&browseQueue);
        return;
    }

    if (CMD_BROWSE_VIEW == msg->command)
    {
        // Iterate over the viewList and enqueue all BrowseItems.
        List *viewPtr = viewList;
        while(viewPtr)
        {
            BrowseItem *item = viewPtr->data;
            if(!enqueueBrowseItem(browseQueue, item))
            {
                EDGE_LOG(TAG, "Failed to enqueue a BrowseItem.");
                destroyViewList(&viewList);
                EdgeFree(currentBrowseItems);
                destroyBrowseQueue(&browseQueue);
                invokeErrorCb(msg->message_id, NULL, STATUS_ERROR, "Failed to enqueue a BrowseItem.");
                return;
            }
            viewPtr->data = NULL;
            viewPtr = viewPtr->link;
        }

        // Reuse the same EdgeMessage param for browsing all nodes in the views newly stored in the queue.
        msg->command = CMD_BROWSE;

        // Start processing the view nodes. Perform general browse for all the nodes.
        if(!browseNodesHelper(client, msg, browseQueue, maxNodesToBrowse, currentBrowseItems, &viewList))
        {
            destroyViewList(&viewList);
            EdgeFree(currentBrowseItems);
            destroyBrowseQueue(&browseQueue);
            return;
        }
    }

    // Destroy items which are created in this function.
    destroyViewList(&viewList);
    EdgeFree(currentBrowseItems);
    destroyBrowseQueue(&browseQueue);
}
