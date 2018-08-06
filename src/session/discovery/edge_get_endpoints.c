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

#include "edge_get_endpoints.h"
#include "edge_discovery_common.h"
#include "edge_logger.h"
#include "edge_utils.h"
#include "edge_list.h"
#include "edge_malloc.h"

#include "open62541.h"

#define TAG "edge_get_endpoints"

static discovery_cb_t g_discoveryCallback = NULL;

static bool parseEndpoints(size_t endpointArraySize, UA_EndpointDescription *endpointArray,
        size_t *count, List **endpointList)
{
    EdgeEndPointInfo *epInfo = NULL;
    for (size_t i = 0; i < endpointArraySize; ++i)
    {
        logEndpointDescription(&endpointArray[i]);

        if(!isApplicationTypeSupported(endpointArray[i].server.applicationType))
        {
             EDGE_LOG(TAG, "Found an endpoint with unsupported application type. Excluding it.");
             continue;
        }

        if (endpointArray[i].endpointUrl.length <= 0)
        {
            EDGE_LOG(TAG, "EndpointURI is empty. Endpoint is invalid.");
            continue;
        }

        if (UA_MESSAGESECURITYMODE_INVALID == endpointArray[i].securityMode)
        {
            EDGE_LOG(TAG, "Invalid message security mode. Endpoint is invalid.");
            continue;
        }

        if (endpointArray[i].securityPolicyUri.length <= 0)
        {
            EDGE_LOG(TAG, "Security policy URI is empty. Endpoint is invalid.");
            continue;
        }

        char securityPolicyUriPrefix[] = "http://opcfoundation.org/UA/SecurityPolicy#";
        if (endpointArray[i].securityPolicyUri.length < sizeof(securityPolicyUriPrefix)
                || memcmp(endpointArray[i].securityPolicyUri.data, securityPolicyUriPrefix,
                        sizeof(securityPolicyUriPrefix) - 1))
        {
            EDGE_LOG(TAG, "Malformed security policy URI. Endpoint is invalid");
            continue;
        }

        if (endpointArray[i].transportProfileUri.length <= 0)
        {
            EDGE_LOG(TAG, "Transport profile URI is empty. Endpoint is invalid.");
            continue;
        }

        char transportProfileUriPrefix[] = "http://opcfoundation.org/UA-Profile/Transport/";
        if (endpointArray[i].transportProfileUri.length < sizeof(transportProfileUriPrefix)
                || memcmp(endpointArray[i].transportProfileUri.data, transportProfileUriPrefix,
                        sizeof(transportProfileUriPrefix) - 1))
        {
            EDGE_LOG(TAG, "Malformed transport profile URI. Endpoint is invalid");
            continue;
        }

        if (endpointArray[i].server.applicationUri.length <= 0)
        {
            EDGE_LOG(TAG, "Application URI is empty. Endpoint is invalid.");
            continue;
        }
#if 0 // Uncomment this part of code after checking why opc ua c server sent an empty list.
        if(endpointArray[i].server.discoveryUrlsSize <= 0)
        {
            EDGE_LOG(TAG, "Discovery URL is empty. Endpoint is invalid.");
            continue;
        }
#endif

        if (UA_APPLICATIONTYPE_CLIENT == endpointArray[i].server.applicationType)
        {
            if (endpointArray[i].server.gatewayServerUri.length != 0)
            {
                EDGE_LOG(TAG, "Application Type is client but gateway server URI is not empty. Endpoint is invalid");
                continue;
            }
            else if (endpointArray[i].server.discoveryProfileUri.length != 0)
            {
                EDGE_LOG(TAG, "Application Type is client but discovery profile URI is not empty. Endpoint is invalid");
                continue;
            }
            else if (endpointArray[i].server.discoveryUrlsSize != 0)
            {
                EDGE_LOG(TAG, "Application Type is client but discovery URL is not empty. Endpoint is invalid");
                continue;
            }
        }

        bool valid = true;
        size_t tokenCount = endpointArray[i].userIdentityTokensSize;
        for (size_t tokenIndex = 0; tokenIndex < tokenCount; ++tokenIndex)
        {
            UA_UserTokenPolicy *policy = &endpointArray[i].userIdentityTokens[tokenIndex];
            if (UA_USERTOKENTYPE_ISSUEDTOKEN == policy->tokenType
                    && policy->issuedTokenType.length <= 0)
            {
                EDGE_LOG(TAG, "Token type is 'ISSUEDTOKEN' but token is empty. Endpoint is invalid");
                valid = false;
                break;
            }
            else if (UA_USERTOKENTYPE_ISSUEDTOKEN != policy->tokenType
                    && policy->issuedTokenType.length > 0)
            {
                EDGE_LOG(TAG, "Token type is not 'ISSUEDTOKEN' but token exists.");
            }
        }

        if (!valid)
        {
            continue;
        }

        UA_ApplicationDescription desc;
        UA_ApplicationDescription_init(&desc);
        if (!memcmp(&desc, &endpointArray[i].server, sizeof(desc)))
        {
            EDGE_LOG(TAG, "Application description is empty. Endpoint is invalid.");
            continue;
        }

        if (0 == endpointArray[i].securityLevel)
        {
            EDGE_LOG(TAG, "Security level is 0. Connection to this endpoint will be insecure.");
        }

        if (UA_MESSAGESECURITYMODE_NONE == endpointArray[i].securityMode)
        {
            EDGE_LOG(TAG, "Security mode is 'NONE'. Connection to this endpoint will be insecure.");
        }

        epInfo = convertToEdgeEndpointInfo(&endpointArray[i]);
        if (!epInfo)
        {
            EDGE_LOG(TAG, "Memory allocation failed.");
            goto ENDPOINT_ERROR;
        }

        if (!addListNode(endpointList, epInfo))
        {
            EDGE_LOG(TAG, "Failed to add endpoint in result list.");
            goto ENDPOINT_ERROR;
        }
        ++(*count);
        epInfo = NULL;
    }

    return true;

    ENDPOINT_ERROR:
    // Deallocate memory
    for (List *listPtr = *endpointList; listPtr; listPtr = listPtr->link)
    {
        freeEdgeEndpointInfo(listPtr->data);
    }
    freeEdgeEndpointInfo(epInfo);

    return false;
}

