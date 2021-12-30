#include "command_parser.h"

ServerCommand CommandParserGetCommand()
{
    char plain_text[INPUT_LEN];
    char command[INPUT_LEN];
    char args[MAX_ARG_NUM][INPUT_LEN];
    int argc = CommandInputSplitCommand(plain_text, command,args,MAX_ARG_NUM);

    if(strcmp("help",command)==0)
    {
        if(argc==0)
            return COMMAND_HELP;
        else
            printf("Usage: help\n");
    }
    else if(strcmp("list",command)==0)
    {
        if(argc==0)
            return COMMAND_LIST;
        else
            printf("Usage: list\n");
    }
    else if(strcmp("esc",command)==0||strcmp("exit",command)==0)
    {
        if(argc==0)
            return COMMAND_ESC;
        else
            printf("Usage: esc|exit\n");
    }
    else 
    {
        printf("Not a valid command, type \"help\" to get a list of valid commands.\n");
        return COMMAND_ERROR;
    }
    return COMMAND_ERROR;
}