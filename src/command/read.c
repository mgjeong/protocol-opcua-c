#include "read.h"
#include "common_client.h"

#include <stdio.h>
#include <inttypes.h>

#define GUID_LENGTH 36

#if 0
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

#endif

UA_Int64
DateTime_toUnixTime(UA_DateTime date) {
    return (date - UA_DATETIME_UNIX_EPOCH) / UA_DATETIME_MSEC;
}

static void sendErrorResponse(EdgeMessage *msg, char *err_desc) {
    EdgeMessage *resultMsg = (EdgeMessage*) malloc(sizeof(EdgeMessage));
    resultMsg->endpointInfo = (EdgeEndPointInfo*) malloc(sizeof(EdgeEndPointInfo));
    memcpy(resultMsg->endpointInfo, msg->endpointInfo, sizeof(EdgeEndPointInfo));
    resultMsg->type = ERROR;
    resultMsg->responseLength = 0;

    EdgeVersatility* message = (EdgeVersatility*) malloc(sizeof(EdgeVersatility));
    message->value = (void*) err_desc;

    EdgeResult *res = (EdgeResult *) malloc(sizeof(EdgeResult));
    res->code = STATUS_ERROR;
    resultMsg->result = res;

    onResponseMessage(resultMsg);
    free(res);
    free(resultMsg);
}

static bool checkMaxAge(UA_DateTime timestamp, UA_DateTime now, double maxAge) {

  // Server timestamp is greater than current time
  if (timestamp > now)
      return false;

  int64_t second = DateTime_toUnixTime(now);
  int64_t first = DateTime_toUnixTime(timestamp);

  int64_t diff = second - first;

  if ((maxAge != 0) && (diff > maxAge))
      return false;

  return true;
}

static bool compareNumber(int64_t number1, int64_t number2) {
    if (number1 > number2)
        return true;

    return false;
}

static bool checkInvalidTime(UA_DateTime serverTime, UA_DateTime sourceTime,
                             int validMilliSec, UA_TimestampsToReturn stamp) {
    bool ret = true;
    UA_DateTime now = UA_DateTime_now();
    int64_t now_time = UA_DateTime_toUnixTime(now);
    int64_t server_time = UA_DateTime_toUnixTime(serverTime);
    int64_t source_time = UA_DateTime_toUnixTime(sourceTime);

    if (UA_TIMESTAMPSTORETURN_BOTH == stamp) {
        if ((0 == server_time) || (0 == source_time)) {
            printf("Invalid timestamp\n\n");
            ret = false;
        }

        // compare number
        if (compareNumber(now_time - server_time, validMilliSec)) {
            ret = false;
        } else if (compareNumber(now_time - source_time, validMilliSec)) {
            ret = false;
        } else if (compareNumber(server_time, now_time)) {
            ret = false;
        } else if (compareNumber(source_time, now_time)) {
            ret = false;
        }
    } else if (UA_TIMESTAMPSTORETURN_SOURCE == stamp) {
        if (0 == source_time) {
            printf("invalid source timestamp\n\n");
            return false;
        }

        // compare number
        if (compareNumber(now_time - source_time, validMilliSec)) {
            ret = false;
        } else if (compareNumber(source_time, now_time)) {
            ret = false;
        }
    } else if (UA_TIMESTAMPSTORETURN_SERVER == stamp) {
        if (0 == server_time) {
            printf("invalid server timestamp\n\n");
            return false;
        }

        // compare number
        if (compareNumber(now_time - server_time, validMilliSec)) {
            ret = false;
        } else if (compareNumber(server_time, now_time)) {
            ret = false;
        }
    }

    return ret;
}

static void* checkValidation(UA_DataValue* value, EdgeMessage *msg, UA_TimestampsToReturn stamp, double maxAge) {

    // if (!checkNaN(value))

    if (!checkInvalidTime(value->serverTimestamp, value->sourceTimestamp, 86400000, stamp)) {
        // Error message handling
        sendErrorResponse(msg, "Invalid Time");
        return NULL;
    }

    if (UA_STATUSCODE_GOOD != value->status) {
        // Error message handling
        sendErrorResponse(msg, "Error status code from server");
        return NULL;
    }

    if (!UA_Variant_isScalar(&(value->value))) {
        if (value->value.arrayLength == 0) {
            // Error message handling
            sendErrorResponse(msg, "Invalid array length in read response");
            return NULL;
        }
    }
    return value;
}

