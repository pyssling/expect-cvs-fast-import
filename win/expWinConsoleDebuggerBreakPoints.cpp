/* ----------------------------------------------------------------------------
 * expWinConsoleDebuggerBreakPoints.cpp --
 *
 *	Breakpoints for the ConsoleDebugger class are in here.  These define
 *	the behavior of what to do when a breakpoint happens in the slave
 *	we are intercepting.  From here, we transfer the stream .... (TBF)
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
 * RCS: @(#) $Id: expWinConsoleDebuggerBreakPoints.cpp,v 1.1.2.6 2002/03/09 03:10:31 davygrvy Exp $
 * ----------------------------------------------------------------------------
 */

#include "expWinConsoleDebugger.hpp"

// NOTE:  black magic abounds...  be warry young padwon...

/*
 *----------------------------------------------------------------------
 *
 * ConsoleDebugger::CreateVtSequence --
 *
 *	When moving the cursor to a new location, this will create
 *	the appropriate VT100 type sequence to get the cursor there.
 *
 * Results:
 *	None
 *
 * Side Effects:
 *	Characters are written to the pipe going to Expect
 *
 *----------------------------------------------------------------------
 */

void
ConsoleDebugger::CreateVtSequence(Process *proc, COORD newPos, DWORD n)
{
    COORD oldPos;
    CHAR buf[2048];
    DWORD count;
//    BOOL b;

    if (n == 0) {
	return;
    }

    oldPos = CursorPosition;

    if (CursorKnown && (newPos.Y > oldPos.Y) && (newPos.X == 0)) {
	buf[0] = '\r';
	memset(&buf[1], '\n', newPos.Y - oldPos.Y);
	count = 1 + newPos.Y - oldPos.Y;
    } else {
	// VT100 sequence
	wsprintfA(buf, "\033[%d;%dH", newPos.Y+1, newPos.X+1);
	count = strlen(buf);
    }
    newPos.X += (SHORT) (n % ConsoleSize.X);
    newPos.Y += (SHORT) (n / ConsoleSize.X);
    CursorPosition = newPos;

//    b = ExpWriteMaster(UseSocket, HMaster, buf, count, &proc->overlapped);
}

/*
 *-----------------------------------------------------------------------------
 *
 * ConsoleDebugger::OnBeep --
 *
 *	This routine gets called when Beep is called.  At least in sshd,
 *	we don't want a beep to show up on the local console.  Instead,
 *	direct it back to the master with a ASCII 7.
 *
 * Results:
 *	None
 *
 * Notes:
 *	XXX: Setting the duration to 0 doesn't seem to make the local
 *	beep go away.  It seems we need to stop the call at this point
 *	(or point it to some other call with the same number of arguments)
 *
 *-----------------------------------------------------------------------------
 */

void
ConsoleDebugger::OnBeep(Process *proc, ThreadInfo *threadInfo,
    Breakpoint *brkpt, PDWORD returnValue, DWORD direction)
{
    CHAR buf[2];

    if (direction == BREAK_IN) {
	// Modify the arguments so a beep doesn't sound on the server
	threadInfo->args[1] = 0;
    } else if (direction == BREAK_OUT) {
	if (*returnValue == 0) {
	    buf[0] = 7; /* ASCII beep */
//	    ExpWriteMaster(UseSocket, HMaster, buf, 1, &proc->overlapped);
	}
    }
}

/*
 *-----------------------------------------------------------------------------
 *
 * ConsoleDebugger::OnFillConsoleOutputCharacter --
 *
 *	This function gets called when an FillConsoleOutputCharacterA
 *	or FillConsoleOutputCharacterW breakpoint is hit.
 *
 * Results:
 *	None
 *
 * Side Effects:
 *	Prints some output.
 *
 *-----------------------------------------------------------------------------
 */

