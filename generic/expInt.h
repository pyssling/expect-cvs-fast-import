/* ----------------------------------------------------------------------------
 * expInt.h --
 *
 *	Declarations of things used internally by Expect.
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
 *	work by David Gravereaux <davygrvy@pobox.com> for any Win32 OS.
 *
 * ----------------------------------------------------------------------------
 * URLs:    http://expect.nist.gov/
 *	    http://expect.sf.net/
 *	    http://bmrc.berkeley.edu/people/chaffee/expectnt.html
 * ----------------------------------------------------------------------------
 * RCS: @(#) $Id: expInt.h,v 1.1.4.2 2002/02/10 10:17:04 davygrvy Exp $
 * ----------------------------------------------------------------------------
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
#if defined(BUILD_spawndriver)
#   define TCL_STORAGE_CLASS
#elif defined(BUILD_exp)
#   define TCL_STORAGE_CLASS DLLEXPORT
#else
#   ifdef USE_EXP_STUBS
#	define TCL_STORAGE_CLASS
#   else
#	define TCL_STORAGE_CLASS DLLIMPORT
#   endif
#endif

/*
 * This is a convenience macro used to initialize a thread local storage ptr.
 * Stolen from tclInt.h
 */
#ifndef TCL_TSD_INIT
#define TCL_TSD_INIT(keyPtr)	(ThreadSpecificData *)Tcl_GetThreadData((keyPtr), sizeof(ThreadSpecificData))
#endif


#define EXP_SPAWN_ID_VARNAME		"spawn_id"
#define EXP_SPAWN_OUT			"spawn_out"

#define EXP_SPAWN_ID_ANY_VARNAME	"any_spawn_id"
#define EXP_SPAWN_ID_ANY		"exp_any"

#define EXP_SPAWN_ID_ERROR		"stderr"
#define EXP_SPAWN_ID_USER		"exp_user"

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

#define streq(x,y)	(0 == strcmp((x),(y)))
#define dprintify(x)	((exp_is_debugging || exp_debugfile)?exp_printify(x):0)

#define exp_flageq(flag,string,minlen) \
(((string)[0] == (flag)[0]) && (exp_flageq_code(((flag)+1),((string)+1),((minlen)-1))))

/* exp_flageq for single char flags */
#define exp_flageq1(flag,string) \
	((string[0] == flag) && (string[1] == '\0'))


/* each process is associated with a 'struct exp_f'.  An array of these */
/* ('exp_fs') keeps track of all processes.  They are indexed by the true fd */
/* to the master side of the pty */
struct exp_f {
	char *spawnId;	/* Spawn identifier name */
	Tcl_HashEntry *hashPtr;	/* The hash entry with this structure */
	Tcl_Interp *interp;
	int pid;	/* pid or EXP_NOPID if no pid */
	Tcl_Pid tclPid;	/* The pid that tcl wants */
	char *buffer;	/* input buffer */
	char *lower;	/* input buffer in lowercase */
	int size;	/* current size of data */
	int msize;	/* size of buffer (true size is one greater
			 * for trailing null) */
	int umsize;	/* user view of size of buffer */
	int rm_nulls;	/* if nulls should be stripped before pat matching */
	int valid;	/* if any of the other fields should be believed */
	int user_closed;/* if user has issued "close" command or close has */
			/* occurred implicitly */
	int user_waited;/* if user has issued "wait" command */
	int sys_waited;	/* if wait() (or variant) has been called */
	WAIT_STATUS_TYPE wait;	/* raw status from wait() */
	int parity;	/* strip parity if false */
	int printed;	/* # of characters written to stdout (if logging on) */
			/* but not actually returned via a match yet */
	int echoed;	/* additional # of chars (beyond "printed" above) */
			/* echoed back but not actually returned via a match */
			/* yet.  This supports interact -echo */
	int key;	/* unique id that identifies what command instance */
			/* last touched this buffer */
	int force_read;	/* force read to occur (even if buffer already has */
			/* data).  This supports interact CAN_MATCH */
	int fg_armed;	/* If Tk_CreateFileHandler is active for responding */
			/* to foreground events */
#ifdef __WIN32__
	OVERLAPPED over;	/* Overlapped result */
#endif
	Tcl_Channel channel;	/* Tcl channel */
	Tcl_Channel Master;	/* corresponds to master fd */
	/*
	 *  explicit fds aren't necessary now, but since the code is already
	 *  here from before Tcl required TclFile, we'll continue using
	 *  the old fds.  If we ever port this code to a non-UNIX system,
	 *  we'll dump the fds totally.
	 */
	   
	int slave_fd;	/* slave fd if "spawn -pty" used */
#ifdef HAVE_PTYTRAP
	char *slave_name;/* Full name of slave, i.e., /dev/ttyp0 */
#endif /* HAVE_PTYTRAP */
	int leaveopen;	/* If we should not call Tcl's close when we close -
			 * only relevant if Tcl does the original open.  It
			 * also serves as a ref count to how many times this
			 * channel has been opened with spawn -leaveopen */
	int alwaysopen;	/* Set if this is identifier that should always exist */
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

