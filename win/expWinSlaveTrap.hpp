/* ----------------------------------------------------------------------------
 * expWinSlaveTrap.hpp --
 *
 *	Declares the SlaveTrap classes.
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
 * RCS: @(#) $Id: expWinSlaveTrap.hpp,v 1.1.2.1 2002/03/12 07:09:36 davygrvy Exp $
 * ----------------------------------------------------------------------------
 */

#ifndef INC_expWinSlaveTrap_hpp__
#define INC_expWinSlaveTrap_hpp__

#include "expWinMessage.hpp"
#include "Mcl/include/CMcl.h"

class SlaveTrap {
public:
    virtual void Write(Message *) = 0;
};


class SlaveTrapDbg : public SlaveTrap {
public:
    SlaveTrapDbg(int argc, char * const argv[], CMclQueue<Message *> &_mQ);
    virtual void Write(Message *);
private:
    CMclQueue<Message *> &mQ;
    CMclThreadAutoPtr debuggerThread;
};

#endif