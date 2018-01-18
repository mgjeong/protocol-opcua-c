#include "subscription.h"
#include "edge_utils.h"

#include <stdio.h>
#include <pthread.h>
#include <unistd.h>

typedef struct subscriptionInfo
{
    EdgeMessage *msg;
    int subId;
    UA_UInt32 monId;
} subscriptionInfo;

static int subscriptionCount = 0;
static bool subscription_thread_running = false;
static pthread_t subscription_thread;

static edgeMap *subscriptionList;

static bool validSubscriptionId(UA_UInt32 subId)
{
    if (subscriptionList)
    {
        edgeMapNode *temp = subscriptionList->head;
        while (temp != NULL)
        {
            subscriptionInfo *subInfo = (subscriptionInfo *)temp->value;  

            if (subInfo->subId == subId)
                return false;

            temp = temp->next;
        }
    }

    return true;
}

static keyValue getSubscriptionInfo(char *valueAlias)
{
    edgeMapNode *temp = subscriptionList->head;
    while (temp != NULL)
    {
        if (!strcmp(temp->key, valueAlias))
        {
            return temp->value;
        }
        temp = temp->next;
    }
    return NULL;
}

static edgeMapNode *removeSubscriptionFromMap(char *valueAlias)
{
    edgeMapNode *temp = subscriptionList->head;
    edgeMapNode *prev = NULL;
    while (temp != NULL)
    {
        if (!strcmp(temp->key, valueAlias))
        {
            if (prev == NULL)
            {
                subscriptionList->head = temp->next;
            }
            else
            {
                prev->next = temp->next;
            }
            
            return temp;
        }
        prev = temp;
        temp = temp->next;
    }
    return NULL;
}

#ifdef TEST_SUBSCRIPTION_LIST
static void printMap()
{
    edgeMapNode *temp = subscriptionList->head;
    while (temp != NULL)
    {
        temp = temp->next;
    }
}
#endif


void sendPublishRequest(UA_Client *client)
{
    UA_Client_Subscriptions_manuallySendPublishRequest(client);
}

