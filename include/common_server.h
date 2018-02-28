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
 * @file common_server.h
 *
 * @brief This file contains the definition, types and structure for server
 */

#ifndef EDGE_COMMON_SERVER_H_
#define EDGE_COMMON_SERVER_H_

#include "opcua_common.h"

#ifdef __cplusplus
extern "C"
{
#endif

typedef struct EdgeNodeId EdgeNodeId;

/**
  * @brief Structure which represents the Node information
  *
  */
typedef struct EdgeNodeItem
{
    /**< Browse name of Node.*/
    char *browseName;

    /**< Display name of Node */
    char *displayName;

    /** EdgeIdentifier type.*/
    EdgeIdentifier nodeType;

    /**< Access level. */
    int accessLevel;

    /**< User access level. */
    int userAccessLevel;

    /**< write mask.*/
    int writeMask;

    /**< write mask with user access rights.*/
    int userWriteMask;

    /**< forward reference.*/
    bool forward;

    /**< EdgeNodeIdentifier type for node data.*/
    int variableIdentifier;

    /**< Node data information. */
    void *variableData;

    /**< Array Length **/
    size_t arrayLength;

    /**< Source node information. */
    EdgeNodeId *sourceNodeId;

    /* Minimum Sampling interval*/
    double minimumSamplingInterval;
} EdgeNodeItem;

/**
  * @brief Structure which represents the references between Nodes
  *
  */
typedef struct EdgeReference
{
    /**< Source node browse path.*/
    char *sourcePath;

    /**< Source node Namespace Uri .*/
    char *sourceNamespace;

    /**< Target Node Browse path.*/
    char *targetPath;

    /**< Target node Namespace Uri.*/
    char *targetNamespace;

    /**< Reference type id.*/
    int referenceId;

    /**< forward reference.*/
    bool forward;
} EdgeReference;

/**
  * @brief Enum which represents the argument types
  *
  */
typedef enum EdgeArgumentType
{
    /**< In/Out Argument type. */
    IN_OUT_ARGUMENTS = 1,
    /**< Input argument type. */
    IN_ARGUMENT = 2,
    /**< Output Argument type. */
    OUT_ARGUMENT = 3,
    /**< No argument type. */
    VOID_ARGUMENT = 4
} EdgeArgumentType;

/**
  * @brief Enum which represents the value type of argument
  *
  */
typedef enum EdgeArgValType
{
    /**< Scalar value. */
    SCALAR = 0,
    /**< One Dimensional array value. */
    ARRAY_1D = 1,
    /**< Two Dimensional array value. Not supported yet */
    ARRAY_2D = 2
} EdgeArgValType;

/**
  * @brief Structure which represents the arguments information.
  *
  */
typedef struct EdgeArgument
{
    /**< Argument data type */
    int argType;

    /**< Argument value type */
    EdgeArgValType valType;

    /**< Argument scalar value data */
    void *scalarValue;

    /**< Number of elements if valType is array */
    size_t arrayLength;

    /**< Argument array value data */
    void *arrayData;
} EdgeArgument;

/**
  * @brief Method callback function.
  * @param[in]  inpSize Number of input arguments.
  * @param[in]  input Input data.
  * @param[in]  outSize Number of output arguments.
  * @param[out]  outout Output data.
  */
typedef void (*method_func) (int inpSize, void **input, int outSize, void **output);

/**
  * @brief Structure which represents the method request data.
  *
  */
typedef struct EdgeMethod
{
    /**< Method Node name to browse for */
    char *methodNodeName;

    /**< Method description */
    char *description;

    /**< Number of input arguments */
    size_t num_inpArgs;

    /**< Input arguments data */
    EdgeArgument **inpArg;

    /**< Number of output arguments */
    size_t num_outArgs;

    /**< Output arguments */
    EdgeArgument **outArg;

    /**< Method callback function */
    method_func method_fn;
} EdgeMethod;

#ifdef __cplusplus
}
#endif

#endif /* EDGE_COMMON_SERVER_H_ */
