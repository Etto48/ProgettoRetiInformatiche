#include "message_relay.h"

RelayMessage *RelayHangingList = NULL;
RelaySyncreadNotice *RelaySyncreadList = NULL;

void RelayHangingAdd(UserName src, UserName dst, time_t timestamp, RelayMessageType type, char *filename, size_t data_size, uint8_t *data)
{
    RelayMessage *new_message = (RelayMessage *)malloc(sizeof(RelayMessage));
    memcpy(new_message->src.str, src.str, USERNAME_MAX_LENGTH + 1);
    memcpy(new_message->dst.str, dst.str, USERNAME_MAX_LENGTH + 1);
    new_message->timestamp = timestamp;
    new_message->type = type;
    new_message->data_size = data_size;
    switch (type)
    {
    case RELAY_MESSAGE_TEXT:
        new_message->filename = NULL;
        new_message->data = (uint8_t *)malloc(strlen((char *)data) + 1);
        strcpy((char *)new_message->data, (char *)data);
        break;
    case RELAY_MESSAGE_FILE:
        new_message->filename = (char *)malloc(strlen(filename) + 1);
        strcpy(new_message->filename, filename);
        new_message->data = (uint8_t *)malloc(data_size);
        memcpy(new_message->data, data, data_size);
        break;
    }
    // we must add in order (by timestamp)
    RelayMessage *last = NULL;
    for (RelayMessage *i = RelayHangingList; i; i = i->next)
    {
        if (i->timestamp >= new_message->timestamp)
            break;
        last = i;
    }
    if (!last)
    { // head
        new_message->next = RelayHangingList;
        RelayHangingList = new_message;
    }
    else
    { // middle
        new_message->next = last->next;
        last->next = new_message;
    }
}

RelayMessage *RelayHangingFindFirst(RelayMessage *message_list, UserName *src, UserName *dst)
{
    for (RelayMessage *i = message_list; i; i = i->next)
    {
        if ((!src || strncmp(i->src.str, src->str, USERNAME_MAX_LENGTH) == 0) && (!dst || strncmp(i->dst.str, dst->str, USERNAME_MAX_LENGTH) == 0))
            return i;
    }
    return NULL;
}

size_t RelayHangingCount(UserName *src, UserName *dst)
{
    size_t count = 0;
    for (RelayMessage *i = RelayHangingList; i; i = i->next)
    {
        if ((!src || strncmp(i->src.str, src->str, USERNAME_MAX_LENGTH) == 0) && (!dst || strncmp(i->dst.str, dst->str, USERNAME_MAX_LENGTH) == 0))
            count++;
    }
    return count;
}

