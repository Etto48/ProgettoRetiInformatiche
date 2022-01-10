#pragma once
#include <stdint.h>
#include "sha256/sha256.h"

#define USERNAME_MAX_LENGTH 20
#define PASSWORD_SIZE SIZE_OF_SHA_256_HASH

/**
 * @brief we use this to store a username, the size of the array is USERNAME_MAX_LENGTH + 1 because of the null termination
 *
 */
typedef struct
{
    char str[USERNAME_MAX_LENGTH + 1];
} UserName;

/**
 * @brief we use this to store a password hashed with SHA256
 *
 */
typedef struct
{
    uint8_t data[PASSWORD_SIZE];
} Password;