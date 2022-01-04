#include "integrity.h"

void sigterm_handler(int sig __attribute__((unused)))
{
    SaveAndExit(0);
}

void Startup()
{
    signal(SIGTERM,sigterm_handler);
}

void Save()
{
    ChatSave();
}

void SaveAndExit(int status)
{
    Save();
    exit(status);
}