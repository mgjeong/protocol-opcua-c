/* ****************************************************************
 *
 * Copyright 2014 Samsung Electronics All Rights Reserved.
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
 * @file
 *
 * This file provides APIs related to thread pool.
 */

#ifndef _GNU_SOURCE
#define _GNU_SOURCE
#endif
#include <errno.h>
#include "cathreadpool.h"
#include "uarraylist.h"
#include "octhread.h"

#include "edge_random.h"
#include "edge_malloc.h"
#include "edge_logger.h"

#define TAG "UTHREADPOOL"

/**
 * empty struct to represent the details.  This implementation has no data
 * that it needs to keep track of, so it only uses NULL for the internal value.
 */
typedef struct ca_thread_pool_details_t
{
    u_arraylist_t* threads_list;
    oc_mutex list_lock;
} ca_thread_pool_details_t;

/**
 * struct to wrap the pthreads callback properly.  The function pointer for
 * pthreads requires a void* return value, however u_thread_func is a void.
 */
typedef struct ca_thread_pool_callback_info_t
{
    ca_thread_func func;
    void* data;
} ca_thread_pool_callback_info_t;

typedef struct ca_thread_pool_thread_info_t
{
    oc_thread thread;
    uint32_t taskId;
} ca_thread_pool_thread_info_t;

// passthrough function to convert the pthreads call to a u_thread_func call
void* ca_thread_pool_pthreads_delegate(void* data)
{
    ca_thread_pool_callback_info_t* info = (ca_thread_pool_callback_info_t*)data;
    info->func(info->data);
    EdgeFree(info);
    return NULL;
}

// this implementation doesn't do a thread pool, so this function is essentially
// a no-op besides creating a valid ca_thread_pool_t object.  It was determined after
// reading through the existing implementation that the thread-pooling was unnecessary
// for the posix platforms.  Behavior shouldn't be changed since previously num_of_threads
// was greater than the number of requested threads.
CAResult_t ca_thread_pool_init(int32_t num_of_threads, ca_thread_pool_t *thread_pool)
{
    EDGE_LOG(TAG, "IN");

    if(!thread_pool)
    {
        EDGE_LOG(TAG, "Parameter thread_pool was null!");
        return CA_STATUS_INVALID_PARAM;
    }

    if(num_of_threads <= 0)
    {
        EDGE_LOG(TAG, "num_of_threads must be positive and non-zero");
        return CA_STATUS_INVALID_PARAM;
    }

    *thread_pool = EdgeMalloc(sizeof(struct ca_thread_pool));

    if(!*thread_pool)
    {
        EDGE_LOG(TAG, "Failed to allocate for thread-pool");
        return CA_MEMORY_ALLOC_FAILED;
    }

    (*thread_pool)->details = EdgeMalloc(sizeof(struct ca_thread_pool_details_t));
    if(!(*thread_pool)->details)
    {
        EDGE_LOG(TAG, "Failed to allocate for thread-pool details");
        EdgeFree(*thread_pool);
        *thread_pool=NULL;
        return CA_MEMORY_ALLOC_FAILED;
    }

    (*thread_pool)->details->list_lock = oc_mutex_new();

    if(!(*thread_pool)->details->list_lock)
    {
        EDGE_LOG(TAG, "Failed to create thread-pool mutex");
        goto exit;
    }

    (*thread_pool)->details->threads_list = u_arraylist_create();

    if(!(*thread_pool)->details->threads_list)
    {
        EDGE_LOG(TAG, "Failed to create thread-pool list");
        if(!oc_mutex_free((*thread_pool)->details->list_lock))
        {
            EDGE_LOG(TAG, "Failed to free thread-pool mutex");
        }
        goto exit;
    }

    EDGE_LOG(TAG, "OUT");
    return CA_STATUS_OK;

exit:
    EdgeFree((*thread_pool)->details);
    EdgeFree(*thread_pool);
    *thread_pool = NULL;
    return CA_STATUS_FAILED;
}

