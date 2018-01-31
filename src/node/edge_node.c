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
#include "edge_logger.h"
#include "edge_malloc.h"

#include <stdio.h>

#define TAG "edge_node"
#define MAX_ARGS  10

static edgeMap *methodNodeMap = NULL;
static int methodNodeCount = 0;
//static int numeric_id = 1000;

/****************************** Static functions ***********************************/

static void addVariableNode(UA_Server *server, int nsIndex, EdgeNodeItem *item)
{
    char *name = item->browseName;
    EdgeNodeIdentifier id = item->variableIdentifier;
    int accessLevel = item->accessLevel;
    int userAccessLevel = item->userAccessLevel;

    UA_VariableAttributes attr = UA_VariableAttributes_default;
    attr.description = UA_LOCALIZEDTEXT("en-US", name);
    attr.displayName = UA_LOCALIZEDTEXT("en-US", name);

    if (accessLevel == READ)
    {
        EDGE_LOG(TAG, "accessLevel :: UA_ACCESSLEVELMASK_READ\n");
        attr.accessLevel = UA_ACCESSLEVELMASK_READ;
    }
    else if (accessLevel == WRITE)
    {
        EDGE_LOG(TAG, "accessLevel :: UA_ACCESSLEVELMASK_WRITE\n");
        attr.accessLevel = UA_ACCESSLEVELMASK_WRITE;
    }
    else
    {
        EDGE_LOG(TAG, "accessLevel :: UA_ACCESSLEVELMASK\n");
        attr.accessLevel = UA_ACCESSLEVELMASK_READ | UA_ACCESSLEVELMASK_WRITE;
    }

    if (userAccessLevel == READ)
    {
        EDGE_LOG(TAG, "userAccessLevel :: UA_ACCESSLEVELMASK_READ\n");
        attr.userAccessLevel = UA_ACCESSLEVELMASK_READ;
    }
    else if (userAccessLevel == WRITE)
    {
        EDGE_LOG(TAG, "userAccessLevel :: UA_ACCESSLEVELMASK_WRITE\n");
        attr.userAccessLevel = UA_ACCESSLEVELMASK_WRITE;
    }
    else
    {
        EDGE_LOG(TAG, "userAccessLevel :: UA_ACCESSLEVELMASK\n");
        attr.userAccessLevel = UA_ACCESSLEVELMASK_READ | UA_ACCESSLEVELMASK_WRITE;
    }

    attr.dataType = UA_TYPES[(int) id - 1].typeId;

    int type = (int) id - 1;
    if (type == UA_TYPES_STRING)
    {
        UA_String val = UA_STRING_ALLOC((char * ) item->variableData);
        UA_Variant_setScalarCopy(&attr.value, &val, &UA_TYPES[type]);
        UA_String_deleteMembers(&val);

        FREE(val.data);
    }
    else
    {
        UA_Variant_setScalarCopy(&attr.value, item->variableData, &UA_TYPES[type]);
    }

    UA_StatusCode status = UA_Server_addVariableNode(server, UA_NODEID_STRING(nsIndex, item->browseName),
            UA_NODEID_NUMERIC(0, UA_NS0ID_OBJECTSFOLDER), UA_NODEID_NUMERIC(0, UA_NS0ID_ORGANIZES),
            UA_QUALIFIEDNAME(nsIndex, name), UA_NODEID_NUMERIC(0, 63), attr, NULL, NULL);

    if (status == UA_STATUSCODE_GOOD)
    {
        EDGE_LOG(TAG, "+++ addVariableNode success +++\n");
    }
    else
    {
        EDGE_LOG(TAG, "+++ addVariableNode failed +++\n");
    }
    UA_Variant_deleteMembers(&attr.value);

//    status = UA_Server_addReference(server, UA_NODEID_NUMERIC(0, UA_NS0ID_OBJECTSFOLDER),
//                                                    UA_NODEID_NUMERIC(0, UA_NS0ID_ORGANIZES),
//                                                    UA_EXPANDEDNODEID_STRING(1, item->browseName), item->forward);
//    status = UA_Server_addReference(server, UA_NODEID_STRING(1, item->browseName),
//                                                    UA_NODEID_NUMERIC(0, UA_NS0ID_ORGANIZES),
//                                                    UA_EXPANDEDNODEID_NUMERIC(0, UA_NS0ID_OBJECTSFOLDER), !(item->forward));
}

