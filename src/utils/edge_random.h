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
 * @file edge_random.h
 * @brief This file contains functions to generate random numbers.
 */

#ifndef EDGE_RANDOM_H
#define EDGE_RANDOM_H

#include <stdint.h>
#include <stdlib.h>
#include <stdbool.h>

#ifdef __cplusplus
extern "C" {
#endif

/**
 * @brief Generate a uniformly distributed 32-bit random number.
 * @return On success, it returns the random value, otherwise 0.
 */
uint32_t EdgeGetRandom();

#ifdef __cplusplus
}
#endif

#endif      // EDGE_RANDOM_H
