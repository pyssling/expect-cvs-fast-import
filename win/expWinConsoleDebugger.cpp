/* ----------------------------------------------------------------------------
 * expWinConsoleDebugger.cpp --
 *
 *	Console debugger core implimentation.
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
 * RCS: @(#) $Id: expWinConsoleDebugger.cpp,v 1.1.2.1 2002/03/07 03:25:43 davygrvy Exp $
 * ----------------------------------------------------------------------------
 */

#include <stddef.h>
#include <assert.h>
#include "expWinConsoleDebugger.hpp"
#ifdef _MSC_VER
#   pragma comment (lib, "imagehlp.lib")
#endif

//  Constructor.
ConsoleDebugger::ConsoleDebugger (int argc, char * const *argv)
    : _argc(argc), _argv(argv), ProcessList(0L), CursorKnown(FALSE)
{
    //  Until further notice, assume this.
    //
    ConsoleSize.X = 80;
    ConsoleSize.Y = 25;

    //  Until further notice, assume this, too.
    //
    CursorPosition.X = 0;
    CursorPosition.Y = 0;

    //  Set all our breakpoint info.  We have to do this the long way, here in
    //  the constructor, because we need to have the this pointer already
    //  set because we refer to ourselves by needing the address to member
    //  functions.
    //
    BreakArrayKernel32[0].funcName = "Beep";
    BreakArrayKernel32[0].nargs = 2;
    BreakArrayKernel32[0].breakProc = OnBeep;
    BreakArrayKernel32[0].dwFlags = BREAK_OUT|BREAK_IN;

    BreakArrayKernel32[1].funcName = "FillConsoleOutputCharacterA";
    BreakArrayKernel32[1].nargs = 5;
    BreakArrayKernel32[1].breakProc = OnFillConsoleOutputCharacter;
    BreakArrayKernel32[1].dwFlags = BREAK_OUT;

    BreakArrayKernel32[2].funcName = "FillConsoleOutputCharacterW";
    BreakArrayKernel32[2].nargs = 5;
    BreakArrayKernel32[2].breakProc = OnFillConsoleOutputCharacter;
    BreakArrayKernel32[2].dwFlags = BREAK_OUT;

    BreakArrayKernel32[3].funcName = "GetStdHandle";
    BreakArrayKernel32[3].nargs = 1;
    BreakArrayKernel32[3].breakProc = OnGetStdHandle;
    BreakArrayKernel32[3].dwFlags = BREAK_OUT;

    BreakArrayKernel32[4].funcName = "OpenConsoleW";
    BreakArrayKernel32[4].nargs = 4;
    BreakArrayKernel32[4].breakProc = OnOpenConsoleW;
    BreakArrayKernel32[4].dwFlags = BREAK_OUT;

    BreakArrayKernel32[5].funcName = "ReadConsoleInputA";
    BreakArrayKernel32[5].nargs = 4;
    BreakArrayKernel32[5].breakProc = OnReadConsoleInput;
    BreakArrayKernel32[5].dwFlags = BREAK_OUT;

    BreakArrayKernel32[6].funcName = "ReadConsoleInputW";
    BreakArrayKernel32[6].nargs = 4;
    BreakArrayKernel32[6].breakProc = OnReadConsoleInput;
    BreakArrayKernel32[6].dwFlags = BREAK_OUT;

    BreakArrayKernel32[7].funcName = "ScrollConsoleScreenBufferA";
    BreakArrayKernel32[7].nargs = 5;
    BreakArrayKernel32[7].breakProc = OnScrollConsoleScreenBuffer;
    BreakArrayKernel32[7].dwFlags = BREAK_OUT;

    BreakArrayKernel32[8].funcName = "ScrollConsoleScreenBufferW";
    BreakArrayKernel32[8].nargs = 5;
    BreakArrayKernel32[8].breakProc = OnScrollConsoleScreenBuffer;
    BreakArrayKernel32[8].dwFlags = BREAK_OUT;

    BreakArrayKernel32[9].funcName = "SetConsoleMode";
    BreakArrayKernel32[9].nargs = 2;
    BreakArrayKernel32[9].breakProc = OnSetConsoleMode;
    BreakArrayKernel32[9].dwFlags = BREAK_OUT;

    BreakArrayKernel32[10].funcName = "SetConsoleActiveScreenBuffer";
    BreakArrayKernel32[10].nargs = 1;
    BreakArrayKernel32[10].breakProc = OnSetConsoleActiveScreenBuffer;
    BreakArrayKernel32[10].dwFlags = BREAK_OUT;

    BreakArrayKernel32[11].funcName = "SetConsoleCursorPosition";
    BreakArrayKernel32[11].nargs = 2;
    BreakArrayKernel32[11].breakProc = OnSetConsoleCursorPosition;
    BreakArrayKernel32[11].dwFlags = BREAK_OUT;

    BreakArrayKernel32[12].funcName = "SetConsoleWindowInfo";
    BreakArrayKernel32[12].nargs = 2;
    BreakArrayKernel32[12].breakProc = OnSetConsoleWindowInfo;
    BreakArrayKernel32[12].dwFlags = BREAK_OUT;

    BreakArrayKernel32[13].funcName = "WriteConsoleA";
    BreakArrayKernel32[13].nargs = 5;
    BreakArrayKernel32[13].breakProc = OnWriteConsoleA;
    BreakArrayKernel32[13].dwFlags = BREAK_OUT;

    BreakArrayKernel32[14].funcName = "WriteConsoleW";
    BreakArrayKernel32[14].nargs = 5;
    BreakArrayKernel32[14].breakProc = OnWriteConsoleW;
    BreakArrayKernel32[14].dwFlags = BREAK_OUT;

    BreakArrayKernel32[15].funcName = "WriteConsoleOutputA";
    BreakArrayKernel32[15].nargs = 5;
    BreakArrayKernel32[15].breakProc = OnWriteConsoleOutputA;
    BreakArrayKernel32[15].dwFlags = BREAK_OUT;

    BreakArrayKernel32[16].funcName = "WriteConsoleOutputW";
    BreakArrayKernel32[16].nargs = 5;
    BreakArrayKernel32[16].breakProc = OnWriteConsoleOutputW;
    BreakArrayKernel32[16].dwFlags = BREAK_OUT;

    BreakArrayKernel32[17].funcName = "WriteConsoleOutputCharacterA";
    BreakArrayKernel32[17].nargs = 5;
    BreakArrayKernel32[17].breakProc = OnWriteConsoleOutputCharacterA;
    BreakArrayKernel32[17].dwFlags = BREAK_OUT;

    BreakArrayKernel32[18].funcName = "WriteConsoleOutputCharacterW";
    BreakArrayKernel32[18].nargs = 5;
    BreakArrayKernel32[18].breakProc = OnWriteConsoleOutputCharacterW;
    BreakArrayKernel32[18].dwFlags = BREAK_OUT;

    BreakArrayKernel32[19].funcName = 0L;
    BreakArrayKernel32[19].nargs = 0;
    BreakArrayKernel32[19].breakProc = 0L;
    BreakArrayKernel32[19].dwFlags = 0;

    BreakArrayUser32[0].funcName = "IsWindowVisible";
    BreakArrayUser32[0].nargs = 1;
    BreakArrayUser32[0].breakProc = OnIsWindowVisible;
    BreakArrayUser32[0].dwFlags = BREAK_OUT;

    BreakArrayUser32[1].funcName = 0L;
    BreakArrayUser32[1].nargs = 0;
    BreakArrayUser32[1].breakProc = 0L;
    BreakArrayUser32[1].dwFlags = 0;

    BreakPoints[0].dllName = "kernel32.dll";
    BreakPoints[0].breakInfo = BreakArrayKernel32;

    BreakPoints[1].dllName = "user32.dll";
    BreakPoints[1].breakInfo = BreakArrayUser32;

    BreakPoints[2].dllName = 0L;
    BreakPoints[2].breakInfo = 0L;
}

