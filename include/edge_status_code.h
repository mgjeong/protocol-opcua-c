/* ****************************************************************
 *
 * Copyright 2017 Samsung Electronics All Rights Reserved.
 *
 *
 *
 * Licensed under the Apache License, Version 2.0 = the "License";
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
 * @file edge_status_code.h
 * @brief This file contains various status codes and error message descriptions.
 */

#ifndef EDGE_STATUS_CODE_H_
#define EDGE_STATUS_CODE_H_

/**
 * Enum for stack results and errors.
 */
typedef enum
{
    /** Success status code.*/
    STATUS_OK = 0,

    /** Error status code.*/
    STATUS_ERROR = 1,

    /** Indicates that the server is already initialized and started.*/
    STATUS_ALREADY_INIT = 2,

    /** Indicates that the client is connected with the server.*/
    STATUS_CONNECTED = 3,

    /** Indicates that the client is disconnected from the server.*/
    STATUS_DISCONNECTED = 4,

    /** Indicates that the server is started (Opened TCP socket and started listening for client connections).*/
    STATUS_SERVER_STARTED = 5,

    /** Indicates that the client is started (Established connection with OPC UA server).*/
    STATUS_CLIENT_STARTED = 6,

    /** Indicates that the server is stopped (Closed TCP socket).*/
    STATUS_STOP_SERVER = 7,

    /** Indicates that the client is stopped (Disconnected from OPC UA server).*/
    STATUS_STOP_CLIENT = 8,

    /** Service(read,write,method,browse,subscription, etc) result is not good.*/
    STATUS_SERVICE_RESULT_BAD = 9,

    /** Failed to enqueue(add) a request into send queue.*/
    STATUS_ENQUEUE_ERROR = 20,

    /** Return fewer Results than the number of nodes specified in the nodesToRead parameter.*/
    STATUS_READ_LESS_RESPONSE = 26,

    /** Not registered.*/
    STATUS_NOT_REGISTER = 30,

    /** Invalid provider.*/
    STATUS_INAVAILD_PROVIDER = 31,

    /** Indicates errors such as memory allocation failure.*/
    STATUS_INTERNAL_ERROR = 32,

    /** Function parameter is invalid(NULL, Empty or Inappropriate value, etc).*/
    STATUS_PARAM_INVALID = 33,

    /** No access permission.*/
    STATUS_NOT_ACCESS_PERMISSION = 34,

    /** Server is not started.*/
    STATUS_NOT_START_SERVER  = 35,

    /** Resonse contains 1 less record than all responses.*/
    STATUS_WRITE_LESS_RESPONSE = 50,

    /** Service result is good, but an empty result is returned.*/
    STATUS_WRITE_EMPTY_RESULT = 51,

    /** Result is more than requests.*/
    STATUS_WRITE_TOO_MANY_RESPONSE = 52,

    /** NodeId of all the results are unknown.*/
    STATUS_VIEW_NODEID_UNKNOWN_ALL_RESULTS = 60,

    /** Continuation point is available but there is no data(Value is empty).*/
    STATUS_VIEW_CONTINUATION_DATA_EMPTY = 61,

    /** Reference data is invalid.*/
    STATUS_VIEW_REFERENCE_DATA_INVALID = 62,

    /** Status code of the result has bad code.*/
    STATUS_VIEW_RESULT_STATUS_CODE_BAD = 63,

    /** Continuation point is re-used.*/
    STATUS_VIEW_CONTINUATION_POINT_REUSED = 64,

    /** Result contains data whose direction does not match the search criteria.*/
    STATUS_VIEW_REFERENCE_DIRECTION_WRONG = 65,

    /** Result direction is different from browse request.*/
    STATUS_VIEW_DIRECTION_NOT_MATCH = 66,

    /** First result contains data that is of a type that does not match.*/
    STATUS_VIEW_REFERENCE_DATA_NOT_MATCH = 67,

    /** Nodeclass is not included in browse description nodeclass mask.*/
    STATUS_VIEW_NOTINCLUDE_NODECLASS = 68,

    /** Browse result is empty.*/
    STATUS_VIEW_BROWSERESULT_EMPTY = 69,

    /** Browse request's size is over maximum size.*/
    STATUS_VIEW_BROWSEREQUEST_SIZEOVER = 70,

    /** Check the revised value and if it is different to the requested.*/
    STATUS_SUB_PUB_INTERVAL_DIFFERENCE = 100,

    /** Check the revised value and if it is different to the requested.*/
    STATUS_SUB_LIFETIME_DIFFERENCE = 101,

    /** Check the revised value and if it is different to the requested.*/
    STATUS_SUB_MAX_KEEPALIVE_DIFFERENCE = 102,

    /** PublishTime to a time in the future.*/
    STATUS_SUB_NOTIFICATION_TIME_INVALID = 103,

    /** Max notification per publish is not matched.*/
    STATUS_SUB_MAX_NOTIFICATION_NOT_MATCH = 104,

    /** The subscription id is not valid.*/
    STATUS_SUB_ID_INVALID = 105,

    /** There was nothing to do because the client passed a list of operations with no elements.*/
    STATUS_SUB_NOTHING_TO_DO = 106,

    /** The server has reached the maximum number of queued publish requests.*/
    STATUS_SUB_TOO_MANY_OPERATION = 107,

    /** An internal error occurred as a result of a programming or configuration error.*/
    STATUS_SUB_LIB_INTERNAL_ERROR = 108,

    /** The sequence number is unknown to the server.*/
    STATUS_SUB_SEQUENCE_NUMBER_UNKNOWN = 109,

    /** The sequence number is not valid.*/
    STATUS_SUB_SEQUENCE_NUMBER_INVALID = 110,

    /** There is no subscription.*/
    STATUS_SUB_NO_SUBSCRIPTION = 111,

    /** There are too many publish requests.*/
    STATUS_SUB_TOO_MANY_PUBLISH_REQUESTS = 113,

    /** Subscription publish data loss.*/
    STATUS_SUB_DATA_LOSS = 114,

    /** Service result is good, but an empty result is returned.*/
    STATUS_SUB_SETPUBLISH_EMPTY_RESULT = 130,

    /** Service result is good, but the length of result is increase.*/
    STATUS_SUB_DELETE_ITEM_INCREASE = 140,

    /** Service result is good, but the length of result is decrease.*/
    STATUS_SUB_DELETE_ITEM_DECREASE = 141,

    /** All items are set to same error codes.*/
    STAUTS_MONITOR_ALL_ITEMS_ERROR = 150,

    /** Sampling interval is invalid.*/
    STATUS_MONITOR_SAMPLING_INVERVAL_INVALID = 151,

    /** Queue size is invalid.*/
    STATUS_MONITOR_QUEUE_SIZE_INVALID = 152,

    /** Function/operation/request is not supported.*/
    STATUS_NOT_SUPPORT = 300,

} EdgeStatusCode;

