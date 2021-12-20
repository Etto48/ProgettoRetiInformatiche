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
     * this message should only be sent from a devie to the server
     */
    MESSAGE_USERINFO_REQ = 5,

    /**
     * @brief payload: <IPv4:4B><port:2B>
     * ip and port on which the other device is listening for connections
     * this message should only be sent from the server to a device
     */
    MESSAGE_USERINFO_RES = 6,

    /**
     * @brief payload: <username:20B><timestamp:8B>
     * used to send a notification of last message recived from <username>
     * this message should be sent only from the server when <username> sends MESSAGE_HANGING with a username
     */
    MESSAGE_SYNCREAD = 7,

    /**
     * @brief payload: <src username:20B><dst username:20B><timestamp:8B><'T'|'F':1B><message:variable>
     * used to send a message from <src username> to <dst username> at time <timestamp>, use T if it's text or F if it's a file
     * if this message is sent from a device <src username> is ignored, if it is sent to a device <dst username> is ignored
     * those are useful only if the server is used as relay
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
 * @param[in] type message type
 * @param[in] payload message payload (refer to MessageType documentation), this should be already serialized
 * @param[out] dst_stream destination stream on which we serialize the input; THIS WILL BE DINAMICALLY ALLOCATED, REMEMBER TO FREE IT
 */
void NetworkSerializeMessage(MessageType type,const char* payload, uint8_t** dst_stream);

/**
 * @brief deserialize a message from a host-independent format
 * 
 * @param[in] src_stream source stream from which we deserialize the input
 * @param[out] type deserialized message type; THIS WILL BE DINAMICALLY ALLOCATED, REMEMBER TO FREE IT
 * @param[out] payload message payload, you have to deserialize this; THIS WILL BE DINAMICALLY ALLOCATED, REMEMBER TO FREE IT
 */
void NetworkDeserializeMessage(const uint8_t* src_stream, MessageType** type, char** payload);

/**
 * @brief send MESSAGE_RESPONSE
 * 
 * @param socket socket fd on which we send the message
 * @param ok is the status ok?
 * @return true if the message was sent correctly
 */
bool NetworkSendMessageResponse(int socket,bool ok);

/**
 * @brief send MESSAGE_SIGNUP
 * 
 * @param socket socket fd on which we send the message
 * @param username username
 * @param password password (it must be hashed)
 * @return true if the message was sent correctly
 */
bool NetworkSendMessageSignup(int socket, UserName username, Password password);

/**
 * @brief send MESSAGE_LOGIN
 * 
 * @param socket socket fd on which we send the message
 * @param port port on which the device will be listening for other device
 * @param username username
 * @param password password (it must be hashed)
 * @return true if the message was sent correctly
 */
bool NetworkSendMessageLogin(int socket, uint16_t port, UserName username, Password password);

/**
 * @brief send MESSAGE_LOGOUT
 * 
 * @param socket socket fd on which we send the message 
 * @return true if the message was sent correctly
 */
bool NetworkSendMessageLogout(int socket);

/**
 * @brief send MESSAGE_HANGIN
 * 
 * @param socket socket fd on which we send the message
 * @param username if set to NULL send MESSAGE_HANGING without a username
 * @return true if the message was sent correctly
 */
bool NetworkSendMessageHanging(int socket, UserName* username);

/**
 * @brief send MESSAGE_USERINFO_REQ
 * 
 * @param socket socket fd on which we send the message
 * @param username username
 * @return true if the message was sent correctly
 */
bool NetworkSendMessageUserinfoReq(int socket, UserName username);

/**
 * @brief send MESSAGE_USERINFO_RES
 * 
 * @param socket socket fd on which we send the message
 * @param ip ip of the device
 * @param port port on which the device is listening
 * @return true if the message was sent correctly 
 */
bool NetworkSendMessageUserinfoRes(int socket, uint32_t ip, uint16_t port);

/**
 * @brief send MESSAGE_SYNCREAD
 * 
 * @param socket socket fd on which we send the message
 * @param username username of the message sender
 * @param timestamp timestamp of the last message read
 * @return true if the message was sent correctly 
 */
bool NetworkSendMessageSyncread(int socket, UserName username, time_t timestamp);

/**
 * @brief send MESSAGE_DATA containing text
 * 
 * @param socket socket fd on which we send the message
 * @param src_username username of the message sender
 * @param dst_username username of the message receiver
 * @param timestamp timestamp of the message
 * @param text pointer from which we read a null terminated string to send
 * @return true if the message was sent correctly
 */
bool NetworkSendMessageDataText(int socket, UserName src_username, UserName dst_username, time_t timestamp, char* text);

/**
 * @brief send MESSAGE_DATA containing a file
 * 
 * @param socket socket fd on which we send the message
 * @param src_username username of the message sender
 * @param dst_username username of the message receiver
 * @param timestamp timestamp of the message
 * @param filename path to the file to send
 * @return true if the message was sent correctly
 */
bool NetworkSendMessageDataFile(int socket, UserName src_username, UserName dst_username, time_t timestamp, char* filename);