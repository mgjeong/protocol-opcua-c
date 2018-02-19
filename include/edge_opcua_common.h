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

#define DEFAULT_TYPE (1)
#define URI_TYPE (2)
#define SYSTEM_NAMESPACE_INDEX (0)
#define DEFAULT_NAMESPACE_INDEX (2)
#define DEFAULT_REQUEST_ID (10000)
#define MAX_BROWSEREQUEST_SIZE (10)

#define WELL_KNOWN_DISCOVERY_VALUE                        "/server/discovery"
#define WELL_KNOWN_DISCOVERY_DESC                        "Well-known discovery uri"

#define WELL_KNOWN_GROUP_VALUE                                "/server/group"
#define WELL_KNOWN_GROUP_DESC                                "Well-known group uri"

#define WELL_KNOWN_LOCALHOST_URI_VALUE                 "opc.tcp://localhost"
#define WELL_KNOWN_LOCALHOST_URI_DESC                 "this local endpoint"

#define WELL_KNOWN_LOCALHOST_ADDRESS_VALUE            "localhost"
#define WELL_KNOWN_LOCALHOST_ADDRESS_DESC            ""

#define WELL_KNOWN_SERVER_NODE_VALUE                                "/server"
#define WELL_KNOWN_SERVER_NODE_DESC                                ""

#define WELL_KNOWN_SERVER_NODE_CURRENTTIME_VALUE          "/server/currenttime"
#define WELL_KNOWN_SERVER_NODE_CURRENTTIME_DESC          ""

#define WELL_KNOWN_SERVER_NODE_STATUS_VALUE                    "/server/status"
#define WELL_KNOWN_SERVER_NODE_STATUS_DESC                    ""

#define WELL_KNOWN_SERVER_NODE_BUILD_INFO_VALUE             "/server/buildInfo"
#define WELL_KNOWN_SERVER_NODE_BUILD_INFO_DESC             ""

#define DEFAULT_SERVER_NAME_VALUE                                "edge-opc-server"
#define DEFAULT_SERVER_NAME_DESC                                "default server name"

#define DEFAULT_SERVER_URI_VALUE                                   "opc.tcp://localhost:12686/"
#define DEFAULT_SERVER_URI_DESC                                   "default server address and port"

#define DEFAULT_SERVER_APP_NAME_VALUE                                "digitalpetri opc-ua client"
#define DEFAULT_SERVER_APP_NAME_DESC                                ""

#define DEFAULT_SERVER_APP_URI_VALUE                                "urn:digitalpetri:opcua:client"
#define DEFAULT_SERVER_APP_URI_DESC                                ""

#define DEFAULT_NAMESPACE_VALUE                     "edge-namespace"
#define DEFAULT_NAMESPACE_DESC                     "default display-name for namespace"

#define DEFAULT_ROOT_NODE_INFO_VALUE            "defaultRootNode"
#define DEFAULT_ROOT_NODE_INFO_DESC            "defalult root node information"

#define DEFAULT_PRODUCT_URI_VALUE            "urn:digitalpetri:opcua:sdk"
#define DEFAULT_PRODUCT_URI_DESC            "defalult product uri"


#define DEFAULT_ENDPOINT_VALUE                        "opc.tcp://localhost:12686/edge-opc-server"
#define DEFAULT_ENDPOINT_DESC                        "this default endpoint is for local test"

#define DEFAULT_ENDPOINT_WITH_CTT_VALUE            "opc.tcp://localhost:4842/"
#define DEFAULT_ENDPOINT_WITH_CTT_DESC            "this default endpoint is for CTT"

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

#endif /* EDGE_OPC_UA_COMMON_H_ */