/** STATUS_OK - Description.*/
#define STATUS_OK_VALUE                    ""

/** STATUS_ERROR - Description.*/
#define STATUS_ERROR_VALUE             "request error"

/** STATUS_ALREADY_INIT - Description.*/
#define STATUS_ALREADY_INIT_VALUE          ""

/** STATUS_CONNECTED - Description.*/
#define STATUS_CONNECTED_VALUE         ""

/** STATUS_DISCONNECTED - Description.*/
#define STATUS_DISCONNECTED_VALUE      ""

/** STATUS_SERVER_STARTED - Description.*/
#define STATUS_SERVER_STARTED_VALUE        ""

/** STATUS_CLIENT_STARTED - Description.*/
#define STATUS_CLIENT_STARTED_VALUE        ""

/** STATUS_STOP_SERVER - Description.*/
#define STATUS_STOP_SERVER_VALUE       ""

/** STATUS_STOP_CLIENT - Description.*/
#define STATUS_STOP_CLIENT_VALUE       ""

/** STATUS_SERVICE_RESULT_BAD - Description.*/
#define STATUS_SERVICE_RESULT_BAD_VALUE        "service result is not good"

/** STATUS_ENQUEUE_ERROR - Description.*/
#define STATUS_ENQUEUE_ERROR_VALUE  ""

/** STATUS_READ_LESS_RESPONSE - Description.*/
#define STATUS_READ_LESS_RESPONSE_VALUE         "Return fewer Results than the number of nodes specified in the nodesToRead parameter."

/** STATUS_NOT_REGISTER - Description.*/
#define STATUS_NOT_REGISTER_VALUE      ""

/** STATUS_INAVAILD_PROVIDER - Description.*/
#define STATUS_INAVAILD_PROVIDER_VALUE         ""

/** STATUS_INTERNAL_ERROR - Description.*/
#define STATUS_INTERNAL_ERROR_VALUE        ""