static void addArrayNode(UA_Server *server, int nsIndex, EdgeNodeItem *item)
{
    char *name = item->browseName;
    EdgeNodeIdentifier id = item->variableIdentifier;

    UA_VariableAttributes attr = UA_VariableAttributes_default;
    attr.description = UA_LOCALIZEDTEXT("en-US", name);
    attr.displayName = UA_LOCALIZEDTEXT("en-US", name);
    attr.dataType = UA_TYPES[(int) id - 1].typeId;
    int accessLevel = item->accessLevel;
    int userAccessLevel = item->userAccessLevel;

    if (accessLevel == READ)
    {
        attr.accessLevel = UA_ACCESSLEVELMASK_READ;
    }
    else if (accessLevel == WRITE)
    {
        attr.accessLevel = UA_ACCESSLEVELMASK_WRITE;
    }
    else if (accessLevel == READ_WRITE)
    {
        attr.accessLevel = UA_ACCESSLEVELMASK_READ | UA_ACCESSLEVELMASK_WRITE;
    }

    if (userAccessLevel == READ)
    {
        attr.userAccessLevel = UA_ACCESSLEVELMASK_READ;
    }
    else if (userAccessLevel == WRITE)
    {
        attr.userAccessLevel = UA_ACCESSLEVELMASK_WRITE;
    }
    else if (userAccessLevel == READ_WRITE)
    {
        attr.userAccessLevel = UA_ACCESSLEVELMASK_READ | UA_ACCESSLEVELMASK_WRITE;
    }

    int type = (int) id - 1;
    if (type == UA_TYPES_STRING)
    {
        int idx = 0;
        char **data1 = (char **) item->variableData;
        UA_String *array = (UA_String *) UA_Array_new(item->arrayLength, &UA_TYPES[type]);
        for (idx = 0; idx < item->arrayLength; idx++)
        {
            array[idx] = UA_STRING_ALLOC(data1[idx]);
        }
        UA_Variant_setArrayCopy(&attr.value, array, item->arrayLength, &UA_TYPES[type]);
        for (idx = 0; idx < item->arrayLength; idx++)
        {
            UA_String_deleteMembers(&array[idx]);
        }
        UA_Array_delete(array, item->arrayLength, &UA_TYPES[type]);
    }
    else if (type == UA_TYPES_BYTESTRING)
    {
        int idx = 0;
        UA_ByteString **dataArray = (UA_ByteString **) item->variableData;
        UA_ByteString *array = (UA_ByteString *) UA_Array_new(item->arrayLength, &UA_TYPES[type]);
        for (idx = 0; idx < item->arrayLength; idx++)
        {
            char *itemData = (char *) dataArray[idx]->data;
            array[idx] = UA_BYTESTRING_ALLOC(itemData);
        }
        UA_Variant_setArrayCopy(&attr.value, array, item->arrayLength, &UA_TYPES[type]);
        for (idx = 0; idx < item->arrayLength; idx++)
        {
            UA_ByteString_deleteMembers(&array[idx]);
        }
        UA_Array_delete(array, item->arrayLength, &UA_TYPES[type]);
    }
    else
    {
        UA_Variant_setArrayCopy(&attr.value, item->variableData, item->arrayLength,
                &UA_TYPES[type]);
    }

    UA_StatusCode status = UA_Server_addVariableNode(server, UA_NODEID_STRING(nsIndex, item->browseName),
            UA_NODEID_NUMERIC(0, UA_NS0ID_OBJECTSFOLDER), UA_NODEID_NUMERIC(0, UA_NS0ID_ORGANIZES),
            UA_QUALIFIEDNAME(nsIndex, name), UA_NODEID_NUMERIC(0, 63), attr, NULL, NULL);

    if (status == UA_STATUSCODE_GOOD)
    {
        EDGE_LOG(TAG, "+++ addArrayNode success +++\n");
    }
    else
    {
        EDGE_LOG(TAG, "+++ addArrayNode failed +++\n");
    }
    UA_Variant_deleteMembers(&attr.value);
}

