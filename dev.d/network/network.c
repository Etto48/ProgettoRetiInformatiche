#include "network.h"

ServerConnectionInfo NetworkServerInfo = {false,-1,{0,0,0},NULL,NULL};

void NetworkHandleNewMessage(int sockfd,__attribute__((unused)) fd_set* master)
{
    switch (NetworkConnectedDevices[sockfd].mh.type)
    {
        case MESSAGE_DATA:
            DebugTag("DEV DATA");
            NetworkHandleData(sockfd);
            break;
        default:
            DebugTag("DEV ERROR");
            NetworkHandleError(sockfd);
    }
}

void NetworkHandleData(int sockfd)
{
    
}

void NetworkHandleError(int sockfd)
{
    NetworkSendMessageResponse(sockfd, false);
}

bool NetworkStartServerConnection(uint16_t port)
{
    if(!NetworkServerInfo.connected)
    {
        NetworkServerInfo.sockfd = socket(AF_INET,SOCK_STREAM,0);
        if(NetworkServerInfo.sockfd<0)
        {
            dbgerror("Error creating a socket");
            return false;
        }
        NetworkServerInfo.address.sin_family = AF_INET;
        NetworkServerInfo.address.sin_addr.s_addr = inet_addr(SERVER_ADDRESS);
        NetworkServerInfo.address.sin_port = htons(port);
        if(connect(NetworkServerInfo.sockfd,(struct sockaddr*)&NetworkServerInfo.address,sizeof(NetworkServerInfo.address))<0)
        {
            dbgerror("Error connecting to the server");
            return false;
        }
        NetworkServerInfo.connected = true;
    }
    return true;
}

void NetworkFreeTime()
{
    if (NetworkServerInfo.connected)
    {
        fd_set only_server;
        FD_ZERO(&only_server);
        FD_SET(NetworkServerInfo.sockfd, &only_server);
        struct timeval zero_timer;
        memset(&zero_timer, 0, sizeof(struct timeval));
        if (select(NetworkServerInfo.sockfd + 1, &only_server, NULL, NULL, &zero_timer) <= 0)
        {
            return;
        }
        else if (FD_ISSET(NetworkServerInfo.sockfd, &only_server))
        {
            if (NetworkReceiveOneFromServer())
                NetworkHandleServerNotifications();
        }
    }
    else
    {
        if(NetworkServerInfo.address.sin_port) // we try to reconnect to the server in case we already attempted the connection once
            NetworkStartServerConnection(ntohs(NetworkServerInfo.address.sin_port));
    }
}

bool NetworkReceiveOneFromServer()
{
    ServerMessage new_message;
    uint8_t message_header[NETWORK_SERIALIZED_HEADER_SIZE];
    ssize_t received_size = recv(NetworkServerInfo.sockfd,message_header,NETWORK_SERIALIZED_HEADER_SIZE,MSG_WAITALL);
    if(received_size<=0)
    {
        NetworkServerInfo.connected=false;
        return false;
    }
    new_message.header = NetworkDeserializeHeader(message_header);
    if(new_message.header.payload_size!=0)
    { // we need to allocate and receive the payload only if it's needed
        new_message.payload = (uint8_t*)malloc(new_message.header.payload_size);
        received_size = recv(NetworkServerInfo.sockfd,new_message.payload,new_message.header.payload_size,MSG_WAITALL);
        if(received_size<=0)
        {
            NetworkServerInfo.connected=false;
            free(new_message.payload);
            return false;
        }
    }
    else
    {
        new_message.payload = NULL;
    }
    new_message.next = NetworkServerInfo.message_list_head;
    ServerMessage* true_new_message = (ServerMessage*)malloc(sizeof(ServerMessage));
    *true_new_message = new_message;
    if(NetworkServerInfo.message_list_tail==NULL)
    { // the list is empty so we set both the head and the tail
        NetworkServerInfo.message_list_tail = true_new_message;
    }
    NetworkServerInfo.message_list_head = true_new_message;
    return true;
}

bool NetworkDeleteOneFromServer()
{
    if(NetworkServerInfo.message_list_head)
    { // if there is anything in the list
        ServerMessage* target = NetworkServerInfo.message_list_head;
        if(target->next)
        { // the new list is not empty so we need to change only the head
            NetworkServerInfo.message_list_head = target->next;
        }
        else
        { // the new list is empty so we need to change the tail
            NetworkServerInfo.message_list_head = NetworkServerInfo.message_list_tail = NULL;
        }
        // now we delete the payload and the message struct
        if(target->payload)
            free(target->payload);
        free(target);
        return true;
    }
    else return false;
}

void NetworkHandleServerNotifications()
{
    if(NetworkServerInfo.message_list_head)
    { // if there is anything in the list
        switch(NetworkServerInfo.message_list_head->header.type)
        {
            case MESSAGE_SYNCREAD:
                NetworkHandleSyncread(NetworkServerInfo.message_list_head);
                NetworkDeleteOneFromServer();
                break;
            default:
                break;
        }
    }
}