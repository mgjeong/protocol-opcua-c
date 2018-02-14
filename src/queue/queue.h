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

#ifndef EDGE_QUEUE_H
#define EDGE_QUEUE_H

#include <stdbool.h>

#include "opcua_common.h"

#ifdef __cplusplus
extern "C"
{
#endif

    typedef struct Queue
    {
        int front;
        int rear;
        int size;
        int capacity;
        EdgeMessage **message;
    } Queue;

    bool isEmpty(Queue *queue);
    bool isFull(Queue *queue);
    Queue *createQueue(int size);
    bool enqueue(Queue *queue, EdgeMessage *msg);
    EdgeMessage *dequeue(Queue *queue);

#ifdef __cplusplus
}
#endif

#endif
