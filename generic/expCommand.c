/*
 * expCommand.c --
 *
 *	The bulk of the Expect commands, platform generic
 *
 * Unix version written by: Don Libes, NIST, 2/6/90
 *
 * Design and implementation of this program was paid for by U.S. tax
 * dollars.  Therefore it is public domain.  However, the author and NIST
 * would appreciate credit if this program or parts of it are used.
 *
 *
 * Windows NT port by Gordon Chaffee
 * Copyright (c) 1997 by Mitel Corporation
 *
 * See the file "license.terms" for information on usage and redistribution
 * of this file, and for a DISCLAIMER OF ALL WARRANTIES.
 *
 */

//#include <math.h>
//#include "tclInt.h"
//#include "tclPort.h"
//#include "exp_port.h"
//#include "expect_tcl.h"
//#include "exp_command.h"
//#include "exp_rename.h"
//#include "exp_log.h"
//#include "exp_event.h"
//#include "exp_prog.h"
//#include "exp_tty.h"

#include <math.h>		/* for log/pow computation in send -h */
#include "expInt.h"

/*
 * These constants refer to the UTF string that encodes a null character.
 */

#define NULL_STRING "\300\200" /* hex C080 */
#define NULL_LENGTH 2

/*
 *  Found in either pty_sgtttyb.c, pty_termios.c, or pty_unicos.c
 */

extern int exp_getptymaster();
extern int exp_getptyslave();

int exp_forked = FALSE;		/* whether we are child process */

/* the following are use to create reserved addresses, to be used as ClientData */
/* args to be used to tell commands how they were called. */
/* The actual values won't be used, only the addresses, but I give them */
/* values out of my irrational fear the compiler might collapse them all. */
static int sendCD_error = 2;	/* called as send_error */
static int sendCD_user = 3;	/* called as send_user */
static int sendCD_proc = 4;	/* called as send or send_spawn */
static int sendCD_tty = 6;	/* called as send_tty */

/*
 * expect_key is just a source for generating a unique stamp.  As each
 * expect/interact command begins, it generates a new key and marks all
 * the spawn ids of interest with it.  Then, if someone comes along and
 * marks them with yet a newer key, the old command will recognize this
 * reexamine the state of the spawned process.
 */
int expect_key = 0;

/*
 * exp_configure_count is incremented whenever a spawned process is closed
 * or an indirect list is modified.  This forces any (stack of) expect or
 * interact commands to reexamine the state of the world and adjust
 * accordingly.
 */
int exp_configure_count = 0;

#ifdef HAVE_PTYTRAP
/* slaveNames provides a mapping from the pty slave names to our */
/* spawn id entry.  This is needed only on HPs for stty, sigh. */
static Tcl_HashTable slaveNames;
#endif /* HAVE_PTYTRAP */

typedef struct ThreadSpecificData {
    /*
     * List of all exp channels currently open.  This is per thread and is
     * used to match up fd's to channels, which rarely occurs.
     */
    
    ExpState *stdinout;
    ExpState *stderrX;   /* grr....stderr is a macro */
    ExpState *devtty;
    ExpState *any; /* for any_spawn_id */

    Tcl_Channel *diagChannel;
    Tcl_DString diagDString;
    int diagEnabled;
} ThreadSpecificData;

static Tcl_ThreadDataKey dataKey;


struct slow_arg {
    int size;
    double time;
};


/* Local prototypes for functions used only here that are not shared. */
static void	exp_wait_zero _ANSI_ARGS_((WAIT_STATUS_TYPE *status));
static void	expBusy _ANSI_ARGS_((ExpState *esPtr));
static int	exact_write _ANSI_ARGS_((ExpState *esPtr,char *buffer,int rembytes));

static int	human_write _ANSI_ARGS_((Tcl_Interp *interp, ExpState *esPtr,
			char *buffer, struct human_arg *arg));
static int	get_slow_args _ANSI_ARGS_((Tcl_Interp *interp, struct slow_arg *x));



static void	tcl_tracer _ANSI_ARGS_((ClientData clientData,
			    Tcl_Interp *interp, int level, char *command,
			    Tcl_CmdProc *cmdProc, ClientData cmdClientData,
			    int argc, char *argv[]));



static void	exp_i_add_f _ANSI_ARGS_((struct exp_i *,
			    struct exp_f *fs));
static void	exp_f_closed _ANSI_ARGS_((struct exp_f *));


/*
 *----------------------------------------------------------------------
 *
 * init_traps --
 *
 *	Not sure what this does.
 *
 * Results:
 *	None
 *
 * Side Effects:
 *	No clue.
 *
 *----------------------------------------------------------------------
 */

#ifdef FULLTRAPS
static void
init_traps(traps)
RETSIGTYPE (*traps[])();
{
	int i;

	for (i=1;i<NSIG;i++) {
		traps[i] = SIG_ERR;
	}
}
#endif


/*
 *----------------------------------------------------------------------
 *
 * exp_error --
 *
 *	Formats an error message into the interp.  Do not terminate
 *	format strings with \n!!!.
 *
 * Results:
 *	None
 *
 * Side Effects:
 *	An error message is written into interp->result
 *
 *----------------------------------------------------------------------
 */
void
exp_error TCL_VARARGS_DEF(Tcl_Interp *,arg1)
{
	Tcl_Interp *interp;
	char *fmt;
	va_list args;
	char buffer[2000];

	interp = TCL_VARARGS_START(Tcl_Interp *,arg1,args);
	fmt = va_arg(args,char *);
	vsprintf(buffer,fmt,args);
	Tcl_SetResult(interp,buffer,TCL_VOLATILE);
	va_end(args);
}

/*
 *----------------------------------------------------------------------
 *
 * expStateCurrent --
 *
 *	If 0, may be immediately followed by return TCL_ERROR.
 *
 * Results:
 *	current ExpState or 0
 *
 *----------------------------------------------------------------------
 */
struct ExpState *
expStateCurrent(interp,opened,adjust,any)
    Tcl_Interp *interp;
    int opened;
    int adjust;
    int any;
{
    static char *user_spawn_id = "exp0";

    char *name = exp_get_var(interp,EXP_SPAWN_ID_VARNAME);
    if (!name) name = user_spawn_id;

    return expStateFromChannelName(interp,name,opened,adjust,any,EXP_SPAWN_ID_VARNAME);
}

/*
 *----------------------------------------------------------------------
 *
 * expStateCheck --
 *
 *	Add comment here.
 *
 * Results:
 *	
 *
 *----------------------------------------------------------------------
 */
ExpState *
expStateCheck(interp,esPtr,open,adjust,msg)
    Tcl_Interp *interp;
    ExpState *esPtr;
    int open;
    int adjust;
    char *msg;
{
    if (open && !esPtr->open) {
	exp_error(interp,"%s: spawn id %s not open",msg,esPtr->name);
	return(0);
    }
    if (adjust) expAdjust(esPtr);
    return esPtr;
}

/*
 *----------------------------------------------------------------------
 *
 * expStateFromChannelName --
 *
 *	Add comment here.
 *
 * Results:
 *	
 *
 *----------------------------------------------------------------------
 */
ExpState *
expStateFromChannelName(interp,name,open,adjust,any,msg)
    Tcl_Interp *interp;
    char *name;
    int open;
    int adjust;
    char *msg;
{
    ExpState *esPtr;
    Tcl_Channel channel;
    char *chanName;

    if (any) {
	if (0 == strcmp(name,EXP_SPAWN_ID_ANY_LIT)) {
	    ThreadSpecificData *tsdPtr = TCL_TSD_INIT(&dataKey);
	    return tsdPtr->any;
	}
    }

    channel = Tcl_GetChannel(interp,name,(int *)0);
    if (!channel) return(0);

    chanName = Tcl_GetChannelName(channel);
    if (!isExpChannelName(chanName)) {
	exp_error(interp,"%s: %s is not an expect channel - use spawn -open to convert",msg,chanName);
	return(0);
    }

    esPtr = (ExpState *)Tcl_GetChannelInstanceData(channel);

    return expStateCheck(interp,esPtr,open,adjust,msg);
}

/*
 *----------------------------------------------------------------------
 *
 * exp_wait_zero --
 *
 *	Zero out the wait status field.
 *
 * Results:
 *	None
 *
 *----------------------------------------------------------------------
 */
void
exp_wait_zero(status)
    WAIT_STATUS_TYPE *status;
{
    int i;

    for (i = 0; i < sizeof(WAIT_STATUS_TYPE); i++) {
	((char *)status)[i] = 0;
    }
}

/*
 *----------------------------------------------------------------------
 *
 * exp_state_prep_for_invalidation --
 *
 *	called just before an ExpState entry is about to be invalidated.
 *
 * Results:
 *	None
 *
 *----------------------------------------------------------------------
 */
void
exp_state_prep_for_invalidation(interp,esPtr)
    Tcl_Interp *interp;
    ExpState *esPtr;
{
    exp_ecmd_remove_state_direct_and_indirect(interp,esPtr);

    exp_configure_count++;

    if (esPtr->fg_armed) {
	exp_event_disarm_fg(esPtr);
    }
}

/*
 *----------------------------------------------------------------------
 *
 * exp_trap_on --
 *
 *	Add comment here.
 *
 * Results:
 *	None
 *
 *----------------------------------------------------------------------
 */
/*ARGSUSED*/
void
exp_trap_on(master)
    int master;
{
#ifdef HAVE_PTYTRAP
    if (master == -1) return;
    exp_slave_control(master,1);
#endif /* HAVE_PTYTRAP */
}