void
ConsoleDebugger::OnFillConsoleOutputCharacter(Process *proc,
    ThreadInfo *threadInfo, Breakpoint *brkpt, PDWORD returnValue,
    DWORD direction)
{
    CHAR buf[4096];
    int bufpos;
    UCHAR c;
    PVOID ptr;
    DWORD i;
    DWORD len;
//    BOOL bRet;
    COORD coord;
    DWORD lines, preCols, postCols;
    BOOL eol, bol;		// Needs clearing to end, beginning of line
    CONSOLE_SCREEN_BUFFER_INFO info;

    if (*returnValue == 0) {
	return;
    }

    c = (UCHAR) threadInfo->args[1];
    len = threadInfo->args[2];
    coord = *((PCOORD) &(threadInfo->args[3]));
    ptr = (PVOID) threadInfo->args[4];
    if (ptr) {
	ReadSubprocessMemory(proc, ptr, &len, sizeof(DWORD));
    }

    preCols = 0;
    bufpos = 0;
    eol = bol = FALSE;
    if (coord.X) {
	preCols = ConsoleSize.X - coord.X;
	if (len <= preCols) {
	    preCols = len;
	    len = 0;
	    if (len == preCols) {
		eol = TRUE;
	    }
	} else {
	    eol = TRUE;
	    len -= preCols;
	}
    } else if (len < (DWORD) ConsoleSize.X) {
	bol = TRUE;
	preCols = len;
	len = 0;
    }

    lines = len / ConsoleSize.X;
    postCols = len % ConsoleSize.X;

    if (preCols) {
	if (bol) {
	    // Beginning of line to before end of line
	    if (c == ' ') {
		wsprintfA(&buf[bufpos], "\033[%d;%dH\033[1K",
			  coord.Y+1, preCols+coord.X);
		bufpos += strlen(&buf[bufpos]);
	    } else {
		wsprintfA(&buf[bufpos], "\033[%d;%dH",
			  coord.Y+1, coord.X+1);
		bufpos += strlen(&buf[bufpos]);
		memset(&buf[bufpos], c, preCols);
		bufpos += preCols;
	    }
	} else {
	    // After beginning of line to end of line
	    wsprintfA(&buf[bufpos], "\033[%d;%dH", coord.Y+1, coord.X+1);
	    bufpos += strlen(&buf[bufpos]);
	    if (eol && c == ' ') {
		wsprintfA(&buf[bufpos], "\033[K");
		bufpos += strlen(&buf[bufpos]);
	    } else {
		memset(&buf[bufpos], c, preCols);
		bufpos += preCols;
	    }
	}
	coord.X = 0;
	coord.Y++;
    }
    if (lines) {
	if ((c == ' ') && ((lines + coord.Y) >= (DWORD) ConsoleSize.Y)) {
	    // Clear to end of screen
	    wsprintfA(&buf[bufpos], "\033[%d;%dH\033[J",
		      coord.Y+1, coord.X+1);
	    bufpos += strlen(&buf[bufpos]);
	} else if ((c == ' ') && (coord.Y == 0) && (lines > 0)) {
	    // Clear to top of screen
	    wsprintfA(&buf[bufpos], "\033[%d;%dH\033[1J", lines, 1);
	    bufpos += strlen(&buf[bufpos]);
	} else {
	    for (i = 0; i < lines; i++) {
		wsprintfA(&buf[bufpos], "\033[%d;%dH",
			  coord.Y+i+1, coord.X+1);
		bufpos += strlen(&buf[bufpos]);
		if (c == ' ') {
		    wsprintfA(&buf[bufpos], "\033[2K");
		    bufpos += strlen(&buf[bufpos]);
		} else {
		    memset(&buf[bufpos], c, ConsoleSize.X);
		    bufpos += ConsoleSize.X;
		}
	    }
	}
	coord.Y += (SHORT) lines;
    }
	
    if (postCols) {
	if (c == ' ') {
	    // Clear to beginning of line
	    wsprintfA(&buf[bufpos], "\033[%d;%dH\033[1K",
		      coord.Y+1, postCols+coord.X);
	    bufpos += strlen(&buf[bufpos]);
	} else {
	    wsprintfA(&buf[bufpos], "\033[%d;%dH", coord.X+1, coord.Y+1);
	    bufpos += strlen(&buf[bufpos]);
	    memset(&buf[bufpos], c, postCols);
	    bufpos += postCols;
	}
    }
    if (GetConsoleScreenBufferInfo(MasterHConsole, &info) == FALSE) {
	char errbuf[200];
	wsprintfA(errbuf, "handle=0x%08x", MasterHConsole);
	EXP_LOG2(MSG_DT_SCREENBUF, errbuf, ExpSyslogGetSysMsg(GetLastError()));
    } else {
	CursorPosition = info.dwCursorPosition;
	wsprintfA(&buf[bufpos], "\033[%d;%dH",
		  CursorPosition.Y+1, CursorPosition.X+1);
	bufpos += strlen(&buf[bufpos]);
	CursorKnown = TRUE;
    }

//    bRet = ExpWriteMaster(UseSocket, HMaster, buf, bufpos, &proc->overlapped);
}

