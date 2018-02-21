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
 * @file edge_identifier.h
 *
 * @brief This file contains the definition, types and APIs for resource= s be implemented.
 */

#ifndef EDGE_IDENTIFIER_H_
#define EDGE_IDENTIFIER_H_

#ifdef __cplusplus
extern "C"
{
#endif

typedef enum
{
    READ_WRITE = 0,
    READ = 1,
    WRITE = 2,
    HISTORY_READ = 4,
    HISTORY_WRITE = 8,
    SEMANTIC_CHANGE = 10,
    STATUS_WRITE = 20,
    TIMESTAMP_WRITE = 40,

    CREATE_SUB = 100,
    MODIFY_SUB = 101,

    VARIABLE_NODE = 1000,
    ARRAY_NODE = 1001,
    OBJECT_NODE = 1002,
    VARIABLE_TYPE_NODE = 1003,
    OBJECT_TYPE_NODE = 1004,
    REFERENCE_TYPE_NODE = 1005,
    DATA_TYPE_NODE = 1006,
    VIEW_NODE = 1007,
    SINGLE_FOLDER_NODE_TYPE = 1008,
    MILTI_FOLDER_NODE_TYPE = 1009
} EdgeIdentifier;

#ifdef __cplusplus
}
#endif

#endif /* EDGE_IDENTIFIER_H_ */
