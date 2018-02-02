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
#include "edge_malloc.h"

#include <stdio.h>
#include <open62541.h>
#include <inttypes.h>

#define TAG "session_client"

static edgeMap *sessionClientMap = NULL;
static int clientCount = 0;

static void getAddressPort(char *endpoint, char **out)
{
    UA_String hostName = UA_STRING_NULL, path = UA_STRING_NULL;
    UA_UInt16 port = 0;
    UA_String endpointUrlString = UA_STRING((char *)(uintptr_t)endpoint);

    UA_StatusCode parse_retval = UA_parseEndpointUrl(&endpointUrlString, &hostName, &port, &path);
    if (parse_retval != UA_STATUSCODE_GOOD)
    {
        EDGE_LOG(TAG, "Server URL is invalid. Unable to get endpoints\n");
        return ;
    }

    char address[512];
    strncpy(address, (char*) hostName.data, hostName.length);
    address[hostName.length] = '\0';

    char addr_port[512];
    memset(addr_port, '\0', 512);
    int written = snprintf(addr_port, 512, "%s:%d", address, port);
    if (written > 0)
    {
        *out = (char*) EdgeMalloc(sizeof(char) * (written+1));
        strncpy(*out,  addr_port, written);
        (*out)[written] = '\0';
    }
}

static keyValue getSessionClient(char *endpoint)
{
    if (!sessionClientMap)
        return NULL;
    char *ep = NULL;
    getAddressPort(endpoint, &ep);
    if (!ep)
        return NULL;

    edgeMapNode *temp = sessionClientMap->head;
    while (temp != NULL)
    {
        if (!strcmp(temp->key, ep))
        {
            free (ep); ep = NULL;
            return temp->value;
        }
        temp = temp->next;
    }
    free (ep); ep = NULL;
    return NULL;
}

static edgeMapNode *removeClientFromSessionMap(char *endpoint)
{
    if (!sessionClientMap)
        return NULL;
    char *ep = NULL;
    getAddressPort(endpoint, &ep);
    if (!ep)
        return NULL;

    edgeMapNode *temp = sessionClientMap->head;
    edgeMapNode *prev = NULL;
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
            free (ep); ep = NULL;
            return temp;
        }
        prev = temp;
        temp = temp->next;
    }
    free (ep); ep = NULL;
    return NULL;
}

EdgeResult readNodesFromServer(EdgeMessage *msg)
{
    EdgeResult result = executeRead((UA_Client*) getSessionClient(msg->endpointInfo->endpointUri), msg);
    return result;
}

EdgeResult writeNodesInServer(EdgeMessage *msg)
{
    EdgeResult result = executeWrite((UA_Client*) getSessionClient(msg->endpointInfo->endpointUri), msg);
    return result;
}

EdgeResult browseNodesInServer(EdgeMessage *msg)
{
    EdgeResult result = executeBrowse((UA_Client*) getSessionClient(msg->endpointInfo->endpointUri), msg, false);
    return result;
}

EdgeResult browseViewsInServer(EdgeMessage *msg)
{
    EdgeResult result = executeBrowseViews((UA_Client*) getSessionClient(msg->endpointInfo->endpointUri), msg);
    return result;
}

EdgeResult browseNextInServer(EdgeMessage *msg)
{
    EdgeResult result = executeBrowse((UA_Client*) getSessionClient(msg->endpointInfo->endpointUri), msg, true);
    return result;
}

EdgeResult callMethodInServer(EdgeMessage *msg)
{
    EdgeResult result = executeMethod((UA_Client*) getSessionClient(msg->endpointInfo->endpointUri), msg);
    return result;
}

EdgeResult executeSubscriptionInServer(EdgeMessage *msg)
{
    EdgeResult result = executeSub((UA_Client*) getSessionClient(msg->endpointInfo->endpointUri), msg);
    return result;
}

