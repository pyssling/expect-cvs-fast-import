/* expect.c - expect commands

Written by: Don Libes, NIST, 2/6/90

Design and implementation of this program was paid for by U.S. tax
dollars.  Therefore it is public domain.  However, the author and NIST
would appreciate credit if this program or parts of it are used.

*/

#include <sys/types.h>
#include <stdio.h>
#include <signal.h>
#include <errno.h>
#include <ctype.h>	/* for isspace */
#include <time.h>	/* for time(3) */
#if 0
#include <setjmp.h>
#endif

#include "expect_cf.h"

#ifdef HAVE_SYS_WAIT_H
#include <sys/wait.h>
#endif

#ifdef HAVE_UNISTD_H
# include <unistd.h>
#endif

#include "tcl.h"

#include "string.h"

#include "exp_rename.h"
#include "exp_prog.h"
#include "exp_command.h"
#include "exp_log.h"
#include "exp_event.h"
#include "exp_tty.h"
#include "exp_tstamp.h"	/* this should disappear when interact */
			/* loses ref's to it */
#ifdef TCL_DEBUGGER
#include "tcldbg.h"
#endif

/* initial length of strings that we can guarantee patterns can match */
int exp_default_match_max =	2000;
#define INIT_EXPECT_TIMEOUT_LIT	"10"	/* seconds */
#define INIT_EXPECT_TIMEOUT	10	/* seconds */
int exp_default_parity =	TRUE;
int exp_default_rm_nulls =	TRUE;

/* user variable names */
#define EXPECT_TIMEOUT		"timeout"
#define EXPECT_OUT		"expect_out"

typedef struct ThreadSpecificData {
    /*
     * List of all exp channels currently open.  This is per thread and is
     * used to match up fd's to channels, which rarely occurs.
     */
    int timeout;
} ThreadSpecificData;

static Tcl_ThreadDataKey dataKey;

/* 1 ecase struct is reserved for each case in the expect command.  Note that
eof/timeout don't use any of theirs, but the algorithm is simpler this way. */

struct ecase {	/* case for expect command */
	struct exp_i	*i_list;
	Tcl_Obj *pat;	/* original pattern spec */
	Tcl_Obj *body;	/* ptr to body to be executed upon match */
#define PAT_EOF		1
#define PAT_TIMEOUT	2
#define PAT_DEFAULT	3
#define PAT_FULLBUFFER	4
#define PAT_GLOB	5 /* glob-style pattern list */
#define PAT_RE		6 /* regular expression */
#define PAT_EXACT	7 /* exact string */
#define PAT_NULL	8 /* ASCII 0 */
#define PAT_TYPES	9 /* used to size array of pattern type descriptions */
	int use;	/* PAT_XXX */
	int simple_start;/* offset from start of buffer denoting where a */
			/* glob or exact match begins */
	int transfer;	/* if false, leave matched chars in input stream */
	int indices;	/* if true, write indices */
/*	int iwrite;*/	/* if true write spawn_id */
	int iread;	/* if true, reread indirects */
	int timestamp;	/* if true, write timestamps */
#define CASE_UNKNOWN	0
#define CASE_NORM	1
#define CASE_LOWER	2
	int Case;	/* convert case before doing match? */
};

/* descriptions of the pattern types, used for debugging */
char *pattern_style[PAT_TYPES];

struct exp_cases_descriptor {
	int count;
	struct ecase **cases;
};

/* This describes an Expect command */
static
struct exp_cmd_descriptor {
	int cmdtype;			/* bg, before, after */
	int duration;			/* permanent or temporary */
	int timeout_specified_by_flag;	/* if -timeout flag used */
	int timeout;			/* timeout period if flag used */
	struct exp_cases_descriptor ecd;
	struct exp_i *i_list;
} exp_cmds[4];
/* note that exp_cmds[FG] is just a fake, the real contents is stored
   in some dynamically-allocated variable.  We use exp_cmds[FG] mostly
   as a well-known address and also as a convenience and so we allocate
   just a few of its fields that we need. */

static void
exp_cmd_init(cmd,cmdtype,duration)
struct exp_cmd_descriptor *cmd;
int duration;
int cmdtype;
{
	cmd->duration = duration;
	cmd->cmdtype = cmdtype;
	cmd->ecd.cases = 0;
	cmd->ecd.count = 0;
	cmd->i_list = 0;
}

static int i_read_errno;/* place to save errno, if i_read() == -1, so it
			   doesn't get overwritten before we get to read it */
#if 0
static jmp_buf env;	/* for interruptable read() */
			/* longjmp(env,1) times out the read */
			/* longjmp(env,2) restarts the read */
static int env_valid = FALSE;	/* whether we can longjmp or not */
#endif

#ifdef SIMPLE_EVENT
static int alarm_fired;	/* if alarm occurs */
#endif

void exp_background_channelhandlers_run_all();

/* exp_indirect_updateX is called by Tcl when an indirect variable is set */
static char *exp_indirect_update1();	/* 1-part Tcl variable names */
static char *exp_indirect_update2();	/* 2-part Tcl variable names */

static int	exp_i_read _ANSI_ARGS_((Tcl_Interp *,int,int,int));

#ifdef SIMPLE_EVENT
/*ARGSUSED*/
static RETSIGTYPE
sigalarm_handler(n)
int n;		       	/* unused, for compatibility with STDC */
{
	alarm_fired = TRUE;
#if 0
	/* check env_valid first to protect us from the alarm occurring */
	/* in the window between i_read and alarm(0) */
	if (env_valid) longjmp(env,1);
#endif /*0*/
}
#endif /*SIMPLE_EVENT*/

#if 0
/*ARGSUSED*/
static RETSIGTYPE
sigalarm_handler(n)
int n;		       	/* unused, for compatibility with STDC */
{
#ifdef REARM_SIG
	signal(SIGALRM,sigalarm_handler);
#endif

	/* check env_valid first to protect us from the alarm occurring */
	/* in the window between i_read and alarm(0) */
	if (env_valid) longjmp(env,1);
}
#endif /*0*/

#if 0

/* upon interrupt, act like timeout */
/*ARGSUSED*/
static RETSIGTYPE
sigint_handler(n)
int n;			/* unused, for compatibility with STDC */
{
#ifdef REARM_SIG
	signal(SIGINT,sigint_handler);/* not nec. for BSD, but doesn't hurt */
#endif

#ifdef TCL_DEBUGGER
	if (exp_tcl_debugger_available) {
		/* if the debugger is active and we're reading something, */
		/* force the debugger to go interactive now and when done, */
		/* restart the read.  */

		Dbg_On(exp_interp,env_valid);

		/* restart the read */
		if (env_valid) longjmp(env,2);

		/* if no read is in progess, just let debugger start at */
		/* the next command. */
		return;
	}
#endif

#if 0
/* the ability to timeout a read via ^C is hereby removed 8-Mar-1993 - DEL */

	/* longjmp if we are executing a read inside of expect command */
	if (env_valid) longjmp(env,1);
#endif

	/* if anywhere else in code, prepare to exit */
	exp_exit(exp_interp,0);
}
#endif /*0*/

/* remove nulls from s.  Initially, the number of chars in s is c, */
/* not strlen(s).  This count does not include the trailing null. */
/* returns number of nulls removed. */
static int
rm_nulls(s,c)
char *s;
int c;
{
	char *s2 = s;	/* points to place in original string to put */
			/* next non-null character */
	int count = 0;
	int i;

	for (i=0;i<c;i++,s++) {
		if (0 == *s) {
			count++;
			continue;
		}
		if (count) *s2 = *s;
		s2++;
	}
	return(count);
}

/* free up everything in ecase */
static void
free_ecase(interp,ec,free_ilist)
Tcl_Interp *interp;
struct ecase *ec;
int free_ilist;		/* if we should free ilist */
{
	if (ec->i_list->duration == EXP_PERMANENT) {
		if (ec->pat) Tcl_DecrRefCount(ec->pat);
		if (ec->body) Tcl_DecrRefCount(ec->body);
	}

	if (free_ilist) {
		ec->i_list->ecount--;
		if (ec->i_list->ecount == 0)
			exp_free_i(interp,ec->i_list,exp_indirect_update2);
	}

	ckfree((char *)ec);	/* NEW */
}

/* free up any argv structures in the ecases */
static void
free_ecases(interp,eg,free_ilist)
Tcl_Interp *interp;
struct exp_cmd_descriptor *eg;
int free_ilist;		/* if true, free ilists */
{
	int i;

	if (!eg->ecd.cases) return;

	for (i=0;i<eg->ecd.count;i++) {
		free_ecase(interp,eg->ecd.cases[i],free_ilist);
	}
	ckfree((char *)eg->ecd.cases);

	eg->ecd.cases = 0;
	eg->ecd.count = 0;
}


#if 0
/* no standard defn for this, and some systems don't even have it, so avoid */
/* the whole quagmire by calling it something else */
static char *exp_strdup(s)
char *s;
{
	char *news = ckalloc(strlen(s) + 1);
	strcpy(news,s);
	return(news);
}
#endif

/* In many places, there is no need to malloc a copy of a string, since it */
/* will be freed before we return to Tcl */
static void
save_str(lhs,rhs,nosave)
char **lhs;	/* left hand side */
char *rhs;	/* right hand side */
int nosave;
{
	if (nosave || (rhs == 0)) {
		*lhs = rhs;
	} else {
		*lhs = ckalloc(strlen(rhs) + 1);
		strcpy(*lhs,rhs);
	}
}

/* return TRUE if string appears to be a set of arguments
   The intent of this test is to support the ability of commands to have
   all their args braced as one.  This conflicts with the possibility of
   actually intending to have a single argument.
   The bad case is in expect which can have a single argument with embedded
   \n's although it's rare.  Examples that this code should handle:
   \n		FALSE (pattern)
   \n\n		FALSE
   \n  \n \n	FALSE
   foo		FALSE
   foo\n	FALSE
   \nfoo\n	TRUE  (set of args)
   \nfoo\nbar	TRUE

   Current test is very cheap and almost always right :-)
*/
int 
exp_one_arg_braced(p)
char *p;
{
	int seen_nl = FALSE;

	for (;*p;p++) {
		if (*p == '\n') {
			seen_nl = TRUE;
			continue;
		}

		if (!isspace(*p)) {
			return(seen_nl);
		}
	}
	return FALSE;
}

/* called to execute a command of only one argument - a hack to commands */
/* to be called with all args surrounded by an outer set of braces */
/* returns TCL_whatever */
/*ARGSUSED*/
int
exp_eval_with_one_arg(clientData,interp,argv)
ClientData clientData;
Tcl_Interp *interp;
char **argv;
{
	char *buf;
	int rc;
	char *a;

	/* + 11 is for " -nobrace " and null at end */
	buf = ckalloc(strlen(argv[0]) + strlen(argv[1]) + 11);
	/* recreate statement (with -nobrace to prevent recursion) */
	sprintf(buf,"%s -nobrace %s",argv[0],argv[1]);

	/*
	 * replace top-level newlines with blanks
	 */

	/* Should only be necessary to run over argv[1] and then sprintf */
	/* that into the buffer, but the ICEM guys insist that writing */
	/* back over the original arguments makes their Tcl compiler very */
	/* unhappy. */
	for (a=buf;*a;) {
		extern char *TclWordEnd();

		for (;isspace(*a);a++) {
			if (*a == '\n') *a = ' ';
		}
		a = TclWordEnd(a,&a[strlen(a)],0,(int *)0)+1;
	}

	rc = Tcl_Eval(interp,buf);

	ckfree(buf);
	return(rc);
}

