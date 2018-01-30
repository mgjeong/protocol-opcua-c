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

typedef enum
{
    DIRECTION_FORWARD,
    DIRECTION_INVERSE,
    DIRECTION_BOTH
} EdgeBrowseDirection;

typedef struct EdgeBrowseParameter
{
    EdgeBrowseDirection direction;
    int maxReferencesPerNode;
} EdgeBrowseParameter;

typedef struct EdgeEndpointConfig
{
    /** requestTimeout.*/
    int requestTimeout;

    /** serverName.*/
    char *serverName;

    /** bindAddress.*/
    char *bindAddress;

    /** bindPort.*/
    int bindPort;
} EdgeEndpointConfig;

/** The types of applications. */
typedef enum {
    EDGE_APPLICATIONTYPE_SERVER = 0,
    EDGE_APPLICATIONTYPE_CLIENT = 1,
    EDGE_APPLICATIONTYPE_CLIENTANDSERVER = 2,
    EDGE_APPLICATIONTYPE_DISCOVERYSERVER = 3,
    EDGE_APPLICATIONTYPE_UNKNOWN = 0x7fffffff
} EdgeApplicationType;

typedef struct EdgeApplicationConfig
{
    /** applicationUri.*/
    char *applicationUri;

    /** productUri.*/
    char *productUri;

    /** applicationName.*/
    char *applicationName;

    /** Type of application. */
    EdgeApplicationType applicationType;

    /** Gateway Server's URL.*/
    char *gatewayServerUri;

    /** Discovery Profile URL.*/
    char *discoveryProfileUri;

    /** Discovery Endpoint URL.*/
    char **discoveryUrls;

    /** Number of discovery endpoint URLs.*/
    int discoveryUrlsSize;
} EdgeApplicationConfig;

/** The type of security to use on a message.*/
typedef enum
{
    EDGE_MESSAGESECURITYMODE_INVALID = 0,
    EDGE_MESSAGESECURITYMODE_NONE = 1,
    EDGE_MESSAGESECURITYMODE_SIGN = 2,
    EDGE_MESSAGESECURITYMODE_SIGNANDENCRYPT = 3,
    EDGE_MESSAGESECURITYMODE_UNKNOWN = 0x7fffffff
} EdgeSecurityMode;

typedef struct EdgeEndPointInfo
{
    /** endpointUri.*/
    char *endpointUri;

    /** EdgeEndpointConfig.*/
    EdgeEndpointConfig *endpointConfig;

    /** EdgeApplicationConfig.*/
    EdgeApplicationConfig *appConfig;

    /** Security mode of messages.*/
    EdgeSecurityMode securityMode;

    /** securityPolicyUri.*/
    char *securityPolicyUri;

    /** transportProfileUri.*/
    char *transportProfileUri;

    /** securityLevel.*/
    int securityLevel;
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

    /** EdgeNodeIdType.*/
    EdgeNodeIdType type;

    /** NodeId.*/
    char *nodeId;

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

typedef struct EdgeMethodRequestParams
{
    /** number of input arguments */
    int num_inpArgs;

    /** Input arguments */
    EdgeArgument **inpArg;

    /** number of output arguments */
    int num_outArgs;

    /** Input arguments */
    EdgeArgument **outArg;

} EdgeMethodRequestParams;


typedef struct EdgeRequest
{
    /** EdgeVersatility.*/
    void *value;

    EdgeNodeIdentifier type;

    /** EdgeSubRequest.*/
    EdgeSubRequest *subMsg;

    /** EdgeMethodRequest */
    EdgeMethodRequestParams *methodParams;

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

typedef struct EdgeContinuationPoint
{
    /** Length of continuation point **/
    int length;

    /** Continuation point **/
    unsigned char *continuationPoint;
} EdgeContinuationPoint;

typedef struct EdgeContinuationPointList
{
    /** Total number of continuation points **/
    int count;

    /** List of continuation points **/
    EdgeContinuationPoint **cp;
} EdgeContinuationPointList;

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

    /** Number of requests */
    int requestLength;

    /** EdgeResponse.*/
    EdgeResponse **responses;

    /** Response length */
    int responseLength;

    /** EdgeResult.*/
    EdgeResult *result;

    /** EdgeBrowseParameter.*/
    EdgeBrowseParameter *browseParam;

    /** EdgeBrowseResult.*/
    EdgeBrowseResult *browseResult;

    /** Total number of browse result objects **/
    int browseResultLength;

    /** List of continuation point **/
    EdgeContinuationPointList *cpList;

} EdgeMessage;

#ifdef __cplusplus
}
#endif

#endif /* EDGE_COMMON_H_ */
