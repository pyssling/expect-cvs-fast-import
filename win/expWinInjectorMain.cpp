/* ----------------------------------------------------------------------------
 * expWinInjectorMain.cpp --
 *
 *	Console event injector DLL that's loaded into the slave's address space
 *	used by the ConsoleDebugger class for "writing" to the slave.
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
 * RCS: @(#) $Id: expWinSlave.hpp,v 1.1.4.10 2002/03/12 22:36:44 davygrvy Exp $
 * ----------------------------------------------------------------------------
 */

#include "CMcl.h"

// It is documented that it "isn't a good idea to spawn threads from a DllMain".
// Pooie on you; this is what we will do.

BOOL WINAPI
DllMain (HINSTANCE hInst, ULONG ulReason, LPVOID lpReserved)
{
    switch (ulReason) {
    case DLL_PROCESS_ATTACH:
	DisableThreadLibraryCalls(hInst);
	break;

    case DLL_PROCESS_DETACH:
	break;
    }
    return TRUE;
}
