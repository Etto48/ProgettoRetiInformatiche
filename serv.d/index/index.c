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
    AuthList = NULL;
}

bool AuthLoad(const char* filename)
{
    AuthDestroy();
    int fd = open(filename,O_RDONLY|O_CREAT,S_IWUSR|S_IRUSR);
    if(fd<0)
    {
        dbgerror("Error opening authentication file");
        return false;
    }
    ssize_t reads = 0;
    bool error = false;
    while(true)
    {
        AuthEntry new_entry;
        memset(new_entry.username.str,0,USERNAME_MAX_LENGTH+1);
        memset(new_entry.password.str,0,PASSWORD_MAX_LENGTH+1);
        reads = read(fd,new_entry.username.str, USERNAME_MAX_LENGTH);
        if(reads==0)//EOF
            break;
        else if (reads < USERNAME_MAX_LENGTH)//Corrupted file
        {
            error = true;
            break;
        }
        reads = read(fd,new_entry.password.str, PASSWORD_MAX_LENGTH);
        if (reads < PASSWORD_MAX_LENGTH)//Corrupted file
        {
            error = true;
            break;
        }
        new_entry.next=AuthList;

        AuthEntry* new_copy_entry = (AuthEntry*)malloc(sizeof(AuthEntry));
        *new_copy_entry = new_entry;
        AuthList = new_copy_entry;
    }
    if(error)
    {
        AuthDestroy();
        return false;
    }
    
    close(fd);
    return true;
}

bool AuthSave(const char* filename)
{
    int fd = open(filename,O_WRONLY|O_CREAT,S_IWUSR|S_IRUSR);
    if(fd<0)
    {
        dbgerror("Error opening authentication file");
        return false;
    }
    for(AuthEntry* i = AuthList; i; i=i->next)
    {
        if(write(fd,i->username.str,USERNAME_MAX_LENGTH)<USERNAME_MAX_LENGTH)
            return false;   
        if(write(fd,i->password.str,PASSWORD_MAX_LENGTH)<PASSWORD_MAX_LENGTH)   
            return false;
    }
    close(fd);
    return true;
}

bool IndexLogin(UserName username, Password password, uint32_t ip, uint16_t port)
{
    if(!AuthCheck(username,password))
        return false;
    IndexEntry* target = IndexFind(username);
    if(IndexIsOnline(target))
        return false;
    if(!target)
    {
        target = (IndexEntry*)malloc(sizeof(IndexEntry));
        target->next = IndexList;
        IndexList = target;
    }
    target->user_dest = username;
    target->timestamp_login = time(NULL);
    target->timestamp_logout = 0;
    target->ip = ip;
    target->port = port;
    return true;
}

bool IndexLogout(UserName username)
{
    IndexEntry* target = IndexFind(username);
    if(!IndexIsOnline(target))
        return false;
    target->timestamp_logout=time(NULL);
    return true;
}

IndexEntry* IndexFind(UserName username)
{
    for(IndexEntry* i = IndexList; i; i=i->next)
    {
        if(strcmp(username.str,i->user_dest.str)==0)
            return i;
    }
    return NULL;
}

bool IndexIsOnline(IndexEntry* user)
{
    return user && user->timestamp_logout==0;
}