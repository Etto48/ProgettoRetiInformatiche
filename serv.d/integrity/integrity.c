#include "integrity.h"

void Startup()
{
    AuthLoad(AUTH_FILE);
}

void SaveAndExit()
{
    AuthSave(AUTH_FILE);
    exit(0);
}