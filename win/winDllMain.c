/* ----------------------------------------------------------------------------
 * winDllMain.h --
 *
 *	entry-point for windows.
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
 * RCS: @(#) $Id: winDllMain.c,v 1.1.2.4 2002/02/10 12:03:30 davygrvy Exp $
 * ----------------------------------------------------------------------------
 */

#include "tcl.h"
#define WIN32_LEAN_AND_MEAN
#include <windows.h>

#ifdef _MSC_VER
    /* Only do this when MSVC++ is compiling us. */
#   ifdef USE_TCL_STUBS
#	pragma comment (lib, "tclstub" \
		STRINGIFY(JOIN(TCL_MAJOR_VERSION,TCL_MINOR_VERSION)) ".lib")
#	if !defined(_MT) || !defined(_DLL) || defined(_DEBUG)
	    /* This fixes a bug with how the Stubs library was compiled.
	     * The requirement for msvcrt.lib from tclstubXX.lib must
	     * be removed.  This bug has been fixed since 8.4a3, I beleive. */
#	    pragma comment(linker, "-nodefaultlib:msvcrt.lib")
#	endif
#   else
#	ifdef _DEBUG
#	    pragma comment (lib, "tcl" \
		    STRINGIFY(JOIN(TCL_MAJOR_VERSION,TCL_MINOR_VERSION)) "d.lib")
#	else
#	    pragma comment (lib, "tcl" \
		    STRINGIFY(JOIN(TCL_MAJOR_VERSION,TCL_MINOR_VERSION)) ".lib")
#	endif
#   endif
#endif


/* public global */
HMODULE expDllInstance = NULL;

#ifndef STATIC_BUILD
BOOL WINAPI
DllMain (HINSTANCE hInst, ULONG ulReason, LPVOID lpReserved)
{
    switch (ulReason) {
    case DLL_PROCESS_ATTACH:
	expDllInstance = hInst;
    }
    return TRUE;
}
#endif