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
 * Copyright (c) 2003 ActiveState Corporation
 *	work by David Gravereaux <davygrvy@pobox.com> for any Win32 OS.
 *
 * ----------------------------------------------------------------------------
 * URLs:    http://expect.nist.gov/
 *	    http://expect.sf.net/
 *	    http://bmrc.berkeley.edu/people/chaffee/expectnt.html
 * ----------------------------------------------------------------------------
 * RCS: @(#) $Id: expWinTestClient.hpp,v 1.1.2.4 2003/08/25 23:17:49 davygrvy Exp $
 * ----------------------------------------------------------------------------
 */

#include "expWinMessage.hpp"
#include "CMcl.h"

class TestClient
{
public:
    virtual ~TestClient() {};
    virtual void Write(Message *) = 0;
};


// Forward reference.
class ReadPipe;
class ReadCon;
class ReadConWithSwitching;

class ClientStdio : public TestClient
{
public:
    ClientStdio(CMclQueue<Message *> &_mQ);
    virtual ~ClientStdio();
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
    ClientConio(CMclQueue<Message *> &_mQ);
    virtual ~ClientConio();
    virtual void Write(Message *);
private:
    CMclQueue<Message *> &mQ;
    HANDLE ConOut, ConIn, oldBuffer;
    ReadCon *reader;
    CMclEvent Stop;
    CMclThread *readThread;
};

class ClientInteract : public TestClient
{
public:
    ClientInteract(CMclQueue<Message *> &_mQ);
    virtual ~ClientInteract();
    virtual void Write(Message *);
private:
    CMclQueue<Message *> &mQ;
    HANDLE ConOut, ConIn, ConInteract, oldBuffer;
    ReadConWithSwitching *reader;
    CMclEvent Stop;
    CMclThread *readThread;
};
