/* ----------------------------------------------------------------------------
 * exp.h --
 *
 *	Public include file for using the Expect extension.
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
 * Copyright (c) 2002 Telindustrie, LLC
 *	work by David Gravereaux <davygrvy@pobox.com> for any Win32 OS.
 *
 * ----------------------------------------------------------------------------
 * URLs:    http://expect.nist.gov/
 *	    http://expect.sf.net/
 *	    http://bmrc.berkeley.edu/people/chaffee/expectnt.html
 * ----------------------------------------------------------------------------
 * RCS: @(#) $Id: exp.h,v 1.1.4.3 2002/02/10 09:03:41 davygrvy Exp $
 * ----------------------------------------------------------------------------
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
#define EXP_RELEASE_SERIAL  0

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


/* according to Karl Vogel, time.h is insufficient on Pyramid */
/* the following is recommended by autoconf */

#ifdef TIME_WITH_SYS_TIME
#   include <sys/time.h>
#   include <time.h>
#else
#   ifdef HAVE_SYS_TIME_H
#	include <sys/time.h>
#   else
#	include <time.h>
#   endif
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
 * Fix the Borland bug that's in Tcl.
 */
#ifndef TCL_EXTERN
#   undef DLLIMPORT
#   undef DLLEXPORT
#   if (defined(__WIN32__) &&  (defined(_MSC_VER) || (__BORLANDC__ >= 0x0550) || \
	    (defined(__GNUC__) && defined(__DECLSPEC_SUPPORTED)))) || \
	    (defined(MAC_TCL) && FUNCTION_DECLSPEC)
#	define DLLIMPORT __declspec(dllimport)
#	define DLLEXPORT __declspec(dllexport)
#   elif defined(__BORLANDC__)
#	define OLD_BORLAND 1
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
     * Pre 5.5 Borland requires the attributes be placed after the return type.
     */
#   if OLD_BORLAND
#	define TCL_EXTERN(rtnType) extern TCL_CPP rtnType TCL_STORAGE_CLASS
#   else
#	define TCL_EXTERN(rtnType) extern TCL_CPP TCL_STORAGE_CLASS rtnType
#   endif
#endif

#define SCRIPTDIR	"example/"
#define EXECSCRIPTDIR	"example/"


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
/*#define EXP_CANTMATCH	EXP_NOMATCH
#define EXP_CANMATCH	-8*/
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

#define EXP_TIME_INFINITY	-1
#define EXP_SPAWN_ID_BAD	-1


/*
 * Include the public function declarations that are accessible via
 * the stubs table.
 */

#include "expDecls.h"

/*
 * Include platform specific public function declarations that are
 * accessible via the stubs table.
 */

#include "expPlatDecls.h"

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
