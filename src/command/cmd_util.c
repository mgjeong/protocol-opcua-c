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
#include "edge_open62541.h"
#include "message_dispatcher.h"

#define TAG "cmd_util"
#define ERROR_DESC_LENGTH (100)

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

/**
 * @brief convertToEdgeLocalizedText - Util function to handle LocalizedText read response
 * @param lt - Localized text to be handled
 * @return Edge_LocalizedText*
 */
static Edge_LocalizedText *convertToEdgeLocalizedText(UA_LocalizedText *lt)
{
    VERIFY_NON_NULL_MSG(lt, "Input parameter(lt) is NULL", NULL);
    Edge_LocalizedText *value = (Edge_LocalizedText *) EdgeCalloc(1, sizeof(Edge_LocalizedText));
    VERIFY_NON_NULL_MSG(value, "Memory allocation failed.", NULL);

    Edge_String *edgeStr = convertToEdgeString(&lt->locale);
    if(IS_NULL(edgeStr))
    {
        EDGE_LOG(TAG, "Failed to convert locale.");
        EdgeFree(value);
        return NULL;
    }
    value->locale = *edgeStr;
    EdgeFree(edgeStr);

    edgeStr = convertToEdgeString(&lt->text);
    if(IS_NULL(edgeStr))
    {
        EDGE_LOG(TAG, "Failed to convert text.");
        EdgeFree(value->locale.data);
        EdgeFree(value);
        return NULL;
    }
    value->text = *edgeStr;
    EdgeFree(edgeStr);
    return value;
}

/**
 * @brief convertToEdgeQualifiedName - Utility function to handle QualifiedName response
 * @param qn - Qualified name to be handled
 * @return Edge_QualifiedName*
 */
static Edge_QualifiedName *convertToEdgeQualifiedName(UA_QualifiedName *qn)
{
    VERIFY_NON_NULL_MSG(qn, "Input parameter(qn) is NULL", NULL);
    Edge_QualifiedName *value = (Edge_QualifiedName *) EdgeCalloc(1, sizeof(Edge_QualifiedName));
    VERIFY_NON_NULL_MSG(value, "Memory allocation failed.", NULL);

    value->namespaceIndex = qn->namespaceIndex;
    Edge_String *edgeStr = convertToEdgeString(&qn->name);
    if(IS_NULL(edgeStr))
    {
        EDGE_LOG(TAG, "Failed to convert name.");
        EdgeFree(value);
        return NULL;
    }
    value->name = *edgeStr;
    EdgeFree(edgeStr);
    return value;
}

