#pragma once
#include <stdio.h>

#ifdef DEBUG

#define dbgerror(str) perror(str)
#define DebugLog(args)        \
    {                         \
        printf("Log: " args); \
        printf("\n");         \
    }
#else
#define DebugTag(str)
#define dbgerror(str)
#define DebugLog(args)
#endif
