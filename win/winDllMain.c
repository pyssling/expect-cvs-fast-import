#include "tcl.h"
#define WIN32_LEAN_AND_MEAN
#include <windows.h>

#ifdef _MSC_VER
    /* Only do this when MSVC++ is compiling us. */
#   ifdef USE_TCL_STUBS
#	pragma comment (lib, "tclstub" \
		STRINGIFY(JOIN(TCL_MAJOR_VERSION,TCL_MINOR_VERSION)) ".lib")
#	if !defined(_MT) || !defined(_DLL) || defined(_DEBUG)
	    /* This fixes a bug with how the Stubs library was compiled.
	     * The requirement for msvcrt.lib from tclstubXX.lib must
	     * be removed.  This bug has been fixed since 8.4a3, I beleive. */
#	    pragma comment(linker, "-nodefaultlib:msvcrt.lib")
#	endif
#   else
#	ifdef _DEBUG
#	    pragma comment (lib, "tcl" \
		    STRINGIFY(JOIN(TCL_MAJOR_VERSION,TCL_MINOR_VERSION)) "d.lib")
#	else
#	    pragma comment (lib, "tcl" \
		    STRINGIFY(JOIN(TCL_MAJOR_VERSION,TCL_MINOR_VERSION)) ".lib")
#	endif
#   endif
#endif


/* public global */
HMODULE expDllInstance;

BOOL WINAPI
DllMain (HINSTANCE hInst, ULONG ulReason, LPVOID lpReserved)
{
    switch (ulReason) {
    case DLL_PROCESS_ATTACH:
	expDllInstance = hInst;
    }
    return TRUE;
}
