/*
 * exp.h --
 *
 *	Public include file for using the Expect extension.
 * 
 * Written by: Don Libes, libes@cme.nist.gov, NIST, 12/3/90
 * 
 * Design and implementation of this program was paid for by U.S. tax
 * dollars.  Therefore it is public domain.  However, the author and NIST
 * would appreciate credit if this program or parts of it are used.
 * 
 * Modified in October, 2001 by David Gravereaux for windows.
 *
 * RCS: @(#) $Id: exp.h,v 1.1.2.2 2001/10/28 01:46:31 davygrvy Exp $
 */

#ifndef _EXP
#define _EXP

#ifndef _TCL
#   include "tcl.h"
#endif

/*
 *  Version stuff.
 */

#define EXP_MAJOR_VERSION   6
#define EXP_MINOR_VERSION   0
#define EXP_RELEASE_LEVEL   TCL_ALPHA_RELEASE
#define EXP_RELEASE_SERIAL  1

#define EXP_VERSION	   STRINGIFY(JOIN(EXP_MAJOR_VERSION,JOIN(.,EXP_MINOR_VERSION)))

#if	EXP_RELEASE_LEVEL == TCL_ALPHA_RELEASE
#	define EXP_PATCH_LEVEL \
		STRINGIFY( \
			JOIN(JOIN(EXP_MAJOR_VERSION, \
			JOIN(., EXP_MINOR_VERSION)), \
			JOIN(a, EXP_RELEASE_SERIAL)))

#elif	EXP_RELEASE_LEVEL == TCL_BETA_RELEASE
#	define EXP_PATCH_LEVEL \
		STRINGIFY( \
			JOIN(JOIN(EXP_MAJOR_VERSION, \
			JOIN(., EXP_MINOR_VERSION)), \
			JOIN(b, EXP_RELEASE_SERIAL)))

#elif	EXP_RELEASE_LEVEL == TCL_FINAL_RELEASE
#	define EXP_PATCH_LEVEL \
		STRINGIFY( \
			JOIN(JOIN(EXP_MAJOR_VERSION, \
			JOIN(., EXP_MINOR_VERSION)), \
			JOIN(., EXP_RELEASE_SERIAL)))

#else
#	include "bad/release/level/used"
#endif

/*
 * The resource compiler defines this by default.  Skip the rest of this
 * file when included from an rc script.
 */
#ifndef RC_INVOKED


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


/*
 * Fix the Borland bug that's been in Tcl since when dinosaurs roamed the
 * earth.
 */
#ifndef TCL_EXTERN
#   undef DLLIMPORT
#   undef DLLEXPORT
#   if defined(__WIN32__) && (defined(_MSC_VER) || (defined(__GNUC__) && defined(__declspec)))
#	define DLLIMPORT __declspec(dllimport)
#	define DLLEXPORT __declspec(dllexport)
#   elif defined(__BORLANDC__)
#	define DLLIMPORT __import
#	define DLLEXPORT __export
#   else
#	define DLLIMPORT
#	define DLLEXPORT
#   endif
    /*
     * Make sure name mangling won't happen when the c++ language extensions
     * are used.
     */
#   ifdef __cplusplus
#	define TCL_CPP "C"
#   else
#	define TCL_CPP
#   endif
    /*
     * Borland requires the attributes be placed after the return type.
     */
#   ifdef __BORLANDC__
#	define TCL_EXTERN(rtnType) extern TCL_CPP rtnType TCL_STORAGE_CLASS
#   else
#	define TCL_EXTERN(rtnType) extern TCL_CPP TCL_STORAGE_CLASS rtnType
#   endif
#endif


/* common return codes for Expect functions */
/* The library actually only uses TIMEOUT and EOF */
#define EXP_ABEOF	-1	/* abnormal eof in Expect */
				/* when in library, this define is not used. */
				/* Instead "-1" is used literally in the */
				/* usual sense to check errors in system */
				/* calls */
#define EXP_TIMEOUT	-2
#define EXP_TCLERROR	-3
#define EXP_FULLBUFFER	-5
#define EXP_MATCH	-6
#define EXP_NOMATCH	-7
#define EXP_CANTMATCH	EXP_NOMATCH
#define EXP_CANMATCH	-8
#define EXP_DATA_NEW	-9	/* if select says there is new data */
#define EXP_DATA_OLD	-10	/* if we already read data in another cmd */
#define EXP_EOF		-11
#define EXP_RECONFIGURE	-12	/* changes to indirect spawn id lists */
				/* require us to reconfigure things */

