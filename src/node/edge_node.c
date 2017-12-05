#include "edge_node.h"

#include <stdio.h>

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

  UA_ExpandedNodeId expanded_nodeId = UA_EXPANDEDNODEID_STRING(1, reference->targetPath);
  UA_StatusCode status = UA_Server_addReference(server, UA_NODEID_STRING(1, reference->sourcePath),
                                                UA_NODEID_NUMERIC(0, reference->referenceId), expanded_nodeId, reference->forward);
  if (status == UA_STATUSCODE_GOOD) {
    printf("\n+++ addReference success +++\n");
  } else {
    printf("\n+++ addReference failed +++\n");
  }

  return NULL;
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

/****************************** Member functions ***********************************/

EdgeResult* addNodes(UA_Server* server, EdgeNodeItem* item) {
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
  }

  return NULL;
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