/*
 *-----------------------------------------------------------------------------
 *
 * ConsoleDebugger::OnGetStdHandle --
 *
 *	This function gets called when a GetStdHandle breakpoint
 *	is hit.
 *
 * Results:
 *	None
 *
 * Side Effects:
 *	Sets some flags that are used in determining echoing
 *	characteristics of the slave driver.
 *
 *-----------------------------------------------------------------------------
 */

void
ConsoleDebugger::OnGetStdHandle(Process *proc, ThreadInfo *threadInfo,
    Breakpoint *brkpt, PDWORD returnValue, DWORD direction)
{
    DWORD i;
    BOOL found;

    if (*returnValue == (DWORD) INVALID_HANDLE_VALUE) {
	return;
    }
    if (threadInfo->args[0] != STD_INPUT_HANDLE) {
	return;
    }
    for (found = FALSE, i = 0; i < proc->consoleHandlesMax; i++) {
	if (proc->consoleHandles[i] == *returnValue) {
	    found = TRUE;
	    break;
	}
    }
    if (! found) {
	if (proc->consoleHandlesMax > 100) {
	    proc->consoleHandlesMax = 100;
	}
	proc->consoleHandles[proc->consoleHandlesMax++] = *returnValue;
    }
    return;
}

/*
 *----------------------------------------------------------------------
 *
 * ConsoleDebugger::OnOpenConsoleW --
 *
 *	This function gets called when an OpenConsoleW breakpoint
 *	is hit.  There is one big problem with this function--it
 *	isn't documented.  However, we only really care about the
 *	return value which is a console handle.  I think this is
 *	what this function declaration should be:
 *
 *	HANDLE OpenConsoleW(LPWSTR lpFileName,
 *			    DWORD dwDesiredAccess,
 *			    DWORD dwShareMode,
 *			    LPSECURITY_ATTRIBUTES lpSecurityAttributes);
 *
 *	So why do we intercept an undocumented function while we
 *	could just intercept CreateFileW and CreateFileA?  Well,
 *	those functions are going to get called alot more than this
 *	one, so limiting the number of intercepted functions
 *	improves performance since fewer breakpoints will be hit.
 *
 * Results:
 *	None
 *
 * Side Effects:
 *	Save the return value in an array of known console handles
 *	with their statuses.
 *
 *----------------------------------------------------------------------
 */

void
ConsoleDebugger::OnOpenConsoleW(Process *proc, ThreadInfo *threadInfo,
    Breakpoint *brkpt, PDWORD returnValue, DWORD direction)
{
    WCHAR name[256];
    PVOID ptr;

    if (*returnValue == (DWORD) INVALID_HANDLE_VALUE) {
	return;
    }

    // Save any console input handle.  No SetConsoleMode() calls will
    // succeed unless they are really attached to a console input buffer.
    //
    ptr = (PVOID) threadInfo->args[0];
    ReadSubprocessStringW(proc, ptr, name, 256);

    if (wcsicmp(name, L"CONIN$") == 0) {
	if (proc->consoleHandlesMax > 100) {
	    proc->consoleHandlesMax = 100;
	}
	proc->consoleHandles[proc->consoleHandlesMax++] = *returnValue;
    }
    return;
}

/*
 *----------------------------------------------------------------------
 *
 * ConsoleDebugger::OnReadConsoleInput --
 *
 *	This function gets called when a ReadConsoleInput breakpoint
 *	is hit.
 *
 * Results:
 *	None
 *
 * Notes:
 *	If this is ever used for real, there need to be ASCII
 *	and UNICODE versions.
 *
 *----------------------------------------------------------------------
 */

