#include "method.h"

#include <stdio.h>

EdgeResult executeMethod(UA_Client *client, EdgeMessage *msg)
{
    EdgeResult result;
    result.code = STATUS_OK;
    if (!client)
    {
        printf("Client handle Invalid\n");
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
            int type = (int)params->inpArg[idx]->argType - 1;
            if (type == UA_TYPES_STRING)
            {
                UA_String val = UA_STRING_ALLOC((char *) params->inpArg[idx]->scalarValue);
                UA_Variant_setScalarCopy(&input[idx], &val, &UA_TYPES[type]);
                UA_String_deleteMembers(&val);
            }
            else
            {
                printf("\n\n======== scalar copy======== \n\n");
                UA_Variant_setScalarCopy(&input[idx], params->inpArg[idx]->scalarValue, &UA_TYPES[type]);
            }
        }
        else if (params->inpArg[idx]->valType == ARRAY_1D)
        {
            int type = (int)params->inpArg[idx]->argType - 1;
            if (type == UA_TYPES_STRING)
            {
                char **data = (char **) params->inpArg[idx]->arrayData;
                UA_String *array = (UA_String *) UA_Array_new(params->inpArg[idx]->arrayLength, &UA_TYPES[type]);
                for (int idx1 = 0; idx1 < params->inpArg[idx]->arrayLength; idx1++)
                {
                    array[idx1] = UA_STRING_ALLOC(data[idx1]);
                }
                UA_Variant_setArrayCopy(&input[idx], array, params->inpArg[idx]->arrayLength, &UA_TYPES[type]);
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
                                          UA_NODEID_STRING(1, request->nodeInfo->valueAlias), params->num_inpArgs, input, &outputSize,
                                          &output);
    if (retVal == UA_STATUSCODE_GOOD)
    {
        printf("method call was success\n\n");

        EdgeResponse **response = (EdgeResponse **) malloc(sizeof(EdgeResponse *) * outputSize);
        if (response)
        {
            for (int i = 0 ; i < outputSize; i++)
            {
                response[i] = (EdgeResponse *) malloc(sizeof(EdgeResponse));

                response[i]->nodeInfo = (EdgeNodeInfo *) malloc(sizeof(EdgeNodeInfo));
                memcpy(response[i]->nodeInfo, msg->request->nodeInfo, sizeof(EdgeNodeInfo));
                response[i]->requestId = msg->request->requestId;
//        response[i]->value = output[i].data;

                EdgeVersatility *versatility = (EdgeVersatility *) malloc(sizeof(EdgeVersatility));
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
            resultMsg->endpointInfo = (EdgeEndPointInfo *) malloc(sizeof(EdgeEndPointInfo));
            memcpy(resultMsg->endpointInfo, msg->endpointInfo, sizeof(EdgeEndPointInfo));
            resultMsg->type = GENERAL_RESPONSE;
            resultMsg->responseLength = outputSize;
            //resultMsg->responses = (EdgeResponse**) malloc(1 * sizeof(EdgeResponse));
            resultMsg->responses = response;
            resultMsg->command = CMD_METHOD;

            onResponseMessage(resultMsg);

            for (int i = 0 ; i < outputSize; i++)
            {
                free(response[i]->nodeInfo); response[i]->nodeInfo = NULL;
                free(response[i]->message); response[i]->message = NULL;
                free(response[i]); response[i] = NULL;
            }
            free(response); response = NULL;
            free(resultMsg->endpointInfo); resultMsg->endpointInfo = NULL;
            free(resultMsg); resultMsg = NULL;

            result.code = STATUS_OK;
        }
    }
    else
    {
        printf("method call failed 0x%08x\n", retVal);
        result.code = STATUS_ERROR;

        EdgeMessage *resultMsg = (EdgeMessage *) malloc(sizeof(EdgeMessage));
        resultMsg->endpointInfo = (EdgeEndPointInfo *) malloc(sizeof(EdgeEndPointInfo));
        memcpy(resultMsg->endpointInfo, msg->endpointInfo, sizeof(EdgeEndPointInfo));
        resultMsg->type = ERROR;
        resultMsg->responseLength = 0;

        EdgeResult *res = (EdgeResult *) malloc(sizeof(EdgeResult));
        res->code = STATUS_ERROR;
        resultMsg->result = res;

        onResponseMessage(resultMsg);
        free(resultMsg->endpointInfo); resultMsg->endpointInfo = NULL;
        free(resultMsg); resultMsg = NULL;
        free(res); res = NULL;
    }

    for (idx = 0; idx < params->num_inpArgs; idx++)
    {
        UA_Variant_deleteMembers(&input[idx]);
    }

    if (input)
    {
        free(input);
        input = NULL;
    }

    return result;
}