bool connect_client(char *endpoint)
{
    UA_StatusCode retVal;
    UA_ClientConfig config = UA_ClientConfig_default;
    UA_Client *m_client = NULL;
    char *m_endpoint = NULL;

    if (NULL != getSessionClient(endpoint)) {
        EDGE_LOG(TAG, "client already connected.\n");
        return false;
    }

    m_client = UA_Client_new(config);

    EDGE_LOG_V(TAG, "endpoint :: %s\n", endpoint);
    VERIFY_NON_NULL(m_client, false);
    retVal = UA_Client_connect(m_client, endpoint);
    if (retVal != UA_STATUSCODE_GOOD)
    {
        EDGE_LOG_V(TAG, "\n [CLIENT] Unable to connect 0x%08x!\n", retVal);
        if (m_client) {
            UA_Client_delete(m_client);
            m_client = NULL;
        }
        return false;
    }

    EDGE_LOG(TAG, "\n [CLIENT] Client connection successful \n");
	getAddressPort(endpoint, &m_endpoint);

    // Add the client to session map
    if (NULL == sessionClientMap) {
        sessionClientMap = createMap();
    }
    insertMapElement(sessionClientMap, (keyValue) m_endpoint, (keyValue) m_client);
    clientCount++;

    EdgeEndPointInfo *ep = (EdgeEndPointInfo *) EdgeCalloc(1, sizeof(EdgeEndPointInfo));
    VERIFY_NON_NULL(ep, false);
    ep->endpointUri = endpoint;
    onStatusCallback(ep, STATUS_CLIENT_STARTED);
    free(ep);
    ep = NULL;

    return true;
}

void disconnect_client(EdgeEndPointInfo *epInfo)
{
    edgeMapNode *session = NULL;
    session = removeClientFromSessionMap(epInfo->endpointUri);
    if (session) {
        if (session->key)
        {
            free(session->key);
            session->key = NULL;
        }
        if (session->value)
        {
            UA_Client *m_client =  (UA_Client*) session->value;
            UA_Client_delete(m_client); m_client = NULL;
        }
        free (session); session = NULL;
        clientCount--;
        onStatusCallback(epInfo, STATUS_STOP_CLIENT);
    }
    if (0 == clientCount)
    {
        free (sessionClientMap);
        sessionClientMap = NULL;
    }
}

static void logEndpointDescription(UA_EndpointDescription *ep)
{
#if DEBUG
    if(!ep)
    {
        return;
    }

    char *str = NULL;
    EDGE_LOG_V(TAG, "%s", "\n\n");
    EDGE_LOG(TAG, "----------Endpoint Description--------------");
    EDGE_LOG_V(TAG, "Endpoint URL: %s.\n", (str = convertUAStringToString(&ep->endpointUrl)));
    free(str);
    EDGE_LOG_V(TAG, "Endpoint security mode: %d.\n", ep->securityMode);
    EDGE_LOG_V(TAG, "Endpoint security policy URI: %s.\n", (str = convertUAStringToString(&ep->securityPolicyUri)));
    free(str);
    EDGE_LOG_V(TAG, "Endpoint user identity token count: %d\n", (int) ep->userIdentityTokensSize);
    EDGE_LOG_V(TAG, "Endpoint transport profile URI: %s.\n", (str = convertUAStringToString(&ep->transportProfileUri)));
    free(str);
    EDGE_LOG_V(TAG, "Endpoint security level: %u.\n", ep->securityLevel);
    EDGE_LOG_V(TAG, "Endpoint application URI: %s.\n", (str = convertUAStringToString(&ep->server.applicationUri)));
    free(str);
    EDGE_LOG_V(TAG, "Endpoint product URI: %s.\n", (str = convertUAStringToString(&ep->server.productUri)));
    free(str);
    EDGE_LOG_V(TAG, "Endpoint application name: %s.\n", (str = convertUAStringToString(&ep->server.applicationName.text)));
    free(str);
    EDGE_LOG_V(TAG, "Endpoint application type: %d.\n", ep->server.applicationType);
    EDGE_LOG_V(TAG, "Endpoint gateway server URI: %s.\n", (str = convertUAStringToString(&ep->server.gatewayServerUri)));
    free(str);
    EDGE_LOG_V(TAG, "Endpoint discovery profile URI: %s.\n", (str = convertUAStringToString(&ep->server.discoveryProfileUri)));
    free(str);
    EDGE_LOG_V(TAG, "Endpoint discovery URL count: %d\n", (int) ep->server.discoveryUrlsSize);
    for(int i = 0; i < ep->server.discoveryUrlsSize; ++i)
    {
        EDGE_LOG_V(TAG, "Endpoint discovery URL(%d): %s.\n", i+1, (str = convertUAStringToString(&ep->server.discoveryUrls[i])));
        free(str);
    }
#else
    (void)ep;
#endif
}

