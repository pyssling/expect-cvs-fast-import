/*
 * expSpawnChan.c --
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

#include "tclInt.h"
#include "tclPort.h"
#include "expect_tcl.h"
#include "exp_command.h"
#include "expWin.h"

static int	ExpSpawnBlockProc _ANSI_ARGS_((ClientData instanceData,
		    int mode));
static int	ExpSpawnCloseProc _ANSI_ARGS_((ClientData instanceData,
		    Tcl_Interp *interp));
static int	ExpSpawnInputProc _ANSI_ARGS_((ClientData instanceData,
		    char *bufPtr, int bufSize, int *errorPtr));
static int	ExpSpawnOutputProc _ANSI_ARGS_((ClientData instanceData,
		    char *bufPtr, int toWrite, int *errorPtr));
static int	ExpSpawnGetOptionProc _ANSI_ARGS_((ClientData instanceData,
		    Tcl_Interp *interp, char *nameStr, Tcl_DString *dsPtr));
static int	ExpSpawnSetOptionProc _ANSI_ARGS_((ClientData instanceData,
		    Tcl_Interp *interp, char *nameStr, char *val));
static void	ExpSpawnWatchProc _ANSI_ARGS_((ClientData instanceData,
		    int mask));
static int	ExpSpawnGetHandleProc _ANSI_ARGS_((ClientData instanceData,
		    int direction, ClientData *handlePtr));

/*
 * This structure describes the channel type structure for Expect-based IO:
 */

Tcl_ChannelType expSpawnChanType = {
    "exp_spawn",			/* Type name. */
    TCL_CHANNEL_VERSION_1,		/* Version of the channel type. */
    ExpSpawnCloseProc,			/* Close proc. */
    ExpSpawnInputProc,			/* Input proc. */
    ExpSpawnOutputProc,			/* Output proc. */
    NULL,				/* Seek proc. */
    ExpSpawnSetOptionProc,		/* Set option proc. */
    ExpSpawnGetOptionProc,		/* Get option proc. */
    ExpSpawnWatchProc,			/* Initialize notifier. */
    ExpSpawnGetHandleProc,		/* Get OS handles out of channel. */
    NULL,				/* Close2 proc */
    ExpSpawnBlockProc			/* Set blocking/nonblocking mode.
					 * Expect channels are always blocking */
};

typedef struct ThreadSpecificData {
    /*
     * List of all exp channels currently open.  This is per thread and is
     * used to match up fd's to channels, which rarely occurs.
     */
    
    ExpState *firstExpPtr;
    int channelCount;	 /* this is process-wide as it is used to
			     give user some hint as to why a spawn has failed
			     by looking at process-wide resource usage */
} ThreadSpecificData;

static Tcl_ThreadDataKey dataKey;


/*
 *----------------------------------------------------------------------
 *
 * expChannelInit --
 *
 *	Inits the TSD structure for the calling thread context.
 *
 * Results:
 *	nothing
 *
 * Side Effects:
 *	A datakey and associated TSD structure now exists for the
 *	calling thread context.
 *
 *----------------------------------------------------------------------
 */
void
expChannelInit() {
    ThreadSpecificData *tsdPtr = TCL_TSD_INIT(&dataKey);
    tsdPtr->channelCount = 0;
}

/*
 *----------------------------------------------------------------------
 *
 * expChannelCountGet --
 *
 *	.
 *
 * Results:
 *	Count of how many spawn channels are open.
 *
 * Side Effects:
 *	None.
 *
 *----------------------------------------------------------------------
 */
int
expChannelCountGet()
{
    ThreadSpecificData *tsdPtr = TCL_TSD_INIT(&dataKey);
    return tsdPtr->channelCount;
}

/*
 *----------------------------------------------------------------------
 *
 * expSizeGet --
 *
 *	Get how much data is currently in the channel's buffer.
 *
 * Results:
 *	bytes in use.
 *
 * Side Effects:
 *	None.
 *
 *----------------------------------------------------------------------
 */
int
expSizeGet(esPtr)
    ExpState *esPtr;
{
    int len;
    Tcl_GetStringFromObj(esPtr->buffer,&len);
    return len;
}

