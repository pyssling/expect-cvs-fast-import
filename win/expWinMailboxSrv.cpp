#include "expInt.h"
#include "expPort.h"
#include "d:\tomasoft_ws\mcl\include\cmcl.h"

#pragma comment (lib, "d:/tomasoft_ws/mcl/lib/mcl.lib")

Tcl_CmdProc expMakeSpawnChannel;
static char *startNewMailbox();

static int mailboxNameId = 0;

__declspec(dllexport) int
Expect_Init(Tcl_Interp *interp)
{
    if (Tcl_InitStubs(interp, "8.1", 0) == NULL) {
	return TCL_ERROR;
    }
    Tcl_CreateCommand(interp, "exp::test", expMakeSpawnChannel, 0L, 0L);
    return TCL_OK;
}


//Tcl_Channel
int
expMakeSpawnChannel(ClientData cd, Tcl_Interp *interp, int argc, char *argv[])
{
    char *val;
    int debug = 0;
    DWORD dwRet;
    TCHAR slavePath[MAX_PATH];
    TCHAR imagePath[MAX_PATH];
    char *mailboxName;
    Tcl_Pid slaveDrvPid;	/* Process id of the slave */
    DWORD globalPid;
    char **nargv = NULL;
    int i,j;

//    val = exp_get_var(interp, "exp_win_debug");
//    if (val) {
//	if (Tcl_GetBoolean(NULL, val, &debug) != TCL_OK) {
//	    // set it anyways.
//	    debug = 1;
//	}
//    } else {
//	debug = 0;
//    }

    /*
     * Get the location of the slavedrv.exe from the $exp::library
     * variable set by Expect_Init().
     */
    val = Tcl_GetVar(interp, "::exp::library", TCL_GLOBAL_ONLY);
    TclWinNoBackslash(val);

    dwRet = SearchPath(val, "slavedrv.exe", NULL, MAX_PATH, slavePath, NULL);
    if (dwRet == 0) {
	Tcl_AppendResult(interp, "unable to find helper program slavedrv.exe",
			 (char *) NULL);
	return 0L;
    }

    /*
     * See that the process we want to intercept is interceptable.
     */
    dwRet = ExpWinApplicationType(argv[0], imagePath);
    switch (dwRet) {
	case EXP_APPL_NONE:
	    TclWinConvertError(ERROR_FILE_NOT_FOUND);
	    //exp_error(interp, "couldn't execute \"%s\": %s",
	//	  argv[0], Tcl_PosixError(interp));
	    return 0L;

	case EXP_APPL_WIN3X:
	case EXP_APPL_WIN32GUI:
	    /*
	     *  Return an errorCode that is close to the truth.
	     */
	    TclWinConvertError(ERROR_PIPE_NOT_CONNECTED);
	    //exp_error(interp, "\"%s\" does not support the CUI subsystem and cannot be intercepted: %s",
	//	  argv[0], Tcl_PosixError(interp));
	    return 0L;
    }

    /*
     * Start a new mailbox IPC transport server for this channel.
     */
    mailboxName = startNewMailbox();

    /*
     * Adjust the arguements.
     */
    nargv = (char **) ckalloc(sizeof(char *) * (argc+2));
    nargv[0] = slavePath;
    nargv[1] = mailboxName;
    nargv[2] = debug    ? "1" : "0";
    j = 3;
    nargv[j++] = imagePath;
    for (i = 0; i < argc; i++, j++) {
	nargv[j] = argv[i];
    }
    argc = j;

    /*
     *  Create the new process.
     */
    dwRet = ExpWinCreateProcess(argc, nargv, NULL, NULL, NULL,
			     TRUE, !(debug), FALSE, FALSE,
			     &slaveDrvPid, &globalPid);
    if (dwRet != 0) {
	TclWinConvertError(dwRet);
	//exp_error(interp, "couldn't execute \"%s\": %s",
	//	  argv[0],Tcl_PosixError(interp));
	//goto end;
	return 0L;
    }

    /*
     * Block waiting for the slavedrv to connect to the named mailbox.
     */

    /*
     * Register the new channel with Tcl
     */

    return 0L;
}


char *startNewMailbox()
{
    char *mailbox;
    char mailboxToExp[24];
    char mailboxFromExp[24];

    mailbox = ckalloc(24);

    sprintf(mailbox, "%s%08x%08x", "exp", GetCurrentProcessId(), mailboxNameId++);
    sprintf(mailboxToExp, "%sTo", mailbox);
    sprintf(mailboxFromExp, "%sFrom", mailbox);

    CMclMailbox *mailTo = new CMclMailbox(20, 128, mailboxToExp);
    CMclMailbox *mailFrom = new CMclMailbox(20, 128, mailboxFromExp);

    return mailbox;
}

