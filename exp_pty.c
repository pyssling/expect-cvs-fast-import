/* exp_pty.c - generic routines to allocate and test ptys

Written by: Don Libes, NIST,  3/9/93

Design and implementation of this program was paid for by U.S. tax
dollars.  Therefore it is public domain.  However, the author and NIST
would appreciate credit if this program or parts of it are used.

*/

#include "expect_cf.h"
#ifdef HAVE_UNISTD_H
#  include <unistd.h>
#endif
#ifdef HAVE_SYS_FCNTL_H
#  include <sys/fcntl.h>
#else
#  include <fcntl.h>
#endif
#include <sys/types.h>
#include <sys/stat.h>

#ifdef TIME_WITH_SYS_TIME
# include <sys/time.h>
# include <time.h>
#else
# if HAVE_SYS_TIME_H
#  include <sys/time.h>
# else
#  include <time.h>
# endif
#endif

#include <signal.h>
#include <setjmp.h>
#include <sys/file.h>
#include "tcl.h"
#include "exp_int.h"
#include "expect_comm.h"
#include "exp_rename.h"
#include "exp_pty.h"

#include <errno.h>

#if 0
void expDiagLog();
void expDiagLogU();
void expDiagLogPtrSet();
#endif

#ifndef TRUE
#define TRUE 1
#define FALSE 0
#endif

#ifdef O_NOCTTY
#define RDWR ((O_RDWR)|(O_NOCTTY))
#else
#define RDWR O_RDWR
#endif

static int locked = FALSE;
static char lock[] = "/tmp/ptylock.XXXX";	/* XX is replaced by pty id */
static char locksrc[50] = "/tmp/expect.pid"; /* pid is replaced by real pid */
	/* locksrc is used as the link source, i.e., something to link from */

static int i_read_errno;/* place to save errno, if i_read() == -1, so it
			   doesn't get overwritten before we get to read it */
#ifdef HAVE_SIGLONGJMP
static sigjmp_buf env;                /* for interruptable read() */
#else
static jmp_buf env;		/* for interruptable read() */
#endif  /* HAVE_SIGLONGJMP */

static int env_valid = FALSE;	/* whether we can longjmp or not */

/* sigalarm_handler and i_read are here just for supporting the sanity */
/* checking of pty slave devices.  I have only seen this happen on BSD */
/* systems, but it may need to be done to the other pty implementations */
/* as well. */

/* Note that this code is virtually replicated from other code in expect */
/* At some point, I'll dump one, but not until I'm satisfied no other */
/* changes are needed */

/*ARGSUSED*/
static RETSIGTYPE
sigalarm_handler(n)
int n;		/* unused, for compatibility with STDC */
{
#ifdef REARM_SIG
	signal(SIGALRM,sigalarm_handler);
#endif

	/* check env_valid first to protect us from the alarm occurring */
	/* in the window between i_read and alarm(0) */
#ifdef HAVE_SIGLONGJMP
	if (env_valid) siglongjmp(env,1);
#else
	if (env_valid) longjmp(env,1);
#endif  /* HAVE_SIGLONGJMP */
}

