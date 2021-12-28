#include "integrity.h"

void sigterm_handler(int sig __attribute__((unused)))
{
    SaveAndExit(0);
}

void Startup()
{
    signal(SIGTERM,sigterm_handler);
}

void SaveAndExit(int status)
{
    exit(status);
}