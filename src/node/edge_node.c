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

#include "edge_node.h"
#include "edge_utils.h"
#include "edge_map.h"
#include "edge_logger.h"
#include "edge_malloc.h"
#include "edge_open62541.h"

#include <stdio.h>

#define TAG "edge_node"
#define MAX_ARGS  (10)

static edgeMap *methodNodeMap = NULL;
static size_t methodNodeCount = 0;
//static int numeric_id = 1000;

/****************************** Static functions ***********************************/
static void getDisplayInfo(const EdgeNodeItem *item, char* displayInfo) {
    if (NULL != item->displayName) {
        snprintf(displayInfo, MAX_DISPLAYNAME_SIZE, "v=%d,n=%s", item->variableIdentifier, item->displayName);
    } else {
        snprintf(displayInfo, MAX_DISPLAYNAME_SIZE, "v=%d", item->variableIdentifier);
    }
}

static UA_Byte getAccessLevel(int level)
{
    UA_Byte access_lvl = UA_ACCESSLEVELMASK_READ | UA_ACCESSLEVELMASK_WRITE;
    if (level == READ)
    {
        access_lvl = UA_ACCESSLEVELMASK_READ;
    }
    else if (level == WRITE)
    {
        access_lvl = UA_ACCESSLEVELMASK_WRITE;
    }
    return access_lvl;
}

/**
 * @brief addVariableNode - Add variable node to server
 * @param server - server handle
 * @param nsIndex - namespace index under which node has to be added
 * @param item - node to add
 */
static void addVariableNode(UA_Server *server, uint16_t nsIndex, const EdgeNodeItem *item)
{
    char *name = item->browseName;
    int id = item->variableIdentifier;
    int accessLevel = item->accessLevel;
    int userAccessLevel = item->userAccessLevel;
    double minimumSamplingInterval = item->minimumSamplingInterval;

    char displayInfo[MAX_DISPLAYNAME_SIZE] = {0,};
    getDisplayInfo(item, displayInfo);
    UA_VariableAttributes attr = UA_VariableAttributes_default;
    attr.description = UA_LOCALIZEDTEXT("en-US", name);
    attr.displayName = UA_LOCALIZEDTEXT("en-US", displayInfo);
    attr.minimumSamplingInterval = minimumSamplingInterval;
    EDGE_LOG_V(TAG, "[%s] sampling interval : %lf\n", item->browseName, attr.minimumSamplingInterval);

    attr.accessLevel = getAccessLevel(accessLevel);
    attr.userAccessLevel = getAccessLevel(userAccessLevel);
    attr.dataType = UA_TYPES[(int) id - 1].typeId;
    attr.valueRank = -1;

    int type = (int) id - 1;
    createScalarVariant(type, item->variableData, &attr.value);

    UA_StatusCode status = UA_Server_addVariableNode(server, UA_NODEID_STRING(nsIndex, item->browseName),
            UA_NODEID_NUMERIC(0, UA_NS0ID_OBJECTSFOLDER), UA_NODEID_NUMERIC(0, UA_NS0ID_ORGANIZES),
            UA_QUALIFIEDNAME(nsIndex, name), UA_NODEID_NUMERIC(0, 63), attr, NULL, NULL);

    UA_Variant_deleteMembers(&attr.value);

    COND_CHECK_NR_MSG((status != UA_STATUSCODE_GOOD), "+++ addVariableNode failed +++\n");
    EDGE_LOG(TAG, "+++ addVariableNode success +++\n");
}

/**
 * @brief addArrayNode - Add array node to server
 * @param server - server handle
 * @param nsIndex - namespace index under which array node has to be added
 * @param item - node to add
 */
