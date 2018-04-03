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

#include "edge_open62541.h"
#include "edge_logger.h"
#include "edge_malloc.h"

#define TAG "edge_open62541"

char *convertUAStringToString(UA_String *uaStr)
{
    if (!uaStr || uaStr->length <= 0)
    {
        return NULL;
    }

    char *str = (char *) EdgeMalloc(uaStr->length + 1);
    VERIFY_NON_NULL_MSG(str, "EdgeMalloc FAILED for conver UA string to string\n", NULL);
    memcpy(str, uaStr->data, uaStr->length);
    str[uaStr->length] = '\0';
    return str;
}

Edge_String *convertToEdgeString(UA_String *uaStr)
{
    VERIFY_NON_NULL_MSG(uaStr, "UA String param is NULL in convertToEdgeString\n", NULL);
    Edge_String *value = (Edge_String *) EdgeCalloc(1, sizeof(Edge_String));
    VERIFY_NON_NULL_MSG(value, "Memory allocation failed", NULL);
    value->length = uaStr->length;
    value->data = (uint8_t*) EdgeCalloc(value->length+1, sizeof(uint8_t));
    if(IS_NULL(value->data))
    {
        EDGE_LOG(TAG, "Memory allocation failed.");
        EdgeFree(value);
        return NULL;
    }
    memcpy(value->data, uaStr->data, value->length);
    return value;
}

EdgeApplicationType convertToEdgeApplicationType(UA_ApplicationType appType)
{
    // Setting SERVER as default application type.
    EdgeApplicationType edgeAppType = EDGE_APPLICATIONTYPE_SERVER;
    switch(appType)
    {
        case UA_APPLICATIONTYPE_SERVER:
            edgeAppType = EDGE_APPLICATIONTYPE_SERVER;
            break;
        case UA_APPLICATIONTYPE_CLIENT:
            edgeAppType = EDGE_APPLICATIONTYPE_CLIENT;
            break;
        case UA_APPLICATIONTYPE_CLIENTANDSERVER:
            edgeAppType = EDGE_APPLICATIONTYPE_CLIENTANDSERVER;
            break;
        case UA_APPLICATIONTYPE_DISCOVERYSERVER:
            edgeAppType = EDGE_APPLICATIONTYPE_DISCOVERYSERVER;
            break;
        default:
            // Default case is empty because SERVER is set as default application type
            // before switch block.
            break;
    }
    return edgeAppType;
}

UA_ApplicationType convertEdgeApplicationType(EdgeApplicationType appType)
{
    // Setting SERVER as default application type.
    UA_ApplicationType uaAppType = UA_APPLICATIONTYPE_SERVER;
    switch(appType)
    {
        case EDGE_APPLICATIONTYPE_SERVER:
            uaAppType = UA_APPLICATIONTYPE_SERVER;
            break;
        case EDGE_APPLICATIONTYPE_CLIENT:
            uaAppType = UA_APPLICATIONTYPE_CLIENT;
            break;
        case EDGE_APPLICATIONTYPE_CLIENTANDSERVER:
            uaAppType = UA_APPLICATIONTYPE_CLIENTANDSERVER;
            break;
        case EDGE_APPLICATIONTYPE_DISCOVERYSERVER:
            uaAppType = UA_APPLICATIONTYPE_DISCOVERYSERVER;
            break;
        default:
            // Default case is empty because SERVER is set as default application type
            // before switch block.
            break;
    }
    return uaAppType;
}

