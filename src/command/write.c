#include "write.h"
#include "common_client.h"

#include <stdio.h>
#include <inttypes.h>

#if 0
static void write(UA_Client *client, EdgeMessage *msg) {

  printf("[WRITE] Node to write :: %s\n", msg->request->nodeInfo->valueAlias);
  UA_Variant *myVariant = UA_Variant_new();

  int type = (int)msg->request->type - 1;
  if (type == UA_TYPES_STRING) {
    UA_String val = UA_STRING_ALLOC((char*) msg->request->value);
    UA_Variant_setScalarCopy(myVariant, &val, &UA_TYPES[type]);
  } else {
    UA_Variant_setScalarCopy(myVariant, msg->request->value, &UA_TYPES[type]);
  }

  UA_StatusCode retVal = UA_Client_writeValueAttribute(client, UA_NODEID_STRING(1, msg->request->nodeInfo->valueAlias), myVariant);
  if  (retVal != UA_STATUSCODE_GOOD) {
    // send error callback;
    printf("Error in read node :: 0x%08x\n", retVal);

    EdgeMessage *resultMsg = (EdgeMessage*) malloc(sizeof(EdgeMessage));
    resultMsg->endpointInfo = (EdgeEndPointInfo*) malloc(sizeof(EdgeEndPointInfo));
    memcpy(resultMsg->endpointInfo, msg->endpointInfo, sizeof(EdgeEndPointInfo));
    resultMsg->type = ERROR;
    resultMsg->responseLength = 0;

    EdgeResult *res = (EdgeResult *) malloc(sizeof(EdgeResult));
    res->code = STATUS_ERROR;
    resultMsg->result = res;

    onResponseMessage(resultMsg);
    UA_Variant_delete(myVariant);
    free(resultMsg);
    free(res);
    return ;
  }

  printf("[WRITE] SUCCESS response received from server\n");
  EdgeResponse* response = (EdgeResponse*) malloc(sizeof(EdgeResponse));
  if (response) {
    response->nodeInfo = (EdgeNodeInfo*) malloc(sizeof(EdgeNodeInfo));
    memcpy(response->nodeInfo, msg->request->nodeInfo, sizeof(EdgeNodeInfo));
    response->requestId = msg->request->requestId;
    response->message = NULL;
//    response->value = UA_STATUSCODE_GOOD;

    EdgeMessage *resultMsg = (EdgeMessage*) malloc(sizeof(EdgeMessage));
    resultMsg->endpointInfo = (EdgeEndPointInfo*) malloc(sizeof(EdgeEndPointInfo));
    memcpy(resultMsg->endpointInfo, msg->endpointInfo, sizeof(EdgeEndPointInfo));
    resultMsg->type = GENERAL_RESPONSE;
    resultMsg->responseLength = 1;
    resultMsg->responses = (EdgeResponse**) malloc(1 * sizeof(EdgeResponse));
    resultMsg->responses[0] =response;

    onResponseMessage(resultMsg);

    free(response->nodeInfo); response->nodeInfo = NULL;
    free(response); response = NULL;
    free (resultMsg->endpointInfo); resultMsg->endpointInfo = NULL;
    free (resultMsg->responses);resultMsg->responses = NULL;
    free(resultMsg); resultMsg = NULL;

    UA_Variant_delete(myVariant);
  }
}

#endif

