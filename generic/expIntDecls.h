/*
 * expIntDecls.h --
 *
 *	Declarations of functions in the platform independent public
 *	Expect API.
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
 * RCS: @(#) $Id: expIntDecls.h,v 1.1.4.2 2002/02/10 10:17:04 davygrvy Exp $
 * ----------------------------------------------------------------------------
 */

#ifndef _EXPINTDECLS
#define _EXPINTDECLS

/*
 * WARNING: This file is automatically generated by the tools/genStubs.tcl
 * script.  Any modifications to the function declarations below should be made
 * in the exp.decls script.
 */

/* !BEGIN!: Do not edit below this line. */

/*
 * Exported function declarations:
 */

/* Slot 0 is reserved */
/* 1 */
TCL_EXTERN(int)		Exp_StringMatch _ANSI_ARGS_((CONST char * string, 
				CONST char * pattern, int * offset));
/* 2 */
TCL_EXTERN(int)		Exp_StringMatch2 _ANSI_ARGS_((CONST char * string, 
				CONST char * pattern));
/* Slot 3 is reserved */
/* 4 */
TCL_EXTERN(struct exp_i *) exp_new_i_complex _ANSI_ARGS_((
				Tcl_Interp * interp, char * arg, 
				int duration, Tcl_VarTraceProc * updateproc, 
				char * msg));
/* 5 */
TCL_EXTERN(struct exp_i *) exp_new_i_simple _ANSI_ARGS_((struct exp_f * fd, 
				int duration));
/* 6 */
TCL_EXTERN(struct exp_fs_list *) exp_new_fs _ANSI_ARGS_((struct exp_f * f));
/* 7 */
TCL_EXTERN(void)	exp_free_i _ANSI_ARGS_((Tcl_Interp * interp, 
				struct exp_i * i, 
				Tcl_VarTraceProc * updateproc));
/* 8 */
TCL_EXTERN(void)	exp_free_fs _ANSI_ARGS_((
				struct exp_fs_list * fs_first));
/* 9 */
TCL_EXTERN(void)	exp_free_fs_single _ANSI_ARGS_((
				struct exp_fs_list * fs));
/* 10 */
TCL_EXTERN(void)	exp_i_update _ANSI_ARGS_((Tcl_Interp * interp, 
				struct exp_i * i));
/* 11 */
TCL_EXTERN(void)	exp_pty_exit _ANSI_ARGS_((void));
/* 12 */
TCL_EXTERN(void)	exp_init_spawn_ids _ANSI_ARGS_((Tcl_Interp * interp));
/* 13 */
TCL_EXTERN(void)	exp_init_pty _ANSI_ARGS_((Tcl_Interp * interp));
/* 14 */
TCL_EXTERN(void)	exp_init_tty _ANSI_ARGS_((Tcl_Interp * interp));
/* 15 */
TCL_EXTERN(void)	exp_init_stdio _ANSI_ARGS_((void));
/* 16 */
TCL_EXTERN(void)	exp_init_sig _ANSI_ARGS_((void));
/* 17 */
TCL_EXTERN(void)	exp_init_trap _ANSI_ARGS_((void));
/* 18 */
TCL_EXTERN(void)	exp_init_unit_random _ANSI_ARGS_((void));
/* 19 */
TCL_EXTERN(void)	exp_init_spawn_id_vars _ANSI_ARGS_((
				Tcl_Interp * interp));
/* 20 */
TCL_EXTERN(void)	exp_adjust _ANSI_ARGS_((struct exp_f * f));
/* 21 */
TCL_EXTERN(void)	exp_ecmd_remove_f_direct_and_indirect _ANSI_ARGS_((
				Tcl_Interp * interp, struct exp_f * f));
/* 22 */
TCL_EXTERN(void)	exp_rearm_sigchld _ANSI_ARGS_((Tcl_Interp * interp));
/* 23 */
TCL_EXTERN(struct exp_f *) exp_chan2f _ANSI_ARGS_((Tcl_Interp * interp, 
				CONST char * chan, int opened, int adjust, 
				CONST char * msg));
