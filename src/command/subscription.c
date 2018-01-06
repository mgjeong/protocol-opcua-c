#include "subscription.h"
#include "edge_utils.h"

#include <stdio.h>
#include <pthread.h>
#include <unistd.h>

typedef struct subscriptionInfo {
  EdgeMessage* msg;
  int subId;
  int monId;
} subscriptionInfo;

static int subscriptionCount = 0;
static bool subscription_thread_running = false;
static pthread_t subscription_thread;

static edgeMap *subscriptionList;

static keyValue getSubscriptionId(char *valueAlias) {
  edgeMapNode* temp = subscriptionList->head;
  while (temp != NULL) {
    if(!strcmp(temp->key, valueAlias)) {
      return temp->value;
    }
    temp = temp->next;
  }
  return NULL;
}

static edgeMapNode* removeSubscriptionFromMap(char *valueAlias) {
  edgeMapNode *temp = subscriptionList->head;
  edgeMapNode *prev = NULL;
  while(temp != NULL) {
    if(!strcmp(temp->key, valueAlias)) {
      if (prev == NULL) {
        subscriptionList->head = temp->next;
      } else {
        prev->next = temp->next;
      }
//      free (temp); temp = NULL;
      return temp;
    }
    prev = temp;
    temp = temp->next;
  }
  return NULL;
}

#ifdef TEST_SUBSCRIPTION_LIST
static void printMap() {
  edgeMapNode* temp = subscriptionList->head;
  while (temp != NULL) {
    temp = temp->next;
  }
}
#endif


void sendPublishRequest(UA_Client *client) {
  UA_Client_Subscriptions_manuallySendPublishRequest(client);
}

static void
monitoredItemHandler(UA_UInt32 monId, UA_DataValue *value, void *context) {
  if( value->hasValue) {
    printf("value is present, monId :: %d\n", monId);

    char *valueAlias = (char*) context;
    subscriptionInfo* subInfo =  (subscriptionInfo*) getSubscriptionId(valueAlias);
    //printf("subscription id retrieved from map :: %d \n\n", subInfo->subId);

    if (!subInfo)
      return ;

    EdgeResponse* response = (EdgeResponse*) malloc(sizeof(EdgeResponse));
    if (response) {
      response->nodeInfo = (EdgeNodeInfo*) malloc(sizeof(EdgeNodeInfo));
      memcpy(response->nodeInfo, subInfo->msg->request->nodeInfo, sizeof(EdgeNodeInfo));
      response->requestId = subInfo->msg->request->requestId;

      EdgeVersatility *versatility = (EdgeVersatility*) malloc(sizeof(EdgeVersatility));
      versatility->arrayLength = 0;
      versatility->isArray = false;
      versatility->value = value->value.data;

      if (value->value.type == &UA_TYPES[UA_TYPES_BOOLEAN])
        response->type = Boolean;
      else if(value->value.type == &UA_TYPES[UA_TYPES_INT16]) {
        response->type = Int16;
      } else if(value->value.type == &UA_TYPES[UA_TYPES_UINT16]) {
        response->type = UInt16;
      } else if(value->value.type == &UA_TYPES[UA_TYPES_INT32]) {
        response->type = Int32;
      } else if(value->value.type == &UA_TYPES[UA_TYPES_UINT32]) {
        response->type = UInt32;
      } else if(value->value.type == &UA_TYPES[UA_TYPES_INT64]) {
        response->type = Int64;
      } else if(value->value.type == &UA_TYPES[UA_TYPES_UINT64]) {
        response->type = UInt64;
      } else if(value->value.type == &UA_TYPES[UA_TYPES_FLOAT]) {
        response->type = Float;
      } else if(value->value.type == &UA_TYPES[UA_TYPES_DOUBLE]) {
        response->type = Double;
      } else if(value->value.type == &UA_TYPES[UA_TYPES_STRING]) {
        UA_String str = *((UA_String*) value->value.data);
        versatility->value = (void*) str.data;
        response->type = String;
      } else if(value->value.type == &UA_TYPES[UA_TYPES_BYTE]) {
        response->type = Byte;
      } else if(value->value.type == &UA_TYPES[UA_TYPES_DATETIME]) {
        response->type = DateTime;
      }
      response->message = versatility;


      EdgeMessage *resultMsg = (EdgeMessage*) malloc(sizeof(EdgeMessage));
      resultMsg->endpointInfo = (EdgeEndPointInfo*) malloc(sizeof(EdgeEndPointInfo));
      memcpy(resultMsg->endpointInfo, subInfo->msg->endpointInfo, sizeof(EdgeEndPointInfo));
      resultMsg->type = REPORT;
      resultMsg->responseLength = 1;
      resultMsg->responses = (EdgeResponse**) malloc(1 * sizeof(EdgeResponse));
      resultMsg->responses[0] =response;

      onResponseMessage(resultMsg);

      free(response->nodeInfo); response->nodeInfo = NULL;
      free(response->message); response->message = NULL;
      free(response); response = NULL;
      free (resultMsg->endpointInfo); resultMsg->endpointInfo = NULL;
      free (resultMsg->responses);resultMsg->responses = NULL;
      free(resultMsg); resultMsg = NULL;
    }
  }
}

