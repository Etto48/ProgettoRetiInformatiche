#pragma once
#include <sys/socket.h>
#include <arpa/inet.h>
#include <stdint.h>
#include <sys/select.h>
#include <stdlib.h>
#include "../../debug/debug.h"
#include "../network_tools.h"

#define NETWORK_MAX_CONNECTIONS 1024

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
 * @brief the index is the socket file descriptor, both the server and the device need this array so it's defined here
 * 
 */
extern NetworkDeviceConnection NetworkConnectedDevices[NETWORK_MAX_CONNECTIONS];

/**
 * @brief is set equal to <port> when we call NetworkMainLoop, this must not be modified
 * 
 */
extern uint16_t NetworkListeningPort;

extern fd_set NetworkMasterFdSet;

/**
 * @brief main loop of the server, here we listen for new connections and we handle open connections
 * 
 * @param port port on which we listen for connections
 */
void NetworkMainLoop(uint16_t port);

/**
 * @brief initialize an entry of NetworkConnectedDevices
 * 
 * @param sockfd socket file descriptor
 * @param addr address
 */
void NetworkNewConnection(int sockfd, struct sockaddr_in addr);

/**
 * @brief destroy an entry of NetworkConnectedDevices
 * 
 * @param sockfd socket file descriptor
 */
void NetworkDeleteConnection(int sockfd);

/**
 * @brief we use this to receive new data and handle the request
 * 
 * @param sockfd socket file descriptor from which we receive data
 */
void NetworkReceiveNewData(int sockfd);

/**
 * @brief check if a socket in NetworkConnectedDevices is logged in
 * 
 * @param sockfd index in NetworkConnectedDevices
 * @return true if logged in, false otherwise
 */
bool NetworkIsSocketLoggedIn(int sockfd);

/**
 * @brief defined inside network.h
 * 
 */
extern void NetworkHandleNewMessage(int sockfd);

/**
 * @brief defined inside cli.h
 * 
 */
extern void CLIHandleInput();

/**
 * @brief defined in integrity.h
 * 
 */
extern void SaveAndExit(int status);

/**
 * @brief defined inside network.h, is called when the program is not busy
 * 
 */
extern void NetworkFreeTime();