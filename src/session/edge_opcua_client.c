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
#include "read.h"
#include "write.h"
#include "browse.h"
#include "method.h"
#include "subscription.h"
#include "edge_logger.h"

#include <stdio.h>
#include <open62541.h>

#define TAG "session_client"

static UA_Client *m_client = NULL;

static char *m_endpointUri;
//static char* m_securityUri;

EdgeResult readNodesFromServer(EdgeMessage *msg)
{
    EdgeResult result = executeRead(m_client, msg);
    return result;
}

EdgeResult writeNodesInServer(EdgeMessage *msg)
{
    EdgeResult result = executeWrite(m_client, msg);
    return result;
}

EdgeResult browseNodesInServer(EdgeMessage *msg)
{
    EdgeResult result = executeBrowse(m_client, msg, false);
    return result;
}

EdgeResult browseNextInServer(EdgeMessage *msg)
{
    EdgeResult result = executeBrowse(m_client, msg, true);
    return result;
}

EdgeResult callMethodInServer(EdgeMessage *msg)
{
    EdgeResult result = executeMethod(m_client, msg);
    return result;
}

EdgeResult executeSubscriptionInServer(EdgeMessage *msg)
{
    EdgeResult result = executeSub(m_client, msg);
    return result;
}

bool connect_client(char *endpoint)
{
    m_endpointUri = endpoint;

    UA_ClientConfig config = UA_ClientConfig_default;

    m_client = UA_Client_new(config);

    UA_StatusCode retVal;

    EDGE_LOG_V(TAG, "endpoint :: %s\n", endpoint);
    retVal = UA_Client_connect(m_client, endpoint);
    if (retVal != UA_STATUSCODE_GOOD)
    {
        EDGE_LOG_V(TAG, "\n [CLIENT] Unable to connect 0x%08x!\n", retVal);
        UA_Client_delete(m_client);
        m_client = NULL;
        return false;
    }

    EDGE_LOG(TAG, "\n [CLIENT] Client connection successful \n");

    EdgeEndPointInfo *ep = (EdgeEndPointInfo *) malloc(sizeof(EdgeEndPointInfo));
    ep->endpointUri = endpoint;
    onStatusCallback(ep, STATUS_CLIENT_STARTED);
    free(ep);
    ep = NULL;

    return true;
}

void disconnect_client(EdgeEndPointInfo *epInfo)
{
    if (m_client)
    {
        UA_Client_delete(m_client);
        m_client = NULL;
    }
    onStatusCallback(epInfo, STATUS_STOP_CLIENT);
}