void
ConsoleDebugger::OnReadConsoleInput(Process *proc, ThreadInfo *threadInfo,
    Breakpoint *brkpt, PDWORD returnValue, DWORD direction)
{
}

/*
 *----------------------------------------------------------------------
 *
 * ConsoleDebugger::OnScrollConsoleScreenBuffer --
 *
 *	This funtions gets called when a ScrollConsoleScreenBuffer
 *	breakpoint is hit.
 *
 * Results:
 *	None
 *
 * Side Effects:
 *	Generate some VT100 sequences to insert lines
 *
 * Notes:
 *	XXX: Ideally, we should check if the screen buffer is the one that
 *	is currently being displayed.  However, that means we have to
 *	track CONOUT$ handles, so we don't do it for now.
 *
 *----------------------------------------------------------------------
 */

void
ConsoleDebugger::OnScrollConsoleScreenBuffer(Process *proc,
    ThreadInfo *threadInfo, Breakpoint *brkpt, PDWORD returnValue,
    DWORD direction)
{
//    BOOL b;
    CHAR buf[100];
    DWORD count = 0;
    SMALL_RECT scroll, clip, *pClip;
    COORD dest;
    CHAR_INFO fill;
    CHAR c;
    PVOID ptr;

    if (*returnValue == FALSE) {
	return;
    }
    ptr = (PVOID) threadInfo->args[1];
    ReadSubprocessMemory(proc, ptr, &scroll, sizeof(SMALL_RECT));
    ptr = (PVOID) threadInfo->args[2];
    pClip = 0L;
    if (ptr) {
	pClip = &clip;
	ReadSubprocessMemory(proc, ptr, &clip, sizeof(SMALL_RECT));
    }
    dest = *((PCOORD) &threadInfo->args[3]);
    ptr = (PVOID) threadInfo->args[4];
    ReadSubprocessMemory(proc, ptr, &fill, sizeof(CHAR_INFO));
    c = fill.Char.AsciiChar;

    // Check for a full line scroll
    if (c == ' ' && scroll.Left == dest.X &&
	scroll.Left == 0 && scroll.Right >= ConsoleSize.X-1)
    {
	if (dest.Y < scroll.Top) {
	    wsprintfA(&buf[count], "\033[%d;%dr\033[%d;%dH\033[%dM",
		      dest.Y+1,scroll.Bottom+1,dest.Y+1,1,
		      scroll.Top - dest.Y);
	} else {
	    wsprintfA(&buf[count], "\033[%d;%dr\033[%d;%dH\033[%dL",
		      scroll.Top+1,dest.Y+1+(scroll.Bottom - scroll.Top),
		      scroll.Top+1,1,
		      dest.Y - scroll.Top);
	}
	count = strlen(&buf[count]);
	wsprintf(&buf[count], "\033[%d;%dr", 1, ConsoleSize.Y);
	count += strlen(&buf[count]);
//	b = ExpWriteMaster(UseSocket, HMaster, buf, count, &proc->overlapped);
    } else {
//	RefreshScreen(&proc->overlapped);
    }
}

/*
 *----------------------------------------------------------------------
 *
 * ConsoleDebugger::OnSetConsoleMode --
 *
 *	This function gets called when a SetConsoleMode breakpoint
 *	is hit.
 *
 * Results:
 *	None
 *
 * Side Effects:
 *	Sets some flags that are used in determining echoing
 *	characteristics of the slave driver.
 *
 *----------------------------------------------------------------------
 */

void
ConsoleDebugger::OnSetConsoleMode(Process *proc, ThreadInfo *threadInfo,
    Breakpoint *brkpt, PDWORD returnValue, DWORD direction)
{
    DWORD i;
    BOOL found;

    // The console mode seems to get set even if the return value is FALSE
    if (*returnValue == FALSE) {
	return;
    }
    for (found = FALSE, i = 0; i < proc->consoleHandlesMax; i++) {
	if (threadInfo->args[0] == proc->consoleHandles[i]) {
	    found = TRUE;
	    break;
	}
    }
    if (found) {
	MasterConsoleInputMode = threadInfo->args[1];
    }
}

