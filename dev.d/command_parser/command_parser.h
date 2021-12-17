#pragma once

#include "../../global.d/command_input/command_input.h"

#define MAX_ARG_NUM 4
/*****************************
 * Available Server Commands *  
 *****************************
 * 
 * - help
 *   shows a command list + instructions
 * 
 * - signup <username> <password>
 * 
 * - in <server port> <username> <password>
 *   
 * - hanging
 * 
 * - show <username>
 * 
 * - chat <username>
 *   you can start a chat with <username> with this command
 *   once in a chat you can use the following commands
 *   prefixing them with a "\"
 *   > q
 *     close the chat
 *   > u 
 *     list online users available for chat
 *   > a <username>
 *     add a user to the chat
 *   > f <filename>
 *     share a file with the chat
 * 
 * - out
 * 
 ****************************/

typedef enum 
{
    COMMAND_HELP,
    COMMAND_SIGNUP,
    COMMAND_IN,
    COMMAND_HANGING,
    COMMAND_SHOW,
    COMMAND_CHAT,
    COMMAND_CHAT_QUIT,
    COMMAND_CHAT_USERS,
    COMMAND_CHAT_ADD,
    COMMAND_CHAT_FILE,
    COMMAND_OUT,
    COMMAND_ERROR
} DeviceCommand;

typedef struct
{
    DeviceCommand command;
    int argc;
    char args[MAX_ARG_NUM-1][INPUT_LEN];
} DeviceCommandInfo;

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