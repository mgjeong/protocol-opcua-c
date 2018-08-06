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

#include <stdio.h>
#include <stdlib.h>
#ifndef _WIN32
#include <pthread.h>
#include <unistd.h>
#else
#include "pthread.h"
#endif

#include "uqueue.h"
#include "cacommon.h"
#include "cathreadpool.h" /* for thread pool */
#include "caqueueingthread.h"
#include "message_dispatcher.h"
#include "edge_utils.h"
#include "edge_malloc.h"
#include "edge_logger.h"

#define SINGLE_HANDLE
#define MAX_THREAD_POOL_SIZE    20

#define TAG "message_handler"

// thread pool handle
static ca_thread_pool_t g_threadPoolHandle = NULL;

// message handler main thread
static CAQueueingThread_t g_sendThread;
static CAQueueingThread_t g_receiveThread;

static response_cb_t g_responseCallback = NULL;
static send_cb_t g_sendCallback = NULL;

static pthread_mutex_t g_queueingThreadMutex = PTHREAD_MUTEX_INITIALIZER;
static bool g_queueingThreadInitialized = false;

static void handleMessage(EdgeMessage *data);
static void destroyData(void *data, uint32_t size);

void delete_queue()
{
    int ret = pthread_mutex_lock(&g_queueingThreadMutex);
    if(ret != 0)
    {
        EDGE_LOG_V(TAG, "Failed to lock the queueing thread mutex. "
            "pthread_mutex_lock() returned (%d)\n.", ret);
        exit(ret);
    }

    // stop thread
    // delete thread data
    if (NULL != g_sendThread.threadMutex)
    {
        CAQueueingThreadStop(&g_sendThread);
    }

    // stop thread
    // delete thread data
    if (NULL != g_receiveThread.threadMutex)
    {
        CAQueueingThreadStop(&g_receiveThread);
    }

    // destroy thread pool
    if (NULL != g_threadPoolHandle)
    {
        ca_thread_pool_free(g_threadPoolHandle);
        g_threadPoolHandle = NULL;
    }

    CAQueueingThreadDestroy(&g_sendThread);
    CAQueueingThreadDestroy(&g_receiveThread);

    g_queueingThreadInitialized = false;

    ret = pthread_mutex_unlock(&g_queueingThreadMutex);
    if(ret != 0)
    {
        EDGE_LOG_V(TAG, "Failed to unlock the queueing thread mutex. "
            "pthread_mutex_unlock() returned (%d)\n.", ret);
        exit(ret);
    }
}

static void sendQ_run(void *ptr)
{
    EdgeMessage *data = (EdgeMessage *) ptr;
    handleMessage(data);
}

static void recvQ_run(void *ptr)
{
    EdgeMessage *data = (EdgeMessage *) ptr;
    handleMessage(data);
}

bool add_to_sendQ(EdgeMessage *msg)
{
    CAQueueingThreadAddData(&g_sendThread, msg, sizeof(EdgeMessage));
    return true;
}

bool add_to_recvQ(EdgeMessage *msg)
{
    CAQueueingThreadAddData(&g_receiveThread, msg, sizeof(EdgeMessage));
    return true;
}

static void handleMessage(EdgeMessage *data)
{
    if (SEND_REQUEST == data->type || SEND_REQUESTS == data->type)
    {
        // Invoke callback to send request.
        g_sendCallback(data);
    }
    else if (GENERAL_RESPONSE == data->type || BROWSE_RESPONSE == data->type || REPORT == data->type
            || ERROR_RESPONSE == data->type)
    {
        // Invoke callback to handle response.
        g_responseCallback(data);
    }
}

void init_queue()
{
    int ret = pthread_mutex_lock(&g_queueingThreadMutex);
    if(ret != 0)
    {
        EDGE_LOG_V(TAG, "Failed to lock the queueing thread mutex. "
            "pthread_mutex_lock() returned (%d)\n.", ret);
        exit(ret);
    }

    if(g_queueingThreadInitialized)
    {
        EDGE_LOG(TAG, "Queuing thread initialized already.");
        goto EXIT;
    }

    CAResult_t res = ca_thread_pool_init(MAX_THREAD_POOL_SIZE, &g_threadPoolHandle);
    if (CA_STATUS_OK != res)
    {
        EDGE_LOG(TAG, "thread pool initialize error.");
        goto EXIT;
    }

    // send thread initialize
    res = CAQueueingThreadInitialize(&g_sendThread, g_threadPoolHandle, sendQ_run, destroyData);
    if (CA_STATUS_OK != res)
    {
        EDGE_LOG(TAG, "Failed to Initialize send queue thread");
        goto EXIT;
    }

    res = CAQueueingThreadStart(&g_sendThread);
    if (CA_STATUS_OK != res)
    {
        EDGE_LOG(TAG, "thread start error(send thread).");
        goto EXIT;
    }

    // receive thread initialize
    res = CAQueueingThreadInitialize(&g_receiveThread, g_threadPoolHandle, recvQ_run, destroyData);
    if (CA_STATUS_OK != res)
    {
        EDGE_LOG(TAG, "Failed to Initialize receive queue thread");
        goto EXIT;
    }

    res = CAQueueingThreadStart(&g_receiveThread);
    if (CA_STATUS_OK != res)
    {
        EDGE_LOG(TAG, "thread start error(receive thread).");
        goto EXIT;
    }

    g_queueingThreadInitialized = true;

EXIT:
    ret = pthread_mutex_unlock(&g_queueingThreadMutex);
    if(ret != 0)
    {
        EDGE_LOG_V(TAG, "Failed to unlock the queueing thread mutex. "
            "pthread_mutex_unlock() returned (%d)\n.", ret);
        exit(ret);
    }
}

void registerMQCallback(response_cb_t resCallback, send_cb_t sendCallback)
{
    g_responseCallback = resCallback;
    g_sendCallback = sendCallback;
}

static void destroyData(void *data, uint32_t size)
{
    EDGE_LOG(TAG, "destroyData IN");
    if ((size_t) size < sizeof(EdgeMessage))
    {
        EDGE_LOG_V(TAG, "Destroy data too small %p %d", data, size);
    }

    EdgeMessage *msg = (EdgeMessage *) data;
    VERIFY_NON_NULL_NR_MSG(msg, "msg is NULL.");
    freeEdgeMessage(msg);
    EDGE_LOG(TAG, "destroyData OUT");
}
