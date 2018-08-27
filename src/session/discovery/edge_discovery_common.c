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

#include "edge_discovery_common.h"
#include "edge_logger.h"
#include "edge_utils.h"
#include "edge_open62541.h"
#include "edge_malloc.h"

#define TAG "edge_discovery_common"

static uint8_t supportedApplicationTypes;

void logEndpointDescription(UA_EndpointDescription *ep)
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
    EDGE_LOG_V(TAG, "Endpoint application type: %u.\n", ep->server.applicationType);
    EDGE_LOG_V(TAG, "Endpoint gateway server URI: %s.\n", (str = convertUAStringToString(&ep->server.gatewayServerUri)));
    free(str);
    EDGE_LOG_V(TAG, "Endpoint discovery profile URI: %s.\n", (str = convertUAStringToString(&ep->server.discoveryProfileUri)));
    free(str);
    EDGE_LOG_V(TAG, "Endpoint discovery URL count: %d\n", (int) ep->server.discoveryUrlsSize);
    for(size_t i = 0; i < ep->server.discoveryUrlsSize; ++i)
    {
        EDGE_LOG_V(TAG, "Endpoint discovery URL(%d): %s.\n", (int) i+1, (str = convertUAStringToString(&ep->server.discoveryUrls[i])));
        free(str);
    }
#else
    (void) ep;
#endif
}

EdgeApplicationConfig *convertToEdgeApplicationConfig(UA_ApplicationDescription *appDesc)
{
    VERIFY_NON_NULL_MSG(appDesc, "NULL UA_ApplicationDescription parameter\n", NULL);

    EdgeApplicationConfig *appConfig = (EdgeApplicationConfig *) EdgeCalloc(1,
            sizeof(EdgeApplicationConfig));
    VERIFY_NON_NULL_MSG(appConfig, "Memory allocation failed for appConfig.", NULL);

    if (appDesc->applicationUri.length > 0)
    {
        appConfig->applicationUri = convertUAStringToString(&appDesc->applicationUri);
        if (!appConfig->applicationUri)
        {
            EDGE_LOG(TAG, "Memory allocation failed for for appConfig applicationUri.");
            freeEdgeApplicationConfig(appConfig);
            return NULL;
        }
    }

    if (appDesc->productUri.length > 0)
    {
        appConfig->productUri = convertUAStringToString(&appDesc->productUri);
        if (!appConfig->productUri)
        {
            EDGE_LOG(TAG, "Memory allocation failed for appConfig productUri.");
            freeEdgeApplicationConfig(appConfig);
            return NULL;
        }
    }

    if (appDesc->applicationName.text.length > 0)
    {
        appConfig->applicationName = convertUAStringToString(&appDesc->applicationName.text);
        if (!appConfig->applicationName)
        {
            EDGE_LOG(TAG, "Memory allocation failed for appConfig applicationName.");
            freeEdgeApplicationConfig(appConfig);
            return NULL;
        }
    }

    if (appDesc->gatewayServerUri.length > 0)
    {
        appConfig->gatewayServerUri = convertUAStringToString(&appDesc->gatewayServerUri);
        if (!appConfig->gatewayServerUri)
        {
            EDGE_LOG(TAG, "Memory allocation failed for appConfig gatewayServerUri.");
            freeEdgeApplicationConfig(appConfig);
            return NULL;
        }
    }

    if (appDesc->discoveryProfileUri.length > 0)
    {
        appConfig->discoveryProfileUri = convertUAStringToString(&appDesc->discoveryProfileUri);
        if (!appConfig->discoveryProfileUri)
        {
            EDGE_LOG(TAG, "Memory allocation failed for appConfig discoveryProfileUri.");
            freeEdgeApplicationConfig(appConfig);
            return NULL;
        }
    }

    appConfig->applicationType = convertToEdgeApplicationType(appDesc->applicationType);
    appConfig->discoveryUrlsSize = appDesc->discoveryUrlsSize;
    appConfig->discoveryUrls = (char **) calloc(appDesc->discoveryUrlsSize, sizeof(char *));
    if (!appConfig->discoveryUrls)
    {
        EDGE_LOG(TAG, "Memory allocation failed for appConfig discoveryUrls.");
        freeEdgeApplicationConfig(appConfig);
        return NULL;
    }

    for (int i = 0; i < appDesc->discoveryUrlsSize; ++i)
    {
        if (appDesc->discoveryUrls[i].length > 0)
        {
            appConfig->discoveryUrls[i] = convertUAStringToString(&appDesc->discoveryUrls[i]);
            if (!appConfig->discoveryUrls[i])
            {
                EDGE_LOG(TAG, "Memory allocation failed for appConfig discoveryUrls.");
                freeEdgeApplicationConfig(appConfig);
                return NULL;
            }
        }
    }
    return appConfig;
}

