/* ----------------------------------------------------------------------------
 * expWinSpawnSocketCli.cpp --
 *
 *	Socket client used as one of the IPC methods for spawndrv.exe
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
 * RCS: @(#) $Id: expWinInit.c,v 1.1.2.3 2001/11/09 01:17:40 davygrvy Exp $
 * ----------------------------------------------------------------------------
 */

#include "expWinInt.h"

ExpSpawnSocketCli::ExpSpawnSocketCli(const char *name)
{
}

void ExpSpawnSocketCli::ExpWriteMaster() {};
void ExpSpawnSocketCli::ExpReadMaster() {};