static EdgeApplicationConfig *convertToEdgeApplicationConfig(UA_ApplicationDescription *appDesc)
{
    if(!appDesc)
    {
        return NULL;
    }

    EdgeApplicationConfig *appConfig = (EdgeApplicationConfig *) EdgeCalloc(1, sizeof(EdgeApplicationConfig));
    if(!appConfig)
    {
        EDGE_LOG(TAG, "Memory allocation failed for appConfig.");
        goto ERROR;
    }

    if (appDesc->applicationUri.length > 0)
    {
        appConfig->applicationUri = convertUAStringToString(&appDesc->applicationUri);
        if(!appConfig->applicationUri)
        {
            EDGE_LOG(TAG, "Memory allocation failed for for appConfig applicationUri.");
            goto ERROR;
        }
    }

    if (appDesc->productUri.length > 0)
    {
        appConfig->productUri = convertUAStringToString(&appDesc->productUri);
        if(!appConfig->productUri)
        {
            EDGE_LOG(TAG, "Memory allocation failed for appConfig productUri.");
            goto ERROR;
        }
    }

    if (appDesc->applicationName.text.length > 0)
    {
        appConfig->applicationName = convertUAStringToString(&appDesc->applicationName.text);
        if(!appConfig->applicationName)
        {
            EDGE_LOG(TAG, "Memory allocation failed for appConfig applicationName.");
            goto ERROR;
        }
    }

    if (appDesc->gatewayServerUri.length > 0)
    {
        appConfig->gatewayServerUri = convertUAStringToString(&appDesc->gatewayServerUri);
        if(!appConfig->gatewayServerUri)
        {
            EDGE_LOG(TAG, "Memory allocation failed for appConfig gatewayServerUri.");
            goto ERROR;
        }
    }

    if (appDesc->discoveryProfileUri.length > 0)
    {
        appConfig->discoveryProfileUri = convertUAStringToString(&appDesc->discoveryProfileUri);
        if(!appConfig->discoveryProfileUri)
        {
            EDGE_LOG(TAG, "Memory allocation failed for appConfig discoveryProfileUri.");
            goto ERROR;
        }
    }

    appConfig->applicationType = appDesc->applicationType;
    appConfig->discoveryUrlsSize = appDesc->discoveryUrlsSize;
    appConfig->discoveryUrls = (char **)calloc(appDesc->discoveryUrlsSize, sizeof(char *));
    if(!appConfig->discoveryUrls)
    {
        EDGE_LOG(TAG, "Memory allocation failed for appConfig discoveryUrls.");
        goto ERROR;
    }

    for(int i = 0; i < appDesc->discoveryUrlsSize; ++i)
    {
        if(appDesc->discoveryUrls[i].length > 0)
        {
            appConfig->discoveryUrls[i] = convertUAStringToString(&appDesc->discoveryUrls[i]);
            if(!appConfig->discoveryUrls[i])
            {
                EDGE_LOG(TAG, "Memory allocation failed for appConfig discoveryUrls.");
                goto ERROR;
            }
        }
    }

    return appConfig;

ERROR:
    freeEdgeApplicationConfig(appConfig);
    return NULL;
}

