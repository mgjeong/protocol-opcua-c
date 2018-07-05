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
 * @file edge_node_identifier.h
 * @brief This file contains various identifiers of standard-defined nodes in namespace zero.
 */

#ifndef EDGE_NODE_IDENTIFIER_H_
#define EDGE_NODE_IDENTIFIER_H_

#ifdef __cplusplus
extern "C"
{
#endif

/**
 * Numeric identifiers of standard-defined nodes in namespace zero.
 */
typedef enum
{
    Edge_Node_Class_Type = 10000,
    Edge_Node_ServerInfo_Type = 10001,
    Edge_Node_Custom_Type = 10002,

    Edge_Create_Sub = 10032,
    Edge_Modify_Sub = 10033,
    Edge_Delete_Sub = 10034,
    Edge_Republish_Sub = 10035,
    #ifdef ENABLE_SUB_QUEUE
    Edge_Publish_Sub = 10036,
    #endif

    Edge_Connection_Status = 10040,
    Edge_Endpoints = 10050
} EdgeNodeType;

#ifdef __cplusplus
}
#endif

#endif /* EDGE_NODE_IDENTIFIER_H_ */
