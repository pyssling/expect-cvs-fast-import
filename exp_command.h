/* command.h - definitions for expect commands

Written by: Don Libes, NIST, 2/6/90

Design and implementation of this program was paid for by U.S. tax
dollars.  Therefore it is public domain.  However, the author and NIST
would appreciate credit if this program or parts of it are used.
*/

EXTERN ExpState *       expGetCurrentState _ANSI_ARGS_((Tcl_Interp *,int,int));
EXTERN ExpState *       expGetState _ANSI_ARGS_((Tcl_Interp
	*,Tcl_Channel,int,int,char *));

#define EXP_CHANNELNAMELEN (16 + TCL_INTEGER_SPACE)

#ifdef OBSOLETE 
EXTERN struct exp_f *	exp_fd2f _ANSI_ARGS_((Tcl_Interp *,int,int,int,char *));
EXTERN struct exp_f *	exp_update_master
				_ANSI_ARGS_((Tcl_Interp *,int *,int,int));
#endif

EXTERN char *		exp_get_var _ANSI_ARGS_((Tcl_Interp *,char *));

EXTERN int exp_default_match_max;
EXTERN int exp_default_parity;
EXTERN int exp_default_rm_nulls;

EXTERN int		exp_one_arg_braced _ANSI_ARGS_((char *));
EXTERN int		exp_eval_with_one_arg _ANSI_ARGS_((ClientData,
				Tcl_Interp *,char **));
EXTERN void		exp_lowmemcpy _ANSI_ARGS_((char *,char *,int));

EXTERN int exp_flageq_code _ANSI_ARGS_((char *,char *,int));

#define exp_flageq(flag,string,minlen) \
(((string)[0] == (flag)[0]) && (exp_flageq_code(((flag)+1),((string)+1),((minlen)-1))))

/* exp_flageq for single char flags */
#define exp_flageq1(flag,string) \
	((string[0] == flag) && (string[1] == '\0'))

/*
 * The type of the status returned by wait varies from UNIX system
 * to UNIX system.  The macro below defines it:
 * (stolen from tclUnix.h)
 */

#define WAIT_STATUS_TYPE int
#if 0
#ifdef AIX
#   define WAIT_STATUS_TYPE pid_t
#else
#ifndef NO_UNION_WAIT
#   define WAIT_STATUS_TYPE union wait
#else
#   define WAIT_STATUS_TYPE int
#endif
#endif /* AIX */

/* These macros are taken from tclUnix.h */

#undef WIFEXITED
#ifndef WIFEXITED
#   define WIFEXITED(stat)  (((*((int *) &(stat))) & 0xff) == 0)
#endif

#undef WEXITSTATUS
#ifndef WEXITSTATUS
#   define WEXITSTATUS(stat) (((*((int *) &(stat))) >> 8) & 0xff)
#endif

#undef WIFSIGNALED
#ifndef WIFSIGNALED
#   define WIFSIGNALED(stat) (((*((int *) &(stat)))) && ((*((int *) &(stat))) == ((*((int *) &(stat))) & 0x00ff)))
#endif

#undef WTERMSIG
#ifndef WTERMSIG
#   define WTERMSIG(stat)    ((*((int *) &(stat))) & 0x7f)
#endif

#undef WIFSTOPPED
#ifndef WIFSTOPPED
#   define WIFSTOPPED(stat)  (((*((int *) &(stat))) & 0xff) == 0177)
#endif

#undef WSTOPSIG
#ifndef WSTOPSIG
#   define WSTOPSIG(stat)    (((*((int *) &(stat))) >> 8) & 0xff)
#endif

#endif /* 0 */

/* These macros are suggested by the autoconf documentation. */

#undef WIFEXITED
#ifndef WIFEXITED
#   define WIFEXITED(stat)  (((stat) & 0xff) == 0)
#endif

#undef WEXITSTATUS
#ifndef WEXITSTATUS
#   define WEXITSTATUS(stat) (((stat) >> 8) & 0xff)
#endif

#undef WIFSIGNALED
#ifndef WIFSIGNALED
#   define WIFSIGNALED(stat) ((stat) && ((stat) == ((stat) & 0x00ff)))
#endif

