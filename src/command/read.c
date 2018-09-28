/******************************************************************
 *
 * Copyright 2017 Samsung Electronics All Rights Reserved.
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

#include "read.h"
#include "cmd_util.h"
#include "common_client.h"
#include "message_dispatcher.h"
#include "edge_logger.h"
#include "edge_malloc.h"
#include "edge_open62541.h"

#include <inttypes.h>

#define TAG "read"

#define GUID_LENGTH (36)
#define ERROR_DESC_LENGTH (100)

#ifdef CTT_ENABLED
UA_Int64 DateTime_toUnixTime(UA_DateTime date)
{
    return (date - UA_DATETIME_UNIX_EPOCH) / UA_DATETIME_MSEC;
}

/**
 * @brief checkMaxAge - Checks whether time exceeds Max age
 * @param timestamp - read request timestamp
 * @param now - current timestamp
 * @param maxAge - max age to check
 * @return true or false
 */
static bool checkMaxAge(UA_DateTime timestamp, UA_DateTime now, double maxAge)
{
    /* Server timestamp is greater than current time */
    COND_CHECK((timestamp > now), false);

    int64_t second = DateTime_toUnixTime(now);
    int64_t first = DateTime_toUnixTime(timestamp);
    int64_t diff = second - first;
    COND_CHECK(((maxAge != 0) && (diff > maxAge)), false);
    return true;
}

/**
 * @brief compareNumber - Compares two numbers and returns whether (number1>number2)
 * @param number1
 * @param number2
 * @return true or false
 */
static bool compareNumber(int64_t number1, int64_t number2)
{
    return ((number1 > number2) ? true : false);
}

/**
 * @brief checkInvalidTime - Checks whether timestamp in read response is valid
 * @param serverTime - server timestamp in read response
 * @param sourceTime- source timestamp in read response
 * @param validMilliSec - threshold milliseconds to compare
 * @param stamp - TimestampsToReturn parameter in read request
 * @return
 */
static bool checkInvalidTime(UA_DateTime serverTime, UA_DateTime sourceTime, int validMilliSec,
        UA_TimestampsToReturn stamp)
{
    UA_DateTime now = UA_DateTime_now();
    int64_t now_time = UA_DateTime_toUnixTime(now);
    int64_t server_time = UA_DateTime_toUnixTime(serverTime);
    int64_t source_time = UA_DateTime_toUnixTime(sourceTime);

    if (UA_TIMESTAMPSTORETURN_BOTH == stamp)
    {
        /* Both server and source timestamps requested */
        COND_CHECK_MSG((server_time == 0), "Invalid server timestamp\n", false);
        COND_CHECK_MSG((source_time == 0), "Invalid source timestamp\n", false);

        COND_CHECK_MSG((compareNumber(now_time - server_time, validMilliSec)), "", false);
        COND_CHECK_MSG((compareNumber(now_time - source_time, validMilliSec)), "", false);
        COND_CHECK_MSG((compareNumber(server_time, now_time)), "", false);
        COND_CHECK_MSG((compareNumber(source_time, now_time)), "", false);
    }
    else if (UA_TIMESTAMPSTORETURN_SOURCE == stamp)
    {
        /* Only source timestamp requested from server */
        COND_CHECK_MSG((source_time == 0), "Invalid source timestamp\n", false);

        COND_CHECK_MSG((compareNumber(now_time - source_time, validMilliSec)), "", false);
        COND_CHECK_MSG((compareNumber(source_time, now_time)), "", false);
    }
    else if (UA_TIMESTAMPSTORETURN_SERVER == stamp)
    {
        /* Only server timestamp requested from server */
        COND_CHECK_MSG((server_time == 0), "Invalid server timestamp\n", false);

        COND_CHECK_MSG((compareNumber(now_time - server_time, validMilliSec)), "", false);
        COND_CHECK_MSG((compareNumber(server_time, now_time)), "", false);
    }

    return true;
}

/**
 * @brief checkValidation - Validates the read response
 * @param value - Read response value
 * @param msg - Edge Message
 * @param stamp - TimestampsToReturn parameter in read request
 * @param maxAge - Max Age
 * @return value on success, NULL on error
 */
