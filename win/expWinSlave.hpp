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
 * RCS: @(#) $Id: expWinSlave.hpp,v 1.1.4.8 2002/03/12 07:59:14 davygrvy Exp $
 * ----------------------------------------------------------------------------
 */
#ifndef _EXPWINSLAVE_HPP
#define _EXPWINSLAVE_HPP

#include <windows.h>

extern DWORD   ExpConsoleInputMode;
extern HANDLE  ExpConsoleOut;
extern int     ExpDebug;

//extern TCL_CPP void ExpAddToWaitQueue(HANDLE handle);
//extern TCL_CPP void ExpKillProcessList();
//extern TCL_CPP DWORD WINAPI ExpSlaveDebugThread(LPVOID arg);
//extern TCL_CPP DWORD WINAPI ExpGetExecutablePathA(PSTR pathInOut);
//extern TCL_CPP DWORD WINAPI ExpGetExecutablePathW(PWSTR pathInOut);
//extern TCL_CPP BOOL ExpWriteMaster(int useSocket, HANDLE hFile, LPCVOID buf, DWORD n, LPOVERLAPPED over);
//extern TCL_CPP BOOL ExpReadMaster(int useSocket, HANDLE hFile, void *buf, DWORD n, PDWORD pCount, LPWSAOVERLAPPED over, PDWORD pError);
//extern TCL_CPP void ExpNewConsoleSequences(int useSocket, HANDLE hMaster, LPWSAOVERLAPPED over);
//extern TCL_CPP void ExpProcessFreeByHandle(HANDLE hProcess);
//extern TCL_CPP void ExpSetConsoleSize(HANDLE hConsoleInW, HANDLE hConsoleOut, int w, int h, int useSocket, HANDLE hMaster, LPWSAOVERLAPPED over);

void DynloadTclStubs (void);
void ShutdownTcl (void);


#include "Mcl/include/CMcl.h"
#include "slavedrvmc.h"
#include "expWinUtils.hpp"
#include "expWinMessage.hpp"
#include "expWinConsoleDebugger.hpp"
#include "expWinSpawnClient.hpp"
#include "expWinSlaveTrap.hpp"

#endif /* _EXPWINSLAVE_HPP */