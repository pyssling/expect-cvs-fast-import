/* ----------------------------------------------------------------------------
 * expWinInit.c --
 *
 *	Win OS specific inits.
 *
 * ----------------------------------------------------------------------------
 *
 * Written by: Don Libes, libes@cme.nist.gov, NIST, 12/3/90
 * 
 * Design and implementation of this program was paid for by U.S. tax
 * dollars.  Therefore it is public domain.  However, the author and NIST
 * would appreciate credit if this program or parts of it are used.
 * 
 * Copyright (c) 1997 Mitel Corporation
 *	work by Gordon Chaffee <chaffee@bmrc.berkeley.edu> for the WinNT port.
 *
 * Copyright (c) 2001-2002 Telindustrie, LLC
 *	work by David Gravereaux <davygrvy@pobox.com> for any Win32 OS.
 *
 * ----------------------------------------------------------------------------
 * URLs:    http://expect.nist.gov/
 *	    http://expect.sf.net/
 *	    http://bmrc.berkeley.edu/people/chaffee/expectnt.html
 * ----------------------------------------------------------------------------
 * RCS: @(#) $Id: expWinInit.c,v 1.1.4.2 2002/02/10 12:03:30 davygrvy Exp $
 * ----------------------------------------------------------------------------
 */

#include "expWinInt.h"

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
    (BOOL (WINAPI *)(LPCTSTR, LPTSTR, DWORD)) GetEnvironmentVariableA,
    (LONG (WINAPI *)(HKEY, LPCTSTR, DWORD, DWORD, CONST BYTE *,
	    DWORD)) RegSetValueExA
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
    (BOOL (WINAPI *)(LPCTSTR, LPTSTR, DWORD)) GetEnvironmentVariableW,
    (LONG (WINAPI *)(HKEY, LPCTSTR, DWORD, DWORD, CONST BYTE *,
	    DWORD)) RegSetValueExW
};

ExpWinProcs *expWinProcs = &asciiProcs;
tclWinMakeFileProcType tclWinMakeFileProc;

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

    /* need TclWinMakeFile() from the outside... bastards! */
    tclWinMakeFileProc = (tclWinMakeFileProcType)
	    GetProcAddress(TclWinGetTclInstance(), "TclWinMakeFile");
}
