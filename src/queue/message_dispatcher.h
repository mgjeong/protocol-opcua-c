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
 * @file message_dispatcher.h
 *
 * @brief This file contains the definition, types and APIs for Message handler
 */

#ifndef EDGE_MESSAGE_DISPATCHER_H
#define EDGE_MESSAGE_DISPATCHER_H

#include "opcua_common.h"
#include "command_adapter.h"

#include <stdbool.h>

/**
 * @brief Add the EdgeMessage data to receiver Queue to send it to application
 * @param[in]  msg EdgeMessage data
 * @return @c true on success, false on failure
 * @retval #true Successful (Queue is empty)
 * @retval #false Failure (Queue is not empty)
 */
bool add_to_recvQ(EdgeMessage *msg);

/**
 * @brief Add the EdgeMessage data to send Queue to send it to server for processing
 * @param[in]  msg EdgeMessage data
 * @return @c true on success, false on failure
 * @retval #true Successful (Queue is empty)
 * @retval #false Failure (Queue is not empty)
 */
bool add_to_sendQ(EdgeMessage *msg);

/**
 * @brief Deletes and destroys the send and receiver queue.
 */
void delete_queue();

/**
 * @brief Initializes the send and receiver queue.
 * @remarks This request will be ignored if initialization is completed already.
 */
void init_queue();

/**
 * @brief Registers the callback for response and message handling
 * @param[in]  resCallback Callback for handling response message
 * @param[in]  sendCallback Callback for handling send request
 */
void registerMQCallback(response_cb_t resCallback, send_cb_t sendCallback);

#endif  // EDGE_MESSAGE_DISPATCHER_H
