/* ----------------------------------------------------------------------------
 * expWinTestClientStdio.cpp --
 *
 *	Simple standard I/O as our client.
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
 * RCS: @(#) $Id: expWinClientTransportStdio.cpp,v 1.1.2.1 2002/06/23 09:20:40 davygrvy Exp $
 * ----------------------------------------------------------------------------
 */

#include "expWinTestClient.hpp"


class ReadPipe : public CMclThreadHandler
{
public:
    ReadPipe(CMclQueue<Message *> &_mQ)	: mQ(_mQ)
    {
	hStdIn = GetStdHandle(STD_INPUT_HANDLE);
    }

private:

#   define READ_BUFFER_SIZE    128
    virtual unsigned ThreadHandlerProc(void)
    {
	BOOL ok;
	DWORD dwRead;
	Message *msg;
	BYTE *readBuf;

    again:
	readBuf = new BYTE [READ_BUFFER_SIZE];
	ok = ReadFile(hStdIn, readBuf, READ_BUFFER_SIZE, &dwRead, 0L);
	if (!ok || dwRead == 0) {
	    CloseHandle(hStdIn);  // <- should this be here?
	    delete [] readBuf;
	    goto done;
	}
	msg = new Message;
	msg->bytes = (CHAR *) readBuf;
	msg->length = dwRead;
	msg->type = Message::TYPE_INSTREAM;
	mQ.Put(msg);
	goto again;

    done:
	return 0;
    }

    CMclQueue<Message *> &mQ;
    HANDLE hStdIn;
};

ClientStdio::ClientStdio(const char *name, CMclQueue<Message *> &_mQ)
    : mQ(_mQ)
{
    hStdOut = GetStdHandle(STD_OUTPUT_HANDLE);
    hStdErr = GetStdHandle(STD_ERROR_HANDLE);
    reader = new ReadPipe(_mQ);
    readThread = new CMclThread(reader);
}

ClientStdio::~ClientStdio()
{
    DWORD dwExit;

    readThread->GetExitCode(&dwExit);
    if (dwExit == STILL_ACTIVE) {
	readThread->Terminate(128);
    }
}

void
ClientStdio::Write(Message *what)
{
    DWORD dwWritten;
    HANDLE where;

    switch (what->type) {
    case Message::TYPE_NORMAL:
	where = hStdOut; break;
    case Message::TYPE_WARNING:
    case Message::TYPE_ERROR:
	where = hStdErr;
    }
    WriteFile(where, what->bytes, what->length, &dwWritten, 0L);
    delete what;
}
