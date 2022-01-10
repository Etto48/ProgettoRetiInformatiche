#include "cli.h"

void CLIHandleInput()
{
    ServerCommand sc = CommandParserGetCommand();
    switch (sc)
    {
    case COMMAND_HELP:
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
        CLIPrintConnectedUsers();
        break;
    case COMMAND_ESC:
        SaveAndExit(0);
        break;
    default:
        break;
    }
}

void CLIPrintConnectedUsers()
{
    int column_size = 20;
    char *labels[] = {"Username", "login time", "address"};
    printf(" %*s | %*s | %*s\n", -column_size, labels[0], -column_size, labels[1], -column_size, labels[2]);
    printf("----------------------+----------------------+----------------------\n");
    for (IndexEntry *i = IndexList; i; i = i->next)
    {
        if (i->timestamp_logout == 0)
        {
            char time_buf[21];
            struct tm login_time = *localtime(&i->timestamp_login);
            snprintf(time_buf, 21, "%02u/%02u/%04u %02u:%02u:%02u",
                     login_time.tm_mday % 100,
                     (login_time.tm_mon + 1) % 100,
                     (login_time.tm_year + 1900) % 10000,
                     login_time.tm_hour % 100,
                     login_time.tm_min % 100,
                     login_time.tm_sec % 100);
            char ip_buf[16];
            char net_buf[22];
            uint32_t ip = htonl(i->ip);
            inet_ntop(AF_INET, &ip, ip_buf, 16);
            snprintf(net_buf, 22, "%s:%d", ip_buf, i->port);
            printf(" %*s | %*s | %*s\n",
                   -column_size,
                   i->user_dest.str,
                   -column_size,
                   time_buf,
                   -column_size,
                   net_buf);
        }
    }
}