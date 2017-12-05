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

#ifndef EDGE_COMMON_H
#define EDGE_COMMON_H

//#include <open62541.h>

#include "edge_command_type.h"
#include "edge_identifier.h"
#include "edge_message_type.h"
#include "edge_node_identifier.h"
#include "edge_node_type.h"
#include "edge_opcua_common.h"
#include "edge_status_code.h"
#include "common_client.h"
#include "common_server.h"

#include "opcua_manager.h"

#ifdef __cplusplus
extern "C"
{
#endif

#define REQUEST_TIMEOUT     60000
#define BIND_PORT                12686

typedef struct ReceivedMessageCallback ReceivedMessageCallback;
typedef struct StatusCallback StatusCallback;
typedef struct DiscoveryCallback DiscoveryCallback;

typedef struct EdgeConfigure
{
    /** ReceivedMessageCallback.*/
    ReceivedMessageCallback *recvCallback;

    /** StatusCallback.*/
    StatusCallback  *statusCallback;

    /** DiscoveryCallback.*/
    DiscoveryCallback *discoveryCallback;

} EdgeConfigure;

typedef struct EdgeBrowseResult
{
    /** browseName.*/
    char *browseName;

} EdgeBrowseResult;

typedef struct EdgeEndpointConfig
{
    /** requestTimeout.*/
    int requestTimeout;

    /** applicationName.*/
    char *applicationName;

    /** applicationUri.*/
    char *applicationUri;

    /** productUri.*/
    char *productUri;

    /** securityPolicyUri.*/
    char *securityPolicyUri;

    /** serverName.*/
    char *serverName;

    /** bindAddress.*/
    char *bindAddress;

    /** bindPort.*/
    int bindPort;

} EdgeEndpointConfig;


typedef struct EdgeEndPointInfo
{
    /** endpointUri.*/
    char *endpointUri;

    /** EdgeEndpointConfig.*/
    EdgeEndpointConfig *config;

} EdgeEndPointInfo;

typedef struct EdgeDevice
{
    /** browseName.*/
    char *address;

    /** port.*/
    int port;

    /** serverName.*/
    char *serverName;

    /** EdgeEndPointInfo.*/
    EdgeEndPointInfo **endpointsInfo;

    /** Number of Endpoints */
    int num_endpoints;

} EdgeDevice;

typedef struct EdgeNodeId
{
    /** nameSpace.*/
    int nameSpace;

    /** nodeUri.*/
    char *nodeUri;

    /** EdgeNodeIdentifier.*/
    EdgeNodeIdentifier nodeIdentifier;

    /** EdgeNodeTypeCommon.*/
    EdgeNodeTypeCommon type;

    /** NodeId.*/
    char* nodeId;

    int integerNodeId;

} EdgeNodeId;


typedef struct EdgeNodeInfo
{
    /** methodName.*/
    char *methodName;

    /** EdgeNodeId.*/
    EdgeNodeId *nodeId;

    /** valueAlias.*/
    char *valueAlias;

} EdgeNodeInfo;


typedef struct EdgeRequest
{
    /** EdgeVersatility.*/
    //EdgeVersatility value;
    void *value;

    EdgeNodeIdentifier type;

    /** EdgeSubRequest.*/
    EdgeSubRequest *subMsg;

    /** EdgeNodeInfo.*/
    EdgeNodeInfo *nodeInfo;

    /** requestId.*/
    int requestId;

    /** returnDiagnostic.*/
    int returnDiagnostic;

    /** browseName.*/
    //int seed;
} EdgeRequest;

typedef struct EdgeResult
{
    /** EdgeNodeInfo.*/
//    EdgeNodeInfo *endpoint;

    /** EdgeStatusCode.*/
    EdgeStatusCode code;
} EdgeResult;

typedef struct EdgeMessage
{
    /** EdgeMessageType.*/
    EdgeMessageType type;

    /** EdgeCommand.*/
    EdgeCommand command;

    /** EdgeEndPointInfo.*/
    EdgeEndPointInfo *endpointInfo;

    /** EdgeRequest.*/
    EdgeRequest *request;

    /** EdgeRequests.*/
    EdgeRequest **requests;

    /** EdgeResponse.*/
    EdgeResponse **responses;

    int responseLength;

    /** EdgeResult.*/
    EdgeResult *result;

    /** EdgeBrowseParameter.*/
    //      EdgeBrowseParameter *m_browseMsg;

    /** EdgeBrowseResult.*/
    EdgeBrowseResult **browseResult;

    int browseResponseLength;

} EdgeMessage;

#ifdef __cplusplus
}
#endif

#endif /* EDGE_COMMON_H_ */
