/*
 * expWinPort.h --
 *
 *	This header file handles porting issues that occur because of
 *	differences between Windows and Unix. 
 *
 * Copyright (c) 1997 Mitel Corporation
 *
 * See the file "license.terms" for information on usage and redistribution
 * of this file, and for a DISCLAIMER OF ALL WARRANTIES.
 *
 */

#ifndef _EXPWINPORT
#define _EXPWINPORT

#include <windows.h>
#include <time.h>

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
#define FILEPOSINFO			__FILE__ "(" STRINGIFY(__LINE__) ")"
#define EXP_LOG0(errCode)		ExpSyslog(errCode, FILEPOSINFO, 0)
#define EXP_LOG1(errCode, arg1)		ExpSyslog(errCode, FILEPOSINFO, arg1, 0)
#define EXP_LOG2(errCode, arg1, arg2)	ExpSyslog(errCode, FILEPOSINFO, arg1, arg2, 0)

/*
 * The following defines identify the various types of applications that 
 * run under windows.  There is special case code for the various types.
 */

#define EXP_APPL_NONE	0
#define EXP_APPL_DOS	1
#define EXP_APPL_WIN3X	2
#define EXP_APPL_WIN32CUI	3
#define EXP_APPL_WIN32GUI	4


extern DWORD		ExpApplicationType (const char *originalName,
			    char *fullPath);
extern DWORD		ExpCreateProcess (int argc, char **argv,
			    HANDLE inputHandle, HANDLE outputHandle,
			    HANDLE errorHandle, int allocConsole,
			    int hideConsole, int debug, int newProcessGroup,
			    Tcl_Pid *pidPtr, PDWORD globalPidPtr);
extern Tcl_Channel	ExpCreateSpawnChannel (Tcl_Interp *, Tcl_Channel chan);
extern void		ExpSyslog TCL_VARARGS(DWORD,arg1);
extern TCHAR*		ExpSyslogGetSysMsg (DWORD);
extern Tcl_Pid		Exp_WaitPid (Tcl_Pid pid, int *statPtr, int options);
extern void		Exp_KillProcess (Tcl_Pid pid);
extern void		ExpInitWinProcessAPI (void);
extern int		ExpDynloadTclStubs (void);

#include "expPlatDecls.h"
#include "expIntPlatDecls.h"


#endif /* _EXPWINPORT */
