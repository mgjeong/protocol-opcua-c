#include "edge_node.h"
#include "edge_utils.h"

#include <stdio.h>

#define MAX_ARGS  10

static edgeMap *methodNodeMap = NULL;
static int methodNodeCount = 0;
//static int numeric_id = 1000;

/****************************** Static functions ***********************************/

static void addVariableNode(UA_Server *server, EdgeNodeItem* item) {
  char* name= item->browseName;
  EdgeNodeIdentifier id = item->variableIdentifier;

  UA_VariableAttributes attr = UA_VariableAttributes_default;
  attr.description = UA_LOCALIZEDTEXT("en-US", name);
  attr.displayName= UA_LOCALIZEDTEXT("en-US", name);
  attr.accessLevel = UA_ACCESSLEVELMASK_READ | UA_ACCESSLEVELMASK_WRITE;
  attr.dataType = UA_TYPES[(int)id - 1].typeId;

  int type = (int)id - 1;
  if (type == UA_TYPES_STRING) {
    UA_String val = UA_STRING_ALLOC((char*) item->variableData);
    UA_Variant_setScalarCopy(&attr.value, &val, &UA_TYPES[type]);
UA_String str = *((UA_String*)attr.value.data);
printf("%s\n\n", (char*)str.data);
  } else {
    UA_Variant_setScalarCopy(&attr.value, item->variableData, &UA_TYPES[type]);
  }

  UA_StatusCode status = UA_Server_addVariableNode(server, UA_NODEID_STRING(1, item->browseName),
                                                   UA_NODEID_NUMERIC(0, UA_NS0ID_OBJECTSFOLDER),
                                                   UA_NODEID_NUMERIC(0, UA_NS0ID_ORGANIZES),
                                                   UA_QUALIFIEDNAME_ALLOC(1, name),
                                                   UA_NODEID_NUMERIC(0, 63), attr, NULL, NULL);
 
  if (status == UA_STATUSCODE_GOOD) {
    printf("\n+++ addVariableNode success +++\n");
  } else {
    printf("\n+++ addVariableNode failed +++\n");
  }
  UA_Variant_deleteMembers(&attr.value);
}

static void addArrayNode(UA_Server *server, EdgeNodeItem *item) {
  char* name= item->browseName;
  EdgeNodeIdentifier id = item->variableIdentifier;

  UA_VariableAttributes attr = UA_VariableAttributes_default;
  attr.description = UA_LOCALIZEDTEXT("en-US", name);
  attr.displayName= UA_LOCALIZEDTEXT("en-US", name);
  attr.accessLevel = UA_ACCESSLEVELMASK_READ | UA_ACCESSLEVELMASK_WRITE;
  attr.dataType = UA_TYPES[(int)id - 1].typeId;

  int type = (int)id - 1;
  if (type == UA_TYPES_STRING) {
    int idx = 0;
    char **data1 = (char**) item->variableData;
//    printf("data :: %s\n", data1[0]);
//    printf("data :: %s\n", data1[1]);

    UA_String *array = (UA_String*) UA_Array_new(item->arrayLength, &UA_TYPES[type]);
    for (idx = 0; idx < item->arrayLength; idx++) {
      array[idx] = UA_STRING_ALLOC(data1[idx]);
    }

    UA_Variant_setArray(&attr.value, array, item->arrayLength, &UA_TYPES[type]);
  } else {
    UA_Variant_setArray(&attr.value, item->variableData, item->arrayLength, &UA_TYPES[type]);
  }

  UA_StatusCode status = UA_Server_addVariableNode(server, UA_NODEID_STRING(1, item->browseName),
                                                   UA_NODEID_NUMERIC(0, UA_NS0ID_OBJECTSFOLDER),
                                                   UA_NODEID_NUMERIC(0, UA_NS0ID_ORGANIZES),
                                                   UA_QUALIFIEDNAME_ALLOC(1, name),
                                                   UA_NODEID_NUMERIC(0, 63), attr, NULL, NULL);

  if (status == UA_STATUSCODE_GOOD) {
    printf("\n+++ addArrayNode success +++\n");
  } else {
    printf("\n+++ addArrayNode failed +++\n");
  }
  UA_Variant_deleteMembers(&attr.value);

}

