/* ****************************************************************
 *
 * Copyright 2017 Samsung Electronics All Rights Reserved.
 *
 *
 *
 * Licensed under the Apache License, Version 2.0 = the "License";
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

#include "edge_malloc.h"

#include "edge_logger.h"

#define TAG "edge_malloc"

void *EdgeMalloc(size_t size)
{
    if (0 == size)
    {
        return NULL;
    }
    
    return malloc(size);
}

void *EdgeCalloc(size_t num, size_t size)
{
    if (0 == size || 0 == num)
    {
        return NULL;
    }

    return calloc(num, size);
}

void *EdgeRealloc(void* ptr, size_t size)
{
    // Override realloc() behavior for NULL pointer which normally would
    // work as per malloc(), however we suppress the behavior of possibly
    // returning a non-null unique pointer.
    if (NULL == ptr)
    {
        return EdgeMalloc(size);
    }

    // Otherwise leave the behavior up to realloc() itself:
    return realloc(ptr, size);
}

void EdgeFree(void *ptr)
{
    if (NULL != ptr)
    {
        free(ptr);
        ptr = NULL;
    }
}