# exp.decls --
#
#	This file contains the declarations for all supported public
#	functions that are exported by the Expect library via the stubs table.
#	This file is used to generate the expDecls.h, expPlatDecls.h,
#	expIntDecls.h, and expStub.c files.
#
# RCS: @(#) $Id: exp.decls,v 1.1.4.1 2002/02/10 02:58:03 davygrvy Exp $

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
#declare 1 generic {
#    int Expect_SafeInit (Tcl_Interp *interp)
#}

### The command procs.
###
### I'm not sure _exactly_ why, but I think they should be in the Stubs table as
### they are functions.

#declare 2 generic {
#    int Exp_CloseObjCmd (ClientData clientData, Tcl_Interp *interp,
#	int argc, char *argv[])
#}
declare 3 generic {
    int Exp_ExpInternalCmd (ClientData clientData, Tcl_Interp *interp,
	int argc, char *argv[])
}
#declare 4 generic {
#    int Exp_DisconnectCmd (ClientData clientData, Tcl_Interp *interp,
#	int argc, char *argv[])
#}
declare 5 generic {
    int Exp_ExitCmd (ClientData clientData, Tcl_Interp *interp,
	int argc, char *argv[])
}
declare 6 generic {
    int Exp_ExpContinueCmd (ClientData clientData, Tcl_Interp *interp,
	int argc, char *argv[])
}
#declare 7 generic {
#    int Exp_ForkCmd (ClientData clientData, Tcl_Interp *interp,
#	int argc, char *argv[])
#}
declare 8 generic {
    int Exp_ExpPidCmd (ClientData clientData, Tcl_Interp *interp,
	int argc, char *argv[])
}
declare 9 generic {
    int Exp_GetpidDeprecatedCmd (ClientData clientData, Tcl_Interp *interp,
	int argc, char *argv[])
}
#declare 10 generic {
#    int Exp_InterpreterObjCmd (ClientData clientData,
#	Tcl_Interp *interp, int objc, struct Tcl_Obj * CONST objv[])
#}
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
#declare 14 generic {
#    int Exp_OverlayCmd (ClientData clientData, Tcl_Interp *interp,
#	int argc, char *argv[])
#}
#declare 15 generic {
#    int Exp_InterReturnObjCmd (ClientData clientData,
#	Tcl_Interp *interp, int objc, struct Tcl_Obj * CONST objv[])
#}
#declare 16 generic {
#    int Exp_SendObjCmd (ClientData clientData,
#	Tcl_Interp *interp, int objc, struct Tcl_Obj * CONST objv[])
#}
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
declare 26 generic {
    int Exp_SttyCmd (ClientData clientData, Tcl_Interp *interp,
	int argc, char *argv[])
}
declare 27 generic {
    int Exp_SystemCmd (ClientData clientData, Tcl_Interp *interp,
	int argc, char *argv[])
}
declare 28 generic {
    int Exp_ExpectCmd (ClientData clientData, Tcl_Interp *interp,
	int objc, Tcl_Obj *CONST objv[])
}
declare 29 generic {
    int Exp_ExpectGlobalCmd (ClientData clientData, Tcl_Interp *interp,
	int argc, Tcl_Obj *CONST objv[])
}
declare 30 generic {
    int Exp_MatchMaxCmd (ClientData clientData, Tcl_Interp *interp,
	int argc, char *argv[])
}
declare 31 generic {
    int Exp_RemoveNullsCmd (ClientData clientData, Tcl_Interp *interp,
	int argc, char *argv[])
}
declare 32 generic {
    int Exp_ParityCmd (ClientData clientData, Tcl_Interp *interp,
	int argc, char *argv[])
}
declare 33 generic {
    int Exp_TimestampCmd  (ClientData clientData, Tcl_Interp *interp,
	int argc, char *argv[])
}
declare 34 generic {
    int Exp_CloseCmd (ClientData clientData, Tcl_Interp *interp,
	int argc, char *argv[])
}
declare 35 generic {
    int Exp_InterpreterCmd (ClientData clientData, Tcl_Interp *interp,
	int argc, char *argv[])
}
declare 36 generic {
    int Exp_SendCmd (ClientData clientData, Tcl_Interp *interp,
	int argc, char *argv[])
}
declare 37 generic {
    int Exp_KillCmd (ClientData clientData,Tcl_Interp *interp,
	int argc, char *argv[])
}

### From the old exp_printify.h
declare 40 generic {
    char *exp_printify (char *s)
}

