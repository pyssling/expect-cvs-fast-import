/*
 * expWinSpawnChan.c --
 *
 *	Implements the Windows specific portion of the exp
 *	channel type.
 *
 * Copyright (c) 1997 by Mitel Corporation
 *
 * See the file "license.terms" for information on usage and redistribution
 * of this file, and for a DISCLAIMER OF ALL WARRANTIES.
 *
 */

#include "expInt.h"
#include "expPort.h"

#define PROTO_SLAVEWRITE(buf,len) \
	{ \
	    buf[0] = EXP_SLAVE_WRITE; \
	    buf[1] = len & 0xff; \
	    buf[2] = (len & 0xff00) >> 8; \
	    buf[3] = (len & 0xff0000) >> 16; \
	    buf[4] = (len & 0xff000000) >> 24; \
	}

#define PROTO_SLAVEWRITE_LEN	5


/*
 *----------------------------------------------------------------------
 *
 * ExpPlatformSpawnOutput --
 *
 *	Windows specific write routine for the exp channel.
 *
 * Results:
 *	Amount written or -1 with errorcode in errorPtr.
 *    
 * Side Effects:
 *	None. 
 *
 *----------------------------------------------------------------------
 */

int
ExpPlatformSpawnOutput(instanceData, bufPtr, toWrite, errorPtr)
    ClientData instanceData;
    char *bufPtr;		/* (in) Ptr to buffer */
    int toWrite;		/* (in) amount to write */
    int *errorPtr;		/* (out) error code */
{
    ExpState *esPtr = (ExpState *) instanceData;
    int n = 0;

    if (expSizeZero(esPtr)) {
	unsigned char proto[PROTO_SLAVEWRITE_LEN];

	/* protocol header */
	PROTO_SLAVEWRITE(proto, expSizeGet(esPtr));

	n = (Tcl_GetChannelType(esPtr->channel)->outputProc)
		(Tcl_GetChannelInstanceData(esPtr->channel), proto,
		PROTO_SLAVEWRITE_LEN, errorPtr);

	if (n < 0) {
	    return n;
	}
	if (n != PROTO_SLAVEWRITE_LEN) {
	    return 0;
	}
	//ssPtr->toWrite = toWrite;
    }

    n = (Tcl_GetChannelType(esPtr->channel)->outputProc)
		(Tcl_GetChannelInstanceData(esPtr->channel), bufPtr, toWrite,
		errorPtr);

    //if (n > 0) {
//	ssPtr->toWrite -= n;
    //}

    return n;
}

/*
 *----------------------------------------------------------------------
 *
 * ExpPlatformSpawnInput --
 *
 *	Read routine for exp channel
 *
 * Results:
 *	Amount read or -1 with errorcode in errorPtr
 *    
 * Side Effects:
 *	None. 
 *
 *----------------------------------------------------------------------
 */

int
ExpPlatformSpawnInput(instanceData, bufPtr, toRead, errorPtr)
    ClientData instanceData;
    char *bufPtr;	    /* (in) Ptr to buffer */
    int toRead;		    /* (in) amount to read */
    int *errorPtr;	    /* (out) error code */
{
    return 0;  /* TODO: fix me!  Make me work. */
}
