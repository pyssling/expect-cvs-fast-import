/* ----------------------------------------------------------------------------
 * expWinSlaveTrapDbg.cpp --
 *
 *	.
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
 * RCS: @(#) $Id: expWinSlaveTrapDbg.cpp,v 1.1.4.13 2002/06/28 01:26:57 davygrvy Exp $
 * ----------------------------------------------------------------------------
 */

#include "expWinSlave.hpp"


SlaveTrapDetour::SlaveTrapDetour(int argc, char * const argv[], CMclQueue<Message *> &_mQ)
    : mQ(_mQ)
{
}

SlaveTrapDetour::~SlaveTrapDetour()
{
}

void SlaveTrapDetour::Write(Message *msg)
{
}