/*
 *----------------------------------------------------------------------
 *
 * expSizeZero --
 *
 *	Asks if the buffer is empty.
 *
 * Results:
 *	Boolean.
 *
 * Side Effects:
 *	None.
 *
 *----------------------------------------------------------------------
 */
int
expSizeZero(esPtr)
    ExpState *esPtr;
{
    int len;
    Tcl_GetStringFromObj(esPtr->buffer,&len);
    return (len == 0);
}

/*
 *----------------------------------------------------------------------
 *
 * expStateFree --
 *
 *	Asks if the buffer is empty.
 *
 * Results:
 *	Boolean.
 *
 * Side Effects:
 *	None.
 *
 *----------------------------------------------------------------------
 */

void
expStateFree(esPtr)
    ExpState *esPtr;
{
    if (esPtr->fdBusy) {
//    close(esPtr->fdin);    /* BUG: not OS neutral */
//      expPlatformStateFree(esPtr);
    }

    esPtr->valid = FALSE;
    
    if (!esPtr->keepForever) {
	ckfree((char *)esPtr);
    }
}

/*
 *----------------------------------------------------------------------
 *
 * exp_close_all --
 *
 *	close all connections
 * 
 *	The kernel would actually do this by default, however Tcl is going to
 *	come along later and try to reap its exec'd processes.  If we have
 *	inherited any via spawn -open, Tcl can hang if we don't close the
 *	connections first.
 *
 * Results:
 *	A Tcl channel
 *
 * Side Effects:
 *	Allocates and registers a channel
 *
 *----------------------------------------------------------------------
 */

void
exp_close_all(interp)
    Tcl_Interp *interp;
{
    ThreadSpecificData *tsdPtr = TCL_TSD_INIT(&dataKey);
    ExpState *esPtr;

    /* no need to keep things in sync (i.e., tsdPtr, count) since we could only
       be doing this if we're exiting.  Just close everything down. */

    for (esPtr = tsdPtr->firstExpPtr; esPtr; esPtr = esPtr->nextPtr) {
	exp_close(interp, esPtr);
    }
}

/*
 *----------------------------------------------------------------------
 *
 * expWaitOnAny --
 *
 *	Wait for any of our own spawned processes we call waitpid rather than
 *	wait to avoid running into someone else's processes.  Yes, according
 *	to Ousterhout this is the best way to do it.
 *
 * Results:
 *	returns the ExpState or 0 if nothing to wait on
 *
 * Side Effects:
 *	
 *
 *----------------------------------------------------------------------
 */

ExpState *
expWaitOnAny()
{
    ThreadSpecificData *tsdPtr = TCL_TSD_INIT(&dataKey);
    int result;
    ExpState *esPtr;

    for (esPtr = tsdPtr->firstExpPtr; esPtr; esPtr = esPtr->nextPtr) {
	if (esPtr->pid == exp_getpid) continue; /* skip ourself */
	if (esPtr->user_waited) continue;	/* one wait only! */
	if (esPtr->sys_waited) break;
      restart:
	result = waitpid(esPtr->pid, &esPtr->wait, WNOHANG);  /* BUG: not OS neutral */
	if (result == esPtr->pid) break;
	if (result == 0) continue;	/* busy, try next */
	if (result == -1) {
	    if (errno == EINTR) goto restart;
	    else break;
	}
    }
    return esPtr;
}

/*
 *----------------------------------------------------------------------
 *
 * expWaitOnOne --
 *
 *	Add comment here.
 *
 * Results:
 *	
 *
 * Side Effects:
 *	
 *
 *----------------------------------------------------------------------
 */

ExpState *
expWaitOnOne()
{
    ThreadSpecificData *tsdPtr = TCL_TSD_INIT(&dataKey);
    ExpState *esPtr;
    int pid;
    /* should really be recoded using the common wait code in command.c */
    WAIT_STATUS_TYPE status;

    pid = wait(&status);   /* BUG: not OS neutral */
    for (esPtr = tsdPtr->firstExpPtr; esPtr; esPtr = esPtr->nextPtr) {
	if (esPtr->pid == pid) {
	    esPtr->sys_waited = TRUE;
	    esPtr->wait = status;
	    return esPtr;
	}
    }
    return 0;
}

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

