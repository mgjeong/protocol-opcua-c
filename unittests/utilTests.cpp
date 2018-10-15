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

#include <gtest/gtest.h>
#include <iostream>

extern "C"
{
#include "opcua_manager.h"
#include "opcua_common.h"
#include "edge_identifier.h"
#include "edge_malloc.h"
#include "edge_utils.h"
#include "edge_open62541.h"
#include "edge_list.h"
#include "edge_map.h"
#include "uqueue.h"
#include "uarraylist.h"
#include "test_common.h"
}

#define PRINT(str) std::cout<<str<<std::endl

edgeMap *sampleMap;

class OPC_utilMap: public ::testing::Test
{
protected:

    virtual void SetUp()
    {
        PRINT("MAP TESTS");
        sampleMap = NULL;
    }

    virtual void TearDown()
    {

    }

};

class OPC_util: public ::testing::Test
{
protected:

    virtual void SetUp()
    {
        PRINT("UTIL TESTS");
    }

    virtual void TearDown()
    {

    }

};

//-----------------------------------------------------------------------------
//  Tests
//-----------------------------------------------------------------------------

TEST_F(OPC_utilMap , createMap_P)
{
    EXPECT_EQ(sampleMap == NULL, true);

    sampleMap = createMap();

    EXPECT_EQ(sampleMap == NULL, false);
}

TEST_F(OPC_utilMap , insertMapElement_P)
{
    sampleMap = createMap();

    insertMapElement(sampleMap, (keyValue) "key1", (keyValue) "value1");
    EXPECT_EQ(sampleMap->head == NULL, false);
    insertMapElement(sampleMap, (keyValue) "key2", (keyValue) "value2");
    insertMapElement(sampleMap, (keyValue) "key6", (keyValue) "value6");
    insertMapElement(sampleMap, (keyValue) "key3", (keyValue) "value3");

    EXPECT_EQ((char * )getMapElement(sampleMap, (keyValue ) "key1"), "value1");
    EXPECT_EQ((char * )getMapElement(sampleMap, (keyValue ) "key2"), "value2");
    EXPECT_EQ((char * )getMapElement(sampleMap, (keyValue ) "key3"), "value3");
    EXPECT_EQ((char * )getMapElement(sampleMap, (keyValue ) "key6"), "value6");

    deleteMap(sampleMap);

    EXPECT_EQ(sampleMap->head == NULL, true);
}

TEST_F(OPC_utilMap , insertMapElement_N)
{
    sampleMap = createMap();

    EXPECT_NE((char * )getMapElement(sampleMap, (keyValue ) "key1"), "value1");
    EXPECT_EQ(getMapElement(sampleMap, (keyValue ) "key1") == NULL, true);

    insertMapElement(sampleMap, (keyValue) "key1", (keyValue) "value1");
    EXPECT_EQ(sampleMap->head == NULL, false);
    insertMapElement(sampleMap, (keyValue) "key2", (keyValue) "value2");
    insertMapElement(sampleMap, (keyValue) "key6", (keyValue) "value6");
    insertMapElement(sampleMap, (keyValue) "key3", (keyValue) "value3");

    EXPECT_NE((char * )getMapElement(sampleMap, (keyValue ) "key4"), "value4");
    EXPECT_NE((char * )getMapElement(sampleMap, (keyValue ) "key1"), "value2");
    EXPECT_NE((char * )getMapElement(sampleMap, (keyValue ) "key2"), "value3");
    EXPECT_EQ((char * )getMapElement(sampleMap, (keyValue ) "key6"), "value6");

    deleteMap(sampleMap);

    EXPECT_NE((char * )getMapElement(sampleMap, (keyValue ) "key1"), "value1");
    EXPECT_NE((char * )getMapElement(sampleMap, (keyValue ) "key2"), "value2");
    EXPECT_NE((char * )getMapElement(sampleMap, (keyValue ) "key3"), "value3");
    EXPECT_NE((char * )getMapElement(sampleMap, (keyValue ) "key6"), "value6");

    EXPECT_EQ(sampleMap->head == NULL, true);
}

