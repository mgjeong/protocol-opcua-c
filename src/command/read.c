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
#include "common_client.h"
#include "message_dispatcher.h"
#include "edge_logger.h"
#include "edge_malloc.h"

#include <inttypes.h>

#define TAG "read"

#define GUID_LENGTH (36)

UA_Int64 DateTime_toUnixTime(UA_DateTime date)
{
    return (date - UA_DATETIME_UNIX_EPOCH) / UA_DATETIME_MSEC;
}

static void sendErrorResponse(const EdgeMessage *msg, char *err_desc)
{
    EdgeMessage *resultMsg = (EdgeMessage *) EdgeCalloc(1, sizeof(EdgeMessage));
    VERIFY_NON_NULL_NR(resultMsg);
    resultMsg->endpointInfo = cloneEdgeEndpointInfo(msg->endpointInfo);
    resultMsg->type = ERROR;
    resultMsg->responseLength = 1;
    resultMsg->message_id = msg->message_id;

    EdgeResponse** responses = (EdgeResponse **) malloc(sizeof(EdgeResponse *) * resultMsg->responseLength);
    for (int i = 0; i < resultMsg->responseLength; i++)
    {
        responses[i] = (EdgeResponse*) EdgeCalloc(1, sizeof(EdgeResponse));
        EdgeVersatility *message = (EdgeVersatility *) EdgeCalloc(1, sizeof(EdgeVersatility));
        if(IS_NULL(message))
        {
            EDGE_LOG(TAG, "Error : Malloc failed for EdgeVersatility sendErrorResponse\n");
            goto EXIT;
        }
        char* err_description = (char*) EdgeMalloc(strlen(err_desc) + 1);
        strncpy(err_description, err_desc, strlen(err_desc));
        err_description[strlen(err_desc)] = '\0';
        message->value = (void *) err_description;
        responses[i]->message = message;
    }
    resultMsg->responses = responses;

    EdgeResult *res = (EdgeResult *) EdgeMalloc(sizeof(EdgeResult));
    if(IS_NULL(res))
    {
        EDGE_LOG(TAG, "Error : Malloc failed for EdgeResult sendErrorResponse\n");
        goto EXIT;
    }
    res->code = STATUS_ERROR;
    resultMsg->result = res;

    add_to_recvQ(resultMsg);
    return ;

    EXIT:
    EdgeFree(res);    
    if(IS_NOT_NULL(resultMsg))
    {
        if(IS_NOT_NULL(resultMsg->responses))
        {
            for (int i = 0; i < resultMsg->responseLength; i++)
            {
                EdgeFree(resultMsg->responses[i]->message);
                EdgeFree(resultMsg->responses[i]);
            }
            EdgeFree(resultMsg->responses);
        }
        EdgeFree(resultMsg->endpointInfo);
        EdgeFree(resultMsg);
    }
}

static bool checkMaxAge(UA_DateTime timestamp, UA_DateTime now, double maxAge)
{

    // Server timestamp is greater than current time
    if (timestamp > now)
        return false;

    int64_t second = DateTime_toUnixTime(now);
    int64_t first = DateTime_toUnixTime(timestamp);

    int64_t diff = second - first;

    if ((maxAge != 0) && (diff > maxAge))
        return false;

    return true;
}

static bool compareNumber(int64_t number1, int64_t number2)
{
    if (number1 > number2)
        return true;

    return false;
}

