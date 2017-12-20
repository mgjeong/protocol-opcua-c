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

#include "edge_utils.h"

#include <stdio.h>
#include <stdlib.h>

edgeMap* createMap()
{
    edgeMap* map = (edgeMap*) malloc(sizeof(edgeMap));
    map->head = NULL;
    return map;
}

void insertMapElement(edgeMap *map, keyValue key, keyValue value)
{
    edgeMapNode* node = (edgeMapNode*)malloc(sizeof(edgeMapNode));
    node->key = key;
    node->value = value;
    node->next = NULL;
        
    edgeMapNode *temp = map->head;
    
    if(temp == NULL)
    {
        map->head = node;
    }
    else
    {
        while(temp->next != NULL)
                temp = temp->next;

        temp->next = node;
    }
}

keyValue getMapElement(edgeMap *map, keyValue key)
{
    edgeMapNode *temp = map->head;

    while(temp != NULL)
    {
            if(temp->key == key)
            {
                    return temp->value;
            }
            temp = temp->next;
    }

return NULL;
}

void deleteMap(edgeMap *map)
{
    edgeMapNode *temp = map->head;
    edgeMapNode *xtemp;

    while(temp != NULL)
    {
            xtemp = temp->next;
            free(temp);
            temp = xtemp;
    }

    map->head = NULL;
}

// USAGE 

/*
    edgeMap* X = createMap();

    insert(X, 10, "arya");
    insert(X, 20, "mango");
    insert(X, 25, "apple");

    char* ret = (char *)get(X, 25);

    deleteMap(X);
*/
