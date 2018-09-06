//******************************************************************
//
// Copyright 2015 Intel Mobile Communications GmbH All Rights Reserved.
//
//-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
//
// Licensed under the Apache License, Version 2.0 (the "License");
// you may not use this file except in compliance with the License.
// You may obtain a copy of the License at
//
//      http://www.apache.org/licenses/LICENSE-2.0
//
// Unless required by applicable law or agreed to in writing, software
// distributed under the License is distributed on an "AS IS" BASIS,
// WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
// See the License for the specific language governing permissions and
// limitations under the License.
//
//-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
//
//
//*********************************************************************

/**
 * @file
 * This file provides APIs related to mutex and semaphores.
 */

// Defining _POSIX_C_SOURCE macro with 199309L (or greater) as value
// causes header files to expose definitions
// corresponding to the POSIX.1b, Real-time extensions
// (IEEE Std 1003.1b-1993) specification
//
// For this specific file, see use of clock_gettime and PTHREAD_MUTEX_DEFAULT
#ifndef _POSIX_C_SOURCE
#define _POSIX_C_SOURCE 200809L
#endif

#include "octhread.h"
#include <string.h>
#include <stdio.h>
#include <errno.h>
#include <assert.h>
#ifndef _WIN32
#include <pthread.h>
#include <unistd.h>
#include <sys/time.h>
#else
#include "pthread.h"
#include <winsock2.h>
#endif
#include "edge_logger.h"
#include "edge_malloc.h"
#include "cacommon.h"

/**
 * TAG
 * Logging tag for module name
 */
#define TAG "UMUTEX"

static const uint64_t USECS_PER_SEC         = 1000000;
static const uint64_t NANOSECS_PER_USECS    = 1000;
static const uint64_t NANOSECS_PER_SEC      = 1000000000L;

typedef struct _tagMutexInfo_t
{
    pthread_mutex_t mutex;
} oc_mutex_internal;

typedef struct _tagEventInfo_t
{
    pthread_cond_t cond;
    pthread_condattr_t condattr;
} oc_cond_internal;

typedef struct _tagThreadInfo_t
{
    pthread_t thread;
    pthread_attr_t  threadattr;
} oc_thread_internal;

OCThreadResult_t oc_thread_new(oc_thread *t, void *(*start_routine)(void *), void *arg)
{
    OCThreadResult_t res = OC_THREAD_SUCCESS;
    oc_thread_internal *threadInfo = (oc_thread_internal*)EdgeMalloc(sizeof(oc_thread_internal));
    if (NULL != threadInfo)
    {
        int result = pthread_create(&threadInfo->thread, NULL, start_routine, arg);
        if (result != 0)
        {
            res = OC_THREAD_CREATE_FAILURE;
            *t = NULL;
            EdgeFree(threadInfo);
            EDGE_LOG_V(TAG, "%s: pthread_create failed", __func__);
        }
        else
        {
            *t = (oc_thread)threadInfo;
        }
    }
    else
    {
        EDGE_LOG_V(TAG, "%s Failed to allocate thread!", __func__);
        *t = NULL;
        res = OC_THREAD_ALLOCATION_FAILURE;
    }
    return res;
}

OCThreadResult_t oc_thread_free(oc_thread t)
{
    OCThreadResult_t res = OC_THREAD_SUCCESS;
    oc_thread_internal *threadInfo = (oc_thread_internal*) t;
    if (threadInfo)
    {
        EdgeFree(threadInfo);
    }
    else
    {
        EDGE_LOG_V(TAG, "%s Invalid thread !", __func__);
        res = OC_THREAD_INVALID;
    }
    return res;
}

OCThreadResult_t oc_thread_wait(oc_thread t)
{
    OCThreadResult_t res = OC_THREAD_SUCCESS;
    oc_thread_internal *threadInfo = (oc_thread_internal*) t;
    int joinres = pthread_join(threadInfo->thread, NULL);
    if (0 != joinres)
    {
        EDGE_LOG_V(TAG, "Failed to join thread with error %d", joinres);
        res = OC_THREAD_WAIT_FAILURE;
    }

    return res;
}