static void
monitoredItemHandler(UA_UInt32 monId, UA_DataValue *value, void *context)
{

    if (value->status != UA_STATUSCODE_GOOD)
    {
        printf("ERROR :: Received Value Status Code %s\n", UA_StatusCode_name(value->status));
        return;
    }

    if ( value->hasValue)
    {
        printf("value is present, monId :: %d\n", monId);

        char *valueAlias = (char *) context;
        subscriptionInfo *subInfo =  (subscriptionInfo *) getSubscriptionInfo(valueAlias);

        if (!subInfo)
            return ;

        sleep(1);

        EdgeResponse *response = (EdgeResponse *) malloc(sizeof(EdgeResponse));
        if (response)
        {
            response->nodeInfo = (EdgeNodeInfo *) malloc(sizeof(EdgeNodeInfo));
            //memcpy(response->nodeInfo, subInfo->msg->request->nodeInfo, sizeof(EdgeNodeInfo));
            response->nodeInfo->valueAlias = (char *) malloc(strlen(valueAlias) + 1);
            strcpy(response->nodeInfo->valueAlias, valueAlias);
            if (subInfo->msg->requests != NULL)
                printf("msg->requests VALID here\n");

            //response->requestId = subInfo->msg->request->requestId;


            EdgeVersatility *versatility = (EdgeVersatility *) malloc(sizeof(EdgeVersatility));
            versatility->arrayLength = 0;
            versatility->isArray = false;
            versatility->value = value->value.data;

            if (value->value.type == &UA_TYPES[UA_TYPES_BOOLEAN])
                response->type = Boolean;
            else if (value->value.type == &UA_TYPES[UA_TYPES_INT16])
            {
                response->type = Int16;
            }
            else if (value->value.type == &UA_TYPES[UA_TYPES_UINT16])
            {
                response->type = UInt16;
            }
            else if (value->value.type == &UA_TYPES[UA_TYPES_INT32])
            {
                response->type = Int32;
            }
            else if (value->value.type == &UA_TYPES[UA_TYPES_UINT32])
            {
                response->type = UInt32;
            }
            else if (value->value.type == &UA_TYPES[UA_TYPES_INT64])
            {
                response->type = Int64;
            }
            else if (value->value.type == &UA_TYPES[UA_TYPES_UINT64])
            {
                response->type = UInt64;
            }
            else if (value->value.type == &UA_TYPES[UA_TYPES_FLOAT])
            {
                response->type = Float;
            }
            else if (value->value.type == &UA_TYPES[UA_TYPES_DOUBLE])
            {
                response->type = Double;
            }
            else if (value->value.type == &UA_TYPES[UA_TYPES_STRING])
            {
                UA_String str = *((UA_String *) value->value.data);
                versatility->value = (void *) str.data;
                response->type = String;
            }
            else if (value->value.type == &UA_TYPES[UA_TYPES_BYTE])
            {
                response->type = Byte;
            }
            else if (value->value.type == &UA_TYPES[UA_TYPES_DATETIME])
            {
                response->type = DateTime;
            }
            response->message = versatility;

            EdgeMessage *resultMsg = (EdgeMessage *) malloc(sizeof(EdgeMessage));
            resultMsg->endpointInfo = (EdgeEndPointInfo *) malloc(sizeof(EdgeEndPointInfo));
            if (subInfo->msg != NULL)
                memcpy(resultMsg->endpointInfo, subInfo->msg->endpointInfo, sizeof(EdgeEndPointInfo));
            else
            {
                printf("Failing here 2\n"); return;
            }
            resultMsg->type = REPORT;
            resultMsg->responseLength = 1;
            resultMsg->responses = (EdgeResponse **) malloc(1 * sizeof(EdgeResponse));

            resultMsg->responses[0] = response;

            onResponseMessage(resultMsg);

            free(response->nodeInfo->valueAlias); response->nodeInfo->valueAlias = NULL;
            free(response->nodeInfo); response->nodeInfo = NULL;
            free(response->message); response->message = NULL;
            free(response); response = NULL;
            free (resultMsg->responses); resultMsg->responses = NULL;
            free (resultMsg->endpointInfo); resultMsg->endpointInfo = NULL;
            free(resultMsg); resultMsg = NULL;
        }
    }
}

static void *subscription_thread_handler(void *ptr)
{
    printf(">>>>>>>>>>>>>>>>>> subscription thread created <<<<<<<<<<<<<<<<<<<< \n");
    UA_Client *client = (UA_Client *) ptr;
    subscription_thread_running = true;
    while (subscription_thread_running)
    {
        sendPublishRequest(client);
        sleep(1);
    }
    printf(">>>>>>>>>>>>>>>>>> subscription thread destroyed <<<<<<<<<<<<<<<<<<<< \n");
    return NULL;
}

