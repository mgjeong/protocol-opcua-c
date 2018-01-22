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

#include <stdio.h>
#include <inttypes.h>

#define TAG "write"

#if 0
static void write(UA_Client *client, EdgeMessage *msg)
{

    EDGE_LOG_V(TAG, "[WRITE] Node to write :: %s\n", msg->request->nodeInfo->valueAlias);
    UA_Variant *myVariant = UA_Variant_new();

    int type = (int)msg->request->type - 1;
    if (type == UA_TYPES_STRING)
    {
        UA_String val = UA_STRING_ALLOC((char *) msg->request->value);
        UA_Variant_setScalarCopy(myVariant, &val, &UA_TYPES[type]);
    }
    else
    {
        UA_Variant_setScalarCopy(myVariant, msg->request->value, &UA_TYPES[type]);
    }

    UA_StatusCode retVal = UA_Client_writeValueAttribute(client, UA_NODEID_STRING(1,
                    msg->request->nodeInfo->valueAlias), myVariant);
    if (retVal != UA_STATUSCODE_GOOD)
    {
        // send error callback;
        EDGE_LOG_V(TAG, "Error in read node :: 0x%08x\n", retVal);

        EdgeMessage *resultMsg = (EdgeMessage *) malloc(sizeof(EdgeMessage));
        resultMsg->endpointInfo = (EdgeEndPointInfo *) malloc(sizeof(EdgeEndPointInfo));
        memcpy(resultMsg->endpointInfo, msg->endpointInfo, sizeof(EdgeEndPointInfo));
        resultMsg->type = ERROR;
        resultMsg->responseLength = 0;

        EdgeResult *res = (EdgeResult *) malloc(sizeof(EdgeResult));
        res->code = STATUS_ERROR;
        resultMsg->result = res;

        onResponseMessage(resultMsg);
        UA_Variant_delete(myVariant);
        free(resultMsg);
        free(res);
        return;
    }

    EDGE_LOG(TAG, "[WRITE] SUCCESS response received from server\n");
    EdgeResponse *response = (EdgeResponse *) malloc(sizeof(EdgeResponse));
    if (response)
    {
        response->nodeInfo = (EdgeNodeInfo *) malloc(sizeof(EdgeNodeInfo));
        memcpy(response->nodeInfo, msg->request->nodeInfo, sizeof(EdgeNodeInfo));
        response->requestId = msg->request->requestId;
        response->message = NULL;
//    response->value = UA_STATUSCODE_GOOD;

        EdgeMessage *resultMsg = (EdgeMessage *) malloc(sizeof(EdgeMessage));
        resultMsg->endpointInfo = (EdgeEndPointInfo *) malloc(sizeof(EdgeEndPointInfo));
        memcpy(resultMsg->endpointInfo, msg->endpointInfo, sizeof(EdgeEndPointInfo));
        resultMsg->type = GENERAL_RESPONSE;
        resultMsg->responseLength = 1;
        resultMsg->responses = (EdgeResponse **) malloc(1 * sizeof(EdgeResponse));
        resultMsg->responses[0] = response;

        onResponseMessage(resultMsg);

        free(response->nodeInfo); response->nodeInfo = NULL;
        free(response); response = NULL;
        free (resultMsg->endpointInfo); resultMsg->endpointInfo = NULL;
        free (resultMsg->responses); resultMsg->responses = NULL;
        free(resultMsg); resultMsg = NULL;

        UA_Variant_delete(myVariant);
    }
}

#endif

