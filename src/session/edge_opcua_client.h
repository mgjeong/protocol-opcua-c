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
 * @file edge_opcua_client.h
 *
 * @brief This file contains the definition, types and APIs for client requests
 */

#ifndef EDGE_OPCUA_CLIENT_H
#define EDGE_OPCUA_CLIENT_H

#include <stdbool.h>

#include "opcua_common.h"
#include "command_adapter.h"

#ifdef __cplusplus
extern "C"
{
#endif

/**
 * @brief Set the supported application types
 * @param[in]  supportedTypes application types to be supported
 */
void setSupportedApplicationTypes(uint8_t supportedTypes);

/**
 * @brief Establishes client connection
 * @param[in]  endpoint Endpoint Uri
 * @return @c true on success, false in case of error
 * @retval #true Successful
 * @retval #false Error
 */
bool connect_client(char *endpoint);

/**
 * @brief Close the client connection
 * @param[in]  epInfo Endpoint information
 */
void disconnect_client(EdgeEndPointInfo *epInfo);

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


/**
 * @brief Gets the detailed end point information
 * @param[in]  endpointUri Endpoint Uri.
 * @return @c EdgeResult code is 0 on success, otherwise an error value
 * @retval #STATUS_OK Successful
 * @retval #STATUS_PARAM_INVALID Invalid parameter
 * @retval #STATUS_ERROR Operation failed
 */
EdgeResult getClientEndpoints(char *endpointUri);

/**
 * @brief Send the read request data to server
 * @param[in]  msg EdgeMessage request data.
 * @return @c EdgeResult code is 0 on success, otherwise an error value
 * @retval #STATUS_OK Successful
 * @retval #STATUS_PARAM_INVALID Invalid parameter
 * @retval #STATUS_ERROR Operation failed
 */
EdgeResult readNodesFromServer(EdgeMessage *msg);

/**
 * @brief Send the write request data to server
 * @param[in]  msg EdgeMessage request data.
 * @return @c EdgeResult code is 0 on success, otherwise an error value
 * @retval #STATUS_OK Successful
 * @retval #STATUS_PARAM_INVALID Invalid parameter
 * @retval #STATUS_ERROR Operation failed
 */
EdgeResult writeNodesInServer(EdgeMessage *msg);

/**
 * @brief Send the browse request data to server
 * @param[in]  msg EdgeMessage request data.
 * @return @c EdgeResult code is 0 on success, otherwise an error value
 * @retval #STATUS_OK Successful
 * @retval #STATUS_PARAM_INVALID Invalid parameter
 * @retval #STATUS_ERROR Operation failed
 */
EdgeResult browseNodesInServer(EdgeMessage *msg);

/**
 * @brief Send the browse view request data to server
 * @param[in]  msg EdgeMessage request data.
 * @return @c EdgeResult code is 0 on success, otherwise an error value
 * @retval #STATUS_OK Successful
 * @retval #STATUS_PARAM_INVALID Invalid parameter
 * @retval #STATUS_ERROR Operation failed
 */
EdgeResult browseViewsInServer(EdgeMessage *msg);

/**
 * @brief Send the browse next request data to server
 * @param[in]  msg EdgeMessage request data.
 * @return @c EdgeResult code is 0 on success, otherwise an error value
 * @retval #STATUS_OK Successful
 * @retval #STATUS_PARAM_INVALID Invalid parameter
 * @retval #STATUS_ERROR Operation failed
 */
EdgeResult browseNextInServer(EdgeMessage *msg);

/**
 * @brief Send the Method Call request data to server
 * @param[in]  msg EdgeMessage request data.
 * @return @c EdgeResult code is 0 on success, otherwise an error value
 * @retval #STATUS_OK Successful
 * @retval #STATUS_PARAM_INVALID Invalid parameter
 * @retval #STATUS_ERROR Operation failed
 */
EdgeResult callMethodInServer(EdgeMessage *msg);

/**
 * @brief Send the Subscription request data to server
 * @param[in]  msg EdgeMessage request data.
 * @return @c EdgeResult code is 0 on success, otherwise an error value
 * @retval #STATUS_OK Successful
 * @retval #STATUS_PARAM_INVALID Invalid parameter
 * @retval #STATUS_ERROR Operation failed
 */
EdgeResult executeSubscriptionInServer(EdgeMessage *msg);

/**
 * @brief Register the client callback function
 * @param[in]  resCallback response callback
 * @param[in]  statusCallback status callback
 * @param[in]  discoveryCallback Discovery callback
 */
void resgisterClientCallback(response_cb_t resCallback, status_cb_t statusCallback, discovery_cb_t discoveryCallback);

#ifdef __cplusplus
}
#endif

#endif
