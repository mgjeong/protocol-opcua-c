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

#include "write.h"
#include "common_client.h"
#include "edge_logger.h"
#include "edge_malloc.h"
#include "message_dispatcher.h"

#include <inttypes.h>

#define TAG "write"

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

    //onResponseMessage(resultMsg);
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

static EdgeDiagnosticInfo *checkDiagnosticInfo(size_t nodesToProcess,
        UA_DiagnosticInfo *diagnosticInfo, size_t diagnosticInfoLength, int returnDiagnostic)
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
            char *additional_info = (char *) EdgeMalloc(diagnosticInfo[0].additionalInfo.length);
            if (IS_NULL(additional_info))
            {
                EDGE_LOG(TAG, "Error : Malloc failed for additional_info in checkDiagnosticInfo");
                diagnostics->msg = (void *) "mismatch entries returned";
                return diagnostics;
            }

            strncpy(additional_info, (char *) (diagnosticInfo[0].additionalInfo.data),
                    diagnosticInfo[0].additionalInfo.length);
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

static void writeGroup(UA_Client *client, const EdgeMessage *msg)
{
    size_t reqLen = msg->requestLength;
    UA_WriteValue *wv = (UA_WriteValue *) EdgeMalloc(sizeof(UA_WriteValue) * reqLen);
    VERIFY_NON_NULL_NR(wv);
    UA_Variant *myVariant = (UA_Variant *) EdgeMalloc(sizeof(UA_Variant) * reqLen);
    if (IS_NULL(myVariant))
    {
        EDGE_LOG(TAG, "Error : Malloc failed for myVariant in write group.");
        EdgeFree(wv);
        return;
    }

    for (size_t i = 0; i < reqLen; i++)
    {
        EDGE_LOG_V(TAG, "[WRITEGROUP] Node to write :: %s\n", msg->requests[i]->nodeInfo->valueAlias);
        EdgeNodeIdentifier Nodeid = msg->requests[i]->type;
        uint32_t type = (uint32_t) Nodeid - 1;
        UA_WriteValue_init(&wv[i]);
        UA_Variant_init(&myVariant[i]);
        wv[i].attributeId = UA_ATTRIBUTEID_VALUE;
        wv[i].nodeId = UA_NODEID_STRING(msg->requests[i]->nodeInfo->nodeId->nameSpace,
                msg->requests[i]->nodeInfo->valueAlias);
        wv[i].value.hasValue = true;
        wv[i].value.value.type = &UA_TYPES[type];
        wv[i].value.value.storageType = UA_VARIANT_DATA_NODELETE; /* do not free the integer on deletion */

        EdgeVersatility *message = (EdgeVersatility * ) msg->requests[i]->value;
        if (message->isArray == 0)
        {
            // scalar value to write
            if (type == UA_TYPES_STRING)
            {
                char *value_to_write = (char*) message->value;
                UA_String val = UA_STRING_ALLOC(value_to_write);
                UA_Variant_setScalarCopy(&myVariant[i], &val, &UA_TYPES[type]);
                UA_String_deleteMembers(&val);
                EdgeFree(val.data);
            }
            else if (type == UA_TYPES_BYTESTRING)
            {
                char *value_to_write = (char*) message->value;
                UA_ByteString val = UA_BYTESTRING_ALLOC(value_to_write);
                UA_Variant_setScalarCopy(&myVariant[i], &val, &UA_TYPES[type]);
                UA_String_deleteMembers(&val);
                EdgeFree(val.data);
            }
            else
            {
                UA_Variant_setScalarCopy(&myVariant[i], message->value, &UA_TYPES[type]);
            }
        }
        else
        {
            // array value to write
            if (type == UA_TYPES_STRING)
            {
                int idx = 0;
                char **data = (char **) message->value;
                UA_String *array = (UA_String *) UA_Array_new(message->arrayLength, &UA_TYPES[type]);
                for (idx = 0; idx < message->arrayLength; idx++)
                {
                    array[idx] = UA_STRING_ALLOC(data[idx]);
                }
                UA_Variant_setArrayCopy(&myVariant[i], array, message->arrayLength, &UA_TYPES[type]);
                for (idx = 0; idx < message->arrayLength; idx++)
                {
                    UA_String_deleteMembers(&array[idx]);
                }
                UA_Array_delete(array, message->arrayLength, &UA_TYPES[type]);
            }
            else if (type == UA_TYPES_BYTESTRING)
            {
                int idx = 0;
                UA_ByteString **dataArray = (UA_ByteString **) message->value;
                UA_ByteString *array = (UA_ByteString *) UA_Array_new(message->arrayLength, &UA_TYPES[type]);
                for (idx = 0; idx < message->arrayLength; idx++)
                {
                    char *data = (char *) dataArray[idx]->data;
                    array[idx] = UA_BYTESTRING_ALLOC(data);
                }
                UA_Variant_setArrayCopy(&myVariant[i], array, message->arrayLength, &UA_TYPES[type]);
                for (idx = 0; idx < message->arrayLength; idx++)
                {
                    UA_ByteString_deleteMembers(&array[idx]);
                }
                UA_Array_delete(array, message->arrayLength, &UA_TYPES[type]);
            }
            else
            {
                UA_Variant_setArrayCopy(&myVariant[i], message->value, message->arrayLength, &UA_TYPES[type]);
            }
        }
        wv[i].value.value = myVariant[i];
    }

    UA_WriteRequest writeRequest;
    UA_WriteRequest_init(&writeRequest);
    writeRequest.nodesToWrite = wv;
    writeRequest.nodesToWriteSize = reqLen;
    //writeRequest.requestHeader.returnDiagnostics = 1;

    UA_WriteResponse writeResponse = UA_Client_Service_write(client, writeRequest);
    if (writeResponse.responseHeader.serviceResult != UA_STATUSCODE_GOOD)
    {
        EDGE_LOG_V(TAG, "Error in write :: 0x%08x(%s)\n", writeResponse.responseHeader.serviceResult,
                UA_StatusCode_name(writeResponse.responseHeader.serviceResult));

        EdgeFree(wv);
        for (size_t i = 0; i < reqLen; i++)
            UA_Variant_deleteMembers(&myVariant[i]);
        EdgeFree(myVariant);

        UA_WriteResponse_deleteMembers(&writeResponse);
        return;
    }

    if (reqLen != writeResponse.resultsSize)
    {
        EDGE_LOG_V(TAG, "Requested(%d) but received(%d) => %s\n", (int) reqLen, (int)writeResponse.resultsSize,
                (reqLen < writeResponse.resultsSize) ? "Received more results" : "Received less results");
        sendErrorResponse(msg, "Error in write operation");

        EdgeFree(wv);
        for (size_t i = 0; i < reqLen; i++)
            UA_Variant_deleteMembers(&myVariant[i]);
        EdgeFree(myVariant);
        UA_WriteResponse_deleteMembers(&writeResponse);
        return;
    }

    size_t respIndex = 0;
    EdgeResponse **responses = (EdgeResponse **) malloc(sizeof(EdgeResponse *) * reqLen);
    if (IS_NULL(responses))
    {
        EDGE_LOG(TAG, "Error : Malloc Failed for responses in Write Group");
        goto EXIT;
    }
    EdgeMessage *resultMsg = (EdgeMessage *) EdgeCalloc(1, sizeof(EdgeMessage));
    if (IS_NULL(resultMsg))
    {
        EDGE_LOG(TAG, "Error : Malloc Failed for resultMsg in Write Group");
        goto EXIT;
    }
    resultMsg->endpointInfo = cloneEdgeEndpointInfo(msg->endpointInfo);
    if (IS_NULL(resultMsg->endpointInfo))
    {
        EDGE_LOG(TAG, "Error : Malloc Failed for resultMsg->endpointInfo in Write Group");
        goto EXIT;
    }
    resultMsg->responseLength = 0;
    resultMsg->command = CMD_WRITE;
    resultMsg->type = GENERAL_RESPONSE;
    resultMsg->message_id = msg->message_id;

    for (size_t i = 0; i < reqLen; i++)
    {
        responses[i] = NULL;
        UA_StatusCode code = writeResponse.results[i];

        if (code != UA_STATUSCODE_GOOD)
        {
            EDGE_LOG_V(TAG, "Error in write response for a particular node :: 0x%08x(%s)\n", code,
                    UA_StatusCode_name(code));

            sendErrorResponse(msg, "Error in write Response");

            if (writeResponse.responseHeader.serviceResult != UA_STATUSCODE_GOOD)
                continue;
        }
        else
        {
            EdgeResponse *response = (EdgeResponse *) EdgeCalloc(1, sizeof(EdgeResponse));
            if (IS_NOT_NULL(response))
            {
                response->nodeInfo = cloneEdgeNodeInfo(msg->requests[i]->nodeInfo);
                if (IS_NULL(response->nodeInfo))
                {
                    EDGE_LOG(TAG, "Error : Malloc Failed for EdgeResponse.NodeInfo in Write Group");
                    goto EXIT;
                }
                response->requestId = msg->requests[i]->requestId;
                response->message = NULL;

                EdgeDiagnosticInfo *diagnosticInfo = checkDiagnosticInfo(msg->requestLength,
                        writeResponse.diagnosticInfos, writeResponse.diagnosticInfosSize,
                        writeRequest.requestHeader.returnDiagnostics);
                response->m_diagnosticInfo = diagnosticInfo;

                EdgeVersatility *message = (EdgeVersatility *) EdgeCalloc(1, sizeof(EdgeVersatility));
                if (IS_NULL(message))
                {
                    EDGE_LOG(TAG, "Error : Malloc Failed for EdgeVersatility in Write Group");
                    goto EXIT;
                }
                const char *retCode = UA_StatusCode_name(code);
                size_t len = strlen(retCode);
                char *code = (char*) malloc(len+1);
                strncpy(code, retCode, len);
                code[len] = '\0';

                message->value = (void *) code;
                message->isArray = false;
                message->arrayLength = 0;
                response->message = message;

                resultMsg->responseLength += 1;
                responses[respIndex] = response;
                respIndex += 1;
            }
        }
    }

    if (resultMsg->responseLength > 0)
    {
        resultMsg->responses = responses;
        add_to_recvQ(resultMsg);
    }

    return ;

    EXIT:

    if (IS_NOT_NULL(resultMsg))
    {
        for (size_t i = 0; i < resultMsg->responseLength; i++)
        {
            if (responses[i]->m_diagnosticInfo)
            {
                if (responses[i]->m_diagnosticInfo->additionalInfo)
                {
                    EdgeFree(responses[i]->m_diagnosticInfo->additionalInfo);
                }
                EdgeFree(responses[i]->m_diagnosticInfo);
            }
            EdgeFree(responses[i]->message);
            EdgeFree(responses[i]->nodeInfo);
            EdgeFree(responses[i]);
        }

        EdgeFree(resultMsg->endpointInfo);
        EdgeFree(resultMsg->responses);
        EdgeFree(resultMsg);
    }
    EdgeFree(wv);
    for (size_t i = 0; i < reqLen; i++)
    {
        UA_Variant_deleteMembers(&myVariant[i]);
    }
    EdgeFree(myVariant);
    UA_WriteResponse_deleteMembers(&writeResponse);
}

EdgeResult executeWrite(UA_Client *client, const EdgeMessage *msg)
{
    EdgeResult result;
    if (!client)
    {
        EDGE_LOG(TAG, "Client handle Invalid\n");
        result.code = STATUS_ERROR;
        return result;
    }

    writeGroup(client, msg);

    result.code = STATUS_OK;
    return result;
}
