/* ----------------------------------------------------------------------------
 * expWinSlave.hpp --
 *
 *	Useful definitions used by the slave driver application but not
 *	useful for anybody else.
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
 * RCS: @(#) $Id: expWinSlave.hpp,v 1.1.4.2 2002/03/08 23:37:16 davygrvy Exp $
 * ----------------------------------------------------------------------------
 */
#ifndef _EXPWINSLAVE_HPP
#define _EXPWINSLAVE_HPP

typedef struct ExpSlaveDebugArg {
    HANDLE hMaster;		/* Output handle */
    HANDLE hConsole;		/* Console handle to use */
    HANDLE process;		/* Handle to subprocess */
    DWORD globalPid;		/* Program identifier of slave */
    HANDLE thread;		/* Handle of debugger thread */

    HANDLE event;		/* Gets set when the process has been created */
    DWORD result;		/* Result of process being started */
    DWORD lastError;		/* GetLastError for result */
    int passThrough;		/* Pass through mode? */
    int useSocket;		/* Communicate to master through socket */

    /* Args for ExpCreateProcess */
    int argc;			/* Number of args to start slave program */
    char **argv;		/* Argument list of slave program (in UTF-8) */
    HANDLE slaveStdin;		/* stdin for slave program */
    HANDLE slaveStdout;		/* stdout for slave program */
    HANDLE slaveStderr;		/* stderr for slave program */
} ExpSlaveDebugArg;

typedef struct _EXP_KEY {
    WORD wVirtualKeyCode;
    WORD wVirtualScanCode;
    DWORD dwControlKeyState;
} EXP_KEY;

#define EXP_KEY_CONTROL 0
#define EXP_KEY_SHIFT   1
#define EXP_KEY_LSHIFT  1
#define EXP_KEY_RSHIFT  2
#define EXP_KEY_ALT     3


/* For ExpVtFunctionToKeyArray.  Ordering must match ExpFunctionToKeyArray[] */
#define EXP_KEY_UP		0
#define EXP_KEY_DOWN		1
#define EXP_KEY_RIGHT		2
#define EXP_KEY_LEFT		3
#define EXP_KEY_END		4
#define EXP_KEY_HOME		5
#define EXP_KEY_PAGEUP		6
#define EXP_KEY_PAGEDOWN	7
#define EXP_KEY_INSERT		8
#define EXP_KEY_DELETE		9
#define EXP_KEY_SELECT		10
#define EXP_KEY_F1		11
#define EXP_KEY_F2		12
#define EXP_KEY_F3		13
#define EXP_KEY_F4		14
#define EXP_KEY_F5		15
#define EXP_KEY_F6		16
#define EXP_KEY_F7		17
#define EXP_KEY_F8		18
#define EXP_KEY_F9		19
#define EXP_KEY_F10		20
#define EXP_KEY_F11		21
#define EXP_KEY_F12		22
#define EXP_KEY_F13		23
#define EXP_KEY_F14		24
#define EXP_KEY_F15		25
#define EXP_KEY_F16		26
#define EXP_KEY_F17		27
#define EXP_KEY_F18		28
#define EXP_KEY_F19		29
#define EXP_KEY_F20		30
#define EXP_WIN_RESIZE		31

extern EXP_KEY ExpModifierKeyArray[];
extern EXP_KEY ExpAsciiToKeyArray[];
extern EXP_KEY ExpFunctionToKeyArray[];
extern DWORD   ExpConsoleInputMode;
extern HANDLE  ExpConsoleOut;
extern int     ExpDebug;

extern TCL_CPP void ExpAddToWaitQueue(HANDLE handle);
extern TCL_CPP void ExpKillProcessList();
extern TCL_CPP DWORD WINAPI ExpSlaveDebugThread(LPVOID arg);
extern TCL_CPP DWORD WINAPI ExpGetExecutablePathA(PSTR pathInOut);
extern TCL_CPP DWORD WINAPI ExpGetExecutablePathW(PWSTR pathInOut);
extern TCL_CPP BOOL ExpWriteMaster(int useSocket, HANDLE hFile, LPCVOID buf, DWORD n, LPOVERLAPPED over);
extern TCL_CPP BOOL ExpReadMaster(int useSocket, HANDLE hFile, void *buf, DWORD n, PDWORD pCount, LPWSAOVERLAPPED over, PDWORD pError);
extern TCL_CPP void ExpNewConsoleSequences(int useSocket, HANDLE hMaster, LPWSAOVERLAPPED over);
extern TCL_CPP void ExpProcessFreeByHandle(HANDLE hProcess);
extern TCL_CPP void ExpSetConsoleSize(HANDLE hConsoleInW, HANDLE hConsoleOut, int w, int h, int useSocket, HANDLE hMaster, LPWSAOVERLAPPED over);
extern TCL_CPP void ExpDynloadTclStubs (void);

#ifdef __cplusplus
#include "./Mcl/include/CMcl.h"

class Message
{
    char *lala;
};

class ExpSpawnClientTransport
{
public:
    virtual void ExpWriteMaster() = 0;
    virtual void ExpReadMaster() = 0;
};

class ExpSpawnMailboxClient : public ExpSpawnClientTransport
{
public:
    ExpSpawnMailboxClient(const char *name);
    virtual void ExpWriteMaster();
    virtual void ExpReadMaster();
private:
    CMclMailbox *MasterToExpect;
    CMclMailbox *MasterFromExpect;
};

class ExpSpawnSocketCli : public ExpSpawnClientTransport
{
public:
    ExpSpawnSocketCli(const char *name);
    virtual void ExpWriteMaster();
    virtual void ExpReadMaster();
private:
    SOCKET sock;
};


class ExpSlaveTrap {
};
class ExpSlaveTrapPipe : public ExpSlaveTrap {
public:
    ExpSlaveTrapPipe(int argc, char * const argv[], CMclQueue<Message> &mQ);
};

class ExpSlaveTrapDbg : public ExpSlaveTrap {
public:
    ExpSlaveTrapDbg(int argc, char * const argv[], CMclQueue<Message> &mQ);
private:
    CMclThreadAutoPtr debuggerThread;
};
#endif /* __cplusplus */

#endif /* _EXPWINSLAVE_HPP */