/*
 *----------------------------------------------------------------------
 *
 * ConsoleDebugger::OnSetConsoleActiveScreenBuffer --
 *
 *	This function gets called when a SetConsoleActiveScreenBuffer
 *	breakpoint is hit.
 *
 * Results:
 *	None
 *
 * Side Effects:
 *	We reread the entire console and send it to the master.
 *	Updates the current console cursor position
 *
 *----------------------------------------------------------------------
 */

void
ConsoleDebugger::OnSetConsoleActiveScreenBuffer(Process *proc,
    ThreadInfo *threadInfo, Breakpoint *brkpt, PDWORD returnValue,
    DWORD direction)
{
    if (*returnValue == FALSE) {
	return;
    }

//    RefreshScreen(&proc->overlapped);
}

/*
 *----------------------------------------------------------------------
 *
 * ConsoleDebugger::OnSetConsoleCursorPosition --
 *
 *	This function gets called when a SetConsoleCursorPosition breakpoint
 *	is hit.
 *
 * Results:
 *	None
 *
 * Side Effects:
 *	Updates the current console cursor position
 *
 *----------------------------------------------------------------------
 */

void
ConsoleDebugger::OnSetConsoleCursorPosition(Process *proc,
    ThreadInfo *threadInfo, Breakpoint *brkpt, PDWORD returnValue,
    DWORD direction)
{
//    BOOL b;
    CHAR buf[50];
    DWORD count;

    if (*returnValue == FALSE) {
	return;
    }
    CursorPosition = *((PCOORD) &threadInfo->args[1]);

    wsprintfA(buf, "\033[%d;%dH", CursorPosition.Y+1, CursorPosition.X+1);
    count = strlen(buf);
//    b = ExpWriteMaster(UseSocket, HMaster, buf, count, &proc->overlapped);
}

/*
 *----------------------------------------------------------------------
 *
 * ConsoleDebugger::OnSetConsoleWindowInfo --
 *
 *	This function gets called when a SetConsoleWindowInfo breakpoint
 *	is hit.
 *
 * Results:
 *	None
 *
 * Side Effects:
 *	Updates the current console cursor position
 *
 *----------------------------------------------------------------------
 */

void
ConsoleDebugger::OnSetConsoleWindowInfo(Process *proc,
    ThreadInfo *threadInfo, Breakpoint *brkpt, PDWORD returnValue,
    DWORD direction)
{
}

/*
 *----------------------------------------------------------------------
 *
 * ConsoleDebugger::OnWriteConsoleA --
 *
 *	This function gets called when an WriteConsoleA breakpoint
 *	is hit.  The data is also redirected to expect since expect
 *	normally couldn't see any output going through this interface.
 *
 * Results:
 *	None
 *
 * Side Effects:
 *	Prints some output.
 *
 *----------------------------------------------------------------------
 */

void
ConsoleDebugger::OnWriteConsoleA(Process *proc, ThreadInfo *threadInfo,
    Breakpoint *brkpt, PDWORD returnValue, DWORD direction)
{
    CHAR buf[1024];
    PVOID ptr;
    DWORD n;
    PCHAR p;
//    BOOL bRet;

    if (*returnValue == 0) {
	return;
    }
    // Get number of bytes written
    ptr = (PVOID) threadInfo->args[3];
    if (ptr == 0L) {
	n = threadInfo->args[2];
    } else {
	ReadSubprocessMemory(proc, ptr, &n, sizeof(DWORD));
    }
    if (n > 1024) {
	p = new CHAR [n];
    } else {
	p = buf;
    }

    ptr = (PVOID) threadInfo->args[1];
    ReadSubprocessMemory(proc, ptr, p, n * sizeof(CHAR));
//    ResetEvent(proc->overlapped.hEvent);

//    bRet = ExpWriteMaster(UseSocket, HMaster, p, n, &proc->overlapped);

    if (p != buf) {
	delete [] p;
    }
    CursorKnown = FALSE;
}

/*
 *----------------------------------------------------------------------
 *
 * ConsoleDebugger::OnWriteConsoleW --
 *
 *	This function gets called when an WriteConsoleW breakpoint
 *	is hit.
 *
 * Results:
 *	None
 *
 * Side Effects:
 *	Prints some output.
 *
 *----------------------------------------------------------------------
 */

