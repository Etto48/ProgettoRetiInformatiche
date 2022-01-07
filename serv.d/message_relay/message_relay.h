#pragma once
#include <time.h>
#include <arpa/inet.h>
#include "../index/index.h"
#include "../../global.d/network_tools/network_tools.h"
#include "../../global.d/network_tools/network_common/network_common.h"

#define RELAY_FILE "./Relay.lst"
#define RELAY_SYNCREAD_FILE "./Syncread.lst"

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
 * @brief when we can't send a syncread message we store it here and send it at login
 * 
 */
typedef struct _RelaySyncreadNotice
{
    time_t timestamp;
    UserName src;
    UserName dst;
    struct _RelaySyncreadNotice* next;
} RelaySyncreadNotice;

/**
 * @brief if a device fails to send a message to a client it must send it to the server
 * that message is stored here
 * when a hanging request is received we check this list
 * when the server is shut down we must save this to a file
 * 
 */
extern RelayMessage* RelayHangingList;

/**
 * @brief this is a list of buffered syncread messages
 * 
 */
extern RelaySyncreadNotice* RelaySyncreadList;

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
 * @brief load RelayHangingList from file, RelayHangingList must be empty
 * 
 * @param filename file path
 * @return true if loaded correctly
 */
bool RelayLoad(const char* filename);

/**
 * @brief save RelayHangingList to file
 * 
 * @param filename file path
 * @return true if saved correctly
 */
bool RelaySave(const char* filename);

/**
 * @brief free every entry of RelayHangingList
 * 
 */
void RelayFree();

/**
 * @brief add an entry to RelaySyncreadList without checking anything
 * 
 * @param src message sender
 * @param dst message receiver
 * @param timestamp last received message timestamp
 */
void RelaySyncreadAdd(UserName src, UserName dst, time_t timestamp);

/**
 * @brief when a relay message is read if possible we try to send a message syncread to src,
 * we update the entry in the list otherwise
 * 
 * @param src message sender, the notification will be delivered to it
 * @param dst message receiver
 * @param timestamp last received message timestamp
 */
void RelaySyncreadEdit(UserName src, UserName dst, time_t timestamp);

/**
 * @brief find an entry in RelaySyncreadList
 * 
 * @param src message sender, set to NULL to ignore it
 * @param dst message receiver, set to NULL to ignore it
 * @return pointer to the first entry found, NULL if no entry was found
 */
RelaySyncreadNotice* RelaySyncreadFind(UserName* src, UserName* dst);

/**
 * @brief delete an entry in RelaySyncreadList after it is delivered
 * 
 * @param src message sender, set to NULL to ignore it
 * @param dst message receiver, set to NULL to ignore it
 */
void RelaySyncreadDelete(UserName* src, UserName* dst);

/**
 * @brief load RelaySyncreadList from file, RelaySyncreadList must be empty
 * 
 * @param filename path to file
 * @return true if loaded correctly
 */
bool RelaySyncreadLoad(const char* filename);

/**
 * @brief save RelaySyncreadList to file
 * 
 * @param filename path to file
 * @return true if saved correctly
 */
bool RelaySyncreadSave(const char* filename);

/**
 * @brief free every entry of RelaySyncreadList
 * 
 */
void RelaySyncreadFree();