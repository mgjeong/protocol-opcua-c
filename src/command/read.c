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

#include <inttypes.h>

#define TAG "read"

#define GUID_LENGTH (36)
#define ERROR_DESC_LENGTH (100)

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

    resultMsg->responses = (EdgeResponse **) malloc(sizeof(EdgeResponse *) * resultMsg->responseLength);
    for (int i = 0; i < resultMsg->responseLength; i++)
    {
        resultMsg->responses[i] = (EdgeResponse*) EdgeCalloc(1, sizeof(EdgeResponse));
        resultMsg->responses[i]->message = (EdgeVersatility *) EdgeCalloc(1, sizeof(EdgeVersatility));
        if(IS_NULL(resultMsg->responses[i]->message))
        {
            EDGE_LOG(TAG, "Error : Malloc failed for EdgeVersatility sendErrorResponse\n");
            goto EXIT;
        }
        char* err_description = (char*) EdgeMalloc(strlen(err_desc) + 1);
        strncpy(err_description, err_desc, strlen(err_desc));
        err_description[strlen(err_desc)] = '\0';
        resultMsg->responses[i]->message->value = (void *) err_description;
    }
    resultMsg->result = (EdgeResult *) EdgeMalloc(sizeof(EdgeResult));
    if(IS_NULL(resultMsg->result))
    {
        EDGE_LOG(TAG, "Error : Malloc failed for EdgeResult sendErrorResponse\n");
        goto EXIT;
    }
    resultMsg->result->code = STATUS_ERROR;

    add_to_recvQ(resultMsg);
    return ;

    EXIT:
    freeEdgeMessage(resultMsg);
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
        if (compareNumber(now_time - server_time, validMilliSec) ||
                compareNumber(now_time - source_time, validMilliSec) ||
                compareNumber(server_time, now_time) ||
                compareNumber(source_time, now_time))
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
        if (compareNumber(now_time - source_time, validMilliSec) ||
                compareNumber(source_time, now_time))
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
        if (compareNumber(now_time - server_time, validMilliSec) ||
                compareNumber(server_time, now_time))
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
        EDGE_LOG_V(TAG, "[READGROUP] Node to read :: %s\n", msg->requests[i]->nodeInfo->valueAlias);
        UA_ReadValueId_init(&rv[i]);
        rv[i].attributeId = attributeId;
        rv[i].nodeId = UA_NODEID_STRING_ALLOC(msg->requests[i]->nodeInfo->nodeId->nameSpace,
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
        strncpy(errorDesc, "Error in read.", ERROR_DESC_LENGTH);
        goto EXIT;
    }

    if (readResponse.results[0].status == UA_STATUSCODE_GOOD)
    {
        if(UA_ATTRIBUTEID_VALUE == attributeId) {
            if (readRequest.timestampsToReturn == UA_TIMESTAMPSTORETURN_NEITHER)
            {
                if (readResponse.results[0].hasSourceTimestamp
                        || readResponse.results[0].hasServerTimestamp)
                {
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
                    EDGE_LOG(TAG, "server Timestamp missing\n\n");
                    strncpy(errorDesc, "server Timestamp missing.", ERROR_DESC_LENGTH);
                    goto EXIT;
                }
            }

            if (readRequest.timestampsToReturn != UA_TIMESTAMPSTORETURN_NEITHER
                    && !checkMaxAge(readResponse.results[0].serverTimestamp, UA_DateTime_now(),
                            readRequest.maxAge * 2))
            {
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
            response->message = (EdgeVersatility *) EdgeCalloc(1, sizeof(EdgeVersatility));
            if(IS_NULL(response->message))
            {
                EDGE_LOG(TAG, "Error : Malloc failed for EdgeVersatility in Read Group\n");
                strncpy(errorDesc, "Memory allocation failed.", ERROR_DESC_LENGTH);
                freeEdgeResponse(response);
                goto EXIT;
            }

            EdgeVersatility *versatility = (EdgeVersatility *) response->message;
            bool isScalar = UA_Variant_isScalar(&val);
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
            response->type = get_response_type(val.type);

            if (isScalar)
            {
                size_t size = get_size(response->type, false);
                if ((response->type == UA_NS0ID_STRING) || (response->type == UA_NS0ID_BYTESTRING)
                		|| (response->type == UA_NS0ID_XMLELEMENT))
                {
                    UA_String str = *((UA_String *) val.data);
                    size_t len = str.length;
                    versatility->value = (void *) EdgeCalloc(1, len+1);
                    if(IS_NULL(versatility->value))
                    {
                        EDGE_LOG(TAG, "Memory allocation failed.");
                        strncpy(errorDesc, "Memory allocation failed.", ERROR_DESC_LENGTH);
                        freeEdgeResponse(response);
                        goto EXIT;
                    }
                    strncpy(versatility->value, (char*) str.data, len);
                   ((char*) versatility->value)[(int) len] = '\0';
                }
                else if (response->type == UA_NS0ID_GUID)
                {
                    UA_Guid str = *((UA_Guid *) val.data);
                    char *value = (char *) EdgeMalloc(GUID_LENGTH + 1);
                    if(IS_NULL(value))
                    {
                        EDGE_LOG(TAG, "Error : Malloc failed for Guid SCALAR value in Read Group\n");
                        strncpy(errorDesc, "Memory allocation failed.", ERROR_DESC_LENGTH);
                        freeEdgeResponse(response);
                        goto EXIT;
                    }

                    snprintf(value, GUID_LENGTH + 1, "%08x-%04x-%04x-%02x%02x-%02x%02x%02x%02x%02x%02x",
                            str.data1, str.data2, str.data3, str.data4[0], str.data4[1], str.data4[2],
                            str.data4[3], str.data4[4], str.data4[5], str.data4[6], str.data4[7]);
                    versatility->value = (void *) value;

                    EDGE_LOG_V(TAG, "%s\n", value);
                }
                else if(response->type == UA_NS0ID_LOCALIZEDTEXT)
                {
                    UA_LocalizedText lt = *((UA_LocalizedText *) val.data);
                    Edge_LocalizedText *value = (Edge_LocalizedText *) EdgeCalloc(1, sizeof(Edge_LocalizedText));
                    if(IS_NULL(value))
                    {
                        EDGE_LOG(TAG, "Memory allocation failed.");
                        strncpy(errorDesc, "Memory allocation failed.", ERROR_DESC_LENGTH);
                        freeEdgeResponse(response);
                        goto EXIT;
                    }
                    versatility->value = value;

                    if(lt.locale.length > 0)
                    {
                        value->locale.data= (uint8_t*) EdgeCalloc(lt.locale.length+1, sizeof(uint8_t));
                        if(IS_NULL(value->locale.data))
                        {
                            EDGE_LOG(TAG, "Memory allocation failed.");
                            strncpy(errorDesc, "Memory allocation failed.", ERROR_DESC_LENGTH);
                            freeEdgeResponse(response);
                            goto EXIT;
                        }
                        value->locale.length = lt.locale.length;
                        strncpy((char *)value->locale.data, (char *)lt.locale.data, lt.locale.length);
                    }

                    if(lt.text.length > 0)
                    {
                        value->text.data= (uint8_t*) EdgeCalloc(lt.text.length+1, sizeof(uint8_t));
                        if(IS_NULL(value->text.data))
                        {
                            EDGE_LOG(TAG, "Memory allocation failed.");
                            strncpy(errorDesc, "Memory allocation failed.", ERROR_DESC_LENGTH);
                            freeEdgeResponse(response);
                            goto EXIT;
                        }
                        value->text.length = lt.text.length;
                        strncpy((char *)value->text.data, (char *)lt.text.data, lt.text.length);
                    }
                }
                else
                {
                    versatility->value = (void *) EdgeCalloc(1, size);
                    if(IS_NULL(versatility->value))
                    {
                        EDGE_LOG(TAG, "Memory allocation failed.");
                        strncpy(errorDesc, "Memory allocation failed.", ERROR_DESC_LENGTH);
                        freeEdgeResponse(response);
                        goto EXIT;
                    }
                    memcpy(versatility->value, val.data, size);
                }
            }
            else
            {
                size_t size = get_size(response->type, true);
                if (response->type == UA_NS0ID_STRING || response->type == UA_NS0ID_BYTESTRING
                		|| response->type == UA_NS0ID_XMLELEMENT)
                {
                    // String Array
                    UA_String *str = ((UA_String *) val.data);
                    versatility->value = EdgeCalloc (val.arrayLength, sizeof(char *));
                    if(IS_NULL(versatility->value))
                    {
                        EDGE_LOG(TAG, "Error : Malloc failed for String Array values in Read Group\n");
                        strncpy(errorDesc, "Memory allocation failed.", ERROR_DESC_LENGTH);
                        freeEdgeResponse(response);
                        goto EXIT;
                    }

                    char **values = (char **) versatility->value;
                    for (int j = 0; j < val.arrayLength; j++)
                    {
                        values[j] = (char *) EdgeMalloc(str[j].length + 1);
                        if(IS_NULL(values[j]))
                        {
                            EDGE_LOG_V(TAG, "Error : Malloc failed for ByteString Array value %d in Read Group\n", i);
                            strncpy(errorDesc, "Memory allocation failed.", ERROR_DESC_LENGTH);
                            freeEdgeResponse(response);
                            goto EXIT;
                        }
                        strncpy(values[j], (char *) str[j].data, str[j].length);
                        values[j][str[j].length] = '\0';
                    }
                }
                else if (response->type == UA_NS0ID_GUID)
                {
                    // Guid Array
                    UA_Guid *str = ((UA_Guid *) val.data);
                    versatility->value = EdgeCalloc(val.arrayLength, sizeof(char *));
                    if(IS_NULL(versatility->value))
                    {
                        EDGE_LOG(TAG, "Error : Malloc failed for Guid Array values in Read Group\n");
                        strncpy(errorDesc, "Memory allocation failed.", ERROR_DESC_LENGTH);
                        freeEdgeResponse(response);
                        goto EXIT;
                    }

                    char **values = (char **) versatility->value;
                    for (int j = 0; j < val.arrayLength; j++)
                    {
                        values[j] = (char *) EdgeMalloc(GUID_LENGTH + 1);
                        if(IS_NULL(values[j]))
                        {
                            EDGE_LOG_V(TAG, "Error : Malloc failed for Guid Array value %d in Read Group\n", i);
                            strncpy(errorDesc, "Memory allocation failed.", ERROR_DESC_LENGTH);
                            freeEdgeResponse(response);
                            goto EXIT;
                        }

                        snprintf(values[j], GUID_LENGTH + 1, "%08x-%04x-%04x-%02x%02x-%02x%02x%02x%02x%02x%02x",
                                str[j].data1, str[j].data2, str[j].data3, str[j].data4[0], str[j].data4[1], str[j].data4[2],
                                str[j].data4[3], str[j].data4[4], str[j].data4[5], str[j].data4[6], str[j].data4[7]);

                        EDGE_LOG_V(TAG, "%s\n", values[j]);
                    }
                }
                else
                {
                    versatility->value = (void *) EdgeCalloc(versatility->arrayLength, size);
                    if(IS_NULL(versatility->value))
                    {
                        EDGE_LOG(TAG, "Memory allocation failed.");
                        strncpy(errorDesc, "Memory allocation failed.", ERROR_DESC_LENGTH);
                        freeEdgeResponse(response);
                        goto EXIT;
                    }

                    memcpy(versatility->value, val.data, get_size(response->type, false) * versatility->arrayLength);
                }
            }

            // ctt check
            response->m_diagnosticInfo = checkDiagnosticInfo(msg->requestLength,
                    readResponse.diagnosticInfos, readResponse.diagnosticInfosSize,
                    readRequest.requestHeader.returnDiagnostics);

            resultMsg->responseLength++;
            resultMsg->responses[respIndex++] = response;
        }
        else
        {
            // failure read response for this particular node
            EDGE_LOG_V(TAG, "Error in group read response for particular node :: 0x%08x(%s)\n",
                    readResponse.results[i].status, UA_StatusCode_name(readResponse.results[i].status));
            sendErrorResponse(msg, "Error in read response");
        }
    }

    if (reqLen > 1 && resultMsg->responseLength < 1)
    {
        EDGE_LOG(TAG, "There are no valid responses.");
        strncpy(errorDesc, "There are no valid responses.", ERROR_DESC_LENGTH);
        goto EXIT;
    }

    add_to_recvQ(resultMsg);
    UA_ReadValueId_deleteMembers(rv);
    EdgeFree(rv);
    UA_ReadResponse_deleteMembers(&readResponse);
    return;

    EXIT:
    sendErrorResponse(msg, errorDesc);
    freeEdgeMessage(resultMsg);
    UA_ReadValueId_deleteMembers(rv);
    EdgeFree(rv);
    UA_ReadResponse_deleteMembers(&readResponse);
}

EdgeResult executeRead(UA_Client *client, const EdgeMessage *msg)
{
    EdgeResult result;
    result.code = STATUS_ERROR;
    VERIFY_NON_NULL(client, result);
    
    if (CMD_READ == msg->command)
    {
        readGroup(client, msg, UA_ATTRIBUTEID_VALUE);
    }
    else if (CMD_READ_SAMPLING_INTERVAL == msg->command)
    {
        readGroup(client,msg, UA_ATTRIBUTEID_MINIMUMSAMPLINGINTERVAL);
    }

    result.code = STATUS_OK;
    return result;
}