TEST_F(OPC_utilMap , deleteMap_P)
{
    sampleMap = createMap();

    EXPECT_EQ(sampleMap == NULL, false);

    deleteMap(sampleMap);

    EXPECT_EQ(sampleMap->head == NULL, true);
}

TEST_F(OPC_util , cloneString_P)
{
    char *retStr = NULL;

    EXPECT_EQ(retStr == NULL, true);

    retStr = cloneString(WELL_KNOWN_DISCOVERY_VALUE);
    EXPECT_NE(retStr == NULL, true);

    EXPECT_EQ(strcmp(retStr, WELL_KNOWN_DISCOVERY_VALUE), 0);

    free(retStr);
    retStr = NULL;
    EXPECT_EQ(retStr == NULL, true);
}

TEST_F(OPC_util , cloneString_N)
{
    char *retStr = NULL;
    EXPECT_EQ(retStr == NULL, true);

    retStr = cloneString(NULL);
    ASSERT_EQ(retStr == NULL, true);
}

TEST_F(OPC_util , cloneData_DataNull)
{
    void *retVal = cloneData(NULL, 10);
    ASSERT_EQ(retVal == NULL, true);
}

TEST_F(OPC_util , cloneData_ZeroLength)
{
    void *retVal = cloneData(WELL_KNOWN_DISCOVERY_VALUE, 0);
    ASSERT_EQ(retVal == NULL, true);
}

TEST_F(OPC_util , addListNode_HeadNull)
{
    int dummyData = 10;
    void *data = (void *) &dummyData;
    ASSERT_EQ(addListNode(NULL, data), false);
}

TEST_F(OPC_util , addListNode_DataNull)
{
    List list;
    List *head = &list;
    ASSERT_EQ(addListNode(&head, NULL), false);
}

TEST_F(OPC_util , getListSize_NullListPointer)
{
    ASSERT_EQ(getListSize(NULL), 0);
}

TEST_F(OPC_util , freeEdgeResult_P)
{
    int dummy = 1;
    EdgeResult *res = (EdgeResult *) EdgeCalloc(1, sizeof(EdgeResult));
    freeEdgeResult(res);

    // Control should come here. If it comes here, then there is no problem with freeEdgeResult().
    ASSERT_EQ(dummy==1, true);
}

TEST_F(OPC_util , freeEdgeVersatility_P)
{
    int dummy = 1;
    EdgeVersatility *versatileValue = (EdgeVersatility *) EdgeCalloc(1, sizeof(EdgeVersatility));
    ASSERT_EQ(versatileValue  != NULL, true);
    versatileValue->value = malloc(1);
    freeEdgeVersatility(versatileValue);

    // Control should come here. If it comes here, then there is no problem with freeEdgeVersatility().
    ASSERT_EQ(dummy==1, true);
}

TEST_F(OPC_util , getEdgeNodeIdType_P)
{
    ASSERT_EQ(getEdgeNodeIdType('N'), EDGE_INTEGER);
    ASSERT_EQ(getEdgeNodeIdType('S'), EDGE_STRING);
    ASSERT_EQ(getEdgeNodeIdType('B'), EDGE_BYTESTRING);
    ASSERT_EQ(getEdgeNodeIdType('G'), EDGE_UUID);
    ASSERT_EQ(getEdgeNodeIdType('X'), EDGE_INTEGER); // Random invalid value.
}

TEST_F(OPC_util , getCharacterNodeIdType_P)
{
    ASSERT_EQ(getCharacterNodeIdType(UA_NODEIDTYPE_NUMERIC), 'N');
    ASSERT_EQ(getCharacterNodeIdType(UA_NODEIDTYPE_STRING), 'S');
    ASSERT_EQ(getCharacterNodeIdType(UA_NODEIDTYPE_BYTESTRING), 'B');
    ASSERT_EQ(getCharacterNodeIdType(UA_NODEIDTYPE_GUID), 'G');
    ASSERT_EQ(getCharacterNodeIdType(21165), '\0'); // Random invalid value.
}

