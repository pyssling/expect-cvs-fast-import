/* ----------------------------------------------------------------------------
 * expWinSlaveTrapDbg.cpp --
 *
 *	The slave driver acts as a debugger for the slave program.  It
 *	does this so that we can determine echoing behavior of the
 *	subprocess.  This isn't perfect as the subprocess can change
 *	echoing behavior while our keystrokes are lying in its console
 *	input buffer, but it is much better than nothing.  The debugger
 *	thread sets up breakpoints on the OS functions we want to intercept,
 *	and it writes data that is written directly to the console of
 *	the master over a method of IPC.
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
 * RCS: @(#) $Id: expWinSlaveTrapDbg.cpp,v 1.1.4.1 2002/03/08 23:37:16 davygrvy Exp $
 * ----------------------------------------------------------------------------
 */

#include "expWinInt.h"
#include <stddef.h>
#include <assert.h>

#include "expWinConsoleDebugger.hpp"
#ifdef _MSC_VER
#   pragma comment (lib, "imagehlp.lib")
#endif


ExpSlaveTrapDbg::ExpSlaveTrapDbg(int argc, char * const argv[])
{
    debuggerThread = new CMclThread(new ConsoleDebugger(argc, argv));
}

