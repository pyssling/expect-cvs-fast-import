/* ----------------------------------------------------------------------------
 * expWinDynloadTclStubs.cpp --
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
 * Copyright (c) 2003 ActiveState Corporation
 *	work by David Gravereaux <davygrvy@pobox.com> for any Win32 OS.
 *
 * ----------------------------------------------------------------------------
 * URLs:    http://expect.sf.net/
 *	    http://expect.nist.gov/
 *	    http://bmrc.berkeley.edu/people/chaffee/expectnt.html
 * ----------------------------------------------------------------------------
 * RCS: @(#) $Id: expWinDynloadTclStubs.cpp,v 1.1.2.2 2002/03/12 21:06:51 davygrvy Exp $
 * ----------------------------------------------------------------------------
 */

#include "tcl.h"

// We need at least the Tcl_Obj interface that was started in 8.0
#if TCL_MAJOR_VERSION < 8
#   error "we need Tcl 8.0 or greater to build this"

// Check for Stubs compatibility when asked for it.
#elif defined(USE_TCL_STUBS) && TCL_MAJOR_VERSION == 8 && \
	(TCL_MINOR_VERSION == 0 || \
        (TCL_MINOR_VERSION == 1 && TCL_RELEASE_LEVEL != TCL_FINAL_RELEASE))
#   error "Stubs interface doesn't work in 8.0 and alpha/beta 8.1; only 8.1.0+"
#endif

#ifdef _MSC_VER
    // Only do this when MSVC++ is compiling us.
#   ifdef USE_TCL_STUBS
	// Mark this .obj as needing tcl's Stubs library.
#	pragma comment(lib, "tclstub" \
	    STRINGIFY(JOIN(TCL_MAJOR_VERSION,TCL_MINOR_VERSION)) ".lib")
#	if !defined(_MT) || !defined(_DLL) || defined(_DEBUG)
	    // This fixes a bug with how the Stubs library was compiled.
	    // The requirement for msvcrt.lib from tclstubXX.lib should
	    // be removed.
#	    pragma comment(linker, "-nodefaultlib:msvcrt.lib")
#	endif
#   else
    // Mark this .obj needing the import library
#   pragma comment(lib, "tcl" \
	STRINGIFY(JOIN(TCL_MAJOR_VERSION,TCL_MINOR_VERSION)) ".lib")
#   endif
#endif

#include "slavedrvmc.h"
#include <windows.h>
#include "expWinUtils.hpp"

static HMODULE hTclMod;

void
DynloadTclStubs (void)
{
    TCHAR TclDLLPath[MAX_PATH+1];
    typedef Tcl_Interp *(*LPFN_createInterpProc) ();
    LPFN_createInterpProc createInterpProc;
    Tcl_Interp *interp;
    char appname[MAX_PATH+1];

    if (GetEnvironmentVariable("EXP_TCLDLL", TclDLLPath, MAX_PATH)) {
	/* Load it */
	if (!(hTclMod = LoadLibrary(TclDLLPath))) {
	    EXP_LOG1(MSG_STUBS_TCLDLLCANTFIND, TclDLLPath);
	}

	/* LoadLibrary() loaded the module correctly.
	 * Get the location of Tcl_CreateInterp. */
	
	if ((createInterpProc = (LPFN_createInterpProc) GetProcAddress(hTclMod,
	    "Tcl_CreateInterp")) == 0L) {
	    EXP_LOG1(MSG_STUBS_NOCREATEINTERP, TclDLLPath);
	}
	interp = createInterpProc();
	if (Tcl_InitStubs(interp, "8.1", 0) == 0L) {
	    EXP_LOG1(MSG_STUBS_INITSTUBS, interp->result);
	}

	/* Discover the calling application. */
	GetModuleFileName(0L, appname, MAX_PATH);
	Tcl_FindExecutable(appname);

	/* we're done initializing the core, and now don't need this
	 * interp anymore. */
	Tcl_DeleteInterp(interp);
    } else {
	/* envar not found */
	EXP_LOG0(MSG_STUBS_ENVARNOTSET);
    }
}

void
ShutdownTcl (void)
{
    Tcl_Finalize();
    FreeLibrary(hTclMod);
}