unsigned
ConsoleDebugger::ThreadHandlerProc(void)
{
    //ExpSlaveDebugArg *arg = (ExpSlaveDebugArg *) lparg;
    Process *proc;
    DWORD result;
    HANDLE process;
    DWORD pid;

    //HConsole = arg->hConsole;
    //HMaster = arg->hMaster;		/* Set the master program */
    //UseSocket = arg->useSocket;

    /* Make sure the master does not ignore Ctrl-C */
    SetConsoleCtrlHandler(0L, FALSE);

    result = ExpWinCreateProcess(
	    _argc,
	    _argv,
	    NULL,
	    NULL,
	    NULL,
	    FALSE,
	    FALSE,
	    TRUE, /* debug */
	    TRUE, /* newProcessGroup */
	    &process,
	    &pid);

    if (result) {
	//arg->lastError = GetLastError();
    }

    /* Make sure we now ignore Ctrl-C */
    SetConsoleCtrlHandler(0L, TRUE);
    //SetEvent(arg->event);

    if (result) {
	return 0;
    }

    proc = ProcessNew();
//    proc->hPid = arg->globalPid;
//    if (arg->passThrough) {
//	ExpProcess *proc;
//
//	proc = ExpProcessNew();
//	proc->overlapped.hEvent = CreateEvent(NULL, TRUE, FALSE, NULL);
//	proc->hProcess = arg->process;
//	ExpAddToWaitQueue(proc->hProcess);
//    } else {
	CloseHandle(process);
	proc->hProcess = process;
//	arg->process = proc->hProcess;
	if (proc->hProcess == 0L) {
	    //arg->lastError = GetLastError();
	    return 0;
	}
//	ExpAddToWaitQueue(proc->hProcess);
//	proc->overlapped.hEvent = CreateEvent(NULL, TRUE, FALSE, NULL);
	CommonDebugger();
//    }

    return 0;			/* Never executes */
}

