#pragma once
#include <stdio.h>
#include <string.h>
#include "../../global.d/globalDefine.h"
#include "../../global.d/command_input/command_input.h"

#define MAX_ARG_NUM 1

/*****************************
 * Available Server Commands *  
 *****************************
 * 
 * - help
 *   show a command list + instructions
 * 
 * - list
 *   show a list of connected users
 * 
 * - esc|exit
 *   close the server
 * 
 ****************************/

typedef enum
{
    COMMAND_HELP,
    COMMAND_LIST,
    COMMAND_ESC,
    COMMAND_ERROR
} ServerCommand;

/**
 * @brief requires an input from stdin
 * 
 * @return enum corresponding to the command, COMMAND_ERROR if not a valid command
 */
ServerCommand CommandParserGetCommand();