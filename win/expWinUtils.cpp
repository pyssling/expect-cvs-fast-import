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
    int quote, i, need = 0;

    // Guess how large we are.
    for (i = 0; i < argc; i++) {
	need += strlen(argv[i]) + 10;
    }

    out = new char [need];

    for (i = 0; i < argc; i++) {
	arg = argv[i];
	if (i != 0) {
	    strcat(out, " ");
	} else {
	    *out = '\0';
	}

	quote = 0;
	if (arg[0] == '\0') {
	    quote = 1;
	} else {
	    // We quote the entire arguement when a space is found in it.
	    for (start = arg; *start != '\0'; start++) {
		if (isspace(*start)) {
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
	    if ((*special == '\\') && 
		    (special[1] == '\\' || special[1] == '"')) {
		strncat(out, start, special - start);
		start = special;
		while (1) {
		    special++;
		    if (*special == '"') {
			// N backslashes followed a quote -> insert 
			// N * 2 + 1 backslashes then a quote.
			strncat(out, start, special - start);
			break;
		    }
		    if (*special != '\\') {
			break;
		    }
		}
		strncat(out, start, special - start);
		start = special;
	    }
	    if (*special == '"') {
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
