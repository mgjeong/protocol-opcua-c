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

#define TAG "methodTest"

extern char node_arr[46][30];

void testSubscription_P1(char *endpointUri)
{
    /* Create Subscription */
    EdgeMessage* msg = createEdgeSubMessage(endpointUri, node_arr[0], 1, Edge_Create_Sub);
    EXPECT_EQ(NULL!=msg, true);

    double samplingInterval = 100.0;
    int keepalivetime = (1 > (int) (ceil(10000.0 / 0.0))) ? 1 : (int) ceil(10000.0 / 0.0);
    insertSubParameter(&msg, node_arr[0], Edge_Create_Sub, samplingInterval, 0.0, keepalivetime, 10000, 1, true, 0, 50);

    EdgeResult result = sendRequest(msg);
    EXPECT_EQ(result.code, STATUS_OK);
    destroyEdgeMessage (msg);
    sleep(1);

    /* Modify Subscription */
    msg = NULL;
    msg = createEdgeSubMessage(endpointUri, node_arr[0], 0, Edge_Modify_Sub);
    EXPECT_EQ(NULL!=msg, true);

    samplingInterval = 500.0;
    keepalivetime = (1 > (int) (ceil(10000.0 / 0.0))) ? 1 : (int) ceil(10000.0 / 0.0);
    insertSubParameter(&msg, node_arr[0], Edge_Modify_Sub, samplingInterval, 0.0, keepalivetime, 10000, 1, true, 0, 50);

    result = sendRequest(msg);
    EXPECT_EQ(result.code, STATUS_OK);
    destroyEdgeMessage (msg);
    sleep(1);

    /* Delete Subscription */
    msg = NULL;
    msg = createEdgeSubMessage(endpointUri, node_arr[0], 0, Edge_Delete_Sub);
    EXPECT_EQ(NULL!=msg, true);
    result = sendRequest(msg);
    EXPECT_EQ(result.code, STATUS_OK);
    destroyEdgeMessage(msg);
    sleep(1);
}

/* Group Subscription */
void testSubscription_P2(char *endpointUri)
{
    /* Create Subscription */
    EdgeMessage* msg = createEdgeSubMessage(endpointUri, "{2;S;v=12}CharArray", 6, Edge_Create_Sub);
    EXPECT_EQ(NULL!=msg, true);

    double samplingInterval = 100.0;
    int keepalivetime = (1 > (int) (ceil(10000.0 / 0.0))) ? 1 : (int) ceil(10000.0 / 0.0);
    insertSubParameter(&msg, "{2;S;v=12}CharArray", Edge_Create_Sub, samplingInterval, 0.0, keepalivetime, 10000, 1, true, 0, 50);
    insertSubParameter(&msg, "{2;S;v=14}Guid", Edge_Create_Sub, samplingInterval, 0.0, keepalivetime, 10000, 1, true, 0, 50);
    insertSubParameter(&msg, "{2;S;v=14}GuidArray", Edge_Create_Sub, samplingInterval, 0.0, keepalivetime, 10000, 1, true, 0, 50);
    insertSubParameter(&msg, "{2;S;v=15}ByteStringArray", Edge_Create_Sub, samplingInterval, 0.0, keepalivetime, 10000, 1, true, 0, 50);
    insertSubParameter(&msg, "{2;S;v=1}BoolArray", Edge_Create_Sub, samplingInterval, 0.0, keepalivetime, 10000, 1, true, 0, 50);
    insertSubParameter(&msg, "{2;S;v=11}Double", Edge_Create_Sub, samplingInterval, 0.0, keepalivetime, 10000, 1, true, 0, 50);

    EdgeResult result = sendRequest(msg);
    EXPECT_EQ(result.code, STATUS_OK);
    destroyEdgeMessage (msg);
    sleep(1);

    /* Delete Subscription */
    msg = NULL;
    msg = createEdgeSubMessage(endpointUri, "{2;S;v=12}CharArray", 0, Edge_Delete_Sub);
    EXPECT_EQ(NULL!=msg, true);
    result = sendRequest(msg);
    EXPECT_EQ(result.code, STATUS_OK);
    destroyEdgeMessage(msg);
    sleep(1);

    /* Delete Subscription */
    msg = NULL;
    msg = createEdgeSubMessage(endpointUri, "{2;S;v=14}Guid", 0, Edge_Delete_Sub);
    EXPECT_EQ(NULL!=msg, true);
    result = sendRequest(msg);
    EXPECT_EQ(result.code, STATUS_OK);
    destroyEdgeMessage(msg);
    sleep(1);

    /* Delete Subscription */
    msg = createEdgeSubMessage(endpointUri, "{2;S;v=14}GuidArray", 0, Edge_Delete_Sub);
    EXPECT_EQ(NULL!=msg, true);
    result = sendRequest(msg);
    EXPECT_EQ(result.code, STATUS_OK);
    destroyEdgeMessage(msg);
    sleep(1);

    /* Delete Subscription */
    msg = NULL;
    msg = createEdgeSubMessage(endpointUri, "{2;S;v=15}ByteStringArray", 0, Edge_Delete_Sub);
    EXPECT_EQ(NULL!=msg, true);
    result = sendRequest(msg);
    EXPECT_EQ(result.code, STATUS_OK);
    destroyEdgeMessage(msg);
    sleep(1);

    /* Delete Subscription */
    msg = NULL;
    msg = createEdgeSubMessage(endpointUri, "{2;S;v=1}BoolArray", 0, Edge_Delete_Sub);
    EXPECT_EQ(NULL!=msg, true);
    result = sendRequest(msg);
    EXPECT_EQ(result.code, STATUS_OK);
    destroyEdgeMessage(msg);
    sleep(1);

    /* Delete Subscription */
    msg = NULL;
    msg = createEdgeSubMessage(endpointUri, "{2;S;v=11}Double", 0, Edge_Delete_Sub);
    EXPECT_EQ(NULL!=msg, true);
    result = sendRequest(msg);
    EXPECT_EQ(result.code, STATUS_OK);
    destroyEdgeMessage(msg);
    sleep(1);
}

