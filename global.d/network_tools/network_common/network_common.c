#include "network_common.h"

NetworkDeviceConnection NetworkConnectedDevices[NETWORK_MAX_CONNECTIONS];

uint16_t NetworkListeningPort = 0;
fd_set NetworkMasterFdSet;

void NetworkMainLoop(uint16_t port)
{
    memset(&NetworkConnectedDevices, 0, sizeof(NetworkDeviceConnection) * (NETWORK_MAX_CONNECTIONS));

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

    // because the first fd available for a connection with a device is #4 we need to exclude the first 4 fds
    if (listen(server_socket, NETWORK_MAX_CONNECTIONS - 3) < 0)
    {
        dbgerror("Error listening on a socket");
        exit(-1);
    }

    fd_set slave_set;
    FD_ZERO(&NetworkMasterFdSet);
    FD_SET(server_socket, &NetworkMasterFdSet);
    FD_SET(STDIN_FILENO, &NetworkMasterFdSet);
    struct timeval select_timer;
    int ready_fd_count;

    NetworkListeningPort = port;

    while (true)
    {
        do
        {
            slave_set = NetworkMasterFdSet;
            // because select changes the timer we must reset it every time
            select_timer.tv_sec = 1;
            select_timer.tv_usec = 0;
            ready_fd_count = select(NETWORK_MAX_CONNECTIONS + 1, &slave_set, NULL, NULL, &select_timer);
            // select is used with a timer to detect when the program has "free time"
            // we set a nonzero time in the select timer to avoid busy waiting
            if (ready_fd_count < 0)
            {
                // if the syscall was interrupted we must close the program ASAP and save the files
                if (errno != EINTR)
                { // the syscall was not interrupted, fatal error
                    dbgerror("Error selecting available FDs");
                    SaveAndExit(-1);
                }
                else
                    SaveAndExit(0);
            }
            if (ready_fd_count == 0)
            { // in case we have nothing to do
                NetworkFreeTime();
            }
        } while (ready_fd_count == 0);

        for (int target_fd = 0; target_fd < NETWORK_MAX_CONNECTIONS; target_fd++)
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
                    NetworkNewConnection(accepted_socket, new_address);
                }
                else if (target_fd == STDIN_FILENO)
                { // cli input ready
                    CLIHandleInput();
                }
                else
                { // handle connection
                    NetworkReceiveNewData(target_fd);
                }
            }
        }
    }
}

void NetworkNewConnection(int sockfd, struct sockaddr_in addr)
{
    FD_SET(sockfd, &NetworkMasterFdSet);
    NetworkConnectedDevices[sockfd].sockfd = sockfd;
    NetworkConnectedDevices[sockfd].address = addr;
    memset(NetworkConnectedDevices[sockfd].username.str, 0, USERNAME_MAX_LENGTH + 1);
    NetworkConnectedDevices[sockfd].header_received = false;
    NetworkConnectedDevices[sockfd].received_bytes = 0;

    //#ifdef DEBUG
    //    printf("DBG: FD %d connected\n", sockfd);
    //#endif
}

void NetworkDeleteConnection(int sockfd)
{
    NetworkDeletedConnectionHook(sockfd);
    FD_CLR(sockfd, &NetworkMasterFdSet);
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
    shutdown(sockfd, SHUT_RDWR);
    close(sockfd);

    //#ifdef DEBUG
    //    printf("DBG: FD %d disconnected\n", sockfd);
    //#endif
}

int NetworkFindConnection(UserName user)
{
    for (int i = 3; i < NETWORK_MAX_CONNECTIONS; i++)
    {
        NetworkDeviceConnection *ncd = NetworkConnectedDevices + i;
        if (ncd->sockfd && strncmp(user.str, ncd->username.str, USERNAME_MAX_LENGTH) == 0)
            return i;
    }
    return -1;
}

void NetworkReceiveNewData(int sockfd)
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
        NetworkDeleteConnection(sockfd);
        if (received_count < 0)
        {
            dbgerror("Error during recv");
        }
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

            if (NetworkConnectedDevices[sockfd].mh.type > MAX_MESSAGE_TYPE) // if an invalid header was provided ignore size
                NetworkConnectedDevices[sockfd].mh.payload_size = 0;
            if(NetworkConnectedDevices[sockfd].mh.payload_size)
                NetworkConnectedDevices[sockfd].receive_buffer = (uint8_t *)malloc(NetworkConnectedDevices[sockfd].mh.payload_size);
            else 
                NetworkConnectedDevices[sockfd].receive_buffer = NULL;
            NetworkConnectedDevices[sockfd].received_bytes = 0;
            NetworkConnectedDevices[sockfd].header_received = true;
        }
        // check if payload was 0 bytes
        if (NetworkConnectedDevices[sockfd].header_received && NetworkConnectedDevices[sockfd].received_bytes == NetworkConnectedDevices[sockfd].mh.payload_size)
        { // payload completely received
            NetworkHandleNewMessage(sockfd);
            if(NetworkConnectedDevices[sockfd].receive_buffer)
                free(NetworkConnectedDevices[sockfd].receive_buffer);
            NetworkConnectedDevices[sockfd].receive_buffer = NULL;
            NetworkConnectedDevices[sockfd].header_received = false;
            NetworkConnectedDevices[sockfd].received_bytes = 0;
        }
    }
}

bool NetworkIsSocketLoggedIn(int sockfd)
{
    return NetworkConnectedDevices[sockfd].username.str[0] != '\0';
}