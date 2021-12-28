#pragma once
#include <sys/socket.h>
#include <arpa/inet.h>
#include <stdint.h>
#include <sys/select.h>
#include "../../global.d/network_tools/network_tools.h"
#include "../../global.d/network_tools/network_common/network_common.h"

//NOTE: we do not handle messages from the server here, those are handled with blocking behaviour when a command is executed inside cli/*

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