static void addArrayNode(UA_Server *server, uint16_t nsIndex, const EdgeNodeItem *item)
{
    char *name = item->browseName;
    int id = item->variableIdentifier;

    UA_VariableAttributes attr = UA_VariableAttributes_default;
    char displayInfo[MAX_DISPLAYNAME_SIZE] = {0,};
    getDisplayInfo(item, displayInfo);
    attr.description = UA_LOCALIZEDTEXT("en-US", name);
    attr.displayName = UA_LOCALIZEDTEXT("en-US", displayInfo);
    attr.dataType = UA_TYPES[(int) id - 1].typeId;
    int accessLevel = item->accessLevel;
    int userAccessLevel = item->userAccessLevel;
    double minimumSamplingInterval = item->minimumSamplingInterval;

    attr.accessLevel = getAccessLevel(accessLevel);
    attr.userAccessLevel = getAccessLevel(userAccessLevel);
    attr.valueRank = 0;
    attr.minimumSamplingInterval = minimumSamplingInterval;
    EDGE_LOG_V(TAG, "[%s] sampling interval : %lf\n", item->browseName, item->minimumSamplingInterval);

    int type = (int) id - 1;
    createArrayVariant(type, item->variableData, item->arrayLength, &attr.value);

    UA_StatusCode status = UA_Server_addVariableNode(server, UA_NODEID_STRING(nsIndex, item->browseName),
            UA_NODEID_NUMERIC(0, UA_NS0ID_OBJECTSFOLDER), UA_NODEID_NUMERIC(0, UA_NS0ID_ORGANIZES),
            UA_QUALIFIEDNAME(nsIndex, name), UA_NODEID_NUMERIC(0, 63), attr, NULL, NULL);

    UA_Variant_deleteMembers(&attr.value);

    COND_CHECK_NR_MSG((status != UA_STATUSCODE_GOOD), "+++ addArrayNode failed +++\n");
    EDGE_LOG(TAG, "+++ addArrayNode success +++\n");
}

/**
 * @brief addObjectNode - Add object node to server
 * @param server - server handle
 * @param nsIndex - namespace index under which object node has to be added
 * @param item - Object node to be added
 */
static void addObjectNode(UA_Server *server, uint16_t nsIndex, const EdgeNodeItem *item)
{
    char *name = item->browseName;

    UA_ObjectAttributes object_attr = UA_ObjectAttributes_default;
    char displayInfo[MAX_DISPLAYNAME_SIZE] = {0,};
    getDisplayInfo(item, displayInfo);
    object_attr.description = UA_LOCALIZEDTEXT("en-US", name);
    object_attr.displayName = UA_LOCALIZEDTEXT("en-US", displayInfo);

    UA_NodeId sourceNodeId;
    char *nodeId = item->sourceNodeId->nodeId;
    if (nodeId != NULL)
    {
        sourceNodeId = UA_NODEID_STRING(nsIndex, nodeId);
    }
    else
    {
        sourceNodeId = UA_NODEID_NUMERIC(0, UA_NS0ID_OBJECTSFOLDER);
    }

    UA_StatusCode status = UA_Server_addObjectNode(server, UA_NODEID_STRING(nsIndex, item->browseName),
            sourceNodeId, UA_NODEID_NUMERIC(0, UA_NS0ID_ORGANIZES), UA_QUALIFIEDNAME(nsIndex, name),
            UA_NODEID_NUMERIC(0, UA_NS0ID_FOLDERTYPE), object_attr, NULL, NULL);

    COND_CHECK_NR_MSG((status != UA_STATUSCODE_GOOD), "+++ addObjectNode failed +++\n");
    EDGE_LOG(TAG, "+++ addObjectNode success +++\n");
}

/**
 * @brief addObjectTypeNode - Add object type node to server
 * @param server - server handle
 * @param nsIndex - namespace index under which object type node has to added
 * @param item - Object type node to add
 */
static void addObjectTypeNode(UA_Server *server, uint16_t nsIndex, const EdgeNodeItem *item)
{
    char *name = item->browseName;
    UA_ObjectTypeAttributes object_attr = UA_ObjectTypeAttributes_default;
    char displayInfo[MAX_DISPLAYNAME_SIZE] = {0,};
    getDisplayInfo(item, displayInfo);
    object_attr.description = UA_LOCALIZEDTEXT("en-US", name);
    object_attr.displayName = UA_LOCALIZEDTEXT("en-US", displayInfo);

    UA_NodeId sourceNodeId;
    char *nodeId = item->sourceNodeId->nodeId;
    if (nodeId != NULL)
    {
        sourceNodeId = UA_NODEID_STRING(nsIndex, nodeId);
    }
    else
    {
        sourceNodeId = UA_NODEID_NUMERIC(0, UA_NS0ID_BASEOBJECTTYPE);
    }

    UA_StatusCode status = UA_Server_addObjectTypeNode(server,
            UA_NODEID_STRING(nsIndex, item->browseName), sourceNodeId,
            UA_NODEID_NUMERIC(0, UA_NS0ID_HASSUBTYPE), UA_QUALIFIEDNAME(nsIndex, name), object_attr, NULL,
            NULL);

    COND_CHECK_NR_MSG((status != UA_STATUSCODE_GOOD), "+++ addObjectTypeNode failed +++\n");
    EDGE_LOG(TAG, "+++ addObjectTypeNode success +++\n");
}