void
ConsoleDebugger::OnWriteConsoleW(Process *proc, ThreadInfo *threadInfo,
    Breakpoint *brkpt, PDWORD returnValue, DWORD direction)
{
    static WCHAR buf[1024];
    static CHAR ansi[2048];
    PVOID ptr;
    DWORD n;
    PWCHAR p;
    PCHAR a;
    int asize;
//    BOOL bRet;
    int w;

    if (*returnValue == 0) {
	return;
    }

    ptr = (PVOID) threadInfo->args[1];
    n = threadInfo->args[2];

    if (n > 1024) {
	p = new WCHAR [n];
	asize = n * 2 * sizeof(CHAR);
	a = new CHAR [n * 2];
    } else {
	p = buf;
	a = ansi;
	asize = sizeof(ansi);
    }
    ReadSubprocessMemory(proc, ptr, p, n * sizeof(WCHAR));
//    ResetEvent(proc->overlapped.hEvent);

    // Convert to ASCII and write the intercepted data to the pipe.
    //
    w = WideCharToMultiByte(CP_ACP, 0, p, n, a, asize, 0L, 0L);
//    bRet = ExpWriteMaster(UseSocket, HMaster, a, w, &proc->overlapped);

    if (p != buf) {
	delete [] p, a;
    }
    CursorKnown = FALSE;
}

/*
 *----------------------------------------------------------------------
 *
 * ConsoleDebugger::OnWriteConsoleOutputA --
 *
 *	This function gets called when an WriteConsoleOutputA breakpoint
 *	is hit.  The data is also redirected to expect since expect
 *	normally couldn't see any output going through this interface.
 *
 * Results:
 *	None
 *
 * Side Effects:
 *	Prints some output.
 *
 *----------------------------------------------------------------------
 */

void
ConsoleDebugger::OnWriteConsoleOutputA(Process *proc,
    ThreadInfo *threadInfo, Breakpoint *brkpt, PDWORD returnValue,
    DWORD direction)
{
    CHAR buf[1024];
    PVOID ptr;
    DWORD n;
    CHAR *p, *end;
    int maxbuf;
//    BOOL b;
    COORD bufferSize;
    COORD bufferCoord;
    COORD curr;
    SMALL_RECT writeRegion;
    CHAR_INFO *charBuf, *pcb;
    SHORT x, y;

    if (*returnValue == 0) {
	return;
    }

    bufferSize = *((PCOORD) &threadInfo->args[2]);
    bufferCoord = *((PCOORD) &threadInfo->args[3]);
    ptr = (PVOID) threadInfo->args[4]; // Get the rectangle written
    if (ptr == 0L) return;
    ReadSubprocessMemory(proc, ptr, &writeRegion,sizeof(SMALL_RECT));

    ptr = (PVOID) threadInfo->args[1]; // Get character array
    if (ptr == 0L) return;

    n = bufferSize.X * bufferSize.Y * sizeof(CHAR_INFO);
    charBuf = new CHAR_INFO [n];

#if 0
    wsprintfA((char *) charBuf, "writeRegion: (%d,%d) to (%d,%d)   bufferCoord: (%d,%d)   bufferSize: (%d,%d)", writeRegion.Left, writeRegion.Top, writeRegion.Right, writeRegion.Bottom, bufferCoord.X, bufferCoord.Y, bufferSize.X, bufferSize.Y);
    ExpSyslog("Debug 0: %s", charBuf);
#endif

    ReadSubprocessMemory(proc, ptr, charBuf, n);

    pcb = charBuf;
    for (y = 0; y <= writeRegion.Bottom - writeRegion.Top; y++) {
	pcb = charBuf;
	pcb += (y + bufferCoord.Y) * bufferSize.X;
	pcb += bufferCoord.X;
	p = buf;
	maxbuf = sizeof(buf);
	end = buf + maxbuf;
	for (x = 0; x <= writeRegion.Right - writeRegion.Left; x++, pcb++) {
	    *p++ = pcb->Char.AsciiChar;
	    if (p == end) {
//		ResetEvent(proc->overlapped.hEvent);
//		b = ExpWriteMaster(UseSocket, HMaster, buf, maxbuf, &proc->overlapped);
		p = buf;
	    }
	}
	curr.X = writeRegion.Left;
	curr.Y = writeRegion.Top + y;
	n = writeRegion.Right - writeRegion.Left;
	CreateVtSequence(proc, curr, n);
//	ResetEvent(proc->overlapped.hEvent);

	maxbuf = p - buf;
//	b = ExpWriteMaster(UseSocket, HMaster, buf, maxbuf, &proc->overlapped);
	buf[maxbuf] = 0;
    }

    delete [] charBuf;
}