static EdgeDiagnosticInfo* checkDiagnosticInfo(int nodesToProcess, UA_DiagnosticInfo* diagnosticInfo,
                                int diagnosticInfoLength, int returnDiagnostic) {

    EdgeDiagnosticInfo *diagnostics = (EdgeDiagnosticInfo*) malloc(sizeof(EdgeDiagnosticInfo));
    diagnostics->symbolicId = 0;
    diagnostics->localizedText = 0;
    diagnostics->additionalInfo = NULL;
    diagnostics->innerDiagnosticInfo = NULL;
    if (0 == returnDiagnostic && 0 == diagnosticInfoLength) {
        diagnostics->msg = NULL;
    } else if (diagnosticInfoLength == nodesToProcess){
        diagnostics->symbolicId = diagnosticInfo[0].symbolicId;
        diagnostics->localizedText = diagnosticInfo[0].localizedText;
        diagnostics->locale = diagnosticInfo[0].locale;
        if (diagnosticInfo[0].hasAdditionalInfo) {
            char *additional_info = (char*) malloc(diagnosticInfo[0].additionalInfo.length);
            strcpy(additional_info, (char*)(diagnosticInfo[0].additionalInfo.data));
            diagnostics->additionalInfo = additional_info;
        }
        if (diagnosticInfo[0].hasInnerDiagnosticInfo)
            diagnostics->innerDiagnosticInfo = diagnosticInfo[0].innerDiagnosticInfo;
    } else if (0 != returnDiagnostic && 0 == diagnosticInfoLength) {
        diagnostics->msg = (void*) "no diagnostics were returned even though returnDiagnostic requested" ;
    } else {
        diagnostics->msg = (void*) "mismatch entries returned" ;
    }
    return diagnostics;
}

