/* ----------------------------------------------------------------------------
 * expWinConsoleDebugger.hpp --
 *
 *	Console debugger class declared here.
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
 * RCS: @(#) $Id: expWinConsoleDebugger.hpp,v 1.1.2.27 2002/06/25 22:50:07 davygrvy Exp $
 * ----------------------------------------------------------------------------
 */

#ifndef INC_expWinConsoleDebugger_hpp__
#define INC_expWinConsoleDebugger_hpp__

#include "expWinSlave.hpp"
#include "TclHash.hpp"	    // for the hash table template.
#include <imagehlp.h>



//  This is our debugger.  We run it in a thread. 
//
class ConsoleDebugger : public CMclThreadHandler, ArgMaker
{
public:
    ConsoleDebugger(int _argc, char * const *_argv, CMclQueue<Message *> &_mQ);
    ~ConsoleDebugger();

    void WriteRecord (INPUT_RECORD *ir);
    void EnterInteract (HANDLE OutConsole);
    void ExitInteract (void);

private:
    virtual unsigned ThreadHandlerProc(void);

    class ThreadInfo {
	friend class ConsoleDebugger;
	HANDLE	    hThread;
	DWORD	    dwThreadId;
	DWORD	    nargs;
	DWORD	    args[16];	// Space for saving 16 args.  We need this
				// space while we are waiting for the return
				// value for the function.
	LPCONTEXT   context;	// Current context.
	ThreadInfo  *nextPtr;	// Linked list.
    };

    // forward reference.
    class Process;
    class Breakpoint;

    class BreakInfo {
	friend class ConsoleDebugger;
	const char  *funcName;	// Name of function to intercept.
	DWORD	    nargs;	// Number of arguments.
	void (ConsoleDebugger::*breakProc)(Process *, ThreadInfo *,
		Breakpoint *, PDWORD, DWORD);
				// Function to call when the breakpoint is hit.
#	define BREAK_IN  1	// Call handler on the way in.
#	define BREAK_OUT 2	// Call handler on the way out.
	DWORD   dwFlags;	// Bits for direction to call handler in.
    };

    class DllBreakpoints {
	friend class ConsoleDebugger;
	const char  *dllName;
	BreakInfo   *breakInfo;
    };

    class Breakpoint {
	friend class ConsoleDebugger;
	Breakpoint() : returning(FALSE), origRetAddr(0), threadInfo(0L) {}
	BOOL	    returning;	    // Is this a returning breakpoint?
	BYTE	    code;	    // Original code.
	PVOID	    codePtr;	    // Address of original code.
	PVOID	    codeReturnPtr;  // Address of return breakpoint.
	DWORD	    origRetAddr;    // Original return address.
	BreakInfo   *breakInfo;	    // Information about the breakpoint.
	ThreadInfo  *threadInfo;    // If this breakpoint is for a specific
				    //  thread.
	Breakpoint  *nextPtr;	    // Linked list.
    };

    class Module {
	friend class ConsoleDebugger;
	BOOL	    loaded;
	HANDLE	    hFile;
	LPVOID	    baseAddr;
	PCHAR	    modName;
	PIMAGE_DEBUG_INFORMATION dbgInfo;
    };

    typedef Tcl::Hash<PVOID, TCL_STRING_KEYS> STRING2PTR;
    typedef Tcl::Hash<Module*, TCL_ONE_WORD_KEYS> PTR2MODULE;

    //  There is one of these instances for each process that we are
    //  controlling.
    //
    class Process {
	friend class ConsoleDebugger;
	Process() : threadList(0L), threadCount(0), brkptList(0L),
	    lastBrkpt(0L), offset(0), nBreakCount(0), consoleHandlesMax(0),
	    hProcess(0L), pSubprocessMemory(0), exeModule(0L) {}
	ThreadInfo  *threadList;	// List of threads in the subprocess.
	Breakpoint  *brkptList;		// List of breakpoints in the subprocess.
	Breakpoint  *lastBrkpt;		// Last breakpoint hit.
	DWORD	    offset;		// Breakpoint offset in allocated mem.
	DWORD	    nBreakCount;	// Number of breakpoints hit.
	DWORD	    consoleHandles[100];// A list of input console handles.
	DWORD	    consoleHandlesMax;
	HANDLE	    hProcess;		// Handle of process.
	DWORD	    pid;		// Global process id.
	DWORD	    threadCount;	// Number of threads in process.
	PVOID	    pSubprocessMemory;	// Pointer to allocated memory in subprocess.
	STRING2PTR  funcTable;		// Function table name to address mapping.
	PTR2MODULE  moduleTable;	// Win32 modules that have been loaded.
	Module	    *exeModule;		// Executable module info.
	Process	    *nextPtr;		// Linked list.
    };

#   include <pshpack1.h>
#   ifdef _M_IX86
    struct LOADLIBRARY_STUB
    {
        BYTE    instr_PUSH;
        DWORD   operand_PUSH_value;
        BYTE    instr_MOV_EAX;
        DWORD   operand_MOV_EAX;
        WORD    instr_CALL_EAX;
        BYTE    instr_INT_3;
        char    data_DllName[MAX_PATH];