static void addObjectNode(UA_Server *server, int nsIndex, EdgeNodeItem *item)
{
    char *name = item->browseName;

    UA_ObjectAttributes object_attr = UA_ObjectAttributes_default;
    object_attr.description = UA_LOCALIZEDTEXT("en-US", name);
    object_attr.displayName = UA_LOCALIZEDTEXT("en-US", name);

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

    if (status == UA_STATUSCODE_GOOD)
    {
        EDGE_LOG(TAG, "+++ addObjectNode success +++\n");
    }
    else
    {
        EDGE_LOG(TAG, "+++ addObjectNode failed +++\n");
    }
}

static void addObjectTypeNode(UA_Server *server, int nsIndex, EdgeNodeItem *item)
{
    char *name = item->browseName;
    UA_ObjectTypeAttributes object_attr = UA_ObjectTypeAttributes_default;
    object_attr.description = UA_LOCALIZEDTEXT("en-US", name);
    object_attr.displayName = UA_LOCALIZEDTEXT("en-US", name);

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

    if (status == UA_STATUSCODE_GOOD)
    {
        EDGE_LOG(TAG, "+++ addObjectTypeNode success +++\n");
    }
    else
    {
        EDGE_LOG(TAG, "+++ addObjectTypeNode failed +++\n");
    }
}

static void addVariableTypeNode(UA_Server *server, int nsIndex, EdgeNodeItem *item)
{
    char *name = item->browseName;
    EdgeNodeIdentifier id = item->variableIdentifier;

    UA_VariableTypeAttributes attr = UA_VariableTypeAttributes_default;
    attr.description = UA_LOCALIZEDTEXT("en-US", name);
    attr.displayName = UA_LOCALIZEDTEXT("en-US", name);
    attr.dataType = UA_TYPES[(int) id - 1].typeId;

    int type = (int) id - 1;
    if (type == UA_TYPES_STRING)
    {
        int idx = 0;
        char **data1 = (char **) item->variableData;
        UA_String *array = (UA_String *) UA_Array_new(item->arrayLength, &UA_TYPES[type]);
        for (idx = 0; idx < item->arrayLength; idx++)
        {
            array[idx] = UA_STRING_ALLOC(data1[idx]);
        }
        UA_Variant_setArrayCopy(&attr.value, array, item->arrayLength, &UA_TYPES[type]);
        for (idx = 0; idx < item->arrayLength; idx++)
        {
            UA_String_deleteMembers(&array[idx]);
        }
        UA_Array_delete(array, item->arrayLength, &UA_TYPES[type]);
    }
    else
    {
        UA_Variant_setArray(&attr.value, item->variableData, item->arrayLength, &UA_TYPES[type]);
    }

    UA_StatusCode status = UA_Server_addVariableTypeNode(server,
            UA_NODEID_STRING(nsIndex, item->browseName),
            UA_NODEID_NUMERIC(0, UA_NS0ID_BASEDATAVARIABLETYPE),
            UA_NODEID_NUMERIC(0, UA_NS0ID_HASSUBTYPE), UA_QUALIFIEDNAME(nsIndex, name),
            UA_NODEID_NUMERIC(0, UA_NS0ID_BASEDATAVARIABLETYPE), attr, NULL, NULL);

    if (status == UA_STATUSCODE_GOOD)
    {
        EDGE_LOG(TAG, "+++ addVariableTypeNode success +++\n");
    }
    else
    {
        EDGE_LOG(TAG, "+++ addVariableTypeNode failed +++\n");
    }
    //UA_Variant_deleteMembers(&attr.value);
}

