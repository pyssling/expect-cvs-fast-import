/* Dbg.h - Tcl Debugger include file

Written by: Don Libes, NIST, 3/23/93

Design and implementation of this program was paid for by U.S. tax
dollars.  Therefore it is public domain.  However, the author and NIST
would appreciate credit if this program or parts of it are used.

*/

/* _DEBUG or _DBG is just too likely, use something more unique */
#ifndef _NIST_DBG
#define _NIST_DBG

#include "tcl.h"

#ifndef TCL_NEWEXTERN
#   ifdef __cplusplus
#	define OLDEXTERN    extern "C" TCL_STORAGE_CLASS
#   else
#	define OLDEXTERN    extern TCL_STORAGE_CLASS
#   endif
#   undef EXTERN
#   define EXTERN(a) OLDEXTERN a
#endif


#undef TCL_STORAGE_CLASS
#define TCL_STORAGE_CLASS DLLEXPORT


typedef int (Dbg_InterProc) _ANSI_ARGS_((Tcl_Interp *interp, ClientData data));
typedef int (Dbg_IgnoreFuncsProc) _ANSI_ARGS_((
			Tcl_Interp *interp,
			char *funcname));
typedef void (Dbg_OutputProc) _ANSI_ARGS_((
			Tcl_Interp *interp,
			char *output,
			ClientData data));

typedef struct {
  Dbg_InterProc *func;
  ClientData data;
} Dbg_InterStruct;

typedef struct {
  Dbg_OutputProc *func;
  ClientData data;
} Dbg_OutputStruct;

char * Dbg_VarName;
char * Dbg_DefaultCmdName;

/* trivial interface, creates a "debug" command in your interp */
EXTERN(int) Tcldbg_Init _ANSI_ARGS_((Tcl_Interp *));

void Dbg_On _ANSI_ARGS_((Tcl_Interp *interp,
					int immediate));
void Dbg_Off _ANSI_ARGS_((Tcl_Interp *interp));
char ** Dbg_ArgcArgv _ANSI_ARGS_((int argc,char *argv[],
					int copy));
int Dbg_Active _ANSI_ARGS_((Tcl_Interp *interp));
Dbg_InterStruct Dbg_Interactor _ANSI_ARGS_((
					Tcl_Interp *interp,
					Dbg_InterProc *interactor,
					ClientData data));
Dbg_IgnoreFuncsProc * Dbg_IgnoreFuncs _ANSI_ARGS_((
					Tcl_Interp *interp,
					Dbg_IgnoreFuncsProc *));
Dbg_OutputStruct Dbg_Output _ANSI_ARGS_((
					Tcl_Interp *interp,
					Dbg_OutputProc *,
					ClientData data));

#endif /* _NIST_DBG */
