/* ----------------------------------------------------------------------------
 * expWinClientTransport.hpp --
 *
 *	Declares all the ClientTransport classes.
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
 * RCS: @(#) $Id: expWinClientTransport.hpp,v 1.1.2.2 2002/06/23 09:20:40 davygrvy Exp $
 * ----------------------------------------------------------------------------
 */

#include "expWinMessage.hpp"
#include "Mcl/include/CMcl.h"

class ClientTransport
{
public:
    virtual void Write(Message *) = 0;
};


// Forward reference.
class ReadPipe;
class ReadCon;

class ClientStdio : public ClientTransport
{
public:
    ClientStdio(const char *name, CMclQueue<Message *> &_mQ);
    ~ClientStdio();
    virtual void Write(Message *);
private:
    CMclQueue<Message *> &mQ;
    HANDLE hStdOut;
    HANDLE hStdErr;
    ReadPipe *reader;
    CMclThreadAutoPtr readThread;
};

class ClientConio : public ClientTransport
{
public:
    ClientConio(const char *name, CMclQueue<Message *> &_mQ);
    ~ClientConio();
    virtual void Write(Message *);
private:
    CMclQueue<Message *> &mQ;
    HANDLE ConOut;
    HANDLE oldBuffer;
    ReadCon *reader;
    CMclEvent Stop;
    CMclThread *readThread;
};
