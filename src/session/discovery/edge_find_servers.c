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

#include "edge_find_servers.h"
#include "edge_discovery_common.h"
#include "edge_logger.h"
#include "edge_utils.h"
#include "edge_malloc.h"

#include "open62541.h"

#define TAG "edge_find_servers"

EdgeResult findServersInternal(const char *endpointUri, size_t serverUrisSize,
    unsigned char **serverUris, size_t localeIdsSize, unsigned char **localeIds,
    size_t *registeredServersSize, EdgeApplicationConfig **registeredServers)
{
    EdgeResult res;
    res.code = STATUS_PARAM_INVALID;
    VERIFY_NON_NULL_MSG(endpointUri, "endpointUri is NULL.", res);

    COND_CHECK_MSG((serverUrisSize > 0 && IS_NULL(serverUris)), "serverUrisSize is > 0 but serverUris is NULL.", res);
    for(size_t i = 0; i < serverUrisSize; ++i)
    {
        if(IS_NULL(serverUris[i]))
        {
            EDGE_LOG_V(TAG, "serverUris[%zu] is NULL.", i);
            res.code = STATUS_PARAM_INVALID;
            return res;
        }
    }

    COND_CHECK_MSG((localeIdsSize > 0 && IS_NULL(localeIds)), "localeIdsSize is > 0 but localeIds is NULL.", res);
    for(size_t i = 0; i < localeIdsSize; ++i)
    {
        if(IS_NULL(localeIds[i]))
        {
            EDGE_LOG_V(TAG, "localeIds[%zu] is NULL.", i);
            res.code = STATUS_PARAM_INVALID;
            return res;
        }
    }

    VERIFY_NON_NULL_MSG(registeredServersSize, "registeredServersSize is NULL.", res);
    VERIFY_NON_NULL_MSG(registeredServers, "registeredServers is NULL.", res);

    UA_String hostName = UA_STRING_NULL, path = UA_STRING_NULL;
    UA_UInt16 port = 0;
    UA_String endpointUrlString = UA_STRING((char *) (uintptr_t) endpointUri);
    UA_StatusCode parse_retval = UA_parseEndpointUrl(&endpointUrlString, &hostName, &port, &path);
    if (parse_retval != UA_STATUSCODE_GOOD)
    {
        EDGE_LOG_V(TAG, "Endpoint URL is invalid. Error Code: %s.", UA_StatusCode_name(parse_retval));
        res.code = STATUS_PARAM_INVALID;
        return res;
    }

    // Convert Server URIs.
    UA_String *serverUrisUA = NULL;
    if(serverUrisSize >0 && !convertUnsignedCharStringsToUAStrings(serverUrisSize, serverUris, &serverUrisUA))
    {
        EDGE_LOG(TAG, "Failed to convert the serverUris from 'unsigned char string' to 'UA_String'.");
        res.code = STATUS_ERROR;
        return res;
    }

    // Convert Locale Ids.
    UA_String *localeIdsUA = NULL;
    if(localeIdsSize >0 && !convertUnsignedCharStringsToUAStrings(localeIdsSize, localeIds, &localeIdsUA))
    {
        EDGE_LOG(TAG, "Failed to convert the localeIds from 'unsigned char string' to 'UA_String'.");
        // Free serverUrisUA.
        destroyUAStringArray(serverUrisUA, serverUrisSize);
        res.code = STATUS_ERROR;
        return res;
    }

    UA_Client *client = UA_Client_new(UA_ClientConfig_default);
    if (IS_NULL(client))
    {
        EDGE_LOG(TAG, "UA_Client_new() failed.");
        destroyUAStringArray(serverUrisUA, serverUrisSize);
        destroyUAStringArray(localeIdsUA, localeIdsSize);
        res.code = STATUS_ERROR;
        return res;
    }

    size_t regServersCount = 0;
    UA_ApplicationDescription *regServers = NULL;
    UA_StatusCode retVal = UA_Client_findServers(client, endpointUri, serverUrisSize,
            serverUrisUA, localeIdsSize, localeIdsUA, &regServersCount, &regServers);
    if (retVal != UA_STATUSCODE_GOOD)
    {
        EDGE_LOG_V(TAG, "Failed to find the servers. Error Code: %s.\n", UA_StatusCode_name(retVal));
        UA_Client_delete(client);
        destroyUAStringArray(serverUrisUA, serverUrisSize);
        destroyUAStringArray(localeIdsUA, localeIdsSize);
        res.code = STATUS_SERVICE_RESULT_BAD;
        return res;
    }

    size_t validServerCount = 0;
    EdgeApplicationConfig **appConfigAll = (EdgeApplicationConfig **)
            EdgeCalloc(regServersCount, sizeof(EdgeApplicationConfig *));
    if (IS_NULL(appConfigAll))
    {
        EDGE_LOG(TAG, "Failed to allocated memory.");
        for(size_t idx = 0; idx < regServersCount ; ++idx)
        {
            UA_ApplicationDescription_deleteMembers(&regServers[idx]);
        }
        EdgeFree(regServers);

        UA_Client_delete(client);
        destroyUAStringArray(serverUrisUA, serverUrisSize);
        destroyUAStringArray(localeIdsUA, localeIdsSize);
        res.code = STATUS_INTERNAL_ERROR;
        return res;
    }

    for(size_t i = 0; i < regServersCount; ++i)
    {
        // Add validation and filtering logic here.
        if(!isServerAppDescriptionValid(&regServers[i], serverUrisSize, serverUris, localeIdsSize, localeIds))
        {
            EDGE_LOG(TAG, "Excluding the invalid server application information.");
            continue;
        }

        appConfigAll[i] = convertToEdgeApplicationConfig(&regServers[i]);
        if(IS_NULL(appConfigAll[i]))
        {
            EDGE_LOG(TAG, "Failed to convert UA_ApplicationDescription");
            for(size_t j = 0; j < i; ++j)
            {
                freeEdgeApplicationConfig(appConfigAll[j]);
            }
            EdgeFree(appConfigAll);

            for(size_t idx = 0; idx < regServersCount ; ++idx)
            {
                UA_ApplicationDescription_deleteMembers(&regServers[idx]);
            }
            EdgeFree(regServers);

            UA_Client_delete(client);
            destroyUAStringArray(serverUrisUA, serverUrisSize);
            destroyUAStringArray(localeIdsUA, localeIdsSize);
            res.code = STATUS_INTERNAL_ERROR;
            return res;
        }

        // If gatewayServerUri is same as the server endpoint, then set the gatewayServerUri to NULL to prevent infinite loop.
        if(regServers[i].gatewayServerUri.length > 0)
        {
            if(0 == strncmp(endpointUri, (char *)regServers[i].gatewayServerUri.data, regServers[i].gatewayServerUri.length)
                && '\0' == endpointUri[regServers[i].gatewayServerUri.length])
            {
                EDGE_LOG(TAG, "Found a discovery server type application with gatewayServerUri same as the server endpoint.");
                EDGE_LOG(TAG, "Setting gatewayServerUri to NULL");
                EdgeFree(appConfigAll[i]->gatewayServerUri);
                appConfigAll[i]->gatewayServerUri = NULL;
            }
        }

        validServerCount++;
    }

    if(validServerCount > 0)
    {
        EdgeApplicationConfig *appConfigFiltered = (EdgeApplicationConfig *)
                EdgeCalloc(validServerCount, sizeof(EdgeApplicationConfig));

        for(size_t i = 0, j = 0; i < regServersCount; ++i)
        {
            if(IS_NOT_NULL(appConfigAll[i]))
            {
                appConfigFiltered[j++] = *appConfigAll[i];
                EdgeFree(appConfigAll[i]);
            }
        }
        *registeredServers = appConfigFiltered;
    }
    else
    {
        if(regServersCount > 0)
        {
            EDGE_LOG(TAG, "All received server applications are invalid.");
        }
        else
        {
            EDGE_LOG(TAG, "No server applications received.");
        }
        *registeredServers = NULL;
    }

    *registeredServersSize = validServerCount;
    res.code = STATUS_OK;

    // Deallocate memory
    EdgeFree(appConfigAll);

    for(size_t idx = 0; idx < regServersCount ; ++idx)
    {
        UA_ApplicationDescription_deleteMembers(&regServers[idx]);
    }
    EdgeFree(regServers);

    destroyUAStringArray(serverUrisUA, serverUrisSize);
    destroyUAStringArray(localeIdsUA, localeIdsSize);
    UA_Client_delete(client);
    return res;
}