/** STATUS_PARAM_INVALID - Description.*/
#define STATUS_PARAM_INVALID_VALUE         "Invalid parameter"

/** STATUS_NOT_ACCESS_PERMISSION - Description.*/
#define STATUS_NOT_ACCESS_PERMISSION_VALUE         ""

/** STATUS_NOT_START_SERVER - Description.*/
#define STATUS_NOT_START_SERVER_VALUE         "server is not started"

/** STATUS_WRITE_LESS_RESPONSE - Description.*/
#define STATUS_WRITE_LESS_RESPONSE_VALUE         "contains 1 less record than all responses"

/** STATUS_WRITE_EMPTY_RESULT - Description.*/
#define STATUS_WRITE_EMPTY_RESULT_VALUE         "service result is good, but an empty result is returned"

/** STATUS_WRITE_TOO_MANY_RESPONSE - Description.*/
#define STATUS_WRITE_TOO_MANY_RESPONSE_VALUE         "result is more than requests"

/** STATUS_VIEW_NODEID_UNKNOWN_ALL_RESULTS - Description.*/
#define STATUS_VIEW_NODEID_UNKNOWN_ALL_RESULTS_VALUE         "all of results has Bad_NodeIdUnknown error"

/** STATUS_VIEW_CONTINUATION_DATA_EMPTY - Description.*/
#define STATUS_VIEW_CONTINUATION_DATA_EMPTY_VALUE         "continuation is availale. but there is no data_VALUEempty)"

/** STATUS_VIEW_REFERENCE_DATA_INVALID - Description.*/
#define STATUS_VIEW_REFERENCE_DATA_INVALID_VALUE         "reference data is invalid"

/** STATUS_VIEW_RESULT_STATUS_CODE_BAD - Description.*/
#define STATUS_VIEW_RESULT_STATUS_CODE_BAD_VALUE         "status code of the result has bad code"

/** STATUS_VIEW_CONTINUATION_POINT_REUSED - Description.*/
#define STATUS_VIEW_CONTINUATION_POINT_REUSED_VALUE         "continuationpoint is re-used"

/** STATUS_VIEW_REFERENCE_DIRECTION_WRONG - Description.*/
#define STATUS_VIEW_REFERENCE_DIRECTION_WRONG_VALUE         "result contains data whose direction does not match the search criteria"

/** STATUS_VIEW_DIRECTION_NOT_MATCH - Description.*/
#define STATUS_VIEW_DIRECTION_NOT_MATCH_VALUE         "result direction is different with browse request"

/** STATUS_VIEW_REFERENCE_DATA_NOT_MATCH - Description.*/
#define STATUS_VIEW_REFERENCE_DATA_NOT_MATCH_VALUE         "first result contains data that is of a type that does not match"

/** STATUS_VIEW_NOTINCLUDE_NODECLASS - Description.*/
#define STATUS_VIEW_NOTINCLUDE_NODECLASS_VALUE         "nodeclass is not include in browse description nodeclass mask"

/** STATUS_VIEW_BROWSERESULT_EMPTY - Description.*/
#define STATUS_VIEW_BROWSERESULT_EMPTY_VALUE         "browse result is empty"

/** STATUS_SUB_PUB_INTERVAL_DIFFERENCE - Description.*/
#define STATUS_SUB_PUB_INTERVAL_DIFFERENCE_VALUE         "check the revised value and if it is different to the requested"

/** STATUS_SUB_LIFETIME_DIFFERENCE - Description.*/
#define STATUS_SUB_LIFETIME_DIFFERENCE_VALUE         "check the revised value and if it is different to the requested"

/** STATUS_SUB_MAX_KEEPALIVE_DIFFERENCE - Description.*/
#define STATUS_SUB_MAX_KEEPALIVE_DIFFERENCE_VALUE         "check the revised value and if it is different to the requested"

/** STATUS_SUB_NOTIFICATION_TIME_INVALID - Description.*/
#define STATUS_SUB_NOTIFICATION_TIME_INVALID_VALUE         "publishTime to a time in the future"

/** STATUS_SUB_MAX_NOTIFICATION_NOT_MATCH - Description.*/
#define STATUS_SUB_MAX_NOTIFICATION_NOT_MATCH_VALUE         "max notification per publish is not matched"

/** STATUS_SUB_ID_INVALID - Description.*/
#define STATUS_SUB_ID_INVALID_VALUE         "The subscription id is not valid"

