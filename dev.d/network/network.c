#include "network.h"

ServerConnectionInfo NetworkServerInfo = {
    .connected = false,
    .sockfd = -1,
    .address = {
        .sin_family = AF_INET,
        .sin_addr = {0},
        .sin_port = 0,
        .sin_zero = {}},
    .message_list_head = NULL,
    .message_list_tail = NULL};

void NetworkHandleNewMessage(int sockfd)
{
    switch (NetworkConnectedDevices[sockfd].mh.type)
    {
    case MESSAGE_DATA: // chat message received
        DebugTag("DEV DATA");
        NetworkHandleData(sockfd);
        break;
    case MESSAGE_LOGIN:
        DebugTag("DEV LOGIN");
        NetworkHandleLogin(sockfd);
        break;
    default:
        DebugTag("DEV ERROR");
        NetworkHandleError(sockfd);
    }
}

void NetworkHandleData(int sockfd)
{
    // TODO: fill me
}

void NetworkHandleLogin(int sockfd)
{
    NetworkDeviceConnection *ncd = NetworkConnectedDevices + sockfd;
    if(ncd->username.str[0]=='\0')
    {
        UserName username;
        uint16_t dummy_port;    
        Password dummy_password;
        NetworkDeserializeMessageLogin(ncd->mh.payload_size, ncd->receive_buffer, &dummy_port, &username, &dummy_password);
        ncd->username = username;
    }
}

void NetworkHandleError(int sockfd)
{
    NetworkSendMessageResponse(sockfd, false);
}

bool NetworkStartServerConnection(uint16_t port)
{
    if (!NetworkServerInfo.connected)
    {
        if (NetworkServerInfo.sockfd < 0)
            NetworkServerInfo.sockfd = socket(AF_INET, SOCK_STREAM, 0);
        if (NetworkServerInfo.sockfd < 0)
        {
            dbgerror("Error creating a socket");
            return false;
        }
        NetworkServerInfo.address.sin_family = AF_INET;
        NetworkServerInfo.address.sin_addr.s_addr = inet_addr(SERVER_ADDRESS);
        NetworkServerInfo.address.sin_port = htons(port);
        if (connect(NetworkServerInfo.sockfd, (struct sockaddr *)&NetworkServerInfo.address, sizeof(NetworkServerInfo.address)) < 0)
        {
            // we disable the error to keep the stdout cleaner
            // dbgerror("Error connecting to the server");
            return false;
        }
        NetworkServerInfo.connected = true;

        // Autologin if we was already logged in
        if (CLIActiveUsername.str[0])
        {
            if (NetworkAutoLogin(CLIActiveUsername, CLIActivePassword))
            {
                //DebugLog("Successfully reconnected to the server");
                printf("Successfully reconnected to the server\n");
            }
        }
    }
    return true;
}

void NetworkServerDisconnected()
{
    if (NetworkServerInfo.sockfd >= 0)
    {
        shutdown(NetworkServerInfo.sockfd, SHUT_RDWR);
        close(NetworkServerInfo.sockfd);
        NetworkServerInfo.sockfd = -1;
        NetworkServerInfo.connected = false;
        //DebugLog("Connection to the server was interrupted");
        printf("Connection to the server was interrupted\n");
    }
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
        if (NetworkServerInfo.address.sin_port) // we try to reconnect to the server in case we already attempted the connection once
            NetworkStartServerConnection(ntohs(NetworkServerInfo.address.sin_port));
    }
}

