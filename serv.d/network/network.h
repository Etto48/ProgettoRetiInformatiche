#pragma once
#include <sys/socket.h>
#include <arpa/inet.h>
#include <stdint.h>
#include <sys/select.h>
#include "../integrity/integrity.h"
#include "../message_relay/message_relay.h"
#include "../../global.d/debug/debug.h"
#include "../../global.d/network_tools/network_tools.h"

#define NETWORK_MAX_CONNECTIONS 1023

typedef struct
{
    int sockfd;
    struct sockaddr_in address;
    /**
     * @brief if set the user has logged in
     * 
     */
    UserName username;

    /**
     * @brief if this is set to true we consider the transaction started and the 
     * payload_buffer should be allocated of the right size
     * 
     */
    bool header_received;

    /**
     * @brief we store here the message header, this is valid only if header_received == true
     * 
     */
    MessageHeader mh;

    /**
     * @brief how many bytes of data we received, if we received the header and 
     * received_bytes is equal to mh.payload_size we serve the client
     * 
     */
    size_t received_bytes;

    /**
     * @brief we use this to store the data while we wait to receive it (first the header an then the payload),
     * this is dynamically allocated (NETWORK_SERIALIZED_HEADER_SIZE or mh.payload_size bytes), it should be set to NULL if not allocated
     * 
     */
    uint8_t* receive_buffer;
} NetworkDeviceConnection;

/**
 * @brief the index is the socket file descriptor
 * 
 */
extern NetworkDeviceConnection NetworkConnectedDevices[NETWORK_MAX_CONNECTIONS+1];
/**
 * @brief main loop of the server, here we listen for new connections and we handle open connections
 * 
 * @param port port on which we listen for connections
 */
void NetworkServerMainLoop(uint16_t port);

/**
 * @brief initialize an entry of NetworkConnectedDevices
 * 
 * @param sockfd socket file descriptor
 * @param addr address
 * @param master we need to set the fd in the fd_set to use it in select
 */
void NetworkNewConnection(int sockfd, struct sockaddr_in addr, fd_set* master);

/**
 * @brief destroy an entry of NetworkConnectedDevices
 * 
 * @param sockfd socket file descriptor
 * @param master we need to clear the fd in fd_set
 */
void NetworkDeleteConnection(int sockfd, fd_set* master);

/**
 * @brief we use this to receive new data and handle the request
 * 
 * @param sockfd socket file descriptor from which we receive data
 * @param master used if the connection is closed
 */
void NetworkReceiveNewData(int sockfd, fd_set* master);

/**
 * @brief handles a signup request
 * 
 * @param sockfd socket file descriptor on which the MESSAGE_SIGNUP arrived
 */
void NetworkHandleSignup(int sockfd);

/**
 * @brief handles a login request
 * 
 * @param sockfd socket file descriptor on which the MESSAGE_LOGIN arrived
 */
void NetworkHandleLogin(int sockfd);

/**
 * @brief handles a logout request
 * 
 * @param sockfd socket file descriptor on which the MESSAGE_LOGOUT arrived
 */
void NetworkHandleLogout(int sockfd);

/**
 * @brief handles a request for hanging messages
 * 
 * @param sockfd socket file descriptor on which the MESSAGE_HANGING arrived
 */
void NetworkHandleHanging(int sockfd);

/**
 * @brief handles a request for user info
 * 
 * @param sockfd socket file descriptor on which the MESSAGE_USERINFO_REQ arrived
 */
void NetworkHandleUserinfoReq(int sockfd);

/**
 * @brief handles a request to relay a message
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

