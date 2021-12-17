#pragma once

#include <stdint.h>

/**
 * @brief payload description is under the message type
 * 
 */
typedef enum
{
    MESSAGE_RESPONSE = 0,
    //<variable:variable>
    MESSAGE_SIGNUP = 1,
    //<username:20B><hashed password:32B>
    MESSAGE_LOGIN = 2,
    //<username:20B><hashed password:32B>
    MESSAGE_LOGOUT = 3,
    //
    MESSAGE_HANGING = 4,
    //[username:20B]
    MESSAGE_USERINFO = 5
    //<username:20B>
} MessageType;


typedef struct 
{
    MessageType type;
    uint32_t payload_size;
} NetworkHeader;

void NetworkSerializeMessage(MessageType type,const char* payload,char* dst);

MessageType NetworkDeserializeMessage(const char* src, char* payload);