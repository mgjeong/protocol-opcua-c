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
#include "edge_utils.h"

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

static bool parseEndpoints(size_t endpointArraySize, UA_EndpointDescription *endpointArray, size_t *count, List **endpointList)
{
    EdgeEndPointInfo *epInfo = NULL;
    for (size_t i = 0; i < endpointArraySize; ++i)
    {
        epInfo = (EdgeEndPointInfo *) calloc(1, sizeof(EdgeEndPointInfo));
        if(!epInfo)
        {
            EDGE_LOG(TAG, "Memory allocation failed.");
            goto ERROR;
        }

        if (UA_APPLICATIONTYPE_CLIENT == endpointArray[i].server.applicationType)
        {
            EDGE_LOG(TAG, "Found client type endpoint. Skipping it.\n");
            continue;
        }

        if (UA_MESSAGESECURITYMODE_SIGN != endpointArray[i].securityMode &&
            UA_MESSAGESECURITYMODE_SIGNANDENCRYPT != endpointArray[i].securityMode)
        {
            EDGE_LOG_V(TAG, "Connection with message security mode(%d) will be insecure.\n", endpointArray[i].securityMode);
        }

        if (endpointArray[i].endpointUrl.data)
        {
            epInfo->endpointUri = convertUAStringToString(&endpointArray[i].endpointUrl);
            if(!epInfo->endpointUri)
            {
                EDGE_LOG(TAG, "Memory allocation failed.");
                goto ERROR;
            }
        }

        EdgeEndpointConfig *config = (EdgeEndpointConfig *) calloc(1, sizeof(EdgeEndpointConfig));
        if(!config)
        {
            EDGE_LOG(TAG, "Memory allocation failed.");
            goto ERROR;
        }
        epInfo->config = config;
        if (endpointArray[i].securityPolicyUri.length > 0)
        {
            config->securityPolicyUri = convertUAStringToString(&endpointArray[i].securityPolicyUri);
            if(!config->securityPolicyUri)
            {
                EDGE_LOG(TAG, "Memory allocation failed.");
                goto ERROR;
            }
        }
        if (endpointArray[i].transportProfileUri.length > 0)
        {
            config->transportProfileUri = convertUAStringToString(&endpointArray[i].transportProfileUri);
            if(!config->transportProfileUri)
            {
                EDGE_LOG(TAG, "Memory allocation failed.");
                goto ERROR;
            }
        }
        addListNode(endpointList, epInfo);
        ++(*count);
        epInfo = NULL;
    }

    return true;

ERROR:
    for (List *listPtr = *endpointList; listPtr; listPtr = listPtr->link)
    {
        freeEdgeEndpointInfo(listPtr->data);
    }
    freeEdgeEndpointInfo(epInfo);

    return false;
}

EdgeResult getClientEndpoints(char *endpointUri)
{
    EdgeResult result;
    UA_StatusCode retVal;
    UA_EndpointDescription *endpointArray = NULL;
    List *endpointList = NULL;
    size_t count = 0, endpointArraySize = 0;
    UA_Client *client = NULL;
    EdgeDevice *device = NULL;
    UA_String hostName = UA_STRING_NULL, path = UA_STRING_NULL;
    UA_UInt16 port = 0;
    UA_String endpointUrlString = UA_STRING((char *)(uintptr_t)endpointUri);
    UA_StatusCode parse_retval = UA_parseEndpointUrl(&endpointUrlString, &hostName, &port, &path);
    if (parse_retval != UA_STATUSCODE_GOOD)
    {
        EDGE_LOG_V(TAG, "Endpoint URL is invalid. Error Code: %s.", UA_StatusCode_name(parse_retval));
        result.code = STATUS_ERROR;
        return result;
    }

    device = (EdgeDevice *) calloc(1, sizeof(EdgeDevice));
    if(!device)
    {
        EDGE_LOG(TAG, "Memory allocation failed.");
        result.code = STATUS_INTERNAL_ERROR;
        goto EXIT;
    }

    device->address = (char *) calloc(hostName.length+1, sizeof(char));
    if(!device->address)
    {
        EDGE_LOG(TAG, "Memory allocation failed.");
        result.code = STATUS_INTERNAL_ERROR;
        goto EXIT;
    }

    memcpy(device->address, hostName.data, hostName.length);
    device->port = port;
    if (path.length > 0)
    {
        device->serverName = (char *) calloc(path.length + 1, sizeof(char));
        if(!device->serverName)
        {
            EDGE_LOG(TAG, "Memory allocation failed.");
            result.code = STATUS_INTERNAL_ERROR;
            goto EXIT;
        }
        memcpy(device->serverName, path.data, path.length);
    }

    client = UA_Client_new(UA_ClientConfig_default);
    if(!client)
    {
        EDGE_LOG(TAG, "UA_Client_new() failed.");
        result.code = STATUS_INTERNAL_ERROR;
        goto EXIT;
    }

    retVal = UA_Client_getEndpoints(client, endpointUri, &endpointArraySize, &endpointArray);
    if (retVal != UA_STATUSCODE_GOOD)
    {
        EDGE_LOG_V(TAG, "Failed to get the endpoints. Error Code: %s.\n", UA_StatusCode_name(retVal));
        result.code = STATUS_INTERNAL_ERROR;
        goto EXIT;
    }

    if(endpointArraySize <= 0)
    {
        EDGE_LOG(TAG, "No endpoints found.");
        result.code = STATUS_OK;
        goto EXIT;
    }

    if(!parseEndpoints(endpointArraySize, endpointArray, &count, &endpointList))
    {
        EDGE_LOG(TAG, "Failed to parse the endpoints.");
        result.code = STATUS_INTERNAL_ERROR;
        goto EXIT;
    }

    if(count <= 0)
    {
        EDGE_LOG(TAG, "No valid endpoints found.");
        result.code = STATUS_OK; // TODO: What should be the result code?
        goto EXIT;
    }

    device->num_endpoints = count;
    device->endpointsInfo = (EdgeEndPointInfo **) calloc(count, sizeof(EdgeEndPointInfo *));
    if (!device->endpointsInfo)
    {
        EDGE_LOG(TAG, "Memory allocation failed.");
        result.code = STATUS_INTERNAL_ERROR;
        goto EXIT;
    }

    List *ptr = endpointList;
    for(int i = 0; i < count; ++i)
    {
        device->endpointsInfo[i] = ptr->data;
        ptr=ptr->link;
    }

    onDiscoveryCallback(device);
    result.code = STATUS_OK;

EXIT:
    if (endpointArray)
    {
        UA_Array_delete(endpointArray, endpointArraySize, &UA_TYPES[UA_TYPES_ENDPOINTDESCRIPTION]);
    }
    deleteList(&endpointList);
    UA_Client_delete(client);
    freeEdgeDevice(device);
    return result;
}
