#pragma once
#include <stdint.h>
#include <stdlib.h>
#include <errno.h>
#include <stdbool.h>
#include <string.h>
#include <malloc.h>
#include <fcntl.h>
#include <unistd.h>
#include <time.h>
#include <sys/stat.h>
#include "../../global.d/tools/tools.h"
#include "../../global.d/globalDefine.h"
#include "../../global.d/debug/debug.h"

#define AUTH_FILE "./Auth.lst"

/**
 * @brief entry for the list of registered users
 * 
 */
typedef struct _AuthEntry
{
    UserName username;
    Password password;
    struct _AuthEntry* next;
} AuthEntry;

/**
 * @brief entry for the list of online users
 * 
 */
typedef struct _IndexEntry 
{
    UserName user_dest;
    uint32_t ip;
    uint16_t port;
    time_t timestamp_login;
    time_t timestamp_logout;
    struct _IndexEntry* next;
} IndexEntry;

extern AuthEntry* AuthList;
extern IndexEntry* IndexList;

/************
 *   AUTH   *
 ************/
/**
 * @brief register a new user 
 * 
 * @param username username
 * @param password password
 * @return true if no error detected, false if username already found
 */
bool AuthRegister(UserName username, Password password);

/**
 * @brief check the auth list for a given user
 * 
 * @param username username
 * @param password password
 * @return true if found, false if not found
 */
bool AuthCheck(UserName username, Password password);

/**
 * @brief load auth information from a file
 * 
 * @param filename path of the file
 * @return true if loaded successfully
 */
bool AuthLoad(const char* filename);

/**
 * @brief save auth information to a file
 * 
 * @param filename path of the file
 * @return true if saved successfully
 */
bool AuthSave(const char* filename);

/*************
 *   INDEX   *
 *************/
/**
 * @brief login a user and check auth info
 * 
 * @param username username
 * @param password password
 * @param ip ip where is located the user
 * @param port listening port of the user
 * @return true if correctly authenticated and not already logged
 */
bool IndexLogin(UserName username, Password password, uint32_t ip, uint16_t port);

/**
 * @brief logout a user
 * 
 * @param username username
 * @return true if everything ok and user was logged
 */
bool IndexLogout(UserName username);

/**
 * @brief like index find but returns NULL if the user is offline
 * 
 * @param username username used as key
 * @return pointer to the user if found online, NULL otherwise
 */
IndexEntry* IndexGetOnline(UserName username);

/**
 * @brief find info about a user
 * 
 * @param username username used as key
 * @return pointer to the user if found, NULL otherwise
 */
IndexEntry* IndexFind(UserName username);

/**
 * @brief check if a user is online
 * 
 * @param user user pointer to check, returned from IndexFind
 * @return true if user is online, false otherwise
 */
bool IndexIsOnline(IndexEntry* user);