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

#ifndef EDGE_OPCUA_SERVER_H
#define EDGE_OPCUA_SERVER_H

#include "opcua_common.h"

#ifdef __cplusplus
extern "C"
{
#endif

    /* Create and add namespace */
    void createNamespaceInServer(char *namespaceUri, char *rootNodeIdentifier,
            char *rootNodeBrowseName, char *rootNodeDisplayName);

    EdgeResult addNodesInServer(EdgeNodeItem *item);
    EdgeResult modifyNodeInServer(char *nodeUri, EdgeVersatility *value);
    EdgeResult addReferenceInServer(EdgeReference *reference);
    EdgeResult addMethodNodeInServer(EdgeNodeItem *item, EdgeMethod *method);

    /* Start the Server */
    EdgeResult start_server(EdgeEndPointInfo *epInfo);

    /* Stop the server */
    void stop_server(EdgeEndPointInfo *epInfo);

    /* Create and delete node item */
    EdgeNodeItem* createNodeItemImpl(char* name, EdgeNodeIdentifier type, void* data);
    EdgeResult deleteNodeItemImpl(EdgeNodeItem* item);

#ifdef __cplusplus
}
#endif

#endif
