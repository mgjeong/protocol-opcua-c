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
 * @file common_client.h
 *
 * @brief This file contains the definition, types and structures for the client
 */

#ifndef EDGE_COMMON_CLIENT_H_
#define EDGE_COMMON_CLIENT_H_

#include "opcua_common.h"
#include "edge_node_identifier.h"

#include <stdbool.h>
#include <stdint.h>

#ifdef __cplusplus
extern "C"
{
#endif

typedef struct EdgeNodeInfo EdgeNodeInfo;
typedef struct EdgeResult EdgeResult;

/**
  * @brief Structure which represents the data
  *
  */
typedef struct EdgeVersatility
{
    /**< Scalar/Array data. */
    void *value;

    /**< Scalar/Array type. */
    bool isArray;

    /**< Array Length */
    size_t arrayLength;
} EdgeVersatility;

/**
  * @brief Structure which represents the diagnostic information
  *
  */
typedef struct EdgeDiagnosticInfo
{

    /**< Symbolic Id used to identify vendor-specific error or condition; typically the result of some server internal operation */
    int symbolicId;

    /**< Namespace Uri */
    int namespaceUri;

    /**< Localized Text string that describes the s ymbolic id */
    int localizedText;

    /**< Locale part of vendor-specific localized text describing the symbolic id */
    int locale;

    /**< Diagnostic information */
    char *additionalInfo;

    /**< Diagnostic information associated with inner status code */
    void *innerDiagnosticInfo;

    /**< Description of diagnostic info */
    char *msg;

} EdgeDiagnosticInfo;

/**
  * @brief Structure which represents the response data
  *
  */
typedef struct EdgeResponse
{
    /**< EdgeVersatility message */
    EdgeVersatility *message;

    /**< EdgeNodeIdentifier type */
//    EdgeNodeIdentifier type;

    /**< identifiers type */
    int type;

    /**< Node information */
    EdgeNodeInfo *nodeInfo;

    /**< EdgeResult representing the status of the operation */
    EdgeResult *result;

    /**< request id */
    int requestId;

    /**< Diagnostic information */
    EdgeDiagnosticInfo *m_diagnosticInfo;
} EdgeResponse;

/**
  * @brief Structure which represents the Subscription Request data
  *
  */
typedef struct EdgeSubRequest
{
    /**< EdgeNodeType type */
    EdgeNodeType subType;

    /**< sampling interval */
    double samplingInterval;

    /**< Publishing Interval */
    double publishingInterval;

    /**< Lifetime count */
    int lifetimeCount;

    /**< max keepalive count */
    int maxKeepAliveCount;

    /**< Max Notifications in a single Publish response */
    int maxNotificationsPerPublish;

    /**< Publishing enable/disable */
    bool publishingEnabled;

    /**< Priority of the subscription */
    int priority;

    /**< Size of MonitoredItem queue */
    uint32_t queueSize;
} EdgeSubRequest;

#ifdef __cplusplus
}
#endif

#endif /* EDGE_COMMON_H_ */
