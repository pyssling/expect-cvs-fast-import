/* ----------------------------------------------------------------------------
 * expWinSlaveCliTransport.cpp --
 *
 *	Generic routines for opening the client IPC transport.  Has knowledge
 *	of all transport types used.
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
 *	work by David Gravereaux <davygrvy@pobox.com> for full Stubs complience
 *	and any Win32 OS.
 *
 * ----------------------------------------------------------------------------
 * URLs:    http://expect.nist.gov/
 *	    http://expect.sf.net/
 *	    http://bmrc.berkeley.edu/people/chaffee/expectnt.html
 * ----------------------------------------------------------------------------
 * RCS: @(#) $Id: expWinSpawnTransport.cpp,v 1.1.2.1 2001/11/09 01:17:40 davygrvy Exp $
 * ----------------------------------------------------------------------------
 */

#include "expWinInt.h"

/*
 *----------------------------------------------------------------------
 *  ExpWinSpawnOpenTransport --
 *
 *	The factory method for creating the client IPC transport from
 *	the name asked of it.
 *
 *  Returns:
 *	a polymorphed ExpSpawnTransportCli pointer or NULL for an error.
 *
 *----------------------------------------------------------------------
 */
ExpSpawnTransportCli *
ExpWinSpawnOpenTransport(const char *name)
{
    /* If the first 2 chars are 'm' and 'b', then it's a mailbox. */
    if (name[0] == 'm' && name[1] == 'b') {
	return new ExpSpawnMailboxCli(name);
    }
    else if (name[0] == 's' && name[1] == 'k') {
	return new ExpSpawnSocketCli(name);
    }
    /* TODO: we can add more transports here when the time is right */
    else return 0L;
}
