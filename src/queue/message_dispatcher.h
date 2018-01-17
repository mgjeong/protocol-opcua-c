#ifndef EDGE_MESSAGE_DISPATCHER_H
#define EDGE_MESSAGE_DISPATCHER_H

#include "opcua_common.h"

#include <stdbool.h>

bool add_to_recvQ(EdgeMessage *msg);
bool add_to_sendQ(EdgeMessage *msg);
void start();
void terminate();

#endif  // EDGE_MESSAGE_DISPATCHER_H
