/* 
 * tclUnixChan.c
 *
 *	Channel driver for Expect channels.
 *      Based on UNIX File channel from TclUnixChan.c
 *
 */

#include	"tclInt.h"	/* Internal definitions for Tcl. */
#include	"tclPort.h"	/* Portability features for Tcl. */

#include "expChan.h"

/*
 * This structure describes the channel type structure for Expect-based IO:
 */

Tcl_ChannelType expChannelType = {
    "exp",				/* Type name. */
    /* Expect channels are always blocking */
    NULL,				/* Set blocking/nonblocking mode.*/
    ExpCloseProc,			/* Close proc. */
    ExpInputProc,			/* Input proc. */
    ExpOutputProc,			/* Output proc. */
    NULL,				/* Seek proc. */
    NULL,				/* Set option proc. */
    NULL,				/* Get option proc. */
    ExpWatchProc,			/* Initialize notifier. */
    ExpGetHandleProc,			/* Get OS handles out of channel. */
/* uncomment as necessary
/*  NULL,				/* Close2 proc */
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
 * ExpInputProc --
 *
 *	This procedure is invoked from the generic IO level to read
 *	input from an exp-based channel.
 *
 * Results:
 *	The number of bytes read is returned or -1 on error. An output
 *	argument contains a POSIX error code if an error occurs, or zero.
 *
 * Side effects:
 *	Reads input from the input device of the channel.
 *
 *----------------------------------------------------------------------
 */

static int
ExpInputProc(instanceData, buf, toRead, errorCodePtr)
    ClientData instanceData;		/* Exp state. */
    char *buf;				/* Where to store data read. */
    int toRead;				/* How much space is available
                                         * in the buffer? */
    int *errorCodePtr;			/* Where to store error code. */
{
    ExpState *esPtr = (ExpState *) instanceData;
    int bytesRead;			/* How many bytes were actually
                                         * read from the input device? */

    *errorCodePtr = 0;
    
    /*
     * Assume there is always enough input available. This will block
     * appropriately, and read will unblock as soon as a short read is
     * possible, if the channel is in blocking mode. If the channel is
     * nonblocking, the read will never block.
     */

    bytesRead = read(esPtr->fdin, buf, (size_t) toRead);
    if (bytesRead > -1) {
        return bytesRead;
    }
    *errorCodePtr = errno;
    return -1;
}

/*
 *----------------------------------------------------------------------
 *
 * ExpOutputProc--
 *
 *	This procedure is invoked from the generic IO level to write
 *	output to an exp channel.
 *
 * Results:
 *	The number of bytes written is returned or -1 on error. An
 *	output argument	contains a POSIX error code if an error occurred,
 *	or zero.
 *
 * Side effects:
 *	Writes output on the output device of the channel.
 *
 *----------------------------------------------------------------------
 */

static int
ExpOutputProc(instanceData, buf, toWrite, errorCodePtr)
    ClientData instanceData;		/* Exp state. */
    char *buf;				/* The data buffer. */
    int toWrite;			/* How many bytes to write? */
    int *errorCodePtr;			/* Where to store error code. */
{
    ExpState *esPtr = (ExpState *) instanceData;
    int written;

    *errorCodePtr = 0;

    written = write(esPtr->fdout, buf, (size_t) toWrite);
    if (written > -1) {
        return written;
    }
    *errorCodePtr = errno;
    return -1;
}

/*
 *----------------------------------------------------------------------
 *
 * ExpCloseProc --
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

static int
ExpCloseProc(instanceData, interp)
    ClientData instanceData;	/* Exp state. */
    Tcl_Interp *interp;		/* For error reporting - unused. */
{
    ExpState *esPtr = (ExpState *) instanceData;
    ExpState **nextPtrPtr;
    int errorCode = 0;
    ThreadSpecificData *tsdPtr = TCL_TSD_INIT(&dataKey);

    /*
      Really should check that we created one first.  Since we're sharing fds
      with Tcl, perhaps a filehandler was created with a plain tcl file - we
      wouldn't want to delete that.  Although if user really close Expect's
      user_spawn_id, it probably doesn't matter anyway.
    */

    Tcl_DeleteFileHandler(esPtr->fdin);

    Tcl_DecrRefCount(esPtr->buffer);

    /* Actually file descriptor should have been closed earlier. */
    /* So do nothing here */

    /*
     * Conceivably, the process may not yet have been waited for.  If this
     * becomes a requirement, we'll have to revisit this code.  But for now, if
     * it's just Tcl exiting, * the processes will exit on their own soon
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
    ckfree((char *) esPtr);
    return errorCode;
}

/*
 *----------------------------------------------------------------------
 *
 * ExpWatchProc --
 *
 *	Initialize the notifier to watch the fd from this channel.
 *
 * Results:
 *	None.
 *
 * Side effects:
 *	Sets up the notifier so that a future event on the channel will
 *	be seen by Tcl.
 *
 *----------------------------------------------------------------------
 */

static void
ExpWatchProc(instanceData, mask)
    ClientData instanceData;		/* The exp state. */
    int mask;				/* Events of interest; an OR-ed
                                         * combination of TCL_READABLE,
                                         * TCL_WRITABLE and TCL_EXCEPTION. */
{
    ExpState *esPtr = (ExpState *) instanceData;

    /*
     * Make sure we only register for events that are valid on this exp.
     * Note that we are passing Tcl_NotifyChannel directly to
     * Tcl_CreateExpHandler with the channel pointer as the client data.
     */

    mask &= esPtr->validMask;
    if (mask) {
	Tcl_CreateFileHandler(esPtr->fdin, mask,
		(Tcl_ExpProc *) Tcl_NotifyChannel,
		(ClientData) esPtr->channel);
    } else {
	Tcl_DeleteFileHandler(esPtr->fdin);
    }
}

/*
 *----------------------------------------------------------------------
 *
 * ExpGetHandleProc --
 *
 *	Called from Tcl_GetChannelHandle to retrieve OS handles from
 *	an exp-based channel.
 *
 * Results:
 *	Returns TCL_OK with the fd in handlePtr, or TCL_ERROR if
 *	there is no handle for the specified direction. 
 *
 * Side effects:
 *	None.
 *
 *----------------------------------------------------------------------
 */

static int
ExpGetHandleProc(instanceData, direction, handlePtr)
    ClientData instanceData;	/* The exp state. */
    int direction;		/* TCL_READABLE or TCL_WRITABLE */
    ClientData *handlePtr;	/* Where to store the handle.  */
{
    ExpState *esPtr = (ExpState *) instanceData;

    if (direction & TCL_WRITABLE) {
	*handlePtr = (ClientData) esPtr->fdin;
    }
    if (direction & TCL_READABLE) {
	*handlePtr = (ClientData) esPtr->fdin;
    } else {
	return TCL_ERROR;
    }
    return TCL_OK;
}

/* close all connections
The kernel would actually do this by default, however Tcl is going to
come along later and try to reap its exec'd processes.  If we have
inherited any via spawn -open, Tcl can hang if we don't close the
connections first.
*/

int
expSizeGet(esPtr)
    ExpState *esPtr;
{
    int len;
    Tcl_GetStringFromObj(esPtr->buffer,&len);
    return len;
}

int
expSizeZero(esPtr)
    ExpState *esPtr;
{
    int len;
    Tcl_GetStringFromObj(esPtr->buffer,&len);
    return (len == 0);
}



void
exp_close_all(interp)
Tcl_Interp *interp;
{
    ThreadSpecificData *tsdPtr = TCL_TSD_INIT(&dataKey);

    /* no need to keep things in sync (i.e., tsdPtr, count) since we could only
       be doing this if we're exiting.  Just close everything down. */

    for (esPtr = tsdPtr->firstExpPtr;esPtr;esPtr = esPtr->nextPtr) {
	exp_close(interp,esPtr->channel);
    }
}

/* wait for any of our own spawned processes */
/* we call waitpid rather than wait to avoid running into */
/* someone else's processes.  Yes, according to Ousterhout */
/* this is the best way to do it. */
/* returns the ExpState or 0 if nothing to wait on */
ExpState *
expWaitOnAny(interp) {
    ThreadSpecificData *tsdPtr = TCL_TSD_INIT(&dataKey);
    int result;
    ExpState *esPtr;

    for (esPtr = tsdPtr->firstExpPtr;esPtr;esPtr = esPtr->nextPtr) {
	if (esPtr->pid == exp_getpid) continue; /* skip ourself */
	if (esPtr->user_waited) continue;	/* one wait only! */
	if (esPtr->sys_waited) break;
      restart:
	result = waitpid(esPtr->pid,&esPtr->wait,WNOHANG);
	if (result == esPtr->pid) break;
	if (result == 0) continue;	/* busy, try next */
	if (result == -1) {
	    if (errno == EINTR) goto restart;
	    else break;
	}
    }
    return esPtr;
}

ExpState *
expWaitOnOne() {
    ThreadSpecificData *tsdPtr = TCL_TSD_INIT(&dataKey);
    int result;
    ExpState *esPtr;
    int pid;
    /* should really be recoded using the common wait code in command.c */
    int status;

    pid = wait(&status);
    for (esPtr = tsdPtr->firstExpPtr;esPtr;esPtr = esPtr->nextPtr) {
	if (esPtr->pid == pid) {
	    esPtr->sys_waited = TRUE;
	    esPtr->wait = status;
	}
    }
}

void
exp_background_channelhandlers_run_all()
{
    ThreadSpecificData *tsdPtr = TCL_TSD_INIT(&dataKey);
    int m;
    ExpState *esPtr;

    /* kick off any that already have input waiting */
    for (esPtr = tsdPtr->firstExpPtr;esPtr;esPtr = esPtr->nextPtr) {
	/* is bg_interp the best way to check if armed? */
	if (esPtr->bg_interp && !expSizeZero(esPtr)) {
	    exp_background_channelhandler((ClientData)esPtr);
	}
    }
}

ExpState *
expCreateChannel(fdin,fdout,pid)
    int fdin;
    int fdout;
    int pid:
{
    ExpState *esPtr;
    char channelName[16 + TCL_INTEGER_SPACE];
    int mask;
    Tcl_ChannelType *channelTypePtr;
    ThreadSpecificData *tsdPtr = TCL_TSD_INIT(&dataKey);

    channelTypePtr = &expChannelType;

    esPtr = (ExpState *) ckalloc((unsigned) sizeof(ExpState));

    esPtr->nextPtr = tsdPtr->firstFilePtr;
    tsdPtr->firstFilePtr = esPtr;

    sprintf(esPtr->name,"exp%d",fdin);

    /*
     * For now, stupidly assume this.  We we will likely have to revisit this
     * later to prevent people from doing stupid things.
     */
    mask = TCL_READABLE | TCL_WRITABLE;

    /* not sure about this - what about adopted channels */
    esPtr->validMask = mask | TCL_EXCEPTION;
    esPtr->fdin = fdin;
    esPtr->fdout = fdout;
    esPtr->channel = Tcl_CreateChannel(channelTypePtr, esPtr->name,
	    (ClientData) esPtr, mask);
    Tcl_RegisterChannel(interp,esPtr->channel);
    esPtr->registered = 1;

    esPtr->pid = pid;
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
    esPtr->user_waited = FALSE;
    esPtr->sys_waited = FALSE;
    esPtr->bg_interp = 0;
    esPtr->bg_status = unarmed;
    esPtr->bg_ecount = 0;

    tsdPtr->channelCount++;

    return esPtr;
}

void
expChannelInit() {
    ThreadSpecificData *tsdPtr = TCL_TSD_INIT(&dataKey);

    tsdPtr->channelCount = 0;
}