/*
 *----------------------------------------------------------------------
 *
 * ConsoleDebugger::OnWriteConsoleOutputW --
 *
 *	This function gets called when an WriteConsoleOutputW breakpoint
 *	is hit.  The data is also redirected to expect since expect
 *	normally couldn't see any output going through this interface.
 *
 * Results:
 *	None
 *
 * Side Effects:
 *	Prints some output.
 *
 *----------------------------------------------------------------------
 */

void
ConsoleDebugger::OnWriteConsoleOutputW(Process *proc,
    ThreadInfo *threadInfo, Breakpoint *brkpt, PDWORD returnValue,
    DWORD direction)
{
    WCHAR buf[1024];
    PVOID ptr;
    DWORD n;
    WCHAR *p, *end;
    int maxbuf;
//    BOOL b;
    COORD bufferSize;
    COORD bufferCoord;
    COORD curr;
    SMALL_RECT writeRegion;
    CHAR_INFO *charBuf, *pcb;
    SHORT x, y;

    if (*returnValue == 0) {
	return;
    }

    bufferSize = *((PCOORD) &threadInfo->args[2]);
    bufferCoord = *((PCOORD) &threadInfo->args[3]);
    ptr = (PVOID) threadInfo->args[4]; // Get the rectangle written
    if (ptr == 0L) return;
    ReadSubprocessMemory(proc, ptr, &writeRegion,sizeof(SMALL_RECT));

    ptr = (PVOID) threadInfo->args[1]; // Get character array
    if (ptr == 0L) return;

    n = bufferSize.X * bufferSize.Y * sizeof(CHAR_INFO);
    charBuf = new CHAR_INFO [n];

#if 0
    wsprintfA((char *) charBuf, "writeRegion: (%d,%d) to (%d,%d)   bufferCoord: (%d,%d)   bufferSize: (%d,%d)", writeRegion.Left, writeRegion.Top, writeRegion.Right, writeRegion.Bottom, bufferCoord.X, bufferCoord.Y, bufferSize.X, bufferSize.Y);
    ExpSyslog("Debug 0: %s", charBuf);
#endif

    ReadSubprocessMemory(proc, ptr, charBuf, n);

    pcb = charBuf;
    for (y = 0; y <= writeRegion.Bottom - writeRegion.Top; y++) {
	pcb = charBuf;
	pcb += (y + bufferCoord.Y) * bufferSize.X;
	pcb += bufferCoord.X;
	p = buf;
	maxbuf = sizeof(buf);
	end = buf + maxbuf;
	for (x = 0; x <= writeRegion.Right - writeRegion.Left; x++, pcb++) {
	    *p++ = (CHAR) (pcb->Char.UnicodeChar & 0xff);
	    if (p == end) {
//		ResetEvent(proc->overlapped.hEvent);
//		b = ExpWriteMaster(UseSocket, HMaster, buf, maxbuf, &proc->overlapped);
		p = buf;
	    }
	}
	curr.X = writeRegion.Left;
	curr.Y = writeRegion.Top + y;
	n = writeRegion.Right - writeRegion.Left;
	CreateVtSequence(proc, curr, n);
//	ResetEvent(proc->overlapped.hEvent);

	maxbuf = p - buf;
//	b = ExpWriteMaster(UseSocket, HMaster, buf, maxbuf, &proc->overlapped);
	buf[maxbuf] = 0;
#if 0
	ExpSyslog("Writing %s", buf);
#endif
    }

    delete [] charBuf;
}

/*
 *----------------------------------------------------------------------
 *
 * ConsoleDebugger::OnWriteConsoleOutputCharacterA --
 *
 *	This function gets called when an WriteConsoleOutputCharacterA breakpoint
 *	is hit.  The data is also redirected to expect since expect
 *	normally couldn't see any output going through this interface.
 *
 * Results:
 *	None
 *
 * Side Effects:
 *	Prints some output.
 *
 *----------------------------------------------------------------------
 */

