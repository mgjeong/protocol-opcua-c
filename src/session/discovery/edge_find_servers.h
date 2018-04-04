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
 * @file edge_find_servers.h
 *
 * @brief This file contains APIs to find registered servers at a given server.
 */

#ifndef EDGE_FIND_SERVERS_H
#define EDGE_FIND_SERVERS_H

#include "opcua_common.h"

#ifdef __cplusplus
extern "C"
{
#endif

/**
 * @brief Gets a list of all registered servers at the given server. Application has to free the memory \n
                   allocated for the resultant array of EdgeApplicationConfig objects and its members.
 * @param[in]  endpointUri Endpoint Uri.
 * @param[in]  serverUrisSize Optional filter for specific server uris
 * @param[in]  serverUris Optional filter for specific server uris
 * @param[in]  localeIdsSize Optional indication which locale you prefer
 * @param[in]  localeIds Optional indication which locale you prefer
 * @param[out]  registeredServersSize Number of registered Servers matching the filter criteria
 * @param[out]  registeredServers Application configuration information of the servers matching the filter criteria
 * @return @c EdgeResult code is 0 on success, otherwise an error value
 * @retval #STATUS_OK Successful
 * @retval #STATUS_PARAM_INVALID Invalid parameter
 * @retval #STATUS_ERROR Operation failed
 */
EdgeResult findServersInternal(const char *endpointUri, size_t serverUrisSize,
        unsigned char **serverUris, size_t localeIdsSize, unsigned char **localeIds,
        size_t *registeredServersSize, EdgeApplicationConfig **registeredServers);

#ifdef __cplusplus
}
#endif

#endif // EDGE_FIND_SERVERS_H