#ifndef BROWSE_H
#define BROWSE_H

#include "opcua_common.h"
#include "open62541.h"

#ifdef __cplusplus
extern "C"
{
#endif

    EdgeResult executeBrowse(UA_Client *client, EdgeMessage *msg, bool browseNext);

#ifdef __cplusplus
}
#endif

#endif
