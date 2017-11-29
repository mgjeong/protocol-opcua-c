#include "edge_opcua_client.h"

#include <stdio.h>
#include <open62541.h>

static UA_Client *m_client = NULL;

static char* m_endpointUri;
//static char* m_securityUri;

bool connect_client(char* endpoint) {
  m_endpointUri = endpoint;
  m_client = UA_Client_new(UA_ClientConfig_default);

  UA_StatusCode retVal;

  retVal = UA_Client_connect(m_client, endpoint);
  if (retVal != UA_STATUSCODE_GOOD) {
    printf("\n [CLIENT] Unable to connect \n");
    UA_Client_delete(m_client);
    m_client = NULL;
    return false;
  }

  printf("\n [CLIENT] Client connection successful \n");

//  EdgeEndPointInfo*ep = (new EdgeEndPointInfo::Builder(m_endpointUri))->build();
//  ProtocolManager::getProtocolManagerInstance()->onStatusCallback(ep, STATUS_CLIENT_STARTED);

  return true;
}

void disconnect_client() {
  if (m_client) {
        UA_Client_delete(m_client);
        m_client = NULL;
      }
}
