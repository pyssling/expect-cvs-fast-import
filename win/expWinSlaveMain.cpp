/* ----------------------------------------------------------------------------
 * SlaveDrvMain.cpp --
 *
 *	Program entry for the Win32 slave driver helper application.
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
 * RCS: @(#) $Id: expWinSlaveMain.cpp,v 1.1.4.7 2002/03/11 05:36:37 davygrvy Exp $
 * ----------------------------------------------------------------------------
 */

#include "expWinInt.h"


#ifdef _MSC_VER
    // Only do this when MSVC++ is compiling us.
#   ifdef USE_TCL_STUBS
#	pragma comment (lib, "tclstub" \
		STRINGIFY(JOIN(TCL_MAJOR_VERSION,TCL_MINOR_VERSION)) ".lib")
#	if !defined(_MT) || !defined(_DLL) || defined(_DEBUG)
	    // This fixes a bug with how the Stubs library was compiled.
	    // The requirement for msvcrt.lib from tclstubXX.lib must
	    // be removed.  This bug has been fixed since 8.4a3, I beleive.
#	    pragma comment(linker, "-nodefaultlib:msvcrt.lib")
#	endif
#   else
#	error "Can only use with Stubs, sorry"
#   endif
#endif


// local protos
static SpawnClientTransport *SpawnOpenClientTransport(const char *name,
	CMclQueue<Message> &mQ);
static ExpSlaveTrap *ExpWinSlaveOpenTrap(const char *meth, int argc,
	char * const argv[], CMclQueue<Message> &mQ);
static int ExpWinMasterDoEvents(SpawnClientTransport *transport,
	ExpSlaveTrap *masterCtrl, CMclQueue<Message> &mQ);

extern "C" HMODULE hTclMod;

int
main (void)
{
    int argc;			    // Number of command-line arguments.
    char **argv;		    // Values of command-line arguments.
    SpawnClientTransport *tclient;  // class pointer of transport client.
    ExpSlaveTrap *slaveCtrl;	    // trap method class pointer.
    CMclQueue<Message> messageQ;    // Our message Queue we hand off to everyone.
    int code;			    // exitcode.
    CHAR *cmdLine;		    // commandline to use.

    //  We use a few APIs from Tcl, dynamically load it now.
    //
    ExpDynloadTclStubs();

    //  Select the unicode or ascii winprocs. Works in cooperation with
    //  Tcl_WinUtfToTChar().
    //
    ExpWinInit();

    //  Get our commandline.  MSVC++ doesn't like to debug spawned processes
    //  without a bit of help.  So help it out.
    //
#define IDE_LATCHED 0
#if defined(_DEBUG) && IDE_LATCHED
    if (IsDebuggerPresent()) {
#   ifdef _MSC_VER
	cmdLine = MsvcDbg_GetCommandLine();
#   else
#	error "Need Debugger control for this IDE"
#   endif
    } else {
#endif
    	cmdLine = GetCommandLine();

#if defined(_DEBUG) && IDE_LATCHED
    }
#endif

    //  Use our custom commandline parser to overcome bugs in the default
    //  crt library.
    //
    SetArgv(cmdLine, &argc, &argv);

    if (argc < 4) {
	EXP_LOG0(MSG_IO_ARGSWRONG);
    }

    //  Open the client side of our IPC transport that connects us back
    //  to Expect.
    //
    tclient = SpawnOpenClientTransport(argv[1], messageQ);

    //  Create the process to be intercepted within the trap method requested
    //  on the commandline.
    //
    slaveCtrl = ExpWinSlaveOpenTrap(argv[2], argc-3, &argv[3], messageQ);

    //  Process messages.
    //
    code = ExpWinMasterDoEvents(tclient, slaveCtrl, messageQ);

    Tcl_Finalize();
    FreeLibrary(hTclMod);
    return code;
}

/*
 *----------------------------------------------------------------------
 *  ExpWinSpawnOpenTransport --
 *
 *	The factory method for creating the client IPC transport from
 *	the name asked of it.
 *
 *  Returns:
 *	a polymorphed SpawnClientTransport pointer or die.
 *
 *----------------------------------------------------------------------
 */

SpawnClientTransport *
SpawnOpenClientTransport(const char *name, CMclQueue<Message> &mQ)
{
    // If the first 2 chars are 'm' and 'b', then it's a mailbox.
    //
/*    if (name[0] == 'm' && name[1] == 'b') {
	return new ExpSpawnMailboxClient(name, mQ);
    }*/
    /* 'sk' is a socket transport.  This is a no-op for now.
    else*/
    if (name[0] == 'p' && name[1] == 'i') {
	return new SpawnPipeClient(name, mQ);
    }
    // TODO: we can add more transports here when the time is right
    //
    else EXP_LOG1(MSG_IO_TRANSPRTARGSBAD, name);

    // not reached.
    return 0L;
}

/*
 *----------------------------------------------------------------------
 *  ExpWinSlaveOpenTrap --
 *
 *	The factory method for creating the trap class instance.
 *
 *  Returns:
 *	a polymorphed ExpSpawnTrap pointer or die.
 *
 *----------------------------------------------------------------------
 */

ExpSlaveTrap *
ExpWinSlaveOpenTrap(const char *meth, int argc, char * const argv[],
    CMclQueue<Message> &mQ)
{
    if (!strcmp(meth, "dbg")) {
	return new ExpSlaveTrapDbg(argc, argv, mQ);
    }
    /* TODO: a simple pipe trap would be good.
	[spawn -open <chanID>] does the same, though.
    else if (!strcmp(meth, "pipe")) {
	return new ExpSlaveTrapPipe(argc, argv);
    }*/
    else EXP_LOG1(MSG_IO_TRAPARGSBAD, meth);

    // not reached.
    return 0L;
}

/*
 *----------------------------------------------------------------------
 *  ExpWinMasterDoEvents --
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
ExpWinMasterDoEvents(SpawnClientTransport *transport,
    ExpSlaveTrap *masterCtrl, CMclQueue<Message> &mQ)
{
    Message &msg = Message();  // create a blank reference to receive into.

    while (mQ.Get(msg)) {
	switch (msg.type) {
	case Message::TYPE_NORMAL:
	case Message::TYPE_ERROR:
	    transport->Write(msg);
	}
    }
    return 0;
}