/**
 * @brief addVariableTypeNode - Add VariableType node to server
 * @param server - server handle
 * @param nsIndex - namespace index under which variableType node has to be added
 * @param item - VariableType node to add
 */
static void addVariableTypeNode(UA_Server *server, uint16_t nsIndex, const EdgeNodeItem *item)
{
    char *name = item->browseName;
    int id = item->variableIdentifier;

    UA_VariableTypeAttributes attr = UA_VariableTypeAttributes_default;
    char displayInfo[MAX_DISPLAYNAME_SIZE] = {0,};
    getDisplayInfo(item, displayInfo);
    attr.description = UA_LOCALIZEDTEXT("en-US", name);
    attr.displayName = UA_LOCALIZEDTEXT("en-US", displayInfo);
    attr.dataType = UA_TYPES[(int) id - 1].typeId;

    int type = (int) id - 1;
    createArrayVariant(type, item->variableData, item->arrayLength, &attr.value);

    UA_StatusCode status = UA_Server_addVariableTypeNode(server,
            UA_NODEID_STRING(nsIndex, item->browseName),
            UA_NODEID_NUMERIC(0, UA_NS0ID_BASEDATAVARIABLETYPE),
            UA_NODEID_NUMERIC(0, UA_NS0ID_HASSUBTYPE), UA_QUALIFIEDNAME(nsIndex, name),
            UA_NODEID_NUMERIC(0, UA_NS0ID_BASEDATAVARIABLETYPE), attr, NULL, NULL);

    //UA_Variant_deleteMembers(&attr.value);

    COND_CHECK_NR_MSG((status != UA_STATUSCODE_GOOD), "+++ addVariableTypeNode failed +++\n");
    EDGE_LOG(TAG, "+++ addVariableTypeNode success +++\n");
}

/**
 * @brief addDataTypeNode - Add datatype node to server
 * @param server - server handle
 * @param nsIndex - namespace index under which node has to be added
 * @param item - datatype node to add
 */
static void addDataTypeNode(UA_Server *server, uint16_t nsIndex, const EdgeNodeItem *item)
{
    char *name = item->browseName;

    UA_DataTypeAttributes attr = UA_DataTypeAttributes_default;
    char displayInfo[MAX_DISPLAYNAME_SIZE] = {0,};
    getDisplayInfo(item, displayInfo);
    attr.description = UA_LOCALIZEDTEXT("en-US", name);
    attr.displayName = UA_LOCALIZEDTEXT("en-US", displayInfo);

    UA_NodeId sourceNodeId = UA_NODEID_NUMERIC(0, UA_NS0ID_BASEDATATYPE);
    char *nodeId = item->sourceNodeId->nodeId;

    UA_StatusCode status = UA_Server_addDataTypeNode(server, UA_NODEID_STRING(nsIndex, item->browseName),
            sourceNodeId, UA_NODEID_NUMERIC(0, UA_NS0ID_HASSUBTYPE), UA_QUALIFIEDNAME(nsIndex, name),
            attr, NULL, NULL);

    if (nodeId != NULL)
    {
        sourceNodeId = UA_NODEID_STRING(nsIndex, nodeId);
        status = UA_Server_addReference(server, sourceNodeId,
                UA_NODEID_NUMERIC(0, UA_NS0ID_ORGANIZES),
                UA_EXPANDEDNODEID_STRING(nsIndex, item->browseName), true);
    }

    COND_CHECK_NR_MSG((status != UA_STATUSCODE_GOOD), "+++ addDataTypeNode failed +++\n");
    EDGE_LOG(TAG, "+++ addDataTypeNode success +++\n");
}