EdgeResult getEndpointsInternal(char *endpointUri)
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
    UA_String endpointUrlString = UA_STRING((char *) (uintptr_t) endpointUri);
    UA_StatusCode parse_retval = UA_parseEndpointUrl(&endpointUrlString, &hostName, &port, &path);
    if (parse_retval != UA_STATUSCODE_GOOD)
    {
        EDGE_LOG_V(TAG, "Endpoint URL is invalid. Error Code: %s.", UA_StatusCode_name(parse_retval));
        result.code = STATUS_PARAM_INVALID;
        return result;
    }

    device = (EdgeDevice *) EdgeCalloc(1, sizeof(EdgeDevice));
    if (!device)
    {
        EDGE_LOG(TAG, "Memory allocation failed.");
        result.code = STATUS_INTERNAL_ERROR;
        goto EXIT;
    }

    device->address = (char *) EdgeCalloc(hostName.length + 1, sizeof(char));
    if (!device->address)
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
        if (!device->serverName)
        {
            EDGE_LOG(TAG, "Memory allocation failed.");
            result.code = STATUS_INTERNAL_ERROR;
            goto EXIT;
        }
        memcpy(device->serverName, path.data, path.length);
    }

    client = UA_Client_new(UA_ClientConfig_default);
    if (!client)
    {
        EDGE_LOG(TAG, "UA_Client_new() failed.");
        result.code = STATUS_INTERNAL_ERROR;
        goto EXIT;
    }

    retVal = UA_Client_getEndpoints(client, endpointUri, &endpointArraySize, &endpointArray);
    if (retVal != UA_STATUSCODE_GOOD)
    {
        EDGE_LOG_V(TAG, "Failed to get the endpoints. Error Code: %s.\n", UA_StatusCode_name(retVal));
        result.code = STATUS_SERVICE_RESULT_BAD;
        goto EXIT;
    }

    if (0 == endpointArraySize)
    {
        EDGE_LOG(TAG, "No endpoints found.");
        g_discoveryCallback(device);
        result.code = STATUS_OK;
        goto EXIT;
    }

    if (!parseEndpoints(endpointArraySize, endpointArray, &count, &endpointList))
    {
        EDGE_LOG(TAG, "Failed to parse the endpoints.");
        result.code = STATUS_INTERNAL_ERROR;
        goto EXIT;
    }

    if (0 == count)
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
    for (size_t i = 0; i < count; ++i)
    {
        device->endpointsInfo[i] = ptr->data;
        ptr = ptr->link;
    }

    g_discoveryCallback(device);
    result.code = STATUS_OK;

    EXIT:
    // Deallocate memory
    if (endpointArray)
    {
        UA_Array_delete(endpointArray, endpointArraySize, &UA_TYPES[UA_TYPES_ENDPOINTDESCRIPTION]);
    }
    deleteList(&endpointList);
    if (client)
    {
        UA_Client_delete(client);
    }
    freeEdgeDevice(device);
    return result;
}

void registerGetEndpointsCb(discovery_cb_t discoveryCallback)
{
    g_discoveryCallback = discoveryCallback;
}
