#include "chat_manager.h"

char filename_tmp_buffer[FILENAME_MAX] = "";

Chat *ChatList = NULL;
ChatTarget *ChatTargetList = NULL;

void ChatHandleSyncread(UserName dst, time_t timestamp)
{
    ChatLoad(dst);
    Chat* chat = ChatFind(dst);
    if(chat)
    {
        for(ChatMessage* i = chat->head; i; i=i->next)
        {
            if(i->timestamp <= timestamp)
                i->read = true;
            else break;
        }
    }
}

int ChatConnectTo(UserName username, uint32_t ip, uint16_t port)
{
    // search in the chat
    for (size_t i = 3; i < NETWORK_MAX_CONNECTIONS; i++)
    {
        if (NetworkConnectedDevices[i].sockfd && strncmp(username.str, NetworkConnectedDevices[i].username.str, USERNAME_MAX_LENGTH))
            return i;
    }
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
    if (connect(sockfd, (struct sockaddr *)&addr, sizeof(addr)) < 0)
    {
        dbgerror("Error connecting to device");
        return -1;
    }
    NetworkNewConnection(sockfd, addr);
    NetworkConnectedDevices[sockfd].username = username;
    NetworkSendMessageLogin(sockfd, 0, username, CreatePassword("")); // send your username
    return sockfd;
}

bool ChatAddTarget(UserName username)
{
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
            new_target->sockfd = ChatConnectTo(username, ip, port);
        }
        else
        { // user offline
            new_target->sockfd = -1;
        }
        new_target->next = ChatTargetList;
        ChatTargetList = new_target;
        return true;
    }
    else
        return false;
}

bool ChatLoad(UserName user)
{
    if(ChatFind(user))
        return true;
    int chat_fd = open(ChatGetFilename(user),O_RDONLY);
    if(chat_fd<0)
    {
        dbgerror("Error reading chat file");
        return false;
    }
    while(true)
    {
        uint64_t timestamp;
        uint8_t type;
        uint8_t direction;
        uint8_t read_b;
        uint32_t content_len;
        if(read(chat_fd,&timestamp,sizeof(uint64_t))<=0)
            break;
        timestamp = ntohq(timestamp);
        read(chat_fd,&type,sizeof(uint8_t));
        read(chat_fd,&direction,sizeof(uint8_t));
        read(chat_fd,&read_b,sizeof(uint8_t));
        read(chat_fd,&content_len,sizeof(uint32_t));
        content_len = ntohl(content_len);
        char* buf = (char*)malloc(sizeof(char)*(content_len+1));
        read(chat_fd,buf,sizeof(char)*content_len);
        buf[content_len]='\0';
        
        ChatAddMessage(user,
            direction=='R'?CHAT_MESSAGE_RECEIVED:CHAT_MESSAGE_SENT,
            read_b=='*',
            type=='F'?CHAT_MESSAGE_FILE:CHAT_MESSAGE_TEXT,
            (time_t)timestamp,
            buf);
        free(buf);
    }
    return true;
}

void ChatSave()
{
    if(mkdir(CHAT_DIR,S_IRUSR|S_IWUSR)<0)
    {
        if(errno!=EEXIST)
        {
            dbgerror("Error creating chat directory");
            return;
        }
    }
    for(Chat* i = ChatList; i; i=i->next)
    {
        int chat_fd = open(ChatGetFilename(i->dst),O_WRONLY|O_CREAT,S_IRUSR|S_IWUSR);
        if(chat_fd<0)
        {
            dbgerror("Error opening chat file");
            return;
        }
        for(ChatMessage* j = i->head; j; j=j->next)
        {   
            uint32_t content_len = htonl(strlen(j->content));
            uint64_t timestamp = htonq(j->timestamp);
            
            write(chat_fd,&timestamp,sizeof(uint64_t));
            write(chat_fd,j->type==CHAT_MESSAGE_FILE?"F":"T",sizeof(uint8_t));
            write(chat_fd,j->direction==CHAT_MESSAGE_RECEIVED?"R":"S",sizeof(uint8_t));
            write(chat_fd,j->read?"*":" ",sizeof(uint8_t));
            write(chat_fd,&content_len,sizeof(uint32_t));
            write(chat_fd,j->content,ntohl(content_len));
        }
        close(chat_fd);
    }
}

