/*
 * expWinChan.c --
 *
 *	Implements the exp_spawn channel id.  This wraps a normal
 *	file channel in another channel so we can close the file
 *	channel normally but still have another id to wait on.
 *	The file channel is not exposed in any interps.
 *
 * Copyright (c) 1997 by Mitel Corporation
 *
 * See the file "license.terms" for information on usage and redistribution
 * of this file, and for a DISCLAIMER OF ALL WARRANTIES.
 *
 */

#include "exp_port.h"
#include "tclInt.h"
#include "tclPort.h"

#define BUILD_expect

#include "exp_command.h"

#ifdef __WIN32__
#include "expWin.h"
#endif

static Tcl_DriverCloseProc ExpSpawnClose;
static Tcl_DriverInputProc ExpSpawnInput;
static Tcl_DriverOutputProc ExpSpawnOutput;
/*static Tcl_DriverSeekProc ExpSpawnSeek;*/
static Tcl_DriverSetOptionProc ExpSpawnSetOption;
static Tcl_DriverGetOptionProc ExpSpawnGetOption;
static Tcl_DriverWatchProc ExpSpawnWatch;
static Tcl_DriverGetHandleProc ExpSpawnGetHandle;
static Tcl_DriverBlockModeProc ExpSpawnBlock;
/*static Tcl_DriverFlushProc ExpSpawnFlush;*/
/*static Tcl_DriverHandlerProc ExpSpawnHandler;*/

static Tcl_ChannelType ExpSpawnChannelType = {
    "exp_spawn",
    TCL_CHANNEL_VERSION_2,
    ExpSpawnClose,
    ExpSpawnInput,
    ExpSpawnOutput,
    NULL,         		/* no seek! */
    ExpSpawnSetOption,
    ExpSpawnGetOption,
    ExpSpawnWatch,
    ExpSpawnGetHandle,
    NULL,			/* no close2 */
    ExpSpawnBlock,
    NULL,			/* no flush */
    NULL			/* no handler */
};

/*
 *----------------------------------------------------------------------
 *
 * ExpCreateSpawnChannel --
 *
 *	Create an expect spawn identifier
 *
 * Results:
 *	A Tcl channel
 *
 * Side Effects:
 *	Allocates and registers a channel
 *
 *----------------------------------------------------------------------
 */

Tcl_Channel
ExpCreateSpawnChannel(interp, chan)
    Tcl_Interp *interp;
    Tcl_Channel chan;
{
    Tcl_Channel chan2;
    ExpSpawnState *ssPtr;

    ssPtr = (ExpSpawnState *) ckalloc(sizeof(ExpSpawnState));
    ssPtr->channelPtr = chan;
    ssPtr->toWrite = 0;

    chan2 = Tcl_StackChannel(interp, &ExpSpawnChannelType,
	    (ClientData) ssPtr, TCL_READABLE|TCL_WRITABLE, chan);

    /*
     * Setup the expect channel to always flush immediately
     */

    Tcl_SetChannelOption(interp, chan2, "-buffering",  "none");
    Tcl_SetChannelOption(interp, chan2, "-blocking",   "0");
    Tcl_SetChannelOption(interp, chan2, "-translation","binary");

    return chan2;
}

/*
 *----------------------------------------------------------------------
 *
 * ExpSpawnClose --
 *
 *	Generic routine to close the expect spawn channel and child.
 *
 * Results:
 *      0 if successful or a POSIX errorcode with
 *      interp updated.
 *    
 * Side Effects:
 *	Channel is deleted.
 *
 *----------------------------------------------------------------------
 */

static int
ExpSpawnClose(instanceData, interp)
    ClientData instanceData;
    Tcl_Interp *interp;
{
    ckfree((char *)(ExpSpawnState *)instanceData);
    return TCL_OK;
}

/*
 *----------------------------------------------------------------------
 *
 * ExpSpawnInput --
 *
 *	Generic read routine for expect console
 *
 * Returns:
 *	Amount read or -1 with errorcode in errorPtr.
 *    
 * Side Effects:
 *	Buffer is updated. 
 *
 *----------------------------------------------------------------------
 */

static int
ExpSpawnInput(instanceData, bufPtr, bufSize, errorPtr)
    ClientData instanceData;
    char *bufPtr;		/* (in) Ptr to buffer */
    int bufSize;		/* (in) sizeof buffer */
    int *errorPtr;		/* (out) error code */
{
    Tcl_Channel channelPtr = ((ExpSpawnState *)instanceData)->channelPtr;

    return (Tcl_GetChannelType(channelPtr)->inputProc)
	(Tcl_GetChannelInstanceData(channelPtr), bufPtr, bufSize, errorPtr);
}

/*
 *----------------------------------------------------------------------
 *
 * ExpSpawnOutput --
 *
 *	Write routine for expect console
 *
 * Results:
 *	Amount written or -1 with errorcode in errorPtr
 *    
 * Side Effects:
 *	None. 
 *
 *----------------------------------------------------------------------
 */

