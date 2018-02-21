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
 * @file edge_utils.h
 * @brief This file contains the utilities APIs for use in OPCUA module be implemented.
 */

#ifndef EDGE_UTILS_H_
#define EDGE_UTILS_H_

#include "edge_logger.h"
#include "opcua_common.h"
#include "open62541.h"

#ifdef __cplusplus
extern "C"
{
#endif

#define FREE(arg) if(arg) {free(arg); arg=NULL; }

#define IS_NULL(arg) ((arg == NULL) ? true : false)
#define IS_NOT_NULL(arg) ((arg != NULL) ? true : false)

#define VERIFY_NON_NULL(arg, retVal) { if (!(arg)) { EDGE_LOG(TAG, \
             #arg " is NULL"); return (retVal); } }
#define VERIFY_NON_NULL_NR(arg) { if (!(arg)) { EDGE_LOG(TAG, \
             #arg " is NULL"); return; } }

    /**
     * @brief Structure for a generic single linked list.
     */
    typedef struct List
    {
        /** Generic data of list.*/
        void *data;

        /** Link to next generic data.*/
        struct List *link;
    } List;

    /** Generic pointer to represent key/value.*/
    typedef void *keyValue;

    /**
     * @brief Structure for a node in the list of generic key-value pairs.
     */
    typedef struct edgeMapNode
    {
        /** Map Key.*/
        keyValue key;

        /** map key-value pair.*/
        keyValue value;

        /** Next node in list.*/
        struct edgeMapNode *next;
    } edgeMapNode;

    /**
     * @brief Structure which holds the head pointer of a generic key-value pairs list.
     * @remarks This structure is used to hold head pointer of a list.
     */
    typedef struct edgeMap
    {
        /** Map Head.*/
        edgeMapNode *head;
    } edgeMap;

    /**
     * @brief API for creating a map for storing edge nodes.
     * @remarks This API will allocate memory required.
     * @return a pointer to the created map, otherwise a null pointer if the memory is insufficient.
     */
    edgeMap *createMap();

    /**
     * @brief Insert a key-value pair into the map.
     * @param[in]  map Pointer to an edgeMap created using createMap().
     * @param[in]  key Generic key.
     * @param[in]  value Generic value.
     */
    void insertMapElement(edgeMap *map, keyValue key, keyValue value);

    /**
     * @brief Get the element value of the given key from the map.
     * @param[in]  map Pointer to an edgeMap created using createMap().
     * @param[in]  key Generic key.
     * @return Value of given key on success, otherwise null.
     */
    keyValue getMapElement(edgeMap *map, keyValue key);

    /**
     * @brief Delete and free memory used by the edge util map.
     * @param[in]  map Pointer to an edgeMap created using createMap().
     */
    void deleteMap(edgeMap *map);

    /**
     * @brief Adds an object into the single linked list.
     * @param[in]  head Address of the head pointer of the single linked list.
     * @param[in]  data Represents the object to be added.
     * @return @c true if object added, otherwise @c false
     */
    bool addListNode(List **head, void *data);

    /**
     * @brief Destroys the single linked list.
     * @param[in]  head Address of the head pointer of the single linked list.
     */
    void deleteList(List **head);

    /**
     * @brief Get the number of elements in the list.
     * @param[in]  ptr Head pointer of the single linked list.
     * @return size of the list
     */
    unsigned int getListSize(List *ptr);

    /**
     * @brief Prints the current system time in "MM/DD HH:MM:SS.sss" format.
     * @remarks Works only if the stack is built in debug mode.
     */
    void logCurrentTimeStamp();

    /**
     * @brief Clones a given string.
     * @remarks Allocated memory should be freed by the caller.
     * @param[in]  str String to be cloned.
     * @return Cloned string on success. Otherwise null.
     */
    char *cloneString(const char *str);

    /**
     * @brief Clones data of a specified length.
     * @remarks Allocated memory should be freed by the caller.
     * @param[in]  src Data to be cloned.
     * @param[in]  lenInbytes Length of the data to be cloned.
     * @return Cloned data on success. Otherwise null.
     */
    void *cloneData(const void *src, int lenInbytes);

    /**
     * @brief Converts string of type UA_String to char string.
     * @remarks Allocated memory should be freed by the caller.
     * @param[in]  uaStr Data to be cloned.
     * @return Converted string on success. Otherwise null.
     */
    char *convertUAStringToString(UA_String *uaStr);

    /**
     * @brief De-allocates the memory consumed by EdgeEndpointConfig and its members.
     * @remarks Both EdgeEndpointConfig and its members should have been allocated dynamically.
     * @param[in]  config Pointer to EdgeEndpointConfig which needs to be freed.
     */
    void freeEdgeEndpointConfig(EdgeEndpointConfig *config);

    /**
     * @brief De-allocates the memory consumed by the members of EdgeApplicationConfig.
     * @remarks EdgeApplicationConfig's members should have been allocated dynamically.
     * @param[in]  config Pointer to EdgeApplicationConfig whose members need to be freed.
     */
    void freeEdgeApplicationConfigMembers(EdgeApplicationConfig *config);

    /**
     * @brief De-allocates the memory consumed by EdgeApplicationConfig and its members.
     * @remarks Both EdgeApplicationConfig and its members should have been allocated dynamically.
     * @param[in]  config Pointer to EdgeApplicationConfig which needs to be freed.
     */
    void freeEdgeApplicationConfig(EdgeApplicationConfig *config);

    /**
     * @brief De-allocates the memory consumed by EdgeEndPointInfo and its members.
     * @remarks Both EdgeEndPointInfo and its members should have been allocated dynamically.
     * @param[in]  endpointInfo Pointer to EdgeEndPointInfo which needs to be freed.
     */
    void freeEdgeEndpointInfo(EdgeEndPointInfo *endpointInfo);

    /**
     * @brief De-allocates the memory consumed by EdgeBrowseResult and its members of the given size.
     * @remarks Both EdgeBrowseResult and its members should have been allocated dynamically.
     * @param[in]  browseResult Pointer to EdgeBrowseResult which needs to be freed.
     * @param[in]  browseResultLength Number of elements in the given EdgeBrowseResult.
     */
    void freeEdgeBrowseResult(EdgeBrowseResult *browseResult, int browseResultLength);

    /**
     * @brief De-allocates the memory consumed by EdgeResult.
     * @remarks EdgeResult should have been allocated dynamically.
     * @param[in]  res Pointer to EdgeResult which needs to be freed.
     */
    void freeEdgeResult(EdgeResult *res);

    /**
     * @brief De-allocates the memory consumed by EdgeNodeId and its members.
     * @remarks Both EdgeNodeId and its members should have been allocated dynamically.
     * @param[in]  nodeId Pointer to EdgeNodeId which needs to be freed.
     */
    void freeEdgeNodeId(EdgeNodeId *nodeId);

    /**
     * @brief De-allocates the memory consumed by EdgeNodeInfo and its members.
     * @remarks Both EdgeNodeInfo and its members should have been allocated dynamically.
     * @param[in]  nodeInfo Pointer to EdgeNodeInfo which needs to be freed.
     */
    void freeEdgeNodeInfo(EdgeNodeInfo *nodeInfo);

    /**
     * @brief De-allocates the memory consumed by EdgeArgument and its members.
     * @remarks Both EdgeArgument and its members should have been allocated dynamically.
     * @param[in]  arg Pointer to EdgeArgument which needs to be freed.
     */
    void freeEdgeArgument(EdgeArgument *arg);

    /**
     * @brief De-allocates the memory consumed by EdgeMethodRequestParams and its members.
     * @remarks Both EdgeMethodRequestParams and its members should have been allocated dynamically.
     * @param[in]  methodParams Pointer to EdgeMethodRequestParams which needs to be freed.
     */
    void freeEdgeMethodRequestParams(EdgeMethodRequestParams *methodParams);

    /**
     * @brief De-allocates the memory consumed by EdgeRequest and its members.
     * @remarks Both EdgeRequest and its members should have been allocated dynamically.
     * @param[in]  req Pointer to EdgeRequest which needs to be freed.
     */
    void freeEdgeRequest(EdgeRequest *req);

    /**
     * @brief De-allocates the memory consumed by EdgeRequest and its members of the given size.
     * @remarks Both EdgeRequest and its members should have been allocated dynamically.
     * @param[in]  requests Pointer to EdgeRequest which needs to be freed.
     * @param[in]  requestLength Number of elements in the given request.
     */
    void freeEdgeRequests(EdgeRequest **requests, int requestLength);

    /**
     * @brief De-allocates the memory consumed by EdgeVersatility and its members.
     * @remarks Both EdgeVersatility and its members should have been allocated dynamically.
     * @param[in]  versatileValue Pointer to EdgeVersatility which needs to be freed.
     */
    void freeEdgeVersatility(EdgeVersatility *versatileValue);

    /**
     * @brief De-allocates the memory consumed by EdgeVersatility and its members.
     * @remarks Both EdgeVersatility and its members should have been allocated dynamically.
     * @param[in]  versatileValue Pointer to EdgeVersatility which needs to be freed.
     * @param[in]  type Type of the value in EdgeVersatility.
     */
    void freeEdgeVersatilityByType(EdgeVersatility *versatileValue, EdgeNodeIdentifier type);

    /**
     * @brief De-allocates the memory consumed by EdgeResponse and its members.
     * @remarks Both EdgeResponse and its members should have been allocated dynamically.
     * @param[in]  response Pointer to EdgeResponse which needs to be freed.
     */
    void freeEdgeResponse(EdgeResponse *response);

    /**
     * @brief De-allocates the memory consumed by EdgeResponse and its members of the given size.
     * @remarks Both EdgeResponse and its members should have been allocated dynamically.
     * @param[in]  responses Pointer to EdgeResponse which needs to be freed.
     * @param[in]  responseLength Number of elements in the given request.
     */
    void freeEdgeResponses(EdgeResponse **responses, int responseLength);

    /**
     * @brief De-allocates the memory consumed by EdgeMessage and its members.
     * @remarks Both EdgeMessage and its members should have been allocated dynamically.
     * @param[in]  msg Pointer to EdgeMessage which needs to be freed.
     */
    void freeEdgeMessage(EdgeMessage *msg);

    /**
     * @brief De-allocates the memory consumed by EdgeContinuationPoint and its members.
     * @remarks Both EdgeContinuationPoint and its members should have been allocated dynamically.
     * @param[in]  cp Pointer to EdgeContinuationPoint which needs to be freed.
     */
    void freeEdgeContinuationPoint(EdgeContinuationPoint *cp);

    /**
     * @brief De-allocates the memory consumed by EdgeContinuationPointList and its members.
     * @remarks Both EdgeContinuationPointList and its members should have been allocated dynamically.
     * @param[in]  cpList Pointer to EdgeContinuationPointList which needs to be freed.
     */
    void freeEdgeContinuationPointList(EdgeContinuationPointList *cpList);

    /**
     * @brief De-allocates the memory consumed by EdgeDevice and its members.
     * @remarks Both EdgeDevice and its members should have been allocated dynamically.
     * @param[in]  dev Pointer to EdgeDevice which needs to be freed.
     */
    void freeEdgeDevice(EdgeDevice *dev);

    /**
     * @brief Clones EdgeEndpointConfig object.
     * @remarks Allocated memory should be freed by the caller.
     * @param[in]  config EdgeEndpointConfig object to be cloned.
     * @return Cloned EdgeEndpointConfig object on success. Otherwise null.
     */
    EdgeEndpointConfig *cloneEdgeEndpointConfig(EdgeEndpointConfig *config);

    /**
     * @brief Clones EdgeApplicationConfig object.
     * @remarks Allocated memory should be freed by the caller.
     * @param[in]  config EdgeApplicationConfig object to be cloned.
     * @return Cloned EdgeApplicationConfig object on success. Otherwise null.
     */
    EdgeApplicationConfig *cloneEdgeApplicationConfig(EdgeApplicationConfig *config);

    /**
     * @brief Clones EdgeEndPointInfo object.
     * @remarks Allocated memory should be freed by the caller.
     * @param[in]  endpointInfo EdgeEndPointInfo object to be cloned.
     * @return Cloned EdgeEndPointInfo object on success. Otherwise null.
     */
    EdgeEndPointInfo *cloneEdgeEndpointInfo(EdgeEndPointInfo *endpointInfo);

    /**
     * @brief Creates an EdgeResult object with the given status code.
     * @remarks Allocated memory should be freed by the caller.
     * @param[in]  code Represents status code.
     * @return EdgeResult object on success. Otherwise null.
     */
    EdgeResult *createEdgeResult(EdgeStatusCode code);

    /**
     * @brief Checks whether the given node class is valid & supported by open62541.
     * @param[in]  nodeClass Represents the node class.
     * @return @c true on success, othewise @c false.
     */
    bool isNodeClassValid(UA_NodeClass nodeClass);

    /**
     * @brief Clones EdgeNodeId object.
     * @remarks Allocated memory should be freed by the caller.
     * @param[in]  nodeId EdgeNodeId object to be cloned.
     * @return Cloned EdgeNodeId object on success. Otherwise null.
     */
    EdgeNodeId *cloneEdgeNodeId(EdgeNodeId *nodeId);

    /**
     * @brief Clones EdgeNodeInfo object.
     * @remarks Allocated memory should be freed by the caller.
     * @param[in]  nodeInfo EdgeNodeInfo object to be cloned.
     * @return Cloned EdgeNodeInfo object on success. Otherwise null.
     */
    EdgeNodeInfo *cloneEdgeNodeInfo(EdgeNodeInfo *nodeInfo);

    /**
     * @brief Clones EdgeMessage object.
     * @remarks Allocated memory should be freed by the caller.
     * @param[in]  msg EdgeMessage object to be cloned.
     * @return Cloned EdgeMessage object on success. Otherwise null.
     */
    EdgeMessage* cloneEdgeMessage(EdgeMessage *msg);

    /**
     * @brief To get the size of the node of a given type.
     * @param[in]  type Type of node.
     * @param[in]  isArray Indicates whether size is required for an array or single element.
     * @return Size of the node of a given type.
     */
    size_t get_size(EdgeNodeIdentifier type, bool isArray);

    /**
     * @brief To get the enum equivalent for the given node type.
     * @param[in]  type Type of node id.
     * @return Enum equivalent for the given type.
     */
    EdgeNodeIdType getEdgeNodeIdType(char type);

    /**
     * @brief To get the char equivalent for the given type.
     * @param[in]  type Type of node id.
     * @return Character equivalent for the given type.
     */
    char getCharacterNodeIdType(uint32_t type);

#ifdef __cplusplus
}
#endif

#endif /* EDGE_UTILS_H_ */
