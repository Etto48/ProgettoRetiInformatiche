#pragma once
#include <sys/socket.h>
#include <arpa/inet.h>
#include <stdint.h>
#include <sys/select.h>
#include "../cli/cli.h"
#include "../chat_manager/chat_manager.h"
#include "../../global.d/network_tools/network_tools.h"
#include "../../global.d/network_tools/network_common/network_common.h"


#define SERVER_ADDRESS "0.0.0.0"

typedef struct _ServerMessage
{
    MessageHeader header;
    uint8_t* payload;
    struct _ServerMessage* next;
} ServerMessage;

typedef struct 
{
    bool connected;
    int sockfd;
    struct sockaddr_in address;
    ServerMessage* message_list_head;
    ServerMessage* message_list_tail;
} ServerConnectionInfo; 

extern ServerConnectionInfo NetworkServerInfo;
/**
 * @brief handles a generic message sending it to the appropriate handler
 * 
 * @param sockfd socket file descriptor on which the message was received
 * @param master master file descriptor set to delete old connections
 */
void NetworkHandleNewMessage(int sockfd, fd_set* master);

/**
 * @brief handles the arrival of a message
 * 
 * @param sockfd socket file descriptor on which the MESSAGE_DATA arrived
 */
void NetworkHandleData(int sockfd);

/**
 * @brief handles the request to add a user in a group chat
 * 
 * @param sockfd socket file descriptor on which the MESSAGE_USERINFO_REQ arrived
 */
void NetworkHandleUserinfoReq(int sockfd);

/**
 * @brief notify the device of an error in the request
 * 
 * @param sockfd socket file descriptor on which the error occurred
 */
void NetworkHandleError(int sockfd);

/**
 * @brief connect to the server
 * 
 * @param port server listening port
 * @return true if successfully connected
 */
bool NetworkStartServerConnection(uint16_t port);

/**
 * @brief we call this when we detect an error in the connection with the server, so we prepare for a future reconnection
 * 
 */
void NetworkServerDisconnected();

/**
 * @brief called by NetworkMainLoop when we have no requests
 * 
 */
void NetworkFreeTime();

/**
 * @brief we use this to receive exactly one packet from the server, then we put it at the end of the NetworkServerInfo.message_list_*
 * this calls blocks the program untill a whole message has been received or the connection is closed (updating the connection status in
 * NetworkServerInfo.connected)
 * 
 * @return true if the message was received correctly
 */
bool NetworkReceiveOneFromServer();

/**
 * @brief receive every message from the server until MESSAGE_RESPONSE ok, if MESSAGE_RESPONSE error 
 * is detected or there was some error during trasmission, the received messages are deleted
 * 
 * @param expected_type type of the message to wait to end reception, if set to MESSAGE_RESPONSE we use the thefault behaveyour
 * @return true if everything ok and received until MESSAGE_RESPONSE ok
 */
bool NetworkReceiveResponseFromServer(MessageType expected_type);

/**
 * @brief we use this to delete the oldest packet received from the server, we need to call this to free a message in the message list
 * 
 * @return true if the list was not empty
 */
bool NetworkDeleteOneFromServer();

/**
 * @brief use this to delete newest packet received, used internally
 * 
 */
void NetworkDeleteOneFromServerTail();

/**
 * @brief we check the list of messages from the server and if it is not empty we handle the first message (only if it is 
 * MESSAGE_SYNCREAD)
 * 
 */
void NetworkHandleServerNotifications();

/**
 * @brief when we get a syncread notificatioon from the server we use this to handle it
 * 
 * @param syncread_message pointer to the notification, remember to free it (with NetworkDeleteOneFromServer) after the call to this function
 */
void NetworkHandleSyncread(ServerMessage* syncread_message);

/**
 * @brief try to log in to the server if you are connected, used in cli.c and in network.c when we reconnect to the server after losing connection
 * 
 * @param username username used for login
 * @param password password (hashed) used for login
 * @return true if the login was successful
 */
bool NetworkAutoLogin(UserName username, Password password);