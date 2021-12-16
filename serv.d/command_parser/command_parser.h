#pragma once

/*****************************
 * Available Server Commands *  
 *****************************
 * 
 * - help
 *   shows a command list + instructions
 * 
 * - list
 *   show a list of connected users
 * 
 * - esc
 *   close the server
 * 
 ****************************/

enum ServerCommand
{
    COMMAND_HELP,
    COMMAND_LIST,
    COMMAND_ESC
};