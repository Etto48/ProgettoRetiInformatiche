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
        NetworkConnectedDevices[sockfd].receive_buffer = (uint8_t *)malloc(5);
    }
    size_t waiting_size = (NetworkConnectedDevices[sockfd].header_received ? NetworkConnectedDevices[sockfd].mh.payload_size : 5);
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
        }
    }
}

void NetworkHandleSignup(int sockfd)
{
    
}

void NetworkHandleLogin(int sockfd)
{
    
}

void NetworkHandleLogout(int sockfd)
{
    
}

void NetworkHandleHanging(int sockfd)
{
    
}

void NetworkHandleUserinfoReq(int sockfd)
{
    
}

void NetworkHandleData(int sockfd)
{
    
}

void NetworkHandleError(int sockfd)
{
    
}