static void addObjectNode(UA_Server *server, EdgeNodeItem* item) {
  char* name= item->browseName;

  UA_ObjectAttributes object_attr = UA_ObjectAttributes_default;
  object_attr.description = UA_LOCALIZEDTEXT("en-US", name);
  object_attr.displayName= UA_LOCALIZEDTEXT("en-US", name);

  UA_NodeId sourceNodeId;
  char* nodeId = item->sourceNodeId->nodeId;
  if (nodeId == NULL) {
    sourceNodeId = UA_NODEID_NUMERIC(0, UA_NS0ID_OBJECTSFOLDER);
  } else {
    sourceNodeId = UA_NODEID_STRING(1, nodeId);
  }

  UA_StatusCode status = UA_Server_addObjectNode(server, UA_NODEID_STRING(1, item->browseName), sourceNodeId,
                                                    UA_NODEID_NUMERIC(0, UA_NS0ID_ORGANIZES),
                                                    UA_QUALIFIEDNAME_ALLOC(1, name),
                                                    UA_NODEID_NUMERIC(0, UA_NS0ID_FOLDERTYPE), object_attr, NULL, NULL);
  if (status == UA_STATUSCODE_GOOD) {
    printf("\n+++ addObjectNode success +++\n");
  } else {
    printf("\n+++ addObjectNode failed +++\n");
  }
}

static void addObjectTypeNode(UA_Server* server, EdgeNodeItem* item) {
  char* name= item->browseName;
  UA_ObjectTypeAttributes object_attr = UA_ObjectTypeAttributes_default;
  object_attr.description = UA_LOCALIZEDTEXT("en-US", name);
  object_attr.displayName= UA_LOCALIZEDTEXT("en-US", name);

  UA_NodeId sourceNodeId;
  char* nodeId = item->sourceNodeId->nodeId;
  if (nodeId == NULL) {
    sourceNodeId = UA_NODEID_NUMERIC(0, UA_NS0ID_BASEOBJECTTYPE);
  } else {
    sourceNodeId = UA_NODEID_STRING(1, nodeId);
  }

  UA_StatusCode status = UA_Server_addObjectTypeNode(server, UA_NODEID_STRING(1, item->browseName), sourceNodeId,
                                                    UA_NODEID_NUMERIC(0, UA_NS0ID_HASSUBTYPE),
                                                    UA_QUALIFIEDNAME_ALLOC(1, name),
                                                    object_attr, NULL, NULL);
  if (status == UA_STATUSCODE_GOOD) {
    printf("\n+++ addObjectTypeNode success +++\n");
  } else {
    printf("\n+++ addObjectTypeNode failed +++\n");
  }
}

static void addVariableTypeNode(UA_Server* server, EdgeNodeItem* item) {
  char* name= item->browseName;
  EdgeNodeIdentifier id = item->variableIdentifier;

  UA_VariableTypeAttributes attr = UA_VariableTypeAttributes_default;
  attr.description = UA_LOCALIZEDTEXT("en-US", name);
  attr.displayName= UA_LOCALIZEDTEXT("en-US", name);
  attr.dataType = UA_TYPES[(int)id - 1].typeId;

  int type = (int)id - 1;
  if (type == UA_TYPES_STRING) {
    int idx = 0;
    char **data1 = (char**) item->variableData;
//    printf("data :: %s\n", data1[0]);
//    printf("data :: %s\n", data1[1]);

    UA_String *array = (UA_String*) UA_Array_new(item->arrayLength, &UA_TYPES[type]);
    for (idx = 0; idx < item->arrayLength; idx++) {
      array[idx] = UA_STRING_ALLOC(data1[idx]);
    }

    UA_Variant_setArray(&attr.value, array, item->arrayLength, &UA_TYPES[type]);
  } else {
    UA_Variant_setArray(&attr.value, item->variableData, item->arrayLength, &UA_TYPES[type]);
  }

  UA_StatusCode status = UA_Server_addVariableTypeNode(server, UA_NODEID_STRING(1, item->browseName),
                                                   UA_NODEID_NUMERIC(0, UA_NS0ID_BASEDATAVARIABLETYPE),
                                                   UA_NODEID_NUMERIC(0, UA_NS0ID_HASSUBTYPE),
                                                   UA_QUALIFIEDNAME_ALLOC(1, name),
                                                   UA_NODEID_NUMERIC(0, UA_NS0ID_BASEDATAVARIABLETYPE), attr, NULL, NULL);

  if (status == UA_STATUSCODE_GOOD) {
    printf("\n+++ addVariableTypeNode success +++\n");
  } else {
    printf("\n+++ addVariableTypeNode failed +++\n");
  }
  //UA_Variant_deleteMembers(&attr.value);
}