/** STATUS_SUB_NOTHING_TO_DO - Description.*/
#define STATUS_SUB_NOTHING_TO_DO_VALUE         "There was nothing to do because the client passed a list of operations with no elements"

/** STATUS_SUB_TOO_MANY_OPERATION - Description.*/
#define STATUS_SUB_TOO_MANY_OPERATION_VALUE         "The server has reached the maximum number of queued publish requests"

/** STATUS_SUB_LIB_INTERNAL_ERROR - Description.*/
#define STATUS_SUB_LIB_INTERNAL_ERROR_VALUE         "An internal error occurred as a result of a programming or configuration error"

/** STATUS_SUB_SEQUENCE_NUMBER_UNKNOWN - Description.*/
#define STATUS_SUB_SEQUENCE_NUMBER_UNKNOWN_VALUE         "The sequence number is unknown to the server"

/** STATUS_SUB_SEQUENCE_NUMBER_INVALID - Description.*/
#define STATUS_SUB_SEQUENCE_NUMBER_INVALID_VALUE         "The sequence number is not valid"

/** STATUS_SUB_NO_SUBSCRIPTION - Description.*/
#define STATUS_SUB_NO_SUBSCRIPTION_VALUE         "there is no subscription"

/** STATUS_SUB_TOO_MANY_PUBLISH_REQUESTS - Description.*/
#define STATUS_SUB_TOO_MANY_PUBLISH_REQUESTS_VALUE         "there aris no subscription"

/** STATUS_SUB_DATA_LOSS - Description.*/
#define STATUS_SUB_DATA_LOSS_VALUE         "subscription publish data loss"

/** STATUS_SUB_SETPUBLISH_EMPTY_RESULT - Description.*/
#define STATUS_SUB_SETPUBLISH_EMPTY_RESULT_VALUE         "service result is good, but an empty result is returned"

/** STATUS_SUB_DELETE_ITEM_INCREASE - Description.*/
#define STATUS_SUB_DELETE_ITEM_INCREASE_VALUE         "service result is good, but the length of result is increase"

/** STATUS_SUB_DELETE_ITEM_DECREASE - Description.*/
#define STATUS_SUB_DELETE_ITEM_DECREASE_VALUE          "service result is good, but the length of result is decrease"

/** STAUTS_MONITOR_ALL_ITEMS_ERROR - Description.*/
#define STAUTS_MONITOR_ALL_ITEMS_ERROR_VALUE          "all items are set to same error codes"

/** STATUS_MONITOR_SAMPLING_INVERVAL_INVALID - Description.*/
#define STATUS_MONITOR_SAMPLING_INVERVAL_INVALID_VALUE          "sampling interval is invalid"

/** STATUS_MONITOR_QUEUE_SIZE_INVALID - Description.*/
#define STATUS_MONITOR_QUEUE_SIZE_INVALID_VALUE           "queue size is invalid"

/** STATUS_NOT_SUPPORT - Description.*/
#define STATUS_NOT_SUPPORT_VALUE          "this function is not supported"

/** CONTINUATIONPOINT_EMPTY - Description.*/
#define CONTINUATIONPOINT_EMPTY          "ContinuationPoint is empty"

/** CONTINUATIONPOINT_LONG - Description.*/
#define CONTINUATIONPOINT_LONG          "ContinuationPoint is very long"

/** BROWSENAME_EMPTY - Description.*/
#define BROWSENAME_EMPTY          "BrowseName is empty"

/** BROWSENAME_LONG - Description.*/
#define BROWSENAME_LONG          "BrowseName is very long"

/** DISPLAYNAME_EMPTY - Description.*/
#define DISPLAYNAME_EMPTY          "DisplayName is empty"

/** DISPLAYNAME_LONG - Description.*/
#define DISPLAYNAME_LONG          "DisplayName is very long"

/** NODECLASS_INVALID - Description.*/
#define NODECLASS_INVALID          "NodeClass has invalid value"

/** NODEID_NULL - Description.*/
#define NODEID_NULL          "NodeId is null"

/** NODEID_SERVERINDEX - Description.*/
#define NODEID_SERVERINDEX          "NodeId's server index is not zero"

/** REFERENCETYPEID_NULL - Description.*/
#define REFERENCETYPEID_NULL          "ReferenceTypeId is null"

/** TYPEDEFINITIONNODEID_NULL - Description.*/
#define TYPEDEFINITIONNODEID_NULL          "TypeDefinition NodeID of the TargetNode is null"

#endif /* EDGE_STATUS_CODE_H_ */
