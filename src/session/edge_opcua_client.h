#ifndef EDGE_OPCUA_CLIENT_H
#define EDGE_OPCUA_CLIENT_H

#include <stdbool.h>

#ifdef __cplusplus
extern "C" {
#endif

  bool connect_client(char* endpoint);
  void disconnect_client();

#ifdef __cplusplus
}
#endif


#endif
