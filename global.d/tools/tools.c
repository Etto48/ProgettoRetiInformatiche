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
    //unsigned char out[8] = {quad,quad>>8,quad>>16,quad>>24,quad>>32,quad>>40,quad>>48,quad>>56};
    //return *(uint64_t*)out;
    return htonq(quad);
}

UserName CreateUserName(const char* username)
{
    UserName ret;
    memset(ret.str,0,USERNAME_MAX_LENGTH+1);
    strncpy(ret.str,username,USERNAME_MAX_LENGTH);
    return ret;
}

/* we hash the password for a little bit of security, please run this program on a secure network and use a strong password,
 * the hash can be replayed, the service is not really secure without SSL
 */
Password CreatePassword(const char* password)
{
    Password ret;
    memset(ret.str,0,PASSWORD_MAX_LENGTH+1);
    calc_sha_256(ret.str,password,strlen(password));
    return ret;
}