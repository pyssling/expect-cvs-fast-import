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
 * RCS: @(#) $Id: expWinSlaveMain.cpp,v 1.1.4.2 2002/03/08 23:29:47 davygrvy Exp $
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
static ExpSpawnClientTransport *ExpWinSpawnOpenClientTransport(const char *name);
static ExpSlaveTrap *ExpWinSlaveOpenTrap(const char *meth, int argc, char * const argv[]);
static int ExpWinMasterDoEvents(ExpSpawnClientTransport *transport, ExpSlaveTrap *masterCtrl);
static void SetArgv(int *argcPtr, char ***argvPtr);


int
main (void)
{
    int argc;				// Number of command-line arguments.
    char **argv;			// Values of command-line arguments.
    ExpSpawnClientTransport *tclient;	// class pointer of transport client.
    ExpSlaveTrap *slaveCtrl;		// trap method class pointer.


    //  We use a few APIs from Tcl, dynamically load it now.
    //
    ExpDynloadTclStubs();

    //  Select the unicode or ascii winprocs. Works in cooperation with
    //  Tcl_WinUtfToTChar().
    //
    ExpWinInit();

    //  Use our custom commandline parser.
    //
    SetArgv(&argc, &argv);

    if (argc < 4) {
	return 3;
    }

    //  Open the client side of our IPC transport that connects us back
    //  to Expect.
    //
    tclient = ExpWinSpawnOpenClientTransport(argv[1]);

    //  Create the process to be intercepted within the trap method requested
    //  on the commandline.
    //
    slaveCtrl = ExpWinSlaveOpenTrap(argv[2], argc-3, &argv[3]);

    //  Process events until the slave closes.
    //
    //  We block on input/events coming from the slave and
    //  input from the IPC coming from expect.
    //
    return ExpWinMasterDoEvents(tclient, slaveCtrl);
}

/*
 *----------------------------------------------------------------------
 *  ExpWinSpawnOpenTransport --
 *
 *	The factory method for creating the client IPC transport from
 *	the name asked of it.
 *
 *  Returns:
 *	a polymorphed ExpSpawnClientTransport pointer or NULL for an error.
 *
 *----------------------------------------------------------------------
 */

ExpSpawnClientTransport *
ExpWinSpawnOpenClientTransport(const char *name)
{
    // If the first 2 chars are 'm' and 'b', then it's a mailbox.
    //
    if (name[0] == 'm' && name[1] == 'b') {
	return new ExpSpawnMailboxClient(name);
    }
    /* 'sk' is a socket transport.  This is a no-op for now.
    else if (name[0] == 's' && name[1] == 'k') {
	return new ExpSpawnSocketCli(name);
    } */
    // TODO: we can add more transports here when the time is right
    //
    else return 0L;
}

/*
 *----------------------------------------------------------------------
 *  ExpWinSlaveOpenTrap --
 *
 *	The factory method for creating the trap class instance.
 *
 *  Returns:
 *	a polymorphed ExpSpawnTrap pointer or NULL for an error.
 *
 *----------------------------------------------------------------------
 */

ExpSlaveTrap *
ExpWinSlaveOpenTrap(const char *meth, int argc, char * const argv[])
{
    if (!strcmp(meth, "dbg")) {
	return new ExpSlaveTrapDbg(argc, argv);
    }
    /* TODO: a simple pipe trap would be good.
	[spawn -open <chanID>] does the same, though.
    else if (!strcmp(meth, "pipe")) {
	return new ExpSlaveTrapPipe(argc, argv);
    }*/
    else return 0L;
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
ExpWinMasterDoEvents(ExpSpawnClientTransport *transport, ExpSlaveTrap *masterCtrl)
{
    CMclWaitableCollection stuff;
    return 0;
}

/*
 *-------------------------------------------------------------------------
 *
 * SetArgv --
 *
 *	Parse the Windows command line string into argc/argv.  Done here
 *	because we don't trust the builtin argument parser in crt0.  
 *	Windows applications are responsible for breaking their command
 *	line into arguments.
 *
 *	2N backslashes + quote -> N backslashes + begin quoted string
 *	2N + 1 backslashes + quote -> literal
 *	N backslashes + non-quote -> literal
 *	quote + quote in a quoted string -> single quote
 *	quote + quote not in quoted string -> empty string
 *	quote -> begin quoted string
 *
 * Results:
 *	Fills argcPtr with the number of arguments and argvPtr with the
 *	array of arguments.
 *
 * Side effects:
 *	Memory allocated.
 *
 *--------------------------------------------------------------------------
 */

static void
SetArgv(
    int *argcPtr,	// Filled with number of argument strings.
    char ***argvPtr)	// Filled with argument strings in UTF (malloc'd).
{
    char *p, *arg, *argSpace;
    char **argv;
    int argc, size, inquote, copy, slashes;
    Tcl_DString cmdLine;
    WCHAR *cmdLineUni;

    Tcl_DStringInit(&cmdLine);

#ifdef _DEBUG
    if (IsDebuggerPresent()) {
#   ifdef _MSC_VER

	//  There will be a unicode loss here.  I don't feel it's a bad
	//  trade-off when running in a debugger.
	//  cp1251 != utf-8, though.
	//
	Tcl_DStringAppend(&cmdLine, MsvcDbg_GetCommandLine(), -1);

#   else
#	error "Need Debugger control for this IDE"
#   endif
    } else {
#endif

	// Always get the unicode commandline because *ALL* Win32 platforms
	// support it.
	//
	cmdLineUni = GetCommandLineW();
	// calculate the size needed.
	size = WideCharToMultiByte(CP_UTF8, 0, cmdLineUni, -1, 0, 0, NULL,
		NULL);
	Tcl_DStringSetLength(&cmdLine, size);
	WideCharToMultiByte(CP_UTF8, 0, cmdLineUni, -1,
		Tcl_DStringValue(&cmdLine), size, NULL, NULL);

#ifdef _DEBUG
    }
#endif

    /*
     * Precompute an overly pessimistic guess at the number of arguments
     * in the command line by counting non-space spans.
     */

    size = 2;
    for (p = Tcl_DStringValue(&cmdLine); *p != '\0'; p++) {
	if ((*p == ' ') || (*p == '\t')) {	/* INTL: ISO space. */
	    size++;
	    while ((*p == ' ') || (*p == '\t')) { /* INTL: ISO space. */
		p++;
	    }
	    if (*p == '\0') {
		break;
	    }
	}
    }
    argSpace = (char *) ckalloc(
	    (unsigned) (size * sizeof(char *) + Tcl_DStringLength(&cmdLine) + 1));
    argv = (char **) argSpace;
    argSpace += size * sizeof(char *);
    size--;

    p = Tcl_DStringValue(&cmdLine);
    for (argc = 0; argc < size; argc++) {
	argv[argc] = arg = argSpace;
	while ((*p == ' ') || (*p == '\t')) {	/* INTL: ISO space. */
	    p++;
	}
	if (*p == '\0') {
	    break;
	}

	inquote = 0;
	slashes = 0;
	while (1) {
	    copy = 1;
	    while (*p == '\\') {
		slashes++;
		p++;
	    }
	    if (*p == '"') {
		if ((slashes & 1) == 0) {
		    copy = 0;
		    if ((inquote) && (p[1] == '"')) {
			p++;
			copy = 1;
		    } else {
			inquote = !inquote;
		    }
                }
                slashes >>= 1;
            }

            while (slashes) {
		*arg = '\\';
		arg++;
		slashes--;
	    }

	    if ((*p == '\0')
		    || (!inquote && ((*p == ' ') || (*p == '\t')))) { /* INTL: ISO space. */
		break;
	    }
	    if (copy != 0) {
		*arg = *p;
		arg++;
	    }
	    p++;
        }
	*arg = '\0';
	argSpace = arg + 1;
    }
    argv[argc] = NULL;

    *argcPtr = argc;
    *argvPtr = argv;

    Tcl_DStringFree(&cmdLine);
}
