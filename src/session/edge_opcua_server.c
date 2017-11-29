#include "edge_opcua_server.h"

#include <stdio.h>
#include <pthread.h>
#include <open62541.h>



static UA_ServerConfig *m_serverConfig;
static UA_Server *m_server;
static UA_Boolean b_running;

static pthread_t m_serverThread;

void createNamespace(char* namespaceUri,
                     char* rootNodeIdentifier,
                     char* rootNodeBrowseName,
                     char* rootNodeDisplayName) {
  // Not implemented yet
}

static void* server_loop(void* ptr) {
  printf("Server Loop entry\n");
  while (b_running) {
    UA_Server_run_iterate(m_server, true);
  }
  printf("Server Loop exit\n");
  return NULL;
}

EdgeResult* start_server(EdgeEndPointInfo* epInfo) {

  EdgeEndpointConfig* config = epInfo->config;

  //    UA_ByteString certificate = loadCertificate();
  m_serverConfig = UA_ServerConfig_new_default();    //UA_ServerConfig_new_minimal(4840, &certificate);

  UA_String_deleteMembers(&m_serverConfig->applicationDescription.applicationUri);
  UA_LocalizedText_deleteMembers(&m_serverConfig->applicationDescription.applicationName);
  UA_String_deleteMembers(&m_serverConfig->applicationDescription.productUri);

  m_serverConfig->applicationDescription.applicationUri = UA_STRING_ALLOC(config->applicationUri);
  m_serverConfig->applicationDescription.applicationName = UA_LOCALIZEDTEXT_ALLOC("en", config->applicationName);
  m_serverConfig->applicationDescription.productUri = UA_STRING_ALLOC(config->productUri);
  m_serverConfig->buildInfo.productUri = UA_STRING_ALLOC("/edge");
  m_serverConfig->buildInfo.manufacturerName = UA_STRING_ALLOC("samsung");
  m_serverConfig->buildInfo.productName = UA_STRING_ALLOC("edgeSolution");
  m_serverConfig->buildInfo.softwareVersion = UA_STRING_ALLOC("0.9");
  m_serverConfig->buildInfo.buildNumber = UA_STRING_ALLOC("0.1");

  //    UA_ByteString_deleteMembers(&certificate);
  m_server = UA_Server_new(m_serverConfig);

  printf("\n [SERVER] starting server \n");
  UA_StatusCode retval = UA_Server_run_startup(m_server);     //, &b_running);

  if (retval != UA_STATUSCODE_GOOD) {
    printf("\n [SERVER] Error in starting server \n");
    b_running = UA_FALSE;

    EdgeResult* result = (EdgeResult*) malloc(sizeof(EdgeResult));
    result->code = STATUS_ERROR;
    return result;
  } else {
    printf("\n [SERVER] Server Start successful \n");
    b_running = UA_TRUE;    
    pthread_create(&m_serverThread, NULL, &server_loop, NULL);
    //ProtocolManager::getProtocolManagerInstance()->onStatusCallback(epInfo, STATUS_SERVER_STARTED);
    //return (new EdgeResult::Builder(STATUS_OK))->build();

    EdgeResult* result = (EdgeResult*) malloc(sizeof(EdgeResult));
    result->code = STATUS_OK;
    return result;
  }

}

void stop_server() {
  b_running = false;
  pthread_join(m_serverThread, NULL);

  UA_Server_delete(m_server);
  UA_ServerConfig_delete(m_serverConfig);
}