static UA_StatusCode createSub(UA_Client *client, EdgeMessage *msg)
{
    EdgeSubRequest *subReq;
    if (msg->type == SEND_REQUESTS)
    {
        EdgeRequest **req = msg->requests;
        subReq = req[0]->subMsg;
    }
    else
    {
        EdgeRequest *req = msg->request;
        subReq = req->subMsg;
    }

    UA_UInt32 subId = 0;
    UA_SubscriptionSettings settings =
    {
        subReq->publishingInterval, /* .requestedPublishingInterval */
        subReq->lifetimeCount, /* .requestedLifetimeCount */
        subReq->maxKeepAliveCount, /* .requestedMaxKeepAliveCount */
        subReq->maxNotificationsPerPublish, /* .maxNotificationsPerPublish */
        subReq->publishingEnabled, /* .publishingEnabled */
        subReq->priority /* .priority */
    };

    /* Create a subscription */
    UA_StatusCode retSub = UA_Client_Subscriptions_new(client, settings, &subId);
    if (!subId)
    {
        // TODO: Handle Error
        printf("Error in creating subscription :: %s\n\n", UA_StatusCode_name(retSub));
        return retSub;
    }
    printf("Subscription ID received is %u\n", subId);

    if (!validSubscriptionId(subId))
    {
        printf("ERROR :: Subscription ID received is already in use, Please unsubcribe and try again %s\n",
               UA_StatusCode_name(UA_STATUSCODE_BADSUBSCRIPTIONIDINVALID));
        return UA_STATUSCODE_BADSUBSCRIPTIONIDINVALID;
    }

    EdgeMessage *msgCopy = (EdgeMessage *) malloc(sizeof(EdgeMessage));
    memcpy(msgCopy, msg, sizeof * msg);

    int itemSize = msgCopy->requestLength;
    UA_MonitoredItemCreateRequest *items = (UA_MonitoredItemCreateRequest *) malloc(
            sizeof(UA_MonitoredItemCreateRequest) * itemSize);
    UA_UInt32 *monId = (UA_UInt32 *) malloc(sizeof(UA_UInt32) * itemSize);
    UA_StatusCode *itemResults = (UA_StatusCode *) malloc(sizeof(UA_StatusCode) * itemSize);
    UA_MonitoredItemHandlingFunction *hfs = (UA_MonitoredItemHandlingFunction *) malloc(
            sizeof(UA_MonitoredItemHandlingFunction) * itemSize);
    char **hfContexts = (char **) malloc(sizeof(char *) * itemSize);

    for (int i = 0; i < itemSize; i++)
    {
        monId[i] = 0;
        hfs[i] = &monitoredItemHandler;
        hfContexts[i] = (char *)malloc(20);
        strcpy(hfContexts[i], msgCopy->requests[i]->nodeInfo->valueAlias);
        hfContexts[strlen(msgCopy->requests[i]->nodeInfo->valueAlias)] = '\0';
        UA_MonitoredItemCreateRequest_init(&items[i]);
        items[i].itemToMonitor.nodeId = UA_NODEID_STRING(1, msgCopy->requests[i]->nodeInfo->valueAlias);
        items[i].itemToMonitor.attributeId = UA_ATTRIBUTEID_VALUE;
        items[i].monitoringMode = UA_MONITORINGMODE_REPORTING;
        items[i].requestedParameters.samplingInterval = msgCopy->requests[i]->subMsg->samplingInterval;
        items[i].requestedParameters.discardOldest = true;
        items[i].requestedParameters.queueSize = 1;
    }


    UA_StatusCode retMon =   UA_Client_Subscriptions_addMonitoredItems(client, subId, items, itemSize,
                             hfs, (void **)hfContexts, itemResults, monId);

    for (int i = 0; i < itemSize; i++)
    {
        if (monId[i])
        {
            printf("Monitoring for item : %d  is %u\n", i, monId[i]);
        }
        else
        {
            // TODO: Handle Error
            printf("ERROR : INVALID Monitoring ID Recevived for item :: #%d,  Error : %d\n", i, retMon);
            return UA_STATUSCODE_BADMONITOREDITEMIDINVALID;
        }
    }

    if (0 == subscriptionCount)
    {
        /* initiate thread for manually sending publish request. */
        pthread_create(&subscription_thread, NULL, &subscription_thread_handler, (void *) client);
    }
    subscriptionCount++;

    if (!subscriptionList)
    {
        /* Create subscription list */
        subscriptionList = createMap();
    }
    if (subscriptionList)
    {
        for (int i = 0; i < itemSize; i++)
        {
            subscriptionInfo *subInfo = (subscriptionInfo *) malloc(sizeof(subscriptionInfo));
            subInfo->msg = msgCopy;
            subInfo->subId = subId;
            subInfo->monId = monId[i];
            printf("Inserting MAP ELEMENT /nvalueAlias :: %s \n",
                   msgCopy->requests[i]->nodeInfo->valueAlias);
            insertMapElement(subscriptionList, (keyValue) hfContexts[i],
                             (keyValue) subInfo);
        }
    }

    return UA_STATUSCODE_GOOD;
}

