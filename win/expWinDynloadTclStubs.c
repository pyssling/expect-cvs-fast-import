/* ----------------------------------------------------------------------------
 * expWinDynloadTclStubs.c --
 *
 *	Grabs and loads tclXX.dll from the EXP_TCLDLL environment variable.
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
 * Copyright (c) 2001 Telindustrie, LLC
 *	work by David Gravereaux <davygrvy@pobox.com> for any Win32 OS.
 *
 * ----------------------------------------------------------------------------
 * URLs:    http://expect.nist.gov/
 *	    http://expect.sf.net/
 *	    http://bmrc.berkeley.edu/people/chaffee/expectnt.html
 * ----------------------------------------------------------------------------
 * RCS: @(#) $Id: exp.h,v 1.1.2.5 2001/10/29 06:40:29 davygrvy Exp $
 * ----------------------------------------------------------------------------
 */

#include "expWinInt.h"

void
ExpDynloadTclStubs (void)
{
    TCHAR TclDLLPath[MAX_PATH+1];
    HMODULE hTclMod;
    typedef Tcl_Interp *(*LPFN_createInterpProc) ();
    LPFN_createInterpProc createInterpProc;
    Tcl_Interp *interp;
    char appname[MAX_PATH+1];


    if (GetEnvironmentVariable(_T("EXP_TCLDLL"), TclDLLPath, MAX_PATH)) {
	/* Load it */
	if (!(hTclMod = LoadLibrary(TclDLLPath))) {
	    EXP_LOG1(MSG_STUBS_TCLDLLCANTFIND, TclDLLPath);
	}

	/* LoadLibrary() loaded the module correctly.
	 * Get the location of Tcl_CreateInterp. */
	
	if ((createInterpProc = (LPFN_createInterpProc) GetProcAddress(hTclMod,
	    "Tcl_CreateInterp")) == NULL) {
	    EXP_LOG1(MSG_STUBS_NOCREATEINTERP, TclDLLPath);
	}
	interp = createInterpProc();
	if (Tcl_InitStubs(interp, "8.1", 0) == NULL) {
	    EXP_LOG1(MSG_STUBS_INITSTUBS, interp->result);
	}

	/* Discover the calling application.
	 * Use the ascii API to be safe. */
	GetModuleFileNameA(NULL, appname, MAX_PATH);
	Tcl_FindExecutable(appname);

	/* we're done initializing the core, and now don't need this
	 * interp anymore. */
	Tcl_DeleteInterp(interp);
    } else {
	/* envar not found */
	EXP_LOG0(MSG_STUBS_ENVARNOTSET);
    }
}
