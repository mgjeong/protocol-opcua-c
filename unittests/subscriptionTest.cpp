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

extern char node_arr[13][30];

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
    EdgeMessage* msg = createEdgeSubMessage(endpointUri, "{2;S;v=12}CharArray", 4, Edge_Create_Sub);
    EXPECT_EQ(NULL!=msg, true);

    double samplingInterval = 100.0;
    int keepalivetime = (1 > (int) (ceil(10000.0 / 0.0))) ? 1 : (int) ceil(10000.0 / 0.0);
    insertSubParameter(&msg, "{2;S;v=12}CharArray", Edge_Create_Sub, samplingInterval, 0.0, keepalivetime, 10000, 1, true, 0, 50);
    insertSubParameter(&msg, "{2;S;v=14}Guid", Edge_Create_Sub, samplingInterval, 0.0, keepalivetime, 10000, 1, true, 0, 50);
    insertSubParameter(&msg, "{2;S;v=15}ByteStringArray", Edge_Create_Sub, samplingInterval, 0.0, keepalivetime, 10000, 1, true, 0, 50);
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
    msg = NULL;
    msg = createEdgeSubMessage(endpointUri, "{2;S;v=15}ByteStringArray", 0, Edge_Delete_Sub);
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