static UA_StatusCode deleteSub(UA_Client *client, EdgeMessage *msg)
{
    subscriptionInfo *subInfo = (subscriptionInfo *) getSubscriptionInfo(
                                    msg->request->nodeInfo->valueAlias);

    if (!subInfo)
    {
        printf("not subscribed yet \n");
        return UA_STATUSCODE_BADNOSUBSCRIPTION;
    }

    printf("Deleting following Subscription \n");
    printf("Node name :: %s\n", (char *)msg->request->nodeInfo->valueAlias);
    printf("SUB ID :: %d\n", subInfo->subId);
    printf("MON ID :: %d\n", subInfo->monId);

    UA_StatusCode ret = UA_Client_Subscriptions_removeMonitoredItem(client,
        subInfo->subId, subInfo->monId);

    
    if (UA_STATUSCODE_GOOD != ret)
    {
        printf("Error in removing monitored item : MON ID %d \n", subInfo->monId);
        return ret;
    }
    else 
    {
        printf("Monitoring deleted successfully\n\n");
        edgeMapNode *removed = removeSubscriptionFromMap(msg->request->nodeInfo->valueAlias);
        if (removed != NULL)
        {
            subscriptionInfo *info = (subscriptionInfo *) removed->value;
            if (info)
            {
                free (info->msg); info->msg = NULL;
                free (info); info = NULL;
            }
            free (removed); removed = NULL;
        }
    }

    if(validSubscriptionId(subInfo->subId))
    {
        printf("Removing the subscription  SID %d \n", subInfo->subId);
        UA_StatusCode retVal = UA_Client_Subscriptions_remove(client, subInfo->subId);
        if (UA_STATUSCODE_GOOD != retVal)
        {
            printf("Error in removing subscription  SID %d \n", subInfo->subId);
            return retVal;
        }
        
        subscriptionCount--;

        if (0 == subscriptionCount)
        {
            /* destroy the subscription thread */
            /* delete the subscription thread as there are no existing subscriptions request */
            printf("subscription thread destroy\n");
            subscription_thread_running = false;
            pthread_join(subscription_thread, NULL);
        }
    }
    else
    {
         printf("Not removing the subscription  SID %d \nOther monitored items present\n", subInfo->subId);
    }
    
    return UA_STATUSCODE_GOOD;
}

