/* exp_printify - printable versions of random ASCII strings
 *
 * Written by: Don Libes, NIST, 2/6/90
 *
 * Design and implementation of this program was paid for by U.S. tax
 * dollars.  Therefore it is public domain.  However, the author and NIST
 * would appreciate credit if this program or parts of it are used.
 *
 * RCS: @(#) $Id: tclWinPort.h,v 1.24 2001/10/15 17:34:53 hobbs Exp $
 */

#include "exp.h"
#ifdef NO_STRING_H
#include "../compat/string.h"
#else
#include <string.h> /* for sprintf() */
#endif

#include <ctype.h>  /* for isascii() and isprint() */

/* generate printable versions of random ASCII strings.  Primarily used */
/* by cmdExpect when -d forces it to print strings it is examining. */
char *
exp_printify(s)
    char *s;
{
    static unsigned int destlen = 0;
    static char *dest = 0;
    char *d;		/* ptr into dest */
    unsigned int need;

    if (s == 0) return("<null>");

    /* worst case is every character takes 4 to printify */
    need = strlen(s)*4 + 1;
    if (need > destlen) {
	if (dest) ckfree(dest);
	dest = ckalloc(need);
	destlen = need;
    }

    for (d = dest;*s;s++) {
	if (*s == '\r') {
	    strcpy(d,"\\r");		d += 2;
	} else if (*s == '\n') {
	    strcpy(d,"\\n");		d += 2;
	} else if (*s == '\t') {
	    strcpy(d,"\\t");		d += 2;
	} else if (isascii(*s) && isprint(*s)) {
	    *d = *s;			d += 1;
	} else {
	    sprintf(d,"\\x%02x",*s & 0xff);	d += 4;
	}
    }
    *d = '\0';
    return(dest);
}
