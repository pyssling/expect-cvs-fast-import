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
 * RCS: @(#) $Id: expWinTestClientInteract.cpp,v 1.1.2.1 2002/06/28 01:26:57 davygrvy Exp $
 * ----------------------------------------------------------------------------
 */

#include "expWinTestClient.hpp"

class ReadConWithSwitching : public CMclThreadHandler
{
    CMclQueue<Message *> &mQ;
    CMclEvent &Stop;
    HANDLE ConIn;
    CMclWaitableCollection groupedHandles;
#   define RECORD_SIZE 15

public:
    ReadConWithSwitching(CMclQueue<Message *> &_mQ, CMclEvent &_Stop, HANDLE _ConIn)
	: mQ(_mQ), Stop(_Stop), ConIn(_ConIn)
    {
	// Turn on mouse and window events, too.
        SetConsoleMode(ConIn, ENABLE_MOUSE_INPUT);
    }

    ~ReadConWithSwitching()
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


ClientInteract::ClientInteract(CMclQueue<Message *> &_mQ)
    : mQ(_mQ), ConOut(0L), ConIn(0L), ConInteract(0L), oldBuffer(0L)
{
    COORD Size = {80, 500};

    ConIn = CreateFile("CONIN$", GENERIC_READ|GENERIC_WRITE,
	    FILE_SHARE_READ|FILE_SHARE_WRITE, 0L, OPEN_EXISTING, 0, 0L);

    reader = new ReadConWithSwitching(mQ, Stop, ConIn);
    readThread = new CMclThread(reader);

    oldBuffer = CreateFile("CONOUT$", GENERIC_READ|GENERIC_WRITE,
	    FILE_SHARE_READ|FILE_SHARE_WRITE, 0L, OPEN_EXISTING, 0, 0L);
    ConOut = CreateConsoleScreenBuffer(GENERIC_READ|GENERIC_WRITE,
	    FILE_SHARE_READ|FILE_SHARE_WRITE, 0L, CONSOLE_TEXTMODE_BUFFER, 0L);
//    SetConsoleCtrlHandler(ConsoleHandler, TRUE);
    SetConsoleActiveScreenBuffer(ConOut);
    SetConsoleScreenBufferSize(ConOut, Size);
}

ClientInteract::~ClientInteract()
{
}

void ClientInteract::Write(Message *)
{
}
