#include "expWinInt.h"

static ExpWinProcs asciiProcs = {
    0,
    (HANDLE (WINAPI *)(CONST TCHAR *, DWORD, DWORD, SECURITY_ATTRIBUTES *, 
	    DWORD, DWORD, HANDLE)) CreateFileA,
    (BOOL (WINAPI *)(CONST TCHAR *, TCHAR *, LPSECURITY_ATTRIBUTES, 
	    LPSECURITY_ATTRIBUTES, BOOL, DWORD, LPVOID, CONST TCHAR *, 
	    LPSTARTUPINFO, LPPROCESS_INFORMATION)) CreateProcessA,
    (DWORD (WINAPI *)(CONST TCHAR *)) GetFileAttributesA,
    (DWORD (WINAPI *)(CONST TCHAR *, TCHAR *, DWORD)) GetShortPathNameA,
    (DWORD (WINAPI *)(CONST TCHAR *, CONST TCHAR *, CONST TCHAR *, DWORD, 
	    TCHAR *, TCHAR **)) SearchPathA
};

static ExpWinProcs unicodeProcs = {
    1,
    (HANDLE (WINAPI *)(CONST TCHAR *, DWORD, DWORD, SECURITY_ATTRIBUTES *, 
	    DWORD, DWORD, HANDLE)) CreateFileW,
    (BOOL (WINAPI *)(CONST TCHAR *, TCHAR *, LPSECURITY_ATTRIBUTES, 
	    LPSECURITY_ATTRIBUTES, BOOL, DWORD, LPVOID, CONST TCHAR *, 
	    LPSTARTUPINFO, LPPROCESS_INFORMATION)) CreateProcessW,
    (DWORD (WINAPI *)(CONST TCHAR *)) GetFileAttributesW,
    (DWORD (WINAPI *)(CONST TCHAR *, TCHAR *, DWORD)) GetShortPathNameW,
    (DWORD (WINAPI *)(CONST TCHAR *, CONST TCHAR *, CONST TCHAR *, DWORD, 
	    TCHAR *, TCHAR **)) SearchPathW
};

ExpWinProcs *expWinProcs = &asciiProcs;


void
ExpWinInit (void)
{
    OSVERSIONINFO os;
    os.dwOSVersionInfoSize = sizeof(OSVERSIONINFO);
    
    if (GetVersionEx(&os) != 0) {    
	switch (os.dwPlatformId) {
	    case VER_PLATFORM_WIN32_WINDOWS:
		expWinProcs = &asciiProcs; break;
	    case VER_PLATFORM_WIN32_NT:
		expWinProcs = &unicodeProcs; break;
	}
    }
}
