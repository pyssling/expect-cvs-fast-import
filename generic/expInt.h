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
 * RCS: @(#) $Id: expInt.h,v 1.1.2.2 2001/10/28 01:46:31 davygrvy Exp $
 */

#ifndef _EXPINT
#define _EXPINT

#ifndef _EXP
#   include "exp.h"
#endif

#ifndef _TCLPORT
#   include "tclPort.h"
#endif


#undef TCL_STORAGE_CLASS
#ifdef BUILD_exp
#   define TCL_STORAGE_CLASS DLLEXPORT
#else
#   ifdef USE_EXP_STUBS
#	define TCL_STORAGE_CLASS
#   else
#	define TCL_STORAGE_CLASS DLLIMPORT
#   endif
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

#define exp_new(x)		(x *)ckalloc(sizeof(x))

struct exp_state_list {
	ExpState *esPtr;
	struct exp_state_list *next;
};

/* describes a -i flag */
struct exp_i {
	int cmdtype;	/* EXP_CMD_XXX.  When an indirect update is */
			/* triggered by Tcl, this helps tell us in what */
			/* exp_i list to look in. */
	int direct;	/* if EXP_DIRECT, then the spawn ids have been given */
			/* literally, else indirectly through a variable */
	int duration;	/* if EXP_PERMANENT, char ptrs here had to be */
			/* malloc'd because Tcl command line went away - */
			/* i.e., in expect_before/after */
	char *variable;
	char *value;	/* if type == direct, this is the string that the */
			/* user originally supplied to the -i flag.  It may */
			/* lose relevance as the fd_list is manipulated */
			/* over time.  If type == direct, this is  the */
			/* cached value of variable use this to tell if it */
			/* has changed or not, and ergo whether it's */
			/* necessary to reparse. */

	int ecount;	/* # of ecases this is used by */

	struct exp_state_list *state_list;
	struct exp_i *next;
};

#define EXP_TEMPORARY	1	/* expect */
#define EXP_PERMANENT	2	/* expect_after, expect_before, expect_bg */
#define EXP_DIRECT	1
#define EXP_INDIRECT	2


/*
 * definitions for creating commands
 */

#define EXP_NOPREFIX	1	/* don't define with "exp_" prefix */
#define EXP_REDEFINE	2	/* stomp on old commands with same name */

#define exp_proc(cmdproc) 0, cmdproc

struct exp_cmd_data {
	char		*name;
	Tcl_ObjCmdProc	*objproc;
	Tcl_CmdProc	*proc;
	ClientData	data;
	int 		flags;
};
#define exp_deleteProc	    NULL
#define exp_deleteObjProc   NULL


#define streq(x,y)	(0 == strcmp((x),(y)))


/* Global variables */
extern int exp_default_match_max;
extern int exp_default_parity;
extern int exp_default_rm_nulls;
extern Tcl_ChannelType expSpawnChanType;
extern int expect_key;
extern int exp_configure_count;	/* # of times descriptors have been closed
				   or indirect lists have been changed */
extern int exp_nostack_dump;	/* TRUE if user has requested unrolling of
				   stack with no trace */
extern char *exp_onexit_action;
extern int exp_cmdlinecmds;
extern int exp_interactive;
extern FILE *exp_cmdfile;
extern char *exp_cmdfilename;
extern int exp_getpid;	/* pid of Expect itself */
extern int exp_buffer_command_input;
extern int exp_tcl_debugger_available;
extern Tcl_Interp *exp_interp;
extern void (*exp_event_exit) _ANSI_ARGS_((Tcl_Interp *));
/*
 * Everything below here should eventually be moved into expect.h
 * and Expect-thread-safe variables.
 */

extern char *exp_pty_error;		/* place to pass a string generated */
					/* deep in the innards of the pty */
					/* code but needed by anyone */
extern int exp_disconnected;		/* proc. disc'd from controlling tty */


/* protos not yet moved to the Stubs table */
TCL_EXTERN(int)		exp_getpidproc _ANSI_ARGS_((void));
TCL_EXTERN(char *)	exp_get_var _ANSI_ARGS_((Tcl_Interp *,char *));
TCL_EXTERN(int)		exp_one_arg_braced _ANSI_ARGS_((Tcl_Obj *));
TCL_EXTERN(int)		exp_eval_with_one_arg _ANSI_ARGS_((ClientData,
				Tcl_Interp *, struct Tcl_Obj * CONST objv[]));
TCL_EXTERN(void)	exp_lowmemcpy _ANSI_ARGS_((char *,char *,int));
TCL_EXTERN(int)		exp_flageq_code _ANSI_ARGS_((char *,char *,int));
TCL_EXTERN(void)	expAdjust _ANSI_ARGS_((ExpState *));
TCL_EXTERN(void)	exp_buffer_shuffle _ANSI_ARGS_((Tcl_Interp *,ExpState *,int,char *,char *));
TCL_EXTERN(int)		exp_close _ANSI_ARGS_((Tcl_Interp *,ExpState *));
TCL_EXTERN(void)	exp_close_all _ANSI_ARGS_((Tcl_Interp *));
//TCL_EXTERN(void)	exp_ecmd_remove_fd_direct_and_indirect 
//				_ANSI_ARGS_((Tcl_Interp *,int));
TCL_EXTERN(void)	exp_trap_on _ANSI_ARGS_((int));
TCL_EXTERN(int)		exp_trap_off _ANSI_ARGS_((char *));
TCL_EXTERN(void)	exp_strftime _ANSI_ARGS_((char *format, const struct tm *timeptr,Tcl_DString *dstring));
TCL_EXTERN(void)	exp_init_pty _ANSI_ARGS_((void));
TCL_EXTERN(void)	exp_pty_exit _ANSI_ARGS_((void));
TCL_EXTERN(void)	exp_init_tty _ANSI_ARGS_((void));
TCL_EXTERN(void)	exp_init_stdio _ANSI_ARGS_((void));
//TCL_EXTERN(void)	exp_init_expect _ANSI_ARGS_((Tcl_Interp *));
TCL_EXTERN(void)	exp_init_spawn_ids _ANSI_ARGS_((Tcl_Interp *));
TCL_EXTERN(void)	exp_init_spawn_id_vars _ANSI_ARGS_((Tcl_Interp *));
TCL_EXTERN(void)	exp_init_trap _ANSI_ARGS_((void));
TCL_EXTERN(void)	exp_init_send _ANSI_ARGS_((void));
TCL_EXTERN(void)	exp_init_unit_random _ANSI_ARGS_((void));
TCL_EXTERN(void)	exp_init_sig _ANSI_ARGS_((void));
TCL_EXTERN(void)	expChannelInit _ANSI_ARGS_((void));
TCL_EXTERN(int)		expChannelCountGet _ANSI_ARGS_((void));
TCL_EXTERN(int)		exp_tcl2_returnvalue _ANSI_ARGS_((int));
TCL_EXTERN(int)		exp_2tcl_returnvalue _ANSI_ARGS_((int));
TCL_EXTERN(void)	exp_rearm_sigchld _ANSI_ARGS_((Tcl_Interp *));
TCL_EXTERN(int)		exp_string_to_signal _ANSI_ARGS_((Tcl_Interp *,char *));
TCL_EXTERN(struct exp_i *) exp_new_i_complex _ANSI_ARGS_((Tcl_Interp *,
					char *, int, Tcl_VarTraceProc *));
TCL_EXTERN(struct exp_i *) exp_new_i_simple _ANSI_ARGS_((ExpState *,int));
TCL_EXTERN(struct exp_state_list *) exp_new_state _ANSI_ARGS_((ExpState *));
TCL_EXTERN(void)	exp_free_i _ANSI_ARGS_((Tcl_Interp *,struct exp_i *,
					Tcl_VarTraceProc *));
TCL_EXTERN(void)	exp_free_state _ANSI_ARGS_((struct exp_state_list *));
TCL_EXTERN(void)	exp_free_state_single _ANSI_ARGS_((struct exp_state_list *));
TCL_EXTERN(void)	exp_i_update _ANSI_ARGS_((Tcl_Interp *,
					struct exp_i *));
TCL_EXTERN(void)	exp_create_commands _ANSI_ARGS_((Tcl_Interp *,
						struct exp_cmd_data *));
TCL_EXTERN(void)	exp_init_main_cmds _ANSI_ARGS_((Tcl_Interp *));
TCL_EXTERN(void)	exp_init_expect_cmds _ANSI_ARGS_((Tcl_Interp *));
TCL_EXTERN(void)	exp_init_most_cmds _ANSI_ARGS_((Tcl_Interp *));
TCL_EXTERN(void)	exp_init_trap_cmds _ANSI_ARGS_((Tcl_Interp *));
TCL_EXTERN(void)	exp_init_interact_cmds _ANSI_ARGS_((Tcl_Interp *));
TCL_EXTERN(void)	exp_init_tty_cmds();

TCL_EXTERN(ExpState *)	expStateCheck _ANSI_ARGS_((Tcl_Interp *,ExpState *,int,int,char *));
TCL_EXTERN(ExpState *)  expStateCurrent _ANSI_ARGS_((Tcl_Interp *,int,int,int));
TCL_EXTERN(ExpState *)  expStateFromChannelName _ANSI_ARGS_((Tcl_Interp *,char *,int,int,int,char *));
TCL_EXTERN(void)	expStateFree _ANSI_ARGS_((ExpState *));

TCL_EXTERN(ExpState *)	expCreateChannel _ANSI_ARGS_((Tcl_Interp *,int,int,int));
TCL_EXTERN(ExpState *)	expWaitOnAny _ANSI_ARGS_((void));
TCL_EXTERN(ExpState *)	expWaitOnOne _ANSI_ARGS_((void));
TCL_EXTERN(void)	expExpectVarsInit _ANSI_ARGS_((void));
TCL_EXTERN(int)		expStateAnyIs _ANSI_ARGS_((ExpState *));
TCL_EXTERN(int)		expDevttyIs _ANSI_ARGS_((ExpState *));
TCL_EXTERN(int)		expStdinOutIs _ANSI_ARGS_((ExpState *));
TCL_EXTERN(ExpState *)	expStdinoutGet _ANSI_ARGS_((void));
TCL_EXTERN(ExpState *)	expDevttyGet _ANSI_ARGS_((void));
TCL_EXTERN(int)		expSizeGet _ANSI_ARGS_((ExpState *));
TCL_EXTERN(int)		expSizeZero _ANSI_ARGS_((ExpState *));

/* for exp_event.c */
TCL_EXTERN(int)		exp_get_next_event _ANSI_ARGS_((Tcl_Interp *,ExpState **, int, ExpState **, int, int));
TCL_EXTERN(int)		exp_get_next_event_info _ANSI_ARGS_((Tcl_Interp *, ExpState *));
TCL_EXTERN(int)		exp_dsleep _ANSI_ARGS_((Tcl_Interp *, double));
TCL_EXTERN(void)	exp_init_event _ANSI_ARGS_((void));
//extern void (*exp_event_exit) _ANSI_ARGS_((Tcl_Interp *));
TCL_EXTERN(void)	exp_event_disarm _ANSI_ARGS_((ExpState *,Tcl_FileProc *));
TCL_EXTERN(void)	exp_event_disarm_bg _ANSI_ARGS_((ExpState *));
TCL_EXTERN(void)	exp_event_disarm_fg _ANSI_ARGS_((ExpState *));
TCL_EXTERN(void)	exp_arm_background_channelhandler _ANSI_ARGS_((ExpState *));
TCL_EXTERN(void)	exp_disarm_background_channelhandler _ANSI_ARGS_((ExpState *));
TCL_EXTERN(void)	exp_disarm_background_channelhandler_force _ANSI_ARGS_((ExpState *));
TCL_EXTERN(void)	exp_unblock_background_channelhandler _ANSI_ARGS_((ExpState *));
TCL_EXTERN(void)	exp_block_background_channelhandler _ANSI_ARGS_((ExpState *));
TCL_EXTERN(void)	exp_background_channelhandler _ANSI_ARGS_((ClientData,int));


/* for exp_log.h */
TCL_EXTERN(void)	expDiagInit _ANSI_ARGS_((void));
TCL_EXTERN(int)		expDiagChannelOpen _ANSI_ARGS_((Tcl_Interp *,char *));
TCL_EXTERN(Tcl_Channel)	expDiagChannelGet _ANSI_ARGS_((void));
TCL_EXTERN(void)	expDiagChannelClose _ANSI_ARGS_((Tcl_Interp *));
TCL_EXTERN(char *)	expDiagFilename _ANSI_ARGS_((void));
TCL_EXTERN(int)		expDiagToStderrGet _ANSI_ARGS_((void));
TCL_EXTERN(void)	expDiagToStderrSet _ANSI_ARGS_((int));
TCL_EXTERN(void)	expDiagWriteBytes _ANSI_ARGS_((char *,int));
TCL_EXTERN(void)	expDiagWriteChars _ANSI_ARGS_((char *,int));
TCL_EXTERN(void)	expDiagWriteObj _ANSI_ARGS_((Tcl_Obj *));
TCL_EXTERN(void)	expDiagLog _ANSI_ARGS_(TCL_VARARGS(char *,fmt));
TCL_EXTERN(void)	expDiagLogU _ANSI_ARGS_((char *));
TCL_EXTERN(char *)	expPrintify _ANSI_ARGS_((char *));
TCL_EXTERN(char *)	expPrintifyObj _ANSI_ARGS_((Tcl_Obj *));
TCL_EXTERN(void)	expLogInit _ANSI_ARGS_((void));
TCL_EXTERN(int)		expLogChannelOpen _ANSI_ARGS_((Tcl_Interp *,char *,int));
TCL_EXTERN(Tcl_Channel)	expLogChannelGet _ANSI_ARGS_((void));
TCL_EXTERN(int)		expLogChannelSet _ANSI_ARGS_((Tcl_Interp *,char *));
TCL_EXTERN(void)	expLogChannelClose _ANSI_ARGS_((Tcl_Interp *));
TCL_EXTERN(char *)	expLogFilenameGet _ANSI_ARGS_((void));
TCL_EXTERN(void)	expLogAppendSet _ANSI_ARGS_((int));
TCL_EXTERN(int)		expLogAppendGet _ANSI_ARGS_((void));
TCL_EXTERN(void)	expLogLeaveOpenSet _ANSI_ARGS_((int));
TCL_EXTERN(int)		expLogLeaveOpenGet _ANSI_ARGS_((void));
TCL_EXTERN(void)	expLogAllSet _ANSI_ARGS_((int));
TCL_EXTERN(int)		expLogAllGet _ANSI_ARGS_((void));
TCL_EXTERN(void)	expLogToStdoutSet _ANSI_ARGS_((int));
TCL_EXTERN(int)		expLogToStdoutGet _ANSI_ARGS_((void));
TCL_EXTERN(void)	expLogDiagU _ANSI_ARGS_((char *));
TCL_EXTERN(int)		expWriteBytesAndLogIfTtyU _ANSI_ARGS_((ExpState *,char *,int));
TCL_EXTERN(int)		expLogUserGet _ANSI_ARGS_((void));
TCL_EXTERN(void)	expLogUserSet _ANSI_ARGS_((int));
TCL_EXTERN(void)	expLogInteractionU _ANSI_ARGS_((ExpState *,char *));



TCL_EXTERN(void)	exp_ecmd_remove_state_direct_and_indirect _ANSI_ARGS_((
			    Tcl_Interp *interp, ExpState *esPtr));

Tcl_ObjCmdProc Exp_CloseObjCmd;
Tcl_CmdProc Exp_ExpInternalCmd;
Tcl_CmdProc Exp_DisconnectCmd;
Tcl_CmdProc Exp_ExitCmd;
Tcl_CmdProc Exp_ExpContinueCmd;
Tcl_CmdProc Exp_ForkCmd;
Tcl_CmdProc Exp_ExpPidCmd;
Tcl_CmdProc Exp_GetpidDeprecatedCmd;
Tcl_ObjCmdProc Exp_InterpreterObjCmd;
Tcl_CmdProc Exp_LogFileCmd;
Tcl_CmdProc Exp_LogUserCmd;
Tcl_CmdProc Exp_OpenCmd;
Tcl_CmdProc Exp_OverlayCmd;
Tcl_ObjCmdProc Exp_InterReturnObjCmd;
Tcl_ObjCmdProc Exp_SendObjCmd;
Tcl_CmdProc Exp_SendLogCmd;
Tcl_CmdProc Exp_SleepCmd;
Tcl_CmdProc Exp_SpawnCmd;
Tcl_CmdProc Exp_StraceCmd;
Tcl_CmdProc Exp_WaitCmd;


/*
 *----------------------------------------------------------------
 * Functions shared among Exp modules but not used by the outside
 * world:
 *----------------------------------------------------------------
 */
extern void	expErrorLog _ANSI_ARGS_(TCL_VARARGS(char *,fmt));
extern void	expErrorLogU _ANSI_ARGS_((char *));
extern void	expStdoutLog _ANSI_ARGS_(TCL_VARARGS(int,force_stdout));
extern void	expStdoutLogU _ANSI_ARGS_((char *buf, int force_stdout));
extern void	exp_debuglog _ANSI_ARGS_(TCL_VARARGS(char *,fmt));

#include "expIntDecls.h"

#undef TCL_STORAGE_CLASS
#define TCL_STORAGE_CLASS DLLIMPORT

#endif /* _EXPINT */