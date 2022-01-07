#include "main.h"

int main(int argc, char **argv)
{
    if (argc != 2)
    {
        printf("Usage: ./dev <listening port>\n");
        return -1;
    }
    uint16_t device_port = atoi(argv[1]);
    Startup();
    printf("Device started on port %u\n", device_port);
    printf("Type \"help\" for a list of commands\n");
    NetworkMainLoop(device_port);
}