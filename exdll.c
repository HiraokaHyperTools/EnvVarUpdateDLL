#include <windows.h>
#include <nsis/pluginapi.h> // nsis plugin

HINSTANCE g_hInstance;

BOOL WINAPI DllMain(HINSTANCE hInst, ULONG ul_reason_for_call, LPVOID lpReserved)
{
    g_hInstance = hInst;
    return TRUE;
}
