#include "network.h"

NetworkDeviceConnection NetworkConnectedDevices[NETWORK_MAX_CONNECTIONS + 1];

void NetworkServerMainLoop(uint16_t port)
{
    memset(&NetworkConnectedDevices, 0, sizeof(NetworkDeviceConnection) * (NETWORK_MAX_CONNECTIONS + 1));

    int server_socket = socket(AF_INET, SOCK_STREAM, 0);
    if (server_socket < 0)
    {
        dbgerror("Error creating a socket for the server");
        exit(-1);
    }
    int yes = 1;
    if (setsockopt(server_socket, SOL_SOCKET, SO_REUSEADDR, &yes, sizeof(yes)) < 0)
    {
        dbgerror("Error setting SO_REUSEADDR");
        exit(-1);
    }
    struct sockaddr_in server_address;
    memset(&server_address, 0, sizeof(server_address));
    server_address.sin_family = AF_INET;
    server_address.sin_port = htons(port);
    server_address.sin_addr.s_addr = INADDR_ANY;
    if (bind(server_socket, (struct sockaddr *)&server_address, sizeof(struct sockaddr_in)) < 0)
    {
        dbgerror("Error binding socket");
        exit(-1);
    }

    if (listen(server_socket, NETWORK_MAX_CONNECTIONS) < 0)
    {
        dbgerror("Error listening on a socket");
        exit(-1);
    }

    fd_set master_set, slave_set;
    int max_fdi = server_socket + 1;
    FD_SET(server_socket, &master_set);
    struct timeval select_timer;
    memset(&select_timer, 0, sizeof(struct timeval));

    while (true)
    {
        int ready_fd_count = 0;
        while (ready_fd_count == 0)
        {
            slave_set = master_set;
            ready_fd_count = select(max_fdi, &slave_set, NULL, NULL, &select_timer);
            if (ready_fd_count < 0)
            {
                dbgerror("Error selecting available FDs");
                SaveAndExit(-1);
            }
        }
        for (int target_fd = 0; target_fd < max_fdi; target_fd++)
        {
            if (FD_ISSET(target_fd, &slave_set))
            {
                if (target_fd == server_socket)
                { // accept connection
                    struct sockaddr_in new_address;
                    socklen_t new_address_len = sizeof(new_address);
                    int accepted_socket = accept(server_socket, (struct sockaddr *)&new_address, &new_address_len);
                    if (accepted_socket < 0)
                    {
                        dbgerror("Error accepting a new connection");
                        SaveAndExit(-1);
                    }
                    NetworkNewConnection(accepted_socket, new_address, &master_set);
                }
                else
                { // handle connection
                }
            }
        }
    }
}

void NetworkNewConnection(int sockfd, struct sockaddr_in addr, fd_set *master)
{
    FD_SET(sockfd, master);
    NetworkConnectedDevices[sockfd].sockfd = sockfd;
    NetworkConnectedDevices[sockfd].address = addr;
    memset(NetworkConnectedDevices[sockfd].username.str, 0, USERNAME_MAX_LENGTH + 1);
    NetworkConnectedDevices[sockfd].header_received = false;
    NetworkConnectedDevices[sockfd].received_bytes = 0;
}

void NetworkDeleteConnection(int sockfd, fd_set *master)
{
    FD_CLR(sockfd, master);
    NetworkConnectedDevices[sockfd].sockfd = 0;
    memset(&NetworkConnectedDevices[sockfd].address, 0, sizeof(struct sockaddr_in));
    memset(NetworkConnectedDevices[sockfd].username.str, 0, USERNAME_MAX_LENGTH + 1);
    if (NetworkConnectedDevices[sockfd].header_received)
    {
        memset(&NetworkConnectedDevices[sockfd].mh, 0, sizeof(MessageHeader));
    }
    if (NetworkConnectedDevices[sockfd].receive_buffer)
    {
        free(NetworkConnectedDevices[sockfd].receive_buffer);
        NetworkConnectedDevices[sockfd].receive_buffer = NULL;
    }
    NetworkConnectedDevices[sockfd].received_bytes = 0;
}

