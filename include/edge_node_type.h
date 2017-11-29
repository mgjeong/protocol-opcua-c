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
 * This file contains the definition, types and APIs for resource= s be implemented.
 */

#ifndef EDGE_NODE_TYPE_H_
#define EDGE_NODE_TYPE_H_

#ifdef __cplusplus
extern "C"
{
#endif

typedef enum
{
    STRING = 1,
    INTEGER = 2,
    UUID = 3,
    OP = 4
} EdgeNodeTypeCommon;

//  typedef enum
//  {
//      Edge_Node_Classical_DataAccess_Type = 1,
//      Edge_Node_ServerInfo_Type = 2,
//      Edge_Node_Custom_Type = 3,
//      Edge_Node_Method_Type = 4
//  } EdgeNodeTypeServer;

#ifdef __cplusplus
}
#endif

#endif /* EDGE_NODE_TYPE_H_ */