        LOADLIBRARY_STUB() :
            instr_PUSH(0x68), instr_MOV_EAX(0xB8),
            instr_CALL_EAX(0xD0FF), instr_INT_3(0xCC)
	{
		// Just a temporary hack.. just ignore for now.
#	ifdef _DEBUG
	    strcpy(data_DllName, "D:\\expect_wslive\\expect_win32_take2\\win\\Debug\\injector.dll");
#	else
	    strcpy(data_DllName, "injector.dll");
#	endif
	}
    };
#   else
#	error need correct stub loader opcodes for this hardware.
#   endif
#   include <poppack.h>

    //  Direct debug event handlers.
    //
    void OnXFirstBreakpoint	(Process *, LPDEBUG_EVENT);
    void OnXSecondBreakpoint	(Process *, LPDEBUG_EVENT);
    void OnXThirdBreakpoint	(Process *, LPDEBUG_EVENT);
    void OnXBreakpoint		(Process *, LPDEBUG_EVENT);
    void OnXCreateProcess	(Process *, LPDEBUG_EVENT);
    void OnXCreateThread	(Process *, LPDEBUG_EVENT);
    void OnXDeleteThread	(Process *, LPDEBUG_EVENT);
    void OnXLoadDll		(Process *, LPDEBUG_EVENT);
    void OnXUnloadDll		(Process *, LPDEBUG_EVENT);
    void OnXDebugString		(Process *, LPDEBUG_EVENT);
    void OnXRip			(Process *, LPDEBUG_EVENT);
    void OnXSecondChanceException (Process *, LPDEBUG_EVENT);
    void OnXSingleStep		(Process *, LPDEBUG_EVENT);

    //  Our breakpoint handlers (indirect).  Called from OnXBreakpoint().
    //
    void OnAllocConsole		(Process *, ThreadInfo *, Breakpoint *, PDWORD, DWORD);
    void OnBeep			(Process *, ThreadInfo *, Breakpoint *, PDWORD, DWORD);
    void OnFillConsoleOutputCharacter (Process *, ThreadInfo *, Breakpoint *, PDWORD, DWORD);
    void OnFreeConsole		(Process *, ThreadInfo *, Breakpoint *, PDWORD, DWORD);
    void OnGetStdHandle		(Process *, ThreadInfo *, Breakpoint *, PDWORD, DWORD);
    void OnIsWindowVisible	(Process *, ThreadInfo *, Breakpoint *, PDWORD, DWORD);
    void OnOpenConsoleW		(Process *, ThreadInfo *, Breakpoint *, PDWORD, DWORD);
    void OnSetConsoleActiveScreenBuffer	(Process *, ThreadInfo *, Breakpoint *, PDWORD, DWORD);
    void OnSetConsoleCP		(Process *, ThreadInfo *, Breakpoint *, PDWORD, DWORD);
    void OnSetConsoleCursorInfo (Process *, ThreadInfo *, Breakpoint *, PDWORD, DWORD);
    void OnSetConsoleCursorPosition (Process *, ThreadInfo *, Breakpoint *, PDWORD, DWORD);
    void OnSetConsoleMode	(Process *, ThreadInfo *, Breakpoint *, PDWORD, DWORD);
    void OnSetConsoleOutputCP	(Process *, ThreadInfo *, Breakpoint *, PDWORD, DWORD);
    void OnSetConsoleWindowInfo	(Process *, ThreadInfo *, Breakpoint *, PDWORD, DWORD);
    void OnScrollConsoleScreenBuffer (Process *, ThreadInfo *, Breakpoint *, PDWORD, DWORD);
    void OnWriteConsoleA	(Process *, ThreadInfo *, Breakpoint *, PDWORD, DWORD);
    void OnWriteConsoleW	(Process *, ThreadInfo *, Breakpoint *, PDWORD, DWORD);
    void OnWriteConsoleOutputA	(Process *, ThreadInfo *, Breakpoint *, PDWORD, DWORD);
    void OnWriteConsoleOutputW	(Process *, ThreadInfo *, Breakpoint *, PDWORD, DWORD);
    void OnWriteConsoleOutputCharacterA	(Process *, ThreadInfo *, Breakpoint *, PDWORD, DWORD);
    void OnWriteConsoleOutputCharacterW	(Process *, ThreadInfo *, Breakpoint *, PDWORD, DWORD);

