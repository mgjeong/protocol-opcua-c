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
        if (params->inpArg[idx]->valType == SCALAR)
        {
            /* Input argument is scalar value */
            int type = (int) params->inpArg[idx]->argType - 1;
            if (type == UA_TYPES_STRING)
            {
                UA_String val = UA_STRING_ALLOC((char * ) params->inpArg[idx]->scalarValue);
                UA_Variant_setScalarCopy(&input[idx], &val, &UA_TYPES[type]);
                UA_String_deleteMembers(&val);
                EdgeFree(val.data);
            }
            else
            {
                UA_Variant_setScalarCopy(&input[idx], params->inpArg[idx]->scalarValue,
                        &UA_TYPES[type]);
            }
        }
        else if (params->inpArg[idx]->valType == ARRAY_1D)
        {
            /* Input argument is array of scalar values */
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
            resultMsg->responses[i]->message = (EdgeVersatility *) EdgeCalloc(1, sizeof(EdgeVersatility));
            if(IS_NULL(resultMsg->responses[i]->message))
            {
                EDGE_LOG(TAG, "ERROR : versatility EdgeMalloc failed in executeMethod");
                goto EXIT;
            }

            EdgeVersatility *versatility = (EdgeVersatility *) resultMsg->responses[i]->message;

            if (UA_Variant_isScalar(&output[i]) == UA_TRUE)
            {
                /* Output Argument is scalar type */
                versatility->arrayLength = 0;
                versatility->isArray = false;
                size_t size = get_size(resultMsg->responses[i]->type, false);
                if ((resultMsg->responses[i]->type == UA_NS0ID_STRING) || (resultMsg->responses[i]->type == UA_NS0ID_BYTESTRING))
                {
                    /* STRING or BYTESTRING response */
                    UA_String str = *((UA_String *) output[i].data);
                    size_t len = str.length;
                    versatility->value = (void *) EdgeCalloc(1, len+1);
                    if(IS_NULL(versatility->value))
                    {
                        EDGE_LOG(TAG, "Memory allocation failed.");
                        goto EXIT;
                    }
                    strncpy(versatility->value, (char*) str.data, len);
                   ((char*) versatility->value)[(int) len] = '\0';
                }
                else if (resultMsg->responses[i]->type == UA_NS0ID_GUID)
                {
                    /* GUID response */
                    UA_Guid str = *((UA_Guid *) output[i].data);
                    char *value = (char *) EdgeMalloc(GUID_LENGTH + 1);
                    if(IS_NULL(value))
                    {
                        EDGE_LOG(TAG, "Error : Malloc failed for Guid SCALAR value in Read Group");
                        goto EXIT;
                    }

                    snprintf(value, GUID_LENGTH + 1, "%08x-%04x-%04x-%02x%02x-%02x%02x%02x%02x%02x%02x",
                            str.data1, str.data2, str.data3, str.data4[0], str.data4[1], str.data4[2],
                            str.data4[3], str.data4[4], str.data4[5], str.data4[6], str.data4[7]);

                    versatility->value = (void *) value;
                    EDGE_LOG_V(TAG, "%s\n", value);
                }
                else
                {
                    versatility->value = (void *) EdgeCalloc(1, size);
                    if(IS_NULL(versatility->value))
                    {
                        EDGE_LOG(TAG, "Memory allocation failed.");
                        goto EXIT;
                    }
                    memcpy(versatility->value, output[i].data, size);
                }
            }
            else
            {
                /* Output Argument is array type */
                versatility->arrayLength = output[i].arrayLength;
                versatility->isArray = true;
                size_t size = get_size(resultMsg->responses[i]->type, true);
                if (resultMsg->responses[i]->type == UA_NS0ID_STRING || resultMsg->responses[i]->type == UA_NS0ID_BYTESTRING)
                {
                    /* STRING or BYTESTRING ARRAY response */
                    UA_String *str = ((UA_String *) output[i].data);
                    versatility->value = EdgeCalloc(output[i].arrayLength, sizeof(char *));
                    if(IS_NULL(versatility->value))
                    {
                        EDGE_LOG(TAG, "Error : Malloc failed for String Array values in Read Group");
                        goto EXIT;
                    }

                    char **values = (char **) versatility->value;
                    for (int j = 0; j < output[i].arrayLength; j++)
                    {
                        values[j] = (char *) EdgeMalloc(str[j].length+1);
                        if(IS_NULL(values[j]))
                        {
                            EDGE_LOG_V(TAG, "Error : Malloc failed for String/ByteString Array value %d in Read Group\n", i);
                            goto EXIT;
                        }
                        strncpy(values[j], (char *) str[j].data, str[j].length);
                        values[j][str[j].length] = '\0';
                    }
                }
                else if (resultMsg->responses[i]->type == UA_NS0ID_GUID)
                {
                    /* GUID ARRAY response */
                    UA_Guid *str = ((UA_Guid *) output[i].data);
                    versatility->value = EdgeCalloc(output[i].arrayLength, sizeof(char *));
                    if(IS_NULL(versatility->value))
                    {
                        EDGE_LOG(TAG, "Error : Malloc failed for Guid Array values in Read Group");
                        goto EXIT;
                    }

                    char **values = (char **) versatility->value;
                    for (int j = 0; j < output[i].arrayLength; j++)
                    {
                        values[j] = (char *) EdgeMalloc(GUID_LENGTH + 1);
                        if(IS_NULL(values[j]))
                        {
                            EDGE_LOG_V(TAG, "Error : Malloc failed for Guid Array value %d in Read Group\n", i);
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
                        EDGE_LOG_V(TAG, "Error : Malloc failed for Guid Array value %d in Read Group\n", i);
                        goto EXIT;
                    }
                    memcpy(versatility->value, output[i].data,
                            get_size(resultMsg->responses[i]->type, false) * versatility->arrayLength);
                }
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
