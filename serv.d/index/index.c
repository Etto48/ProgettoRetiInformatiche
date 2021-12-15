#include "index.h"

AuthEntry* AuthList = NULL;
IndexEntry* IndexList = NULL;

bool AuthRegister(UserName username, Password password)
{
    for(AuthEntry* i = AuthList; i; i=i->next)
    {
        if(strcmp(username.str,i->username.str)==0)
            return false;
    }
    AuthEntry* new_entry = (AuthEntry*)malloc(sizeof(AuthEntry));
    new_entry->next=AuthList;
    strcpy(new_entry->username.str,username.str);
    strcpy(new_entry->password.str,password.str);
    AuthList = new_entry;
    return true;
}

bool AuthCheck(UserName username, Password password)
{
    for(AuthEntry* i = AuthList; i; i=i->next)
    {
        if(strcmp(username.str,i->username.str)==0 && strcmp(password.str,i->password.str)==0)
            return true;
    }
    return false;
}

void AuthDestroy()
{
    AuthEntry* next = NULL;
    for(AuthEntry* i = AuthList; i; i=next)
    {
        next = i->next;
        free(i);
    }
}

bool AuthLoad(const char* filename)
{
    AuthDestroy();
    int fd = open(filename,O_RDONLY);
    if(fd<0)
    {
        #ifdef DEBUG
            perror("Error opening authentication file");
        #endif
        return false;
    }
    ssize_t reads = 0;
    while(true)
    {
        AuthEntry new_entry;
        bzero(new_entry.username.str,USERNAME_MAX_LENGTH+1);
        bzero(new_entry.password.str,PASSWORD_MAX_LENGTH+1);
        reads = read(fd,new_entry.username.str, USERNAME_MAX_LENGTH);
        if(reads==0)
            break;
        else if (reads < USERNAME_MAX_LENGTH)
            return false;
        reads = read(fd,new_entry.password.str, PASSWORD_MAX_LENGTH);
        if (reads < PASSWORD_MAX_LENGTH)
            return false;
        
    }

    close(fd);
    return true;
}