TEST_F(OPC_util , get_size_P)
{
    ASSERT_EQ(get_size(EDGE_NODEID_BOOLEAN, false) != -1, true);
    ASSERT_EQ(get_size(EDGE_NODEID_SBYTE, false) != -1, true);
    ASSERT_EQ(get_size(EDGE_NODEID_BYTE, false) != -1, true);
    ASSERT_EQ(get_size(EDGE_NODEID_INT16, false) != -1, true);
    ASSERT_EQ(get_size(EDGE_NODEID_UINT16, false) != -1, true);
    ASSERT_EQ(get_size(EDGE_NODEID_INT32, false) != -1, true);
    ASSERT_EQ(get_size(EDGE_NODEID_UINT32, false) != -1, true);
    ASSERT_EQ(get_size(EDGE_NODEID_INT64, false) != -1, true);
    ASSERT_EQ(get_size(EDGE_NODEID_UINT64, false) != -1, true);
    ASSERT_EQ(get_size(EDGE_NODEID_FLOAT, false) != -1, true);
    ASSERT_EQ(get_size(EDGE_NODEID_DOUBLE, false) != -1, true);
    ASSERT_EQ(get_size(EDGE_NODEID_STRING, false) != -1, true);
}

TEST_F(OPC_util , cloneEdgeEndpoint_P)
{
    EdgeEndPointInfo *retEndpoint = NULL;

    EdgeEndpointConfig *endpointConfig = (EdgeEndpointConfig *) EdgeCalloc(1, sizeof(EdgeEndpointConfig));
    endpointConfig->bindAddress = "100.100.100.100";
    endpointConfig->bindPort = 12686;
    endpointConfig->serverName = (char *) DEFAULT_SERVER_NAME_VALUE;

    EXPECT_EQ(endpointConfig  != NULL, true);

    EdgeApplicationConfig *appConfig = (EdgeApplicationConfig *) EdgeCalloc(1, sizeof(EdgeApplicationConfig));
    ASSERT_EQ(appConfig  != NULL, true);
    appConfig->applicationName = copyString(DEFAULT_SERVER_APP_NAME_VALUE);
    appConfig->applicationUri = copyString(DEFAULT_SERVER_URI_VALUE);
    appConfig->productUri = copyString(DEFAULT_PRODUCT_URI_VALUE);
    appConfig->gatewayServerUri = copyString(DEFAULT_SERVER_URI_VALUE);
    appConfig->discoveryProfileUri  = copyString(DEFAULT_SERVER_URI_VALUE);

    char *discoveryUrls[1] = {copyString(DEFAULT_SERVER_URI_VALUE)};
    appConfig->discoveryUrlsSize = 1;
    appConfig->discoveryUrls = discoveryUrls;

    EdgeEndPointInfo *ep = (EdgeEndPointInfo *) EdgeMalloc(sizeof(EdgeEndPointInfo));
    ep->endpointUri = "opc.tcp://107.108.81.116:12686/edge-opc-server";
    ep->endpointConfig = endpointConfig;
    ep->appConfig = appConfig;
    ep->securityPolicyUri = NULL;
    ep->transportProfileUri = NULL;

    EXPECT_EQ(ep  != NULL, true);

    EXPECT_EQ(endpointConfig != NULL, true);
    EXPECT_EQ(appConfig != NULL, true);
    EXPECT_EQ(ep != NULL, true);
    EXPECT_EQ(retEndpoint == NULL, true);

    retEndpoint = cloneEdgeEndpointInfo(ep);

    EXPECT_EQ(retEndpoint != NULL, true);
    EXPECT_EQ(strcmp(retEndpoint->endpointUri, ep->endpointUri), 0);
    EXPECT_EQ(strcmp(retEndpoint->endpointConfig->bindAddress, endpointConfig->bindAddress), 0);
    EXPECT_EQ(strcmp(retEndpoint->endpointConfig->bindAddress, endpointConfig->bindAddress), 0);
    EXPECT_EQ(strcmp(retEndpoint->appConfig->applicationUri, appConfig->applicationUri), 0);
    EXPECT_EQ(strcmp(retEndpoint->appConfig->productUri, appConfig->productUri), 0);
    EXPECT_EQ(strcmp(retEndpoint->endpointConfig->serverName, endpointConfig->serverName), 0);
    EXPECT_EQ(endpointConfig->bindPort, retEndpoint->endpointConfig->bindPort);

    freeEdgeEndpointInfo(retEndpoint);
    retEndpoint = NULL;
    free(endpointConfig);
    endpointConfig = NULL;
    free(appConfig);
    appConfig = NULL;
    free(ep);
    ep = NULL;

    EXPECT_EQ(endpointConfig == NULL, true);
    EXPECT_EQ(appConfig == NULL, true);
    EXPECT_EQ(ep == NULL, true);
    EXPECT_EQ(retEndpoint == NULL, true);

}

