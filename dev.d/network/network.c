#include "network.h"

NetworkDeviceConnection NetworkConnectedDevices[NETWORK_MAX_CONNECTIONS + 1];

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