oc_mutex oc_mutex_new(void)
{
    oc_mutex retVal = NULL;
    oc_mutex_internal *mutexInfo = (oc_mutex_internal*) EdgeMalloc(sizeof(oc_mutex_internal));
    if (NULL != mutexInfo)
    {
        // create the mutex with the attributes set
        int ret=pthread_mutex_init(&(mutexInfo->mutex), PTHREAD_MUTEX_DEFAULT);
        if (0 == ret)
        {
            retVal = (oc_mutex) mutexInfo;
        }
        else
        {
            EDGE_LOG_V(TAG, "%s Failed to initialize mutex !", __func__);
            EdgeFree(mutexInfo);
        }
    }
    else
    {
        EDGE_LOG_V(TAG, "%s Failed to allocate mutex!", __func__);
    }

    return retVal;
}

bool oc_mutex_free(oc_mutex mutex)
{
    bool bRet=false;
    oc_mutex_internal *mutexInfo = (oc_mutex_internal*) mutex;
    if (mutexInfo)
    {
        int ret = pthread_mutex_destroy(&mutexInfo->mutex);
        if (0 == ret)
        {
            EdgeFree(mutexInfo);
            bRet=true;
        }
        else
        {
            EDGE_LOG_V(TAG, "%s Failed to free mutex !", __func__);
        }
    }
    else
    {
        EDGE_LOG_V(TAG, "%s Invalid mutex !", __func__);
    }

    return bRet;
}

void oc_mutex_lock(oc_mutex mutex)
{
    oc_mutex_internal *mutexInfo = (oc_mutex_internal*) mutex;
    if (mutexInfo)
    {
        int ret = pthread_mutex_lock(&mutexInfo->mutex);
        if(ret != 0)
        {
            EDGE_LOG_V(TAG, "Pthread Mutex lock failed: %d", ret);
            exit(ret);
        }
    }
    else
    {
        EDGE_LOG_V(TAG, "%s Invalid mutex !", __func__);
    }
}

void oc_mutex_unlock(oc_mutex mutex)
{
    oc_mutex_internal *mutexInfo = (oc_mutex_internal*) mutex;
    if (mutexInfo)
    {
        int ret = pthread_mutex_unlock(&mutexInfo->mutex);
        if (ret != 0)
        {
            EDGE_LOG_V(TAG, "Pthread Mutex unlock failed: %d", ret);
            exit(ret);
        }
    }
    else
    {
        EDGE_LOG_V(TAG, "%s: Invalid mutex !", __func__);
    }
}

oc_cond oc_cond_new(void)
{
    oc_cond retVal = NULL;
    oc_cond_internal *eventInfo = (oc_cond_internal*) EdgeMalloc(sizeof(oc_cond_internal));
    if (NULL != eventInfo)
    {
        int ret = pthread_condattr_init(&(eventInfo->condattr));
        if(0 != ret)
        {
            EDGE_LOG_V(TAG, "%s: Failed to initialize condition variable attribute %d!",
                    __func__, ret);
            EdgeFree(eventInfo);
            return retVal;
        }

#if defined(__ANDROID__) || _POSIX_TIMERS > 0
        {
            ret = pthread_condattr_setclock(&(eventInfo->condattr), CLOCK_MONOTONIC);
            if(0 != ret)
            {
                EDGE_LOG_V(TAG, "%s: Failed to set condition variable clock %d!",
                        __func__, ret);
                pthread_condattr_destroy(&(eventInfo->condattr));
                EdgeFree(eventInfo);
                return retVal;
            }
        }
#endif /* defined(__ANDROID__) || _POSIX_TIMERS > 0 */
        ret = pthread_cond_init(&(eventInfo->cond), &(eventInfo->condattr));
        if (0 == ret)
        {
            retVal = (oc_cond) eventInfo;
        }
        else
        {
            EDGE_LOG_V(TAG, "%s: Failed to initialize condition variable %d!", __func__, ret);
            pthread_condattr_destroy(&(eventInfo->condattr));
            EdgeFree(eventInfo);
        }
    }
    else
    {
        EDGE_LOG_V(TAG, "%s: Failed to allocate condition variable!", __func__);
    }

    return retVal;
}

void oc_cond_free(oc_cond cond)
{
    oc_cond_internal *eventInfo = (oc_cond_internal*) cond;
    if (eventInfo != NULL)
    {
        int ret = pthread_cond_destroy(&(eventInfo->cond));
        int ret2 = pthread_condattr_destroy(&(eventInfo->condattr));
        if (0 == ret && 0 == ret2)
        {
            EdgeFree(cond);
        }
        else
        {
            EDGE_LOG_V(TAG, "%s: Failed to destroy condition variable %d, %d",
                    __func__, ret, ret2);
        }
    }
    else
    {
        EDGE_LOG_V(TAG, "%s: Invalid parameter", __func__);
    }
}