/**
 * @brief addViewNode - Add view node to server
 * @param server - server handle
 * @param nsIndex - namespace index under which node has to be added
 * @param item - node to add
 */
static void addViewNode(UA_Server *server, uint16_t nsIndex, const EdgeNodeItem *item)
{
    char *name = item->browseName;

    UA_ViewAttributes attr = UA_ViewAttributes_default;
    char displayInfo[MAX_DISPLAYNAME_SIZE] = {0,};
    getDisplayInfo(item, displayInfo);
    attr.description = UA_LOCALIZEDTEXT("en-US", name);
    attr.displayName = UA_LOCALIZEDTEXT("en-US", displayInfo);

    UA_NodeId sourceNodeId; // = UA_NODEID_NUMERIC(0, UA_NS0ID_VIEWSFOLDER);
    char *nodeId = item->sourceNodeId->nodeId;
    if (nodeId != NULL)
    {
        sourceNodeId = UA_NODEID_STRING(nsIndex, nodeId);
    }
    else
    {
        sourceNodeId = UA_NODEID_NUMERIC(0, UA_NS0ID_VIEWSFOLDER);
    }

    UA_StatusCode status = UA_Server_addViewNode(server, UA_NODEID_STRING(nsIndex, item->browseName),
            sourceNodeId, UA_NODEID_NUMERIC(0, UA_NS0ID_ORGANIZES), UA_QUALIFIEDNAME(nsIndex, name), attr,
            NULL, NULL);

    COND_CHECK_NR_MSG((status != UA_STATUSCODE_GOOD), "+++ addViewNode failed +++\n");
    EDGE_LOG(TAG, "+++ addViewNode success +++\n");
}

EdgeResult addReferences(UA_Server *server, EdgeReference *reference, uint16_t src_nsIndex, uint16_t target_nsIndex)
{

    EdgeResult result;
    result.code = STATUS_ERROR;

    COND_CHECK_MSG((!server), "Server Handle Invalid!! \n", result);

    if (!reference->referenceId)
    {
        reference->referenceId = UA_NS0ID_ORGANIZES;
    }

    UA_ExpandedNodeId expanded_nodeId = UA_EXPANDEDNODEID_STRING(target_nsIndex, reference->targetPath);
    UA_StatusCode status = UA_Server_addReference(server,
            UA_NODEID_STRING(src_nsIndex, reference->sourcePath),
            UA_NODEID_NUMERIC(0, reference->referenceId), expanded_nodeId, reference->forward);

    COND_CHECK_MSG((status != UA_STATUSCODE_GOOD), "+++ addReference failed +++\n", result);

    result.code = STATUS_OK;
    EDGE_LOG(TAG, "+++ addReference success +++\n");
    return result;
}

static void addReferenceTypeNode(UA_Server *server, uint16_t nsIndex, const EdgeNodeItem *item)
{
    char *name = item->browseName;

    UA_ReferenceTypeAttributes attr = UA_ReferenceTypeAttributes_default;
    char displayInfo[MAX_DISPLAYNAME_SIZE] = {0,};
    getDisplayInfo(item, displayInfo);
    attr.description = UA_LOCALIZEDTEXT("en-US", name);
    attr.displayName = UA_LOCALIZEDTEXT("en-US", displayInfo);

    UA_NodeId sourceNodeId;
    UA_ExpandedNodeId expandedSourceNodeId;
    char *nodeId = item->sourceNodeId->nodeId;
    if (nodeId == NULL)
    {
        sourceNodeId = UA_NODEID_NUMERIC(0, UA_NS0ID_ORGANIZES);
        expandedSourceNodeId = UA_EXPANDEDNODEID_NUMERIC(0, UA_NS0ID_ORGANIZES);
    }
    else
    {
        sourceNodeId = UA_NODEID_STRING(nsIndex, nodeId);
        expandedSourceNodeId = UA_EXPANDEDNODEID_STRING(nsIndex, nodeId);
    }
    UA_StatusCode status = UA_Server_addReferenceTypeNode(server,
            UA_NODEID_STRING(nsIndex, item->browseName), sourceNodeId,
            UA_NODEID_NUMERIC(0, UA_NS0ID_HASSUBTYPE), UA_QUALIFIEDNAME(nsIndex, name), attr, NULL, NULL);

    if (status == UA_STATUSCODE_GOOD)
    {
        EDGE_LOG(TAG, "+++ addReferenceTypeNode success +++\n");
    }
    else
    {
        EDGE_LOG(TAG, "+++ addReferenceTypeNode failed +++\n");
    }

    status = UA_Server_addReference(server, sourceNodeId, UA_NODEID_NUMERIC(0, UA_NS0ID_ORGANIZES),
            UA_EXPANDEDNODEID_STRING(nsIndex, item->browseName), true);
    if (status != UA_STATUSCODE_GOOD)
    {
        EDGE_LOG(TAG, "+++ UA_Server_addReference(forward) failed +++\n");
    }

    status = UA_Server_addReference(server, UA_NODEID_STRING(nsIndex, item->browseName),
            UA_NODEID_NUMERIC(0, UA_NS0ID_ORGANIZES), expandedSourceNodeId, false);
    COND_CHECK_NR_MSG((status != UA_STATUSCODE_GOOD), "+++ UA_Server_addReference failed +++\n");
}