EdgeEndPointInfo *convertToEdgeEndpointInfo(UA_EndpointDescription *endpoint)
{
    VERIFY_NON_NULL_MSG(endpoint, "NULL UA_EndpointDescription parameter recevied\n", NULL);

    EdgeEndPointInfo *epInfo = (EdgeEndPointInfo *) EdgeCalloc(1, sizeof(EdgeEndPointInfo));
    VERIFY_NON_NULL_MSG(epInfo, "EdgeCalloc :: Memory allocation failed for epInfo.", NULL);

    if (endpoint->endpointUrl.length > 0)
    {
        epInfo->endpointUri = convertUAStringToString(&endpoint->endpointUrl);
        if (!epInfo->endpointUri)
        {
            EDGE_LOG(TAG, "Memory allocation failed for endpoint->endpointUrl.length.");
            freeEdgeEndpointInfo(epInfo);
            return NULL;
        }
    }

    if (endpoint->securityPolicyUri.length > 0)
    {
        epInfo->securityPolicyUri = convertUAStringToString(&endpoint->securityPolicyUri);
        if (!epInfo->securityPolicyUri)
        {
            EDGE_LOG(TAG, "Memory allocation failed for endpoint->securityPolicyUri.length.");
            freeEdgeEndpointInfo(epInfo);
            return NULL;
        }
    }

    if (endpoint->transportProfileUri.length > 0)
    {
        epInfo->transportProfileUri = convertUAStringToString(&endpoint->transportProfileUri);
        if (!epInfo->transportProfileUri)
        {
            EDGE_LOG(TAG, "Memory allocation failed for endpoint->transportProfileUri.length.");
            freeEdgeEndpointInfo(epInfo);
            return NULL;
        }
    }

    epInfo->securityMode = endpoint->securityMode;
    epInfo->securityLevel = endpoint->securityLevel;
    epInfo->appConfig = convertToEdgeApplicationConfig(&endpoint->server);
    if (!epInfo->appConfig)
    {
        EDGE_LOG(TAG, "Memory allocation failed for epInfo->appConfig.");
        freeEdgeEndpointInfo(epInfo);
        return NULL;
    }
    return epInfo;
}

bool convertUnsignedCharStringToUAString(const unsigned char *str, UA_String *uaStr)
{
    VERIFY_NON_NULL_MSG(str, "NULL unsigned char parameter in convertUnsignedCharStringToUAString\n", false);
    VERIFY_NON_NULL_MSG(uaStr, "NULL UA_String parameter in convertUnsignedCharStringToUAString\n", false);

    uaStr->length = strlen((const char *)str);
    uaStr->data = (UA_Byte *) EdgeMalloc(uaStr->length);
    VERIFY_NON_NULL_MSG(uaStr->data, "Memory allocation failed", false);

    for(int i = 0; i < uaStr->length; ++i)
    {
        uaStr->data[i] = str[i];
    }

    return true;
}

bool convertUnsignedCharStringsToUAStrings(size_t strArrSize,
    unsigned char **strArr, UA_String **uaStrArr)
{
    COND_CHECK_MSG((0 == strArrSize), "Size is 0. There is nothing to convert.", true);
    VERIFY_NON_NULL_MSG(strArr, "Invalid parameter(s).", false);
    VERIFY_NON_NULL_MSG(uaStrArr, "Invalid parameter(s).", false);

    UA_String *uaStrArrLocal = (UA_String *) EdgeCalloc(strArrSize, sizeof(UA_String));
    VERIFY_NON_NULL_MSG(uaStrArrLocal, "Memory allocation failed.", false);

    for(size_t i = 0; i < strArrSize; ++i)
    {
        if(!convertUnsignedCharStringToUAString(strArr[i], &uaStrArrLocal[i]))
        {
            EDGE_LOG(TAG, "Failed to convert 'unsigned char string' to 'UA_String'.");
            for(size_t j = 0; j < i; ++j)
            {
                EdgeFree(uaStrArrLocal[j].data);
            }
            EdgeFree(uaStrArrLocal);
            return false;
        }
    }

    *uaStrArr = uaStrArrLocal;
    return true;
}

void destroyUAStringArrayContents(UA_String *uaStr, size_t uaStrSize)
{
    VERIFY_NON_NULL_NR_MSG(uaStr, "uaStr is null\n");
    COND_CHECK_NR_MSG((0 == uaStrSize), "");

    for(size_t i = 0; i < uaStrSize; ++i)
    {
        EdgeFree(uaStr[i].data);
    }
}

void destroyUAStringArray(UA_String *uaStr, size_t uaStrSize)
{
    VERIFY_NON_NULL_NR_MSG(uaStr, "uaStr is null\n");
    if(uaStrSize > 0)
    {
        destroyUAStringArrayContents(uaStr, uaStrSize);
    }
    EdgeFree(uaStr);
}