void oc_cond_signal(oc_cond cond)
{
    oc_cond_internal *eventInfo = (oc_cond_internal*) cond;
    if (eventInfo != NULL)
    {
        int ret = pthread_cond_signal(&(eventInfo->cond));
        if (0 != ret)
        {
            EDGE_LOG_V(TAG, "%s: Failed to signal condition variable", __func__);
        }
    }
    else
    {
        EDGE_LOG_V(TAG, "%s: Invalid parameter", __func__);
    }
}

void oc_cond_broadcast(oc_cond cond)
{
    oc_cond_internal* eventInfo = (oc_cond_internal*) cond;
    if (eventInfo != NULL)
    {
        int ret = pthread_cond_broadcast(&(eventInfo->cond));
        if (0 != ret)
        {
            EDGE_LOG_V(TAG, "%s: failed to signal condition variable", __func__);
        }
    }
    else
    {
        EDGE_LOG_V(TAG, "%s: Invalid parameter", __func__);
    }
}

void oc_cond_wait(oc_cond cond, oc_mutex mutex)
{
    oc_cond_wait_for(cond, mutex, 0L);
}

#ifndef TIMEVAL_TO_TIMESPEC
#define TIMEVAL_TO_TIMESPEC(tv, ts) {               \
    (ts)->tv_sec = (tv)->tv_sec;                    \
    (ts)->tv_nsec = (tv)->tv_usec * 1000;           \
}
#endif

struct timespec oc_get_current_time()
{
#if defined(__ANDROID__) || _POSIX_TIMERS > 0
    struct timespec ts;
    clock_gettime(CLOCK_MONOTONIC, &ts);
    return ts;
#else
    struct timeval tv;
    #ifndef _WIN32
        gettimeofday(&tv, NULL);
    #else
        getTimeofDay(&tv, NULL);		
    #endif
    struct timespec ts;
    TIMEVAL_TO_TIMESPEC(&tv, &ts);
    return ts;
#endif
}

void oc_add_microseconds_to_timespec(struct timespec* ts, uint64_t microseconds)
{
    time_t secPart = microseconds/USECS_PER_SEC;
    uint64_t nsecPart = (microseconds % USECS_PER_SEC) * NANOSECS_PER_USECS;
    uint64_t totalNs = ts->tv_nsec + nsecPart;
    time_t secOfNs = totalNs/NANOSECS_PER_SEC;

    ts->tv_nsec = (totalNs)% NANOSECS_PER_SEC;
    ts->tv_sec += secPart + secOfNs;
}

OCWaitResult_t oc_cond_wait_for(oc_cond cond, oc_mutex mutex, uint64_t microseconds)
{
    OCWaitResult_t retVal = OC_WAIT_INVAL;

    oc_cond_internal *eventInfo = (oc_cond_internal*) cond;
    oc_mutex_internal *mutexInfo = (oc_mutex_internal*) mutex;

    if (NULL == mutexInfo)
    {
        EDGE_LOG_V(TAG, "%s: Invalid mutex", __func__);
        return OC_WAIT_INVAL;
    }

    if (NULL == eventInfo)
    {
        EDGE_LOG_V(TAG, "%s: Invalid condition", __func__);
        return OC_WAIT_INVAL;
    }

    if (microseconds > 0)
    {
        int ret = 0;
        struct timespec abstime = { .tv_sec = 0 };
        {
             abstime = oc_get_current_time();
            oc_add_microseconds_to_timespec(&abstime, microseconds);

            //Wait for the given time
            ret = pthread_cond_timedwait(&(eventInfo->cond), &(mutexInfo->mutex), &abstime);
        }

        switch (ret)
        {
            case 0:
                // Success
                retVal = OC_WAIT_SUCCESS;
                break;
            case ETIMEDOUT:
                retVal = OC_WAIT_TIMEDOUT;
                break;
            case EINVAL:
                EDGE_LOG_V(TAG, "%s: condition, mutex, or abstime is Invalid", __func__);
                retVal = OC_WAIT_INVAL;
                break;
            default:
                EDGE_LOG_V(TAG, "%s: pthread_cond_timedwait returned %d", __func__, retVal);
                retVal = OC_WAIT_INVAL;
                break;
        }
    }
    else
    {
        // Wait forever
        int ret = pthread_cond_wait(&eventInfo->cond, &mutexInfo->mutex);
        retVal = ret == 0 ? OC_WAIT_SUCCESS : OC_WAIT_INVAL;
    }
    return retVal;
}

