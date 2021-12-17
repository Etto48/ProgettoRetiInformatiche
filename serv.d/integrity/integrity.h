#pragma once
#include <stdlib.h>
#include "../index/index.h"

/**
 * @brief load necessary data from file, you must call this at the beginning of main
 * 
 */
void Startup();

/**
 * @brief save necessary data to file and exit, do not exit in other way
 * 
 */
void SaveAndExit();