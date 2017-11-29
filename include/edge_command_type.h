
/**
 * @file
 *
 * This file contains the definition, types and APIs for resource= s be implemented.
 */

#ifndef EDGE_COMMAND_TYPE_H_
#define EDGE_COMMAND_TYPE_H_

typedef enum
{
    CMD_READ = 0,
    CMD_WRITE = 1,
    CMD_SUB = 2,
    CMD_START_SERVER = 3,
    CMD_START_CLIENT = 4,
    CMD_STOP_SERVER = 5,
    CMD_STOP_CLIENT = 6,
    CMD_GET_ENDPOINTS = 7,
    CMD_BROWSE = 8,
    CMD_METHOD = 9
} EdgeCommand;

#define  CMD_READ_VALUE                   "read"
#define CMD_READ_DESC                    "read async"

#define  CMD_WRITE_VALUE                    "write"
#define CMD_WRITE_DESC                    "write  async"

#define  CMD_SUB_VALUE                    "sub"
#define CMD_SUB_DESC                   "subscription"

#define  CMD_START_SERVER_VALUE                    "start_server"
#define CMD_START_SERVER_DESC                    "start server"

#define  CMD_START_CLIENT_VALUE                    "start_client"
#define CMD_START_CLIENT_DESC                    "start client and connect to server"

#define  CMD_STOP_SERVER_VALUE                    "stop_server"
#define CMD_STOP_SERVER_DESC                    "stop server"

#define  CMD_STOP_CLIENT_VALUE                    "stop_client"
#define CMD_STOP_CLIENT_DESC                    "stop client and disconnect"

#define  CMD_GET_ENDPOINTS_VALUE                    "endpoint_discovery"
#define CMD_GET_ENDPOINTS_DESC                    "get end points from server"

#define  CMD_BROWSE_VALUE                    "browse"
#define CMD_BROWSE_DESC                    "browse nodes from server"

#define  CMD_METHOD_VALUE                    "method"
#define CMD_METHOD_DESC                    "call method nodes from server"

#endif /* EDGE_COMMAND_TYPE_H_ */
