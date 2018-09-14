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
#ifdef _WIN32
#include <winsock2.h>
#include <time.h>
#else
#include <sys/time.h>
#endif
#include <time.h>

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
#define UNIQUE_NODE_PATH     "{%d;%c;v=%d}%1000[^\n]s"

#define EDGE_NODEID_UNKNOWN (0)
#define EDGE_NODEID_BOOLEAN (1)
#define EDGE_NODEID_SBYTE (2)
#define EDGE_NODEID_BYTE (3)
#define EDGE_NODEID_INT16 (4)
#define EDGE_NODEID_UINT16 (5)
#define EDGE_NODEID_INT32 (6)
#define EDGE_NODEID_UINT32 (7)
#define EDGE_NODEID_INT64 (8)
#define EDGE_NODEID_UINT64 (9)
#define EDGE_NODEID_FLOAT (10)
#define EDGE_NODEID_DOUBLE (11)
#define EDGE_NODEID_STRING (12)
#define EDGE_NODEID_DATETIME (13)
#define EDGE_NODEID_GUID (14)
#define EDGE_NODEID_BYTESTRING (15)
#define EDGE_NODEID_XMLELEMENT (16)
#define EDGE_NODEID_NODEID (17)
#define EDGE_NODEID_EXPANDEDNODEID (18)
#define EDGE_NODEID_STATUSCODE (19)
#define EDGE_NODEID_QUALIFIEDNAME (20)
#define EDGE_NODEID_LOCALIZEDTEXT (21)

