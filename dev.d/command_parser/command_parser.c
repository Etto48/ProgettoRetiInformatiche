#include "command_parser.h"

DeviceCommandInfo CommandParserGetCommand(CommandMode mode)
{
    char plain_text[INPUT_LEN];
    char command[INPUT_LEN];
    char args[MAX_ARG_NUM][INPUT_LEN];
    int argc = CommandInputSplitCommand(plain_text, command, args, MAX_ARG_NUM);

    DeviceCommandInfo ret;
    ret.argc = argc;
    for (unsigned int i = 0; i < MAX_ARG_NUM - 1; i++)
    {
        memcpy(ret.args[i], args[i], INPUT_LEN);
    }
    ret.command = COMMAND_ERROR;

    bool usage_prompt = false;

    switch (mode)
    {
    case MODE_LOGIN:
        if (strcmp("help", command) == 0)
        {
            if (argc == 0)
                ret.command = COMMAND_HELP;
            else
            {
                usage_prompt = true;
                printf("Usage: help\n");
            }
        }
        else if (strcmp("esc", command) == 0 || strcmp("exit", command) == 0)
        {
            if (argc == 0)
                ret.command = COMMAND_ESC;
            else
            {
                usage_prompt = true;
                printf("Usage: esc|exit\n");
            }
        }
        else if (strcmp("signup", command) == 0)
        {
            if (argc == 3)
                ret.command = COMMAND_SIGNUP;
            else
            {
                usage_prompt = true;
                printf("Usage: signup <username> <password>\n");
            }
        }
        else if (strcmp("in", command) == 0 || strcmp("login", command) == 0)
        {
            if (argc == 3)
                ret.command = COMMAND_IN;
            else
            {
                usage_prompt = true;
                printf("Usage: [log]in <server port> <username> <password>\n");
            }
        }
        break;
    case MODE_STANDARD:
        if (strcmp("help", command) == 0)
        {
            if (argc == 0)
                ret.command = COMMAND_HELP;
            else
            {
                usage_prompt = true;
                printf("Usage: help\n");
            }
        }
        else if (strcmp("esc", command) == 0 || strcmp("exit", command) == 0)
        {
            if (argc == 0)
                ret.command = COMMAND_ESC;
            else
            {
                usage_prompt = true;
                printf("Usage: esc|exit\n");
            }
        }
        else if (strcmp("hanging", command) == 0)
        {
            if (argc == 0)
                ret.command = COMMAND_HANGING;
            else
            {
                usage_prompt = true;
                printf("Usage: hanging\n");
            }
        }
        else if (strcmp("show", command) == 0)
        {
            if (argc == 1)
                ret.command = COMMAND_SHOW;
            else
            {
                usage_prompt = true;
                printf("Usage: show <username>\n");
            }
        }
        else if (strcmp("rmchat", command) == 0)
        {
            if (argc == 1)
                ret.command = COMMAND_RMCHAT;
            else
            {
                usage_prompt = true;
                printf("Usage: rmchat <username>\n");
            }
        }
        else if (strcmp("users", command) == 0)
        {
            if (argc == 0)
                ret.command = COMMAND_USERS;
            else
            {
                usage_prompt = true;
                printf("Usage: users\n");
            }
        }
        else if (strcmp("chat", command) == 0)
        {
            if (argc == 1)
                ret.command = COMMAND_CHAT;
            else
            {
                usage_prompt = true;
                printf("Usage: chat <username>\n");
            }
        }
        else if (strcmp("out", command) == 0 || strcmp("logout", command) == 0)
        {
            if (argc == 0)
                ret.command = COMMAND_OUT;
            else
            {
                usage_prompt = true;
                printf("Usage: [log]out\n");
            }
        }
        break;
    case MODE_CHAT:
        if (strcmp("\\q", command) == 0)
        {
            if (argc == 0)
                ret.command = COMMAND_CHAT_QUIT;
            else
            {
                usage_prompt = true;
                printf("Usage: \\q\n");
            }
        }
        else if (strcmp("\\u", command) == 0)
        {
            if (argc == 0)
                ret.command = COMMAND_CHAT_USERS;
            else
            {
                usage_prompt = true;
                printf("Usage: \\u\n");
            }
        }
        else if (strcmp("\\a", command) == 0)
        {
            if (argc == 1)
                ret.command = COMMAND_CHAT_ADD;
            else
            {
                usage_prompt = true;
                printf("Usage: \\a <username>\n");
            }
        }
        else if (strcmp("\\f", command) == 0)
        {
            if (argc == 1)
                ret.command = COMMAND_CHAT_FILE;
            else
            {
                usage_prompt = true;
                printf("Usage: \\f filename\n");
            }
        }
        else if (strcmp("\\h", command) == 0)
        {
            if (argc == 0)
                ret.command = COMMAND_HELP;
            else
            {
                usage_prompt = true;
                printf("Usage: \\h\n");
            }
        }
        break;
    }
    if (ret.command == COMMAND_ERROR && !usage_prompt)
        printf("Not a valid command, type \"help\" to get a list of commands\n");
    return ret;
}