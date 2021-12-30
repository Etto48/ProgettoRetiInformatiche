#pragma once
#include "../../global.d/network_tools/network_tools.h"

/**
 * @brief add a new message
 * 
 * @param payload_size number of bytes of the payload 
 * @param payload pointer to the payload containing the message data
 */
void ChatReceiveMessage(size_t payload_size, uint8_t* payload);

/**
 * @brief for each message to dst user, set those before timestamp as received
 * 
 * @param dst receiver user
 * @param timestamp timestamp of the last received message
 */
void ChatHandleSyncread(UserName dst, time_t timestamp);

/**
 * @brief load from file the chat with user
 * 
 * @param user which chat we need to load from file
 */
void ChatLoad(UserName user);

/**
 * @note we should save the chat to a different user in a different file
 * 
 */
