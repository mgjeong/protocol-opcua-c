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
 * @file edge_list.h
 * @brief This file contains APIs for generic single linked list.
 */

#ifndef EDGE_LIST_H_
#define EDGE_LIST_H_

#include <stdbool.h>

#ifdef __cplusplus
extern "C"
{
#endif

/**
 * @brief Structure for a generic single linked list.
 */
typedef struct List
{
    /** Generic data of list.*/
    void *data;

    /** Link to next generic data.*/
    struct List *link;
} List;

/**
 * @brief Adds an object into the single linked list.
 * @param[in]  head Address of the head pointer of the single linked list.
 * @param[in]  data Represents the object to be added.
 * @return @c true if object added, otherwise @c false
 */
bool addListNode(List **head, void *data);

/**
 * @brief Destroys the single linked list.
 * @param[in]  head Address of the head pointer of the single linked list.
 */
void deleteList(List **head);

/**
 * @brief Get the number of elements in the list.
 * @param[in]  ptr Head pointer of the single linked list.
 * @return size of the list
 */
unsigned int getListSize(List *ptr);

#ifdef __cplusplus
}
#endif

#endif /* EDGE_LIST_H_ */
