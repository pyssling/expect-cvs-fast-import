/* ----------------------------------------------------------------------------
 * expWinTestClient.hpp --
 *
 *	Declares all the TestClient classes.
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
 * RCS: @(#) $Id: expWinClientTransport.hpp,v 1.1.2.4 2002/06/27 03:43:34 davygrvy Exp $
 * ----------------------------------------------------------------------------
 */

#include "expWinMessage.hpp"
#include "Mcl/include/CMcl.h"

class TestClient
{
public:
    virtual ~TestClient() {};
    virtual void Write(Message *) = 0;
};


// Forward reference.
class ReadPipe;
class ReadCon;

class ClientStdio : public TestClient
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

class ClientConio : public TestClient
{
public:
    ClientConio(const char *name, CMclQueue<Message *> &_mQ);
    ~ClientConio();
    virtual void Write(Message *);
private:
    CMclQueue<Message *> &mQ;
    HANDLE ConOut, ConIn, oldBuffer;
    ReadCon *reader;
    CMclEvent Stop;
    CMclThread *readThread;
};
