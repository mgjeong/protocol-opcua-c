#ifndef EDGE_OPCUA_SERVER_H
#define EDGE_OPCUA_SERVER_H

#include "opcua_common.h"

#ifdef __cplusplus
extern "C" {
#endif

  /* Create and add namespace */
  void createNamespace(char* namespaceUri,
                       char* rootNodeIdentifier,
                       char* rootNodeBrowseName,
                       char* rootNodeDisplayName);

  /* Start the Server */
  EdgeResult* start_server(EdgeEndPointInfo* epInfo);

  /* Stop the server */
  void stop_server();

#ifdef __cplusplus
}
#endif

#endif
