/* ----------------------------------------------------------------------------
 * expSlaveTrapPipe.cpp --
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
 * Copyright (c) 2001 Telindustrie, LLC
 *	work by David Gravereaux <davygrvy@pobox.com> for full Stubs complience
 *	and any Win32 OS.
 *
 * ----------------------------------------------------------------------------
 * URLs:    http://expect.nist.gov/
 *	    http://expect.sf.net/
 *	    http://bmrc.berkeley.edu/people/chaffee/expectnt.html
 * ----------------------------------------------------------------------------
 * RCS: @(#) $Id: expWinSpawnTransport.cpp,v 1.1.2.1 2001/11/09 01:17:40 davygrvy Exp $
 * ----------------------------------------------------------------------------
 */

#include "expWinInt.h"

#define BUFSIZE 4096


ExpSlaveTrapPipe::ExpSlaveTrapPipe(int argc, char *argv[])
{

};


class DrivePipeThread : public CMclThreadHandler
{
public:
    DrivePipeThread()
    {
    }

private:
    unsigned ThreadHandlerProc(void)
    {
	OVERLAPPED over;
	DWORD nread;
	DWORD max;
	DWORD count, n;
	BOOL ret;
	UCHAR buf[BUFSIZE];
	DWORD exitVal;
	LONG err;

	n = 0;

	if (hMaster != NULL) {
	    over.hEvent = CreateEvent(NULL, TRUE, FALSE, NULL);
	}

	while (1) {
	    ret = PeekNamedPipe(hIn, NULL, 0, NULL, &nread, NULL);
	    if (ret == FALSE) {
		break;
	    }
	    if (nread == 0) {
		nread = 1;
	    }
	    do {
		max = sizeof(buf) - 1 - n;
		nread = min(nread, max);
		ret = ReadFile(hIn, &buf[n], nread, &count, NULL);
		if (ret == FALSE) {
		    err = GetLastError();
		    goto done;
		}
		if (count > 0) {
		    n += count;
		    if (n - 1 >= sizeof(buf)) {
			break;
		    }
		} else {
		    break;
		}
		ret = PeekNamedPipe(hIn, NULL, 0, NULL, &nread, NULL);
		if (ret == FALSE) {
		    err = GetLastError();
		    goto done;
		}
		/*
		 * To allow subprocess to do something without continuous
		 * process switching, give it a bit of processing time before
		 * we check for more data.
		 */
		if (count == 1 && nread == 0) {
		    Sleep(40);
		    ret = PeekNamedPipe(hIn, NULL, 0, NULL, &nread, NULL);
		    if (ret == FALSE) {
			err = GetLastError();
			goto done;
		    }
		}
	    } while (nread > 0);

//	    if (WaitForSingleObject(hMutex, INFINITE) != WAIT_OBJECT_0) {
//		goto error;
//	    }
	    WriteFile(ExpConsoleOut, buf, n, &count, NULL);
//	    ReleaseMutex(hMutex);
	    ret = ExpWriteMaster(useSocket, hMaster, buf, n, &over);
	    n = 0;
	    if (ret == FALSE) {
		break;
	    }
	}

     done:
	if (n > 0) {
//	    if (WaitForSingleObject(hMutex, INFINITE) == WAIT_OBJECT_0) {
		WriteFile(ExpConsoleOut, buf, n, &count, NULL);
//		ReleaseMutex(hMutex);
		ret = ExpWriteMaster(useSocket, hMaster, buf, n, &over);
//	    }
	}
     error:
	CloseHandle(hIn);
	running = 0;
	if (err == ERROR_HANDLE_EOF || err == ERROR_BROKEN_PIPE) {
	    exitVal = 0;
	} else {
	    exitVal = err;
	}
	ExitThread(exitVal);
	return 0;
    }
    HANDLE hIn;
    HANDLE hMaster;		/* Output when we are using pipes */
    int useSocket;
    BOOL running;
    HANDLE thread;
};