TEST_F(OPC_util , cloneNode_P)
{
    EdgeNodeInfo *nodeInfo = (EdgeNodeInfo *) EdgeCalloc(1, sizeof(EdgeNodeInfo));
    nodeInfo->nodeId = (EdgeNodeId *) EdgeCalloc(1, sizeof(EdgeNodeId));
    char* nodeName = "String";
    nodeInfo->valueAlias = (char *) EdgeMalloc(strlen(nodeName) + 1);
    strcpy(nodeInfo->valueAlias, nodeName);
    nodeInfo->valueAlias[strlen(nodeName)] = '\0';
    nodeInfo->methodName = "methodName";
    nodeInfo->nodeId->type = EDGE_INTEGER;
    nodeInfo->nodeId->integerNodeId = EDGE_NODEID_ROOTFOLDER;
    nodeInfo->nodeId->nameSpace = SYSTEM_NAMESPACE_INDEX;

    EdgeNodeInfo *retNodeInfo = NULL;

    EXPECT_EQ(nodeInfo != NULL, true);
    EXPECT_EQ(retNodeInfo == NULL, true);

    retNodeInfo = cloneEdgeNodeInfo(nodeInfo);

    EXPECT_EQ(retNodeInfo != NULL, true);

    EXPECT_EQ(strcmp(retNodeInfo->valueAlias, nodeInfo->valueAlias), 0);
    EXPECT_EQ(strcmp(retNodeInfo->methodName, nodeInfo->methodName), 0);

    EXPECT_EQ(nodeInfo->nodeId->type, retNodeInfo->nodeId->type);
    EXPECT_EQ(nodeInfo->nodeId->integerNodeId, retNodeInfo->nodeId->integerNodeId);
    EXPECT_EQ(nodeInfo->nodeId->nameSpace, retNodeInfo->nodeId->nameSpace);

    free(nodeInfo->valueAlias);
    nodeInfo->valueAlias = NULL;
    free(nodeInfo);
    nodeInfo = NULL;
    freeEdgeNodeInfo(retNodeInfo);
    retNodeInfo = NULL;

    EXPECT_EQ(nodeInfo == NULL, true);
    EXPECT_EQ(retNodeInfo == NULL, true);
}

TEST_F(OPC_util , convertUAStringToString_N)
{
    char *retStr = NULL;
    EXPECT_EQ(retStr == NULL, true);

    retStr = convertUAStringToString(NULL);
    ASSERT_EQ(retStr == NULL, true);
}

TEST_F(OPC_util , edgeMalloc_P)
{
    int *ptr = (int*) EdgeMalloc(sizeof(int) * 5);
    ASSERT_EQ(NULL != ptr, true);
    free(ptr);
}

TEST_F(OPC_util , edgeMalloc_N)
{
    int *ptr = (int*) EdgeMalloc(0);
    ASSERT_EQ(NULL, ptr);
}

