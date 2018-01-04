#include "read.h"
#include "common_client.h"

#include <stdio.h>
#include <inttypes.h>

#define GUID_LENGTH 36

static void read(UA_Client *client, EdgeMessage *msg) {

  printf("[READ] Node to read :: %s\n", msg->request->nodeInfo->valueAlias);
  UA_Variant *val = UA_Variant_new();
  UA_StatusCode retVal = UA_Client_readValueAttribute(client, UA_NODEID_STRING(1, msg->request->nodeInfo->valueAlias), val);
  if  (retVal != UA_STATUSCODE_GOOD) {
    // send error callback;
    printf("Error in read node :: 0x%08x(%s)\n", retVal, UA_StatusCode_name(retVal));

    EdgeMessage *resultMsg = (EdgeMessage*) malloc(sizeof(EdgeMessage));
    resultMsg->endpointInfo = (EdgeEndPointInfo*) malloc(sizeof(EdgeEndPointInfo));
    memcpy(resultMsg->endpointInfo, msg->endpointInfo, sizeof(EdgeEndPointInfo));
    resultMsg->type = ERROR;
    resultMsg->responseLength = 0;

    EdgeResult *res = (EdgeResult *) malloc(sizeof(EdgeResult));
    res->code = STATUS_ERROR;
    resultMsg->result = res;

    onResponseMessage(resultMsg);
    UA_Variant_delete(val);
    free(resultMsg);
    free(res);
    return;
  }

  printf("[READ] SUCCESS response received from server\n");
  bool isScalar = UA_Variant_isScalar(val);
  EdgeResponse* response = (EdgeResponse*) malloc(sizeof(EdgeResponse));
  if (response) {
    response->nodeInfo = (EdgeNodeInfo*) malloc(sizeof(EdgeNodeInfo));
    memcpy(response->nodeInfo, msg->request->nodeInfo, sizeof(EdgeNodeInfo));
    response->requestId = msg->request->requestId;

    EdgeVersatility *versatility = (EdgeVersatility*) malloc(sizeof(EdgeVersatility));
    if (isScalar) {
      versatility->arrayLength = 0;
      versatility->isArray = false;
    } else {
      versatility->arrayLength = val->arrayLength;
      versatility->isArray = true;
    }
    versatility->value = val->data;

    if (val->type == &UA_TYPES[UA_TYPES_BOOLEAN]) {
      response->type = Boolean;
    } else if(val->type == &UA_TYPES[UA_TYPES_INT16]) {
      response->type = Int16;
    } else if(val->type == &UA_TYPES[UA_TYPES_UINT16]) {
      response->type = UInt16;
    } else if(val->type == &UA_TYPES[UA_TYPES_INT32]) {
      response->type = Int32;
    } else if(val->type == &UA_TYPES[UA_TYPES_UINT32]) {
      response->type = UInt32;
    } else if(val->type == &UA_TYPES[UA_TYPES_INT64]) {
      response->type = Int64;
    } else if(val->type == &UA_TYPES[UA_TYPES_UINT64]) {
      response->type = UInt64;
    } else if(val->type == &UA_TYPES[UA_TYPES_FLOAT]) {
      response->type = Float;
    } else if(val->type == &UA_TYPES[UA_TYPES_DOUBLE]) {
      response->type = Double;
    } else if(val->type == &UA_TYPES[UA_TYPES_STRING]) {
      if (isScalar) {
        UA_String str = *((UA_String*) val->data);
        if((int)str.length)
          versatility->value = (void*) str.data;
        else
         versatility->value = NULL;
      } else {
        // String Array
        UA_String *str = ((UA_String*) val->data);
        char ** values = (char**) malloc(sizeof(char*) * val->arrayLength);
        for (int i = 0; i < val->arrayLength; i++) {
          values[i] = (char*) malloc(str[i].length);
          strcpy(values[i], (char*) str[i].data);
        }
        versatility->value = (void*) values;
      }
      response->type = String;
    } else if(val->type == &UA_TYPES[UA_TYPES_BYTESTRING]) {
      if (isScalar) {
          UA_ByteString byteStr = *((UA_ByteString*) val->data);
          if((int)byteStr.length)
            versatility->value = (void*) byteStr.data;
          else
            versatility->value = NULL;
      } else {
        // String Array
        UA_ByteString *str = ((UA_ByteString*) val->data);
        char ** values = (char**) malloc(sizeof(char*) * val->arrayLength);
        for (int i = 0; i < val->arrayLength; i++) {
          values[i] = (char*) malloc(str[i].length);
          strcpy(values[i], (char*) str[i].data);
        }
        versatility->value = (void*) values;
      }
      response->type = ByteString;
    } else if(val->type == &UA_TYPES[UA_TYPES_GUID]) {
      if (isScalar) {
          UA_Guid str = *((UA_Guid*) val->data);
          char *value = (char*) malloc(GUID_LENGTH+1);
          snprintf(value, (GUID_LENGTH/2) + 1, "%08x-%04x-%04x", str.data1, str.data2, str.data3);
          sprintf(value, "%s-%02x", value, str.data4[0]);
          sprintf(value, "%s%02x", value, str.data4[1]);
          sprintf(value, "%s-", value);
          for (int j = 2; j < 8; j++)
            sprintf(value, "%s%02x", value, str.data4[j]);
          value[GUID_LENGTH] = '\0';
          versatility->value = (void*) value;
          printf("%s\n", value);
      } else {
        // Guid Array
        UA_Guid *str = ((UA_Guid*) val->data);
        char ** values = (char**) malloc(sizeof(char*) * val->arrayLength);
        for (int i = 0; i < val->arrayLength; i++) {
          values[i] = (char*) malloc(GUID_LENGTH+1);
          snprintf(values[i], (GUID_LENGTH/2) + 1, "%08x-%04x-%04x", str[i].data1, str[i].data2, str[i].data3);
          sprintf(values[i], "%s-%02x", values[i], str[i].data4[0]);
          sprintf(values[i], "%s%02x", values[i], str[i].data4[1]);
          sprintf(values[i], "%s-", values[i]);
          for (int j = 2; j < 8; j++)
            sprintf(values[i], "%s%02x", values[i], str[i].data4[j]);
          values[GUID_LENGTH] = '\0';
          printf("%s\n", values[i]);
        }
        versatility->value = (void*) values;
      }
      response->type = Guid;
    } else if(val->type == &UA_TYPES[UA_TYPES_SBYTE]) {
      response->type = SByte;
    } else if(val->type == &UA_TYPES[UA_TYPES_BYTE]) {
      response->type = Byte;
    } else if(val->type == &UA_TYPES[UA_TYPES_DATETIME]) {
      response->type = DateTime;
      //UA_DateTime *str = ((UA_DateTime*) val->data);
      //printf("%s\n", (char*) (UA_DateTime_toString(str[0])).data);
    }
    response->message = versatility;

    EdgeMessage *resultMsg = (EdgeMessage*) malloc(sizeof(EdgeMessage));
    resultMsg->endpointInfo = (EdgeEndPointInfo*) malloc(sizeof(EdgeEndPointInfo));
    memcpy(resultMsg->endpointInfo, msg->endpointInfo, sizeof(EdgeEndPointInfo));
    resultMsg->type = GENERAL_RESPONSE;
    resultMsg->responseLength = 1;
    resultMsg->responses = (EdgeResponse**) malloc(1 * sizeof(EdgeResponse));
    resultMsg->responses[0] =response;

    onResponseMessage(resultMsg);

    if (response->type == String || response->type == ByteString) {
      if (!isScalar) {
        // Free String array
        char **values = response->message->value;
        if (values) {
          for (int i = 0; i < val->arrayLength; i++) {
            free (values[i]); values[i] = NULL;
          }
          free(values);
        }
      }
    }

    free(response->nodeInfo); response->nodeInfo = NULL;
    free(response->message); response->message = NULL;
    free(response); response = NULL;
    free (resultMsg->endpointInfo); resultMsg->endpointInfo = NULL;
    free (resultMsg->responses);resultMsg->responses = NULL;
    free(resultMsg); resultMsg = NULL;

    UA_Variant_delete(val);
  }
}


EdgeResult executeRead(UA_Client *client, EdgeMessage *msg) {
  EdgeResult result;
  if (!client) {
    printf("Client handle Invalid\n");
    result.code = STATUS_ERROR;
    return result;
  }
  read(client, msg);
  result.code = STATUS_OK;
  return result;
}
