/* ----------------------------------------------------------------------------
 * expWinLog.c --
 *
 *	This file logs to the NT system log.  Use the Event Viewer to
 *	see these logs.  This was predominately used to debug the
 *	slavedrv.exe process.
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
 * RCS: @(#) $Id: expWinLog.c,v 1.1.2.1.2.3 2002/03/09 05:48:51 davygrvy Exp $
 * ----------------------------------------------------------------------------
 */

#include "expWinInt.h"

static char sysMsgSpace[1024];

/* local protos */
static TCHAR *Exp95Log (DWORD errCode, char *errData[], int cnt);

#define GETSEVERITY(code)   (UCHAR)((code >> 30) & 0x3) 
#define GETFACILITY(code)   (WORD)((code >> 16) & 0x0FFF)
#define GETCODE(code)	    (WORD)(code & 0xFFFF)

/*
 *----------------------------------------------------------------------
 *
 * ExpWinSyslog --
 *
 *	Logs error messages to the system application event log.
 *	It is normally called through the macro EXP_LOG() when
 *	errors occur in the slave driver process, but it can be
 *	used elsewhere.
 *
 * Results:
 *	None
 *
 *----------------------------------------------------------------------
 */

void
ExpWinSyslog TCL_VARARGS_DEF(DWORD,arg1)
{
    DWORD errCode;
    va_list args;
    char *errData[10];
    int cnt = 0;
    TCHAR *errMsg;
    static char codeBuf[33];
    DWORD dwWritten;
    char *file;
    int line;
    static char fileInfo[MAX_PATH];

    /* Get the error code */
    errCode = TCL_VARARGS_START(DWORD,arg1,args);

    /* Get the file info */
    file = va_arg(args, char *);
    line = va_arg(args, int);
    wsprintfA(fileInfo, "%s(%d)", file, line);
    errData[cnt++] = fileInfo;

    /* Set the textual severity */
    switch(GETSEVERITY(errCode)) {
	case STATUS_SEVERITY_WARNING:
	    errData[cnt++] = "Warning"; break;
	case STATUS_SEVERITY_SUCCESS:
	    errData[cnt++] = "Success"; break;
	case STATUS_SEVERITY_INFORMATIONAL:
	    errData[cnt++] = "Info"; break;
	case STATUS_SEVERITY_FATAL:
	    errData[cnt++] = "Fatal"; break;
    }

    /* Set the textual Facility */
    switch(GETFACILITY(errCode)) {
	case FACILITY_WINSOCK:
	    errData[cnt++] = "Winsock IPC"; break;
	case FACILITY_SYSTEM:
	    errData[cnt++] = "System"; break;
	case FACILITY_STUBS:
	    errData[cnt++] = "Stubs"; break;
	case FACILITY_NAMEDPIPE:
	    errData[cnt++] = "NamedPipe IPC"; break;
	case FACILITY_MSPROTO:
	    errData[cnt++] = "Master/Slave Protocol"; break;
	case FACILITY_MAILBOX:
	    errData[cnt++] = "MailBoxing IPC"; break;
	case FACILITY_IO:
	    errData[cnt++] = "I/O general"; break;
	case FACILITY_DBGTRAP:
	    errData[cnt++] = "Debug/Trap"; break;
    }
    /* Set the textual Code */
    errData[cnt++] = codeBuf;
    wsprintfA(codeBuf, "0x%04X", GETCODE(errCode));

    /* set everyone else */
    while ((errData[cnt] = va_arg(args, char *)) != NULL) cnt++;
    va_end(args);

    /* format this error according to the message catalog contained in the exe. */
    errMsg = Exp95Log(errCode, errData, cnt);
    OutputDebugString(errMsg);

    /* If running under NT, also save this error in the event log. */
    /* TODO: add platform test */
//    ExpNTLog(errCode, errData, cnt);

    if (GETSEVERITY(errCode) & STATUS_SEVERITY_FATAL) {
	/* I could have used printf() and fflush(), but chose the direct
	 * route instead */
	WriteFile(GetStdHandle(STD_OUTPUT_HANDLE), errMsg, _tcslen(errMsg),
		&dwWritten, NULL);

	/* Stop the world, I want to get off. */
	//DebugBreak();

	Sleep(5000);
	ExitProcess(255);
    }

    LocalFree(errMsg);
}

char *ExpSyslogGetSysMsg (DWORD id)
{
    int chars;

    chars = wsprintfA(sysMsgSpace, "[%d] ", id);

    FormatMessage(
	    FORMAT_MESSAGE_FROM_SYSTEM |
	    FORMAT_MESSAGE_MAX_WIDTH_MASK,
	    NULL,
	    id,
	    0,
	    (LPVOID) &sysMsgSpace[chars],
	    (1024-chars),
	    0);

    return sysMsgSpace;
}

TCHAR *Exp95Log(DWORD errCode, char *errData[], int cnt)
{
    TCHAR *msg;

    FormatMessage(
	    FORMAT_MESSAGE_FROM_HMODULE |
	    FORMAT_MESSAGE_ALLOCATE_BUFFER |
	    FORMAT_MESSAGE_ARGUMENT_ARRAY,
	    GetModuleHandle(NULL),
	    errCode,
	    0,
	    (LPVOID) &msg,
	    0,
	    errData);

    return msg;
}
