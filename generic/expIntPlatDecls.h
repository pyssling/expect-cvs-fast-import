/*
 * expPlatIntDecls.h --
 *
 *	Declarations of platform specific Expect APIs.
 *
 * RCS: @(#) $Id: expPlatIntDecls.h,v 1.1.2.1 2001/10/29 20:40:19 davygrvy Exp $
 */

#ifndef _EXPPLATINTDECLS
#define _EXPPLATINTDECLS

/*
 *  Pull in the definition of TCHAR.  Hopefully the compile flags
 *  of the core are matching against your project build for these
 *  public functions.  BE AWARE.
 */
#ifdef __WIN32__
#   ifndef _TCHAR_DEFINED
#	include <tchar.h>
#	ifndef _TCHAR_DEFINED
	    /* Borland seems to forget to set this. */
	    typedef _TCHAR TCHAR;
#	    define _TCHAR_DEFINED
#	endif
#   endif
#endif

/* !BEGIN!: Do not edit below this line. */

/*
 * Exported function declarations:
 */

#ifdef __WIN32__
/* 0 */
TCL_EXTERN(DWORD)	ExpWinApplicationType _ANSI_ARGS_((
				const char * originalName, char * fullPath));
/* 1 */
TCL_EXTERN(DWORD)	ExpWinCreateProcess _ANSI_ARGS_((int argc, 
				char ** argv, HANDLE inputHandle, 
				HANDLE outputHandle, HANDLE errorHandle, 
				int allocConsole, int hideConsole, int debug, 
				int newProcessGroup, Tcl_Pid * pidPtr, 
				PDWORD globalPidPtr));
/* 2 */
TCL_EXTERN(void)	ExpWinSyslog _ANSI_ARGS_(TCL_VARARGS(DWORD,errId));
/* 3 */
TCL_EXTERN(TCHAR*)	ExpSyslogGetSysMsg _ANSI_ARGS_((DWORD errId));
/* 4 */
TCL_EXTERN(Tcl_Pid)	Exp_WaitPid _ANSI_ARGS_((Tcl_Pid pid, int * statPtr, 
				int options));
/* 5 */
TCL_EXTERN(void)	Exp_KillProcess _ANSI_ARGS_((Tcl_Pid pid));
#endif /* __WIN32__ */

typedef struct ExpIntPlatStubs {
    int magic;
    struct ExpIntPlatStubHooks *hooks;

#ifdef __WIN32__
    DWORD (*expWinApplicationType) _ANSI_ARGS_((const char * originalName, char * fullPath)); /* 0 */
    DWORD (*expWinCreateProcess) _ANSI_ARGS_((int argc, char ** argv, HANDLE inputHandle, HANDLE outputHandle, HANDLE errorHandle, int allocConsole, int hideConsole, int debug, int newProcessGroup, Tcl_Pid * pidPtr, PDWORD globalPidPtr)); /* 1 */
    void (*expWinSyslog) _ANSI_ARGS_(TCL_VARARGS(DWORD,errId)); /* 2 */
    TCHAR* (*expSyslogGetSysMsg) _ANSI_ARGS_((DWORD errId)); /* 3 */
    Tcl_Pid (*exp_WaitPid) _ANSI_ARGS_((Tcl_Pid pid, int * statPtr, int options)); /* 4 */
    void (*exp_KillProcess) _ANSI_ARGS_((Tcl_Pid pid)); /* 5 */
#endif /* __WIN32__ */
} ExpIntPlatStubs;

#ifdef __cplusplus
extern "C" {
#endif
extern ExpIntPlatStubs *expIntPlatStubsPtr;
#ifdef __cplusplus
}
#endif

#if defined(USE_EXP_STUBS) && !defined(USE_EXP_STUB_PROCS)

/*
 * Inline function declarations:
 */

#ifdef __WIN32__
#ifndef ExpWinApplicationType
#define ExpWinApplicationType \
	(expIntPlatStubsPtr->expWinApplicationType) /* 0 */
#endif
#ifndef ExpWinCreateProcess
#define ExpWinCreateProcess \
	(expIntPlatStubsPtr->expWinCreateProcess) /* 1 */
#endif
#ifndef ExpWinSyslog
#define ExpWinSyslog \
	(expIntPlatStubsPtr->expWinSyslog) /* 2 */
#endif
#ifndef ExpSyslogGetSysMsg
#define ExpSyslogGetSysMsg \
	(expIntPlatStubsPtr->expSyslogGetSysMsg) /* 3 */
#endif
#ifndef Exp_WaitPid
#define Exp_WaitPid \
	(expIntPlatStubsPtr->exp_WaitPid) /* 4 */
#endif
#ifndef Exp_KillProcess
#define Exp_KillProcess \
	(expIntPlatStubsPtr->exp_KillProcess) /* 5 */
#endif
#endif /* __WIN32__ */

#endif /* defined(USE_EXP_STUBS) && !defined(USE_EXP_STUB_PROCS) */

/* !END!: Do not edit above this line. */

#endif /* _EXPPLATINTDECLS */