/*
 *----------------------------------------------------------------------
 *
 * exp_trap_off --
 *
 *	Add comment here.
 *
 * Results:
 *	None
 *
 *----------------------------------------------------------------------
 */
int
exp_trap_off(name)
    char *name;
{
#ifdef HAVE_PTYTRAP
    ExpState *esPtr;
    int enable = 0;

    Tcl_HashEntry *entry = Tcl_FindHashEntry(&slaveNames,name);
    if (!entry) {
	expDiagLog("exp_trap_off: no entry found for %s\n",name);
	return -1;
    }

    esPtr = (ExpState *)Tcl_GetHashValue(entry);
    
    exp_slave_control(esPtr->fdin,0);

    return esPtr->fdin;
#else
    return name[0];	/* pacify lint, use arg and return something */
#endif
}

/*
 *----------------------------------------------------------------------
 *
 * expBusy --
 *
 *	Add comment here.  Has OS depedancies.
 *
 * Results:
 *	None
 *
 *----------------------------------------------------------------------
 */
void
expBusy(esPtr)
     ExpState *esPtr;
{
#ifndef __WIN32__
    int x = open("/dev/null",0);
    if (x != esPtr->fdin) {
	fcntl(x,F_DUPFD,esPtr->fdin);
	close(x);
    }
    expCloseOnExec(esPtr->fdin);
    esPtr->fdBusy = TRUE;
#else
    /* what goes here? */
#endif
}





























/*
 *----------------------------------------------------------------------
 *
 * exp_close --
 *
 *	Close a connection.
 *
 * Results:
 *	None
 *
 * Side Effects:
 *	A native file handle is closed
 *
 *----------------------------------------------------------------------
 */
int
exp_close(interp, esPtr)
    Tcl_Interp *interp;
    ExpState *esPtr;
{
    if (0 == expStateCheck(interp,esPtr,1,0,"close")) return TCL_ERROR;
    esPtr->open = FALSE;

    /*
     * Ignore close errors from ptys.  Ptys on some systems return errors for
     * no evident reason.  Anyway, receiving an error upon pty-close doesn't
     * mean anything anyway as far as I know.  
     */

    close(esPtr->fdin);
    if (esPtr->fd_slave != EXP_NOFD) close(esPtr->fd_slave);
    if (esPtr->fdin != esPtr->fdout) close(esPtr->fdout);

    if (esPtr->channel_orig && !esPtr->leaveopen) {
	/*
	 * Ignore close errors from Tcl channels.  They indicate things
	 * like broken pipelines, etc, which don't affect our
	 * subsequent handling.
	 */
	Tcl_VarEval(interp,"close ",Tcl_GetChannelName(esPtr->channel_orig),
		(char *)0);
    }

#ifdef HAVE_PTYTRAP
    if (esPtr->slave_name) {
	Tcl_HashEntry *entry;
	
	entry = Tcl_FindHashEntry(&slaveNames,esPtr->slave_name);
	Tcl_DeleteHashEntry(entry);

	ckfree(esPtr->slave_name);
	esPtr->slave_name = 0;
    }
#endif

    exp_state_prep_for_invalidation(interp,esPtr);

    if (esPtr->user_waited) {
	if (esPtr->registered) {
	    Tcl_UnregisterChannel(interp,esPtr->channel);
	    /* at this point esPtr may have been freed so don't touch it
               any longer */
	}
    } else {
      expBusy(esPtr);
    }

    return(TCL_OK);
}

/*
 *----------------------------------------------------------------------
 *
 * expStateAnyIs --
 *
 *	report whether this ExpState represents special spawn_id_any
 *	we need a separate function because spawn_id_any is thread-specific
 *	and can't be seen outside this file.
 *
 * Results:
 *	
 *
 *----------------------------------------------------------------------
 */
int
expStateAnyIs(esPtr)
    ExpState *esPtr;
{
    ThreadSpecificData *tsdPtr = TCL_TSD_INIT(&dataKey);
    return (esPtr == tsdPtr->any);
}

/*
 *----------------------------------------------------------------------
 *
 * expDevttyIs --
 *
 *	Add comment here.
 *
 * Results:
 *	
 *
 *----------------------------------------------------------------------
 */
int
expDevttyIs(esPtr)
    ExpState *esPtr;
{
    ThreadSpecificData *tsdPtr = TCL_TSD_INIT(&dataKey);
    return (esPtr == tsdPtr->devtty);
}

/*
 *----------------------------------------------------------------------
 *
 * expStdinoutIs --
 *
 *	Add comment here.
 *
 * Results:
 *	
 *
 *----------------------------------------------------------------------
 */
int
expStdinoutIs(esPtr)
ExpState *esPtr;
{
    ThreadSpecificData *tsdPtr = TCL_TSD_INIT(&dataKey);
    return (tsdPtr->stdinout == esPtr);
}

/*
 *----------------------------------------------------------------------
 *
 * expStdinoutGet --
 *
 *	Add comment here.
 *
 * Results:
 *	
 *
 *----------------------------------------------------------------------
 */
ExpState *
expStdinoutGet()
{
    ThreadSpecificData *tsdPtr = TCL_TSD_INIT(&dataKey);
    return tsdPtr->stdinout;
}

/*
 *----------------------------------------------------------------------
 *
 * expDevttyGet --
 *
 *	Add comment here.
 *
 * Results:
 *	
 *
 *----------------------------------------------------------------------
 */
ExpState *
expDevttyGet()
{
    ThreadSpecificData *tsdPtr = TCL_TSD_INIT(&dataKey);
    return tsdPtr->devtty;
}

/*
 *----------------------------------------------------------------------
 *
 * Exp_ExpPidCmd --
 *
 *	Implements the "exp_pid" command
 *
 * Results:
 *	A standard Tcl result
 *
 * Side Effects:
 *	None
 *
 * Notes:
 *	OS independent
 *
 *----------------------------------------------------------------------
 */

/*ARGSUSED*/
int
Exp_ExpPidCmd(clientData,interp,argc,argv)
    ClientData clientData;
    Tcl_Interp *interp;
    int argc;
    char **argv;
{
    char *chanName = 0;
    ExpState *esPtr = 0;

    argc--; argv++;

    for (;argc>0;argc--,argv++) {
	if (streq(*argv,"-i")) {
	    argc--; argv++;
	    if (!*argv) goto usage;
	    chanName = *argv;
	} else goto usage;
    }

    if (chanName) {
	if (!(esPtr = expStateFromChannelName(interp,chanName,0,0,0,"exp_pid"))) return TCL_ERROR;
    } else {
	if (!(esPtr = expStateCurrent(interp,0,0,0))) return TCL_ERROR;
    }
    
    sprintf(interp->result,"%d",esPtr->pid);
    return TCL_OK;
  usage:
    exp_error(interp,"usage: -i spawn_id");
    return TCL_ERROR;
}

#if 0
/*
 *----------------------------------------------------------------------
 *
 * Exp_GetpidDeprecatedCmd --
 *
 *	Implements the old 'getpid' command.  This command is has
 *	been deprecated and may not be supported in the future
 *
 * Results:
 *	A standard Tcl result
 *
 * Side Effects:
 *	None
 *
 * Notes:
 *	OS independent
 *
 *----------------------------------------------------------------------
 */

/*ARGSUSED*/
static int
Exp_GetpidDeprecatedCmd(clientData, interp, argc, argv)
    ClientData clientData;
    Tcl_Interp *interp;
    int argc;
    char **argv;
{
    debuglog("getpid is deprecated, use pid\r\n");
    sprintf(interp->result,"%d",exp_getpidproc());
    return(TCL_OK);
}
#endif

/*
 *----------------------------------------------------------------------
 *
 * Exp_SleepCmd --
 *
 *	Implements the 'sleep' (alias 'exp_sleep') command.
 *	Can sleep for fractional seconds.
 *
 * Results:
 *	A standard Tcl result
 *
 * Side Effects:
 *	May not return immediately, and it may service other
 *	events during the sleep period
 *	
 * Notes:
 *	OS independent
 *
 *----------------------------------------------------------------------
 */

/*ARGSUSED*/
static int
Exp_SleepCmd(clientData,interp,argc,argv)
    ClientData clientData;
    Tcl_Interp *interp;
    int argc;
    char **argv;
{
    argc--; argv++;

    if (argc != 1) {
	exp_error(interp,"must have one arg: seconds");
	return TCL_ERROR;
    }

    return(exp_dsleep(interp,(double)atof(*argv)));
}

/*
 *----------------------------------------------------------------------
 *
 * exact_write --
 *
 *	If this works, exact_write should disappear and function should
 *	call Tcl_WriteChars directly.
 *
 * Notes:
 *	OS independent
 *
 *----------------------------------------------------------------------
 */
int
exact_write(esPtr,buffer,rembytes) /* INTL */
    ExpState *esPtr;
    char *buffer;
    int rembytes;
{
    Tcl_WriteChars(esPtr->channel,buffer,rembytes);
    return(0);
}


/*
 *----------------------------------------------------------------------
 *
 * get_slow_args --
 *
 *	Get the arguments the the 'send -s' command
 *
 * Results:
 *	0 on success, -1 on failure
 *	
 * Side Effects:
 *	The slow_arg structure is filled in
 *
 *----------------------------------------------------------------------
 */

