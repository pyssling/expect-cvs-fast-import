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
 * RCS: @(#) $Id: expWinConsoleDebugger.cpp,v 1.1.2.16 2002/06/18 23:14:18 davygrvy Exp $
 * ----------------------------------------------------------------------------
 */

#include <stddef.h>
#include <assert.h>
#include "expWinConsoleDebugger.hpp"
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


typedef LPVOID (__stdcall *PFNVIRTALLEX)(HANDLE,LPVOID,SIZE_T,DWORD,DWORD);

//  Constructor.
ConsoleDebugger::ConsoleDebugger (int _argc, char * const *_argv, CMclQueue<Message *> &_mQ)
    : argc(_argc), argv(_argv), ProcessList(0L), CursorKnown(FALSE), ConsoleOutputCP(0),
    ConsoleCP(0), mQ(_mQ)
{
    OSVERSIONINFO osvi;

    osvi.dwOSVersionInfoSize = sizeof(OSVERSIONINFO);
    GetVersionEx(&osvi);
    dwPlatformId = osvi.dwPlatformId;

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

    BreakArrayKernel32[0].funcName = "AllocConsole";
    BreakArrayKernel32[0].nargs = 0;
    BreakArrayKernel32[0].breakProc = OnAllocConsole;
    BreakArrayKernel32[0].dwFlags = BREAK_OUT;

    BreakArrayKernel32[1].funcName = "Beep";
    BreakArrayKernel32[1].nargs = 2;
    BreakArrayKernel32[1].breakProc = OnBeep;
    BreakArrayKernel32[1].dwFlags = BREAK_OUT|BREAK_IN;

    BreakArrayKernel32[2].funcName = "FillConsoleOutputCharacterA";
    BreakArrayKernel32[2].nargs = 5;
    BreakArrayKernel32[2].breakProc = OnFillConsoleOutputCharacter;
    BreakArrayKernel32[2].dwFlags = BREAK_OUT;

    BreakArrayKernel32[3].funcName = "FillConsoleOutputCharacterW";
    BreakArrayKernel32[3].nargs = 5;
    BreakArrayKernel32[3].breakProc = OnFillConsoleOutputCharacter;
    BreakArrayKernel32[3].dwFlags = BREAK_OUT;

    BreakArrayKernel32[4].funcName = "FreeConsole";
    BreakArrayKernel32[4].nargs = 0;
    BreakArrayKernel32[4].breakProc = OnFreeConsole;
    BreakArrayKernel32[4].dwFlags = BREAK_OUT;

    BreakArrayKernel32[5].funcName = "GetStdHandle";
    BreakArrayKernel32[5].nargs = 1;
    BreakArrayKernel32[5].breakProc = OnGetStdHandle;
    BreakArrayKernel32[5].dwFlags = BREAK_OUT;

    BreakArrayKernel32[6].funcName = "OpenConsoleW";
    BreakArrayKernel32[6].nargs = 4;
    BreakArrayKernel32[6].breakProc = OnOpenConsoleW;
    BreakArrayKernel32[6].dwFlags = BREAK_OUT;

    BreakArrayKernel32[7].funcName = "ScrollConsoleScreenBufferA";
    BreakArrayKernel32[7].nargs = 5;
    BreakArrayKernel32[7].breakProc = OnScrollConsoleScreenBuffer;
    BreakArrayKernel32[7].dwFlags = BREAK_OUT;

    BreakArrayKernel32[8].funcName = "ScrollConsoleScreenBufferW";
    BreakArrayKernel32[8].nargs = 5;
    BreakArrayKernel32[8].breakProc = OnScrollConsoleScreenBuffer;
    BreakArrayKernel32[8].dwFlags = BREAK_OUT;

    BreakArrayKernel32[9].funcName = "SetConsoleActiveScreenBuffer";
    BreakArrayKernel32[9].nargs = 1;
    BreakArrayKernel32[9].breakProc = OnSetConsoleActiveScreenBuffer;
    BreakArrayKernel32[9].dwFlags = BREAK_OUT;

    BreakArrayKernel32[10].funcName = "SetConsoleCP";
    BreakArrayKernel32[10].nargs = 1;
    BreakArrayKernel32[10].breakProc = OnSetConsoleCP;
    BreakArrayKernel32[10].dwFlags = BREAK_OUT;

    BreakArrayKernel32[11].funcName = "SetConsoleCursorInfo";
    BreakArrayKernel32[11].nargs = 2;
    BreakArrayKernel32[11].breakProc = OnSetConsoleCursorInfo;
    BreakArrayKernel32[11].dwFlags = BREAK_OUT;

    BreakArrayKernel32[12].funcName = "SetConsoleCursorPosition";
    BreakArrayKernel32[12].nargs = 2;
    BreakArrayKernel32[12].breakProc = OnSetConsoleCursorPosition;
    BreakArrayKernel32[12].dwFlags = BREAK_OUT;

    BreakArrayKernel32[13].funcName = "SetConsoleMode";
    BreakArrayKernel32[13].nargs = 2;
    BreakArrayKernel32[13].breakProc = OnSetConsoleMode;
    BreakArrayKernel32[13].dwFlags = BREAK_OUT;

    BreakArrayKernel32[14].funcName = "SetConsoleOutputCP";
    BreakArrayKernel32[14].nargs = 1;
    BreakArrayKernel32[14].breakProc = OnSetConsoleOutputCP;
    BreakArrayKernel32[14].dwFlags = BREAK_OUT;

    BreakArrayKernel32[15].funcName = "SetConsoleWindowInfo";
    BreakArrayKernel32[15].nargs = 2;
    BreakArrayKernel32[15].breakProc = OnSetConsoleWindowInfo;
    BreakArrayKernel32[15].dwFlags = BREAK_OUT;

    BreakArrayKernel32[16].funcName = "WriteConsoleA";
    BreakArrayKernel32[16].nargs = 5;
    BreakArrayKernel32[16].breakProc = OnWriteConsoleA;
    BreakArrayKernel32[16].dwFlags = BREAK_OUT;

    BreakArrayKernel32[17].funcName = "WriteConsoleW";
    BreakArrayKernel32[17].nargs = 5;
    BreakArrayKernel32[17].breakProc = OnWriteConsoleW;
    BreakArrayKernel32[17].dwFlags = BREAK_OUT;

    BreakArrayKernel32[18].funcName = "WriteConsoleOutputA";
    BreakArrayKernel32[18].nargs = 5;
    BreakArrayKernel32[18].breakProc = OnWriteConsoleOutputA;
    BreakArrayKernel32[18].dwFlags = BREAK_OUT;

    BreakArrayKernel32[19].funcName = "WriteConsoleOutputW";
    BreakArrayKernel32[19].nargs = 5;
    BreakArrayKernel32[19].breakProc = OnWriteConsoleOutputW;
    BreakArrayKernel32[19].dwFlags = BREAK_OUT;

    BreakArrayKernel32[20].funcName = "WriteConsoleOutputCharacterA";
    BreakArrayKernel32[20].nargs = 5;
    BreakArrayKernel32[20].breakProc = OnWriteConsoleOutputCharacterA;
    BreakArrayKernel32[20].dwFlags = BREAK_OUT;

    BreakArrayKernel32[21].funcName = "WriteConsoleOutputCharacterW";
    BreakArrayKernel32[21].nargs = 5;
    BreakArrayKernel32[21].breakProc = OnWriteConsoleOutputCharacterW;
    BreakArrayKernel32[21].dwFlags = BREAK_OUT;

    BreakArrayKernel32[22].funcName = 0L;
    BreakArrayKernel32[22].nargs = 0;
    BreakArrayKernel32[22].breakProc = 0L;
    BreakArrayKernel32[22].dwFlags = 0;

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
    si.wShowWindow = SW_SHOWDEFAULT;

    cmdline = ArgMaker::BuildCommandLine(argc, argv);

    // Make sure the master does not ignore Ctrl-C
    //SetConsoleCtrlHandler(0L, FALSE);

    ok = CreateProcess(
	    0L,		// Module name (not needed).
	    cmdline,	// Command line.
	    0L,		// Process handle will not be inheritable.
	    0L,		// Thread handle will not be inheritable.
	    FALSE,	// No handle inheritance.
	    DEBUG_PROCESS|CREATE_NEW_CONSOLE|CREATE_DEFAULT_ERROR_MODE,
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

    // Make sure we now ignore Ctrl-C
    //SetConsoleCtrlHandler(0L, TRUE);

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
	    switch (proc->nBreakCount < 3 ? ++(proc->nBreakCount) : 3) {
	    case 1:
		OnXFirstBreakpoint(proc, &debEvent); break;
	    case 2:
//		OnXSecondBreakpoint(proc, &debEvent); break;
	    case 3:
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

	default:
	    if (! debEvent.u.Exception.dwFirstChance) {
		// An exception was hit and it was not handled by the program.
		// Now it is time to get a backtrace.
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
	// Display the output debugging string.
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
#   define RETBUF_SIZE 2048
    BYTE retbuf[RETBUF_SIZE];
    ThreadInfo *tinfo;
    BreakInfo *binfo;
    int i;

    for (tinfo = proc->threadList; tinfo != 0L; tinfo = tinfo->nextPtr) {
	if (pDebEvent->dwThreadId == tinfo->dwThreadId) {
	    break;
	}
    }

    // Set up the memory that will serve as the place for our
    // intercepted function return points.
    //
    MakeSubprocessMemory(proc, RETBUF_SIZE, &(proc->pSubprocessMemory));

    // Fill the buffer with all breakpoint opcodes.
    //
    memset(retbuf, BRK_OPCODE, RETBUF_SIZE);
    WriteSubprocessMemory(proc, proc->pSubprocessMemory, retbuf, RETBUF_SIZE);

    // Set all breakpoints
    //
    for (i = 0; BreakPoints[i].dllName; i++) {
	for (binfo = BreakPoints[i].breakInfo; binfo->funcName; binfo++) {
	    SetBreakpoint(proc, binfo);
	}
    }
/*
    //  Make some memory for our stub that we place into the slave's address
    //  space and cause it to run it as if was part of the slave's application.
    //  This stub (or set of opcodes) calls LoadLibrary() to bring in our
    //  injector dll that acts as the receiver for "injecting" console events.
    //
    MakeSubprocessMemory(proc, sizeof(LOADLIBRARY_STUB), &pInjectorStub,
	    PAGE_EXECUTE_READWRITE);
    injectStub.operand_PUSH_value = (DWORD) pInjectorStub +
	    offsetof(LOADLIBRARY_STUB, data_DllName);
    injectStub.operand_MOV_EAX = (DWORD) GetProcAddress(GetModuleHandle(
	    "KERNEL32.DLL"),"LoadLibraryA");
    WriteSubprocessMemory(proc, pInjectorStub, &injectorStub,
	    sizeof(LOADLIBRARY_STUB));

    //  Save the instruction pointer so we can restore it later.
    //
    preStubContext.ContextFlags = CONTEXT_FULL;
    GetThreadContext(tinfo->hThread, &preStubContext);

    //  Set instruction pointer to run the Stub instead of continuing
    //  from where we are.
    //
    CONTEXT stubContext = preStubContext;
    stubContext.Eip = (DWORD) pInjectorStub;
    SetThreadContext(tinfo->hThread, &stubContext);
*/
    return;
#   undef RETBUF_SIZE
}

/*
 *----------------------------------------------------------------------
 *
 * ConsoleDebugger::OnXSecondBreakpoint --
 *
 *	This routine is called when the second breakpoint is hit and
 *	our stub has run and our injector is loaded.
 *
 * Results:
 *	None
 *
 *----------------------------------------------------------------------
 */

void
ConsoleDebugger::OnXSecondBreakpoint(Process *proc, LPDEBUG_EVENT pDebEvent)
{
/*    ThreadInfo *tinfo;

    for (tinfo = proc->threadList; tinfo != 0L; tinfo = tinfo->nextPtr) {
	if (pDebEvent->dwThreadId == tinfo->dwThreadId) {
	    break;
	}
    }

    SetThreadContext(tinfo->hThread, &preStubContext);

    // We should now remove the memory allocated in the sub process and
    // remove our stub.
    //
    RemoveSubprocessMemory(proc, pStubInjector);
*/
    return;
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
    // Do nothing.
    return;
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
 * OnXLoadDll --
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
 * OnXUnloadDll --
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
    PVOID funcPtr;

    if (proc->funcTable.Find((void *)info->funcName, &funcPtr) == TCL_ERROR)
    {
//	EXP_LOG("Unable to set breakpoint at %s", info->funcName);
	return FALSE;
    }

    // Set a breakpoint at the function start in the subprocess and
    // save the original code at the function start.
    //
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
    BYTE code;

    bpt = new Breakpoint;
    bpt->codePtr = funcPtr;
    bpt->codeReturnPtr = (PVOID) (proc->offset + (DWORD) proc->pSubprocessMemory);
    bpt->breakInfo = info;
    proc->offset += 2;
    bpt->nextPtr = proc->brkptList;
    proc->brkptList = bpt;

    ReadSubprocessMemory(proc, funcPtr, &bpt->code, sizeof(BYTE));
    code = BRK_OPCODE;
    WriteSubprocessMemory(proc, funcPtr, &code, sizeof(BYTE));
    return bpt;
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

    // if not committed memory abort.
    //
    if (!VirtualQueryEx(proc->hProcess, addr, &mbi, sizeof(mbi)) ||
	    mbi.State != MEM_COMMIT) {
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

    // if not committed memory abort
    if (!VirtualQueryEx(proc->hProcess, addr, &mbi, sizeof(mbi)) ||
	mbi.State != MEM_COMMIT)
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
	if (!(mbi.Protect & PAGE_READWRITE)) {
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

    context.ContextFlags = CONTEXT_FULL;
    GetThreadContext(tinfo->hThread, &context);

    if (! brkpt->returning) {
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
ConsoleDebugger::WriteMaster(CHAR *buf, DWORD len)
{
    Message *msg;
    msg = new Message;
    msg->bytes = (BYTE *) _strdup(buf);
    msg->length = len;
    msg->type = Message::TYPE_NORMAL;
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
ConsoleDebugger::MakeSubprocessMemory(Process *proc, SIZE_T amount,
    LPVOID *pBuff, DWORD access)
{
    if (dwPlatformId == VER_PLATFORM_WIN32_NT) {
        // We're on NT, so use VirtualAllocEx to allocate memory in the other
        // process' address space.  Alas, we can't just call VirtualAllocEx
        // since it's not defined in the Windows 95 KERNEL32.DLL.
	//
        static PFNVIRTALLEX pfnVirtualAllocEx = (PFNVIRTALLEX)
            GetProcAddress(GetModuleHandle("KERNEL32.DLL"),"VirtualAllocEx");

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

void
ConsoleDebugger::RemoveSubprocessMemory(Process *proc, LPVOID buff)
{
    if (dwPlatformId == VER_PLATFORM_WIN32_NT) {
        // We're on NT, so use VirtualAllocEx to control memory in the other
        // process' address space.  Alas, we can't just call VirtualAllocEx
        // since it's not defined in the Windows 95 KERNEL32.DLL.
	//
        static PFNVIRTALLEX pfnVirtualAllocEx = (PFNVIRTALLEX)
            GetProcAddress(GetModuleHandle("KERNEL32.DLL"),"VirtualAllocEx");

	pfnVirtualAllocEx(proc->hProcess, buff, 0, MEM_RELEASE, 0);
    } else {
	HANDLE hFileMapping;
	if (spMemMapping.Extract(buff, &hFileMapping) != TCL_OK) {
	}
	UnmapViewOfFile(buff);
	CloseHandle(hFileMapping);
    }
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

    WriteMaster(buf, bufpos);
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
    WriteMaster(buf, bufpos);
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
