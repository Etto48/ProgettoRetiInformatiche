#pragma once
#include <stdlib.h>
#include <signal.h>
#include "../index/index.h"

/**
 * @brief load necessary data from file, you must call this at the beginning of main
 * 
 */
void Startup();

/**
 * @brief save necessary data to file and exit, do not exit in other way
 * 
 * @param status the status to pass to the exit() call
 */
void SaveAndExit(int status);