CAResult_t ca_thread_pool_add_task(ca_thread_pool_t thread_pool, ca_thread_func method, void *data,
                                   uint32_t *taskId)
{
    EDGE_LOG_V(TAG, "In %s", __func__);

    if(NULL == thread_pool || NULL == method)
    {
        EDGE_LOG(TAG, "thread_pool or method was NULL");
        return CA_STATUS_INVALID_PARAM;
    }

    ca_thread_pool_callback_info_t* info = EdgeMalloc(sizeof(ca_thread_pool_callback_info_t));
    if(!info)
    {
        EDGE_LOG(TAG, "Failed to allocate for memory wrapper");
        return CA_MEMORY_ALLOC_FAILED;
    }

    info->func = method;
    info->data = data;

    ca_thread_pool_thread_info_t *threadInfo =
            (ca_thread_pool_thread_info_t *) EdgeCalloc(1, sizeof(ca_thread_pool_thread_info_t));
    if (!threadInfo)
    {
        EDGE_LOG(TAG, "Memory allocation failed");
        EdgeFree(info);
        return CA_STATUS_FAILED;
    }
    threadInfo->taskId = EdgeGetRandom();
    if (taskId)
    {
        *taskId = threadInfo->taskId;
    }

    oc_mutex_lock(thread_pool->details->list_lock);
    bool addResult = u_arraylist_add(thread_pool->details->threads_list, (void*) threadInfo);
    if (!addResult)
    {
        // Note that this is considered non-fatal.
        oc_mutex_unlock(thread_pool->details->list_lock);
        EDGE_LOG(TAG, "Arraylist add failed");
        EdgeFree(info);
        EdgeFree(threadInfo);
        return CA_STATUS_FAILED;
    }

    int thrRet = oc_thread_new(&threadInfo->thread, ca_thread_pool_pthreads_delegate, info);
    if (thrRet != 0)
    {
        uint32_t index = 0;
        if (u_arraylist_get_index(thread_pool->details->threads_list, threadInfo, &index))
        {
            u_arraylist_remove(thread_pool->details->threads_list, index);
        }
        oc_mutex_unlock(thread_pool->details->list_lock);
        EDGE_LOG_V(TAG, "Thread start failed with error %d", thrRet);
        EdgeFree(info);
        return CA_STATUS_FAILED;
    }
    EDGE_LOG_V(TAG, "created taskId: %u", threadInfo->taskId);
    oc_mutex_unlock(thread_pool->details->list_lock);

    EDGE_LOG_V(TAG, "Out %s", __func__);
    return CA_STATUS_OK;
}

CAResult_t ca_thread_pool_remove_task(ca_thread_pool_t thread_pool, uint32_t taskId)
{
    EDGE_LOG_V(TAG, "In %s", __func__);

    if (!thread_pool)
    {
        EDGE_LOG(TAG, "Invalid parameter thread_pool was NULL");
        return CA_STATUS_FAILED;
    }

    oc_mutex_lock(thread_pool->details->list_lock);
    for (uint32_t i = 0; i < u_arraylist_length(thread_pool->details->threads_list); ++i)
    {
        ca_thread_pool_thread_info_t *threadInfo = (ca_thread_pool_thread_info_t *)
                u_arraylist_get(thread_pool->details->threads_list, i);
        if (threadInfo)
        {
            if (threadInfo->taskId == taskId)
            {
                EDGE_LOG_V(TAG, "waiting.. taskId: %u", threadInfo->taskId);
                oc_thread_wait(threadInfo->thread);

                EDGE_LOG_V(TAG, "removed taskId: %u", threadInfo->taskId);
                u_arraylist_remove(thread_pool->details->threads_list, i);
                oc_thread_free(threadInfo->thread);
                EdgeFree(threadInfo);
                break;
            }
        }
    }
    oc_mutex_unlock(thread_pool->details->list_lock);

    EDGE_LOG_V(TAG, "Out %s", __func__);
    return CA_STATUS_OK;
}

void ca_thread_pool_free(ca_thread_pool_t thread_pool)
{
    EDGE_LOG_V(TAG, "In %s", __func__);

    if (!thread_pool)
    {
        EDGE_LOG(TAG, "Invalid parameter thread_pool was NULL");
        return;
    }

    oc_mutex_lock(thread_pool->details->list_lock);

    for (uint32_t i = 0; i < u_arraylist_length(thread_pool->details->threads_list); ++i)
    {
        ca_thread_pool_thread_info_t *threadInfo = (ca_thread_pool_thread_info_t *)
                u_arraylist_get(thread_pool->details->threads_list, i);
        if (threadInfo)
        {
            if (threadInfo->thread)
            {
                EDGE_LOG_V(TAG, "waiting.. thread: %p", threadInfo->thread);
                oc_thread_wait(threadInfo->thread);
                oc_thread_free(threadInfo->thread);
            }
            EdgeFree(threadInfo);
        }
    }

    u_arraylist_free(&(thread_pool->details->threads_list));

    oc_mutex_unlock(thread_pool->details->list_lock);
    oc_mutex_free(thread_pool->details->list_lock);

    EdgeFree(thread_pool->details);
    EdgeFree(thread_pool);

    EDGE_LOG_V(TAG, "Out %s", __func__);
}