TEST_F(OPC_util , edgeCalloc_P)
{
    int *ptr = (int*) EdgeCalloc(5, sizeof(int));
    ASSERT_EQ(NULL != ptr, true);
    free(ptr);
}

TEST_F(OPC_util , edgeCalloc_N1)
{
    int *ptr = (int*) EdgeCalloc(0, sizeof(int));
    ASSERT_EQ(NULL, ptr);
}

TEST_F(OPC_util , edgeCalloc_N2)
{
    int *ptr = (int*) EdgeCalloc(5, 0);
    ASSERT_EQ(NULL, ptr);
}

TEST_F(OPC_util , edgeRealloc_P1)
{
    int *ptr = (int*) EdgeRealloc(NULL, sizeof(int) * 5);
    ASSERT_EQ(NULL != ptr, true);
    free(ptr);
}

TEST_F(OPC_util , edgeRealloc_P2)
{
    int *ptr = (int*) EdgeMalloc(sizeof(int) * 5);
    ASSERT_EQ(NULL != ptr, true);

    ptr = (int*) EdgeRealloc((void *) ptr, sizeof(int) * 10);
    ASSERT_EQ(NULL != ptr, true);

    free(ptr);
}

TEST_F(OPC_util , edgeStringAlloc_P1)
{
    Edge_String str = EdgeStringAlloc("COUNTRY");
    ASSERT_EQ(str.data != NULL, true);
    EdgeFree(str.data);
}

TEST_F(OPC_util , edgeStringAlloc_P2)
{
    Edge_String str = EdgeStringAlloc("");
    ASSERT_EQ(str.data == EDGE_EMPTY_ARRAY_SENTINEL, true);
}

TEST_F(OPC_util , createQueue_P)
{
    u_queue_t *queue = u_queue_create();
    ASSERT_EQ(queue != NULL, true);

    EdgeFree(queue);
}

TEST_F(OPC_util , getListSize_P)
{
    List *head = NULL;
    int dummyData = 10;
    ASSERT_TRUE(addListNode(&head, &dummyData));
    ASSERT_EQ(getListSize(head), 1);
    EdgeFree(head);
}

TEST_F(OPC_util , getListSize_N)
{
    ASSERT_EQ(getListSize(NULL), 0);
}

TEST_F(OPC_util , convertToEdgeApplicationType_P)
{
    ASSERT_EQ(convertToEdgeApplicationType(UA_APPLICATIONTYPE_SERVER), EDGE_APPLICATIONTYPE_SERVER);
    ASSERT_EQ(convertToEdgeApplicationType(UA_APPLICATIONTYPE_CLIENT), EDGE_APPLICATIONTYPE_CLIENT);
    ASSERT_EQ(convertToEdgeApplicationType(UA_APPLICATIONTYPE_CLIENTANDSERVER), EDGE_APPLICATIONTYPE_CLIENTANDSERVER);
    ASSERT_EQ(convertToEdgeApplicationType(UA_APPLICATIONTYPE_DISCOVERYSERVER), EDGE_APPLICATIONTYPE_DISCOVERYSERVER);
}

TEST_F(OPC_util , getEdgeNodeIdByteString_P)
{
    uint16_t namespaceIdx = 0;
    const char *str = "Node1";
    UA_NodeId node = UA_NODEID_BYTESTRING_ALLOC(namespaceIdx, str);

    EdgeNodeId *edgeNode = getEdgeNodeId(&node);
    ASSERT_TRUE(edgeNode != NULL);
    ASSERT_TRUE(edgeNode->nameSpace == namespaceIdx);
    ASSERT_TRUE(edgeNode->type == EDGE_BYTESTRING);
    ASSERT_TRUE(edgeNode->nodeId != NULL);
    ASSERT_TRUE(strcmp(edgeNode->nodeId, str) == 0);

    freeEdgeNodeId(edgeNode);
    EdgeFree(node.identifier.byteString.data);
}

