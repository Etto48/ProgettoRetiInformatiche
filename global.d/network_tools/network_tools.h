#pragma once

#include <stdint.h>
#include <arpa/inet.h>
#include <sys/stat.h>
#include <stdint.h>
#include <stdbool.h>
#include <string.h>
#include <malloc.h>
#include <fcntl.h>
#include <unistd.h>
#include "../globalDefine.h"
#include "../debug/debug.h"
#include "../tools/tools.h"
#include "../base64/base64.h"

#define NETWORK_SERIALIZED_HEADER_SIZE 5

#define MESSAGE_RESPONSE_SIZE sizeof(uint8_t)
#define MESSAGE_SIGNUP_SIZE (USERNAME_MAX_LENGTH+PASSWORD_MAX_LENGTH)
#define MESSAGE_LOGIN_SIZE (sizeof(uint16_t)+USERNAME_MAX_LENGTH+PASSWORD_MAX_LENGTH)
#define MESSAGE_LOGOUT_SIZE 0
#define MESSAGE_HANGING_MIN_SIZE 0
#define MESSAGE_HANGING_MAX_SIZE USERNAME_MAX_LENGTH
#define MESSAGE_USERINFO_REQ_SIZE USERNAME_MAX_LENGTH
#define MESSAGE_USERINFO_RES_SIZE (sizeof(uint32_t)+sizeof(uint16_t))
#define MESSAGE_SYNCREAD_SIZE (USERNAME_MAX_LENGTH+sizeof(uint64_t))
#define MESSAGE_DATA_TEXT_MIN_SIZE (USERNAME_MAX_LENGTH + USERNAME_MAX_LENGTH + sizeof(uint64_t) + sizeof(uint8_t))
#define MESSAGE_DATA_FILE_MIN_SIZE (USERNAME_MAX_LENGTH + USERNAME_MAX_LENGTH + sizeof(uint64_t) + sizeof(uint8_t) + sizeof(uint32_t))

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
     * expect a MESSAGE_RESPONSE ok if signed up correctly
     */
    MESSAGE_SIGNUP = 1,

    /**
     * @brief payload: <port:2B><username:20B><hashed password:32B>
     * used for the login message or for authenticating to another device (only username is read)
     * expect a MESSAGE_RESPONSE ok if logged in correctly
     */
    MESSAGE_LOGIN = 2,

    /**
     * @brief payload: empty
     * used for the logout message
     * expect a MESSAGE_RESPONSE ok if logged out correctly
     */
    MESSAGE_LOGOUT = 3,

    /**
     * @brief payload: [username:20B]
     * used for the hanging and show command (response to the last one will be served with multiple MESSAGE_DATA messages)
     * 
     * if no username was specified expect a sequence of MESSAGE_HANGING with username followed by a MESSAGE_RESPONSE ok
     * if a username was provided expect a sequence of MESSAGE_DATA followed by a MESSAGE_RESPONSE ok
     */
    MESSAGE_HANGING = 4,

    /**
     * @brief payload: <username:20B>
     * used to request info about ip/port of a user
     * this message should be sent from a device to the server
     * expect a MESSAGE_USERINFO_RES
     */
    MESSAGE_USERINFO_REQ = 5,

    /**
     * @brief payload: <IPv4:4B><port:2B>
     * ip and port on which the other device is listening for connections
     * if client is offline, the payload must be filled with 0
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
     * @brief payload: <src username:20B><dst username:20B><timestamp:8B><'T':1B<text:variable>|'F':1B<file name length:4B><file content:variable>>
     * used to send a message from <src username> to <dst username> at time <timestamp>, use T if it's text or F if it's a file
     * if this message is sent from a device <src username> is ignored, if it is sent to a device <dst username> is ignored
     * those are useful only if the server is used as relay
     */
    MESSAGE_DATA = 8

    //devices must never respond to the server
} MessageType;

#define MAX_MESSAGE_TYPE 8

/**
 * @brief this header is sent before any message on the network, when serialized it is NETWORK_SERIALIZED_HEADER_SIZE bytes long
 * serialized header format: <type:1B><payload_size:4B>
 *
 */
typedef struct
{
    MessageType type;
    uint32_t payload_size;
} MessageHeader;

/**
 * @brief serialize a message in a host-independent format
 * WARNINGS:
 *  in case of MESSAGE_HANGING: payload should be NULL or an empty string if you don't want to send a username
 *  in case of MESSAGE_DATA: payload should be null terminated, if it is a file, you must encode it in base64
 *
 * @param[in] type message type
 * @param[in] payload message payload (refer to MessageType documentation), this should be already serialized
 * @param[out] dst_stream destination stream on which we serialize the input; THIS WILL BE DINAMICALLY ALLOCATED, REMEMBER TO FREE IT
 * @return total size of the message (header + payload)
 */
