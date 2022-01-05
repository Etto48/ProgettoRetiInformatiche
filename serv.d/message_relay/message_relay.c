#include "message_relay.h"

RelayMessage* RelayHangingList = NULL;
RelaySyncreadNotice* RelaySyncreadList = NULL;

void RelayHangingAdd(UserName src, UserName dst, time_t timestamp, RelayMessageType type, char *filename, size_t data_size, uint8_t *data)
{
    RelayMessage *new_message = (RelayMessage *)malloc(sizeof(RelayMessage));
    memcpy(new_message->src.str,src.str,USERNAME_MAX_LENGTH+1);
    memcpy(new_message->dst.str,dst.str,USERNAME_MAX_LENGTH+1);
    new_message->timestamp = timestamp;
    new_message->type = type;
    new_message->data_size = data_size;
    switch (type)
    {
    case RELAY_MESSAGE_TEXT:
        new_message->filename = NULL;
        new_message->data = (uint8_t*)malloc(strlen((char*)data)+1);
        strcpy((char*)new_message->data,(char*)data);
        break;
    case RELAY_MESSAGE_FILE:
        new_message->filename = (char *)malloc(strlen(filename)+1);
        strcpy(new_message->filename,filename);
        new_message->data = (uint8_t *)malloc(data_size);
        memcpy(new_message->data,data,data_size);
        break;
    }
    new_message->next = RelayHangingList;
    RelayHangingList = new_message;
}

RelayMessage *RelayHangingFindFirst(RelayMessage* message_list, UserName *src, UserName *dst)
{
    for(RelayMessage* i = message_list; i; i=i->next)
    {
        if ((!src || strcmp(i->src.str, src->str) == 0) && (!dst || strcmp(i->dst.str, dst->str) == 0))
            return i;
    }
    return NULL;
}

size_t RelayHangingCount(UserName *src, UserName *dst)
{
    size_t count = 0;
    for(RelayMessage* i = RelayHangingList; i; i=i->next)
    {
        if ((!src || strcmp(i->src.str, src->str) == 0) && (!dst || strcmp(i->dst.str, dst->str) == 0))
            count ++;
    }
    return count;
}

RelayMessage* RelayHangingPopFirst(UserName *src, UserName *dst)
{
    RelayMessage* last = NULL;
    RelayMessage* i = NULL;
    for(i = RelayHangingList; i; i=i->next)
    {
        if ((!src || strcmp(i->src.str, src->str) == 0) && (!dst || strcmp(i->dst.str, dst->str) == 0))
            break;
        last = i;
    }
    if (i)
    {
        if (last == NULL) // head
        {
            RelayHangingList = i->next;
        }
        else
        {
            last->next = i->next;
        }
        return i;
    }
    else return NULL;
}

void RelayHangingDestroyMessage(RelayMessage* msg)
{
    if(msg)
    {
        free(msg->data);
        if(msg->filename)
            free(msg->filename);
        free(msg);
    }
}

bool RelayLoad(const char* filename)
{
    // TODO: fill me
    if(RelayHangingList)
        return false;
    int fd = open(filename,O_RDWR|O_CREAT,S_IRUSR|S_IWUSR);
    if (fd < 0)
    {
        dbgerror("Error opening relay file");
        return false;
    }
    while(true)
    {
        UserName src;
        UserName dst;
        uint64_t timestamp;
        char type;
        uint32_t data_len = 0;
        char* filename = NULL;
        uint8_t* data = NULL;
        if(read(fd,src.str,USERNAME_MAX_LENGTH)==0)
            break;
        src.str[USERNAME_MAX_LENGTH] = '\0';
        read(fd,dst.str,USERNAME_MAX_LENGTH);
        dst.str[USERNAME_MAX_LENGTH] = '\0';
        read(fd,&timestamp,sizeof(uint64_t));
        timestamp = ntohq(timestamp);
        read(fd,&type,sizeof(char));
        switch (type)
        {
        case 'F':
            {
                uint32_t filename_len;
                read(fd,&filename_len,sizeof(uint32_t));
                filename_len = ntohl(filename_len);
                filename = (char*)malloc(filename_len+1);
                read(fd,filename,filename_len);
                filename[filename_len] = '\0';
                read(fd,&data_len,sizeof(uint32_t));
                data_len = ntohl(data_len);
                data = (uint8_t*)malloc(data_len);
                read(fd,data,data_len);
            }
            break;
        case 'T':
            {
                read(fd,&data_len,sizeof(uint32_t));
                data_len = ntohl(data_len);
                data = (uint8_t*)malloc(data_len+1);
                read(fd,data,data_len);
                data[data_len]='\0';
            }
            break;
        }
        RelayHangingAdd(src,dst,timestamp,
            type=='F'?RELAY_MESSAGE_FILE:RELAY_MESSAGE_TEXT,
            filename,
            data_len,
            data);
        if(filename)
            free(filename);
        free(data);
    }
    close(fd);
    return true;
}

bool RelaySave(const char* filename)
{
    // TODO: fill me
    int fd = open(filename,O_WRONLY|O_CREAT,S_IRUSR|S_IWUSR);
    if (fd < 0)
    {
        dbgerror("Error opening relay file");
        return false;
    }
    for(RelayMessage* i = RelayHangingList; i; i=i->next)
    {
        write(fd,i->src.str,USERNAME_MAX_LENGTH);
        write(fd,i->dst.str,USERNAME_MAX_LENGTH);
        uint64_t timestamp = htonq(i->timestamp);
        write(fd,&timestamp,sizeof(uint64_t));
        switch (i->type)
        {
        case RELAY_MESSAGE_FILE:
            {
                write(fd,"F",sizeof(char));
                uint32_t filename_len = htonl(strlen(i->filename));
                write(fd,&filename_len,sizeof(uint32_t));
                write(fd,i->filename,strlen(i->filename));
                uint32_t file_len = htonl(i->data_size);
                write(fd,&file_len,sizeof(uint32_t));
                write(fd,i->data,i->data_size);
            }
            break;
        
        case RELAY_MESSAGE_TEXT:
            {
                write(fd,"T",sizeof(char));
                uint32_t text_len = htonl(strlen((char*)i->data));
                write(fd,&text_len,sizeof(uint32_t));
                write(fd,i->data,strlen((char*)i->data));
            }
            break;
        }        
    }
    close(fd);
    return true;
}

void RelaySyncreadEdit(UserName src, UserName dst, time_t timestamp)
{
    //TODO: fill me
}

RelaySyncreadNotice* RelaySyncreadFind(UserName* src, UserName* dst)
{
    for(RelaySyncreadNotice* i = RelaySyncreadList;i;i=i->next)
    {
        if((!src||strncmp(src->str,i->src.str,USERNAME_MAX_LENGTH)==0) && (!dst || strncmp(dst->str,i->dst.str,USERNAME_MAX_LENGTH)==0))
            return i;
    }
    return NULL;
}

void RelaySyncreadDelete(UserName* src, UserName* dst)
{
    //TODO: fill me
}

bool RelaySyncreadLoad(const char* filename)
{
    //TODO: fill me
    return true;
}

bool RelaySyncreadSave(const char* filename)
{
    //TODO: fill me
    return true;
}