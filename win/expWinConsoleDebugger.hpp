/* ----------------------------------------------------------------------------
 * expWinConsoleDebugger.hpp --
 *
 *	Console debugger class defined here.
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
 * RCS: @(#) $Id: expWinConsoleDebugger.hpp,v 1.1.2.1 2002/03/07 00:21:57 davygrvy Exp $
 * ----------------------------------------------------------------------------
 */

#ifndef INC_expWinConsoleDebugger_hpp__
#define INC_expWinConsoleDebugger_hpp__

// Although we pull in the internal Expect headers, no references are made
// to it from here.
#include "expWinInt.h"
#include <imagehlp.h>

#ifdef _M_IX86
    // 4096 is for ix86 only
#   define PAGESIZE 0x1000
    // This only works on ix86
#   define SINGLE_STEP_BIT 0x100;
#else
#   error "need platform page size"
#endif
#define PAGEMASK (PAGESIZE-1)


//  This is our debugger.  We run it as a thread. 
//
class ConsoleDebugger : public CMclThreadHandler
{
public:
    ConsoleDebugger(int argc, char * const *argv);

private:
    virtual unsigned ThreadHandlerProc(void);

    // forward reference.
    class Process;
    class Breakpoint;

    class CreateProcessInfo {
	friend class ConsoleDebugger;
	TCHAR		    appName[8192];
	TCHAR		    cmdLine[8192];
	SECURITY_ATTRIBUTES procAttrs;
	SECURITY_ATTRIBUTES threadAttrs;
	BOOL		    bInheritHandles;
	DWORD		    dwCreationFlags;
	LPVOID		    lpEnvironment;
	TCHAR		    currDir[8192];
	STARTUPINFO	    si;
	PROCESS_INFORMATION pi;
	PVOID		    piPtr;  // Pointer to PROCESS_INFORMATION in slave.
	DWORD		    flags;
    };

    class CreateProcessThreadArgs {
	friend class ConsoleDebugger;
	CreateProcessInfo   *cp;
	Process		    *proc;
	//ExpSlaveDebugArg debugInfo;
    };

    class ThreadInfo {
	friend class ConsoleDebugger;
	HANDLE	    hThread;
	DWORD	    dwThreadId;
	DWORD	    nargs;
	DWORD	    args[16];	// Space for saving 16 args.  We need this
				// space while we are waiting for the return
				// value for the function.
	LPCONTEXT   context;	// Current context.
	CreateProcessInfo *createProcess; // Create process pointer.
	ThreadInfo  *nextPtr;	// Linked list.
    };

    class BreakInfo {
	friend class ConsoleDebugger;
	const char  *funcName;	// Name of function to intercept.
	DWORD	    nargs;	// Number of arguments.
	void (ConsoleDebugger::*breakProc)(Process *, ThreadInfo *, Breakpoint *, PDWORD returnValue, DWORD direction);
				// Function to call when the breakpoint is hit.
#	define BREAK_IN  1	// Call handler on the way in.
#	define BREAK_OUT 2	// Call handler on the way out.
	DWORD	    dwFlags;	// Bits for direction to call handler in.
    };

    class DllBreakpoints {
	friend class	ConsoleDebugger;
	const char	*dllName;
	BreakInfo	*breakInfo;
    };

    class Breakpoint {
	friend class ConsoleDebugger;
	BOOL	    returning;	    // Is this a returning breakpoint?
	UCHAR	    code;	    // Original code.
	PVOID	    codePtr;	    // Address of original code.
	PVOID	    codeReturnPtr;  // Address of return breakpoint.
	DWORD	    origRetAddr;    // Original return address.
	BreakInfo   *breakInfo;	    // Information about the breakpoint.
	ThreadInfo  *threadInfo;    // If this breakpoint is for a specific thread.
	Breakpoint  *nextPtr;	    // Linked list.
    };

    class Module {
	friend class ConsoleDebugger;
	BOOL   loaded;
	HANDLE hFile;
	LPVOID baseAddr;
	PCHAR  modName;
	PIMAGE_DEBUG_INFORMATION dbgInfo;
    };

    //  There is one of these instances for each subprocess that we are
    //  controlling.
    //
    class Process {
	friend class ConsoleDebugger;
	ThreadInfo  *threadList;	// List of threads in the subprocess.
	Breakpoint  *brkptList;		// List of breakpoints in the subprocess.
	Breakpoint  *lastBrkpt;		// Last breakpoint hit.
	DWORD	    offset;		// Breakpoint offset in allocated mem.
	DWORD	    nBreakCount;	// Number of breakpoints hit.
	DWORD	    consoleHandles[100];// A list of input console handles.
	DWORD	    consoleHandlesMax;
	BOOL	    isConsoleApp;	// Is this a console app?
	BOOL	    isShell;		// Is this some sort of console shell?
	HANDLE	    hProcess;		// handle to subprocess.
	DWORD	    hPid;		// Global process id.
	DWORD	    threadCount;	// Number of threads in process.
	DWORD	    pSubprocessMemory;	// Pointer to allocated memory in subprocess.
	DWORD	    pSubprocessBuffer;	// Pointer to buffer memory in subprocess.
	DWORD	    pMemoryCacheBase;	// Base address of memory cache.
	BYTE	    pMemoryCache[PAGESIZE];// Subprocess memory cache.
	Tcl_HashTable *funcTable;	// Function table name to address mapping.
	Tcl_HashTable *moduleTable;	// Win32 modules that have been loaded.
	Module	    *exeModule;		// Executable module info.
	Process	    *nextPtr;		// Linked list.
    };

