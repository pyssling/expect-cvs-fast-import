/* ----------------------------------------------------------------------------
 * SlaveDrvMain.c --
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
 *	work by Gordon Chaffee <chaffee@bmrc.berkeley.edu>
 *
 * Copyright (c) 2001 Telindustrie, LLC
 *	work by David Gravereaux <davygrvy@pobox.com>
 *
 * ----------------------------------------------------------------------------
 * URLs:    http://expect.nist.gov/
 *	    http://expect.sf.net/
 *	    http://bmrc.berkeley.edu/people/chaffee/expectnt.html
 * ----------------------------------------------------------------------------
 * RCS: @(#) $Id: expWinSlaveMain.cpp,v 1.1.2.1 2001/11/09 01:17:40 davygrvy Exp $
 * ----------------------------------------------------------------------------
 */

#include "expWinInt.h"


#ifdef _MSC_VER
    /* Only do this when MSVC++ is compiling us. */
#   ifdef USE_TCL_STUBS
#	pragma comment (lib, "tclstub" \
		STRINGIFY(JOIN(TCL_MAJOR_VERSION,TCL_MINOR_VERSION)) ".lib")
#	if !defined(_MT) || !defined(_DLL) || defined(_DEBUG)
	    /*
	     * This fixes a bug with how the Stubs library was compiled.
	     * The requirement for msvcrt.lib from tclstubXX.lib must
	     * be removed.  This bug has been fixed since 8.4a3, I beleive.
	     */
#	    pragma comment(linker, "-nodefaultlib:msvcrt.lib")
#	endif
#   endif
#endif

/* local protos */
static void SetArgv(int *argcPtr, char ***argvPtr);


int
#ifdef _UNICODE
wmain (void)
#else
main (void)
#endif
{
    int argc;			    // Number of command-line arguments.
    char **argv;		    // Values of command-line arguments.
    ExpSpawnTransportCli *tclient;  // class pointer of transport client.
    ExpSlaveTrap *masterCtrl;	    // trap method class pointer.


    //  We use a few APIs from Tcl, dynamically load it now.
    ExpDynloadTclStubs();

    //  Select the unicode or ascii winprocs. Works in cooperation with
    //  Tcl_WinUtfToTChar().
    ExpWinInit();

    //  Use our custom commandline parser
    SetArgv(&argc, &argv);

    if (argc < 4) {
	EXP_LOG0(MSG_IO_ARGSWRONG);
    }

    // Open the client side of our IPC transport.
    tclient = ExpWinSpawnOpenTransport(argv[1]);

    //  Create the process to be intercepted within the trap method requested.
    masterCtrl = ExpWinSlaveOpenTrap(argv[2], argc-3, &argv[3]);

    //  Process events until the slave closes.
    //
    //  We block on input/events coming from the slave and
    //  input from the IPC coming from expect.
    return ExpWinSlaveEvents(tclient, masterCtrl);
}

/*
    HANDLE hConsoleInW;	// Master side (us), writeable input handle.
    HANDLE hConsoleOut;	// Master side (us), readable output handle.
    if ((hConsoleInW = CreateFile(
	    _T("CONIN$"),
	    GENERIC_WRITE,
	    FILE_SHARE_WRITE,
	    NULL,
	    OPEN_EXISTING,
	    0,
	    NULL)) == INVALID_HANDLE_VALUE)
    {
	EXP_LOG2(MSG_DT_CANTGETCONSOLEHANDLE, "CONIN$",
		ExpSyslogGetSysMsg(GetLastError()));
    }
    if ((hConsoleOut = CreateFile(
	    _T("CONOUT$"),
	    GENERIC_READ|GENERIC_WRITE,
	    FILE_SHARE_READ|FILE_SHARE_WRITE,
	    NULL, 
	    OPEN_EXISTING,
	    0,
	    NULL)) == INVALID_HANDLE_VALUE)
    {
	EXP_LOG2(MSG_DT_CANTGETCONSOLEHANDLE, "CONOUT$",
		ExpSyslogGetSysMsg(GetLastError()));
    }

*/

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

void
SetArgv(
    int *argcPtr,		/* Filled with number of argument strings. */
    char ***argvPtr)		/* Filled with argument strings in UTF (malloc'd). */
{
    char *p, *arg, *argSpace;
    char **argv;
    int argc, size, inquote, copy, slashes;
    TCHAR *cmdLineExt;
    Tcl_Encoding enc;
    Tcl_DString ds;

    cmdLineExt = GetCommandLine();

    /*
     * We might have compiled spawndrv.exe as a native NT app for unicode.
     * This needs to be compile-time set rather than run-time as
     * GetCommandLine() could be GetCommandLineW() rather than
     * GetCommandLineA().  Tcl_WinTCharToUtf() is run-time based and will
     * flop on it's face if used here in this case.
     */
#ifdef _UNICODE
    enc = Tcl_GetEncoding(NULL, "unicode");
#else
    enc = NULL;  /* Use system. cp1252? */
#endif

    Tcl_DStringInit(&ds);
    Tcl_ExternalToUtfDString(enc, (CONST char *)cmdLineExt, _tcslen(cmdLineExt), &ds);
    if (enc != NULL) {
	Tcl_FreeEncoding(enc);
    }

    /*
     * Precompute an overly pessimistic guess at the number of arguments
     * in the command line by counting non-space spans.
     */

    size = 2;
    for (p = Tcl_DStringValue(&ds); *p != '\0'; p++) {
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
	    (unsigned) (size * sizeof(char *) + Tcl_DStringLength(&ds) + 1));
    argv = (char **) argSpace;
    argSpace += size * sizeof(char *);
    size--;

    p = Tcl_DStringValue(&ds);
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

    Tcl_DStringFree(&ds);
}
