#include "cli.h"

void CLIHandleInput()
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