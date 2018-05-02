/******************************************************************
 *
 * Copyright 2018 Samsung Electronics All Rights Reserved.
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

#include <gtest/gtest.h>
#include <iostream>
#include <math.h>

extern "C"
{
#include "opcua_manager.h"
#include "opcua_common.h"
#include "edge_identifier.h"
#include "edge_utils.h"
#include "edge_logger.h"
#include "edge_malloc.h"
#include "open62541.h"
}

#define TAG "readTest"

extern char node_arr[46][30];

// String1
// String2
// LocalizedText
// QualifiedName
// NodeId1
// NodeId2
// NodeId3
// NodeId4
void testRead_P1(char *endpointUri)
{
    int num_requests  = 8;
    EdgeMessage *msg = createEdgeAttributeMessage(endpointUri, num_requests, CMD_READ);
    EXPECT_EQ(NULL != msg, true);
    for (int i = 0; i < 2; i++)
    {
        printf("node :: %s\n", node_arr[i]);
        insertReadAccessNode(&msg, node_arr[i]);
    }

    for (int i = 14; i <= 19; i++)
    {
        printf("node :: %s\n", node_arr[i]);
        insertReadAccessNode(&msg, node_arr[i]);
    }

    EdgeResult result = sendRequest(msg);
    destroyEdgeMessage(msg);
    ASSERT_EQ(result.code, STATUS_OK);
    sleep(1);
}

// DoubleArray
// CharArray
// ByteStringArray
// GuidArray
void testRead_P2(char *endpointUri)
{
    int num_requests  = 4;
    EdgeMessage *msg = createEdgeAttributeMessage(endpointUri, num_requests, CMD_READ);
    EXPECT_EQ(NULL != msg, true);
    int indx = 10;
    for (int i = 0; i < num_requests; i++)
    {
        printf("node :: %s\n", node_arr[i+indx]);
        insertReadAccessNode(&msg, node_arr[i+indx]);
    }
    EdgeResult result = sendRequest(msg);
    destroyEdgeMessage(msg);
    ASSERT_EQ(result.code, STATUS_OK);
    sleep(1);
}

// Double
// Guid
void testRead_P3(char *endpointUri)
{
    int num_requests  = 2;
    EdgeMessage *msg = createEdgeAttributeMessage(endpointUri, num_requests, CMD_READ);
    EXPECT_EQ(NULL != msg, true);
    insertReadAccessNode(&msg, node_arr[3]);
    insertReadAccessNode(&msg, node_arr[9]);
    EdgeResult result = sendRequest(msg);
    destroyEdgeMessage(msg);
    ASSERT_EQ(result.code, STATUS_OK);
    sleep(1);
}

// Invalid Node
void testRead_P4(char *endpointUri)
{
    int num_requests  = 1;
    EdgeMessage *msg = createEdgeAttributeMessage(endpointUri, num_requests, CMD_READ);
    EXPECT_EQ(NULL != msg, true);
    insertReadAccessNode(&msg, node_arr[8]);
    EdgeResult result = sendRequest(msg);
    destroyEdgeMessage(msg);
    ASSERT_EQ(result.code, STATUS_OK);
    sleep(1);
}

void testRead_P5(char *endpointUri)
{
    int num_requests  = 30;
    EdgeMessage *msg = createEdgeAttributeMessage(endpointUri, num_requests, CMD_READ);
    EXPECT_EQ(NULL != msg, true);
    int indx = 4;
    for (int i = 0; i < 4; i++)
    {
        printf("node :: %s\n", node_arr[i+indx]);
        insertReadAccessNode(&msg, node_arr[i+indx]);
    }
    indx = 20;
    for (int i = 0; i < 26; i++)
    {
        printf("node :: %s\n", node_arr[i+indx]);
        insertReadAccessNode(&msg, node_arr[i+indx]);
    }
    EdgeResult result = sendRequest(msg);
    destroyEdgeMessage(msg);
    ASSERT_EQ(result.code, STATUS_OK);
    sleep(1);
}

void testReadWithoutCommand()
{
    int num_requests  = 1;
    // Invalid command
    EdgeMessage *msg = createEdgeAttributeMessage(NULL, num_requests, CMD_INVALID);
    ASSERT_EQ(NULL != msg, false);
    EdgeResult result = insertReadAccessNode(&msg, node_arr[0]);
    destroyEdgeMessage(msg);
    ASSERT_EQ(result.code, STATUS_PARAM_INVALID);

    // Wrong command type
    msg = createEdgeAttributeMessage(NULL, num_requests, CMD_INVALID);
    ASSERT_EQ(NULL != msg, false);
    result = insertReadAccessNode(&msg, node_arr[0]);
    destroyEdgeMessage(msg);
    ASSERT_EQ(result.code, STATUS_PARAM_INVALID);
}

void testReadWithoutEndpoint()
{
    int num_requests  = 1;
    EdgeMessage *msg = createEdgeAttributeMessage(NULL, num_requests, CMD_READ);
    ASSERT_EQ(NULL != msg, false);

    for (int i = 0; i < num_requests; i++)
    {
        insertReadAccessNode(&msg, node_arr[i]);
    }

    EdgeResult result = sendRequest(msg);
    destroyEdgeMessage(msg);
    ASSERT_EQ(result.code, STATUS_PARAM_INVALID);
}

void testReadWithoutValueAlias(char *endpointUri)
{
    int num_requests  = 1;
    EdgeMessage *msg = createEdgeAttributeMessage(endpointUri, num_requests, CMD_READ);
    ASSERT_EQ(NULL != msg, true);
    insertReadAccessNode(&msg, NULL);
    EdgeResult result = sendRequest(msg);
    destroyEdgeMessage(msg);
    ASSERT_EQ(result.code, STATUS_PARAM_INVALID);
}

void testReadWithoutMessage()
{
    EdgeResult result = sendRequest(NULL);
    ASSERT_EQ(result.code, STATUS_PARAM_INVALID);
}
