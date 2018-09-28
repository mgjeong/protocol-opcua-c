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

/**
 * @file cmd_util.h
 *
 * @brief This file contains the Utility functions for commands.
 */

#ifndef EDGE_CMD_UTIL_H
#define EDGE_CMD_UTIL_H

#include "opcua_common.h"
#include "open62541.h"

/**
 * @brief Get the numeric identifier of the data type.
 * @param[in]  type UA_DataType.
 * @return Numeric identifier of the given data type.
 */
int get_response_type(const UA_DataType *type);

/**
 * @brief Sends error response message
 * @param[in]  msg EdgeMessage
 * @param[in]  err_desc error message description
 */
void sendErrorResponse(const EdgeMessage *msg, char *err_desc);

/**
 * @brief Get the data type of the response message
 * @param[in]  nodesToProcess number of nodes
 * @param[in]  diagnosticInfo Diagnostics information
 * @param[in]  diagnosticInfoLength Diagnostic information length
 * @param[in]  returnDiagnostic Return Diagnostic
 * @return EdgeDiagnosticInfo object
 */
EdgeDiagnosticInfo *checkDiagnosticInfo(int nodesToProcess,
        UA_DiagnosticInfo *diagnosticInfo, int diagnosticInfoLength, int returnDiagnostic);


EdgeVersatility* parseResponse(EdgeResponse *response, UA_Variant val);


#endif // EDGE_CMD_UTIL_H
