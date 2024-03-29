#include "integrity.h"

void sigterm_handler(int sig __attribute__((unused)))
{
    NetworkShutdownRequested = true;
}

void Startup()
{
    signal(SIGTERM, sigterm_handler);
    signal(SIGINT, sigterm_handler);
}

void Save()
{
    bool ok = true;
    if (CLIMode != MODE_LOGIN)
        ok = ok && ChatSave();
    if (!ok)
        printf("An errror occurred saving files\n");
}

void FreeResources()
{
    ChatFree();
}

void SaveAndExit(int status)
{
    Save();
    FreeResources();
    exit(status);
}