void *getClientEndpoints(char *endpointUri)
{
    UA_StatusCode retVal;
    UA_EndpointDescription *endpointArray = NULL;
    size_t endpointArraySize = 0;
    UA_Client *client = NULL;
    EdgeDevice *device = NULL;
    UA_String hostName = UA_STRING_NULL, path = UA_STRING_NULL;
    UA_UInt16 port = 0;
    UA_String endpointUrlString = UA_STRING((char *) (uintptr_t) endpointUri);

    UA_StatusCode parse_retval = UA_parseEndpointUrl(&endpointUrlString, &hostName, &port, &path);
    if (parse_retval != UA_STATUSCODE_GOOD)
    {
        EDGE_LOG(TAG, "Server URL is invalid. Unable to get endpoints\n");
        return NULL;
    }

    device = (EdgeDevice *) malloc(sizeof(EdgeDevice));
    char *addr = (char *) hostName.data;
    int idx = 0, len = 0;
    for (idx = 0; idx < hostName.length; idx++)
    {
        if (addr[idx] == ':')
            break;
        len += 1;
    }

    device->address = (char *) malloc(len + 1);
    memcpy(device->address, hostName.data, len);
    device->address[len] = '\0';
    device->port = port;
    if (path.length > 0)
    {
        device->serverName = (char *) malloc(path.length + 1);
        memcpy(device->serverName, path.data, path.length);
        device->serverName[path.length] = '\0';
    }
    else
    {
        device->serverName = (char *) malloc(2);
        device->serverName[0] = ' ';
        device->serverName[1] = '\0';
    }
    device->endpointsInfo = NULL;
    device->num_endpoints = 0;

    client = UA_Client_new(UA_ClientConfig_default);

    retVal = UA_Client_getEndpoints(client, endpointUri, &endpointArraySize, &endpointArray);
    if (retVal != UA_STATUSCODE_GOOD)
    {
        EDGE_LOG(TAG, "\n [CLIENT] Unable to get endpoints \n");

        if (device)
        {
            if (device->address)
            {
                free(device->address);
                device->address = NULL;
            }
            if (device->serverName)
            {
                free(device->serverName);
                device->serverName = NULL;
            }
            free(device);
            device = NULL;
        }

        UA_Array_delete(endpointArray, endpointArraySize, &UA_TYPES[UA_TYPES_ENDPOINTDESCRIPTION]);
        UA_Client_delete(client);
        client = NULL;
        return NULL;
    }

    device->num_endpoints = endpointArraySize;
    device->endpointsInfo = (EdgeEndPointInfo **) malloc(
            sizeof(EdgeEndPointInfo *) * endpointArraySize);
    for (size_t i = 0; i < endpointArraySize; i++)
    {
        device->endpointsInfo[i] = (EdgeEndPointInfo *) malloc(sizeof(EdgeEndPointInfo));

        if (endpointArray[i].endpointUrl.data)
        {
            device->endpointsInfo[i]->endpointUri = (char *) malloc(
                    endpointArray[i].endpointUrl.length + 1);
            memcpy(device->endpointsInfo[i]->endpointUri, endpointArray[i].endpointUrl.data,
                    endpointArray[i].endpointUrl.length);
            device->endpointsInfo[i]->endpointUri[endpointArray[i].endpointUrl.length] = '\0';
        }

        EdgeEndpointConfig *config = (EdgeEndpointConfig *) malloc(sizeof(EdgeEndpointConfig));
        if (endpointArray[i].securityPolicyUri.data)
        {
            char *secPolicy = (char *) malloc(endpointArray[i].securityPolicyUri.length + 1);
            memcpy(secPolicy, endpointArray[i].securityPolicyUri.data,
                    endpointArray[i].securityPolicyUri.length);
            secPolicy[endpointArray[i].securityPolicyUri.length] = '\0';
            config->securityPolicyUri = secPolicy;
        }
        device->endpointsInfo[i]->config = config;
    }
    onDiscoveryCallback(device);

    // free device
    if (device)
    {
        for (size_t i = 0; i < endpointArraySize; i++)
        {
            if (device->endpointsInfo[i]->endpointUri)
            {
                free(device->endpointsInfo[i]->endpointUri);
                device->endpointsInfo[i]->endpointUri = NULL;
            }
            if (device->endpointsInfo[i]->config)
            {
                if (device->endpointsInfo[i]->config->securityPolicyUri)
                {
                    free(device->endpointsInfo[i]->config->securityPolicyUri);
                    device->endpointsInfo[i]->config->securityPolicyUri = NULL;
                }
                free(device->endpointsInfo[i]->config);
                device->endpointsInfo[i]->config = NULL;
            }
            free(device->endpointsInfo[i]);
            device->endpointsInfo[i] = NULL;
        }
        if (device->endpointsInfo)
        {
            free(device->endpointsInfo);
            device->endpointsInfo = NULL;
        }
        if (device->address)
        {
            free(device->address);
            device->address = NULL;
        }
        if (device->serverName)
        {
            free(device->serverName);
            device->serverName = NULL;
        }
        free(device);
        device = NULL;
    }

    UA_Array_delete(endpointArray, endpointArraySize, &UA_TYPES[UA_TYPES_ENDPOINTDESCRIPTION]);
    UA_Client_delete(client);
    client = NULL;

    return NULL;
}
