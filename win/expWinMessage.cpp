/* ----------------------------------------------------------------------------
 * expWinMessage.cpp --
 *
 *	Defines the Message class.  This is what is passed over the
 *	thread-safe event queue.
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
 * RCS: @(#) $Id: expWinMessage.cpp,v 1.1.2.3 2002/03/12 07:09:36 davygrvy Exp $
 * ----------------------------------------------------------------------------
 */

#include "expWinMessage.hpp"

Message::Message()
    : bytes(0L), length(0), type(TYPE_BLANK)
{
}
/*
I'm not defining this as a check to see if the Queue is using it :)
Message::Message(Message &other)
{
    type = other.type;
    bytes = other.bytes;
    length = other.length;
}*/