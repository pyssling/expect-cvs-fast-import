/* exp_event.c - event interface for Expect

Written by: Don Libes, NIST, 2/6/90

I hereby place this software in the public domain.  However, the author and
NIST would appreciate credit if this program or parts of it are used.

*/

/* Notes:
I'm only a little worried because Tk does not check for errno == EBADF
after calling select.  I imagine that if the user passes in a bad file
descriptor, we'll never get called back, and thus, we'll hang forever
- it would be better to at least issue a diagnostic to the user.

Another possible problem: Tk does not do file callbacks round-robin.

Another possible problem: Calling Create/DeleteChannelHandler
before/after every Tcl_Eval... in expect/interact could be very
expensive.

*/


#include "expect_cf.h"
#include <stdio.h>
#include <errno.h>
#include <sys/types.h>

#ifdef HAVE_SYS_WAIT_H
#include <sys/wait.h>
#endif

#ifdef HAVE_PTYTRAP
#  include <sys/ptyio.h>
#endif

#include "tcl.h"
#include "exp_prog.h"
#include "exp_command.h"	/* for ExpState defs */
#include "exp_event.h"

typedef struct ThreadSpecificData {
    /*
     * List of all exp channels currently open.  This is per thread and is
     * used to match up fd's to channels, which rarely occurs.
     */
    ExpState *ready_esPtr;
    int ready_mask;
    int rr;		/* round robin ptr */
} ThreadSpecificData;

static Tcl_ThreadDataKey dataKey;

static int default_mask = TCL_READABLE | TCL_EXCEPTION;

void
exp_event_disarm_bg(esPtr)
ExpState *esPtr;
{
    Tcl_DeleteChannelHandler(esPtr->channel,exp_background_channelhandler),(ClientData)esPtr);
}

static void
exp_arm_background_channelhandler_force(esPtr)
ExpState *esPtr;
{
    Tcl_CreateChannelHandler(esPtr->channel,
	    TCL_READABLE|TCL_EXCEPTION,
	    exp_background_channelhandler,
	    (ClientData)esPtr);

    esPtr->bg_status = armed;
}

void
exp_arm_background_channelhandler(esPtr)
ExpState *esPtr;
{
    switch (esPtr->bg_status) {
	case unarmed:
	    exp_arm_background_channelhandler_force(esPtr);
	    break;
	case disarm_req_while_blocked:
	    esPtr->bg_status = blocked;	/* forget request */
	    break;
	case armed:
	case blocked:
	    /* do nothing */
	    break;
    }
}

void
exp_disarm_background_channelhandler(esPtr)
ExpState *esPtr;
{
    switch (esPtr->bg_status) {
	case blocked:
	    esPtr->bg_status = disarm_req_while_blocked;
	    break;
	case armed:
	    esPtr->bg_status = unarmed;
	    exp_event_disarm_bg(esPtr);
	    break;
	case disarm_req_while_blocked:
	case unarmed:
	    /* do nothing */
	    break;
    }
}

/* ignore block status and forcibly disarm handler - called from exp_close. */
/* After exp_close returns, we will not have an opportunity to disarm */
/* because the fd will be invalid, so we force it here. */
void
exp_disarm_background_channelhandler_force(esPtr)
ExpState *esPtr;
{
    switch (esPtr->bg_status) {
	case blocked:
	case disarm_req_while_blocked:
	case armed:
	    esPtr->bg_status = unarmed;
	    exp_event_disarm_bg(esPtr);
	    break;
	case unarmed:
	    /* do nothing */
	    break;
    }
}

/* this can only be called at the end of the bg handler in which */
/* case we know the status is some kind of "blocked" */
void
exp_unblock_background_channelhandler(esPtr)
    ExpState *esPtr;
{
    switch (esPtr->bg_status) {
	case blocked:
	    exp_arm_background_channelhandler_force(esPtr);
	    break;
	case disarm_req_while_blocked:
	    exp_disarm_background_channelhandler_force(esPtr);
	    break;
    }
}

/* this can only be called at the beginning of the bg handler in which */
/* case we know the status must be "armed" */
void
exp_block_background_channelhandler(esPtr)
ExpState *esPtr;
{
    esPtr->bg_status = blocked;
    exp_event_disarm_bg(esPtr);
}


