# exp.decls --
#
#	This file contains the declarations for all supported public
#	functions that are exported by the Expect library via the stubs table.
#	This file is used to generate the expDecls.h, expPlatDecls.h,
#	expIntDecls.h, and expStub.c files.
#
# RCS: @(#) $Id: exp.decls,v 1.1.2.3 2001/10/29 20:54:09 davygrvy Exp $

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

### The command procs.
###
### I'm not sure _exactly_ why, but I think they should be in the Stubs table as
### they are functions.

declare 2 generic {
    int Exp_CloseObjCmd (ClientData clientData, Tcl_Interp *interp,
	int argc, char *argv[])
}
declare 3 generic {
    int Exp_ExpInternalCmd (ClientData clientData, Tcl_Interp *interp,
	int argc, char *argv[])
}
declare 4 generic {
    int Exp_DisconnectCmd (ClientData clientData, Tcl_Interp *interp,
	int argc, char *argv[])
}
declare 5 generic {
    int Exp_ExitCmd (ClientData clientData, Tcl_Interp *interp,
	int argc, char *argv[])
}
declare 6 generic {
    int Exp_ExpContinueCmd (ClientData clientData, Tcl_Interp *interp,
	int argc, char *argv[])
}
declare 7 generic {
    int Exp_ForkCmd (ClientData clientData, Tcl_Interp *interp,
	int argc, char *argv[])
}
declare 8 generic {
    int Exp_ExpPidCmd (ClientData clientData, Tcl_Interp *interp,
	int argc, char *argv[])
}
declare 9 generic {
    int Exp_GetpidDeprecatedCmd (ClientData clientData, Tcl_Interp *interp,
	int argc, char *argv[])
}
declare 10 generic {
    int Exp_InterpreterObjCmd (ClientData clientData,
	Tcl_Interp *interp, int objc, struct Tcl_Obj * CONST objv[])
}
declare 11 generic {
    int Exp_LogFileCmd (ClientData clientData, Tcl_Interp *interp,
	int argc, char *argv[])
}
declare 12 generic {
    int Exp_LogUserCmd (ClientData clientData, Tcl_Interp *interp,
	int argc, char *argv[])
}
declare 13 generic {
    int Exp_OpenCmd (ClientData clientData, Tcl_Interp *interp,
	int argc, char *argv[])
}
declare 14 generic {
    int Exp_OverlayCmd (ClientData clientData, Tcl_Interp *interp,
	int argc, char *argv[])
}
declare 15 generic {
    int Exp_InterReturnObjCmd (ClientData clientData,
	Tcl_Interp *interp, int objc, struct Tcl_Obj * CONST objv[])
}
declare 16 generic {
    int Exp_SendObjCmd (ClientData clientData,
	Tcl_Interp *interp, int objc, struct Tcl_Obj * CONST objv[])
}
declare 17 generic {
    int Exp_SendLogCmd (ClientData clientData, Tcl_Interp *interp,
	int argc, char *argv[])
}
declare 18 generic {
    int Exp_SleepCmd (ClientData clientData, Tcl_Interp *interp,
	int argc, char *argv[])
}
declare 19 generic {
    int Exp_SpawnCmd (ClientData clientData, Tcl_Interp *interp,
	int argc, char *argv[])
}
declare 20 generic {
    int Exp_StraceCmd (ClientData clientData, Tcl_Interp *interp,
	int argc, char *argv[])
}
declare 21 generic {
    int Exp_WaitCmd (ClientData clientData, Tcl_Interp *interp,
	int argc, char *argv[])
}
declare 22 generic {
    int Exp_ExpVersionCmd (ClientData clientData, Tcl_Interp *interp,
	int argc, char *argv[])
}
declare 23 generic {
    int Exp_Prompt1Cmd (ClientData clientData, Tcl_Interp *interp,
	int argc, char *argv[])
}
declare 24 generic {
    int Exp_Prompt2Cmd (ClientData clientData, Tcl_Interp *interp,
	int argc, char *argv[])
}
declare 25 generic {
    int Exp_TrapCmd (ClientData clientData, Tcl_Interp *interp,
	int argc, char *argv[])
}


### From exp_printify.c
declare 26 generic {
    char *exp_printify (char *s)
}


## all below are NOT final ->
declare 32 generic {
	void exp_parse_argv (Tcl_Interp *interp, int argc, char **argv)
}
declare 33 generic {
	int exp_interpreter (Tcl_Interp *interp, Tcl_Obj *eofObj)
}
declare 34 generic {
	int exp_interpret_cmdfile (Tcl_Interp *interp, Tcl_Channel file)
}
declare 35 generic {
	int exp_interpret_cmdfilename (Tcl_Interp *interp, char *filename)
}
declare 36 generic {
	void exp_interpret_rcfiles (Tcl_Interp *interp, int my_rc, int sys_rc)
}
declare 37 generic {
	char *exp_cook (char *s, int *len)
}
declare 38 generic {
	void expCloseOnExec (int fd)
}
declare 39 generic {
	int exp_getpidproc (void)
}
declare 40 generic {
	Tcl_Channel ExpCreateSpawnChannel (Tcl_Interp *interp, Tcl_Channel chan)
}

interface expPlat

interface expInt

interface expIntPlat

#====================================================================================
# UNIX specific publics.


#====================================================================================
# WIN32 specific privates.
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
declare 6 win {
	void ExpWinInit (void)
}


#====================================================================================
# MAC specific publics.

### You nutts!!  can't do Mac... sorry..


