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
 * Copyright (c) 2002 Telindustrie, LLC
 *	work by David Gravereaux <davygrvy@pobox.com> for any Win32 OS.
 *
 * ----------------------------------------------------------------------------
 * URLs:    http://expect.nist.gov/
 *	    http://expect.sf.net/
 *	    http://bmrc.berkeley.edu/people/chaffee/expectnt.html
 * ----------------------------------------------------------------------------
 * RCS: @(#) $Id: expWinInt.h,v 1.1.4.1 2002/02/10 02:59:46 davygrvy Exp $
 * ----------------------------------------------------------------------------
 */
#ifndef _EXPWININT
#define _EXPWININT

#ifndef _EXPINT
#   ifndef _EXP
	/* Ask windows.h to be agressive about the HANDLE type. */
#	define STRICT
	/* make sure we get the Win95 API */
#	define WINVER 0x0400
#	ifdef _DEBUG
	    /* Make sure we add the WinNT API for IsDebuggerPresent(). */
#	    define _WIN32_WINNT 0x0400
#	endif
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
#   include "expWinSlave.h"
#   ifdef _DEBUG
#	include "MsvcDbgControl.h"
#   endif
/*#   include "spawndrvmc.h"*/
#elif defined(BUILD_exp)
#   define TCL_STORAGE_CLASS DLLEXPORT
#else
#   ifdef USE_EXP_STUBS
#	define TCL_STORAGE_CLASS
#   else
#	define TCL_STORAGE_CLASS DLLIMPORT
#   endif
#endif


#define EXP_SLAVE_CREATE 'c'
#define EXP_SLAVE_KEY    'k'
#define EXP_SLAVE_MOUSE  'm'
#define EXP_SLAVE_WRITE  'w'
#define EXP_SLAVE_KILL   'x'

/*
 * Define the types of attempts to use to kill the subprocess
 */
#define EXP_KILL_TERMINATE  0x1
#define EXP_KILL_CTRL_C     0x2
#define EXP_KILL_CTRL_BREAK 0x4

#define EXP_LOG(format, args) \
    ExpSyslog("Expect SlaveDriver (%s: %d): " format, __FILE__, __LINE__, args)

/*
 * The following defines identify the various types of applications that 
 * run under windows.  There is special case code for the various types.
 */

#define EXP_APPL_NONE	    0
#define EXP_APPL_BATCH	    1
#define EXP_APPL_DOS16	    2
#define EXP_APPL_OS2	    3
#define EXP_APPL_OS2DRV	    4
#define EXP_APPL_WIN16	    5
#define EXP_APPL_WIN16DRV   6
#define EXP_APPL_WIN32CUI   7
#define EXP_APPL_WIN32GUI   8
#define EXP_APPL_WIN32DLL   9
#define EXP_APPL_WIN32DRV   10
#define EXP_APPL_WIN64CUI   11
#define EXP_APPL_WIN64GUI   12
#define EXP_APPL_WIN64DLL   13
#define EXP_APPL_WIN64DRV   14


extern HMODULE expDllInstance;

typedef struct {
    int useWide;
    HANDLE (WINAPI *createFileProc)(LPCTSTR, DWORD, DWORD,
	    LPSECURITY_ATTRIBUTES, DWORD, DWORD, HANDLE);
    BOOL (WINAPI *createProcessProc)(LPCTSTR, LPTSTR, LPSECURITY_ATTRIBUTES,
	    LPSECURITY_ATTRIBUTES, BOOL, DWORD, LPVOID, LPCTSTR, LPSTARTUPINFO,
	    LPPROCESS_INFORMATION);
    DWORD (WINAPI *getFileAttributesProc)(LPCTSTR);
    DWORD (WINAPI *getShortPathNameProc)(LPCTSTR, LPTSTR, DWORD); 
    DWORD (WINAPI *searchPathProc)(LPCTSTR, LPCTSTR, LPCTSTR, DWORD, LPTSTR,
	    LPTSTR *);
    VOID (WINAPI *outputDebugStringProc)(LPCTSTR);
    DWORD (WINAPI *getModuleFileNameProc)(HMODULE, LPTSTR, DWORD);
    BOOL (WINAPI *setEnvironmentVariableProc)(LPCTSTR, LPCTSTR);
    BOOL (WINAPI *getEnvironmentVariableProc)(LPCTSTR, LPTSTR, DWORD);
    LONG (WINAPI *regSetValueExProc)(HKEY, LPCTSTR, DWORD, DWORD, CONST BYTE *,
	    DWORD);
} ExpWinProcs;

extern ExpWinProcs *expWinProcs;


#include "expIntPlatDecls.h"

#undef TCL_STORAGE_CLASS
#define TCL_STORAGE_CLASS DLLIMPORT

#endif /* _EXPWININT */
