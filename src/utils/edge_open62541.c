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


UA_StatusCode createScalarVariant(int type, void *data, UA_Variant *out)
{
    UA_StatusCode ret = UA_STATUSCODE_GOOD;
    if (type == UA_TYPES_STRING)
    {
        UA_String val = UA_STRING_ALLOC((char * ) data);
        ret = UA_Variant_setScalarCopy(out, &val, &UA_TYPES[type]);
        UA_String_deleteMembers(&val);

        EdgeFree(val.data);
    }
    else if (type == UA_TYPES_BYTESTRING)
    {
        UA_ByteString val = UA_BYTESTRING_ALLOC((char * ) data);
        ret = UA_Variant_setScalarCopy(out, &val, &UA_TYPES[type]);
        UA_ByteString_deleteMembers(&val);

        EdgeFree(val.data);
    }
    else if (type == UA_TYPES_LOCALIZEDTEXT)
    {
        Edge_LocalizedText *lt = (Edge_LocalizedText *) data;
        UA_LocalizedText val;
        val.locale.length = lt->locale.length;
        val.locale.data = (uint8_t*)EdgeCalloc(val.locale.length, sizeof(uint8_t));
        memcpy(val.locale.data, lt->locale.data, lt->locale.length);
        val.text.length = lt->text.length;
        val.text.data = (uint8_t*)EdgeCalloc(val.text.length, sizeof(uint8_t));
        memcpy(val.text.data, lt->text.data, lt->text.length);
        ret = UA_Variant_setScalarCopy(out, &val, &UA_TYPES[type]);
        EdgeFree(val.locale.data);
        EdgeFree(val.text.data);
    }
    else if (type == UA_TYPES_QUALIFIEDNAME)
    {
        Edge_QualifiedName *eqn = (Edge_QualifiedName *) data;
        UA_QualifiedName qn;
        qn.namespaceIndex=eqn->namespaceIndex;
        qn.name=*((UA_String *)&eqn->name);
        ret = UA_Variant_setScalarCopy(out, &qn, &UA_TYPES[type]);
    }
    else if (type == UA_TYPES_NODEID)
    {
        Edge_NodeId *n1 = (Edge_NodeId *) data;
        ret = UA_Variant_setScalarCopy(out, n1, &UA_TYPES[type]);
    }
    else
    {
        ret = UA_Variant_setScalarCopy(out, data, &UA_TYPES[type]);
    }
    return ret;
}

UA_StatusCode createArrayVariant(int type, void *data, int len, UA_Variant *out)
{
    UA_StatusCode ret = UA_STATUSCODE_GOOD;
    // array
    if (type == UA_TYPES_STRING)
    {
        size_t idx = 0;
        char **data1 = (char **) data;
        UA_String *array = (UA_String *) UA_Array_new(len, &UA_TYPES[type]);
        for (idx = 0; idx < len; idx++)
        {
            array[idx] = UA_STRING_ALLOC(data1[idx]);
        }
        ret = UA_Variant_setArrayCopy(out, array, len, &UA_TYPES[type]);
        for (idx = 0; idx < len; idx++)
        {
            UA_String_deleteMembers(&array[idx]);
        }
        UA_Array_delete(array, len, &UA_TYPES[type]);
    }
    else if (type == UA_TYPES_BYTESTRING)
    {
        size_t idx = 0;
        char **data1 = (char **) data;
        UA_ByteString *array = (UA_ByteString *) UA_Array_new(len, &UA_TYPES[type]);
        for (idx = 0; idx < len; idx++)
        {
            array[idx] = UA_BYTESTRING_ALLOC(data1[idx]);
        }
        ret = UA_Variant_setArrayCopy(out, array, len, &UA_TYPES[type]);
        for (idx = 0; idx < len; idx++)
        {
            UA_ByteString_deleteMembers(&array[idx]);
        }
        UA_Array_delete(array, len, &UA_TYPES[type]);
    }
    else
    {
        ret = UA_Variant_setArrayCopy(out, data, len,
                &UA_TYPES[type]);
    }
    return ret;
}

