/* ----------------------------------------------------------------------------
 * expWinUtils.cpp --
 *
 *	Misc stuff.
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
 * RCS: @(#) $Id: expWinUtils.cpp,v 1.1.2.5 2002/03/12 07:59:14 davygrvy Exp $
 * ----------------------------------------------------------------------------
 */

#include "expWinUtils.hpp"
#include "slavedrvmc.h"
#include <ctype.h>
#include <string.h>

/*
 *----------------------------------------------------------------------
 *
 * ArgMaker::BuildCommandLine --
 *
 *	Takes an array form and turns it into a single string according
 *	to the rules of quoting needed for windows.
 *
 * Results:
 *	The new string.
 *
 * Side Effects:
 *	Memory is allocated.
 *
 *----------------------------------------------------------------------
 */

char *
ArgMaker::BuildCommandLine(
    int argc,			// Number of arguments.
    char *const *argv)		// Argument strings (in ascii).
{
    const char *arg, *start, *special;
    char *out;
    int quote, i, need;

    // Guess how large we are.
    for (i = need = 0; i < argc; i++) {
	need += strlen(argv[i]) + 10;
    }

    out = new char [need];
    *out = '\0';

    for (i = 0; i < argc; i++) {
	arg = argv[i];
	if (i != 0) {
	    strcat(out, " ");
	}

	quote = 0;
	if (arg[0] == '\0') {
	    quote = 1;
	} else {
	    for (start = arg; *start != '\0'; start++) {
		if (isspace(*start) || *start == '"' ) {
		    quote = 1;
		    break;
		}
	    }
	}
	if (quote) {
	    strcat(out, "\"");
	}

	start = arg;	    
	for (special = arg; ; ) {
	    if (*special == '"' && quote) {
		strncat(out, start, special - start);
		// replace a single double quote with 2 double quotes.
		strcat(out, "\"\"");
		start = special + 1;
	    }
	    if (*special == '\0') {
		break;
	    }
	    special++;
	}
	strncat(out, start, special - start);
	if (quote) {
	    strcat(out, "\"");
	}
    }
    return out;
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

void
SetArgv(
    const char *cmdLine,// commandline string.
    int *argcPtr,	// Filled with number of argument strings.
    char ***argvPtr)	// Filled with argument strings in UTF (alloc'd with new).
{
    char *arg, *argSpace;
    char **argv;
    const char *p;
    int inquote, copy, slashes;
    size_t size, argc;

    // Precompute an overly pessimistic guess at the number of arguments
    // in the command line by counting non-space spans.
    //
    size = 2;
    for (p = cmdLine; *p != '\0'; p++) {
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
    argSpace = new char [(size * sizeof(char *)) + strlen(cmdLine) + 1];
    argv = (char **) argSpace;
    argSpace += size * sizeof(char *);
    size--;

    p = cmdLine;
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
    argv[argc] = 0L;

    *argcPtr = argc;
    *argvPtr = argv;
}


static char sysMsgSpace[1024];

/* local protos */
static TCHAR *Exp95Log (DWORD errCode, char *errData[], int cnt);

#define GETSEVERITY(code)   (UCHAR)((code >> 30) & 0x3) 
#define GETFACILITY(code)   (WORD)((code >> 16) & 0x0FFF)
#define GETCODE(code)	    (WORD)(code & 0xFFFF)

/*
 *----------------------------------------------------------------------
 *
 * ExpWinSyslog --
 *
 *	Logs error messages to the system application event log.
 *	It is normally called through the macro EXP_LOG() when
 *	errors occur in the slave driver process, but it can be
 *	used elsewhere.
 *
 * Results:
 *	None
 *
 *----------------------------------------------------------------------
 */

void
ExpWinSyslog (DWORD errCode, ...)
{
    va_list args;
    char *errData[10];
    int cnt = 0;
    char *errMsg;
    static char codeBuf[33];
    DWORD dwWritten;
    char *file;
    int line;
    static char fileInfo[MAX_PATH];

    va_start(args,errCode);

    /* Get the file info */
    file = va_arg(args, char *);
    line = va_arg(args, int);
    wsprintfA(fileInfo, "%s(%d)", file, line);
    errData[cnt++] = fileInfo;

    /* Set the textual severity */
    switch(GETSEVERITY(errCode)) {
	case STATUS_SEVERITY_WARNING:
	    errData[cnt++] = "Warning"; break;
	case STATUS_SEVERITY_SUCCESS:
	    errData[cnt++] = "Success"; break;
	case STATUS_SEVERITY_INFORMATIONAL:
	    errData[cnt++] = "Info"; break;
	case STATUS_SEVERITY_FATAL:
	    errData[cnt++] = "Fatal"; break;
    }

    /* Set the textual Facility */
    switch(GETFACILITY(errCode)) {
	case FACILITY_WINSOCK:
	    errData[cnt++] = "Winsock IPC"; break;
	case FACILITY_SYSTEM:
	    errData[cnt++] = "System"; break;
	case FACILITY_STUBS:
	    errData[cnt++] = "Stubs"; break;
	case FACILITY_NAMEDPIPE:
	    errData[cnt++] = "NamedPipe IPC"; break;
	case FACILITY_MSPROTO:
	    errData[cnt++] = "Master/Slave Protocol"; break;
	case FACILITY_MAILBOX:
	    errData[cnt++] = "MailBoxing IPC"; break;
	case FACILITY_IO:
	    errData[cnt++] = "I/O general"; break;
	case FACILITY_DBGTRAP:
	    errData[cnt++] = "Debug/Trap"; break;
    }
    /* Set the textual Code */
    errData[cnt++] = codeBuf;
    wsprintfA(codeBuf, "0x%04X", GETCODE(errCode));

    /* set everyone else */
    while ((errData[cnt] = va_arg(args, char *)) != NULL) cnt++;
    va_end(args);

    /* format this error according to the message catalog contained in the exe. */
    errMsg = Exp95Log(errCode, errData, cnt);
    OutputDebugString(errMsg);

    if (GETSEVERITY(errCode) & STATUS_SEVERITY_FATAL) {
	/* I could have used printf() and fflush(), but chose the direct
	 * route instead */
	WriteFile(GetStdHandle(STD_ERROR_HANDLE), errMsg, strlen(errMsg),
		&dwWritten, NULL);

	/* Stop the world, I want to get off. */
	//DebugBreak();

	Sleep(5000);
	ExitProcess(255);
    }

    LocalFree(errMsg);
}

char *ExpSyslogGetSysMsg (DWORD id)
{
    int chars;

    chars = wsprintf(sysMsgSpace, "[%d] ", id);

    FormatMessage(
	    FORMAT_MESSAGE_FROM_SYSTEM |
	    FORMAT_MESSAGE_MAX_WIDTH_MASK,
	    0L,
	    id,
	    0,
	    &sysMsgSpace[chars],
	    (1024-chars),
	    0);

    return sysMsgSpace;
}

char *Exp95Log(DWORD errCode, char *errData[], int cnt)
{
    char *msg;

    FormatMessage(
	    FORMAT_MESSAGE_FROM_HMODULE |
	    FORMAT_MESSAGE_ALLOCATE_BUFFER |
	    FORMAT_MESSAGE_ARGUMENT_ARRAY,
	    GetModuleHandle(NULL),
	    errCode,
	    0,
	    (char *) &msg,
	    0,
	    errData);

    return msg;
}
