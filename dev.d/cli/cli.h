#pragma once
#include <stdint.h>
#include <string.h>
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

/**
 * @brief run every signup command operation
 * 
 * @param dci command args
 */
void CLISignup(DeviceCommandInfo dci);

/**
 * @brief run every login command operation
 * 
 * @param dci command args
 */
void CLILogin(DeviceCommandInfo dci);

/**
 * @brief run every hanging command operation
 * 
 * @param dci command args
 */
void CLIHanging(DeviceCommandInfo dci);

/**
 * @brief run every show command operation
 * 
 * @param dci command args
 */
void CLIShow(DeviceCommandInfo dci);

/**
 * @brief run every chat command operation
 * 
 * @param dci command args
 */
void CLIChat(DeviceCommandInfo dci);

/**
 * @brief run every logout command operation
 * 
 * @param dci command args
 */
void CLILogout(DeviceCommandInfo dci);

/**
 * @brief run every quit command operation (chat mode)
 * 
 * @param dci command args
 */
void CLIChatQuit(DeviceCommandInfo dci);

/**
 * @brief run every users command operation (chat mode)
 * 
 * @param dci command args
 */
void CLIChatUsers(DeviceCommandInfo dci);

/**
 * @brief run every add command operation (chat mode)
 * 
 * @param dci command args
 */
void CLIChatAdd(DeviceCommandInfo dci);

/**
 * @brief run every file command operation (chat mode)
 * 
 * @param dci command args
 */
void CLIChatFile(DeviceCommandInfo dci);