/* 24 */
TCL_EXTERN(int)		exp_fcheck _ANSI_ARGS_((Tcl_Interp * interp, 
				struct exp_f * f, int opened, int adjust, 
				CONST char * msg));
/* 25 */
TCL_EXTERN(int)		exp_close _ANSI_ARGS_((Tcl_Interp * interp, 
				struct exp_f * f));
/* 26 */
TCL_EXTERN(void)	exp_strftime _ANSI_ARGS_((char * format, 
				const struct tm * timeptr, 
				Tcl_DString * dstring));

typedef struct ExpIntStubs {
    int magic;
    struct ExpIntStubHooks *hooks;

    void *reserved0;
    int (*exp_StringMatch) _ANSI_ARGS_((CONST char * string, CONST char * pattern, int * offset)); /* 1 */
    int (*exp_StringMatch2) _ANSI_ARGS_((CONST char * string, CONST char * pattern)); /* 2 */
    void *reserved3;
    struct exp_i * (*exp_new_i_complex) _ANSI_ARGS_((Tcl_Interp * interp, char * arg, int duration, Tcl_VarTraceProc * updateproc, char * msg)); /* 4 */
    struct exp_i * (*exp_new_i_simple) _ANSI_ARGS_((struct exp_f * fd, int duration)); /* 5 */
    struct exp_fs_list * (*exp_new_fs) _ANSI_ARGS_((struct exp_f * f)); /* 6 */
    void (*exp_free_i) _ANSI_ARGS_((Tcl_Interp * interp, struct exp_i * i, Tcl_VarTraceProc * updateproc)); /* 7 */
    void (*exp_free_fs) _ANSI_ARGS_((struct exp_fs_list * fs_first)); /* 8 */
    void (*exp_free_fs_single) _ANSI_ARGS_((struct exp_fs_list * fs)); /* 9 */
    void (*exp_i_update) _ANSI_ARGS_((Tcl_Interp * interp, struct exp_i * i)); /* 10 */
    void (*exp_pty_exit) _ANSI_ARGS_((void)); /* 11 */
    void (*exp_init_spawn_ids) _ANSI_ARGS_((Tcl_Interp * interp)); /* 12 */
    void (*exp_init_pty) _ANSI_ARGS_((Tcl_Interp * interp)); /* 13 */
    void (*exp_init_tty) _ANSI_ARGS_((Tcl_Interp * interp)); /* 14 */
    void (*exp_init_stdio) _ANSI_ARGS_((void)); /* 15 */
    void (*exp_init_sig) _ANSI_ARGS_((void)); /* 16 */
    void (*exp_init_trap) _ANSI_ARGS_((void)); /* 17 */
    void (*exp_init_unit_random) _ANSI_ARGS_((void)); /* 18 */
    void (*exp_init_spawn_id_vars) _ANSI_ARGS_((Tcl_Interp * interp)); /* 19 */
    void (*exp_adjust) _ANSI_ARGS_((struct exp_f * f)); /* 20 */
    void (*exp_ecmd_remove_f_direct_and_indirect) _ANSI_ARGS_((Tcl_Interp * interp, struct exp_f * f)); /* 21 */
    void (*exp_rearm_sigchld) _ANSI_ARGS_((Tcl_Interp * interp)); /* 22 */
    struct exp_f * (*exp_chan2f) _ANSI_ARGS_((Tcl_Interp * interp, CONST char * chan, int opened, int adjust, CONST char * msg)); /* 23 */
    int (*exp_fcheck) _ANSI_ARGS_((Tcl_Interp * interp, struct exp_f * f, int opened, int adjust, CONST char * msg)); /* 24 */
    int (*exp_close) _ANSI_ARGS_((Tcl_Interp * interp, struct exp_f * f)); /* 25 */
    void (*exp_strftime) _ANSI_ARGS_((char * format, const struct tm * timeptr, Tcl_DString * dstring)); /* 26 */
} ExpIntStubs;

#ifdef __cplusplus
extern "C" {
#endif
extern ExpIntStubs *expIntStubsPtr;
#ifdef __cplusplus
}
#endif

