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

extern "C" {
#include "opcua_manager.h"
#include "opcua_common.h"
#include "edge_identifier.h"
#include "edge_utils.h"
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

    EXPECT_EQ((char *)getMapElement(sampleMap, (keyValue) "key1"), "value1");
    EXPECT_EQ((char *)getMapElement(sampleMap, (keyValue) "key2"), "value2");
    EXPECT_EQ((char *)getMapElement(sampleMap, (keyValue) "key3"), "value3");
    EXPECT_EQ((char *)getMapElement(sampleMap, (keyValue) "key6"), "value6");

    deleteMap(sampleMap);

    EXPECT_EQ(sampleMap->head == NULL, true);
}

TEST_F(OPC_utilMap , insertMapElement_N)
{
    sampleMap = createMap();

    EXPECT_NE((char *)getMapElement(sampleMap, (keyValue) "key1"), "value1");
    EXPECT_EQ(getMapElement(sampleMap, (keyValue) "key1") == NULL, true);

    insertMapElement(sampleMap, (keyValue) "key1", (keyValue) "value1");
    EXPECT_EQ(sampleMap->head == NULL, false);
    insertMapElement(sampleMap, (keyValue) "key2", (keyValue) "value2");
    insertMapElement(sampleMap, (keyValue) "key6", (keyValue) "value6");
    insertMapElement(sampleMap, (keyValue) "key3", (keyValue) "value3");

    EXPECT_NE((char *)getMapElement(sampleMap, (keyValue) "key4"), "value4");
    EXPECT_NE((char *)getMapElement(sampleMap, (keyValue) "key1"), "value2");
    EXPECT_NE((char *)getMapElement(sampleMap, (keyValue) "key2"), "value3");
    EXPECT_EQ((char *)getMapElement(sampleMap, (keyValue) "key6"), "value6");

    deleteMap(sampleMap);

    EXPECT_NE((char *)getMapElement(sampleMap, (keyValue) "key1"), "value1");
    EXPECT_NE((char *)getMapElement(sampleMap, (keyValue) "key2"), "value2");
    EXPECT_NE((char *)getMapElement(sampleMap, (keyValue) "key3"), "value3");
    EXPECT_NE((char *)getMapElement(sampleMap, (keyValue) "key6"), "value6");

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

    retStr= cloneString(WELL_KNOWN_DISCOVERY_VALUE);
    EXPECT_NE(retStr == NULL, true);

    EXPECT_EQ(strcmp(retStr, WELL_KNOWN_DISCOVERY_VALUE), 0);

    free(retStr); retStr = NULL;
    EXPECT_EQ(retStr == NULL, true);
}

TEST_F(OPC_util , cloneEdgeEndpoint_P)
{
    EdgeEndPointInfo *retEndpoint = NULL;
    
    EdgeEndpointConfig *endpointConfig = (EdgeEndpointConfig *) malloc(sizeof(EdgeEndpointConfig));
    endpointConfig->bindAddress = "100.100.100.100";
    endpointConfig->bindPort = 12686;
    endpointConfig->applicationName = (char *)DEFAULT_SERVER_APP_NAME_VALUE;
    endpointConfig->applicationUri = (char *)DEFAULT_SERVER_URI_VALUE;
    endpointConfig->productUri = (char *)DEFAULT_PRODUCT_URI_VALUE;
    endpointConfig->securityPolicyUri = NULL;
    endpointConfig->serverName = (char *)DEFAULT_SERVER_NAME_VALUE;

    EdgeEndPointInfo *ep = (EdgeEndPointInfo *) malloc(sizeof(EdgeEndPointInfo));
    ep->endpointUri = "opc.tcp://107.108.81.116:12686/edge-opc-server";
    ep->config = endpointConfig;

    EXPECT_EQ(endpointConfig != NULL, true);
    EXPECT_EQ(ep != NULL, true);
    EXPECT_EQ(retEndpoint == NULL, true);

    retEndpoint= cloneEdgeEndpointInfo(ep);

    EXPECT_EQ(retEndpoint != NULL, true);
    EXPECT_EQ(strcmp(retEndpoint->endpointUri, ep->endpointUri), 0);
    EXPECT_EQ(strcmp(retEndpoint->config->bindAddress, endpointConfig->bindAddress), 0);
    EXPECT_EQ(strcmp(retEndpoint->config->bindAddress,  endpointConfig->bindAddress), 0);
    EXPECT_EQ(strcmp(retEndpoint->config->applicationUri, endpointConfig->applicationUri), 0);
    EXPECT_EQ(strcmp(retEndpoint->config->productUri, endpointConfig->productUri), 0);
    EXPECT_EQ(strcmp(retEndpoint->config->serverName, endpointConfig->serverName), 0);
    EXPECT_EQ(endpointConfig->bindPort, retEndpoint->config->bindPort);

    freeEdgeEndpointInfo(retEndpoint);
    retEndpoint = NULL;
    free(endpointConfig); endpointConfig = NULL;
    free(ep); ep = NULL;

    EXPECT_EQ(endpointConfig == NULL, true);
    EXPECT_EQ(ep == NULL, true);
    EXPECT_EQ(retEndpoint == NULL, true);
    
}

TEST_F(OPC_util , cloneNode_P)
{
    EdgeNodeInfo *nodeInfo = (EdgeNodeInfo *) calloc(1, sizeof(EdgeNodeInfo));
    nodeInfo->nodeId = (EdgeNodeId *) calloc(1, sizeof(EdgeNodeId));
    char* nodeName = "String";
    nodeInfo->valueAlias = (char *) malloc(strlen(nodeName) + 1);
    strcpy(nodeInfo->valueAlias, nodeName);
    nodeInfo->valueAlias[strlen(nodeName)] = '\0';
    nodeInfo->methodName = "methodName";
    nodeInfo->nodeId->type = INTEGER;
    nodeInfo->nodeId->integerNodeId = RootFolder;
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

    
    free(nodeInfo->valueAlias);nodeInfo->valueAlias = NULL;
    free(nodeInfo);nodeInfo = NULL;
    freeEdgeNodeInfo(retNodeInfo);
    retNodeInfo = NULL;
    
    EXPECT_EQ(nodeInfo == NULL, true);
    EXPECT_EQ(retNodeInfo == NULL, true);
}

/*
int main(int argc, char **argv) {
  ::testing::InitGoogleTest(&argc, argv);
  return RUN_ALL_TESTS();
}*/