static EdgeEndPointInfo *convertToEdgeEndpointInfo(UA_EndpointDescription *endpoint)
{
    EDGE_LOG(TAG, "INSIDE convertToEdgeEndpointInfo \n");
    if(!endpoint)
    {
        return NULL;
    }

    EDGE_LOG(TAG, "INSIDE convertToEdgeEndpointInfo 2\n");

    EdgeEndPointInfo *epInfo = (EdgeEndPointInfo *) EdgeCalloc(1, sizeof(EdgeEndPointInfo));
    if(!epInfo)
    {
        EDGE_LOG(TAG, "EdgeCalloc :: Memory allocation failed for epInfo.");
        goto ERROR;
    }

    if (endpoint->endpointUrl.length > 0)
    {
        epInfo->endpointUri = convertUAStringToString(&endpoint->endpointUrl);
        if(!epInfo->endpointUri)
        {
            EDGE_LOG(TAG, "Memory allocation failed for endpoint->endpointUrl.length.");
            goto ERROR;
        }
    }

    if (endpoint->securityPolicyUri.length > 0)
    {
        epInfo->securityPolicyUri = convertUAStringToString(&endpoint->securityPolicyUri);
        if(!epInfo->securityPolicyUri)
        {
            EDGE_LOG(TAG, "Memory allocation failed for endpoint->securityPolicyUri.length.");
            goto ERROR;
        }
    }

    if (endpoint->transportProfileUri.length > 0)
    {
        epInfo->transportProfileUri = convertUAStringToString(&endpoint->transportProfileUri);
        if(!epInfo->transportProfileUri)
        {
            EDGE_LOG(TAG, "Memory allocation failed for endpoint->transportProfileUri.length.");
            goto ERROR;
        }
    }

    epInfo->securityMode = endpoint->securityMode;
    epInfo->securityLevel = endpoint->securityLevel;
    epInfo->appConfig = convertToEdgeApplicationConfig(&endpoint->server);
    if(!epInfo->appConfig)
    {
        EDGE_LOG(TAG, "Memory allocation failed for epInfo->appConfig.");
        goto ERROR;
    }

    return epInfo;

ERROR:
    freeEdgeEndpointInfo(epInfo);
    return NULL;
}

