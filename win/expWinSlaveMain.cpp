/* ----------------------------------------------------------------------------
 * expWinSlaveMain.cpp --
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
 * RCS: @(#) $Id: expWinSlaveMain.cpp,v 1.1.4.11 2002/03/12 07:59:14 davygrvy Exp $
 * ----------------------------------------------------------------------------
 */

#include "expWinSlave.hpp"


// local protos
static SpawnClientTransport *SpawnOpenClientTransport(const char *, CMclQueue<Message *> &);
static SlaveTrap *SlaveOpenTrap(const char *, int, char * const *, CMclQueue<Message *> &);
static int DoEvents(SpawnClientTransport *, SlaveTrap *, CMclQueue<Message *> &, CMclEvent &);

int
main (void)
{
    int argc;			    // Number of command-line arguments.
    char **argv;		    // Values of command-line arguments.
    SpawnClientTransport *tclient;  // class pointer of transport client.
    SlaveTrap *slaveCtrl;	    // trap method class pointer.
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
#ifndef IDE_LATCHED
#   define IDE_LATCHED 0
#endif
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
    slaveCtrl = SlaveOpenTrap(argv[2], argc-3, &argv[3], messageQ);

    //  Process messages.
    //
    code = DoEvents(tclient, slaveCtrl, messageQ, Shutdown);

    //  Close up.
    //
    ShutdownTcl();
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
SpawnOpenClientTransport(const char *method, CMclQueue<Message *> &mQ)
{
    if (!strcmp(method, "stdio")) {
	return new SpawnStdioClient(method, mQ);
    }
    else EXP_LOG1(MSG_IO_TRANSPRTARGSBAD, name);

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
 *	a polymorphed ExpSpawnTrap pointer or die.
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
DoEvents(SpawnClientTransport *transport,
    SlaveTrap *slaveCtrl, CMclQueue<Message *> &mQ, CMclEvent &sd)
{
    Message *msg;

    while (mQ.Get(msg, INFINITE, &sd)) {
	switch (msg->type) {
	case Message::TYPE_NORMAL:
	case Message::TYPE_ERROR:
	    //  Send stuff back to the parent.
	    //
	    transport->Write(msg);
	    break;

	case Message::TYPE_INSTREAM:
	    //  Send data into the slave.
	    //
	    slaveCtrl->Write(msg);
	    break;

	case Message::TYPE_FUNCTION:
	    //  Internal mode switching and info gathering.
	    //
	    break;
	}
    }
    return 0;
}
