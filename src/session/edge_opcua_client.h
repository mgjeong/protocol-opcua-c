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

#ifdef ENABLE_SUB_QUEUE
#include "edge_utils.h"
#include "edge_map.h"
#endif

#include <open62541.h>

#ifdef __cplusplus
extern "C"
{
#endif

#define LIST_HEAD(name, type) struct name { struct type *lh_first;} \

typedef enum {
    UA_SECURECHANNELSTATE_FRESH,
    UA_SECURECHANNELSTATE_OPEN,
    UA_SECURECHANNELSTATE_CLOSED
} UA_SecureChannelState;

typedef enum {
    UA_CLIENTAUTHENTICATION_NONE,
    UA_CLIENTAUTHENTICATION_USERNAME
} UA_Client_Authentication;

struct UA_SecureChannel {
    UA_SecureChannelState   state;
    UA_MessageSecurityMode  securityMode;
    UA_ChannelSecurityToken securityToken; /* the channelId is contained in the securityToken */
    UA_ChannelSecurityToken nextSecurityToken;

    /* The endpoint and context of the channel */
    const UA_SecurityPolicy *securityPolicy;
    void *channelContext; /* For interaction with the security policy */
    UA_Connection *connection;

    /* Asymmetric encryption info */
    UA_ByteString remoteCertificate;
    UA_Byte remoteCertificateThumbprint[20]; /* The thumbprint of the remote certificate */

    /* Symmetric encryption info */
    UA_ByteString remoteNonce;
    UA_ByteString localNonce;

    UA_UInt32 receiveSequenceNumber;
    UA_UInt32 sendSequenceNumber;

    LIST_HEAD(session_pointerlist, UA_SessionHeader) sessions;
    LIST_HEAD(chunk_pointerlist, ChunkEntry) chunks;
};

typedef struct UA_Client {
    /* State */
    UA_ClientState state;

    UA_ClientConfig config;

    /* Connection */
    UA_Connection connection;
    UA_String endpointUrl;

    /* SecureChannel */
    UA_SecurityPolicy securityPolicy;
    UA_SecureChannel channel;
    UA_UInt32 requestId;
    UA_DateTime nextChannelRenewal;

    /* Authentication */
    UA_Client_Authentication authenticationMethod;
    UA_String username;
    UA_String password;

    /* Session */
    UA_UserTokenPolicy token;
    UA_NodeId authenticationToken;
    UA_UInt32 requestHandle;

    /* Async Service */
    LIST_HEAD(ListOfAsyncServiceCall, AsyncServiceCall) asyncServiceCalls;

    /* Subscriptions */
#ifdef UA_ENABLE_SUBSCRIPTIONS
    UA_UInt32 monitoredItemHandles;
    LIST_HEAD(ListOfUnacknowledgedNotifications, UA_Client_NotificationsAckNumber) pendingNotificationsAcks;
    LIST_HEAD(ListOfClientSubscriptionItems, UA_Client_Subscription) subscriptions;
#endif
} UA_Client;

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
EdgeResult client_findServers(const char *endpointUri, size_t serverUrisSize,
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
EdgeResult client_getEndpoints(char *endpointUri);

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
 */
void browseNodesInServer(EdgeMessage *msg);

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
void registerClientCallback(response_cb_t resCallback, status_cb_t statusCallback, discovery_cb_t discoveryCallback);

#ifdef ENABLE_SUB_QUEUE
keyValue getSessionClient(char *endpoint);
#endif

#ifdef __cplusplus
}
#endif

#endif