#undef WTERMSIG
#ifndef WTERMSIG
#   define WTERMSIG(stat)    ((stat) & 0x7f)
#endif

#undef WIFSTOPPED
#ifndef WIFSTOPPED
#   define WIFSTOPPED(stat)  (((stat) & 0xff) == 0177)
#endif

#undef WSTOPSIG
#ifndef WSTOPSIG
#   define WSTOPSIG(stat)    (((stat) >> 8) & 0xff)
#endif

#define EXP_SPAWN_ID_USER		0
#define EXP_SPAWN_ID_ANY_LIT		"-1"

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
    Tcl_Channel channel;	/* Channel associated with this file. */
    char name[EXP_CHANNELNAMELEN+1]; /* expect and interact set variables
				   to channel name, so for efficiency
				   cache it here */
    int fdin;			/* File handle */
    int fdout;			/* In some special cases (such as stdin), fd
				   (above) is only for input and fd_output is
				   only for output. */
    Tcl_Channel channel_orig;   /* If opened by someone else, i.e. Tcl's open */
    int fd_slave;		/* slave fd if "spawn -pty" used */

    /* this may go away if we find it is not needed */
    /* it might be needed by inherited channels */
    int validMask;		/* OR'ed combination of TCL_READABLE,
				 * TCL_WRITABLE, or TCL_EXCEPTION: indicates
				 * which operations are valid on the file. */

    int pid;		/* pid or EXP_NOPID if no pid */
    Tcl_Obj *buffer;	/* input buffer */

    /* hmm - thought we deleted this, Scott */
    /* Tcl_Obj *lower;	/* input buffer in lowercase */

    /* no longer necessary?
    /* int size;	/* current size of data */

    int msize;	        /* size of buffer (true size is one greater
			   for trailing null) */
    int umsize;	        /* user view of size of buffer */
    int rm_nulls;	/* if nulls should be stripped before pat matching */
    int user_closed;    /* if user has issued "close" command or close has */
    		        /* occurred implicitly */
    int sys_closed;	/* if close() has been called */
    int user_waited;    /* if user has issued "wait" command */
    int sys_waited;	/* if wait() (or variant) has been called */
    WAIT_STATUS_TYPE wait;	/* raw status from wait() */
    int parity;	        /* strip parity if false */
    int printed;	/* # of characters written to stdout (if logging on) */
                        /* but not actually returned via a match yet */
    int echoed;	        /* additional # of chars (beyond "printed" above) */
                        /* echoed back but not actually returned via a match */
                        /* yet.  This supports interact -echo */
    int key;	        /* unique id that identifies what command instance */
                        /* last touched this buffer */
    int force_read;	/* force read to occur (even if buffer already has */
                        /* data).  This supports interact CAN_MATCH */
    int fg_armed;	/* If Tk_CreateFileHandler is active for responding */
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
	disarm_req_while_blocked	/* while blocked, a request */
				/* was received to disarm it.  Rather than */
				/* processing the request immediately, defer */
				/* it so that when we later try to unblock */
				/* we will see at that time that it should */
				/* instead be disarmed */
    } bg_status;
    struct ExpState *nextPtr;	/* Pointer to next file in list of all
				 * file channels. */
} ExpState;

extern Tcl_ChannelType expChannelType;

#define EXP_TEMPORARY	1	/* expect */
#define EXP_PERMANENT	2	/* expect_after, expect_before, expect_bg */

#define EXP_DIRECT	1
#define EXP_INDIRECT	2

EXTERN struct exp_f *exp_fs;

EXTERN void		exp_adjust _ANSI_ARGS_((struct exp_f *));
EXTERN void		exp_buffer_shuffle _ANSI_ARGS_((Tcl_Interp *,struct exp_f *,int,char *,char *));
EXTERN int		exp_close _ANSI_ARGS_((Tcl_Interp *,int));
EXTERN void		exp_close_all _ANSI_ARGS_((Tcl_Interp *));
EXTERN void		exp_ecmd_remove_fd_direct_and_indirect 
				_ANSI_ARGS_((Tcl_Interp *,int));
EXTERN void		exp_trap_on _ANSI_ARGS_((int));
EXTERN int		exp_trap_off _ANSI_ARGS_((char *));

EXTERN void		exp_strftime();

