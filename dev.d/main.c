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