/*
 *----------------------------------------------------------------------
 *
 * ConsoleDebugger::ProcessNew --
 *
 *	Allocates a new structure for debugging a process and
 *	initializes it.
 *
 * Results:
 *	A new structure
 *
 * Side Effects:
 *	Memory is allocated, an event is created.
 *
 *----------------------------------------------------------------------
 */

ConsoleDebugger::Process *
ConsoleDebugger::ProcessNew(void)
{
    Process *proc;
    proc = new Process;
    proc->threadList = NULL;
    proc->threadCount = 0;
    proc->brkptList = NULL;
    proc->lastBrkpt = NULL;
    proc->offset = 0;
    proc->nBreakCount = 0;
    proc->consoleHandlesMax = 0;
    proc->isConsoleApp = FALSE;
    proc->isShell = FALSE;
    proc->hProcess = NULL;
    proc->pSubprocessMemory = 0;
    proc->pSubprocessBuffer = 0;
    proc->pMemoryCacheBase = 0;
    proc->funcTable = new Tcl_HashTable;
    Tcl_InitHashTable(proc->funcTable, TCL_STRING_KEYS);
    proc->moduleTable = new Tcl_HashTable;
    Tcl_InitHashTable(proc->moduleTable, TCL_ONE_WORD_KEYS);
    proc->exeModule = NULL;
    proc->nextPtr = ProcessList;
    ProcessList = proc;
    return proc;
}

/*
 *----------------------------------------------------------------------
 *
 * ConsoleDebugger::ProcessFree --
 *
 *	Frees all allocated memory for a process and closes any
 *	open handles
 *
 * Results:
 *	None
 *
 *----------------------------------------------------------------------
 */

void
ConsoleDebugger::ProcessFree(Process *proc)
{
    ThreadInfo *tcurr, *tnext;
    Breakpoint *bcurr, *bnext;
    Process *pcurr, *pprev;
    
    for (tcurr = proc->threadList; tcurr != NULL; tcurr = tnext) {
	tnext = tcurr->nextPtr;
	proc->threadCount--;
	CloseHandle(tcurr->hThread);
	delete tcurr;
    }
    for (bcurr = proc->brkptList; bcurr != NULL; bcurr = bnext) {
	bnext = bcurr->nextPtr;
	delete bcurr;
    }
    Tcl_DeleteHashTable(proc->funcTable);
    delete proc->funcTable;
    Tcl_DeleteHashTable(proc->moduleTable);
    delete proc->moduleTable;

    for (pprev = NULL, pcurr = ProcessList; pcurr != NULL;
	 pcurr = pcurr->nextPtr)
    {
	if (pcurr == proc) {
	    if (pprev == NULL) {
		ProcessList = pcurr->nextPtr;
	    } else {
		pprev->nextPtr = pcurr->nextPtr;
	    }
	    break;
	}
    }

    delete proc;
}

/*
 *----------------------------------------------------------------------
 *
 * ConsoleDebugger::CommonDebugger --
 *
 *	This is the function that is the debugger for all slave processes
 *
 * Results:
 *	None.  This thread exits with ExitThread() when the subprocess dies.
 *
 * Side Effects:
 *	Adds the process to the things being waited for by
 *	WaitForMultipleObjects
 *
 *----------------------------------------------------------------------
 */
