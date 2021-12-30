#include "cli.h"

CommandMode CLIMode = MODE_LOGIN;

void CLIHandleInput()
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
            CLISignup(dci);
            break;
        case COMMAND_IN:
            DebugTag("IN");
            CLILogin(dci);
            CLIMode = MODE_STANDARD;
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
            CLIMode = MODE_CHAT;
            break;
        case COMMAND_OUT:
            DebugTag("OUT");
            CLILogout(dci);
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

void CLISignup(DeviceCommandInfo dci)
{
    
}
void CLILogin(DeviceCommandInfo dci)
{

}
void CLIHanging(__attribute__((unused)) DeviceCommandInfo dci)
{

}
void CLIShow(DeviceCommandInfo dci)
{

}
void CLIChat(DeviceCommandInfo dci)
{

}
void CLILogout(__attribute__((unused)) DeviceCommandInfo dci)
{

}
void CLIChatQuit(__attribute__((unused)) DeviceCommandInfo dci)
{

}
void CLIChatUsers(__attribute__((unused)) DeviceCommandInfo dci)
{

}
void CLIChatAdd(DeviceCommandInfo dci)
{

}
void CLIChatFile(DeviceCommandInfo dci)
{

}
