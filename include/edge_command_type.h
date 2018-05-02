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
 * @file edge_command_type.h
 * @brief This file contains various command types and text descriptions.
 */

#ifndef EDGE_COMMAND_TYPE_H_
#define EDGE_COMMAND_TYPE_H_

/**
 * Declares the commands(ie., services) supported by OPC UA wrapper.
 */
typedef enum
{
    /** Command to read one or more attributes of scalar and array types.*/
    CMD_READ = 0,

    /** Command to write one or more attributes of scalar and array types.*/
    CMD_WRITE = 1,

    /** Command to create, modify and delete subscriptions.*/
    CMD_SUB = 2,

    /** Command to start the OPC UA server which opens TCP socket and starts event loop.*/
    CMD_START_SERVER = 3,

    /** Command to start the OPC UA client which establishes TCP connection with OPC UA server.*/
    CMD_START_CLIENT = 4,

    /** Command to stop the OPC UA server which stops event loop and closes TCP socket.*/
    CMD_STOP_SERVER = 5,

    /** Command to stop the OPC UA client which terminates the TCP connection with OPC UA server.*/
    CMD_STOP_CLIENT = 6,

    /** Command to get all the endpoints supported by an OPC UA server.*/
    CMD_GET_ENDPOINTS = 7,

    /** Command to browse one or more nodes in server's address space.*/
    CMD_BROWSE = 8,

    /** Command to browse all view nodes in server's address space.*/
    CMD_BROWSE_VIEW = 9,

    /** Command to execute a method on server.*/
    CMD_METHOD = 10,

    /** Command to read sampling interval on server.*/
    CMD_READ_SAMPLING_INTERVAL = 11,

    /** Invalid command */
    CMD_INVALID = 100
} EdgeCommand;

/** Read - String value.*/
#define  CMD_READ_VALUE                   "read"

/** Read - Command description.*/
#define CMD_READ_DESC                    "read async"

/** Read - Minimum Sampling Interval*/
#define CMD_READ_SAMPLING_INTERVAL_ATTRIBUTE       "read interval"

/** Write - String value.*/
#define  CMD_WRITE_VALUE                    "write"

/** Write - Command description.*/
#define CMD_WRITE_DESC                    "write  async"

/** Subscription - String value.*/
#define  CMD_SUB_VALUE                    "sub"

/** Subscription - Command description.*/
#define CMD_SUB_DESC                   "subscription"

/** Start server - String value.*/
#define  CMD_START_SERVER_VALUE                    "start_server"

/** Start server - Command value.*/
#define CMD_START_SERVER_DESC                    "start server"

/** Start client - String value.*/
#define  CMD_START_CLIENT_VALUE                    "start_client"

/** Start client - Command description.*/
#define CMD_START_CLIENT_DESC                    "start client and connect to server"

/** Stop server - String value.*/
#define  CMD_STOP_SERVER_VALUE                    "stop_server"

/** Stop server - Command description.*/
#define CMD_STOP_SERVER_DESC                    "stop server"

/** Stop client - String value.*/
#define  CMD_STOP_CLIENT_VALUE                    "stop_client"

/** Stop client - Command description.*/
#define CMD_STOP_CLIENT_DESC                    "stop client and disconnect"

/** Get endpoints - String value.*/
#define  CMD_GET_ENDPOINTS_VALUE                    "endpoint_discovery"

/** Get endpoints - Command description.*/
#define CMD_GET_ENDPOINTS_DESC                    "get end points from server"

/** Browse - String value.*/
#define  CMD_BROWSE_VALUE                    "browse"

/** Browse - Command description.*/
#define CMD_BROWSE_DESC                    "browse nodes from server"

/** Method - String value.*/
#define  CMD_METHOD_VALUE                    "method"

/** Method - Command description.*/
#define CMD_METHOD_DESC                    "call method nodes from server"

#endif /* EDGE_COMMAND_TYPE_H_ */
