/* ----------------------------------------------------------------------------
 * exp_printify.c --
 *
 *	make non-printable characters printable.
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
 * RCS: @(#) $Id: exp.h,v 1.1.4.4 2002/02/10 10:17:04 davygrvy Exp $
 * ----------------------------------------------------------------------------
 */

#include "expInt.h"

#ifdef NO_STRING_H
#include "../compat/string.h"
#else
#include <string.h>
#endif
#ifdef NO_STDLIB_H
#include "../compat/stdlib.h"
#else
#include <stdlib.h>		/* for malloc */
#endif
#include <ctype.h>

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