    // Internal utilities
    //
    Process *ProcessNew		();
    void ProcessFree		(Process *);
    DWORD CommonDebugger	();
    BOOL SetBreakpoint		(Process *, BreakInfo *);
    Breakpoint *SetBreakpointAtAddr (Process *, BreakInfo *, PVOID);
    int LoadedModule		(Process *, HANDLE, LPVOID, int, LPVOID, DWORD);
    BOOL ReadSubprocessMemory	(Process *, LPVOID, LPVOID, DWORD);
    BOOL WriteSubprocessMemory	(Process *, LPVOID, LPVOID, DWORD);
    void MakeSubprocessMemory   (Process *, SIZE_T, LPVOID *, DWORD = PAGE_READWRITE);
    BOOL RemoveSubprocessMemory (Process *, LPVOID);
    int ReadSubprocessStringA	(Process *, PVOID, PCHAR, int);
    int ReadSubprocessStringW	(Process *, PVOID, PWCHAR, int);
    void CreateVtSequence	(Process *, COORD, DWORD);
//    void RefreshScreen		(void);

    // send info back to the parent
    void WriteMasterCopy	(CHAR *, DWORD);
    void WriteMasterWarning	(CHAR *, DWORD);
    void WriteMasterError	(CHAR *, DWORD);


    // announce we are done.
    void NotifyDone		();

    // The arrays of functions where we set breakpoints
    //
    BreakInfo	BreakArrayKernel32[23];
    BreakInfo	BreakArrayUser32[2];
    DllBreakpoints BreakPoints[3];

    // private vars
    //
    Process	*ProcessList;   // Top of linked list of Process instances.
    HANDLE	hMasterConsole;	// Master console handle (us).
    DWORD	dwPlatformId;	// what OS are we under?
    DWORD	MasterConsoleInputMode;// Current flags for the master console.
    COORD	ConsoleSize;    // Size of the console in the slave.
    COORD	CursorPosition; // Coordinates of the cursor in the slave.
    char	*SymbolPath;
    UINT	ConsoleCP;	// console input code page of the slave.
    UINT	ConsoleOutputCP;// console output code page of the slave.
    BOOL	CursorKnown;    // Do we know where the slave's cursor is?
    CONSOLE_CURSOR_INFO CursorInfo;// Cursor info structure that is a copy of
				   //  the slave's.
    int		argc;		// Debugee process commandline count
    char * const * argv;	// Debugee process commandline args

    // Thread-safe message queue used for communication back to the parent.
    //
    CMclQueue<Message *> &mQ;

    // This critical section is set when breakpoints are running.
    //
//    CMclCritSec bpCritSec;

    LPVOID	pStartAddress;	// Start address of the top process.
    BYTE	originalExeEntryPointOpcode;

    LOADLIBRARY_STUB injectorStub;// opcodes we use to force load our injector
				//  dll.
    PVOID	pInjectorStub;	// Pointer to memory in sub process used
				//  for the injector's loader.
    CONTEXT	preStubContext; // Thread context info before switching to run
				//  the stub.

    typedef Tcl::Hash<HANDLE, TCL_ONE_WORD_KEYS> PTR2HANDLE;
    PTR2HANDLE	spMemMapping;	// Used on Win9x to associate the file mapping
				//  handle to the memory address it provides.

    // A couple NT routines we'll might need when running
    // on NT/2K/XP
    typedef LPVOID (__stdcall *PFNVIRTUALALLOCEX)(HANDLE,LPVOID,SIZE_T,DWORD,DWORD);
    typedef BOOL (__stdcall *PFNVIRTUALFREEEX)(HANDLE,LPVOID,SIZE_T,DWORD);

    PFNVIRTUALALLOCEX pfnVirtualAllocEx;
    PFNVIRTUALFREEEX pfnVirtualFreeEx;

    CMclMailbox *injectorIPC;	// IPC transfer mechanism to the injector dll.

    bool interacting;
    HANDLE interactingConsole;
};

#endif // INC_expWinConsoleDebugger_hpp__
