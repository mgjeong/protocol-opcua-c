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

#include "edge_list.h"
#include "edge_malloc.h"
#include "edge_utils.h"

#define TAG "edge_list"

/**
 * Function to create a node in the linked list.
 * 'data' parameter is a generic pointer to the actual data.
 * Created node will hold this generic pointer.
 */
static List *createListNode(void *data)
{
    List *node = (List *) EdgeCalloc(1, sizeof(List));
    VERIFY_NON_NULL_MSG(node, "EdgeCalloc failed for create list node\n", NULL);
    node->data = data;
    return node;
}

bool addListNode(List **head, void *data)
{
    VERIFY_NON_NULL_MSG(head, "HEAD NULL in add list node\n", false);
    VERIFY_NON_NULL_MSG(data, "DATA NULL in add list node\n", false);

    List *newnode = createListNode(data);
    VERIFY_NON_NULL_MSG(newnode, "FAILED to create new node in create list node\n", false);

    newnode->link = *head;
    *head = newnode;
    return true;
}

unsigned int getListSize(List *ptr)
{
    VERIFY_NON_NULL_MSG(ptr, "NULL list ptr in getListSize\n", 0);

    size_t size = 0;
    while (ptr)
    {
        size++;
        ptr = ptr->link;
    }
    return size;
}

void deleteList(List **head)
{
    VERIFY_NON_NULL_NR_MSG(head, "NULL head param in delete LIST\n");

    List *next = NULL;
    List *ptr = *head;
    while (ptr)
    {
        next = ptr->link;
        EdgeFree(ptr);
        ptr = next;
    }
    *head = NULL;
}