void ChatWrite()
{
    char text[CHAT_MAX_MESSAGE_LEN];
    memset(text, 0, CHAT_MAX_MESSAGE_LEN);
    fgets(text, CHAT_MAX_MESSAGE_LEN, STDIN_FILENO);
    time_t timestamp = time(NULL);
    for (ChatTarget *i = ChatTargetList; i; i = i->next)
    {
        // we must send the text to every user in the target list
        NetworkSendMessageDataText(i->sockfd < 0 ? NetworkServerInfo.sockfd : i->sockfd, CLIActiveUsername, i->dst, timestamp, text);
        // we must add the message to the list and write it to file
        ChatLoad(i->dst);
        ChatAddMessage(i->dst,CHAT_MESSAGE_SENT,i->sockfd>=0,CHAT_MESSAGE_TEXT,timestamp,text);
    }
}

void ChatQuit()
{
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
    snprintf(time_buf, 21, "%02d/%02d/%d %02d:%02d:%02d",
             timestamp.tm_mday,
             timestamp.tm_mon + 1,
             timestamp.tm_year + 1900,
             timestamp.tm_hour,
             timestamp.tm_min,
             timestamp.tm_sec);
    printf("%-20s | %-20s | %2s %s%s\n",
           time_buf,
           msg.direction == CHAT_MESSAGE_SENT ? "You" : dst.str,
           msg.read ? "**" : "*",
           msg.type == CHAT_MESSAGE_FILE ? "file://" : "",
           msg.content);
}

Chat *ChatFind(UserName dst)
{
    for (Chat *i = ChatList; i; i = i->next)
    {
        if (strncmp(i->dst.str, dst.str, USERNAME_MAX_LENGTH) == 0)
            return i;
    }
    return NULL;
}

char* ChatGetFilename(UserName dst)
{
    snprintf(filename_tmp_buffer,FILENAME_MAX,"%s/%s.chat",CHAT_DIR,dst.str);
    return filename_tmp_buffer;
}

void ChatAddMessage(UserName dst, ChatMessageDirection dir, bool read, ChatMessageType type, time_t timestamp, char* content)
{
    Chat* target_chat = ChatAddChat(dst);

    ChatMessage* target = (ChatMessage*)malloc(sizeof(ChatMessage));
    target->direction = dir;
    target->read = read;
    target->timestamp = timestamp;
    target->type = type;
    target->content = (char*)malloc(strlen(content)+1);
    strcpy(target->content,content);
    target->next = NULL;

    if(!target_chat->tail)
    { // edit both head and tail
        target_chat->head = target;
    }
    else
    {
        target_chat->tail->next = target;
    }
    target_chat->tail = target;
}

Chat* ChatAddChat(UserName dst)
{
    Chat* ret = ChatFind(dst);
    if(!ret)
    {
        Chat* target = (Chat*)malloc(sizeof(Chat));
        target->dst = dst;
        target->head = NULL;
        target->tail = NULL;
        target->next = ChatList;
        ChatList = target;
        return target;
    }
    else return ret;
}

void ChatFree()
{
    ChatSave();
    Chat* next_chat = NULL;
    for(Chat* i = ChatList; i;i=next_chat)
    {
        next_chat = i->next;
        ChatMessage* next_message = NULL;
        for(ChatMessage* j = i->head; j;j=next_message)
        {
            next_message = j->next;
            free(j->content);
            free(j);
        }
        free(i);
    }
    ChatList = NULL;
}