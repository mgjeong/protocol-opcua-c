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

#ifndef EDGE_NODE_H
#define EDGE_NODE_H

#include "opcua_common.h"

#include <open62541.h>

#ifdef __cplusplus
extern "C"
{
#endif

    EdgeResult addNodes(UA_Server *server, uint16_t nsIndex, const EdgeNodeItem *item);
    EdgeResult addMethodNode(UA_Server *server, uint16_t nsIndex, const EdgeNodeItem *item, EdgeMethod *method);
    EdgeResult modifyNode(UA_Server *server, uint16_t nsIndex, char *nodeUri, EdgeVersatility *value);
    EdgeResult modifyNode2(EdgeNodeIdentifier nodeType);
    EdgeResult addReferences(UA_Server *server, EdgeReference *reference, uint16_t src_nsIndex, uint16_t target_nsIndex);

#ifdef __cplusplus
}
#endif

#endif