char *convertUAStringToString(UA_String *uaStr)
{
    VERIFY_NON_NULL_MSG(uaStr, "", NULL);
    COND_CHECK((uaStr->length <= 0), NULL);

    char *str = (char *) EdgeMalloc(uaStr->length + 1);
    VERIFY_NON_NULL_MSG(str, "EdgeMalloc FAILED for convert UA string to string\n", NULL);
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
    // Setting SERVER as default application type. ****
    EdgeApplicationType edgeAppType = EDGE_APPLICATIONTYPE_SERVER;
    COND_CHECK((appType == UA_APPLICATIONTYPE_SERVER), EDGE_APPLICATIONTYPE_SERVER);
    COND_CHECK((appType == UA_APPLICATIONTYPE_CLIENT), EDGE_APPLICATIONTYPE_CLIENT);
    COND_CHECK((appType == UA_APPLICATIONTYPE_CLIENTANDSERVER), EDGE_APPLICATIONTYPE_CLIENTANDSERVER);
    COND_CHECK((appType == UA_APPLICATIONTYPE_DISCOVERYSERVER), EDGE_APPLICATIONTYPE_DISCOVERYSERVER);
    return edgeAppType;
}

UA_ApplicationType convertEdgeApplicationType(EdgeApplicationType appType)
{
    // Setting SERVER as default application type. *****
    UA_ApplicationType uaAppType = UA_APPLICATIONTYPE_SERVER;
    COND_CHECK((appType == EDGE_APPLICATIONTYPE_SERVER), UA_APPLICATIONTYPE_SERVER);
    COND_CHECK((appType == EDGE_APPLICATIONTYPE_CLIENT), UA_APPLICATIONTYPE_CLIENT);
    COND_CHECK((appType == EDGE_APPLICATIONTYPE_CLIENTANDSERVER), UA_APPLICATIONTYPE_CLIENTANDSERVER);
    COND_CHECK((appType == EDGE_APPLICATIONTYPE_DISCOVERYSERVER), UA_APPLICATIONTYPE_DISCOVERYSERVER);
    return uaAppType;
}

void convertGuidToString(UA_Guid guid, char **out)
{
    snprintf(*out, GUID_LENGTH + 1, "%08x-%04x-%04x-%02x%02x-%02x%02x%02x%02x%02x%02x",
            guid.data1, guid.data2, guid.data3, guid.data4[0], guid.data4[1], guid.data4[2],
            guid.data4[3], guid.data4[4], guid.data4[5], guid.data4[6], guid.data4[7]);
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

EdgeNodeId *getEdgeNodeId(UA_NodeId *node)
{
    VERIFY_NON_NULL_MSG(node, "NodeID Parameter is NULL\n", NULL);

    EdgeNodeId *edgeNodeId = (EdgeNodeId *) EdgeCalloc(1, sizeof(EdgeNodeId));
    VERIFY_NON_NULL_MSG(edgeNodeId, "EdgeCalloc FAILED for edge node ID in getEdgeNodeId\n", NULL);

    edgeNodeId->nameSpace = node->namespaceIndex;
    
    if(node->identifierType == UA_NODEIDTYPE_NUMERIC)
    {
        edgeNodeId->type = EDGE_INTEGER;
        edgeNodeId->integerNodeId = node->identifier.numeric;
    }

    if(node->identifierType == UA_NODEIDTYPE_STRING)
    {
        edgeNodeId->type = EDGE_STRING;
        edgeNodeId->nodeId = convertUAStringToString(&node->identifier.string);
    }

    if(node->identifierType == UA_NODEIDTYPE_BYTESTRING)
    {
        edgeNodeId->type = EDGE_BYTESTRING;
        edgeNodeId->nodeId = convertUAStringToString(&node->identifier.string);
    }

    if(node->identifierType == UA_NODEIDTYPE_GUID)
    {
        edgeNodeId->type = EDGE_UUID;
        UA_Guid guid = node->identifier.guid;
        char *value = (char *) EdgeMalloc(GUID_LENGTH + 1);
        if (IS_NULL(value))
        {
            EDGE_LOG(TAG, "Memory allocation failed.");
            EdgeFree(edgeNodeId);
            edgeNodeId = NULL;
        }
        else
        {
            convertGuidToString(guid, &value);
            edgeNodeId->nodeId = value;
        }
    }

    return edgeNodeId;
}

EdgeMethodRequestParams* cloneEdgeMethodRequestParams(EdgeMethodRequestParams *methodParams)
{
    VERIFY_NON_NULL_MSG(methodParams, "Null method params in cloneEdgeMethodRequestParams\n", NULL);
    EdgeMethodRequestParams* clone = (EdgeMethodRequestParams *) EdgeCalloc(1, sizeof(EdgeMethodRequestParams));
    VERIFY_NON_NULL_MSG(clone, "EdgeCalloc FAILED for clone in cloneEdgeMethodRequestParams\n", NULL);

    clone->num_outArgs = methodParams->num_outArgs;

    COND_CHECK((methodParams->num_inpArgs < 1), clone);

    clone->inpArg = (EdgeArgument**) EdgeCalloc(methodParams->num_inpArgs, sizeof(EdgeArgument*));
    if(IS_NULL(clone->inpArg))
    {
        EDGE_LOG(TAG, "Memory allocation failed.");
        goto CLONE_ERROR;
    }

    clone->num_inpArgs = methodParams->num_inpArgs;
    for (size_t i  = 0; i < clone->num_inpArgs; i++)
    {
        clone->inpArg[i] = (EdgeArgument*) EdgeCalloc(1, sizeof(EdgeArgument));
        if(IS_NULL(clone->inpArg[i]))
        {
            EDGE_LOG(TAG, "Memory allocation failed.");
            goto CLONE_ERROR;
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
                    goto CLONE_ERROR;
                }
            }
            else
            {
                size_t size = get_size(methodParams->inpArg[i]->argType, false);
                clone->inpArg[i]->scalarValue = (void *) EdgeCalloc(1, size);
                if(IS_NULL(clone->inpArg[i]->scalarValue))
                {
                    EDGE_LOG(TAG, "Memory allocation failed.");
                    goto CLONE_ERROR;
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
                    goto CLONE_ERROR;
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
                        goto CLONE_ERROR;
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
                    goto CLONE_ERROR;
                }
                memcpy(clone->inpArg[i]->arrayData, methodParams->inpArg[i]->arrayData,
                        get_size(methodParams->inpArg[i]->argType, false) * methodParams->inpArg[i]->arrayLength);
            }

        }
    }
    return clone;