### From the old exp_log.h
declare 50 generic {
    void exp_errorlog (char *fmt, ...)
}
declare 51 generic {
    void exp_log (int force_stdout, ...)
}
declare 52 generic {
    void exp_debuglog (char *fmt, ...)
}
declare 53 generic {
    void exp_nflog (char *buf, int force_stdout)
}
declare 54 generic {
    void exp_nferrorlog (char *buf, int force_stdout)
}
declare 55 generic {
    void exp_error (Tcl_Interp *interp, ...)
}


## all below are NOT final ->
declare 60 generic {
    void exp_parse_argv (Tcl_Interp *interp, int argc, char **argv)
}
declare 61 generic {
    int exp_interpreter (Tcl_Interp *interp)
}
declare 62 generic {
    int exp_interpret_cmdfile (Tcl_Interp *interp, Tcl_Channel cmdfile)
}
declare 63 generic {
    int exp_interpret_cmdfilename (Tcl_Interp *interp, char *filename)
}
declare 64 generic {
    void exp_interpret_rcfiles (Tcl_Interp *interp, int my_rc, int sys_rc)
}
declare 65 generic {
    char *exp_cook (CONST char *s, int *len)
}
#declare 66 generic {
#    void expCloseOnExec (int fd)
#}
declare 67 generic {
    int exp_getpidproc (void)
}
declare 68 generic {
    Tcl_Channel ExpCreateSpawnChannel (Tcl_Interp *interp, Tcl_Channel chan)
}
declare 69 generic {
    int ExpPlatformSpawnOutput (ClientData instanceData, CONST char *bufPtr,
	int toWrite, int *errorPtr)
}
declare 70 generic {
    void exp_create_commands (Tcl_Interp *interp, struct exp_cmd_data *c)
}
declare 71 generic {
    void exp_init_main_cmds (Tcl_Interp *interp)
}
declare 72 generic {
    void exp_init_expect_cmds (Tcl_Interp *interp)
}
declare 73 generic {
    void exp_init_most_cmds (Tcl_Interp *interp)
}
declare 74 generic {
    void exp_init_trap_cmds (Tcl_Interp *interp)
}
declare 75 generic {
    void exp_init_interact_cmds (Tcl_Interp *interp)
}
declare 76 generic {
    int exp_init_tty_cmds (Tcl_Interp *interp)
}
#declare 77 generic {
#    int exp_getpidproc (void)
#}
#declare 78 generic {
#    void exp_busy (int fd)
#}
declare 79 generic {
    int exp_exact_write (struct exp_f * f, char *buffer, int rembytes)
}
#declare 80 generic {
#    void exp_sys_close (int fd, struct exp_f *f)
#}
declare 81 generic {
    struct exp_f * exp_f_find (Tcl_Interp *interp, char *spawnId)
}
declare 82 generic {
    struct exp_f * exp_f_new (Tcl_Interp *interp, Tcl_Channel chan,
	char *spawnId, int pid)
}
declare 83 generic {
    int exp_f_new_platform (struct exp_f *f)
}
declare 84 generic {
    void exp_f_free (struct exp_f *f)
}
declare 85 generic {
    void exp_f_free_platform (struct exp_f *f)
}
declare 86 generic {
    Tcl_Channel ExpCreatePairChannel (Tcl_Interp *interp, CONST char *chanInId,
	CONST char *chanOutId, CONST char *chanName)
}
declare 87 generic {
    int ExpSpawnOpen (Tcl_Interp *interp, char *chanId, int leaveopen)
}
declare 88 generic {
    struct exp_f * exp_update_master (Tcl_Interp *interp ,int opened,
	int adjust)
}
declare 89 generic {
    CONST char * exp_get_var (Tcl_Interp *interp, char *var)
}
declare 90 generic {
    void exp_exit (Tcl_Interp *interp, int status)
}

### From exp_event.h
declare 91 generic {
    int exp_get_next_event (Tcl_Interp *interp, struct exp_f **masters, int n,
	struct exp_f **master_out, int timeout, int key)
}
declare 92 generic {
    int exp_get_next_event_info (Tcl_Interp *interp, struct exp_f *fd, int ready_mask)
}
declare 93 generic {
    int exp_dsleep (Tcl_Interp *interp, double sec)
}
declare 94 generic {
    void exp_init_event (void)
}
#declare 95 generic {
#    void exp_event_exit (Tcl_Interp *interp)
#}
declare 96 generic {
    void exp_event_disarm (struct exp_f *f)
}
declare 97 generic {
    void exp_arm_background_filehandler (struct exp_f *f)
}
declare 98 generic {
    void exp_disarm_background_filehandler (struct exp_f *f)
}
declare 99 generic {
    void exp_disarm_background_filehandler_force (struct exp_f *f)
}
declare 100 generic {
    void exp_unblock_background_filehandler (struct exp_f *f)
}
declare 101 generic {
    void exp_block_background_filehandler (struct exp_f *f)
}
declare 102 generic {
    void exp_background_filehandler (ClientData clientData, int mask)
}
declare 103 generic {
    void exp_exit_handlers (ClientData clientData)
}
declare 104 generic {
    void exp_close_on_exec (int fd)
}
declare 105 generic {
    int exp_flageq_code (char *flag, char *string, int minlen)
}
declare 106 generic {
    void exp_tty_break (Tcl_Interp *interp, struct exp_f *f)
}
declare 107 generic {
    void exp_close_tcl_files (void)
}
declare 108 generic {
    void exp_lowmemcpy (char *dest, CONST char *src, int n)
}
declare 109 generic {
    void exp_timestamp (Tcl_Interp *interp, time_t *timeval, char *array)
}

