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

#include "method.h"
#include "cmd_util.h"
#include "edge_logger.h"
#include "edge_malloc.h"
#include "message_dispatcher.h"
#include "edge_open62541.h"

#define TAG "method"

#define GUID_LENGTH (36)

EdgeResult executeMethod(UA_Client *client, const EdgeMessage *msg)
{
    EdgeResult result;
    result.code = STATUS_ERROR;
    VERIFY_NON_NULL_MSG(client, "NULL param CLIENT in executeMethod\n", result);
    EdgeRequest *request = msg->request;
    EdgeMethodRequestParams *params = request->methodParams;
    int idx = 0;
    UA_Variant *input = NULL;
    int num_inpArgs = 0;
    if (params)
    {
        num_inpArgs = params->num_inpArgs;
    }
    if (num_inpArgs > 0)
    {
        input = (UA_Variant *) EdgeCalloc(params->num_inpArgs, sizeof(UA_Variant));
        VERIFY_NON_NULL_MSG(input, "EdgeCalloc FAILED for UA_Variant in executeMethod\n", result);
    }

    for (idx = 0; idx < num_inpArgs; idx++)
    {
        UA_Variant_init(&input[idx]);
        int type = (int) params->inpArg[idx]->argType - 1;
        if (params->inpArg[idx]->valType == SCALAR)
        {
            /* Input argument is scalar value */
            createScalarVariant(type, params->inpArg[idx]->scalarValue, &input[idx]);
        }
        else if (params->inpArg[idx]->valType == ARRAY_1D)
        {
            /* Input argument is array of scalar values */
            createArrayVariant(type, params->inpArg[idx]->arrayData,
                               params->inpArg[idx]->arrayLength, &input[idx]);
        }
    }

    size_t outputSize = 0;
    UA_Variant *output = NULL;
    EdgeMessage *resultMsg = NULL;
    /* Execute Method Call */
    UA_StatusCode retVal = UA_Client_call(client, UA_NODEID_NUMERIC(0, UA_NS0ID_OBJECTSFOLDER),
            UA_NODEID_STRING(request->nodeInfo->nodeId->nameSpace, request->nodeInfo->valueAlias),
            num_inpArgs, input, &outputSize, &output);
    if (retVal != UA_STATUSCODE_GOOD)
    {
        /* Method call failed */
        EDGE_LOG_V(TAG, "method call failed 0x%08x\n", retVal);
        sendErrorResponse(msg, "Error in executing METHOD OPERATION.");
    }
    else
    {
        EDGE_LOG(TAG, "method call was success");

        resultMsg = (EdgeMessage *) EdgeCalloc(1, sizeof(EdgeMessage));
        if(IS_NULL(resultMsg))
        {
            EDGE_LOG(TAG, "Memory allocation failed.");
            goto EXIT;
        }

        resultMsg->endpointInfo = cloneEdgeEndpointInfo(msg->endpointInfo);
        if(IS_NULL(resultMsg->endpointInfo))
        {
            EDGE_LOG(TAG, "Memory allocation failed.");
            goto EXIT;
        }

        resultMsg->type = GENERAL_RESPONSE;
        resultMsg->responseLength = outputSize;
        resultMsg->command = CMD_METHOD;
        resultMsg->message_id = msg->message_id;

        if(outputSize > 0)
        {
            resultMsg->responses = (EdgeResponse **) EdgeCalloc(outputSize, sizeof(EdgeResponse *));
            if(IS_NULL(resultMsg->responses))
            {
                EDGE_LOG(TAG, "ERROR : EdgeResponse EdgeMalloc failed in executeMethod");
                goto EXIT;
            }
        }

        for (int i = 0; i < outputSize; i++)
        {
            resultMsg->responses[i] = (EdgeResponse *) EdgeCalloc(1, sizeof(EdgeResponse));
            if(IS_NULL(resultMsg->responses[i]))
            {
                EDGE_LOG_V(TAG, "ERROR : EdgeResponse %d Malloc failed in executeMethod\n", i);
                goto EXIT;
            }

            resultMsg->responses[i]->nodeInfo = cloneEdgeNodeInfo(msg->request->nodeInfo);
            resultMsg->responses[i]->requestId = msg->request->requestId;
            resultMsg->responses[i]->type = get_response_type(output[i].type);
            resultMsg->responses[i]->message = parseResponse(resultMsg->responses[i], output[i]);
            if(IS_NULL(resultMsg->responses[i]->message))
            {
                EDGE_LOG(TAG, "ERROR : versatility EdgeMalloc failed in executeMethod");
                goto EXIT;
            }
            resultMsg->responses[i]->m_diagnosticInfo = NULL;
        }

        /* Adding the method response to receiverQ */
        add_to_recvQ(resultMsg);
        result.code = STATUS_OK;
        resultMsg = NULL;
    }

EXIT:
    /* Free the memory */
    if(IS_NOT_NULL(resultMsg))
    {
        freeEdgeMessage(resultMsg);
    }

    for (idx = 0; idx < num_inpArgs; idx++)
    {
        UA_Variant_deleteMembers(&input[idx]);
    }

    UA_Array_delete(output, outputSize, &UA_TYPES[UA_TYPES_VARIANT]);

    EdgeFree(input);
    return result;
}
