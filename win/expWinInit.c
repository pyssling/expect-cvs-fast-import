/* ----------------------------------------------------------------------------
 * expWinInit.c --
 *
 *	Contains startup code needed for the Expect extension on Windows.
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
 * Copyright (c) 2001 Telindustrie, LLC
 *	work by David Gravereaux <davygrvy@pobox.com> for any Win32 OS.
 *
 * ----------------------------------------------------------------------------
 * URLs:    http://expect.nist.gov/
 *	    http://expect.sf.net/
 *	    http://bmrc.berkeley.edu/people/chaffee/expectnt.html
 * ----------------------------------------------------------------------------
 * RCS: @(#) $Id: expWinLog.c,v 1.1.2.5 2001/11/07 10:04:57 davygrvy Exp $
 * ----------------------------------------------------------------------------
 */

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
