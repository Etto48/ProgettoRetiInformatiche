#include "integrity.h"

void sigterm_handler(int sig __attribute__((unused)))
{
    NetworkShutdownRequested = true;
}

void Startup()
{
    AuthLoad(AUTH_FILE);
    RelayLoad(RELAY_FILE);
    RelaySyncreadLoad(RELAY_SYNCREAD_FILE);
    signal(SIGTERM, sigterm_handler);
    signal(SIGINT, sigterm_handler);
}

void Save()
{
    bool ok = true;
    ok = ok && AuthSave(AUTH_FILE);
    ok = ok && RelaySave(RELAY_FILE);
    ok = ok && RelaySyncreadSave(RELAY_SYNCREAD_FILE);
    if (!ok)
        printf("An errror occurred saving files\n");
}

void FreeResources()
{
    AuthFree();
    IndexFree();
    RelayFree();
    RelaySyncreadFree();
}

void SaveAndExit(int status)
{
    Save();
    FreeResources();
    exit(status);
}