#include "tools.h"

size_t ToolsBasename(const char* path)
{
    size_t ret = 0;
    for(size_t i = 0; path[i] != '\0'; i++)
    {
        if(path[i]=='/')
            ret = i;
    }
    return ret;
}

uint64_t htonq(uint64_t quad)
{
    unsigned char out[8] = {quad>>56,quad>>48,quad>>40,quad>>32,quad>>24,quad>>16,quad>>8,quad};
    return *(uint64_t*)out;
}

uint64_t ntohq(uint64_t quad)
{
    unsigned char out[8] = {quad,quad>>8,quad>>16,quad>>24,quad>>32,quad>>40,quad>>48,quad>>56};
    return *(uint64_t*)out;
}