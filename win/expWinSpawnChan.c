/* ----------------------------------------------------------------------------
 * expWinSpawnChan.c --
 *
 *	Implements the Windows specific portion of the exp_spawn
 *	channel id.
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
 * RCS: @(#) $Id: expWinSpawnChan.c,v 1.1.2.1.2.3 2002/02/10 12:03:30 davygrvy Exp $
 * ----------------------------------------------------------------------------
 */

#include "expWinInt.h"

/*
 *----------------------------------------------------------------------
 *
 * ExpPlatformSpawnOutput --
 *
 *	Write routine for exp_spawn channel
 *
 * Results:
 *	Amount written or -1 with errorcode in errorPtr
 *    
 * Side Effects:
 *	None. 
 *
 *----------------------------------------------------------------------
 */

int
ExpPlatformSpawnOutput(instanceData, bufPtr, toWrite, errorPtr)
    ClientData instanceData;
    CONST char *bufPtr;		/* (in) Ptr to buffer */
    int toWrite;		/* (in) amount to write */
    int *errorPtr;		/* (out) error code */
{
    ExpSpawnState *ssPtr = (ExpSpawnState *) instanceData;
    Tcl_Channel channelPtr = ssPtr->channelPtr;
    unsigned char lenbuf[5];
    int n;

    if (ssPtr->toWrite == 0) {
	lenbuf[0] = EXP_SLAVE_WRITE;
	lenbuf[1] = toWrite & 0xff;
	lenbuf[2] = (toWrite & 0xff00) >> 8;
	lenbuf[3] = (toWrite & 0xff0000) >> 16;
	lenbuf[4] = (toWrite & 0xff000000) >> 24;

	n = Tcl_WriteRaw(channelPtr, lenbuf, 5);
	if (n < 0) {
	    return n;
	}
	if (n != 5) {
	    return 0;
	}
	ssPtr->toWrite = toWrite;
    }

    n = Tcl_WriteRaw(channelPtr, bufPtr, toWrite);
    if (n > 0) {
	ssPtr->toWrite -= n;
    }
    return n;
}