bool NetworkReceiveOneFromServer()
{
    ServerMessage new_message;
    uint8_t message_header[NETWORK_SERIALIZED_HEADER_SIZE];
    ssize_t received_size = recv(NetworkServerInfo.sockfd, message_header, NETWORK_SERIALIZED_HEADER_SIZE, MSG_WAITALL);
    if (received_size <= 0)
    {
        NetworkServerDisconnected();
        return false;
    }
    new_message.header = NetworkDeserializeHeader(message_header);
    if (new_message.header.payload_size != 0)
    { // we need to allocate and receive the payload only if it's needed
        new_message.payload = (uint8_t *)malloc(new_message.header.payload_size);
        received_size = recv(NetworkServerInfo.sockfd, new_message.payload, new_message.header.payload_size, MSG_WAITALL);
        if (received_size <= 0)
        {
            NetworkServerDisconnected();
            free(new_message.payload);
            return false;
        }
    }
    else
    {
        new_message.payload = NULL;
    }
    new_message.next = NULL;
    ServerMessage *true_new_message = (ServerMessage *)malloc(sizeof(ServerMessage));
    *true_new_message = new_message;
    if (NetworkServerInfo.message_list_head == NULL)
    { // the list is empty so we set both the head and the tail
        NetworkServerInfo.message_list_head = true_new_message;
    }
    if (NetworkServerInfo.message_list_tail)
    {
        NetworkServerInfo.message_list_tail->next = true_new_message;
    }
    NetworkServerInfo.message_list_tail = true_new_message;
    return true;
}

bool NetworkReceiveResponseFromServer(MessageType expected_type)
{
    bool error = false;
    size_t count = 0;
    while (true)
    {
        if (!NetworkReceiveOneFromServer())
        {
            error = true;
            break;
        }
        if (NetworkServerInfo.message_list_tail->header.type == MESSAGE_RESPONSE)
        {
            bool ok = false;
            NetworkDeserializeMessageResponse(
                NetworkServerInfo.message_list_tail->header.payload_size,
                NetworkServerInfo.message_list_tail->payload,
                &ok);
            error = !ok;
            break; // we have received everything
        }
        else if (NetworkServerInfo.message_list_tail->header.type == expected_type)
            break; // we are requested to stop here
        else
        {
            NetworkHandleServerNotifications();
        }
        count++;
    }
    if (error)
    { // if we have some error receiving from the server mid-transmission, we delete the trasmission
        for (size_t i = 0; i < count; i++)
        {
            NetworkDeleteOneFromServerTail();
        }
    }
    return !error;
}

void NetworkDeleteOneFromServerTail()
{
    if (NetworkServerInfo.message_list_tail)
    {
        ServerMessage *target = NetworkServerInfo.message_list_tail;
        ServerMessage *last = NULL;
        for (ServerMessage *i = NetworkServerInfo.message_list_head; i; i = i->next)
        {
            if (i == target)
                break;
            last = i;
        }
        if (last)
        { // middle
            NetworkServerInfo.message_list_tail = last;
            last->next = NULL;
        }
        else
        { // head
            NetworkServerInfo.message_list_tail = NetworkServerInfo.message_list_head = NULL;
        }
        if (target->payload)
            free(target->payload);
        free(target);
    }
}

bool NetworkDeleteOneFromServer()
{
    if (NetworkServerInfo.message_list_head)
    { // if there is anything in the list
        ServerMessage *target = NetworkServerInfo.message_list_head;
        if (target->next)
        { // the new list is not empty so we need to change only the head
            NetworkServerInfo.message_list_head = target->next;
        }
        else
        { // the new list is empty so we need to change the tail
            NetworkServerInfo.message_list_head = NetworkServerInfo.message_list_tail = NULL;
        }
        // now we delete the payload and the message struct
        if (target->payload)
            free(target->payload);
        free(target);
        return true;
    }
    else
        return false;
}

void NetworkHandleServerNotifications()
{
    if (NetworkServerInfo.message_list_head)
    { // if there is anything in the list
        switch (NetworkServerInfo.message_list_head->header.type)
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

void NetworkHandleSyncread(ServerMessage *syncread_message)
{
    // TODO: fill me
    UserName username;
    time_t timestamp;
    NetworkDeserializeMessageSyncread(syncread_message->header.payload_size, syncread_message->payload, &username, &timestamp);
    ChatHandleSyncread(username, timestamp);
}

bool NetworkAutoLogin(UserName username, Password password)
{
    if (NetworkSendMessageLogin(NetworkServerInfo.sockfd, NetworkListeningPort, username, password))
    {
        if (NetworkReceiveResponseFromServer(MESSAGE_RESPONSE))
        {
            NetworkDeleteOneFromServer(); // we expect just one MESSAGE_RESPONSE ok
            return true;
        }
        else
            return false;
    }
    else
        return false;
}