Edge_NodeId *convertToEdgeNodeIdType(UA_NodeId *nodeId)
{
    VERIFY_NON_NULL_MSG(nodeId, "Node ID param is NULL in convertToEdgeNodeIdType\n", NULL);
    Edge_NodeId *edgeNodeId = (Edge_NodeId *) EdgeCalloc(1, sizeof(Edge_NodeId));
    VERIFY_NON_NULL_MSG(edgeNodeId, "Memory allocation failed", NULL);

    edgeNodeId->namespaceIndex = nodeId->namespaceIndex;
    edgeNodeId->identifierType = nodeId->identifierType;
    if(nodeId->identifierType == UA_NODEIDTYPE_NUMERIC)
    {
        edgeNodeId->identifier.numeric = nodeId->identifier.numeric;
    }
    else if(nodeId->identifierType == UA_NODEIDTYPE_STRING)
    {
        Edge_String *edgeStr = convertToEdgeString(&nodeId->identifier.string);
        if(IS_NULL(edgeStr))
        {
            EDGE_LOG_V(TAG, "Failed to convert the Node Id of type (%d).", nodeId->identifierType);
            EdgeFree(edgeNodeId);
            return NULL;
        }
        edgeNodeId->identifier.string = *edgeStr;
        EdgeFree(edgeStr);
    }
    else if(nodeId->identifierType == UA_NODEIDTYPE_GUID)
    {
        edgeNodeId->identifier.guid = *((Edge_Guid *)&nodeId->identifier.guid);
    }
    else
    {
        // For UA_NODEIDTYPE_BYTESTRING
        Edge_String *edgeStr = convertToEdgeString(&nodeId->identifier.byteString);
        if(IS_NULL(edgeStr))
        {
            EDGE_LOG_V(TAG, "Failed to convert the Node Id of type (%d).", nodeId->identifierType);
            EdgeFree(edgeNodeId);
            return NULL;
        }
        edgeNodeId->identifier.byteString = *edgeStr;
        EdgeFree(edgeStr);
    }

    return edgeNodeId;
}

