#pragma once
#include <sys/socket.h>
#include <arpa/inet.h>
#include <stdint.h>
#include <sys/select.h>
#include "../integrity/integrity.h"
#include "../message_relay/message_relay.h"
#include "../../global.d/debug/debug.h"
#include "../../global.d/network_tools/network_tools.h"
#include "../cli/cli.h"
#include "../../global.d/network_tools/network_common/network_common.h"

#define AUTOSAVE_TIME_INTERVAL 60

/**
 * @brief if set to true the program will execute SaveAndExit ASAP
 *
 */
extern bool NetworkShutdownRequested;

/**
 * @brief handles a generic message sending it to the appropriate handler
 *
 * @param sockfd socket file descriptor on which the message was received
 */
void NetworkHandleNewMessage(int sockfd);

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

/**
 * @brief called by NetworkMainLoop when we have no requests
 *
 */
void NetworkFreeTime();

/**
 * @brief called when a user disconnects
 *
 * @param sockfd socket file descriptor of the user
 */
void NetworkDeletedConnectionHook(int sockfd);