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

#ifndef EDGE_COMMON_CLIENT_H_
#define EDGE_COMMON_CLIENT_H_

#include "opcua_common.h"
#include "edge_node_identifier.h"

#include <stdbool.h>

#ifdef __cplusplus
extern "C"
{
#endif

typedef struct EdgeNodeInfo EdgeNodeInfo;
typedef struct EdgeResult EdgeResult;

typedef struct EdgeVersatility
{
    /** data*/
    void *value;

    /** Array/Scalar type */
    bool isArray;

    /** Array Size */
    int arrayLength;

} EdgeVersatility;

typedef struct EdgeDiagnosticInfo
{

    /** Symbolic Id */
    int symbolicId;

    /** Namespace Uri */
    int namespaceUri;

    /** Localized Text */
    int localizedText;

    /** Locale */
    int locale;

    /** additional info */
    char *additionalInfo;

    /** Inner Diagnostics */
    void *innerDiagnosticInfo;

    /** msg */
    char *msg;

} EdgeDiagnosticInfo;

typedef struct EdgeResponse
{
    /** EdgeVersatility.*/
    EdgeVersatility *message;

    void *value;

    /** EdgeNodeIdentifier */
    EdgeNodeIdentifier type;

    /** EdgeNodeInfo.*/
    EdgeNodeInfo *nodeInfo;

    /** EdgeResult.*/
    EdgeResult *result;

    /** requestId.*/
    int requestId;

    /** EdgeDiagnosticInfo.*/
    EdgeDiagnosticInfo *m_diagnosticInfo;
} EdgeResponse;


typedef struct EdgeSubRequest
{
    /** EdgeNodeIdentifier */
    EdgeNodeIdentifier subType;

    /** sampling interval */
    double samplingInterval;

    /** Publishing Interval */
    double publishingInterval;

    /** Lifetime count */
    int lifetimeCount;

    /** max keepalive count */
    int maxKeepAliveCount;

    /** Max Notifications per Publish */
    int maxNotificationsPerPublish;

    /** Publishing enabled ? */
    bool publishingEnabled;

    /** Priority */
    int priority;

    /** Queue Size */
    int queueSize;
} EdgeSubRequest;

#ifdef __cplusplus
}
#endif

#endif /* EDGE_COMMON_H_ */