static void addDataTypeNode(UA_Server* server, EdgeNodeItem* item) {
  char* name= item->browseName;

  UA_DataTypeAttributes attr = UA_DataTypeAttributes_default;
  attr.description = UA_LOCALIZEDTEXT("en-US", name);
  attr.displayName= UA_LOCALIZEDTEXT("en-US", name);

  UA_NodeId sourceNodeId;
  char* nodeId = item->sourceNodeId->nodeId;
  if (nodeId == NULL) {
    sourceNodeId = UA_NODEID_NUMERIC(0, UA_NS0ID_BASEDATATYPE);
  } else {
    sourceNodeId = UA_NODEID_STRING(1, nodeId);
  }
  UA_StatusCode status = UA_Server_addDataTypeNode(server, UA_NODEID_STRING(1, item->browseName),
                                                   sourceNodeId,
                                                   UA_NODEID_NUMERIC(0, UA_NS0ID_HASSUBTYPE),
                                                   UA_QUALIFIEDNAME_ALLOC(1, name),
                                                   attr, NULL, NULL);

  if (status == UA_STATUSCODE_GOOD) {
    printf("\n+++ addDataTypeNode success +++\n");
  } else {
    printf("\n+++ addDataTypeNode failed +++\n");
  }
}

static void addViewNode(UA_Server *server, EdgeNodeItem* item) {
  char *name = item->browseName;

  UA_ViewAttributes attr = UA_ViewAttributes_default;
  attr.description = UA_LOCALIZEDTEXT("en-US", name);
  attr.displayName= UA_LOCALIZEDTEXT("en-US", name);

  UA_NodeId sourceNodeId;
  char* nodeId = item->sourceNodeId->nodeId;
  if (nodeId == NULL) {
    sourceNodeId = UA_NODEID_NUMERIC(0, UA_NS0ID_VIEWSFOLDER);
  } else {
    sourceNodeId = UA_NODEID_STRING(1, nodeId);
  }
  UA_StatusCode status = UA_Server_addViewNode(server, UA_NODEID_STRING(1, item->browseName),
                                                   sourceNodeId,
                                                   UA_NODEID_NUMERIC(0, UA_NS0ID_ORGANIZES),
                                                   UA_QUALIFIEDNAME_ALLOC(1, name),
                                                   attr, NULL, NULL);

  if (status == UA_STATUSCODE_GOOD) {
    printf("\n+++ addViewNode success +++\n");
  } else {
    printf("\n+++ addViewNode failed +++\n");
  }
}

EdgeResult* addReferences(UA_Server* server, EdgeReference* reference) {

    EdgeResult* result = (EdgeResult*) malloc(sizeof(EdgeResult));
    result->code = STATUS_OK;

  UA_ExpandedNodeId expanded_nodeId = UA_EXPANDEDNODEID_STRING(1, reference->targetPath);
  UA_StatusCode status = UA_Server_addReference(server, UA_NODEID_STRING(1, reference->sourcePath),
                                                UA_NODEID_NUMERIC(0, reference->referenceId), expanded_nodeId, reference->forward);
  if (status == UA_STATUSCODE_GOOD) {
    printf("\n+++ addReference success +++\n");
  } else {
    result->code = STATUS_ERROR;
    printf("\n+++ addReference failed +++\n");
  }

  return result;
}

static void addReferenceTypeNode(UA_Server* server, EdgeNodeItem* item) {
  char *name = item->browseName;

  UA_ReferenceTypeAttributes attr = UA_ReferenceTypeAttributes_default;
  attr.description = UA_LOCALIZEDTEXT("en-US", name);
  attr.displayName= UA_LOCALIZEDTEXT("en-US", name);

  UA_NodeId sourceNodeId;
  char* nodeId = item->sourceNodeId->nodeId;
  if (nodeId == NULL) {
    sourceNodeId = UA_NODEID_NUMERIC(0, UA_NS0ID_ORGANIZES);
  } else {
    sourceNodeId = UA_NODEID_STRING(1, nodeId);
  }
  UA_StatusCode status = UA_Server_addReferenceTypeNode(server, UA_NODEID_STRING(1, item->browseName),
                                                   sourceNodeId,
                                                   UA_NODEID_NUMERIC(0, UA_NS0ID_HASSUBTYPE),
                                                   UA_QUALIFIEDNAME_ALLOC(1, name),
                                                   attr, NULL, NULL);

  if (status == UA_STATUSCODE_GOOD) {
    printf("\n+++ addReferenceTypeNode success +++\n");
  } else {
    printf("\n+++ addReferenceTypeNode failed +++\n");
  }
}