#define EDGE_NODEID_STRUCTURE 22
#define EDGE_NODEID_DATAVALUE 23
#define EDGE_NODEID_BASEDATATYPE 24
#define EDGE_NODEID_DIAGNOSTICINFO 25
#define EDGE_NODEID_NUMBER 26
#define EDGE_NODEID_INTEGER 27
#define EDGE_NODEID_UINTEGER 28
#define EDGE_NODEID_ENUMERATION 29
#define EDGE_NODEID_IMAGE 30
#define EDGE_NODEID_REFERENCES 31
#define EDGE_NODEID_NONHIERARCHICALREFERENCES 32
#define EDGE_NODEID_HIERARCHICALREFERENCES 33
#define EDGE_NODEID_HASCHILD 34
#define EDGE_NODEID_ORGANIZES 35
#define EDGE_NODEID_HASEVENTSOURCE 36
#define EDGE_NODEID_HASMODELLINGRULE 37
#define EDGE_NODEID_HASENCODING 38
#define EDGE_NODEID_HASDESCRIPTION 39
#define EDGE_NODEID_HASTYPEDEFINITION 40
#define EDGE_NODEID_GENERATESEVENT 41
#define EDGE_NODEID_AGGREGATES 44
#define EDGE_NODEID_HASSUBTYPE 45
#define EDGE_NODEID_HASPROPERTY 46
#define EDGE_NODEID_HASCOMPONENT 47
#define EDGE_NODEID_HASNOTIFIER 48
#define EDGE_NODEID_HASORDEREDCOMPONENT 49
#define EDGE_NODEID_FROMSTATE 51
#define EDGE_NODEID_TOSTATE 52
#define EDGE_NODEID_HASCAUSE 53
#define EDGE_NODEID_HASEFFECT 54
#define EDGE_NODEID_HASHISTORICALCONFIGURATION 56
#define EDGE_NODEID_BASEOBJECTTYPE 58
#define EDGE_NODEID_FOLDERTYPE 61
#define EDGE_NODEID_BASEVARIABLETYPE 62
#define EDGE_NODEID_BASEDATAVARIABLETYPE 63
#define EDGE_NODEID_PROPERTYTYPE 68
#define EDGE_NODEID_DATATYPEDESCRIPTIONTYPE 69
#define EDGE_NODEID_DATATYPEDICTIONARYTYPE 72
#define EDGE_NODEID_DATATYPESYSTEMTYPE 75
#define EDGE_NODEID_DATATYPEENCODINGTYPE 76
#define EDGE_NODEID_MODELLINGRULETYPE 77
#define EDGE_NODEID_MODELLINGRULE_MANDATORY 78
#define EDGE_NODEID_MODELLINGRULE_MANDATORYSHARED 79
#define EDGE_NODEID_MODELLINGRULE_OPTIONAL 80
#define EDGE_NODEID_MODELLINGRULE_EXPOSESITSARRAY 83
#define EDGE_NODEID_ROOTFOLDER 84
#define EDGE_NODEID_OBJECTSFOLDER 85
#define EDGE_NODEID_TYPESFOLDER 86
#define EDGE_NODEID_VIEWSFOLDER 87
#define EDGE_NODEID_OBJECTTYPESFOLDER 88
#define EDGE_NODEID_VARIABLETYPESFOLDER 89
#define EDGE_NODEID_DATATYPESFOLDER 90
#define EDGE_NODEID_REFERENCETYPESFOLDER 91
#define EDGE_NODEID_XMLSCHEMA_TYPESYSTEM 92
#define EDGE_NODEID_OPCBINARYSCHEMA_TYPESYSTEM 93
#define EDGE_NODEID_MODELLINGRULE_MANDATORY_NAMINGRULE 112
#define EDGE_NODEID_MODELLINGRULE_OPTIONAL_NAMINGRULE 113
#define EDGE_NODEID_MODELLINGRULE_EXPOSESITSARRAY_NAMINGRULE 114
#define EDGE_NODEID_MODELLINGRULE_MANDATORYSHARED_NAMINGRULE 116
#define EDGE_NODEID_HASSUBSTATEMACHINE 117
#define EDGE_NODEID_NAMINGRULETYPE 120
#define EDGE_NODEID_DECIMAL128 121
#define EDGE_NODEID_IDTYPE 256
#define EDGE_NODEID_NODECLASS 257
#define EDGE_NODEID_NODE 258
#define EDGE_NODEID_NODE_ENCODING_DEFAULTXML 259
#define EDGE_NODEID_NODE_ENCODING_DEFAULTBINARY 260
#define EDGE_NODEID_OBJECTNODE 261
#define EDGE_NODEID_OBJECTNODE_ENCODING_DEFAULTXML 262
#define EDGE_NODEID_OBJECTNODE_ENCODING_DEFAULTBINARY 263
#define EDGE_NODEID_OBJECTTYPENODE 264
#define EDGE_NODEID_OBJECTTYPENODE_ENCODING_DEFAULTXML 265
#define EDGE_NODEID_OBJECTTYPENODE_ENCODING_DEFAULTBINARY 266
#define EDGE_NODEID_VARIABLENODE 267
#define EDGE_NODEID_VARIABLENODE_ENCODING_DEFAULTXML 268
#define EDGE_NODEID_VARIABLENODE_ENCODING_DEFAULTBINARY 269
#define EDGE_NODEID_VARIABLETYPENODE 270
#define EDGE_NODEID_VARIABLETYPENODE_ENCODING_DEFAULTXML 271
#define EDGE_NODEID_VARIABLETYPENODE_ENCODING_DEFAULTBINARY 272
#define EDGE_NODEID_REFERENCETYPENODE 273
#define EDGE_NODEID_REFERENCETYPENODE_ENCODING_DEFAULTXML 274
#define EDGE_NODEID_REFERENCETYPENODE_ENCODING_DEFAULTBINARY 275
#define EDGE_NODEID_METHODNODE 276
#define EDGE_NODEID_METHODNODE_ENCODING_DEFAULTXML 277
#define EDGE_NODEID_METHODNODE_ENCODING_DEFAULTBINARY 278
#define EDGE_NODEID_VIEWNODE 279
#define EDGE_NODEID_VIEWNODE_ENCODING_DEFAULTXML 280
#define EDGE_NODEID_VIEWNODE_ENCODING_DEFAULTBINARY 281
#define EDGE_NODEID_DATATYPENODE 282
#define EDGE_NODEID_DATATYPENODE_ENCODING_DEFAULTXML 283
#define EDGE_NODEID_DATATYPENODE_ENCODING_DEFAULTBINARY 284
#define EDGE_NODEID_REFERENCENODE 285
#define EDGE_NODEID_REFERENCENODE_ENCODING_DEFAULTXML 286
#define EDGE_NODEID_REFERENCENODE_ENCODING_DEFAULTBINARY 287
#define EDGE_NODEID_INTEGERID 288
#define EDGE_NODEID_COUNTER 289
#define EDGE_NODEID_DURATION 290
#define EDGE_NODEID_NUMERICRANGE 291
#define EDGE_NODEID_TIME 292
#define EDGE_NODEID_DATE 293
#define EDGE_NODEID_UTCTIME 294
#define EDGE_NODEID_LOCALEID 295
#define EDGE_NODEID_ARGUMENT 296
#define EDGE_NODEID_ARGUMENT_ENCODING_DEFAULTXML 297
#define EDGE_NODEID_ARGUMENT_ENCODING_DEFAULTBINARY 298
#define EDGE_NODEID_STATUSRESULT 299
#define EDGE_NODEID_STATUSRESULT_ENCODING_DEFAULTXML 300
#define EDGE_NODEID_STATUSRESULT_ENCODING_DEFAULTBINARY 301

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
    uint32_t bindPort;
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

    /**< Node Identifier.*/
    int nodeIdentifier;

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
    int type;

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

typedef struct EdgeTimeInfo
{
    struct tm *timeInfo;
    struct timeval tv;
} EdgeTimeInfo;

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

    /**< Message Id **/
    uint32_t message_id;

    /**< Server Time Stamp **/
    EdgeTimeInfo serverTime;

} EdgeMessage;

#ifdef __cplusplus
}
#endif

#endif /* EDGE_COMMON_H_ */