int
get_slow_args(interp,x)
    Tcl_Interp *interp;
    struct slow_arg *x;
{
    int sc;		/* return from scanf */
    char *s = exp_get_var(interp,"send_slow");
    if (!s) {
	exp_error(interp,"send -s: send_slow has no value");
	return(-1);
    }
    if (2 != (sc = sscanf(s,"%d %lf",&x->size,&x->time))) {
	exp_error(interp,"send -s: found %d value(s) in send_slow but need 2",sc);
	return(-1);
    }
    if (x->size <= 0) {
	exp_error(interp,"send -s: size (%d) in send_slow must be positive", x->size);
	return(-1);
    }
    if (x->time <= 0) {
	exp_error(interp,"send -s: time (%f) in send_slow must be larger",x->time);
	return(-1);
    }
    return(0);
}

/*
 *----------------------------------------------------------------------
 *
 * slow_write --
 *
 *	Write some bytes   s  l  o  w  l  y
 *
 * Results:
 *	0 on success, -1 on failure, positive for standard Tcl result
 *	
 * Side Effects:
 *	Data is written to an output object
 *
 * Notes:
 *	OS independent
 *
 *----------------------------------------------------------------------
 */

/* returns 0 for success, -1 for failure, pos. for Tcl return value */
static int
slow_write(interp,esPtr,buffer,rembytes,arg) /* INTL */
    Tcl_Interp *interp;
    ExpState *esPtr;
    char *buffer;
    int rembytes;
    struct slow_arg *arg;
{
    int rc;

    while (rembytes > 0) {
	int len;
		
	len = (arg->size<rembytes?arg->size:rembytes);
	if (0 > Tcl_WriteChars(esPtr->channel,buffer,len)) return -1;
	rembytes -= arg->size;
	buffer += arg->size;

	/* skip sleep after last write */
	if (rembytes > 0) {
	    rc = exp_dsleep(interp,arg->time);
	    if (rc > 0) return rc;
	}
    }
    return 0;
}

struct human_arg {
    float alpha;		/* average interarrival time in seconds */
    float alpha_eow;		/* as above but for eow transitions */
    float c;			/* shape */
    float min, max;
};


/*
 *----------------------------------------------------------------------
 *
 * get_human_args --
 *
 *	Get the arguments the the 'send -h' command
 *
 * Results:
 *	0 on success, -1 on failure
 *	
 * Side Effects:
 *	The human_arg structure is filled in
 *
 * Notes:
 *	OS independent
 *
 *----------------------------------------------------------------------
 */

static int
get_human_args(interp,x)
    Tcl_Interp *interp;
    struct human_arg *x;
{
    int sc;			/* return from scanf */
    char *s = exp_get_var(interp,"send_human");

    if (!s) {
	exp_error(interp,"send -h: send_human has no value");
	return(-1);
    }
    if (5 != (sc = sscanf(s,"%f %f %f %f %f",
			  &x->alpha,&x->alpha_eow,&x->c,&x->min,&x->max))) {
	if (sc == EOF) sc = 0;	/* make up for overloaded return */
	exp_error(interp,"send -h: found %d value(s) in send_human but need 5",sc);
	return(-1);
    }
    if (x->alpha < 0 || x->alpha_eow < 0) {
	exp_error(interp,"send -h: average interarrival times (%f %f) must be non-negative in send_human", x->alpha,x->alpha_eow);
	return(-1);
    }
    if (x->c <= 0) {
	exp_error(interp,"send -h: variability (%f) in send_human must be positive",x->c);
	return(-1);
    }
    x->c = 1/x->c;

    if (x->min < 0) {
	exp_error(interp,"send -h: minimum (%f) in send_human must be non-negative",x->min);
	return(-1);
    }
    if (x->max < 0) {
	exp_error(interp,"send -h: maximum (%f) in send_human must be non-negative",x->max);
	return(-1);
    }
    if (x->max < x->min) {
	exp_error(interp,"send -h: maximum (%f) must be >= minimum (%f) in send_human",x->max,x->min);
	return(-1);
    }
    return(0);
}

/*
 *----------------------------------------------------------------------
 *
 * unit_random --
 *
 *	Compute random numbers from 0 to 1, for expect's send -h
 *	This implementation sacrifices beauty for portability.
 *	Current implementation is pathetic but works
 *
 * Results:
 *	A floating point number between 0 and 1
 *
 * Side Effects:
 *	None
 *
 * Notes:
 *	OS independent
 *
 *----------------------------------------------------------------------
 */

static float
unit_random()
{
    /* 99991 is largest prime in my CRC - can't hurt, eh? */
    return((float)(1+(rand()%99991))/(float) 99991.0);
}

/*
 *----------------------------------------------------------------------
 *
 * exp_init_unit_random --
 *
 *	Initialize the random number generator
 *
 * Results:
 *	None
 *
 * Side Effects:
 *	None
 *
 * Notes:
 *	OS independent
 *
 *----------------------------------------------------------------------
 */

void
exp_init_unit_random()
{
    srand(exp_getpidproc());
}

/*
 *----------------------------------------------------------------------
 *
 * human_write --
 *
 *	This function is my implementation of the Weibull distribution.
 *	I've added a max time and an "alpha_eow" that captures the slight
 *	but noticable change in human typists when hitting end-of-word
 *	transitions.
 *
 * Results:
 *	0 for success, -1 for failure, positive for standard Tcl result
 *
 * Side Effects:
 *	None
 *
 *----------------------------------------------------------------------
 */
static int
human_write(interp,esPtr,buffer,arg) /* INTL */
    Tcl_Interp *interp;
    ExpState *esPtr;
    char *buffer;
    struct human_arg *arg;
{
    char *sp;
    int size;
    float t;
    float alpha;
    int wc;
    int in_word = TRUE;
    Tcl_UniChar ch;

    expDiagLog("human_write: avg_arr=%f/%f  1/shape=%f  min=%f  max=%f\r\n",
	    arg->alpha,arg->alpha_eow,arg->c,arg->min,arg->max);

    for (sp = buffer;*sp;sp += size) {
	size = Tcl_UtfToUniChar(sp, &ch);
	/* use the end-of-word alpha at eow transitions */
	if (in_word && (Tcl_UniCharIsPunct(ch) || Tcl_UniCharIsSpace(ch)))
	    alpha = arg->alpha_eow;
	else alpha = arg->alpha;
	in_word = !(Tcl_UniCharIsPunct(ch) || Tcl_UniCharIsSpace(ch));

	t = alpha * pow(-log((double)unit_random()),arg->c);

	/* enforce min and max times */
	if (t<arg->min) t = arg->min;
	else if (t>arg->max) t = arg->max;

		/* skip sleep before writing first character */
	if (sp != buffer) {
	    wc = exp_dsleep(interp,(double)t);
	    if (wc > 0) return wc;
	}

	wc = Tcl_WriteChars(esPtr->channel, sp, size);
	if (0 > wc) return(wc);
    }
    return(0);
}

struct exp_i *exp_i_pool = 0;
struct exp_state_list *exp_state_list_pool = 0;

#define EXP_I_INIT_COUNT	10
#define EXP_FD_INIT_COUNT	10

struct exp_i *
exp_new_i()
{
    int n;
    struct exp_i *i;

    if (!exp_i_pool) {
	/* none avail, generate some new ones */
	exp_i_pool = i = (struct exp_i *)ckalloc(
		EXP_I_INIT_COUNT * sizeof(struct exp_i));
	for (n=0;n<EXP_I_INIT_COUNT-1;n++,i++) {
	    i->next = i+1;
	}
	i->next = 0;
    }

    /* now that we've made some, unlink one and give to user */

    i = exp_i_pool;
    exp_i_pool = exp_i_pool->next;
    i->value = 0;
    i->variable = 0;
    i->state_list = 0;
    i->ecount = 0;
    i->next = 0;
    return i;
}

struct exp_state_list *
exp_new_state(esPtr)
    ExpState *esPtr;
{
    int n;
    struct exp_state_list *fd;

    if (!exp_state_list_pool) {
	/* none avail, generate some new ones */
	exp_state_list_pool = fd = (struct exp_state_list *)ckalloc(
	    EXP_FD_INIT_COUNT * sizeof(struct exp_state_list));
	for (n=0;n<EXP_FD_INIT_COUNT-1;n++,fd++) {
	    fd->next = fd+1;
	}
	fd->next = 0;
    }

    /* now that we've made some, unlink one and give to user */

    fd = exp_state_list_pool;
    exp_state_list_pool = exp_state_list_pool->next;
    fd->esPtr = esPtr;
    /* fd->next is assumed to be changed by caller */
    return fd;
}

void
exp_free_state(fd_first)
    struct exp_state_list *fd_first;
{
    struct exp_state_list *fd, *penultimate;

    if (!fd_first) return;

    /* link entire chain back in at once by first finding last pointer */
    /* making that point back to pool, and then resetting pool to this */

    /* run to end */
    for (fd = fd_first;fd;fd=fd->next) {
	penultimate = fd;
    }
    penultimate->next = exp_state_list_pool;
    exp_state_list_pool = fd_first;
}

/* free a single fd */
void
exp_free_state_single(fd)
    struct exp_state_list *fd;
{
    fd->next = exp_state_list_pool;
    exp_state_list_pool = fd;
}

