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
 * RCS: @(#) $Id: expWinConsoleDebugger.cpp,v 1.1.2.32 2003/08/25 23:17:49 davygrvy Exp $
 * ----------------------------------------------------------------------------
 */

#include <stddef.h>
#include <assert.h>
#include "expWinConsoleDebugger.hpp"
#include "expWinInjectorIPC.hpp"
#include <vdmdbg.h>
#pragma comment (lib, "vdmdbg.lib")

#ifdef _MSC_VER
#   pragma comment (lib, "imagehlp.lib")
#endif

#ifdef _M_IX86
    // Breakpoint opcode on i386
#   define BRK_OPCODE	    0xCC
    // Single step flag
#   define SINGLE_STEP_BIT  0x100
#else
#   error need opcodes for this hardware.
#endif



//  Constructor.
ConsoleDebugger::ConsoleDebugger (
	int _argc,
	char * const *_argv,
	CMclQueue<Message *> &_mQ
    )
    : argc(_argc), argv(_argv), ProcessList(0L), CursorKnown(FALSE),
    ConsoleOutputCP(0), ConsoleCP(0), mQ(_mQ), pStartAddress(0L),
    originalExeEntryPointOpcode(0), pInjectorStub(0), injectorIPC(0L),
    interacting(false), interactingConsole(0L)
{
    OSVERSIONINFO osvi;
    DWORD n, i;

    osvi.dwOSVersionInfoSize = sizeof(OSVERSIONINFO);
    GetVersionEx(&osvi);
    dwPlatformId = osvi.dwPlatformId;

    if (dwPlatformId == VER_PLATFORM_WIN32_NT) {
	pfnVirtualAllocEx = (PFNVIRTUALALLOCEX) GetProcAddress(
		GetModuleHandle("KERNEL32.DLL"),"VirtualAllocEx");
	pfnVirtualFreeEx = (PFNVIRTUALFREEEX) GetProcAddress(
		GetModuleHandle("KERNEL32.DLL"),"VirtualFreeEx");
    }

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

    //  Until further notice, assume this.
    //
    ConsoleSize.X = 80;
    ConsoleSize.Y = 25;

    //  Until further notice, assume this, too.
    //
    CursorPosition.X = 0;
    CursorPosition.Y = 0;

    //  Set all our breakpoint info.  We have to do this the long way, here in
    //  the constructor, because we need to have the 'this' pointer already
    //  set because we refer to ourselves by needing the address to member
    //  functions.
    //

    // interesting ones I might want to add:
    // DuplicateConsoleHandle

    BreakArrayKernel32[0].funcName = "AllocConsole";
    BreakArrayKernel32[0].nargs = 0;
    BreakArrayKernel32[0].breakProc = OnAllocConsole;
    BreakArrayKernel32[0].dwFlags = BREAK_OUT;

    BreakArrayKernel32[1].funcName = "Beep";
    BreakArrayKernel32[1].nargs = 2;
    BreakArrayKernel32[1].breakProc = OnBeep;
    BreakArrayKernel32[1].dwFlags = BREAK_OUT|BREAK_IN;

    BreakArrayKernel32[2].funcName = "CreateConsoleScreenBuffer";
    BreakArrayKernel32[2].nargs = 5;
    BreakArrayKernel32[2].breakProc = OnCreateConsoleScreenBuffer;
    BreakArrayKernel32[2].dwFlags = BREAK_OUT;

    BreakArrayKernel32[3].funcName = "FillConsoleOutputAttribute";
    BreakArrayKernel32[3].nargs = 5;
    BreakArrayKernel32[3].breakProc = OnFillConsoleOutputAttribute;
    BreakArrayKernel32[3].dwFlags = BREAK_OUT;

    BreakArrayKernel32[4].funcName = "FillConsoleOutputCharacterA";
    BreakArrayKernel32[4].nargs = 5;
    BreakArrayKernel32[4].breakProc = OnFillConsoleOutputCharacterA;
    BreakArrayKernel32[4].dwFlags = BREAK_OUT;

    BreakArrayKernel32[5].funcName = "FillConsoleOutputCharacterW";
    BreakArrayKernel32[5].nargs = 5;
    BreakArrayKernel32[5].breakProc = OnFillConsoleOutputCharacterW;
    BreakArrayKernel32[5].dwFlags = BREAK_OUT;

    BreakArrayKernel32[6].funcName = "FreeConsole";
    BreakArrayKernel32[6].nargs = 0;
    BreakArrayKernel32[6].breakProc = OnFreeConsole;
    BreakArrayKernel32[6].dwFlags = BREAK_OUT;

    BreakArrayKernel32[7].funcName = "GetStdHandle";
    BreakArrayKernel32[7].nargs = 1;
    BreakArrayKernel32[7].breakProc = OnGetStdHandle;
    BreakArrayKernel32[7].dwFlags = BREAK_OUT;

    BreakArrayKernel32[8].funcName = "OpenConsoleW";
    BreakArrayKernel32[8].nargs = 4;
    BreakArrayKernel32[8].breakProc = OnOpenConsoleW;
    BreakArrayKernel32[8].dwFlags = BREAK_OUT;

    BreakArrayKernel32[9].funcName = "ScrollConsoleScreenBufferA";
    BreakArrayKernel32[9].nargs = 5;
    BreakArrayKernel32[9].breakProc = OnScrollConsoleScreenBuffer;
    BreakArrayKernel32[9].dwFlags = BREAK_OUT;

    BreakArrayKernel32[10].funcName = "ScrollConsoleScreenBufferW";
    BreakArrayKernel32[10].nargs = 5;
    BreakArrayKernel32[10].breakProc = OnScrollConsoleScreenBuffer;
    BreakArrayKernel32[10].dwFlags = BREAK_OUT;

    BreakArrayKernel32[11].funcName = "SetConsoleActiveScreenBuffer";
    BreakArrayKernel32[11].nargs = 1;
    BreakArrayKernel32[11].breakProc = OnSetConsoleActiveScreenBuffer;
    BreakArrayKernel32[11].dwFlags = BREAK_OUT;

    BreakArrayKernel32[12].funcName = "SetConsoleCP";
    BreakArrayKernel32[12].nargs = 1;
    BreakArrayKernel32[12].breakProc = OnSetConsoleCP;
    BreakArrayKernel32[12].dwFlags = BREAK_OUT;

    BreakArrayKernel32[13].funcName = "SetConsoleCursorInfo";
    BreakArrayKernel32[13].nargs = 2;
    BreakArrayKernel32[13].breakProc = OnSetConsoleCursorInfo;
    BreakArrayKernel32[13].dwFlags = BREAK_OUT;

    BreakArrayKernel32[14].funcName = "SetConsoleCursorPosition";
    BreakArrayKernel32[14].nargs = 2;
    BreakArrayKernel32[14].breakProc = OnSetConsoleCursorPosition;
    BreakArrayKernel32[14].dwFlags = BREAK_OUT;

    BreakArrayKernel32[15].funcName = "SetConsoleMode";
    BreakArrayKernel32[15].nargs = 2;
    BreakArrayKernel32[15].breakProc = OnSetConsoleMode;
    BreakArrayKernel32[15].dwFlags = BREAK_OUT;

    BreakArrayKernel32[16].funcName = "SetConsoleOutputCP";
    BreakArrayKernel32[16].nargs = 1;
    BreakArrayKernel32[16].breakProc = OnSetConsoleOutputCP;
    BreakArrayKernel32[16].dwFlags = BREAK_OUT;

    BreakArrayKernel32[17].funcName = "SetConsoleTextAttribute";
    BreakArrayKernel32[17].nargs = 2;
    BreakArrayKernel32[17].breakProc = OnSetConsoleTextAttribute;
    BreakArrayKernel32[17].dwFlags = BREAK_OUT;

    BreakArrayKernel32[18].funcName = "SetConsoleWindowInfo";
    BreakArrayKernel32[18].nargs = 3;
    BreakArrayKernel32[18].breakProc = OnSetConsoleWindowInfo;
    BreakArrayKernel32[18].dwFlags = BREAK_OUT;

    BreakArrayKernel32[19].funcName = "WriteConsoleA";
    BreakArrayKernel32[19].nargs = 5;
    BreakArrayKernel32[19].breakProc = OnWriteConsoleA;
    BreakArrayKernel32[19].dwFlags = BREAK_OUT;

    BreakArrayKernel32[20].funcName = "WriteConsoleW";
    BreakArrayKernel32[20].nargs = 5;
    BreakArrayKernel32[20].breakProc = OnWriteConsoleW;
    BreakArrayKernel32[20].dwFlags = BREAK_OUT;

    BreakArrayKernel32[21].funcName = "WriteConsoleOutputA";
    BreakArrayKernel32[21].nargs = 5;
    BreakArrayKernel32[21].breakProc = OnWriteConsoleOutputA;
    BreakArrayKernel32[21].dwFlags = BREAK_OUT;

    BreakArrayKernel32[22].funcName = "WriteConsoleOutputW";
    BreakArrayKernel32[22].nargs = 5;
    BreakArrayKernel32[22].breakProc = OnWriteConsoleOutputW;
    BreakArrayKernel32[22].dwFlags = BREAK_OUT;

    BreakArrayKernel32[23].funcName = "WriteConsoleOutputCharacterA";
    BreakArrayKernel32[23].nargs = 5;
    BreakArrayKernel32[23].breakProc = OnWriteConsoleOutputCharacterA;
    BreakArrayKernel32[23].dwFlags = BREAK_OUT;

    BreakArrayKernel32[24].funcName = "WriteConsoleOutputCharacterW";
    BreakArrayKernel32[24].nargs = 5;
    BreakArrayKernel32[24].breakProc = OnWriteConsoleOutputCharacterW;
    BreakArrayKernel32[24].dwFlags = BREAK_OUT;

    BreakArrayKernel32[25].funcName = "WriteFile";
    BreakArrayKernel32[25].nargs = 5;
    BreakArrayKernel32[25].breakProc = OnWriteFile;
    BreakArrayKernel32[25].dwFlags = BREAK_OUT;

    BreakArrayKernel32[26].funcName = 0L;
    BreakArrayKernel32[26].nargs = 0;
    BreakArrayKernel32[26].breakProc = 0L;
    BreakArrayKernel32[26].dwFlags = 0;

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

    hMasterConsole = CreateFile("CONOUT$", GENERIC_READ|GENERIC_WRITE,
	    FILE_SHARE_READ|FILE_SHARE_WRITE, 0L, OPEN_EXISTING, 0, 0L);
}

ConsoleDebugger::~ConsoleDebugger()
{
    delete [] SymbolPath;
    if (injectorIPC) delete injectorIPC;
    CloseHandle(hMasterConsole);
}

unsigned
ConsoleDebugger::ThreadHandlerProc(void)
{
    DWORD ok, exitcode;
    STARTUPINFO si;
    PROCESS_INFORMATION pi;
    char *cmdline;

    ZeroMemory(&pi, sizeof(PROCESS_INFORMATION));
    ZeroMemory(&si, sizeof(STARTUPINFO));
    si.cb = sizeof(STARTUPINFO);
    si.dwXCountChars = 80;
    si.dwYCountChars = 25;
    si.wShowWindow = SW_SHOWNOACTIVATE;
    si.dwFlags = STARTF_FORCEONFEEDBACK | STARTF_USESHOWWINDOW |
	STARTF_USECOUNTCHARS;

    cmdline = ArgMaker::BuildCommandLine(argc, argv);

    ok = CreateProcess(
	    0L,		// Module name (not needed).
	    cmdline,	// Command line.
	    0L,		// Process handle will not be inheritable.
	    0L,		// Thread handle will not be inheritable.
	    FALSE,	// No handle inheritance.
	    DEBUG_PROCESS |
	    CREATE_NEW_CONSOLE |
	    CREATE_DEFAULT_ERROR_MODE |
	    CREATE_SEPARATE_WOW_VDM |
	    0,
			// Creation flags.
	    0L,		// Use parent's environment block.
	    0L,		// Use parent's starting directory.
	    &si,	// Pointer to STARTUPINFO structure.
	    &pi);	// Pointer to PROCESS_INFORMATION structure.

    if (!ok) {
	EXP_LOG2(MSG_DT_BADDEBUGGEE, cmdline, GetSysMsg(GetLastError()));
    }
    delete [] cmdline;

    WaitForInputIdle(pi.hProcess, 5000);
    CloseHandle(pi.hThread);
    CloseHandle(pi.hProcess);

    exitcode = CommonDebugger();
    NotifyDone();
    return exitcode;
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
DWORD
ConsoleDebugger::CommonDebugger()
{
    DEBUG_EVENT debEvent;	// debugging event info.
    DWORD dwContinueStatus;	// exception continuation.
    Process *proc;
    int breakCount = 0;

again:
    dwContinueStatus = DBG_CONTINUE;

    // Wait (forever) for a debugging event to occur.
    //
    if (WaitForDebugEvent(&debEvent, INFINITE) == FALSE) {
	return GetLastError();
    }

    // Find the process that is responsible for this event.
    //
    for (proc = ProcessList; proc; proc = proc->nextPtr) {
	if (proc->pid == debEvent.dwProcessId) {
	    break;
	}
    }

    if (!proc && debEvent.dwDebugEventCode != CREATE_PROCESS_DEBUG_EVENT) {
/*	char buf[50];
	wsprintf(buf, "%d/%d (%d)", 
		debEvent.dwProcessId, debEvent.dwThreadId,
		debEvent.dwDebugEventCode);
	EXP_LOG1(MSG_DT_UNEXPECTEDDBGEVENT, buf);*/
	if (debEvent.dwDebugEventCode == EXCEPTION_DEBUG_EVENT) {
	    /*char buf[50];
	    wsprintf(buf, "0x%08x", debEvent.u.Exception.ExceptionRecord.ExceptionCode);
	    EXP_LOG1(MSG_DT_EXCEPTIONDBGEVENT, buf);*/
	    dwContinueStatus = DBG_EXCEPTION_NOT_HANDLED;
	}
	goto skip;
    }

    bpCritSec.Enter();

    // Process the debugging event code.
    //
    switch (debEvent.dwDebugEventCode) {
    case EXCEPTION_DEBUG_EVENT:
	switch (debEvent.u.Exception.ExceptionRecord.ExceptionCode) {
	case EXCEPTION_BREAKPOINT:
	{
	    // only the first three get special attention.
	    switch (breakCount < 4 ? ++breakCount : 4) {
	    case 1:
		OnXFirstBreakpoint(proc, &debEvent); break;
	    case 2:
		OnXSecondBreakpoint(proc, &debEvent); break;
	    case 3:
		OnXThirdBreakpoint(proc, &debEvent); break;
	    case 4:
		OnXBreakpoint(proc, &debEvent);
	    }
	    break;
	}

	case EXCEPTION_SINGLE_STEP:
	    OnXSingleStep(proc, &debEvent);
	    break;

	case DBG_CONTROL_C:
	case DBG_CONTROL_BREAK:
	    dwContinueStatus = DBG_EXCEPTION_NOT_HANDLED;
	    break;

	case EXCEPTION_NONCONTINUABLE_EXCEPTION:
	    // we die here.
	    return EXCEPTION_NONCONTINUABLE_EXCEPTION;

	case STATUS_VDM_EVENT:
	    OnXVDMException(proc, &debEvent);
	    break;

	default:
	    if (!debEvent.u.Exception.dwFirstChance) {
		// An exception was hit and it was not handled by the program.
		// Now it is time to get a backtrace before it's death.
		//
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
	ProcessFree(proc);
	if (ProcessList == 0L) {
	    // When the last process exits, we exit.
	    //
	    return debEvent.u.ExitProcess.dwExitCode;
	}
	break;

    case LOAD_DLL_DEBUG_EVENT:
	OnXLoadDll(proc, &debEvent);
	break;

    case UNLOAD_DLL_DEBUG_EVENT:
	OnXUnloadDll(proc, &debEvent);
	break;

    case OUTPUT_DEBUG_STRING_EVENT:
	OnXDebugString(proc, &debEvent);
	break;

    case RIP_EVENT:
	OnXRip(proc, &debEvent);
	break;
    }

    bpCritSec.Leave();

skip:
    // Resume executing the thread that reported the debugging event.
    //
    ContinueDebugEvent(debEvent.dwProcessId, debEvent.dwThreadId,
	    dwContinueStatus);
    goto again;
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


void
ConsoleDebugger::OnXFirstBreakpoint(Process *proc, LPDEBUG_EVENT pDebEvent)
{
    ThreadInfo *tinfo;

    for (tinfo = proc->threadList; tinfo != 0L; tinfo = tinfo->nextPtr) {
	if (pDebEvent->dwThreadId == tinfo->dwThreadId) {
	    break;
	}
    }

    // Install our entry breakpoint.  We can't run the injector Stub now
    // as the process heap hasn't been created.  (it was some memory related
    // error and I don't know exactly why, but moving the stub run later
    // solved it).  It seems that the first breakpoint naturally fired is
    // much too early in the creation of the initial process for much of
    // anything to be ready.
    //
    ReadSubprocessMemory(proc, pStartAddress,
	    &originalExeEntryPointOpcode, sizeof(BYTE));

    BYTE bpOpcode = BRK_OPCODE;
    WriteSubprocessMemory(proc, pStartAddress, &bpOpcode, sizeof(BYTE));

    return;
}

/*
 *----------------------------------------------------------------------
 *
 * ConsoleDebugger::OnXSecondBreakpoint --
 *
 *	This routine is called when the second breakpoint is hit.
 *	The processes' entry-point is getting called.  We intercept this
 *	and make our injector.dll load first before the entry-point is
 *	called.
 *
 * Results:
 *	None
 *
 *----------------------------------------------------------------------
 */

void
ConsoleDebugger::OnXSecondBreakpoint(Process *proc, LPDEBUG_EVENT pDebEvent)
{
    ThreadInfo *tinfo;

    for (tinfo = proc->threadList; tinfo != 0L; tinfo = tinfo->nextPtr) {
	if (pDebEvent->dwThreadId == tinfo->dwThreadId) {
	    break;
	}
    }

    // Put the first opcode at the entry point back in place.
    WriteSubprocessMemory(proc, pStartAddress,
	    &originalExeEntryPointOpcode, sizeof(BYTE));

    //  Make some memory for our stub that we place into the processes' address
    //  space.  This stub (or set of opcodes) calls LoadLibrary() to bring in our
    //  injector dll that acts as the receiver for "injecting" console events.
    //
    MakeSubprocessMemory(proc, sizeof(LOADLIBRARY_STUB), &pInjectorStub,
	    PAGE_EXECUTE_READWRITE);
    injectorStub.operand_PUSH_value = (DWORD) pInjectorStub +
	    offsetof(LOADLIBRARY_STUB, data_DllName);
    injectorStub.operand_MOV_EAX = (DWORD) GetProcAddress(GetModuleHandle(
	    "KERNEL32.DLL"), "LoadLibraryA");
    WriteSubprocessMemory(proc, pInjectorStub, &injectorStub,
	    sizeof(LOADLIBRARY_STUB));
    FlushInstructionCache(proc->hProcess, pInjectorStub, sizeof(LOADLIBRARY_STUB));


    //  Save the instruction pointer so we can restore it later.
    //
    preStubContext.ContextFlags = CONTEXT_FULL;
    GetThreadContext(tinfo->hThread, &preStubContext);

    //  Set instruction pointer to run the Stub instead of continuing
    //  from where we are (which is the app's entry point).
    //
    CONTEXT stubContext = preStubContext;
    stubContext.Eip = (DWORD) pInjectorStub;
    SetThreadContext(tinfo->hThread, &stubContext);
}

/*
 *----------------------------------------------------------------------
 *
 * ConsoleDebugger::OnXThirdBreakpoint --
 *
 *	This routine is called when the third breakpoint is hit and
 *	our stub has run and our injector is loaded.
 *
 * Results:
 *	None
 *
 *----------------------------------------------------------------------
 */

void
ConsoleDebugger::OnXThirdBreakpoint(Process *proc, LPDEBUG_EVENT pDebEvent)
{
    ThreadInfo *tinfo;
    CHAR boxName[50];
    DWORD err;
#   define RETBUF_SIZE 2048
    BYTE retbuf[RETBUF_SIZE];
    BreakInfo *binfo;
    int i;

    for (tinfo = proc->threadList; tinfo != 0L; tinfo = tinfo->nextPtr) {
	if (pDebEvent->dwThreadId == tinfo->dwThreadId) {
	    break;
	}
    }

    // Create the IPC connection to our loaded injector.dll
    //
    wsprintf(boxName, "ExpectInjector_pid%d", proc->pid);
    injectorIPC = new CMclMailbox(IPC_NUMSLOTS, IPC_SLOTSIZE, boxName);

    // Check status.
    err = injectorIPC->Status();
    if (err != NO_ERROR && err != ERROR_ALREADY_EXISTS) {
	char *error = new char [512];
	strcpy(error, "IPC connection to injector.dll could not be made: ");
	strcat(error, GetSysMsg(err));
	strcat(error, "\n");
	WriteMasterError(error, strlen(error));
	delete injectorIPC;
	injectorIPC = 0L;
    }

    // Set our thread to run the entry point, now, starting the
    // application once we return from this breakpoint.
    preStubContext.Eip -= sizeof(BYTE);
    SetThreadContext(tinfo->hThread, &preStubContext);


    // We should now remove the memory allocated in the sub process for
    // our injector stub.  The dll is already loaded and there's no sense
    // hogging a virtual memory page.
    //
    RemoveSubprocessMemory(proc, pInjectorStub);


    /////////////////////////////////////////////////////////////////////
    // Now create our breakpoints on all calls to the OS console API.
    /////////////////////////////////////////////////////////////////////


    // Set up the memory that will serve as the place for our
    // intercepted function return points.
    //
    MakeSubprocessMemory(proc, RETBUF_SIZE, &(proc->pSubprocessMemory));

    // Fill the buffer with all breakpoint opcodes.
    //
    memset(retbuf, BRK_OPCODE, RETBUF_SIZE);

    // Write it out to our buffer space in the other process.
    //
    WriteSubprocessMemory(proc, proc->pSubprocessMemory, retbuf, RETBUF_SIZE);

    // Set all Console API breakpoints.
    //
    for (i = 0; BreakPoints[i].dllName; i++) {
	for (binfo = BreakPoints[i].breakInfo; binfo->funcName; binfo++) {
	    SetBreakpoint(proc, binfo);
	}
    }

#   undef RETBUF_SIZE
    return;
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
ConsoleDebugger::SetBreakpoint(Process *process, BreakInfo *info)
{
    PVOID funcPtr;

    if (process->funcTable.Find(info->funcName, &funcPtr) == TCL_ERROR)
    {
//	PCHAR buffer;
//	DWORD len;
//
//	buffer = new CHAR [128];
//	len = wsprintf(buffer, "Unable to set breakpoint at %s", info->funcName);
//	WriteMasterError(buffer, len);
	return FALSE;
    }

    // Set a breakpoint at the function start in the subprocess and
    // save the original code at the function start.
    //
    return SetBreakpointAtAddr(process, info, funcPtr);
}

/*
 *----------------------------------------------------------------------
 *
 * ConsoleDebugger::SetBreakpointAtAddr --
 *
 *	Inserts a single breakpoint at the given address
 *
 * Results:
 *	The new BreakPoint instance.
 *
 *----------------------------------------------------------------------
 */

BOOL
ConsoleDebugger::SetBreakpointAtAddr(Process *proc, BreakInfo *info, PVOID funcPtr)
{
    Breakpoint *bpt, *lastBpt;
    BYTE code;
    BOOL ok;

    bpt = new Breakpoint;
    bpt->codePtr = funcPtr;
    bpt->codeReturnPtr = (PVOID) (proc->offset + (DWORD) proc->pSubprocessMemory);
    bpt->breakInfo = info;
    proc->offset += 2;
    bpt->nextPtr = lastBpt = proc->brkptList;
    proc->brkptList = bpt;

    if ((ok = ReadSubprocessMemory(proc, funcPtr, &bpt->code, sizeof(BYTE)))
	    == TRUE) {
	code = BRK_OPCODE;
	ok = WriteSubprocessMemory(proc, funcPtr, &code, sizeof(BYTE));
    }

    if (!ok) {
	// unsplice it.
	proc->brkptList = lastBpt;
	delete bpt;
	bpt = 0L;
    }

    return ok;
}

/*
 *----------------------------------------------------------------------
 *
 * ConsoleDebugger::OnXSingleStep --
 *
 *	This routine is called when a EXCEPTION_DEBUG_EVENT with
 *	an exception code of EXCEPTION_SINGLE_STEP.
 *
 * Results:
 *	None
 *
 *----------------------------------------------------------------------
 */

void
ConsoleDebugger::OnXSingleStep(Process *proc, LPDEBUG_EVENT pDebEvent)
{
    BYTE code;

    // Now, we need to restore the breakpoint that we had removed.
    //
    code = BRK_OPCODE;
    WriteSubprocessMemory(proc, proc->lastBrkpt->codePtr, &code, sizeof(BYTE));
}

/*
 *----------------------------------------------------------------------
 *
 * ConsoleDebugger::OnXSecondChanceException --
 *
 *	Handle a second chance exception
 *
 *----------------------------------------------------------------------
 */

void
ConsoleDebugger::OnXSecondChanceException(Process *proc,
    LPDEBUG_EVENT pDebEvent)
{
    BOOL b;
    STACKFRAME frame;
    CONTEXT context;
    ThreadInfo *tinfo;
    Module *modPtr;
    DWORD displacement;
    BYTE symbolBuffer[sizeof(IMAGEHLP_SYMBOL) + 512];
    PIMAGEHLP_SYMBOL pSymbol = (PIMAGEHLP_SYMBOL)symbolBuffer;
    char *s;
    int result;
    unsigned int level = 0;

    for (tinfo = proc->threadList; tinfo != NULL; tinfo = tinfo->nextPtr) {
	if (pDebEvent->dwThreadId == tinfo->dwThreadId) {
	    break;
	}
    }

    context.ContextFlags = CONTEXT_FULL;
    GetThreadContext(tinfo->hThread, &context);

    /*
     * XXX: From what I can tell, SymInitialize is broken on Windows NT 4.0
     * if you try to have it iterate the modules in a process.  It always
     * returns an object mismatch error.  Instead, initialize without iterating
     * the modules.  Contrary to what MSDN documentation says,
     * Microsoft debuggers do not exclusively use the imagehlp API.  In
     * fact, the only thing VC 5.0 uses is the StackWalk function.
     * Windbg uses a few more functions, but it doesn't use SymInitialize.
     * We will then do the hard work of finding all the
     * modules and doing the right thing.
     */

    if (!SymInitialize(proc->hProcess, SymbolPath, FALSE))
    {
	CHAR *buffer = new CHAR [512];
	DWORD len;
	len = wsprintf(buffer, "Unable to get backtrace: %s\n",
		GetSysMsg(GetLastError()));
	WriteMasterError(buffer, len);
	return;
    }

#ifdef _X86_
    memset(&frame, 0, sizeof(frame));
    frame.AddrPC.Mode = AddrModeFlat;
    frame.AddrPC.Segment = 0;
    frame.AddrPC.Offset = context.Eip;

    frame.AddrReturn.Mode = AddrModeFlat;
    frame.AddrReturn.Segment = 0;
    frame.AddrReturn.Offset = context.Ebp; /* I think this is correct */

    frame.AddrFrame.Mode = AddrModeFlat;
    frame.AddrFrame.Segment = 0;
    frame.AddrFrame.Offset = context.Ebp;

    frame.AddrStack.Mode = AddrModeFlat;
    frame.AddrStack.Segment = 0;
    frame.AddrStack.Offset = context.Esp;

    frame.FuncTableEntry = NULL;
    frame.Params[0] = context.Eax;
    frame.Params[1] = context.Ecx;
    frame.Params[2] = context.Edx;
    frame.Params[3] = context.Ebx;
    frame.Far = FALSE;
    frame.Virtual = FALSE;
    frame.Reserved[0] = 0;
    frame.Reserved[1] = 0;
    frame.Reserved[2] = 0;
    /* frame.KdHelp.* is not set */

    /*
     * Iterate through the loaded modules and load symbols for each one.
     */
    for (
	result = proc->moduleTable.Top(&modPtr);
	result == TCL_OK;
	result = proc->moduleTable.Next(&modPtr)
    ) {
	if (!modPtr->loaded) {
	    modPtr->dbgInfo = MapDebugInformation(modPtr->hFile, 0L,
		SymbolPath, (DWORD)modPtr->baseAddr);

	    SymLoadModule(proc->hProcess, modPtr->hFile,
		0L, 0L, (DWORD) modPtr->baseAddr, 0);
	    modPtr->loaded = TRUE;
	}
    }


    if (proc->exeModule && proc->exeModule->dbgInfo && 
	proc->exeModule->dbgInfo->ImageFileName) {
	s = proc->exeModule->dbgInfo->ImageFileName;
    } else {
	s = "";
    }

    {
	DWORD len;
	CHAR *buffer = new CHAR [MAX_PATH+600];
	DWORD exCode = pDebEvent->u.Exception.ExceptionRecord.ExceptionCode;
	len = wsprintf(buffer,
		"\nA fatal, second-chance exception has occured in \"%s\".\n"
		"(0x%X) -- %s\n"
		"This is the backtrace:\n"
		"-------------------------------------\n", s, exCode,
		GetExceptionCodeString(exCode));
	WriteMasterError(buffer, len);
    }

    while (1) {
        pSymbol->SizeOfStruct = sizeof(symbolBuffer);
        pSymbol->MaxNameLength = 512;

	b = StackWalk(
#ifdef _WIN64
	    IMAGE_FILE_MACHINE_IA64,
#else
	    IMAGE_FILE_MACHINE_I386,
#endif
	    proc->hProcess,
	    tinfo->hThread, &frame, &context, 0L,
	    SymFunctionTableAccess, SymGetModuleBase,
	    0L);

	if (b == FALSE || frame.AddrPC.Offset == 0) {
	    break;
	}

	level++;
	    
        if (SymGetSymFromAddr(proc->hProcess, frame.AddrPC.Offset,
	    &displacement, pSymbol) )
        {
	    DWORD base, len;
	    CHAR *buffer = new CHAR [128];

	    base = SymGetModuleBase(proc->hProcess, frame.AddrPC.Offset);
	    if (proc->moduleTable.Find((void *)base, &modPtr) != TCL_ERROR) {
		if (modPtr->dbgInfo && modPtr->dbgInfo->ImageFileName) {
		    s = modPtr->dbgInfo->ImageFileName;
		} else {
		    s = "";
		}
	    } else {
		s = "";
	    }
	    len = wsprintf(buffer, "%u) %.20s 0x%08.8x\t%s+%X\n", level, s, frame.AddrPC.Offset,
		    pSymbol->Name, displacement);
	    WriteMasterError(buffer, len);
	} else {
	    DWORD len;
	    CHAR *buffer = new CHAR [33];
	    len = wsprintf(buffer, "%08.8x\n", frame.AddrPC.Offset);
	    WriteMasterError(buffer, len);
	}
    }

    SymCleanup(proc->hProcess);
#else
#  error Unsupported architecture	    
#endif
}

/*
 *----------------------------------------------------------------------
 *
 * ConsoleDebugger::GetExceptionCodeString --
 *
 *	Returns a string about the exception code.  Much more can
 *	be added.
 *
 *----------------------------------------------------------------------
 */

PCSTR
ConsoleDebugger::GetExceptionCodeString (DWORD exCode)
{
    switch (exCode)
    {
	case EXCEPTION_ACCESS_VIOLATION:
	    return "Access Violation.";
	case EXCEPTION_ARRAY_BOUNDS_EXCEEDED:
	    return "Array access was out-of-bounds.";
	case EXCEPTION_INT_DIVIDE_BY_ZERO:
	case EXCEPTION_FLT_DIVIDE_BY_ZERO:
	    return "Divide by zero.";
	case EXCEPTION_ILLEGAL_INSTRUCTION:
	case EXCEPTION_PRIV_INSTRUCTION:
	    return "Illegal opcode.";
	case EXCEPTION_STACK_OVERFLOW:
	    return "Stack overflow.";
	default:
	    return "exception unknown.";
    }
}

/*
 *----------------------------------------------------------------------
 *
 * ConsoleDebugger::OnXCreateThread --
 *
 *	This routine is called when a CREATE_THREAD_DEBUG_EVENT
 *	occurs.
 *
 * Results:
 *	None
 *
 *----------------------------------------------------------------------
 */

void
ConsoleDebugger::OnXCreateThread(Process *proc, LPDEBUG_EVENT pDebEvent)
{
    ThreadInfo *threadInfo;

    threadInfo = new ThreadInfo;
    threadInfo->dwThreadId = pDebEvent->dwThreadId;
    threadInfo->hThread = pDebEvent->u.CreateThread.hThread;
    proc->threadCount++;
    threadInfo->nextPtr = proc->threadList;
    proc->threadList = threadInfo;
}

/*
 *----------------------------------------------------------------------
 *
 * ConsoleDebugger::OnXDeleteThread --
 *
 *	This routine is called when a CREATE_THREAD_DEBUG_EVENT
 *	occurs.
 *
 * Results:
 *	None
 *
 *----------------------------------------------------------------------
 */

void
ConsoleDebugger::OnXDeleteThread(Process *proc, LPDEBUG_EVENT pDebEvent)
{
    ThreadInfo *threadInfo;
    ThreadInfo *prev;

    prev = 0L;
    for (threadInfo = proc->threadList; threadInfo;
	 prev = threadInfo, threadInfo = threadInfo->nextPtr)
    {
	if (threadInfo->dwThreadId == pDebEvent->dwThreadId) {
	    if (prev == 0L) {
		proc->threadList = threadInfo->nextPtr;
	    } else {
		prev->nextPtr = threadInfo->nextPtr;
	    }
	    proc->threadCount--;
	    CloseHandle(threadInfo->hThread);
	    delete threadInfo;
	    break;
	}
    }
}

/*
 *----------------------------------------------------------------------
 *
 * ConsoleDebugger::OnXCreateProcess --
 *
 *	This routine is called when a CREATE_PROCESS_DEBUG_EVENT
 *	occurs.
 *
 * Results:
 *	None
 *
 *----------------------------------------------------------------------
 */

void
ConsoleDebugger::OnXCreateProcess(Process *proc, LPDEBUG_EVENT pDebEvent)
{
    ThreadInfo *threadInfo;
    CREATE_PROCESS_DEBUG_INFO *info = &pDebEvent->u.CreateProcessInfo;
    int known;

    // Save the first processes' start address.
    if (!pStartAddress)
	    pStartAddress = info->lpStartAddress;

    if (proc == 0L) {
	proc = ProcessNew();
	proc->hProcess = info->hProcess;
	proc->pid = pDebEvent->dwProcessId;
    }

    known = LoadedModule(proc, info->hFile, info->lpImageName, info->fUnicode,
	    info->lpBaseOfImage, info->dwDebugInfoFileOffset);

    threadInfo = new ThreadInfo;
    threadInfo->dwThreadId = pDebEvent->dwThreadId;
    threadInfo->hThread = info->hThread;
    threadInfo->nextPtr = proc->threadList;
    proc->threadCount++;
    proc->threadList = threadInfo;
}

/*
 *----------------------------------------------------------------------
 *
 * ConsoleDebugger::OnXLoadDll --
 *
 *	This routine is called when a LOAD_DLL_DEBUG_EVENT is seen
 *
 * Results:
 *	None
 *
 * Side Effects:
 *	Some information is printed
 *
 *----------------------------------------------------------------------
 */

void
ConsoleDebugger::OnXLoadDll(Process *proc, LPDEBUG_EVENT pDebEvent)
{
    WORD w;
    DWORD dw;
    DWORD ImageHdrOffset;
    PIMAGE_FILE_HEADER pfh;	/* File header image in subprocess memory */
    PIMAGE_SECTION_HEADER psh;
    PIMAGE_OPTIONAL_HEADER poh;
    IMAGE_DATA_DIRECTORY dataDir;
    PIMAGE_EXPORT_DIRECTORY ped;
    IMAGE_EXPORT_DIRECTORY exportDir;
    DWORD n;
    DWORD base;
    CHAR funcName[256];
    CHAR dllname[256];
    PVOID ptr, namePtr, funcPtr;
    DWORD p;
    LPLOAD_DLL_DEBUG_INFO info = &pDebEvent->u.LoadDll;
    BOOL bFound;

    int unknown = !LoadedModule(proc, info->hFile,
	info->lpImageName, info->fUnicode,
	info->lpBaseOfDll, info->dwDebugInfoFileOffset);

    base = (DWORD) info->lpBaseOfDll;

    // Check for the DOS signature
    //
    ReadSubprocessMemory(proc, info->lpBaseOfDll, &w, sizeof(WORD));
    if (w != IMAGE_DOS_SIGNATURE) return;
    
    // Skip over the DOS signature and check the PE signature
    //
    p = base;
    p += 15 * sizeof(DWORD);
    ptr = (PVOID) p;
    ReadSubprocessMemory(proc, (PVOID) p, &ImageHdrOffset, sizeof(DWORD));

    p = base;
    p += ImageHdrOffset;
    ReadSubprocessMemory(proc, (PVOID) p, &dw, sizeof(DWORD));
    if (dw != IMAGE_NT_SIGNATURE) {
	return;
    }
    ImageHdrOffset += sizeof(DWORD);
    p += sizeof(DWORD);

    pfh = (PIMAGE_FILE_HEADER) p;
    ptr = &pfh->SizeOfOptionalHeader;
    ReadSubprocessMemory(proc, ptr, &w, sizeof(WORD));

    // We want to find the exports section.  It can be found in the
    // data directory that is part of the IMAGE_OPTIONAL_HEADER
    //
    if (!w) return;
    p += sizeof(IMAGE_FILE_HEADER);
    poh = (PIMAGE_OPTIONAL_HEADER) p;

    // Find the number of entries in the data directory
    //
    ptr = &poh->NumberOfRvaAndSizes;
    ReadSubprocessMemory(proc, ptr, &dw, sizeof(DWORD));
    if (dw == 0) return;

    // Read the export data directory
    //
    ptr = &poh->DataDirectory[IMAGE_DIRECTORY_ENTRY_EXPORT];
    ReadSubprocessMemory(proc, ptr, &dataDir, sizeof(IMAGE_DATA_DIRECTORY));

    // This points us to the exports section
    //
    ptr = (PVOID) (base + dataDir.VirtualAddress);
    ped = (PIMAGE_EXPORT_DIRECTORY) ptr;
    ReadSubprocessMemory(proc, ptr, &exportDir, sizeof(IMAGE_EXPORT_DIRECTORY));

    // See if this is a DLL we are interested in
    //
    ptr = &ped->Name;
    ReadSubprocessMemory(proc, ptr, &dw, sizeof(DWORD));
    ptr = (PVOID) (base + dw);
    ReadSubprocessStringA(proc, ptr, dllname, sizeof(dllname));

    {
	int len;
	CHAR msg[256];

	if (dllname[0] == '\0') {
	    // image has no export section, so get the name another way.
	    // TODO: How??

	} else {
	    strcpy(msg, dllname);
	    strcat(msg, " has loaded.\n");
	    len = strlen(msg);
	}
	WriteMasterWarning(strdup(msg), len);
    }

    bFound = FALSE;
    for (n = 0; BreakPoints[n].dllName; n++) {
	if (stricmp(dllname, BreakPoints[n].dllName) == 0) {
	    bFound = TRUE;
	    break;
	}
    }
    if (!bFound) {
	return;
    }

    ptr = (PVOID) (base + (DWORD) exportDir.AddressOfNames);
    for (n = 0; n < exportDir.NumberOfNames; n++) {
	ReadSubprocessMemory(proc, ptr, &dw, sizeof(DWORD));
	namePtr = (PVOID) (base + dw);

	// Now, we should hopefully have a pointer to the name of the
	// function, so lets get it.
	//
	ReadSubprocessStringA(proc, namePtr, funcName, sizeof(funcName));

	// Keep a list of all function names in a hash table
	//
	funcPtr = (PVOID) (base + n*sizeof(DWORD) +
	    (DWORD) exportDir.AddressOfFunctions);
	ReadSubprocessMemory(proc, funcPtr, &dw, sizeof(DWORD));
	funcPtr = (PVOID) (base + dw);

	proc->funcTable.Add(funcName, funcPtr);

	ptr = (PVOID) (sizeof(DWORD) + (ULONG) ptr);
    }

    // The IMAGE_SECTION_HEADER comes after the IMAGE_OPTIONAL_HEADER
    // (if the IMAGE_OPTIONAL_HEADER exists)
    //
    p += w;

    psh = (PIMAGE_SECTION_HEADER) p;
}

/*
 *----------------------------------------------------------------------
 *
 * ConsoleDebugger::OnXUnloadDll --
 *
 *	This routine is called when a UNLOAD_DLL_DEBUG_EVENT is seen
 *
 * Results:
 *	None
 *
 * Side Effects:
 *	Some information is printed
 *
 *----------------------------------------------------------------------
 */

void
ConsoleDebugger::OnXUnloadDll(Process *proc, LPDEBUG_EVENT pDebEvent)
{
    Module *modPtr;

    if (proc->moduleTable.Extract(pDebEvent->u.UnloadDll.lpBaseOfDll, &modPtr)
	    != TCL_ERROR)
    {
	if (modPtr->hFile) {
	    CloseHandle(modPtr->hFile);
	}
	if (modPtr->modName) {
	    delete [] modPtr->modName;
	}
	if (modPtr->dbgInfo) {
	    UnmapDebugInformation(modPtr->dbgInfo);
	}
	delete modPtr;
    }
}

/*
 *----------------------------------------------------------------------
 *
 * ConsoleDebugger::OnXDebugString --
 *
 *	This routine is called when a OUTPUT_DEBUG_STRING_EVENT
 *	happens.
 *
 * Results:
 *	None
 *
 * Side Effects:
 *	Allocates a buffer for the string that is not cleared *here*.
 *
 *----------------------------------------------------------------------
 */
void
ConsoleDebugger::OnXDebugString(Process *proc, LPDEBUG_EVENT pDebEvent)
{
    CHAR *buffer;
    DWORD len;

    if (pDebEvent->u.DebugString.fUnicode) {
	int wlen = pDebEvent->u.DebugString.nDebugStringLength;
	WCHAR *wbuffer = new WCHAR [wlen];
	ReadSubprocessStringW(proc,
		pDebEvent->u.DebugString.lpDebugStringData,
		wbuffer, wlen);
	len = WideCharToMultiByte(CP_ACP, 0, wbuffer, wlen, 0L, 0, 0L, 0L);
	buffer = new CHAR [len+1];
	WideCharToMultiByte(CP_ACP, 0, wbuffer, wlen, buffer, len, 0L, 0L);
    } else {
	len = pDebEvent->u.DebugString.nDebugStringLength;
	buffer = new CHAR [len+1];
	ReadSubprocessStringA(proc,
		pDebEvent->u.DebugString.lpDebugStringData,
		buffer, len);
    }

//    buffer[len] = '\0';  // Oops, Win9x forgets this.
    WriteMasterWarning(buffer, len-1);
}

/*
 *----------------------------------------------------------------------
 *
 * ConsoleDebugger::OnXVDMException --
 *
 *	Exceptions raised by the NTVDM (Virtual Dos Machine).  We only
 *	get here when a DOS or Win16 (not likely for us, though)
 *	application is the slave or a child of the slave.
 *
 * Results:
 *	None
 *
 * Side Effects:
 *	.
 *
 *----------------------------------------------------------------------
 */

void
ConsoleDebugger::OnXVDMException(Process *proc, LPDEBUG_EVENT pDebEvent)
{
    EXCEPTION_RECORD &exrec = pDebEvent->u.Exception.ExceptionRecord;
    USHORT exCode = W1(exrec);
    VDMProcessException(pDebEvent);

    switch (exCode) {
        case DBG_SEGLOAD:
	    __asm nop;
	    break;

        case DBG_SEGMOVE:
	    __asm nop;
	    break;

        case DBG_SEGFREE:
	    __asm nop;
	    break;

        case DBG_MODFREE:
        case DBG_MODLOAD:
	    {
		SEGMENT_NOTE segNote;
		char *buffer;
		DWORD len;
		const char *verb;

                switch (exCode)
                {
                    case DBG_MODLOAD:
                        verb = "loaded into"; break;
                    case DBG_MODFREE:
                        verb = "unloaded from"; break;
		}


		ReadSubprocessMemory(proc, (PVOID)DW3(exrec), &segNote, sizeof(segNote));
		len = strlen(segNote.FileName) + 25;
		buffer = new char [len];
		len = wsprintf(buffer, "%s %s the VDM.\n", segNote.FileName, verb);
		WriteMasterWarning(buffer, len);
	    }
	    break;

        case DBG_SINGLESTEP:
	    __asm nop;
	    break;

        case DBG_BREAK:
	    __asm nop;
	    break;

        case DBG_GPFAULT:
	    __asm nop;
	    break;

        case DBG_DIVOVERFLOW:
	    __asm nop;
	    break;

        case DBG_INSTRFAULT:
	    __asm nop;
	    break;

        case DBG_TASKSTART:
        case DBG_TASKSTOP:
        case DBG_DLLSTART:
        case DBG_DLLSTOP:
	    {
		IMAGE_NOTE imgNote;
		char *buffer;
		DWORD len;
		const char *verb;

                switch (exCode)
                {
                    case DBG_TASKSTART:
                        verb = "started in"; break;
                    case DBG_DLLSTART:
                        verb = "started in"; break;
                    case DBG_DLLSTOP:
                        verb = "stopped in"; break;
                    case DBG_TASKSTOP:
                        verb = "stopped in"; break;
                }

		ReadSubprocessMemory(proc, (PVOID)DW3(exrec), &imgNote, sizeof(IMAGE_NOTE));
		len = strlen(imgNote.FileName) + 25;
		buffer = new char [len];
		len = wsprintf(buffer, "%s %s the VDM.\n", imgNote.FileName, verb);
		WriteMasterWarning(buffer, len);
	    }
	    break;

        case DBG_ATTACH:
	    __asm nop;
	    break;

	case DBG_TOOLHELP:
	    __asm nop;
	    break;

	case DBG_STACKFAULT:
	    __asm nop;
	    break;

	case DBG_WOWINIT:
	    __asm nop;
	    break;

	case DBG_TEMPBP:
	    __asm nop;
	    break;

	case DBG_MODMOVE:
	    __asm nop;
	    break;

	case DBG_INIT:
	    VDMSetDbgFlags(proc->hProcess,
//		    VDMDBG_BREAK_DOSTASK |  // <- seems to cause a non-continueable exception on it's own.
//		    VDMDBG_BREAK_WOWTASK |
		    VDMDBG_BREAK_LOADDLL |
		    VDMDBG_BREAK_EXCEPTIONS |
		    VDMDBG_BREAK_DEBUGGER |
//		    VDMDBG_TRACE_HISTORY |
		    0);
	    break;

	case DBG_GPFAULT2:
	    __asm nop;
	    break;

	default:
	    __asm nop;
	    break;
    }
}

/*
 *----------------------------------------------------------------------
 *
 * ConsoleDebugger::OnXRip --
 *
 *	Catches and reports RIP events (system error messages).
 *	Is RIP short for Rest-In-Peace??
 *
 * Results:
 *	None
 *
 * Side Effects:
 *	allocates memory that is freed by the Message destructor.
 *
 *----------------------------------------------------------------------
 */
void
ConsoleDebugger::OnXRip(Process *proc, LPDEBUG_EVENT pDebEvent)
{
    char *errorMsg = new CHAR [512];
    DWORD len;

    if (pDebEvent->u.RipInfo.dwType == SLE_ERROR) {
	len = wsprintf(errorMsg, "A fatal RIP error was caught: %s",
		GetSysMsg(pDebEvent->u.RipInfo.dwError));
	WriteMasterError(errorMsg, len);
    } else {
	len = wsprintf(errorMsg, "A non-fatal RIP error was caught: %s",
		GetSysMsg(pDebEvent->u.RipInfo.dwError));
	WriteMasterWarning(errorMsg, len);
    }
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

    if (brkpt == 0L) {
	// shouldn't happen, but does..
	return;
    }

    context.ContextFlags = CONTEXT_FULL;
    GetThreadContext(tinfo->hThread, &context);

    if (!brkpt->returning) {
	Breakpoint *bpt;
	// Get the arguments to the function and store them in the thread
	// specific data structure.
	for (pdw = tinfo->args, i=0; i < brkpt->breakInfo->nargs; i++, pdw++) {
	    ReadSubprocessMemory(proc, (PVOID) (context.Esp+(4*(i+1))),
				 pdw, sizeof(DWORD));
	}
	tinfo->nargs = brkpt->breakInfo->nargs;
	tinfo->context = &context;

	if (brkpt->breakInfo->dwFlags & BREAK_IN) {
	    ((this)->*(brkpt->breakInfo->breakProc))(proc, tinfo, brkpt, &context.Eax, BREAK_IN);
	}

	// Only set a return breakpoint if something is interested
	// in the return value
	if (brkpt->breakInfo->dwFlags & BREAK_OUT) {
	    bpt = new Breakpoint;
	    ReadSubprocessMemory(proc, (PVOID) context.Esp,
		&bpt->origRetAddr, sizeof(DWORD));
	    dw = (DWORD) brkpt->codeReturnPtr;
	    WriteSubprocessMemory(proc, (PVOID) context.Esp,
		&dw, sizeof(DWORD));
	    bpt->codePtr = brkpt->codeReturnPtr;
	    bpt->returning = TRUE;
	    bpt->codeReturnPtr = 0L;	// Doesn't matter
	    bpt->breakInfo = brkpt->breakInfo;
	    bpt->threadInfo = tinfo;
	    bpt->nextPtr = proc->brkptList;
	    proc->brkptList = bpt;

	}

	// Now, we need to restore the original code for this breakpoint.
	// Put the program counter back, then do a single-step and put
	// the breakpoint back again.
	//
	WriteSubprocessMemory(proc, brkpt->codePtr, &brkpt->code, sizeof(BYTE));

	context.EFlags |= SINGLE_STEP_BIT;
	context.Eip--;

	proc->lastBrkpt = brkpt;
    } else {
	// Make the callback with the params and the return value
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
 * ConsoleDebugger::ReadSubprocessMemory --
 *
 *	Reads memory from the subprocess.  Takes care of all the
 *	issues with page protection.
 *
 * Results:
 *	FALSE if unsuccessful, TRUE if successful.
 *
 * Notes:
 *	Currently safe, but slow.
 *
 *----------------------------------------------------------------------
 */

BOOL
ConsoleDebugger::ReadSubprocessMemory(Process *proc, LPVOID addr, LPVOID buf, DWORD len)
{
    DWORD oldProtection = 0;
    MEMORY_BASIC_INFORMATION mbi;
    BOOL ret = TRUE;
    DWORD err = ERROR_SUCCESS;

    // if inaccessible or not committed memory, abort
    //
    if (!VirtualQueryEx(proc->hProcess, addr, &mbi,
	    sizeof(MEMORY_BASIC_INFORMATION)) || mbi.State != MEM_COMMIT)
    {
	return FALSE;
    }

    // On Win9x, special ranges can't have their protection changed.
    //
    if (dwPlatformId == VER_PLATFORM_WIN32_WINDOWS
	    && addr >= (LPVOID) 0x80000000 && addr <= (LPVOID) 0xBFFFFFFF) {
	ret = ReadProcessMemory(proc->hProcess, addr, buf, len, 0L);
    } else {
	// if guarded memory, change protection temporarily.
	//
	if (!(mbi.Protect & PAGE_READONLY) && !(mbi.Protect & PAGE_READWRITE)) {
	    VirtualProtectEx(proc->hProcess, addr, len, PAGE_READONLY,
		    &oldProtection);
	}
    
	if (!ReadProcessMemory(proc->hProcess, addr, buf, len, 0L)) {
	    err = GetLastError();
	    ret = FALSE;
	}
    
	// reset protection if changed.
	//
	if (oldProtection) {
	    VirtualProtectEx(proc->hProcess, addr, len, oldProtection,
		    &oldProtection);
	    SetLastError(err);
	}
    }
    return ret;
}

/*
 *----------------------------------------------------------------------
 *
 * ConsoleDebugger::WriteSubprocessMemory --
 *
 *	Writes memory from the subprocess.  Takes care of all the
 *	issues with page protection.
 *
 * Results:
 *	zero if unsuccessful, non-zero if successful.
 *
 *----------------------------------------------------------------------
 */

BOOL
ConsoleDebugger::WriteSubprocessMemory(Process *proc, LPVOID addr, LPVOID buf, DWORD len)
{
    DWORD oldProtection = 0;
    MEMORY_BASIC_INFORMATION mbi;
    BOOL ret = TRUE;
    DWORD err = ERROR_SUCCESS;

    // if inaccessible or not committed memory, abort
    if (!VirtualQueryEx(proc->hProcess, addr, &mbi,
	    sizeof(MEMORY_BASIC_INFORMATION)) || mbi.State != MEM_COMMIT)
    {
	return FALSE;
    }
    
    // On Win9x, special ranges can't have their protection changed.
    //
    if (dwPlatformId == VER_PLATFORM_WIN32_WINDOWS
	    && addr >= (LPVOID) 0x80000000 && addr <= (LPVOID) 0xBFFFFFFF) {
	ret = WriteProcessMemory(proc->hProcess, addr, buf, len, 0L);
    } else {
	// if guarded memory, change protection temporarily.
	//
	if (!(mbi.Protect & PAGE_READWRITE || mbi.Protect & PAGE_EXECUTE_READWRITE)) {
	    if (!VirtualProtectEx(proc->hProcess, addr, len, PAGE_READWRITE,
		    &oldProtection)) {
		return FALSE;
	    }
	}
    
	if (!WriteProcessMemory(proc->hProcess, addr, buf, len, 0L)) {
	    ret = FALSE;
	    err = GetLastError();
	}
    
	// reset protection if changed.
	//
	if (oldProtection) {
	    VirtualProtectEx(proc->hProcess, addr, len, oldProtection,
		    &oldProtection);
	    SetLastError(err);
	}
    }
    return ret;
}

/*
 *----------------------------------------------------------------------
 *
 * ConsoleDebugger::ReadSubprocessStringA --
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

/*
 *----------------------------------------------------------------------
 *
 * ConsoleDebugger::LoadedModule --
 *
 *	A module with the specifed name was loaded.  Add it to our
 *	list of loaded modules and print any debugging information
 *	if debugging is enabled.
 *
 * Results:
 *	If the module is known, return TRUE.  Otherwise, return FALSE
 *
 *----------------------------------------------------------------------
 */

int
ConsoleDebugger::LoadedModule(Process *proc, HANDLE hFile, LPVOID modname,
    int isUnicode, LPVOID baseAddr, DWORD debugOffset)
{
    int known = 1;
    PVOID ptr;
    char mbstr[512];
    char *s = 0L;
    Module *modPtr;

    if (modname) {
	// This modname is a pointer to the name of the
	// DLL in the process space of the subprocess
	//
	if (ReadSubprocessMemory(proc, modname, &ptr, sizeof(PVOID)) && ptr) {
	    if (isUnicode) {
		WCHAR name[MAX_PATH];
		int len;
		ReadSubprocessStringW(proc, ptr, name, 512);
		len = WideCharToMultiByte(CP_ACP, 0, name, -1, 0L, 0, 0L, 0L);
		s = new char [len];
		WideCharToMultiByte(CP_ACP, 0, name, -1, s, len, 0L, 0L);
	    } else {
		ReadSubprocessStringA(proc, ptr, mbstr, sizeof(mbstr));
		s = new char [strlen(mbstr)];
		strcpy(s, mbstr);
	    }
	} else {
	    known = 0;
	}
    }

    modPtr = new Module;
    modPtr->loaded = FALSE;
    modPtr->hFile = hFile;
    modPtr->baseAddr = baseAddr;
    modPtr->modName = s;
    modPtr->dbgInfo = 0L;
    if (proc->exeModule == 0L) {
	proc->exeModule = modPtr;
    }

    proc->moduleTable.Add(baseAddr, modPtr);

    return known;
}

void
ConsoleDebugger::MakeSubprocessMemory(Process *proc, SIZE_T amount,
    LPVOID *pBuff, DWORD access)
{
    if (dwPlatformId == VER_PLATFORM_WIN32_NT) {
        if (!(*pBuff = pfnVirtualAllocEx(proc->hProcess, 0, amount, MEM_COMMIT,
		access))) {
	    EXP_LOG1(MSG_DT_CANTMAKENTSPMEM, GetSysMsg(GetLastError()));
	}
    } else {
        // In Windows 9X, create a small memory mapped file.  On this
        // platform, memory mapped files are above 2GB, and thus are
        // accessible to all processes.
	//
        HANDLE hFileMapping = CreateFileMapping(INVALID_HANDLE_VALUE, 0,
		access | SEC_COMMIT, 0, amount, 0);

	if (!hFileMapping) {
	    EXP_LOG1(MSG_DT_CANTMAKE95SPMEM, GetSysMsg(GetLastError()));
	}

	LPVOID buffer = MapViewOfFile(hFileMapping, FILE_MAP_WRITE, 0, 0,
		amount);

	if (!buffer) {
	    EXP_LOG1(MSG_DT_CANTMAKE95SPMEM, GetSysMsg(GetLastError()));
	}

	// Save the association
	//
	spMemMapping.Add(buffer, hFileMapping);
	*pBuff = buffer;
    }
}

BOOL
ConsoleDebugger::RemoveSubprocessMemory(Process *proc, LPVOID buff)
{
    BOOL ret;

    if (dwPlatformId == VER_PLATFORM_WIN32_NT) {
	ret = pfnVirtualFreeEx(proc->hProcess, buff, 0, MEM_RELEASE);
    } else {
	HANDLE hFileMapping;
	if (spMemMapping.Extract(buff, &hFileMapping) != TCL_OK) {
	    SetLastError(ERROR_FILE_NOT_FOUND);
	    return FALSE;
	}
	ret = UnmapViewOfFile(buff);
	CloseHandle(hFileMapping);
    }
    return ret;
}

/*
 *----------------------------------------------------------------------
 *
 * RefreshScreen --
 *
 *	Redraw the entire screen
 *
 * Results:
 *	None
 *
 *----------------------------------------------------------------------
 */
/*
void
ConsoleDebugger::RefreshScreen(void)
{
    CONSOLE_SCREEN_BUFFER_INFO info;
    CHAR buf[4096];
    DWORD bufpos = 0;
    CHAR_INFO consoleBuf[4096];
    COORD size = {ConsoleSize.X, ConsoleSize.Y};
    COORD begin = {0, 0};
    SMALL_RECT rect = {0, 0, ConsoleSize.X-1, ConsoleSize.Y-1};
    int x, y, prespaces, postspaces, offset;

    // Clear the screen
    bufpos += wsprintf(&buf[bufpos], "\033[2J");
    bufpos += wsprintf(&buf[bufpos], "\033[%d;%dH", CursorPosition.Y+1,
	    CursorPosition.X+1);
    CursorKnown = TRUE;

    WriteMasterCopy(buf, bufpos);
    bufpos = 0;

//    if (GetConsoleScreenBufferInfo(HConsole, &info) != FALSE) {
//	return;
//    }

    CursorPosition = info.dwCursorPosition;

//    if (! ReadConsoleOutput(HConsole, consoleBuf, size, begin, &rect)) {
//	return;
//    }

    offset = 0;
    for (y = 0; y < ConsoleSize.Y; y++) {
	offset += ConsoleSize.X;
	for (x = 0; x < ConsoleSize.X; x++) {
	    if (consoleBuf[offset+x].Char.AsciiChar != ' ') {
		break;
	    }
	}
	prespaces = x;
	if (prespaces == ConsoleSize.X) {
	    continue;
	}

	for (x = ConsoleSize.X-1; x >= 0; x--) {
	    if (consoleBuf[offset+x].Char.AsciiChar != ' ') {
		break;
	    }
	}
	postspaces = x;
	bufpos += wsprintf(&buf[bufpos], "\033[%d;%dH", y+1, prespaces+1);

	for (x = prespaces; x < postspaces; x++) {
	    buf[bufpos++] = consoleBuf[offset+x].Char.AsciiChar;
	}
    }

    bufpos += wsprintf(&buf[bufpos], "\033[%d;%dH", CursorPosition.Y+1,
	    CursorPosition.X+1);
    CursorKnown = TRUE;
    WriteMasterCopy(buf, bufpos);
}
*/

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

    for (tcurr = proc->threadList; tcurr != 0L; tcurr = tnext) {
	tnext = tcurr->nextPtr;
	proc->threadCount--;
	CloseHandle(tcurr->hThread);
	delete tcurr;
    }
    for (bcurr = proc->brkptList; bcurr != 0L; bcurr = bnext) {
	bnext = bcurr->nextPtr;
	delete bcurr;
    }
    for (pprev = 0L, pcurr = ProcessList; pcurr != 0L;
	 pcurr = pcurr->nextPtr)
    {
	if (pcurr == proc) {
	    if (pprev == 0L) {
		ProcessList = pcurr->nextPtr;
	    } else {
		pprev->nextPtr = pcurr->nextPtr;
	    }
	    break;
	}
    }
    CloseHandle(proc->hProcess);

    delete proc;
}

void
ConsoleDebugger::WriteMasterCopy(CHAR *buf, DWORD len)
{
    Message *msg;
    DWORD i;

    msg = new Message;
    msg->bytes = new CHAR [len];
    for (i = 0; i < len; i++)
	    ((PCHAR)msg->bytes)[i] = buf[i];
    msg->length = len;
    msg->type = Message::TYPE_NORMAL;
    mQ.Put(msg);
}

/* doesn't copy! */
void
ConsoleDebugger::WriteMasterWarning(CHAR *buf, DWORD len)
{
    Message *msg;
    msg = new Message;
    msg->bytes = buf;
    msg->length = len;
    msg->type = Message::TYPE_WARNING;
    mQ.Put(msg);
}

/* doesn't copy! */
void
ConsoleDebugger::WriteMasterError(CHAR *buf, DWORD len)
{
    Message *msg;
    msg = new Message;
    msg->bytes = buf;
    msg->length = len;
    msg->type = Message::TYPE_ERROR;
    mQ.Put(msg);
}

void
ConsoleDebugger::NotifyDone()
{
    Message *msg;
    msg = new Message;
    msg->type = Message::TYPE_SLAVEDONE;
    mQ.Put(msg);
}

void
ConsoleDebugger::WriteRecord (INPUT_RECORD *ir)
{
    if (injectorIPC != 0L) {
	injectorIPC->Post(ir);
    }
}

void
ConsoleDebugger::EnterInteract (HANDLE OutConsole)
{
    bpCritSec.Enter();

    interactingConsole = OutConsole;

    // More stuff to do here... What?
    // Copy entire screen contents, how?
    // Set interactingConsole to the proper size?
    // more ???  help!

    interacting = true;
    bpCritSec.Leave();
}

void
ConsoleDebugger::ExitInteract ()
{
    interactingConsole = 0L;
    interacting = false;
}