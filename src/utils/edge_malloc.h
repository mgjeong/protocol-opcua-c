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
 * @file
 *
 * This file contains the utilities APIs for use in OPCUA module be implemented.
 */

#ifndef EDGE_MALLOC_H_
#define EDGE_MALLOC_H_

#ifdef __cplusplus
extern "C"
{
#endif

#define FREE(arg) if(arg) {free(arg); arg=NULL; }

#define IS_NULL(arg) ((arg == NULL) ? true : false)
#define IS_NOT_NULL(arg) ((arg != NULL) ? true : false)

#define VERIFY_NON_NULL(arg, retVal) { if (!(arg)) { EDGE_LOG(TAG, \
             #arg " is NULL"); return (retVal); } }
#define VERIFY_NON_NULL_NR(arg) { if (!(arg)) { EDGE_LOG(TAG, \
             #arg " is NULL"); return; } }

#ifdef __cplusplus
}
#endif

#endif /* EDGE_UTILS_H_ */