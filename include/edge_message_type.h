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
 * @file edge_message_type.h
 * @brief This file contains various message types and text descriptions.
 */

#ifndef EDGE_MESSAGE_TYPE_H_
#define EDGE_MESSAGE_TYPE_H_

#ifdef __cplusplus
extern "C"
{
#endif

/**
 * Declares the different types of messages sent between OPC UA wrapper and application.
 */
typedef enum
{
    /** Server information.*/
    SERVER_INFO = 0,

    /** Server product URI for device.*/
    SERVER_INFO_PRODUCT_URI = 1,

    /** Response for read, write & method operations.*/
    GENERAL_RESPONSE = 10,

    /** Response for browse, browse-view, browse-next and browse-multi operations.*/
    BROWSE_RESPONSE = 11,

    /** Response for subscription notifications.*/
    REPORT = 20,

    /** Sampling data.*/
    SAMPLING = 21,

    /** Message with single request.*/
    SEND_REQUEST = 30,

    /** Message with multiple requests.*/
    SEND_REQUESTS = 31,

    /** Error message.*/
    ERROR_RESPONSE = 40
} EdgeMessageType;

/** Server Info - Description.*/
#define SERVER_INFO_DESC                                "server information"

/** Server Info Product URI - Description.*/
#define SERVER_INFO_PRODUCT_URI_DESC          "specific product uri for device"

/** General reponse - Description.*/
#define GENERAL_RESPONSE_DESC                         "General Data"

/** Browse response - Description.*/
#define BROWSE_RESPONSE_DESC                     "Browse Response"

/** Report - Description.*/
#define REPORT_DESC                                       "Report Data"

/** Sampling - Description.*/
#define SAMPLING_DESC                                     "sampleing data"

/** Send request - Description.*/
#define SEND_REQUEST_DESC                            "send message"

/** Send requests - Description.*/
#define SEND_REQUESTS_DESC                        "send multi-messages"

/** Error - Description.*/
#define ERROR_DESC                                        "error message"

#ifdef __cplusplus
}
#endif

#endif /* EDGE_MESSAGE_TYPE_H_ */
