/* ----------------------------------------------------------------------------
 * expWinTestClientConio.cpp --
 *
 *	Simple console I/O as our client.
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
 * RCS: @(#) $Id: expWinTestClientConio.cpp,v 1.1.2.3 2002/06/29 00:44:36 davygrvy Exp $
 * ----------------------------------------------------------------------------
 */

#include "expWinTestClient.hpp"

class ReadCon : public CMclThreadHandler
{
    CMclQueue<Message *> &mQ;
    CMclEvent &Stop;
    HANDLE ConIn;
    CMclWaitableCollection groupedHandles;
#   define RECORD_SIZE 15

public:
    ReadCon(CMclQueue<Message *> &_mQ, CMclEvent &_Stop, HANDLE _ConIn)
	: mQ(_mQ), Stop(_Stop), ConIn(_ConIn)
    {
	// Turn on mouse and window events, too.
        SetConsoleMode(ConIn, ENABLE_MOUSE_INPUT);
    }

    ~ReadCon()
    {
    }

private:
    virtual unsigned ThreadHandlerProc(void)
    {
	DWORD dwRet, dwRead, i;
	INPUT_RECORD Records[RECORD_SIZE], *record;
	MOUSE_EVENT_RECORD lastMouse;
	Message *msg;

	// Index 0, highest priority goes first.
	groupedHandles.AddObject(Stop);

	// Console handles are waitable objects.
	// Index 1
	groupedHandles.AddObject(ConIn);

    again:
	// Wait for any (either) to signal.
	dwRet = groupedHandles.Wait(FALSE, INFINITE);

	if (!CMclWaitSucceeded(dwRet, 2) ||
		CMclWaitSucceeded(dwRet, 2) && CMclWaitSucceededIndex(dwRet) == 0) {
	    // stop event or some other error, so die.
	    return 0;
	}

	ReadConsoleInput(ConIn, Records, RECORD_SIZE, &dwRead);
	for (i = 0; i < dwRead; i++) {
	    switch (Records[i].EventType) {
	    case FOCUS_EVENT:
	    case MENU_EVENT:
		// ignore these.
		continue;

	    case MOUSE_EVENT:
		// only send a mouse event if it's different then the last one.
		if (!isDiff(&lastMouse, &Records[i].Event.MouseEvent)) {
		    continue;
		}
		lastMouse = Records[i].Event.MouseEvent;
		break;
	    }
	    msg = new Message;
	    msg->type = Message::TYPE_INRECORD;
	    msg->length = sizeof(INPUT_RECORD);
	    record = new INPUT_RECORD;
	    memcpy(record, &Records[i], sizeof(INPUT_RECORD));
	    msg->bytes = (BYTE *)record;
	    mQ.Put(msg);
	}
	goto again;
    }

    BOOL
    isDiff(MOUSE_EVENT_RECORD *mer1, MOUSE_EVENT_RECORD *mer2)
    {
	if (
	    mer1->dwMousePosition.X ^ mer2->dwMousePosition.X ||
	    mer1->dwMousePosition.Y ^ mer2->dwMousePosition.Y ||
	    mer1->dwButtonState     ^ mer2->dwButtonState ||
	    mer1->dwControlKeyState ^ mer2->dwControlKeyState ||
	    mer1->dwEventFlags      ^ mer2->dwEventFlags
	) {
	    return TRUE;
	} else {
	    return FALSE;
	}

    }
};


ClientConio::ClientConio(CMclQueue<Message *> &_mQ)
    : mQ(_mQ), Stop()
{
    COORD Size = {80, 500};

    ConIn = CreateFile("CONIN$", GENERIC_READ|GENERIC_WRITE,
	    FILE_SHARE_READ|FILE_SHARE_WRITE, 0L, OPEN_EXISTING, 0, 0L);

    reader = new ReadCon(_mQ, Stop, ConIn);
    readThread = new CMclThread(reader);

    oldBuffer = CreateFile("CONOUT$", GENERIC_READ|GENERIC_WRITE,
	    FILE_SHARE_READ|FILE_SHARE_WRITE, 0L, OPEN_EXISTING, 0, 0L);
    ConOut = CreateConsoleScreenBuffer(GENERIC_READ|GENERIC_WRITE,
	    FILE_SHARE_READ|FILE_SHARE_WRITE, 0L, CONSOLE_TEXTMODE_BUFFER, 0L);
//    SetConsoleCtrlHandler(ConsoleHandler, TRUE);
    SetConsoleActiveScreenBuffer(ConOut);
    SetConsoleScreenBufferSize(ConOut, Size);
}

ClientConio::~ClientConio()
{
    DWORD dwRead;
    INPUT_RECORD record;

    Stop.Set();
    readThread->Wait(INFINITE);
    delete reader, readThread;

    SetConsoleTextAttribute(ConOut, FOREGROUND_BLUE | FOREGROUND_INTENSITY);
    WriteConsole(ConOut, "\nPress any key to exit...\n", 26, &dwRead, 0L);
    FlushConsoleInputBuffer(ConIn);

again:
    ReadConsoleInput(ConIn, &record, 1, &dwRead);
    if (record.EventType != KEY_EVENT) {
	goto again;
    }

    SetConsoleActiveScreenBuffer(oldBuffer);
    CloseHandle(ConOut);
    CloseHandle(ConIn);
}

void
ClientConio::Write(Message *what)
{
    WORD color;
    DWORD dwWritten;

    switch (what->type) {
    case Message::TYPE_NORMAL:
	// grey
	color = FOREGROUND_BLUE |FOREGROUND_GREEN | FOREGROUND_RED;
	break;
    case Message::TYPE_WARNING:
	// bright yellow
	color = FOREGROUND_GREEN | FOREGROUND_RED | FOREGROUND_INTENSITY;
	break;
    case Message::TYPE_ERROR:
	// bright red
	color = FOREGROUND_RED | FOREGROUND_INTENSITY;
	break;
    }
    SetConsoleTextAttribute(ConOut, color);
    WriteConsole(ConOut, what->bytes, what->length, &dwWritten, 0L);
    delete what;
}