static void* subscription_thread_handler(void* ptr) {
  printf(">>>>>>>>>>>>>>>>>> subscription thread created <<<<<<<<<<<<<<<<<<<< \n");
  UA_Client *client = (UA_Client*) ptr;
  subscription_thread_running = true;
  while (subscription_thread_running) {
    sendPublishRequest(client);
    sleep(1);
  }
  printf(">>>>>>>>>>>>>>>>>> subscription thread destroyed <<<<<<<<<<<<<<<<<<<< \n");
  return NULL;
}

static UA_StatusCode createSub(UA_Client *client, EdgeMessage *msg) {
  EdgeSubRequest *subReq = msg->request->subMsg;

  UA_UInt32 subId = 0;
  UA_SubscriptionSettings settings = {
    subReq->publishingInterval, /* .requestedPublishingInterval */
    subReq->lifetimeCount, /* .requestedLifetimeCount */
    subReq->maxKeepAliveCount, /* .requestedMaxKeepAliveCount */
    subReq->maxNotificationsPerPublish, /* .maxNotificationsPerPublish */
    subReq->publishingEnabled, /* .publishingEnabled */
    subReq->priority /* .priority */
  };

  /* Create a subscription */
  UA_StatusCode retSub = UA_Client_Subscriptions_new(client, settings, &subId);
  if(!subId) {
    // TODO: Handle Error
    printf("Error in creating subscription :: %s\n\n", UA_StatusCode_name(retSub));
    return retSub;
  }

  EdgeMessage *msgCopy = (EdgeMessage*) malloc(sizeof(EdgeMessage));
  memcpy(msgCopy, msg, sizeof *msg);

  /* Add a MonitoredItem */
  UA_NodeId monitorThis = UA_NODEID_STRING(2, msg->request->nodeInfo->valueAlias);
  UA_UInt32 monId = 0;
  UA_StatusCode retMon = UA_Client_Subscriptions_addMonitoredItem(client, subId, monitorThis, UA_ATTRIBUTEID_VALUE,
                                           &monitoredItemHandler, msgCopy->request->nodeInfo->valueAlias, &monId, subReq->samplingInterval);
  if (monId) {
    printf("Monitoring 'the.answer', id %u\n", subId);
  } else {
    // TODO: Handle Error
    printf("Error in adding monitored item to subscription :: %s\n", UA_StatusCode_name(retMon));
    return retMon;
  }

  if (0 == subscriptionCount) {
    /* initiate thread for manually sending publish request. */
    pthread_create(&subscription_thread, NULL, &subscription_thread_handler, (void*) client);
  }
  subscriptionCount++;

  subscriptionInfo *subInfo = (subscriptionInfo*) malloc(sizeof(subscriptionInfo));
  subInfo->msg = msgCopy;
  subInfo->subId = subId;
  subInfo->monId = monId;

  if (!subscriptionList) {
    /* Create subscription list */
    subscriptionList = createMap();
  }
  if (subscriptionList)
    insertMapElement(subscriptionList, (keyValue) msg->request->nodeInfo->valueAlias, (keyValue) subInfo);

  return UA_STATUSCODE_GOOD;
}

