#include "network.h"

void NetworkServerMainLoop(uint16_t port)
{
    int server_socket = socket(AF_INET,SOCK_STREAM,0);
    if(server_socket<0)
    {
        dbgerror("Error creating a socket for the server");
        exit(-1);
    }
    int yes = 1;
    if(setsockopt(server_socket,SOL_SOCKET,SO_REUSEADDR,&yes,sizeof(yes))<0)
    {
        dbgerror("Error setting SO_REUSEADDR");
        exit(-1);
    }
    struct sockaddr_in server_address;
    memset(&server_address,0,sizeof(server_address));   
    server_address.sin_family = AF_INET;
    server_address.sin_port = htons(port);
    server_address.sin_addr.s_addr = INADDR_ANY;
    if(bind(server_socket,(struct sockaddr *)&server_address,sizeof(struct sockaddr_in))<0)
    {
        dbgerror("Error binding socket");
        exit(-1);
    }

    if(listen(server_socket,NETWORK_MAX_CONNECTIONS)<0)
    {
        dbgerror("Error listening on a socket");
        exit(-1);
    }

    fd_set master_set, slave_set;
    int max_fdi = server_socket + 1;
    FD_SET(server_socket,&master_set);
    struct timeval select_timer;
    memset(&select_timer,0,sizeof(struct timeval));

    while(true)
    {
        int ready_fd_count = 0;
        while(ready_fd_count == 0)
        {
            slave_set = master_set;
            ready_fd_count = select(max_fdi,&slave_set,NULL,NULL,&select_timer);
            if(ready_fd_count<0)
            {
                dbgerror("Error selecting available FDs");
                SaveAndExit(-1);
            }
        }
        for(int target_fd = 0; target_fd < max_fdi; target_fd++)
        {
            if(FD_ISSET(target_fd,&slave_set))
            {
                if(target_fd == server_socket)
                {//accept connection

                }
                else
                {//handle connection

                }
            }
        }

    }
}