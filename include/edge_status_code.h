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
 * @file
 *
 * This file contains the definition, types and APIs for resource= s be implemented.
 */

#ifndef EDGE_STATUS_CODE_H_
#define EDGE_STATUS_CODE_H_

typedef enum
{
    STATUS_OK = 0,
    STATUS_ERROR = 1,
    STATUS_ALREADY_INIT = 2,
    STATUS_CONNECTED = 3,
    STATUS_DISCONNECTED = 4,
    STATUS_SERVER_STARTED = 5,
    STATUS_CLIENT_STARTED = 6,
    STATUS_STOP_SERVER = 7,
    STATUS_STOP_CLIENT = 8,
    STATUS_SERVICE_RESULT_BAD = 9,

    STATUS_ENQUEUE_ERROR = 20,
    STATUS_READ_LESS_RESPONSE = 26,

    STATUS_NOT_REGISTER = 30,
    STATUS_INAVAILD_PROVIDER = 31,
    STATUS_INTERNAL_ERROR = 32,
    STATUS_PARAM_INVALID = 33,
    STATUS_NOT_ACCESS_PERMISSION = 34,
    STATUS_NOT_START_SERVER  = 35,

    STATUS_WRITE_LESS_RESPONSE = 50,
    STATUS_WRITE_EMPTY_RESULT = 51,
    STATUS_WRITE_TOO_MANY_RESPONSE = 52,

    STATUS_VIEW_NOIDID_UNKNOWN_ALL_RESULTS = 60,
    STATUS_VIEW_CONTINUATION_DATA_EMPTY = 61,
    STATUS_VIEW_REFERENCE_DATA_INVALID = 62,
    STATUS_VIEW_RESULT_STATUS_CODE_BAD = 63,
    STATUS_VIEW_CONTINUATION_POINT_REUSED = 64,
    STATUS_VIEW_REFERENCE_DIRECTION_WRONG = 65,
    STATUS_VIEW_DIRECTION_NOT_MATCH = 66,
    STATUS_VIEW_REFERENCE_DATA_NOT_MATCH = 67,
    STATUS_VIEW_NOTINCLUDE_NODECLASS = 68,
    STATUS_VIEW_BROWSERESULT_EMPTY = 69,
    STATUS_VIEW_BROWSEREQUEST_SIZEOVER = 70,

    STATUS_SUB_PUB_INTERVAL_DIFFERENCE = 100,
    STATUS_SUB_LIFETIME_DIFFERENCE = 101,
    STATUS_SUB_MAX_KEEPALIVE_DIFFERENCE = 102,
    STATUS_SUB_NOTIFICATION_TIME_INVALID = 103,
    STATUS_SUB_MAX_NOTIFICATION_NOT_MATCH = 104,
    STATUS_SUB_ID_INVALID = 105,
    STATUS_SUB_NOTHING_TO_DO = 106,
    STATUS_SUB_TOO_MANY_OPERATION = 107,
    STATUS_SUB_LIB_INTERNAL_ERROR = 108,
    STATUS_SUB_SEQUENCE_NUMBER_UNKNOWN = 109,
    STATUS_SUB_SEQUENCE_NUMBER_INVALID = 110,
    STATUS_SUB_NO_SUBSCRIPTION = 111,
    STATUS_SUB_TOO_MANY_PUBLISH_REQUESTS = 113,
    STATUS_SUB_DATA_LOSS = 114,
    STATUS_SUB_SETPUBLISH_EMPTY_RESULT = 130,
    STATUS_SUB_DELETE_ITEM_INCREASE = 140,
    STATUS_SUB_DELETE_ITEM_DECREASE = 141,

    STAUTS_MONITOR_ALL_ITEMS_ERROR = 150,
    STATUS_MONITOR_SAMPLING_INVERVAL_INVALID = 151,
    STATUS_MONITOR_QUEUE_SIZE_INVALID = 152,

    STATUS_NOT_SUPPROT = 300,

} EdgeStatusCode;

#define STATUS_OK_VALUE                    ""

#define STATUS_ERROR_VALUE             "request error"

#define STATUS_ALREADY_INIT_VALUE          ""

#define STATUS_CONNECTED_VALUE         ""

#define STATUS_DISCONNECTED_VALUE      ""

#define STATUS_SERVER_STARTED_VALUE        ""

#define STATUS_CLIENT_STARTED_VALUE        ""

#define STATUS_STOP_SERVER_VALUE       ""

#define STATUS_STOP_CLIENT_VALUE       ""

#define STATUS_SERVICE_RESULT_BAD_VALUE        "service result is not good"

#define STATUS_ENQUEUE_ERROR_VALUE  ""

#define STATUS_READ_LESS_RESPONSE_VALUE         "Return fewer Results than the number of nodes specified in the nodesToRead parameter."

