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
 - esc|exit\n\
   stop the server and exit\n");
            break;
        case COMMAND_LIST:
        {
            DebugTag("LIST");
            CLIPrintConnectedUsers();
        }
            break;
        case COMMAND_ESC:
            DebugTag("ESC");
            SaveAndExit(0);
            break;
        default:
            DebugTag("ERROR");
        }
}

void CLIPrintConnectedUsers()
{
    printf("Username - port\n");
    for(IndexEntry* i = IndexList; i; i = i->next)
    {
        if(i->timestamp_logout==0)
        {
            printf("%s - %d\n",i->user_dest.str,i->port);
        }
    }
}