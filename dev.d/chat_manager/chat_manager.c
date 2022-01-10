#include "chat_manager.h"

#define MK_CHAT_DIR                                                                                      \
    if (mkdir(CHAT_DIR, S_IRWXU | S_IRGRP | S_IXGRP | S_IROTH | S_IXOTH) < 0 && errno != EEXIST)         \
    {                                                                                                    \
        dbgerror("Error creating chat directory");                                                       \
        return false;                                                                                    \
    }                                                                                                    \
    if (mkdir(ChatGetDirname(), S_IRWXU | S_IRGRP | S_IXGRP | S_IROTH | S_IXOTH) < 0 && errno != EEXIST) \
    {                                                                                                    \
        dbgerror("Error creating chat user directory");                                                  \
        return false;                                                                                    \
    }

/**
 * @brief used as interal buffer for ChatGetFilename and ChatGetDirname
 *
 */
char filename_tmp_buffer[FILENAME_MAX] = "";

Chat *ChatList = NULL;
ChatTarget *ChatTargetList = NULL;

void ChatHandleSyncread(UserName dst, time_t timestamp)
{
    // we try to load the chat first, to prevent inconsistencies
    ChatLoad(dst);
    // ChatTarget* target = ChatTargetFind(dst); //NOTE: if target!=NULL the messages you are seeing may have * instead of **
    Chat *chat = ChatFind(dst);
    if (chat)
    {
        // update every entry before the timestamp
        for (ChatMessage *i = chat->head; i; i = i->next)
        {
            if (i->timestamp <= timestamp)
                i->read = true;
        }
    }
}

int ChatConnectTo(UserName username, uint32_t ip, uint16_t port)
{
    // search in the chat
    int check_connected = NetworkFindConnection(username);
    if (check_connected >= 0)
        return check_connected;
    // if we know that the client is offline
    if (port == 0)
        return -1;
    // failed to find it, now we must try to connect
    int sockfd = socket(AF_INET, SOCK_STREAM, 0);
    if (sockfd < 0)
    {
        dbgerror("Error creating a socket");
        SaveAndExit(-1);
    }
    struct sockaddr_in addr = {
        .sin_family = AF_INET,
        .sin_addr = {.s_addr = htonl(ip)},
        .sin_port = htons(port),
        .sin_zero = {0}};
    // fd -1 will tell the caller the server must be used as relay
    if (connect(sockfd, (struct sockaddr *)&addr, sizeof(addr)) < 0)
        return -1;
    // add the connection to NetworkConnectedDevices
    NetworkNewConnection(sockfd, addr);
    NetworkConnectedDevices[sockfd].username = username;
    NetworkSendMessageLogin(sockfd, 0, CLIActiveUsername, CreatePassword("")); // send your username
    return sockfd;
}

bool ChatAddTarget(UserName username)
{
    // first we check if the user is already present in the target list
    if (ChatTargetFind(username))
        return false;
    // we must request info to the server about the user we want to add
    if (NetworkServerInfo.connected)
    {
        ChatTarget *new_target = (ChatTarget *)malloc(sizeof(ChatTarget));
        new_target->dst = username;
        // if we find the user online we connect to it, if it is offline we use the server as relay
        if (NetworkSendMessageUserinfoReq(NetworkServerInfo.sockfd, username) && NetworkReceiveResponseFromServer(MESSAGE_USERINFO_RES))
        { // user online
            uint32_t ip;
            uint16_t port;
            NetworkDeserializeMessageUserinfoRes(NetworkServerInfo.message_list_head->header.payload_size, NetworkServerInfo.message_list_head->payload, &ip, &port);
            NetworkDeleteOneFromServer();
            // we try to connect to the requested user
            new_target->sockfd = ChatConnectTo(username, ip, port);
            if (new_target->sockfd < 0)
                printf("%s is offline, connecting in relay mode\n", username.str);
            else
                printf("%s is online, connecting in p2p mode\n", username.str);
            new_target->next = ChatTargetList;
            ChatTargetList = new_target;
            return true;
        }
        else
        { // user does not exists
            free(new_target);
            return false;
        }
    }
    else // we are offline
        return false;
}