void
ConsoleDebugger::CommonDebugger()
{
    DEBUG_EVENT debEvent;	// debugging event info.
    DWORD dwContinueStatus;	// exception continuation.
    DWORD err;
    Process *proc;
    DWORD n, i;

    n = GetEnvironmentVariable("Path", NULL, 0);
    n += GetEnvironmentVariable("_NT_SYMBOL_PATH", NULL, 0) + 1;
    n += GetEnvironmentVariable("_NT_ALT_SYMBOL_PATH", NULL, 0) + 1;
    n += GetEnvironmentVariable("SystemRoot", NULL, 0) + 1;

    SymbolPath = new char [n];

    i = GetEnvironmentVariable("Path", SymbolPath, n);
    SymbolPath[i++] = ';';
    i += GetEnvironmentVariable("_NT_SYMBOL_PATH", &SymbolPath[i], n-i);
    SymbolPath[i++] = ';';
    i += GetEnvironmentVariable("_NT_ALT_SYMBOL_PATH", &SymbolPath[i], n-i);
    SymbolPath[i++] = ';';
    i += GetEnvironmentVariable("SystemRoot", &SymbolPath[i], n-i);

    for(;;) {
	dwContinueStatus = DBG_CONTINUE;

	/*
	 * Wait for a debugging event to occur. The second parameter
	 * indicates that the function does not return until
	 * a debugging event occurs.
	 */

	if (WaitForDebugEvent(&debEvent, INFINITE) == FALSE) {
	    err = GetLastError();
	    *((char *) 0L) = 0;   // cause an exception.
	}

	/*
	 * Find the process that is responsible for this event.
	 */
	for (proc = ProcessList; proc; proc = proc->nextPtr) {
	    if (proc->hPid == debEvent.dwProcessId) {
		break;
	    }
	}

	if (!proc && debEvent.dwDebugEventCode != CREATE_PROCESS_DEBUG_EVENT) {
	    char buf[50];
	    wsprintfA(buf, "%d/%d (%d)", 
		    debEvent.dwProcessId, debEvent.dwThreadId,
		    debEvent.dwDebugEventCode);
//	    EXP_LOG1(MSG_DT_UNEXPECTEDDBGEVENT, buf);
	    EXP_LOG("Unexpected debug event for %s", buf);
	    if (debEvent.dwDebugEventCode == EXCEPTION_DEBUG_EVENT) {
//		char buf[50];
//		wsprintfA(buf, "0x%08x", debEvent.u.Exception.ExceptionRecord.ExceptionCode);
//		EXP_LOG1(MSG_DT_EXCEPTIONDBGEVENT, buf);
		EXP_LOG("ExceptionCode: 0x%08x",
			debEvent.u.Exception.ExceptionRecord.ExceptionCode);
		dwContinueStatus = DBG_EXCEPTION_NOT_HANDLED;
	    }
	    goto skip;
	}

	/* Process the debugging event code. */

	switch (debEvent.dwDebugEventCode) {
	case EXCEPTION_DEBUG_EVENT:
	    /*
	     * Process the exception code. When handling
	     * exceptions, remember to set the continuation
	     * status parameter (dwContinueStatus). This value
	     * is used by the ContinueDebugEvent function.
	     */

	    switch (debEvent.u.Exception.ExceptionRecord.ExceptionCode) {
	    case EXCEPTION_BREAKPOINT:
	    {
		if (proc->nBreakCount < 1000) {
		    proc->nBreakCount++;
		}
		if (proc->nBreakCount == 1) {
		    OnXFirstBreakpoint(proc, &debEvent);
		} else if (proc->nBreakCount == 2) {
		    OnXSecondBreakpoint(proc, &debEvent);
		} else {
		    OnXBreakpoint(proc, &debEvent);
		}
		break;
	    }

	    case EXCEPTION_SINGLE_STEP:
		OnXSingleStep(proc, &debEvent);
		break;

	    case DBG_CONTROL_C:
		dwContinueStatus = DBG_EXCEPTION_NOT_HANDLED;
		break;

	    case DBG_CONTROL_BREAK:
		dwContinueStatus = DBG_EXCEPTION_NOT_HANDLED;
		break;

	    case EXCEPTION_DATATYPE_MISALIGNMENT:
	    case EXCEPTION_ACCESS_VIOLATION:
	    default:
		/*
		 * An exception was hit and it was not handled by the program.
		 * Now it is time to get a backtrace.
		 */
		if (! debEvent.u.Exception.dwFirstChance) {
		    OnXSecondChanceException(proc, &debEvent);
		}
		dwContinueStatus = DBG_EXCEPTION_NOT_HANDLED;
	    }
	    break;

	case CREATE_THREAD_DEBUG_EVENT:
	    OnXCreateThread(proc, &debEvent);
	    break;

	case CREATE_PROCESS_DEBUG_EVENT:
	    OnXCreateProcess(proc, &debEvent);
	    break;

	case EXIT_THREAD_DEBUG_EVENT:
	    OnXDeleteThread(proc, &debEvent);
	    break;

	case EXIT_PROCESS_DEBUG_EVENT:
	    /*
	     * XXX: This is really screwed up, but we get breakpoints
	     * for processes that are already dead.  So we cannot remove
	     * and cleanup a process until some later (How much later?)
	     * point.  This really, really sucks....
	     */
//	    CloseHandle(proc->overlapped.hEvent);
#if 0 /* This gets closed in WaitQueueThread */
	    CloseHandle(proc->hProcess);
#endif
	    err = debEvent.u.ExitProcess.dwExitCode;
	    ProcessFree(proc);
	    if (ProcessList == NULL) {
		// When the last process exits, we exit.
		return;
	    }
	    break;

	case LOAD_DLL_DEBUG_EVENT:
	    OnXLoadDll(proc, &debEvent);
	    break;

	case UNLOAD_DLL_DEBUG_EVENT:
	    OnXUnloadDll(proc, &debEvent);
	    break;

	case OUTPUT_DEBUG_STRING_EVENT:
	    // Display the output debugging string.
	    break;
	}

    skip:
	// Resume executing the thread that reported the debugging event.
	ContinueDebugEvent(debEvent.dwProcessId, debEvent.dwThreadId,
		dwContinueStatus);
    }
}

/*
 *----------------------------------------------------------------------
 *
 * ConsoleDebugger::SetBreakpoint --
 *
 *	Inserts a single breakpoint
 *
 * Results:
 *	TRUE if successful, FALSE if unsuccessful.
 *
 *----------------------------------------------------------------------
 */

