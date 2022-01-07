#include "cli.h"

UserName CLIActiveUsername = {.str = {0}};
Password CLIActivePassword = {.str = {0}};

CommandMode CLIMode = MODE_LOGIN;

void CLIHandleInput()
{
    char first_char = getchar();
    char second_char = getchar();
    // you can escape a command to send something that begins with "\"
    ungetc(second_char, stdin);
    if (!(first_char == '\\' && second_char == '\\'))
        ungetc(first_char, stdin);
    if (CLIMode != MODE_CHAT || (first_char == '\\' && second_char != '\\'))
    {
        DeviceCommandInfo dci = CommandParserGetCommand(CLIMode);
        switch (dci.command)
        {
        case COMMAND_HELP:
            // DebugTag("HELP");
            printf("\
Available commands:\n\
 - help\n\
   (This command is available before and after the login) shows a command list + instructions\n\
\n\
 - signup <server port> <username> <password>\n\
   request the creation of an account to the main server located on port <server port>\n\
\n\
 - [log]in <server port> <username> <password>\n\
   request a login to the main server located on port <server port>\n\
\n\
 - hanging\n\
   request a list of users who sent you a message while you was offline (This command is only available after the login)\n\
\n\
 - show <username>\n\
   request a list of hanging messages from <username> (This command is only available after the login)\n\
\n\
 - chat <username>\n\
   you can start a chat with <username> with this command\n\
   once in a chat you can use the following commands\n\
   prefixing them with a \"\\\" (This command is only available after the login)\n\
   > q\n\
     close the chat\n\
   > u\n\
     list online users available for chat\n\
   > a <username>\n\
     add a user to the chat\n\
   > f <filename>\n\
     share a file with the chat\n\
\n\
 - [log]out\n\
   logs you out and closes the program (This command is only available after the login)\n\
\n\
 - esc|exit\n\
   saves everything and closes the program\n");
            break;
        case COMMAND_SIGNUP:
            // DebugTag("SIGNUP");
            CLISignup(dci);
            break;
        case COMMAND_IN:
            // DebugTag("IN");
            CLILogin(dci);
            break;
        case COMMAND_HANGING:
            // DebugTag("HANGING");
            CLIHanging(dci);
            break;
        case COMMAND_SHOW:
            // DebugTag("SHOW");
            CLIShow(dci);
            break;
        case COMMAND_CHAT:
            // DebugTag("CHAT");
            CLIChat(dci);
            break;
        case COMMAND_OUT:
            // DebugTag("OUT");
            CLILogout(dci);
            break;
        case COMMAND_ESC:
            // DebugTag("ESC");
            CLIEsc(dci);
            break;
        case COMMAND_CHAT_QUIT:
            // DebugTag("CHAT->QUIT");
            CLIChatQuit(dci);
            break;
        case COMMAND_CHAT_USERS:
            // DebugTag("CHAT->USERS");
            CLIChatUsers(dci);
            break;
        case COMMAND_CHAT_ADD:
            // DebugTag("CHAT->ADD");
            CLIChatAdd(dci);
            break;
        case COMMAND_CHAT_FILE:
            // DebugTag("CHAT->FILE");
            CLIChatFile(dci);
            break;
        default:
            // DebugTag("ERROR");
            printf("The command is not valid");
            if (CLIMode == MODE_LOGIN)
                printf(" or you need to login first\n");
            else
                printf("\n");
        }
    }
    else
    {
        ChatWrite();
    }
}