static void readGroup(UA_Client *client, EdgeMessage *msg) {
  int reqLen = msg->requestLength;
  UA_ReadValueId* rv = (UA_ReadValueId*) malloc(sizeof(UA_ReadValueId) * reqLen);

  for (int i = 0; i < reqLen; i++) {
    printf("[READGROUP] Node to read :: %s\n", msg->requests[i]->nodeInfo->valueAlias);
    UA_ReadValueId_init(&rv[i]);
    rv[i].attributeId = UA_ATTRIBUTEID_VALUE;
    rv[i].nodeId = UA_NODEID_STRING(1, msg->requests[i]->nodeInfo->valueAlias);
  }

  UA_ReadRequest readRequest;
  UA_ReadRequest_init(&readRequest);
  readRequest.nodesToRead = rv;
  readRequest.nodesToReadSize = reqLen;
  readRequest.maxAge = 2000;
  readRequest.timestampsToReturn = UA_TIMESTAMPSTORETURN_BOTH;

  //UA_RequestHeader_init(&(readRequest.requestHeader));
  //readRequest.requestHeader.returnDiagnostics = 1;

  UA_ReadResponse readResponse = UA_Client_Service_read(client, readRequest);
  if(readResponse.responseHeader.serviceResult != UA_STATUSCODE_GOOD) {
    // send error callback;
    UA_StatusCode serviceResult = readResponse.responseHeader.serviceResult;
    printf("Error in group read :: 0x%08x(%s)\n", serviceResult, UA_StatusCode_name(serviceResult));

    sendErrorResponse(msg, "Error in read");
    UA_ReadValueId_deleteMembers(rv);
    UA_ReadResponse_deleteMembers(&readResponse);
    return ;
  }

  if (readResponse.results[0].status == UA_STATUSCODE_GOOD) {
      if (readRequest.timestampsToReturn == UA_TIMESTAMPSTORETURN_NEITHER) {
          if (readResponse.results[0].hasSourceTimestamp || readResponse.results[0].hasServerTimestamp) {
              printf("BadInvalidTimestamp\n\n");
              sendErrorResponse(msg, "BadInvalidTimestamp");
              UA_ReadValueId_deleteMembers(rv);
              return ;
          }
      }
      if (readRequest.timestampsToReturn == UA_TIMESTAMPSTORETURN_BOTH) {
          if (!readResponse.results[0].hasSourceTimestamp || !readResponse.results[0].hasServerTimestamp) {
              printf("Timestamp missing\n\n");
              sendErrorResponse(msg, "Missing Timestamp");
              UA_ReadValueId_deleteMembers(rv);
              return ;
          }
      }
      if (readRequest.timestampsToReturn == UA_TIMESTAMPSTORETURN_SOURCE) {
          if (!readResponse.results[0].hasSourceTimestamp || readResponse.results[0].hasServerTimestamp) {
              printf("source Timestamp missing\n\n");
              sendErrorResponse(msg, "Missing Timestamp");
              UA_ReadValueId_deleteMembers(rv);
              return ;
          }
      }
      if (readRequest.timestampsToReturn == UA_TIMESTAMPSTORETURN_SERVER) {
          if (readResponse.results[0].hasSourceTimestamp || !readResponse.results[0].hasServerTimestamp) {
              printf("server Timestamp missing\n\n");
              sendErrorResponse(msg, "Missing Timestamp");
              UA_ReadValueId_deleteMembers(rv);
              return ;
          }
      }

      if (readRequest.timestampsToReturn != UA_TIMESTAMPSTORETURN_NEITHER && !checkMaxAge(readResponse.results[0].serverTimestamp, UA_DateTime_now(), readRequest.maxAge * 2)) {
          printf("Max age failed\n\n");
          sendErrorResponse(msg, "");
          UA_ReadValueId_deleteMembers(rv);
          UA_ReadResponse_deleteMembers(&readResponse);
          return ;
      }

      if (readRequest.timestampsToReturn != UA_TIMESTAMPSTORETURN_NEITHER && !checkValidation(&(readResponse.results[0]), msg, readRequest.timestampsToReturn, readRequest.maxAge)) {
          UA_ReadValueId_deleteMembers(rv);
          UA_ReadResponse_deleteMembers(&readResponse);
          return ;
      }
  }

  int respIndex = 0;
  EdgeResponse** responses = (EdgeResponse**) malloc(sizeof(EdgeResponse*) * reqLen);
  EdgeMessage *resultMsg = (EdgeMessage*) malloc(sizeof(EdgeMessage));
  resultMsg->endpointInfo = (EdgeEndPointInfo*) malloc(sizeof(EdgeEndPointInfo));
  resultMsg->responseLength = 0;
  resultMsg->command = CMD_READ;
  memcpy(resultMsg->endpointInfo, msg->endpointInfo, sizeof(EdgeEndPointInfo));
  resultMsg->type = GENERAL_RESPONSE;

  for (int i = 0; i < reqLen; i++) {
    responses[i]  = NULL;
    if (readResponse.results[i].status == UA_STATUSCODE_GOOD) {
      UA_Variant val = readResponse.results[i].value;
      bool isScalar = UA_Variant_isScalar(&val);
      EdgeResponse* response = (EdgeResponse*) malloc(sizeof(EdgeResponse));
      if (response) {
        response->nodeInfo = (EdgeNodeInfo*) malloc(sizeof(EdgeNodeInfo));
        memcpy(response->nodeInfo, msg->requests[i]->nodeInfo, sizeof(EdgeNodeInfo));
        response->requestId = msg->requests[i]->requestId;

        EdgeVersatility *versatility = (EdgeVersatility*) malloc(sizeof(EdgeVersatility));
        if (isScalar) {
          versatility->arrayLength = 0;
          versatility->isArray = false;
        } else {
          versatility->arrayLength = val.arrayLength;
          versatility->isArray = true;
        }
        versatility->value = val.data;

        if (val.type == &UA_TYPES[UA_TYPES_BOOLEAN]) {
          response->type = Boolean;
        } else if(val.type == &UA_TYPES[UA_TYPES_INT16]) {
          response->type = Int16;
        } else if(val.type == &UA_TYPES[UA_TYPES_UINT16]) {
          response->type = UInt16;
        } else if(val.type == &UA_TYPES[UA_TYPES_INT32]) {
          response->type = Int32;
        } else if(val.type == &UA_TYPES[UA_TYPES_UINT32]) {
          response->type = UInt32;
        } else if(val.type == &UA_TYPES[UA_TYPES_INT64]) {
          response->type = Int64;
        } else if(val.type == &UA_TYPES[UA_TYPES_UINT64]) {
          response->type = UInt64;
        } else if(val.type == &UA_TYPES[UA_TYPES_FLOAT]) {
          response->type = Float;
        } else if(val.type == &UA_TYPES[UA_TYPES_DOUBLE]) {
          response->type = Double;
        } else if(val.type == &UA_TYPES[UA_TYPES_STRING]) {
          if (isScalar) {
            UA_String str = *((UA_String*) val.data);
            if((int)str.length)
              versatility->value = (void*) str.data;
            else
             versatility->value = NULL;
          } else {
            // String Array
            UA_String *str = ((UA_String*) val.data);
            char ** values = (char**) malloc(sizeof(char*) * val.arrayLength);
            for (int i = 0; i < val.arrayLength; i++) {
              values[i] = (char*) malloc(str[i].length);
              strcpy(values[i], (char*) str[i].data);
            }
            versatility->value = (void*) values;
          }
          response->type = String;
        } else if(val.type == &UA_TYPES[UA_TYPES_BYTESTRING]) {
          if (isScalar) {
              UA_ByteString byteStr = *((UA_ByteString*) val.data);
              if((int)byteStr.length)
                versatility->value = (void*) byteStr.data;
              else
                versatility->value = NULL;
          } else {
            // String Array
            UA_ByteString *str = ((UA_ByteString*) val.data);
            char ** values = (char**) malloc(sizeof(char*) * val.arrayLength);
            for (int i = 0; i < val.arrayLength; i++) {
              values[i] = (char*) malloc(str[i].length);
              strcpy(values[i], (char*) str[i].data);
            }
            versatility->value = (void*) values;
          }
          response->type = ByteString;
        } else if(val.type == &UA_TYPES[UA_TYPES_GUID]) {
          if (isScalar) {
              UA_Guid str = *((UA_Guid*) val.data);
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
            UA_Guid *str = ((UA_Guid*) val.data);
            char ** values = (char**) malloc(sizeof(char*) * val.arrayLength);
            for (int i = 0; i < val.arrayLength; i++) {
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
        } else if(val.type == &UA_TYPES[UA_TYPES_SBYTE]) {
          response->type = SByte;
        } else if(val.type == &UA_TYPES[UA_TYPES_BYTE]) {
          response->type = Byte;
        } else if(val.type == &UA_TYPES[UA_TYPES_DATETIME]) {
          response->type = DateTime;
          //UA_DateTime *str = ((UA_DateTime*) val.data);
          //printf("%s\n", (char*) (UA_DateTime_toString(str[0])).data);
        }
        response->message = versatility;
        // ctt check
        EdgeDiagnosticInfo * diagnosticInfo = checkDiagnosticInfo(msg->requestLength,
                                                                  readResponse.diagnosticInfos,
                                                                  readResponse.diagnosticInfosSize,
                                                                  readRequest.requestHeader.returnDiagnostics);
        response->m_diagnosticInfo = diagnosticInfo;

        resultMsg->responseLength += 1;
        responses[respIndex] =response;
        respIndex += 1;

      }
    } else {
      // failure read response for this particular node
      printf("Error in group read response for particular node :: 0x%08x(%s)\n",
            readResponse.results[i].status, UA_StatusCode_name(readResponse.results[i].status));
      sendErrorResponse(msg, "");
    }
  }

  resultMsg->responses = responses;  
  onResponseMessage(resultMsg);

  for (int i = 0; i < resultMsg->responseLength; i++) {
    if (responses[i]->type == String || responses[i]->type == ByteString) {
      if (responses[i]->message->isArray) {
        // Free String array
        char **values = responses[i]->message->value;
        if (values) {
          for (int j = 0; j < responses[i]->message->arrayLength; j++) {
            free (values[j]); values[j] = NULL;
          }
          free(values);
        }
      }
    }
    if (responses[i]->m_diagnosticInfo->additionalInfo) {
      free(responses[i]->m_diagnosticInfo->additionalInfo); responses[i]->m_diagnosticInfo->additionalInfo = NULL;
    }
    free(responses[i]->m_diagnosticInfo); responses[i]->m_diagnosticInfo = NULL;
    free(responses[i]->nodeInfo); responses[i]->nodeInfo = NULL;
    free(responses[i]->message); responses[i]->message = NULL;
    free(responses[i]); responses[i] = NULL;
  }
  free (resultMsg->endpointInfo); resultMsg->endpointInfo = NULL;
  free (resultMsg->responses);resultMsg->responses = NULL;
  free(resultMsg); resultMsg = NULL;

  free(rv);
  UA_ReadResponse_deleteMembers(&readResponse);

  return ;
}

EdgeResult executeRead(UA_Client *client, EdgeMessage *msg) {
  EdgeResult result;
  if (!client) {
    printf("Client handle Invalid\n");
    result.code = STATUS_ERROR;
    return result;
  }
  //if (msg->requestLength == 1)
  //  read(client, msg);
  //else
  readGroup(client, msg);

  result.code = STATUS_OK;
  return result;
}
