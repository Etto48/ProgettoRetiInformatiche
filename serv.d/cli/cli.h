#pragma once
#include "../command_parser/command_parser.h"
#include "../../global.d/debug/debug.h"
#include "../integrity/integrity.h"

/**
 * @brief get a line from stdin and execute the command, this function is called inside the network main loop when stdin contains input
 *
 */
void CLIHandleInput();

/**
 * @brief used when the command "list" is executed
 *
 */
void CLIPrintConnectedUsers();