static void
ecase_clear(ec)
struct ecase *ec;
{
	ec->i_list = 0;
	ec->pat = 0;
	ec->body = 0;
	ec->transfer = TRUE;
	ec->indices = FALSE;
/*	ec->iwrite = FALSE;*/
	ec->iread = FALSE;
	ec->timestamp = FALSE;
	ec->Case = CASE_NORM;
	ec->use = PAT_GLOB;
}

static struct ecase *
ecase_new()
{
	struct ecase *ec = (struct ecase *)ckalloc(sizeof(struct ecase));

	ecase_clear(ec);
	return ec;
}

/*

parse_expect_args parses the arguments to expect or its variants. 
It normally returns TCL_OK, and returns TCL_ERROR for failure.
(It can't return i_list directly because there is no way to differentiate
between clearing, say, expect_before and signalling an error.)

eg (expect_global) is initialized to reflect the arguments parsed
eg->ecd.cases is an array of ecases
eg->ecd.count is the # of ecases
eg->i_list is a linked list of exp_i's which represent the -i info

Each exp_i is chained to the next so that they can be easily free'd if
necessary.  Each exp_i has a reference count.  If the -i is not used
(e.g., has no following patterns), the ref count will be 0.

Each ecase points to an exp_i.  Several ecases may point to the same exp_i.
Variables named by indirect exp_i's are read for the direct values.

If called from a foreground expect and no patterns or -i are given, a
default exp_i is forced so that the command "expect" works right.

The exp_i chain can be broken by the caller if desired.

*/

static int
parse_expect_args(interp,eg,default_esPtr,objc,objv)
Tcl_Interp *interp;
struct exp_cmd_descriptor *eg;
ExpState *default_esPtr;	/* suggested master if called as expect_user or _tty */
int objc;
Tcl_Obj *CONST objv[];		/* Argument objects. */
{
	int i;
	char *arg;
	struct ecase ec;	/* temporary to collect args */

	static char *options[] = {
	    "timeout", "eof", "full_buffer", "default", "null", "--",
	    "-glob", "-regexp", "-exact", "-notransfer", "-nocase",
	    "-i", "-indices", "-iwrite", "-iread", "-timestamp", "-timeout",
	    "-nobrace", (char *) NULL
	};
	enum options {
	    EXP_ARG_TIMEOUT, EXP_ARG_EOF, EXP_ARG_FULL_BUFFER,
	    EXP_ARG_DEFAULT, EXP_ARG_NULL, EXP_ARG_DASH, EXP_ARG_GLOB,
	    EXP_ARG_REGEXP, EXP_ARG_EXACT, EXP_ARG_NOTRANSFER, EXP_ARG_NOCASE,
	    EXP_ARG_SPAWN_ID, EXP_ARG_INDICES, EXP_ARG_IWRITE, EXP_ARG_IREAD,
	    EXP_ARG_TIMESTAMP, EXP_ARG_DASH_TIMEOUT, EXP_ARG_NOBRACE
	};
	int index;

	eg->timeout_specified_by_flag = FALSE;

	ecase_clear(&ec);

	/* Allocate an array to store the ecases.  Force array even if 0 */
	/* cases.  This will often be too large (i.e., if there are flags) */
	/* but won't affect anything. */

	eg->ecd.cases = (struct ecase **)ckalloc(
		sizeof(struct ecase *) * (1+(objc/2)));

	eg->ecd.count = 0;

	for (i = 1;i<objc;i++) {
	    if (Tcl_GetIndexFromObj(interp, objv[i], options, "option", 0,
		    &index) == TCL_OK) {
		switch ((enum options) index) {
		    case EXP_ARG_TIMEOUT:
			ec.use = PAT_TIMEOUT;
			goto pattern;
		    case EXP_ARG_EOF:
			ec.use = PAT_EOF;
			goto pattern;
		    case EXP_ARG_FULL_BUFFER:
			ec.use = PAT_FULLBUFFER;
			goto pattern;
		    case EXP_ARG_DEFAULT:
			ec.use = PAT_DEFAULT;
			goto pattern;
		    case EXP_ARG_NULL:
			ec.use = PAT_NULL;
			goto pattern;
		    case EXP_ARG_DASH:
		    case EXP_ARG_GLOB:
			/* assignment here is not actually necessary */
			/* since cases are initialized this way above */
			/* ec.use = PAT_GLOB; */
			goto pattern;
		    case EXP_ARG_REGEXP:
			if (i >= objc-1) {
			    Tcl_WrongNumArgs(interp, 1, objv,
				    "-regexp regexp");
			    return TCL_ERROR;
			}
			ec.use = PAT_RE;

			/*
			 * Try compiling the expression so we can report
			 * any errors now rather then when we first try to
			 * use it.
			 */

			if (!(Tcl_GetRegExpFromObj(interp, objv[i+1],
				REG_ADVANCED))) {
			    goto error;
			}
			goto pattern;
		    case EXP_ARG_EXACT:
			ec.use = PAT_EXACT;
			goto pattern;

		    case EXP_ARG_NOTRANSFER:
			ec.transfer = 0;
			break;
		    case EXP_ARG_NOCASE:
			ec.Case = CASE_LOWER;
			break;
		    case EXP_ARG_SPAWN_ID:
			i++;
			if (i>=objc) {
			    exp_error(interp,"-i requires following spawn_id");
			    goto error;
			}

			ec.i_list = exp_new_i_complex(interp,
				Tcl_GetString(objv[i]),
				eg->duration, exp_indirect_update2);

			ec.i_list->cmdtype = eg->cmdtype;

				/* link new i_list to head of list */
			ec.i_list->next = eg->i_list;
			eg->i_list = ec.i_list;
			break;
		    case EXP_ARG_INDICES:
			ec.indices = TRUE;
			break;
		    case EXP_ARG_IWRITE:
/*				ec.iwrite = TRUE;*/
			break;
		    case EXP_ARG_IREAD:
			ec.iread = TRUE;
			break;
		    case EXP_ARG_TIMESTAMP:
			ec.timestamp = TRUE;
			break;
		    case EXP_ARG_DASH_TIMEOUT:
			i++;
			if (i>=objc) {
			    exp_error(interp,"-timeout requires following # of seconds");
			    goto error;
			}
			if (Tcl_GetIntFromObj(interp, objv[i],
				&eg->timeout) != TCL_OK) {
			    goto error;
			}
			eg->timeout_specified_by_flag = TRUE;
			break;
		    case EXP_ARG_NOBRACE:
				/* nobrace does nothing but take up space */
				/* on the command line which prevents */
				/* us from re-expanding any command lines */
				/* of one argument that looks like it should */
				/* be expanded to multiple arguments. */
			break;
		}
		/*
		 * Keep processing arguments, we aren't ready for the
		 * pattern yet.
		 */
		continue;
	    }

	    /*
	     * We have a pattern of some kind.
	     */

	    pattern:
	    /* if no -i, use previous one */
	    if (!ec.i_list) {
		/* if no -i flag has occurred yet, use default */
		if (!eg->i_list) {
		    if (default_esPtr != EXP_SPAWN_ID_BAD) {
			eg->i_list = exp_new_i_simple(default_esPtr,eg->duration);
		    } else {
			/* it'll be checked later, if used */
			*default_esPtr = expGetCurrentState(interp,0,0);
			eg->i_list = exp_new_i_simple(default_esPtr,eg->duration);
		    }
		}
		ec.i_list = eg->i_list;
	    }
	    ec.i_list->ecount++;

	    /* save original pattern spec */
	    /* keywords such as "-timeout" are saved as patterns here */
	    /* useful for debugging but not otherwise used */

	    i++;
	    ec.pat = objv[i];
	    Tcl_IncrRefCount(ec.pat);

	    i++;
	    if (i < objc) {
		ec.body = objv[i];
		Tcl_IncrRefCount(ec.body);
	    } else {
		ec.body = NULL;
	    }

	    *(eg->ecd.cases[eg->ecd.count] = ecase_new()) = ec;

		/* clear out for next set */
	    ecase_clear(&ec);

	    eg->ecd.count++;
	}

	/* if no patterns at all have appeared force the current */
	/* spawn id to be added to list anyway */

	if (eg->i_list == 0) {
		if (default_esPtr != EXP_SPAWN_ID_BAD) {
			eg->i_list = exp_new_i_simple(default_esPtr,eg->duration);
		} else {
		    /* it'll be checked later, if used */
		    *default_esPtr = expGetCurrentState(interp,0,0);
		    eg->i_list = exp_new_i_simple(default_esPtr,eg->duration);
		}
	}

	return(TCL_OK);

 error:
	/* very hard to free case_master_list here if it hasn't already */
	/* been attached to a case, ugh */

	/* note that i_list must be avail to free ecases! */
	free_ecases(interp,eg,0);

	if (eg->i_list)
		exp_free_i(interp,eg->i_list,exp_indirect_update2);
	return(TCL_ERROR);
}

#define EXP_IS_DEFAULT(x)	((x) == EXP_TIMEOUT || (x) == EXP_EOF)

static char yes[] = "yes\r\n";
static char no[] = "no\r\n";

/* this describes status of a successful match */
struct eval_out {
    struct ecase *e;		/* ecase that matched */
    ExpState *esPtr;		/* ExpState that matched */
    Tcl_Obj *buffer;		/* buffer that matched */
    int match;			/* # of chars in buffer that matched */
			        /* or # of chars in buffer at EOF */
};




/*
 *----------------------------------------------------------------------
 *
 * string_case_first --
 *
 *	Find the first instance of a pattern in a string.
 *
 * Results:
 *	Returns the pointer to the first instance of the pattern
 *	in the given string, or NULL if no match was found.
 *
 * Side effects:
 *	None.
 *
 *----------------------------------------------------------------------
 */

char *
string_case_first(string,pattern)	/* INTL */
    register char *string;	/* String. */
    register char *pattern;	/* Pattern, which may contain
				 * special characters. */
{
    char *s, *p;
    int offset;
    Tcl_UniChar ch1, ch2;
    
    while (*string != 0) {
	s = string;
	p = pattern;
	while (*s) {
	    s += Tcl_UtfToUniChar(s, &ch1);
	    offset = Tcl_UtfToUniChar(p, &ch2);
	    if (Tcl_UniCharToLower(ch1) != Tcl_UniCharToLower(ch2)) {
		break;
	    }
	    p += offset;
	}
	if (*p == '\0') {
	    return string;
	}
	string++;
    }
    return NULL;
}