#define STATUS_NOT_REGISTER_VALUE      ""

#define STATUS_INAVAILD_PROVIDER_VALUE         ""

#define STATUS_INTERNAL_ERROR_VALUE        ""

#define STATUS_PARAM_INVALID_VALUE         ""

#define STATUS_NOT_ACCESS_PERMISSION_VALUE         ""

#define STATUS_NOT_START_SERVER _VALUE         "server is not started"

#define STATUS_WRITE_LESS_RESPONSE_VALUE         "contains 1 less record than all responses"

#define STATUS_WRITE_EMPTY_RESULT_VALUE         "service result is good, but an empty result is returned"

#define STATUS_WRITE_TOO_MANY_RESPONSE_VALUE         "result is more than requests"

#define STATUS_VIEW_NOIDID_UNKNOWN_ALL_RESULTS_VALUE         "all of results has Bad_NodeIdUnknown error"

#define STATUS_VIEW_CONTINUATION_DATA_EMPTY_VALUE         "continuation is availale. but there is no data_VALUEempty)"

#define STATUS_VIEW_REFERENCE_DATA_INVALID_VALUE         "reference is not availale"

#define STATUS_VIEW_RESULT_STATUS_CODE_BAD_VALUE         "status code of the result has bad code"

#define STATUS_VIEW_CONTINUATION_POINT_REUSED_VALUE         "continuationpoint is re-used"

#define STATUS_VIEW_REFERENCE_DIRECTION_WRONG_VALUE         "result contains data whose direction does not match the search criteria"

#define STATUS_VIEW_DIRECTION_NOT_MATCH_VALUE         "result direction is different with browse request"

#define STATUS_VIEW_REFERENCE_DATA_NOT_MATCH_VALUE         "first result contains data that is of a type that does not match"

#define STATUS_VIEW_NOTINCLUDE_NODECLASS_VALUE         "nodeclass is not include in browse description nodeclass mask"

#define STATUS_VIEW_BROWSERESULT_EMPTY_VALUE         "browse result is empty"

#define STATUS_VIEW_BROWSEREQUEST_SIZEOVER_VALUE         "browser request's size is over maximum size"


#define STATUS_SUB_PUB_INTERVAL_DIFFERENCE_VALUE         "check the revised value and if it is different to the requested"

#define STATUS_SUB_LIFETIME_DIFFERENCE_VALUE         "check the revised value and if it is different to the requested"

#define STATUS_SUB_MAX_KEEPALIVE_DIFFERENCE_VALUE         "check the revised value and if it is different to the requested"

#define STATUS_SUB_NOTIFICATION_TIME_INVALID_VALUE         "publishTime to a time in the future"

#define STATUS_SUB_MAX_NOTIFICATION_NOT_MATCH_VALUE         "max notification per publish is not matched"

#define STATUS_SUB_ID_INVALID_VALUE         "The subscription id is not valid"

#define STATUS_SUB_NOTHING_TO_DO_VALUE         "There was nothing to do because the client passed a list of operations with no elements"

#define STATUS_SUB_TOO_MANY_OPERATION_VALUE         "The server has reached the maximum number of queued publish requests"

#define STATUS_SUB_LIB_INTERNAL_ERROR_VALUE         "An internal error occurred as a result of a programming or configuration error"

#define STATUS_SUB_SEQUENCE_NUMBER_UNKNOWN_VALUE         "The sequence number is unknown to the server"

#define STATUS_SUB_SEQUENCE_NUMBER_INVALID_VALUE         "The sequence number is not valid"

#define STATUS_SUB_NO_SUBSCRIPTION_VALUE         "there is no subscription"

#define STATUS_SUB_TOO_MANY_PUBLISH_REQUESTS_VALUE         "there aris no subscription"

#define STATUS_SUB_DATA_LOSS_VALUE         "subscription publish data loss"

#define STATUS_SUB_SETPUBLISH_EMPTY_RESULT_VALUE         "service result is good, but an empty result is returned"

#define STATUS_SUB_DELETE_ITEM_INCREASE_VALUE         "service result is good, but the length of result is increase"

#define STATUS_SUB_DELETE_ITEM_DECREASE_VALUE          "service result is good, but the length of result is decrease"

#define STAUTS_MONITOR_ALL_ITEMS_ERROR_VALUE          "all items are set to same error codes"

#define STATUS_MONITOR_SAMPLING_INVERVAL_INVALID_VALUE          "sampling interval is invalid"

#define STATUS_MONITOR_QUEUE_SIZE_INVALID_VALUE           "queue size is invalid"

#define STATUS_NOT_SUPPROT_VALUE          "this function is not suppored"

#endif /* EDGE_STATUS_CODE_H_ */
