#include "main.h"

int main(int argc, char **argv)
{
#ifdef SPECIFICATION_STRICT
    if (argc != 2)
    {
        printf("Usage: ./dev <listening port>\n");
#else
    if (argc < 2 || argc > 3)
    {
        printf("Usage: ./dev <listening port> [server hostname|ip]\n");
#endif
        return -1;
    }
    uint16_t device_port = atoi(argv[1]);
    Startup();
    struct hostent *server_host;
    if (argc == 3 && (server_host = gethostbyname(argv[2])))
        NetworkServerAddress = *(uint32_t *)(server_host->h_addr_list[0]);
    else
        NetworkServerAddress = inet_addr(SERVER_DEFAULT_ADDRESS);
    printf("Device v%u started on port %u\n",VERSION, device_port);
    printf("Type \"help\" for a list of commands\n");
    NetworkMainLoop(device_port);
}