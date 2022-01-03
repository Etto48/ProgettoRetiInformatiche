#include "chat_manager.h"

Chat* ChatList = NULL;
ChatTarget* ChatTargetList = NULL;

void ChatHandleSyncread(UserName dst, time_t timestamp)
{
    //TODO: fill this
}

int ChatConnectTo(UserName username, uint32_t ip, uint16_t port)
{
    //search in the chat
    for (size_t i = 3; i < NETWORK_MAX_CONNECTIONS; i++)
    {
        if(NetworkConnectedDevices[i].sockfd && strncmp(username.str,NetworkConnectedDevices[i].username.str,USERNAME_MAX_LENGTH))
        {
            return i;
        }
    }
    //failed to find it, now we must try to connect
    int sockfd = socket(AF_INET,SOCK_STREAM,0);
    if(sockfd<0)
    {
        dbgerror("Error creating a socket");
        SaveAndExit(-1);
    }
    struct sockaddr_in addr = {
        .sin_family = AF_INET,
        .sin_addr = {.s_addr = htonl(ip)},
        .sin_port = htons(port),
        .sin_zero = {0}
    };
    if(connect(sockfd,(struct sockaddr*)&addr,sizeof(addr))<0)
    {
        dbgerror("Error connecting to device");
        return -1;
    }
    NetworkNewConnection(sockfd,addr);
    NetworkConnectedDevices[sockfd].username = username;
    NetworkSendMessageLogin(sockfd,0,username,CreatePassword(""));//send your username
    return sockfd;
}

bool ChatAdd(UserName username)
{
    //we must request info to the server about the user we want to add
    if(NetworkServerInfo.connected)
    {
        ChatTarget *new_target = (ChatTarget *)malloc(sizeof(ChatTarget));
        new_target->dst = username;
        //if we find the user online we connect to it, if it is offline we use the server as relay
        if(NetworkSendMessageUserinfoReq(NetworkServerInfo.sockfd,username) && NetworkReceiveResponseFromServer(MESSAGE_USERINFO_RES))
        { // user online
            uint32_t ip;
            uint16_t port;
            NetworkDeserializeMessageUserinfoRes(NetworkServerInfo.message_list_head->header.payload_size,NetworkServerInfo.message_list_head->payload,&ip,&port);
            NetworkDeleteOneFromServer();
            new_target->sockfd = ChatConnectTo(username,ip,port);
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

void ChatWrite()
{
    char text[CHAT_MAX_MESSAGE_LEN];
    memset(text,0,CHAT_MAX_MESSAGE_LEN);
    fgets(text,CHAT_MAX_MESSAGE_LEN,STDIN_FILENO);
    time_t timestamp = time(NULL);
    for(ChatTarget* i = ChatTargetList; i; i=i->next)
    {
        //we must send the text to every user in the target list
        NetworkSendMessageDataText(i->sockfd<0?NetworkServerInfo.sockfd:i->sockfd,CLIActiveUsername,i->dst,timestamp,text);
        //we must add the message to the list and write it to file
        //TODO: write to file the message
    }
}

void ChatQuit()
{
    while(ChatTargetList)
    {
        ChatTarget* target = ChatTargetList;
        ChatTargetList = target->next;
        free(target);
    }
}

void ChatPrintMessage(ChatMessage msg,UserName dst)
{
    struct tm timestamp = *localtime(&msg.timestamp);
    char time_buf[21];
    snprintf(time_buf,21,"%02d/%02d/%d %02d:%02d:%02d",
                timestamp.tm_mday,
                timestamp.tm_mon+1, 
                timestamp.tm_year+1900, 
                timestamp.tm_hour, 
                timestamp.tm_min, 
                timestamp.tm_sec);
    printf("%-20s | %-20s | %2s %s%s\n",
        time_buf,
        msg.direction==CHAT_MESSAGE_SENT?"You":dst.str,
        msg.read?"**":"*",
        msg.type==CHAT_MESSAGE_FILE?"file://":"",
        msg.content);
}