/* ----------------------------------------------------------------------------
 * expWinPort.h --
 *
 *	This header file handles porting issues that occur because of
 *	differences between Windows and Unix. 
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
 * RCS: @(#) $Id: expWinPort.h,v 1.1.2.1.2.4 2002/02/11 09:56:00 davygrvy Exp $
 * ----------------------------------------------------------------------------
 */

#ifndef _EXPWINPORT
#define _EXPWINPORT

#ifndef _EXPINT
#   include "expInt.h"
#endif


#define HAVE_SV_TIMEZONE 1

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

/*
 * Errors and logging
 */
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


#undef TCL_STORAGE_CLASS
#if defined(BUILD_slavedriver)
#   define TCL_STORAGE_CLASS
#elif defined(BUILD_exp)
#   define TCL_STORAGE_CLASS DLLEXPORT
#else
#   ifdef USE_EXP_STUBS
#	define TCL_STORAGE_CLASS
#   else
#	define TCL_STORAGE_CLASS DLLIMPORT
#   endif
#endif

typedef TclFile (__cdecl *tclWinMakeFileProcType)(HANDLE handle);
extern tclWinMakeFileProcType tclWinMakeFileProc;


#include "expPlatDecls.h"
/*#include "expIntPlatDecls.h"*/

#undef TCL_STORAGE_CLASS
#define TCL_STORAGE_CLASS DLLIMPORT

#endif /* _EXPWINPORT */
