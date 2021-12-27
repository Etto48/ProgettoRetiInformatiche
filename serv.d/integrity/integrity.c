#include "integrity.h"

void sigterm_handler(int sig __attribute__((unused)))
{
    SaveAndExit(0);
}

void Startup()
{
    AuthLoad(AUTH_FILE);
    RelayLoad(RELAY_FILE);
    signal(SIGTERM,sigterm_handler);
}

void SaveAndExit(int status)
{
    AuthSave(AUTH_FILE);
    RelaySave(RELAY_FILE);
    exit(status);
}