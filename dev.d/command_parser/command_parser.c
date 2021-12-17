#include "command_parser.h"

DeviceCommandInfo CommandParserGetCommand(CommandMode mode)
{
    char plain_text[INPUT_LEN];
    char command[INPUT_LEN];
    char args[MAX_ARG_NUM][INPUT_LEN];
    int argc = CommandInputSplitCommand(plain_text, command, args, MAX_ARG_NUM);

    DeviceCommandInfo ret;
    ret.argc = argc;
    for (unsigned int i = 0; i < MAX_ARG_NUM-1; i++)
    {
        memcpy(ret.args[i], args[i], INPUT_LEN);
    }
    ret.command = COMMAND_ERROR;

    switch (mode)
    {
    case MODE_LOGIN:
        if (strcmp("help", command) == 0)
        {
            if (argc == 0)
            {
                ret.command = COMMAND_HELP;
            }
            else
            {
                printf("Usage: help\n");
            }
        }
        else if (strcmp("signup", command) == 0)
        {
            if (argc == 2)
            {
                ret.command = COMMAND_SIGNUP;
            }
            else
            {
                printf("Usage: signup <username> <password>\n");
            }
        }
        else if (strcmp("in", command) == 0)
        {
            if (argc == 3)
            {
                ret.command = COMMAND_IN;
            }
            else
            {
                printf("Usage: in <server port> <username> <password>\n");
            }
        }
        break;
    case MODE_STANDARD:
        if (strcmp("help", command) == 0)
        {
            if (argc == 0)
            {
                ret.command = COMMAND_HELP;
            }
            else
            {
                printf("Usage: help\n");
            }
        }
        else if (strcmp("hanging", command) == 0)
        {
            if (argc == 0)
            {
                ret.command = COMMAND_HANGING;
            }
            else
            {
                printf("Usage: hanging\n");
            }
        }
        else if (strcmp("show", command) == 0)
        {
            if (argc == 1)
            {
                ret.command = COMMAND_SHOW;
            }
            else
            {
                printf("Usage: show <username>\n");
            }
        }
        else if (strcmp("chat", command) == 0)
        {
            if (argc == 1)
            {
                ret.command = COMMAND_CHAT;
            }
            else
            {
                printf("Usage: chat <username>\n");
            }
        }
        else if (strcmp("out", command) == 0)
        {
            if (argc == 0)
            {
                ret.command = COMMAND_OUT;
            }
            else
            {
                printf("Usage: out\n");
            }
        }
        break;
    case MODE_CHAT:
        if (strcmp("\\q", command) == 0)
        {
            if (argc == 0)
            {
                ret.command = COMMAND_CHAT_QUIT;
            }
            else
            {
                printf("Usage: \\q\n");
            }
        }
        else if (strcmp("\\u", command) == 0)
        {
            if (argc == 0)
            {
                ret.command = COMMAND_CHAT_USERS;
            }
            else
            {
                printf("Usage: \\u\n");
            }
        }
        else if (strcmp("\\a", command) == 0)
        {
            if (argc == 1)
            {
                ret.command = COMMAND_CHAT_ADD;
            }
            else
            {
                printf("Usage: \\a <username>\n");
            }
        }
        else if (strcmp("\\f", command) == 0)
        {
            if (argc == 1)
            {
                ret.command = COMMAND_CHAT_FILE;
            }
            else
            {
                printf("Usage: \\f filename\n");
            }
        }
        break;
    }
    return ret;
}