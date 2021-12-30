#include "cli.h"

UserName CLIActiveUsername = {.str = {0}};
Password CLIActivePassword = {.str = {0}};

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

void CLISignup(DeviceCommandInfo dci)
{
    if(!NetworkStartServerConnection((uint16_t)atoi(dci.args[0])))
    {
        printf("An error occurred trying to connect to the server\n");
        return;
    }
    UserName username;
    Password password;
    memset(username.str,0,USERNAME_MAX_LENGTH+1);
    strncpy(username.str,dci.args[1],USERNAME_MAX_LENGTH);
    /* we hash the password for a little bit of security, please run this program on a secure network and use a strong password,
     * the hash can be replayed, the service is not really secure without SSL
     */
    memset(password.str,0,PASSWORD_MAX_LENGTH+1);
    calc_sha_256((uint8_t*)password.str,dci.args[2],strlen(dci.args[2]));
    NetworkSendMessageSignup(NetworkServerInfo.sockfd,username,password);
    if(NetworkReceiveResponseFromServer())
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
    if(!NetworkStartServerConnection((uint16_t)atoi(dci.args[0])))
    {
        printf("An error occurred trying to connect to the server\n");
        return;
    }
    UserName username;
    Password password;
    memset(username.str,0,USERNAME_MAX_LENGTH+1);
    strncpy(username.str,dci.args[1],USERNAME_MAX_LENGTH);
    /* we hash the password for a little bit of security, please run this program on a secure network and use a strong password,
     * the hash can be replayed, the service is not really secure without SSL
     */
    memset(password.str,0,PASSWORD_MAX_LENGTH+1);
    calc_sha_256((uint8_t*)password.str,dci.args[2],strlen(dci.args[2]));
    
    if(NetworkAutoLogin(username,password))
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
    //TODO: fill me
}
void CLIShow(DeviceCommandInfo dci)
{
  //TODO: fill me
}
void CLIChat(DeviceCommandInfo dci)
{
    //TODO: fill me
    //TODO: set CLIMode = MODE_CHAT; if success
}
void CLILogout(__attribute__((unused)) DeviceCommandInfo dci)
{
    if(NetworkServerInfo.connected)
    {
        NetworkSendMessageLogout(NetworkServerInfo.sockfd);
        if(NetworkReceiveResponseFromServer())
        {
            NetworkDeleteOneFromServer();
            printf("Successfully logged out\n");
            memset(CLIActiveUsername.str,0,USERNAME_MAX_LENGTH+1);
            memset(CLIActivePassword.str,0,PASSWORD_MAX_LENGTH+1);
            CLIMode = MODE_LOGIN;
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
    //TODO: fill me
}
void CLIChatUsers(__attribute__((unused)) DeviceCommandInfo dci)
{
    //TODO: fill me
}
void CLIChatAdd(DeviceCommandInfo dci)
{
    //TODO: fill me
}
void CLIChatFile(DeviceCommandInfo dci)
{
    //TODO: fill me
}