BOOL
ConsoleDebugger::SetBreakpoint(Process *proc, BreakInfo *info)
{
    Tcl_HashEntry *tclEntry;
    PVOID funcPtr;

    tclEntry = Tcl_FindHashEntry(proc->funcTable, info->funcName);
    if (tclEntry == NULL) {
//	EXP_LOG("Unable to set breakpoint at %s", info->funcName);
	return FALSE;
    }

#if 0
    fprintf(stderr, "%s: ", info->funcName);
#endif
    /*
     * Set a breakpoint at the function start in the subprocess and
     * save the original code at the function start.
     */
    funcPtr = Tcl_GetHashValue(tclEntry);
    SetBreakpointAtAddr(proc, info, funcPtr);
    return TRUE;
}

/*
 *----------------------------------------------------------------------
 *
 * ConsoleDebugger::SetBreakpointAtAddr --
 *
 *	Inserts a single breakpoint at the given address
 *
 * Results:
 *	TRUE if successful, FALSE if unsuccessful.
 *
 *----------------------------------------------------------------------
 */

ConsoleDebugger::Breakpoint *
ConsoleDebugger::SetBreakpointAtAddr(Process *proc, BreakInfo *info, PVOID funcPtr)
{
    Breakpoint *bpt;
    UCHAR code;

#if 0
    fprintf(stderr, "SetBreakpointAtAddr: addr=0x%08x\n", funcPtr);
#endif
    bpt = new Breakpoint;
    bpt->returning = FALSE;
    bpt->codePtr = funcPtr;
    bpt->codeReturnPtr = (PVOID) (proc->offset + (DWORD) proc->pSubprocessMemory);
    bpt->origRetAddr = 0;
    bpt->breakInfo = info;
    bpt->threadInfo = NULL;
    proc->offset += 2;
    bpt->nextPtr = proc->brkptList;
    proc->brkptList = bpt;

    ReadSubprocessMemory(proc, funcPtr, &bpt->code, sizeof(UCHAR));
#ifdef _M_IX86
    // Breakpoint opcode on i386
    code = 0xcc;
#else
#   error "need breakpoint opcode for this hardware"
#endif
    WriteSubprocessMemory(proc, funcPtr, &code, sizeof(UCHAR));
    return bpt;
}

/*
 *----------------------------------------------------------------------
 *
 * ConsoleDebugger::OnXFirstBreakpoint --
 *
 *	This routine is called when a EXCEPTION_DEBUG_EVENT with
 *	an exception code of EXCEPTION_BREAKPOINT, and it is the
 *	first one to occur in the program.  This happens when the
 *	process finally gets loaded into memory and is about to
 *	start.
 *
 * Results:
 *	None
 *
 *----------------------------------------------------------------------
 */

static CONTEXT FirstContext;
static UCHAR   FirstPage[PAGESIZE];
static HANDLE  FirstThread;
#include <pshpack1.h>
typedef struct _InjectCode {
    UCHAR instPush1;
    DWORD argMemProtect;
    UCHAR instPush2;
    DWORD argMemType;
    UCHAR instPush3;
    DWORD argMemSize;
    UCHAR instPush4;
    DWORD argMemAddr;
    UCHAR instCall;
    DWORD argCallAddr;
    DWORD instIntr;
} InjectCode;
#include <poppack.h>

void
ConsoleDebugger::OnXFirstBreakpoint(Process *proc, LPDEBUG_EVENT pDebEvent)
{
    DWORD base;
    ThreadInfo *tinfo;

#if 0
    fprintf(stderr, "OnXFirstBreakpoint: proc=0x%08x\n", proc);
#endif
    for (tinfo = proc->threadList; tinfo != NULL; tinfo = tinfo->nextPtr) {
	if (pDebEvent->dwThreadId == tinfo->dwThreadId) {
	    break;
	}
    }

    /*
     * Set up the memory that will serve as the place for our
     * intercepted function return points.
     */

    {
	InjectCode code;
	Tcl_HashEntry *tclEntry;
	DWORD addr;

	FirstThread = tinfo->hThread;
	FirstContext.ContextFlags = CONTEXT_FULL;
	GetThreadContext(FirstThread, &FirstContext);

	tclEntry = Tcl_FindHashEntry(proc->funcTable, "VirtualAlloc");
	if (tclEntry == 0L) {
	    proc->nBreakCount++;	/* Don't stop at second breakpoint */
//	    EXP_LOG0(MSG_DT_NOVIRT);
	    EXP_LOG("Unable to find entry for VirtualAlloc", NULL);
	    return;
	}
	addr = (DWORD) Tcl_GetHashValue(tclEntry);

	code.instPush1     = 0x68;
	code.argMemProtect = PAGE_EXECUTE_READWRITE;
	code.instPush2     = 0x68;
	code.argMemType    = MEM_COMMIT;
	code.instPush3     = 0x68;
	code.argMemSize    = 2048;
	code.instPush4     = 0x68;
	code.argMemAddr    = 0;
	code.instCall      = 0xe8;
	code.argCallAddr   = addr - FirstContext.Eip - offsetof(InjectCode, instCall) - 5;
	code.instIntr      = 0xCC;

	base = FirstContext.Eip;
	if (!ReadSubprocessMemory(proc, (PVOID) base, FirstPage, sizeof(InjectCode))) {
//	    EXP_LOG0(MSG_DT_CANTREADSPMEM);
	    EXP_LOG("Error reading subprocess memory", NULL);
	}
	if (!WriteSubprocessMemory(proc, (PVOID) base, &code, sizeof(InjectCode))) {
//	    EXP_LOG0(MSG_DT_CANTWRITESPMEM);
    	    EXP_LOG("Error writing subprocess memory", NULL);
	}
    }
    return;
}