static void addDataTypeNode(UA_Server *server, int nsIndex, EdgeNodeItem *item)
{
    char *name = item->browseName;

    UA_DataTypeAttributes attr = UA_DataTypeAttributes_default;
    attr.description = UA_LOCALIZEDTEXT("en-US", name);
    attr.displayName = UA_LOCALIZEDTEXT("en-US", name);

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

    if (status == UA_STATUSCODE_GOOD)
    {
        EDGE_LOG(TAG, "+++ addDataTypeNode success +++\n");
    }
    else
    {
        EDGE_LOG(TAG, "+++ addDataTypeNode failed +++\n");
    }
}

static void addViewNode(UA_Server *server, int nsIndex, EdgeNodeItem *item)
{
    char *name = item->browseName;

    UA_ViewAttributes attr = UA_ViewAttributes_default;
    attr.description = UA_LOCALIZEDTEXT("en-US", name);
    attr.displayName = UA_LOCALIZEDTEXT("en-US", name);

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

    if (status == UA_STATUSCODE_GOOD)
    {
        EDGE_LOG(TAG, "+++ addViewNode success +++\n");
    }
    else
    {
        EDGE_LOG(TAG, "+++ addViewNode failed +++\n");
    }
}

EdgeResult addReferences(UA_Server *server, EdgeReference *reference, int src_nsIndex, int target_nsIndex)
{

    EdgeResult result;
    result.code = STATUS_OK;

    if (!server)
    {
        EDGE_LOG(TAG, "Server Handle invalid!! \n");
        result.code = STATUS_ERROR;
        return result;
    }

    if (!reference->referenceId)
    {
        reference->referenceId = Organizes;
    }

    UA_ExpandedNodeId expanded_nodeId = UA_EXPANDEDNODEID_STRING(target_nsIndex, reference->targetPath);
    UA_StatusCode status = UA_Server_addReference(server,
            UA_NODEID_STRING(src_nsIndex, reference->sourcePath),
            UA_NODEID_NUMERIC(0, reference->referenceId), expanded_nodeId, reference->forward);
    if (status == UA_STATUSCODE_GOOD)
    {
        EDGE_LOG(TAG, "+++ addReference success +++\n");
    }
    else
    {
        result.code = STATUS_ERROR;
        EDGE_LOG(TAG, "+++ addReference failed +++\n");
    }

    return result;
}

static void addReferenceTypeNode(UA_Server *server, int nsIndex, EdgeNodeItem *item)
{
    char *name = item->browseName;

    UA_ReferenceTypeAttributes attr = UA_ReferenceTypeAttributes_default;
    attr.description = UA_LOCALIZEDTEXT("en-US", name);
    attr.displayName = UA_LOCALIZEDTEXT("en-US", name);

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
    status = UA_Server_addReference(server, UA_NODEID_STRING(nsIndex, item->browseName),
            UA_NODEID_NUMERIC(0, UA_NS0ID_ORGANIZES), expandedSourceNodeId, false);
}

static keyValue getMethodMapElement(edgeMap *map, keyValue key)
{
    edgeMapNode *temp = map->head;
    while (temp != NULL)
    {
        if (!strcmp(temp->key, key))
            return temp->value;
        temp = temp->next;
    }
    return NULL;
}

