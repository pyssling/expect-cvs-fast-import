/* ----------------------------------------------------------------------
 * expWinConsoleDetour.cpp --
 *
 *	Console detour core implimentation.  At first, we will use the
 *	"Detours" library provided by Microsoft Research for testing.
 *	We will have to "heavily customize it" later to remove all
 *	copywrite before we can distribute this.
 *
 *	see http://research.microsoft.com/sn/detours/
 *
 * ----------------------------------------------------------------------
 *
 * Written by: Don Libes, libes@cme.nist.gov, NIST, 12/3/90
 * 
 * Design and implementation of this program was paid for by U.S. tax
 * dollars.  Therefore it is public domain.  However, the author and
 * NIST would appreciate credit if this program or parts of it are used.
 * 
 * Copyright (c) 1997 Mitel Corporation
 *	work by Gordon Chaffee <chaffee@bmrc.berkeley.edu> for the NT
 *	port.
 *
 * Copyright (c) 2001-2002 Telindustrie, LLC
 *	work by David Gravereaux <davygrvy@pobox.com> for any Win32 OS.
 *
 * ----------------------------------------------------------------------
 * URLs:    http://expect.nist.gov/
 *	    http://expect.sf.net/
 *	    http://bmrc.berkeley.edu/people/chaffee/expectnt.html
 * ----------------------------------------------------------------------
 * RCS: @(#) $Id: expWinConsoleDebugger.cpp,v 1.1.2.31 2002/06/29 00:44:35 davygrvy Exp $
 * ----------------------------------------------------------------------
 */

#include "expWinSlave.hpp"


ConsoleDetour::ConsoleDetour (
	int _argc,
	char * const *_argv,
	CMclQueue<Message *> &_mQ)
    : mQ(_mQ)
{
}

ConsoleDetour::~ConsoleDetour ()
{
}

unsigned
ConsoleDetour::ThreadHandlerProc (void)
{
    return 0;
}

void
ConsoleDetour::WriteRecord (INPUT_RECORD *ir)
{
}

void
ConsoleDetour::EnterInteract (HANDLE OutConsole)
{
}

void
ConsoleDetour::ExitInteract (void)
{
}
