#include "tclPort.h"
#include "expWin.h"
#include "spawndrvmc.h"

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
	    EXP_LOG1(MSG_STUBS_TCLDLLCANTFIND, TclDLLPath);
	}

	/* LoadLibrary() loaded the module correctly.
	 * Get the location of Tcl_CreateInterp. */
	
	if ((createInterpProc = (LPFN_createInterpProc) GetProcAddress(hTclMod,
	    _T("Tcl_CreateInterp"))) == NULL) {
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
    return 1;
}
