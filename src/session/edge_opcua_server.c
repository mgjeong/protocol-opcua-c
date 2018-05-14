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

#include "edge_opcua_server.h"
#include "edge_node.h"
#include "edge_utils.h"
#include "edge_open62541.h"
#include "edge_map.h"
#include "edge_logger.h"
#include "edge_malloc.h"

#include <stdio.h>
#include <pthread.h>
#include <open62541.h>

#define TAG "session_server"

typedef struct
{
    uint16_t ns_index;
    char *rootNodeIdentifier;
    char *rootNodeBrowseName;
    char *rootNodeDisplayName;
} EdgeNamespace;

static UA_ServerConfig *m_serverConfig;
static UA_Server *m_server;
static UA_Boolean b_running = UA_FALSE;

static pthread_t m_serverThread;

/* Namespace Map */
static edgeMap *namespaceMap = NULL;

static int namespaceType = DEFAULT_TYPE;

static status_cb_t g_statusCallback = NULL;

void printNode(void *visitorContext, const UA_Node *node)
{
    if (node == NULL)
    {
        EDGE_LOG(TAG, "UA_Node is null\n");
        return;
    }

    UA_NodeId nodeId = node->nodeId;
    UA_QualifiedName browseNameUA = node->browseName;
    char *type = NULL;
    switch (nodeId.identifierType)
    {
        case UA_NODEIDTYPE_NUMERIC:
            type = "numeric";
            break;
        case UA_NODEIDTYPE_STRING:
            type = "string";
            break;
        case UA_NODEIDTYPE_GUID:
            type = "guid";
            break;
        case UA_NODEIDTYPE_BYTESTRING:
            type = "byteString";
            break;
        default:
            type = "unknown";
    }

    char *browseName = convertUAStringToString(&browseNameUA.name);
    printf("namespaceIndex : %d, type : %s, browseName : %s\n", nodeId.namespaceIndex, type, browseName);
    if(IS_NOT_NULL(browseName))
    {
        EdgeFree(browseName);
    }
}

void printNodeListInServer()
{
    m_serverConfig->nodestore.iterate(m_serverConfig->nodestore.context, NULL, printNode);
}

static void* getNamespaceIndex(const char *namespaceUri)
{
    if (namespaceMap)
    {
        edgeMapNode *temp = namespaceMap->head;
        while (temp != NULL)
        {
            char *uri = (char*) temp->key;
            if (!strcmp(uri, namespaceUri))
                return temp->value;

            temp = temp->next;
        }
    }
    return NULL;
}

EdgeResult createNamespaceInServer(const char *namespaceUri, const char *rootNodeIdentifier,
		const char *rootNodeBrowseName,	const char *rootNodeDisplayName)
{
    EdgeResult result;
    result.code = STATUS_OK;
    if (!namespaceUri || !rootNodeIdentifier || !rootNodeBrowseName || !rootNodeDisplayName)
    {
        result.code = STATUS_PARAM_INVALID;
        return result;
    }
    if (namespaceType != URI_TYPE && namespaceType != DEFAULT_TYPE)
    {
        result.code = STATUS_ERROR;
        return result;
    }

    if (getNamespaceIndex(namespaceUri))
    {
        EDGE_LOG(TAG, "Namespace already added\n");
        result.code = STATUS_PARAM_INVALID;
        return result;
    }

    uint16_t idx = UA_Server_addNamespace(m_server, namespaceUri);
    EDGE_LOG_V(TAG, "[SERVER] Namespace with Index Created:: [%d]\n", idx);

    EdgeNamespace *ns = (EdgeNamespace*) EdgeCalloc(1, sizeof(EdgeNamespace));
    ns->ns_index = idx;
    ns->rootNodeIdentifier = (char*) EdgeMalloc(sizeof(char) * (strlen(rootNodeIdentifier)+1));
    if(IS_NULL(ns->rootNodeIdentifier))
    {
        EDGE_LOG(TAG, "Memory allocation failed.");
        goto ERROR;
    }
    strncpy(ns->rootNodeIdentifier, rootNodeIdentifier, strlen(rootNodeIdentifier)+1);

    ns->rootNodeBrowseName = (char*) EdgeMalloc(sizeof(char) * (strlen(rootNodeBrowseName)+1));
    if(IS_NULL(ns->rootNodeBrowseName))
    {
        EDGE_LOG(TAG, "Memory allocation failed.");
        goto ERROR;
    }
    strncpy(ns->rootNodeBrowseName, rootNodeBrowseName, strlen(rootNodeBrowseName)+1);

    ns->rootNodeDisplayName = (char*) EdgeMalloc(sizeof(char) * (strlen(rootNodeDisplayName)+1));
    if(IS_NULL(ns->rootNodeDisplayName))
    {
        EDGE_LOG(TAG, "Memory allocation failed.");
        goto ERROR;
    }
    strncpy(ns->rootNodeDisplayName, rootNodeDisplayName, strlen(rootNodeDisplayName)+1);

    if (namespaceMap == NULL)
        namespaceMap = createMap();
    insertMapElement(namespaceMap, (keyValue) namespaceUri, (keyValue) ns);
    return result;

ERROR:
    // Deallocate memory.
    EdgeFree(ns->rootNodeIdentifier);
    EdgeFree(ns->rootNodeBrowseName);
    EdgeFree(ns->rootNodeDisplayName);
    EdgeFree(ns);
	result.code = STATUS_ERROR;
    return result;
}