	int matched;		/* Chars matched.  Used by expectlib */
	Tcl_ChannelProc *event_proc; /* Currently installed channel handler */
	ClientData event_data; /* Argument that was installed */
};

struct exp_fs_list {
	struct exp_f *f;
	struct exp_fs_list *next;
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

	struct exp_fs_list *fs_list;
	struct exp_i *next;
};

#define EXP_TEMPORARY	1	/* expect */
#define EXP_PERMANENT	2	/* expect_after, expect_before, expect_bg */
#define EXP_DIRECT	1
#define EXP_INDIRECT	2


/*
 * definitions for creating commands
 */

#define EXP_NOPREFIX	    1	/* don't define with "exp_" prefix */
#define EXP_REDEFINE	    2	/* stomp on old commands with same name */
#define exp_proc(cmdproc)   0, cmdproc
#define exp_deleteProc	    ((Tcl_CmdDeleteProc *) NULL)
#define exp_deleteProc	    ((Tcl_CmdDeleteProc *) NULL)

struct exp_cmd_data {
    char	    *name;
    Tcl_ObjCmdProc  *objproc;
    Tcl_CmdProc	    *proc;
    ClientData	    data;
    int		    flags;
};



#define EXP_TEMPORARY	1	/* expect */
#define EXP_PERMANENT	2	/* expect_after, expect_before, expect_bg */

#define EXP_DIRECT	1
#define EXP_INDIRECT	2

typedef struct {
    Tcl_Channel channelPtr;
    int		toWrite;
} ExpSpawnState;


/*
 * ----------------------------------------
 * Global variables that are externalized.
 * ----------------------------------------
 */


/* Table of struct exp_f */
TCL_EXTERN(Tcl_HashTable *) exp_f_table;

TCL_EXTERN(char *)	    exp_onexit_action;
TCL_EXTERN(Tcl_Channel)	    exp_debugfile;
TCL_EXTERN(Tcl_Channel)	    exp_logfile;
TCL_EXTERN(int)		    exp_logfile_all;
TCL_EXTERN(int)		    exp_loguser;
/* useful to know to avoid debug calls */
TCL_EXTERN(int)		    exp_is_debugging;
TCL_EXTERN(struct exp_f *)  exp_f_any;
TCL_EXTERN(int)		    exp_default_match_max;
TCL_EXTERN(int)		    exp_default_parity;
TCL_EXTERN(int)		    exp_default_rm_nulls;
TCL_EXTERN(struct exp_f *)  exp_dev_tty;
TCL_EXTERN(char *)	    exp_dev_tty_id;
TCL_EXTERN(int)		    exp_stdin_is_tty;
TCL_EXTERN(int)		    exp_stdout_is_tty;
/* procedure to close files in child */
TCL_EXTERN(void)	    (*exp_close_in_child) _ANSI_ARGS_((void));
/* place to pass a string generated */
TCL_EXTERN(char *)	    exp_pty_error;
/* pid of Expect itself */
TCL_EXTERN(int)		    exp_getpid;
TCL_EXTERN(Tcl_Interp *)    exp_interp;
TCL_EXTERN(void)	    (*exp_event_exit) _ANSI_ARGS_((Tcl_Interp *interp));
/* # of times descriptors have been closed
 * or indirect lists have been changed */
TCL_EXTERN(int)		    exp_configure_count;	
/* TRUE if user has requested unrolling of
 * stack with no trace */
TCL_EXTERN(int)		    exp_nostack_dump;
TCL_EXTERN(int)		    expect_key;

/* protos not yet moved to the Stubs table */
//TCL_EXTERN(int)		exp_fcheck _ANSI_ARGS_((Tcl_Interp *, struct exp_f *,int,int,char *));
//TCL_EXTERN(void)		exp_buffer_shuffle _ANSI_ARGS_((Tcl_Interp *,struct exp_f *,int,char *,char *));
//TCL_EXTERN(int)		exp_close_fd _ANSI_ARGS_((Tcl_Interp *,int));
//TCL_EXTERN(void)		exp_close_all _ANSI_ARGS_((Tcl_Interp *));
//TCL_EXTERN(void)		exp_trap_on _ANSI_ARGS_((int));
//TCL_EXTERN(int)		exp_trap_off _ANSI_ARGS_((char *));
/*EXTERN(void)		exp_init_expect _ANSI_ARGS_((Tcl_Interp *));*/
//TCL_EXTERN(int)		exp_tcl2_returnvalue _ANSI_ARGS_((int));
//TCL_EXTERN(int)		exp_2tcl_returnvalue _ANSI_ARGS_((int));
//TCL_EXTERN(int)		exp_string_to_signal _ANSI_ARGS_((Tcl_Interp *,char *));

#include "expIntDecls.h"

#undef TCL_STORAGE_CLASS
#define TCL_STORAGE_CLASS DLLIMPORT

#endif /* _EXPINT */