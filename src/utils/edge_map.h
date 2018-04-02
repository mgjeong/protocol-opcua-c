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
 * @file edge_map.h
 * @brief This file contains APIs for generic key-value pairs.
 */

#ifndef EDGE_MAP_H_
#define EDGE_MAP_H_

#ifdef __cplusplus
extern "C"
{
#endif

/** Generic pointer to represent key/value.*/
typedef void *keyValue;

/**
 * @brief Structure for a node in the list of generic key-value pairs.
 */
typedef struct edgeMapNode
{
    /** Map Key.*/
    keyValue key;

    /** map key-value pair.*/
    keyValue value;

    /** Next node in list.*/
    struct edgeMapNode *next;
} edgeMapNode;

/**
 * @brief Structure which holds the head pointer of a generic key-value pairs list.
 * @remarks This structure is used to hold head pointer of a list.
 */
typedef struct edgeMap
{
    /** Map Head.*/
    edgeMapNode *head;
} edgeMap;

/**
 * @brief API for creating a map for storing edge nodes.
 * @remarks This API will allocate memory required.
 * @return a pointer to the created map, otherwise a null pointer if the memory is insufficient.
 */
edgeMap *createMap();

/**
 * @brief Insert a key-value pair into the map.
 * @param[in]  map Pointer to an edgeMap created using createMap().
 * @param[in]  key Generic key.
 * @param[in]  value Generic value.
 */
void insertMapElement(edgeMap *map, keyValue key, keyValue value);

/**
 * @brief Get the element value of the given key from the map.
 * @param[in]  map Pointer to an edgeMap created using createMap().
 * @param[in]  key Generic key.
 * @return Value of given key on success, otherwise null.
 */
keyValue getMapElement(edgeMap *map, keyValue key);

/**
 * @brief Delete and free memory used by the edge util map.
 * @param[in]  map Pointer to an edgeMap created using createMap().
 */
void deleteMap(edgeMap *map);

#ifdef __cplusplus
}
#endif

#endif /* EDGE_MAP_H_ */
