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

void testMethod_P1(char *endpointUri)
{
    EdgeMessage *msg = createEdgeMessage(endpointUri, 1, CMD_METHOD);
    EXPECT_EQ(NULL != msg, true);

    double *input = (double*) EdgeCalloc(1, sizeof(double));                       //  16.0;
    *input = 16.0;
    EdgeResult ret = insertEdgeMethodParameter(&msg, "{2;S;v=0}square(x)", 1, EDGE_NODEID_DOUBLE, SCALAR, (void *) input, NULL, 0);
    EXPECT_EQ(ret.code, STATUS_OK);
    sendRequest(msg);
    destroyEdgeMessage(msg);

    msg = createEdgeMessage(endpointUri, 1, CMD_METHOD);
    ASSERT_EQ(NULL != msg, true);

    int32_t array[5] = {10, 20, 30, 40, 50};
    ret = insertEdgeMethodParameter(&msg, "{2;S;v=0}incrementInc32Array(x,delta)", 2,
            EDGE_NODEID_INT32, ARRAY_1D, NULL, (void *) array, 5);
    EXPECT_EQ(ret.code, STATUS_OK);

    int delta = 5;
    ret = insertEdgeMethodParameter(&msg, "{2;S;v=0}incrementInc32Array(x,delta)", 2,
            EDGE_NODEID_INT32, SCALAR, (void *) &delta, NULL, 0);
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
            EDGE_NODEID_STRING, ARRAY_1D, NULL, (void *) array, 5);
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
                   EDGE_NODEID_STRING, SCALAR, (void*) "string method", NULL, 0);
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
    EdgeResult ret = insertEdgeMethodParameter(&msg, "{2;S;v=0}InvalidNode", 1, EDGE_NODEID_DOUBLE, SCALAR, (void *) &input, NULL, 0);
    EXPECT_EQ(ret.code, STATUS_OK);
    sendRequest(msg);
    //destroyEdgeMessage(msg);
}

void testMethodWithoutEndpoint()
{
    EdgeMessage *msg = createEdgeMessage(NULL, 1, CMD_METHOD);
    ASSERT_EQ(NULL != msg, false);

    double input = 16.0;
    EdgeResult ret = insertEdgeMethodParameter(&msg, "{2;S;v=0}square(x)", 1, EDGE_NODEID_DOUBLE, SCALAR, (void *) &input, NULL, 0);
    ASSERT_EQ(ret.code, STATUS_PARAM_INVALID);
    EdgeResult result = sendRequest(msg);
    ASSERT_EQ(result.code, STATUS_PARAM_INVALID);
}

void testMethodWithoutCommand()
{
    /* Invalid command */
    EdgeMessage *msg = createEdgeMessage(NULL, 1, CMD_INVALID);
    ASSERT_EQ(NULL != msg, false);

    double input = 16.0;
    EdgeResult ret = insertEdgeMethodParameter(&msg, "{2;S;v=0}square(x)", 1, EDGE_NODEID_DOUBLE, SCALAR, (void *) &input, NULL, 0);
    ASSERT_EQ(ret.code, STATUS_PARAM_INVALID);
    destroyEdgeMessage(msg);

    /* Wrong Command (READ instead of METHOD) */
    msg = createEdgeMessage(NULL, 1, CMD_READ);
    ASSERT_EQ(NULL != msg, false);
    ret = insertEdgeMethodParameter(&msg, "{2;S;v=0}square(x)", 1, EDGE_NODEID_DOUBLE, SCALAR, (void *) &input, NULL, 0);
    ASSERT_EQ(ret.code, STATUS_PARAM_INVALID);
    destroyEdgeMessage(msg);
}

void testMethodWithoutValueAlias(char *endpointUri)
{
    EdgeMessage *msg = createEdgeMessage(endpointUri, 1, CMD_METHOD);
    ASSERT_EQ(NULL != msg, true);

    /* Without SCALAR param */
    double input = 16.0;
    EdgeResult ret = insertEdgeMethodParameter(&msg, NULL, 1, EDGE_NODEID_DOUBLE, SCALAR, (void *) &input, NULL, 0);
    ASSERT_EQ(ret.code, STATUS_PARAM_INVALID);
    EdgeResult result = sendRequest(msg);
    ASSERT_EQ(result.code, STATUS_PARAM_INVALID);
}

void testMethodWithoutParam()
{
    /* Invalid command */
    EdgeMessage *msg = createEdgeMessage(NULL, 1, CMD_METHOD);
    ASSERT_EQ(NULL != msg, false);
    EdgeResult ret = insertEdgeMethodParameter(&msg, "{2;S;v=0}square(x)", 1, EDGE_NODEID_DOUBLE, SCALAR, (void *) NULL, NULL, 0);
    ASSERT_EQ(ret.code, STATUS_PARAM_INVALID);
    destroyEdgeMessage(msg);

    msg = createEdgeMessage(NULL, 1, CMD_METHOD);
    ASSERT_EQ(NULL != msg, false);
    ret = insertEdgeMethodParameter(&msg, "{2;S;v=0}incrementInc32Array(x,delta)", 2, EDGE_NODEID_INT32, ARRAY_1D, (void *) NULL, NULL, 0);
    ASSERT_EQ(ret.code, STATUS_PARAM_INVALID);
    destroyEdgeMessage(msg);
}

void testMethodWithoutMessage()
{
    EdgeResult result = sendRequest(NULL);
    ASSERT_EQ(result.code, STATUS_PARAM_INVALID);
}

