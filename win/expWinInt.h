/* ----------------------------------------------------------------------------
 * expWinInt.h --
 *
 *	Declarations of Windows-specific shared variables and procedures.
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
 * RCS: @(#) $Id: expWinInt.h,v 1.1.2.1 2001/11/07 10:04:57 davygrvy Exp $
 * ----------------------------------------------------------------------------
 */
#ifndef _EXPWININT
#define _EXPWININT

#ifndef _EXPINT
#   ifndef _EXP
#	define STRICT		    /* Ask windows.h to be agressive about the HANDLE type. */
#	define WINVER 0x0400	    /* Make sure we get the Win95 API. */
#   endif
#   include "expInt.h"
#endif

#ifndef _EXPPORT
#   include "expPort.h"
#endif

#undef TCL_STORAGE_CLASS
#if defined(BUILD_spawndriver)
#   define TCL_STORAGE_CLASS
extern TCL_CPP void ExpInitWinProcessAPI (void);
extern TCL_CPP void ExpDynloadTclStubs (void);
#   include "expWinSlave.hpp"
#   include "spawndrvmc.h"
#elif defined(BUILD_exp)
#   define TCL_STORAGE_CLASS DLLEXPORT
#else
#   ifdef USE_EXP_STUBS
#	define TCL_STORAGE_CLASS
#   else
#	define TCL_STORAGE_CLASS DLLIMPORT
#   endif
#endif


typedef struct {
    int useWide;
    HANDLE (WINAPI *createFileProc)(CONST TCHAR *, DWORD, DWORD, 
	    LPSECURITY_ATTRIBUTES, DWORD, DWORD, HANDLE);
    BOOL (WINAPI *createProcessProc)(CONST TCHAR *, TCHAR *, 
	    LPSECURITY_ATTRIBUTES, LPSECURITY_ATTRIBUTES, BOOL, DWORD, 
	    LPVOID, CONST TCHAR *, LPSTARTUPINFO, LPPROCESS_INFORMATION);
    DWORD (WINAPI *getFileAttributesProc)(CONST TCHAR *);
    DWORD (WINAPI *getShortPathNameProc)(CONST TCHAR *, TCHAR *, DWORD); 
    DWORD (WINAPI *searchPathProc)(CONST TCHAR *, CONST TCHAR *, 
	    CONST TCHAR *, DWORD, TCHAR *, TCHAR **);
} ExpWinProcs;


extern ExpWinProcs *expWinProcs;



#include "expIntPlatDecls.h"

#undef TCL_STORAGE_CLASS
#define TCL_STORAGE_CLASS DLLIMPORT

#endif /* _EXPWININT */
