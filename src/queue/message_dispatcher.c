#include "message_dispatcher.h"
#include "queue.h"

#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>

static pthread_t m_sendQ_Thread;
static pthread_t m_recvQ_Thread;
static bool b_sendQ_Thread_Running = false;
static bool b_recvQ_Thread_Running = false;

static Queue *recvQueue = NULL;
static Queue *sendQueue = NULL;

static int queue_capacity = 1000;

static void handleMessage(EdgeMessage *data);

void terminate()
{
    b_sendQ_Thread_Running = false;
    pthread_join(m_sendQ_Thread, NULL);

    b_recvQ_Thread_Running = false;
    pthread_join(m_recvQ_Thread, NULL);

    while (!isEmpty(sendQueue))
    {
        EdgeMessage *data = dequeue(sendQueue);
        if (data)
        {
            free(data);
            data = NULL;
        }
    }

    while (!isEmpty(recvQueue))
    {
        EdgeMessage *data = dequeue(recvQueue);
        if (data)
        {
            free(data);
            data = NULL;
        }
    }
}

void start()
{
//  if (sendQ)
}

static void *sendQ_run(void *ptr)
{
    EdgeMessage *data;
    b_sendQ_Thread_Running = true;

    while (b_sendQ_Thread_Running)
    {
        if (!isEmpty(sendQueue))
        {
            data = front(sendQueue); // retrieve the front element from queue
            handleMessage(data); // process the queue message
            dequeue(sendQueue); // remove the element from queue
        }
    }
    return NULL;
}

static void *recvQ_run(void *ptr)
{
    EdgeMessage *data;
    b_recvQ_Thread_Running = true;

    while (b_recvQ_Thread_Running)
    {
        if (!isEmpty(recvQueue))
        {
            data = front(recvQueue); // retrieve the front element from queue
            handleMessage(data); // process the queue message
            dequeue(recvQueue); // remove the element from queue
        }
    }
    return NULL;
}

bool add_to_sendQ(EdgeMessage *msg)
{
    if (NULL == sendQueue)
    {
        sendQueue = createQueue(queue_capacity);
        pthread_create(&m_sendQ_Thread, NULL, &sendQ_run, NULL);
    }
    bool ret = enqueue(sendQueue, msg);
    return ret;
}

bool add_to_recvQ(EdgeMessage *msg)
{
    if (NULL == recvQueue)
    {
        recvQueue = createQueue(queue_capacity);
        pthread_create(&m_recvQ_Thread, NULL, &recvQ_run, NULL);
    }
    bool ret = enqueue(recvQueue, msg);
    return ret;
}

static void handleMessage(EdgeMessage *data)
{

    if (SEND_REQUEST == data->type || SEND_REQUESTS == data->type)
    {
        onSendMessage(data);
    }
    else if (GENERAL_RESPONSE == data->type || BROWSE_RESPONSE == data->type)
    {
//         ProtocolManager* receiver = ProtocolManager::getProtocolManagerInstance();
//         receiver->onResponseMessage(data);
    }
    else if (REPORT == data->type)
    {
//        ProtocolManager* receiver = ProtocolManager::getProtocolManagerInstance();
//        receiver->onMonitoredMessage(data);;
    }
    else if (ERROR == data->type)
    {
//        ProtocolManager* receiver = ProtocolManager::getProtocolManagerInstance();
//        receiver->onErrorCallback(data);
    }
}
