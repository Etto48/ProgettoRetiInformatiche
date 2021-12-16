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

#define USERNAME_MAX_LENGTH 20
#define PASSWORD_MAX_LENGTH 32

typedef struct _UserName
{
    char str[USERNAME_MAX_LENGTH+1];
} UserName;

typedef struct _Password
{
    char str[PASSWORD_MAX_LENGTH+1];
} Password;

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
 * @return true if loaded successful
 */
bool AuthLoad(const char* filename);

/**
 * @brief save auth information to a file
 * 
 * @param filename path of the file
 * @return true if everything ok
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
 * @param port listening port of the user
 * @return true if correctly authenticated and not already logged
 */
bool IndexLogin(UserName username, Password password, uint16_t port);

/**
 * @brief logout a user
 * 
 * @param username username
 * @return true if everything ok and user was logged
 */
bool IndexLogout(UserName username);

/**
 * @brief find info about a user
 * 
 * @param username username used as key
 * @return IndexEntry* pointer to the user if found, NULL otherwise
 */
IndexEntry* IndexFind(UserName username);

/**
 * @brief check if a user is online
 * 
 * @param user user pointer to check, returned from IndexFind
 * @return true if user is online, false otherwise
 */
bool IndexIsOnline(IndexEntry* user);