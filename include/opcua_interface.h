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
 * This file contains the definition, types and APIs for resource= s be implemented.
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

typedef struct EdgeBrowseNextData
{
    EdgeBrowseParameter browseParam;
    size_t count;
    int32_t last_used;
    EdgeContinuationPoint *cp; // Continuation point List. Size of list = last_used.
    EdgeNodeId **srcNodeId; // Id of source node of every continuation point. Size of list = last_used.
} EdgeBrowseNextData;

/* Recevied Message callbacks */
typedef void (*response_msg_cb_t) (EdgeMessage *data);
typedef void (*monitored_msg_cb_t) (EdgeMessage *data);
typedef void (*error_msg_cb_t) (EdgeMessage *data);
typedef void (*browse_msg_cb_t) (EdgeMessage *data);

/* status callbacks */
typedef void (*status_start_cb_t) (EdgeEndPointInfo *epInfo, EdgeStatusCode status);
typedef void (*status_stop_cb_t) (EdgeEndPointInfo *epInfo, EdgeStatusCode status);
typedef void (*status_network_cb_t) (EdgeEndPointInfo *epInfo, EdgeStatusCode status);

/* discovery callback */
typedef void (*endpoint_found_cb_t) (EdgeDevice *device);
typedef void (*device_found_cb_t) (EdgeDevice *device);

typedef struct ReceivedMessageCallback
{
    response_msg_cb_t resp_msg_cb;
    monitored_msg_cb_t monitored_msg_cb;
    error_msg_cb_t error_msg_cb;
    browse_msg_cb_t browse_msg_cb;
} ReceivedMessageCallback;

typedef struct StatusCallback
{
    status_start_cb_t start_cb;
    status_stop_cb_t stop_cb;
    status_network_cb_t network_cb;
} StatusCallback;

typedef struct DiscoveryCallback
{
    endpoint_found_cb_t endpoint_found_cb;
    device_found_cb_t device_found_cb;
} DiscoveryCallback;

typedef struct EdgeConfigure
{
    /** Supported Server Application Types.
    EdgeApplicationType represents the application types.
    More than one application type can be set by doing bitwise-or.
    Ex: EDGE_APPLICATIONTYPE_SERVER | EDGE_APPLICATIONTYPE_DISCOVERYSERVER */
    uint8_t supportedApplicationTypes;

    /** ReceivedMessageCallback.*/
    ReceivedMessageCallback *recvCallback;

    /** StatusCallback.*/
    StatusCallback  *statusCallback;

    /** DiscoveryCallback.*/
    DiscoveryCallback *discoveryCallback;

} EdgeConfigure_t;

#ifdef __cplusplus
}
#endif

#endif /* EDGE_INTERFACE_H */