static keyValue getMethodMapElement(const edgeMap *map, keyValue key)
{
    edgeMapNode *temp = map->head;
    while (temp != NULL)
    {
        COND_CHECK_MSG((!strcmp(temp->key, key)), "", temp->value);
        temp = temp->next;
    }
    return NULL;
}

static void destroyInputArgs(void **inp, size_t inputSize, const UA_Variant *input)
{
    VERIFY_NON_NULL_NR_MSG(inp, "");
    VERIFY_NON_NULL_NR_MSG(input, "");

    for (size_t i = 0; i < inputSize; i++)
    {
        if (input[i].type == &UA_TYPES[UA_TYPES_STRING])
        {
            if (input[i].arrayLength == 0)
            {
                EdgeFree(inp[i]);
            }
            else
            {
                char **values = (char**) inp[i];
                for (size_t j = 0; j < input[i].arrayLength; j++)
                {
                    EdgeFree(values[j]);
                }
                EdgeFree(values);
            }
        }
    }
    EdgeFree(inp);
}

static UA_StatusCode methodCallback(UA_Server *server, const UA_NodeId *sessionId,
        void *sessionContext, const UA_NodeId *methodId, void *methodContext,
        const UA_NodeId *objectId, void *objectContext, size_t inputSize, const UA_Variant *input,
        size_t outputSize, UA_Variant *output)
{
    char *key = (char *) methodId->identifier.string.data;
    keyValue value = getMethodMapElement(methodNodeMap, (keyValue) key);
    VERIFY_NON_NULL_MSG(value, "", UA_STATUSCODE_BADMETHODINVALID);

    EdgeMethod *method = (EdgeMethod *) value;
    method_func method_to_call = (method_func) (method->method_fn);

    void **inp = NULL;
    if (inputSize > 0)
    {
        inp = EdgeCalloc(inputSize, sizeof(void *));
        VERIFY_NON_NULL_MSG(inp, "EdgeCalloc FAILED for inp in methodCallback\n", STATUS_ERROR);
        for (size_t i = 0; i < inputSize; i++)
        {
            if (input[i].type == &UA_TYPES[UA_TYPES_STRING])
            {
                if (input[i].arrayLength == 0)
                {
                    /* Scalar string value */
                    UA_String *str = ((UA_String*) input[i].data);
                    char *values = (char*) EdgeMalloc(sizeof(char) * (str->length+1));
                    if(IS_NULL(values))
                    {
                        destroyInputArgs(inp, i, input);
                        return STATUS_ERROR;
                    }
                    strncpy(values, (char *) str->data, str->length);
                    values[str->length] = '\0';
                    inp[i] = (void*) values;
                }
                else
                {
                    UA_String* str = ((UA_String*) input[i].data);
                    char **values = (char**) EdgeCalloc(input[i].arrayLength, sizeof(char*));
                    if(IS_NULL(values))
                    {
                        destroyInputArgs(inp, i, input);
                        return STATUS_ERROR;
                    }

                    for (size_t j = 0; j < input[i].arrayLength; j++)
                    {
                        values[j] = (char *) EdgeMalloc(str[j].length+1);
                        if(IS_NULL(values[j]))
                        {
                            destroyInputArgs(inp, i, input);
                            for(size_t k = 0; k < j; ++k)
                            {
                                EdgeFree(values[j]);
                            }
                            EdgeFree(values);
                            return STATUS_ERROR;
                        }
                        strncpy(values[j], (char *) str[j].data, str[j].length);
                        values[j][str[j].length] = '\0';
                    }
                    inp[i] = (void*) values;
                }
            }
            else
            {
                inp[i] = input[i].data;
            }
        }
    }

    void **out = NULL;
    if (outputSize > 0)
    {
        out = EdgeCalloc(outputSize, sizeof(void *));
        if(IS_NULL(out))
        {
            EDGE_LOG(TAG, "ERROR : out in methodCallback Malloc FAILED\n");
            destroyInputArgs(inp, inputSize, input);
            return STATUS_ERROR;
        }
    }

    method_to_call(inputSize, inp, outputSize, out);

    destroyInputArgs(inp, inputSize, input);

    for (size_t idx = 0; idx < outputSize; idx++)
    {
        if (out[idx] != NULL)
        {
            int type = (int) method->outArg[idx]->argType - 1;
            if (method->outArg[idx]->valType == SCALAR)
            {
                // Scalar copy
                createScalarVariant(type, *out, &output[idx]);
            }
            else if (method->outArg[idx]->valType == ARRAY_1D)
            {
                // Array copy
                createArrayVariant(type, out[idx], method->outArg[idx]->arrayLength, &output[idx]);
            }
            EdgeFree(out[idx]);
        }
    }
    EdgeFree(out);
    return UA_STATUSCODE_GOOD;
}

