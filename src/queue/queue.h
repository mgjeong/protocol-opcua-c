#ifndef QUEUE_H
#define QUEUE_H

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
    EdgeMessage *front(Queue *queue);
    EdgeMessage *rear(Queue *queue);

#ifdef __cplusplus
}
#endif

#endif