static bool checkInvalidTime(UA_DateTime serverTime, UA_DateTime sourceTime, int validMilliSec,
        UA_TimestampsToReturn stamp)
{
    bool ret = true;
    UA_DateTime now = UA_DateTime_now();
    int64_t now_time = UA_DateTime_toUnixTime(now);
    int64_t server_time = UA_DateTime_toUnixTime(serverTime);
    int64_t source_time = UA_DateTime_toUnixTime(sourceTime);

    if (UA_TIMESTAMPSTORETURN_BOTH == stamp)
    {
        if ((0 == server_time) || (0 == source_time))
        {
            EDGE_LOG(TAG, "Invalid timestamp\n\n");
            ret = false;
        }

        // compare number
        if (compareNumber(now_time - server_time, validMilliSec))
        {
            ret = false;
        }
        else if (compareNumber(now_time - source_time, validMilliSec))
        {
            ret = false;
        }
        else if (compareNumber(server_time, now_time))
        {
            ret = false;
        }
        else if (compareNumber(source_time, now_time))
        {
            ret = false;
        }
    }
    else if (UA_TIMESTAMPSTORETURN_SOURCE == stamp)
    {
        if (0 == source_time)
        {
            EDGE_LOG(TAG, "invalid source timestamp\n\n");
            return false;
        }

        // compare number
        if (compareNumber(now_time - source_time, validMilliSec))
        {
            ret = false;
        }
        else if (compareNumber(source_time, now_time))
        {
            ret = false;
        }
    }
    else if (UA_TIMESTAMPSTORETURN_SERVER == stamp)
    {
        if (0 == server_time)
        {
            EDGE_LOG(TAG, "invalid server timestamp\n\n");
            return false;
        }

        // compare number
        if (compareNumber(now_time - server_time, validMilliSec))
        {
            ret = false;
        }
        else if (compareNumber(server_time, now_time))
        {
            ret = false;
        }
    }

    return ret;
}