void
exp_free_i(interp,i,updateproc)
    Tcl_Interp *interp;
    struct exp_i *i;
    Tcl_VarTraceProc *updateproc;	/* proc to invoke if indirect is written */
{
    if (i->next) exp_free_i(interp,i->next,updateproc);

    exp_free_state(i->state_list);

    if (i->direct == EXP_INDIRECT) {
	Tcl_UntraceVar(interp,i->variable,
		TCL_GLOBAL_ONLY|TCL_TRACE_WRITES,
		updateproc,(ClientData)i);
    }

    /* here's the long form
       if duration & direct	free(var)  free(val)
	PERM	  DIR	    		1
	PERM	  INDIR	    1		1
	TMP	  DIR
	TMP	  INDIR			1
	Also if i->variable was a bogus variable name, i->value might not be
	set, so test i->value to protect this
	TMP in this case does NOT mean from the "expect" command.  Rather
	it means "an implicit spawn id from any expect or expect_XXX
	command".  In other words, there was no variable name provided.
    */
    if (i->value
	    && (((i->direct == EXP_DIRECT) && (i->duration == EXP_PERMANENT))
	    || ((i->direct == EXP_INDIRECT) && (i->duration == EXP_TEMPORARY)))) {
	ckfree(i->value);
    } else if (i->duration == EXP_PERMANENT) {
	if (i->value) ckfree(i->value);
	if (i->variable) ckfree(i->variable);
    }

    i->next = exp_i_pool;
    exp_i_pool = i;
}

/* generate a descriptor for a "-i" flag */
/* cannot fail */
struct exp_i *
exp_new_i_complex(interp,arg,duration,updateproc)
    Tcl_Interp *interp;
    char *arg;		/* spawn id list or a variable containing a list */
    int duration;	/* if we have to copy the args */
			/* should only need do this in expect_before/after */
    Tcl_VarTraceProc *updateproc;	/* proc to invoke if indirect is written */
{
    struct exp_i *i;
    char **stringp;

    i = exp_new_i();

    i->direct = (isExpChannelName(arg)?EXP_DIRECT:EXP_INDIRECT);
#if OBSOLETE
    i->direct = (isdigit(arg[0]) || (arg[0] == '-'))?EXP_DIRECT:EXP_INDIRECT;
#endif
    if (i->direct == EXP_DIRECT) {
	stringp = &i->value;
    } else {
	stringp = &i->variable;
    }

    i->duration = duration;
    if (duration == EXP_PERMANENT) {
	*stringp = ckalloc(strlen(arg)+1);
	strcpy(*stringp,arg);
    } else {
	*stringp = arg;
    }

    i->state_list = 0;
    exp_i_update(interp,i);

    /* if indirect, ask Tcl to tell us when variable is modified */

    if (i->direct == EXP_INDIRECT) {
	Tcl_TraceVar(interp, i->variable,
		TCL_GLOBAL_ONLY|TCL_TRACE_WRITES,
		updateproc, (ClientData) i);
    }

    return i;
}

void
exp_i_add_state(i,esPtr)
    struct exp_i *i;
    ExpState *esPtr;
{
    struct exp_state_list *new_state;

    new_state = exp_new_state(esPtr);
    new_state->next = i->state_list;
    i->state_list = new_state;
}

/* this routine assumes i->esPtr is meaningful */
static void
exp_i_parse_states(interp,i) /* INTL */
    Tcl_Interp *interp;
    struct exp_i *i;
{
    struct ExpState *esPtr;
    char *p = i->value;
    int argc;
    char **argv;
    int j;

    if (Tcl_SplitList(NULL, p, &argc, &argv) != TCL_OK) goto error;

    for (j = 0; j < argc; j++) {
        esPtr = expStateFromChannelName(interp,argv[j],1,0,0,"");
	if (!esPtr) goto error;
	exp_i_add_state(i,esPtr);
    }
    ckfree((char*)argv);
    return;
error:
    expDiagLogU("exp_i_parse_states: ");
    expDiagLogU(Tcl_GetStringResult(interp));
    return;
}
	
/* updates a single exp_i struct */
void
exp_i_update(interp,i)
    Tcl_Interp *interp;
    struct exp_i *i;
{
    char *p;	/* string representation of list of spawn ids */

    if (i->direct == EXP_INDIRECT) {
	p = Tcl_GetVar(interp,i->variable,TCL_GLOBAL_ONLY);
	if (!p) {
	    p = "";
	    /* *really* big variable names could blow up expDiagLog! */
	    expDiagLog("warning: indirect variable %s undefined",i->variable);
	}
    
	if (i->value) {
	    if (streq(p,i->value)) return;
      
	    /* replace new value with old */
	    ckfree(i->value);
	}
	i->value = ckalloc(strlen(p)+1);
	strcpy(i->value,p);

	exp_free_state(i->state_list);
	i->state_list = 0;
    } else {
	/* no free, because this should only be called on */
	/* "direct" i's once */
	i->state_list = 0;
    }
    exp_i_parse_states(interp, i);
    return;
}

struct exp_i *
exp_new_i_simple(esPtr,duration)
    ExpState *esPtr;
    int duration;   /* if we have to copy the args */
		    /* should only need do this in expect_before/after */
{
    struct exp_i *i;

    i = exp_new_i();
    i->direct = EXP_DIRECT;
    i->duration = duration;
    exp_i_add_state(i,esPtr);
    return i;
}

/*ARGSUSED*/
static int
Exp_SendLogCmd(clientData, interp, argc, argv)
    ClientData clientData;
    Tcl_Interp *interp;
    int argc;
    char **argv;
{
    argv++;
    argc--;
    if (argc) {
	if (streq(*argv,"--")) {
	    argc--; argv++;
	}
    }

    if (argc != 1) {
	exp_error(interp,"usage: send [args] string");
	return TCL_ERROR;
    }

    expLogDiagU(*argv);
    return TCL_OK;
}


/* I've rewritten this to be unbuffered.  I did this so you could shove */
/* large files through "send".  If you are concerned about efficiency */
/* you should quote all your send args to make them one single argument. */
/*ARGSUSED*/
int
Exp_SendObjCmd(clientData, interp, objc, objv) /* INTL */
    ClientData clientData;
    Tcl_Interp *interp;
    int objc;
    Tcl_Obj *CONST objv[];
{
    ThreadSpecificData *tsdPtr = TCL_TSD_INIT(&dataKey);
    ExpState *esPtr = 0;
    int rc; 	/* final result of this procedure */
    struct human_arg human_args;
    struct slow_arg slow_args;
#define SEND_STYLE_STRING_MASK	0x07	/* mask to detect a real string arg */
#define SEND_STYLE_PLAIN	0x01
#define SEND_STYLE_HUMAN	0x02
#define SEND_STYLE_SLOW		0x04
#define SEND_STYLE_ZERO		0x10
#define SEND_STYLE_BREAK	0x20
    int send_style = SEND_STYLE_PLAIN;
    int want_cooked = TRUE;
    char *string;		/* string to send */
    int len = -1;		/* length of string to send */
    int zeros;		/* count of how many ascii zeros to send */

    char *chanName = 0;
    struct exp_state_list *state_list;
    struct exp_i *i;
    int j;

    static char *options[] = {
	"-i", "-h", "-s", "-null", "-0", "-raw", "-break", "--", (char *)0
    };
    enum options {
	SEND_SPAWNID, SEND_HUMAN, SEND_SLOW, SEND_NULL, SEND_ZERO,
	SEND_RAW, SEND_BREAK, SEND_LAST
    };

    for (j = 1; j < objc; j++) {
	char *name;
	int index;

	name = Tcl_GetString(objv[j]);
	if (name[0] != '-') {
	    break;
	}
	if (Tcl_GetIndexFromObj(interp, objv[j], options, "flag", 0,
		&index) != TCL_OK) {
	    return TCL_ERROR;
	}
	switch ((enum options) index) {
	    case SEND_SPAWNID:
		j++;
		chanName = Tcl_GetString(objv[j]);
		break;

	    case SEND_LAST:
		j++;
		goto getString;

	    case SEND_HUMAN:
		if (-1 == get_human_args(interp,&human_args))
		    return(TCL_ERROR);
		send_style = SEND_STYLE_HUMAN;
		break;

	    case SEND_SLOW:
		if (-1 == get_slow_args(interp,&slow_args))
		    return(TCL_ERROR);
		send_style = SEND_STYLE_SLOW;
		break;

	    case SEND_NULL:
	    case SEND_ZERO:
		j++;
		if (j >= objc) {
		    zeros = 1;
		} else if (Tcl_GetIntFromObj(interp, objv[j], &zeros)
			!= TCL_OK) {
		    return TCL_ERROR;
		}
		if (zeros < 1) return TCL_OK;
		send_style = SEND_STYLE_ZERO;
		string = "<zero(s)>";
		break;

	    case SEND_RAW:
		want_cooked = FALSE;
		break;

	    case SEND_BREAK:
		send_style = SEND_STYLE_BREAK;
		string = "<break>";
		break;
	}
    }

    if (send_style & SEND_STYLE_STRING_MASK) {
	if (j != objc-1) {
	    exp_error(interp,"usage: send [args] string");
	    return TCL_ERROR;
	}
getString:
	string = Tcl_GetStringFromObj(objv[j], &len);
    } else {
	len = strlen(string);
    }

    if (clientData == &sendCD_user) esPtr = tsdPtr->stdinout;
    else if (clientData == &sendCD_error) esPtr = tsdPtr->stderrX;
    else if (clientData == &sendCD_tty) esPtr = tsdPtr->devtty;
    else if (!chanName) {
	/* we want to check if it is open */
	/* but since stdin could be closed, we have to first */
	/* get the fd and then convert it from 0 to 1 if necessary */
	if (!(esPtr = expStateCurrent(interp,0,0,0))) return(TCL_ERROR);
    }

    if (esPtr) {
	i = exp_new_i_simple(esPtr,EXP_TEMPORARY);
    } else {
	i = exp_new_i_complex(interp,chanName,FALSE,(Tcl_VarTraceProc *)0);
    }

#define send_to_stderr	(clientData == &sendCD_error)
#define send_to_proc	(clientData == &sendCD_proc)
#define send_to_user	((clientData == &sendCD_user) || \
			 (clientData == &sendCD_tty))

    if (send_to_proc) {
	want_cooked = FALSE;
	expDiagLogU("send: sending \"");
	expDiagLogU(expPrintify(string));
	expDiagLogU("\" to {");
	/* if closing brace doesn't appear, that's because an error */
	/* was encountered before we could send it */
    } else {
	expLogDiagU(string);
    }

    for (state_list=i->state_list;state_list;state_list=state_list->next) {
	esPtr = state_list->esPtr;

	if (send_to_proc) {
	    expDiagLog(" %s ",esPtr->name);
	}

	/* check validity of each - i.e., are they open */
	if (0 == expStateCheck(interp,esPtr,1,0,"send")) {
	    rc = TCL_ERROR;
	    goto finish;
	}
	if (want_cooked) string = exp_cook(string,&len);

	switch (send_style) {
	    case SEND_STYLE_PLAIN:
		rc = exact_write(esPtr,string,len);
		break;
	    case SEND_STYLE_SLOW:
		rc = slow_write(interp,esPtr,string,len,&slow_args);
		break;
	    case SEND_STYLE_HUMAN:
		rc = human_write(interp,esPtr,string,&human_args);
		break;
	    case SEND_STYLE_ZERO:
		for (;zeros>0;zeros--) {
		    rc = Tcl_WriteChars(esPtr->channel,
			    NULL_STRING, NULL_LENGTH);
		}
		/* catching error on last write is sufficient */
		rc = ((rc==1) ? 0 : -1);   /* normal is 1 not 0 */
		break;
	    case SEND_STYLE_BREAK:
		exp_tty_break(interp,esPtr->fdout);
		rc = 0;
		break;
	}

	if (rc != 0) {
	    if (rc == -1) {
		exp_error(interp,"write(spawn_id=%d): %s",esPtr->fdout,Tcl_PosixError(interp));
		rc = TCL_ERROR;
	    }
	    goto finish;
	}
    }
    if (send_to_proc) expDiagLogU("}\r\n");

    rc = TCL_OK;
 finish:
    exp_free_i(interp,i,(Tcl_VarTraceProc *)0);
    return rc;
}

