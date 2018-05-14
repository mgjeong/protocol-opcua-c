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
 * @file subscription.h
 *
 * @brief This file contains the definition, types and APIs for SUBSCRIPTION command request.
 */

#ifndef EDGE_SUBSCRIPTION_H
#define EDGE_SUBSCRIPTION_H

#include "opcua_common.h"
#include "open62541.h"

#include "edge_utils.h"

#ifdef __cplusplus
extern "C"
{
#endif

/**
 * @brief Executes Subscription operation
 * @param[in]  client Client Handle.
 * @param[in]  msg EdgeMessage request data
 * @return @c EdgeResult code is 0 on success, otherwise an error value
 * @retval #STATUS_OK Successful
 * @retval #STATUS_PARAM_INVALID Invalid parameter
 * @retval #STATUS_ERROR Operation failed
 */
EdgeResult executeSub(UA_Client *client, const EdgeMessage *msg);

/**
 * @brief Acquires the lock on mutex for synchronizing requests.
 * @remarks After acquiring the lock, it's mandatory to release the mutex.
 * @param[in]  client Client Handle.
 * @return @c 0 on success.
 */
int acquireSubscriptionLockInternal(UA_Client *client);

/**
 * @brief Releases the lock on mutex used for synchronizing requests.
 * @remarks Mutex should have been acquired before the invocation of this function.
 * @param[in]  client Client Handle.
 * @return @c 0 on success.
 */
int releaseSubscriptionLockInternal(UA_Client *client);

/**
 * @brief Stops the running subscription thread for a particular server.
 * @remarks It is assumed that all subscriptions(subscriptionList in clientSubscription) are unsubscribed.
 * @param[in]  client Client Handle.
 */
void stopSubscriptionThread(UA_Client *client);

#ifdef __cplusplus
}
#endif

#endif  // EDGE_SUBSCRIPTION_H
