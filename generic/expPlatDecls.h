/* ----------------------------------------------------------------------------
 * expPlatDecls.h --
 *
 *	Declarations of platform specific Expect APIs.
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
 * RCS: @(#) $Id: expPlatDecls.h,v 1.1.4.1 2002/02/10 02:58:53 davygrvy Exp $
 * ----------------------------------------------------------------------------
 */

#ifndef _EXPPLATDECLS
#define _EXPPLATDECLS

/*
 *  Pull in the definition of TCHAR.  Hopefully the compile flags
 *  of the core are matching against your project build for these
 *  public functions.  BE AWARE.
 */
#ifdef __WIN32__
#   ifndef _TCHAR_DEFINED
#	include <tchar.h>
#	ifndef _TCHAR_DEFINED
	    /* Borland seems to forget to set this. */
	    typedef _TCHAR TCHAR;
#	    define _TCHAR_DEFINED
#	endif
#   endif
#endif

	    
/* !BEGIN!: Do not edit below this line. */

/*
 * Exported function declarations:
 */


typedef struct ExpPlatStubs {
    int magic;
    struct ExpPlatStubHooks *hooks;

} ExpPlatStubs;

#ifdef __cplusplus
extern "C" {
#endif
extern ExpPlatStubs *expPlatStubsPtr;
#ifdef __cplusplus
}
#endif

#if defined(USE_EXP_STUBS) && !defined(USE_EXP_STUB_PROCS)

/*
 * Inline function declarations:
 */


#endif /* defined(USE_EXP_STUBS) && !defined(USE_EXP_STUB_PROCS) */

/* !END!: Do not edit above this line. */

#endif /* _EXPPLATDECLS */