/*ARGSUSED*/
static int
Exp_LogFileCmd(clientData, interp, argc, argv)
    ClientData clientData;
    Tcl_Interp *interp;
    int argc;
    char **argv;
{
    static char resultbuf[1000];
    char *chanName = 0;
    int leaveOpen = FALSE;
    int logAll = FALSE;
    int append = TRUE;
    char *filename = 0;

    argv++;
    argc--;
    for (;argc>0;argc--,argv++) {
	if (streq(*argv,"-open")) {
	    if (!argv[1]) goto usage_error;
	    chanName = argv[1];
	    argc--; argv++;
	} else if (streq(*argv,"-leaveopen")) {
	    if (!argv[1]) goto usage_error;
	    chanName = argv[1];
	    leaveOpen = TRUE;
	    argc--; argv++;
	} else if (streq(*argv,"-a")) {
	    logAll = TRUE;
	} else if (streq(*argv,"-info")) {
	    resultbuf[0] = '\0';
	    if (expLogChannelGet()) {
		if (expLogAllGet()) strcat(resultbuf,"-a ");
		if (!expLogAppendGet()) strcat(resultbuf,"-noappend ");
		if (expLogFilenameGet()) {
		    strcat(resultbuf,expLogFilenameGet());
		} else {
		    if (expLogLeaveOpenGet()) {
			strcat(resultbuf,"-leaveopen ");
		    }
		    strcat(resultbuf,Tcl_GetChannelName(expLogChannelGet()));
		}
		Tcl_SetResult(interp,resultbuf,TCL_STATIC);
	    }
	    return TCL_OK;
	} else if (streq(*argv,"-noappend")) {
	    append = FALSE;
	} else break;
    }
    
    if (argc == 1) {
	filename = argv[0];
    } else if (argc > 1) {
	/* too many arguments */
	goto usage_error;
    } 
    
    if (chanName && filename) {
	goto usage_error;
    }

    /* check if user merely wants to change logAll (-a) */
    if (expLogChannelGet() && (chanName || filename)) {
	if (filename && (0 == strcmp(filename,expLogFilenameGet()))) {
	    expLogAllSet(logAll);
	    return TCL_OK;
	} else if (chanName && (0 == strcmp(filename,Tcl_GetChannelName(expLogChannelGet())))) {
	    expLogAllSet(logAll);
	    return TCL_OK;
	} else {
	    exp_error(interp,"cannot start logging without first stopping logging");
	    return TCL_ERROR;
	}
    }

    if (filename) {
	if (TCL_ERROR == expLogChannelOpen(interp,filename,append)) {
	    return TCL_ERROR;
	}
    } else if (chanName) {
	if (TCL_ERROR == expLogChannelSet(interp,chanName)) {
	    return TCL_ERROR;
	}
    } else {
	expLogChannelClose(interp);
	if (logAll) {
	    exp_error(interp,"cannot use -a without a file or channel");
	    return TCL_ERROR;
	}
    }
    expLogAllSet(logAll);
    expLogLeaveOpenSet(leaveOpen);

    return TCL_OK;

 usage_error:
    exp_error(interp,"usage: log_file [-info] [-noappend] [[-a] file] [-[leave]open [open ...]]");
    return TCL_ERROR;
}

/*ARGSUSED*/
int
Exp_LogUserCmd(clientData, interp, argc, argv)
    ClientData clientData;
    Tcl_Interp *interp;
    int argc;
    char **argv;
{
    int old_loguser = expLogUserGet();

    if (argc == 0 || (argc == 2 && streq(argv[1],"-info"))) {
	/* do nothing */
    } else if (argc == 2) {
	expLogUserSet(atoi(argv[1]));
    } else {
	exp_error(interp,"usage: [-info|1|0]");
    }

    sprintf(interp->result,"%d",old_loguser);

    return(TCL_OK);
}

#ifdef TCL_DEBUGGER
/*ARGSUSED*/
int
Exp_DebugCmd(clientData, interp, argc, argv)
    ClientData clientData;
    Tcl_Interp *interp;
    int argc;
    char **argv;
{
    int now = FALSE;	/* soon if FALSE, now if TRUE */
    int exp_tcl_debugger_was_available = exp_tcl_debugger_available;

    if (argc > 3) goto usage;

    if (argc == 1) {
	sprintf(interp->result,"%d",exp_tcl_debugger_available);
	return TCL_OK;
    }

    argv++;

    while (*argv) {
	if (streq(*argv,"-now")) {
	    now = TRUE;
	    argv++;
	}
	else break;
    }

    if (!*argv) {
	if (now) {
	    Dbg_On(interp,1);
	    exp_tcl_debugger_available = 1;
	} else {
	    goto usage;
	}
    } else if (streq(*argv,"0")) {
	Dbg_Off(interp);
	exp_tcl_debugger_available = 0;
    } else {
	Dbg_On(interp,now);
	exp_tcl_debugger_available = 1;
    }
    sprintf(interp->result,"%d",exp_tcl_debugger_was_available);
    return(TCL_OK);
 usage:
    exp_error(interp,"usage: [[-now] 1|0]");
    return TCL_ERROR;
}
#endif


/*ARGSUSED*/
int
Exp_ExpInternalCmd(clientData, interp, argc, argv)
    ClientData clientData;
    Tcl_Interp *interp;
    int argc;
    char **argv;
{
    int newChannel = FALSE;
    Tcl_Channel oldChannel;
    static char resultbuf[1000];

    if ((argc > 1) && streq(argv[1],"-info")) {
	resultbuf[0] = '\0';
	oldChannel = expDiagChannelGet();
	if (oldChannel) {
	    sprintf(resultbuf,"-f %s ",expDiagFilename());
	}
	strcat(resultbuf,expDiagToStderrGet()?"1":"0");
	Tcl_SetResult(interp,resultbuf,TCL_STATIC);
	return TCL_OK;
    }

    argv++;
    argc--;

    while (argc) {
	if (!streq(*argv,"-f")) break;
	argc--;argv++;
	if (argc < 1) goto usage;
	expDiagChannelClose(interp);
	if (TCL_OK != expDiagChannelOpen(interp,argv[0])) {
	    return TCL_ERROR;
	}
	newChannel = TRUE;
	argc--;argv++;
    }

    if (argc != 1) goto usage;
    
    /* if no -f given, close file */
    if (!newChannel) {
	expDiagChannelClose(interp);
    }
    expDiagToStderrSet(atoi(*argv));
    return(TCL_OK);
 usage:
    exp_error(interp,"usage: [-f file] 0|1");
    return TCL_ERROR;
}

char *exp_onexit_action = 0;

/*ARGSUSED*/
int
Exp_ExitCmd(clientData, interp, argc, argv)
    ClientData clientData;
    Tcl_Interp *interp;
    int argc;
    char **argv;
{
    int value = 0;

    argv++;

    if (*argv) {
	if (exp_flageq(*argv,"-onexit",3)) {
	    argv++;
	    if (*argv) {
		int len = strlen(*argv);
		if (exp_onexit_action)
			ckfree(exp_onexit_action);
		exp_onexit_action = ckalloc(len + 1);
		strcpy(exp_onexit_action,*argv);
	    } else if (exp_onexit_action) {
		Tcl_AppendResult(interp,exp_onexit_action,(char *)0);
	    }
	    return TCL_OK;
	} else if (exp_flageq(*argv,"-noexit",3)) {
	    argv++;
	    exp_exit_handlers((ClientData)interp);
	    return TCL_OK;
	}
    }

    if (*argv) {
	if (Tcl_GetInt(interp, *argv, &value) != TCL_OK) {
	    return TCL_ERROR;
	}
    }

    Tcl_Exit(value);

    /*NOTREACHED*/
    return TCL_OK;
}

