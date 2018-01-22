#ifndef EDGE_NODE_H
#define EDGE_NODE_H

#include "opcua_common.h"

#include <open62541.h>

#ifdef __cplusplus
extern "C"
{
#endif

    EdgeResult addNodes(UA_Server *server, EdgeNodeItem *item);
    EdgeResult addMethodNode(UA_Server *server, EdgeNodeItem *item, EdgeMethod *method);
    EdgeResult addDataAccessNode(EdgeNodeItem *item);
    EdgeResult modifyNode(UA_Server *server, char *nodeUri, EdgeVersatility *value);
    EdgeResult modifyNode2(EdgeNodeIdentifier nodeType);
    EdgeResult addReferences(UA_Server *server, EdgeReference *reference);

#ifdef __cplusplus
}
#endif

#endif