/****************************** Member functions ***********************************/

EdgeResult addNodes(UA_Server *server, uint16_t nsIndex, const EdgeNodeItem *item)
{
    EdgeResult result = { STATUS_PARAM_INVALID };
    VERIFY_NON_NULL_MSG(item, "", result);

    if (item->nodeType == ARRAY_NODE)
    {
        addArrayNode(server, nsIndex, item);
    }
    else if (item->nodeType == OBJECT_NODE)
    {
        addObjectNode(server, nsIndex, item);
    }
    else if (item->nodeType == OBJECT_TYPE_NODE)
    {
        addObjectTypeNode(server, nsIndex, item);
    }
    else if (item->nodeType == VARIABLE_TYPE_NODE)
    {
        addVariableTypeNode(server, nsIndex, item);
    }
    else if (item->nodeType == DATA_TYPE_NODE)
    {
        addDataTypeNode(server, nsIndex, item);
    }
    else if (item->nodeType == VIEW_NODE)
    {
        addViewNode(server, nsIndex, item);
    }
    else if (item->nodeType == REFERENCE_TYPE_NODE)
    {
        addReferenceTypeNode(server, nsIndex, item);
    }
    else
    {
        addVariableNode(server, nsIndex, item);
    }

    result.code = STATUS_OK;
    return result;
}

