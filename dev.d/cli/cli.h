#pragma once
#include "../command_parser/command_parser.h"
#include "../../global.d/debug/debug.h"

/**
 * @brief must be changed if device state changes (between login/general/chat)
 * 
 */
extern CommandMode CLIMode;

/**
 * @brief get a line from stdin and execute the command, this function is called inside the network main loop when stdin contains input
 * 
 */
void CLIHandleInput();