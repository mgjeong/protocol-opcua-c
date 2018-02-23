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
#ifdef HAVE_STRING_H
#include <string.h>
#elif defined(HAVE_STRINGS_H)
#include <strings.h>
#endif

#include "edge_random.h"
#include "edge_utils.h"
#include "edge_logger.h"
#include <stdio.h>
#include <stdbool.h>
#include <assert.h>
#include <ctype.h>

#define TAG "edge_random"

bool EdgeGetRandomBytes(uint8_t * output, size_t len)
{
    if ( (output == NULL) || (len == 0) )
    {
        return false;
    }

#if defined(__unix__) || defined(__APPLE__)
    FILE* urandom = fopen("/dev/urandom", "r");
    if (urandom == NULL)
    {
        EDGE_LOG(TAG, "Failed open /dev/urandom!");
        assert(false);
        return false;
    }

    if (fread(output, sizeof(uint8_t), len, urandom) != len)
    {
        EDGE_LOG(TAG, "Failed while reading /dev/urandom!");
        assert(false);
        fclose(urandom);
        return false;
    }
    fclose(urandom);

#elif defined(_WIN32)
    /*
     * size_t may be 64 bits, but ULONG is always 32.
     * If len is larger than the maximum for ULONG, just fail.
     * It's unlikely anything ever will want to ask for this much randomness.
     */
    if (len > 0xFFFFFFFFULL)
    {
        EDGE_LOG(TAG, "Requested number of bytes too large for ULONG");
        assert(false);
        return false;
    }

    NTSTATUS status = BCryptGenRandom(NULL, output, (ULONG)len, BCRYPT_USE_SYSTEM_PREFERRED_RNG);
    if (!BCRYPT_SUCCESS(status))
    {
        EDGE_LOG_V(TAG, "BCryptGenRandom failed (%X)!", status);
        assert(false);
        return false;
    }

#elif defined(ARDUINO)
    if (!g_isSeeded)
    {
        OCSeedRandom();
    }

    size_t i;
    for (i = 0; i < len; i++)
    {
        output[i] = OC_arduino_random_function() & 0x00ff;
    }

#else
    #error Unrecognized platform
#endif

    return true;
}

uint32_t EdgeGetRandom()
{
    uint32_t result = 0;
    if (!EdgeGetRandomBytes((uint8_t*)&result, sizeof(result)))
    {
        EDGE_LOG(TAG, "EdgeGetRandom failed!");
        assert(false);
    }
    return result;
}
