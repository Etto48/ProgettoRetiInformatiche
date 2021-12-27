#include "message_relay.h"

RelayMessage* RelayHangingList = NULL;

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

void RelayLoad(const char* filename)
{

}

void RelaySave(const char* filename)
{
    
}