#include "cli.h"

UserName CLIActiveUsername = {.str = {0}};
Password CLIActivePassword = {.str = {0}};

CommandMode CLIMode = MODE_LOGIN;

void CLIHandleInput()
{
    char first_char = getchar();
    ungetc(first_char, stdin);
    if (CLIMode != MODE_CHAT || first_char == '\\')
    {
        DeviceCommandInfo dci = CommandParserGetCommand(CLIMode);
        switch (dci.command)
        {
        case COMMAND_HELP:
            DebugTag("HELP");
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
            DebugTag("SIGNUP");
            CLISignup(dci);
            break;
        case COMMAND_IN:
            DebugTag("IN");
            CLILogin(dci);
            break;
        case COMMAND_HANGING:
            DebugTag("HANGING");
            CLIHanging(dci);
            break;
        case COMMAND_SHOW:
            DebugTag("SHOW");
            CLIShow(dci);
            break;
        case COMMAND_CHAT:
            DebugTag("CHAT");
            CLIChat(dci);
            break;
        case COMMAND_OUT:
            DebugTag("OUT");
            CLILogout(dci);
            break;
        case COMMAND_ESC:
            DebugTag("ESC");
            CLIEsc(dci);
            break;
        case COMMAND_CHAT_QUIT:
            DebugTag("CHAT->QUIT");
            CLIChatQuit(dci);
            break;
        case COMMAND_CHAT_USERS:
            DebugTag("CHAT->USERS");
            CLIChatUsers(dci);
            break;
        case COMMAND_CHAT_ADD:
            DebugTag("CHAT->ADD");
            CLIChatAdd(dci);
            break;
        case COMMAND_CHAT_FILE:
            DebugTag("CHAT->FILE");
            CLIChatFile(dci);
            break;
        default:
            DebugTag("ERROR");
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
    if(NetworkServerInfo.connected)
    {
        UserName username = CreateUserName(dci.args[0]);
        if(NetworkSendMessageHanging(NetworkServerInfo.sockfd,&username) && NetworkReceiveResponseFromServer(MESSAGE_RESPONSE))
        {
            bool done = false;
            while(NetworkServerInfo.message_list_head && !done)
            {
                switch(NetworkServerInfo.message_list_head->header.type)
                {
                    case MESSAGE_RESPONSE:
                        done = true;
                        printf("Successfully received message list\n");
                        break;
                    case MESSAGE_DATA:
                    {
                        bool is_file = NetworkMessageDataContainsFile(NetworkServerInfo.message_list_head->header.payload_size,NetworkServerInfo.message_list_head->payload);
                        if(is_file)
                            ChatSaveMessageFile(NetworkServerInfo.message_list_head->header.payload_size,NetworkServerInfo.message_list_head->payload);
                        else
                            ChatSaveMessageText(NetworkServerInfo.message_list_head->header.payload_size,NetworkServerInfo.message_list_head->payload);
                        break;
                    }
                    default:
                        printf("Message not expected\n");
                }
                NetworkDeleteOneFromServer();
            }
        }
    }
    else
    {
        printf("An error occurred trying to connect to the server\n");
    }
}
void CLIChat(DeviceCommandInfo dci)
{
    UserName target = CreateUserName(dci.args[0]);
    if(ChatAddTarget(target))
    {
        CLIMode = MODE_CHAT;
        ChatLoad(target);
        Chat* chat = ChatAddChat(target);
        for(ChatMessage* i = chat->head;i;i=i->next)
            ChatPrintMessage(*i,target);
    }
}
void CLILogout(__attribute__((unused)) DeviceCommandInfo dci)
{
    if (NetworkServerInfo.connected)
    {
        if (NetworkSendMessageLogout(NetworkServerInfo.sockfd) && NetworkReceiveResponseFromServer(MESSAGE_RESPONSE))
        {
            NetworkDeleteOneFromServer();
            printf("Successfully logged out\n");
            memset(CLIActiveUsername.str, 0, USERNAME_MAX_LENGTH + 1);
            memset(CLIActivePassword.str, 0, PASSWORD_MAX_LENGTH + 1);
            CLIMode = MODE_LOGIN;
            NetworkServerInfo.address.sin_port = 0; // prevent auto reconnect
        }
        else
        {
            printf("An error occurred during the logout procedure\n");
        }
    }
    else
    {
        printf("An error occurred trying to connect to the server\n");
    }
}
void CLIEsc(__attribute__((unused)) DeviceCommandInfo dci)
{
    SaveAndExit(0);
}
void CLIChatQuit(__attribute__((unused)) DeviceCommandInfo dci)
{
    CLIMode = MODE_STANDARD;
    ChatQuit();
}
void CLIChatUsers(__attribute__((unused)) DeviceCommandInfo dci)
{
    // TODO: fill me
}
void CLIChatAdd(DeviceCommandInfo dci)
{
    UserName target = CreateUserName(dci.args[0]);
    if(ChatAddTarget(target))
    {
        printf("%s added to the chat\n",target.str);
    }
}
void CLIChatFile(DeviceCommandInfo dci)
{
    // TODO: fill me
}
