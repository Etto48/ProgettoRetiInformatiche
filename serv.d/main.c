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
    printf("Server started on port %u\n",server_port);
    NetworkServerMainLoop(server_port);
}