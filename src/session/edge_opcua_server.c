#include "edge_opcua_server.h"
#include "edge_node.h"

#include <stdio.h>
#include <pthread.h>
#include <open62541.h>

static UA_ServerConfig *m_serverConfig;
static UA_Server *m_server;
static UA_Boolean b_running = UA_FALSE;

static pthread_t m_serverThread;

static int namespaceType = DEFAULT_TYPE;

void createNamespaceInServer(char* namespaceUri,
                     char* rootNodeIdentifier,
                     char* rootNodeBrowseName,
                     char* rootNodeDisplayName) {
  if (namespaceType == URI_TYPE || namespaceType == DEFAULT_TYPE) {
    int idx = UA_Server_addNamespace(m_server, namespaceUri);
    printf("\n [SERVER] Namespace Index :: [%d]", idx);
    printf("\n [SERVER] Namespace created\n");

//    nameSpace = ((new EdgeNamespace::Builder(m_server, idx, namespaceUri))->setNodeId(rootNodeIdentifier)->
//        setBrowseName(rootNodeBrowseName)->setDisplayName(rootNodeDisplayName))->build();
//    EdgeNamespaceManager::getInstance()->addNamespace(namespaceUri, nameSpace);
    }
  }

EdgeResult addNodesInServer(EdgeNodeItem *item) {
  EdgeResult result = addNodes(m_server, item);
  return result;
}

EdgeResult modifyNodeInServer(char* nodeUri, EdgeVersatility *value) {
  EdgeResult result = modifyNode(m_server, nodeUri, value);
  return result;
}

EdgeResult addReferenceInServer(EdgeReference *reference) {
  EdgeResult result = addReferences(m_server, reference);
  return result;
}

EdgeResult addMethodNodeInServer(EdgeNodeItem *item, EdgeMethod *method) {
  EdgeResult result = addMethodNode(m_server, item, method);
  return result;
}

static void* server_loop(void* ptr) {
  while (b_running) {
    UA_Server_run_iterate(m_server, true);
  }
  printf(" [SERVER] server loop exit\n");
  return NULL;
}

EdgeResult start_server(EdgeEndPointInfo* epInfo) {
    
    EdgeResult result;
    result.code = STATUS_OK;
    
    if(!epInfo)
    {
        result.code = STATUS_PARAM_INVALID;
        return result;
    }

  EdgeEndpointConfig* config = epInfo->config;

  //UA_ByteString certificate = loadCertificate();
  //m_serverConfig = UA_ServerConfig_new_default();    //UA_ServerConfig_new_minimal(4840, &certificate);
  m_serverConfig = UA_ServerConfig_new_minimal(config->bindPort, NULL);

  UA_String_deleteMembers(&m_serverConfig->applicationDescription.applicationUri);
  UA_LocalizedText_deleteMembers(&m_serverConfig->applicationDescription.applicationName);
  UA_String_deleteMembers(&m_serverConfig->applicationDescription.productUri);
  UA_String_deleteMembers(&m_serverConfig->buildInfo.productUri);
  UA_String_deleteMembers(&m_serverConfig->buildInfo.manufacturerName);
  UA_String_deleteMembers(&m_serverConfig->buildInfo.productName);
  UA_String_deleteMembers(&m_serverConfig->buildInfo.softwareVersion);
  UA_String_deleteMembers(&m_serverConfig->buildInfo.buildNumber);

  m_serverConfig->applicationDescription.applicationUri = UA_STRING_ALLOC(config->applicationUri);
  m_serverConfig->applicationDescription.applicationName = UA_LOCALIZEDTEXT_ALLOC("en-US", config->applicationName);
  m_serverConfig->applicationDescription.productUri = UA_STRING_ALLOC(config->productUri);
  m_serverConfig->buildInfo.productUri = UA_STRING_ALLOC("/edge");
  m_serverConfig->buildInfo.manufacturerName = UA_STRING_ALLOC("samsung");
  m_serverConfig->buildInfo.productName = UA_STRING_ALLOC("edgeSolution");
  m_serverConfig->buildInfo.softwareVersion = UA_STRING_ALLOC("0.9");
  m_serverConfig->buildInfo.buildNumber = UA_STRING_ALLOC("0.1");

  //    UA_ByteString_deleteMembers(&certificate);
  m_server = UA_Server_new(m_serverConfig);

  printf("\n [SERVER] starting server \n");
  UA_StatusCode retval = UA_Server_run_startup(m_server);

  if (retval != UA_STATUSCODE_GOOD) {
    printf("\n [SERVER] Error in starting server \n");
    b_running = UA_FALSE;

    result.code = STATUS_ERROR;
    return result;
  } else {
    printf("\n ========= [SERVER] Server Start successful ============= \n");
    b_running = UA_TRUE;    
    pthread_create(&m_serverThread, NULL, &server_loop, NULL);
    onStatusCallback(epInfo, STATUS_SERVER_STARTED);
    //return (new EdgeResult::Builder(STATUS_OK))->build();

    result.code = STATUS_OK;
    return result;
  }

}

void stop_server(EdgeEndPointInfo* epInfo) {
  b_running = false;
  pthread_join(m_serverThread, NULL);

  UA_Server_delete(m_server);
  UA_ServerConfig_delete(m_serverConfig);
  printf("\n ========= [SERVER] Server Stopped ============= \n");

  onStatusCallback(epInfo, STATUS_STOP_SERVER);
}
