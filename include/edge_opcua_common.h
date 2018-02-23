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