EdgeResult addNodesInServer(const char *namespaceUri, EdgeNodeItem *item)
{
    EdgeResult result;
    if (!namespaceUri || !item)
    {
        result.code = STATUS_PARAM_INVALID;
        return result;
    }
    result.code = STATUS_ERROR;
    EdgeNamespace *ns = (EdgeNamespace*) getNamespaceIndex(namespaceUri);
    if (ns)
    {
        result = addNodes(m_server, ns->ns_index, item);
    }
    return result;
}

EdgeResult modifyNodeInServer(const char *namespaceUri, const char *nodeUri, EdgeVersatility *value)
{
    EdgeResult result;
    if (!namespaceUri || !nodeUri || !value)
    {
        result.code = STATUS_PARAM_INVALID;
        return result;
    }
    result.code = STATUS_ERROR;
    EdgeNamespace *ns = (EdgeNamespace*) getNamespaceIndex(namespaceUri);
    if (ns)
    {
        result = modifyNode(m_server, ns->ns_index, nodeUri, value);
    }
    return result;
}

EdgeResult addReferenceInServer(EdgeReference *reference)
{
    EdgeResult result;
    if (!reference)
    {
        result.code = STATUS_PARAM_INVALID;
        return result;
    }
    if (!reference->sourceNamespace || !reference->targetNamespace)
    {
        result.code = STATUS_PARAM_INVALID;
        return result;
    }
    result.code = STATUS_ERROR;
    EdgeNamespace *src_ns = (EdgeNamespace*) getNamespaceIndex(reference->sourceNamespace);
    EdgeNamespace *target_ns = (EdgeNamespace*) getNamespaceIndex(reference->targetNamespace);
    if (src_ns && target_ns)
    {
        result = addReferences(m_server, reference, src_ns->ns_index, target_ns->ns_index);
    }
    return result;
}

EdgeResult addMethodNodeInServer(const char *namespaceUri, EdgeNodeItem *item, EdgeMethod *method)
{
    EdgeResult result;
    if (!namespaceUri || !item || !method)
    {
        result.code = STATUS_PARAM_INVALID;
        return result;
    }
    result.code = STATUS_ERROR;
    EdgeNamespace *ns = (EdgeNamespace*) getNamespaceIndex(namespaceUri);
    if (ns)
    {
        result = addMethodNode(m_server, ns->ns_index, item, method);
    }
    return result;
}

EdgeNodeItem* createVariableNodeItemImpl(const char* name, int type, void* data,
        EdgeIdentifier nodeType, double minimumInterval)
{
    if (!name)
    {
        return NULL;
    }
    EdgeNodeItem* item = (EdgeNodeItem *) EdgeCalloc(1, sizeof(EdgeNodeItem));
    //VERIFY_NON_NULL_RET(item);
    item->accessLevel = READ_WRITE;
    item->userAccessLevel = READ_WRITE;
    item->writeMask = 0;
    item->userWriteMask = 0;
    item->nodeType = nodeType;
    item->forward = true;
    item->browseName = (char*)name;
    item->variableIdentifier = type;
    item->variableData = data;
    item->minimumSamplingInterval = minimumInterval;

    return item;
}

EdgeNodeItem* createNodeItemImpl(const char* name, EdgeIdentifier nodeType, EdgeNodeId *sourceNodeId)
{
    if (!name)
    {
        return NULL;
    }
    EdgeNodeItem* item = (EdgeNodeItem *) EdgeCalloc(1, sizeof(EdgeNodeItem));
    if(IS_NULL(item))
    {
        EDGE_LOG(TAG, "Memory allocation failed.");
        return NULL;
    }

    //VERIFY_NON_NULL_RET(item);
    item->accessLevel = READ_WRITE;
    item->userAccessLevel = READ_WRITE;
    item->writeMask = 0;
    item->userWriteMask = 0;
    item->nodeType = nodeType;
    item->forward = true;
    item->browseName = (char*)name;
    item->sourceNodeId = sourceNodeId;

    return item;
}

EdgeResult deleteNodeItemImpl(EdgeNodeItem* item)
{
    EdgeResult result;
    free(item);
    result.code = STATUS_OK;
    return result;
}

static void *server_loop(void *ptr)
{
    while (b_running)
    {
        UA_Server_run_iterate(m_server, true);
    }

    EDGE_LOG(TAG, " [SERVER] server loop exit\n");
    return NULL;
}

