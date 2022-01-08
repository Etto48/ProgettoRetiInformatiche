#pragma once

#include <stdbool.h>
#include "../../global.d/command_input/command_input.h"

#define MAX_ARG_NUM 4

/**
 * @brief this enum is used to return the type of the command
 * 
 */
typedef enum
{
    COMMAND_HELP,
    COMMAND_SIGNUP,
    COMMAND_IN,
    COMMAND_HANGING,
    COMMAND_SHOW,
    COMMAND_CHAT,
    COMMAND_RMCHAT,
    COMMAND_USERS,
    COMMAND_CHAT_QUIT,
    COMMAND_CHAT_USERS,
    COMMAND_CHAT_ADD,
    COMMAND_CHAT_FILE,
    COMMAND_OUT,
    COMMAND_ESC,
    COMMAND_ERROR
} DeviceCommand;

/**
 * @brief this struct contains the type of the command and the arguments that were used
 * 
 */
typedef struct
{
    DeviceCommand command;

    /**
     * @brief argument count
     */
    int argc;

    /**
     * @brief argument content
     */
    char args[MAX_ARG_NUM - 1][INPUT_LEN];
} DeviceCommandInfo;

/**
 * @brief this is used to select what commands are available
 * 
 */
typedef enum
{
    MODE_LOGIN,
    MODE_STANDARD,
    MODE_CHAT
} CommandMode;

/**
 * @brief requires an input from stdin
 *
 * @param mode use it to select the mode you want, if a command is not accepted in the selected mode an error will be displayed
 * and the return will be COMMAND_ERROR
 * @return struct containing command and args, command is COMMAND_ERROR if an error occured during parsing
 */
DeviceCommandInfo CommandParserGetCommand(CommandMode mode);