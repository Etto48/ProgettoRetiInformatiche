#pragma once
#include <stdio.h>

#ifdef DEBUG
/**
 * @brief print a line if in debug mode
 *
 * @param str tag to print
 */
void DebugTag(const char *str);
#else
#define DebugTag(str)
#endif