static void writeGroup(UA_Client *client, EdgeMessage *msg) {

  int reqLen = msg->requestLength;
  UA_WriteValue* wv = (UA_WriteValue*) malloc(sizeof(UA_WriteValue) * reqLen);
  UA_Variant *myVariant = (UA_Variant*) malloc(sizeof(UA_Variant) * reqLen);

  for (int i = 0; i < reqLen; i++) {
    printf("[WRITEGROUP] Node to write :: %s\n", msg->requests[i]->nodeInfo->valueAlias);
    int type = (int)msg->requests[i]->type - 1;
    UA_WriteValue_init(&wv[i]);
    UA_Variant_init(&myVariant[i]);
    wv[i].attributeId = UA_ATTRIBUTEID_VALUE;
    wv[i].nodeId = UA_NODEID_STRING(1, msg->requests[i]->nodeInfo->valueAlias);
    wv[i].value.hasValue = true;
    wv[i].value.value.type = &UA_TYPES[type];
    wv[i].value.value.storageType = UA_VARIANT_DATA_NODELETE; /* do not free the integer on deletion */
    if (type == UA_TYPES_STRING) {
      UA_String val = UA_STRING_ALLOC((char*) msg->requests[i]->value);
      UA_Variant_setScalarCopy(&myVariant[i], &val, &UA_TYPES[type]);
    } else {
      UA_Variant_setScalarCopy(&myVariant[i], msg->requests[i]->value, &UA_TYPES[type]);
    }
    wv[i].value.value = myVariant[i];
  }

  UA_WriteRequest writeRequest;
  UA_WriteRequest_init(&writeRequest);
  writeRequest.nodesToWrite = wv;
  writeRequest.nodesToWriteSize = reqLen;

  UA_WriteResponse writeResponse = UA_Client_Service_write(client, writeRequest);
  if(writeResponse.responseHeader.serviceResult != UA_STATUSCODE_GOOD) {
      printf("Error in write :: 0x%08x(%s)\n", writeResponse.responseHeader.serviceResult,
            UA_StatusCode_name(writeResponse.responseHeader.serviceResult));
    UA_WriteRequest_deleteMembers(&writeRequest);
    UA_WriteResponse_deleteMembers(&writeResponse);
    return;
  }

  if(reqLen != writeResponse.resultsSize) {
      printf("Requested(%d) but received(%d) => %s\n", reqLen, (int)writeResponse.resultsSize,
            (reqLen < writeResponse.resultsSize)?"Received more results":"Received less results");
      EdgeMessage *resultMsg = (EdgeMessage*) malloc(sizeof(EdgeMessage));
      resultMsg->endpointInfo = (EdgeEndPointInfo*) malloc(sizeof(EdgeEndPointInfo));
      memcpy(resultMsg->endpointInfo, msg->endpointInfo, sizeof(EdgeEndPointInfo));
      resultMsg->type = ERROR;
      resultMsg->responseLength = 0;

      EdgeResult *res = (EdgeResult *) malloc(sizeof(EdgeResult));
      res->code = STATUS_ERROR;
      resultMsg->result = res;

      onResponseMessage(resultMsg);
      UA_WriteRequest_deleteMembers(&writeRequest);
      UA_WriteResponse_deleteMembers(&writeResponse);
      free(resultMsg);
      free(res);
      return;
  }

  int respIndex = 0;
  EdgeResponse** responses = (EdgeResponse**) malloc(sizeof(EdgeResponse*) * reqLen);
  EdgeMessage *resultMsg = (EdgeMessage*) malloc(sizeof(EdgeMessage));
  resultMsg->endpointInfo = (EdgeEndPointInfo*) malloc(sizeof(EdgeEndPointInfo));
  resultMsg->responseLength = 0;
  memcpy(resultMsg->endpointInfo, msg->endpointInfo, sizeof(EdgeEndPointInfo));
  resultMsg->type = GENERAL_RESPONSE;

  for (int i = 0; i < reqLen; i++) {
    responses[i]  = NULL;
    UA_StatusCode code = writeResponse.results[i];

    if (code != UA_STATUSCODE_GOOD) {
      printf("Error in write response for a particular node :: 0x%08x(%s)\n", code, UA_StatusCode_name(code));
      EdgeMessage *resultMsg = (EdgeMessage*) malloc(sizeof(EdgeMessage));
      resultMsg->endpointInfo = (EdgeEndPointInfo*) malloc(sizeof(EdgeEndPointInfo));
      memcpy(resultMsg->endpointInfo, msg->endpointInfo, sizeof(EdgeEndPointInfo));
      resultMsg->type = ERROR;
      resultMsg->responseLength = 0;

      EdgeResult *res = (EdgeResult *) malloc(sizeof(EdgeResult));
      res->code = STATUS_ERROR;
      resultMsg->result = res;

      onResponseMessage(resultMsg);
      free(resultMsg);
      free(res);

      if (writeResponse.responseHeader.serviceResult != UA_STATUSCODE_GOOD)
        continue;
    }

    EdgeResponse* response = (EdgeResponse*) malloc(sizeof(EdgeResponse));
    if (response) {
      response->nodeInfo = (EdgeNodeInfo*) malloc(sizeof(EdgeNodeInfo));
      memcpy(response->nodeInfo, msg->requests[i]->nodeInfo, sizeof(EdgeNodeInfo));
      response->requestId = msg->requests[i]->requestId;
      response->message = NULL;

      resultMsg->responseLength += 1;
      responses[respIndex] = response;
      respIndex += 1;
    }
  }

  resultMsg->responses = responses;
  onResponseMessage(resultMsg);

  for (int i = 0; i < resultMsg->responseLength; i++) {
    free(responses[i]->nodeInfo); responses[i]->nodeInfo = NULL;
    free(responses[i]); responses[i] = NULL;
  }
  free (resultMsg->endpointInfo); resultMsg->endpointInfo = NULL;
  free (resultMsg->responses);resultMsg->responses = NULL;
  free(resultMsg); resultMsg = NULL;
  UA_WriteResponse_deleteMembers(&writeResponse);
}

EdgeResult executeWrite(UA_Client *client, EdgeMessage *msg) {
  EdgeResult result;
  if (!client) {
    printf("Client handle Invalid\n");
    result.code = STATUS_ERROR;
    return result;
  }

  //if (msg->type == SEND_REQUEST)
  //  write(client, msg);
  //else
  writeGroup(client, msg);

  result.code = STATUS_OK;
  return result;
}
