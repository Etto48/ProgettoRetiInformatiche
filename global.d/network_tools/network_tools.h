#pragma once

#include <stdint.h>
#include <arpa/inet.h>
#include <stdint.h>
#include <stdbool.h>
#include <string.h>
#include <malloc.h>
#include "../globalDefine.h"

/**
 * @brief payload description is under the message type
 * 
 */
typedef enum
{
    /**
     * @brief payload: <0|1:1B> 
     *  0->Error
     *  1->Ok
     * used by the server to respond to any request that only needs a confirmation or an error
     */
    MESSAGE_RESPONSE = 0,

    /**
     * @brief payload: <username:20B><hashed password:32B>
     * used for the signup message
     */
    MESSAGE_SIGNUP = 1,

    /**
     * @brief payload: <port:2B><username:20B><hashed password:32B>
     * used for the login message
     */
    MESSAGE_LOGIN = 2,

    /**
     * @brief payload: empty
     * used for the logout message
     */
    MESSAGE_LOGOUT = 3,

    /**
     * @brief payload: [username:20B]
     * used for the hanging and show message (response to the last one will be served with multiple MESSAGE_DATA messages)
     */
    MESSAGE_HANGING = 4,

    /**
     * @brief payload: <username:20B>
     * used to request info about ip/port of a user
     */
    MESSAGE_USERINFO_REQ = 5,

    /**
     * @brief payload: <IPv4:4B><port:2B>
     * ip and port on which the other client is listening for connections
     */
    MESSAGE_USERINFO_RES = 6,

    /**
     * @brief payload: <username:20B><timestamp:8B>
     * used to send a notification of last message recived from <username>
     */
    MESSAGE_SYNCREAD = 7,

    /**
     * @brief payload: <username:20B><timestamp:8B><'T'|'F':1B><message:variable>
     * used to send a message to a client from <username> at time <timestamp>, use T if it's text or F if it's a file
     */
    MESSAGE_DATA = 8
} MessageType;

/**
 * @brief this header is sent before any message on the network
 * 
 */
typedef struct 
{
    MessageType type;
    uint32_t payload_size;
} MessageHeader;

/**
 * @brief serialize a message in a host-independent format
 * 
 * @param type message type
 * @param payload message payload (refer to MessageType documentation), this should be already serialized
 * @param dst_stream destination stream on which we serialize the input; THIS WILL BE DINAMICALLY ALLOCATED, REMEMBER TO FREE IT
 */
void NetworkSerializeMessage(MessageType type,const char* payload, uint8_t** dst_stream);

/**
 * @brief deserialize a message from a host-independent format
 * 
 * @param src_stream source stream from which we deserialize the input
 * @param type deserialized message type; THIS WILL BE DINAMICALLY ALLOCATED, REMEMBER TO FREE IT
 * @param payload message payload, you have to deserialize this; THIS WILL BE DINAMICALLY ALLOCATED, REMEMBER TO FREE IT
 */
void NetworkDeserializeMessage(const uint8_t* src_stream, MessageType** type, char** payload);

bool SendMessageResponse();