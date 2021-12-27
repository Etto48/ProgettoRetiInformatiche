#pragma once
#include <time.h>
#include "../index/index.h"

#define RELAY_FILE "./Relay.lst"

typedef enum 
{
    RELAY_MESSAGE_TEXT = 'T',
    RELAY_MESSAGE_FILE = 'F'
} RelayMessageType;

/**
 * @brief used to store a message
 * 
 */
typedef struct _RelayMessage
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
    struct _RelayMessage* next;
} RelayMessage;

/**
 * @brief if a device fails to send a message to a client it must send it to the server
 * that message is stored here
 * when a hanging request is received we check this list
 * when the server is shut down we must save this to a file
 * 
 */
extern RelayMessage* RelayHangingList;

/**
 * @brief add a new message to RelayHangingList
 * 
 * @param src sender username
 * @param dst receiver username
 * @param timestamp creation timestamp
 * @param type message contains text or file
 * @param filename if message contains a file this must contain the filename (null terminated), it must be NULL otherwise
 * @param data_size must contain the size of the file or the strlen(data) if it is text
 * @param data text or file content
 */
void RelayHangingAdd(UserName src, UserName dst, time_t timestamp, RelayMessageType type, char* filename, size_t data_size, uint8_t* data);

/**
 * @brief finds the first message from src to dst
 * 
 * @param message_list a starting point from which we start searching
 * @param src sender username, set it to NULL to not use it
 * @param dst receiver username, set it to NULL to not use it
 * @return pointer to the entry in RelayHangingList
 */
RelayMessage* RelayHangingFindFirst(RelayMessage* message_list, UserName *src, UserName *dst);

/**
 * @brief count how many messages there are from src to dst
 * 
 * @param src sender username, set it to NULL to not use it
 * @param dst receiver username, set it to NULL to not use it
 * @return message count
 */
size_t RelayHangingCount(UserName *src, UserName *dst);

/**
 * @brief find the first occurence and remove it from RelayHangingList
 * WARINING: you must free the return with RelayHangingDestroyMessage
 * @param src sender username, set it to NULL to not use it
 * @param dst receiver username, set it to NULL to not use it
 * @return pointer to the entry removed, YOU MUST FREE IT with RelayHangingDestroyMessage
 */
RelayMessage* RelayHangingPopFirst(UserName *src, UserName *dst);

/**
 * @brief use this function to free a message popped with RelayHangingPopFirst
 * 
 * @param msg message to free
 */
void RelayHangingDestroyMessage(RelayMessage* msg);

/**
 * @brief load RelayHangingList from file
 * 
 * @param filename file path
 */
void RelayLoad(const char* filename);

/**
 * @brief save RelayHangingList to file
 * 
 * @param filename file path
 */
void RelaySave(const char* filename);