static keyValue getMethodMapElement(edgeMap *map, keyValue key) {
  edgeMapNode *temp = map->head;
  while(temp != NULL) {
    if(!strcmp(temp->key, key))
      return temp->value;
    temp = temp->next;
  }
  return NULL;
}

static UA_StatusCode methodCallback(UA_Server *server,
                                    const UA_NodeId *sessionId, void *sessionContext,
                                    const UA_NodeId *methodId, void *methodContext,
                                    const UA_NodeId *objectId, void *objectContext,
                                    size_t inputSize, const UA_Variant *input,
                                    size_t outputSize, UA_Variant *output) {

  char *key = (char*) methodId->identifier.string.data;
  keyValue value = getMethodMapElement(methodNodeMap, (keyValue) key);
  if (value != NULL) {
    EdgeMethod *method = (EdgeMethod*) value;
    method_func method_to_call = (method_func) (method->method_fn);

    void **inp = malloc(sizeof(void*) * inputSize);
    for (int i = 0; i < inputSize; i++) {
      inp[i] = input[i].data;
    }
    void **out = malloc(sizeof(void*) * outputSize);
    method_to_call(inputSize, inp, outputSize, out);

    free (inp); inp = NULL;

    for (int idx = 0; idx < method->num_outArgs; idx++) {
      int type = (int) method->outArg[idx]->argType - 1;

      if (method->outArg[idx]->valType == SCALAR) {
        // Scalar copy
        if (type == UA_TYPES_STRING) {
          UA_String val = UA_STRING_ALLOC((char*) *out);
          UA_Variant_setScalarCopy(&output[idx], &val, &UA_TYPES[type]);
        } else {
          UA_Variant *variant = &output[idx];
          UA_Variant_setScalarCopy(variant, *out, &UA_TYPES[type]);
        }
      } else if (method->outArg[idx]->valType == ARRAY_1D) {
        // Array copy
        if (type == UA_TYPES_STRING) {
          char **data = (char**) out;
          UA_String *array = (UA_String*) UA_Array_new(method->outArg[idx]->arrayLength, &UA_TYPES[type]);
          for (int idx1 = 0; idx1 <method->outArg[idx]->arrayLength; idx++) {
            array[idx] = UA_STRING_ALLOC(data[idx]);
          }
          UA_Variant *variant = &output[idx];          
          UA_Variant_setArrayCopy(variant, array, method->outArg[idx]->arrayLength, &UA_TYPES[type]);
        } else {
          UA_Variant *variant = &output[idx];
          UA_Variant_setArrayCopy(variant, out[idx], method->outArg[idx]->arrayLength, &UA_TYPES[type]);
        }
      }
      free (out[idx]);
      out[idx] = NULL;
    }
    return UA_STATUSCODE_GOOD;
  } else {
    return UA_STATUSCODE_BADMETHODINVALID;
  }
}

/****************************** Member functions ***********************************/

EdgeResult* addNodes(UA_Server* server, EdgeNodeItem* item) {

  EdgeResult* result = (EdgeResult*) malloc(sizeof(EdgeResult));
  result->code = STATUS_OK;

    if(item == NULL)
    {
        result->code = STATUS_PARAM_INVALID;
        return result;
    }
      

  if (item->nodeType == VARIABLE_NODE) {
    addVariableNode(server, item);
  } else if (item->nodeType == ARRAY_NODE) {
    addArrayNode(server, item);
  } else if (item->nodeType == OBJECT_NODE) {
    addObjectNode(server, item);
  } else if (item->nodeType == OBJECT_TYPE_NODE) {
    addObjectTypeNode(server, item);
  } else if (item->nodeType == VARIABLE_TYPE_NODE) {
    addVariableTypeNode(server, item);
  } else if (item->nodeType == DATA_TYPE_NODE) {
    addDataTypeNode(server, item);
  } else if (item->nodeType == VIEW_NODE) {
    addViewNode(server, item);
  } else if (item->nodeType == REFERENCE_TYPE_NODE) {
    addReferenceTypeNode(server, item);
  } else {
    result->code = STATUS_ERROR;
  }

  return result;
}

