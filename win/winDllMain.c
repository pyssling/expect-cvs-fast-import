#define WIN32_LEAN_AND_MEAN
#include <windows.h>

/* public global */
HMODULE expDllInstance;

BOOL WINAPI
DllMain (HINSTANCE hInst, ULONG ulReason, LPVOID lpReserved)
{
    switch (ulReason) {
    case DLL_PROCESS_ATTACH:
	expDllInstance = hInst;
    }
    return TRUE;
}