static UA_StatusCode deleteSub(UA_Client *client, EdgeMessage *msg) {

  subscriptionInfo* subInfo = (subscriptionInfo*) getSubscriptionId(msg->request->nodeInfo->valueAlias);
  //printf("subscription id retrieved from map :: %d \n\n", subInfo->subId);

  if (!subInfo) {
    printf("not subscribed yet \n");
    return UA_STATUSCODE_BADNOSUBSCRIPTION;
  }

  UA_StatusCode retVal = UA_Client_Subscriptions_remove(client, subInfo->subId);

  if (UA_STATUSCODE_GOOD == retVal) {
    printf("subscription deleted successfully\n\n");
   edgeMapNode* removed = removeSubscriptionFromMap(msg->request->nodeInfo->valueAlias);
   if (!removed) {
     subscriptionInfo* info = (subscriptionInfo*) removed->value;
     if (info) {
       free (info->msg); info->msg = NULL;
       free (info); info = NULL;
     }
   }
  } else {
    printf("subscription delete error : %s\n\n", UA_StatusCode_name(retVal));
    return retVal;
  }

  subscriptionCount--;

  if (0 == subscriptionCount) {
    /* destroy the subscription thread */
    /* delete the subscription thread as there are no existing subscriptions request */
    subscription_thread_running = false;
    pthread_cancel(subscription_thread);
  }

  return UA_STATUSCODE_GOOD;
}

