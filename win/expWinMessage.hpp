/* ----------------------------------------------------------------------------
 * expWinMessage.hpp --
 *
 *	Declare the Message class.  This is what is passed over the thread-safe
 *	event queue.
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
 * RCS: @(#) $Id: expWinMessage.hpp,v 1.1.2.3 2002/03/12 07:09:36 davygrvy Exp $
 * ----------------------------------------------------------------------------
 */

#ifndef INC_expWinMessage_hpp__
#define INC_expWinMessage_hpp__

#include <stddef.h>	// for size_t

class Message
{
public:
    Message();
    Message(Message &);

    enum Mode {TYPE_BLANK, TYPE_NORMAL, TYPE_ERROR, TYPE_INSTREAM, TYPE_FUNCTION};
    Mode type;
    unsigned char *bytes;
    size_t length;
};

#endif