EdgeVersatility* parseResponse(EdgeResponse *response, UA_Variant val)
{
    char errorDesc[ERROR_DESC_LENGTH] = {'\0'};
    EdgeVersatility *versatility = NULL;
    bool isScalar = false;

    isScalar = UA_Variant_isScalar(&val);
    if (!isScalar && val.arrayLength == 0)
        return versatility;

    response->type = get_response_type(val.type);
    if (response->type < 0)
        return versatility;

    versatility = (EdgeVersatility*) EdgeCalloc(1, sizeof(EdgeVersatility));

    if (isScalar)
    {
        /* Scalar response handling */
        versatility->arrayLength = 0;
        versatility->isArray = false;
        size_t size = get_size(response->type, false);
        if ((response->type == UA_NS0ID_STRING) || (response->type == UA_NS0ID_BYTESTRING)
                || (response->type == UA_NS0ID_XMLELEMENT))
        {
            /* STRING or BYTESTRING or XMLELEMENT scalar response handling */
            UA_String str = *((UA_String *) val.data);
            size_t len = str.length;
            versatility->value = (void *) EdgeCalloc(1, len+1);
            if(IS_NULL(versatility->value))
            {
                EDGE_LOG(TAG, "Memory allocation failed.");
                strncpy(errorDesc, "Memory allocation failed.", ERROR_DESC_LENGTH);
                freeEdgeResponse(response);
                goto EXIT;
            }
            strncpy(versatility->value, (char*) str.data, len);
           ((char*) versatility->value)[(int) len] = '\0';
        }
        else if (response->type == UA_NS0ID_GUID)
        {
            /* GUID scalar response handling */
            UA_Guid str = *((UA_Guid *) val.data);
            char *value = (char *) EdgeMalloc(GUID_LENGTH + 1);
            if(IS_NULL(value))
            {
                EDGE_LOG(TAG, "Error : Malloc failed for Guid SCALAR value in Read Group\n");
                strncpy(errorDesc, "Memory allocation failed.", ERROR_DESC_LENGTH);
                freeEdgeResponse(response);
                goto EXIT;
            }

            convertGuidToString(str, &value);
            versatility->value = (void *) value;

            EDGE_LOG_V(TAG, "%s\n", value);
        }
        else if(response->type == UA_NS0ID_LOCALIZEDTEXT)
        {
            /* LOCALIZEDTEXT scalar response handling */
            Edge_LocalizedText *value = convertToEdgeLocalizedText((UA_LocalizedText *) val.data);
            if(IS_NULL(value))
            {
                EDGE_LOG(TAG, "Failed to parse localized text.");
                strncpy(errorDesc, "Failed to parse localized text.", ERROR_DESC_LENGTH);
                freeEdgeResponse(response);
                goto EXIT;
            }
            versatility->value = value;
        }
        else if(response->type == UA_NS0ID_QUALIFIEDNAME)
        {
            /* QUALIFIEDNAME scalar response handling */
            Edge_QualifiedName *value = convertToEdgeQualifiedName((UA_QualifiedName *) val.data);
            if(IS_NULL(value))
            {
                EDGE_LOG(TAG, "Failed to convert qualified name.");
                strncpy(errorDesc, "Failed to convert qualified name.", ERROR_DESC_LENGTH);
                freeEdgeResponse(response);
                goto EXIT;
            }
            versatility->value = value;
        }
        else if(response->type == UA_NS0ID_NODEID)
        {
            /* NODEID scalar response handling */
            versatility->value = convertToEdgeNodeIdType((UA_NodeId *) val.data);
            if(IS_NULL(versatility->value))
            {
                EDGE_LOG(TAG, "Failed to convert NodeId.");
                strncpy(errorDesc, "Failed to convert NodeId.", ERROR_DESC_LENGTH);
                freeEdgeResponse(response);
                goto EXIT;
            }
        }
        else
        {
            /* Response handling for other array data types */
            versatility->value = (void *) EdgeCalloc(1, size);
            if(IS_NULL(versatility->value))
            {
                EDGE_LOG(TAG, "Memory allocation failed.");
                strncpy(errorDesc, "Memory allocation failed.", ERROR_DESC_LENGTH);
                freeEdgeResponse(response);
                goto EXIT;
            }
            memcpy(versatility->value, val.data, size);
        }
    }
    else
    {
        /* Array response handling */
        versatility->arrayLength = val.arrayLength;
        versatility->isArray = true;
        if (response->type == UA_NS0ID_STRING || response->type == UA_NS0ID_BYTESTRING
                || response->type == UA_NS0ID_XMLELEMENT)
        {
            /* STRING or BYTESTRING or XMLELEMENT array response handling */
            UA_String *str = ((UA_String *) val.data);
            versatility->value = EdgeCalloc (val.arrayLength, sizeof(char *));
            if(IS_NULL(versatility->value))
            {
                EDGE_LOG(TAG, "Error : Malloc failed for String Array values in Read Group\n");
                strncpy(errorDesc, "Memory allocation failed.", ERROR_DESC_LENGTH);
                freeEdgeResponse(response);
                goto EXIT;
            }

            char **values = (char **) versatility->value;
            for (int j = 0; j < val.arrayLength; j++)
            {
                values[j] = (char *) EdgeMalloc(str[j].length + 1);
                if(IS_NULL(values[j]))
                {
                    EDGE_LOG_V(TAG, "Error : Malloc failed for ByteString Array value %d in Read Group\n", j);
                    strncpy(errorDesc, "Memory allocation failed.", ERROR_DESC_LENGTH);
                    freeEdgeResponse(response);
                    goto EXIT;
                }
                strncpy(values[j], (char *) str[j].data, str[j].length);
                values[j][str[j].length] = '\0';
            }
        }
        else if (response->type == UA_NS0ID_GUID)
        {
            /* GUID Array response handling */
            UA_Guid *str = ((UA_Guid *) val.data);
            versatility->value = EdgeCalloc(val.arrayLength, sizeof(char *));
            if(IS_NULL(versatility->value))
            {
                EDGE_LOG(TAG, "Error : Malloc failed for Guid Array values in Read Group\n");
                strncpy(errorDesc, "Memory allocation failed.", ERROR_DESC_LENGTH);
                freeEdgeResponse(response);
                goto EXIT;
            }

            char **values = (char **) versatility->value;
            for (int j = 0; j < val.arrayLength; j++)
            {
                values[j] = (char *) EdgeMalloc(GUID_LENGTH + 1);
                if(IS_NULL(values[j]))
                {
                    EDGE_LOG_V(TAG, "Error : Malloc failed for Guid Array value %d in Read Group\n", j);
                    strncpy(errorDesc, "Memory allocation failed.", ERROR_DESC_LENGTH);
                    freeEdgeResponse(response);
                    goto EXIT;
                }
                convertGuidToString(str[j], &(values[j]));
                EDGE_LOG_V(TAG, "%s\n", values[j]);
            }
        }
        else if(response->type == UA_NS0ID_QUALIFIEDNAME)
        {
            /* QUALIFIEDNAME array response handling */
            UA_QualifiedName *qnArr = ((UA_QualifiedName *) val.data);
            versatility->value = EdgeCalloc(val.arrayLength, sizeof(UA_QualifiedName *));
            if(IS_NULL(versatility->value))
            {
                EDGE_LOG(TAG, "Error : Malloc failed for QualifiedName Array values in Read Group\n");
                strncpy(errorDesc, "Memory allocation failed.", ERROR_DESC_LENGTH);
                freeEdgeResponse(response);
                goto EXIT;
            }

            Edge_QualifiedName **values = (Edge_QualifiedName **) versatility->value;
            for (int j = 0; j < val.arrayLength; j++)
            {
                values[j] = convertToEdgeQualifiedName(&qnArr[j]);
                if(IS_NULL(values[j]))
                {
                    EDGE_LOG(TAG, "Failed to convert the qualified name.");
                    strncpy(errorDesc, "Failed to convert the qualified name.", ERROR_DESC_LENGTH);
                    freeEdgeResponse(response);
                    goto EXIT;
                }
            }
        }
        else if(response->type == UA_NS0ID_LOCALIZEDTEXT)
        {
            /* LOCALIZEDTEXT array response handling */
            UA_LocalizedText *ltArr = ((UA_LocalizedText *) val.data);
            versatility->value = EdgeCalloc(val.arrayLength, sizeof(UA_LocalizedText *));
            if(IS_NULL(versatility->value))
            {
                EDGE_LOG(TAG, "Error : Malloc failed for LocalizedText Array values in Read Group\n");
                strncpy(errorDesc, "Memory allocation failed.", ERROR_DESC_LENGTH);
                freeEdgeResponse(response);
                goto EXIT;
            }

            Edge_LocalizedText **values = (Edge_LocalizedText **) versatility->value;
            for (int j = 0; j < val.arrayLength; j++)
            {
                values[j] = convertToEdgeLocalizedText(&ltArr[j]);
                if(IS_NULL(values[j]))
                {
                    EDGE_LOG(TAG, "Failed to convert the localized text.");
                    strncpy(errorDesc, "Failed to convert the localized text.", ERROR_DESC_LENGTH);
                    freeEdgeResponse(response);
                    goto EXIT;
                }
            }
        }
        else if(response->type == UA_NS0ID_NODEID)
        {
            /* NODEID array response handling */
            UA_NodeId *nodeIdArr = ((UA_NodeId *) val.data);
            versatility->value = EdgeCalloc(val.arrayLength, sizeof(Edge_NodeId *));
            if(IS_NULL(versatility->value))
            {
                EDGE_LOG(TAG, "Error : Malloc failed for NodeId Array values in Read Group\n");
                strncpy(errorDesc, "Memory allocation failed.", ERROR_DESC_LENGTH);
                freeEdgeResponse(response);
                goto EXIT;
            }

            Edge_NodeId **values = (Edge_NodeId **) versatility->value;
            for (int j = 0; j < val.arrayLength; j++)
            {
                values[j] = convertToEdgeNodeIdType(&nodeIdArr[j]);
                if(IS_NULL(values[j]))
                {
                    EDGE_LOG(TAG, "Failed to convert the NodeId.");
                    strncpy(errorDesc, "Failed to convert the NodeId.", ERROR_DESC_LENGTH);
                    freeEdgeResponse(response);
                    goto EXIT;
                }
            }
        }
        else
        {
            /* Response handling for other array data types */
            if(IS_NULL(val.type))
            {
                EDGE_LOG(TAG, "Vaue type is NULL ERROR.");
                strncpy(errorDesc, "Vaue type is NULL ERROR..", ERROR_DESC_LENGTH);
                freeEdgeResponse(response);
                goto EXIT;
            }

            versatility->value = (void *) EdgeCalloc(versatility->arrayLength, val.type->memSize);
            if(IS_NULL(versatility->value))
            {
                EDGE_LOG(TAG, "Memory allocation failed.");
                strncpy(errorDesc, "Memory allocation failed.", ERROR_DESC_LENGTH);
                freeEdgeResponse(response);
                goto EXIT;
            }

            memcpy(versatility->value, val.data, val.type->memSize * versatility->arrayLength);
        }
    }

    return versatility;

    EXIT:
    freeEdgeVersatility(versatility);
    return NULL;
}
