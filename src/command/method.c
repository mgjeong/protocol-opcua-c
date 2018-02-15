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
#include "edge_malloc.h"
#include "message_dispatcher.h"

#define TAG "method"

#define GUID_LENGTH (36)

EdgeResult executeMethod(UA_Client *client, const EdgeMessage *msg)
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
    int num_inpArgs = 0;
    if (params)
    {
        num_inpArgs = params->num_inpArgs;
    }
    if (num_inpArgs > 0)
        input = (UA_Variant *) EdgeMalloc(sizeof(UA_Variant) * params->num_inpArgs);

    for (idx = 0; idx < num_inpArgs; idx++)
    {
        UA_Variant_init(&input[idx]);
        if (params->inpArg[idx]->valType == SCALAR)
        {
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
            UA_NODEID_STRING(request->nodeInfo->nodeId->nameSpace, request->nodeInfo->valueAlias),
            num_inpArgs, input, &outputSize, &output);
    if (retVal == UA_STATUSCODE_GOOD)
    {
        EDGE_LOG(TAG, "method call was success\n\n");

        EdgeResponse **response = (EdgeResponse **) malloc(sizeof(EdgeResponse *) * outputSize);
        if(IS_NULL(response))
        {
            EDGE_LOG(TAG, "ERROR : EdgeResponse EdgeMalloc failed in executeMethod\n");
            result.code = STATUS_ERROR;
            goto EXIT;
        }
        if (response)
        {
            bool error_flag = true;
            for (int i = 0; i < outputSize; i++)
            {
                response[i] = (EdgeResponse *) EdgeCalloc(1, sizeof(EdgeResponse));
                if(IS_NULL(response[i]))
                {
                    EDGE_LOG_V(TAG, "ERROR : EdgeResponse %d Malloc failed in executeMethod\n", i);
                    result.code = STATUS_ERROR;
                    goto EXIT;
                }
                response[i]->nodeInfo = cloneEdgeNodeInfo(msg->request->nodeInfo);
                response[i]->requestId = msg->request->requestId;

                EdgeVersatility *versatility = (EdgeVersatility *) EdgeCalloc(1, sizeof(EdgeVersatility));
                if(IS_NULL(versatility))
                {
                    EDGE_LOG(TAG, "ERROR : versatility EdgeMalloc failed in executeMethod\n");
                    result.code = STATUS_ERROR;
                    goto EXIT;
                }

                if (output[i].type == &UA_TYPES[UA_TYPES_BOOLEAN])
                {
                    response[i]->type = Boolean;
                }
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
                    response[i]->type = String;
                }
                else if (output[i].type == &UA_TYPES[UA_TYPES_BYTESTRING])
                {
                    response[i]->type = ByteString;
                }
                else if (output[i].type == &UA_TYPES[UA_TYPES_GUID])
                {
                    response[i]->type = Guid;
                }
                else if (output[i].type == &UA_TYPES[UA_TYPES_SBYTE])
                {
                    response[i]->type = SByte;
                }
                else if (output[i].type == &UA_TYPES[UA_TYPES_BYTE])
                {
                    response[i]->type = Byte;
                }
                else if (output[i].type == &UA_TYPES[UA_TYPES_DATETIME])
                {
                    response[i]->type = DateTime;
                }

                if (UA_Variant_isScalar(&output[i]) == UA_TRUE)
                {
                    /* Output Argument is scalar type */
                    versatility->arrayLength = 0;
                    versatility->isArray = false;
                    size_t size = get_size(response[i]->type, false);
                    if ((response[i]->type == String) || (response[i]->type == ByteString))
                    {
                        UA_String str = *((UA_String *) output[i].data);
                        size_t len = str.length;
                        versatility->value = (void *) EdgeCalloc(1, len+1);
                        strncpy(versatility->value, (char*) str.data, len);
                       ((char*) versatility->value)[(int) len] = '\0';
                    }
                    else if (response[i]->type == Guid)
                    {
                        UA_Guid str = *((UA_Guid *) output[i].data);
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
                        memcpy(versatility->value, output[i].data, size);
                    }
                }
                else
                {
                    /* Output Argument is array type */
                    versatility->arrayLength = output[i].arrayLength;
                    versatility->isArray = true;
                    size_t size = get_size(response[i]->type, true);
                    if (response[i]->type == String || response[i]->type == ByteString)
                    {
                        // String Array
                        UA_String *str = ((UA_String *) output[i].data);
                        char **values = (char **) malloc(sizeof(char *) * output[i].arrayLength);
                        if(IS_NULL(values))
                        {
                            EDGE_LOG(TAG, "Error : Malloc failed for String Array values in Read Group\n");
                            goto EXIT;
                        }
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
                        versatility->value = (void *) values;
                    }
                    else if (response[i]->type == Guid)
                    {
                        // Guid Array
                        UA_Guid *str = ((UA_Guid *) output[i].data);
                        char **values = (char **) EdgeMalloc(sizeof(char *) * output[i].arrayLength);
                        if(IS_NULL(values))
                        {
                            EDGE_LOG(TAG, "Error : Malloc failed for Guid Array values in Read Group\n");
                            goto EXIT;
                        }
                        for (int j = 0; j < output[i].arrayLength; j++)
                        {
                            values[j] = (char *) EdgeMalloc(GUID_LENGTH + 1);
                            if(IS_NULL(values[j]))
                            {
                                EDGE_LOG_V(TAG, "Error : Malloc failed for Guid Array value %d in Read Group\n", i);
                                goto EXIT;
                            }
                            snprintf(values[j], (GUID_LENGTH / 2) + 1, "%08x-%04x-%04x",
                                    str[j].data1, str[j].data2, str[j].data3);
                            sprintf(values[j], "%s-%02x", values[j], str[j].data4[0]);
                            sprintf(values[j], "%s%02x", values[j], str[j].data4[1]);
                            sprintf(values[j], "%s-", values[j]);
                            for (int k = 2; k < 8; k++)
                                sprintf(values[j], "%s%02x", values[j], str[j].data4[k]);
                            values[GUID_LENGTH] = '\0';
                            EDGE_LOG_V(TAG, "%s\n", values[j]);
                        }
                        versatility->value = (void *) values;
                    }
                    else
                    {
                        versatility->value = (void *) EdgeCalloc(versatility->arrayLength, size);
                        memcpy(versatility->value, output[i].data, get_size(response[i]->type, false) * versatility->arrayLength);
                    }
                }

                response[i]->message = versatility;
                response[i]->m_diagnosticInfo = NULL;
            }

            EdgeMessage *resultMsg = (EdgeMessage *) EdgeCalloc(1, sizeof(EdgeMessage));
            if(IS_NULL(resultMsg))
            {
                EDGE_LOG(TAG, "ERROR : ResultMsg EdgeMalloc failed in executeMethod\n");
                result.code = STATUS_ERROR;
                goto EXIT;
            }
            resultMsg->endpointInfo = cloneEdgeEndpointInfo(msg->endpointInfo);
            if(IS_NULL(resultMsg->endpointInfo))
            {
                EDGE_LOG(TAG, "ERROR : ResultMsg.endpointInfo EdgeCalloc failed in executeMethod\n");
                result.code = STATUS_ERROR;
                goto EXIT;
            }
            resultMsg->type = GENERAL_RESPONSE;
            resultMsg->responseLength = outputSize;
            resultMsg->responses = response;
            resultMsg->command = CMD_METHOD;
            resultMsg->message_id = msg->message_id;

            add_to_recvQ(resultMsg);
            error_flag = false;

            EXIT:
            if (error_flag)
            {
                for (int i = 0; i < outputSize; i++)
                {
                    if(IS_NOT_NULL(response[i]->nodeInfo))
                    {
                        EdgeFree(response[i]->nodeInfo);
                    }
                    if(IS_NOT_NULL(response[i]->message))
                    {
                        EdgeFree(response[i]->message);
                    }
                    if(IS_NOT_NULL(response[i]->message))
                    {
                        EdgeFree(response[i]);
                    }
                }
                EdgeFree(response);
                if(IS_NOT_NULL(resultMsg))
                {
                    EdgeFree(resultMsg->endpointInfo);
                    EdgeFree(resultMsg);
                }
                error_flag = true;
            }
        }
    }
    else
    {
        EDGE_LOG_V(TAG, "method call failed 0x%08x\n", retVal);
        result.code = STATUS_ERROR;
        bool error_flag = true;

        EdgeMessage *resultMsg = (EdgeMessage *) EdgeCalloc(1, sizeof(EdgeMessage));
        if(IS_NULL(resultMsg))
        {
            EDGE_LOG(TAG, "Error : Malloc failed for resultMsg in executeMethod\n");
            goto EXIT_ERROR;
        }
        resultMsg->endpointInfo = cloneEdgeEndpointInfo(msg->endpointInfo);
        if(IS_NULL(resultMsg))
        {
            EDGE_LOG(TAG, "Error : EdgeCalloc failed for resultMsg.endpointInfo in executeMethod\n");
            goto EXIT_ERROR;
        }
        resultMsg->type = ERROR;
        resultMsg->responseLength = 1;
        resultMsg->message_id = msg->message_id;

        EdgeResponse** responses = (EdgeResponse **) EdgeMalloc(sizeof(EdgeResponse *) * resultMsg->responseLength);
        for (int i = 0; i < resultMsg->responseLength; i++)
        {
            responses[i] = (EdgeResponse*) EdgeCalloc(1, sizeof(EdgeResponse));
            EdgeVersatility *message = (EdgeVersatility *) EdgeCalloc(1, sizeof(EdgeVersatility));
            if(IS_NULL(message))
            {
                EDGE_LOG(TAG, "Error : Malloc failed for EdgeVersatility sendErrorResponse\n");
                goto EXIT;
            }
            const char *err_desc = "Error in executing METHOD OPERATION";
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
            EDGE_LOG(TAG, "Error : Malloc failed for result in executeMethod\n");
            goto EXIT_ERROR;
        }
        res->code = STATUS_ERROR;
        resultMsg->result = res;

        add_to_recvQ(resultMsg);
        error_flag = false;

        EXIT_ERROR:
        if (error_flag)
        {
            if(IS_NOT_NULL(resultMsg))
            {
                EdgeFree(resultMsg->endpointInfo);
                EdgeFree(resultMsg);
            }
            EdgeFree(res);
        }
    }

    for (idx = 0; idx < num_inpArgs; idx++)
    {
        UA_Variant_deleteMembers(&input[idx]);
    }

    EdgeFree(input);
    return result;
}
