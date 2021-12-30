#pragma once
#include "../../global.d/network_tools/network_tools.h"

/**
 * @brief add a new message
 * 
 * @param mh message header (sould be only MESSAGE_DATA)
 * @param payload pointer to the payload containing the message data
 */
void ChatReceiveMessage(MessageHeader mh, uint8_t* payload);

/**
 * @brief load from file the chat from src
 * 
 * @param src 
 */
void ChatLoad(UserName src);

/**
 * @note we should save the chat to a different user in a different file
 * 
 */
