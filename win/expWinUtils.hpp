//  This was pulled from ConsoleDebugger to enable a test suite
//  to use this in isolation.
//

#ifndef INC_expWinUtils_hpp__
#define INC_expWinUtils_hpp__

class ArgMaker
{
public:
    char *BuildCommandLine (int, char * const *);
};

void SetArgv (char *cmdLine, int *argcPtr, char ***argvPtr);

#endif