#define exp_deleteProc (void (*)())0
#define exp_deleteObjProc (void (*)())0

EXTERN int expect_key;
EXTERN int exp_configure_count;	/* # of times descriptors have been closed */
				/* or indirect lists have been changed */
EXTERN int exp_nostack_dump;	/* TRUE if user has requested unrolling of */
				/* stack with no trace */

EXTERN void		exp_init_pty _ANSI_ARGS_((void));
EXTERN void		exp_pty_exit _ANSI_ARGS_((void));
EXTERN void		exp_init_tty _ANSI_ARGS_((void));
EXTERN void		exp_init_stdio _ANSI_ARGS_((void));
/*EXTERN void		exp_init_expect _ANSI_ARGS_((Tcl_Interp *));*/
EXTERN void		exp_init_spawn_ids _ANSI_ARGS_((void));
EXTERN void		exp_init_spawn_id_vars _ANSI_ARGS_((Tcl_Interp *));
EXTERN void		exp_init_trap _ANSI_ARGS_((void));
EXTERN void		exp_init_send _ANSI_ARGS_((void));
EXTERN void		exp_init_unit_random _ANSI_ARGS_((void));
EXTERN void		exp_init_sig _ANSI_ARGS_((void));

EXTERN int		exp_tcl2_returnvalue _ANSI_ARGS_((int));
EXTERN int		exp_2tcl_returnvalue _ANSI_ARGS_((int));

EXTERN void		exp_rearm_sigchld _ANSI_ARGS_((Tcl_Interp *));
EXTERN int		exp_string_to_signal _ANSI_ARGS_((Tcl_Interp *,char *));

EXTERN char *exp_onexit_action;

#define exp_new(x)	(x *)malloc(sizeof(x))

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

EXTERN struct exp_i *	exp_new_i_complex _ANSI_ARGS_((Tcl_Interp *,
					char *, int, Tcl_VarTraceProc *));
EXTERN struct exp_i *	exp_new_i_simple _ANSI_ARGS_((int,int));
EXTERN struct exp_state_list *exp_new_state _ANSI_ARGS_((int));
EXTERN void		exp_free_i _ANSI_ARGS_((Tcl_Interp *,struct exp_i *,
					Tcl_VarTraceProc *));
EXTERN void		exp_free_state _ANSI_ARGS_((struct exp_state_list *));
EXTERN void		exp_free_state_single _ANSI_ARGS_((struct exp_state_list *));
EXTERN void		exp_i_update _ANSI_ARGS_((Tcl_Interp *,
					struct exp_i *));

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

EXTERN void		exp_create_commands _ANSI_ARGS_((Tcl_Interp *,
						struct exp_cmd_data *));
EXTERN void		exp_init_main_cmds _ANSI_ARGS_((Tcl_Interp *));
EXTERN void		exp_init_expect_cmds _ANSI_ARGS_((Tcl_Interp *));
EXTERN void		exp_init_most_cmds _ANSI_ARGS_((Tcl_Interp *));
EXTERN void		exp_init_trap_cmds _ANSI_ARGS_((Tcl_Interp *));
EXTERN void		exp_init_interact_cmds _ANSI_ARGS_((Tcl_Interp *));
EXTERN void		exp_init_tty_cmds();

EXTERN ExpState *	expCreateChannel _ANSI_ARGS_((int,int,int));
EXTERN ExpState *	expWaitOnAny _ANSI_ARGS_((Tcl_Interp *));
EXTERN int		expIsStateAny _ANSI_ARGS_((ExpState *));
EXTERN int		expIsStateStdinOut _ANSI_ARGS_((ExpState *));
EXTERN int		expIsStateDevtty _ANSI_ARGS_((ExpState *));
EXTERN ExpState *	expGetChannel _ANSI_ARGS_((Tcl_Interp *,char *));
EXTERN void		expSysClose _ANSI_ARGS_((ExpState *));
EXTERN void		expInitExpectVars _ANSI_ARGS_((void));
EXTERN int		expIsExpChannelName _ANSI_ARGS_((char *));
EXTERN int		expIsStdinout _ANSI_ARGS((ExpState *));
EXTERN ExpState *	expStdinout _ANSI_ARGS((void));
EXTERN ExpState *	expDevtty _ANSI_ARGS((void));