static UA_StatusCode methodCallback(UA_Server *server, const UA_NodeId *sessionId,
        void *sessionContext, const UA_NodeId *methodId, void *methodContext,
        const UA_NodeId *objectId, void *objectContext, size_t inputSize, const UA_Variant *input,
        size_t outputSize, UA_Variant *output)
{

    char *key = (char *) methodId->identifier.string.data;
    keyValue value = getMethodMapElement(methodNodeMap, (keyValue) key);
    if (value != NULL)
    {
        EdgeMethod *method = (EdgeMethod *) value;
        method_func method_to_call = (method_func) (method->method_fn);

        void **inp = NULL;
        if (inputSize > 0)
        {
            inp = malloc(sizeof(void *) * inputSize);
            VERIFY_NON_NULL(inp, STATUS_ERROR);
            for (int i = 0; i < inputSize; i++)
            {
                inp[i] = input[i].data;
            }
        }
        void **out = NULL;
        if (outputSize > 0)
        {
            out = malloc(sizeof(void *) * outputSize);
            if (IS_NULL(out))
            {
                EDGE_LOG(TAG, "ERROR : out in methodCallback Malloc FAILED\n");
                FREE(inp);
                return STATUS_ERROR;
            }
            for (int i = 0; i < outputSize; i++)
            {
                out[i] = NULL;
            }
        }
        method_to_call(inputSize, inp, outputSize, out);

        FREE(inp);

        for (int idx = 0; idx < method->num_outArgs; idx++)
        {
            if (out[idx] != NULL)
            {
                int type = (int) method->outArg[idx]->argType - 1;
                if (method->outArg[idx]->valType == SCALAR)
                {
                    // Scalar copy
                    if (type == UA_TYPES_STRING)
                    {
                        UA_String val = UA_STRING_ALLOC((char * ) *out);
                        UA_Variant_setScalarCopy(&output[idx], &val, &UA_TYPES[type]);
                        UA_String_deleteMembers(&val);
                        FREE(val.data);
                    }
                    else
                    {
                        UA_Variant *variant = &output[idx];
                        UA_Variant_setScalarCopy(variant, *out, &UA_TYPES[type]);
                    }
                }
                else if (method->outArg[idx]->valType == ARRAY_1D)
                {
                    // Array copy
                    if (type == UA_TYPES_STRING)
                    {
                        char **data = (char **) out;
                        UA_String *array = (UA_String *) UA_Array_new(
                                method->outArg[idx]->arrayLength, &UA_TYPES[type]);
                        for (int idx1 = 0; idx1 < method->outArg[idx]->arrayLength; idx1++)
                        {
                            array[idx1] = UA_STRING_ALLOC(data[idx1]);
                        }
                        UA_Variant *variant = &output[idx];
                        UA_Variant_setArrayCopy(variant, array, method->outArg[idx]->arrayLength,
                                &UA_TYPES[type]);
                        for (int idx1 = 0; idx1 < method->outArg[idx]->arrayLength; idx1++)
                        {
                            UA_String_deleteMembers(&array[idx1]);
                        }
                        UA_Array_delete(array, method->outArg[idx]->arrayLength, &UA_TYPES[type]);
                    }
                    else
                    {
                        UA_Variant *variant = &output[idx];
                        UA_Variant_setArrayCopy(variant, out[idx], method->outArg[idx]->arrayLength,
                                &UA_TYPES[type]);
                    }
                }
                FREE(out[idx]);
            }
        }
        FREE(out);
        return UA_STATUSCODE_GOOD;
    }
    else
    {
        return UA_STATUSCODE_BADMETHODINVALID;
    }
}

/****************************** Member functions ***********************************/

EdgeResult addNodes(UA_Server *server, int nsIndex, EdgeNodeItem *item)
{
    EdgeResult result;
    result.code = STATUS_OK;

    if (item == NULL)
    {
        result.code = STATUS_PARAM_INVALID;
        return result;
    }

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

    return result;
}