EdgeResult addMethodNode(UA_Server *server, uint16_t nsIndex, const EdgeNodeItem *item, EdgeMethod *method)
{
    EdgeResult result;
    result.code = STATUS_ERROR;

    VERIFY_NON_NULL_MSG(server, "NULL server parameter in addMethodNode\n", result);

    int num_inpArgs = method->num_inpArgs;
    int num_outArgs = method->num_outArgs;
    size_t idx = 0;

    /* Input Arguments */
    UA_Argument inputArguments[MAX_ARGS];
    for (idx = 0; idx < num_inpArgs; idx++)
    {
        UA_Argument_init(&inputArguments[idx]);
        inputArguments[idx].description = UA_LOCALIZEDTEXT("en-US", method->description);
        inputArguments[idx].name = UA_STRING(method->description);
        inputArguments[idx].dataType = UA_TYPES[(int) method->inpArg[idx]->argType - 1].typeId;

        if (method->inpArg[idx]->valType == SCALAR)
        {
            inputArguments[idx].valueRank = -1; /* Scalar */
        }
        else if (method->inpArg[idx]->valType == ARRAY_1D)
        {
            inputArguments[idx].valueRank = 1; /* Array with one dimensions */
            UA_UInt32 *inputDimensions = (UA_UInt32 *) EdgeMalloc(sizeof(UA_UInt32));
            VERIFY_NON_NULL_MSG(inputDimensions, "EdgeMalloc FAILEd for inputDimensions in addMethodNode\n", result);
            inputDimensions[0] = method->inpArg[idx]->arrayLength;
            inputArguments[idx].arrayDimensionsSize = 1;
            inputArguments[idx].arrayDimensions = inputDimensions;
        }
    }

    /* Output Arguments */
    UA_Argument outputArguments[MAX_ARGS];
    for (idx = 0; idx < num_outArgs; idx++)
    {
        UA_Argument_init(&outputArguments[idx]);
        outputArguments[idx].description = UA_LOCALIZEDTEXT("en-US", method->description);
        outputArguments[idx].name = UA_STRING(method->description);
        outputArguments[idx].dataType = UA_TYPES[(size_t) method->outArg[idx]->argType - 1].typeId;

        if (method->outArg[idx]->valType == SCALAR)
        {
            outputArguments[idx].valueRank = -1; /* Scalar */
        }
        else if (method->outArg[idx]->valType == ARRAY_1D)
        {
            outputArguments[idx].valueRank = 1; /* Array with one dimensions */
            UA_UInt32 *outputDimensions = (UA_UInt32 *) EdgeMalloc(sizeof(UA_UInt32));
            if (IS_NULL(outputDimensions))
            {
                EDGE_LOG(TAG, "ERROR : outputDimensions MALLOC failed\n");
                for (idx = 0; idx < num_inpArgs; idx++)
                {
                    if (method->inpArg[idx]->valType == ARRAY_1D)
                    {
                        EdgeFree(inputArguments[idx].arrayDimensions);
                    }
                }
                result.code = STATUS_ERROR;
                return result;
            }
            outputDimensions[0] = method->outArg[idx]->arrayLength;
            outputArguments[idx].arrayDimensionsSize = 1;
            outputArguments[idx].arrayDimensions = outputDimensions;
        }
    }

    UA_MethodAttributes methodAttr = UA_MethodAttributes_default;
    char displayInfo[MAX_DISPLAYNAME_SIZE] = {0,};
    getDisplayInfo(item, displayInfo);
    methodAttr.description = UA_LOCALIZEDTEXT("en-US", method->methodNodeName);
    methodAttr.displayName = UA_LOCALIZEDTEXT("en-US", displayInfo);
    methodAttr.executable = true;
    methodAttr.userExecutable = true;

    UA_NodeId sourceNodeId;
    UA_ExpandedNodeId expandedSourceNodeId;
    EdgeNodeId *sourceNode = item->sourceNodeId;
    if (sourceNode == NULL || sourceNode->nodeId == NULL)
    {
        sourceNodeId = UA_NODEID_NUMERIC(0, UA_NS0ID_OBJECTSFOLDER);
        expandedSourceNodeId = UA_EXPANDEDNODEID_NUMERIC(nsIndex, UA_NS0ID_OBJECTSFOLDER);
    }
    else
    {
        sourceNodeId = UA_NODEID_STRING(nsIndex, sourceNode->nodeId);
        expandedSourceNodeId = UA_EXPANDEDNODEID_STRING(nsIndex, sourceNode->nodeId);
    }

    UA_StatusCode status = UA_Server_addMethodNode(server, UA_NODEID_STRING(nsIndex, item->browseName),
            sourceNodeId, UA_NODEID_NUMERIC(0, UA_NS0ID_HASCOMPONENT),
            UA_QUALIFIEDNAME(nsIndex, item->browseName), methodAttr, &methodCallback, num_inpArgs,
            inputArguments, num_outArgs, outputArguments, NULL, NULL);
    if (status == UA_STATUSCODE_GOOD)
    {
        EDGE_LOG(TAG, "+++ addMethodNode success +++\n");
        if (NULL == methodNodeMap)
            methodNodeMap = createMap();

        char *browseName = (char *) EdgeMalloc(strlen(item->browseName) + 1);
        VERIFY_NON_NULL_MSG(browseName, "EdgeMalloc FAILED for browseName in addMethodNode\n", result);
        strncpy(browseName, item->browseName, strlen(item->browseName));
        browseName[strlen(item->browseName)] = '\0';
        insertMapElement(methodNodeMap, (void *) browseName, method);
        methodNodeCount += 1;
    }
    else
    {
        EDGE_LOG(TAG, "+++ addMethodNode failed +++\n");
    }

    status = UA_Server_addReference(server, UA_NODEID_NUMERIC(0, UA_NS0ID_OBJECTSFOLDER),
            UA_NODEID_NUMERIC(0, UA_NS0ID_ORGANIZES), UA_EXPANDEDNODEID_STRING(nsIndex, item->browseName),
            true);
    if (status != UA_STATUSCODE_GOOD)
    {
        EDGE_LOG(TAG, "+++ UA_Server_addReference(forward) failed +++\n");
    }

    if (sourceNode != NULL && sourceNode->nodeId != NULL)
    {
        status = UA_Server_addReference(server, sourceNodeId,
                UA_NODEID_NUMERIC(0, UA_NS0ID_ORGANIZES),
                UA_EXPANDEDNODEID_STRING(nsIndex, item->browseName), true);
        if (status != UA_STATUSCODE_GOOD)
        {
            EDGE_LOG(TAG, "+++ UA_Server_addReference(forward) failed +++\n");
        }

        status = UA_Server_addReference(server, UA_NODEID_STRING(nsIndex, item->browseName),
                UA_NODEID_NUMERIC(0, UA_NS0ID_ORGANIZES), expandedSourceNodeId, false);
        if (status != UA_STATUSCODE_GOOD)
        {
            EDGE_LOG(TAG, "+++ UA_Server_addReference failed +++\n");
        }
    }

    for (idx = 0; idx < num_outArgs; idx++)
    {
        if (method->outArg[idx]->valType == ARRAY_1D)
        {
            EdgeFree(outputArguments[idx].arrayDimensions);
        }
    }
    for (idx = 0; idx < num_inpArgs; idx++)
    {

        if (method->inpArg[idx]->valType == ARRAY_1D)
        {
            EdgeFree(inputArguments[idx].arrayDimensions);
        }
    }

    result.code = STATUS_OK;
    return result;
}