TEST_F(OPC_util , getEdgeNodeIdGuid_P)
{
    uint16_t namespaceIdx = 0;
    UA_Guid guid = { 1, 0, 1, { 0, 0, 0, 0, 1, 1, 1, 1 } };
    const char *str = "00000001-0000-0001-0000-000001010101";
    UA_NodeId node = UA_NODEID_GUID(namespaceIdx, guid);

    EdgeNodeId *edgeNode = getEdgeNodeId(&node);
    ASSERT_TRUE(edgeNode != NULL);
    ASSERT_TRUE(edgeNode->nameSpace == namespaceIdx);
    ASSERT_TRUE(edgeNode->type == EDGE_UUID);
    ASSERT_TRUE(edgeNode->nodeId != NULL);
    ASSERT_TRUE(strcmp(edgeNode->nodeId, str) == 0);

    freeEdgeNodeId(edgeNode);
}

TEST_F(OPC_util , cloneNodeIdByteString_P)
{
    uint16_t namespaceIdx = 0;
    const char *str = "Node1";
    UA_NodeId node = UA_NODEID_BYTESTRING_ALLOC(namespaceIdx, str);
    UA_NodeId *clone = cloneNodeId(&node);

    ASSERT_TRUE(clone != NULL);
    ASSERT_TRUE(clone->namespaceIndex == namespaceIdx);
    ASSERT_TRUE(clone->identifierType == UA_NODEIDTYPE_BYTESTRING);
    ASSERT_TRUE(clone->identifier.byteString.data != NULL);
    ASSERT_TRUE(strncmp((const char *)clone->identifier.byteString.data, str, strlen(str)) == 0);

    UA_NodeId_delete(clone);
    EdgeFree(node.identifier.byteString.data);
}

TEST_F(OPC_util , cloneNodeIdGuid_P)
{
    uint16_t namespaceIdx = 0;
    UA_Guid guid = { 1, 0, 1, { 0, 0, 0, 0, 1, 1, 1, 1 } };
    UA_NodeId node = UA_NODEID_GUID(namespaceIdx, guid);
    UA_NodeId *clone = cloneNodeId(&node);

    ASSERT_TRUE(clone != NULL);
    ASSERT_TRUE(clone->namespaceIndex == namespaceIdx);
    ASSERT_TRUE(clone->identifierType == UA_NODEIDTYPE_GUID);
    ASSERT_TRUE(UA_Guid_equal(&clone->identifier.guid, &guid));

    UA_NodeId_delete(clone);
}

// uarraylist.c - Adding unit tests for missed out cases.
TEST_F(OPC_util , u_arraylist_add_N)
{
    int dummyData = 100;
    ASSERT_FALSE(u_arraylist_add(NULL, &dummyData));
}

TEST_F(OPC_util , u_arraylist_length_N)
{
    ASSERT_EQ(u_arraylist_length(NULL), 0);
}

TEST_F(OPC_util , u_arraylist_contains_N)
{
    ASSERT_FALSE(u_arraylist_contains(NULL, NULL));
}

TEST_F(OPC_util , u_arraylist_reserve_P)
{
    u_arraylist_t  *list = u_arraylist_create(); // List's initial capacity is 1
    bool ret = u_arraylist_reserve(list, 2); // Increasing the capacity.
    ASSERT_TRUE(ret);
    u_arraylist_free(&list);
}

TEST_F(OPC_util , u_arraylist_shrink_to_fit_P)
{
    u_arraylist_t  *list = u_arraylist_create(); // List's initial capacity is 1
    ASSERT_TRUE(u_arraylist_reserve(list, 2)); // Increasing the capacity.

    int dummyData = 100;
    ASSERT_TRUE(u_arraylist_add(list, &dummyData)); // Adding an item to increase the length
    ASSERT_TRUE(u_arraylist_length(list) == 1);
    u_arraylist_shrink_to_fit(NULL); // No action
    u_arraylist_shrink_to_fit(list); // Decreases the capacity by 1.
    ASSERT_TRUE(u_arraylist_length(list) == 1);
    ASSERT_TRUE(u_arraylist_remove(list, 0) != NULL);

    u_arraylist_free(&list);
}