interface expPlat

interface expInt

declare 1 generic {
    int Exp_StringMatch (CONST char *string, CONST char *pattern, int *offset)
}
declare 2 generic {
    int Exp_StringMatch2 (CONST char *string, CONST char *pattern)
}
#declare 3 generic {
#    void exp_console_set (void)
#}
declare 4 generic {
    struct exp_i *exp_new_i_complex (Tcl_Interp *interp, char *arg,
	int duration, Tcl_VarTraceProc *updateproc, char *msg)
}
declare 5 generic {
    struct exp_i *exp_new_i_simple (struct exp_f *fd, int duration)
}
declare 6 generic {
    struct exp_fs_list *exp_new_fs (struct exp_f *f)
}
declare 7 generic {
    void exp_free_i (Tcl_Interp *interp ,struct exp_i *i,
	Tcl_VarTraceProc *updateproc)
}
declare 8 generic {
    void exp_free_fs (struct exp_fs_list *fs_first)
}
declare 9 generic {
    void exp_free_fs_single (struct exp_fs_list *fs)
}
declare 10 generic {
    void exp_i_update (Tcl_Interp *interp, struct exp_i *i)
}
declare 11 generic {
    void exp_pty_exit (void)
}
declare 12 generic {
    void exp_init_spawn_ids (Tcl_Interp *interp)
}
declare 13 generic {
    void exp_init_pty (Tcl_Interp *interp)
}
declare 14 generic {
    void exp_init_tty (Tcl_Interp *interp)
}
declare 15 generic {
    void exp_init_stdio (void)
}
declare 16 generic {
    void exp_init_sig (void)
}
declare 17 generic {
    void exp_init_trap (void)
}
declare 18 generic {
    void exp_init_unit_random (void)
}
declare 19 generic {
    void exp_init_spawn_id_vars (Tcl_Interp *interp)
}
declare 20 generic {
    void exp_adjust (struct exp_f *f)
}
declare 21 generic {
    void exp_ecmd_remove_f_direct_and_indirect (Tcl_Interp *interp,
	struct exp_f *f)
}
declare 22 generic {
    void exp_rearm_sigchld (Tcl_Interp *interp)
}
declare 23 generic {
    struct exp_f * exp_chan2f (Tcl_Interp *interp, CONST char *chan,
	int opened, int adjust, CONST char *msg)
}
declare 24 generic {
    int exp_fcheck (Tcl_Interp *interp, struct exp_f *f, int opened,
	int adjust, CONST char *msg)
}
declare 25 generic {
    int exp_close (Tcl_Interp *interp, struct exp_f *f)
}
declare 26 generic {
    void exp_strftime (char *format, const struct tm *timeptr,
	Tcl_DString *dstring)
}

interface expIntPlat

#====================================================================================
# UNIX specific publics.


#====================================================================================
# WIN32 specific privates.
declare 0 win {
    DWORD ExpWinApplicationType(const char *originalName,
	Tcl_DString *fullPath)
}
declare 1 win {
    DWORD ExpWinCreateProcess (int argc, char **argv, HANDLE inputHandle,
	HANDLE outputHandle, HANDLE errorHandle, int allocConsole,
	int hideConsole, int debug, int newProcessGroup, Tcl_Pid *pidPtr,
	PDWORD globalPidPtr)
}
declare 2 win {
    void ExpSyslog (char *fmt, ...)
}
#declare 3 win {
#    char *ExpSyslogGetSysMsg (DWORD errId)
#}
declare 4 win {
    Tcl_Pid Exp_WaitPid (Tcl_Pid pid, int *statPtr, int options)
}
declare 5 win {
    void Exp_KillProcess (Tcl_Pid pid)
}
declare 6 win {
    void ExpWinInit (void)
}
declare 7 win {
    void BuildCommandLine (CONST char *executable, int argc, char **argv,
	Tcl_DString *linePtr)
}


#====================================================================================
# MAC specific publics.

### We aren't doing Mac... sorry..


