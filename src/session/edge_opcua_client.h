#ifndef EDGE_OPCUA_CLIENT_H
#define EDGE_OPCUA_CLIENT_H

#include <stdbool.h>

#include "opcua_common.h"

#ifdef __cplusplus
extern "C" {
#endif

bool connect_client(char* endpoint);
void disconnect_client(EdgeEndPointInfo* epInfo);
void* getClientEndpoints(char *endpointUri);
EdgeResult* readNodesFromServer(EdgeMessage* msg);
EdgeResult* writeNodesInServer(EdgeMessage* msg);
EdgeResult* browseNodesInServer(EdgeMessage *msg);
EdgeResult* callMethodInServer(EdgeMessage *msg);

#ifdef __cplusplus
}
#endif


#endif