/*ARGSUSED*/
static void
exp_timehandler(clientData)
ClientData clientData;
{
    *(int *)clientData = TRUE;	
}

static void exp_channelhandler(clientData,mask)
ClientData clientData;
int mask;
{
    ThreadSpecificData *tsdPtr = TCL_TSD_INIT(&dataKey);

    /* if input appears, record the fd on which it appeared */

    tsdPtr->ready_esPtr = (ExpState *)clientData;
    tsdPtr->ready_mask = mask;
    exp_event_disarm_fg(tsdPtr->ready_esptr);
}

void
exp_event_disarm_fg(esPtr)
{
    Tcl_DeleteChannelHandler(esPtr->channel,exp_channelhandler,(ClientData)esPtr);

    /* remember that ChannelHandler has been disabled so that */
    /* it can be turned on for fg expect's as well as bg */
    esPtr->fg_armed = FALSE;
}

/* returns status, one of EOF, TIMEOUT, ERROR or DATA */
/* can now return RECONFIGURE, too */
/*ARGSUSED*/
int exp_get_next_event(interp,esPtrs,n,esPtrOut,timeout,key)
Tcl_Interp *interp;
ExpState *(esPtrs[]);
int n;			/* # of esPtrs */
ExpState **esPtrOut;	/* 1st ready esPtr, not set if none */
int timeout;		/* seconds */
int key;
{
    ThreadSpecificData *tsdPtr = TCL_TSD_INIT(&dataKey);

    ExpState *esPtr;
    int i;	/* index into in-array */
#ifdef HAVE_PTYTRAP
    struct request_info ioctl_info;
#endif

    int old_configure_count = exp_configure_count;

    int timer_created = FALSE;
    int timer_fired = FALSE;
    Tcl_TimerToken timetoken;/* handle to Tcl timehandler descriptor */

    for (;;) {
	/* if anything has been touched by someone else, report that */
	/* an event has been received */

	for (i=0;i<n;i++) {
	    tsdPtr->rr++;
	    if (tsdPtr->rr >= n) tsdPtr->rr = 0;

	    esPtr = esPtrs[tsdPtr->rr];

	    if (esPtr->key != key) {
		esPtr->key = key;
		esPtr->force_read = FALSE;
		*esPtrOut = esPtr;
		return(EXP_DATA_OLD);
	    } else if ((!esPtr->force_read) && (!expSizeZero(esPtr))) {
		*esPtrOut = esPtr;
		return(EXP_DATA_OLD);
	    }
	}

	if (!timer_created) {
	    if (timeout >= 0) {
		timetoken = Tcl_CreateTimerHandler(1000*timeout,
			exp_timehandler,
			(ClientData)&timer_fired);
		timer_created = TRUE;
	    }
	}

	for (;;) {
	    int j;

	    /* make sure that all fds that should be armed are */
	    for (j=0;j<n;j++) {
		esPtr = esPtrs[j];

		if (!esPtr->fg_armed) {
		    Tcl_CreateChannelHandler(
			esPtr->channel,
			default_mask,
			exp_channelhandler,
			(ClientData)esPtr);
		    esPtr->fg_armed = TRUE;
		}
	    }

	    Tcl_DoOneEvent(0);	/* do any event */

	    if (timer_fired) return(EXP_TIMEOUT);

	    if (old_configure_count != exp_configure_count) {
		if (timer_created)
		    Tcl_DeleteTimerHandler(timetoken);
		return EXP_RECONFIGURE;
	    }

	    esPtr = tsdPtr->ready_esPtr;

	    if (esPtr) continue;

	    /* if it was from something we're not looking for at */
	    /* the moment, ignore it */
	    for (j=0;j<n;j++) {
		if (esPtr == esPtrs[j]) goto found;
	    }

	    /* not found */
	    exp_event_disarm_fg(esPtr);
	    tsdPtr->ready_esPtr = 0;
	    continue;
	    found:
	    *esPtrOut = esPtr;
	    tsdPtr->ready_esPtr = EXP_SPAWN_ID_BAD;

	    /* this test should be redundant but SunOS */
	    /* raises both READABLE and EXCEPTION (for no */
	    /* apparent reason) when selecting on a plain file */
	    if (tsdPtr->ready_mask & TCL_READABLE) {
		if (timer_created) Tcl_DeleteTimerHandler(timetoken);
		return EXP_DATA_NEW;
	    }

	    /* ready_mask must contain TCL_EXCEPTION */
#ifndef HAVE_PTYTRAP
	    if (timer_created) Tcl_DeleteTimerHandler(timetoken);
	    return(EXP_EOF);
#else
	    if (ioctl(esPtr->fdin,TIOCREQCHECK,&ioctl_info) < 0) {
		expDiagLog("ioctl error on TIOCREQCHECK: %s", Tcl_PosixError(interp));
		if (timer_created) Tcl_DeleteTimerHandler(timetoken);
		return(EXP_TCLERROR);
	    }
	    if (ioctl_info.request == TIOCCLOSE) {
		if (timer_created) Tcl_DeleteTimerHandler(timetoken);
		return(EXP_EOF);
	    }
	    if (ioctl(esPtr->fdin, TIOCREQSET, &ioctl_info) < 0) {
		expDiagLog("ioctl error on TIOCREQSET after ioctl or open on slave: %s", Tcl_ErrnoMsg(errno));
	    }
	    /* presumably, we trapped an open here */
	    continue;
#endif /* !HAVE_PTYTRAP */
	}
    }
}

