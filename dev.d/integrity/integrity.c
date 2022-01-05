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
    bool ok = true;
    ok = ok && ChatSave();
    if(!ok)
        printf("An errror occurred saving files\n");
}

void SaveAndExit(int status)
{
    Save();
    exit(status);
}