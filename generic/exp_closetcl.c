/* ----------------------------------------------------------------------------
 * exp_closetcl.c --
 *
 *	close tcl files.  Isolated in it's own file since it has hooks into
 *	Tcl and exp_clib user might like to avoid dragging it in.
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

void (*exp_close_in_child)() = 0;

void
exp_close_tcl_files() {

	/* So much for close-on-exec.  Tcl doesn't mark its files that way */
	/* everything has to be closed explicitly. */

#if 0
	int i;

/* Not necessary with Tcl 7.5? */
	for (i=3; i<tclNumFiles;i++) close(i);
#endif
}
