# exp.decls --
#
#	This file contains the declarations for all supported public
#	functions that are exported by the Expect library via the stubs table.
#	This file is used to generate the expDecls.h, expPlatDecls.h,
#	expIntDecls.h, and expStub.c files.
#
# RCS: @(#) $Id: exp.decls,v 1.1.2.2 2001/10/29 06:40:29 davygrvy Exp $

library exp

# Define the tcl interface with several sub interfaces:
#     expPlat	 - platform specific public
#     expInt	 - generic private
#     expIntPlat - platform specific private

interface exp
hooks {expPlat expInt expIntPlat}

# Declare each of the functions in the public Expect interface.  Note that
# the an index should never be reused for a different function in order
# to preserve backwards compatibility.

declare 0 generic {
	int Expect_Init (Tcl_Interp *interp)
}
declare 1 generic {
	int Expect_SafeInit (Tcl_Interp *interp)
}
declare 2 generic {
	void exp_parse_argv (Tcl_Interp *interp, int argc, char **argv)
}
declare 3 generic {
	int exp_interpreter (Tcl_Interp *interp, Tcl_Obj *eofObj)
}
declare 4 generic {
	int exp_interpret_cmdfile (Tcl_Interp *interp, Tcl_Channel file)
}
declare 5 generic {
	int exp_interpret_cmdfilename (Tcl_Interp *interp, char *filename)
}
declare 6 generic {
	void exp_interpret_rcfiles (Tcl_Interp *interp, int my_rc, int sys_rc)
}
declare 7 generic {
	char *exp_cook (char *s, int *len)
}
declare 8 generic {
	void expCloseOnExec (int fd)
}
declare 9 generic {
	int exp_getpidproc (void)
}
declare 10 generic {
	Tcl_Channel ExpCreateSpawnChannel (Tcl_Interp *interp, Tcl_Channel chan)
}

interface expPlat

interface expInt

interface expIntPlat

#====================================================================================
# UNIX specific publics.


#====================================================================================
# WIN32 specific publics.
declare 0 win {
	DWORD ExpWinApplicationType (const char *originalName, char *fullPath)
}
declare 1 win {
	DWORD ExpWinCreateProcess (int argc, char **argv, HANDLE inputHandle,
	    HANDLE outputHandle, HANDLE errorHandle, int allocConsole,
	    int hideConsole, int debug, int newProcessGroup, Tcl_Pid *pidPtr,
	    PDWORD globalPidPtr)
}
declare 2 win {
	void ExpWinSyslog (DWORD errId, ...)
}
declare 3 win {
	TCHAR* ExpSyslogGetSysMsg (DWORD errId)
}
declare 4 win {
	Tcl_Pid	Exp_WaitPid (Tcl_Pid pid, int *statPtr, int options)
}
declare 5 win {
	void Exp_KillProcess (Tcl_Pid pid)
}


#====================================================================================
# MAC specific publics.

### You nutts!!  can't do Mac... sorry..


