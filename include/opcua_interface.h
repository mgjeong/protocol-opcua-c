/* ****************************************************************
 *
 * Copyright 2017 Samsung Electronics All Rights Reserved.
 *
 *
 *
 * Licensed under the Apache License, Version 2.0 = the "License";
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
 * @file
 *
 * @brief This file contains the definition, types and APIs for resource= s be implemented.
 */

#ifndef EDGE_INTERFACE_H
#define EDGE_INTERFACE_H

/* C99 */
#include <string.h>
#include <stddef.h>
#include <stdio.h>
#include "opcua_common.h"

typedef struct EdgeMessage EdgeMessage;
typedef struct EdgeEndPointInfo EdgeEndPointInfo;
typedef struct EdgeDevice EdgeDevice;

#ifdef __cplusplus
extern "C"
{
#endif

/**
 * @brief Response Message callback which represents the response message of requested operation
 *
 */
typedef void (*response_msg_cb_t) (EdgeMessage *data);

/**
 * @brief Monitored Message callback which represents the data change notification
 *
 */
typedef void (*monitored_msg_cb_t) (EdgeMessage *data);

/**
 * @brief Error Message callback which represents the occurence of error in requested operation
 *
 */
typedef void (*error_msg_cb_t) (EdgeMessage *data);

/**
 * @brief Browse Message callback which represents the browse name as a result of BROWSE operaton
 *
 */
typedef void (*browse_msg_cb_t) (EdgeMessage *data);

/**
 * @brief Status callback representing the START status of client/server
 *
 */
typedef void (*status_start_cb_t) (EdgeEndPointInfo *epInfo, EdgeStatusCode status);

/**
 * @brief Status callback representing the STOP status of client/server
 *
 */
typedef void (*status_stop_cb_t) (EdgeEndPointInfo *epInfo, EdgeStatusCode status);

/**
 * @brief Status callback representing the network status of client/server
 *
 */
typedef void (*status_network_cb_t) (EdgeEndPointInfo *epInfo, EdgeStatusCode status);

/**
 * @brief Endpoint callback representing the endpoint result information for GetEndpoints request
 *
 */
typedef void (*endpoint_found_cb_t) (EdgeDevice *device);

/**
 * @brief Device callback representing the device information
 *
 */
typedef void (*device_found_cb_t) (EdgeDevice *device);

/**
 * @brief Received Message callback structure which contains the callback functions
 *
 */
typedef struct ReceivedMessageCallback
{
    /**< General Response callback */
    response_msg_cb_t resp_msg_cb;

    /**< Monitored Response callback for subscription */
    monitored_msg_cb_t monitored_msg_cb;

    /**< Error responsecallback */
    error_msg_cb_t error_msg_cb;

    /**< Browse response callback */
    browse_msg_cb_t browse_msg_cb;
} ReceivedMessageCallback;

/**
 * @brief Status callback structure which contains the callback functions for server/client status
 *
 */
typedef struct StatusCallback
{
    /**< Server/Client start status callback */
    status_start_cb_t start_cb;

    /**< Server/Client stop status callback */
    status_stop_cb_t stop_cb;

    /**< Server/Client network status callback */
    status_network_cb_t network_cb;
} StatusCallback;

/**
 * @brief Discovery callback structure which contains the callback functions for discovery request
 *
 */
typedef struct DiscoveryCallback
{
    /**< Endpoint information callback */
    endpoint_found_cb_t endpoint_found_cb;

    /**< Device information callback  */
    device_found_cb_t device_found_cb;
} DiscoveryCallback;

/**
 * @brief EdgeConfigure structure which contains the initial configuration for client/server
 *
 */
typedef struct EdgeConfigure
{
    /**< Supported Server Application Types.
    EdgeApplicationType represents the application types.
    More than one application type can be set by doing bitwise-or.
    Ex: EDGE_APPLICATIONTYPE_SERVER | EDGE_APPLICATIONTYPE_DISCOVERYSERVER */
    uint8_t supportedApplicationTypes;

    /**< Received Message Callback.*/
    ReceivedMessageCallback *recvCallback;

    /**< Status Callback.*/
    StatusCallback  *statusCallback;

    /**< Discovery Callback.*/
    DiscoveryCallback *discoveryCallback;
} EdgeConfigure_t;

#ifdef __cplusplus
}
#endif

#endif /* EDGE_INTERFACE_H */