static void *checkValidation(UA_DataValue *value, const EdgeMessage *msg, UA_TimestampsToReturn stamp,
        double maxAge)
{

    // if (!checkNaN(value))

    if (!checkInvalidTime(value->serverTimestamp, value->sourceTimestamp, 86400000, stamp))
    {
        // Error message handling
        sendErrorResponse(msg, "Invalid Time");
        return NULL;
    }

    if (UA_STATUSCODE_GOOD != value->status)
    {
        // Error message handling
        sendErrorResponse(msg, "Error status code from server");
        return NULL;
    }

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

static EdgeDiagnosticInfo *checkDiagnosticInfo(int nodesToProcess,
        UA_DiagnosticInfo *diagnosticInfo, int diagnosticInfoLength, int returnDiagnostic)
{

    EdgeDiagnosticInfo *diagnostics = (EdgeDiagnosticInfo *) EdgeMalloc(sizeof(EdgeDiagnosticInfo));
    VERIFY_NON_NULL(diagnostics, NULL);
    diagnostics->symbolicId = 0;
    diagnostics->localizedText = 0;
    diagnostics->additionalInfo = NULL;
    diagnostics->innerDiagnosticInfo = NULL;
    if (0 == returnDiagnostic && 0 == diagnosticInfoLength)
    {
        diagnostics->msg = NULL;
    }
    else if (diagnosticInfoLength == nodesToProcess)
    {
        diagnostics->symbolicId = diagnosticInfo[0].symbolicId;
        diagnostics->localizedText = diagnosticInfo[0].localizedText;
        diagnostics->locale = diagnosticInfo[0].locale;
        if (diagnosticInfo[0].hasAdditionalInfo)
        {
            char *additional_info = (char *) EdgeMalloc(diagnosticInfo[0].additionalInfo.length + 1);
            if(IS_NULL(additional_info))
            {
                EDGE_LOG(TAG, "Error : Malloc for additional_info failed in checkDiagnosticInfo");
                diagnostics->msg = (void *) "mismatch entries returned";
                return diagnostics;
            }
            strncpy(additional_info, (char *) (diagnosticInfo[0].additionalInfo.data),
                strlen((char *) (diagnosticInfo[0].additionalInfo.data)));
            additional_info[diagnosticInfo[0].additionalInfo.length] = '\0';
            diagnostics->additionalInfo = additional_info;
        }
        if (diagnosticInfo[0].hasInnerDiagnosticInfo)
            diagnostics->innerDiagnosticInfo = diagnosticInfo[0].innerDiagnosticInfo;
    }
    else if (0 != returnDiagnostic && 0 == diagnosticInfoLength)
    {
        diagnostics->msg =
                (void *) "no diagnostics were returned even though returnDiagnostic requested";
    }
    else
    {
        diagnostics->msg = (void *) "mismatch entries returned";
    }
    return diagnostics;
}

static void readGroup(UA_Client *client, const EdgeMessage *msg)
{
    size_t reqLen = msg->requestLength;
    UA_ReadValueId *rv = (UA_ReadValueId *) EdgeMalloc(sizeof(UA_ReadValueId) * reqLen);

    for (size_t i = 0; i < reqLen; i++)
    {
        EDGE_LOG_V(TAG, "[READGROUP] Node to read :: %s\n", msg->requests[i]->nodeInfo->valueAlias);
        UA_ReadValueId_init(&rv[i]);
        rv[i].attributeId = UA_ATTRIBUTEID_VALUE;
        rv[i].nodeId = UA_NODEID_STRING(msg->requests[i]->nodeInfo->nodeId->nameSpace,
                msg->requests[i]->nodeInfo->valueAlias);
    }

    UA_ReadRequest readRequest;
    UA_ReadRequest_init(&readRequest);
    readRequest.nodesToRead = rv;
    readRequest.nodesToReadSize = reqLen;
    readRequest.maxAge = 2000;
    readRequest.timestampsToReturn = UA_TIMESTAMPSTORETURN_BOTH;

    //UA_RequestHeader_init(&(readRequest.requestHeader));
    //readRequest.requestHeader.returnDiagnostics = 1;

    UA_ReadResponse readResponse = UA_Client_Service_read(client, readRequest);
    if (readResponse.responseHeader.serviceResult != UA_STATUSCODE_GOOD)
    {
        // send error callback;
        EDGE_LOG_V(TAG, "Error in group read :: 0x%08x(%s)\n", readResponse.responseHeader.serviceResult,
                UA_StatusCode_name(readResponse.responseHeader.serviceResult));

        sendErrorResponse(msg, "Error in read");
        UA_ReadValueId_deleteMembers(rv);
        UA_ReadResponse_deleteMembers(&readResponse);
        return;
    }

    if (readResponse.results[0].status == UA_STATUSCODE_GOOD)
    {
        if (readRequest.timestampsToReturn == UA_TIMESTAMPSTORETURN_NEITHER)
        {
            if (readResponse.results[0].hasSourceTimestamp
                    || readResponse.results[0].hasServerTimestamp)
            {
                EDGE_LOG(TAG, "BadInvalidTimestamp\n\n");
                sendErrorResponse(msg, "BadInvalidTimestamp");
                UA_ReadValueId_deleteMembers(rv);
                return;
            }
        }
        if (readRequest.timestampsToReturn == UA_TIMESTAMPSTORETURN_BOTH)
        {
            if (!readResponse.results[0].hasSourceTimestamp
                    || !readResponse.results[0].hasServerTimestamp)
            {
                EDGE_LOG(TAG, "Timestamp missing\n\n");
                sendErrorResponse(msg, "Missing Timestamp");
                UA_ReadValueId_deleteMembers(rv);
                return;
            }
        }
        if (readRequest.timestampsToReturn == UA_TIMESTAMPSTORETURN_SOURCE)
        {
            if (!readResponse.results[0].hasSourceTimestamp
                    || readResponse.results[0].hasServerTimestamp)
            {
                EDGE_LOG(TAG, "source Timestamp missing\n\n");
                sendErrorResponse(msg, "Missing Timestamp");
                UA_ReadValueId_deleteMembers(rv);
                return;
            }
        }
        if (readRequest.timestampsToReturn == UA_TIMESTAMPSTORETURN_SERVER)
        {
            if (readResponse.results[0].hasSourceTimestamp
                    || !readResponse.results[0].hasServerTimestamp)
            {
                EDGE_LOG(TAG, "server Timestamp missing\n\n");
                sendErrorResponse(msg, "Missing Timestamp");
                UA_ReadValueId_deleteMembers(rv);
                return;
            }
        }

        if (readRequest.timestampsToReturn != UA_TIMESTAMPSTORETURN_NEITHER
                && !checkMaxAge(readResponse.results[0].serverTimestamp, UA_DateTime_now(),
                        readRequest.maxAge * 2))
        {
            EDGE_LOG(TAG, "Max age failed\n\n");
            sendErrorResponse(msg, "");
            UA_ReadValueId_deleteMembers(rv);
            UA_ReadResponse_deleteMembers(&readResponse);
            return;
        }

        if (readRequest.timestampsToReturn != UA_TIMESTAMPSTORETURN_NEITHER
                && !checkValidation(&(readResponse.results[0]), msg, readRequest.timestampsToReturn,
                        readRequest.maxAge))
        {
            UA_ReadValueId_deleteMembers(rv);
            UA_ReadResponse_deleteMembers(&readResponse);
            return;
        }
    }

    int respIndex = 0;
    EdgeResponse **responses = (EdgeResponse **) malloc(sizeof(EdgeResponse *) * reqLen);
    if(IS_NULL(responses))
    {
        EDGE_LOG(TAG, "Error : Malloc failed for responses in Read Group\n");
        goto EXIT;
    }
    EdgeMessage *resultMsg = (EdgeMessage *) EdgeCalloc(1, sizeof(EdgeMessage));
    if(IS_NULL(resultMsg))
    {
        EDGE_LOG(TAG, "Error : Malloc failed for resultMsg in Read Group\n");
        goto EXIT;
    }

    resultMsg->responseLength = 0;
    resultMsg->command = CMD_READ;
    resultMsg->type = GENERAL_RESPONSE;
    resultMsg->message_id = msg->message_id;
    resultMsg->endpointInfo = cloneEdgeEndpointInfo(msg->endpointInfo);
    if(IS_NULL(resultMsg->endpointInfo))
    {
        EDGE_LOG(TAG, "Error : EdgeCalloc failed for resultMsg.endpointInfo in Read Group\n");
        goto EXIT;
    }

    for (int i = 0; i < reqLen; i++)
    {
        responses[i] = NULL;
        if (readResponse.results[i].status == UA_STATUSCODE_GOOD)
        {
            UA_Variant val = readResponse.results[i].value;
            bool isScalar = UA_Variant_isScalar(&val);
            EdgeResponse *response = (EdgeResponse *) EdgeCalloc(1, sizeof(EdgeResponse));
            if (IS_NOT_NULL(response))
            {
                response->nodeInfo = cloneEdgeNodeInfo(msg->requests[i]->nodeInfo);
                if(IS_NULL(response->nodeInfo))
                {
                    EDGE_LOG(TAG, "Error : Malloc failed for response.Nodeinfo in Read Group\n");
                    goto EXIT;
                }
                response->requestId = msg->requests[i]->requestId;

                EdgeVersatility *versatility = (EdgeVersatility *) EdgeMalloc(sizeof(EdgeVersatility));
                if(IS_NULL(versatility))
                {
                    EDGE_LOG(TAG, "Error : Malloc failed for EdgeVersatility in Read Group\n");
                    goto EXIT;
                }
                if (isScalar)
                {
                    versatility->arrayLength = 0;
                    versatility->isArray = false;
                }
                else
                {
                    versatility->arrayLength = val.arrayLength;
                    versatility->isArray = true;
                }

                if (val.type == &UA_TYPES[UA_TYPES_BOOLEAN])
                {
                    response->type = Boolean;
                }
                else if (val.type == &UA_TYPES[UA_TYPES_INT16])
                {
                    response->type = Int16;
                }
                else if (val.type == &UA_TYPES[UA_TYPES_UINT16])
                {
                    response->type = UInt16;
                }
                else if (val.type == &UA_TYPES[UA_TYPES_INT32])
                {
                    response->type = Int32;
                }
                else if (val.type == &UA_TYPES[UA_TYPES_UINT32])
                {
                    response->type = UInt32;
                }
                else if (val.type == &UA_TYPES[UA_TYPES_INT64])
                {
                    response->type = Int64;
                }
                else if (val.type == &UA_TYPES[UA_TYPES_UINT64])
                {
                    response->type = UInt64;
                }
                else if (val.type == &UA_TYPES[UA_TYPES_FLOAT])
                {
                    response->type = Float;
                }
                else if (val.type == &UA_TYPES[UA_TYPES_DOUBLE])
                {
                    response->type = Double;
                }
                else if (val.type == &UA_TYPES[UA_TYPES_STRING])
                {
                    response->type = String;
                }
                else if (val.type == &UA_TYPES[UA_TYPES_BYTESTRING])
                {
                    response->type = ByteString;
                }
                else if (val.type == &UA_TYPES[UA_TYPES_GUID])
                {
                    response->type = Guid;
                }
                else if (val.type == &UA_TYPES[UA_TYPES_SBYTE])
                {
                    response->type = SByte;
                }
                else if (val.type == &UA_TYPES[UA_TYPES_BYTE])
                {
                    response->type = Byte;
                }
                else if (val.type == &UA_TYPES[UA_TYPES_DATETIME])
                {
                    response->type = DateTime;
                }

                if (isScalar)
                {
                    size_t size = get_size(response->type, false);
                    if ((response->type == String) || (response->type == ByteString))
                    {
                        UA_String str = *((UA_String *) val.data);
                        size_t len = str.length;
                        versatility->value = (void *) EdgeCalloc(1, len+1);
                        strncpy(versatility->value, (char*) str.data, len);
                       ((char*) versatility->value)[(int) len] = '\0';
                    }
                    else if (response->type == Guid)
                    {
                        UA_Guid str = *((UA_Guid *) val.data);
                        char *value = (char *) EdgeMalloc(GUID_LENGTH + 1);
                        if(IS_NULL(value))
                        {
                            EDGE_LOG(TAG, "Error : Malloc failed for Guid SCALAR value in Read Group\n");
                            goto EXIT;
                        }
                        snprintf(value, (GUID_LENGTH / 2) + 1, "%08x-%04x-%04x", str.data1,
                                str.data2, str.data3);
                        sprintf(value, "%s-%02x", value, str.data4[0]);
                        sprintf(value, "%s%02x", value, str.data4[1]);
                        sprintf(value, "%s-", value);
                        for (int j = 2; j < 8; j++)
                            sprintf(value, "%s%02x", value, str.data4[j]);
                        value[GUID_LENGTH] = '\0';
                        versatility->value = (void *) value;
                        EDGE_LOG_V(TAG, "%s\n", value);
                    }
                    else
                    {
                        versatility->value = (void *) EdgeCalloc(1, size);
                        memcpy(versatility->value, val.data, size);
                    }
                }
                else
                {
                    size_t size = get_size(response->type, true);
                    if (response->type == String)
                    {
                        // String Array
                        UA_String *str = ((UA_String *) val.data);
                        char **values = (char **) malloc(sizeof(char *) * val.arrayLength);
                        if(IS_NULL(values))
                        {
                            EDGE_LOG(TAG, "Error : Malloc failed for String Array values in Read Group\n");
                            goto EXIT;
                        }
                        for (int i = 0; i < val.arrayLength; i++)
                        {
                            values[i] = (char *) EdgeMalloc(str[i].length+1);
                            if(IS_NULL(values[i]))
                            {
                                EDGE_LOG_V(TAG, "Error : Malloc failed for String Array value %d in Read Group\n", i);
                                goto EXIT;
                            }
                            strncpy(values[i], (char *) str[i].data, str[i].length);
                            values[i][str[i].length] = '\0';
                        }
                        versatility->value = (void *) values;
                    }
                    else if (response->type == ByteString)
                    {
                        // ByteString Array
                        UA_ByteString *str = ((UA_ByteString *) val.data);
                        char **values = (char **) malloc(sizeof(char *) * val.arrayLength);
                        if(IS_NULL(values))
                        {
                            EDGE_LOG(TAG, "Error : Malloc failed for ByteString Array value in Read Group\n");
                            goto EXIT;
                        }
                        for (int i = 0; i < val.arrayLength; i++)
                        {
                            values[i] = (char *) EdgeMalloc(str[i].length + 1);
                            if(IS_NULL(values[i]))
                            {
                                EDGE_LOG_V(TAG, "Error : Malloc failed for ByteString Array value %d in Read Group\n", i);
                                goto EXIT;
                            }
                            strncpy(values[i], (char *) str[i].data, str[i].length);
                            values[i][str[i].length] = '\0';
                        }
                        versatility->value = (void *) values;
                    }
                    else if (response->type == Guid)
                    {
                        // Guid Array
                        UA_Guid *str = ((UA_Guid *) val.data);
                        char **values = (char **) malloc(sizeof(char *) * val.arrayLength);
                        if(IS_NULL(values))
                        {
                            EDGE_LOG(TAG, "Error : Malloc failed for Guid Array values in Read Group\n");
                            goto EXIT;
                        }
                        for (int i = 0; i < val.arrayLength; i++)
                        {
                            values[i] = (char *) EdgeMalloc(GUID_LENGTH + 1);
                            if(IS_NULL(values[i]))
                            {
                                EDGE_LOG_V(TAG, "Error : Malloc failed for Guid Array value %d in Read Group\n", i);
                                goto EXIT;
                            }
                            snprintf(values[i], (GUID_LENGTH / 2) + 1, "%08x-%04x-%04x",
                                    str[i].data1, str[i].data2, str[i].data3);
                            sprintf(values[i], "%s-%02x", values[i], str[i].data4[0]);
                            sprintf(values[i], "%s%02x", values[i], str[i].data4[1]);
                            sprintf(values[i], "%s-", values[i]);
                            for (int j = 2; j < 8; j++)
                                sprintf(values[i], "%s%02x", values[i], str[i].data4[j]);
                            values[GUID_LENGTH] = '\0';
                            EDGE_LOG_V(TAG, "%s\n", values[i]);
                        }
                        versatility->value = (void *) values;
                    }
                    else
                    {
                        versatility->value = (void *) EdgeCalloc(versatility->arrayLength, size);
                        memcpy(versatility->value, val.data, get_size(response->type, false) * versatility->arrayLength);
                    }
                }

                response->message = versatility;
                // ctt check
                EdgeDiagnosticInfo *diagnosticInfo = checkDiagnosticInfo(msg->requestLength,
                        readResponse.diagnosticInfos, readResponse.diagnosticInfosSize,
                        readRequest.requestHeader.returnDiagnostics);
                response->m_diagnosticInfo = diagnosticInfo;

                resultMsg->responseLength += 1;
                responses[respIndex] = response;
                respIndex += 1;

            }
        }
        else
        {
            // failure read response for this particular node
            EDGE_LOG_V(TAG, "Error in group read response for particular node :: 0x%08x(%s)\n",
                    readResponse.results[i].status, UA_StatusCode_name(readResponse.results[i].status));
            sendErrorResponse(msg, "Error in read response");
        }
    }

    if (resultMsg->responseLength > 0)
    {
        resultMsg->responses = responses;
        add_to_recvQ(resultMsg);
    }

    return ;

    EXIT:

    if(IS_NOT_NULL(resultMsg))
    {
        for (int i = 0; i < resultMsg->responseLength; i++)
        {
            if (responses[i]->type == String || responses[i]->type == ByteString)
            {
                if (responses[i]->message->isArray)
                {
                    // Free String array
                    char **values = responses[i]->message->value;
                    if (values)
                    {
                        for (int j = 0; j < responses[i]->message->arrayLength; j++)
                        {
                            EdgeFree(values[j]);
                        }
                        EdgeFree(values);
                    }
                }
            }
            if (IS_NOT_NULL(responses[i]->m_diagnosticInfo->additionalInfo))
            {
                EdgeFree(responses[i]->m_diagnosticInfo->additionalInfo);
            }
            EdgeFree(responses[i]->m_diagnosticInfo);
            EdgeFree(responses[i]->nodeInfo);
            EdgeFree(responses[i]->message);
            EdgeFree(responses[i]);
        }
        EdgeFree(resultMsg->endpointInfo);
        EdgeFree(resultMsg->responses);
        EdgeFree(resultMsg);
    }

    EdgeFree(rv);
    UA_ReadResponse_deleteMembers(&readResponse);

    return;
}

EdgeResult executeRead(UA_Client *client, const EdgeMessage *msg)
{
    EdgeResult result;
    if (!client)
    {
        EDGE_LOG(TAG, "Client handle Invalid\n");
        result.code = STATUS_ERROR;
        return result;
    }
    readGroup(client, msg);

    result.code = STATUS_OK;
    return result;
}
