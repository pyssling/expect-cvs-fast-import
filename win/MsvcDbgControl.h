#ifndef INC_MsvcDbgControl_h__
#define INC_MsvcDbgControl_h__

#include <windows.h>

extern WCHAR *	MsvcDbg_GetCommandLine(void);
extern int	MsvcDbg_Launch(const CHAR *wrkspace, Tcl_DString *cmdline,
			void **token);
#endif