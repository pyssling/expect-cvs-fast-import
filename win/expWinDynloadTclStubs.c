#include "tcl.h"
#include "tclPort.h"
#include "expWin.h"

int
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
	    EXP_LOG("\"%s\" failed to load", TclDLLPath);
	    return 0;
	}

	/* LoadLibrary() loaded the module correctly.
	   Get the location of Tcl_CreateInterp. */
	
	if ((createInterpProc = (LPFN_createInterpProc) GetProcAddress(hTclMod,
	    _T("Tcl_CreateInterp"))) == NULL) {
	    EXP_LOG("Tcl_CreateInterp() not found in \"%s\"!", TclDLLPath);
	    return 0;
	}
	interp = createInterpProc();
	if (Tcl_InitStubs(interp, "8.1", 0) == NULL) {
	    EXP_LOG("Tcl_InitStubs() failed with \"%s\".", interp->result);
	}

	/* Discover the calling application.
	 * Use the ascii API to be safe. */
	GetModuleFileNameA(NULL, appname, MAX_PATH);
	Tcl_FindExecutable(appname);

	/* we're done initializing the core, and now don't need this
	 * interp anymore. */
	Tcl_DeleteInterp(interp);
    } else {
	exit(999);  /* envar not found */
    }
    return 1;
}