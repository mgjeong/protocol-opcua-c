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

extern char node_arr[13][30];

// String1
// String2
void testRead_P1(char *endpointUri)
{
    int num_requests  = 2;
    EdgeMessage *msg = createEdgeAttributeMessage(endpointUri, num_requests, CMD_READ);
    EXPECT_EQ(NULL != msg, true);
    for (int i = 0; i < num_requests; i++)
    {
        printf("node :: %s\n", node_arr[i]);
        insertReadAccessNode(&msg, node_arr[i]);
    }
    EdgeResult result = sendRequest(msg);
    destroyEdgeMessage(msg);
    ASSERT_EQ(result.code, STATUS_OK);
    sleep(1);
}

//DoubleArray
//CharArray
//ByteStringArray
void testRead_P2(char *endpointUri)
{
    int num_requests  = 3;
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
