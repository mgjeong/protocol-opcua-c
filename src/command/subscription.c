/******************************************************************
 *
 * Copyright 2017 Samsung Electronics All Rights Reserved.
 *
 *
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *      http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 *
 ******************************************************************/

#include "subscription.h"
#include "edge_logger.h"
#include "edge_malloc.h"
#include "message_dispatcher.h"

#include <pthread.h>
#include <unistd.h>

#define TAG "subscription"

#define EDGE_UA_SUBSCRIPTION_ITEM_SIZE (20)
#define EDGE_UA_MINIMUM_PUBLISHING_TIME (100)
#define DEFAULT_RETRANSMIT_SEQUENCENUM (2)
#define GUID_LENGTH (36)

typedef struct subscriptionInfo
{
    EdgeMessage *msg;
    UA_UInt32 subId;
    UA_UInt32 monId;
    void *hfContext;
} subscriptionInfo;

typedef struct clientSubscription
{
    int subscriptionCount;
    pthread_t subscription_thread;
    bool subscription_thread_running;
    edgeMap *subscriptionList;
} clientSubscription;

typedef struct client_valueAlias
{
    UA_Client *client;
    char *valueAlias;
} client_valueAlias;

static edgeMap *clientSubMap  = NULL;

static bool validateMonitoringId(edgeMap *list, UA_UInt32 subId, UA_UInt32 monId)
{
    if (list)
    {
        edgeMapNode *temp = list->head;
        while (temp != NULL)
        {
            subscriptionInfo *subInfo = (subscriptionInfo *) temp->value;

            if (subInfo->subId == subId && subInfo->monId == monId)
                return false;

            temp = temp->next;
        }
    }

    return true;
}

static bool hasSubscriptionId(edgeMap *list, UA_UInt32 subId)
{
    if (list)
    {
        edgeMapNode *temp = list->head;
        while (temp != NULL)
        {
            subscriptionInfo *subInfo = (subscriptionInfo *)temp->value;

            if (subInfo->subId == subId)
                return true;

            temp = temp->next;
        }
    }
    return false;
}

static void* get_subscription_list(UA_Client *client)
{
    if (clientSubMap)
    {
        edgeMapNode *temp = clientSubMap->head;
        while (NULL != temp)
        {
            if (temp->key == client)
            {
                // client valid
                // get all subscription information
                return temp->value;
            }
            temp = temp->next;
        }
    }
    return NULL;
}

static keyValue getSubInfo(edgeMap* list, const char *valueAlias)
{
    if(IS_NOT_NULL(list))
    {
        edgeMapNode *temp = list->head;
        while (temp != NULL)
        {
            if (!strcmp(temp->key, valueAlias))
            {
                return temp->value;
            }
            temp = temp->next;
        }
    }
    return NULL;
}