/* Having been told there was an event for a specific ExpState, get it */
/* This returns status, one of EOF, TIMEOUT, ERROR or DATA */
/*ARGSUSED*/
int
exp_get_next_event_info(interp,esPtr,ready_mask)
Tcl_Interp *interp;
ExpState *esPtr;
int ready_mask;
{
#ifdef HAVE_PTYTRAP
    struct request_info ioctl_info;
#endif

    if (ready_mask & TCL_READABLE) return EXP_DATA_NEW;

    /* ready_mask must contain TCL_EXCEPTION */
#ifndef HAVE_PTYTRAP
    return(EXP_EOF);
#else
    if (ioctl(esPtr->fdin,TIOCREQCHECK,&ioctl_info) < 0) {
	expDiagLog("ioctl error on TIOCREQCHECK: %s",
		Tcl_PosixError(interp));
	return(EXP_TCLERROR);
    }
    if (ioctl_info.request == TIOCCLOSE) {
	return(EXP_EOF);
    }
    if (ioctl(esPtr->fdin, TIOCREQSET, &ioctl_info) < 0) {
	expDiagLog("ioctl error on TIOCREQSET after ioctl or open on slave: %s", Tcl_ErrnoMsg(errno));
    }
    /* presumably, we trapped an open here */
    /* call it an error for lack of anything more descriptive */
    /* it will be thrown away by caller anyway */
    return EXP_TCLERROR;
#endif
}

/*ARGSUSED*/
int	/* returns TCL_XXX */
exp_dsleep(interp,sec)
Tcl_Interp *interp;
double sec;
{
    int timer_fired = FALSE;

    ThreadSpecificData *tsdPtr = TCL_TSD_INIT(&dataKey);

    Tcl_CreateTimerHandler((int)(sec*1000),exp_timehandler,(ClientData)&timer_fired);

    while (1) {
	Tcl_DoOneEvent(0);
	if (timer_fired) return TCL_OK;

	if (!tsdPtr->ready_esPtr) continue;

	exp_event_disarm(tsdPtr->ready_esPtr);
	tsdPtr->ready_esPtr = EXP_SPAWN_ID_BAD;
    }
}

static char destroy_cmd[] = "destroy .";

static void
exp_event_exit_real(interp)
Tcl_Interp *interp;
{
    Tcl_Eval(interp,destroy_cmd);
}

/* set things up for later calls to event handler */
void
exp_init_event()
{
    ThreadSpecificData *tsdPtr = TCL_TSD_INIT(&dataKey);

    tsdPtr->ready_esPtr = 0;
    tsdPtr->rr = 0;

    exp_event_exit = exp_event_exit_real;
}