/* like eval_cases, but handles only a single cases that needs a real */
/* string match */
/* returns EXP_X where X is MATCH, NOMATCH, FULLBUFFER, TCLERRROR */
static int
eval_case_string(interp,e,esPtr,o,last_esPtr,last_case,suffix)
Tcl_Interp *interp;
struct ecase *e;
ExpState *esPtr;
struct eval_out *o;		/* 'output' - i.e., final case of interest */
/* next two args are for debugging, when they change, reprint buffer */
ExpState **last_esPtr;
int *last_case;
char *suffix;
{
	Tcl_Obj *buffer;
	Tcl_RegExp re;
	Tcl_RegExpInfo info;
	char *str;
	int length, flags;

	buffer = esPtr->buffer;
	str = Tcl_GetStringFromObj(buffer, &length);

	/* if master or case changed, redisplay debug-buffer */
	if ((f != *last_esPtr) || e->Case != *last_case) {
		debuglog("\r\nexpect%s: does \"%s\" (spawn_id %s) match %s ",
			suffix,
			dprintify(buffer),esPtr->name,
			pattern_style[e->use]);
		*last_esPtr = esPtr;
		*last_case = e->Case;
	}

	if (e->use == PAT_RE) {
	    debuglog("\"%s\"? ",dprintify(e->pat));
	    if (e->Case == CASE_NORM) {
		flags = TCL_REG_ADVANCED;
	    } else {
		flags = TCL_REG_ADVANCED | TCL_REG_NOCASE;
	    }
		    
	    re = Tcl_GetRegExpFromObj(interp, e->pat, flags);

	    if (!buffer) {
		debuglog(no);
	    } else if (Tcl_RegExpMatchObj(interp, re, buffer, 0) < 0) {
		debuglog(no);
		return(EXP_TCLERROR);
	    } else {

		o->e = e;

		/*
		 * Retrieve the byte offset of the end of the
		 * matched string.  
		 */

		Tcl_RegExpGetInfo(re, &info);
		o->match = Tcl_UtfAtIndex(str, info.matches[0].end) - str;
		o->buffer = buffer;
		o->esPtr = esPtr;
		debuglog(yes);
		return(EXP_MATCH);
	    }
	} else if (e->use == PAT_GLOB) {
	    int match; /* # of chars that matched */

	    debuglog("\"%s\"? ",dprintify(e->pat));
	    if (buffer) {
		match = Exp_StringCaseMatch(Tcl_GetString(buffer),
			Tcl_GetString(e->pat),
			(e->Case == CASE_NORM) ? 0 : 1,
			&e->simple_start);
		if (match != -1) {
		    o->e = e;
		    o->match = match;
		    o->buffer = buffer;
		    o->esPtr = esPtr;
		    debuglog(yes);
		    return(EXP_MATCH);
		}
	    }
	    debuglog(no);
	} else if (e->use == PAT_EXACT) {
	    int patLength;
	    char *pat = Tcl_GetStringFromObj(e->pat, &patLength);
	    int size = (patLength < length) ? patLength : length;
	    char *p;

	    if (e->Case == CASE_NORM) {
		p = strstr(str, pat);
	    } else {
		p = string_case_first(str, pat);
	    }	    

	    debuglog("\"%s\"? ",dprintify(pat));
	    if (p) {
		e->simple_start = p - str;
		o->e = e;
		o->match = e->simple_start + patLength;
		o->buffer = buffer;
		o->esPtr = esPtr;
		debuglog(yes);
		return(EXP_MATCH);
	    } else debuglog(no);
	} else if (e->use == PAT_NULL) {
	    Tcl_UniChar ch;
	    char *p;
	    debuglog("null? ");
	    p = Tcl_UtfFindFirst(str, 0);

	    if (p) {
		o->e = e;
		o->match = p-str;
		o->buffer = buffer;
		o->esPtr = esPtr;
		debuglog(yes);
		return EXP_MATCH;
	    }
	    debuglog(no);
	} else if ((Tcl_GetCharLength(esPtr->buffer) == esPtr->msize)
		&& (length > 0)) {
		debuglog("%s? ",e->pat);
		o->e = e;
		o->match = length;
		o->buffer = esPtr->buffer;
		o->esPtr = esPtr;
		debuglog(yes);
		return(EXP_FULLBUFFER);
	}
	return(EXP_NOMATCH);
}

/* sets o.e if successfully finds a matching pattern, eof, timeout or deflt */
/* returns original status arg or EXP_TCLERROR */
static int
eval_cases(interp,eg,esPtr,o,last_esPtr,last_case,status,masters,mcount,suffix)
Tcl_Interp *interp;
struct exp_cmd_descriptor *eg;
ExpState *esPtr;
struct eval_out *o;		/* 'output' - i.e., final case of interest */
/* next two args are for debugging, when they change, reprint buffer */
ExpState **last_esPtr;
int *last_case;
int status;
int *masters;
int mcount;
char *suffix;
{
    int i;
    int em;	/* master of ecase */
    struct ecase *e;

    if (o->e || status == EXP_TCLERROR || eg->ecd.count == 0) return(status);

    if (status == EXP_TIMEOUT) {
	for (i=0;i<eg->ecd.count;i++) {
	    e = eg->ecd.cases[i];
	    if (e->use == PAT_TIMEOUT || e->use == PAT_DEFAULT) {
		o->e = e;
		break;
	    }
	}
	return(status);
    } else if (status == EXP_EOF) {
	for (i=0;i<eg->ecd.count;i++) {
	    e = eg->ecd.cases[i];
	    if (e->use == PAT_EOF || e->use == PAT_DEFAULT) {
		struct exp_state_list *slPtr;

		for (slPtr=e->i_list->state_list; slPtr ;slPtr=slPtr->next) {
		    em = slPtr->esPtr;
		    if (expIsStateAny(em) || em == m) {
			o->e = e;
			return(status);
		    }
		}
	    }
	}
	return(status);
    }

    /* the top loops are split from the bottom loop only because I can't */
    /* split'em further. */

    /* The bufferful condition does not prevent a pattern match from */
    /* occurring and vice versa, so it is scanned with patterns */
    for (i=0;i<eg->ecd.count;i++) {
	struct exp_state_list *slPtr;
	int j;

	e = eg->ecd.cases[i];
	if (e->use == PAT_TIMEOUT ||
		e->use == PAT_DEFAULT ||
		e->use == PAT_EOF) continue;

	for (slPtr = e->i_list->state_list; slPtr; slPtr = slPtr->next) {
	    em = slPtr->esPtr;
	    /* if em == EXP_SPAWN_ID_ANY, then user is explicitly asking */
	    /* every case to be checked against every master */
	    if (expIsStateAny(em)) {
		/* test against each spawn_id */
		for (j=0;j<mcount;j++) {
		    status = eval_case_string(interp,e,masters[j],o,
			    last_esPtr,last_case,suffix);
		    if (status != EXP_NOMATCH) return(status);
		}
	    } else {
		/* reject things immediately from wrong spawn_id */
		if (em != m) continue;

		status = eval_case_string(interp,e,m,o,last_esPtr,last_case,suffix);
		if (status != EXP_NOMATCH) return(status);
	    }
	}
    }
    return(EXP_NOMATCH);
}

static void
ecases_remove_by_expi(interp,ecmd,exp_i)
Tcl_Interp *interp;
struct exp_cmd_descriptor *ecmd;
struct exp_i *exp_i;
{
	int i;

	/* delete every ecase dependent on it */
	for (i=0;i<ecmd->ecd.count;) {
		struct ecase *e = ecmd->ecd.cases[i];
		if (e->i_list == exp_i) {
			free_ecase(interp,e,0);

			/* shift remaining elements down */
			/* but only if there are any left */
			if (i+1 != ecmd->ecd.count) {
				memcpy(&ecmd->ecd.cases[i],
				       &ecmd->ecd.cases[i+1],
					((ecmd->ecd.count - i) - 1) * 
					sizeof(struct exp_cmd_descriptor *));
			}
			ecmd->ecd.count--;
			if (0 == ecmd->ecd.count) {
				ckfree((char *)ecmd->ecd.cases);
				ecmd->ecd.cases = 0;
			}
		} else {
			i++;
		}
	}
}

/* remove exp_i from list */
static void
exp_i_remove(interp,ei,exp_i)
Tcl_Interp *interp;
struct exp_i **ei;	/* list to remove from */
struct exp_i *exp_i;	/* element to remove */
{
	/* since it's in middle of list, free exp_i by hand */
	for (;*ei; ei = &(*ei)->next) {
		if (*ei == exp_i) {
			*ei = exp_i->next;
			exp_i->next = 0;
			exp_free_i(interp,exp_i,exp_indirect_update2);
			break;
		}
	}
}

/* remove exp_i from list and remove any dependent ecases */
static void
exp_i_remove_with_ecases(interp,ecmd,exp_i)
Tcl_Interp *interp;
struct exp_cmd_descriptor *ecmd;
struct exp_i *exp_i;
{
	ecases_remove_by_expi(interp,ecmd,exp_i);
	exp_i_remove(interp,&ecmd->i_list,exp_i);
}

/* remove ecases tied to a single direct spawn id */
static void
ecmd_remove_state(interp,ecmd,esPtr,direct)
Tcl_Interp *interp;
struct exp_cmd_descriptor *ecmd;
ExpState *esPtr;
int direct;
{
    struct exp_i *exp_i, *next;
    struct exp_state_list **slPtr;

    for (exp_i=ecmd->i_list;exp_i;exp_i=next) {
	next = exp_i->next;

	if (!(direct & exp_i->direct)) continue;

	for (slPtr = &exp_i->state_list;*slPtr;) {
	    if (esPtr == ((*slPtr)->esPtr)) {
		struct exp_state_list *tmp = *slPtr;
		*slPtr = (*slPtr)->next;
		exp_free_state_single(tmp);

		/* if last bg ecase, disarm spawn id */
		if ((ecmd->cmdtype == EXP_CMD_BG) && (expIsStateAny(esPtr))) {
		    esPtr->bg_ecount--;
		    if (esPtr->bg_ecount == 0) {
			exp_disarm_background_channelhandler(esPtr);
			esPtr->bg_interp = 0;
		    }
		}
		
		continue;
	    }
	    slPtr = &(*slPtr)->next;
	}

	/* if left with no ExpStates (and is direct), get rid of it */
	/* and any dependent ecases */
	if (exp_i->direct == EXP_DIRECT && !exp_i->state_list) {
	    exp_i_remove_with_ecases(interp,ecmd,exp_i);
	}
    }
}

/* this is called from exp_close to clean up the ExpState */
void
exp_ecmd_remove_state_direct_and_indirect(interp,esPtr)
Tcl_Interp *interp;
ExpState *esPtr;
{
	ecmd_remove_state(interp,&exp_cmds[EXP_CMD_BEFORE],esPtr,EXP_DIRECT|EXP_INDIRECT);
	ecmd_remove_state(interp,&exp_cmds[EXP_CMD_AFTER],esPtr,EXP_DIRECT|EXP_INDIRECT);
	ecmd_remove_state(interp,&exp_cmds[EXP_CMD_BG],esPtr,EXP_DIRECT|EXP_INDIRECT);

	/* force it - explanation in exp_tk.c where this func is defined */
	exp_disarm_background_channelhandler_force(esPtr);
}

/* arm a list of background ExpState's */
static void
state_list_arm(interp,slPtr)
Tcl_Interp *interp;
struct exp_state_list *slPtr;
{
    /* for each spawn id in list, arm if necessary */
    for (;slPtr;slPtr=slPtr->next) {
	ExpState *esPtr = slPtr->esPtr;    
	if (expIsStateAny(esPtr)) continue;

	if (esPtr->bg_ecount == 0) {
	    exp_arm_background_channelhandler(esPtr);
	    esPtr->bg_interp = interp;
	}
	esPtr->bg_ecount++;
    }
}

/* return TRUE if this ecase is used by this fd */
static int
exp_i_uses_fd(exp_i,fd)
struct exp_i *exp_i;
int fd;
{
	struct exp_state_list *fdp;

	for (fdp = exp_i->state_list;fdp;fdp=fdp->next) {
		if (fdp->esPtr == fd) return 1;
	}
	return 0;
}

static void
ecase_append(interp,ec)
Tcl_Interp *interp;
struct ecase *ec;
{
	if (!ec->transfer) Tcl_AppendElement(interp,"-notransfer");
	if (ec->indices) Tcl_AppendElement(interp,"-indices");
/*	if (ec->iwrite) Tcl_AppendElement(interp,"-iwrite");*/
	if (!ec->Case) Tcl_AppendElement(interp,"-nocase");

