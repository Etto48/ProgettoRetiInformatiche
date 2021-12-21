#include "main.h"

int main(int argc, char **argv)
{
    if (argc < 1 || argc > 2)
    {
        printf("Usage: ./serv [port]\n");
        return -1;
    }
    Startup();
    uint16_t server_port = argc == 2 ? atoi(argv[1]) : DEFAULT_SERVER_PORT;
    printf("Server started on port %u\n",server_port);
    while (true)
    {
        ServerCommand sc = CommandParserGetCommand();
        switch (sc)
        {
        case COMMAND_HELP:
            DebugTag("HELP");
            printf("\
Available commands:\n\
 - help\n\
   show this message\n\
\n\
 - list\n\
   show a list of connected users\n\
\n\
 - esc\n\
   stop the server and exit\n");
            break;
        case COMMAND_LIST:
            DebugTag("LIST");
            break;
        case COMMAND_ESC:
            DebugTag("ESC");
            SaveAndExit(0);
            break;
        default:
            DebugTag("ERROR");
        }
    }
}