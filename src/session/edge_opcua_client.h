#ifndef EDGE_OPCUA_CLIENT_H
#define EDGE_OPCUA_CLIENT_H

#include <stdbool.h>

#include "opcua_common.h"

#ifdef __cplusplus
extern "C" {
#endif

bool connect_client(char* endpoint);
void disconnect_client(EdgeEndPointInfo* epInfo);
void* getEndpoints(char *endpointUri);
EdgeResult* readNodesFromServer(EdgeMessage* msg);
EdgeResult* writeNodesInServer(EdgeMessage* msg);

#ifdef __cplusplus
}
#endif


#endif
