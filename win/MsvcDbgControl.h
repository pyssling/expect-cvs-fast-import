/* ----------------------------------------------------------------------------
 * MsvcDbgControl.h --
 *
 *	Debugger friendly replacements for CreateProcess() on the parent side
 *	and GetCommandLine() on the child side.
 *
 *	This stuff may not be perfect, but the intent is to avoid all
 *	the manual-ness of having to set a specific commandline by hand
 *	while debugging the system.
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

#ifndef INC_MsvcDbgControl_h__
#define INC_MsvcDbgControl_h__

#include <windows.h>

extern CHAR *	MsvcDbg_GetCommandLine(void);
extern int	MsvcDbg_Launch(const CHAR *wrkspace, Tcl_DString *cmdline,
			void **token);
#endif