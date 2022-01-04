#include "integrity.h"

void sigterm_handler(int sig __attribute__((unused)))
{
    SaveAndExit(0);
}

void Startup()
{
    AuthLoad(AUTH_FILE);
    RelayLoad(RELAY_FILE);
    RelaySyncreadLoad(RELAY_SYNCREAD_FILE);
    signal(SIGTERM,sigterm_handler);
}

void Save()
{
    AuthSave(AUTH_FILE);
    RelaySave(RELAY_FILE);
    RelaySyncreadSave(RELAY_SYNCREAD_FILE);
}

void SaveAndExit(int status)
{
    Save();
    exit(status);
}