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
 * RCS: @(#) $Id: expWinUtils.cpp,v 1.1.2.3 2002/03/11 07:03:35 davygrvy Exp $
 * ----------------------------------------------------------------------------
 */

#include "expWinUtils.hpp"
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
    char *cmdLine,	// commandline string.
    int *argcPtr,	// Filled with number of argument strings.
    char ***argvPtr)	// Filled with argument strings in UTF (alloc'd with new).
{
    char *p, *arg, *argSpace;
    char **argv;
    int argc, size, inquote, copy, slashes;

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
    argSpace = new char [size + strlen(cmdLine) + 1];
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
    argv[argc] = NULL;

    *argcPtr = argc;
    *argvPtr = argv;
}