/*ARGSUSED*/
static int
Exp_CloseObjCmd(clientData, interp, objc, objv)
ClientData clientData;
Tcl_Interp *interp;
int objc;
Tcl_Obj *CONST objv[];	/* Argument objects. */
{
    int onexec_flag = FALSE;	/* true if -onexec seen */
    int close_onexec;
    int slave_flag = FALSE;
    ExpState *esPtr = 0;
    char *chanName = 0;

    int objc_orig = objc;
    Tcl_Obj *CONST *objv_orig = objv;

    objc--; objv++;

    for (;objc>0;objc--,objv++) {
	if (streq("-i",Tcl_GetString(*objv))) {
	    objc--; objv++;
	    if (objc == 0) {
		exp_error(interp,"usage: -i spawn_id");
		return(TCL_ERROR);
	    }
	    chanName = Tcl_GetString(*objv);
	} else if (streq(Tcl_GetString(*objv),"-slave")) {
	    slave_flag = TRUE;
	} else if (streq(Tcl_GetString(*objv),"-onexec")) {
	    objc--; objv++;
	    if (objc == 0) {
		exp_error(interp,"usage: -onexec 0|1");
		return(TCL_ERROR);
	    }
	    onexec_flag = TRUE;
	    close_onexec = atoi(Tcl_GetString(*objv));
	} else break;
    }

    if (objc) {
	/* doesn't look like our format, it must be a Tcl-style file */
	/* handle.  Lucky that formats are easily distinguishable. */
	/* Historical note: we used "close"  long before there was a */
	/* Tcl builtin by the same name. */

	Tcl_CmdInfo info;
	Tcl_ResetResult(interp);
	if (0 == Tcl_GetCommandInfo(interp,"close",&info)) {
	    info.clientData = 0;
	}
	return(Tcl_CloseObjCmd(info.clientData,interp,objc_orig,objv_orig));
    }

    if (chanName) {
	if (!(esPtr = expStateFromChannelName(interp,chanName,1,0,0,"close"))) return TCL_ERROR;
    } else {
	if (!(esPtr = expStateCurrent(interp,1,0,0))) return TCL_ERROR;
    }

    if (slave_flag) {
	if (esPtr->fd_slave != EXP_NOFD) {
	    close(esPtr->fd_slave);
	    esPtr->fd_slave = EXP_NOFD;

	    exp_slave_control(esPtr->fdin,1);

	    return TCL_OK;
	} else {
	    exp_error(interp,"no such slave");
	    return TCL_ERROR;
	}
    }

    if (onexec_flag) {
	/* heck, don't even bother to check if fd is open or a real */
	/* spawn id, nothing else depends on it */
//	fcntl(esPtr->fdin,F_SETFD,close_onexec);
	return TCL_OK;
    }

    return(exp_close(interp,esPtr));
}

/*ARGSUSED*/
static void
tcl_tracer(clientData,interp,level,command,cmdProc,cmdClientData,argc,argv)
    ClientData clientData;
    Tcl_Interp *interp;
    int level;
    char *command;
    Tcl_CmdProc *cmdProc;
    ClientData cmdClientData;
    int argc;
    char *argv[];
{
    int i;

    /* come out on stderr, by using expErrorLog */
    expErrorLog("%2d",level);
    for (i = 0;i<level;i++) expErrorLogU("  ");
    expErrorLogU(command);
    expErrorLogU("\r\n");
}

/*ARGSUSED*/
static int
Exp_StraceCmd(clientData, interp, argc, argv)
    ClientData clientData;
    Tcl_Interp *interp;
    int argc;
    char **argv;
{
    static int trace_level = 0;
    static Tcl_Trace trace_handle;

    if (argc > 1 && streq(argv[1],"-info")) {
	sprintf(interp->result,"%d",trace_level);
	return TCL_OK;
    }

    if (argc != 2) {
	exp_error(interp,"usage: trace level");
	return(TCL_ERROR);
    }
    /* tracing already in effect, undo it */
    if (trace_level > 0) Tcl_DeleteTrace(interp,trace_handle);

    /* get and save new trace level */
    trace_level = atoi(argv[1]);
    if (trace_level > 0)
	trace_handle = Tcl_CreateTrace(interp, trace_level, tcl_tracer, NULL);
    return(TCL_OK);
}

/* following defn's are stolen from tclUnix.h */

/*
 * The type of the status returned by wait varies from UNIX system
 * to UNIX system.  The macro below defines it:
 */

#if 0
#ifndef NO_UNION_WAIT
#   define WAIT_STATUS_TYPE union wait
#else
#   define WAIT_STATUS_TYPE int
#endif
#endif /* 0 */

/*
 * following definitions stolen from tclUnix.h
 * (should have been made public!)

 * Supply definitions for macros to query wait status, if not already
 * defined in header files above.
 */

#if 0
#ifndef WIFEXITED
#   define WIFEXITED(stat)  (((*((int *) &(stat))) & 0xff) == 0)
#endif

#ifndef WEXITSTATUS
#   define WEXITSTATUS(stat) (((*((int *) &(stat))) >> 8) & 0xff)
#endif

#ifndef WIFSIGNALED
#   define WIFSIGNALED(stat) (((*((int *) &(stat)))) && ((*((int *) &(stat))) == ((*((int *) &(stat))) & 0x00ff)))
#endif

#ifndef WTERMSIG
#   define WTERMSIG(stat)    ((*((int *) &(stat))) & 0x7f)
#endif

#ifndef WIFSTOPPED
#   define WIFSTOPPED(stat)  (((*((int *) &(stat))) & 0xff) == 0177)
#endif

#ifndef WSTOPSIG
#   define WSTOPSIG(stat)    (((*((int *) &(stat))) >> 8) & 0xff)
#endif
#endif /* 0 */

/* end of stolen definitions */

/* Describe the processes created with Expect's fork.
This allows us to wait on them later.

This is maintained as a linked list.  As additional procs are forked,
new links are added.  As procs disappear, links are marked so that we
can reuse them later.
*/

struct forked_proc {
	int pid;
	WAIT_STATUS_TYPE wait_status;
	enum {not_in_use, wait_done, wait_not_done} link_status;
	struct forked_proc *next;
} *forked_proc_base = 0;

void
fork_clear_all()
{
	struct forked_proc *f;

	for (f=forked_proc_base;f;f=f->next) {
		f->link_status = not_in_use;
	}
}

void
fork_init(f,pid)
struct forked_proc *f;
int pid;
{
	f->pid = pid;
	f->link_status = wait_not_done;
}

/* make an entry for a new proc */
void
fork_add(pid)
int pid;
{
	struct forked_proc *f;

	for (f=forked_proc_base;f;f=f->next) {
		if (f->link_status == not_in_use) break;
	}

	/* add new entry to the front of the list */
	if (!f) {
		f = (struct forked_proc *)ckalloc(sizeof(struct forked_proc));
		f->next = forked_proc_base;
		forked_proc_base = f;
	}
	fork_init(f,pid);
}

/* Provide a last-chance guess for this if not defined already */
#ifndef WNOHANG
#define WNOHANG WNOHANG_BACKUP_VALUE
#endif

