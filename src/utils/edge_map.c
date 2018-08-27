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

#include "edge_map.h"
#include "edge_malloc.h"
#include "edge_utils.h"

#define TAG "edge_map"

// USAGE

/*
 edgeMap* X = createMap();

 insertMapElement(X, 10, "arya");
 insertMapElement(X, 20, "mango");
 insertMapElement(X, 25, "apple");

 char* ret = (char *)getMapElement(X, 25);

 deleteMap(X);
 */

edgeMap *createMap()
{
    edgeMap *map = (edgeMap *) EdgeMalloc(sizeof(edgeMap));
    VERIFY_NON_NULL_MSG(map, "EdgeMalloc FAILED for create edge map\n", NULL);
    map->head = NULL;
    return map;
}

void insertMapElement(edgeMap *map, keyValue key, keyValue value)
{
    edgeMapNode *node = (edgeMapNode *) EdgeMalloc(sizeof(edgeMapNode));
    VERIFY_NON_NULL_NR_MSG(node, "EdgeMalloc failed for insert map element\n");
    node->key = key;
    node->value = value;
    node->next = NULL;

    edgeMapNode *temp = map->head;

    if (temp == NULL)
    {
        // Adding first node in the map.
        map->head = node;
    }
    else
    {
        // Iterate till the end of the map and append.
        while (temp->next != NULL)
            temp = temp->next;

        temp->next = node;
    }
}

keyValue getMapElement(edgeMap *map, keyValue key)
{
    edgeMapNode *temp = map->head;

    while (temp != NULL)
    {
        COND_CHECK((temp->key == key), temp->value);
        temp = temp->next;
    }

    return NULL;
}

void deleteMap(edgeMap *map)
{
    edgeMapNode *temp = map->head;
    edgeMapNode *xtemp;

    while (temp != NULL)
    {
        xtemp = temp->next;
        EdgeFree(temp);
        temp = xtemp;
    }

    map->head = NULL;
}