size_t NetworkSerializeMessage(MessageType type, const uint8_t *payload, uint8_t **dst_stream);

/**
 * @brief we get the header in network format (serialized) from src_stream and deserialze it
 *
 * @param src_stream source stream from which we deserialize the input
 * @return deserialized header
 */
MessageHeader NetworkDeserializeHeader(const uint8_t *src_stream);

/**
 * @brief deserialize a MESSAGE_RESPONSE
 * 
 * @param[in] payload_size header field payload_size
 * @param[in] payload pointer to the payload
 * @param[out] ok response ok?
 */
void NetworkDeserializeMessageResponse(size_t payload_size, const uint8_t *payload, bool *ok);

/**
 * @brief deserialize a MESSAGE_SIGNUP
 * 
 * @param[in] payload_size header field payload_size
 * @param[in] payload pointer to the payload
 * @param[out] username sender username
 * @param[out] password sender password (hashed) 
 */
void NetworkDeserializeMessageSignup(size_t payload_size, const uint8_t *payload, UserName *username, Password *password);

/**
 * @brief deserialize a MESSAGE_LOGIN
 * 
 * @param[in] payload_size header field payload_size
 * @param[in] payload pointer to the payload
 * @param[out] port port on which the device listens to other devices
 * @param[out] username sender username
 * @param[out] password sender password (hashed)
 */
void NetworkDeserializeMessageLogin(size_t payload_size, const uint8_t *payload, uint16_t* port, UserName *username, Password *password);

/**
 * @brief deserialize a MESSAGE_HANGING
 * 
 * @param[in] payload_size header field payload_size
 * @param[in] payload pointer to the payload
 * @param[out] username message source username to check for, if empty no username was provided
 */
void NetworkDeserializeMessageHanging(size_t payload_size, const uint8_t *payload, UserName *username);

/**
 * @brief deserialize a MESSAGE_USERINFO_REQ
 * 
 * @param[in] payload_size header field payload_size
 * @param[in] payload pointer to the payload
 * @param[out] username requested user
 */
void NetworkDeserializeMessageUserinfoReq(size_t payload_size, const uint8_t *payload, UserName *username);

/**
 * @brief deserialize a MESSAGE_USERINFO_RES
 * 
 * @param[in] payload_size header field payload_size
 * @param[in] payload pointer to the payload
 * @param[out] ip requested user ip (if user was not found this should be 0)
 * @param[out] port requested user port (if user not found this MUST be 0)
 */
void NetworkDeserializeMessageUserinfoRes(size_t payload_size, const uint8_t *payload, uint32_t *ip, uint16_t *port);

/**
 * @brief deserialize a MESSAGE_SYNCREAD
 * 
 * @param[in] payload_size header field payload_size
 * @param[in] payload pointer to the payload
 * @param[out] username message receiver username
 * @param[out] timestamp timestamp of the last message read from username
 */
void NetworkDeserializeMessageSyncread(size_t payload_size, const uint8_t *payload, UserName *username, time_t *timestamp);

/**
 * @brief check if a MESSAGE_DATA is containing a file or just text
 * 
 * @param payload_size header field payload_size
 * @param payload pointer to the payload
 * @return true if payload contains a file, false otherwise
 */
bool NetworkMessageDataContainsFile(size_t payload_size, const uint8_t *payload);

/**
 * @brief get text length (excluding null termination) contained in the payload of a MESSAGE_DATA (text)
 * 
 * @param payload_size header field payload_size
 * @param payload pointer to the payload
 * @return text length
 */
size_t NetworkMessageDataTextLength(size_t payload_size, const uint8_t *payload);

/**
 * @brief get filename length (excluding null termination) contained in the payload of a MESSAGE_DATA (file)
 * 
 * @param payload_size header field payload_size
 * @param payload pointer to the payload
 * @return file length
 */
size_t NetworkMessageDataFilenameLength(size_t payload_size, const uint8_t *payload);

/**
 * @brief get file size after decoding from base64 contained in the payload of a MESSAGE_DATA (file)
 * 
 * @param payload_size header field payload_size
 * @param payload pointer to the payload
 * @return file size
 */
size_t NetworkMessageDataFileSize(size_t payload_size, const uint8_t *payload);

/**
 * @brief deserialize a MESSAGE_DATA (text)
 * 
 * @param[in] payload_size header field payload_size
 * @param[in] payload pointer to the payload
 * @param[out] src_username sender username
 * @param[out] dst_username receiver username
 * @param[out] timestamp timestamp of the message creation
 * @param[out] text buffer where the text will be saved, use NetworkMessageDataTextLength to get the length to allocate for this buffer (remember +1 for null termination)
 */
