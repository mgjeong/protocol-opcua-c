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
#include "edge_open62541.h"
#include "message_dispatcher.h"
#include "cmd_util.h"

#include <inttypes.h>

#define TAG "write"

/**
 * @brief writeGroup - Executes write operation
 * @param client - Client handle
 * @param msg - Request Edge Message
 */
static void writeGroup(UA_Client *client, const EdgeMessage *msg)
{
    size_t reqLen = msg->requestLength;
    UA_WriteValue *wv = (UA_WriteValue *) EdgeMalloc(sizeof(UA_WriteValue) * reqLen);
    VERIFY_NON_NULL_NR_MSG(wv, "EdgeMalloc FAILED for UA_WriteValue in wroteGroup\n");
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
        uint32_t Nodeid = (uint32_t)(msg->requests[i]->type);
        uint32_t type = Nodeid - 1;
        UA_WriteValue_init(&wv[i]);
        UA_Variant_init(&myVariant[i]);
        /* Attribute Id to write to */
        wv[i].attributeId = UA_ATTRIBUTEID_VALUE;
        /* Node id */
        wv[i].nodeId = UA_NODEID_STRING(msg->requests[i]->nodeInfo->nodeId->nameSpace,
                msg->requests[i]->nodeInfo->valueAlias);
        wv[i].value.hasValue = true;
        /* Data type */
        wv[i].value.value.type = &UA_TYPES[type];
        wv[i].value.value.storageType = UA_VARIANT_DATA_NODELETE; /* do not free the integer on deletion */

        EdgeVersatility *message = (EdgeVersatility * ) msg->requests[i]->value;
        if (message->isArray == 0)
        {
            /* scalar value to write */
            createScalarVariant(type, message->value, &myVariant[i]);
        }
        else
        {
            /* array value to write */
            createArrayVariant(type, message->value, message->arrayLength, &myVariant[i]);
        }
        wv[i].value.value = myVariant[i];
    }

    UA_WriteRequest writeRequest;
    UA_WriteRequest_init(&writeRequest);
    /* Node information */
    writeRequest.nodesToWrite = wv;
    /* Number of nodes to write */
    writeRequest.nodesToWriteSize = reqLen;
    //writeRequest.requestHeader.returnDiagnostics = 1;

    /* Execute write operation */
    UA_WriteResponse writeResponse = UA_Client_Service_write(client, writeRequest);
    if (writeResponse.responseHeader.serviceResult != UA_STATUSCODE_GOOD)
    {
        /* Error in write request */
        EDGE_LOG_V(TAG, "Error in write :: 0x%08x(%s)\n", writeResponse.responseHeader.serviceResult,
                UA_StatusCode_name(writeResponse.responseHeader.serviceResult));

        EdgeFree(wv);
        for (size_t i = 0; i < reqLen; i++)
            UA_Variant_deleteMembers(&myVariant[i]);
        EdgeFree(myVariant);

        UA_WriteResponse_deleteMembers(&writeResponse);
        return;
    }

    EdgeFree(wv);
    for (size_t i = 0; i < reqLen; i++)
        UA_Variant_deleteMembers(&myVariant[i]);
    EdgeFree(myVariant);

    if (reqLen != writeResponse.resultsSize)
    {
        EDGE_LOG_V(TAG, "Requested(%d) but received(%d) => %s\n", (int) reqLen, (int)writeResponse.resultsSize,
                (reqLen < writeResponse.resultsSize) ? "Received more results" : "Received less results");
        sendErrorResponse(msg, "Error in write operation");

        UA_WriteResponse_deleteMembers(&writeResponse);
        return;
    }

    EdgeMessage *resultMsg = (EdgeMessage *) EdgeCalloc(1, sizeof(EdgeMessage));
    if (IS_NULL(resultMsg))
    {
        EDGE_LOG(TAG, "Error : Malloc Failed for resultMsg in Write Group");
        goto WRITE_ERROR;
    }

    resultMsg->endpointInfo = cloneEdgeEndpointInfo(msg->endpointInfo);
    if (IS_NULL(resultMsg->endpointInfo))
    {
        EDGE_LOG(TAG, "Error : Malloc Failed for resultMsg->endpointInfo in Write Group");
        goto WRITE_ERROR;
    }
    resultMsg->responseLength = 0;
    resultMsg->command = CMD_WRITE;
    resultMsg->type = GENERAL_RESPONSE;
    resultMsg->message_id = msg->message_id;

    resultMsg->responses = (EdgeResponse **) EdgeCalloc(reqLen, sizeof(EdgeResponse *));
    if (IS_NULL(resultMsg->responses))
    {
        EDGE_LOG(TAG, "Error : Malloc Failed for responses in Write Group");
        goto WRITE_ERROR;
    }

    size_t respIndex = 0;
    for (size_t i = 0; i < reqLen; i++)
    {
        UA_StatusCode code = writeResponse.results[i];

        if (code != UA_STATUSCODE_GOOD)
        {
            /* Error in write response for a particular node */
            EDGE_LOG_V(TAG, "Error in write response for a particular node :: 0x%08x(%s)\n", code,
                    UA_StatusCode_name(code));

            sendErrorResponse(msg, "Error in write Response");

            if (writeResponse.responseHeader.serviceResult != UA_STATUSCODE_GOOD)
                continue;
        }
        else
        {
            EdgeResponse *response = (EdgeResponse *) EdgeCalloc(1, sizeof(EdgeResponse));
            if (IS_NULL(response))
            {
                goto WRITE_ERROR;
            }
            response->nodeInfo = cloneEdgeNodeInfo(msg->requests[i]->nodeInfo);
            if (IS_NULL(response->nodeInfo))
            {
                EDGE_LOG(TAG, "Error : Malloc Failed for EdgeResponse.NodeInfo in Write Group");
                freeEdgeResponse(response);
                goto WRITE_ERROR;
            }
            response->requestId = msg->requests[i]->requestId;
            response->m_diagnosticInfo = checkDiagnosticInfo(msg->requestLength,
                    writeResponse.diagnosticInfos, writeResponse.diagnosticInfosSize,
                    writeRequest.requestHeader.returnDiagnostics);
            if (IS_NULL(response->m_diagnosticInfo))
            {
                EDGE_LOG(TAG, "Error : Malloc Failed for EdgeResponse.DagnosticInfo in Write Group");
                freeEdgeResponse(response);
                goto WRITE_ERROR;
            }

            response->message = (EdgeVersatility *) EdgeCalloc(1, sizeof(EdgeVersatility));
            if (IS_NULL(response->message))
            {
                EDGE_LOG(TAG, "Error : Malloc Failed for EdgeVersatility in Write Group");
                freeEdgeResponse(response);
                goto WRITE_ERROR;
            }
            const char *retCode = UA_StatusCode_name(code);
            size_t len = strlen(retCode);
            char *code = (char*) malloc(len+1);
            strncpy(code, retCode, len+1);

            response->message->value = (void *) code;
            response->message->isArray = false;
            response->message->arrayLength = 0;

            resultMsg->responseLength++;
            resultMsg->responses[respIndex++] = response;
        }
    }

    if (respIndex < 1)
    {
        goto WRITE_ERROR;
    }
    /* Adding the write response to receiver Q */
    add_to_recvQ(resultMsg);

    UA_WriteResponse_deleteMembers(&writeResponse);
    return;

    WRITE_ERROR:
    /* Free memory */
    freeEdgeMessage(resultMsg);
    UA_WriteResponse_deleteMembers(&writeResponse);
}

EdgeResult executeWrite(UA_Client *client, const EdgeMessage *msg)
{
    EdgeResult result;
    result.code = STATUS_ERROR;
    VERIFY_NON_NULL_MSG(client, "Client param is NULL in execute WRITE\n", result);
    writeGroup(client, msg);
    result.code = STATUS_OK;
    return result;
}