static void *checkValidation(UA_DataValue *value, const EdgeMessage *msg, UA_TimestampsToReturn stamp,
        double maxAge)
{
    /* Error check for invalid timestamp returned by server */
    if (!checkInvalidTime(value->serverTimestamp, value->sourceTimestamp, 86400000, stamp))
    {
        // Error message handling
        sendErrorResponse(msg, "Invalid Time");
        return NULL;
    }

    /* Error check of status code returned by server */
    if (UA_STATUSCODE_GOOD != value->status)
    {
        // Error message handling
        sendErrorResponse(msg, "Error status code from server");
        return NULL;
    }

    /* Error check for array value response */
    if (!UA_Variant_isScalar(&(value->value)))
    {
        if (value->value.arrayLength == 0)
        {
            // Error message handling
            sendErrorResponse(msg, "Invalid array length in read response");
            return NULL;
        }
    }
    return value;
}
#endif // CTT_ENABLED

/**
 * @brief readGroup - Executes read operation of single/group nodes
 * @param client - Client handle
 * @param msg - Request edge message
 * @param attributeId - Attribute Id to read
 */
static void readGroup(UA_Client *client, const EdgeMessage *msg, UA_UInt32 attributeId)
{
    char errorDesc[ERROR_DESC_LENGTH] = {'\0'};
    EdgeMessage *resultMsg = NULL;
    size_t reqLen = msg->requestLength;
    UA_ReadValueId *rv = (UA_ReadValueId *) EdgeMalloc(sizeof(UA_ReadValueId) * reqLen);
    if(IS_NULL(rv))
    {
        EDGE_LOG(TAG, "Memory allocation failed.");
        sendErrorResponse(msg, "Memory allocation failed.");
        return;
    }

    for (size_t i = 0; i < reqLen; i++)
    {
        EDGE_LOG_V(TAG, "[READGROUP] Node to read :: %s [ns : %d]\n", msg->requests[i]->nodeInfo->valueAlias,
                msg->requests[i]->nodeInfo->nodeId->nameSpace);
        UA_ReadValueId_init(&rv[i]);
        rv[i].attributeId = attributeId;
        rv[i].nodeId = UA_NODEID_STRING_ALLOC(msg->requests[i]->nodeInfo->nodeId->nameSpace,
                msg->requests[i]->nodeInfo->valueAlias);
    }

    UA_ReadRequest readRequest;
    UA_ReadRequest_init(&readRequest);
    /* Nodes information to read */
    readRequest.nodesToRead = rv;
    /* Number of nodes to read */
    readRequest.nodesToReadSize = reqLen;

    /* Max age */
    #ifdef CTT_ENABLED
        readRequest.maxAge = 2000;
    #else
        readRequest.maxAge = 0;
    #endif

    /* Timestamp information requested from server */
    readRequest.timestampsToReturn = UA_TIMESTAMPSTORETURN_BOTH;

    //UA_RequestHeader_init(&(readRequest.requestHeader));
    //readRequest.requestHeader.returnDiagnostics = 1;

    UA_ReadResponse readResponse = UA_Client_Service_read(client, readRequest);

    if (readResponse.responseHeader.serviceResult != UA_STATUSCODE_GOOD)
    {
        /* Error response in processing read request */
        EDGE_LOG_V(TAG, "Error in group read :: 0x%08x(%s)\n", readResponse.responseHeader.serviceResult,
                UA_StatusCode_name(readResponse.responseHeader.serviceResult));
        strncpy(errorDesc, "Error in read.", ERROR_DESC_LENGTH);
        goto EXIT;
    }

#ifdef CTT_ENABLED
    if (readResponse.results[0].status == UA_STATUSCODE_GOOD)
    {
        if(UA_ATTRIBUTEID_VALUE == attributeId) {
            if (readRequest.timestampsToReturn == UA_TIMESTAMPSTORETURN_NEITHER)
            {
                if (readResponse.results[0].hasSourceTimestamp
                        || readResponse.results[0].hasServerTimestamp)
                {
                    /* Invalid timestamp error */
                    EDGE_LOG(TAG, "BadInvalidTimestamp\n\n");
                    strncpy(errorDesc, "Bad Invalid Timestamp.", ERROR_DESC_LENGTH);
                    goto EXIT;
                }
            }
            else if (readRequest.timestampsToReturn == UA_TIMESTAMPSTORETURN_BOTH)
            {
                if (!readResponse.results[0].hasSourceTimestamp
                        || !readResponse.results[0].hasServerTimestamp)
                {
                    /* Missing timestamp information in response */
                    EDGE_LOG(TAG, "Timestamp missing\n\n");
                    strncpy(errorDesc, "Timestamp missing.", ERROR_DESC_LENGTH);
                    goto EXIT;
                }
            }
            else if (readRequest.timestampsToReturn == UA_TIMESTAMPSTORETURN_SOURCE)
            {
                if (!readResponse.results[0].hasSourceTimestamp
                        || readResponse.results[0].hasServerTimestamp)
                {
                    /* Source timestamp requested. But source timestamp missing in response */
                    EDGE_LOG(TAG, "source Timestamp missing\n\n");
                    strncpy(errorDesc, "source Timestamp missing.", ERROR_DESC_LENGTH);
                    goto EXIT;
                }
            }
            else if (readRequest.timestampsToReturn == UA_TIMESTAMPSTORETURN_SERVER)
            {
                if (readResponse.results[0].hasSourceTimestamp
                        || !readResponse.results[0].hasServerTimestamp)
                {
                    /* Server timestamp requested. But server timestamp missing in response */
                    EDGE_LOG(TAG, "server Timestamp missing\n\n");
                    strncpy(errorDesc, "server Timestamp missing.", ERROR_DESC_LENGTH);
                    goto EXIT;
                }
            }

            if (readRequest.timestampsToReturn != UA_TIMESTAMPSTORETURN_NEITHER
                    && !checkMaxAge(readResponse.results[0].serverTimestamp, UA_DateTime_now(),
                            readRequest.maxAge * 2))
            {
                /* MaxAge error */
                EDGE_LOG(TAG, "Max age failed\n\n");
                strncpy(errorDesc, "Max Age failed.", ERROR_DESC_LENGTH);
                goto EXIT;
            }

            if (readRequest.timestampsToReturn != UA_TIMESTAMPSTORETURN_NEITHER
                    && !checkValidation(&(readResponse.results[0]), msg, readRequest.timestampsToReturn,
                            readRequest.maxAge))
            {
                strncpy(errorDesc, "", ERROR_DESC_LENGTH);
                goto EXIT;
            }
        }
    }
#endif // CTT_ENABLED

    resultMsg = (EdgeMessage *) EdgeCalloc(1, sizeof(EdgeMessage));
    if(IS_NULL(resultMsg))
    {
        EDGE_LOG(TAG, "Error : Malloc failed for resultMsg in Read Group\n");
        strncpy(errorDesc, "Memory allocation failed.", ERROR_DESC_LENGTH);
        goto EXIT;
    }

    resultMsg->responses = (EdgeResponse **) EdgeCalloc(reqLen, sizeof(EdgeResponse *));
    if(IS_NULL(resultMsg->responses))
    {
        EDGE_LOG(TAG, "Error : Malloc failed for responses in Read Group\n");
        strncpy(errorDesc, "Memory allocation failed.", ERROR_DESC_LENGTH);
        goto EXIT;
    }

    resultMsg->responseLength = 0;
    if (UA_ATTRIBUTEID_VALUE == attributeId) {
        resultMsg->command = CMD_READ;
    } else if (UA_ATTRIBUTEID_MINIMUMSAMPLINGINTERVAL == attributeId) {
        resultMsg->command = CMD_READ_SAMPLING_INTERVAL;
    }
    resultMsg->type = GENERAL_RESPONSE;
    resultMsg->message_id = msg->message_id;
    resultMsg->endpointInfo = cloneEdgeEndpointInfo(msg->endpointInfo);
    if(IS_NULL(resultMsg->endpointInfo))
    {
        EDGE_LOG(TAG, "Error : EdgeCalloc failed for resultMsg.endpointInfo in Read Group\n");
        strncpy(errorDesc, "Memory allocation failed.", ERROR_DESC_LENGTH);
        goto EXIT;
    }

    int respIndex = 0;
    for (int i = 0; i < reqLen; i++)
    {
        if (readResponse.results[i].status == UA_STATUSCODE_GOOD)
        {
            UA_Variant val = readResponse.results[i].value;

            EdgeResponse *response = (EdgeResponse *) EdgeCalloc(1, sizeof(EdgeResponse));
            if (IS_NULL(response))
            {
                EDGE_LOG(TAG, "Memory allocation failed\n");
                strncpy(errorDesc, "Memory allocation failed.", ERROR_DESC_LENGTH);
                goto EXIT;
            }

            response->nodeInfo = cloneEdgeNodeInfo(msg->requests[i]->nodeInfo);
            if(IS_NULL(response->nodeInfo))
            {
                EDGE_LOG(TAG, "Error : Malloc failed for response.Nodeinfo in Read Group\n");
                strncpy(errorDesc, "Memory allocation failed.", ERROR_DESC_LENGTH);
                freeEdgeResponse(response);
                goto EXIT;
            }

            response->requestId = msg->requests[i]->requestId;
            response->message = parseResponse(response, val);
            if (IS_NULL(response->message))
            {
                freeEdgeResponse(response);
                goto EXIT;
            }

            /* Check for diagnostic information in read response */
            response->m_diagnosticInfo = checkDiagnosticInfo(msg->requestLength,
                    readResponse.diagnosticInfos, readResponse.diagnosticInfosSize,
                    readRequest.requestHeader.returnDiagnostics);

            resultMsg->responseLength++;
            resultMsg->responses[respIndex++] = response;
        }
        else
        {
            /* Error in read response for a particular node */
            EDGE_LOG_V(TAG, "Error in group read response for particular node :: 0x%08x(%s)\n",
                    readResponse.results[i].status, UA_StatusCode_name(readResponse.results[i].status));
            if(1 == reqLen)
            {
                // Error response for the node(only one) in the given read request.
                snprintf(errorDesc, ERROR_DESC_LENGTH, "Bad service result for the given node");
                goto EXIT;
            }
            snprintf(errorDesc, ERROR_DESC_LENGTH, "Bad service result for the node at position(%d)", i);
            sendErrorResponse(msg, errorDesc);
        }
    }

    if (reqLen > 1 && resultMsg->responseLength < 1)
    {
        EDGE_LOG(TAG, "There are no valid responses.");
        strncpy(errorDesc, "There are no valid responses.", ERROR_DESC_LENGTH);
        goto EXIT;
    }
    /* Adding the read response to receiver Q */
    add_to_recvQ(resultMsg);
    for (size_t i = 0; i < reqLen; i++)
    {
        UA_NodeId_deleteMembers(&rv[i].nodeId);
    }
    UA_ReadValueId_deleteMembers(rv);
    EdgeFree(rv);
    UA_ReadResponse_deleteMembers(&readResponse);
    return;

    EXIT:
    /* Free the memory */
    sendErrorResponse(msg, errorDesc);
    freeEdgeMessage(resultMsg);
    for (size_t i = 0; i < reqLen; i++)
    {
        UA_NodeId_deleteMembers(&rv[i].nodeId);
    }
    UA_ReadValueId_deleteMembers(rv);
    EdgeFree(rv);
    UA_ReadResponse_deleteMembers(&readResponse);
}

EdgeResult executeRead(UA_Client *client, const EdgeMessage *msg)
{
    EdgeResult result;
    result.code = STATUS_ERROR;
    VERIFY_NON_NULL_MSG(client, "Client param is NULL in execute READ\n", result);

    if (CMD_READ == msg->command)
    {
        /* Read value attribute */
        readGroup(client, msg, UA_ATTRIBUTEID_VALUE);
    }
    else if (CMD_READ_SAMPLING_INTERVAL == msg->command)
    {
        /* Read sampling interval attribute */
        readGroup(client,msg, UA_ATTRIBUTEID_MINIMUMSAMPLINGINTERVAL);
    }

    result.code = STATUS_OK;
    return result;
}
