/* ----------------------------------------------------------------------------
 * expWinInjectorMain.cpp --
 *
 *	Console event injector DLL that's loaded into the slave's address space
 *	used by the ConsoleDebugger class for "writing" to the slave.
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
 * RCS: @(#) $Id: expWinInjectorMain.cpp,v 1.1.2.8 2002/06/21 03:01:51 davygrvy Exp $
 * ----------------------------------------------------------------------------
 */

#include "CMcl.h"

class Injector : public CMclThreadHandler
{
    CMclMailbox *ConsoleDebuggerIPC;
    HANDLE console;
    CMclEvent *interrupt;

public:

    Injector(HANDLE _console, CMclEvent *_interrupt) 
	: console(_console), interrupt(_interrupt), ConsoleDebuggerIPC(0L)
    {}
    
    ~Injector() {}

private:

    virtual unsigned ThreadHandlerProc(void)
    {
	CHAR	boxName[50];
	DWORD	err, dwWritten;
	INPUT_RECORD ir;

	wsprintf(boxName, "ExpectInjector_pid%d", GetCurrentProcessId());

	// Create the shared memory IPC transfer mechanism by name
	// (a mailbox).
	ConsoleDebuggerIPC = 
		new CMclMailbox(10, sizeof(INPUT_RECORD), boxName);

	// Check status.
	switch (err = ConsoleDebuggerIPC->Status()) {
	case NO_ERROR:
	    OutputDebugString("Expect's injector DLL loaded and ready.\n");
	    break;
	case ERROR_ALREADY_EXISTS:
	    OutputDebugString("Expect's injector DLL could not start IPC: "
		    "another mailbox of the same name already exists.\n");
	    break;
	default:
	    OutputDebugString(GetSysMsg(err));
	}

	if (err != NO_ERROR) {
	    delete ConsoleDebuggerIPC;
	    return 0x666;
	}

	// forever loop receiving INPUT_RECORDs over IPC.
	while (ConsoleDebuggerIPC->GetAlertable(&ir, interrupt)) {
	    // Stuff it into our slave console as if it had been entered
	    // by the user.
	    WriteConsoleInput(console, &ir, 1, &dwWritten);
	}

	delete ConsoleDebuggerIPC;
	return 0;
    }

    const char *GetSysMsg(DWORD id)
    {
	int chars;

	chars = wsprintf(sysMsgSpace,
		"Expect's injector DLL could not start IPC: [%u] ", id);
	FormatMessage(FORMAT_MESSAGE_FROM_SYSTEM |
		FORMAT_MESSAGE_IGNORE_INSERTS |
		FORMAT_MESSAGE_MAX_WIDTH_MASK, 0L, id, 0,
		sysMsgSpace+chars, (512-chars),	0);
	return sysMsgSpace;
    }

    char sysMsgSpace[512];
};

CMclEvent *interrupt;
CMclThread *injectorThread;
Injector *inject;
HANDLE console;

BOOL WINAPI
DllMain (HINSTANCE hInst, ULONG ulReason, LPVOID lpReserved)
{

    switch (ulReason) {
    case DLL_PROCESS_ATTACH:
	/*MessageBox(NULL, "hi mom!", "lala", MB_OK|MB_SETFOREGROUND);*/
	//DisableThreadLibraryCalls(hInst);
	//console = CreateFile("CONIN$", GENERIC_WRITE,
	//	FILE_SHARE_WRITE, 0L, OPEN_EXISTING, 0, 0L);
	//interrupt = new CMclEvent();
	//inject = new Injector(console, interrupt);
	//injectorThread = new CMclThread(inject);
	break;
    case DLL_PROCESS_DETACH:
	//interrupt->Set();
	//injectorThread->Wait(INFINITE);
	//CloseHandle(console);
	//delete interrupt;
	//delete inject;
	//delete injectorThread;
	break;
    }
    return TRUE;
}
