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
#include <string.h>
#include <math.h>

#include "edge_malloc.h"

#define CHAR_SIZE   128

#define COLOR_GREEN        "\x1b[32m"
#define COLOR_PURPLE      "\x1b[35m"
#define COLOR_RESET         "\x1b[0m"

////////////////////////////////////////////////////////////////////////////////////////////////////////////////
//                                                                  METHOD CALLBACK FUNCTIONS                                                 //
////////////////////////////////////////////////////////////////////////////////////////////////////////////////


/**
 *  No Argument method
 **/
void test_method_shutdown(int inpSize, void **input, int outSize, void **output)
{
    printf(COLOR_GREEN "\n[shutdown() method called]\n" COLOR_RESET);
}

void test_method_print(int inpSize, void **input, int outSize, void **output)
{
    int *inp = (int*) input[0];
    printf(COLOR_GREEN "\n[print() method called]" COLOR_RESET);
    printf(COLOR_PURPLE " %d\n" COLOR_RESET, *inp );
}

void test_method_version(int inpSize, void **input, int outSize, void **output)
{
    char *version = (char*) EdgeMalloc(sizeof(char) * CHAR_SIZE);
    strncpy(version, "09131759", CHAR_SIZE);
    output[0] = version;
    printf(COLOR_GREEN "\n[version() method called] :: %s\n" COLOR_RESET, version);
}

void test_method_sqrt(int inpSize, void **input, int outSize, void **output)
{
    double *inp = (double *) input[0];
    double *sq_root = (double *) EdgeMalloc(sizeof(double));
    if (sq_root == NULL)
    {
        printf("allocation has failed");
        return;
    }

    *sq_root = sqrt(*inp);
    output[0] = (void *) sq_root;
    printf(COLOR_GREEN "\n[sqrt(%.2f) method called] :: %.2f\n" COLOR_RESET, *inp, *sq_root);
}

void test_method_increment_int32Array(int inpSize, void **input, int outSize, void **output)
{
    int32_t *inputArray = (int32_t *) input[0];
    int *delta = (int *) input[1];

    printf(COLOR_GREEN "\n[incrementInt32Array({%d %d %d %d %d}, %d) method called] :: ",
            inputArray[0], inputArray[1], inputArray[2], inputArray[3], inputArray[4], *delta);

    int32_t *outputArray = (int32_t *) EdgeMalloc(sizeof(int32_t) * 5);
    if (outputArray == NULL)
    {
        printf("allocation has failed");
        return;
    }

    for (int i = 0; i < 5; i++)
    {
        outputArray[i] = inputArray[i] + *delta;
        printf("%d ", outputArray[i]);
    }

    printf("\n"COLOR_RESET);
    output[0] = (void *) outputArray;
}
