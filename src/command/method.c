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
#include "edge_logger.h"

#include <stdio.h>

#define TAG "method"

EdgeResult executeMethod(UA_Client *client, EdgeMessage *msg)
{
    EdgeResult result;
    result.code = STATUS_OK;
    if (IS_NULL(client))
    {
        EDGE_LOG(TAG, "Client handle Invalid\n");
        result.code = STATUS_ERROR;
        return result;
    }
    EdgeRequest *request = msg->request;
    EdgeMethodRequestParams *params = request->methodParams;
    int idx = 0;
    UA_Variant *input = NULL;
    if (params->num_inpArgs > 0)
        input = (UA_Variant *) malloc(sizeof(UA_Variant) * params->num_inpArgs);

    for (idx = 0; idx < params->num_inpArgs; idx++)
    {
        UA_Variant_init(&input[idx]);
        if (params->inpArg[idx]->valType == SCALAR)
        {
            int type = (int) params->inpArg[idx]->argType - 1;
            if (type == UA_TYPES_STRING)
            {
                UA_String val = UA_STRING_ALLOC((char * ) params->inpArg[idx]->scalarValue);
                UA_Variant_setScalarCopy(&input[idx], &val, &UA_TYPES[type]);
                FREE(val.data);
                UA_String_deleteMembers(&val);
            }
            else
            {
                EDGE_LOG(TAG, "\n\n======== scalar copy======== \n\n");
                UA_Variant_setScalarCopy(&input[idx], params->inpArg[idx]->scalarValue,
                        &UA_TYPES[type]);
            }
        }
        else if (params->inpArg[idx]->valType == ARRAY_1D)
        {
            int type = (int) params->inpArg[idx]->argType - 1;
            if (type == UA_TYPES_STRING)
            {
                char **data = (char **) params->inpArg[idx]->arrayData;
                UA_String *array = (UA_String *) UA_Array_new(params->inpArg[idx]->arrayLength,
                        &UA_TYPES[type]);
                for (int idx1 = 0; idx1 < params->inpArg[idx]->arrayLength; idx1++)
                {
                    array[idx1] = UA_STRING_ALLOC(data[idx1]);
                }
                UA_Variant_setArrayCopy(&input[idx], array, params->inpArg[idx]->arrayLength,
                        &UA_TYPES[type]);
                for (int idx1 = 0; idx1 < params->inpArg[idx]->arrayLength; idx1++)
                {
                    UA_String_deleteMembers(&array[idx1]);
                }
                UA_Array_delete(array, params->inpArg[idx]->arrayLength, &UA_TYPES[type]);
            }
            else
            {
                UA_Variant_setArrayCopy(&input[idx], params->inpArg[idx]->arrayData,
                        params->inpArg[idx]->arrayLength, &UA_TYPES[type]);
            }
        }
    }

    size_t outputSize;
    UA_Variant *output;
    UA_StatusCode retVal = UA_Client_call(client, UA_NODEID_NUMERIC(0, UA_NS0ID_OBJECTSFOLDER),
            UA_NODEID_STRING(1, request->nodeInfo->valueAlias), params->num_inpArgs, input,
            &outputSize, &output);
    if (retVal == UA_STATUSCODE_GOOD)
    {
        EDGE_LOG(TAG, "method call was success\n\n");

        EdgeResponse **response = (EdgeResponse **) malloc(sizeof(EdgeResponse *) * outputSize);
        if(IS_NULL(response))
        {
            EDGE_LOG(TAG, "ERROR : EdgeResponse malloc failed in executeMethod\n");
            result.code = STATUS_ERROR;
            goto EXIT;
        }
        if (response)
        {
            for (int i = 0; i < outputSize; i++)
            {
                response[i] = (EdgeResponse *) malloc(sizeof(EdgeResponse));
                if(IS_NULL(response[i]))
                {
                    EDGE_LOG_V(TAG, "ERROR : EdgeResponse %d Malloc failed in executeMethod\n", i);
                    result.code = STATUS_ERROR;
                    goto EXIT;
                }
                response[i]->nodeInfo = (EdgeNodeInfo *) malloc(sizeof(EdgeNodeInfo));
                memcpy(response[i]->nodeInfo, msg->request->nodeInfo, sizeof(EdgeNodeInfo));
                response[i]->requestId = msg->request->requestId;
                //response[i]->value = output[i].data;

                EdgeVersatility *versatility = (EdgeVersatility *) malloc(sizeof(EdgeVersatility));
                if(IS_NULL(versatility))
                {
                    EDGE_LOG(TAG, "ERROR : versatility malloc failed in executeMethod\n");
                    result.code = STATUS_ERROR;
                    goto EXIT;
                }
                if (UA_Variant_isScalar(&output[i]) == UA_TRUE)
                {
                    // Output argument is scalar type
                    versatility->arrayLength = 0;
                    versatility->isArray = false;
                    versatility->value = output[i].data;

                    if (output[i].type == &UA_TYPES[UA_TYPES_BOOLEAN])
                        response[i]->type = Boolean;
                    else if (output[i].type == &UA_TYPES[UA_TYPES_INT16])
                    {
                        response[i]->type = Int16;
                    }
                    else if (output[i].type == &UA_TYPES[UA_TYPES_UINT16])
                    {
                        response[i]->type = UInt16;
                    }
                    else if (output[i].type == &UA_TYPES[UA_TYPES_INT32])
                    {
                        response[i]->type = Int32;
                    }
                    else if (output[i].type == &UA_TYPES[UA_TYPES_UINT32])
                    {
                        response[i]->type = UInt32;
                    }
                    else if (output[i].type == &UA_TYPES[UA_TYPES_INT64])
                    {
                        response[i]->type = Int64;
                    }
                    else if (output[i].type == &UA_TYPES[UA_TYPES_UINT64])
                    {
                        response[i]->type = UInt64;
                    }
                    else if (output[i].type == &UA_TYPES[UA_TYPES_FLOAT])
                    {
                        response[i]->type = Float;
                    }
                    else if (output[i].type == &UA_TYPES[UA_TYPES_DOUBLE])
                    {
                        response[i]->type = Double;
                    }
                    else if (output[i].type == &UA_TYPES[UA_TYPES_STRING])
                    {
                        UA_String str = *((UA_String *) output[i].data);
                        response[i]->value = (void *) str.data;
                        response[i]->type = String;
                    }
                    else if (output[i].type == &UA_TYPES[UA_TYPES_BYTE])
                    {
                        response[i]->type = Byte;
                    }
                    else if (output[i].type == &UA_TYPES[UA_TYPES_DATETIME])
                    {
                        response[i]->type = DateTime;
                    }
                }
                else
                {
                    // Handle for array result variant
                    versatility->arrayLength = output[i].arrayLength;
                    versatility->isArray = true;
                    versatility->value = output[i].data;

                    if (output[i].type == &UA_TYPES[UA_TYPES_BOOLEAN])
                        response[i]->type = Boolean;
                    else if (output[i].type == &UA_TYPES[UA_TYPES_INT16])
                    {
                        response[i]->type = Int16;
                    }
                    else if (output[i].type == &UA_TYPES[UA_TYPES_UINT16])
                    {
                        response[i]->type = UInt16;
                    }
                    else if (output[i].type == &UA_TYPES[UA_TYPES_INT32])
                    {
                        response[i]->type = Int32;
                    }
                    else if (output[i].type == &UA_TYPES[UA_TYPES_UINT32])
                    {
                        response[i]->type = UInt32;
                    }
                    else if (output[i].type == &UA_TYPES[UA_TYPES_INT64])
                    {
                        response[i]->type = Int64;
                    }
                    else if (output[i].type == &UA_TYPES[UA_TYPES_UINT64])
                    {
                        response[i]->type = UInt64;
                    }
                    else if (output[i].type == &UA_TYPES[UA_TYPES_FLOAT])
                    {
                        response[i]->type = Float;
                    }
                    else if (output[i].type == &UA_TYPES[UA_TYPES_DOUBLE])
                    {
                        response[i]->type = Double;
                    }
                    else if (output[i].type == &UA_TYPES[UA_TYPES_STRING])
                    {
                        UA_String str = *((UA_String *) output[i].data);
                        versatility->value = (void *) str.data;
                        response[i]->type = String;
                    }
                    else if (output[i].type == &UA_TYPES[UA_TYPES_BYTE])
                    {
                        response[i]->type = Byte;
                    }
                    else if (output[i].type == &UA_TYPES[UA_TYPES_DATETIME])
                    {
                        response[i]->type = DateTime;
                    }
                }
                response[i]->message = versatility;
                response[i]->m_diagnosticInfo = NULL;
            }

            EdgeMessage *resultMsg = (EdgeMessage *) malloc(sizeof(EdgeMessage));
            if(IS_NULL(resultMsg))
            {
                EDGE_LOG(TAG, "ERROR : ResultMsg malloc failed in executeMethod\n");
                result.code = STATUS_ERROR;
                goto EXIT;
            }
            resultMsg->endpointInfo = (EdgeEndPointInfo *) calloc(1, sizeof(EdgeEndPointInfo));
            if(IS_NULL(resultMsg->endpointInfo))
            {
                EDGE_LOG(TAG, "ERROR : ResultMsg.endpointInfo malloc failed in executeMethod\n");
                result.code = STATUS_ERROR;
                goto EXIT;
            }
            memcpy(resultMsg->endpointInfo, msg->endpointInfo, sizeof(EdgeEndPointInfo));
            resultMsg->type = GENERAL_RESPONSE;
            resultMsg->responseLength = outputSize;
            //resultMsg->responses = (EdgeResponse**) malloc(1 * sizeof(EdgeResponse));
            resultMsg->responses = response;
            resultMsg->command = CMD_METHOD;

            onResponseMessage(resultMsg);

            EXIT:
            for (int i = 0; i < outputSize; i++)
            {
                if(IS_NOT_NULL(response[i]->nodeInfo))
                {
                    FREE(response[i]->nodeInfo);
                }
                if(IS_NOT_NULL(response[i]->message))
                {
                    FREE(response[i]->message);
                }
                if(IS_NOT_NULL(response[i]->message))
                {
                    FREE(response[i]);
                }
            }
            FREE(response);
            if(IS_NOT_NULL(resultMsg))
            {
                FREE(resultMsg->endpointInfo);
                FREE(resultMsg);
            }
        }
    }
    else
    {
        EDGE_LOG_V(TAG, "method call failed 0x%08x\n", retVal);
        result.code = STATUS_ERROR;

        EdgeMessage *resultMsg = (EdgeMessage *) malloc(sizeof(EdgeMessage));
        if(IS_NULL(resultMsg))
        {
            EDGE_LOG(TAG, "Error : Malloc failed for resultMsg in executeMethod\n");
            goto EXIT_ERROR;
        }
        resultMsg->endpointInfo = (EdgeEndPointInfo *) calloc(1, sizeof(EdgeEndPointInfo));
        if(IS_NULL(resultMsg))
        {
            EDGE_LOG(TAG, "Error : Malloc failed for resultMsg.endpointInfo in executeMethod\n");
            goto EXIT_ERROR;
        }
        memcpy(resultMsg->endpointInfo, msg->endpointInfo, sizeof(EdgeEndPointInfo));
        resultMsg->type = ERROR;
        resultMsg->responseLength = 0;

        EdgeResult *res = (EdgeResult *) malloc(sizeof(EdgeResult));
        if(IS_NULL(res))
        {
            EDGE_LOG(TAG, "Error : Malloc failed for result in executeMethod\n");
            goto EXIT_ERROR;
        }
        res->code = STATUS_ERROR;
        resultMsg->result = res;

        onResponseMessage(resultMsg);

        EXIT_ERROR:
        if(IS_NOT_NULL(resultMsg))
        {
            FREE(resultMsg->endpointInfo);
            FREE(resultMsg);
        }
        FREE(res);
    }

    for (idx = 0; idx < params->num_inpArgs; idx++)
    {
        UA_Variant_deleteMembers(&input[idx]);
    }

    FREE(input);
    return result;
}
