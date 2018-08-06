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
    int max = 20; // UA_TYPES_LOCALIZEDTEXT
    for (int i = 0; i <= max; i++)
    {
        COND_CHECK((datatype == &UA_TYPES[i]), i+1);
    }
    return -1;
}

void sendErrorResponse(const EdgeMessage *msg, char *err_desc)
{
    EdgeMessage *resultMsg = (EdgeMessage *) EdgeCalloc(1, sizeof(EdgeMessage));
    VERIFY_NON_NULL_NR_MSG(resultMsg, "EdgeCalloc FAILED for EdgeMessage in sendErrorResponse\n");
    resultMsg->endpointInfo = cloneEdgeEndpointInfo(msg->endpointInfo);
    resultMsg->type = ERROR_RESPONSE;
    resultMsg->responseLength = 1;
    resultMsg->message_id = msg->message_id;

    resultMsg->responses = (EdgeResponse **) malloc(sizeof(EdgeResponse *) * resultMsg->responseLength);
    for (int i = 0; i < resultMsg->responseLength; i++)
    {
        resultMsg->responses[i] = (EdgeResponse*) EdgeCalloc(1, sizeof(EdgeResponse));
        if(IS_NULL(resultMsg->responses[i]))
        {
            EDGE_LOG(TAG, "Error : Allocation has failed\n");
            goto EXIT;
        }

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

    /* Adding Error response message to receiver Q */
    add_to_recvQ(resultMsg);
    return ;

    EXIT:
    /* Free the memory */
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
        /* ReturnDiagnostic not requested */
        diagnostics->msg = NULL;
    }
    else if (diagnosticInfoLength == nodesToProcess)
    {
        /* Diagnostics information received from server */
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
        /* Diagnostic info requested. But no diagnostic info returned from server */
        diagnostics->msg =
                (void *) "no diagnostics were returned even though returnDiagnostic requested";
    }
    else
    {
        /* Diagnostic info length returned by server doesn't
         * match with the number of nodes processed */
        diagnostics->msg = (void *) "mismatch entries returned";
    }
    return diagnostics;
}
