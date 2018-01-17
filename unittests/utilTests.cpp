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

/*
int main(int argc, char **argv) {
  ::testing::InitGoogleTest(&argc, argv);
  return RUN_ALL_TESTS();
}*/
