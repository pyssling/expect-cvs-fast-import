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
 *	work by David Gravereaux <davygrvy@pobox.com> for any Win32 OS.
 *
 * ----------------------------------------------------------------------------
 * URLs:    http://expect.nist.gov/
 *	    http://expect.sf.net/
 *	    http://bmrc.berkeley.edu/people/chaffee/expectnt.html
 * ----------------------------------------------------------------------------
 * RCS: @(#) $Id: exp.h,v 1.1.4.4 2002/02/10 10:17:04 davygrvy Exp $
 * ----------------------------------------------------------------------------
 */

#include "expWinInt.h"

static HANDLE hSyslog = NULL;

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
ExpSyslog TCL_VARARGS_DEF(char *,arg1)
{
    char *fmt;
    va_list args;
    char buf[16384];
    char *strings[1];

    fmt = TCL_VARARGS_START(char *,arg1,args);
    
    if (hSyslog == NULL) {
	hSyslog = OpenEventLog(NULL, "ExpectSlaveDrv");
    }
    vsprintf(buf, fmt, args);
    va_end(args);

    strings[0] = buf;
    ReportEvent(hSyslog, EVENTLOG_ERROR_TYPE, 0, 0, NULL, 1, 0,
		strings, NULL);
}
