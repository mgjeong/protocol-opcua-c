#ifndef READ_H
#define READ_H

#include "opcua_common.h"
#include "open62541.h"

#ifdef __cplusplus
extern "C" {
#endif

EdgeResult executeRead(UA_Client *client, EdgeMessage *msg);

#ifdef __cplusplus
}
#endif

#endif  // READ_H
