#include "integrity.h"

void Startup()
{
    AuthLoad();
}

void SaveAndExit()
{
    AuthSave();
    exit(0);
}