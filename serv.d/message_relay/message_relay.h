#pragma once
#include <time.h>
#include "../index/index.h"

typedef enum 
{
    RELAY_MESSAGE_TEXT = 'T',
    RELAY_MESSAGE_FILE = 'F'
} RelayMessageType;

/**
 * @brief used to store a message
 * 
 */
typedef struct 
{
    /**
     * @brief sender user name
     * 
     */
    UserName src;

    /**
     * @brief receiver user name
     * 
     */
    UserName dst;

    /**
     * @brief message creation timestamp
     * 
     */
    time_t timestamp;

    /**
     * @brief defines if the message contains a file or some text
     * RELAY_MESSAGE_TEXT: data contains the text (this must be null terminated), filename must be NULL,
     *  preferably data_size should be equal to strlen(data)
     * RELAY_MESSAGE_FILE: data contains the file, filename contains the file name (null terminated) 
     *  and data_size stores the file size
     */
    RelayMessageType type;

    /**
     * @brief if type == RELAY_MESSAGE_FILE this contains the file name, is NULL otherwise
     * 
     * (DYNAMICALLY ALLOCATED, must be NULL if free)
     */
    char* filename;

    /**
     * @brief if type == RELAY_MESSAGE_FILE this contains the size of the file in bytes, otherwise this should contain strlen(data)
     * 
     */
    size_t data_size;

    /**
     * @brief if type == RELAY_MESSAGE_TEXT this contains the text (must be null terminated)
     * if type == RELAY_MESSAGE_FILE this contains the file data, the length of this buffer must be specified in data_size
     * 
     * (DYNAMICALLY ALLOCATED, must be NULL if free)
     */
    uint8_t* data;
} RelayMessage;