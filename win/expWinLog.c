/* 
 * expWinLog.c --
 *
 *	This file logs to the NT system log.  Use the Event Viewer to
 *	see these logs.  This was predominately used to debug the
 *	slavedrv.exe process.
 *
 * Copyright (c) 1997 Mitel Corporation
 *
 * See the file "license.terms" for information on usage and redistribution
 * of this file, and for a DISCLAIMER OF ALL WARRANTIES.
 *
 */

#include "tcl.h"
#include "tclPort.h"
#include "expWin.h"
#include "spawndrvmc.h"

#ifdef _MSC_VER
#   pragma comment (lib, "advapi32.lib")
#endif

static HANDLE hSyslog = NULL;
static HANDLE hToken;
static TOKEN_USER *hUserTokPtr;
static TCHAR sysMsgSpace[1024];

/* local protos */
static void ExpNTLog (DWORD errCode, char *errData[], int cnt);
static TCHAR *Exp95Log (DWORD errCode, char *errData[], int cnt);
static void AddEventSource();

#define GETSEVERITY(code)   (UCHAR)((code >> 30) & 0x3) 
#define GETFACILITY(code)   (WORD)((code >> 16) & 0x0FFF)
#define GETCODE(code)	    (WORD)(code & 0xFFFF)

/*
 *----------------------------------------------------------------------
 *
 * ExpSyslog --
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
ExpSyslog TCL_VARARGS_DEF(DWORD,arg1)
{
    DWORD errCode;
    va_list args;
    char *errData[10];
    int cnt = 0;
    TCHAR *errMsg;
    static TCHAR codeBuf[33];
    DWORD dwWritten;

    /* Get the error code */
    errCode = TCL_VARARGS_START(DWORD,arg1,args);

    /* Get the file info */
    errData[cnt++] = va_arg(args, char *);

    /* Set the textual severity */
    switch(GETSEVERITY(errCode)) {
	case STATUS_SEVERITY_WARNING:
	    errData[cnt++] = _T("Warning"); break;
	case STATUS_SEVERITY_SUCCESS:
	    errData[cnt++] = _T("Success"); break;
	case STATUS_SEVERITY_INFORMATIONAL:
	    errData[cnt++] = _T("Info"); break;
	case STATUS_SEVERITY_FATAL:
	    errData[cnt++] = _T("Fatal"); break;
    }

    /* Set the textual Facility */
    switch(GETFACILITY(errCode)) {
	case FACILITY_WINSOCK:
	    errData[cnt++] = _T("Winsock IPC"); break;
	case FACILITY_SYSTEM:
	    errData[cnt++] = _T("System"); break;
	case FACILITY_STUBS:
	    errData[cnt++] = _T("Stubs"); break;
	case FACILITY_NAMEDPIPE:
	    errData[cnt++] = _T("NamedPipe IPC"); break;
	case FACILITY_MSPROTO:
	    errData[cnt++] = _T("Master/Slave Protocol"); break;
	case FACILITY_MAILBOX:
	    errData[cnt++] = _T("MailBoxing IPC"); break;
	case FACILITY_IO:
	    errData[cnt++] = _T("I/O [general]"); break;
	case FACILITY_DBGTRAP:
	    errData[cnt++] = _T("Debug/Trap"); break;
    }
    /* Set the textual Code */
    errData[cnt++] = codeBuf;
    wsprintf(codeBuf, "0x%04X", GETCODE(errCode));

    /* set everyone else */
    while ((errData[cnt] = va_arg(args, char *)) != NULL) cnt++;
    va_end(args);

    /* format this error according to the message catalog contained in the exe. */
    errMsg = Exp95Log(errCode, errData, cnt);
    OutputDebugString(errMsg);

    /* If running under NT, also save this error in the event log. */
    /* TODO: add platform test */
    ExpNTLog(errCode, errData, cnt);

    if (GETSEVERITY(errCode) & STATUS_SEVERITY_FATAL) {
	/* I could have used printf() and fflush(), but chose the direct
	 * route instead */
	WriteFile(GetStdHandle(STD_OUTPUT_HANDLE), errMsg, _tcslen(errMsg),
		&dwWritten, NULL);
	/* Stop the world, I want to get off. */
	if (IsDebuggerPresent()) DebugBreak();

	Sleep(5000);
	ExitProcess(255);
    }

    LocalFree(errMsg);
}

