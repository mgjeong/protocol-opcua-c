#ifndef EDGE_SUBSCRIPTION_H
#define EDGE_SUBSCRIPTION_H

#include "opcua_common.h"
#include "open62541.h"

#ifdef __cplusplus
extern "C" {
#endif

EdgeResult executeSub(UA_Client *client, EdgeMessage *msg);
void sendPublishRequest(UA_Client *client);

#ifdef __cplusplus
}
#endif



#endif  // EDGE_SUBSCRIPTION_H
