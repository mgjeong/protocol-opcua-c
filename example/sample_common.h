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

#ifndef EDGE_OPC_UA_SAMPLE_COMMON_H_
#define EDGE_OPC_UA_SAMPLE_COMMON_H_

const char WELL_KNOWN_DISCOVERY_VALUE[] = "/server/discovery";
const char WELL_KNOWN_DISCOVERY_DESC[] = "Well-known discovery uri";

const char WELL_KNOWN_GROUP_VALUE[] = "/server/group";
const char WELL_KNOWN_GROUP_DESC[] = "Well-known group uri";

const char WELL_KNOWN_LOCALHOST_ADDRESS_VALUE[] = "localhost";
const char WELL_KNOWN_LOCALHOST_ADDRESS_DESC[] = "";

const char WELL_KNOWN_SERVER_NODE_VALUE[] = "/server";
const char WELL_KNOWN_SERVER_NODE_DESC[] = "";

const char WELL_KNOWN_SERVER_NODE_CURRENTTIME_VALUE[] = "/server/currenttime";
const char WELL_KNOWN_SERVER_NODE_CURRENTTIME_DESC[] = "";

const char WELL_KNOWN_SERVER_NODE_STATUS_VALUE[] = "/server/status";
const char WELL_KNOWN_SERVER_NODE_STATUS_DESC[] = "";

const char WELL_KNOWN_SERVER_NODE_BUILD_INFO_VALUE[] = "/server/buildInfo";
const char WELL_KNOWN_SERVER_NODE_BUILD_INFO_DESC[] = "";

const char DEFAULT_SERVER_NAME_VALUE[] = "edge-opc-server";
const char DEFAULT_SERVER_NAME_DESC[] =  "default server name";

const char DEFAULT_SERVER_URI_VALUE[] = "opc.tcp://localhost:12686/";
const char DEFAULT_SERVER_URI_DESC[] = "default server address and port";

const char DEFAULT_SERVER_APP_NAME_VALUE[] = "digitalpetri opc-ua client";
const char DEFAULT_SERVER_APP_NAME_DESC[] = "";

const char DEFAULT_SERVER_APP_URI_VALUE[] = "urn:digitalpetri:opcua:client";
const char DEFAULT_SERVER_APP_URI_DESC[] = "";

const char DEFAULT_NAMESPACE_VALUE[] = "edge-namespace";
const char DEFAULT_NAMESPACE_DESC[] = "default display-name for namespace";

const char DEFAULT_ROOT_NODE_INFO_VALUE[] = "defaultRootNode";
const char DEFAULT_ROOT_NODE_INFO_DESC[] = "defalult root node information";

const char DEFAULT_PRODUCT_URI_VALUE[] = "urn:digitalpetri:opcua:sdk";
const char DEFAULT_PRODUCT_URI_DESC[] = "defalult product uri";


const char DEFAULT_ENDPOINT_VALUE[] = "opc.tcp://localhost:12686/edge-opc-server";
const char DEFAULT_ENDPOINT_DESC[] = "this default endpoint is for local test";

const char DEFAULT_ENDPOINT_WITH_CTT_VALUE[] = "opc.tcp://localhost:4842/";
const char DEFAULT_ENDPOINT_WITH_CTT_DESC[] = "this default endpoint is for CTT";

#endif /* EDGE_OPC_UA_SAMPLE_COMMON_H_ */
