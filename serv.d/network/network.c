#include "network.h"

void NetworkHandleNewMessage(int sockfd)
{
    switch (NetworkConnectedDevices[sockfd].mh.type)
    {
    case MESSAGE_SIGNUP:
        DebugTag("SERV SIGNUP");
        NetworkHandleSignup(sockfd);
        break;
    case MESSAGE_LOGIN:
        DebugTag("SERV LOGIN");
        NetworkHandleLogin(sockfd);
        break;
    case MESSAGE_LOGOUT:
        DebugTag("SERV LOGOUT");
        NetworkHandleLogout(sockfd);
        NetworkDeleteConnection(sockfd);
        break;
    case MESSAGE_HANGING:
        DebugTag("SERV HANGING");
        NetworkHandleHanging(sockfd);
        break;
    case MESSAGE_USERINFO_REQ:
        DebugTag("SERV USERINFO_REQ");
        NetworkHandleUserinfoReq(sockfd);
        break;
    case MESSAGE_DATA:
        DebugTag("SERV DATA");
        NetworkHandleData(sockfd);
        break;
    default:
        DebugTag("SERV ERROR");
        NetworkHandleError(sockfd);
    }
}

void NetworkHandleSignup(int sockfd)
{
    NetworkDeviceConnection *ncd = &NetworkConnectedDevices[sockfd];
    UserName username;
    Password password;
    NetworkDeserializeMessageSignup(ncd->mh.payload_size, ncd->receive_buffer, &username, &password);
    bool ok = AuthRegister(username, password);
    NetworkSendMessageResponse(sockfd, ok);
}

void NetworkHandleLogin(int sockfd)
{
    NetworkDeviceConnection *ncd = &NetworkConnectedDevices[sockfd];
    uint16_t port;
    UserName username;
    Password password;
    NetworkDeserializeMessageLogin(ncd->mh.payload_size, ncd->receive_buffer, &port, &username, &password);
    bool ok = IndexLogin(username, password, ntohl(ncd->address.sin_addr.s_addr), port);
    if (ok)
        ncd->username = username;
    NetworkSendMessageResponse(sockfd, ok);
    if (ok)
    { // we check if user has a pending syncread message
        RelaySyncreadNotice* rsn;
        while((rsn = RelaySyncreadFind(&username,NULL)))
        {
            if(NetworkSendMessageSyncread(sockfd,rsn->dst,rsn->timestamp))
                RelaySyncreadDelete(&username,NULL);
            else break; // we failed to send a syncread message so we can't delete it, we will try next time
        }
    }
}

void NetworkHandleLogout(int sockfd)
{
    NetworkDeviceConnection *ncd = &NetworkConnectedDevices[sockfd];
    bool ok;
    if (NetworkIsSocketLoggedIn(sockfd))
    {
        ok = IndexLogout(ncd->username);
        if (ok)
            memset(ncd->username.str, 0, USERNAME_MAX_LENGTH + 1);
    }
    else
        ok = false;
    NetworkSendMessageResponse(sockfd, ok);
    shutdown(sockfd, SHUT_RDWR);
    close(sockfd);
}

void NetworkHandleHanging(int sockfd)
{
    NetworkDeviceConnection *ncd = &NetworkConnectedDevices[sockfd];
    if (NetworkIsSocketLoggedIn(sockfd))
    {
        UserName from;
        NetworkDeserializeMessageHanging(ncd->mh.payload_size, ncd->receive_buffer, &from);
        if (from.str[0] == '\0')
        { // only users
            for (RelayMessage *i = RelayHangingFindFirst(RelayHangingList, NULL, &ncd->username); i; i = RelayHangingFindFirst(i->next, NULL, &ncd->username))
            {
                NetworkSendMessageHanging(sockfd, &i->src);
            }
        }
        else
        { // only messages from user
            time_t last_timestamp = 0;
            for (RelayMessage *i = RelayHangingPopFirst(&from, &ncd->username); i; RelayHangingDestroyMessage(i))
            {
                switch (i->type)
                {
                case RELAY_MESSAGE_TEXT:
                    NetworkSendMessageDataText(sockfd, from, ncd->username, i->timestamp, (char *)i->data);
                    break;
                case RELAY_MESSAGE_FILE:
                    NetworkSendMessageDataFileBuffer(sockfd, from, ncd->username, i->timestamp, i->filename, i->data_size, i->data);
                    break;
                }
                last_timestamp = i->timestamp > last_timestamp ? i->timestamp : last_timestamp;
            }
            if(last_timestamp > 0)
            { // now we must send a message syncread to the other end if possible
                RelaySyncreadEdit(from,ncd->username,last_timestamp);
            }
        }
        NetworkSendMessageResponse(sockfd, true);
        
    }
    else
        NetworkSendMessageResponse(sockfd, false);
}

void NetworkHandleUserinfoReq(int sockfd)
{
    NetworkDeviceConnection *ncd = &NetworkConnectedDevices[sockfd];
    if (NetworkIsSocketLoggedIn(sockfd))
    {
        UserName username;
        NetworkDeserializeMessageUserinfoReq(ncd->mh.payload_size, ncd->receive_buffer, &username);
        IndexEntry *res = IndexFind(username);
        uint32_t ip = res ? res->ip : 0;
        uint16_t port = res ? res->port : 0;
        NetworkSendMessageUserinfoRes(sockfd, ip, port);
    }
    else
        NetworkSendMessageResponse(sockfd, false);
}

void NetworkHandleData(int sockfd)
{
    NetworkDeviceConnection *ncd = &NetworkConnectedDevices[sockfd];
    if (NetworkIsSocketLoggedIn(sockfd))
    {
        if (NetworkMessageDataContainsFile(ncd->mh.payload_size, ncd->receive_buffer))
        { // file
            size_t filename_len = NetworkMessageDataFilenameLength(ncd->mh.payload_size, ncd->receive_buffer);
            size_t file_size = NetworkMessageDataFileSize(ncd->mh.payload_size, ncd->receive_buffer);
            char *filename = (char *)malloc(filename_len + 1);
            uint8_t *file_data = (uint8_t *)malloc(file_size);
            UserName src;
            UserName dst;
            time_t timestamp;
            NetworkDeserializeMessageDataFile(ncd->mh.payload_size, ncd->receive_buffer, &src, &dst, &timestamp, filename, file_data);
            // we must use ncd->username instead of src to avoid security issues
            RelayHangingAdd(ncd->username, dst, timestamp, RELAY_MESSAGE_FILE, filename, file_size, file_data);
            free(filename);
            free(file_data);
        }
        else
        { // text
            size_t text_len = NetworkMessageDataTextLength(ncd->mh.payload_size, ncd->receive_buffer);
            char *text = (char *)malloc(text_len + 1);
            UserName src;
            UserName dst;
            time_t timestamp;
            NetworkDeserializeMessageDataText(ncd->mh.payload_size, ncd->receive_buffer, &src, &dst, &timestamp, text);
            // we must use ncd->username instead of src to avoid security issues
            RelayHangingAdd(ncd->username, dst, timestamp, RELAY_MESSAGE_TEXT, NULL, text_len, (uint8_t *)text);
            free(text);
        }
        NetworkSendMessageResponse(sockfd, true);
    }
    else
        NetworkSendMessageResponse(sockfd, false);
}

void NetworkHandleError(int sockfd)
{
    NetworkSendMessageResponse(sockfd, false);
}

void NetworkFreeTime()
{
    
}