void NetworkReceiveNewData(int sockfd, fd_set *master)
{
    if (!NetworkConnectedDevices[sockfd].header_received && !NetworkConnectedDevices[sockfd].receive_buffer)
    { // we need to allocate a buffer big enough to contain the header
        NetworkConnectedDevices[sockfd].receive_buffer = (uint8_t *)malloc(NETWORK_SERIALIZED_HEADER_SIZE);
    }
    size_t waiting_size = (NetworkConnectedDevices[sockfd].header_received ? NetworkConnectedDevices[sockfd].mh.payload_size : NETWORK_SERIALIZED_HEADER_SIZE);
    // now we try to receive the exact amount of data we expect from the device
    int received_count = recv(
        sockfd,
        NetworkConnectedDevices[sockfd].receive_buffer + NetworkConnectedDevices[sockfd].received_bytes,
        waiting_size - NetworkConnectedDevices[sockfd].received_bytes,
        0);
    if (received_count <= 0) // client disconnected or error
    {
        NetworkDeleteConnection(sockfd, master);
        return;
    }
    else // here we update how much data we received
        NetworkConnectedDevices[sockfd].received_bytes += received_count;

    if (NetworkConnectedDevices[sockfd].received_bytes == waiting_size)
    {
        if (!NetworkConnectedDevices[sockfd].header_received)
        { // header completely received
            NetworkConnectedDevices[sockfd].mh =
                NetworkDeserializeHeader(NetworkConnectedDevices[sockfd].receive_buffer);
            free(NetworkConnectedDevices[sockfd].receive_buffer);
            NetworkConnectedDevices[sockfd].receive_buffer = (uint8_t *)malloc(NetworkConnectedDevices[sockfd].mh.payload_size);
            NetworkConnectedDevices[sockfd].header_received = true;
        }
        else
        { // payload completely received
            switch (NetworkConnectedDevices[sockfd].mh.type)
            {
            case MESSAGE_SIGNUP:
                DebugTag("SERV_SIGNUP");
                NetworkHandleSignup(sockfd);
                break;
            case MESSAGE_LOGIN:
                DebugTag("SERV_LOGIN");
                NetworkHandleLogin(sockfd);
                break;
            case MESSAGE_LOGOUT:
                DebugTag("SERV_LOGOUT");
                NetworkHandleLogout(sockfd);
                NetworkDeleteConnection(sockfd,master);
                break;
            case MESSAGE_HANGING:
                DebugTag("SERV_HANGING");
                NetworkHandleHanging(sockfd);
                break;
            case MESSAGE_USERINFO_REQ:
                DebugTag("SERV_USERINFO_REQ");
                NetworkHandleUserinfoReq(sockfd);
                break;
            case MESSAGE_DATA:
                DebugTag("SERV_DATA");
                NetworkHandleData(sockfd);
                break;
            default:
                DebugTag("SERV_ERROR");
                NetworkHandleError(sockfd);
            }
            free(NetworkConnectedDevices[sockfd].receive_buffer);
            NetworkConnectedDevices[sockfd].receive_buffer = NULL;
            NetworkConnectedDevices[sockfd].header_received = false;
            NetworkConnectedDevices[sockfd].received_bytes = 0;
        }
    }
}

bool isSocketLoggedIn(int sockfd)
{
    return NetworkConnectedDevices[sockfd].username.str[0];
}

void NetworkHandleSignup(int sockfd)
{
    NetworkDeviceConnection* ncd = &NetworkConnectedDevices[sockfd];
    UserName username;
    Password password;
    NetworkDeserializeMessageSignup(ncd->mh.payload_size,ncd->receive_buffer,&username,&password);
    bool ok = AuthRegister(username,password);
    NetworkSendMessageResponse(sockfd,ok);
}

void NetworkHandleLogin(int sockfd)
{
    NetworkDeviceConnection* ncd = &NetworkConnectedDevices[sockfd];
    uint16_t port;
    UserName username;
    Password password;
    NetworkDeserializeMessageLogin(ncd->mh.payload_size,ncd->receive_buffer,&port,&username,&password);
    bool ok = IndexLogin(username,password,ntohl(ncd->address.sin_addr.s_addr),port);
    if(ok)
        ncd->username=username;
    NetworkSendMessageResponse(sockfd,ok);
}