static EdgeDiagnosticInfo *checkDiagnosticInfo(int nodesToProcess,
        UA_DiagnosticInfo *diagnosticInfo, int diagnosticInfoLength, int returnDiagnostic)
{

    EdgeDiagnosticInfo *diagnostics = (EdgeDiagnosticInfo *) malloc(sizeof(EdgeDiagnosticInfo));
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
            char *additional_info = (char *) malloc(diagnosticInfo[0].additionalInfo.length);
            strcpy(additional_info, (char *) (diagnosticInfo[0].additionalInfo.data));
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

static void writeGroup(UA_Client *client, EdgeMessage *msg)
{

    int reqLen = msg->requestLength;
    UA_WriteValue *wv = (UA_WriteValue *) malloc(sizeof(UA_WriteValue) * reqLen);
    UA_Variant *myVariant = (UA_Variant *) malloc(sizeof(UA_Variant) * reqLen);

    for (int i = 0; i < reqLen; i++)
    {
        EDGE_LOG_V(TAG, "[WRITEGROUP] Node to write :: %s\n", msg->requests[i]->nodeInfo->valueAlias);
        int type = (int) msg->requests[i]->type - 1;
        UA_WriteValue_init(&wv[i]);
        UA_Variant_init(&myVariant[i]);
        wv[i].attributeId = UA_ATTRIBUTEID_VALUE;
        wv[i].nodeId = UA_NODEID_STRING(1, msg->requests[i]->nodeInfo->valueAlias);
        wv[i].value.hasValue = true;
        wv[i].value.value.type = &UA_TYPES[type];
        wv[i].value.value.storageType = UA_VARIANT_DATA_NODELETE; /* do not free the integer on deletion */
        if (type == UA_TYPES_STRING)
        {
            UA_String val = UA_STRING_ALLOC((char * ) msg->requests[i]->value);
            UA_Variant_setScalarCopy(&myVariant[i], &val, &UA_TYPES[type]);
            UA_String_deleteMembers(&val);
        }
        else
        {
            UA_Variant_setScalarCopy(&myVariant[i], msg->requests[i]->value, &UA_TYPES[type]);
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

        free(wv);
        for (int i = 0; i < reqLen; i++)
            UA_Variant_deleteMembers(&myVariant[i]);
        free(myVariant);
        myVariant = NULL;

        UA_WriteResponse_deleteMembers(&writeResponse);
        return;
    }

    if (reqLen != writeResponse.resultsSize)
    {
        EDGE_LOG_V(TAG, "Requested(%d) but received(%d) => %s\n", reqLen, (int)writeResponse.resultsSize,
                (reqLen < writeResponse.resultsSize) ? "Received more results" : "Received less results");
        EdgeMessage *resultMsg = (EdgeMessage *) malloc(sizeof(EdgeMessage));
        resultMsg->endpointInfo = (EdgeEndPointInfo *) malloc(sizeof(EdgeEndPointInfo));
        memcpy(resultMsg->endpointInfo, msg->endpointInfo, sizeof(EdgeEndPointInfo));
        resultMsg->type = ERROR;
        resultMsg->responseLength = 0;

        EdgeResult *res = (EdgeResult *) malloc(sizeof(EdgeResult));
        res->code = STATUS_ERROR;
        resultMsg->result = res;

        onResponseMessage(resultMsg);
        free(resultMsg);
        free(res);

        free(wv);
        for (int i = 0; i < reqLen; i++)
            UA_Variant_deleteMembers(&myVariant[i]);
        free(myVariant);
        myVariant = NULL;
        UA_WriteResponse_deleteMembers(&writeResponse);
        return;
    }

    int respIndex = 0;
    EdgeResponse **responses = (EdgeResponse **) malloc(sizeof(EdgeResponse *) * reqLen);
    EdgeMessage *resultMsg = (EdgeMessage *) malloc(sizeof(EdgeMessage));
    resultMsg->endpointInfo = (EdgeEndPointInfo *) malloc(sizeof(EdgeEndPointInfo));
    resultMsg->responseLength = 0;
    resultMsg->command = CMD_WRITE;
    memcpy(resultMsg->endpointInfo, msg->endpointInfo, sizeof(EdgeEndPointInfo));
    resultMsg->type = GENERAL_RESPONSE;

    for (int i = 0; i < reqLen; i++)
    {
        responses[i] = NULL;
        UA_StatusCode code = writeResponse.results[i];

        if (code != UA_STATUSCODE_GOOD)
        {
            EDGE_LOG_V(TAG, "Error in write response for a particular node :: 0x%08x(%s)\n", code,
                    UA_StatusCode_name(code));
            EdgeMessage *resultMsg = (EdgeMessage *) malloc(sizeof(EdgeMessage));
            resultMsg->endpointInfo = (EdgeEndPointInfo *) malloc(sizeof(EdgeEndPointInfo));
            memcpy(resultMsg->endpointInfo, msg->endpointInfo, sizeof(EdgeEndPointInfo));
            resultMsg->type = ERROR;
            resultMsg->responseLength = 0;

            EdgeResult *res = (EdgeResult *) malloc(sizeof(EdgeResult));
            res->code = STATUS_ERROR;
            resultMsg->result = res;

            onResponseMessage(resultMsg);
            free(resultMsg->endpointInfo);
            resultMsg->endpointInfo = NULL;
            free(res);
            res = NULL;
            free(resultMsg);
            resultMsg = NULL;

            if (writeResponse.responseHeader.serviceResult != UA_STATUSCODE_GOOD)
                continue;
        }
        else
        {
            EdgeResponse *response = (EdgeResponse *) malloc(sizeof(EdgeResponse));
            if (response)
            {
                response->nodeInfo = (EdgeNodeInfo *) malloc(sizeof(EdgeNodeInfo));
                memcpy(response->nodeInfo, msg->requests[i]->nodeInfo, sizeof(EdgeNodeInfo));
                response->requestId = msg->requests[i]->requestId;
                response->message = NULL;

                EdgeDiagnosticInfo *diagnosticInfo = checkDiagnosticInfo(msg->requestLength,
                        writeResponse.diagnosticInfos, writeResponse.diagnosticInfosSize,
                        writeRequest.requestHeader.returnDiagnostics);
                response->m_diagnosticInfo = diagnosticInfo;

                EdgeVersatility *message = (EdgeVersatility *) malloc(sizeof(EdgeVersatility));
                message->value = (void *) UA_StatusCode_name(code);
                response->message = message;

                resultMsg->responseLength += 1;
                responses[respIndex] = response;
                respIndex += 1;
            }
        }
    }

    resultMsg->responses = responses;
    onResponseMessage(resultMsg);

    for (int i = 0; i < resultMsg->responseLength; i++)
    {
        if (responses[i]->m_diagnosticInfo)
        {
            if (responses[i]->m_diagnosticInfo->additionalInfo)
            {
                free(responses[i]->m_diagnosticInfo->additionalInfo);
                responses[i]->m_diagnosticInfo->additionalInfo = NULL;
            }
            free(responses[i]->m_diagnosticInfo);
            responses[i]->m_diagnosticInfo = NULL;
        }
        free(responses[i]->message);
        responses[i]->message = NULL;
        free(responses[i]->nodeInfo);
        responses[i]->nodeInfo = NULL;
        free(responses[i]);
        responses[i] = NULL;
    }
    free(resultMsg->endpointInfo);
    resultMsg->endpointInfo = NULL;
    free(resultMsg->responses);
    resultMsg->responses = NULL;
    free(resultMsg);
    resultMsg = NULL;
    free(wv);
    for (int i = 0; i < reqLen; i++)
        UA_Variant_deleteMembers(&myVariant[i]);
    free(myVariant);
    myVariant = NULL;
    UA_WriteResponse_deleteMembers(&writeResponse);
}

EdgeResult executeWrite(UA_Client *client, EdgeMessage *msg)
{
    EdgeResult result;
    if (!client)
    {
        EDGE_LOG(TAG, "Client handle Invalid\n");
        result.code = STATUS_ERROR;
        return result;
    }

    //if (msg->type == SEND_REQUEST)
    //  write(client, msg);
    //else
    writeGroup(client, msg);

    result.code = STATUS_OK;
    return result;
}