/* wait returns are a hodgepodge of things
 If wait fails, something seriously has gone wrong, for example:
   bogus arguments (i.e., incorrect, bogus spawn id)
   no children to wait on
   async event failed
 If wait succeeeds, something happened on a particular pid
   3rd arg is 0 if successfully reaped (if signal, additional fields supplied)
   3rd arg is -1 if unsuccessfully reaped (additional fields supplied)
*/
/*ARGSUSED*/
static int
Exp_WaitCmd(clientData, interp, argc, argv)
ClientData clientData;
Tcl_Interp *interp;
int argc;
char **argv;
{
    char *chanName = 0;
    struct ExpState *esPtr;
    struct forked_proc *fp = 0;	/* handle to a pure forked proc */
    struct ExpState esTmp;	/* temporary memory for either f or fp */
    char spawn_id[20];

    int nowait = FALSE;
    int result = 0;		/* 0 means child was successfully waited on */
				/* -1 means an error occurred */
				/* -2 means no eligible children to wait on */
#define NO_CHILD -2

    argv++;
    argc--;
    for (;argc>0;argc--,argv++) {
	if (streq(*argv,"-i")) {
	    argc--; argv++;
	    if (argc==0) {
		exp_error(interp,"usage: -i spawn_id");
		return(TCL_ERROR);
	    }
	    chanName = *argv;
	} else if (streq(*argv,"-nowait")) {
	    nowait = TRUE;
	}
    }

    if (!chanName) {
	if (!(esPtr = expStateCurrent(interp,0,0,1))) return TCL_ERROR;
    } else {
	if (!(esPtr = expStateFromChannelName(interp,chanName,0,0,1,"wait")))
	    return TCL_ERROR;
    }

    if (!expStateAnyIs(esPtr)) {
	/* check if waited on already */
	/* things opened by "open" or set with -nowait */
	/* are marked sys_waited already */
	if (!esPtr->sys_waited) {
	    if (nowait) {
		/* should probably generate an error */
		/* if SIGCHLD is trapped. */

		/* pass to Tcl, so it can do wait */
		/* in background */
		Tcl_DetachPids(1,(Tcl_Pid *)&esPtr->pid);
		exp_wait_zero(&esPtr->wait);
	    } else {
		while (1) {
		    if (Tcl_AsyncReady()) {
			int rc = Tcl_AsyncInvoke(interp,TCL_OK);
			if (rc != TCL_OK) return(rc);
		    }

		    result = waitpid(esPtr->pid,&esPtr->wait,0);
		    if (result == esPtr->pid) break;
		    if (result == -1) {
			if (errno == EINTR) continue;
			else break;
		    }
		}
	    }
	}

	/*
	 * Now have Tcl reap anything we just detached. 
	 * This also allows procs user has created with "exec &"
	 * and and associated with an "exec &" process to be reaped.
	 */
	
	Tcl_ReapDetachedProcs();
	exp_rearm_sigchld(interp); /* new */

	strcpy(spawn_id,esPtr->name);
    } else {
	/* wait for any of our own spawned processes */
	/* we call waitpid rather than wait to avoid running into */
	/* someone else's processes.  Yes, according to Ousterhout */
	/* this is the best way to do it. */

	int waited_on_forked_process = 0;

	esPtr = expWaitOnAny();
	if (!esPtr) {
	    /* if it's not a spawned process, maybe its a forked process */
	    for (fp=forked_proc_base;fp;fp=fp->next) {
		if (fp->link_status == not_in_use) continue;
	restart:
		result = waitpid(fp->pid,&fp->wait_status,WNOHANG);
		if (result == fp->pid) {
		    waited_on_forked_process = 1;
		    break;
		}
		if (result == 0) continue;	/* busy, try next */
		if (result == -1) {
		    if (errno == EINTR) goto restart;
		    else break;
		}
	    }

	    if (waited_on_forked_process) {
		/*
		 * The literal spawn id in the return value from wait appears
		 * as a -1 to indicate a forked process was waited on.  
		 */
		strcpy(spawn_id,"-1");
	    } else {
		result = NO_CHILD;	/* no children */
		Tcl_ReapDetachedProcs();
	    }
	    exp_rearm_sigchld(interp);
	}
    }

    /*  sigh, wedge forked_proc into an ExpState structure so we don't
     *  have to rewrite remaining code (too much)
     */
    if (fp) {
	esPtr = &esTmp;
	esPtr->pid = fp->pid;
	esPtr->wait = fp->wait_status;
    }

    /* non-portable assumption that pid_t can be printed with %d */

    if (result == -1) {
	sprintf(interp->result,"%d %s -1 %d POSIX %s %s",
		esPtr->pid,spawn_id,errno,Tcl_ErrnoId(),Tcl_ErrnoMsg(errno));
	result = TCL_OK;
    } else if (result == NO_CHILD) {
	exp_error(interp,"no children");
	return TCL_ERROR;
    } else {
	sprintf(interp->result,"%d %s 0 %d",
		esPtr->pid,spawn_id,WEXITSTATUS(esPtr->wait));
	if (WIFSIGNALED(esPtr->wait)) {
	    Tcl_AppendElement(interp,"CHILDKILLED");
	    Tcl_AppendElement(interp,Tcl_SignalId((int)(WTERMSIG(esPtr->wait))));
	    Tcl_AppendElement(interp,Tcl_SignalMsg((int) (WTERMSIG(esPtr->wait))));
	} else if (WIFSTOPPED(esPtr->wait)) {
	    Tcl_AppendElement(interp,"CHILDSUSP");
	    Tcl_AppendElement(interp,Tcl_SignalId((int) (WSTOPSIG(esPtr->wait))));
	    Tcl_AppendElement(interp,Tcl_SignalMsg((int) (WSTOPSIG(esPtr->wait))));
	}
    }
			
    if (fp) {
	fp->link_status = not_in_use;
	return ((result == -1)?TCL_ERROR:TCL_OK);		
    }

    esPtr->sys_waited = TRUE;
    esPtr->user_waited = TRUE;

    /* if user has already called close, forget about this entry entirely */
    if (!esPtr->open) {
      if (esPtr->registered) {
	Tcl_UnregisterChannel(interp,esPtr->channel);
      }
    }

    return ((result == -1)?TCL_ERROR:TCL_OK);
}

/*ARGSUSED*/
static int
Exp_ForkCmd(clientData, interp, argc, argv)
    ClientData clientData;
    Tcl_Interp *interp;
    int argc;
    char **argv;
{
	int rc;
	if (argc > 1) {
		exp_error(interp,"usage: fork");
		return(TCL_ERROR);
	}

	rc = fork();
	if (rc == -1) {
		exp_error(interp,"fork: %s",Tcl_PosixError(interp));
		return TCL_ERROR;
	} else if (rc == 0) {
		/* child */
		exp_forked = TRUE;
		exp_getpid = getpid();
		fork_clear_all();
	} else {
		/* parent */
		fork_add(rc);
	}

	/* both child and parent follow remainder of code */
	sprintf(interp->result,"%d",rc);
	expDiagLog("fork: returns {%s}\r\n",interp->result);
	return(TCL_OK);
}

/*ARGSUSED*/
static int
Exp_DisconnectCmd(clientData, interp, argc, argv)
ClientData clientData;
Tcl_Interp *interp;
int argc;
char **argv;
{
    ThreadSpecificData *tsdPtr = TCL_TSD_INIT(&dataKey);
    

    /* tell CenterLine to ignore non-use of ttyfd */
    /*SUPPRESS 591*/
    int ttyfd;

    if (argc > 1) {
	exp_error(interp,"usage: disconnect");
	return(TCL_ERROR);
    }

    if (exp_disconnected) {
	exp_error(interp,"already disconnected");
	return(TCL_ERROR);
    }
    if (!exp_forked) {
	exp_error(interp,"can only disconnect child process");
	return(TCL_ERROR);
    }
    exp_disconnected = TRUE;

    /* ignore hangup signals generated by testing ptys in getptymaster */
    /* and other places */
//    signal(SIGHUP,SIG_IGN);

    /* reopen prevents confusion between send/expect_user */
    /* accidentally mapping to a real spawned process after a disconnect */

    /* if we're in a child that's about to be disconnected from the
       controlling tty, close and reopen 0, 1, and 2 but associated
       with /dev/null.  This prevents send and expect_user doing
       special things if newly spawned processes accidentally
       get allocated 0, 1, and 2.
    */
	   
    if (isatty(0)) {
	ExpState *stdinout = tsdPtr->stdinout;
	if (stdinout->valid) {
	    exp_close(interp,stdinout);
	    if (stdinout->registered) {
		Tcl_UnregisterChannel(interp,stdinout->channel);
	    }
	}
	open("/dev/null",0);
	open("/dev/null",1);
	/* tsdPtr->stdinout = expCreateChannel(interp,0,1,EXP_NOPID);*/
	/* tsdPtr->stdinout->keepForever = 1;*/
	}
    if (isatty(2)) {
	ExpState *devtty = tsdPtr->devtty;
	
	/* reopen stderr saves error checking in error/log routines. */
	if (devtty->valid) {
	    exp_close(interp,devtty);
	    if (devtty->registered) {
		Tcl_UnregisterChannel(interp,devtty->channel);
	    }
	}
	open("/dev/null",1);
	/* tsdPtr->devtty = expCreateChannel(interp,2,2,EXP_NOPID);*/
	/* tsdPtr->devtty->keepForever = 1;*/
    }

    Tcl_UnsetVar(interp,"tty_spawn_id",TCL_GLOBAL_ONLY);

#ifdef DO_SETSID
    setsid();
#else
#ifdef SYSV3
    /* put process in our own pgrp, and lose controlling terminal */
#ifdef sysV88
    /* With setpgrp first, child ends up with closed stdio */
    /* according to Dave Schmitt <daves@techmpc.csg.gss.mot.com> */
    if (fork()) exit(0);
    expSetpgrp();
#else
    expSetpgrp();
    /*signal(SIGHUP,SIG_IGN); moved out to above */
    if (fork()) exit(0);	/* first child exits (as per Stevens, */
    /* UNIX Network Programming, p. 79-80) */
    /* second child process continues as daemon */
#endif
#else /* !SYSV3 */
    expSetpgrp();

/* Pyramid lacks this defn */
#ifdef TIOCNOTTY
    ttyfd = open("/dev/tty", O_RDWR);
    if (ttyfd >= 0) {
	/* zap controlling terminal if we had one */
	(void) ioctl(ttyfd, TIOCNOTTY, (char *)0);
	(void) close(ttyfd);
    }
#endif /* TIOCNOTTY */

#endif /* SYSV3 */
#endif /* DO_SETSID */
    return(TCL_OK);
}

/*ARGSUSED*/
static int
Exp_OverlayCmd(clientData, interp, argc, argv)
ClientData clientData;
Tcl_Interp *interp;
int argc;
char **argv;
{
	int newfd, oldfd;
	int dash_name = 0;
	char *command;

	argc--; argv++;
	while (argc) {
		if (*argv[0] != '-') break;	/* not a flag */
		if (streq(*argv,"-")) {		/* - by itself */
			argc--; argv++;
			dash_name = 1;
			continue;
		}
		newfd = atoi(argv[0]+1);
		argc--; argv++;
		if (argc == 0) {
			exp_error(interp,"overlay -# requires additional argument");
			return(TCL_ERROR);
		}
		oldfd = atoi(argv[0]);
		argc--; argv++;
		expDiagLog("overlay: mapping fd %d to %d\r\n",oldfd,newfd);
		if (oldfd != newfd) (void) dup2(oldfd,newfd);
		else expDiagLog("warning: overlay: old fd == new fd (%d)\r\n",oldfd);
	}
	if (argc == 0) {
		exp_error(interp,"need program name");
		return(TCL_ERROR);
	}
	command = argv[0];
	if (dash_name) {
		argv[0] = ckalloc(1+strlen(command));
		sprintf(argv[0],"-%s",command);
	}

	signal(SIGINT, SIG_DFL);
//	signal(SIGQUIT, SIG_DFL);
        (void) execvp(command,argv);
	exp_error(interp,"execvp(%s): %s\r\n",argv[0],Tcl_PosixError(interp));
	return(TCL_ERROR);
}

