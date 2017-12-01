#include "edge_opcua_client.h"
#include "read.h"
#include "write.h"

#include <stdio.h>
#include <open62541.h>

static UA_Client *m_client = NULL;

static char* m_endpointUri;
//static char* m_securityUri;

EdgeResult* readNodesFromServer(EdgeMessage* msg) {
  EdgeResult* result = executeRead(m_client, msg);
  return result;
}

EdgeResult* writeNodesInServer(EdgeMessage* msg) {
  EdgeResult* result = executeWrite(m_client, msg);
  return result;
}

bool connect_client(char* endpoint) {
  m_endpointUri = endpoint;

  UA_ClientConfig config = UA_ClientConfig_default;

  m_client = UA_Client_new(config);

  UA_StatusCode retVal;

  printf("endpoint :: %s\n", endpoint);
  retVal = UA_Client_connect(m_client, endpoint);
  if (retVal != UA_STATUSCODE_GOOD) {
    printf("\n [CLIENT] Unable to connect 0x%08x!\n", retVal);
    UA_Client_delete(m_client);
    m_client = NULL;
    return false;
  }

  printf("\n [CLIENT] Client connection successful \n");

  EdgeEndPointInfo* ep = (EdgeEndPointInfo*) malloc(sizeof(EdgeEndPointInfo));
  ep->endpointUri = endpoint;
  onStatusCallback(ep, STATUS_CLIENT_STARTED);

  return true;
}

void disconnect_client(EdgeEndPointInfo *epInfo) {
  if (m_client) {
    UA_Client_delete(m_client);
    m_client = NULL;
  }
  onStatusCallback(epInfo, STATUS_STOP_CLIENT);
}

void* getEndpoints(char *endpointUri) {
    UA_StatusCode retVal;
    UA_EndpointDescription *endpointArray = NULL;
    size_t endpointArraySize = 0;
    UA_Client* client = NULL;
//    std::vector<EdgeEndPointInfo*> endpointList;

    client = UA_Client_new(UA_ClientConfig_default);

    retVal = UA_Client_getEndpoints(client, /*endpointUri*/  "opc.tcp://mukunth-ubuntu:12686",
                                                  &endpointArraySize, &endpointArray);
    if (retVal != UA_STATUSCODE_GOOD) {
      printf("\n [CLIENT] Unable to get endpoints \n");
      UA_Array_delete(endpointArray, endpointArraySize, &UA_TYPES[UA_TYPES_ENDPOINTDESCRIPTION]);
      UA_Client_delete(client);
      client = NULL;
      return NULL;
    }

    printf("\n endpoint found :: [%d]\n" ,(int) endpointArraySize);
    for (size_t i = 0; i < endpointArraySize; i++) {
      printf("==========================================================");
      printf("\n endpoint :: %s ",endpointArray[i].endpointUrl.data);
      printf("endpoint security policy uri :: [%s] ", endpointArray[i].securityPolicyUri.data);
      printf("==========================================================");

//      EdgeEndpointConfig* config = ((new EdgeEndpointConfig::Builder())->setSecurityPolicyUri((char*) endpointArray[i].securityPolicyUri.data))->build();
//      EdgeEndPointInfo* epInfo = ((new EdgeEndPointInfo::Builder((char*) endpointArray[i].endpointUrl.data))->setConfig(config))->build();
//      endpointList.push_back(epInfo);
    }
    UA_Array_delete(endpointArray, endpointArraySize, &UA_TYPES[UA_TYPES_ENDPOINTDESCRIPTION]);
    UA_Client_delete(client);
    client = NULL;

    return NULL;
  }
