#pragma once

#include <stddef.h>
#include <stdint.h>

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