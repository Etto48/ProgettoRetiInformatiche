#pragma once
#include <sys/socket.h>
#include <arpa/inet.h>
#include <stdint.h>
#include <sys/select.h>
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
 * @brief we use this to delete the oldest packet received from the server, we need to call this to free a message in the message list
 * 
 * @return true if the list was not empty
 */
bool NetworkDeleteOneFromServer();

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