EdgeMethodRequestParams* cloneEdgeMethodRequestParams(EdgeMethodRequestParams *methodParams)
{
    VERIFY_NON_NULL_MSG(methodParams, "Null method params in cloneEdgeMethodRequestParams\n", NULL);
    EdgeMethodRequestParams* clone = (EdgeMethodRequestParams *) EdgeCalloc(1, sizeof(EdgeMethodRequestParams));
    VERIFY_NON_NULL_MSG(clone, "EdgeCalloc FAILED for clone in cloneEdgeMethodRequestParams\n", NULL);

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
            if (methodParams->inpArg[i]->argType == UA_NS0ID_STRING)
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
            if (methodParams->inpArg[i]->argType == UA_NS0ID_STRING)
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

void freeEdgeArgument(EdgeArgument *arg)
{
    VERIFY_NON_NULL_NR_MSG(arg, "NULL param edge argument in freeEdgeArgument\n");
    if(SCALAR == arg->valType)
    {
        EdgeFree(arg->scalarValue);
    }
    else if(ARRAY_1D == arg->valType)
    {
        if(UA_NS0ID_STRING == arg->argType)
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

void freeEdgeVersatilityByType(EdgeVersatility *versatileValue, int type)
{
    VERIFY_NON_NULL_NR_MSG(versatileValue, "NULL param versatileValue in freeEdgeVersatilityByType\n");

    if (versatileValue->isArray)
    {
        if (type == UA_NS0ID_STRING || type == UA_NS0ID_BYTESTRING
            || type == UA_NS0ID_GUID || type == UA_NS0ID_XMLELEMENT)
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
        else if (type == UA_NS0ID_QUALIFIEDNAME)
        {
            Edge_QualifiedName **values = (Edge_QualifiedName **) versatileValue->value;
            for (int j = 0; j < versatileValue->arrayLength; j++)
            {
                freeEdgeQualifiedName(values[j]);
            }
            EdgeFree(values);
        }
        else if (type == UA_NS0ID_LOCALIZEDTEXT)
        {
            Edge_LocalizedText **values = (Edge_LocalizedText **) versatileValue->value;
            for (int j = 0; j < versatileValue->arrayLength; j++)
            {
                freeEdgeLocalizedText(values[j]);
            }
            EdgeFree(values);
        }
        else if (type == UA_NS0ID_NODEID)
        {
            Edge_NodeId **values = (Edge_NodeId **) versatileValue->value;
            for (int j = 0; j < versatileValue->arrayLength; j++)
            {
                freeEdgeNodeIdType(values[j]);
            }
            EdgeFree(values);
        }
        else
        {
            EdgeFree(versatileValue->value);
        }
    }
    else
    {
        if(type == UA_NS0ID_QUALIFIEDNAME)
        {
            freeEdgeQualifiedName((Edge_QualifiedName *)versatileValue->value);
        }
        else if(type == UA_NS0ID_LOCALIZEDTEXT)
        {
            freeEdgeLocalizedText((Edge_LocalizedText *)versatileValue->value);
        }
        else if (type == UA_NS0ID_NODEID)
        {
            freeEdgeNodeIdType((Edge_NodeId *)versatileValue->value);
        }
        else
        {
            EdgeFree(versatileValue->value);
        }
    }

    EdgeFree(versatileValue);
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

size_t get_size(int type, bool isArray)
{
    size_t size = -1;
    switch (type)
    {
        case UA_NS0ID_BOOLEAN:
            {
                size = (isArray) ? sizeof(bool*) : sizeof(bool);
            }
            break;
        case UA_NS0ID_SBYTE:
            {
                size = (isArray) ? sizeof(int8_t*) : sizeof(int8_t);
            }
            break;
        case UA_NS0ID_BYTE:
            {
                size = (isArray) ? sizeof(uint8_t*) : sizeof(uint8_t);
            }
            break;
        case UA_NS0ID_INT16:
            {
                size = (isArray) ? sizeof(int16_t*) : sizeof(int16_t);
            }
            break;
        case UA_NS0ID_UINT16:
            {
                size = (isArray) ? sizeof(uint16_t*) : sizeof(uint16_t);
            }
            break;
        case UA_NS0ID_INT32:
            {
                size = (isArray) ? sizeof(int32_t*) : sizeof(int32_t);
            }
            break;
        case UA_NS0ID_UINT32:
            {
                size = (isArray) ? sizeof(uint32_t*) : sizeof(uint32_t);
            }
            break;
        case UA_NS0ID_INT64:
            {
                size = (isArray) ? sizeof(int64_t*) : sizeof(int64_t);
            }
            break;
        case UA_NS0ID_UINT64:
            {
                size = (isArray) ? sizeof(uint64_t*) : sizeof(uint64_t);
            }
            break;
        case UA_NS0ID_FLOAT:
            {
                size = (isArray) ? sizeof(float*) : sizeof(float);
            }
            break;
        case UA_NS0ID_DOUBLE:
            {
                size = (isArray) ? sizeof(double*) : sizeof(double);
            }
            break;
        case UA_NS0ID_STRING:
            {
                size = (isArray) ? sizeof(char*) : sizeof(char);
            }
            break;
        case UA_NS0ID_DATETIME:
            {
                size = (isArray) ? sizeof(UA_DateTime*) : sizeof(UA_DateTime);
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
    VERIFY_NON_NULL_MSG(msg, "NULL param EdgeMessage in cloneEdgeMessage\n", NULL);
    EdgeMessage *clone = (EdgeMessage *)EdgeCalloc(1, sizeof(EdgeMessage));
    VERIFY_NON_NULL_MSG(clone, "EdgeCalloc failed for clone in cloneEdgeMessage\n", NULL);

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
                        if (msg->requests[i]->type == UA_NS0ID_STRING || msg->requests[i]->type == UA_NS0ID_BYTESTRING)
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
                        if (msg->requests[i]->type == UA_NS0ID_STRING || msg->requests[i]->type == UA_NS0ID_BYTESTRING)
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
                            memcpy(cloneVersatility->value, val, get_size(msg->requests[i]->type, true) * srcVersatility->arrayLength);
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

char getCharacterNodeIdType(uint32_t type)
{
    char nodeType;
    char nodeTypeArray[5] = { 'N', 'S', 'B', 'G', '\0' };
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
            // Ideally, as all valid types are handled above, there is no need for a default case.
            // Defined this for handling cases which comes with illegal types.
            // This value indicates an error. Callers should handle it properly.
            nodeType = nodeTypeArray[4];
            break;
    }
    return nodeType;
}

void logNodeId(UA_NodeId id)
{
#if DEBUG
    char *str = NULL;
    switch (id.identifierType)
    {
        case UA_NODEIDTYPE_NUMERIC:
            EDGE_LOG_V(TAG, "Numeric: %d\t", id.identifier.numeric);
            break;
        case UA_NODEIDTYPE_STRING:
            str = convertUAStringToString(&id.identifier.string);
            EDGE_LOG_V(TAG, "String: %s\t", str);
            break;
        case UA_NODEIDTYPE_BYTESTRING:
            str = convertUAStringToString(&id.identifier.byteString);
            EDGE_LOG_V(TAG, "Byte String: %s\t", str);
            break;
        case UA_NODEIDTYPE_GUID:
            EDGE_LOG(TAG, "GUID\n");
            break;
    }
    EdgeFree(str);
#else
    (void) id;
#endif
}