    //  Direct debug event handlers.
    //
    void OnXFirstBreakpoint	    (Process *, LPDEBUG_EVENT);
    void OnXSecondBreakpoint	    (Process *, LPDEBUG_EVENT);
    void OnXBreakpoint		    (Process *, LPDEBUG_EVENT);
    void OnXCreateProcess	    (Process *, LPDEBUG_EVENT);
    void OnXCreateThread	    (Process *, LPDEBUG_EVENT);
    void OnXDeleteThread	    (Process *, LPDEBUG_EVENT);
    void OnXLoadDll		    (Process *, LPDEBUG_EVENT);
    void OnXUnloadDll		    (Process *, LPDEBUG_EVENT);
    void OnXSecondChanceException   (Process *, LPDEBUG_EVENT);
    void OnXSingleStep		    (Process *, LPDEBUG_EVENT);

    //  Our breakpoint handlers (indirect).  Called from OnXBreakpoint().
    //
    void OnBeep				(Process *, ThreadInfo *, Breakpoint *, PDWORD returnValue, DWORD direction);
    void OnFillConsoleOutputCharacter	(Process *, ThreadInfo *, Breakpoint *, PDWORD returnValue, DWORD direction);
    void OnGetStdHandle			(Process *, ThreadInfo *, Breakpoint *, PDWORD returnValue, DWORD direction);
    void OnIsWindowVisible		(Process *, ThreadInfo *, Breakpoint *, PDWORD returnValue, DWORD direction);
    void OnOpenConsoleW			(Process *, ThreadInfo *, Breakpoint *, PDWORD returnValue, DWORD direction);
    void OnReadConsoleInput		(Process *, ThreadInfo *, Breakpoint *, PDWORD returnValue, DWORD direction);
    void OnSetConsoleActiveScreenBuffer	(Process *, ThreadInfo *, Breakpoint *, PDWORD returnValue, DWORD direction);
    void OnSetConsoleCursorPosition	(Process *, ThreadInfo *, Breakpoint *, PDWORD returnValue, DWORD direction);
    void OnSetConsoleMode		(Process *, ThreadInfo *, Breakpoint *, PDWORD returnValue, DWORD direction);
    void OnSetConsoleWindowInfo		(Process *, ThreadInfo *, Breakpoint *, PDWORD returnValue, DWORD direction);
    void OnScrollConsoleScreenBuffer	(Process *, ThreadInfo *, Breakpoint *, PDWORD returnValue, DWORD direction);
    void OnWriteConsoleA		(Process *, ThreadInfo *, Breakpoint *, PDWORD returnValue, DWORD direction);
    void OnWriteConsoleW		(Process *, ThreadInfo *, Breakpoint *, PDWORD returnValue, DWORD direction);
    void OnWriteConsoleOutputA		(Process *, ThreadInfo *, Breakpoint *, PDWORD returnValue, DWORD direction);
    void OnWriteConsoleOutputW		(Process *, ThreadInfo *, Breakpoint *, PDWORD returnValue, DWORD direction);
    void OnWriteConsoleOutputCharacterA	(Process *, ThreadInfo *, Breakpoint *, PDWORD returnValue, DWORD direction);
    void OnWriteConsoleOutputCharacterW	(Process *, ThreadInfo *, Breakpoint *, PDWORD returnValue, DWORD direction);
    void OnExpGetExecutablePathA	(Process *, ThreadInfo *, Breakpoint *, PDWORD returnValue, DWORD direction);
    void OnExpGetExecutablePathW	(Process *, ThreadInfo *, Breakpoint *, PDWORD returnValue, DWORD direction);
    void OnSearchPathW			(Process *, ThreadInfo *, Breakpoint *, PDWORD returnValue, DWORD direction);
    void OnlstrcpynW			(Process *, ThreadInfo *, Breakpoint *, PDWORD returnValue, DWORD direction);
    void OnlstrrchrW			(Process *, ThreadInfo *, Breakpoint *, PDWORD returnValue, DWORD direction);
    void OnGetFileAttributesW		(Process *, ThreadInfo *, Breakpoint *, PDWORD returnValue, DWORD direction);

    // Internal utilities
    //
    Process *ProcessNew		();
    void ProcessFree		(Process *);
    void CommonDebugger		();
    BOOL SetBreakpoint		(Process *, BreakInfo *);
    Breakpoint * SetBreakpointAtAddr(Process *, BreakInfo *, PVOID);
    BOOL ReadSubprocessMemory	(Process *, LPVOID, LPVOID, DWORD);
    BOOL WriteSubprocessMemory	(Process *, LPVOID, LPVOID, DWORD);
    int ReadSubprocessStringA	(Process *, PVOID, PCHAR, int);
    int ReadSubprocessStringW	(Process *, PVOID, PWCHAR, int);
    void CreateVtSequence	(Process *proc, COORD newPos, DWORD n);

    // The arrays of functions where we set breakpoints
    //
    BreakInfo BreakArrayKernel32[20];
    BreakInfo BreakArrayUser32[2];
    DllBreakpoints BreakPoints[3];

    // private vars
    //
    Process *ProcessList;   // Top of linked list of Process instances.
    HANDLE  HConsole;	    // Master console handle (us).
    COORD   ConsoleSize;    // Size of the console in the slave.
    COORD   CursorPosition; // Coordinates of the cursor in the slave.
    BOOL    CursorKnown;    // Do we know where the remote cursor is?
    char    *SymbolPath;    // Storage for setting OS kernel symbols path.
    int	    _argc;	    // Debugged process commandline count
    char * const * _argv;   // Debugged process commandline args
};

#endif // INC_expWinConsoleDebugger_hpp__