TEST_F(OPC_util , u_arraylist_get_N)
{
    u_arraylist_t  *list = u_arraylist_create();
    ASSERT_TRUE(u_arraylist_get(NULL, 0) == NULL);
    ASSERT_TRUE(u_arraylist_get(list, 0) == NULL);
    u_arraylist_free(&list);
}

TEST_F(OPC_util , u_arraylist_get_index_P)
{
    u_arraylist_t  *list = u_arraylist_create(); // List's initial capacity is 1

    int dummyData = 100;
    ASSERT_TRUE(u_arraylist_add(list, &dummyData)); // Adding an item to increase the length
    ASSERT_TRUE(u_arraylist_length(list) == 1);
    uint32_t index = 0;
    ASSERT_TRUE(u_arraylist_get_index(list, &dummyData, &index));
    ASSERT_TRUE(index == 0);
    u_arraylist_free(&list);
}

TEST_F(OPC_util , u_arraylist_get_index_N)
{
    u_arraylist_t  *list = u_arraylist_create();
    int dummyData = 100;
    uint32_t index = 0;
    ASSERT_FALSE(u_arraylist_get_index(NULL, &dummyData, &index));
    ASSERT_FALSE(u_arraylist_get_index(list, NULL, &index));
    ASSERT_FALSE(u_arraylist_get_index(list, &dummyData, &index));
    u_arraylist_free(&list);
}

TEST_F(OPC_util , u_arraylist_destroy_P)
{
    u_arraylist_destroy(NULL);
    u_arraylist_t  *list = u_arraylist_create();
    int *dummyData = (int *)EdgeMalloc(sizeof(int));
    ASSERT_TRUE(u_arraylist_add(list, dummyData)); // Adding an item to increase the length
    ASSERT_TRUE(u_arraylist_length(list) == 1);
    u_arraylist_destroy(list);
}

// uqueue.c - Adding unit tests for missed out cases.
TEST_F(OPC_util , u_queue_add_element_N)
{
    u_queue_t queue;
    u_queue_message_t msg;
    ASSERT_EQ(u_queue_add_element(NULL, &msg), CA_STATUS_FAILED); // Queue is NULL
    ASSERT_EQ(u_queue_add_element(&queue, NULL), CA_STATUS_FAILED); // Msg is NULL
}

TEST_F(OPC_util , u_queue_get_element_N)
{
    u_queue_t queue = {NULL, 0};
    ASSERT_EQ(u_queue_get_element(NULL), (void *)NULL); // Queue is NULL
    ASSERT_EQ(u_queue_get_element(&queue), (void *)NULL); // Element is NULL
}

TEST_F(OPC_util , u_queue_remove_element_N)
{
    u_queue_t queue = {NULL, 0};
    ASSERT_EQ(u_queue_remove_element(NULL), CA_STATUS_FAILED); // Queue is NULL
    ASSERT_EQ(u_queue_remove_element(&queue), CA_STATUS_OK); // Element is NULL
}

TEST_F(OPC_util , u_queue_get_size_N)
{
    ASSERT_EQ(u_queue_get_size(NULL), 0); // Queue is NULL
}

TEST_F(OPC_util , u_queue_reset_N)
{
    ASSERT_EQ(u_queue_reset(NULL), CA_STATUS_FAILED); // Queue is NULL
}

TEST_F(OPC_util , u_queue_get_head_N)
{
    u_queue_t queue = {NULL, 0};
    ASSERT_EQ(u_queue_get_head(NULL), (void *)NULL); // Queue is NULL
    ASSERT_EQ(u_queue_get_head(&queue), (void *)NULL); // Element is NULL
}

/*
 int main(int argc, char **argv) {
 ::testing::InitGoogleTest(&argc, argv);
 return RUN_ALL_TESTS();
 }*/