void
ConsoleDebugger::OnWriteConsoleOutputCharacterA(Process *proc,
    ThreadInfo *threadInfo, Breakpoint *brkpt, PDWORD returnValue, DWORD direction)
{
    static CHAR buf[1024];
    PVOID ptr;
    DWORD n;
    PCHAR p;
//    BOOL b;

    if (*returnValue == 0) {
	return;
    }
    // Get number of bytes written
    ptr = (PVOID) threadInfo->args[4];
    if (ptr == 0L) {
	n = threadInfo->args[2];
    } else {
	ReadSubprocessMemory(proc, ptr, &n, sizeof(DWORD));
    }

    CreateVtSequence(proc, *((PCOORD) &threadInfo->args[3]), n);

    if (n > 1024) {
	p = new CHAR [n];
    } else {
	p = buf;
    }

    ptr = (PVOID) threadInfo->args[1];
    ReadSubprocessMemory(proc, ptr, p, n * sizeof(CHAR));
//    ResetEvent(proc->overlapped.hEvent);

//    b = ExpWriteMaster(UseSocket, HMaster, p, n, &proc->overlapped);

    if (p != buf) {
	delete [] p;
    }
    CursorKnown = FALSE;
}

/*
 *----------------------------------------------------------------------
 *
 * ConsoleDebugger::OnWriteConsoleOutputCharacterW --
 *
 *	This function gets called when an WriteConsoleOutputCharacterW
 *	breakpoint is hit.
 *
 * Results:
 *	None
 *
 * Side Effects:
 *	Prints some output.
 *
 *----------------------------------------------------------------------
 */

void
ConsoleDebugger::OnWriteConsoleOutputCharacterW(Process *proc,
    ThreadInfo *threadInfo, Breakpoint *brkpt, PDWORD returnValue,
    DWORD direction)
{
    static WCHAR buf[1024];
    static CHAR ansi[2048];
    PVOID ptr;
    DWORD n;
    PWCHAR p;
    PCHAR a;
    int asize;
//    BOOL b;
    int w;

    if (*returnValue == 0) {
	return;
    }
    // Get number of bytes written
    ptr = (PVOID) threadInfo->args[4];
    if (ptr == 0L) {
	n = threadInfo->args[2];
    } else {
	ReadSubprocessMemory(proc, ptr, &n, sizeof(DWORD));
    }

    CreateVtSequence(proc, *((PCOORD) &threadInfo->args[3]), n);

    if (n > 1024) {
	p = new WCHAR [n];
	asize = n * 2 * sizeof(CHAR);
	a = new CHAR [n * 2];
    } else {
	p = buf;
	a = ansi;
	asize = sizeof(ansi);
    }

    ptr = (PVOID) threadInfo->args[1];
    ReadSubprocessMemory(proc, ptr, p, n * sizeof(WCHAR));
//    ResetEvent(proc->overlapped.hEvent);

    // Convert to ASCI and Write the intercepted data to the pipe.
    w = WideCharToMultiByte(CP_ACP, 0, p, n, a, asize, 0L, 0L);
//    b = ExpWriteMaster(UseSocket, HMaster, a, w, &proc->overlapped);

    if (p != buf) {
	delete [] p, a;
    }
    CursorKnown = FALSE;
}

/*
 *----------------------------------------------------------------------
 *
 * ConsoleDebugger::OnIsWindowVisible --
 *
 *	This routine gets called when IsWindowVisible is called.
 *	The MKS Korn shell uses this as an indication of a window
 *	that can be seen by the user.  If the window can't be seen,
 *	it pops up a graphical error notification.  We really, really
 *	don't want those damn things popping up, so this helps avoid
 *	it.  And there really doesn't seem to be any good reason to
 *	return FALSE given that nobody is ever going to see anything.
 *
 * Results:
 *	None
 *
 *----------------------------------------------------------------------
 */

void
ConsoleDebugger::OnIsWindowVisible(Process *proc, ThreadInfo *threadInfo,
    Breakpoint *brkpt, PDWORD returnValue, DWORD direction)
{
    *returnValue = TRUE;
}