void
exp_background_channelhandlers_run_all()
{
    ThreadSpecificData *tsdPtr = TCL_TSD_INIT(&dataKey);
    ExpState *esPtr;

    /* kick off any that already have input waiting */
    for (esPtr = tsdPtr->firstExpPtr; esPtr; esPtr = esPtr->nextPtr) {
	/* is bg_interp the best way to check if armed? */
	if (esPtr->bg_interp && !expSizeZero(esPtr)) {
	    exp_background_channelhandler((ClientData)esPtr,0);
	}
    }
}

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
    ExpSpawnState *ssPtr;
    ExpState *esPtr;
    ThreadSpecificData *tsdPtr = TCL_TSD_INIT(&dataKey);

    esPtr = (ExpState *) ckalloc((unsigned) sizeof(ExpState));
    esPtr->nextPtr = tsdPtr->firstExpPtr;
    tsdPtr->firstExpPtr = esPtr;

    //ssPtr->channelPtr = chan;
    //ssPtr->toWrite = 0;

    esPtr->fdBusy = FALSE;


    /*
     * Setup the expect channel to always flush immediately
     */

    sprintf(esPtr->name, "exp_spawn%d", tsdPtr->channelCount++);

    chan = Tcl_CreateChannel(&expSpawnChanType, esPtr->name,
			     (ClientData) esPtr, TCL_READABLE|TCL_WRITABLE);
    Tcl_RegisterChannel(interp, chan);
    Tcl_SetChannelOption(interp, chan, "-blocking", "0");
    Tcl_SetChannelOption(interp, chan, "-buffering", "none");
    Tcl_SetChannelOption(interp, chan, "-translation","binary");

    esPtr->msize = 0;

    /* initialize a dummy buffer */
    esPtr->buffer = Tcl_NewStringObj("",0);
    Tcl_IncrRefCount(esPtr->buffer);
    esPtr->umsize = exp_default_match_max;

    /* this will reallocate object with an appropriate sized buffer */
    expAdjust(esPtr);

    esPtr->printed = 0;
    esPtr->echoed = 0;
    esPtr->rm_nulls = exp_default_rm_nulls;
    esPtr->parity = exp_default_parity;
    esPtr->key = expect_key++;
    esPtr->force_read = FALSE;
    esPtr->fg_armed = FALSE;
    esPtr->channel_orig = 0;
    esPtr->fd_slave = EXP_NOFD;
#ifdef HAVE_PTYTRAP
    esPtr->slave_name = 0;
#endif /* HAVE_PTYTRAP */
    esPtr->open = TRUE;
    esPtr->notified = FALSE;
    esPtr->user_waited = FALSE;
    esPtr->sys_waited = FALSE;
    esPtr->bg_interp = 0;
    esPtr->bg_status = unarmed;
    esPtr->bg_ecount = 0;
    esPtr->freeWhenBgHandlerUnblocked = FALSE;
    esPtr->keepForever = FALSE;
    esPtr->valid = TRUE;

    return chan;
}

/*
 *----------------------------------------------------------------------
 *
 * ExpSpawnBlockProc --
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

int
ExpSpawnBlockProc(instanceData, mode)
    ClientData instanceData;
    int mode;			/* (in) Block or not */
{
    return 0;   /* BUG: fix me! */
}

/*
 *----------------------------------------------------------------------
 *
 * ExpSpawnInputProc --
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

int
ExpSpawnInputProc(instanceData, buf, toRead, errorCodePtr)
    ClientData instanceData;
    char *buf;		/* (in) Ptr to buffer */
    int toRead;		/* (in) sizeof buffer */
    int *errorCodePtr;		/* (out) error code */
{
    ExpState *esPtr = (ExpState *) instanceData;
    return ExpPlatformSpawnInput(esPtr, buf, toRead, errorCodePtr);
}


/*
 *----------------------------------------------------------------------
 *
 * ExpSpawnOutputProc --
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

int
ExpSpawnOutputProc(instanceData, bufPtr, toWrite, errorPtr)
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
 * ExpSpawnCloseProc --
 *
 *	This procedure is called from the generic IO level to perform
 *	channel-type-specific cleanup when an exp-based channel is closed.
 *
 * Results:
 *	0 if successful, errno if failed.
 *
 * Side effects:
 *	Closes the device of the channel.
 *
 *----------------------------------------------------------------------
 */