CLONE_ERROR:
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
    static const int NODECLASS_MASK = UA_NODECLASS_UNSPECIFIED | UA_NODECLASS_OBJECT
            | UA_NODECLASS_VARIABLE | UA_NODECLASS_METHOD | UA_NODECLASS_OBJECTTYPE
            | UA_NODECLASS_VARIABLETYPE | UA_NODECLASS_REFERENCETYPE | UA_NODECLASS_DATATYPE
            | UA_NODECLASS_VIEW;
    if (nodeClass & NODECLASS_MASK)
        return true;
    return false;
}

size_t get_size(int type, bool isArray)
{
    COND_CHECK((isArray), sizeof(void*));
    size_t size[13] = {sizeof(bool), sizeof(int8_t), sizeof(uint8_t), sizeof(int16_t), sizeof(uint16_t),
                      sizeof(int32_t), sizeof(uint32_t), sizeof(int64_t), sizeof(uint64_t),
                      sizeof(float), sizeof(double), sizeof(char), sizeof(UA_DateTime)};
    return size[type-1];
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
            goto CLONE_ERROR;
        }
    }

    clone->requestLength = msg->requestLength;
    clone->message_id = msg->message_id;

    if (msg->browseParam)
    {
        clone->browseParam = (EdgeBrowseParameter *) EdgeCalloc(1, sizeof(EdgeBrowseParameter));
        if(IS_NULL(clone->browseParam))
        {
            goto CLONE_ERROR;
        }
        clone->browseParam->direction = msg->browseParam->direction;
        clone->browseParam->maxReferencesPerNode = msg->browseParam->maxReferencesPerNode;
    }

    if (msg->type == SEND_REQUEST)
    {
        if (msg->request)
        {
            clone->request = (EdgeRequest*) EdgeCalloc(1, sizeof(EdgeRequest));
            if(IS_NULL(clone->request))
            {
                goto CLONE_ERROR;
            }

            if (msg->request->nodeInfo)
            {
                clone->request->nodeInfo = cloneEdgeNodeInfo(msg->request->nodeInfo);
                if(IS_NULL(clone->request->nodeInfo))
                {
                    goto CLONE_ERROR;
                }
            }

            if (msg->request->subMsg)
            {
                clone->request->subMsg = cloneSubRequest(msg->request->subMsg);
                if(IS_NULL(clone->request->subMsg))
                {
                    goto CLONE_ERROR;
                }
            }

            if (msg->request->methodParams)
            {
                clone->request->methodParams = cloneEdgeMethodRequestParams(msg->request->methodParams);
                if(IS_NULL(clone->request->methodParams))
                {
                    goto CLONE_ERROR;
                }
            }
        }
    }

    if (msg->type == SEND_REQUESTS)
    {
        clone->requests = (EdgeRequest**) EdgeCalloc(msg->requestLength, sizeof(EdgeRequest*));
        if(IS_NULL(clone->requests))
        {
            goto CLONE_ERROR;
        }

        for (size_t i = 0; i < msg->requestLength; i++)
        {
            clone->requests[i] = (EdgeRequest*) EdgeCalloc(1, sizeof(EdgeRequest));
            if(IS_NULL(clone->requests[i]))
            {
                goto CLONE_ERROR;
            }

            if (msg->requests[i]->nodeInfo)
            {
                clone->requests[i]->nodeInfo = cloneEdgeNodeInfo(msg->requests[i]->nodeInfo);
                if(IS_NULL(clone->requests[i]->nodeInfo))
                {
                    goto CLONE_ERROR;
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
                        goto CLONE_ERROR;
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
                                goto CLONE_ERROR;
                            }

                            strncpy(cloneVersatility->value, (char*) srcVersatility->value, len+1);
                        }
                        else
                        {
                            cloneVersatility->value = (void *) EdgeCalloc(1, size);
                            if(IS_NULL(cloneVersatility->value))
                            {
                                goto CLONE_ERROR;
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
                                goto CLONE_ERROR;
                            }

                            char **dstVal = (char **) cloneVersatility->value;
                            size_t len;
                            for (int j = 0; j < srcVersatility->arrayLength; j++)
                            {
                                len = strlen(srcVal[j]);
                                dstVal[j] = (char*) EdgeCalloc(1, sizeof(char) * (len+1));
                                if(IS_NULL(dstVal[j]))
                                {
                                    goto CLONE_ERROR;
                                }
                                strncpy(dstVal[j], srcVal[j], len+1);
                            }
                        }
                        else
                        {
                            cloneVersatility->value = EdgeCalloc(srcVersatility->arrayLength, size);
                            if(IS_NULL(cloneVersatility->value))
                            {
                                goto CLONE_ERROR;
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
                        goto CLONE_ERROR;
                    }
                }
            }
        }
    }

    return clone;

