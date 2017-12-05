#ifndef WRITE_H
#define WRITE_H

#include "opcua_common.h"
#include "open62541.h"

#ifdef __cplusplus
extern "C" {
#endif

EdgeResult *executeWrite(UA_Client *client, EdgeMessage *msg);

#ifdef __cplusplus
}
#endif

#endif  // WRITE_H
