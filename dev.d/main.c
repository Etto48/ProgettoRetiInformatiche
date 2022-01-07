#include "main.h"

int main(int argc, char **argv)
{
    if (argc < 2 || argc > 3)
    {
        printf("Usage: ./dev <listening port> [server ip]\n");
        return -1;
    }
    uint16_t device_port = atoi(argv[1]);
    Startup();
    if(argc == 3)
    {
        NetworkServerAddress = inet_addr(argv[2]);
    }
    else
    {
        NetworkServerAddress = inet_addr(SERVER_DEFAULT_ADDRESS);
    }
    printf("Device started on port %u\n", device_port);
    printf("Type \"help\" for a list of commands\n");
    NetworkMainLoop(device_port);
}