/* ----------------------------------------------------------------------------
 * expWinMailboxCli.cpp --
 *
 *	Inter-Process-Communication (IPC) transport using shared memory (file
 *	mapping).  These are the client routines used by spawndrv.exe to
 *	connect to the "exp_spawn" channel driver within the Expect extension.
 *	This is bi-directional like sockets and namedpipes.  This works for
 *	ALL versions of windows.  This IPC method does not traverse a network
 *	and is only local to a single computer (or instance of a Win32 OS).
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
 *	work by David Gravereaux <davygrvy@pobox.com> for any Win32 OS.
 *
 * ----------------------------------------------------------------------------
 * URLs:    http://expect.nist.gov/
 *	    http://expect.sf.net/
 *	    http://bmrc.berkeley.edu/people/chaffee/expectnt.html
 * ----------------------------------------------------------------------------
 * RCS: @(#) $Id: exp.h,v 1.1.2.5 2001/10/29 06:40:29 davygrvy Exp $
 * ----------------------------------------------------------------------------
 */
#include "expWinInt.h"

#include "./Mcl/include/CMcl.h"

CMclMailbox *MasterToExpect = 0L;
CMclMailbox *MasterFromExpect = 0L;

void
SpawnOpenClientMailbox (const char *box)
{
    TCHAR boxName[24];
    DWORD err;

    /* Connect to the out-going. */
    wsprintf(boxName, _T("%sTo"), box);
    MasterToExpect = new CMclMailbox(boxName);

    /* Check status. */
    err = MasterToExpect->Status();
    if (err == NO_ERROR) {
	/* Not allowed to be the creator. */
	delete MasterToExpect;
	EXP_LOG1(MSG_MB_CANTOPENCLIENT1, box);
    } else if (err != ERROR_ALREADY_EXISTS) {
	delete MasterToExpect;
	EXP_LOG2(MSG_MB_CANTOPENCLIENT2, box, ExpSyslogGetSysMsg(err));
    }

    /* Connect to the in-coming. */
    wsprintf(boxName, _T("%sFrom"), box);
    MasterFromExpect = new CMclMailbox(boxName);

    /* Check status. */
    err = MasterToExpect->Status();
    if (err == NO_ERROR) {
	/* Not allowed to be the creator. */
	delete MasterToExpect;
	EXP_LOG1(MSG_MB_CANTOPENCLIENT1, box);
    } else if (err != ERROR_ALREADY_EXISTS) {
	delete MasterToExpect;
	EXP_LOG2(MSG_MB_CANTOPENCLIENT2, box, ExpSyslogGetSysMsg(err));
    }
}