/*
 *----------------------------------------------------------------------
 *
 * ConsoleDebugger::OnXSecondBreakpoint --
 *
 *	This routine is called when the second breakpoint is hit.
 *	The second breakpoint is at the end of our call to GlobalAlloc().
 *	Save the returned pointer from GlobalAlloc, then restore the
 *	first page of memory and put everything back the way it was.
 *	Finally, we can start.
 *
 * Results:
 *	None
 *
 *----------------------------------------------------------------------
 */

void
ConsoleDebugger::OnXSecondBreakpoint(Process *proc, LPDEBUG_EVENT pDebEvent)
{
    CONTEXT context;
    UCHAR retbuf[2048];
    DWORD base;
    LPEXCEPTION_DEBUG_INFO exceptInfo;
    BreakInfo *info;
    int i;

    exceptInfo = &pDebEvent->u.Exception;

    context.ContextFlags = CONTEXT_FULL;
    GetThreadContext(FirstThread, &context);
    proc->pSubprocessMemory = context.Eax;

    memset(retbuf, 0xcc, sizeof(retbuf));	/* All breakpoints */
    WriteSubprocessMemory(proc, (PVOID) proc->pSubprocessMemory,
			  retbuf, sizeof(retbuf));

    base = FirstContext.Eip;
    if (!WriteSubprocessMemory(proc, (PVOID) base, FirstPage, sizeof(InjectCode))) {
//	EXP_LOG0(MSG_DT_CANTWRITESPMEM);
	EXP_LOG("Error writing subprocess memory", NULL);
    }
    SetThreadContext(FirstThread, &FirstContext);

    /*
     * Set all breakpoints
     */
    for (i = 0; BreakPoints[i].dllName; i++) {
	for (info = BreakPoints[i].breakInfo; info->funcName; info++) {
	    SetBreakpoint(proc, info);
	}
    }
}


/*
 *----------------------------------------------------------------------
 *
 * ConsoleDebugger::ReadSubprocessMemory --
 *
 *	Reads memory from the subprocess.  Takes care of all the
 *	issues with page protection.
 *
 * Results:
 *	FALSE if unsuccessful, TRUE if successful.
 *
 * Notes:
 *	The efficient memory reading routine is disabled here
 *	because it doesn't quite work right.  I don't see the
 *	problem in the code, but there must be something there
 *	since the test suite fails when run with this code
 *	enabled.  When it works, it should be much faster than
 *	the current safe but slow implementation.
 *
 *----------------------------------------------------------------------
 */

#if 0
BOOL
ConsoleDebugger::ReadSubprocessMemory(ExpProcess *proc, LPVOID addr, LPVOID buf, DWORD len)
{
    DWORD oldProtection = 0;
    MEMORY_BASIC_INFORMATION mbi;
    BOOL ret = TRUE;
    DWORD offset;
    DWORD base, curr, end, n;
    HANDLE hProcess;
    PBYTE bufpos = buf;

    hProcess = proc->hProcess;

    end = len + (DWORD) addr;
    for (curr = (DWORD) addr; curr < end; ) {
	base = curr & (~PAGEMASK);
	offset = curr & PAGEMASK;
	if (offset + len > PAGESIZE) {
	    n = PAGESIZE - offset;
	} else {
	    n = len;
	}
	if (proc->pMemoryCacheBase != (curr & PAGEMASK)) {
	    /* if not committed memory abort */
	    if (!VirtualQueryEx(hProcess, (LPVOID) base, &mbi, sizeof(mbi)) ||
		(mbi.State != MEM_COMMIT))
	    {
		return FALSE;
	    }

	    /* if guarded memory, change protection temporarily */
	    if (!(mbi.Protect & PAGE_READONLY) &&
		!(mbi.Protect & PAGE_READWRITE))
	    {
		VirtualProtectEx(hProcess, (LPVOID) base, PAGESIZE,
		    PAGE_READONLY, &oldProtection);
	    }

	    if (!ReadProcessMemory(hProcess, (LPVOID) base, proc->pMemoryCache,
		PAGESIZE, NULL)) {
		ret = FALSE;
	    }
    
	    /* reset protection if changed */
	    if (oldProtection) {
		VirtualProtectEx(hProcess, (LPVOID) base, PAGESIZE,
		    oldProtection, &oldProtection);
	    }
	    if (ret == FALSE) {
		return FALSE;
	    }
	    proc->pMemoryCacheBase = base;
	}

	memcpy(bufpos, &proc->pMemoryCache[offset], n);
	bufpos += n;
	curr += n;
    }

    return ret;
}

