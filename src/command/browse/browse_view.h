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
 * @file browse_view.h
 *
 * @brief This file contains APIs for BrowseView request.
 */

#ifndef EDGE_BROWSE_VIEW_H
#define EDGE_BROWSE_VIEW_H

#include "opcua_common.h"
#include "open62541.h"

#ifdef __cplusplus
extern "C"
{
#endif

/**
 * @brief Executes BrowseView operation.
 * @param[in]  client Client Handle.
 * @param[in]  msg EdgeMessage request data.
 */
void browseView(UA_Client *client, EdgeMessage *msg);

#ifdef __cplusplus
}
#endif

#endif // EDGE_BROWSE_VIEW_H