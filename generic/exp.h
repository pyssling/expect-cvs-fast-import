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
 * RCS: @(#) $Id: exp.h,v 1.1.2.1 2001/10/28 01:02:39 davygrvy Exp $
 */

#ifndef _EXP_H__
#define _EXP_H__

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
#	error "bad release level"
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
     * Make sure name mangling won't happen when the c++ language is used.
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




/*
 * Include the public function declarations that are accessible via
 * the stubs table.
 */

#include "expDecls.h"


/*
 * Exp_InitStubs is used by extensions that can be linked
 * against the exp stubs library.  If we are not using stubs,
 * then this won't be declared.
 */

#ifdef USE_EXP_STUBS

#ifdef __cplusplus
extern "C"
#endif
CONST char *Exp_InitStubs _ANSI_ARGS_((Tcl_Interp *interp,
			    char *version, int exact));
#endif

/*
 * end of header
 * reset TCL_STORAGE_CLASS to DLLIMPORT.
 */
#undef TCL_STORAGE_CLASS
#define TCL_STORAGE_CLASS DLLIMPORT

#endif /* RC_INVOKED */
#endif /* _EXP_H__ */