#else
BOOL
ConsoleDebugger::ReadSubprocessMemory(Process *proc, LPVOID addr, LPVOID buf, DWORD len)
{
    DWORD oldProtection = 0;
    MEMORY_BASIC_INFORMATION mbi;
    BOOL ret;
    LONG error;

    /* if not committed memory abort */
    if (!VirtualQueryEx(proc->hProcess, addr, &mbi, sizeof(mbi)) ||
	mbi.State != MEM_COMMIT)
    {
	return FALSE;
    }
    
    /* if guarded memory, change protection temporarily */
    if (!(mbi.Protect & PAGE_READONLY) && !(mbi.Protect & PAGE_READWRITE)) {
	VirtualProtectEx(proc->hProcess, addr, len, PAGE_READONLY, &oldProtection);
    }
    
    ret = ReadProcessMemory(proc->hProcess, addr, buf, len, NULL);
    if (ret == FALSE) {
	error = GetLastError();
    }
    
    /* reset protection if changed */
    if (oldProtection) {
	VirtualProtectEx(proc->hProcess, addr, len, oldProtection, &oldProtection);
	SetLastError(error);
    }
    return ret;
}
#endif /* XXX */

/*
 *----------------------------------------------------------------------
 *
 * ConsoleDebugger::WriteSubprocessMemory --
 *
 *	Writes memory from the subprocess.  Takes care of all the
 *	issues with page protection.
 *
 * Results:
 *	0 if unsuccessful, 1 if successful.
 *
 *----------------------------------------------------------------------
 */

BOOL
ConsoleDebugger::WriteSubprocessMemory(Process *proc, LPVOID addr, LPVOID buf, DWORD len)
{
    DWORD oldProtection = 0;
    MEMORY_BASIC_INFORMATION mbi;
    BOOL ret = TRUE;
    DWORD err;
    HANDLE hProcess;

    hProcess = proc->hProcess;

    /* Flush the read cache */
    proc->pMemoryCacheBase = 0;

    /* if not committed memory abort */
    if (!VirtualQueryEx(hProcess, addr, &mbi, sizeof(mbi)) ||
	mbi.State != MEM_COMMIT)
    {
	ret = FALSE;
	/* assert(ret != FALSE); */
	return ret;
    }
    
    /* if guarded memory, change protection temporarily */
    if (!(mbi.Protect & PAGE_READWRITE)) {
	if (!VirtualProtectEx(hProcess, addr, len, PAGE_READWRITE,
			      &oldProtection)) {
	    err = GetLastError();
	}
    }
    
    if (!WriteProcessMemory(hProcess, addr, buf, len, NULL)) {
	ret = FALSE;
	err = GetLastError();
    }
    
    /* reset protection if changed */
    if (oldProtection) {
	VirtualProtectEx(hProcess, addr, len, oldProtection, &oldProtection);
    }
#if 0 /* Debugging purposes only */
    if (ret == FALSE) {
	assert(ret != FALSE); 
    }
#endif
    return ret;
}

/*
 *----------------------------------------------------------------------
 *
 * ConsoleDebugger::OnXBreakpoint --
 *
 *	This routine is called when a EXCEPTION_DEBUG_EVENT with
 *	an exception code of EXCEPTION_BREAKPOINT.
 *
 * Results:
 *	None
 *
 *----------------------------------------------------------------------
 */