static UA_StatusCode modifySub(UA_Client *client, EdgeMessage *msg)
{
    subscriptionInfo *subInfo =  (subscriptionInfo *) getSubscriptionInfo(
                                     msg->request->nodeInfo->valueAlias);
    //printf("subscription id retrieved from map :: %d \n\n", subInfo->subId);

    if (!subInfo)
    {
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

    UA_ModifySubscriptionResponse response = UA_Client_Service_modifySubscription(client,
            modifySubscriptionRequest);
    if (response.responseHeader.serviceResult != UA_STATUSCODE_GOOD)
    {
        printf ("Error in modify subscription :: %s\n\n",
                UA_StatusCode_name(response.responseHeader.serviceResult));
        return response.responseHeader.serviceResult;
    }
    else
    {
        printf("modify subscription success\n\n");
    }

    if (response.revisedPublishingInterval != subReq->publishingInterval)
    {
        printf("Publishing Interval Changed in the Response ");
        printf("Requested Interval:: %f Response Interval:: %f \n", subReq->publishingInterval,
               response.revisedPublishingInterval);
    }

    UA_ModifySubscriptionRequest_deleteMembers(&modifySubscriptionRequest);

    // modifyMonitoredItems
    UA_ModifyMonitoredItemsRequest modifyMonitoredItemsRequest;
    UA_ModifyMonitoredItemsRequest_init(&modifyMonitoredItemsRequest);
    modifyMonitoredItemsRequest.subscriptionId = subInfo->subId;
    modifyMonitoredItemsRequest.itemsToModifySize = 1;
    modifyMonitoredItemsRequest.itemsToModify = UA_malloc(sizeof(UA_MonitoredItemModifyRequest));

    UA_UInt32 monId = 1;

    if (subscriptionList)
    {
        subscriptionInfo *subInfo = (subscriptionInfo *) getSubscriptionInfo(
                                        msg->request->nodeInfo->valueAlias);

        if (subInfo != NULL)
            monId = subInfo->monId;
        else
            printf("Subinfo is NULL \n");

        printf("Mon ID for request ::  %u\n", monId);
    }

    modifyMonitoredItemsRequest.itemsToModify[0].monitoredItemId = monId;   //monId;
    //UA_MonitoringParameters parameters = modifyMonitoredItemsRequest.itemsToModify[0].requestedParameters;
    UA_MonitoringParameters_init(&modifyMonitoredItemsRequest.itemsToModify[0].requestedParameters);
    (modifyMonitoredItemsRequest.itemsToModify[0].requestedParameters).clientHandle = (UA_UInt32) 1;
    (modifyMonitoredItemsRequest.itemsToModify[0].requestedParameters).discardOldest = true;
    (modifyMonitoredItemsRequest.itemsToModify[0].requestedParameters).samplingInterval =
        subReq->samplingInterval;
    (modifyMonitoredItemsRequest.itemsToModify[0].requestedParameters).queueSize = subReq->queueSize;
    UA_ModifyMonitoredItemsResponse modifyMonitoredItemsResponse;
    __UA_Client_Service(client, &modifyMonitoredItemsRequest,
                        &UA_TYPES[UA_TYPES_MODIFYMONITOREDITEMSREQUEST],
                        &modifyMonitoredItemsResponse, &UA_TYPES[UA_TYPES_MODIFYMONITOREDITEMSRESPONSE]);
    if (UA_STATUSCODE_GOOD == modifyMonitoredItemsResponse.responseHeader.serviceResult)
    {


        UA_MonitoredItemModifyResult result = *modifyMonitoredItemsResponse.results;

        for (size_t index = 0; index < modifyMonitoredItemsResponse.resultsSize; index++)
        {
            printf("Result [%d] modify monitoreditem :: %s\n\n", (int) index + 1, UA_StatusCode_name(
                       modifyMonitoredItemsResponse.results[index].statusCode));

            if (modifyMonitoredItemsResponse.results[index].statusCode != UA_STATUSCODE_GOOD)
                return modifyMonitoredItemsResponse.results[index].statusCode;
        }
        printf("modify monitored item success\n\n");

        if (result.revisedQueueSize != subReq->queueSize)
        {
            printf("WARNING : Revised Queue Size in Response MISMATCH\n\n");
            printf("Result Queue Size : %u\n", result.revisedQueueSize);
            printf("Queue Size : %u\n", subReq->queueSize);
        }


        if (result.revisedSamplingInterval != subReq->samplingInterval)
        {
            printf("WARNING : Revised Sampling Interval in Response MISMATCH\n\n");
            printf(" Result Sampling Interval %f\n", result.revisedSamplingInterval);
            printf(" Sampling Interval %f\n", subReq->samplingInterval);
        }

    }
    else
    {
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
    if (UA_STATUSCODE_GOOD == setMonitoringModeResponse.responseHeader.serviceResult)
    {
        printf("set monitor mode success\n\n");
    }
    else
    {
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
    if (UA_STATUSCODE_GOOD !=  setPublishingModeResponse.responseHeader.serviceResult)
    {
        printf("set publish mode failed :: %s\n\n", UA_StatusCode_name(
                   setPublishingModeResponse.responseHeader.serviceResult));
        return setPublishingModeResponse.responseHeader.serviceResult;
    }

    bool publishFail = false;
    for (size_t index = 0; index < setPublishingModeResponse.resultsSize; index++)
    {
        if (setPublishingModeResponse.results[index] != UA_STATUSCODE_GOOD)
            publishFail = true;

        printf("Result [%d] set publish mode :: %s\n\n", (int) index + 1, UA_StatusCode_name(
                   setPublishingModeResponse.results[index]));
    }

    if (publishFail)
    {
        printf("ERROR :: Set publish mode failed :: %s\n\n",
               UA_StatusCode_name(UA_STATUSCODE_BADMONITOREDITEMIDINVALID));
        return UA_STATUSCODE_BADMONITOREDITEMIDINVALID;
    }


    printf("set publish mode success\n\n");

    UA_SetPublishingModeRequest_deleteMembers(&setPublishingModeRequest);
    UA_SetPublishingModeResponse_deleteMembers(&setPublishingModeResponse);


    UA_Client_Subscriptions_manuallySendPublishRequest(client);

    return UA_STATUSCODE_GOOD;
}

static UA_StatusCode rePublish(UA_Client *client, EdgeMessage *msg)
{
    subscriptionInfo *subInfo =  (subscriptionInfo *) getSubscriptionInfo(
                                     msg->request->nodeInfo->valueAlias);
    //printf("subscription id retrieved from map :: %d \n\n", subInfo->subId);

    if (!subInfo)
    {
        printf("not subscribed yet\n");
        return UA_STATUSCODE_BADNOSUBSCRIPTION;
    }

    // re PublishingMode
    UA_RepublishRequest republishRequest;
    UA_RepublishRequest_init(&republishRequest);
    republishRequest.retransmitSequenceNumber = 2;
    republishRequest.subscriptionId = subInfo->subId;

    UA_RepublishResponse republishResponse;
    __UA_Client_Service(client, &republishRequest, &UA_TYPES[UA_TYPES_REPUBLISHREQUEST],
                        &republishResponse, &UA_TYPES[UA_TYPES_REPUBLISHRESPONSE]);

    if (UA_STATUSCODE_GOOD !=  republishResponse.responseHeader.serviceResult)
    {
        if (UA_STATUSCODE_BADMESSAGENOTAVAILABLE == republishResponse.responseHeader.serviceResult)
            printf("No Message in republish response");
        else
        {
            printf("re publish failed :: %s\n\n",
                   UA_StatusCode_name(republishResponse.responseHeader.serviceResult));
            return republishResponse.responseHeader.serviceResult;
        }
    }

    if (republishResponse.notificationMessage.notificationDataSize != 0)
        printf("Re publish Response Sequence number :: %u \n",
               republishResponse.notificationMessage.sequenceNumber);
    else
        printf("Re publish Response has NULL notification Message\n");

    UA_RepublishRequest_deleteMembers(&republishRequest);
    UA_RepublishResponse_deleteMembers(&republishResponse);

    return UA_STATUSCODE_GOOD;
}

EdgeResult executeSub(UA_Client *client, EdgeMessage *msg)
{
    EdgeResult result;
    if (!client)
    {
        printf("client handle invalid!\n");
        result.code = STATUS_ERROR;
        return result;
    }

    EdgeSubRequest *subReq;

    UA_StatusCode retVal =  UA_STATUSCODE_GOOD;
    if (msg->type == SEND_REQUESTS)
    {
        EdgeRequest **req = msg->requests;
        subReq = req[0]->subMsg;
    }
    else
    {
        EdgeRequest *req = msg->request;
        subReq = req->subMsg;
    }

    if (subReq->subType == Edge_Create_Sub)
    {
        retVal = createSub(client, msg);
    }
    else if (subReq->subType == Edge_Modify_Sub)
    {
        retVal = modifySub(client, msg);
    }
    else if (subReq->subType == Edge_Delete_Sub)
    {
        retVal = deleteSub(client, msg);
    }
    else if (subReq->subType == Edge_Republish_Sub)
    {
        retVal = rePublish(client, msg);
    }

    if (retVal == UA_STATUSCODE_GOOD)
    {
        result.code = STATUS_OK;
    }
    else
    {
        result.code = STATUS_ERROR;
    }

    return result;
}