/* Republish Subscription */
void testSubscription_P3(char *endpointUri)
{
    /* Create Subscription */
    EdgeMessage* msg = createEdgeSubMessage(endpointUri, node_arr[0], 1, Edge_Create_Sub);
    EXPECT_EQ(NULL!=msg, true);

    double samplingInterval = 100.0;
    int keepalivetime = (1 > (int) (ceil(10000.0 / 0.0))) ? 1 : (int) ceil(10000.0 / 0.0);
    insertSubParameter(&msg, node_arr[0], Edge_Create_Sub, samplingInterval, 0.0, keepalivetime, 10000, 1, true, 0, 50);

    EdgeResult result = sendRequest(msg);
    EXPECT_EQ(result.code, STATUS_OK);
    destroyEdgeMessage (msg);
    sleep(1);

    msg = NULL;
    msg = createEdgeSubMessage(endpointUri, node_arr[0], 0, Edge_Republish_Sub);
    EXPECT_EQ(NULL!=msg, true);
    result = sendRequest(msg);
    EXPECT_EQ(result.code, STATUS_OK);
    destroyEdgeMessage(msg);
    sleep(1);

    /* Delete Subscription */
    msg = NULL;
    msg = createEdgeSubMessage(endpointUri, node_arr[0], 0, Edge_Delete_Sub);
    EXPECT_EQ(NULL!=msg, true);
    result = sendRequest(msg);
    EXPECT_EQ(result.code, STATUS_OK);
    destroyEdgeMessage(msg);
    sleep(1);
}

void testSubscriptionWithoutCommand(char *endpointUri)
{
    /* Create Subscription */
    EdgeMessage* msg = createEdgeSubMessage(endpointUri, node_arr[0], 1, Edge_Create_Sub);
    EXPECT_EQ(NULL!=msg, true);
    /* Invalid command */
    msg->command = CMD_INVALID;
    double samplingInterval = 100.0;
    int keepalivetime = (1 > (int) (ceil(10000.0 / 0.0))) ? 1 : (int) ceil(10000.0 / 0.0);
    EdgeResult ret = insertSubParameter(&msg, node_arr[0], Edge_Create_Sub, samplingInterval, 0.0, keepalivetime, 10000, 1, true, 0, 50);
    ASSERT_EQ(ret.code, STATUS_PARAM_INVALID);
    //destroyEdgeMessage (msg);
}

void testSubscriptionWithoutEndpoint()
{
    /* Create Subscription */
    EdgeMessage* msg = createEdgeSubMessage(NULL, node_arr[0], 1, Edge_Create_Sub);
    EXPECT_EQ(NULL!=msg, false);

    double samplingInterval = 100.0;
    int keepalivetime = (1 > (int) (ceil(10000.0 / 0.0))) ? 1 : (int) ceil(10000.0 / 0.0);
    EdgeResult ret = insertSubParameter(&msg, node_arr[0], Edge_Create_Sub, samplingInterval, 0.0, keepalivetime, 10000, 1, true, 0, 50);
    ASSERT_EQ(ret.code, STATUS_PARAM_INVALID);

    EdgeResult result = sendRequest(msg);
    destroyEdgeMessage (msg);
    ASSERT_EQ(result.code, STATUS_PARAM_INVALID);
}

void testSubscriptionWithoutValueAlias(char *endpointUri)
{
    /* Create Subscription */
    EdgeMessage* msg = createEdgeSubMessage(endpointUri, NULL, 1, Edge_Create_Sub);
    ASSERT_EQ(NULL!=msg, true);

    double samplingInterval = 100.0;
    int keepalivetime = (1 > (int) (ceil(10000.0 / 0.0))) ? 1 : (int) ceil(10000.0 / 0.0);
    EdgeResult ret = insertSubParameter(&msg, NULL, Edge_Create_Sub, samplingInterval, 0.0, keepalivetime, 10000, 1, true, 0, 50);
    ASSERT_EQ(ret.code, STATUS_PARAM_INVALID);

    EdgeResult result = sendRequest(msg);
    destroyEdgeMessage (msg);
    ASSERT_EQ(result.code, STATUS_PARAM_INVALID);
}

void testSubscriptionWithoutMessage()
{
    EdgeResult result = sendRequest(NULL);
    ASSERT_EQ(result.code, STATUS_PARAM_INVALID);
}

void testSubscriptionWithoutSubReq(char *endpointUri)
{
    /* Create Subscription */
    EdgeMessage* msg = createEdgeSubMessage(endpointUri, "{2;S;v=12}CharArray", 6, Edge_Create_Sub);
    EXPECT_EQ(NULL!=msg, true);

    /* Send subscription request with SubMsg */
    EdgeResult result = sendRequest(msg);
    EXPECT_EQ(result.code, STATUS_PARAM_INVALID);
    destroyEdgeMessage (msg);
}