EdgeResult modifyNode(UA_Server *server, uint16_t nsIndex, const char *nodeUri, EdgeVersatility *value)
{
    EdgeResult result;
    result.code = STATUS_ERROR;

    VERIFY_NON_NULL_MSG(server, "NULL server parameter in modifyNode\n", result);

    // read the value;
    UA_NodeId node = UA_NODEID_STRING(nsIndex, (char*)nodeUri);
    UA_Variant *readval = UA_Variant_new();
    VERIFY_NON_NULL_MSG(readval, "NULL UA_Variant received in modifyNode\n", result);

    UA_StatusCode ret = UA_Server_readValue(server, node, readval);
    if (ret != UA_STATUSCODE_GOOD)
    {
        EDGE_LOG(TAG, "error in read value during modify node \n");
        UA_Variant_delete(readval);
        result.code = STATUS_ERROR;
        return result;
    }

    const UA_DataType *type = readval->type;
    UA_Variant *myVariant = UA_Variant_new();
    if (UA_Variant_isScalar(readval))
    {
        ret = createScalarVariant(type->typeIndex, value->value, myVariant);
    }
    else
    {
        ret = createArrayVariant(type->typeIndex, value->value, value->arrayLength, myVariant);
    }

    if (ret != UA_STATUSCODE_GOOD)
    {
        EDGE_LOG(TAG, "error in set scalar copy during modify node \n");
        result.code = STATUS_ERROR;
        UA_Variant_delete(readval);
        UA_Variant_delete(myVariant);
        return result;
    }

    UA_StatusCode retVal = UA_Server_writeValue(server, node, *myVariant);
    if (retVal != UA_STATUSCODE_GOOD)
    {
        EDGE_LOG_V(TAG, "Error in modifying node value:: 0x%08x\n", retVal);
        result.code = STATUS_ERROR;
        UA_Variant_delete(readval);
        UA_Variant_delete(myVariant);
        return result;
    }

    EDGE_LOG(TAG, "+++ write successful +++\n\n");
    UA_Variant_delete(myVariant);
    UA_Variant_delete(readval);

    result.code = STATUS_OK;
    return result;
}
/***********************************************************************************/