void
ConsoleDebugger::OnXBreakpoint(Process *proc, LPDEBUG_EVENT pDebEvent)
{
    LPEXCEPTION_DEBUG_INFO exceptInfo;
    CONTEXT context;
    ThreadInfo *tinfo;
    Breakpoint *pbrkpt, *brkpt;
    PDWORD pdw;
    DWORD i;
    DWORD dw;

    for (tinfo = proc->threadList; tinfo != 0L; tinfo = tinfo->nextPtr) {
	if (pDebEvent->dwThreadId == tinfo->dwThreadId) {
	    break;
	}
    }
    assert(tinfo != 0L);

    exceptInfo = &pDebEvent->u.Exception;
#if 0
    fprintf(stderr, "OnXBreakpoint: proc=0x%x, count=%d, addr=0x%08x\n", proc, proc->nBreakCount, exceptInfo->ExceptionRecord.ExceptionAddress);
#endif

    pbrkpt = 0L;
    for (brkpt = proc->brkptList; brkpt != 0L;
	 pbrkpt = brkpt, brkpt = brkpt->nextPtr) {
	if (brkpt->codePtr == exceptInfo->ExceptionRecord.ExceptionAddress) {
	    if (brkpt->threadInfo == 0L) {
		break;
	    }
	    if (brkpt->threadInfo == tinfo) {
		break;
	    }
	}
    }
#if 0 /* Allow user breakpoints to be hit when we are not debugging */
    if (brkpt == 0L) {
	fprintf(stderr, "OnXBreakpoint: proc=0x%x, count=%d, addr=0x%08x\n", proc, proc->nBreakCount, exceptInfo->ExceptionRecord.ExceptionAddress);
    }
    assert(brkpt != 0L);
#endif

    context.ContextFlags = CONTEXT_FULL;
    GetThreadContext(tinfo->hThread, &context);

    if (! brkpt->returning) {
	Breakpoint *bpt;
	/*
	 * Get the arguments to the function and store them in the thread
	 * specific data structure.
	 */
	for (pdw = tinfo->args, i=0; i < brkpt->breakInfo->nargs; i++, pdw++) {
	    ReadSubprocessMemory(proc, (PVOID) (context.Esp+(4*(i+1))),
				 pdw, sizeof(DWORD));
	}
	tinfo->nargs = brkpt->breakInfo->nargs;
	tinfo->context = &context;

	if (brkpt->breakInfo->dwFlags & BREAK_IN) {
	    ((this)->*(brkpt->breakInfo->breakProc))(proc, tinfo, brkpt, &context.Eax, BREAK_IN);
	}

	/*
	 * Only set a return breakpoint if something is interested
	 * in the return value
	 */
	if (brkpt->breakInfo->dwFlags & BREAK_OUT) {
	    bpt = new Breakpoint;
	    ReadSubprocessMemory(proc, (PVOID) context.Esp,
		&bpt->origRetAddr, sizeof(DWORD));
	    dw = (DWORD) brkpt->codeReturnPtr;
	    WriteSubprocessMemory(proc, (PVOID) context.Esp,
		&dw, sizeof(DWORD));
	    bpt->codePtr = brkpt->codeReturnPtr;
	    bpt->returning = TRUE;
	    bpt->codeReturnPtr = NULL;	/* Doesn't matter */
	    bpt->breakInfo = brkpt->breakInfo;
	    bpt->threadInfo = tinfo;
	    bpt->nextPtr = proc->brkptList;
	    proc->brkptList = bpt;

	}

	/*
	 * Now, we need to restore the original code for this breakpoint.
	 * Put the program counter back, then do a single-step and put
	 * the breakpoint back again.
	 */
	WriteSubprocessMemory(proc, brkpt->codePtr, &brkpt->code, sizeof(UCHAR));

	context.EFlags |= SINGLE_STEP_BIT;
	context.Eip--;

	proc->lastBrkpt = brkpt;
    } else {
	/*
	 * Make the callback with the params and the return value
	 */
	if (brkpt->breakInfo->dwFlags & BREAK_OUT) {
	    ((this)->*(brkpt->breakInfo->breakProc))(proc, tinfo, brkpt, &context.Eax, BREAK_OUT);
	}
	context.Eip = brkpt->origRetAddr;

	if (pbrkpt == 0L) {
	    proc->brkptList = brkpt->nextPtr;
	} else {
	    pbrkpt->nextPtr = brkpt->nextPtr;
	}
	delete brkpt;
    }
    SetThreadContext(tinfo->hThread, &context);
}

/*
 *----------------------------------------------------------------------
 *
 * ReadSubprocessStringA --
 *
 *	Read a character string from the subprocess
 *
 * Results:
 *	The length of the string
 *
 *----------------------------------------------------------------------
 */

int
ConsoleDebugger::ReadSubprocessStringA(Process *proc, PVOID base,
    PCHAR buf, int buflen)
{
    CHAR *ip, *op;
    int i;
    
    ip = static_cast<CHAR *>(base);
    op = buf;
    i = 0;
    while (i < buflen-1) {
	if (! ReadSubprocessMemory(proc, ip, op, sizeof(CHAR))) {
	    break;
	}
	if (*op == 0) break;
	op++; ip++; i++;
    }
    *op = 0;
    return i;
}

/*
 *----------------------------------------------------------------------
 *
 * ConsoleDebugger::ReadSubprocessStringW --
 *
 *	Read a character string from the subprocess
 *
 * Results:
 *	The length of the string
 *
 *----------------------------------------------------------------------
 */

int
ConsoleDebugger::ReadSubprocessStringW(Process *proc, PVOID base,
    PWCHAR buf, int buflen)
{
    WCHAR *ip, *op;
    int i;
    
    ip = static_cast<WCHAR *>(base);
    op = buf;
    i = 0;
    while (i < buflen-1) {
	if (! ReadSubprocessMemory(proc, ip, op, sizeof(WCHAR))) {
	    break;
	}
	if (*op == 0) break;
	op++; ip++; i++;
    }
    *op = 0;
    return i;
}