bool ChatLoad(UserName user)
{
    // we can't load a chat if we aren't logged in because the file path depends on our username
    if (CLIMode != MODE_LOGIN)
    {
        // check if the chat is already loaded
        if (ChatFind(user))
            return true;

        MK_CHAT_DIR

        int chat_fd = open(ChatGetFilename(user), O_RDWR | O_CREAT, S_IRUSR | S_IWUSR);
        if (chat_fd < 0)
        {
            dbgerror("Error reading chat file");
            return false;
        }
        // now we deserialize the chat reading from the appropriate file
        while (true)
        {
            uint64_t timestamp;
            uint8_t type;
            uint8_t direction;
            uint8_t read_b;
            uint32_t content_len;
            if (read(chat_fd, &timestamp, sizeof(uint64_t)) == 0)
                break; // EOF
            timestamp = ntohq(timestamp);
            if(read(chat_fd, &type, sizeof(uint8_t))<0) break;
            if(read(chat_fd, &direction, sizeof(uint8_t))<0) break;
            if(read(chat_fd, &read_b, sizeof(uint8_t))<0) break;
            if(read(chat_fd, &content_len, sizeof(uint32_t))<0) break;
            content_len = ntohl(content_len);
            char *buf = (char *)malloc(sizeof(char) * (content_len + 1));
            if(read(chat_fd, buf, sizeof(char) * content_len)<0) break;
            buf[content_len] = '\0';

            ChatAddMessage(user,
                           direction == 'R' ? CHAT_MESSAGE_RECEIVED : CHAT_MESSAGE_SENT,
                           read_b == '*',
                           type == 'F' ? CHAT_MESSAGE_FILE : CHAT_MESSAGE_TEXT,
                           (time_t)timestamp,
                           buf);
            free(buf);
        }
        close(chat_fd);
        return true;
    }
    else
        return false;
}

bool ChatSave()
{
    // we can't save a chat if we aren't logged in because the file path depends on our username
    if (CLIMode != MODE_LOGIN)
    {
        MK_CHAT_DIR

        // we save every loaded chat
        for (Chat *i = ChatList; i; i = i->next)
        {
            int chat_fd = open(ChatGetFilename(i->dst), O_WRONLY | O_CREAT | O_TRUNC, S_IRUSR | S_IWUSR);
            if (chat_fd < 0)
            {
                dbgerror("Error opening chat file");
                return false;
            }
            // now we serialize the chat writing on the appropriate file
            for (ChatMessage *j = i->head; j; j = j->next)
            {
                uint32_t content_len = htonl(strlen(j->content));
                uint64_t timestamp = htonq(j->timestamp);

                if(write(chat_fd, &timestamp, sizeof(uint64_t))<0) break;
                if(write(chat_fd, j->type == CHAT_MESSAGE_FILE ? "F" : "T", sizeof(uint8_t))<0) break;
                if(write(chat_fd, j->direction == CHAT_MESSAGE_RECEIVED ? "R" : "S", sizeof(uint8_t))<0) break;
                if(write(chat_fd, j->read ? "*" : " ", sizeof(uint8_t))<0) break;
                if(write(chat_fd, &content_len, sizeof(uint32_t))<0) break;
                if(write(chat_fd, j->content, ntohl(content_len))<0) break;
            }
            close(chat_fd);
        }
        return true;
    }
    else
        return false;
}

void ChatWrite()
{
    char text[CHAT_MAX_MESSAGE_LEN];
    memset(text, 0, CHAT_MAX_MESSAGE_LEN);
    // get a line from stdin
    if(!fgets(text, CHAT_MAX_MESSAGE_LEN, stdin)) return;
    // we erase the stdin line to overwrite it afterwards using ChatPrintMessage
    printf("\033[1A\r");
    *strrchr(text, '\n') = '\0';
    time_t timestamp = time(NULL);
    for (ChatTarget *i = ChatTargetList; i; i = i->next)
    {
        // we must send the text to every user in the target list
        NetworkSendMessageDataText(i->sockfd < 0 ? NetworkServerInfo.sockfd : i->sockfd, CLIActiveUsername, i->dst, timestamp, text);
        // we must add the message to the list and write it to file
        ChatLoad(i->dst);
        ChatAddMessage(i->dst, CHAT_MESSAGE_SENT, i->sockfd >= 0, CHAT_MESSAGE_TEXT, timestamp, text);
    }
    // print our message in a nice format
    ChatPrintMessage(*(ChatFind(ChatTargetList->dst)->tail), ChatTargetList->dst);
}

void ChatQuit()
{
    // free each entry of ChatTargetList
    while (ChatTargetList)
    {
        ChatTarget *target = ChatTargetList;
        ChatTargetList = target->next;
        free(target);
    }
}

