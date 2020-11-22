#define WIN32_LEAN_AND_MEAN

#include <Windows.h>
#include <detours.h>

#include "hooks.h"

BOOL APIENTRY DllMain(
    HMODULE hModule,
    DWORD dwReason,
    LPVOID lpReserved)
{
    if (DetourIsHelperProcess())
    {
        return TRUE;
    }

    switch (dwReason)
    {
    case DLL_PROCESS_ATTACH:
        DetourRestoreAfterWith();
        HooksAttachAll();
        break;

    case DLL_PROCESS_DETACH:
        HooksDetachAll();
        break;
    }

    return TRUE;
}
