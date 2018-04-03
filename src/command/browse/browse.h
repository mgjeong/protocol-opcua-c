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
 * @file browse.h
 *
 * @brief This file is the entry point for browse, browse view and browse next APIs.
 */

#ifndef EDGE_BROWSE_H
#define EDGE_BROWSE_H

#include "opcua_common.h"
#include "open62541.h"
#include "command_adapter.h"

#ifdef __cplusplus
extern "C"
{
#endif

/**
 * @brief Executes Browse operation
 * @param[in]  client Client Handle.
 * @param[in]  msg EdgeMessage request data
 */
void executeBrowse(UA_Client *client, EdgeMessage *msg);

/**
 * @brief Register callback for browse response
 * @param[in]  callback response callback
 */
void registerBrowseResponseCallback(response_cb_t callback);

#ifdef __cplusplus
}
#endif

#endif // EDGE_BROWSE_H