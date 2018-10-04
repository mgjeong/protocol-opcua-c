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

#include "edge_opcua_client.h"
#include "edge_get_endpoints.h"
#include "edge_find_servers.h"
#include "edge_discovery_common.h"
#include "read.h"
#include "write.h"
#include "browse.h"
#include "method.h"
#include "message_dispatcher.h"
#include "subscription.h"
#include "edge_logger.h"
#include "edge_utils.h"
#include "edge_open62541.h"
#include "edge_list.h"
#include "edge_map.h"
#include "edge_malloc.h"

#include <stdio.h>
#include <inttypes.h>

#define TAG "session_client"

#define MAX_ADDRESS_SIZE (512)

static edgeMap *sessionClientMap = NULL;
static size_t clientCount = 0;

static status_cb_t g_statusCallback = NULL;

static void getAddressPort(char *endpoint, char **out)
{
    UA_String hostName = UA_STRING_NULL, path = UA_STRING_NULL;
    UA_UInt16 port = 0;
    UA_String endpointUrlString = UA_STRING((char *) (uintptr_t) endpoint);

    UA_StatusCode parse_retval = UA_parseEndpointUrl(&endpointUrlString, &hostName, &port, &path);
    COND_CHECK_NR_MSG((parse_retval != UA_STATUSCODE_GOOD), "Server URL is invalid. Unable to get endpoints\n");

    char address[MAX_ADDRESS_SIZE];
    strncpy(address, (char*) hostName.data, hostName.length);
    address[hostName.length] = '\0';

    char addr_port[MAX_ADDRESS_SIZE];
    memset(addr_port, '\0', MAX_ADDRESS_SIZE);
    int written = snprintf(addr_port, MAX_ADDRESS_SIZE, "%s:%d", address, port);
    if (written > 0)
    {
        *out = (char*) EdgeMalloc(sizeof(char) * (written + 1));
        strncpy(*out, addr_port, written);
        (*out)[written] = '\0';
    }
}

#ifndef ENABLE_SUB_QUEUE
static keyValue getSessionClient(char *endpoint)
#else
keyValue getSessionClient(char *endpoint)
#endif
{
    VERIFY_NON_NULL_MSG(sessionClientMap, "sessionClientMap is NULL\n", NULL);

    EDGE_LOG_V(TAG, "Endpoint : %s\n", endpoint);
    char *ep = NULL;
    getAddressPort(endpoint, &ep);
    VERIFY_NON_NULL_MSG(ep, "NULL EP received in getSessionClient \n", NULL);

    keyValue value = NULL;
    for(edgeMapNode *temp = sessionClientMap->head; temp != NULL; temp = temp->next)
    {
        if (!strcmp(temp->key, ep))
        {
            value = temp->value;
            break;
        }
    }
    EdgeFree(ep);
    return value;
}

static edgeMapNode *removeClientFromSessionMap(char *endpoint)
{
    VERIFY_NON_NULL_MSG(sessionClientMap, "sessionClientMap is NULL\n", NULL);
    char *ep = NULL;
    getAddressPort(endpoint, &ep);
    VERIFY_NON_NULL_MSG(ep, "NULL EP received in removeClientFromSessionMap\n", NULL);

    edgeMapNode *prev = NULL;
    edgeMapNode *temp = sessionClientMap->head;
    while (temp != NULL)
    {
        if (!strcmp(temp->key, ep))
        {
            if (prev == NULL)
            {
                sessionClientMap->head = temp->next;
            }
            else
            {
                prev->next = temp->next;
            }
            break;
        }
        prev = temp;
        temp = temp->next;
    }
    EdgeFree(ep);
    return temp;
}

void setSupportedApplicationTypes(uint8_t supportedTypes)
{
    setSupportedApplicationTypesInternal(supportedTypes);
}

EdgeResult readNodesFromServer(EdgeMessage *msg)
{
    UA_Client *clientHandle = (UA_Client*) getSessionClient(msg->endpointInfo->endpointUri);
    EdgeResult ret = executeRead(clientHandle, msg);
    return ret;
}

EdgeResult writeNodesInServer(EdgeMessage *msg)
{
    UA_Client *clientHandle = (UA_Client*) getSessionClient(msg->endpointInfo->endpointUri);
    EdgeResult ret = executeWrite(clientHandle, msg);
    return ret;
}

void browseNodesInServer(EdgeMessage *msg)
{
    UA_Client *clientHandle = (UA_Client*) getSessionClient(msg->endpointInfo->endpointUri);
    executeBrowse(clientHandle, msg);
}

EdgeResult callMethodInServer(EdgeMessage *msg)
{
    UA_Client *clientHandle = (UA_Client*) getSessionClient(msg->endpointInfo->endpointUri);
    EdgeResult ret = executeMethod(clientHandle, msg);
    return ret;
}

