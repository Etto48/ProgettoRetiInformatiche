#include "chat_manager.h"

Chat* ChatList = NULL;
ChatTarget* ChatTargetList = NULL;

void ChatHandleSyncread(UserName dst, time_t timestamp)
{

}

void ChatWrite()
{

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