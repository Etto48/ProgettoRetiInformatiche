#pragma once
#include <stdio.h>

#ifdef DEBUG
/**
 * @brief print a line if in debug mode
 *
 * @param str tag to print
 */
void DebugTag(const char *str);

#define dbgerror(str) perror(str)
#define DebugLog(args) {printf("Log: " args); printf("\n");}
#else
#define DebugTag(str)
#define dbgerror(str)
#define DebugLog(args)
#endif
