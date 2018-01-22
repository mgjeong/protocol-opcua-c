#include "queue.h"

#include <stdlib.h>

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
    Queue *queue = (Queue *) malloc(sizeof(Queue));
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

EdgeMessage *front(Queue *queue)
{
    if (isEmpty(queue))
        return NULL;
    return queue->message[queue->front];
}

EdgeMessage *rear(Queue *queue)
{
    if (isEmpty(queue))
        return NULL;
    return queue->message[queue->rear];
}
