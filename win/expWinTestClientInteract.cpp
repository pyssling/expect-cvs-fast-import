/* ----------------------------------------------------------------------------
 * expWinTestClientInteract.cpp --
 *
 *	This tests the interact capabilities.
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
 * RCS: @(#) $Id: expWinTestClientConio.cpp,v 1.1.2.1 2002/06/27 04:37:51 davygrvy Exp $
 * ----------------------------------------------------------------------------
 */

#include "expWinTestClient.hpp"

ClientInteract::ClientInteract(CMclQueue<Message *> &_mQ)
    : mQ(_mQ), ConOut(0L), ConIn(0L), ConInteract(0L), oldBuffer(0L)
{
}

ClientInteract::~ClientInteract()
{
}

void ClientInteract::Write(Message *)
{
}