RelayMessage *RelayHangingPopFirst(UserName *src, UserName *dst)
{
    RelayMessage *last = NULL;
    RelayMessage *i = NULL;
    for (i = RelayHangingList; i; i = i->next)
    {
        if ((!src || strncmp(i->src.str, src->str, USERNAME_MAX_LENGTH) == 0) && (!dst || strncmp(i->dst.str, dst->str, USERNAME_MAX_LENGTH) == 0))
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
    else
        return NULL;
}

void RelayHangingDestroyMessage(RelayMessage *msg)
{
    if (msg)
    {
        free(msg->data);
        if (msg->filename)
            free(msg->filename);
        free(msg);
    }
}

bool RelayLoad(const char *filename)
{
    if (RelayHangingList)
        return false;
    int fd = open(filename, O_RDWR | O_CREAT, S_IRUSR | S_IWUSR);
    if (fd < 0)
    {
        dbgerror("Error opening relay file");
        return false;
    }
    while (true)
    {
        UserName src;
        UserName dst;
        uint64_t timestamp;
        char type;
        uint32_t data_len = 0;
        char *filename = NULL;
        uint8_t *data = NULL;
        if (read(fd, src.str, USERNAME_MAX_LENGTH) <= 0)
            break;
        src.str[USERNAME_MAX_LENGTH] = '\0';
        if (read(fd, dst.str, USERNAME_MAX_LENGTH) < 0)
            break;
        dst.str[USERNAME_MAX_LENGTH] = '\0';
        if (read(fd, &timestamp, sizeof(uint64_t)) < 0)
            break;
        timestamp = ntohq(timestamp);
        if (read(fd, &type, sizeof(char)) < 0)
            break;
        switch (type)
        {
        case 'F':
        {
            uint32_t filename_len;
            if (read(fd, &filename_len, sizeof(uint32_t)) < 0)
                goto endloop;
            filename_len = ntohl(filename_len);
            filename = (char *)malloc(filename_len + 1);
            if (read(fd, filename, filename_len) < 0)
                goto endloop;
            filename[filename_len] = '\0';
            if (read(fd, &data_len, sizeof(uint32_t)) < 0)
                goto endloop;
            data_len = ntohl(data_len);
            data = (uint8_t *)malloc(data_len);
            if (read(fd, data, data_len) < 0)
                goto endloop;
        }
        break;
        case 'T':
        {
            if (read(fd, &data_len, sizeof(uint32_t)) < 0)
                goto endloop;
            data_len = ntohl(data_len);
            data = (uint8_t *)malloc(data_len + 1);
            if (read(fd, data, data_len) < 0)
                goto endloop;
            data[data_len] = '\0';
        }
        break;
        }
        RelayHangingAdd(src, dst, timestamp,
                        type == 'F' ? RELAY_MESSAGE_FILE : RELAY_MESSAGE_TEXT,
                        filename,
                        data_len,
                        data);
        if (filename)
            free(filename);
        free(data);
    }
endloop:

    close(fd);
    return true;
}

bool RelaySave(const char *filename)
{
    int fd = open(filename, O_WRONLY | O_CREAT | O_TRUNC, S_IRUSR | S_IWUSR);
    if (fd < 0)
    {
        dbgerror("Error opening relay file");
        return false;
    }
    for (RelayMessage *i = RelayHangingList; i; i = i->next)
    {
        if (write(fd, i->src.str, USERNAME_MAX_LENGTH) < 0)
            break;
        if (write(fd, i->dst.str, USERNAME_MAX_LENGTH) < 0)
            break;
        uint64_t timestamp = htonq(i->timestamp);
        if (write(fd, &timestamp, sizeof(uint64_t)) < 0)
            break;
        switch (i->type)
        {
        case RELAY_MESSAGE_FILE:
        {
            if (write(fd, "F", sizeof(char)) < 0)
                goto endloop;
            uint32_t filename_len = htonl(strlen(i->filename));
            if (write(fd, &filename_len, sizeof(uint32_t)) < 0)
                goto endloop;
            if (write(fd, i->filename, strlen(i->filename)) < 0)
                goto endloop;
            uint32_t file_len = htonl(i->data_size);
            if (write(fd, &file_len, sizeof(uint32_t)) < 0)
                goto endloop;
            if (write(fd, i->data, i->data_size) < 0)
                goto endloop;
        }
        break;

        case RELAY_MESSAGE_TEXT:
        {
            if (write(fd, "T", sizeof(char)) < 0)
                goto endloop;
            uint32_t text_len = htonl(strlen((char *)i->data));
            if (write(fd, &text_len, sizeof(uint32_t)) < 0)
                goto endloop;
            if (write(fd, i->data, strlen((char *)i->data)) < 0)
                goto endloop;
        }
        break;
        }
    }
endloop:

    close(fd);
    return true;
}

void RelayFree()
{
    RelayMessage *next = NULL;
    for (RelayMessage *i = RelayHangingList; i; i = next)
    {
        next = i->next;
        RelayHangingDestroyMessage(i);
    }
    RelayHangingList = NULL;
}

void RelaySyncreadAdd(UserName src, UserName dst, time_t timestamp)
{
    RelaySyncreadNotice *new_notice = (RelaySyncreadNotice *)malloc(sizeof(RelaySyncreadNotice));
    new_notice->src = src;
    new_notice->dst = dst;
    new_notice->timestamp = timestamp;
    new_notice->next = RelaySyncreadList;
    RelaySyncreadList = new_notice;
}

void RelaySyncreadEdit(UserName src, UserName dst, time_t timestamp)
{
    // first we check if the user is online
    int check_connected = NetworkFindConnection(src);
    bool sent = false;
    if (check_connected >= 0)
    { // the user is online, we must send to him the message directly
        sent = NetworkSendMessageSyncread(check_connected, dst, timestamp);
    }

    if (!sent)
    { // the user is offline we store the message for the future, if we find an existing syncread message we must edit that
        RelaySyncreadNotice *target = RelaySyncreadFind(&src, &dst);
        if (target)
        { // existing entry found, update it
            target->timestamp = timestamp;
        }
        else
        { // nothing found, add an entry
            RelaySyncreadAdd(src, dst, timestamp);
        }
    }
}

RelaySyncreadNotice *RelaySyncreadFind(UserName *src, UserName *dst)
{
    for (RelaySyncreadNotice *i = RelaySyncreadList; i; i = i->next)
    {
        if ((!src || strncmp(src->str, i->src.str, USERNAME_MAX_LENGTH) == 0) && (!dst || strncmp(dst->str, i->dst.str, USERNAME_MAX_LENGTH) == 0))
            return i;
    }
    return NULL;
}

void RelaySyncreadDelete(UserName *src, UserName *dst)
{
    RelaySyncreadNotice *last = NULL;
    RelaySyncreadNotice *i = NULL;
    for (i = RelaySyncreadList; i; i = i->next)
    {
        if ((!src || strncmp(src->str, i->src.str, USERNAME_MAX_LENGTH) == 0) && (!dst || strncmp(dst->str, i->dst.str, USERNAME_MAX_LENGTH) == 0))
            break;
        last = i;
    }
    if (i)
    { // we need to delete something
        if (!last)
        { // delete the head
            RelaySyncreadList = i->next;
        }
        else
        { // delete the middle
            last->next = i->next;
        }
        free(i);
    }
}

bool RelaySyncreadLoad(const char *filename)
{
    if (RelaySyncreadList)
        return false;
    int fd = open(filename, O_RDWR | O_CREAT, S_IRUSR | S_IWUSR);
    if (fd < 0)
    {
        dbgerror("Error writing syncread file");
        return false;
    }
    while (true)
    {
        UserName src;
        UserName dst;
        uint64_t timestamp;
        if (read(fd, src.str, USERNAME_MAX_LENGTH) <= 0)
            break;
        src.str[USERNAME_MAX_LENGTH] = '\0';
        if (read(fd, dst.str, USERNAME_MAX_LENGTH) < 0)
            break;
        dst.str[USERNAME_MAX_LENGTH] = '\0';
        if (read(fd, &timestamp, sizeof(uint64_t)) < 0)
            break;
        timestamp = ntohq(timestamp);

        RelaySyncreadAdd(src, dst, timestamp);
    }
    close(fd);
    return true;
}

bool RelaySyncreadSave(const char *filename)
{
    int fd = open(filename, O_WRONLY | O_CREAT | O_TRUNC, S_IRUSR | S_IWUSR);
    if (fd < 0)
    {
        dbgerror("Error writing syncread file");
        return false;
    }
    for (RelaySyncreadNotice *i = RelaySyncreadList; i; i = i->next)
    {
        uint64_t timestamp = htonq(i->timestamp);
        if (write(fd, i->src.str, USERNAME_MAX_LENGTH) < 0)
            break;
        if (write(fd, i->dst.str, USERNAME_MAX_LENGTH) < 0)
            break;
        if (write(fd, &timestamp, sizeof(uint64_t)) < 0)
            break;
    }
    close(fd);
    return true;
}

void RelaySyncreadFree()
{
    RelaySyncreadNotice *next = NULL;
    for (RelaySyncreadNotice *i = RelaySyncreadList; i; i = next)
    {
        next = i->next;
        free(i);
    }
    RelaySyncreadList = NULL;
}