CLONE_ERROR:
    freeEdgeMessage(clone);
    return NULL;
}

char getCharacterNodeIdType(uint32_t type)
{
    char nodeType;
    char nodeTypeArray[5] = { 'N', 'S', 'B', 'G', '\0' };
    nodeType = nodeTypeArray[4];

    if(type == UA_NODEIDTYPE_NUMERIC)
        nodeType = nodeTypeArray[0];
    if(type == UA_NODEIDTYPE_STRING)
        nodeType = nodeTypeArray[1];
    if(type == UA_NODEIDTYPE_BYTESTRING)
        nodeType = nodeTypeArray[2];
    if(type == UA_NODEIDTYPE_GUID)
        nodeType = nodeTypeArray[3];
    
    return nodeType;
}

UA_NodeId *cloneNodeId(UA_NodeId *nodeId)
{
    VERIFY_NON_NULL_MSG(nodeId, "nodeId param is NULL", NULL);
    UA_NodeId *clone = UA_NodeId_new();
    VERIFY_NON_NULL_MSG(clone, "Memory allocation failed.", NULL);
    switch(nodeId->identifierType)
    {
        case UA_NODEIDTYPE_NUMERIC:
            *clone = UA_NODEID_NUMERIC(nodeId->namespaceIndex, nodeId->identifier.numeric);
            break;
        case UA_NODEIDTYPE_STRING:
            clone->namespaceIndex = nodeId->namespaceIndex;
            clone->identifierType = UA_NODEIDTYPE_STRING;
            clone->identifier.string.length = nodeId->identifier.string.length;
            clone->identifier.string.data = (UA_Byte *) EdgeMalloc(clone->identifier.string.length);
            if(IS_NULL(clone->identifier.string.data))
            {
                EDGE_LOG(TAG, "Memory allocation failed.");
                goto CLONE_ERROR;
            }
            memcpy(clone->identifier.string.data, nodeId->identifier.string.data, nodeId->identifier.string.length);
            break;
        case UA_NODEIDTYPE_GUID:
            *clone = UA_NODEID_GUID(nodeId->namespaceIndex, nodeId->identifier.guid);
            break;
        case UA_NODEIDTYPE_BYTESTRING:
            clone->namespaceIndex = nodeId->namespaceIndex;
            clone->identifierType = UA_NODEIDTYPE_BYTESTRING;
            clone->identifier.byteString.length = nodeId->identifier.byteString.length;
            clone->identifier.byteString.data = (UA_Byte *) EdgeMalloc(clone->identifier.byteString.length);
            if(IS_NULL(clone->identifier.byteString.data))
            {
                EDGE_LOG(TAG, "Memory allocation failed.");
                goto CLONE_ERROR;
            }
            memcpy(clone->identifier.byteString.data, nodeId->identifier.byteString.data, nodeId->identifier.byteString.length);
            break;
    }
    return clone;

CLONE_ERROR:
    UA_NodeId_delete(clone);
    return NULL;
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
