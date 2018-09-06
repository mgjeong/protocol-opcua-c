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

#ifdef HAVE_FCNTL_H
#include <fcntl.h>
#endif
#ifdef HAVE_UNISTD_H
#include <unistd.h>
#endif
#ifdef HAVE_STDLIB_H
#include <stdlib.h>
#endif

#include <stdio.h>
#include <stdint.h>
#include "edge_random.h"
#include "edge_utils.h"
#include "edge_logger.h"

#define TAG "edge_random"

uint32_t EdgeGetRandom()
{
    uint32_t result = 0;
	int len = sizeof(result);
#ifndef _WIN32    
    FILE* urandom = fopen("/dev/urandom", "r");
    VERIFY_NON_NULL_MSG(urandom, "Failed to open /dev/urandom", 0);

    // Reads 4 bytes of data from "/dev/urandom".
    if (fread((uint8_t*)&result, sizeof(uint8_t), len, urandom) != len)
    {
        // Number of bytes read is not 4. There is an error and hence 0 will be returned.
        EDGE_LOG(TAG, "Failed while reading /dev/urandom. EdgeGetRandom Failed.");
        result = 0;
    }
    fclose(urandom);
#else
    //NTSTATUS status = BCryptGenRandom(NULL, (uint8_t*)&result, (ULONG) len, BCRYPT_USE_SYSTEM_PREFERRED_RNG);
    //if (!BCRYPT_SUCCESS(status))
    //{
    //    EDGE_LOG_V(TAG, "BCryptGenRandom failed (%X)!", status);        
    //}
#endif
    return result;
}
