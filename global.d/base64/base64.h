#pragma once
//Source: https://github.com/joedf/base64.c
//See global.d/base64/LICENSE

#include <stdio.h>

/**
 * @brief calculate the required memory to save the encoded data
 * 
 * @param in_size the number bytes to be encoded
 * @return the recommended memory size to be allocated for the output buffer excluding the null byte
 */
unsigned int B64EncSize(unsigned int in_size);

/**
 * @brief calculate the required memory to save the decoded data
 * 
 * @param in_size the number bytes to be decoded
 * @return the recommended memory size to be allocated for the output buffer
 */
unsigned int B64DecSize(unsigned int in_size);

/**
 * @brief encode "raw" to base64
 * 
 * @param in buffer of "raw" binary to be encoded
 * @param in_len number of bytes to be encoded
 * @param out pointer to buffer with enough memory, user is responsible for memory allocation, receives null-terminated string
 * @return size of output including null byte
 */
unsigned int B64Encode(const unsigned char* in, unsigned int in_len, unsigned char* out);

/**
 * @brief decode base64 to "raw"
 * 
 * @param in buffer of base64 string to be decoded
 * @param in_len number of bytes to be decoded
 * @param out pointer to buffer with enough memory, user is responsible for memory allocation, receives "raw" binary
 * @return size of output including null byte
 */
unsigned int B64Decode(const unsigned char* in, unsigned int in_len, unsigned char* out);
