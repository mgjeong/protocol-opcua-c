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

/**
 * @file
 *
 * This file contains the utilities APIs for use in OPCUA module be implemented.
 */

#ifndef EDGE_UTILS_H_
#define EDGE_UTILS_H_

#include "open62541.h"
#include "opcua_common.h"

#ifdef __cplusplus
extern "C"
{
#endif

    typedef struct List
    {
        void *data;
        struct List *link;
    } List;

    typedef void *keyValue;

    typedef struct edgeMapNode
    {
        /** Map Key.*/
        keyValue key;

        /** map key-value pair.*/
        keyValue value;

        /** Next node in list.*/
        struct edgeMapNode *next;
    } edgeMapNode;

    typedef struct edgeMap
    {
        /** Map Head.*/
        edgeMapNode *head;
    } edgeMap;

    /**
     * Insert key-value pair into the edge util map
     *
     * @return edgeMap
     */
    edgeMap *createMap();

    /**
     * Insert key-value pair into the edge util map
     *
     * @param map
     * @param key
     * @param value
     * @return
     */
    void insertMapElement(edgeMap *map, keyValue key, keyValue value);

    /**
     * Get element value from the edge util map
     *
     * @param map
     * @param key
     * @return keyValue
     */
    keyValue getMapElement(edgeMap *map, keyValue key);

    /**
     * Delete and free memory of the edge util map
     *
     * @param map
     * @return keyValue
     */
    void deleteMap(edgeMap *map);

    bool addListNode(List **head, void *data);

    void deleteList(List **head);

    unsigned int getListSize(List *ptr);

    char *cloneString(const char *str);

    void *cloneData(const void *src, int lenInbytes);

    char *convertUAStringToString(UA_String *uaStr);

    void freeEdgeEndpointConfig(EdgeEndpointConfig *config);

    void freeEdgeApplicationConfigMembers(EdgeApplicationConfig *config);

    void freeEdgeApplicationConfig(EdgeApplicationConfig *config);

    void freeEdgeEndpointInfo(EdgeEndPointInfo *endpointInfo);

    void freeEdgeBrowseResult(EdgeBrowseResult *browseResult, int browseResultLength);

    void freeEdgeResult(EdgeResult *res);

    void freeEdgeNodeId(EdgeNodeId *nodeId);

    void freeEdgeNodeInfo(EdgeNodeInfo *nodeInfo);

    void freeEdgeArgument(EdgeArgument *arg);

    void freeEdgeMethodRequestParams(EdgeMethodRequestParams *methodParams);

    void freeEdgeRequest(EdgeRequest *req);

    void freeEdgeRequests(EdgeRequest **requests, int requestLength);

    void freeEdgeVersatility(EdgeVersatility *versatileValue);

    void freeEdgeVersatilityByType(EdgeVersatility *versatileValue, EdgeNodeIdentifier type);

    void freeEdgeResponse(EdgeResponse *response);

    void freeEdgeResponses(EdgeResponse **responses, int responseLength);

    void freeEdgeMessage(EdgeMessage *msg);

    void freeEdgeContinuationPoint(EdgeContinuationPoint *cp);

    void freeEdgeContinuationPointList(EdgeContinuationPointList *cpList);

    void freeEdgeDevice(EdgeDevice *dev);

    EdgeEndpointConfig *cloneEdgeEndpointConfig(EdgeEndpointConfig *config);

    EdgeApplicationConfig *cloneEdgeApplicationConfig(EdgeApplicationConfig *config);

    EdgeEndPointInfo *cloneEdgeEndpointInfo(EdgeEndPointInfo *endpointInfo);

    EdgeResult *createEdgeResult(EdgeStatusCode code);

    bool isNodeClassValid(UA_NodeClass nodeClass);

    EdgeNodeId *cloneEdgeNodeId(EdgeNodeId *nodeId);

    EdgeNodeInfo *cloneEdgeNodeInfo(EdgeNodeInfo *nodeInfo);

    EdgeMessage* cloneEdgeMessage(EdgeMessage *msg);

    size_t get_size(EdgeNodeIdentifier type, bool isArray);

    EdgeNodeIdType getEdgeNodeIdType(char type);

    char getCharacterNodeIdType(uint32_t type);

#ifdef __cplusplus
}
#endif

#endif /* EDGE_UTILS_H_ */
