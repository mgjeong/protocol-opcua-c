#include "browse.h"
#include "edge_utils.h"
#include "edge_node_type.h"

#include <stdio.h>

static edgeMap *browseMap;
static int browseNamesCount = 0;

static bool isMapElementPresent(keyValue key) {
  edgeMapNode *temp = browseMap->head;
  while(temp != NULL)
  {
    if(!strcmp((char*) temp->key, (char*) key))
      return true;
    temp = temp->next;
  }
  return false;
}

static void deleteBrowseMap() {
  edgeMapNode *temp = browseMap->head;
  edgeMapNode *xtemp;
  while(temp != NULL) {
          xtemp = temp->next;
          free(temp->key);
          free(temp);
          temp = xtemp;
  }
  browseMap->head = NULL;
}

static EdgeBrowseResult** getBrowseNamesFromMap() {
  EdgeBrowseResult **browseResult = (EdgeBrowseResult**) malloc(sizeof(EdgeBrowseResult*) * browseNamesCount);
  int idx = 0;
  for (idx = 0; idx < browseNamesCount; idx++)
    browseResult[idx] = (EdgeBrowseResult*) malloc(sizeof(EdgeBrowseResult));

  edgeMapNode *temp = browseMap->head;
  idx = 0;
  while(temp != NULL)
  {
    browseResult[idx]->browseName  = (char*) malloc(strlen((char*) temp->key) + 1);
    strncpy(browseResult[idx]->browseName, (char*) temp->key, strlen((char*) temp->key));
    browseResult[idx]->browseName[strlen((char*) temp->key)] = '\0';
    idx += 1;
    temp = temp->next;
  }
  return browseResult;
}


void browse(UA_Client *client, EdgeMessage *msg, UA_NodeId *node) {
  UA_BrowseRequest bReq;
  UA_BrowseRequest_init(&bReq);
  bReq.requestedMaxReferencesPerNode = 0;
  bReq.nodesToBrowse = UA_BrowseDescription_new();
  bReq.nodesToBrowseSize = 1;
  bReq.nodesToBrowse[0].nodeId = *node;
  bReq.nodesToBrowse[0].resultMask = UA_BROWSERESULTMASK_ALL; /* return everything */
  UA_BrowseResponse bResp = UA_Client_Service_browse(client, bReq);
  for (size_t i = 0; i < bResp.resultsSize; ++i) {
          for (size_t j = 0; j < bResp.results[i].referencesSize; ++j) {
            UA_ReferenceDescription *ref = &(bResp.results[i].references[j]);

            bool isPresent = isMapElementPresent((char*) ref->browseName.name.data);
            if (!isPresent) {
              browseNamesCount += 1;
              char *name = malloc(strlen((char*) ref->browseName.name.data) + 1);
              strcpy(name, (char*) ref->browseName.name.data);
              name[strlen((char*) ref->browseName.name.data)]  ='\0';
              insertMapElement(browseMap, (keyValue) name, (keyValue) &(ref->nodeId.nodeId));

              if (ref->nodeClass == UA_NODECLASS_VIEW ||
                  ref->nodeClass == UA_NODECLASS_OBJECT ||
                  ref->nodeClass == UA_NODECLASS_OBJECTTYPE ||
                  ref->nodeClass == UA_NODECLASS_VARIABLETYPE ||
                  ref->nodeClass == UA_NODECLASS_REFERENCETYPE) {
                browse(client, msg, &(ref->nodeId.nodeId));
              }
            }
          }
      }
      //UA_BrowseRequest_deleteMembers(&bReq);
      UA_BrowseResponse_deleteMembers(&bResp);
}

EdgeResult *executeBrowse(UA_Client *client, EdgeMessage *msg) {
  EdgeResult* result = (EdgeResult*) malloc(sizeof(EdgeResult));
  if (!client) {
    printf("Client handle Invalid\n");
    result->code = STATUS_ERROR;
    return result;
  }

  browseNamesCount = 0;
  browseMap = createMap();

  UA_NodeId node;
  if (msg->request->nodeInfo->nodeId->type == INTEGER)
    node = UA_NODEID_NUMERIC(msg->request->nodeInfo->nodeId->nameSpace, msg->request->nodeInfo->nodeId->integerNodeId);
  else if (msg->request->nodeInfo->nodeId->type == STRING)
    node = UA_NODEID_STRING_ALLOC(msg->request->nodeInfo->nodeId->nameSpace, msg->request->nodeInfo->nodeId->nodeId);
  else
    node = UA_NODEID_NUMERIC(msg->request->nodeInfo->nodeId->nameSpace, UA_NS0ID_ROOTFOLDER);
  browse(client, msg, &node);

  EdgeBrowseResult **browseResult = getBrowseNamesFromMap();

  deleteBrowseMap();
  browseMap = NULL;

  EdgeMessage *resultMsg = (EdgeMessage*) malloc(sizeof(EdgeMessage));
  resultMsg->endpointInfo = msg->endpointInfo;
  resultMsg->type = BROWSE_RESPONSE;
  resultMsg->browseResult = browseResult;
  resultMsg->browseResponseLength = browseNamesCount;

  onResponseMessage(resultMsg);

  // free browseResult
  int idx = 0;
  for (idx = 0; idx < browseNamesCount; idx++) {
    free(browseResult[idx]->browseName);
    browseResult[idx]->browseName = NULL;
  }

  free (resultMsg);

  result->code = STATUS_OK;
  return result;
}
