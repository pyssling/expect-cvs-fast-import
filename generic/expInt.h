/*
 * expInt.h --
 *
 *	Declarations of things used internally by Expect.
 *
 * Written by: Don Libes, libes@cme.nist.gov, NIST, 12/3/90
 *
 * Design and implementation of this program was paid for by U.S. tax
 * dollars.  Therefore it is public domain.  However, the author and NIST
 * would appreciate credit if this program or parts of it are used.
 *
 * Modified in October, 2001 by David Gravereaux for windows.
 *
 * RCS: @(#) $Id: expInt.h,v 1.1.2.1 2001/10/28 01:02:39 davygrvy Exp $
 */

#ifndef _EXPINT_H__
#define _EXPINT_H__

#ifndef _EXP_H__
#   include "exp.h"
#endif

#ifdef HAVE_SYS_WAIT_H
  /* ISC doesn't def WNOHANG unless _POSIX_SOURCE is def'ed */
#   ifdef WNOHANG_REQUIRES_POSIX_SOURCE
#	define _POSIX_SOURCE
#   endif
#   include <sys/wait.h>
#   ifdef WNOHANG_REQUIRES_POSIX_SOURCE
#	undef _POSIX_SOURCE
#   endif
#endif

#ifndef _TCLPORT
#   include <tclPort.h>
#endif


#define EXP_CHANNELNAMELEN	    (16 + TCL_INTEGER_SPACE)
#define exp_flageq(flag,string,minlen) \
	(((string)[0] == (flag)[0]) && (exp_flageq_code(((flag)+1), \
	((string)+1), ((minlen)-1))))

/* exp_flageq for single char flags */
#define exp_flageq1(flag,string) \
	((string[0] == flag) && (string[1] == '\0'))

#define EXP_SPAWN_ID_USER	    0
#define EXP_SPAWN_ID_ANY_LIT	    "-1"
#define EXP_SPAWN_ID_VARNAME	    "spawn_id"

#define EXP_CHANNEL_PREFIX	    "exp"
#define EXP_CHANNEL_PREFIX_LENGTH   3
#define isExpChannelName(name) \
    (0 == strncmp(name,EXP_CHANNEL_PREFIX,EXP_CHANNEL_PREFIX_LENGTH))

#define exp_is_stdinfd(x)	((x) == 0)
#define exp_is_devttyfd(x)	((x) == exp_dev_tty)

#define EXP_NOPID	0	/* Used when there is no associated pid to */
				/* wait for.  For example: */
				/* 1) When fd opened by someone else, e.g., */
				/* Tcl's open */
				/* 2) When entry not in use */
				/* 3) To tell user pid of "spawn -open" */
				/* 4) stdin, out, error */

#define EXP_NOFD	-1

/* these are occasionally useful to distinguish between various expect */
/* commands and are also used as array indices into the per-fd eg[] arrays */
#define EXP_CMD_BEFORE	0
#define EXP_CMD_AFTER	1
#define EXP_CMD_BG	2
#define EXP_CMD_FG	3

/*
 * This structure describes per-instance state of an Exp channel.
 */

typedef struct ExpState {
    Tcl_Channel channel;	    /* Channel associated with this file. */
    char name[EXP_CHANNELNAMELEN+1]; /* expect and interact set variables
					to channel name, so for efficiency
					cache it here */
    int fdin;			    /* input fd */
    int fdout;			    /* output fd - usually the same as fdin,
				       although may be different if channel
				       opened by tcl::open */
    Tcl_Channel channel_orig;	    /* If opened by someone else, i.e. tcl::open */
    int fd_slave;		    /* slave fd if "spawn -pty" used */

    /* this may go away if we find it is not needed */
    /* it might be needed by inherited channels */
    int validMask;		/* OR'ed combination of TCL_READABLE,
				 * TCL_WRITABLE, or TCL_EXCEPTION: indicates
				 * which operations are valid on the file. */

    int pid;		/* pid or EXP_NOPID if no pid */
    Tcl_Obj *buffer;	/* input buffer */

    int msize;	        /* # of bytes that buffer can hold (max) */
    int umsize;	        /* # of bytes (min) that is guaranteed to match */
			/* this comes from match_max command */
    int printed;	/* # of bytes written to stdout (if logging on) */
                        /* but not actually returned via a match yet */
    int echoed;	        /* additional # of bytes (beyond "printed" above) */
                        /* echoed back but not actually returned via a match */
                        /* yet.  This supports interact -echo */

    int rm_nulls;	/* if nulls should be stripped before pat matching */
    int open;		/* if fdin/fdout open */
    int user_waited;    /* if user has issued "wait" command */
    int sys_waited;	/* if wait() (or variant) has been called */
    int registered;	/* if channel registered */
    WAIT_STATUS_TYPE wait;	/* raw status from wait() */
    int parity;	        /* if parity should be preserved */
    int key;	        /* unique id that identifies what command instance */
                        /* last touched this buffer */
    int force_read;	/* force read to occur (even if buffer already has */
                        /* data).  This supports interact CAN_MATCH */
    int notified;	/* If Tcl_NotifyChannel has been called and we */
		        /* have not yet read from the channel. */
    int notifiedMask;	/* Mask reported when notified. */
    int fg_armed;	/* If Tcl_CreateFileHandler is active for responding */
                        /* to foreground events */	   
#ifdef HAVE_PTYTRAP
    char *slave_name;   /* Full name of slave, i.e., /dev/ttyp0 */
#endif /* HAVE_PTYTRAP */
    /* may go away */
    int leaveopen;	/* If we should not call Tcl's close when we close - */
                        /* only relevant if Tcl does the original open */

    Tcl_Interp *bg_interp;	/* interp to process the bg cases */
    int bg_ecount;		/* number of background ecases */
    enum {
	blocked,	/* blocked because we are processing the */
			/* file handler */
	armed,		/* normal state when bg handler in use */
	unarmed,	/* no bg handler in use */
	disarm_req_while_blocked /* while blocked, a request 
				    was received to disarm it.  Rather than 
				    processing the request immediately, defer
				    it so that when we later try to unblock
				    we will see at that time that it should
				    instead be disarmed */
    } bg_status;

    /*
     * If the channel is freed while in the middle of a bg event handler,
     * remember that and defer freeing of the ExpState structure until
     * it is safe.
     */
    int freeWhenBgHandlerUnblocked;

    /* If channel is closed but not yet waited on, we tie up the fd by
     * attaching it to /dev/null.  We play this little game so that we
     * can embed the fd in the channel name.  If we didn't tie up the
     * fd, we'd get channel name collisions.  I'd consider naming the
     * channels independently of the fd, but this makes debugging easier.
     */
    int fdBusy;

    /* 
     * stdinout and stderr never go away so that our internal refs to them
     * don't have to be invalidated.  Having to worry about invalidating them
     * would be a major pain.  */
    int keepForever;

    /*  Remember that "reserved" esPtrs are no longer in use. */
    int valid;
    
    struct ExpState *nextPtr;	/* Pointer to next file in list of all
				 * file channels. */
} ExpState;

#define EXP_SPAWN_ID_BAD	((ExpState *)0)

#define EXP_TIME_INFINITY	-1


/*
 *----------------------------------------------------------------
 * Functions shared among Exp modules but not used by the outside
 * world:
 *----------------------------------------------------------------
 */

void exp_ecmd_remove_state_direct_and_indirect _ANSI_ARGS_((
			    Tcl_Interp *interp, ExpState *esPtr));

#include "expIntDecls.h"

#endif /* _EXPINT_H__ */