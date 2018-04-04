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
 * @file edge_get_endpoints.h
 *
 * @brief This file contains APIs to get the endpoints supported by a Server.
 */

#ifndef EDGE_GET_ENDPOINTS_H
#define EDGE_GET_ENDPOINTS_H

#include "opcua_common.h"
#include "command_adapter.h"

#ifdef __cplusplus
extern "C"
{
#endif

/**
 * @brief Gets the detailed end point information
 * @param[in]  endpointUri Endpoint Uri.
 * @return @c EdgeResult code is 0 on success, otherwise an error value
 * @retval #STATUS_OK Successful
 * @retval #STATUS_PARAM_INVALID Invalid parameter
 * @retval #STATUS_ERROR Operation failed
 */
EdgeResult getEndpointsInternal(char *endpointUri);

/**
 * @brief Register the client callback to receive the endpoint information.
 * @param[in]  discoveryCallback Client callback.
 */
void registerGetEndpointsCb(discovery_cb_t discoveryCallback);

#ifdef __cplusplus
}
#endif

#endif // EDGE_GET_ENDPOINTS_H