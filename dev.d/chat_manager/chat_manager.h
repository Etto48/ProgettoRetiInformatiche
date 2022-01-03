#pragma once
#include <time.h>
#include "../../global.d/network_tools/network_tools.h"

/**
 * @brief specify if a message contains text or a file
 * 
 */
typedef enum 
{
    CHAT_MESSAGE_TEXT,
    CHAT_MESSAGE_FILE
} ChatMessageType;

/**
 * @brief specify if you sent or received the message
 * 
 */
typedef enum
{
    CHAT_MESSAGE_SENT,
    CHAT_MESSAGE_RECEIVED
} ChatMessageDirection;

/**
 * @brief entry of a list of messages in a chat
 * 
 */
typedef struct _ChatMessage
{
    /**
     * @brief specify if you sent or received the message
     * 
     */
    ChatMessageDirection direction;

    /**
     * @brief true if the message was read by the other end
     * 
     */
    bool read;

    /**
     * @brief specify if content is just text or is a path on your system directing to the file
     * 
     */
    ChatMessageType type;

    /**
     * @brief when the message was sent
     * 
     */
    time_t timestamp;

    /**
     * @brief text or path to a file, depending on type
     * 
     */
    char* content;
    
    /**
     * @brief pointer to the next entry
     * 
     */
    struct _ChatMessage* next;
} ChatMessage;

/**
 * @brief entry of the list of chats, contains a list of messages
 * 
 */
typedef struct _Chat
{
    /**
     * @brief the other user with who you are chatting
     * 
     */
    UserName dst;

    /**
     * @brief first entry of the message list
     * 
     */
    ChatMessage* head;

    /**
     * @brief last entry of the message list
     * 
     */
    ChatMessage* tail;

    /**
     * @brief next entry in the chat list
     * 
     */
    struct _Chat* next;
} Chat;

/**
 * @brief when you are chatting with someone their info are stored in this struct
 * 
 */
typedef struct _ChatTarget
{
    /**
     * @brief target username
     * 
     */
    UserName dst;

    /**
     * @brief target ip
     * 
     */
    uint32_t ip;

    /**
     * @brief target port
     * 
     */
    uint16_t port;

    /**
     * @brief next item in the list
     * 
     */
    struct _ChatTarget* next;
} ChatTarget;

/**
 * @brief list of all the loaded chats, if a chat with a certain user is not there we must first look for it in the files (call chat load)
 * 
 */
extern Chat* ChatList;

/**
 * @brief list of all the users with who you are chatting
 * 
 */
extern ChatTarget* ChatTargetList;

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
 * @return true if the chat with user was found
 */
bool ChatLoad(UserName user);

/**
 * @brief get a line from stdin and write it to the ative chat
 * 
 */
void ChatWrite();

/**
 * @brief ask to the server how to reach a specified username and add it's info the the chat list
 * 
 * @param username target username
 * @return true if user added correctly,
 */
bool ChatAdd(UserName username);

/**
 * @brief close the chat
 * 
 */
void ChatQuit();

/**
 * @brief print a message formatting it correctly
 * 
 * @param msg message to print
 * @param dst other end username
 */
void ChatPrintMessage(ChatMessage msg, UserName dst);

/**
 * @note we should save the chat to a different user in a different file
 * 
 */