bool isApplicationTypeSupported(UA_ApplicationType appType)
{
    COND_CHECK_MSG((appType == __UA_APPLICATIONTYPE_FORCE32BIT), "Application type is invalid.", false);

    bool supported = false;   
    if(appType==UA_APPLICATIONTYPE_SERVER)
        supported = (supportedApplicationTypes & EDGE_APPLICATIONTYPE_SERVER) ? true: false;
    else if(appType==UA_APPLICATIONTYPE_CLIENT)
        supported = (supportedApplicationTypes & EDGE_APPLICATIONTYPE_CLIENT) ? true: false;
    else if(UA_APPLICATIONTYPE_CLIENTANDSERVER)
        supported = (supportedApplicationTypes & EDGE_APPLICATIONTYPE_CLIENTANDSERVER) ? true: false;
    else if(appType==UA_APPLICATIONTYPE_DISCOVERYSERVER)
        supported = (supportedApplicationTypes & EDGE_APPLICATIONTYPE_DISCOVERYSERVER) ? true: false;

    return supported;
}

/**
 * @brief Checks whether the given server URI/locale is present in the given array of acceptable server URIs/ locales.
 * @param[in]  rcvd Received server URI/locale which needs to be checked.
 * @param[in]  size Size of the array of acceptable server URIs / locales.
 * @param[in]  list Array of acceptable server URIs / locales.
 * @return @c True if the given server uri / locale is present in the given array of acceptable server uris / locales, @c False otherwise.
 */
static bool isPresentInList(UA_String *rcvd, size_t size, unsigned char **list)
{
    for(size_t i = 0; i < size; ++i)
    {
        COND_CHECK((0 == memcmp(list[i], rcvd->data, rcvd->length) &&
                    '\0' == list[i][rcvd->length]), true);
    }
    return false;
}

// Behaviour is undefined if hostName is NULL
bool isIPv4AddressValid(UA_String *ipv4Address)
{
    size_t len = ipv4Address->length;
    COND_CHECK((len < 7 || len > 15), false);
    unsigned int value = 0;
    unsigned int numOfDigitsInSegment = 0, numOfDots = 0;
    UA_Byte *data = ipv4Address->data;
    for(int i = 0; i < len; ++i)
    {
        if(data[i] == '.')
        {
            COND_CHECK((numOfDigitsInSegment < 1 || numOfDigitsInSegment > 3 || value > 255), false);
            value = numOfDigitsInSegment = 0;
            numOfDots++;
        }
        else if(data[i] < '0' || data[i] > '9')
        {
            return false;
        }
        else
        {
            value = (value * 10) + (data[i] - '0');
            numOfDigitsInSegment++;
        }
    }
    COND_CHECK((numOfDots != 3 || numOfDigitsInSegment < 1 || numOfDigitsInSegment > 3 || value > 255), false);

    return true;
}

bool isServerAppDescriptionValid(UA_ApplicationDescription *regServer, size_t serverUrisSize,
    unsigned char **serverUris, size_t localeIdsSize, unsigned char **localeIds)
{
    // Application Type and DiscoveryUrls check.
    COND_CHECK_MSG((!isApplicationTypeSupported(regServer->applicationType)), "Application type is not supported.", false);
    COND_CHECK_MSG((0 == regServer->applicationUri.length), "Application URI is empty.", false);

    if(regServer->applicationUri.length < 5)
    {
        EDGE_LOG_V(TAG, "Application URI is invalid. Its length is %zu.\n", regServer->applicationUri.length);
        return false;
    }

    // For those application uri which doesn't start with 'urn:', the uri is assumed to start with 'opc.tcp://'.
    if(strncmp((char*)regServer->applicationUri.data, "urn:", 4) != 0)
    {
        UA_String hostName = UA_STRING_NULL, path = UA_STRING_NULL;
        UA_UInt16 port = 0;
        UA_StatusCode parse_retval = UA_parseEndpointUrl(&regServer->applicationUri, &hostName, &port, &path);
        if (parse_retval != UA_STATUSCODE_GOOD)
        {
            EDGE_LOG_V(TAG, "Application URI is invalid. Error Code: %s.\n", UA_StatusCode_name(parse_retval));
            return false;
        }
        COND_CHECK_MSG((0 == hostName.length), "Hostname in application URI is empty.", false);

        // Validate IPv4 address
        if(hostName.data[0] != '[' && (hostName.data[0] == '1' || hostName.data[0] == '2'))
        {
            COND_CHECK_MSG((!isIPv4AddressValid(&hostName)), "IPv4 address in application URI is invalid.", false);
        }
    }

    // Check whether the received application uri matches with the requested list of serverUris.
    if(serverUrisSize > 0)
    {
        COND_CHECK_MSG((!isPresentInList(&regServer->applicationUri, serverUrisSize, serverUris)),
                       "Application URI doesn't match with the requested list of serverUris.", false);
    }

    // For FindServers CTT TC ERR-012.
    // Application Name Locale check.
    // Check whether the received application name's locale matches with the requested list of locales.
    if(localeIdsSize > 0)
    {
        COND_CHECK_MSG((0 == regServer->applicationName.locale.length),
                       "Application Name's locale is empty.", false);
        COND_CHECK_MSG((!isPresentInList(&regServer->applicationName.locale, localeIdsSize, localeIds)),
                       "Locale of Application Name doesn't match with the requested list of locales.", false);
    }
    return true;
}

void setSupportedApplicationTypesInternal(uint8_t supportedTypes)
{
    supportedApplicationTypes = supportedTypes;
}