/* interruptable read */
static int
i_read(fd,buffer,length,timeout)
int fd;
char *buffer;
int length;
int timeout;
{
	int cc = -2;

	/* since setjmp insists on returning 1 upon longjmp(,0), */
	/* longjmp(,2) instead. */

	/* restart read if setjmp returns 0 (first time) or 2. */
	/* abort if setjmp returns 1. */

	alarm(timeout);

#ifdef HAVE_SIGLONGJMP
	if (1 != sigsetjmp(env,1)) {
#else
	if (1 != setjmp(env)) {
#endif  /* HAVE_SIGLONGJMP */
		env_valid = TRUE;
		cc = read(fd,buffer,length);
	}
	env_valid = FALSE;
	i_read_errno = errno;	/* errno can be overwritten by the */
				/* time we return */
	alarm(0);
	return(cc);
}

static RETSIGTYPE (*oldAlarmHandler)();
static RETSIGTYPE (*oldHupHandler)();
static time_t current_time;	/* time when testing began */

/* if TRUE, begin testing, else end testing */
/* returns -1 for failure, 0 for success */
int
exp_pty_test_start()
{
	int lfd;	/* locksrc file descriptor */

	oldAlarmHandler = signal(SIGALRM,sigalarm_handler);
#ifndef O_NOCTTY
	/* Ignore hangup signals generated by pty testing */
	/* when running in background with no control tty. */
	/* Very few systems don't define O_NOCTTY.  Only one */
	/* I know of is Next. */
	oldAlarmHandler = signal(SIGHUP,SIG_IGN);
#endif

	time(&current_time);

	/* recreate locksrc to prevent locks from 'looking old', so */
	/* that they are not deleted (later on in this code) */
	sprintf(locksrc,"/tmp/expect.%d",getpid());
	(void) unlink(locksrc);
	/* stanislav shalunov <shalunov@mccme.ru> notes that creat allows */
	/* race - someone could link to important file which root could then */
	/* smash. */
/*	if (-1 == (lfd = creat(locksrc,0777))) { */
       if (-1 == (lfd = open(locksrc,O_RDWR|O_CREAT|O_EXCL,0777))) {
		static char buf[256];
		exp_pty_error = buf;
		sprintf(exp_pty_error,"can't create %s, errno = %d\n",locksrc, errno);
		return(-1);
	}
	close(lfd);
	return 0;
}

void
exp_pty_test_end()
{
	signal(SIGALRM,oldAlarmHandler);
#ifndef O_NOCTTY
	signal(SIGALRM,oldHupHandler);
#endif
	(void) unlink(locksrc);
}

/* returns non-negative if successful */
int
exp_pty_test(master_name,slave_name,bank,num)
char *master_name;
char *slave_name;
char bank;
char *num;	/* string representation of number */
{
	int master, slave;
	int cc;
	char c;

	/* make a lock file to prevent others (for now only */
	/* expects) from allocating pty while we are playing */
	/* with it.  This allows us to rigorously test the */
	/* pty is usable. */
	if (exp_pty_lock(bank,num) == 0) {
		expDiagLogPtrStr("pty master (%s) is locked...skipping\r\n",master_name);
		return(-1);
	}
	/* verify no one else is using slave by attempting */
	/* to read eof from master side */
	if (0 > (master = open(master_name,RDWR))) return(-1);

#ifdef __QNX__

	/* QNX ptys don't have a lot of the same properties such as
           read 0 at EOF, etc */
	/* if 1 should pacify C compiler without using nested ifdefs */
 	if (1) return master;
#endif

#ifdef HAVE_PTYTRAP
	if (access(slave_name, R_OK|W_OK) != 0) {
		expDiagLogPtrStr("could not open slave for pty master (%s)...skipping\r\n",
			master_name);
		(void) close(master);
		return -1;
	}
	return(master);
#else
	if (0 > (slave = open(slave_name,RDWR))) {
		(void) close(master);
		return -1;
	}
	(void) close(slave);
	cc = i_read(master,&c,1,10);
	(void) close(master);
	if (!(cc == 0 || cc == -1)) {
		expDiagLogPtrStr("%s slave open, skipping\r\n",slave_name);
		locked = FALSE;	/* leave lock file around so Expect's avoid */
				/* retrying this pty for near future */
		return -1;
	}

	/* verify no one else is using master by attempting */
	/* to read eof from slave side */
	if (0 > (master = open(master_name,RDWR))) return(-1);
	if (0 > (slave = open(slave_name,RDWR))) {
		(void) close(master);
		return -1;
	}
	(void) close(master);
	cc = i_read(slave,&c,1,10);
	(void) close(slave);
	if (!(cc == 0 || cc == -1)) {
		expDiagLogPtrStr("%s master open, skipping\r\n",master_name);
		return -1;
	}

	/* seems ok, let's use it */
	expDiagLogPtrStr("using master pty %s\n",master_name);
	return(open(master_name,RDWR));
#endif
}

void
exp_pty_unlock()
{
	if (locked) {
		(void) unlink(lock);
		locked = FALSE;
	}
}

/* returns 1 if successfully locked, 0 otherwise */
int
exp_pty_lock(bank,num)
char bank;
char *num;	/* string representation of number */
{
	struct stat statbuf;

	if (locked) {
		unlink(lock);
		locked = FALSE;
	}

	sprintf(lock,"/tmp/ptylock.%c%s",bank,num);

	if ((0 == stat(lock,&statbuf)) &&
	    (statbuf.st_mtime+3600 < current_time)) {
		(void) unlink(lock);
	}

	if (-1 == (link(locksrc,lock))) locked = FALSE;
	else locked = TRUE;

	return locked;
}

/* 
 * expDiagLog needs a different definition, depending on whether its
 * called inside of Expect or the clib.  Allow it to be set using this
 * function.  It's done here because this file (and pty_XXX.c) are the 
 * ones that call expDiagLog from the two different environments.
 */

static void		(*expDiagLogPtrVal) _ANSI_ARGS_((CONST char *));

void
expDiagLogPtrSet(fn)
     void (*fn) _ANSI_ARGS_((CONST char *));
{
  expDiagLogPtrVal = fn;
}

void
expDiagLogPtr(str)
     char *str;
{
  (*expDiagLogPtrVal)(str);
}



void
expDiagLogPtrX(fmt,num)
     char *fmt;
     int num;
{
  static char buf[1000];
  sprintf(buf,fmt,num);
  (*expDiagLogPtrVal)(buf);
}


void
expDiagLogPtrStr(fmt,str1)
     char *fmt;
     char *str1;
{
  static char buf[1000];
  sprintf(buf,fmt,str1);
  (*expDiagLogPtrVal)(buf);
}

void
expDiagLogPtrStrStr(fmt,str1,str2)
     char *fmt;
     char *str1, *str2;
{
  static char buf[1000];
  sprintf(buf,fmt,str1,str2);
  (*expDiagLogPtrVal)(buf);
}

static CONST char *		(*expErrnoMsgVal) _ANSI_ARGS_((int));

CONST char *
expErrnoMsg(errorNo)
int errorNo;
{
  return (*expErrnoMsgVal)(errorNo);
}

void
expErrnoMsgSet(fn)
     CONST char * (*fn) _ANSI_ARGS_((int));
{
  expErrnoMsgVal = fn;
}