/* in the unlikely event that a signal handler forces us to return this */
/* through expect's read() routine, we temporarily convert it to this. */
#define EXP_TCLRET	-20
#define EXP_TCLCNT	-21
#define EXP_TCLCNTTIMER	-22
#define EXP_TCLBRK	-23
#define EXP_TCLCNTEXP	-24
#define EXP_TCLRETTCL	-25

/* yet more TCL return codes */
/* Tcl does not safely provide a way to define the values of these, so */
/* use ridiculously different numbers for safety */
#define EXP_CONTINUE		-101	/* continue expect command */
					/* and restart timer */
#define EXP_CONTINUE_TIMER	-102	/* continue expect command */
					/* and continue timer */
#define EXP_TCL_RETURN		-103	/* converted by interact */
					/* and interpeter from */
					/* inter_return into */
					/* TCL_RETURN*/


/* Protos for exp_win.c, not yet move to the Stubs table. */
TCL_EXTERN(int)	    exp_window_size_set	    _ANSI_ARGS_((int fd));
TCL_EXTERN(int)	    exp_window_size_get	    _ANSI_ARGS_((int fd));
TCL_EXTERN(void)    exp_win_rows_set	    _ANSI_ARGS_((char *rows));
TCL_EXTERN(void)    exp_win_rows_get	    _ANSI_ARGS_((char *rows));
TCL_EXTERN(void)    exp_win_columns_set	    _ANSI_ARGS_((char *columns));
TCL_EXTERN(void)    exp_win_columns_get	    _ANSI_ARGS_((char *columns));
TCL_EXTERN(int)	    exp_win2_size_get	    _ANSI_ARGS_((int fd));
TCL_EXTERN(int)	    exp_win2_size_set	    _ANSI_ARGS_((int fd));
TCL_EXTERN(void)    exp_win2_rows_set	    _ANSI_ARGS_((int fd, char *rows));
TCL_EXTERN(void)    exp_win2_rows_get	    _ANSI_ARGS_((int fd, char *rows));
TCL_EXTERN(void)    exp_win2_columns_set    _ANSI_ARGS_((int fd, char *columns));
TCL_EXTERN(void)    exp_win2_columns_get    _ANSI_ARGS_((int fd, char *columns));

/* from expect_tcl.h */
TCL_EXTERN(int)	Expect_Init _ANSI_ARGS_((Tcl_Interp *));	/* for Tcl_AppInit apps */
TCL_EXTERN(void)	exp_parse_argv _ANSI_ARGS_((Tcl_Interp *,int argc,char **argv));
TCL_EXTERN(int)	exp_interpreter _ANSI_ARGS_((Tcl_Interp *,Tcl_Obj *));
TCL_EXTERN(int)	exp_interpret_cmdfile _ANSI_ARGS_((Tcl_Interp *,FILE *));
TCL_EXTERN(int)	exp_interpret_cmdfilename _ANSI_ARGS_((Tcl_Interp *,char *));
TCL_EXTERN(void)	exp_interpret_rcfiles _ANSI_ARGS_((Tcl_Interp *,int my_rc,int sys_rc));

TCL_EXTERN(char *)	exp_cook _ANSI_ARGS_((char *s,int *len));
TCL_EXTERN(void)	expCloseOnExec _ANSI_ARGS_((int));

			/* app-specific exit handler */
//TCL_EXTERN(void)	*exp_app_exit _ANSI_ARGS_((Tcl_Interp *));
//TCL_EXTERN(void)	exp_exit_handlers _ANSI_ARGS_((ClientData));
TCL_EXTERN(void)	exp_error _ANSI_ARGS_(TCL_VARARGS(Tcl_Interp *,interp));
TCL_EXTERN(int)		exp_getpidproc _ANSI_ARGS_((void));



/*
 * Include the public function declarations that are accessible via
 * the stubs table.
 */

#include "expDecls.h"


/*
 * Exp_InitStubs is used by apps/extensions that want to link
 * against the expect stubs library.  If we are not using stubs,
 * then this won't be declared.
 */

#ifdef USE_EXP_STUBS
extern TCL_CPP
CONST char *Exp_InitStubs _ANSI_ARGS_((Tcl_Interp *interp, char *version,
		int exact));
#endif


#undef TCL_STORAGE_CLASS
#define TCL_STORAGE_CLASS DLLIMPORT

#endif /* RC_INVOKED */
#endif /* _EXP */