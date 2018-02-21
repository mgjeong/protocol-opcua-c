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
 * @file edge_logger.h
 * @brief This file contains logger related macro definitions.
 */

#ifndef EDGE_LOGGER_H_
#define EDGE_LOGGER_H_

#include <stdio.h>

#ifdef __cplusplus
extern "C"
{
#endif

#if DEBUG

/** Log the given tag and a string parameter.*/
#define EDGE_LOG(tag, param) printf("[%s] %s\n", tag, param)

/** Log the given tag and a variable number of arguments.*/
#define EDGE_LOG_V(tag, param, ...) fprintf(stdout, param, __VA_ARGS__)
#else

/** Log nothing.*/
#define EDGE_LOG(tag, param)

/** Log nothing.*/
#define EDGE_LOG_V(tag, param, ...)
#endif

#ifdef __cplusplus
}
#endif

#endif /* EDGE_LOGGER_H_ */