EdgeResult addMethodNode(UA_Server *server, int nsIndex, EdgeNodeItem *item, EdgeMethod *method)
{
    EdgeResult result;
    result.code = STATUS_ERROR;

    VERIFY_NON_NULL(server, result);

    int num_inpArgs = method->num_inpArgs;
    int num_outArgs = method->num_outArgs;
    int idx = 0;

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
            UA_UInt32 *inputDimensions = (UA_UInt32 *) malloc(sizeof(UA_UInt32));
            VERIFY_NON_NULL(inputDimensions, result);
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
        outputArguments[idx].dataType = UA_TYPES[(int) method->outArg[idx]->argType - 1].typeId;

        if (method->outArg[idx]->valType == SCALAR)
        {
            outputArguments[idx].valueRank = -1; /* Scalar */
        }
        else if (method->outArg[idx]->valType == ARRAY_1D)
        {
            outputArguments[idx].valueRank = 1; /* Array with one dimensions */
            UA_UInt32 *outputDimensions = (UA_UInt32 *) malloc(sizeof(UA_UInt32));
            if (IS_NULL(outputDimensions))
            {
                EDGE_LOG(TAG, "ERROR : outputDimensions MALLOC failed\n");
                for (idx = 0; idx < num_inpArgs; idx++)
                {
                    if (method->inpArg[idx]->valType == ARRAY_1D)
                    {
                        FREE(inputArguments[idx].arrayDimensions);
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
    methodAttr.description = UA_LOCALIZEDTEXT("en-US", method->methodNodeName);
    methodAttr.displayName = UA_LOCALIZEDTEXT("en-US", method->methodNodeName);
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

        char *browseName = (char *) malloc(strlen(item->browseName) + 1);
        VERIFY_NON_NULL(browseName, result);
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
    if (sourceNode != NULL && sourceNode->nodeId != NULL)
    {
        status = UA_Server_addReference(server, sourceNodeId,
                UA_NODEID_NUMERIC(0, UA_NS0ID_ORGANIZES),
                UA_EXPANDEDNODEID_STRING(nsIndex, item->browseName), true);
        status = UA_Server_addReference(server, UA_NODEID_STRING(nsIndex, item->browseName),
                UA_NODEID_NUMERIC(0, UA_NS0ID_ORGANIZES), expandedSourceNodeId, false);
    }

    for (idx = 0; idx < num_outArgs; idx++)
    {
        if (method->outArg[idx]->valType == ARRAY_1D)
        {
            FREE(outputArguments[idx].arrayDimensions);
        }
    }
    for (idx = 0; idx < num_inpArgs; idx++)
    {

        if (method->inpArg[idx]->valType == ARRAY_1D)
        {
            FREE(inputArguments[idx].arrayDimensions);
        }
    }

    result.code = STATUS_OK;
    return result;
}

EdgeResult addDataAccessNode(EdgeNodeItem *item)
{
    EdgeResult result;
    result.code = STATUS_OK;
    return result;
}

EdgeResult modifyNode(UA_Server *server, int nsIndex, char *nodeUri, EdgeVersatility *value)
{
    EdgeResult result;
    result.code = STATUS_ERROR;

    VERIFY_NON_NULL(server, result);

    // read the value;
    UA_NodeId node = UA_NODEID_STRING(nsIndex, nodeUri);
    UA_Variant *readval = UA_Variant_new();
    VERIFY_NON_NULL(readval, result);

    UA_StatusCode ret = UA_Server_readValue(server, node, readval);
    if (ret != UA_STATUSCODE_GOOD)
    {
        EDGE_LOG(TAG, "error in read value during modify node \n");
        result.code = STATUS_ERROR;
        return result;
    }

    const UA_DataType *type = readval->type;
    UA_Variant *myVariant = UA_Variant_new();
    if (type == &UA_TYPES[UA_TYPES_STRING])
    {
        UA_String val = UA_STRING_ALLOC((char * ) value->value);
        ret = UA_Variant_setScalarCopy(myVariant, &val, type);
        UA_String_deleteMembers(&val);
        FREE(val.data);
    }
    else
    {
        ret = UA_Variant_setScalarCopy(myVariant, value->value, type);
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
        return result;
    }
    else
    {
        EDGE_LOG(TAG, "+++ write successful +++\n\n");
    }
    UA_Variant_delete(myVariant);
    UA_Variant_delete(readval);
    
    result.code = STATUS_OK;
    return result;
}

EdgeResult modifyNode2(EdgeNodeIdentifier nodeType)
{
    EdgeResult result;
    result.code = STATUS_OK;
    return result;
}
/***********************************************************************************/
