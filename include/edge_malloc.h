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
 * @file edge_malloc.h
 *
 * @brief This file contains the utilities APIs for use in OPCUA module be implemented.
 */

#ifndef EDGE_MALLOC_H_
#define EDGE_MALLOC_H_

#include <malloc.h>
#include <edge_opcua_common.h>

#ifdef __cplusplus
extern "C"
{
#endif

#ifndef _WIN32
#define EXPORT __attribute__((visibility("default")))
#else
#define EXPORT
#endif

/**
 * Allocates a block of size bytes, returning a pointer to the beginning of
 * the allocated block.
 *
 * NOTE: This function is intended to be used internally by the TB Stack.
 *       It is not intended to be used by applications.
 *
 * @param size - Size of the memory block in bytes, where size > 0
 *
 * @return
 *     on success, a pointer to the allocated memory block
 *     on failure, a null pointer is returned
 */
EXPORT void *EdgeMalloc(size_t size);

/**
 * Re-allocates a block of memory, pointed to by ptr to the size specified
 * in size.  The returned value contains a pointer to the new location, with
 * all data copied into it.  If the new size of the memory-object require movement,
 * the previous space is freed.  If the new size is larger, the newly allocated
 * area has non-deterministic content. If the space cannot be allocated, the value
 * ptr is left unchanged.
 *
 * @param ptr - Pointer to a block of memory previously allocated by OICCalloc,
 *              OICMalloc, or a previous call to this function.  If this value is
 *              NULL, this function will work identically to a call to OICMalloc.
 *
 * @param size - Size of the new memory block in bytes, where size > 0
 *
 * @return
 *      on success, a pointer to the newly sized memory block
 *      on failure, a null pointer is returned, and the memory pointed to by *ptr is untouched
 */
EXPORT void *EdgeRealloc(void *ptr, size_t size);

/**
 * Allocates a block of memory for an array of num elements, each of them
 * size bytes long and initializes all its bits to zero.
 *
 * @param num - The number of elements
 * @param size - Size of the element type in bytes, where size > 0
 *
 * @return
 *     on success, a pointer to the allocated memory block
 *     on failure, a null pointer is returned
 */
EXPORT void *EdgeCalloc(size_t num, size_t size);

/**
 * Deallocate a block of memory previously allocated by a call to OICMalloc.
 *
 * NOTE: This function is intended to be used internally by the TB Stack.
 *       It is not intended to be used by applications.
 *
 * @param ptr - Pointer to block of memory previously allocated by OICMalloc.
 *              If ptr is a null pointer, the function does nothing.
 */
EXPORT void EdgeFree(void *ptr);

EXPORT Edge_String EdgeStringAlloc(char const src[]);

#ifdef __cplusplus
}
#endif

#endif /* EDGE_UTILS_H_ */