int
ExpSpawnCloseProc(instanceData, interp)
    ClientData instanceData;
    Tcl_Interp *interp;
{
    ExpState *esPtr = (ExpState *) instanceData;
    ExpState **nextPtrPtr;
    ThreadSpecificData *tsdPtr = TCL_TSD_INIT(&dataKey);

    esPtr->registered = FALSE;

#if 0
    /*
      Really should check that we created one first.  Since we're sharing fds
      with Tcl, perhaps a filehandler was created with a plain tcl file - we
      wouldn't want to delete that.  Although if user really close Expect's
      user_spawn_id, it probably doesn't matter anyway.
    */

    Tcl_DeleteFileHandler(esPtr->fdin);
#endif /*0*/

    Tcl_DecrRefCount(esPtr->buffer);

    /* Actually file descriptor should have been closed earlier. */
    /* So do nothing here */

    /*
     * Conceivably, the process may not yet have been waited for.  If this
     * becomes a requirement, we'll have to revisit this code.  But for now, if
     * it's just Tcl exiting, the processes will exit on their own soon
     * anyway.
     */

    for (nextPtrPtr = &(tsdPtr->firstExpPtr); (*nextPtrPtr) != NULL;
	 nextPtrPtr = &((*nextPtrPtr)->nextPtr)) {
	if ((*nextPtrPtr) == esPtr) {
	    (*nextPtrPtr) = esPtr->nextPtr;
	    break;
	}
    }
    tsdPtr->channelCount--;

    if (esPtr->bg_status == blocked ||
	    esPtr->bg_status == disarm_req_while_blocked) {
	esPtr->freeWhenBgHandlerUnblocked = 1;
	/*
	 * If we're in the middle of a bg event handler, then the event
	 * handler will have to take care of freeing esPtr.
	 */
    } else {
	expStateFree(esPtr);
    }
    return 0;
}


/*
 *----------------------------------------------------------------------
 *
 * ExpSpawnSetOptionProc --
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

int
ExpSpawnSetOptionProc(instanceData, interp, nameStr, valStr)
    ClientData instanceData;
    Tcl_Interp *interp;
    char *nameStr;		/* (in) Name of option */
    char *valStr;		/* (in) New value of option */
{
    Tcl_Channel channelPtr = ((ExpSpawnState *)instanceData)->channelPtr;

    return (Tcl_GetChannelType(channelPtr)->setOptionProc)
	(Tcl_GetChannelInstanceData(channelPtr), interp, nameStr, valStr);
}

/*
 *----------------------------------------------------------------------
 *
 * ExpSpawnGetOptionProc --
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

int
ExpSpawnGetOptionProc(instanceData, interp, nameStr, dsPtr)
    ClientData instanceData;
    Tcl_Interp *interp;
    char *nameStr;		/* (in) Name of option to retrieve */		
    Tcl_DString *dsPtr;		/* (in) String to place value */
{
    Tcl_Channel channelPtr = ((ExpSpawnState *)instanceData)->channelPtr;

    return (Tcl_GetChannelType(channelPtr)->getOptionProc)
	(Tcl_GetChannelInstanceData(channelPtr), interp, nameStr, dsPtr);
}

/*
 *----------------------------------------------------------------------
 *
 * ExpSpawnGetHandleProc --
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
ExpSpawnGetHandleProc(instanceData, direction, handlePtr)
    ClientData instanceData;
    int direction;
    ClientData *handlePtr;
{
    Tcl_Channel channelPtr = ((ExpSpawnState *)instanceData)->channelPtr;

    return (Tcl_GetChannelType(channelPtr)->getHandleProc)
	(Tcl_GetChannelInstanceData(channelPtr), direction, handlePtr);
}

/*
 *----------------------------------------------------------------------
 *
 * ExpSpawnWatchProc --
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
ExpSpawnWatchProc(instanceData, mask)
    ClientData instanceData;
    int mask;
{
    Tcl_Channel channelPtr = ((ExpSpawnState *)instanceData)->channelPtr;

    (Tcl_GetChannelType(channelPtr)->watchProc)
	(Tcl_GetChannelInstanceData(channelPtr), mask);
    return;
}