/*ARGSUSED*/
int
Exp_InterpreterObjCmd(clientData, interp, objc, objv)
ClientData clientData;
Tcl_Interp *interp;
int objc;
Tcl_Obj *CONST objv[];		/* Argument objects. */
{
    Tcl_Obj *eofObj = 0;
    int i;
    int index;
    int rc;

    static char *options[] = {
	"-eof", (char *)0
    };
    enum options {
	FLAG_EOF
    };

    for (i = 1; i < objc; i++) {
	if (Tcl_GetIndexFromObj(interp, objv[i], options, "flag", 0,
				&index) != TCL_OK) {
	    return TCL_ERROR;
	}
	switch ((enum options) index) {
	case FLAG_EOF:
	    i++;
	    if (i >= objc) {
		Tcl_WrongNumArgs(interp, 1, objv,"-eof cmd");
		return TCL_ERROR;
	    }
	    eofObj = objv[i];
	    Tcl_IncrRefCount(eofObj);
	    break;
	}
    }

    /* errors and ok, are caught by exp_interpreter() and discarded */
    /* to return TCL_OK, type "return" */
    rc = exp_interpreter(interp,eofObj);
    if (eofObj) Tcl_DecrRefCount(eofObj);
    return rc;
}

/* this command supercede's Tcl's builtin CONTINUE command */
/*ARGSUSED*/
int
Exp_ExpContinueCmd(clientData, interp, argc, argv)
ClientData clientData;
Tcl_Interp *interp;
int argc;
char **argv;
{
    if (argc == 1) {
	return EXP_CONTINUE;
    } else if ((argc == 2) && (0 == strcmp(argv[1],"-continue_timer"))) {
	return EXP_CONTINUE_TIMER;
    }

    exp_error(interp,"usage: exp_continue [-continue_timer]\n");
    return(TCL_ERROR);
}

/* most of this is directly from Tcl's definition for return */
/*ARGSUSED*/
int
Exp_InterReturnObjCmd(clientData, interp, objc, objv)
ClientData clientData;
Tcl_Interp *interp;
int objc;
Tcl_Obj *CONST objv[];
{
    /* let Tcl's return command worry about args */
    /* if successful (i.e., TCL_RETURN is returned) */
    /* modify the result, so that we will handle it specially */

    int result = Tcl_ReturnObjCmd(clientData,interp,objc,objv);
    if (result == TCL_RETURN)
        result = EXP_TCL_RETURN;
    return result;
}

/*ARGSUSED*/
int
Exp_OpenCmd(clientData, interp, argc, argv)
ClientData clientData;
Tcl_Interp *interp;
int argc;
char **argv;
{
    ExpState *esPtr;
    char *chanName = 0;
    int newfd;
    int leaveopen = FALSE;
    Tcl_Channel channel;

    argc--; argv++;

    for (;argc>0;argc--,argv++) {
	if (streq(*argv,"-i")) {
	    argc--; argv++;
	    if (!*argv) {
		exp_error(interp,"usage: -i spawn_id");
		return TCL_ERROR;
	    }
	    chanName = *argv;
	} else if (streq(*argv,"-leaveopen")) {
	    leaveopen = TRUE;
	    argc--; argv++;
	} else break;
    }

    if (!chanName) {
	if (!(esPtr = expStateCurrent(interp,1,0,0))) return TCL_ERROR;
    } else {
	if (!(esPtr = expStateFromChannelName(interp,chanName,1,0,0,"exp_open")))
return TCL_ERROR;
    }

    /* make a new copy of file descriptor */
    if (-1 == (newfd = dup(esPtr->fdin))) {
	exp_error(interp,"dup: %s",Tcl_PosixError(interp));
	return TCL_ERROR;
    }

    if (!leaveopen) {
	/* remove from Expect's memory in anticipation of passing to Tcl */
	if (esPtr->pid != EXP_NOPID) {
	    Tcl_DetachPids(1,(Tcl_Pid *)&esPtr->pid);
	    esPtr->pid = EXP_NOPID;
	    esPtr->sys_waited = esPtr->user_waited = TRUE;
	}
	exp_close(interp,esPtr);
    }

    /*
     * Tcl's MakeFileChannel only allows us to pass a single file descriptor
     * but that shouldn't be a problem in practice since all of the channels
     * that Expect generates only have one fd.  Of course, this code won't
     * work if someone creates a pipeline, then passes it to spawn, and then
     * again to exp_open.  For that to work, Tcl would need a new API.
     * Oh, and we're also being rather cavalier with the permissions here,
     * but they're likely to be right for the same reasons.
     */
    channel = Tcl_MakeFileChannel((ClientData)newfd,TCL_READABLE|TCL_WRITABLE);
    Tcl_RegisterChannel(interp, channel);
    Tcl_AppendResult(interp, Tcl_GetChannelName(channel), (char *) NULL);
    return TCL_OK;
}

/* return 1 if a string is substring of a flag */
/* this version is the code used by the macro that everyone calls */
int
exp_flageq_code(flag,string,minlen)
char *flag;
char *string;
int minlen;		/* at least this many chars must match */
{
	for (;*flag;flag++,string++,minlen--) {
		if (*string == '\0') break;
		if (*string != *flag) return 0;
	}
	if (*string == '\0' && minlen <= 0) return 1;
	return 0;
}

void
exp_create_commands(interp,c)
Tcl_Interp *interp;
struct exp_cmd_data *c;
{
	Namespace *globalNsPtr = (Namespace *) Tcl_GetGlobalNamespace(interp);
	Namespace *currNsPtr   = (Namespace *) Tcl_GetCurrentNamespace(interp);
	char cmdnamebuf[80];

	for (;c->name;c++) {
		/* if already defined, don't redefine */
		if ((c->flags & EXP_REDEFINE) ||
		    !(Tcl_FindHashEntry(&globalNsPtr->cmdTable,c->name) ||
		      Tcl_FindHashEntry(&currNsPtr->cmdTable,c->name))) {
			if (c->objproc)
				Tcl_CreateObjCommand(interp,c->name,
						     c->objproc,c->data,exp_deleteObjProc);
			else
				Tcl_CreateCommand(interp,c->name,c->proc,
						  c->data,exp_deleteProc);
		}
		if (!(c->name[0] == 'e' &&
		      c->name[1] == 'x' &&
		      c->name[2] == 'p')
		    && !(c->flags & EXP_NOPREFIX)) {
			sprintf(cmdnamebuf,"exp_%s",c->name);
			if (c->objproc)
				Tcl_CreateObjCommand(interp,cmdnamebuf,c->objproc,c->data,
						     exp_deleteObjProc);
			else
				Tcl_CreateCommand(interp,cmdnamebuf,c->proc,
						  c->data,exp_deleteProc);
		}
	}
}

static struct exp_cmd_data cmd_data[]  = {
{"close",	Exp_CloseObjCmd,	0,	0,	EXP_REDEFINE},
#ifdef TCL_DEBUGGER
{"debug",	exp_proc(Exp_DebugCmd),	0,	0},
#endif
{"exp_internal",exp_proc(Exp_ExpInternalCmd),	0,	0},
{"disconnect",	exp_proc(Exp_DisconnectCmd),	0,	0},
{"exit",	exp_proc(Exp_ExitCmd),	0,	EXP_REDEFINE},
{"exp_continue",exp_proc(Exp_ExpContinueCmd),0,	0},
{"fork",	exp_proc(Exp_ForkCmd),	0,	0},
{"exp_pid",	exp_proc(Exp_ExpPidCmd),	0,	0},
{"getpid",	exp_proc(Exp_GetpidDeprecatedCmd),0,	0},
{"interpreter",	Exp_InterpreterObjCmd,	0,	0,	0},
{"log_file",	exp_proc(Exp_LogFileCmd),	0,	0},
{"log_user",	exp_proc(Exp_LogUserCmd),	0,	0},
{"exp_open",	exp_proc(Exp_OpenCmd),	0,	0},
{"overlay",	exp_proc(Exp_OverlayCmd),	0,	0},
{"inter_return",Exp_InterReturnObjCmd,	0,	0,	0},
{"send",	Exp_SendObjCmd,		0,	(ClientData)&sendCD_proc,0},
{"send_error",	Exp_SendObjCmd,		0,	(ClientData)&sendCD_error,0},
{"send_log",	exp_proc(Exp_SendLogCmd),	0,	0},
{"send_tty",	Exp_SendObjCmd,		0,	(ClientData)&sendCD_tty,0},
{"send_user",	Exp_SendObjCmd,		0,	(ClientData)&sendCD_user,0},
{"sleep",	exp_proc(Exp_SleepCmd),	0,	0},
{"spawn",	exp_proc(Exp_SpawnCmd),	0,	0},
{"strace",	exp_proc(Exp_StraceCmd),	0,	0},
{"wait",	exp_proc(Exp_WaitCmd),	0,	0},
{0}};

void
exp_init_most_cmds(interp)
Tcl_Interp *interp;
{
    exp_create_commands(interp,cmd_data);

#ifdef HAVE_PTYTRAP
    Tcl_InitHashTable(&slaveNames,TCL_STRING_KEYS);
#endif /* HAVE_PTYTRAP */
}