EdgeResult start_server(EdgeEndPointInfo *epInfo)
{
    EdgeResult result;
    result.code = STATUS_OK;

    if (!epInfo || !epInfo->endpointConfig || !epInfo->appConfig)
    {
        result.code = STATUS_PARAM_INVALID;
        return result;
    }

    EdgeEndpointConfig *epConfig = epInfo->endpointConfig;
    EdgeApplicationConfig *appConfig = epInfo->appConfig;

    //UA_ByteString certificate = loadCertificate();
    //m_serverConfig = UA_ServerConfig_new_default();    //UA_ServerConfig_new_minimal(4840, &certificate);
    m_serverConfig = UA_ServerConfig_new_minimal(epConfig->bindPort, NULL);

    UA_String_deleteMembers(&m_serverConfig->applicationDescription.applicationUri);
    UA_LocalizedText_deleteMembers(&m_serverConfig->applicationDescription.applicationName);
    UA_String_deleteMembers(&m_serverConfig->applicationDescription.productUri);
    UA_String_deleteMembers(&m_serverConfig->buildInfo.productUri);
    UA_String_deleteMembers(&m_serverConfig->buildInfo.manufacturerName);
    UA_String_deleteMembers(&m_serverConfig->buildInfo.productName);
    UA_String_deleteMembers(&m_serverConfig->buildInfo.softwareVersion);
    UA_String_deleteMembers(&m_serverConfig->buildInfo.buildNumber);

    UA_String_deleteMembers(&m_serverConfig->endpoints->endpointDescription.server.applicationUri);
    UA_LocalizedText_deleteMembers(
            &m_serverConfig->endpoints->endpointDescription.server.applicationName);

    m_serverConfig->applicationDescription.applicationUri = UA_STRING_ALLOC(
            appConfig->applicationUri);
    m_serverConfig->applicationDescription.applicationName = UA_LOCALIZEDTEXT_ALLOC("en-US",
            appConfig->applicationName);
    m_serverConfig->applicationDescription.productUri = UA_STRING_ALLOC(appConfig->productUri);
    m_serverConfig->applicationDescription.applicationType = convertEdgeApplicationType(
            appConfig->applicationType);

    m_serverConfig->buildInfo.productUri = UA_STRING_ALLOC("/edge");
    m_serverConfig->buildInfo.manufacturerName = UA_STRING_ALLOC("samsung");
    m_serverConfig->buildInfo.productName = UA_STRING_ALLOC("edgeSolution");
    m_serverConfig->buildInfo.softwareVersion = UA_STRING_ALLOC("0.9");
    m_serverConfig->buildInfo.buildNumber = UA_STRING_ALLOC("0.1");

    m_serverConfig->endpoints->endpointDescription.server.applicationUri = UA_STRING_ALLOC(
            appConfig->applicationUri);
    m_serverConfig->endpoints->endpointDescription.server.applicationName = UA_LOCALIZEDTEXT_ALLOC(
            "en-US", appConfig->applicationName);

    UA_DurationRange duration;

    duration.min = 1.0;
    duration.max = 24.0 * 3600.0 * 1000.0;
    m_serverConfig->samplingIntervalLimits = duration;

    duration.min = 5.0;
    duration.max = 3600.0 * 1000.0;
    m_serverConfig->publishingIntervalLimits = duration;

    //    UA_ByteString_deleteMembers(&certificate);
    m_server = UA_Server_new(m_serverConfig);

    EDGE_LOG(TAG, "\n [SERVER] starting server \n");
    UA_StatusCode retval = UA_Server_run_startup(m_server);

    if (retval != UA_STATUSCODE_GOOD)
    {
        EDGE_LOG(TAG, "\n [SERVER] Error in starting server \n");
        b_running = UA_FALSE;

        result.code = STATUS_ERROR;
        return result;
    }
    else
    {
        EDGE_LOG(TAG, "\n ========= [SERVER] Server Start successful ============= \n");
        b_running = UA_TRUE;
        pthread_create(&m_serverThread, NULL, &server_loop, NULL);
        g_statusCallback(epInfo, STATUS_SERVER_STARTED);
        //return (new EdgeResult::Builder(STATUS_OK))->build();

        result.code = STATUS_OK;
        return result;
    }

}

void stop_server(EdgeEndPointInfo *epInfo)
{
    b_running = false;
    pthread_join(m_serverThread, NULL);
    UA_Server_run_shutdown(m_server);
    UA_Server_delete(m_server);
    UA_ServerConfig_delete(m_serverConfig);
    EDGE_LOG(TAG, "\n ========= [SERVER] Server Stopped ============= \n");

    namespaceMap = NULL;
    g_statusCallback(epInfo, STATUS_STOP_SERVER);
}

void registerServerCallback(status_cb_t statusCallback)
{
    g_statusCallback = statusCallback;
}
