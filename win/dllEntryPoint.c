/* ----------------------------------------------------------------------------
 * dllEntryPoint.c --
 *
 *	This file implements the Dll entry point as needed by Windows.
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
 * Copyright (c) 2001 Telindustrie, LLC
 *	work by David Gravereaux <davygrvy@pobox.com> for any Win32 OS.
 *
 * ----------------------------------------------------------------------------
 * URLs:    http://expect.nist.gov/
 *	    http://expect.sf.net/
 *	    http://bmrc.berkeley.edu/people/chaffee/expectnt.html
 * ----------------------------------------------------------------------------
 * RCS: @(#) $Id: exp.h,v 1.1.2.5 2001/10/29 06:40:29 davygrvy Exp $
 * ----------------------------------------------------------------------------
 */

#define WIN32_LEAN_AND_MEAN
#include <windows.h>

#ifdef _MSC_VER
    /* Only do this when MSVC++ is compiling us. */
#   define DllEntryPoint DllMain
#   if defined(USE_TCL_STUBS) && (!defined(_MT) || !defined(_DLL) || defined(_DEBUG))
	/*
	 * This fixes a bug with how the Stubs library was compiled.
	 * The requirement for msvcrt.lib from tclstubXX.lib must
	 * be removed.
	 */
#	pragma comment(linker, "-nodefaultlib:msvcrt.lib")
#   endif
#endif

/*
 *----------------------------------------------------------------------
 *
 * DllEntryPoint --
 *
 *	This wrapper function is used by Windows to invoke the
 *	initialization code for the DLL.  If we are compiling
 *	with Visual C++, this routine will be renamed to DllMain.
 *
 * Results:
 *	Returns TRUE;
 *
 * Side effects:
 *	None.
 *
 *----------------------------------------------------------------------
 */

#ifndef STATIC_BUILD

BOOL APIENTRY
DllEntryPoint(hInst, reason, reserved)
    HINSTANCE hInst;		/* Library instance handle. */
    DWORD reason;		/* Reason this function is being called. */
    LPVOID reserved;		/* Not used. */
{
    return TRUE;
}

#endif