void NetworkHandleLogout(int sockfd)
{
    NetworkDeviceConnection* ncd = &NetworkConnectedDevices[sockfd];
    bool ok;
    if(isSocketLoggedIn(sockfd))
    {
        ok = IndexLogout(ncd->username);
        if(ok)
            memset(ncd->username.str,0,USERNAME_MAX_LENGTH+1);
    }
    else
        ok = false;
    NetworkSendMessageResponse(sockfd,ok);
    shutdown(sockfd,SHUT_RDWR);
    close(sockfd);
}

void NetworkHandleHanging(int sockfd)
{
    NetworkDeviceConnection* ncd = &NetworkConnectedDevices[sockfd];
    if(isSocketLoggedIn(sockfd))
    {
        UserName from;
        NetworkDeserializeMessageHanging(ncd->mh.payload_size,ncd->receive_buffer,&from);
        if(from.str[0]=='\0')
        { // only users
            RelayMessage* list = RelayHangingList;
            for(RelayMessage* i = RelayHangingFindFirst(list,NULL,&ncd->username); i; list = i->next)
            {
                NetworkSendMessageHanging(sockfd,&i->src);
            }
        }
        else
        { // only messages from user
            for(RelayMessage* i = RelayHangingPopFirst(&from,&ncd->username); i; RelayHangingDestroyMessage(i))
            {
                switch(i->type)
                {
                    case RELAY_MESSAGE_TEXT:
                        NetworkSendMessageDataText(sockfd,from,ncd->username,i->timestamp,(char*)i->data);
                        break;
                    case RELAY_MESSAGE_FILE:
                        NetworkSendMessageDataFileBuffer(sockfd,from,ncd->username,i->timestamp,i->filename,i->data_size,i->data);
                        break;
                }
            }
        }
        NetworkSendMessageResponse(sockfd,true);
    }
    else
        NetworkSendMessageResponse(sockfd,false);
}

void NetworkHandleUserinfoReq(int sockfd)
{
    NetworkDeviceConnection* ncd = &NetworkConnectedDevices[sockfd];
    if(isSocketLoggedIn(sockfd))
    {
        UserName username;
        NetworkDeserializeMessageUserinfoReq(ncd->mh.payload_size,ncd->receive_buffer,&username);
        IndexEntry* res = IndexFind(username);
        uint32_t ip = res ? res->ip : 0;
        uint16_t port = res ? res->port : 0;
        NetworkSendMessageUserinfoRes(sockfd,ip,port);
    }
    else
        NetworkSendMessageResponse(sockfd,false);
}

void NetworkHandleData(int sockfd)
{
    NetworkDeviceConnection* ncd = &NetworkConnectedDevices[sockfd];
    if(isSocketLoggedIn(sockfd))
    {
        if(NetworkMessageDataContainsFile(ncd->mh.payload_size,ncd->receive_buffer))
        { // file 
            size_t filename_len = NetworkMessageDataFilenameLength(ncd->mh.payload_size,ncd->receive_buffer);
            size_t file_size = NetworkMessageDataFileSize(ncd->mh.payload_size,ncd->receive_buffer);
            char* filename = (char *)malloc(filename_len+1);
            uint8_t* file_data = (uint8_t *)malloc(file_size);
            UserName src;
            UserName dst;
            time_t timestamp;
            NetworkDeserializeMessageDataFile(ncd->mh.payload_size,ncd->receive_buffer,&src,&dst,&timestamp,filename,file_data);
            //we must use ncd->username instead of src to avoid security issues
            RelayHangingAdd(ncd->username,dst,timestamp,RELAY_MESSAGE_FILE,filename,file_size,file_data);
            free(filename);
            free(file_data);
        }
        else
        { // text
            size_t text_len = NetworkMessageDataTextLength(ncd->mh.payload_size,ncd->receive_buffer);
            char* text = (char*)malloc(text_len+1);
            UserName src;
            UserName dst;
            time_t timestamp;
            NetworkDeserializeMessageDataText(ncd->mh.payload_size,ncd->receive_buffer,&src,&dst,&timestamp,text);
            //we must use ncd->username instead of src to avoid security issues
            RelayHangingAdd(ncd->username,dst,timestamp,RELAY_MESSAGE_TEXT,NULL,text_len,(uint8_t*)text);
            free(text);
        }
        NetworkSendMessageResponse(sockfd,true);
    }
    else
        NetworkSendMessageResponse(sockfd,false);
}

void NetworkHandleError(int sockfd)
{
    NetworkSendMessageResponse(sockfd,false);
}

