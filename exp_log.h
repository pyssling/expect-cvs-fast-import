/* exp_log.h */

#include "exp_printify.h"

extern void		expErrorLog _ANSI_ARGS_(TCL_VARARGS(char *,fmt));
extern void		expErrorLogU _ANSI_ARGS_((char *buf, int force_stdout));

extern void		expStdoutLog _ANSI_ARGS_(TCL_VARARGS(int,force_stdout));
extern void		expStdoutLogU _ANSI_ARGS_((char *buf, int force_stdout));

EXTERN void		expDiagInit _ANSI_ARGS_((void));
EXTERN int		expDiagChannelOpen _ANSI_ARGS_((Tcl_Interp *,char *));
EXTERN Tcl_Channel	expDiagChannelGet _ANSI_ARGS_((void));
EXTERN void		expDiagChannelClose _ANSI_ARGS_((void));
EXTERN char *		expDiagFilename _ANSI_ARGS_((void));
EXTERN int		expDiagToStderrGet _ANSI_ARGS_((void));
EXTERN void		expDiagToStderrSet _ANSI_ARGS_((int));
EXTERN void		expDiagWriteBytes _ANSI_ARGS_((char *));
EXTERN void		expDiagWriteChars _ANSI_ARGS_((char *));
EXTERN void		expDiagWriteObj _ANSI_ARGS_((Tcl_Object *));
EXTERN void		expDiagLog _ANSI_ARGS_(TCL_VARARGS(char *,fmt));
EXTERN void		expDiagLogU _ANSI_ARGS_((char *));

EXTERN void		expPrintify _ANSI_ARGS_((char *));
EXTERN void		expPrintifyObj _ANSI_ARGS_((Tcl_Obj *));

EXTERN void		expLogInit _ANSI_ARGS_((void));
EXTERN int		expLogChannelOpen _ANSI_ARGS_((Tcl_Interp *,char *));
EXTERN Tcl_Channel 	expLogChannelGet _ANSI_ARGS_((void));
EXTERN void		expLogChannelSet _ANSI_ARGS_((Tcl_Channel));
EXTERN void		expLogChannelClose _ANSI_ARGS_((void));
EXTERN char *		expLogFilenameGet _ANSI_ARGS_((void));
EXTERN void		expLogAppendSet _ANSI_ARGS_((int));
EXTERN int		expLogAppendGet _ANSI_ARGS_((void));
EXTERN void		expLogLeaveOpenSet _ANSI_ARGS_((int));
EXTERN int		expLogLeaveOpenGet _ANSI_ARGS_((void));
EXTERN void		expLogAllSet _ANSI_ARGS_((int));
EXTERN int		expLogAllGet _ANSI_ARGS_((void));
EXTERN void		expLogToStdoutSet _ANSI_ARGS_((int));
EXTERN int		expLogToStdoutGet _ANSI_ARGS_((void));
EXTERN void		expLogDiagU _ANSI_ARGS_((char *));
EXTERN void		expWriteBytesAndLogIfTtyU _ANSI_ARGS_((ExpState *,char *,int));

EXTERN int		expLogUserGet _ANSI_ARGS_((void));
EXTERN void		expLogUserSet _ANSI_ARGS_((int));

EXTERN void		expLogInteractionU _ANSI_ARGS_((ExpState *,char *));
