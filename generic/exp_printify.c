/* exp_printify - printable versions of random ASCII strings
 *
 * Written by: Don Libes, NIST, 2/6/90
 *
 * Design and implementation of this program was paid for by U.S. tax
 * dollars.  Therefore it is public domain.  However, the author and NIST
 * would appreciate credit if this program or parts of it are used.
 *
 * RCS: @(#) $Id: exp_printify.c,v 1.1.2.2 2001/10/29 23:34:48 davygrvy Exp $
 */

#include "expInt.h"
#ifdef NO_STRING_H
#include "../compat/string.h"
#else
#include <string.h> /* for sprintf() */
#endif

#include <ctype.h>  /* for isascii() and isprint() */


typedef struct ThreadSpecificData {
    unsigned int destlen;
    char *dest;
} ThreadSpecificData;

static Tcl_ThreadDataKey dataKey;


/*
 *----------------------------------------------------------------------
 *
 * exp_printify --
 *
 *	Generate printable versions of random ASCII strings.  Primarily
 *	used by cmdExpect when -d forces it to print strings it is
 *	examining.
 *
 * Results:
 *	Pointer to the thread specific buffer.
 *
 * Side effects:
 *	buffer is not freed.
 *
 * Comment:
 *	Thread-safe.
 *
 *----------------------------------------------------------------------
 */

char *
exp_printify(s)
    char *s;
{
    ThreadSpecificData *tsdPtr = TCL_TSD_INIT(&dataKey);
    unsigned int need;
    char *d;		/* ptr into dest */

    if (s == 0) return("<null>");

    /* worst case is every character takes 4 to printify */
    need = strlen(s)*4 + 1;
    if (need > tsdPtr->destlen) {
	if (tsdPtr->dest) ckfree(tsdPtr->dest);
	tsdPtr->dest = ckalloc(tsdPtr->destlen = need);
    }

    for (d = tsdPtr->dest;*s;s++) {
	if (*s == '\r') {
	    strcpy(d,"\\r");		    d += 2;
	} else if (*s == '\n') {
	    strcpy(d,"\\n");		    d += 2;
	} else if (*s == '\t') {
	    strcpy(d,"\\t");		    d += 2;
	} else if (isascii(*s) && isprint(*s)) {
	    *d = *s;			    d += 1;
	} else {
	    sprintf(d,"\\x%02x",*s & 0xff); d += 4;
	}
    }
    *d = '\0';
    return(tsdPtr->dest);
}