TCHAR *ExpSyslogGetSysMsg (DWORD id)
{
    int chars;

    chars = wsprintf(sysMsgSpace, "[%d] ", id);

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

void ExpNTLog (DWORD errCode, char *errData[], int cnt)
{
    DWORD dwSize = 0;
    WORD wSev;

    /* Write-out the registry data each time */
    AddEventSource();
    hSyslog = RegisterEventSource(NULL, _T("ExpectSlaveDrv"));

    /* aquire the user SID */
    OpenProcessToken(GetCurrentProcess(), TOKEN_READ, &hToken);
    GetTokenInformation(hToken, TokenUser, NULL, dwSize, &dwSize);
    hUserTokPtr = (PTOKEN_USER) GlobalAlloc(GPTR, dwSize);
    GetTokenInformation(hToken, TokenUser, hUserTokPtr, dwSize, &dwSize);

    switch(GETSEVERITY(errCode)) {
	case STATUS_SEVERITY_WARNING:
	    wSev = EVENTLOG_WARNING_TYPE; break;
	case STATUS_SEVERITY_SUCCESS:
	case STATUS_SEVERITY_INFORMATIONAL:
	    wSev = EVENTLOG_INFORMATION_TYPE; break;
	case STATUS_SEVERITY_FATAL:
	    wSev = EVENTLOG_ERROR_TYPE; break;
    }

    ReportEvent(hSyslog, wSev, GETFACILITY(errCode), errCode,
	    hUserTokPtr->User.Sid, cnt, 0, errData, NULL);
    GlobalFree(hUserTokPtr);
    DeregisterEventSource(hSyslog);
}

void AddEventSource()
{
    HKEY hk; 
    DWORD dwData; 
    TCHAR szBuf[MAX_PATH];
    int len;
 
    /*
     * Add your source name as a subkey under the Application 
     * key in the EventLog registry key. 
     */
    RegCreateKey(HKEY_LOCAL_MACHINE, 
	_T("SYSTEM\\CurrentControlSet\\Services\\EventLog\\Application\\ExpectSlaveDrv"),
	&hk); 

    /*
     * Set the name of the message file.  This is us, literally.
     */
    len = GetModuleFileName(GetModuleHandle(NULL), szBuf, MAX_PATH);

    /*
     * Add the name to the EventMessageFile subkey.
     */
    RegSetValueEx(hk,
            _T("EventMessageFile"),
            0,
            REG_EXPAND_SZ,
            (LPBYTE) szBuf,
            (len + 1));

    /* We also do catagories, too. */
    RegSetValueEx(hk,
            _T("CategoryMessageFile"),
            0,
            REG_EXPAND_SZ,
            (LPBYTE) szBuf,
            (len + 1));

    /* Last catagory (facility) */
    dwData = FACILITY_MSPROTO; 

    RegSetValueEx(hk,
            _T("CategoryCount"),
            0,
            REG_DWORD,
            (LPBYTE) &dwData,
            sizeof(DWORD));

    /* Set the supported event types in the TypesSupported subkey. */

    dwData = EVENTLOG_ERROR_TYPE | EVENTLOG_WARNING_TYPE | 
        EVENTLOG_INFORMATION_TYPE; 

    RegSetValueEx(hk,
            _T("TypesSupported"),
            0,
            REG_DWORD,
            (LPBYTE) &dwData,
            sizeof(DWORD));

    RegCloseKey(hk); 
}