	if (ec->use == PAT_RE) Tcl_AppendElement(interp,"-re");
	else if (ec->use == PAT_GLOB) Tcl_AppendElement(interp,"-gl");
	else if (ec->use == PAT_EXACT) Tcl_AppendElement(interp,"-ex");
	Tcl_AppendElement(interp,Tcl_GetString(ec->pat));
	Tcl_AppendElement(interp,ec->body?Tcl_GetString(ec->body):"");
}

/* append all ecases that match this exp_i */
static void
ecase_by_exp_i_append(interp,ecmd,exp_i)
Tcl_Interp *interp;
struct exp_cmd_descriptor *ecmd;
struct exp_i *exp_i;
{
	int i;
	for (i=0;i<ecmd->ecd.count;i++) {
		if (ecmd->ecd.cases[i]->i_list == exp_i) {
			ecase_append(interp,ecmd->ecd.cases[i]);
		}
	}
}

static void
exp_i_append(interp,exp_i)
Tcl_Interp *interp;
struct exp_i *exp_i;
{
	Tcl_AppendElement(interp,"-i");
	if (exp_i->direct == EXP_INDIRECT) {
		Tcl_AppendElement(interp,exp_i->variable);
	} else {
		struct exp_state_list *fdp;

		/* if more than one element, add braces */
		if (exp_i->state_list->next)
			Tcl_AppendResult(interp," {",(char *)0);

		for (fdp = exp_i->state_list;fdp;fdp=fdp->next) {
			char buf[10];	/* big enough for a small int */
			sprintf(buf,"%d",fdp->esPtr);
			Tcl_AppendElement(interp,buf);
		}

		if (exp_i->state_list->next)
			Tcl_AppendResult(interp,"} ",(char *)0);
	}
}

