/* ----------------------------------------------------------------------------
 * expWinSpawnPipeClient.cpp --
 *
 *	Simple pipes as our IPC mechanism.
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
 * RCS: @(#) $Id: expWinSlaveTrapDbg.cpp,v 1.1.4.4 2002/03/10 01:02:37 davygrvy Exp $
 * ----------------------------------------------------------------------------
 */

#include "expWinInt.h"

SpawnPipeClient::SpawnPipeClient(const char *name, CMclQueue<Message> &_mQ)
    : mQ(_mQ)
{
    hStdIn  = GetStdHandle(STD_INPUT_HANDLE);
    hStdOut = GetStdHandle(STD_OUTPUT_HANDLE);
    hStdErr = GetStdHandle(STD_ERROR_HANDLE);
}

void
SpawnPipeClient::Write(Message &what)
{
    DWORD dwWritten;
    HANDLE where;

    switch (what.mode) {
    case Message::TYPE_NORMAL:
	where = hStdOut;
    case Message::TYPE_ERROR:
	where = hStdErr;
    }

    WriteFile(where, what.bytes, what.length, &dwWritten, 0L);
}
