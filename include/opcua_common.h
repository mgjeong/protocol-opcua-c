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
 * @file opcua_common.h
 *
 * @brief This file contains the definition, types and structures common for both client and server
 */

#ifndef EDGE_COMMON_H
#define EDGE_COMMON_H

/* C99 */
#include <string.h>
#include <stddef.h>
#include <stdio.h>

#include "edge_command_type.h"
#include "edge_identifier.h"
#include "edge_message_type.h"
#include "edge_node_identifier.h"
#include "edge_node_type.h"
#include "edge_opcua_common.h"
#include "edge_status_code.h"
#include "common_client.h"
#include "common_server.h"

#ifdef __cplusplus
extern "C"
{
#endif

#define REQUEST_TIMEOUT      (60000)
#define BIND_PORT            (12686)
#define MAX_BROWSENAME_SIZE  (1000)
#define MAX_DISPLAYNAME_SIZE (1000)
#define UNIQUE_NODE_PATH     "{%d;%c;v=%d}%1000s"

/**
  * @brief Structure which represents the result response for Browse request
  *
  */
typedef struct EdgeBrowseResult
{
    /**< Node browse name.*/
    char *browseName;

} EdgeBrowseResult;

/**
  * @brief Enum which represents the browse direction
  *
  */
typedef enum
{
    /**< Forward direction. */
    DIRECTION_FORWARD,
    /**< Inverse direction. */
    DIRECTION_INVERSE,
    /**< Both direction */
    DIRECTION_BOTH
} EdgeBrowseDirection;

/**
  * @brief Structure which represents the parameters for Browse request data
  *
  */
typedef struct EdgeBrowseParameter
{
    /**< Browse direction. */
    EdgeBrowseDirection direction;
    /**< Max references per node to browse. */
    int maxReferencesPerNode;
} EdgeBrowseParameter;

/**
  * @brief Structure which represents the endpoint configuratino information
  *
  */
typedef struct EdgeEndpointConfig
{
    /**< Request timeout.*/
    int requestTimeout;

    /**< Server name.*/
    char *serverName;

    /**< Bind Address.*/
    char *bindAddress;

    /**< Port.*/
    uint16_t bindPort;
} EdgeEndpointConfig;

/**
  * @brief Enum which represents the application type
  *
  */
typedef enum {
    /**< Server Application type */
    EDGE_APPLICATIONTYPE_SERVER = 1,
    /**< Client Application type */
    EDGE_APPLICATIONTYPE_CLIENT = 2,
    /**< Client and Server Application type */
    EDGE_APPLICATIONTYPE_CLIENTANDSERVER = 4,
    /**< DiscoveryServer Application type */
    EDGE_APPLICATIONTYPE_DISCOVERYSERVER = 8
} EdgeApplicationType;

/**
  * @brief Structure which represents the application configuration information
  *
  */
typedef struct EdgeApplicationConfig
{
    /**< Application URI.*/
    char *applicationUri;

    /**< Product URI .*/
    char *productUri;

    /**< Application name.*/
    char *applicationName;

    /**< Application type. */
    EdgeApplicationType applicationType;

    /** Gateway Server URI.*/
    char *gatewayServerUri;

    /**< Discovery Profile URI.*/
    char *discoveryProfileUri;

    /**< Discovery Endpoint URL.*/
    char **discoveryUrls;

    /**< Number of discovery endpoint URLs.*/
    size_t discoveryUrlsSize;
} EdgeApplicationConfig;

/**
  * @brief Enum which represents the message security mode
  *
  */
typedef enum
{
    /**< Invalid message security mode */
    EDGE_MESSAGESECURITYMODE_INVALID = 0,
    /**< NONE message security mode */
    EDGE_MESSAGESECURITYMODE_NONE = 1,
    /**< SIGN message security mode */
    EDGE_MESSAGESECURITYMODE_SIGN = 2,
    /**< SIGN and ENCRYPT message security mode */
    EDGE_MESSAGESECURITYMODE_SIGNANDENCRYPT = 3,
    /**< Unknown message security mode */
    EDGE_MESSAGESECURITYMODE_UNKNOWN = 0x7fffffff
} EdgeSecurityMode;

/**
  * @brief Structure which represents the endpoint information
  *
  */
typedef struct EdgeEndPointInfo
{
    /**< Endpoint Uri.*/
    char *endpointUri;

    /**< Endpoint configuration .*/
    EdgeEndpointConfig *endpointConfig;

    /**< Application configuration .*/
    EdgeApplicationConfig *appConfig;

    /**< Message Security mode.*/
    EdgeSecurityMode securityMode;

    /**< Security Policy Uri.*/
    char *securityPolicyUri;

    /**< Transport Profile Uri.*/
    char *transportProfileUri;

    /**<  Security Level.*/
    int securityLevel;
} EdgeEndPointInfo;

/**
  * @brief Structure which represents the device information
  *
  */
typedef struct EdgeDevice
{
    /**< Address.*/
    char *address;

    /**< Port.*/
    uint16_t port;

    /**< Server Name.*/
    char *serverName;

    /**< Endpoint information.*/
    EdgeEndPointInfo **endpointsInfo;

    /**< Number of endpoints */
    size_t num_endpoints;
} EdgeDevice;

/**
  * @brief Structure which represents the Node Id information
  *
  */
typedef struct EdgeNodeId
{
    /**< Namespace index.*/
    uint16_t nameSpace;

    /**< Node Uri.*/
    char *nodeUri;

    /**< EdgeNodeIdentifier type.*/
    EdgeNodeIdentifier nodeIdentifier;

    /**< EdgeNodeId Type.*/
    EdgeNodeIdType type;

    /**< Node browse name.*/
    char *nodeId;

    /**< Integer Node id */
    int integerNodeId;
} EdgeNodeId;

/**
  * @brief Structure which represents the Node information
  *
  */
typedef struct EdgeNodeInfo
{
    /**< Method Name.*/
    char *methodName;

    /**< Node Id.*/
    EdgeNodeId *nodeId;

    /**< Node Value Alias.*/
    char *valueAlias;
} EdgeNodeInfo;

/**
  * @brief Structure which represents the parameters in method request data
  *
  */
typedef struct EdgeMethodRequestParams
{
    /**< Number of input arguments */
    size_t num_inpArgs;

    /**< Input arguments */
    EdgeArgument **inpArg;

    /**< Number of output arguments */
    size_t num_outArgs;

    /**< Input arguments */
    EdgeArgument **outArg;
} EdgeMethodRequestParams;

/**
  * @brief Structure which represents the Request data
  *
  */
typedef struct EdgeRequest
{
    /**< EdgeVersatility data.*/
    void *value;

    /**< data type */
    EdgeNodeIdentifier type;

    /**< Subscription Request.*/
    EdgeSubRequest *subMsg;

    /**< Method Request */
    EdgeMethodRequestParams *methodParams;

    /**< Node information.*/
    EdgeNodeInfo *nodeInfo;

    /**< Request id.*/
    int requestId;

    /**< Return Diagnostics.*/
    int returnDiagnostic;
} EdgeRequest;

/**
  * @brief Structure which represents the result of requested operation
  *
  */
typedef struct EdgeResult
{
//    EdgeNodeInfo *endpoint;

    /**< Status code of requested operation.*/
    EdgeStatusCode code;
} EdgeResult;

/**
  * @brief Structure which represents the ContinuationPoints in Browse request data
  *
  */
typedef struct EdgeContinuationPoint
{
    /**< Length of continuation point **/
    size_t length;

    /**< Continuation point **/
    unsigned char *continuationPoint;
} EdgeContinuationPoint;

/**
  * @brief Structure which represents the List of ContinuationPoints in Browse request data
  *
  */
typedef struct EdgeContinuationPointList
{
    /**< Total number of continuation points **/
    size_t count;

    /**< List of continuation points **/
    EdgeContinuationPoint **cp;
} EdgeContinuationPointList;

/**
  * @brief Structure which represents the request and response data
  *
  */
typedef struct EdgeMessage
{
    /**< Message type.*/
    EdgeMessageType type;

    /**< Command type.*/
    EdgeCommand command;

    /**< Endpoint information.*/
    EdgeEndPointInfo *endpointInfo;

    /**< Single request data.*/
    EdgeRequest *request;

    /**< Group requests.*/
    EdgeRequest **requests;

    /**< Number of requests */
    size_t requestLength;

    /**< Response message data.*/
    EdgeResponse **responses;

    /**< Response length */
    size_t responseLength;

    /**< Status code of requested operation.*/
    EdgeResult *result;

    /**< Browse parameter for Browse request.*/
    EdgeBrowseParameter *browseParam;

    /**< Browse response containing the browse node name.*/
    EdgeBrowseResult *browseResult;

    /**< Total number of browse result objects **/
    size_t browseResultLength;

    /**< List of continuation point **/
    EdgeContinuationPointList *cpList;

    /**< Message Id **/
    uint32_t message_id;
} EdgeMessage;

#ifdef __cplusplus
}
#endif

#endif /* EDGE_COMMON_H_ */