static UA_StatusCode modifySub(UA_Client *client, EdgeMessage *msg) {
  subscriptionInfo* subInfo =  (subscriptionInfo*) getSubscriptionId(msg->request->nodeInfo->valueAlias);
  //printf("subscription id retrieved from map :: %d \n\n", subInfo->subId);

  if (!subInfo) {
    printf("not subscribed yet\n");
    return UA_STATUSCODE_BADNOSUBSCRIPTION;
  }

  EdgeSubRequest *subReq = msg->request->subMsg;

  UA_ModifySubscriptionRequest modifySubscriptionRequest;
  UA_ModifySubscriptionRequest_init(&modifySubscriptionRequest);
  modifySubscriptionRequest.subscriptionId = subInfo->subId;
  modifySubscriptionRequest.maxNotificationsPerPublish = subReq->maxNotificationsPerPublish;
  modifySubscriptionRequest.priority = subReq->priority;
  modifySubscriptionRequest.requestedLifetimeCount = subReq->lifetimeCount;
  modifySubscriptionRequest.requestedMaxKeepAliveCount = subReq->maxKeepAliveCount;
  modifySubscriptionRequest.requestedPublishingInterval = subReq->publishingInterval;

  UA_ModifySubscriptionResponse response = UA_Client_Service_modifySubscription(client, modifySubscriptionRequest);
  if (response.responseHeader.serviceResult != UA_STATUSCODE_GOOD) {
    printf ("Error in modify subscription :: %s\n\n", UA_StatusCode_name(response.responseHeader.serviceResult));
    return response.responseHeader.serviceResult;
  } else {
    printf("modify suhscription success\n\n");
  }
  UA_ModifySubscriptionRequest_deleteMembers(&modifySubscriptionRequest);

  // modifyMonitoredItems
  UA_ModifyMonitoredItemsRequest modifyMonitoredItemsRequest;
  UA_ModifyMonitoredItemsRequest_init(&modifyMonitoredItemsRequest);
  modifyMonitoredItemsRequest.subscriptionId = subInfo->subId;
  modifyMonitoredItemsRequest.itemsToModifySize = 1;
  modifyMonitoredItemsRequest.itemsToModify = UA_malloc(sizeof(UA_MonitoredItemModifyRequest));
  modifyMonitoredItemsRequest.itemsToModify[0].monitoredItemId = 1;   //monId;
//  UA_MonitoringParameters parameters = modifyMonitoredItemsRequest.itemsToModify[0].requestedParameters;
  UA_MonitoringParameters_init(&modifyMonitoredItemsRequest.itemsToModify[0].requestedParameters);
  (modifyMonitoredItemsRequest.itemsToModify[0].requestedParameters).clientHandle = (UA_UInt32) 1;
  (modifyMonitoredItemsRequest.itemsToModify[0].requestedParameters).discardOldest = true;
  (modifyMonitoredItemsRequest.itemsToModify[0].requestedParameters).samplingInterval = subReq->samplingInterval;
  (modifyMonitoredItemsRequest.itemsToModify[0].requestedParameters).queueSize = subReq->queueSize;
  UA_ModifyMonitoredItemsResponse modifyMonitoredItemsResponse;
  __UA_Client_Service(client, &modifyMonitoredItemsRequest, &UA_TYPES[UA_TYPES_MODIFYMONITOREDITEMSREQUEST],
                &modifyMonitoredItemsResponse, &UA_TYPES[UA_TYPES_MODIFYMONITOREDITEMSRESPONSE]);
  if (UA_STATUSCODE_GOOD == modifyMonitoredItemsResponse.responseHeader.serviceResult) {
    printf("modify monitored item success\n\n");
  } else {
    printf("modify monitored item failed :: %s\n\n", UA_StatusCode_name(
        modifyMonitoredItemsResponse.responseHeader.serviceResult));
    return modifyMonitoredItemsResponse.responseHeader.serviceResult;
  }
  UA_ModifyMonitoredItemsRequest_deleteMembers(&modifyMonitoredItemsRequest);
  UA_ModifyMonitoredItemsResponse_deleteMembers(&modifyMonitoredItemsResponse);

  // setMonitoringMode
  UA_SetMonitoringModeRequest setMonitoringModeRequest;
  UA_SetMonitoringModeRequest_init(&setMonitoringModeRequest);
  setMonitoringModeRequest.subscriptionId = subInfo->subId;
  setMonitoringModeRequest.monitoredItemIdsSize = 1;
  setMonitoringModeRequest.monitoredItemIds = UA_malloc(sizeof(UA_UInt32));
  setMonitoringModeRequest.monitoredItemIds[0] = 1; //monId;
  setMonitoringModeRequest.monitoringMode = UA_MONITORINGMODE_REPORTING;
  UA_SetMonitoringModeResponse setMonitoringModeResponse;
  __UA_Client_Service(client, &setMonitoringModeRequest, &UA_TYPES[UA_TYPES_SETMONITORINGMODEREQUEST],
                      &setMonitoringModeResponse, &UA_TYPES[UA_TYPES_SETMONITORINGMODERESPONSE]);
  if (UA_STATUSCODE_GOOD == setMonitoringModeResponse.responseHeader.serviceResult) {
    printf("set monitor mode success\n\n");
  } else {
    printf("set monitor mode failed :: %s\n\n", UA_StatusCode_name(
        setMonitoringModeResponse.responseHeader.serviceResult));
    return setMonitoringModeResponse.responseHeader.serviceResult;
  }
  UA_SetMonitoringModeRequest_deleteMembers(&setMonitoringModeRequest);
  UA_SetMonitoringModeResponse_deleteMembers(&setMonitoringModeResponse);

  // setPublishingMode
  UA_SetPublishingModeRequest setPublishingModeRequest;
  UA_SetPublishingModeRequest_init(&setPublishingModeRequest);
  setPublishingModeRequest.subscriptionIdsSize = 1;
  setPublishingModeRequest.subscriptionIds = UA_malloc(sizeof(UA_UInt32));
  setPublishingModeRequest.subscriptionIds[0] = subInfo->subId;
  setPublishingModeRequest.publishingEnabled = subReq->publishingEnabled;     //UA_TRUE;
  UA_SetPublishingModeResponse setPublishingModeResponse;
  __UA_Client_Service(client, &setPublishingModeRequest, &UA_TYPES[UA_TYPES_SETPUBLISHINGMODEREQUEST],
                      &setPublishingModeResponse, &UA_TYPES[UA_TYPES_SETPUBLISHINGMODERESPONSE]);
  if (UA_STATUSCODE_GOOD ==  setPublishingModeResponse.responseHeader.serviceResult) {
    printf("set publish mode success\n\n");
  } else {
    printf("set publish mode failed :: %s\n\n", UA_StatusCode_name(
        setPublishingModeResponse.responseHeader.serviceResult));
    return setPublishingModeResponse.responseHeader.serviceResult;
  }
  UA_SetPublishingModeRequest_deleteMembers(&setPublishingModeRequest);
  UA_SetPublishingModeResponse_deleteMembers(&setPublishingModeResponse);


  UA_Client_Subscriptions_manuallySendPublishRequest(client);

  return UA_STATUSCODE_GOOD;
}

EdgeResult executeSub(UA_Client *client, EdgeMessage *msg) {
  printf("execute subscription\n");
  EdgeResult result;
  if (!client) {
    printf("client handle invalid!\n");
    result.code = STATUS_ERROR;
    return result;
  }

  UA_StatusCode retVal =  UA_STATUSCODE_GOOD;
  EdgeRequest *req = msg->request;
  EdgeSubRequest *subReq = req->subMsg;
  if (subReq->subType == Edge_Create_Sub) {
    retVal = createSub(client, msg);
  } else if (subReq->subType == Edge_Modify_Sub) {
    retVal = modifySub(client, msg);
  } else if (subReq->subType == Edge_Delete_Sub) {
    retVal = deleteSub(client, msg);
  }

    if(retVal == UA_STATUSCODE_GOOD)
    {
        result.code = STATUS_OK;
    }
    else
    {
        result.code = STATUS_ERROR;
    }
  
  return result;
}