/* return current setting of the permanent expect_before/after/bg */
int
expect_info(interp,ecmd,argc,argv)
Tcl_Interp *interp;
struct exp_cmd_descriptor *ecmd;
int argc;
char **argv;
{
	struct exp_i *exp_i;
	int i;
	int direct = EXP_DIRECT|EXP_INDIRECT;
	char *iflag = 0;
	int all = FALSE;	/* report on all fds */
	ExpState *esPtr = 0;

	while (*argv) {
		if (streq(argv[0],"-i") && argv[1]) {
			iflag = argv[1];
			argc-=2; argv+=2;
		} else if (streq(argv[0],"-all")) {
			all = TRUE;
			argc--; argv++;
		} else if (streq(argv[0],"-noindirect")) {
			direct &= ~EXP_INDIRECT;
			argc--; argv++;
		} else {
			exp_error(interp,"usage: -info [-all | -i spawn_id]\n");
			return TCL_ERROR;
		}
	}

	if (all) {
		/* avoid printing out -i when redundant */
		struct exp_i *previous = 0;

		for (i=0;i<ecmd->ecd.count;i++) {
			if (previous != ecmd->ecd.cases[i]->i_list) {
				exp_i_append(interp,ecmd->ecd.cases[i]->i_list);
				previous = ecmd->ecd.cases[i]->i_list;
			}
			ecase_append(interp,ecmd->ecd.cases[i]);
		}
		return TCL_OK;
	}

	if (!iflag) {
	    if (!(esPtr = expGetCurrentState(interp,0,0))) {
		return TCL_ERROR;
	    }
	} else {
	    if (!(esPtr = expGetState(interp,iflag,0,0,"dummy"))) {
		/* if this is not a valid ExpState, then assume it is an
		   indirect */
		Tcl_ResetResult(interp);
		for (i=0;i<ecmd->ecd.count;i++) {
			if (ecmd->ecd.cases[i]->i_list->direct == EXP_INDIRECT &&
			    streq(ecmd->ecd.cases[i]->i_list->variable,iflag)) {
				ecase_append(interp,ecmd->ecd.cases[i]);
			}
		}
		return TCL_OK;
	}

	/* print ecases of this direct_fd */
	for (exp_i=ecmd->i_list;exp_i;exp_i=exp_i->next) {
		if (!(direct & exp_i->direct)) continue;
		if (!exp_i_uses_state(exp_i,esPtr)) continue;
		ecase_by_exp_i_append(interp,ecmd,exp_i);
	}

	return TCL_OK;
}

/* Exp_ExpectGlobalCmd is invoked to process expect_before/after */
/*ARGSUSED*/
int
Exp_ExpectGlobalCmd(clientData, interp, argc, argv)
ClientData clientData;
Tcl_Interp *interp;
int argc;
char **argv;
{
    int result = TCL_OK;
    struct exp_i *exp_i, **eip;
    struct exp_state_list *slPtr;	/* temp for interating over state_list */
    struct exp_cmd_descriptor eg;
    int count;

    struct exp_cmd_descriptor *ecmd = (struct exp_cmd_descriptor *) clientData;

    if ((argc == 2) && exp_one_arg_braced(argv[1])) {
	return(exp_eval_with_one_arg(clientData,interp,argv));
    } else if ((argc == 3) && streq(argv[1],"-brace")) {
	char *new_argv[2];
	new_argv[0] = argv[0];
	new_argv[1] = argv[2];
	return(exp_eval_with_one_arg(clientData,interp,new_argv));
    }

    if (argc > 1 && (argv[1][0] == '-')) {
	if (exp_flageq("info",&argv[1][1],4)) {
	    return(expect_info(interp,ecmd,argc-2,argv+2));
	} 
    }

    exp_cmd_init(&eg,ecmd->cmdtype,EXP_PERMANENT);

    if (TCL_ERROR == parse_expect_args(interp,&eg,EXP_SPAWN_ID_BAD,
	    argc,argv)) {
	return TCL_ERROR;
    }

    /*
     * visit each NEW direct exp_i looking for spawn ids.
     * When found, remove them from any OLD exp_i's.
     */

    /* visit each exp_i */
    for (exp_i=eg.i_list;exp_i;exp_i=exp_i->next) {
	if (exp_i->direct == EXP_INDIRECT) continue;

	/* for each spawn id, remove it from ecases */
	for (slPtr=exp_i->state_list;slPtr;slPtr=slPtr->next) {
	    int m = slPtr->esPtr;

	    /* validate all input descriptors */
	    if (!expIsStateAny(esPtr)) {
		if (!exp_fd2f(interp,m,1,1,"expect")) {
		    result = TCL_ERROR;
		    goto cleanup;
		}
	    }
	    
	    /* remove spawn id from exp_i */
	    ecmd_remove_state(interp,ecmd,m,EXP_DIRECT);
	}
    }
	
    /*
     * For each indirect variable, release its old ecases and 
     * clean up the matching spawn ids.
     * Same logic as in "expect_X delete" command.
     */

    for (exp_i=eg.i_list;exp_i;exp_i=exp_i->next) {
	struct exp_i **old_i;

	if (exp_i->direct == EXP_DIRECT) continue;

	for (old_i = &ecmd->i_list;*old_i;) {
	    struct exp_i *tmp;

	    if (((*old_i)->direct == EXP_DIRECT) ||
		    (!streq((*old_i)->variable,exp_i->variable))) {
		old_i = &(*old_i)->next;
		continue;
	    }

	    ecases_remove_by_expi(interp,ecmd,*old_i);
	    
	    /* unlink from middle of list */
	    tmp = *old_i;
	    *old_i = tmp->next;
	    tmp->next = 0;
	    exp_free_i(interp,tmp,exp_indirect_update2);
	}

	/* if new one has ecases, update it */
	if (exp_i->ecount) {
	    char *msg = exp_indirect_update1(interp,ecmd,exp_i);
	    if (msg) {
		/* unusual way of handling error return */
		/* because of Tcl's variable tracing */
		strcpy(interp->result,msg);
		result = TCL_ERROR;
		goto indirect_update_abort;
	    }
	}
    }
    /* empty i_lists have to be removed from global eg.i_list */
    /* before returning, even if during error */
 indirect_update_abort:

    /*
     * New exp_i's that have 0 ecases indicate fd/vars to be deleted.
     * Now that the deletions have been done, discard the new exp_i's.
     */

    for (exp_i=eg.i_list;exp_i;) {
	struct exp_i *next = exp_i->next;

	if (exp_i->ecount == 0) {
	    exp_i_remove(interp,&eg.i_list,exp_i);
	}
	exp_i = next;
    }
    if (result == TCL_ERROR) goto cleanup;

    /*
     * arm all new bg direct fds
     */

    if (ecmd->cmdtype == EXP_CMD_BG) {
	for (exp_i=eg.i_list;exp_i;exp_i=exp_i->next) {
	    if (exp_i->direct == EXP_DIRECT) {
		state_list_arm(interp,exp_i->state_list);
	    }
	}
    }

    /*
     * now that old ecases are gone, add new ecases and exp_i's (both
     * direct and indirect).
     */

    /* append ecases */

    count = ecmd->ecd.count + eg.ecd.count;
    if (eg.ecd.count) {
	int start_index; /* where to add new ecases in old list */

	if (ecmd->ecd.count) {
	    /* append to end */
	    ecmd->ecd.cases = (struct ecase **)ckrealloc((char *)ecmd->ecd.cases, count * sizeof(struct ecase *));
	    start_index = ecmd->ecd.count;
	} else {
	    /* append to beginning */
	    ecmd->ecd.cases = (struct ecase **)ckalloc(eg.ecd.count * sizeof(struct ecase *));
	    start_index = 0;
	}
	memcpy(&ecmd->ecd.cases[start_index],eg.ecd.cases,
		eg.ecd.count*sizeof(struct ecase *));
	ecmd->ecd.count = count;
    }

    /* append exp_i's */
    for (eip = &ecmd->i_list;*eip;eip = &(*eip)->next) {
	/* empty loop to get to end of list */
    }
    /* *exp_i now points to end of list */

    *eip = eg.i_list;	/* connect new list to end of current list */

  cleanup:
    if (result == TCL_ERROR) {
	/* in event of error, free any unreferenced ecases */
	/* but first, split up i_list so that exp_i's aren't */
	/* freed twice */

	for (exp_i=eg.i_list;exp_i;) {
	    struct exp_i *next = exp_i->next;
	    exp_i->next = 0;
	    exp_i = next;
	}
	free_ecases(interp,&eg,1);
    } else {
	if (eg.ecd.cases) ckfree((char *)eg.ecd.cases);
    }

    if (ecmd->cmdtype == EXP_CMD_BG) {
	exp_background_channelhandlers_run_all();
    }

    return(result);
}

/* adjusts file according to user's size request */
void
exp_adjust(esPtr)
ExpState *esPtr;
{
    int new_msize;
    int length;
    Tcl_Obj *newObj;
    char *string;

    /* get the latest buffer size.  Double the user input for */
    /* two reasons.  1) Need twice the space in case the match */
    /* straddles two bufferfuls, 2) easier to hack the division */
    /* by two when shifting the buffers later on.  The extra  */
    /* byte in the malloc's is just space for a null we can slam on the */
    /* end.  It makes the logic easier later.  The -1 here is so that */
    /* requests actually come out to even/word boundaries (if user */
    /* gives "reasonable" requests) */
    new_msize = esPtr->umsize*2 - 1;
    if (new_msize != esPtr->msize) {
	if (!esPtr->buffer) {
	    esPtr->buffer = Tcl_NewObj();
	    Tcl_IncrRefCount(esPtr->buffer);
	} else {
	    /* buffer already exists - resize */
	    string = Tcl_GetStringFromObj(esPtr->buffer, &length);
		
	    /* if truncated, forget about some data */
	    if (length > new_msize) {
		newObj = Tcl_NewStringObj(string, new_msize);
		Tcl_IncrRefCount(newObj);
		Tcl_DecrRefCount(esPtr->buffer);
		esPtr->buffer = newObj;
	    } else {
		/*
		 * Force the object to allocate a buffer at least
		 * new_msize bytes long, then reset the correct string
		 * length.
		 */

		Tcl_SetObjLength(esPtr->buffer, new_msize);
		Tcl_SetObjLength(esPtr->buffer, length);
	    }
	    esPtr->key = expect_key++;
	}
	esPtr->msize = new_msize;
    }
}


/*

 expect_read() does the logical equivalent of a read() for the
expect command.  This includes figuring out which descriptor should
be read from.

The result of the read() is left in a spawn_id's buffer rather than
explicitly passing it back.  Note that if someone else has modified a
buffer either before or while this expect is running (i.e., if we or
some event has called Tcl_Eval which did another expect/interact),
expect_read will also call this a successful read (for the purposes if
needing to pattern match against it).

*/
/* if it returns a negative number, it corresponds to a EXP_XXX result */
/* if it returns a non-negative number, it means there is data */
/* (0 means nothing new was actually read, but it should be looked at again) */
int
expect_read(interp,masters,masters_max,esPtrOut,timeout,key)
Tcl_Interp *interp;
ExpState (*esPtrs)[];		/* If 0, then m is already known and set. */
int masters_max;		/* If *masters is not-zero, then masters_max */
				/* is the number of masters. */
				/* If *masters is zero, then masters_max */
				/* is used as the mask (ready vs except). */
				/* Crude but simplifies the interface. */
ExpState **esPtrOut;		/* Out variable to leave new master. */
int timeout;
int key;
{
    ExpState *esPtr;
    int cc;
    int write_count;
    int tcl_set_flags;	/* if we have to discard chars, this tells */
			/* whether to show user locally or globally */

    if (esPtrs == 0) {
	/* we already know the master, just find out what happened */
	cc = exp_get_next_event_info(interp,&esPtr,masters_max);
	tcl_set_flags = TCL_GLOBAL_ONLY;
    } else {
	cc = exp_get_next_event(interp,esPtrs,masters_max,&esPtr,timeout,key);
	tcl_set_flags = 0;
    }

    if (cc == EXP_DATA_NEW) {
	/* try to read it */
	
	cc = exp_i_read(interp,esPtr,timeout,tcl_set_flags);
	
	/* the meaning of 0 from i_read means eof.  Muck with it a */
	/* little, so that from now on it means "no new data arrived */
	/* but it should be looked at again anyway". */
	if (cc == 0) {
	    cc = EXP_EOF;
	} else if (cc > 0) {
	    esPtr->buffer[esPtr->size += cc] = '\0';
	    
	    /* strip parity if requested */
	    if (esPtr->parity == 0) {
		/* do it from end backwards */
		char *p = esPtr->buffer + esPtr->size - 1;
		int count = cc;
		while (count--) {
		    *p-- &= 0x7f;
		}
	    }
	} /* else {
	     assert(cc < 0) in which case some sort of error was
	     encountered such as an interrupt with that forced an
	     error return
	     } */
    } else if (cc == EXP_DATA_OLD) {
	cc = 0;
    } else if (cc == EXP_RECONFIGURE) {
	return EXP_RECONFIGURE;
    }

    if (cc == EXP_ABEOF) {	/* abnormal EOF */
	/* On many systems, ptys produce EIO upon EOF - sigh */
	if (i_read_errno == EIO) {
	    /* Sun, Cray, BSD, and others */
	    cc = EXP_EOF;
	} else if (i_read_errno == EINVAL) {
	    /* Solaris 2.4 occasionally returns this */
	    cc = EXP_EOF;
	} else {
	    if (i_read_errno == EBADF) {
		exp_error(interp,"bad spawn_id (process died earlier?)");
	    } else {
		exp_error(interp,"i_read(spawn_id fd=%d): %s",esPtr->fdin,
			Tcl_PosixError(interp));
		exp_close(interp,esPtr);
	    }
	    return(EXP_TCLERROR);
	    /* was goto error; */
	}
    }

    /* EOF, TIMEOUT, and ERROR return here */
    /* In such cases, there is no need to update screen since, if there */
    /* was prior data read, it would have been sent to the screen when */
    /* it was read. */
    if (cc < 0) return (cc);

    /* update display */

    if (esPtr->size) write_count = esPtr->size - esPtr->printed;
    else write_count = 0;
    
    if (write_count) {
	if (logfile_all || (loguser && logfile)) {
	    fwrite(esPtr->buffer + esPtr->printed,1,write_count,logfile);
	}
	/* don't write to user if they're seeing it already, */
	/* that is, typing it! */
	if (loguser && !expIsStateStdinout(esPtr) && !expIsStateDevtty(esPtr))
	    fwrite(esPtr->buffer + esPtr->printed,
		    1,write_count,stdout);
	if (debugfile) fwrite(esPtr->buffer + esPtr->printed,
		1,write_count,debugfile);
	    
	/* remove nulls from input, since there is no way */
	/* for Tcl to deal with such strings.  Doing it here */
	/* lets them be sent to the screen, just in case */
	/* they are involved in formatting operations */
	if (esPtr->rm_nulls) {
	    esPtr->size -= rm_nulls(esPtr->buffer + esPtr->printed,write_count);
	}
	esPtr->buffer[esPtr->size] = '\0';
	
	/* copy to lowercase buffer */
	exp_lowmemcpy(esPtr->lower+esPtr->printed,
		esPtr->buffer+esPtr->printed,
		1 + esPtr->size - esPtr->printed);
	    
	esPtr->printed = esPtr->size; /* count'm even if not logging */
    }
    return(cc);
}

/* when buffer fills, copy second half over first and */
/* continue, so we can do matches over multiple buffers */
void
exp_buffer_shuffle(interp,esPtr,save_flags,array_name,caller_name) /* INTL */
Tcl_Interp *interp;
ExpState *esPtr;
int save_flags;
char *array_name;
char *caller_name;
{
	Tcl_UniChar *ustr;
	int first_half, second_half, length;
	char *str;

	/*
	 * allow user to see data we are discarding
	 */

	debuglog("%s: set %s(spawn_id) \"%s\"\r\n",
		 caller_name,array_name,esPtr->name);
	Tcl_SetVar2(interp,array_name,"spawn_id",esPtr->name,save_flags);

	/*
	 * The internal storage buffer object should only be referred
	 * to by the channel that uses it.  We always copy the contents
	 * out of the object before passing the data to anyone outside
	 * of these routines.  This ensures that the object always has
	 * a refcount of 1 so we can safely modify the contents in place.
	 */

	if (Tcl_IsShared(esPtr->buffer)) {
	    panic("exp_buffer_shuffle called with shared buffer object");
	}

	str = Tcl_GetString(esPtr->buffer);
	ustr = Tcl_GetUnicode(esPtr->buffer);
	length = Tcl_GetCharLength(esPtr->buffer);

	first_half = length/2;
	second_half = length - first_half;

	first_half_bytes = Tcl_UtfAtIndex(str, first_half) - str;

	memcpy(ustr, ustr+first_half, second_half);
	Tcl_SetUnicodeLength(esPtr->buffer, first_half);

	debuglog("%s: set %s(buffer) \"%s\"\r\n",
		 caller_name,array_name,dprintify(esPtr->buffer));
	Tcl_SetVar2(interp,array_name,"buffer",Tcl_GetString(esPtr->buffer),
		save_flags);

	esPtr->printed -= first_half_bytes;
	if (esPtr->printed < 0) esPtr->printed = 0;
}

/* map EXP_ style return value to TCL_ style return value */
/* not defined to work on TCL_OK */
int
exp_tcl2_returnvalue(x)
int x;
{
	switch (x) {
	case TCL_ERROR:			return EXP_TCLERROR;
	case TCL_RETURN:		return EXP_TCLRET;
	case TCL_BREAK:			return EXP_TCLBRK;
	case TCL_CONTINUE:		return EXP_TCLCNT;
	case EXP_CONTINUE:		return EXP_TCLCNTEXP;
	case EXP_CONTINUE_TIMER:	return EXP_TCLCNTTIMER;
	case EXP_TCL_RETURN:		return EXP_TCLRETTCL;
	}
}

/* map from EXP_ style return value to TCL_ style return values */
int
exp_2tcl_returnvalue(x)
int x;
{
	switch (x) {
	case EXP_TCLERROR:		return TCL_ERROR;
	case EXP_TCLRET:		return TCL_RETURN;
	case EXP_TCLBRK:		return TCL_BREAK;
	case EXP_TCLCNT:		return TCL_CONTINUE;
	case EXP_TCLCNTEXP:		return EXP_CONTINUE;
	case EXP_TCLCNTTIMER:		return EXP_CONTINUE_TIMER;
	case EXP_TCLRETTCL:		return EXP_TCL_RETURN;
	}
}

/* returns # of chars read or (non-positive) error of form EXP_XXX */
/* returns 0 for end of file */
/* If timeout is non-zero, set an alarm before doing the read, else assume */
/* the read will complete immediately. */
/*ARGSUSED*/
static int
exp_i_read(interp,esPtr,timeout,save_flags) /* INTL */
Tcl_Interp *interp;
ExpState *esPtr;
int timeout;
int save_flags;
{
    int cc = EXP_TIMEOUT;
    int size = Tcl_GetCharLength(esPtr->buffer);

    if (size == esPtr->msize) 
	exp_buffer_shuffle(interp,f,save_flags,EXPECT_OUT,"expect");

#ifdef SIMPLE_EVENT
 restart:

    alarm_fired = FALSE;

    if (timeout > -1) {
	signal(SIGALRM,sigalarm_handler);
	alarm((timeout > 0)?timeout:1);
    }
#endif

    
    cc = Tcl_ReadChars(esPtr->channel, esPtr->buffer, esPtr->msize - size,
	    1 /* Append */);
    i_read_errno = errno;

#ifdef SIMPLE_EVENT
    alarm(0);

    if (cc == -1) {
	/* check if alarm went off */
	if (i_read_errno == EINTR) {
	    if (alarm_fired) {
		return EXP_TIMEOUT;
	    } else {
		if (Tcl_AsyncReady()) {
		    int rc = Tcl_AsyncInvoke(interp,TCL_OK);
		    if (rc != TCL_OK) return(exp_tcl2_returnvalue(rc));
		}
		goto restart;
	    }
	}
    }
#endif
    return(cc);
}

/* variables predefined by expect are retrieved using this routine
which looks in the global space if they are not in the local space.
This allows the user to localize them if desired, and also to
avoid having to put "global" in procedure definitions.
*/
char *
exp_get_var(interp,var)
Tcl_Interp *interp;
char *var;
{
    char *val;

    if (NULL != (val = Tcl_GetVar(interp,var,0 /* local */)))
	return(val);
    return(Tcl_GetVar(interp,var,TCL_GLOBAL_ONLY));
}

static int
get_timeout(interp)
Tcl_Interp *interp;
{
    ThreadSpecificData *tsdPtr = TCL_TSD_INIT(&dataKey);
    char *t;

    if (NULL != (t = exp_get_var(interp,EXPECT_TIMEOUT))) {
	tsdPtr->timeout = atoi(t);
    }
    return(tsdPtr->timeout);
}

/* make a copy of a linked list (1st arg) and attach to end of another (2nd
arg) */
static int
update_expect_states(i_list,i_union)
struct exp_i *i_list;
struct exp_state_list **i_union;
{
    struct exp_i *p;

    /* for each i_list in an expect statement ... */
    for (p=i_list;p;p=p->next) {
	struct exp_state_list *slPtr;

	/* for each esPtr in the i_list */
	for (slPtr=p->state_list;slPtr;slPtr=slPtr->next) {
	    struct exp_state_list *tmpslPtr;
	    struct exp_state_list *u;

	    if (expIsStateAny(slPtr->esPtr)) continue;
	    
	    /* check this one against all so far */
	    for (u = *i_union;u;u=u->next) {
		if (slPtr->esPtr == u->esPtr) goto found;
	    }
	    /* if not found, link in as head of list */
	    tmpslPtr = exp_new_state(slPtr->esPtr);
	    tmpslPtr->next = *i_union;
	    *i_union = tmpslPtr;
	    found:;
	}
    }
    return TCL_OK;
}

char *
exp_cmdtype_printable(cmdtype)
int cmdtype;
{
	switch (cmdtype) {
	case EXP_CMD_FG: return("expect");
	case EXP_CMD_BG: return("expect_background");
	case EXP_CMD_BEFORE: return("expect_before");
	case EXP_CMD_AFTER: return("expect_after");
	}
#ifdef LINT
	return("unknown expect command");
#endif
}

/* exp_indirect_update2 is called back via Tcl's trace handler whenever */
/* an indirect spawn id list is changed */
/*ARGSUSED*/
static char *
exp_indirect_update2(clientData, interp, name1, name2, flags)
ClientData clientData;
Tcl_Interp *interp;	/* Interpreter containing variable. */
char *name1;		/* Name of variable. */
char *name2;		/* Second part of variable name. */
int flags;		/* Information about what happened. */
{
	char *msg;

	struct exp_i *exp_i = (struct exp_i *)clientData;
	exp_configure_count++;
	msg = exp_indirect_update1(interp,&exp_cmds[exp_i->cmdtype],exp_i);

	exp_background_channelhandlers_run_all();

	return msg;
}

static char *
exp_indirect_update1(interp,ecmd,exp_i)
Tcl_Interp *interp;
struct exp_cmd_descriptor *ecmd;
struct exp_i *exp_i;
{
	struct exp_state_list *slPtr;	/* temp for interating over state_list */

	/*
	 * disarm any ExpState's that lose all their ecases
	 */

	if (ecmd->cmdtype == EXP_CMD_BG) {
		/* clean up each spawn id used by this exp_i */
		for (slPtr=exp_i->state_list;slPtr;slPtr=slPtr->next) {
			int m = slPtr->esPtr;

			if (expIsStateAny(esPtr)) continue;

			/* silently skip closed or preposterous fds */
			/* since we're just disabling them anyway */
			/* preposterous fds will have been reported */
			/* by code in next section already */
			if (!expCheckState(interp,slPtr->esPtr,1,0,"")) continue;

			/* check before decrementing, ecount may not be */
			/* positive if update is called before ecount is */
			/* properly synchronized */
			if (esPtr->bg_ecount > 0) {
				esPtr->bg_ecount--;
			}
			if (esPtr->bg_ecount == 0) {
				exp_disarm_background_channelhandler(esPtr);
				esPtr->bg_interp = 0;
			}
		}
	}

	/*
	 * reread indirect variable
	 */

	exp_i_update(interp,exp_i);

	/*
	 * check validity of all fd's in variable
	 */

	for (slPtr=exp_i->state_list;slPtr;slPtr=slPtr->next) {
	    /* validate all input descriptors */

	    if (expIsStateAny(slPtr->esPtr)) continue;

		if (!exp_fd2f(interp,slPtr->esPtr,1,1,
				exp_cmdtype_printable(ecmd->cmdtype))) {
			static char msg[200];
			sprintf(msg,"%s from indirect variable (%s)",
				interp->result,exp_i->variable);
			return msg;
		}
	}

	/* for each spawn id in list, arm if necessary */
	if (ecmd->cmdtype == EXP_CMD_BG) {
		state_list_arm(interp,exp_i->state_list);
	}

	return (char *)0;
}

int
exp_process_matches(interp, eo, bg, detail)
    Tcl_Interp interp;
    struct eval_out *eo;	/* final case of interest */
    int bg;			/* 1 if called from background handler, */
				/* else 0 */
    char *detail;
{
    ExpState *esPtr = 0;	/* ExpState associated with master */
    char *body = 0;
    Tcl_Obj *buffer;
    struct ecase *e = 0;	/* points to current ecase */
    int match = -1;		/* characters matched */
    char match_char;	/* place to hold char temporarily */
    /* uprooted by a NULL */
    char *eof_body = 0;
    int result = TCL_OK;

#define out(indexName, value) \
    debuglog("%s: set %s(%s) \"%s\"\r\n", detail, EXPECT_OUT,indexName, \
	    dprintify(value)); \
    Tcl_SetVar2(interp, EXPECT_OUT,index,value,(bg ? TCL_GLOBAL_ONLY : 0);

    if (eo->e) {
	e = eo->e;
	body = e->body;
	if (cc != EXP_TIMEOUT) {
	    esPtr = eo->esPtr;
	    match = eo->match;
	    buffer = eo->buffer;
	}
    } else if (cc == EXP_EOF) {
	/* read an eof but no user-supplied case */
	esPtr = eo->esPtr;
	match = eo->match;
	buffer = eo->buffer;
    }			

    if (match >= 0) {
	char name[20], value[20];

	if (e && e->use == PAT_RE) {
	    regexp *re;
	    int flags;
	    Tcl_RegExpInfo info;

	    if (e->Case == CASE_NORM) {
		flags = TCL_REG_ADVANCED;
	    } else {
		flags = TCL_REG_ADVANCED | TCL_REG_NOCASE;
	    }
		    
	    re = Tcl_GetRegExpFromObj(interp, e->pat, flags);
	    Tcl_RegExpGetInfo(re, &info);

	    for (i=0;i<info.nsub;i++) {
		int offset, start, end;
		Tcl_Obj *val;

		start = info.matches[i].start;
		end = info.matches[i].end-1;
		if (start == -1) continue;

		if (e->indices) {
				/* start index */
		    sprintf(name,"%d,start",i);
		    offset = 
			sprintf(value,"%d",start);
		    out(name,value);

				/* end index */
		    sprintf(name,"%d,end",i);
		    sprintf(value,"%d",end);
		    out(name,value);
		}

				/* string itself */
		sprintf(name,"%d,string",i);
		val = Tcl_GetRange(buffer, start, end);
		debuglog("expect_background: set %s(%s) \"%s\"\r\n",EXPECT_OUT,name, dprintifyobj(val));
		Tcl_SetVar2Ex(interp,EXPECT_OUT,name,
			val, TCL_GLOBAL_ONLY);
	    }
	} else if (e && (e->use == PAT_GLOB || e->use == PAT_EXACT)) {
	    char *str;

	    if (e->indices) {
		/* start index */
		sprintf(value,"%d",e->simple_start);
		out("0,start",value);

		/* end index */
		sprintf(value,"%d",e->simple_start + match - 1);
		out("0,end",value);
	    }

	    /* string itself */
	    str = Tcl_GetString(esPtr->buffer);
	    /* temporarily null-terminate in middle */
	    match_char = str[match];
	    str[match] = 0;
	    out("0,string",str + e->simple_start);
	    str[match] = match_char;

				/* redefine length of string that */
				/* matched for later extraction */
	    match += e->simple_start;
	} else if (e && e->use == PAT_NULL && e->indices) {
				/* start index */
	    sprintf(value,"%d",match-1);
	    out("0,start",value);
				/* end index */
	    sprintf(value,"%d",match-1);
	    out("0,end",value);
	} else if (e && e->use == PAT_FULLBUFFER) {
	    debuglog("expect_background: full buffer\r\n");
	}
    }

    /* this is broken out of (match > 0) (above) since it can */
    /* that an EOF occurred with match == 0 */
    if (eo->f) {
	char spawn_id[10];	/* enough for a %d */
	char *str;

	sprintf(spawn_id,"%d",f-exp_fs);
	out("spawn_id",spawn_id);

	str = Tcl_GetStringFromObj(esPtr->buffer, &length);
			
	/* save buf[0..match] */
	/* temporarily null-terminate string in middle */
	match_char = str[match];
	str[match] = 0;
	out("buffer",str);
	/* remove middle-null-terminator */
	str[match] = match_char;

	/* "!e" means no case matched - transfer by default */
	if (!e || e->transfer) {
	    /* delete matched chars from input buffer */
	    esPtr->printed -= match;
	    if (length != 0) {
		memmove(str,str+match,length-match);
	    }
	    Tcl_SetObjLength(esPtr->buffer, length-match);
	}

	if (cc == EXP_EOF) {
				/* exp_close() deletes all background bodies */
				/* so save eof body temporarily */
	    if (body) {
		eof_body = ckalloc(strlen(body)+1);
		strcpy(eof_body,body);
		body = eof_body;
	    }

	    exp_close(interp,f - exp_fs);
	}

    }

    if (body) {
	if (!bg) {
	    result = Tcl_Eval(interp,body);
	} else {
	    result = Tcl_GlobalEval(interp,body);
	    if (result != TCL_OK) Tcl_BackgroundError(interp);
	}
	if (eof_body) ckfree(eof_body);
    }
    return result;
}
#undef out


/* this function is called from the background when input arrives */
/*ARGSUSED*/
void
exp_background_channelhandler(clientData,mask) /* INTL */
ClientData clientData;
int mask;
{
    ExpState *esPtr;

    Tcl_Interp *interp;
    int cc;			/* number of chars returned in a single read */
				/* or negative EXP_whatever */

    int i;			/* trusty temporary */

    struct eval_out eo;	/* final case of interest */
    ExpState *last_esPtr;	/* for differentiating when multiple f's */
				/* to print out better debugging messages */
    int last_case;		/* as above but for case */
    int length;

    /* restore our environment */
    esPtr = *(ExpState *)clientData;
    interp = esPtr->bg_interp;

    /* temporarily prevent this handler from being invoked again */
    exp_block_background_channelhandler(esPtr);

    /* if mask == 0, then we've been called because the patterns changed */
    /* not because the waiting data has changed, so don't actually do */
    /* any I/O */

    if (mask == 0) {
	cc = 0;
    } else {
	cc = expect_read(interp,(ExpState **)0,mask,&esPtr,EXP_TIME_INFINITY,0);
    }

do_more_data:
    eo.e = 0;		/* no final case yet */
    eo.f = 0;		/* no final file selected yet */
    eo.match = 0;		/* nothing matched yet */

    /* force redisplay of buffer when debugging */
    last_esPtr = 0;

    if (cc == EXP_EOF) {
	/* do nothing */
    } else if (cc < 0) { /* EXP_TCLERROR or any other weird value*/
	goto finish;
	/* if we were going to do this right, we should */
	/* differentiate between things like HP ioctl-open-traps */
	/* that fall out here and should rightfully be ignored */
	/* and real errors that should be reported.  Come to */
	/* think of it, the only errors will come from HP */
	/* ioctl handshake botches anyway. */
    } else {
	/* normal case, got data */
	/* new data if cc > 0, same old data if cc == 0 */

	/* below here, cc as general status */
	cc = EXP_NOMATCH;
    }

    cc = eval_cases(interp,&exp_cmds[EXP_CMD_BEFORE],
	    esPtr,&eo,&last_esPtr,&last_case,cc,&esPtr,1,"_background");
    cc = eval_cases(interp,&exp_cmds[EXP_CMD_BG],
	    esPtr,&eo,&last_esPtr,&last_case,cc,&esPtr,1,"_background");
    cc = eval_cases(interp,&exp_cmds[EXP_CMD_AFTER],
	    esPtr,&eo,&last_esPtr,&last_case,cc,&esPtr,1,"_background");
    if (cc == EXP_TCLERROR) {
		/* only likely problem here is some internal regexp botch */
		Tcl_BackgroundError(interp);
		goto finish;
	}
	/* special eof code that cannot be done in eval_cases */
	/* or above, because it would then be executed several times */
	if (cc == EXP_EOF) {
		eo.f = exp_fs + m;
		eo.match = eo.esPtr->size;
		eo.buffer = eo.esPtr->buffer;
		debuglog("expect_background: read eof\r\n");
		goto matched;
	}
	if (!eo.e) {
		/* if we get here, there must not have been a match */
		goto finish;
	}

 matched:
	{
		exp_process_matches(interp, &eo, 1 /* bg */,
			"expect_background");

		/*
		 * Event handler will not call us back if there is more input
		 * pending but it has already arrived.  bg_status will be
		 * "blocked" only if armed.
		 */
		if (esPtr->valid && (esPtr->bg_status == blocked)) {
		    Tcl_GetStringFromObj(esPtr->buffer, &length);
		    if (length > 0) {
			cc = length;
			goto do_more_data;
		    }
		}
	}
 finish:
	/* fd could have gone away, so check before using */
	if (esPtr->valid)
		exp_unblock_background_channelhandler(esPtr);
}

/*ARGSUSED*/
int
Exp_ExpectCmd(clientData, interp, argc, argv)
ClientData clientData;
Tcl_Interp *interp;
int argc;
char **argv;
{
	int cc;			/* number of chars returned in a single read */
				/* or negative EXP_whatever */
	ExpState *esPtr = 0;	/* ExpState associated with master */

	int i;			/* trusty temporary */
	struct exp_cmd_descriptor eg;
	struct exp_state_list *state_list;	/* list of masters to watch */
	struct exp_state_list *slPtr;	/* temp for interating over state_list */
	int *masters;		/* array of masters to watch */
	int mcount;		/* number of masters to watch */

	struct eval_out eo;	/* final case of interest */

	int result;		/* Tcl result */

	time_t start_time_total;/* time at beginning of this procedure */
	time_t start_time = 0;	/* time when restart label hit */
	time_t current_time = 0;/* current time (when we last looked)*/
	time_t end_time;	/* future time at which to give up */
	time_t elapsed_time_total;/* time from now to match/fail/timeout */
	time_t elapsed_time;	/* time from restart to (ditto) */

	ExpState *last_esPtr;	/* for differentiating when multiple f's */
				/* to print out better debugging messages */
	int last_case;		/* as above but for case */
	int first_time = 1;	/* if not "restarted" */

	int key;		/* identify this expect command instance */
	int configure_count;	/* monitor exp_configure_count */

	int timeout;		/* seconds */
	int remtime;		/* remaining time in timeout */
	int reset_timer;	/* should timer be reset after continue? */

	if ((argc == 2) && exp_one_arg_braced(argv[1])) {
		return(exp_eval_with_one_arg(clientData,interp,argv));
	} else if ((argc == 3) && streq(argv[1],"-brace")) {
		char *new_argv[2];
		new_argv[0] = argv[0];
		new_argv[1] = argv[2];
		return(exp_eval_with_one_arg(clientData,interp,new_argv));
	}

	time(&start_time_total);
	start_time = start_time_total;
	reset_timer = TRUE;

	/* make arg list for processing cases */
	/* do it dynamically, since expect can be called recursively */

	exp_cmd_init(&eg,EXP_CMD_FG,EXP_TEMPORARY);
	state_list = 0;
	masters = 0;
	if (TCL_ERROR == parse_expect_args(interp,&eg,
					(ExpState *)clientData,argc,argv))
		return TCL_ERROR;

 restart_with_update:
	/* validate all descriptors */
	/* and flatten ExpStates into array */

	if ((TCL_ERROR == update_expect_states(exp_cmds[EXP_CMD_BEFORE].i_list,&state_list))
	 || (TCL_ERROR == update_expect_states(exp_cmds[EXP_CMD_AFTER].i_list, &state_list))
	 || (TCL_ERROR == update_expect_states(eg.i_list,&state_list))) {
		result = TCL_ERROR;
		goto cleanup;
	}

	/* declare ourselves "in sync" with external view of close/indirect */
	configure_count = exp_configure_count;

	/* count and validate state_list */
	mcount = 0;
	for (slPtr=state_list;slPtr;slPtr=slPtr->next) {
		mcount++;
		/* validate all input descriptors */
		if (!exp_fd2f(interp,slPtr->esPtr,1,1,"expect")) {
			result = TCL_ERROR;
			goto cleanup;
		}
	}

	/* make into an array */
	masters = (ExpState **)ckalloc(mcount * sizeof(ExpState *));
	for (slPtr=state_list,i=0;slPtr;slPtr=slPtr->next,i++) {
		masters[i] = slPtr->esPtr;
	}

     restart:
	if (first_time) first_time = 0;
	else time(&start_time);

	if (eg.timeout_specified_by_flag) {
		timeout = eg.timeout;
	} else {
		/* get the latest timeout */
		timeout = get_timeout(interp);
	}

	key = expect_key++;

	result = TCL_OK;
	last_esPtr = 0;

	/* end of restart code */

	eo.e = 0;		/* no final case yet */
	eo.esPtr = 0;		/* no final ExpState selected yet */
	eo.match = 0;		/* nothing matched yet */

	/* timeout code is a little tricky, be very careful changing it */
	if (timeout != EXP_TIME_INFINITY) {
		/* if exp_continue -continue_timer, do not update end_time */
		if (reset_timer) {
			time(&current_time);
			end_time = current_time + timeout;
		} else {
			reset_timer = TRUE;
		}
	}

	/* remtime and current_time updated at bottom of loop */
	remtime = timeout;

	for (;;) {
		if ((timeout != EXP_TIME_INFINITY) && (remtime < 0)) {
			cc = EXP_TIMEOUT;
		} else {
			cc = expect_read(interp,masters,mcount,&esPtr,remtime,key);
		}

		/*SUPPRESS 530*/
		if (cc == EXP_EOF) {
			/* do nothing */
		} else if (cc == EXP_TIMEOUT) {
			debuglog("expect: timed out\r\n");
		} else if (cc == EXP_RECONFIGURE) {
			reset_timer = FALSE;
			goto restart_with_update;
		} else if (cc < 0) { /* EXP_TCLERROR or any other weird value*/
			goto error;
		} else {
			/* new data if cc > 0, same old data if cc == 0 */

			/* below here, cc as general status */
			cc = EXP_NOMATCH;

			/* force redisplay of buffer when debugging */
			last_esPtr = 0;
		}

		cc = eval_cases(interp,&exp_cmds[EXP_CMD_BEFORE],
			esPtr,&eo,&last_esPtr,&last_case,cc,masters,mcount,"");
		cc = eval_cases(interp,&eg,
			esPtr,&eo,&last_esPtr,&last_case,cc,masters,mcount,"");
		cc = eval_cases(interp,&exp_cmds[EXP_CMD_AFTER],
			esPtr,&eo,&last_esPtr,&last_case,cc,masters,mcount,"");
		if (cc == EXP_TCLERROR) goto error;
		/* special eof code that cannot be done in eval_cases */
		/* or above, because it would then be executed several times */
		if (cc == EXP_EOF) {
			eo.esPtr = esPtr;
			eo.match = eo.esPtr->size;
			eo.buffer = eo.esPtr->buffer;
			debuglog("expect: read eof\r\n");
			break;
		} else if (cc == EXP_TIMEOUT) break;
		/* break if timeout or eof and failed to find a case for it */

		if (eo.e) break;

		/* no match was made with current data, force a read */
		esPtr->force_read = TRUE;

		if (timeout != EXP_TIME_INFINITY) {
			time(&current_time);
			remtime = end_time - current_time;
		}
	}

	goto done;

error:
	result = exp_2tcl_returnvalue(cc);
 done:
#define out(i,val)  debuglog("expect: set %s(%s) \"%s\"\r\n",EXPECT_OUT,i, \
						dprintify(val)); \
		    Tcl_SetVar2(interp,EXPECT_OUT,i,val,0);

	if (result != TCL_ERROR) {
		result = exp_process_matches(interp, &eo, 0 /* not bg */,
			"expect");
	}

 cleanup:
	if (result == EXP_CONTINUE_TIMER) {
		reset_timer = FALSE;
		result = EXP_CONTINUE;
	}

	if ((result == EXP_CONTINUE)
	     && (configure_count == exp_configure_count)) {
		debuglog("expect: continuing expect\r\n");
		goto restart;
	}

	if (state_list) {
		exp_free_state(state_list);
		state_list = 0;
	}
	if (masters) {
		ckfree((char *)masters);
		masters = 0;
	}

	if (result == EXP_CONTINUE) {
		debuglog("expect: continuing expect after update\r\n");
		goto restart_with_update;
	}

	free_ecases(interp,&eg,0);	/* requires i_lists to be avail */
	exp_free_i(interp,eg.i_list,exp_indirect_update2);

	return(result);
}
#undef out

/* beginning of deprecated code */

#define out(elt)		Tcl_SetVar2(interp,array,elt,ascii,0);
void
exp_timestamp(interp,timeval,array)
Tcl_Interp *interp;
time_t *timeval;
char *array;
{
	struct tm *tm;
	char *ascii;

	tm = localtime(timeval);	/* split */
	ascii = asctime(tm);		/* print */
	ascii[24] = '\0';		/* zap trailing \n */

	out("timestamp");

	sprintf(ascii,"%ld",*timeval);
	out("epoch");

	sprintf(ascii,"%d",tm->tm_sec);
	out("sec");
	sprintf(ascii,"%d",tm->tm_min);
	out("min");
	sprintf(ascii,"%d",tm->tm_hour);
	out("hour");
	sprintf(ascii,"%d",tm->tm_mday);
	out("mday");
	sprintf(ascii,"%d",tm->tm_mon);
	out("mon");
	sprintf(ascii,"%d",tm->tm_year);
	out("year");
	sprintf(ascii,"%d",tm->tm_wday);
	out("wday");
	sprintf(ascii,"%d",tm->tm_yday);
	out("yday");
	sprintf(ascii,"%d",tm->tm_isdst);
	out("isdst");
}
/* end of deprecated code */

/*ARGSUSED*/
static int
Exp_TimestampCmd(clientData, interp, argc, argv)
ClientData clientData;
Tcl_Interp *interp;
int argc;
char **argv;
{
	char *format = 0;
	time_t seconds = -1;
	int gmt = FALSE;	/* local time by default */
	struct tm *tm;
	Tcl_DString dstring;

	argc--; argv++;

	while (*argv) {
		if (streq(*argv,"-format")) {
			argc--; argv++;
			if (!*argv) goto usage_error;
			format = *argv;
			argc--; argv++;
		} else if (streq(*argv,"-seconds")) {
			argc--; argv++;
			if (!*argv) goto usage_error;
			seconds = atoi(*argv);
			argc--; argv++;
		} else if (streq(*argv,"-gmt")) {
			gmt = TRUE;
			argc--; argv++;
		} else break;
	}

	if (argc) goto usage_error;

	if (seconds == -1) {
		time(&seconds);
	}

	Tcl_DStringInit(&dstring);

	if (format) {
		if (gmt) {
			tm = gmtime(&seconds);
		} else {
			tm = localtime(&seconds);
		}
/*		exp_strftime(interp->result,TCL_RESULT_SIZE,format,tm);*/
		exp_strftime(format,tm,&dstring);
		Tcl_DStringResult(interp,&dstring);
	} else {
		sprintf(interp->result,"%ld",seconds);
	}
	
	return TCL_OK;
 usage_error:
	exp_error(interp,"args: [-seconds #] [-format format]");
	return TCL_ERROR;

}

/* lowmemcpy - like memcpy but it lowercases result */
void
exp_lowmemcpy(dest,src,n)
char *dest;
char *src;
int n;
{
	for (;n>0;n--) {
		*dest = ((isascii(*src) && isupper(*src))?tolower(*src):*src);
		src++;	dest++;
	}
}

/*ARGSUSED*/
int
Exp_MatchMaxCmd(clientData,interp,argc,argv)
ClientData clientData;
Tcl_Interp *interp;
int argc;
char **argv;
{
	int size = -1;
	ExpState *esPtr = 0;
	int m = -1;
	ExpState *f;
	int Default = FALSE;

	argc--; argv++;

	for (;argc>0;argc--,argv++) {
		if (streq(*argv,"-d")) {
			Default = TRUE;
		} else if (streq(*argv,"-i")) {
			argc--;argv++;
			if (argc < 1) {
				exp_error(interp,"-i needs argument");
				return(TCL_ERROR);
			}
			m = atoi(*argv);
		} else break;
	}

	if (!Default) {
		if (m == -1) {
			if (!(f = exp_update_master(interp,&esPtr,0,0)))
				return(TCL_ERROR);
		} else {
			if (!(f = exp_fd2f(interp,esPtr,0,0,"match_max")))
				return(TCL_ERROR);
		}
	} else if (m != -1) {
		exp_error(interp,"cannot do -d and -i at the same time");
		return(TCL_ERROR);
	}

	if (argc == 0) {
		if (Default) {
			size = exp_default_match_max;
		} else {
			size = esPtr->umsize;
		}
		sprintf(interp->result,"%d",size);
		return(TCL_OK);
	}

	if (argc > 1) {
		exp_error(interp,"too many arguments");
		return(TCL_OK);
	}

	/* all that's left is to set the size */
	size = atoi(argv[0]);
	if (size <= 0) {
		exp_error(interp,"must be positive");
		return(TCL_ERROR);
	}

	if (Default) exp_default_match_max = size;
	else esPtr->umsize = size;

	return(TCL_OK);
}

/*ARGSUSED*/
int
Exp_RemoveNullsCmd(clientData,interp,argc,argv)
ClientData clientData;
Tcl_Interp *interp;
int argc;
char **argv;
{
	int value = -1;
	int m = -1;
	ExpState *f;
	int Default = FALSE;

	argc--; argv++;

	for (;argc>0;argc--,argv++) {
		if (streq(*argv,"-d")) {
			Default = TRUE;
		} else if (streq(*argv,"-i")) {
			argc--;argv++;
			if (argc < 1) {
				exp_error(interp,"-i needs argument");
				return(TCL_ERROR);
			}
			m = atoi(*argv);
		} else break;
	}

	if (!Default) {
		if (m == -1) {
			if (!(f = exp_update_master(interp,&esPtr,0,0)))
				return(TCL_ERROR);
		} else {
			if (!(f = exp_fd2f(interp,esPtr,0,0,"remove_nulls")))
				return(TCL_ERROR);
		}
	} else if (m != -1) {
		exp_error(interp,"cannot do -d and -i at the same time");
		return(TCL_ERROR);
	}

	if (argc == 0) {
		if (Default) {
			value = exp_default_match_max;
		} else {
			value = esPtr->rm_nulls;
		}
		sprintf(interp->result,"%d",value);
		return(TCL_OK);
	}

	if (argc > 1) {
		exp_error(interp,"too many arguments");
		return(TCL_OK);
	}

	/* all that's left is to set the value */
	value = atoi(argv[0]);
	if (value != 0 && value != 1) {
		exp_error(interp,"must be 0 or 1");
		return(TCL_ERROR);
	}

	if (Default) exp_default_rm_nulls = value;
	else esPtr->rm_nulls = value;

	return(TCL_OK);
}

/*ARGSUSED*/
int
Exp_ParityCmd(clientData,interp,argc,argv)
ClientData clientData;
Tcl_Interp *interp;
int argc;
char **argv;
{
    int parity;
    ExpState *esPtr = 0;
    char *chanName = 0;
    int Default = FALSE;

    argc--; argv++;

    for (;argc>0;argc--,argv++) {
	if (streq(*argv,"-d")) {
	    Default = TRUE;
	} else if (streq(*argv,"-i")) {
	    argc--;argv++;
	    if (argc < 1) {
		exp_error(interp,"-i needs argument");
		return(TCL_ERROR);
	    }
	    chanName = *argv;
	} else break;
    }

    if (!Default) {
	if (!chanName) {
	    if (!(esPtr = expGetCurrentState(interp,0,0))) {
		return(TCL_ERROR);
	    }
	} else {
	    if (!(esPtr = expGetState(interp,chanName,0,0,"parity"))) {
		return(TCL_ERROR);
	    }
	}
    } else if (chanName) {
	exp_error(interp,"cannot do -d and -i at the same time");
	return(TCL_ERROR);
    }

    if (argc == 0) {
	if (Default) {
	    parity = exp_default_parity;
	} else {
	    parity = esPtr->parity;
	}
	sprintf(interp->result,"%d",parity);
	return(TCL_OK);
    }

    if (argc > 1) {
	exp_error(interp,"too many arguments");
	return(TCL_OK);
    }

    /* all that's left is to set the parity */
    parity = atoi(argv[0]);

    if (Default) exp_default_parity = parity;
    else esPtr->parity = parity;

    return(TCL_OK);
}

#if DEBUG_PERM_ECASES
/* This big chunk of code is just for debugging the permanent */
/* expect cases */
void
exp_fd_print(slPtr)
struct exp_state_list *slPtr;
{
	if (!slPtr) return;
	printf("%d ",slPtr->esPtr);
	exp_fd_print(slPtr->next);
}

void
exp_i_print(exp_i)
struct exp_i *exp_i;
{
	if (!exp_i) return;
	printf("exp_i %x",exp_i);
	printf((exp_i->direct == EXP_DIRECT)?" direct":" indirect");
	printf((exp_i->duration == EXP_PERMANENT)?" perm":" tmp");
	printf("  ecount = %d\n",exp_i->ecount);
	printf("variable %s, value %s\n",
		((exp_i->variable)?exp_i->variable:"--"),
		((exp_i->value)?exp_i->value:"--"));
	printf("ExpStates: ");
	exp_fd_print(exp_i->state_list); printf("\n");
	exp_i_print(exp_i->next);
}

void
exp_ecase_print(ecase)
struct ecase *ecase;
{
	printf("pat <%s>\n",ecase->pat);
	printf("exp_i = %x\n",ecase->i_list);
}

void
exp_ecases_print(ecd)
struct exp_cases_descriptor *ecd;
{
	int i;

	printf("%d cases\n",ecd->count);
	for (i=0;i<ecd->count;i++) exp_ecase_print(ecd->cases[i]);
}

void
exp_cmd_print(ecmd)
struct exp_cmd_descriptor *ecmd;
{
	printf("expect cmd type: %17s",exp_cmdtype_printable(ecmd->cmdtype));
	printf((ecmd->duration==EXP_PERMANENT)?" perm ": "tmp ");
	/* printdict */
	exp_ecases_print(&ecmd->ecd);
	exp_i_print(ecmd->i_list);
}

void
exp_cmds_print()
{
	exp_cmd_print(&exp_cmds[EXP_CMD_BEFORE]);
	exp_cmd_print(&exp_cmds[EXP_CMD_AFTER]);
	exp_cmd_print(&exp_cmds[EXP_CMD_BG]);
}

/*ARGSUSED*/
int
cmdX(clientData, interp, argc, argv)
ClientData clientData;
Tcl_Interp *interp;
int argc;
char **argv;
{
	exp_cmds_print();
	return TCL_OK;
}
#endif /*DEBUG_PERM_ECASES*/

void
expInitExpectVars()
{
    ThreadSpecificData *tsdPtr = TCL_TSD_INIT(&dataKey);

    tsdPtr->timeout = INIT_EXPECT_TIMEOUT;
}

/* need address for passing into cmdExpect */
static int spawn_id_bad = EXP_SPAWN_ID_BAD;
static int spawn_id_user = EXP_SPAWN_ID_USER;

static struct exp_cmd_data
cmd_data[]  = {
{"expect",	exp_proc(Exp_ExpectCmd),	(ClientData)&spawn_id_bad,	0},
{"expect_after",exp_proc(Exp_ExpectGlobalCmd),(ClientData)&exp_cmds[EXP_CMD_AFTER],0},
{"expect_before",exp_proc(Exp_ExpectGlobalCmd),(ClientData)&exp_cmds[EXP_CMD_BEFORE],0},
{"expect_user",	exp_proc(Exp_ExpectCmd),	(ClientData)&spawn_id_user,	0},
{"expect_tty",	exp_proc(Exp_ExpectCmd),	(ClientData)&exp_dev_tty,	0},
{"expect_background",exp_proc(Exp_ExpectGlobalCmd),(ClientData)&exp_cmds[EXP_CMD_BG],0},
{"match_max",	exp_proc(Exp_MatchMaxCmd),	0,	0},
{"remove_nulls",exp_proc(Exp_RemoveNullsCmd),	0,	0},
{"parity",	exp_proc(Exp_ParityCmd),		0,	0},
{"timestamp",	exp_proc(Exp_TimestampCmd),	0,	0},
{0}};

void
exp_init_expect_cmds(interp)
Tcl_Interp *interp;
{
	exp_create_commands(interp,cmd_data);

	Tcl_SetVar(interp,EXPECT_TIMEOUT,INIT_EXPECT_TIMEOUT_LIT,0);

	exp_cmd_init(&exp_cmds[EXP_CMD_BEFORE],EXP_CMD_BEFORE,EXP_PERMANENT);
	exp_cmd_init(&exp_cmds[EXP_CMD_AFTER ],EXP_CMD_AFTER, EXP_PERMANENT);
	exp_cmd_init(&exp_cmds[EXP_CMD_BG    ],EXP_CMD_BG,    EXP_PERMANENT);
	exp_cmd_init(&exp_cmds[EXP_CMD_FG    ],EXP_CMD_FG,    EXP_TEMPORARY);

	/* preallocate to one element, so future realloc's work */
	exp_cmds[EXP_CMD_BEFORE].ecd.cases = 0;
	exp_cmds[EXP_CMD_AFTER ].ecd.cases = 0;
	exp_cmds[EXP_CMD_BG    ].ecd.cases = 0;

	pattern_style[PAT_EOF] = "eof";
	pattern_style[PAT_TIMEOUT] = "timeout";
	pattern_style[PAT_DEFAULT] = "default";
	pattern_style[PAT_FULLBUFFER] = "full buffer";
	pattern_style[PAT_GLOB] = "glob pattern";
	pattern_style[PAT_RE] = "regular expression";
	pattern_style[PAT_EXACT] = "exact string";
	pattern_style[PAT_NULL] = "null";

#if 0
	Tcl_CreateCommand(interp,"x",
		cmdX,(ClientData)0,exp_deleteProc);
#endif
}

void
exp_init_sig() {
#if 0
	signal(SIGALRM,sigalarm_handler);
	signal(SIGINT,sigint_handler);
#endif
}
