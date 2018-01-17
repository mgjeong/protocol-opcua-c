#ifndef EDGE_OPCUA_SERVER_H
#define EDGE_OPCUA_SERVER_H

#include "opcua_common.h"

#ifdef __cplusplus
extern "C" {
#endif

/* Create and add namespace */
void createNamespaceInServer(char *namespaceUri,
                             char *rootNodeIdentifier,
                             char *rootNodeBrowseName,
                             char *rootNodeDisplayName);

EdgeResult addNodesInServer(EdgeNodeItem *item);
EdgeResult modifyNodeInServer(char *nodeUri, EdgeVersatility *value);
EdgeResult addReferenceInServer(EdgeReference *reference);
EdgeResult addMethodNodeInServer(EdgeNodeItem *item, EdgeMethod *method);

/* Start the Server */
EdgeResult start_server(EdgeEndPointInfo *epInfo);

/* Stop the server */
void stop_server(EdgeEndPointInfo *epInfo);

#ifdef __cplusplus
}
#endif

#endif