EdgeResult* addMethodNode(UA_Server *server, EdgeNodeItem *item, EdgeMethod *method) {
  EdgeResult* result = (EdgeResult*) malloc(sizeof(EdgeResult));
  if (!server) {
    printf("Server handle Invalid\n");
    result->code = STATUS_ERROR;
    return result;
  }

  int num_inpArgs = method->num_inpArgs;
  int num_outArgs = method->num_outArgs;
  int idx = 0;
\
  /* Input Arguments */
  UA_Argument inputArguments[MAX_ARGS];
  for (idx = 0; idx < num_inpArgs; idx++) {
    UA_Argument_init(&inputArguments[idx]);
    inputArguments[idx].description = UA_LOCALIZEDTEXT("en-US", method->description);
    inputArguments[idx].name = UA_STRING(method->description);
    inputArguments[idx].dataType = UA_TYPES[(int)method->inpArg[idx]->argType - 1].typeId;

    if (method->inpArg[idx]->valType == SCALAR) {
      inputArguments[idx].valueRank = -1;    /* Scalar */
    } else if (method->inpArg[idx]->valType == ARRAY_1D) {
      inputArguments[idx].valueRank = 1;    /* Array with one dimensions */      
      UA_UInt32 inputDimensions = method->inpArg[idx]->arrayLength;
      inputArguments[idx].arrayDimensionsSize = 1;
      inputArguments[idx].arrayDimensions =&inputDimensions;
    }
  }

  /* Output Arguments */
  UA_Argument outputArguments[MAX_ARGS];
  for (idx = 0; idx < num_outArgs; idx++) {
    UA_Argument_init(&outputArguments[idx]);
    outputArguments[idx].description = UA_LOCALIZEDTEXT("en-US", method->description);
    outputArguments[idx].name = UA_STRING(method->description);
    outputArguments[idx].dataType = UA_TYPES[(int)method->outArg[idx]->argType - 1].typeId;

    if (method->outArg[idx]->valType == SCALAR) {
      outputArguments[idx].valueRank = -1;    /* Scalar */
    } else if (method->outArg[idx]->valType == ARRAY_1D) {
      outputArguments[idx].valueRank = 1;    /* Array with one dimensions */
      UA_UInt32 outputDimensions = method->outArg[idx]->arrayLength;
      outputArguments[idx].arrayDimensionsSize = 1;
      outputArguments[idx].arrayDimensions =&outputDimensions;
    }
  }

  UA_MethodAttributes methodAttr = UA_MethodAttributes_default;
  methodAttr.description = UA_LOCALIZEDTEXT("en-US", method->methodNodeName);
  methodAttr.displayName = UA_LOCALIZEDTEXT("en-US", method->methodNodeName);
  methodAttr.executable = true;
  methodAttr.userExecutable = true;

  UA_NodeId sourceNodeId;
  EdgeNodeId* sourceNode = item->sourceNodeId;
  if (sourceNode == NULL || sourceNode->nodeId == NULL) {
    sourceNodeId = UA_NODEID_NUMERIC(0, UA_NS0ID_OBJECTSFOLDER);
  } else {
    sourceNodeId = UA_NODEID_STRING(1, sourceNode->nodeId);
  }

  UA_StatusCode status = UA_Server_addMethodNode(server, UA_NODEID_STRING(1, item->browseName),
                          sourceNodeId, UA_NODEID_NUMERIC(0, UA_NS0ID_HASCOMPONENT),
                          UA_QUALIFIEDNAME(1, item->browseName), methodAttr, &methodCallback,
                          num_inpArgs, inputArguments, num_outArgs, outputArguments,
                          NULL, NULL);
  if (status == UA_STATUSCODE_GOOD) {
    printf("\n+++ addMethodNode success +++\n");
    if (NULL == methodNodeMap)
      methodNodeMap = createMap();

    char *browseName = (char*) malloc(strlen(item->browseName) + 1);
    strncpy(browseName, item->browseName, strlen(item->browseName));
    browseName[strlen(item->browseName)] = '\0';
    insertMapElement(methodNodeMap, (void*) browseName, method);
    methodNodeCount += 1;
  } else {
    printf("\n+++ addMethodNode failed +++\n");
  }

  result->code = STATUS_OK;
  return result;
}

EdgeResult* addDataAccessNode(EdgeNodeItem* item) {
  return NULL;
}

EdgeResult* modifyNode(char* nodeUri) {
  return NULL;
}

EdgeResult* modifyNode2(EdgeNodeIdentifier nodeType) {
  return NULL;
}

/***********************************************************************************/
