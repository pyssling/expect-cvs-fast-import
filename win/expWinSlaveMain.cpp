/* ----------------------------------------------------------------------------
 * expWinSlaveMain.cpp --
 *
 *	Program entry for the Win32 slave tester application.
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
 * RCS: @(#) $Id: expWinSlaveMain.cpp,v 1.1.4.22 2002/06/23 09:35:43 davygrvy Exp $
 * ----------------------------------------------------------------------------
 */

#include "expWinSlave.hpp"


// local protos
static ClientTransport *OpenClientTransport(const char *, CMclQueue<Message *> &);
static SlaveTrap *SlaveOpenTrap(const char *, int, char * const *, CMclQueue<Message *> &);
static int DoEvents(ClientTransport *, SlaveTrap *, CMclQueue<Message *> &, CMclEvent &);
static char *OurGetCmdLine();

// Turns on/off special debugger hooks used in development.
//
#ifndef IDE_LATCHED
#   define IDE_LATCHED 0
#endif


int
main (void)
{
    int argc;			    // Number of command-line arguments.
    char **argv;		    // Values of command-line arguments.
    ClientTransport *transport;	    // class pointer of transport client.
    SlaveTrap *slave;		    // trap method class pointer.
    CMclQueue<Message *> messageQ;  // Our message Queue we hand off to everyone.
    CMclEvent Shutdown;		    // global shutdown for the event queue.
    int code;			    // exitcode.
    CHAR *cmdLine;		    // commandline to use.

    //  We need a few Tcl APIs in here.  Load it now.
    //
    DynloadTclStubs();

    //  Get our commandline.  MSVC++ doesn't like to debug spawned processes
    //  without a bit of help.  So help it out.
    //
    cmdLine = OurGetCmdLine();

    //  Use our custom commandline parser to overcome bugs in the default
    //  crt library as well as allowing us to hook at GetCommandLine().
    //
    SetArgv(cmdLine, &argc, &argv);

    if (argc < 4) {
	EXP_LOG0(MSG_IO_ARGSWRONG);
    }

    //  Open the client side of our IPC transport that connects us back
    //  to the parent (ie. the Expect extension).
    //
    transport = OpenClientTransport(argv[1], messageQ);

    //  Start the process to be intercepted within the trap method requested
    //  on the commandline (ie. run telnet in a debugger and trap OS calls).
    //
    slave = SlaveOpenTrap(argv[2], argc-3, &argv[3], messageQ);

    //  Process messages.
    //
    code = DoEvents(transport, slave, messageQ, Shutdown);

    //  Close up.
    //
    ShutdownTcl();
    return code;
}

/*
 *----------------------------------------------------------------------
 *  SpawnOpenTransport --
 *
 *	The factory method for creating the client IPC transport from
 *	the name asked of it.
 *
 *  Returns:
 *	a polymorphed SpawnClientTransport pointer or die.
 *
 *----------------------------------------------------------------------
 */

ClientTransport *
OpenClientTransport(const char *method, CMclQueue<Message *> &mQ)
{
    if (!strcmp(method, "stdio")) {
	return new ClientStdio(method, mQ);
    }
    else if (!strcmp(method, "conio")) {
	return new ClientConio(method, mQ);
    }
    else EXP_LOG1(MSG_IO_TRANSPRTARGSBAD, method);

    // not reached.
    return 0L;
}

/*
 *----------------------------------------------------------------------
 *  SlaveOpenTrap --
 *
 *	The factory method for creating the trap class instance.
 *
 *  Returns:
 *	a polymorphed SpawnTrap pointer or die.
 *
 *----------------------------------------------------------------------
 */

SlaveTrap *
SlaveOpenTrap(const char *method, int argc, char * const argv[],
    CMclQueue<Message *> &mQ)
{
    if (!strcmp(method, "dbg")) {
	return new SlaveTrapDbg(argc, argv, mQ);
    }
    else EXP_LOG1(MSG_IO_TRAPARGSBAD, method);

    // not reached.
    return 0L;
}

/*
 *----------------------------------------------------------------------
 *  DoEvents --
 *
 *	Process all events for the slavedrv application.  We are the
 *	master.  The slave is the process we are trapping.
 *
 *  Returns:
 *	an exit code.
 *
 *----------------------------------------------------------------------
 */

int
DoEvents(ClientTransport *transport, SlaveTrap *slave,
    CMclQueue<Message *> &msgQ, CMclEvent &sd)
{
    Message *msg;

    while (msgQ.Get(msg, INFINITE)) {
	switch (msg->type) {
	case Message::TYPE_NORMAL:
	case Message::TYPE_WARNING:
	case Message::TYPE_ERROR:
	    //  Send stuff back to the parent.
	    //
	    transport->Write(msg);
	    break;

	case Message::TYPE_INRECORD:
	    //  Send stuff to the slave.
	    //
	    slave->Write(msg);
	    break;

	case Message::TYPE_INSTREAM:
	    // do conversion here.  These get reposted back as
	    // TYPE_INRECORD messages.
	    MapToKeys(msg, msgQ);
	    break;

	case Message::TYPE_FUNCTION:
	    //  Internal mode switching and info gathering.
	    //
	    break;

	case Message::TYPE_SLAVEDONE:
	    delete slave, transport;
	    return 0;
	}
    }
    return 0;
}

/*
 *----------------------------------------------------------------------
 *  OurGetCmdLine --
 *
 *	Handles the logic for how to retrieve the commandline.
 *
 *  Returns:
 *	the commandline in a single string.
 *
 *----------------------------------------------------------------------
 */

char *
OurGetCmdLine()
{
#if defined(_DEBUG) && IDE_LATCHED
    if (IsDebuggerPresent()) {
#   ifdef _MSC_VER
	CHAR * MsvcDbg_GetCommandLine();
	return MsvcDbg_GetCommandLine();
#   else
#	error "Need Debugger control for this IDE"
#   endif
    } else {
#endif
    	return GetCommandLine();

#if defined(_DEBUG) && IDE_LATCHED
    }
#endif
}

#if defined(_DEBUG) && defined(_MSC_VER) && IDE_LATCHED
CHAR *
MsvcDbg_GetCommandLine(void)
{
    HKEY root;
    HANDLE event1;
    CHAR pidChar[33], *buf;
    DWORD type = REG_SZ, size = 0;
    int pid;     // <- this is read by the parent's debugger.

    pid = GetCurrentProcessId();

    event1 = CreateEvent(0L, FALSE, FALSE, "SpawnStartup");
    SetEvent(event1);
    CloseHandle(event1);

    // >>>>   IMPORTANT!   <<<<

    // Set a soft break on the next line for this to work.
    // It is essential that the app stops here during startup
    // and syncs to the parent properly.
    __asm nop;

    // >>>> END IMPORTANT! <<<<

    itoa(pid, pidChar, 10);
    RegOpenKeyEx(HKEY_LOCAL_MACHINE, "Software\\Tomasoft\\MsDevDbgCtrl",
	    0, KEY_ALL_ACCESS, &root);
    RegQueryValueEx(root, pidChar, 0, &type, 0L, &size);
    buf = new CHAR [size];
    RegQueryValueEx(root, pidChar, 0, &type, (LPBYTE) buf, &size);
    RegDeleteValue(root, pidChar);
    RegCloseKey(root);
    return buf;
}
#endif  // _DEBUG && _MSC_VER && IDE_LATCHED
