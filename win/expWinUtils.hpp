//  This was pulled from ConsoleDebugger to enable a test suite
//  to use this in isolation.
//

#ifndef INC_expWinUtils_hpp__
#define INC_expWinUtils_hpp__

#include <windows.h>

// Errors and logging
#define EXP_LOG0(errCode)		ExpWinSyslog(errCode, __FILE__, (int)__LINE__, 0)
#define EXP_LOG1(errCode, arg1)		ExpWinSyslog(errCode, __FILE__, (int)__LINE__, arg1, 0)
#define EXP_LOG2(errCode, arg1, arg2)	ExpWinSyslog(errCode, __FILE__, (int)__LINE__, arg1, arg2, 0)

void ExpWinSyslog (DWORD errCode, ...);
char *ExpSyslogGetSysMsg (DWORD id);

class ArgMaker
{
public:
    char *BuildCommandLine (int, char * const *);
};

void SetArgv (char *cmdLine, int *argcPtr, char ***argvPtr);

#endif