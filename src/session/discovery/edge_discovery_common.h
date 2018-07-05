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

/**
 * @file edge_dicovery_common.h
 *
 * @brief This file contains common APIs for discovery services such as GetEndpoints, FindServers, etc.
 */

#ifndef EDGE_DISCOVERY_COMMON_H
#define EDGE_DISCOVERY_COMMON_H

#include <inttypes.h>

#include "opcua_common.h"
#include "command_adapter.h"
#include "open62541.h"

#ifdef __cplusplus
extern "C"
{
#endif

/**
 * @brief Set the supported application types
 * @param[in]  supportedTypes application types to be supported
 */
void setSupportedApplicationTypesInternal(uint8_t supportedTypes);

/**
 * @brief Prints the endpoint information on console.
 * @remarks Works only when the stack is built in debug mode.
 * @param[in]  ep Valid pointer to UA_EndpointDescription.
 */
void logEndpointDescription(UA_EndpointDescription *ep);

/**
 * @brief Creates an EdgeApplicationConfig from the given UA_ApplicationDescription.
 * @remarks Allocated memory should be freed by the caller.
 * @param[in]  appDesc Valid pointer to UA_ApplicationDescription.
 * @return Pointer to EdgeApplicationConfig on success, otherwise NULL.
 */
EdgeApplicationConfig *convertToEdgeApplicationConfig(UA_ApplicationDescription *appDesc);

/**
 * @brief Creates an EdgeEndPointInfo from the given UA_EndpointDescription.
 * @remarks Allocated memory should be freed by the caller.
 * @param[in]  appDesc Valid pointer to UA_EndpointDescription.
 * @return Pointer to EdgeEndPointInfo on success, otherwise NULL.
 */
EdgeEndPointInfo *convertToEdgeEndpointInfo(UA_EndpointDescription *endpoint);

/**
 * @brief Creates an UAString from the given unsigned char string.
 * @remarks Allocated memory ('data' field of UA_String) should be freed by the caller.
 * @param[in]  str Valid pointer to unsigned char string.
 * @param[out] uaStr UA_String with 'data' field carrying the copy of the given unsigned char string.
 * @return @c True on success, @c False otherwise.
 */
bool convertUnsignedCharStringToUAString(const unsigned char *str, UA_String *uaStr);

/**
 * @brief Creates an array of UAStrings from the given unsigned char strings.
 * @remarks Allocated memory (Array of UA_Strings and 'data' field of each UA_String) should be freed by the caller.
 * @param[in]  strArrSize Size of the unsigned char string array.
 * @param[in]  strArr Array of unsigned char strings.
 * @param[out] uaStrArr Array of UA_Strings with 'data' field carrying the copy of the given unsigned char strings.
 * @return @c True on success, @c False otherwise.
 */
bool convertUnsignedCharStringsToUAStrings(size_t strArrSize,
    unsigned char **strArr, UA_String **uaStrArr);

/**
 * @brief De-allocates the memory used by the 'data' field of the given UA_Strings.
 * @param[in]  uaStr Array of UA_Strings.
 * @param[in]  uaStrSize Size of the UA_String array.
 */
void destroyUAStringArrayContents(UA_String *uaStr, size_t uaStrSize);

/**
 * @brief De-allocates an array of UA_Strings.
 * @param[in]  uaStr Array of UA_Strings.
 * @param[in]  uaStrSize Size of the UA_String array.
 */
void destroyUAStringArray(UA_String *uaStr, size_t uaStrSize);

/**
 * @brief Checks whether the given application type is supported by the client application.
 * @param[in]  appType ApplicationType which needs to be checked.
 * @return @c True if the application type is supported by the client application, @c False otherwise.
 */
bool isApplicationTypeSupported(UA_ApplicationType appType);

/**
 * @brief Checks whether the given IPv4 is valid.
 * @param[in]  ipv4Address IPv4 address.
 * @return @c True if the given IPv4 address is valid, @c False otherwise.
 */
bool isIPv4AddressValid(UA_String *ipv4Address);

/**
 * @brief Checks whether the given application description is valid.
 * @param[in]  regServer UA_ApplicationDescription which needs to be validated.
 * @param[in]  serverUrisSize Size of the array of acceptable server URIs.
 * @param[in]  serverUris Array of acceptable server URIs.
 * @param[in]  localeIdsSize Size of the array of acceptable locales.
 * @param[in]  localeIds Array of acceptable locales.
 * @return @c True if the given application description is valid, @c False otherwise.
 */
bool isServerAppDescriptionValid(UA_ApplicationDescription *regServer, size_t serverUrisSize,
    unsigned char **serverUris, size_t localeIdsSize, unsigned char **localeIds);

#ifdef __cplusplus
}
#endif

#endif // EDGE_DISCOVERY_COMMON_H