void ChatPrintMessage(ChatMessage msg, UserName dst)
{
    struct tm timestamp = *localtime(&msg.timestamp);
    char time_buf[21];
    // first we format time
    snprintf(time_buf, 21, "%02u/%02u/%04u %02u:%02u:%02u",
             timestamp.tm_mday % 100,
             (timestamp.tm_mon + 1) % 100,
             (timestamp.tm_year + 1900) % 10000,
             timestamp.tm_hour % 100,
             timestamp.tm_min % 100,
             timestamp.tm_sec % 100);
    // then we print time | sender | ** text leaving always the same spacing, aligning text to the left
    printf(" %-20s | %-20s | %2s %s%s\n",
           time_buf,
           msg.direction == CHAT_MESSAGE_SENT ? "You" : dst.str,
           msg.read ? "**" : "*",
           msg.type == CHAT_MESSAGE_FILE ? "file://" : "",
           msg.content);
}

Chat *ChatFind(UserName dst)
{
    // we simply scroll through the list
    for (Chat *i = ChatList; i; i = i->next)
    {
        if (strncmp(i->dst.str, dst.str, USERNAME_MAX_LENGTH) == 0)
            return i;
    }
    return NULL;
}

char *ChatGetFilename(UserName dst)
{
    char dirname_cpy[FILENAME_MAX - 26];
    char *dirname = ChatGetDirname();
    if (dirname)
    {
        if(strlen(dirname)>FILENAME_MAX-26) return NULL;
        strncpy(dirname_cpy, dirname, FILENAME_MAX - 26);
        snprintf(filename_tmp_buffer, FILENAME_MAX, "%s/%s.chat", dirname_cpy, dst.str);
        return filename_tmp_buffer;
    }
    else
        return NULL;
}

char *ChatGetDirname()
{
    if (CLIMode != MODE_LOGIN)
    {
        snprintf(filename_tmp_buffer, FILENAME_MAX, "%s/%s", CHAT_DIR, CLIActiveUsername.str);
        return filename_tmp_buffer;
    }
    else
        return NULL;
}

void ChatAddMessage(UserName dst, ChatMessageDirection dir, bool read, ChatMessageType type, time_t timestamp, char *content)
{
    Chat *target_chat = ChatAddChat(dst);

    ChatMessage *target = (ChatMessage *)malloc(sizeof(ChatMessage));
    target->direction = dir;
    target->read = read;
    target->timestamp = timestamp;
    target->type = type;
    target->content = (char *)malloc(strlen(content) + 1);
    strcpy(target->content, content);
    target->next = NULL;

    if (!target_chat->tail)
    { // edit both head and tail
        target_chat->head = target;
    }
    else
    {
        target_chat->tail->next = target;
    }
    target_chat->tail = target;
}

Chat *ChatAddChat(UserName dst)
{
    Chat *ret = ChatFind(dst);
    if (!ret)
    {
        Chat *target = (Chat *)malloc(sizeof(Chat));
        target->dst = dst;
        target->head = NULL;
        target->tail = NULL;
        target->next = ChatList;
        ChatList = target;
        return target;
    }
    else
        return ret;
}

void ChatFree()
{
    // for each chat we delete every message in it
    Chat *next_chat = NULL;
    for (Chat *i = ChatList; i; i = next_chat)
    {
        next_chat = i->next;
        ChatMessage *next_message = NULL;
        for (ChatMessage *j = i->head; j; j = next_message)
        {
            next_message = j->next;
            free(j->content);
            free(j);
        }
        free(i);
    }
    ChatList = NULL;
}

void ChatDelete(UserName user)
{
    // first we find the chat
    Chat *last = NULL;
    Chat *i = NULL;
    for (i = ChatList; i; i = i->next)
    {
        if (strncmp(i->dst.str, user.str, USERNAME_MAX_LENGTH) == 0)
            break;
        last = i;
    }
    if (i)
    { // if found we free every entry in it
        if (!last)
        { // head
            ChatList = i->next;
        }
        else
        { // middle
            last->next = i->next;
        }
        ChatMessage *next = NULL;
        for (ChatMessage *j = i->head; j; j = next)
        {
            next = j->next;
            free(j->content);
            free(j);
        }
        // then we free the chat itself
        free(i);
    }
}

