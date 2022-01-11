#include "main.h"

int main(int argc, char **argv)
{
    if (argc < 1 || argc > 2)
    {
        printf("Usage: ./serv [port]\n");
        return -1;
    }
    Startup();
    uint16_t server_port = argc == 2 ? atoi(argv[1]) : DEFAULT_SERVER_PORT;
    printf("Server v%u started on port %u\n",VERSION, server_port);
    printf("Type \"help\" for a list of commands\n");
    NetworkMainLoop(server_port);
}