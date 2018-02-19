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
 * @file queue.h
 *
 * @brief This file contains the definition, types and APIs for queue handling
 */

#ifndef EDGE_QUEUE_H
#define EDGE_QUEUE_H

#include <stdbool.h>

#include "opcua_common.h"

#ifdef __cplusplus
extern "C"
{
#endif

/**
  * @brief Structure to store the elements in queue
  *
  */
typedef struct Queue
{
    /**< Front element index */
    int front;

    /**Rear element index */
    int rear;

    /**< Queue size */
    int size;

    /**< Queue capacity */
    int capacity;

    /**< EdgeMessage request data */
    EdgeMessage **message;
} Queue;

/**
 * @brief Check whether queue is empty
 * @param[in]  queue Queue to check
 * @return @c true on success, false on failure
 * @retval #true Successful (Queue is empty)
 * @retval #false Failure (Queue is not empty)
 */
bool isEmpty(Queue *queue);

/**
 * @brief Check whether queue is full
 * @param[in]  queue Queue to check
 * @return @c true on success, false on failure
 * @retval #true Successful (Queue is full)
 * @retval #false Failure (Queue is not full)
 */
bool isFull(Queue *queue);

/**
 * @brief Creates the queue
 * @param[in]  size size of the queue to be created
 * @return Queue on success, NULL in case of error
 */
Queue *createQueue(int size);

/**
 * @brief Inserts the edge message data to the queue
 * @param[in]  queue Queue to add
 * @param[in]  msg Edge Message data to be added
 * @return @c true on success, false on failure
 * @retval #true Successful
 * @retval #false Failure
 */
bool enqueue(Queue *queue, EdgeMessage *msg);

/**
 * @brief Dequeues the edge message data from the front index in the queue
 * @param[in]  queue Queue to remove
 * @return EdgeMessage on success, NULL in case of empty queue or failure
 */
EdgeMessage *dequeue(Queue *queue);

#ifdef __cplusplus
}
#endif

#endif
