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

#ifndef EDGE_COMMON_SERVER_H_
#define EDGE_COMMON_SERVER_H_

#include "opcua_common.h"

#ifdef __cplusplus
extern "C"
{
#endif

typedef struct EdgeNodeId EdgeNodeId;

typedef struct EdgeNodeItem
{
    /** browseName.*/
    char *browseName;

    /** EdgeIdentifier.*/
    EdgeIdentifier nodeType;

    /** EdgeNodeIdentifier.*/
    EdgeNodeIdentifier daNodeIdentifier;

    /** accessLevel.*/
    int accessLevel;

    /** userAccessLevel.*/
    int userAccessLevel;

    /** writeMask.*/
    int writeMask;

    /** userWriteMask.*/
    int userWriteMask;

    /** forward.*/
    bool forward;

    /** variableItemSet.*/
    //Object[][] variableItemSet;

    /** variableItemName.*/
    char *variableItemName;

    /** EdgeNodeIdentifier.*/
    EdgeNodeIdentifier variableIdentifier;

    /** variableData.*/
    void *variableData;

    /** array length **/
    int arrayLength;

    /** EdgeNodeId.*/
    EdgeNodeId *sourceNodeId;
} EdgeNodeItem;


typedef struct EdgeReference
{
    /** sourcePath.*/
    char *sourcePath;

    /** sourceNamespace.*/
    char *sourceNamespace;

    /** targetPath.*/
    char *targetPath;

    /** targetNamespace.*/
    char *targetNamespace;

    /** EdgeNodeIdentifier.*/
    EdgeNodeIdentifier referenceId;

    /** forward.*/
    bool forward;
} EdgeReference;

typedef enum EdgeArgumentType
{
    IN_OUT_ARGUMENTS = 1,
    IN_ARGUMENT = 2,
    OUT_ARGUMENT = 3,
    VOID_ARGUMENT = 4
} EdgeArgumentType;

typedef enum EdgeArgValType
{
    SCALAR = 0,
    ARRAY_1D = 1,
    ARRAY_2D = 2
} EdgeArgValType;

typedef struct EdgeArgument
{
    /** Argument data type */
    EdgeNodeIdentifier argType;

    /** Argument value type */
    EdgeArgValType valType;

    /** Argument scalar data */
    void *scalarValue;

    /* Number of elements if array */
    int arrayLength;

    /** Argument array data */
    void *arrayData;
} EdgeArgument;

typedef void (*method_func) (int inpSize, void **input, int outSize, void **output);

typedef struct EdgeMethod
{
    /** Method name to browse for */
    char *methodNodeName;

    /** Method description */
    char *description;

    /** number of input arguments */
    int num_inpArgs;

    /** Input arguments */
    EdgeArgument **inpArg;

    /** number of output arguments */
    int num_outArgs;

    /** Input arguments */
    EdgeArgument **outArg;

    /** Method */
    method_func method_fn;
} EdgeMethod;

#ifdef __cplusplus
}
#endif

#endif /* EDGE_COMMON_SERVER_H_ */