static bool parseEndpoints(size_t endpointArraySize, UA_EndpointDescription *endpointArray, size_t *count, List **endpointList)
{
    EdgeEndPointInfo *epInfo = NULL;
    for (size_t i = 0; i < endpointArraySize; ++i)
    {
        logEndpointDescription(&endpointArray[i]);

        /*if (UA_APPLICATIONTYPE_CLIENT == endpointArray[i].server.applicationType)
        {
            EDGE_LOG(TAG, "Found client type endpoint. Skipping it.");
            continue;
        }*/

        if(endpointArray[i].endpointUrl.length <= 0)
        {
            EDGE_LOG(TAG, "EndpointURI is empty. Endpoint is invalid.");
            continue;
        }

        if (UA_MESSAGESECURITYMODE_INVALID == endpointArray[i].securityMode)
        {
            EDGE_LOG(TAG, "Invalid message security mode. Endpoint is invalid.");
            continue;
        }

        if(endpointArray[i].securityPolicyUri.length <= 0)
        {
            EDGE_LOG(TAG, "Security policy URI is empty. Endpoint is invalid.");
            continue;
        }

        char securityPolicyUriPrefix[] = "http://opcfoundation.org/UA/SecurityPolicy#";
        if(endpointArray[i].securityPolicyUri.length < sizeof(securityPolicyUriPrefix) ||
            memcmp(endpointArray[i].securityPolicyUri.data, securityPolicyUriPrefix, sizeof(securityPolicyUriPrefix)-1))
        {
            EDGE_LOG(TAG, "Malformed security policy URI. Endpoint is invalid");
            continue;
        }

        if(endpointArray[i].transportProfileUri.length <= 0)
        {
            EDGE_LOG(TAG, "Transport profile URI is empty. Endpoint is invalid.");
            continue;
        }

        char transportProfileUriPrefix[] = "http://opcfoundation.org/UA-Profile/Transport/";
        if(endpointArray[i].transportProfileUri.length < sizeof(transportProfileUriPrefix) ||
            memcmp(endpointArray[i].transportProfileUri.data, transportProfileUriPrefix, sizeof(transportProfileUriPrefix)-1))
        {
            EDGE_LOG(TAG, "Malformed transport profile URI. Endpoint is invalid");
            continue;
        }

        if(endpointArray[i].server.applicationUri.length <= 0)
        {
            EDGE_LOG(TAG, "Application URI is empty. Endpoint is invalid.");
            continue;
        }
#if 0
        if(endpointArray[i].server.discoveryUrlsSize <= 0)
        {
            EDGE_LOG(TAG, "Discovery URL is empty. Endpoint is invalid.");
            continue;
        }
#endif

        if(UA_APPLICATIONTYPE_CLIENT == endpointArray[i].server.applicationType)
        {
            if(endpointArray[i].server.gatewayServerUri.length != 0)
            {
                EDGE_LOG(TAG, "Application Type is client but gateway server URI is not empty. Endpoint is invalid");
                continue;
            }
            else if(endpointArray[i].server.discoveryProfileUri.length != 0)
            {
                EDGE_LOG(TAG, "Application Type is client but discovery profile URI is not empty. Endpoint is invalid");
                continue;
            }
            else if(endpointArray[i].server.discoveryUrlsSize != 0)
            {
                EDGE_LOG(TAG, "Application Type is client but discovery URL is not empty. Endpoint is invalid");
                continue;
            }
        }

        bool valid = true;
        int tokenCount = endpointArray[i].userIdentityTokensSize;
        for(int tokenIndex = 0; tokenIndex < tokenCount; ++tokenIndex)
        {
            UA_UserTokenPolicy *policy = &endpointArray[i].userIdentityTokens[tokenIndex];
            if(UA_USERTOKENTYPE_ISSUEDTOKEN == policy->tokenType && policy->issuedTokenType.length <= 0)
            {
                EDGE_LOG(TAG, "Token type is 'ISSUEDTOKEN' but token is empty. Endpoint is invalid");
                valid = false;
                break;
            }
            else if(UA_USERTOKENTYPE_ISSUEDTOKEN != policy->tokenType && policy->issuedTokenType.length > 0)
            {
                EDGE_LOG(TAG, "Token type is not 'ISSUEDTOKEN' but token exists.");
            }
        }

        if(!valid)
        {
            continue;
        }

        UA_ApplicationDescription desc;
        UA_ApplicationDescription_init(&desc);
        if(!memcmp(&desc, &endpointArray[i].server, sizeof(desc)))
        {
            EDGE_LOG(TAG, "Application description is empty. Endpoint is invalid.");
            continue;
        }

        if(0 == endpointArray[i].securityLevel)
        {
            EDGE_LOG(TAG, "Security level is 0. Connection to this endpoint will be insecure.");
        }

        if (UA_MESSAGESECURITYMODE_NONE == endpointArray[i].securityMode)
        {
            EDGE_LOG(TAG, "Security mode is 'NONE'. Connection to this endpoint will be insecure.");
        }

        epInfo = convertToEdgeEndpointInfo(&endpointArray[i]);
        if(!epInfo)
        {
            EDGE_LOG(TAG, "Memory allocation failed.");
            goto ERROR;
        }

        if(!addListNode(endpointList, epInfo))
        {
            EDGE_LOG(TAG, "Failed to add endpoint in result list.");
            goto ERROR;
        }
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

    device = (EdgeDevice *) EdgeCalloc(1, sizeof(EdgeDevice));
    if(!device)
    {
        EDGE_LOG(TAG, "Memory allocation failed.");
        result.code = STATUS_INTERNAL_ERROR;
        goto EXIT;
    }

    device->address = (char *) EdgeCalloc(hostName.length+1, sizeof(char));
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
        device->serverName = (char *) EdgeCalloc(path.length + 1, sizeof(char));
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
    if(client)
    {
        UA_Client_delete(client);
    }
    freeEdgeDevice(device);
    return result;
}