static int
ExpSpawnOutput(instanceData, bufPtr, toWrite, errorPtr)
    ClientData instanceData;
    char *bufPtr;		/* (in) Ptr to buffer */
    int toWrite;		/* (in) amount to write */
    int *errorPtr;		/* (out) error code */
{
    return ExpPlatformSpawnOutput(instanceData, bufPtr, toWrite, errorPtr);
}

/*
 *----------------------------------------------------------------------
 *
 * ExpSpawnSetOption --
 *
 *	Set the value of an ExpSpawn channel option
 *
 * Results:
 *	TCL_OK and dsPtr updated with the value or TCL_ERROR.
 *
 * Side Effects
 *	None.
 *
 *----------------------------------------------------------------------
 */

static int
ExpSpawnSetOption(instanceData, interp, nameStr, valStr)
    ClientData instanceData;
    Tcl_Interp *interp;
    char *nameStr;		/* (in) Name of option */
    char *valStr;		/* (in) New value of option */
{
    Tcl_Channel channelPtr = ((ExpSpawnState *)instanceData)->channelPtr;
    Tcl_DriverSetOptionProc *setOpt;

    setOpt = Tcl_GetChannelType(channelPtr)->setOptionProc;

    if (setOpt) {
	return (setOpt)(Tcl_GetChannelInstanceData(channelPtr), interp,
		nameStr, valStr);
    } else {
	return Tcl_BadChannelOption(interp, nameStr, "");
    }
}

/*
 *----------------------------------------------------------------------
 *
 * ExpSpawnGetOption --
 *
 *	Queries ExpSpawn channel for the current value of
 *      the given option.
 *
 * Results:
 *	TCL_OK and dsPtr updated with the value or TCL_ERROR.
 *
 * Side Effects
 *	None.
 *
 *----------------------------------------------------------------------
 */

static int
ExpSpawnGetOption(instanceData, interp, nameStr, dsPtr)
    ClientData instanceData;
    Tcl_Interp *interp;
    char *nameStr;		/* (in) Name of option to retrieve */		
    Tcl_DString *dsPtr;		/* (in) String to place value */
{
    Tcl_Channel channelPtr = ((ExpSpawnState *)instanceData)->channelPtr;
    Tcl_DriverGetOptionProc *getOpt;

    getOpt = Tcl_GetChannelType(channelPtr)->getOptionProc;
    if (getOpt) {
	return (getOpt)(Tcl_GetChannelInstanceData(channelPtr), interp,
		nameStr, dsPtr);
    } else if (nameStr != NULL) {
	return Tcl_BadChannelOption(interp, nameStr, "");
    } else {
	return TCL_OK;
    }
}
/*
 *----------------------------------------------------------------------
 *
 * ExpSpawnWatch --
 *
 *	Sets up event handling on a expect console Tcl_Channel using
 *	the underlying channel type.
 *
 * Results:
 *	Nothing
 *
 * Side Effects
 *	None.
 *
 *----------------------------------------------------------------------
 */

void
ExpSpawnWatch(instanceData, mask)
    ClientData instanceData;
    int mask;
{
    Tcl_Channel channelPtr = ((ExpSpawnState *)instanceData)->channelPtr;

    (Tcl_GetChannelType(channelPtr)->watchProc)
	(Tcl_GetChannelInstanceData(channelPtr), mask);
    return;
}

/*
 *----------------------------------------------------------------------
 *
 * ExpSpawnGetHandle --
 *
 *	Get the Tcl_File for the appropriate direction in from the
 *	Tcl_Channel.
 *
 * Results:
 *	NULL because ExpSpawn ids are handled through other channel
 *	types.
 *
 * Side Effects
 *	None.
 *
 *----------------------------------------------------------------------
 */

int
ExpSpawnGetHandle(instanceData, direction, handlePtr)
    ClientData instanceData;
    int direction;
    ClientData *handlePtr;
{
    Tcl_Channel channelPtr = ((ExpSpawnState *)instanceData)->channelPtr;

    return Tcl_GetChannelHandle(channelPtr, direction, handlePtr);
}

/*
 *----------------------------------------------------------------------
 *
 * ExpSpawnBlock --
 *
 *	Generic routine to set I/O to blocking or non-blocking.
 *
 * Results:
 *	TCL_OK or TCL_ERROR.
 *    
 * Side Effects:
 *	None.
 *
 *----------------------------------------------------------------------
 */

static int
ExpSpawnBlock(instanceData, mode)
    ClientData instanceData;
    int mode;			/* (in) Block or not */
{
    Tcl_Channel channelPtr = ((ExpSpawnState *)instanceData)->channelPtr;

    return (Tcl_GetChannelType(channelPtr)->blockModeProc)
	(Tcl_GetChannelInstanceData(channelPtr), mode);
}
