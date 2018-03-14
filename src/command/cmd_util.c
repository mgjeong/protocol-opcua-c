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

#include "cmd_util.h"
#include "edge_malloc.h"
#include "edge_utils.h"
#include "message_dispatcher.h"

#define TAG "cmd_util"

int get_response_type(const UA_DataType *datatype)
{
    if (datatype == &UA_TYPES[UA_TYPES_BOOLEAN])
    {
        return UA_NS0ID_BOOLEAN;
    }
    else if (datatype == &UA_TYPES[UA_TYPES_INT16])
    {
        return UA_NS0ID_INT16;
    }
    else if (datatype == &UA_TYPES[UA_TYPES_UINT16])
    {
        return UA_NS0ID_UINT16;
    }
    else if (datatype == &UA_TYPES[UA_TYPES_INT32])
    {
        return UA_NS0ID_INT32;
    }
    else if (datatype == &UA_TYPES[UA_TYPES_UINT32])
    {
        return UA_NS0ID_UINT32;
    }
    else if (datatype == &UA_TYPES[UA_TYPES_INT64])
    {
        return UA_NS0ID_INT64;
    }
    else if (datatype == &UA_TYPES[UA_TYPES_UINT64])
    {
        return UA_NS0ID_UINT64;
    }
    else if (datatype == &UA_TYPES[UA_TYPES_FLOAT])
    {
        return UA_NS0ID_FLOAT;
    }
    else if (datatype == &UA_TYPES[UA_TYPES_DOUBLE])
    {
        return UA_NS0ID_DOUBLE;
    }
    else if (datatype == &UA_TYPES[UA_TYPES_STRING])
    {
        return UA_NS0ID_STRING;
    }
    else if (datatype == &UA_TYPES[UA_TYPES_BYTESTRING])
    {
        return UA_NS0ID_BYTESTRING;
    }
    else if (datatype == &UA_TYPES[UA_TYPES_GUID])
    {
        return UA_NS0ID_GUID;
    }
    else if (datatype == &UA_TYPES[UA_TYPES_SBYTE])
    {
        return UA_NS0ID_SBYTE;
    }
    else if (datatype == &UA_TYPES[UA_TYPES_BYTE])
    {
        return UA_NS0ID_BYTE;
    }
    else if (datatype == &UA_TYPES[UA_TYPES_DATETIME])
    {
        return UA_NS0ID_DATETIME;
    }
    else if (datatype == &UA_TYPES[UA_TYPES_XMLELEMENT])
    {
        return UA_NS0ID_XMLELEMENT;
    }
    else if (datatype == &UA_TYPES[UA_TYPES_QUALIFIEDNAME])
    {
        return UA_NS0ID_QUALIFIEDNAME;
    }
    else if (datatype == &UA_TYPES[UA_TYPES_LOCALIZEDTEXT])
    {
        return UA_NS0ID_LOCALIZEDTEXT;
    }
    else if (datatype == &UA_TYPES[UA_TYPES_NODEID])
    {
        return UA_NS0ID_NODEID;
    }

    return -1;
}

void sendErrorResponse(const EdgeMessage *msg, char *err_desc)
{
    EdgeMessage *resultMsg = (EdgeMessage *) EdgeCalloc(1, sizeof(EdgeMessage));
    VERIFY_NON_NULL_NR_MSG(resultMsg, "EdgeCalloc FAILED for EdgeMessage in sendErrorResponse\n");
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

EdgeDiagnosticInfo *checkDiagnosticInfo(int nodesToProcess,
        UA_DiagnosticInfo *diagnosticInfo, int diagnosticInfoLength, int returnDiagnostic)
{
    EdgeDiagnosticInfo *diagnostics = (EdgeDiagnosticInfo *) EdgeMalloc(sizeof(EdgeDiagnosticInfo));
    VERIFY_NON_NULL_MSG(diagnostics, "EdgeMalloc FAILED for EdgeDiagnosticInfo in checkDiagnosticInfo\n", NULL);
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
