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

#ifndef CA_EDGE_COMMON_H_
#define CA_EDGE_COMMON_H_

#include <stdint.h>
#include <stdlib.h>
#include <stdbool.h>

#ifdef __cplusplus
extern "C"
{
#endif // __cplusplus

    typedef enum
    {
        CA_STATUS_OK = 0,
        CA_STATUS_INVALID_PARAM,
        CA_MEMORY_ALLOC_FAILED,
        CA_STATUS_FAILED,

    } CAResult_t;

#ifdef __cplusplus
} /* extern "C" */
#endif /* __cplusplus */

#endif /* CA_EDGE_COMMON_H_ */
