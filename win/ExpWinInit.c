#include "tclPort.h"
#include "expWin.h"

static ExpWinProcs asciiProcs = {
    0,
    (HANDLE (WINAPI *)(LPCTSTR, DWORD, DWORD, SECURITY_ATTRIBUTES *, 
	    DWORD, DWORD, HANDLE)) CreateFileA,
    (BOOL (WINAPI *)(LPCTSTR, LPTSTR, LPSECURITY_ATTRIBUTES, 
	    LPSECURITY_ATTRIBUTES, BOOL, DWORD, LPVOID, LPCTSTR, 
	    LPSTARTUPINFO, LPPROCESS_INFORMATION)) CreateProcessA,
    (DWORD (WINAPI *)(LPCTSTR)) GetFileAttributesA,
    (DWORD (WINAPI *)(LPCTSTR, LPTSTR, DWORD)) GetShortPathNameA,
    (DWORD (WINAPI *)(LPCTSTR, LPCTSTR, LPCTSTR, DWORD, 
	    LPTSTR, LPTSTR *)) SearchPathA,
    (VOID (WINAPI *)(LPCTSTR)) OutputDebugStringA,
    (DWORD (WINAPI *)(HMODULE, LPTSTR, DWORD)) GetModuleFileNameA,
    (BOOL (WINAPI *)(LPCTSTR, LPCTSTR)) SetEnvironmentVariableA,
    (BOOL (WINAPI *)(LPCTSTR, LPTSTR, DWORD)) GetEnvironmentVariableA
};

static ExpWinProcs unicodeProcs = {
    1,
    (HANDLE (WINAPI *)(LPCTSTR, DWORD, DWORD, SECURITY_ATTRIBUTES *, 
	    DWORD, DWORD, HANDLE)) CreateFileW,
    (BOOL (WINAPI *)(LPCTSTR, LPTSTR, LPSECURITY_ATTRIBUTES, 
	    LPSECURITY_ATTRIBUTES, BOOL, DWORD, LPVOID, LPCTSTR, 
	    LPSTARTUPINFO, LPPROCESS_INFORMATION)) CreateProcessW,
    (DWORD (WINAPI *)(LPCTSTR)) GetFileAttributesW,
    (DWORD (WINAPI *)(LPCTSTR, LPTSTR, DWORD)) GetShortPathNameW,
    (DWORD (WINAPI *)(LPCTSTR, LPCTSTR, LPCTSTR, DWORD, 
	    LPTSTR, LPTSTR *)) SearchPathW,
    (VOID (WINAPI *)(LPCTSTR)) OutputDebugStringW,
    (DWORD (WINAPI *)(HMODULE, LPTSTR, DWORD)) GetModuleFileNameW,
    (BOOL (WINAPI *)(LPCTSTR, LPCTSTR)) SetEnvironmentVariableW,
    (BOOL (WINAPI *)(LPCTSTR, LPTSTR, DWORD)) GetEnvironmentVariableW
};

ExpWinProcs *expWinProcs = &asciiProcs;

/*
 *----------------------------------------------------------------------
 *  ExpWinInit --
 *
 *	Switches to the correct native API at run-time.  Works in
 *	tandem with Tcl_WinUtfToTchar().
 *
 *  Returns:
 *	nothing.
 *
 *----------------------------------------------------------------------
 */
void
ExpWinInit(void)
{
    if (TclWinGetPlatformId() == VER_PLATFORM_WIN32_NT) {    
	expWinProcs = &unicodeProcs;
    }
}