char *ChatNewFilePath(char *filename)
{
    filename += ToolsBasename(filename);
    size_t filename_no_ext_len = 0;
    // we count every char before the first '.'
    for (char *c = filename; *c != '\0' && *c != '.'; c++)
        filename_no_ext_len++;

    char *filename_no_ext = (char *)malloc(filename_no_ext_len + 1);
    memset(filename_no_ext, 0, filename_no_ext_len + 1);
    strncpy(filename_no_ext, filename, filename_no_ext_len);
    bool success = false;
    size_t try = 0;
    // first we try to create the directory
    snprintf(filename_tmp_buffer, FILENAME_MAX, "%s/%s", CHAT_FILE_DIR, CLIActiveUsername.str);

    if (mkdir(CHAT_FILE_DIR, S_IRWXU | S_IRGRP | S_IXGRP | S_IROTH | S_IXOTH) < 0 && errno != EEXIST)
    {
        dbgerror("Error creating file directory");
        return NULL;
    }
    if (mkdir(filename_tmp_buffer, S_IRWXU | S_IRGRP | S_IXGRP | S_IROTH | S_IXOTH) < 0 && errno != EEXIST)
    {
        dbgerror("Error creating file directory");
        free(filename_no_ext);
        return NULL;
    }
    // now we try to find a filename not yet in use
    while (!success)
    {
        if (try == 0)
            snprintf(filename_tmp_buffer, FILENAME_MAX, "%s/%s/%s", CHAT_FILE_DIR, CLIActiveUsername.str, filename);
        else
            snprintf(filename_tmp_buffer, FILENAME_MAX, "%s/%s/%s_%lu%s", CHAT_FILE_DIR, CLIActiveUsername.str, filename_no_ext, (long unsigned int)try, filename + filename_no_ext_len);

        struct stat s;
        // we eliminate the race condition using the username in the path
        if (stat(filename_tmp_buffer, &s) < 0)
        { // file desn't exist
            success = true;
        }
        try++;
    }
    free(filename_no_ext);
    return filename_tmp_buffer;
}

void ChatSaveMessageFile(uint32_t payload_size, const uint8_t *payload)
{
    size_t filename_len = NetworkMessageDataFilenameLength(payload_size, payload);
    size_t file_len = NetworkMessageDataFileSize(payload_size, payload);

    UserName src;
    UserName dst;
    time_t timestamp;
    char *filename = (char *)malloc(sizeof(char) * (filename_len + 1));
    uint8_t *file = (uint8_t *)malloc(file_len);
    // simly deserialize the message
    NetworkDeserializeMessageDataFile(payload_size, payload, &src, &dst, &timestamp, filename, file);
    // save the file and it's path to the message list
    char *file_path = ChatNewFilePath(filename);
    ChatAddMessage(src, CHAT_MESSAGE_RECEIVED, true, CHAT_MESSAGE_FILE, timestamp, file_path);
    int file_fd = open(file_path, O_WRONLY | O_CREAT, S_IRUSR | S_IWUSR);
    if (file_fd < 0)
    {
        dbgerror("Error creating file");
    }
    else
    {
        if(write(file_fd, file, file_len)<0)
            printf("Error saving %s\n",file_path);
        close(file_fd);
    }

    free(filename);
    free(file);
}

void ChatSaveMessageText(uint32_t payload_size, const uint8_t *payload)
{
    size_t text_len = NetworkMessageDataTextLength(payload_size, payload);
    char *text = (char *)malloc(text_len + 1);

    UserName src;
    UserName dst;
    time_t timestamp;
    // simly deserialize the message
    NetworkDeserializeMessageDataText(payload_size, payload, &src, &dst, &timestamp, text);
    // and add it to the message list
    ChatAddMessage(src, CHAT_MESSAGE_RECEIVED, true, CHAT_MESSAGE_TEXT, timestamp, text);

    free(text);
}

ChatTarget *ChatTargetFind(UserName user)
{
    for (ChatTarget *i = ChatTargetList; i; i = i->next)
    {
        if (strncmp(i->dst.str, user.str, USERNAME_MAX_LENGTH) == 0)
            return i;
    }
    return NULL;
}

bool ChatSyncWith(UserName user)
{
    ChatLoad(user);
    // now we request hanging messages from the server and receive the entire list until a MESSAGE_OK is received
    if (NetworkSendMessageHanging(NetworkServerInfo.sockfd, &user) && NetworkReceiveResponseFromServer(MESSAGE_RESPONSE))
    {
        bool done = false;
        while (NetworkServerInfo.message_list_head && !done)
        {
            switch (NetworkServerInfo.message_list_head->header.type)
            {
            case MESSAGE_RESPONSE:
                done = true;
                break;
            case MESSAGE_DATA:
            {
                bool is_file = NetworkMessageDataContainsFile(NetworkServerInfo.message_list_head->header.payload_size, NetworkServerInfo.message_list_head->payload);
                if (is_file)
                    ChatSaveMessageFile(NetworkServerInfo.message_list_head->header.payload_size, NetworkServerInfo.message_list_head->payload);
                else
                    ChatSaveMessageText(NetworkServerInfo.message_list_head->header.payload_size, NetworkServerInfo.message_list_head->payload);
                break;
            }
            default:
                printf("Message not expected\n");
            }
            NetworkDeleteOneFromServer();
        }
        return true;
    }
    else
        return false;
}