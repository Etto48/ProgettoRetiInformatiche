#pragma once

#include <stdio.h>
#include <string.h>
#include "../globalDefine.h"


#define INPUT_LEN 1024

/**
 * @brief requires an input from stdin and splits it in a command and args
 * 
 * @param plain_text string where the entire line will be put
 * @param command string where the command will be put (length should be 1024)
 * @param args array of strings where arguments will be put (length should be 1024 for each one)
 * @param max_arg_num maximum number of arguments (must be 1+number of arguments required from your syntax)
 * @return number of args read or -1 if encountered an error
 */
int CommandInputSplitCommand(char *plain_text,char *command, char args[][INPUT_LEN], unsigned int max_arg_num);

