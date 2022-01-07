#pragma once
#include <stdint.h>

#define PROMPT "> "
#define USERNAME_MAX_LENGTH 20
#define PASSWORD_MAX_LENGTH 32

typedef struct
{
    char str[USERNAME_MAX_LENGTH + 1];
} UserName;

typedef struct
{
    uint8_t str[PASSWORD_MAX_LENGTH + 1];
} Password;