EdgeResult executeSubscriptionInServer(EdgeMessage *msg)
{
    UA_Client *clientHandle = (UA_Client*) getSessionClient(msg->endpointInfo->endpointUri);
    EdgeResult ret = executeSub(clientHandle, msg);
    return ret;
}

void edgeStatusCallback(UA_Client *client, UA_ClientState clientState)
{
    if(IS_NOT_NULL(client->endpointUrl.data))
    {
        char *m_endpoint = (char*) EdgeCalloc(strlen((const char*)client->endpointUrl.data) + 1, sizeof(char));
        strncpy(m_endpoint, (const char*)client->endpointUrl.data, strlen((const char*)client->endpointUrl.data));

        EdgeEndPointInfo *ep = (EdgeEndPointInfo *) EdgeCalloc(1, sizeof(EdgeEndPointInfo));
        ep->endpointUri = m_endpoint;

        if(clientState == UA_CLIENTSTATE_DISCONNECTED)
        {
            removeClientFromSessionMap(ep->endpointUri);
            g_statusCallback(ep, STATUS_DISCONNECTED);
        }
        else if(clientState == UA_CLIENTSTATE_CONNECTED)
        {
            g_statusCallback(ep, STATUS_CONNECTED);
        }
    }
}

bool connect_client(char *endpoint)
{
    UA_StatusCode retVal;
    UA_ClientConfig config = UA_ClientConfig_default;
    config.stateCallback = edgeStatusCallback;

    UA_Client *m_client = NULL;
    char *m_port = NULL;
    char *m_endpoint = (char*) EdgeCalloc(strlen(endpoint) + 1, sizeof(char));
    strncpy(m_endpoint, endpoint, strlen(endpoint));

    printf("connect endpoint :: %s\n", endpoint);

    if (NULL != getSessionClient(m_endpoint))
    {
        EDGE_LOG(TAG, "client already connected.\n");
        EdgeFree(m_endpoint);
        return false;
    }

    m_client = UA_Client_new(config);
    VERIFY_NON_NULL_MSG(m_client, "NULL CLIENT received in connect_client\n", false);

    retVal = UA_Client_connect(m_client, m_endpoint);
    /* Connect with User name and Password */
    //retVal = UA_Client_connect_username(m_client, m_endpoint, "user2", "password1");
    //retVal = UA_Client_connect_username(m_client, m_endpoint, "user1", "password");
    if (retVal != UA_STATUSCODE_GOOD)
    {
        EDGE_LOG_V(TAG, "\n [CLIENT] Unable to connect 0x%08x!\n", retVal);
        UA_Client_delete(m_client);
        EdgeFree(m_endpoint);
        return false;
    }

    EDGE_LOG(TAG, "\n [CLIENT] Client connection successful \n");
    getAddressPort(m_endpoint, &m_port);

    // Add the client to session map
    if (NULL == sessionClientMap)
    {
        sessionClientMap = createMap();
    }
    insertMapElement(sessionClientMap, (keyValue) m_port, (keyValue) m_client);
    clientCount++;

    EdgeEndPointInfo *ep = (EdgeEndPointInfo *) EdgeCalloc(1, sizeof(EdgeEndPointInfo));
    VERIFY_NON_NULL_MSG(ep, "EdgeCalloc FAILED for EdgeEndPointInfo\n", false);
    ep->endpointUri = m_endpoint;
    g_statusCallback(ep, STATUS_CLIENT_STARTED);
    EdgeFree(m_endpoint);
    EdgeFree(ep);

    return true;
}

void disconnect_client(EdgeEndPointInfo *epInfo)
{
    edgeMapNode *session = NULL;
    session = removeClientFromSessionMap(epInfo->endpointUri);
    if (session)
    {
        if (session->key)
        {
            free(session->key);
            session->key = NULL;
        }
        if (session->value)
        {
            UA_Client *m_client = (UA_Client*) session->value;
            UA_Client_delete(m_client);
            m_client = NULL;
        }
        free(session);
        session = NULL;
        clientCount--;
        g_statusCallback(epInfo, STATUS_STOP_CLIENT);

        if (0 == clientCount)
        {
            free(sessionClientMap);
            sessionClientMap = NULL;
            /* Delete all the messages in send and receiver queue */
            delete_queue();
        }
    }
}

EdgeResult client_findServers(const char *endpointUri, size_t serverUrisSize,
    unsigned char **serverUris, size_t localeIdsSize, unsigned char **localeIds,
    size_t *registeredServersSize, EdgeApplicationConfig **registeredServers)
{
    return findServersInternal(endpointUri, serverUrisSize, serverUris, localeIdsSize,
            localeIds, registeredServersSize, registeredServers);
}

EdgeResult client_getEndpoints(char *endpointUri)
{
    return getEndpointsInternal(endpointUri);
}

void registerClientCallback(response_cb_t resCallback, status_cb_t statusCallback, discovery_cb_t discoveryCallback)
{
    registerBrowseResponseCallback(resCallback);
    g_statusCallback = statusCallback;
    registerGetEndpointsCb(discoveryCallback);
}
