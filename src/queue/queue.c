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

#include "queue.h"

#include <stdlib.h>
#include "edge_malloc.h"

bool isEmpty(Queue *queue)
{
    return (queue->size == 0);
}

bool isFull(Queue *queue)
{
    return (queue->size == queue->capacity);
}

Queue *createQueue(int capacity)
{
    Queue *queue = (Queue *) EdgeMalloc(sizeof(Queue));
    if (queue == NULL)
        return NULL;
    queue->size = 0;
    queue->capacity = capacity;
    queue->front = 0;
    queue->rear = queue->capacity - 1;
    queue->message = (EdgeMessage **) malloc(queue->capacity * sizeof(EdgeMessage *));
    return queue;
}

bool enqueue(Queue *queue, EdgeMessage *msg)
{
    if (isFull(queue))
        return false;

    queue->rear = (queue->rear + 1) % queue->capacity;
    queue->size += 1;
    queue->message[queue->rear] = msg;
    return true;
}

EdgeMessage *dequeue(Queue *queue)
{
    if (isEmpty(queue))
        return NULL;

    EdgeMessage *msg = queue->message[queue->front];
    queue->front = (queue->front + 1) % queue->capacity;
    queue->size -= 1;
    return msg;
}