#if defined(USE_EXP_STUBS) && !defined(USE_EXP_STUB_PROCS)

/*
 * Inline function declarations:
 */

/* Slot 0 is reserved */
#ifndef Exp_StringMatch
#define Exp_StringMatch \
	(expIntStubsPtr->exp_StringMatch) /* 1 */
#endif
#ifndef Exp_StringMatch2
#define Exp_StringMatch2 \
	(expIntStubsPtr->exp_StringMatch2) /* 2 */
#endif
/* Slot 3 is reserved */
#ifndef exp_new_i_complex
#define exp_new_i_complex \
	(expIntStubsPtr->exp_new_i_complex) /* 4 */
#endif
#ifndef exp_new_i_simple
#define exp_new_i_simple \
	(expIntStubsPtr->exp_new_i_simple) /* 5 */
#endif
#ifndef exp_new_fs
#define exp_new_fs \
	(expIntStubsPtr->exp_new_fs) /* 6 */
#endif
#ifndef exp_free_i
#define exp_free_i \
	(expIntStubsPtr->exp_free_i) /* 7 */
#endif
#ifndef exp_free_fs
#define exp_free_fs \
	(expIntStubsPtr->exp_free_fs) /* 8 */
#endif
#ifndef exp_free_fs_single
#define exp_free_fs_single \
	(expIntStubsPtr->exp_free_fs_single) /* 9 */
#endif
#ifndef exp_i_update
#define exp_i_update \
	(expIntStubsPtr->exp_i_update) /* 10 */
#endif
#ifndef exp_pty_exit
#define exp_pty_exit \
	(expIntStubsPtr->exp_pty_exit) /* 11 */
#endif
#ifndef exp_init_spawn_ids
#define exp_init_spawn_ids \
	(expIntStubsPtr->exp_init_spawn_ids) /* 12 */
#endif
#ifndef exp_init_pty
#define exp_init_pty \
	(expIntStubsPtr->exp_init_pty) /* 13 */
#endif
#ifndef exp_init_tty
#define exp_init_tty \
	(expIntStubsPtr->exp_init_tty) /* 14 */
#endif
#ifndef exp_init_stdio
#define exp_init_stdio \
	(expIntStubsPtr->exp_init_stdio) /* 15 */
#endif
#ifndef exp_init_sig
#define exp_init_sig \
	(expIntStubsPtr->exp_init_sig) /* 16 */
#endif
#ifndef exp_init_trap
#define exp_init_trap \
	(expIntStubsPtr->exp_init_trap) /* 17 */
#endif
#ifndef exp_init_unit_random
#define exp_init_unit_random \
	(expIntStubsPtr->exp_init_unit_random) /* 18 */
#endif
#ifndef exp_init_spawn_id_vars
#define exp_init_spawn_id_vars \
	(expIntStubsPtr->exp_init_spawn_id_vars) /* 19 */
#endif
#ifndef exp_adjust
#define exp_adjust \
	(expIntStubsPtr->exp_adjust) /* 20 */
#endif
#ifndef exp_ecmd_remove_f_direct_and_indirect
#define exp_ecmd_remove_f_direct_and_indirect \
	(expIntStubsPtr->exp_ecmd_remove_f_direct_and_indirect) /* 21 */
#endif
#ifndef exp_rearm_sigchld
#define exp_rearm_sigchld \
	(expIntStubsPtr->exp_rearm_sigchld) /* 22 */
#endif
#ifndef exp_chan2f
#define exp_chan2f \
	(expIntStubsPtr->exp_chan2f) /* 23 */
#endif
#ifndef exp_fcheck
#define exp_fcheck \
	(expIntStubsPtr->exp_fcheck) /* 24 */
#endif
#ifndef exp_close
#define exp_close \
	(expIntStubsPtr->exp_close) /* 25 */
#endif
#ifndef exp_strftime
#define exp_strftime \
	(expIntStubsPtr->exp_strftime) /* 26 */
#endif

#endif /* defined(USE_EXP_STUBS) && !defined(USE_EXP_STUB_PROCS) */

/* !END!: Do not edit above this line. */

#endif /* _EXPINTDECLS */