void NetworkDeserializeMessageDataText(size_t payload_size, const uint8_t *payload, UserName *src_username, UserName *dst_username, time_t *timestamp, char *text);

/**
 * @brief deserialize a MESSAGE_DATA (file)
 * 
 * @param[in] payload_size header field payload_size
 * @param[in] payload pointer to the payload
 * @param[out] src_username sender username
 * @param[out] dst_username receiver username
 * @param[out] timestamp timestamp of the message creation
 * @param[out] filename buffer where the file name will be saved, use NetworkMessageDataFilenameLength to get the correct length to allocate this buffer (remember +1 for null termination)
 * @param[out] data buffer where the file content will be saved, use NetworkMessageDataFileSize to get the correct length to allocate this buffer
 */
void NetworkDeserializeMessageDataFile(size_t payload_size, const uint8_t *payload, UserName *src_username, UserName *dst_username, time_t *timestamp, char *filename, uint8_t *data);

/**
 * @brief send MESSAGE_RESPONSE
 *
 * @param sockfd socket fd on which we send the message
 * @param ok is the status ok?
 * @return true if the message was sent correctly
 */
bool NetworkSendMessageResponse(int sockfd, bool ok);

/**
 * @brief send MESSAGE_SIGNUP
 *
 * @param sockfd socket fd on which we send the message
 * @param username username
 * @param password password (it must be hashed)
 * @return true if the message was sent correctly
 */
bool NetworkSendMessageSignup(int sockfd, UserName username, Password password);

/**
 * @brief send MESSAGE_LOGIN
 *
 * @param sockfd socket fd on which we send the message
 * @param port port on which the device will be listening for other device
 * @param username username
 * @param password password (it must be hashed)
 * @return true if the message was sent correctly
 */
bool NetworkSendMessageLogin(int sockfd, uint16_t port, UserName username, Password password);

/**
 * @brief send MESSAGE_LOGOUT
 *
 * @param sockfd socket fd on which we send the message
 * @return true if the message was sent correctly
 */
bool NetworkSendMessageLogout(int sockfd);

/**
 * @brief send MESSAGE_HANGIN
 *
 * @param sockfd socket fd on which we send the message
 * @param username if set to NULL send MESSAGE_HANGING without a username
 * @return true if the message was sent correctly
 */
bool NetworkSendMessageHanging(int sockfd, UserName *username);

/**
 * @brief send MESSAGE_USERINFO_REQ
 *
 * @param sockfd socket fd on which we send the message
 * @param username username
 * @return true if the message was sent correctly
 */
bool NetworkSendMessageUserinfoReq(int sockfd, UserName username);

/**
 * @brief send MESSAGE_USERINFO_RES
 *
 * @param sockfd socket fd on which we send the message
 * @param ip ip of the device (set to 0 if device is offline)
 * @param port port on which the device is listening (MUST BE 0 IF DEVICE IS OFFLINE)
 * @return true if the message was sent correctly
 */
bool NetworkSendMessageUserinfoRes(int sockfd, uint32_t ip, uint16_t port);

/**
 * @brief send MESSAGE_SYNCREAD
 *
 * @param sockfd socket fd on which we send the message
 * @param username username of the message sender
 * @param timestamp timestamp of the last message read
 * @return true if the message was sent correctly
 */
bool NetworkSendMessageSyncread(int sockfd, UserName username, time_t timestamp);

/**
 * @brief send MESSAGE_DATA containing text
 *
 * @param sockfd socket fd on which we send the message
 * @param src_username username of the message sender
 * @param dst_username username of the message receiver
 * @param timestamp timestamp of the message
 * @param text pointer from which we read a null terminated string to send
 * @return true if the message was sent correctly
 */
bool NetworkSendMessageDataText(int sockfd, UserName src_username, UserName dst_username, time_t timestamp, char *text);

/**
 * @brief send MESSAGE_DATA containing a file
 *
 * @param sockfd socket fd on which we send the message
 * @param src_username username of the message sender
 * @param dst_username username of the message receiver
 * @param timestamp timestamp of the message
 * @param filename path to the file to send
 * @return true if the message was sent correctly
 */
bool NetworkSendMessageDataFile(int sockfd, UserName src_username, UserName dst_username, time_t timestamp, const char *filename);

/**
 * @brief send MESSAGE_DATA containing a file that was already read from the disk
 * 
 * @param sockfd socket fd on which we send the message
 * @param src_username username of the message sender
 * @param dst_username username of the message receiver
 * @param timestamp timestamp of the message
 * @param filename string containing the file name
 * @param file_size size of the data buffer
 * @param data buffer containing the file
 * @return true if the message was sent correctly
 */
bool NetworkSendMessageDataFileBuffer(int sockfd, UserName src_username, UserName dst_username, time_t timestamp, const char *filename, size_t file_size, const uint8_t* data);