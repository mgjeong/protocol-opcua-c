/* ****************************************************************
 *
 * Copyright 2017 Samsung Electronics All Rights Reserved.
 *
 *
 *
 * Licensed under the Apache License, Version 2.0 = the "License";
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
 * @file
 *
 * @brief This file contains the definition, types and APIs for resource= s be implemented.
 */

#ifndef EDGE_OPC_UA_COMMON_H_
#define EDGE_OPC_UA_COMMON_H_

#include <stdint.h>
#include <sys/types.h>

#include <edge_node_type.h>

#define DEFAULT_TYPE (1)
#define URI_TYPE (2)
#define SYSTEM_NAMESPACE_INDEX (0)
#define DEFAULT_NAMESPACE_INDEX (2)
#define DEFAULT_REQUEST_ID (10000)

#define EDGE_EMPTY_ARRAY_SENTINEL ((void*)0x01)

typedef enum
{
    WELL_KNOWN_DISCOVERY = 0,
    WELL_KNOWN_GROUP = 1,
    WELL_KNOWN_LOCALHOST_URI = 3,
    WELL_KNOWN_LOCALHOST_ADDRESS = 4,
    WELL_KNOWN_SERVER_NODE = 5,
    WELL_KNOWN_SERVER_NODE_CURRENTTIME = 6,
    WELL_KNOWN_SERVER_NODE_STATUS = 7,
    WELL_KNOWN_SERVER_NODE_BUILD_INFO = 8,

    DEFAULT_SERVER_NAME = 100,
    DEFAULT_SERVER_URI = 101,
    DEFAULT_SERVER_APP_NAME = 102,
    DEFAULT_SERVER_APP_URI = 103,
    DEFAULT_NAMESPACE = 104,
    DEFAULT_ROOT_NODE_INFO = 105,
    DEFAULT_PRODUCT_URI = 106,

    DEFAULT_ENDPOINT = 120,
    DEFAULT_ENDPOINT_WITH_CTT = 121
} EdgeOpcUaCommon;


/**
 * Edge_String
 *
 * A sequence of Unicode characters. Strings are just an array of UA_Byte. */
typedef struct {
    size_t length; /* The length of the string */
    uint8_t *data; /* The content (not null-terminated) */
} Edge_String;

/**
 * Edge_LocalizedText
 *
 * Human readable text with an optional locale identifier. */
typedef struct {
    Edge_String locale;
    Edge_String text;
} Edge_LocalizedText;

/**
 * Edge_SByte
 *
 * An integer value between -128 and 127. */
typedef int8_t Edge_SByte;

/**
 * Edge_Byte
 *
 * An integer value between 0 and 255. */
typedef uint8_t Edge_Byte;

/**
 * ByteString
 *
 * A sequence of octets. */
typedef Edge_String Edge_ByteString;

/**
 * Edge_XmlElement
 *
 * An XML element. */
typedef Edge_String Edge_XmlElement;

/**
 * Edge_DateTime
 *
 * An instance in time. A DateTime value is encoded as a 64-bit signed integer
 * which represents the number of 100 nanosecond intervals since January 1, 1601
 * (UTC). */
typedef int64_t Edge_DateTime;

/**
 * Guid
 *
 * A 16 byte value that can be used as a globally unique identifier. */
typedef struct {
    uint32_t data1;
    uint16_t data2;
    uint16_t data3;
    Edge_Byte   data4[8];
} Edge_Guid;

/**
 * QualifiedName
 *
 * A name qualified by a namespace. */
typedef struct {
    uint16_t namespaceIndex;
    Edge_String name;
} Edge_QualifiedName;

/**
 * Edge_NodeId
 *
 * A name qualified by a namespace. */
typedef struct {
    uint16_t namespaceIndex;
    EdgeNodeIdType identifierType;
    union {
        uint32_t     numeric;
        Edge_String     string;
        Edge_Guid       guid;
        Edge_ByteString byteString;
    } identifier;
} Edge_NodeId;

#endif /* EDGE_OPC_UA_COMMON_H_ */
