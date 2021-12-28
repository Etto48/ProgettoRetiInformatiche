#include "main.h"

int main(int argc, char **argv)
{
    if (argc != 2)
    {
        printf("Usage: ./dev <listening port>\n");
        return -1;
    }
    Startup();
    uint16_t device_port = atoi(argv[1]);
    printf("Device started on port %u\n",device_port);
    NetworkMainLoop(device_port);
}
/*
 * NOTE: when a chat is initiated, if the device can't connect to a selected client 
 * it must connect to the server and send to it the messages 
 * periodically it must request a hanging messages list from the server to update the message list
 */