static edgeMapNode *removeSubFromMap(edgeMap *list, const char *valueAlias)
{
    edgeMapNode *temp = list->head;
    edgeMapNode *prev = NULL;
    while (temp != NULL)
    {
        if (!strcmp(temp->key, valueAlias))
        {
            if (prev == NULL)
            {
                list->head = temp->next;
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

void sendPublishRequest(UA_Client *client)
{
    UA_Client_Subscriptions_manuallySendPublishRequest(client);
}

static void monitoredItemHandler(UA_UInt32 monId, UA_DataValue *value, void *context)
{

    if (value->status != UA_STATUSCODE_GOOD)
    {
        EDGE_LOG_V(TAG, "ERROR :: Received Value Status Code %s\n", UA_StatusCode_name(value->status));
        return;
    }

    if (value->hasValue)
    {
        EDGE_LOG_V(TAG, "value is present, monId :: %d\n", monId);

        client_valueAlias *client_alias = (client_valueAlias*) context;
        char *valueAlias = client_alias->valueAlias;

        subscriptionInfo *subInfo =  NULL;
        clientSubscription *clientSub = NULL;
        clientSub = (clientSubscription*) get_subscription_list(client_alias->client);

        if (!clientSub)
            return ;

        subInfo = (subscriptionInfo *) getSubInfo(clientSub->subscriptionList, client_alias->valueAlias);
        if (!subInfo)
            return;

        EdgeResponse *response = (EdgeResponse *) EdgeCalloc(1, sizeof(EdgeResponse));
        if (IS_NOT_NULL(response))
        {
            response->nodeInfo = (EdgeNodeInfo *) EdgeCalloc(1, sizeof(EdgeNodeInfo));
            if(IS_NULL(response->nodeInfo))
            {
                EDGE_LOG(TAG, "Error : Malloc failed for response->nodeInfo in monitor item handler\n");
                goto EXIT;
            }
            response->nodeInfo->valueAlias = (char *) EdgeMalloc(strlen(valueAlias) + 1);
            if(IS_NULL(response->nodeInfo->valueAlias))
            {
                EDGE_LOG(TAG, "Error : Malloc failed for response->nodeInfo->valueAlias in monitor item handler\n");
                goto EXIT;
            }
            strncpy(response->nodeInfo->valueAlias, valueAlias, strlen(valueAlias));
            response->nodeInfo->valueAlias[strlen(valueAlias)] = '\0';

            // TODO: Handle requestId when implemented
            //if (subInfo->msg != NULL && subInfo->msg->requests != NULL)
                //response->requestId = subInfo->msg->request->requestId;

            EdgeVersatility *versatility = (EdgeVersatility *) EdgeCalloc(1, sizeof(EdgeVersatility));
            if(IS_NULL(versatility))
            {
                EDGE_LOG(TAG, "Error : Malloc failed for versatility in monitor item handler\n");
                goto EXIT;
            }

            bool isScalar = UA_Variant_isScalar(&(value->value));
            if (isScalar)
            {
                versatility->arrayLength = 0;
                versatility->isArray = false;
            }
            else
            {
                versatility->arrayLength = value->value.arrayLength;
                versatility->isArray = true;
            }

            if (value->value.type == &UA_TYPES[UA_TYPES_BOOLEAN])
            {
                response->type = Boolean;
            }
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
                response->type = String;
            }
            else if (value->value.type == &UA_TYPES[UA_TYPES_BYTESTRING])
            {
                response->type = ByteString;
            }
            else if (value->value.type == &UA_TYPES[UA_TYPES_GUID])
            {
                response->type = Guid;
            }
            else if (value->value.type == &UA_TYPES[UA_TYPES_SBYTE])
            {
                response->type = SByte;
            }
            else if (value->value.type == &UA_TYPES[UA_TYPES_BYTE])
            {
                response->type = Byte;
            }
            else if (value->value.type == &UA_TYPES[UA_TYPES_DATETIME])
            {
                response->type = DateTime;
            }

            if (isScalar)
            {
                size_t size = get_size(response->type, false);
                if ((response->type == String) || (response->type == ByteString))
                {
                    UA_String str = *((UA_String *) value->value.data);
                    size_t len = str.length;
                    versatility->value = (void *) EdgeCalloc(1, len+1);
                    strncpy(versatility->value, (char*) str.data, len);
                    ((char*) versatility->value)[(int) len] = '\0';
                }
                else if (response->type == Guid)
                {
                    UA_Guid str = *((UA_Guid *) value->value.data);
                    char *value = (char *) EdgeMalloc(GUID_LENGTH + 1);
                    if(IS_NULL(value))
                    {
                        EDGE_LOG(TAG, "Error : Malloc failed for Guid SCALAR value in Read Group\n");
                        goto EXIT;
                    }
                    snprintf(value, (GUID_LENGTH / 2) + 1, "%08x-%04x-%04x", str.data1,
                            str.data2, str.data3);
                    sprintf(value, "%s-%02x", value, str.data4[0]);
                    sprintf(value, "%s%02x", value, str.data4[1]);
                    sprintf(value, "%s-", value);
                    for (int j = 2; j < 8; j++)
                        sprintf(value, "%s%02x", value, str.data4[j]);
                    value[GUID_LENGTH] = '\0';
                    versatility->value = (void *) value;
                    EDGE_LOG_V(TAG, "%s\n", value);
                }
                else
                {
                    versatility->value = (void *) EdgeCalloc(1, size);
                    memcpy(versatility->value, value->value.data, size);
                }
            }
            else
            {
                size_t size = get_size(response->type, true);
                if (response->type == String)
                {
                    // String Array
                    UA_String *str = ((UA_String *) value->value.data);
                    char **values = (char **) malloc(sizeof(char *) * value->value.arrayLength);
                    if(IS_NULL(values))
                    {
                        EDGE_LOG(TAG, "Error : Malloc failed for String Array values in Read Group\n");
                        goto EXIT;
                    }
                    for (int i = 0; i < value->value.arrayLength; i++)
                    {
                        values[i] = (char *) EdgeMalloc(str[i].length+1);
                        if(IS_NULL(values[i]))
                        {
                            EDGE_LOG_V(TAG, "Error : Malloc failed for String Array value %d in Read Group\n", i);
                            goto EXIT;
                        }
                        strncpy(values[i], (char *) str[i].data, str[i].length);
                        values[i][str[i].length] = '\0';
                    }
                    versatility->value = (void *) values;
                }
                else if (response->type == ByteString)
                {
                    // ByteString Array
                    UA_ByteString *str = ((UA_ByteString *) value->value.data);
                    char **values = (char **) malloc(sizeof(char *) * value->value.arrayLength);
                    if(IS_NULL(values))
                    {
                        EDGE_LOG(TAG, "Error : Malloc failed for ByteString Array value in Read Group\n");
                        goto EXIT;
                    }
                    for (int i = 0; i < value->value.arrayLength; i++)
                    {
                        values[i] = (char *) EdgeMalloc(str[i].length + 1);
                        if(IS_NULL(values[i]))
                        {
                            EDGE_LOG_V(TAG, "Error : Malloc failed for ByteString Array value %d in Read Group\n", i);
                            goto EXIT;
                        }
                        strncpy(values[i], (char *) str[i].data, str[i].length);
                        values[i][str[i].length] = '\0';
                    }
                    versatility->value = (void *) values;
                }
                else if (response->type == Guid)
                {
                    // Guid Array
                    UA_Guid *str = ((UA_Guid *) value->value.data);
                    char **values = (char **) malloc(sizeof(char *) * value->value.arrayLength);
                    if(IS_NULL(values))
                    {
                        EDGE_LOG(TAG, "Error : Malloc failed for Guid Array values in Read Group\n");
                        goto EXIT;
                    }
                    for (int i = 0; i < value->value.arrayLength; i++)
                    {
                        values[i] = (char *) EdgeMalloc(GUID_LENGTH + 1);
                        if(IS_NULL(values[i]))
                        {
                            EDGE_LOG_V(TAG, "Error : Malloc failed for Guid Array value %d in Read Group\n", i);
                            goto EXIT;
                        }
                        snprintf(values[i], (GUID_LENGTH / 2) + 1, "%08x-%04x-%04x",
                                str[i].data1, str[i].data2, str[i].data3);
                        sprintf(values[i], "%s-%02x", values[i], str[i].data4[0]);
                        sprintf(values[i], "%s%02x", values[i], str[i].data4[1]);
                        sprintf(values[i], "%s-", values[i]);
                        for (int j = 2; j < 8; j++)
                            sprintf(values[i], "%s%02x", values[i], str[i].data4[j]);
                        values[GUID_LENGTH] = '\0';
                        EDGE_LOG_V(TAG, "%s\n", values[i]);
                    }
                    versatility->value = (void *) values;
                }
                else
                {
                    versatility->value = (void *) EdgeCalloc(versatility->arrayLength, size);
                    memcpy(versatility->value, value->value.data, get_size(response->type, false) * versatility->arrayLength);
                }
            }

            response->message = versatility;

            EdgeMessage *resultMsg = (EdgeMessage *) EdgeCalloc(1, sizeof(EdgeMessage));
            if(IS_NULL(resultMsg))
            {
                EDGE_LOG(TAG, "Error : Malloc failed for resultMsg in monitor item handler\n");
                goto EXIT;
            }
            resultMsg->endpointInfo = cloneEdgeEndpointInfo(subInfo->msg->endpointInfo);
            if(IS_NULL(resultMsg->endpointInfo))
            {
                EDGE_LOG(TAG, "Error : EdgeCalloc failed for resultMsg.endpointInfo in monitor item handler\n");
                goto EXIT;
            }

            resultMsg->type = REPORT;
            resultMsg->responseLength = 1;
            resultMsg->responses = (EdgeResponse **) malloc(1 * sizeof(EdgeResponse*));
            if(IS_NULL(resultMsg->responses))
            {
                EDGE_LOG(TAG, "Error : Malloc failed for resultMsg.responses in monitor item handler\n");
                goto EXIT;
            }

            resultMsg->responses[0] = response;

            //onResponseMessage(resultMsg);
            add_to_recvQ(resultMsg);

            return ;

            EXIT:
            if(IS_NOT_NULL(response))
            {
                if (response->type == String || response->type == ByteString)
                {
                    if (response->message->isArray)
                    {
                        // Free String array
                        char **values = response->message->value;
                        if (values)
                        {
                            for (int j = 0; j < response->message->arrayLength; j++)
                            {
                                EdgeFree(values[j]);
                            }
                            EdgeFree(values);
                        }
                    }
                }
                if(IS_NOT_NULL(response->nodeInfo))
                {
                    EdgeFree(response->nodeInfo->valueAlias);
                }
                EdgeFree(response->nodeInfo);
                EdgeFree(response->message);
                EdgeFree(response);
            }
            if(IS_NOT_NULL(resultMsg))
            {
                EdgeFree(resultMsg->responses);
                EdgeFree(resultMsg->endpointInfo);
                EdgeFree(resultMsg);
            }
        }
    }
}

static void *subscription_thread_handler(void *ptr)
{
    EDGE_LOG(TAG, ">>>>>>>>>>>>>>>>>> subscription thread created <<<<<<<<<<<<<<<<<<<< \n");
    UA_Client *client = (UA_Client *) ptr;
    clientSubscription *clientSub = NULL;
    clientSub = (clientSubscription*) get_subscription_list(client);
    clientSub->subscription_thread_running = true;
    while (clientSub->subscription_thread_running)
    {
        sendPublishRequest(client);
        usleep(EDGE_UA_MINIMUM_PUBLISHING_TIME * 1000);
    }

    EDGE_LOG(TAG, ">>>>>>>>>>>>>>>>>> subscription thread destroyed <<<<<<<<<<<<<<<<<<<< \n");
    return NULL;
}

static UA_StatusCode createSub(UA_Client *client, const EdgeMessage *msg)
{
    clientSubscription *clientSub = NULL;
    clientSub = get_subscription_list(client);

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

    for (int i = 0; i < msg->requestLength; i++)
    {
        for(int j = 0; j < msg->requestLength-1; j++)
        {
            if(i != j)
            {
                if (!strcmp(msg->requests[i]->nodeInfo->valueAlias, msg->requests[j]->nodeInfo->valueAlias))
                {
                    EDGE_LOG_V(TAG, "Error :Message contains dublicate requests\n"
                        "Item No : %d & %d\nItem Name : %s\nThis Subscription request was not processed to server.\n",
                        i+1, j+1, msg->requests[i]->nodeInfo->valueAlias);
                    return UA_STATUSCODE_BADREQUESTCANCELLEDBYCLIENT;
                }
            }
        }
    }

    if(IS_NOT_NULL(clientSub))
    {
        subscriptionInfo *subInfo =  NULL;

        for (int i = 0; i < msg->requestLength; i++)
        {
            subInfo = (subscriptionInfo *) getSubInfo(clientSub->subscriptionList, 
                msg->requests[i]->nodeInfo->valueAlias);

            if (IS_NOT_NULL(subInfo))
            {
                EDGE_LOG_V(TAG, "Error : Already subscribed Node %s\n"
                    "This Subscription request was not processed to server.\n", msg->requests[i]->nodeInfo->valueAlias);
                return UA_STATUSCODE_BADREQUESTCANCELLEDBYCLIENT;
            }
        }
    }

    UA_UInt32 subId = 0;
    UA_SubscriptionSettings settings =
    { subReq->publishingInterval, /* .requestedPublishingInterval */
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
        EDGE_LOG_V(TAG, "Error in creating subscription :: %s\n\n", UA_StatusCode_name(retSub));
        return retSub;
    }

    EDGE_LOG_V(TAG, "Subscription ID received is %u\n", subId);

    if (NULL != clientSub && hasSubscriptionId(clientSub->subscriptionList, subId))
    {
        EDGE_LOG_V(TAG, "ERROR :: Subscription ID is already present in subscriptionList %s\n",
                UA_StatusCode_name(UA_STATUSCODE_BADSUBSCRIPTIONIDINVALID));
        return UA_STATUSCODE_BADSUBSCRIPTIONIDINVALID;
    }

    size_t itemSize = msg->requestLength;
    UA_MonitoredItemCreateRequest *items = (UA_MonitoredItemCreateRequest *) EdgeMalloc(
            sizeof(UA_MonitoredItemCreateRequest) * itemSize);
    if(IS_NULL(items))
    {
        EDGE_LOG(TAG, "Error : Malloc failed for items in create subscription");
        goto EXIT;
    }
    UA_UInt32 *monId = (UA_UInt32 *) EdgeMalloc(sizeof(UA_UInt32) * itemSize);
    if(IS_NULL(monId))
    {
        EDGE_LOG(TAG, "Error : Malloc failed for monId in create subscription");
        goto EXIT;
    }
    UA_StatusCode *itemResults = (UA_StatusCode *) EdgeMalloc(sizeof(UA_StatusCode) * itemSize);
    if(IS_NULL(itemResults))
    {
        EDGE_LOG(TAG, "Error : Malloc failed for itemResults in create subscription");
        goto EXIT;
    }
    UA_MonitoredItemHandlingFunction *hfs = (UA_MonitoredItemHandlingFunction *) EdgeMalloc(
            sizeof(UA_MonitoredItemHandlingFunction) * itemSize);
    if(IS_NULL(hfs))
    {
        EDGE_LOG(TAG, "Error : Malloc failed for UA_MonitoredItemHandlingFunction in create subscription");
        goto EXIT;
    }
    client_valueAlias **client_alias = (client_valueAlias**) EdgeMalloc(sizeof(client_valueAlias*) * itemSize);
    if(IS_NULL(client_alias))
    {
        EDGE_LOG(TAG, "Error : Malloc failed for client_alias in create subscription");
        goto EXIT;
    }

    for (int i = 0; i < itemSize; i++)
    {
        monId[i] = 0;
        hfs[i] = &monitoredItemHandler;
        client_alias[i] = (client_valueAlias*) EdgeMalloc(sizeof(client_valueAlias));
         if(IS_NULL(client_alias[i]))
        {
            EDGE_LOG_V(TAG, "Error : Malloc failed for client_alias id %d in create subscription\n", i);
            goto EXIT;
        }
        client_alias[i]->client = client;
        client_alias[i]->valueAlias = (char *)EdgeMalloc(EDGE_UA_SUBSCRIPTION_ITEM_SIZE);
        if(IS_NULL(client_alias[i]->valueAlias))
        {
            EDGE_LOG_V(TAG, "Error : Malloc failed for client_alias.valuealias id %d in create subscription\n", i);
            goto EXIT;
        }
        strncpy(client_alias[i]->valueAlias, msg->requests[i]->nodeInfo->valueAlias,
            strlen(msg->requests[i]->nodeInfo->valueAlias));
        client_alias[i]->valueAlias[strlen(msg->requests[i]->nodeInfo->valueAlias)] = '\0';

        EDGE_LOG_V(TAG, "%s, %s, %d", msg->requests[i]->nodeInfo->valueAlias,
                msg->requests[i]->nodeInfo->nodeId->nodeUri, msg->requests[i]->nodeInfo->nodeId->nameSpace);
        UA_MonitoredItemCreateRequest_init(&items[i]);
        items[i].itemToMonitor.nodeId = UA_NODEID_STRING(msg->requests[i]->nodeInfo->nodeId->nameSpace,
                msg->requests[i]->nodeInfo->valueAlias);
        items[i].itemToMonitor.attributeId = UA_ATTRIBUTEID_VALUE;
        items[i].monitoringMode = UA_MONITORINGMODE_REPORTING;
        items[i].requestedParameters.samplingInterval =
                msg->requests[i]->subMsg->samplingInterval;
        items[i].requestedParameters.discardOldest = true;
        items[i].requestedParameters.queueSize = 1;
    }

    UA_StatusCode retMon = UA_Client_Subscriptions_addMonitoredItems(client, subId, items, itemSize,
            hfs, (void **) client_alias, itemResults, monId);
    (void) retMon;
    for (int i = 0; i < itemSize; i++)
    {
        EDGE_LOG_V(TAG, "Monitoring Details for item : %d\n", i);
        if (monId[i])
        {
            if (clientSub != NULL && !validateMonitoringId(clientSub->subscriptionList, subId, monId[i]))
            {
                EDGE_LOG_V(TAG, "Error :: Existing Monitored ID received:: %u\n", monId[i]);
                EDGE_LOG_V(TAG, "Existing Node Details : Sub ID %d, Monitored ID :: %u\n"

                    "Error :: %s Not added to subscription list\n\n ", subId, monId[i], client_alias[i]->valueAlias);
                continue;
            }

            EDGE_LOG_V(TAG, "\tMonitoring ID :: %u\n", monId[i]);
        }
        else
        {
            // TODO: Handle Error
            EDGE_LOG_V(TAG, "ERROR : INVALID Monitoring ID Recevived for item :: #%d,  Error : %d\n", i, retMon);
            return UA_STATUSCODE_BADMONITOREDITEMIDINVALID;
        }

        if (itemResults[i] == UA_STATUSCODE_GOOD)
        {
            EDGE_LOG_V(TAG, "\tMonitoring Result ::  %s\n", UA_StatusCode_name(itemResults[i]));
        }
        else
        {
            EDGE_LOG_V(TAG, "ERROR Result Recevied for this item : %s\n", UA_StatusCode_name(itemResults[i]));
            return itemResults[i];
        }

        if (IS_NULL(clientSub))
        {
            EDGE_LOG(TAG, "subscription list for the client is empty\n");
            clientSub = (clientSubscription*) EdgeMalloc(sizeof(clientSubscription));
            if(IS_NULL(clientSub))
            {
                EDGE_LOG(TAG, "Error : Malloc failed for clientSub in create subscription\n");
                goto EXIT;
            }
            clientSub->subscriptionCount = 0;
            clientSub->subscriptionList = NULL;
        }

        if (IS_NULL(clientSub->subscriptionList))
        {
            clientSub->subscriptionList = createMap();
        }
        if (clientSub->subscriptionList)
        {
            subscriptionInfo *subInfo = (subscriptionInfo *) EdgeMalloc(sizeof(subscriptionInfo));
            if(IS_NULL(subInfo))
            {
                EDGE_LOG(TAG, "Error : Malloc failed for subInfo in create subscription");
                goto EXIT;
            }
            EdgeMessage *msgCopy = cloneEdgeMessage((EdgeMessage*) msg);      //    (EdgeMessage *) EdgeMalloc(sizeof(EdgeMessage));
            if(IS_NULL(msgCopy))
            {
                EdgeFree(subInfo);
                EDGE_LOG(TAG, "Error : Malloc failed for msgCopy in create subscription");
                goto EXIT;
            }
            //memcpy(msgCopy, msg, sizeof * msg);
            subInfo->msg = msgCopy;
            subInfo->subId = subId;
            subInfo->monId = monId[i];
            subInfo->hfContext = client_alias[i];
            EDGE_LOG_V(TAG, "Inserting MAP ELEMENT valueAlias :: %s \n",
                   msgCopy->requests[i]->nodeInfo->valueAlias);
            char *valueAlias = (char *)EdgeMalloc(sizeof(char) * (strlen(msgCopy->requests[i]->nodeInfo->valueAlias) + 1));
            if(IS_NULL(valueAlias))
            {
                EdgeFree(subInfo);
                EDGE_LOG(TAG, "Error : Malloc failed for valueAlias in create subscription");
                goto EXIT;
            }

            strncpy(valueAlias, msgCopy->requests[i]->nodeInfo->valueAlias,
                strlen(msgCopy->requests[i]->nodeInfo->valueAlias));
            valueAlias[strlen(msgCopy->requests[i]->nodeInfo->valueAlias)] = '\0';
            insertMapElement(clientSub->subscriptionList, (keyValue) valueAlias,
                             (keyValue) subInfo);
        }
    }

    if (NULL == clientSubMap)
    {
        clientSubMap = createMap();
    }
    insertMapElement(clientSubMap, (keyValue) client, (keyValue) clientSub);

    if (0 == clientSub->subscriptionCount)
    {
        /* initiate thread for manually sending publish request. */
        pthread_create(&(clientSub->subscription_thread), NULL, &subscription_thread_handler, (void *) client);
    }
    clientSub->subscriptionCount++;

    EXIT:
    EdgeFree(monId);
    EdgeFree(hfs);
    EdgeFree(itemResults);
    EdgeFree(items);

    return UA_STATUSCODE_GOOD;
}

static UA_StatusCode deleteSub(UA_Client *client, const EdgeMessage *msg)
{
    subscriptionInfo *subInfo =  NULL;
    clientSubscription *clientSub = NULL;
    clientSub = (clientSubscription*) get_subscription_list(client);
    if (IS_NULL(clientSub))
        return UA_STATUSCODE_BADNOSUBSCRIPTION;

    subInfo = (subscriptionInfo *) getSubInfo(clientSub->subscriptionList, msg->request->nodeInfo->valueAlias);
    if (!subInfo)
    {
        EDGE_LOG(TAG, "not subscribed yet \n");
        return UA_STATUSCODE_BADNOSUBSCRIPTION;
    }

    EDGE_LOG(TAG, "Deleting following Subscription \n");

    EDGE_LOG_V(TAG, "Node name :: %s\n", (char *)msg->request->nodeInfo->valueAlias);

    EDGE_LOG_V(TAG, "SUB ID :: %d\n", subInfo->subId);

    EDGE_LOG_V(TAG, "MON ID :: %d\n", subInfo->monId);

    UA_StatusCode ret = UA_Client_Subscriptions_removeMonitoredItem(client, subInfo->subId,
            subInfo->monId);

    if (UA_STATUSCODE_GOOD != ret)
    {
        EDGE_LOG_V(TAG, "Error in removing monitored item : MON ID %d \n", subInfo->monId);
        return ret;
    }
    else
    {
        EDGE_LOG(TAG, "Monitoring deleted successfully\n\n");
        edgeMapNode *removed = removeSubFromMap(clientSub->subscriptionList,
            msg->request->nodeInfo->valueAlias);
        if (removed != NULL)
        {
            subscriptionInfo *info = (subscriptionInfo *) removed->value;
            if (IS_NOT_NULL(info))
            {
                client_valueAlias *alias = (client_valueAlias*) info->hfContext;
                EdgeFree(alias->valueAlias);
                EdgeFree(alias);
                EdgeFree(info->msg);
                EdgeFree(info);
            }
            EdgeFree(removed->key);
            EdgeFree(removed);
        }
    }

    if (!hasSubscriptionId(clientSub->subscriptionList, subInfo->subId))
    {
        EDGE_LOG_V(TAG, "Removing the subscription  SID %d \n", subInfo->subId);
        UA_StatusCode retVal = UA_Client_Subscriptions_remove(client, subInfo->subId);
        if (UA_STATUSCODE_GOOD != retVal)
        {
            EDGE_LOG_V(TAG, "Error in removing subscription  SID %d \n", subInfo->subId);
            return retVal;
        }
        clientSub->subscriptionCount--;
        if (0 == clientSub->subscriptionCount)
        {
            /* destroy the subscription thread */
            /* delete the subscription thread as there are no existing subscriptions request */
            EDGE_LOG(TAG, "subscription thread destroy\n");
            clientSub->subscription_thread_running = false;
            pthread_join(clientSub->subscription_thread, NULL);
        }
    }

    return UA_STATUSCODE_GOOD;
}

static UA_StatusCode modifySub(UA_Client *client, const EdgeMessage *msg)
{
    subscriptionInfo *subInfo =  NULL;
    clientSubscription *clientSub = NULL;
    clientSub = (clientSubscription*) get_subscription_list(client);
    if (!clientSub)
        return UA_STATUSCODE_BADNOSUBSCRIPTION;

    subInfo =  (subscriptionInfo *) getSubInfo(clientSub->subscriptionList,
                                     msg->request->nodeInfo->valueAlias);
    //EDGE_LOG(TAG, "subscription id retrieved from map :: %d \n\n", subInfo->subId);

    if (!subInfo)
    {
        EDGE_LOG(TAG, "not subscribed yet\n");
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
        EDGE_LOG_V(TAG, "Error in modify subscription :: %s\n\n",
                UA_StatusCode_name(response.responseHeader.serviceResult));
        return response.responseHeader.serviceResult;
    }
    else
    {
        EDGE_LOG(TAG, "modify subscription success\n\n");
    }

    if (response.revisedPublishingInterval != subReq->publishingInterval)
    {
        EDGE_LOG(TAG, "Publishing Interval Changed in the Response ");

        EDGE_LOG_V(TAG, "Requested Interval:: %f Response Interval:: %f \n", subReq->publishingInterval,
                response.revisedPublishingInterval);
    }

    UA_ModifySubscriptionRequest_deleteMembers(&modifySubscriptionRequest);

    // modifyMonitoredItems
    UA_ModifyMonitoredItemsRequest modifyMonitoredItemsRequest;
    UA_ModifyMonitoredItemsRequest_init(&modifyMonitoredItemsRequest);
    modifyMonitoredItemsRequest.subscriptionId = subInfo->subId;
    modifyMonitoredItemsRequest.itemsToModifySize = 1;
    modifyMonitoredItemsRequest.itemsToModify = UA_malloc(sizeof(UA_MonitoredItemModifyRequest));

    UA_UInt32 monId = subInfo->monId;

    modifyMonitoredItemsRequest.itemsToModify[0].monitoredItemId = monId; //monId;
    //UA_MonitoringParameters parameters = modifyMonitoredItemsRequest.itemsToModify[0].requestedParameters;
    UA_MonitoringParameters_init(&modifyMonitoredItemsRequest.itemsToModify[0].requestedParameters);
    (modifyMonitoredItemsRequest.itemsToModify[0].requestedParameters).clientHandle = (UA_UInt32) 1;
    (modifyMonitoredItemsRequest.itemsToModify[0].requestedParameters).discardOldest = true;
    (modifyMonitoredItemsRequest.itemsToModify[0].requestedParameters).samplingInterval =
            subReq->samplingInterval;
    (modifyMonitoredItemsRequest.itemsToModify[0].requestedParameters).queueSize =
            subReq->queueSize;
    UA_ModifyMonitoredItemsResponse modifyMonitoredItemsResponse;
    __UA_Client_Service(client, &modifyMonitoredItemsRequest,
            &UA_TYPES[UA_TYPES_MODIFYMONITOREDITEMSREQUEST], &modifyMonitoredItemsResponse,
            &UA_TYPES[UA_TYPES_MODIFYMONITOREDITEMSRESPONSE]);
    if (UA_STATUSCODE_GOOD == modifyMonitoredItemsResponse.responseHeader.serviceResult)
    {

        UA_MonitoredItemModifyResult result = *modifyMonitoredItemsResponse.results;

        for (size_t index = 0; index < modifyMonitoredItemsResponse.resultsSize; index++)
        {
            EDGE_LOG_V(TAG, "Result [%d] modify monitoreditem :: %s\n\n", (int) index + 1, UA_StatusCode_name(
                            modifyMonitoredItemsResponse.results[index].statusCode));

            if (modifyMonitoredItemsResponse.results[index].statusCode != UA_STATUSCODE_GOOD)
                return modifyMonitoredItemsResponse.results[index].statusCode;
        }

        EDGE_LOG(TAG, "modify monitored item success\n\n");

        if (result.revisedQueueSize != subReq->queueSize)
        {
            EDGE_LOG(TAG, "WARNING : Revised Queue Size in Response MISMATCH\n\n");
            EDGE_LOG_V(TAG, "Result Queue Size : %u\n", result.revisedQueueSize);
            EDGE_LOG_V(TAG, "Queue Size : %u\n", subReq->queueSize);
        }

        if (result.revisedSamplingInterval != subReq->samplingInterval)
        {
            EDGE_LOG(TAG, "WARNING : Revised Sampling Interval in Response MISMATCH\n\n");
            EDGE_LOG_V(TAG, " Result Sampling Interval %f\n", result.revisedSamplingInterval);
            EDGE_LOG_V(TAG, " Sampling Interval %f\n", subReq->samplingInterval);
        }
    }
    else
    {
        EDGE_LOG_V(TAG, "modify monitored item failed :: %s\n\n", UA_StatusCode_name(
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
    VERIFY_NON_NULL(setMonitoringModeRequest.monitoredItemIds, UA_STATUSCODE_BADOUTOFMEMORY);
    setMonitoringModeRequest.monitoredItemIds[0] = monId;
    setMonitoringModeRequest.monitoringMode = UA_MONITORINGMODE_REPORTING;
    UA_SetMonitoringModeResponse setMonitoringModeResponse;
    __UA_Client_Service(client, &setMonitoringModeRequest,
            &UA_TYPES[UA_TYPES_SETMONITORINGMODEREQUEST], &setMonitoringModeResponse,
            &UA_TYPES[UA_TYPES_SETMONITORINGMODERESPONSE]);
    if (UA_STATUSCODE_GOOD == setMonitoringModeResponse.responseHeader.serviceResult)
    {
        EDGE_LOG(TAG, "set monitor mode success\n\n");
    }
    else
    {
        EDGE_LOG_V(TAG, "set monitor mode failed :: %s\n\n", UA_StatusCode_name(
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
    VERIFY_NON_NULL(setPublishingModeRequest.subscriptionIds, UA_STATUSCODE_BADOUTOFMEMORY);
    setPublishingModeRequest.subscriptionIds[0] = subInfo->subId;
    setPublishingModeRequest.publishingEnabled = subReq->publishingEnabled; //UA_TRUE;
    UA_SetPublishingModeResponse setPublishingModeResponse;
    __UA_Client_Service(client, &setPublishingModeRequest,
            &UA_TYPES[UA_TYPES_SETPUBLISHINGMODEREQUEST], &setPublishingModeResponse,
            &UA_TYPES[UA_TYPES_SETPUBLISHINGMODERESPONSE]);
    if (UA_STATUSCODE_GOOD != setPublishingModeResponse.responseHeader.serviceResult)
    {
        EDGE_LOG_V(TAG, "set publish mode failed :: %s\n\n", UA_StatusCode_name(
                        setPublishingModeResponse.responseHeader.serviceResult));
        return setPublishingModeResponse.responseHeader.serviceResult;
    }

    bool publishFail = false;
    for (size_t index = 0; index < setPublishingModeResponse.resultsSize; index++)
    {
        if (setPublishingModeResponse.results[index] != UA_STATUSCODE_GOOD)
            publishFail = true;

        EDGE_LOG_V(TAG, "Result [%d] set publish mode :: %s\n\n", (int) index + 1, UA_StatusCode_name(
                        setPublishingModeResponse.results[index]));
    }

    if (publishFail)
    {
        EDGE_LOG_V(TAG, "ERROR :: Set publish mode failed :: %s\n\n",
                UA_StatusCode_name(UA_STATUSCODE_BADMONITOREDITEMIDINVALID));
        return UA_STATUSCODE_BADMONITOREDITEMIDINVALID;
    }

    EDGE_LOG(TAG, "set publish mode success\n\n");

    UA_SetPublishingModeRequest_deleteMembers(&setPublishingModeRequest);
    UA_SetPublishingModeResponse_deleteMembers(&setPublishingModeResponse);

    return UA_STATUSCODE_GOOD;
}

static UA_StatusCode rePublish(UA_Client *client, const EdgeMessage *msg)
{
    subscriptionInfo *subInfo =  NULL;
    clientSubscription *clientSub = NULL;
    clientSub = (clientSubscription*) get_subscription_list(client);
    if (!clientSub)
        return UA_STATUSCODE_BADNOSUBSCRIPTION;

    subInfo =  (subscriptionInfo *) getSubInfo(clientSub->subscriptionList,
            msg->request->nodeInfo->valueAlias);
    //EDGE_LOG(TAG, "subscription id retrieved from map :: %d \n\n", subInfo->subId);

    if (!subInfo)
    {
        EDGE_LOG(TAG, "not subscribed yet\n");
        return UA_STATUSCODE_BADNOSUBSCRIPTION;
    }

    // re PublishingMode
    UA_RepublishRequest republishRequest;
    UA_RepublishRequest_init(&republishRequest);
    republishRequest.retransmitSequenceNumber = DEFAULT_RETRANSMIT_SEQUENCENUM;
    republishRequest.subscriptionId = subInfo->subId;

    UA_RepublishResponse republishResponse;
    __UA_Client_Service(client, &republishRequest, &UA_TYPES[UA_TYPES_REPUBLISHREQUEST],
            &republishResponse, &UA_TYPES[UA_TYPES_REPUBLISHRESPONSE]);

    if (UA_STATUSCODE_GOOD != republishResponse.responseHeader.serviceResult)
    {
        if (UA_STATUSCODE_BADMESSAGENOTAVAILABLE == republishResponse.responseHeader.serviceResult)
            EDGE_LOG(TAG, "No Message in republish response");
        else
        {
            EDGE_LOG_V(TAG, "re publish failed :: %s\n\n",
                    UA_StatusCode_name(republishResponse.responseHeader.serviceResult));
            return republishResponse.responseHeader.serviceResult;
        }
    }

    if (republishResponse.notificationMessage.notificationDataSize != 0)
    {
        EDGE_LOG_V(TAG, "Re publish Response Sequence number :: %u \n",
                republishResponse.notificationMessage.sequenceNumber);
    }
    else
    {
        EDGE_LOG(TAG, "Re publish Response has NULL notification Message\n");
    }

    UA_RepublishRequest_deleteMembers(&republishRequest);
    UA_RepublishResponse_deleteMembers(&republishResponse);

    return UA_STATUSCODE_GOOD;
}

EdgeResult executeSub(UA_Client *client, const EdgeMessage *msg)
{
    EdgeResult result;
    if (!client)
    {
        EDGE_LOG(TAG, "client handle invalid!\n");
        result.code = STATUS_ERROR;
        return result;
    }

    EdgeSubRequest *subReq;

    UA_StatusCode retVal = UA_STATUSCODE_GOOD;
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