void CLISignup(DeviceCommandInfo dci)
{
    if (!NetworkStartServerConnection((uint16_t)atoi(dci.args[0])))
    {
        printf("An error occurred trying to connect to the server\n");
        return;
    }
    UserName username = CreateUserName(dci.args[1]);
    Password password = CreatePassword(dci.args[2]);
    if (NetworkSendMessageSignup(NetworkServerInfo.sockfd, username, password) && NetworkReceiveResponseFromServer(MESSAGE_RESPONSE))
    {
        NetworkDeleteOneFromServer(); // we expect just one MESSAGE_RESPONSE ok
        printf("Successfully signed up\n");
    }
    else
    {
        printf("An error occurred during the signup process\n");
    }
}
void CLILogin(DeviceCommandInfo dci)
{
    if (!NetworkStartServerConnection((uint16_t)atoi(dci.args[0])))
    {
        printf("An error occurred trying to connect to the server\n");
        return;
    }

    UserName username = CreateUserName(dci.args[1]);
    Password password = CreatePassword(dci.args[2]);
    if (NetworkAutoLogin(username, password))
    {
        printf("Successfully logged in\n");
        CLIMode = MODE_STANDARD; // we are logged in
        CLIActiveUsername = username;
        CLIActivePassword = password;
    }
    else
        printf("An error occurred during the login procedure\n");
}
void CLIHanging(__attribute__((unused)) DeviceCommandInfo dci)
{
    if (NetworkServerInfo.connected)
    {
        if (NetworkSendMessageHanging(NetworkServerInfo.sockfd, NULL) && NetworkReceiveResponseFromServer(MESSAGE_RESPONSE))
        {
            bool done = false;
            while (NetworkServerInfo.message_list_head && !done)
            {
                switch (NetworkServerInfo.message_list_head->header.type)
                {
                case MESSAGE_RESPONSE:
                    done = true;
                    printf("Successfully received hanging list\n");
                    break;
                case MESSAGE_HANGING:
                {
                    UserName username;
                    NetworkDeserializeMessageHanging(
                        NetworkServerInfo.message_list_head->header.payload_size,
                        NetworkServerInfo.message_list_head->payload,
                        &username);
                    printf("- %s\n", username.str);
                }
                break;
                default:
                    printf("Message not expected\n");
                    break;
                }
                NetworkDeleteOneFromServer();
            }
        }
        else
        {
            printf("An error occurred while receiving hanging users\n");
        }
    }
    else
    {
        printf("An error occurred trying to connect to the server\n");
    }
}
void CLIShow(DeviceCommandInfo dci)
{
    if (NetworkServerInfo.connected)
    {
        UserName username = CreateUserName(dci.args[0]);
        ChatLoad(username);
        Chat *chat = ChatAddChat(username);
        ChatMessage *latest = chat->tail;
        if (ChatSyncWith(username))
        {
            for (ChatMessage *i = latest ? latest->next : chat->head; i; i = i->next)
                ChatPrintMessage(*i, username);
            printf("Successfully received message list\n");
        }
        else
            printf("An error occurred while receiving hanging messages\n");
    }
    else
    {
        printf("An error occurred trying to connect to the server\n");
    }
}
void CLIChat(DeviceCommandInfo dci)
{
    UserName target = CreateUserName(dci.args[0]);
    if (strncmp(target.str, CLIActiveUsername.str, USERNAME_MAX_LENGTH) == 0)
    {
        printf("You can't chat with yourself\n");
    }
    else if (ChatAddTarget(target))
    {
        CLIMode = MODE_CHAT;
        if (!ChatSyncWith(target))
        {
            printf("This chat might not be up to date\n");
        }
        Chat *chat = ChatAddChat(target);
        for (ChatMessage *i = chat->head; i; i = i->next)
            ChatPrintMessage(*i, target);
    }
    else
        printf("%s is not a valid username\n", target.str);
}
void CLILogout(__attribute__((unused)) DeviceCommandInfo dci)
{
    if (NetworkServerInfo.connected)
    {
        if (NetworkSendMessageLogout(NetworkServerInfo.sockfd) && NetworkReceiveResponseFromServer(MESSAGE_RESPONSE))
        {
            NetworkDeleteOneFromServer();

            Save();          // we must save the chat until we know the username
            FreeResources(); // we deallocate every chat structure used as it can cause problems if we login with another user

            memset(CLIActiveUsername.str, 0, USERNAME_MAX_LENGTH + 1);
            memset(CLIActivePassword.str, 0, PASSWORD_MAX_LENGTH + 1);
            CLIMode = MODE_LOGIN;
            NetworkServerInfo.address.sin_port = 0;              // prevent auto reconnect
            for (size_t i = 3; i < NETWORK_MAX_CONNECTIONS; i++) // disconnect from every peer
            {
                if (NetworkConnectedDevices[i].sockfd)
                    NetworkDeleteConnection(i);
            }
            printf("Successfully logged out\n");
        }
        else
            printf("An error occurred during the logout procedure\n");
    }
    else
        printf("An error occurred trying to connect to the server\n");
}
void CLIEsc(__attribute__((unused)) DeviceCommandInfo dci)
{
    SaveAndExit(0);
}
void CLIChatQuit(__attribute__((unused)) DeviceCommandInfo dci)
{
    CLIMode = MODE_STANDARD;
    ChatQuit();
    printf("You closed the chat\n");
}
void CLIChatUsers(__attribute__((unused)) DeviceCommandInfo dci)
{
    if (NetworkServerInfo.connected)
    {
        printf("Users (+ online | - offline):\n");
        struct dirent *dir;
        DIR *d = opendir(ChatGetDirname());
        if (d)
        {
            while ((dir = readdir(d)))
            {
                *strchr(dir->d_name, '.') = '\0'; // we don't want to display ".chat"
                if (strlen(dir->d_name))
                {
                    if (NetworkSendMessageUserinfoReq(NetworkServerInfo.sockfd, CreateUserName(dir->d_name)) && NetworkReceiveResponseFromServer(MESSAGE_USERINFO_RES))
                    {
                        uint32_t ip;
                        uint16_t port;
                        NetworkDeserializeMessageUserinfoRes(NetworkServerInfo.message_list_head->header.payload_size,NetworkServerInfo.message_list_head->payload,&ip,&port);
                        NetworkDeleteOneFromServer();
                        if(port)
                            printf("+ %s (%u)\n", dir->d_name,port);
                        else
                            printf("- %s (%u)\n", dir->d_name,port);
                    }
                }
            }
            closedir(d);
        }
    }
    else
        printf("An error occurred trying to connect to the server\n");
}
void CLIChatAdd(DeviceCommandInfo dci)
{
    UserName target = CreateUserName(dci.args[0]);
    if (strncmp(target.str, CLIActiveUsername.str, USERNAME_MAX_LENGTH) == 0)
    {
        printf("You can't chat with yourself\n");
    }
    else if (ChatTargetFind(target))
    {
        printf("You're already chatting with %s", target.str);
    }
    else if (ChatAddTarget(target))
    {
        printf("%s added to the chat\n", target.str);
    }
    else
    {
        printf("%s is not a valid username\n", target.str);
    }
}
void CLIChatFile(DeviceCommandInfo dci)
{
    char *filename = dci.args[0];
    struct stat st;
    int fd = -1;
    if (stat(filename, &st) < 0 || (fd = open(filename, O_RDONLY)) < 0)
    {
        printf("Could not open %s\n", filename);
        return;
    }
    uint8_t *file_buffer = (uint8_t *)malloc(st.st_size);
    read(fd, file_buffer, st.st_size);
    close(fd);
    time_t timestamp = time(NULL);
    for (ChatTarget *i = ChatTargetList; i; i = i->next)
    {
        NetworkSendMessageDataFileBuffer(
            (i->sockfd < 0 ? NetworkServerInfo.sockfd : i->sockfd),
            CLIActiveUsername, i->dst, timestamp, filename + ToolsBasename(filename), st.st_size, file_buffer);
        ChatAddMessage(i->dst, CHAT_MESSAGE_SENT, i->sockfd >= 0, CHAT_MESSAGE_FILE, timestamp, filename);
    }
    ChatPrintMessage(*(ChatFind(ChatTargetList->dst)->tail), ChatTargetList->dst);
    free(file_buffer);
}
