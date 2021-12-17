#include "debug.h"

#ifdef DEBUG
void DebugTag(const char* str)
{
    printf("DBG: %s\n",str);
}
#endif