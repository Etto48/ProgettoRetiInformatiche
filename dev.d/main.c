#include "main.h"

int main(int argc, char **argv)
{
    if (argc != 2)
    {
        printf("Usage: ./dev <listening port>\n");
        return -1;
    }
    uint16_t device_port = atoi(argv[1]);
    printf("Device started on port %u\n",device_port);
    CommandMode mode = MODE_LOGIN;
    while (true)
    {
        DeviceCommandInfo dci = CommandParserGetCommand(mode);
        switch (dci.command)
        {
        case COMMAND_HELP:
            DebugTag("HELP");
            printf("\
Available commands:\n\
 - help\n\
   (This command is available before and after the login) shows a command list + instructions\n\
\n\
 - signup <username> <password>\n\
   request the creation of an account to the main server\n\
\n\
 - in <server port> <username> <password>\n\
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
 - out\n\
   logs you out and closes the program (This command is only available after the login)\n");
            break;
        case COMMAND_SIGNUP:
            DebugTag("SIGNUP");
            break;
        case COMMAND_IN:
            DebugTag("IN");
            mode = MODE_STANDARD;
            break;
        case COMMAND_HANGING:
            DebugTag("HANGING");
            break;
        case COMMAND_SHOW:
            DebugTag("SHOW");
            break;
        case COMMAND_CHAT:
            DebugTag("CHAT");
            break;
        case COMMAND_OUT:
            DebugTag("OUT");
            break;
        case COMMAND_CHAT_QUIT:
            DebugTag("CHAT->QUIT");
            break;
        case COMMAND_CHAT_USERS:
            DebugTag("CHAT->USERS");
            break;
        case COMMAND_CHAT_ADD:
            DebugTag("CHAT->ADD");
            break;
        case COMMAND_CHAT_FILE:
            DebugTag("CHAT->FILE");
            break;
        default:
            DebugTag("ERROR");
        }
    }
}
/*
 * when a chat is initiated, if the device can't connect to a selected client 
 * it must connect to the server and send to it the messages 
 * periodically it must request a hanging messages list from the server to update the message list
 */