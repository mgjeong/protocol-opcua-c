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
#include "queue.h"
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

TEST_F(OPC_util , isNodeClassValid_N)
{
    UA_NodeClass invalidNodeClass = (UA_NodeClass) -1;
    ASSERT_EQ(isNodeClassValid(invalidNodeClass), false);
}

TEST_F(OPC_util , getEdgeNodeIdType_P)
{
    ASSERT_EQ(getEdgeNodeIdType('N'), INTEGER);
    ASSERT_EQ(getEdgeNodeIdType('S'), STRING);
    ASSERT_EQ(getEdgeNodeIdType('B'), BYTESTRING);
    ASSERT_EQ(getEdgeNodeIdType('G'), UUID);
}

TEST_F(OPC_util , getCharacterNodeIdType_P)
{
    ASSERT_EQ(getCharacterNodeIdType(UA_NODEIDTYPE_NUMERIC), 'N');
    ASSERT_EQ(getCharacterNodeIdType(UA_NODEIDTYPE_STRING), 'S');
    ASSERT_EQ(getCharacterNodeIdType(UA_NODEIDTYPE_BYTESTRING), 'B');
    ASSERT_EQ(getCharacterNodeIdType(UA_NODEIDTYPE_GUID), 'G');
}

TEST_F(OPC_util , get_size_P)
{
    ASSERT_EQ(get_size(Boolean, false) != -1, true);
    ASSERT_EQ(get_size(SByte, false) != -1, true);
    ASSERT_EQ(get_size(Byte, false) != -1, true);
    ASSERT_EQ(get_size(Int16, false) != -1, true);
    ASSERT_EQ(get_size(UInt16, false) != -1, true);
    ASSERT_EQ(get_size(Int32, false) != -1, true);
    ASSERT_EQ(get_size(UInt32, false) != -1, true);
    ASSERT_EQ(get_size(Int64, false) != -1, true);
    ASSERT_EQ(get_size(UInt64, false) != -1, true);
    ASSERT_EQ(get_size(Float, false) != -1, true);
    ASSERT_EQ(get_size(Double, false) != -1, true);
    ASSERT_EQ(get_size(String, false) != -1, true);

    int invalidValue = -1;
    ASSERT_EQ(get_size(invalidValue, false) == -1, true);
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
    nodeInfo->nodeId->type = INTEGER;
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

TEST_F(OPC_util , createQueue_P)
{
    Queue *queue = createQueue(100);
    ASSERT_EQ(queue != NULL, true);

    EdgeFree(queue->message);
    EdgeFree(queue);
}

TEST_F(OPC_util , enqueue_N)
{
    EdgeMessage *msg = (EdgeMessage*) EdgeMalloc(sizeof(EdgeMessage));
    bool ret = enqueue(NULL, msg);
    ASSERT_EQ(ret, false);
    EdgeFree(msg);
}

TEST_F(OPC_util ,dequeue_N)
{
    EdgeMessage *msg = dequeue(NULL);
    ASSERT_EQ(msg == NULL, true);
}

/*
 int main(int argc, char **argv) {
 ::testing::InitGoogleTest(&argc, argv);
 return RUN_ALL_TESTS();
 }*/
