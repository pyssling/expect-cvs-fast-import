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
 * RCS: @(#) $Id: expWinInjectorMain.cpp,v 1.1.2.3 2002/06/19 06:42:45 davygrvy Exp $
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

	// open the mailbox
	ConsoleDebuggerIPC = new CMclMailbox(boxName);

	// Check status.
	err = ConsoleDebuggerIPC->Status();
	if (err != NO_ERROR) {
	    // Bad!
	    delete ConsoleDebuggerIPC;
	    return 666;
	}

	// forever loop receiving.
	while (ConsoleDebuggerIPC->GetAlertable(&ir, interrupt)) {
	    WriteConsoleInput(console, &ir, 1, &dwWritten);
	}

	delete ConsoleDebuggerIPC;
	return 0;
    }
};

CMclEvent interrupt;
CMclThread *injectorThread;
Injector *inject;

// It is documented that it "isn't a good idea to spawn threads from a DllMain".
// Pooie on you; this is what we will do.

BOOL WINAPI
DllMain (HINSTANCE hInst, ULONG ulReason, LPVOID lpReserved)
{
    HANDLE console;

    switch (ulReason) {
    case DLL_PROCESS_ATTACH:
	DisableThreadLibraryCalls(hInst);

	console = CreateFile("CONIN$", GENERIC_WRITE,
		FILE_SHARE_WRITE, 0L, OPEN_EXISTING, 0, 0L);

	inject = new Injector(console, &interrupt);
	injectorThread = new CMclThread(inject);
	break;

    case DLL_PROCESS_DETACH:
	interrupt.Set();
	injectorThread->Wait(INFINITE);
	delete inject;
	delete injectorThread;
	break;
    }
    return TRUE;
}
