/*
 * expWin.h --
 *
 *	Useful definitions for Expect on NT.
 *
 * Copyright (c) 1997 by Mitel, Inc.
 * Copyright (c) 1997 by Gordon Chaffee (chaffee@home.com)
 *
 * See the file "license.terms" for information on usage and redistribution
 * of this file, and for a DISCLAIMER OF ALL WARRANTIES.
 *
 */

#undef TCL_STORAGE_CLASS
#ifdef BUILD_expect
#   define TCL_STORAGE_CLASS DLLEXPORT
#else
#   define TCL_STORAGE_CLASS DLLIMPORT
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

typedef struct {
    Tcl_Channel channelPtr;
    int toWrite;
} ExpSpawnState;

extern void		ExpWinProcessInit(void);
extern DWORD		ExpWinApplicationType(const char *originalName,
			    Tcl_DString *fullPath);
extern DWORD		ExpWinCreateProcess(int argc, char **argv,
			    HANDLE inputHandle, HANDLE outputHandle,
			    HANDLE errorHandle, int allocConsole,
			    int hideConsole, int debug, int newProcessGroup,
			    Tcl_Pid *pidPtr, PDWORD globalPidPtr);
EXTERN Tcl_Channel	ExpCreateSpawnChannel _ANSI_ARGS_((Tcl_Interp *,
			    Tcl_Channel chan));
extern void		ExpSyslog TCL_VARARGS(char *,fmt);
extern Tcl_Pid		Exp_WaitPid(Tcl_Pid pid, int *statPtr, int options);
extern void		Exp_KillProcess(Tcl_Pid pid);
