#include "tools.h"

size_t ToolsBasename(const char *path)
{
    size_t ret = 0;
    for (size_t i = 0; path[i] != '\0'; i++)
    {
        if (path[i] == '/')
            ret = i + 1;
    }
    return ret;
}

uint64_t htonq(uint64_t quad)
{
    uint8_t out[8] = {quad >> 56, quad >> 48, quad >> 40, quad >> 32, quad >> 24, quad >> 16, quad >> 8, quad};
    uint64_t ret;
    memcpy(&ret,out,sizeof(uint64_t));
    return ret;
}

uint64_t ntohq(uint64_t quad)
{
    // unsigned char out[8] = {quad,quad>>8,quad>>16,quad>>24,quad>>32,quad>>40,quad>>48,quad>>56};
    // return *(uint64_t*)out;
    return htonq(quad);
}

UserName CreateUserName(const char *username)
{
    UserName ret;
    memset(ret.str, 0, USERNAME_MAX_LENGTH + 1);
    strncpy(ret.str, username, USERNAME_MAX_LENGTH);
    return ret;
}

/* we hash the password for a little bit of security, please run this program on a secure network and use a strong password,
 * the hash can be replayed, the service is not really secure without SSL
 */
Password CreatePassword(const char *password)
{
    Password ret;
    memset(ret.data, 0, PASSWORD_SIZE);
    calc_sha_256(ret.data, password, strlen(password));
    return ret;
}