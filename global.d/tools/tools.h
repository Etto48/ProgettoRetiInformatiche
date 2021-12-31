#pragma once

#include <stddef.h>
#include <stdint.h>
#include <string.h>
#include "../sha256/sha256.h"
#include "../globalDefine.h"

/**
 * @brief finds the starting index of the basename of the file
 * 
 * @param path full path of the file
 * @return starting index of the basename 
 */
size_t ToolsBasename(const char* path);

/**
 * @brief host to network quadword
 * 
 * @param quad data in host format
 * @return data in network format
 */
uint64_t htonq(uint64_t quad);

/**
 * @brief network to host quadword
 * 
 * @param quad data in network format
 * @return data in host format
 */
uint64_t ntohq(uint64_t quad);

/**
 * @brief Create a UserName struct
 * 
 * @param username string, max 20 chars
 * @return created struct
 */
UserName CreateUserName(const char* username);

/**
 * @brief Create a Password struct
 * 
 * @param password not hashed password
 * @return created struct
 */
Password CreatePassword(const char* password);