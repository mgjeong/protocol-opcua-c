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

void testMethod_P1(char *endpointUri)
{
    EdgeMessage *msg = createEdgeMessage(endpointUri, 1, CMD_METHOD);
    EXPECT_EQ(NULL != msg, true);

    double input = 16.0;
    EdgeResult ret = insertEdgeMethodParameter(&msg, "{2;S;v=0}square(x)", 1, Double, SCALAR, (void *) &input, NULL, 0);
    EXPECT_EQ(ret.code, STATUS_OK);
    sendRequest(msg);
    //destroyEdgeMessage(msg);

    msg = createEdgeMessage(endpointUri, 1, CMD_METHOD);
    ASSERT_EQ(NULL != msg, true);

    int32_t array[5] = {10, 20, 30, 40, 50};
    ret = insertEdgeMethodParameter(&msg, "{2;S;v=0}incrementInc32Array(x,delta)", 2,
            Int32, ARRAY_1D, NULL, (void *) array, 5);
    EXPECT_EQ(ret.code, STATUS_OK);

    int delta = 5;
    ret = insertEdgeMethodParameter(&msg, "{2;S;v=0}incrementInc32Array(x,delta)", 2,
            Int32, SCALAR, (void *) &delta, NULL, 0);
    ASSERT_EQ(ret.code, STATUS_OK);
    sendRequest(msg);
    //destroyEdgeMessage(msg);

    sleep(1);
}

void testMethod_P2(char *endpointUri)
{
    EdgeMessage *msg = createEdgeMessage(endpointUri, 1, CMD_METHOD);
    ASSERT_EQ(NULL != msg, true);

    char **array = (char**) EdgeMalloc(sizeof(char*) * 5);
    ASSERT_EQ(NULL != array, true);
    array[0] = (char *) EdgeMalloc(sizeof(char) * 10);
    ASSERT_EQ(NULL != array[0], true);
    array[1] = (char *) EdgeMalloc(sizeof(char) * 10);
    ASSERT_EQ(NULL != array[1], true);
    array[2] = (char *) EdgeMalloc(sizeof(char) * 10);
    ASSERT_EQ(NULL != array[2], true);
    array[3] = (char *) EdgeMalloc(sizeof(char) * 10);
    ASSERT_EQ(NULL != array[3], true);
    array[4] = (char *) EdgeMalloc(sizeof(char) * 10);
    ASSERT_EQ(NULL != array[4], true);

    strncpy(array[0], "apple", strlen("apple"));
    array[0][strlen("apple")] = '\0';
    strncpy(array[1], "ball", strlen("ball"));
    array[1][strlen("ball")] = '\0';
    strncpy(array[2], "cats", strlen("cats"));
    array[2][strlen("cats")] = '\0';
    strncpy(array[3], "dogs", strlen("dogs"));
    array[3][strlen("dogs")] = '\0';
    strncpy(array[4], "elephant", strlen("elephant"));
    array[4][strlen("elephant")] = '\0';

    EdgeResult ret = insertEdgeMethodParameter(&msg, "{2;S;v=0}print_string_array(x)", 1,
            String, ARRAY_1D, NULL, (void *) array, 5);
    EXPECT_EQ(ret.code, STATUS_OK);
    sendRequest(msg);
    //destroyEdgeMessage(msg);

    sleep(1);
}

void testMethod_P3(char *endpointUri)
{
    EdgeMessage *msg = createEdgeMessage(endpointUri, 1, CMD_METHOD);
    ASSERT_EQ(NULL != msg, true);

    EdgeResult ret = insertEdgeMethodParameter(&msg, "{2;S;v=0}print_string(x)", 1,
                   String, SCALAR, (void*) "string method", NULL, 0);
    ASSERT_EQ(ret.code, STATUS_OK);
    ret = sendRequest(msg);
    ASSERT_EQ(ret.code, STATUS_OK);
    //destroyEdgeMessage(msg);

    sleep(1);
}

void testMethod_P4(char *endpointUri)
{
    EdgeMessage *msg = createEdgeMessage(endpointUri, 1, CMD_METHOD);
    EXPECT_EQ(NULL != msg, true);

    double input = 16.0;
    EdgeResult ret = insertEdgeMethodParameter(&msg, "{2;S;v=0}InvalidNode", 1, Double, SCALAR, (void *) &input, NULL, 0);
    EXPECT_EQ(ret.code, STATUS_OK);
    sendRequest(msg);
    //destroyEdgeMessage(msg);
}

void testMethodWithoutEndpoint()
{
    EdgeMessage *msg = createEdgeMessage(NULL, 1, CMD_METHOD);
    EXPECT_EQ(NULL != msg, true);

    double input = 16.0;
    EdgeResult ret = insertEdgeMethodParameter(&msg, "{2;S;v=0}square(x)", 1, Double, SCALAR, (void *) &input, NULL, 0);
    ASSERT_EQ(ret.code, STATUS_OK);
    EdgeResult result = sendRequest(msg);
    ASSERT_EQ(result.code, STATUS_PARAM_INVALID);
}

void testMethodWithoutValueAlias(char *endpointUri)
{
    EdgeMessage *msg = createEdgeMessage(endpointUri, 1, CMD_METHOD);
    EXPECT_EQ(NULL != msg, true);

    double input = 16.0;
    EdgeResult ret = insertEdgeMethodParameter(&msg, NULL, 1, Double, SCALAR, (void *) &input, NULL, 0);
    ASSERT_EQ(ret.code, STATUS_PARAM_INVALID);
    EdgeResult result = sendRequest(msg);
    ASSERT_EQ(result.code, STATUS_PARAM_INVALID);
}

void testMethodWithoutMessage()
{
    EdgeResult result = sendRequest(NULL);
    ASSERT_EQ(result.code, STATUS_PARAM_INVALID);
}

