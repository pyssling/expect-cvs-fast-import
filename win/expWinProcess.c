/* 
 * expWinProcess.c --
 *
 *	This file contains utility procedures.  It primarily handled
 *	processes for Expect.
 *
 * Copyright (c) 1987-1993 The Regents of the University of California.
 * Copyright (c) 1994-1997 Sun Microsystems, Inc.
 * Copyright (c) 1997 Mitel Corporation
 *
 * See the file "license.terms" for information on usage and redistribution
 * of this file, and for a DISCLAIMER OF ALL WARRANTIES.
 *
 */

#include "tclPort.h"
#include "expWin.h"

/*
 * This list is used to map from pids to process handles.
 */

typedef struct ProcInfo {
    HANDLE hProcess;
    DWORD dwProcessId;
    struct ProcInfo *nextPtr;
} ProcInfo;

static ProcInfo *procList = NULL;


typedef struct {
    int useWide;
    HANDLE (WINAPI *createFileProc)(CONST TCHAR *, DWORD, DWORD, 
	    LPSECURITY_ATTRIBUTES, DWORD, DWORD, HANDLE);
    BOOL (WINAPI *createProcessProc)(CONST TCHAR *, TCHAR *, 
	    LPSECURITY_ATTRIBUTES, LPSECURITY_ATTRIBUTES, BOOL, DWORD, 
	    LPVOID, CONST TCHAR *, LPSTARTUPINFOA, LPPROCESS_INFORMATION);
    DWORD (WINAPI *getFileAttributesProc)(CONST TCHAR *);
    DWORD (WINAPI *getFullPathNameProc)(CONST TCHAR *, DWORD nBufferLength, 
	    WCHAR *, TCHAR **);
    DWORD (WINAPI *getShortPathNameProc)(CONST TCHAR *, WCHAR *, DWORD); 
} ExpWinProcs;

static TclWinProcs asciiProcs = {
    0,
    (HANDLE (WINAPI *)(CONST TCHAR *, DWORD, DWORD, SECURITY_ATTRIBUTES *, 
	    DWORD, DWORD, HANDLE)) CreateFileA,
    (BOOL (WINAPI *)(CONST TCHAR *, TCHAR *, LPSECURITY_ATTRIBUTES, 
	    LPSECURITY_ATTRIBUTES, BOOL, DWORD, LPVOID, CONST TCHAR *, 
	    LPSTARTUPINFOA, LPPROCESS_INFORMATION)) CreateProcessA,
    (DWORD (WINAPI *)(CONST TCHAR *)) GetFileAttributesA,
    (DWORD (WINAPI *)(CONST TCHAR *, DWORD nBufferLength, WCHAR *, 
	    TCHAR **)) GetFullPathNameA,
    (DWORD (WINAPI *)(CONST TCHAR *, WCHAR *, DWORD)) GetShortPathNameA,
};

static TclWinProcs unicodeProcs = {
    1,
    (HANDLE (WINAPI *)(CONST TCHAR *, DWORD, DWORD, SECURITY_ATTRIBUTES *, 
	    DWORD, DWORD, HANDLE)) CreateFileW,
    (BOOL (WINAPI *)(CONST TCHAR *, TCHAR *, LPSECURITY_ATTRIBUTES, 
	    LPSECURITY_ATTRIBUTES, BOOL, DWORD, LPVOID, CONST TCHAR *, 
	    LPSTARTUPINFOA, LPPROCESS_INFORMATION)) CreateProcessW,
    (DWORD (WINAPI *)(CONST TCHAR *)) GetFileAttributesW,
    (DWORD (WINAPI *)(CONST TCHAR *, DWORD nBufferLength, WCHAR *, 
	    TCHAR **)) GetFullPathNameW,
    (DWORD (WINAPI *)(CONST TCHAR *, WCHAR *, DWORD)) GetShortPathNameW,
};

#define tclWinProcs expWinProcs
static ExpWinProcs *expWinProcs;

void
ExpInitWinProcessAPI (void)
{
    OSVERSIONINFO os;
    os.dwOSVersionInfoSize = sizeof(OSVERSIONINFO);
    expWinProcs = &asciiProcs;
    
    if (GetVersionEx(&os) != 0) {    
	switch (os.dwPlatformId) {
	    case VER_PLATFORM_WIN32_WINDOWS:
		expWinProcs = &asciiProcs; break;
	    case VER_PLATFORM_WIN32_NT:
		expWinProcs = &unicodeProcs; break;
	}
    }
}

/*
 *----------------------------------------------------------------------
 *
 * HasConsole --
 *
 *	Determines whether the current application is attached to a
 *	console.
 *
 * Results:
 *	Returns TRUE if this application has a console, else FALSE.
 *
 * Side effects:
 *	None.
 *
 *----------------------------------------------------------------------
 */

static BOOL
HasConsole()
{
    HANDLE handle = CreateFile("CONOUT$", GENERIC_WRITE, FILE_SHARE_WRITE,
	    NULL, OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL, NULL);

    if (handle != INVALID_HANDLE_VALUE) {
        CloseHandle(handle);
	return TRUE;
    } else {
        return FALSE;
    }
}

/*
 *--------------------------------------------------------------------
 *
 * ExpApplicationType --
 *
 *	Search for the specified program and identify if it refers to a DOS,
 *	Windows 3.X, or Win32 program.  Used to determine how to invoke 
 *	a program, or if it can even be invoked.
 *
 *	It is possible to almost positively identify DOS and Windows 
 *	applications that contain the appropriate magic numbers.  However, 
 *	DOS .com files do not seem to contain a magic number; if the program 
 *	name ends with .com and could not be identified as a Windows .com
 *	file, it will be assumed to be a DOS application, even if it was
 *	just random data.  If the program name does not end with .com, no 
 *	such assumption is made.
 *
 *	The Win32 procedure GetBinaryType incorrectly identifies any 
 *	junk file that ends with .exe as a dos executable and some 
 *	executables that don't end with .exe as not executable.  Plus it 
 *	doesn't exist under win95, so I won't feel bad about reimplementing
 *	functionality.
 *
 * Results:
 *	The return value is one of EXP_APPL_DOS, EXP_APPL_WIN3X, or EXP_APPL_WIN32
 *	if the filename referred to the corresponding application type.
 *	If the file name could not be found or did not refer to any known 
 *	application type, EXP_APPL_NONE is returned and the caller can use
 *	GetLastError() to find out what went wrong.
 *
 * Side effects:
 *	None.
 *
 *----------------------------------------------------------------------
 */

DWORD
ExpApplicationType(
    const char *originalName,	/* Name of the application to find. */
    char fullName[])		/* Filled with complete path to 
				 * application. */
{
    int applType, i, nameLen, found;
    HANDLE hFile;
    TCHAR *rest;
    char *ext;
    DWORD attr, read;
    IMAGE_DOS_HEADER p236;  /* p236, DOS (old-style) executable-file header */
    union {
	BYTE buf[200];
	IMAGE_NT_HEADERS pe;
	IMAGE_OS2_HEADER ne;
	IMAGE_VXD_HEADER le;
    } header;
    Tcl_DString nameBuf, ds;
    TCHAR *nativeName;
    WCHAR nativeFullPath[MAX_PATH];
    static char extensions[][5] = {"", ".com", ".exe", ".bat", ".cmd"};

    /* Look for the program as an external program.  First try the name
     * as it is, then try adding .com, .exe, and .bat, in that order, to
     * the name, looking for an executable.
     *
     * Using the raw SearchPath() procedure doesn't do quite what is 
     * necessary.  If the name of the executable already contains a '.' 
     * character, it will not try appending the specified extension when
     * searching (in other words, SearchPath will not find the program 
     * "a.b.exe" if the arguments specified "a.b" and ".exe").   
     * So, first look for the file as it is named.  Then manually append 
     * the extensions, looking for a match.  
     */

    applType = EXP_APPL_NONE;
    Tcl_DStringInit(&nameBuf);
    Tcl_DStringAppend(&nameBuf, originalName, -1);
    nameLen = Tcl_DStringLength(&nameBuf);

    for (i = 0; i < (int) (sizeof(extensions) / sizeof(extensions[0])); i++) {
	Tcl_DStringSetLength(&nameBuf, nameLen);
	Tcl_DStringAppend(&nameBuf, extensions[i], -1);
        nativeName = Tcl_WinUtfToTChar(Tcl_DStringValue(&nameBuf), 
		Tcl_DStringLength(&nameBuf), &ds);
	found = (*tclWinProcs->searchPathProc)(NULL, nativeName, NULL, 
		MAX_PATH, nativeFullPath, &rest);
	Tcl_DStringFree(&ds);
	if (found == 0) {
	    continue;
	}

	/*
	 * Ignore matches on directories or data files, return if identified
	 * a known type.
	 */

	attr = (*tclWinProcs->getFileAttributesProc)((TCHAR *) nativeFullPath);
	if ((attr == 0xffffffff) || (attr & FILE_ATTRIBUTE_DIRECTORY)) {
	    continue;
	}
	strcpy(fullName, Tcl_WinTCharToUtf((TCHAR *) nativeFullPath, -1, &ds));
	Tcl_DStringFree(&ds);

	ext = strrchr(fullName, '.');
	if ((ext != NULL) && (stricmp(ext, ".bat") == 0 ||
		stricmp(ext, ".cmd") == 0)) {
	    applType = EXP_APPL_DOS;
	    break;
	}
	
	hFile = (*tclWinProcs->createFileProc)((TCHAR *) nativeFullPath, 
		GENERIC_READ, FILE_SHARE_READ, NULL, OPEN_EXISTING, 
		FILE_ATTRIBUTE_NORMAL, NULL);
	if (hFile == INVALID_HANDLE_VALUE) {
	    continue;
	}

	p236.e_magic = 0;
	ReadFile(hFile, &p236, sizeof(IMAGE_DOS_HEADER), &read, NULL);
	if (p236.e_magic != IMAGE_DOS_SIGNATURE) {
	    /* 
	     * Doesn't have the magic number for relocatable executables.  If 
	     * filename ends with .com, assume it's a DOS application anyhow.
	     * Note that we didn't make this assumption at first, because some
	     * supposed .com files are really 32-bit executables with all the
	     * magic numbers and everything.  
	     */

	    /*
	     * Additional notes from Ralf Brown's interupt list:
	     *
	     * The COM files are raw binary executables and are a leftover
	     * from the old CP/M machines with 64K RAM.  A COM program can
	     * only have a size of less than one segment (64K), including code
	     * and static data since no fixups for segment relocation or
	     * anything else is included. One method to check for a COM file
	     * is to check if the first byte in the file could be a valid jump
	     * or call opcode, but this is a very weak test since a COM file
	     * is not required to start with a jump or a call. In principle,
	     * a COM file is just loaded at offset 100h in the segment and
	     * then executed.
	     *
	     * OFFSET              Count TYPE   Description
	     * 0000h                   1 byte   ID=0E9h
	     *                                  ID=0EBh
	     */

	    CloseHandle(hFile);
	    if ((ext != NULL) && (strcmp(ext, ".com") == 0)) {
		applType = EXP_APPL_DOS;
		break;
	    }
	    continue;
	}
	if (p236.e_lfarlc < 0x40 || p236.e_lfanew == 0 /* reserved */) {
	    /* 
	     * Old-style header only.  Can't be more than a DOS executable.
	     */

	    CloseHandle(hFile);
	    applType = EXP_APPL_DOS;
	    break;
	}

	/* 
	 * The LONG at p236.e_lfanew points to the real exe header only
	 * when p236.e_lfarlc is set to 40h (or greater).
	 */
	
	SetFilePointer(hFile, p236.e_lfanew, NULL, FILE_BEGIN);
	ReadFile(hFile, header.buf, 200, &read, NULL);
	CloseHandle(hFile);

	/*
	 * Check the sigs against the following list:
	 *  'PE\0\0'  Win32 (Windows NT and Win32s) portable executable based
	 *	    on Unix COFF.
	 *  'NE'  Windows or OS/2 1.x segmented ("new") executable.
	 *  'LE'  Windows virtual device driver (VxD) linear executable.
	 *  'LX'  variant of LE used in OS/2 2.x
	 *  'W3'  Windows WIN386.EXE file; a collection of LE files
	 *	    (protected mode windows).
	 *  'MZ'  old-style p236 DOS executable.
	 */

	if (header.pe.Signature == IMAGE_NT_SIGNATURE) {
	    /*
	     * Win32, "PE\0\0" which is short for "Portable Executable".
	     */

	    if (header.pe.OptionalHeader.Magic != IMAGE_NT_OPTIONAL_HDR_MAGIC
		    || !(header.pe.FileHeader.Characteristics &
		    IMAGE_FILE_EXECUTABLE_IMAGE)) {
		/*
		 * Not an executable. Might be a dll or a COMDAT library.
		 */

		applType = EXP_APPL_NONE;
		break;
	    }

	    switch (header.pe.OptionalHeader.Subsystem) {
		case IMAGE_SUBSYSTEM_WINDOWS_CUI:
		case IMAGE_SUBSYSTEM_OS2_CUI:
		case IMAGE_SUBSYSTEM_POSIX_CUI:
		    /* 
		     * image runs in the Windows, OS/2, or Posix character
		     * subsystem.
		     */

		    applType = EXP_APPL_WIN32CUI;
		    break;

		default:
		    /*
		     * Non-CUI applications are run detached.  Return a flag
		     * that will indicate this error.
		     */

		    applType = EXP_APPL_WIN32GUI;
	    }
	} else if (header.ne.ne_magic == IMAGE_OS2_SIGNATURE) {
	    switch (header.ne.ne_exetyp) {
		case 0x1:  /* Microsoft/IBM OS/2 (default)  */
		case 0x3:  /* Microsoft (European?) MS-DOS 4.x */
		    /* Only these might be character-mode. */
		    applType = EXP_APPL_DOS;
		    break;

		default:
		    applType = EXP_APPL_NONE;
	    }
	} else if (header.le.e32_magic == IMAGE_OS2_SIGNATURE_LE ||
		header.le.e32_magic == 0x584C /* 'LX' */ ||
		header.le.e32_magic == 0x3357 /* 'W3' */) {
	    /*
	     * Virtual device drivers are not executables, per se.
	     */

	    applType = EXP_APPL_NONE;
	} else {
	    /*
	     * NOTE: The Lahey Fortran90 compiler might make executables
	     * that have a bogus signature and end-up here.
	     */

	    applType = EXP_APPL_DOS;
	}
	break;
    }
    Tcl_DStringFree(&nameBuf);

    if (applType == EXP_APPL_NONE) {
	return EXP_APPL_NONE;
    }

    if ((applType == EXP_APPL_DOS) || (applType == EXP_APPL_WIN3X)) {
	/* 
	 * Replace long path name of executable with short path name for
	 * 16-bit applications.  Otherwise the application may not be able
	 * to correctly parse its own command line to separate off the
	 * application name from the arguments.
	 */

	(*tclWinProcs->getShortPathNameProc)((TCHAR *) nativeFullPath,
		nativeFullPath, MAX_PATH);
	strcpy(fullName, Tcl_WinTCharToUtf((TCHAR *) nativeFullPath, -1, &ds));
	Tcl_DStringFree(&ds);
    }
    return applType;
}

/*    
 *----------------------------------------------------------------------
 *
 * BuildCommandLine --
 *
 *	The command line arguments are stored in linePtr separated
 *	by spaces, in a form that CreateProcess() understands.  Special 
 *	characters in individual arguments from argv[] must be quoted 
 *	when being stored in cmdLine.
 *
 * Results:
 *	None.
 *
 * Side effects:
 *	None.
 *
 *----------------------------------------------------------------------
 */
void
BuildCommandLine(
    CONST char *executable,	/* Full path of executable (including 
				 * extension).  Replacement for argv[0]. */
    int argc,			/* Number of arguments. */
    char **argv,		/* Argument strings in UTF. */
    Tcl_DString *linePtr)	/* Initialized Tcl_DString that receives the
				 * command line (TCHAR). */
{
    CONST char *arg, *start, *special;
    int quote, i;
    Tcl_DString ds;

    Tcl_DStringInit(&ds);

    /*
     * Prime the path.
     */
    
    Tcl_DStringAppend(&ds, Tcl_DStringValue(linePtr), -1);
    
    for (i = 0; i < argc; i++) {
	if (i == 0) {
	    arg = executable;
	} else {
	    arg = argv[i];
	    Tcl_DStringAppend(&ds, " ", 1);
	}

	quote = 0;
	if (arg[0] == '\0') {
	    quote = 1;
	} else {
	    for (start = arg; *start != '\0'; start++) {
		if (isspace(*start)) { /* INTL: ISO space. */
		    quote = 1;
		    break;
		}
	    }
	}
	if (quote) {
	    Tcl_DStringAppend(&ds, "\"", 1);
	}

	start = arg;	    
	for (special = arg; ; ) {
	    if ((*special == '\\') && 
		    (special[1] == '\\' || special[1] == '"')) {
		Tcl_DStringAppend(&ds, start, special - start);
		start = special;
		while (1) {
		    special++;
		    if (*special == '"') {
			/* 
			 * N backslashes followed a quote -> insert 
			 * N * 2 + 1 backslashes then a quote.
			 */

			Tcl_DStringAppend(&ds, start, special - start);
			break;
		    }
		    if (*special != '\\') {
			break;
		    }
		}
		Tcl_DStringAppend(&ds, start, special - start);
		start = special;
	    }
	    if (*special == '"') {
		Tcl_DStringAppend(&ds, start, special - start);
		Tcl_DStringAppend(&ds, "\\\"", 2);
		start = special + 1;
	    }
	    if (*special == '\0') {
		break;
	    }
	    special++;
	}
	Tcl_DStringAppend(&ds, start, special - start);
	if (quote) {
	    Tcl_DStringAppend(&ds, "\"", 1);
	}
    }
    Tcl_DStringFree(linePtr);
    Tcl_WinUtfToTChar(Tcl_DStringValue(&ds), Tcl_DStringLength(&ds), linePtr);
    Tcl_DStringFree(&ds);
}

/*
 *----------------------------------------------------------------------
 *
 * Exp_WaitPid --
 *
 *	Emulates the waitpid system call.
 *
 * Results:
 *	Returns 0 if the process is still alive, -1 on an error, or
 *	the pid on a clean close.  
 *
 * Side effects:
 *	Unless WNOHANG is set and the wait times out, the process
 *	information record will be deleted and the process handle
 *	will be closed.
 *
 *----------------------------------------------------------------------
 */

Tcl_Pid
Exp_WaitPid(pid, statPtr, options)
    Tcl_Pid pid;
    int *statPtr;
    int options;
{
    ProcInfo *infoPtr, **prevPtrPtr;
    int flags;
    Tcl_Pid result;
    DWORD ret;

    if (options & WNOHANG) {
	flags = 0;
    } else {
	flags = INFINITE;
    }
    if (pid == 0) {
	*statPtr = 0;
	return 0;
    }

    /*
     * Find the process on the process list.
     */

    prevPtrPtr = &procList;
    for (infoPtr = procList; infoPtr != NULL;
	    prevPtrPtr = &infoPtr->nextPtr, infoPtr = infoPtr->nextPtr) {
	 if (infoPtr->hProcess == (HANDLE) pid) {
	    break;
	}
    }
    if (infoPtr == NULL) {
	return 0;
    }

    ret = WaitForSingleObject(infoPtr->hProcess, flags);
    if (ret == WAIT_TIMEOUT) {
	*statPtr = 0;
	if (options & WNOHANG) {
	    return 0;
	} else {
	    result = 0;
	}
    } else if (ret != WAIT_FAILED) {
	GetExitCodeProcess(infoPtr->hProcess, (DWORD*)statPtr);
	*statPtr = ((*statPtr << 8) & 0xff00);
	result = pid;
    } else {
	errno = ECHILD;
	result = (Tcl_Pid) -1;
    }

    /*
     * Remove the process from the process list and close the process handle.
     */
    CloseHandle(infoPtr->hProcess);
    *prevPtrPtr = infoPtr->nextPtr;
    ckfree((char*)infoPtr);

    return result;
}

/*
 *----------------------------------------------------------------------
 *
 * Exp_KillProcess --
 *
 *	Kills the subprocess
 *
 * Results:
 *	Nothing
 *
 * Side effects:
 *	The subprocess is killed.
 *
 *----------------------------------------------------------------------
 */

void
Exp_KillProcess(pid)
    Tcl_Pid pid;
{
    TerminateProcess((HANDLE) pid, 0xFFFF);
}

/*
 *----------------------------------------------------------------------
 *
 * ExpCreateProcess --
 *
 *	Create a child process that has the specified files as its 
 *	standard input, output, and error.  The child process is set
 *	to run properly under only Windows NT right now, and runs with 
 *	the same environment variables as the creating process.
 *
 *	The complete Windows search path is searched to find the specified 
 *	executable.  If an executable by the given name is not found, 
 *	automatically tries appending ".com", ".exe", ".bat", and ".cmd" to
 *	the executable name.
 *
 * Results:
 *	0 on success, an error value otherwise.
 * 
 * Side effects:
 *	A process is created.
 *
 *----------------------------------------------------------------------
 */

DWORD
ExpCreateProcess(argc, argv, inputHandle, outputHandle, errorHandle,
		 allocConsole, hideConsole, debug, newProcessGroup,
		 pidPtr, globalPidPtr)
    int argc;			/* Number of arguments in following array. */
    char **argv;		/* Array of argument strings.  argv[0]
				 * contains the name of the executable
				 * converted to native format (using the
				 * Tcl_TranslateFileName call).  Additional
				 * arguments have not been converted. */
    HANDLE inputHandle;		/* If non-NULL, gives the file to use as
				 * input for the child process.  If inputHandle
				 * is NULL, the child will receive no standard
				 * input. */
    HANDLE outputHandle;		/* If non-NULL, gives the file that
				 * receives output from the child process.  If
				 * outputHandle is NULL, output from the child
				 * will be discarded. */
    HANDLE errorHandle;		/* If non-NULL, gives the file that
				 * receives errors from the child process.  If
				 * errorFile file is not writeable or is NULL,
				 * errors from the child will be discarded.
				 * errorFile may be the same as outputFile. */
    int allocConsole;		/* Should a console be allocated */
    int hideConsole;		/* Hide or display the created console */
    int debug;			/* Is this process going to be debugged? */
    int newProcessGroup;	/* Create a new process group */
    Tcl_Pid *pidPtr;		/* If this procedure is successful, pidPtr
				 * is filled with the process id of the child
				 * process. */
    PDWORD globalPidPtr;	/* Globally unique pid */
{
    DWORD applType;
    int createFlags;
    Tcl_DString cmdLine;
    STARTUPINFO startInfo;
    PROCESS_INFORMATION procInfo;
    SECURITY_ATTRIBUTES secAtts;
    HANDLE hProcess, h;
    char execPath[MAX_PATH * TCL_UTF_MAX];
    char *originalName;
    LONG result;
    BOOL b;

    result = 0;
    applType = ExpApplicationType(argv[0], execPath);
    if (applType == EXP_APPL_NONE) {
	return GetLastError();
    } else if (applType == EXP_APPL_WIN32GUI) {
	return ERROR_BAD_PIPE;  /* GUI applications can't use pipes. */
    }
    originalName = argv[0];
    argv[0] = execPath;

    Tcl_DStringInit(&cmdLine);
    hProcess = GetCurrentProcess();

    /*
     * STARTF_USESTDHANDLES must be used to pass handles to child process.
     * Using SetStdHandle() and/or dup2() only works when a console mode 
     * parent process is spawning an attached console mode child process.
     */

    ZeroMemory(&startInfo, sizeof(startInfo));
    startInfo.cb = sizeof(startInfo);

    secAtts.nLength = sizeof(SECURITY_ATTRIBUTES);
    secAtts.lpSecurityDescriptor = NULL;
    secAtts.bInheritHandle = TRUE;

    if (inputHandle || outputHandle || errorHandle) {
	startInfo.dwFlags   = STARTF_USESTDHANDLES;
	if (! inputHandle) {
	    inputHandle = GetStdHandle(STD_INPUT_HANDLE);
	}
	if (! outputHandle) {
	    outputHandle = GetStdHandle(STD_OUTPUT_HANDLE);
	}
	if (! errorHandle) {
	    errorHandle = GetStdHandle(STD_ERROR_HANDLE);
	}
    }

    startInfo.hStdInput	= INVALID_HANDLE_VALUE;
    startInfo.hStdOutput= INVALID_HANDLE_VALUE;
    startInfo.hStdError = INVALID_HANDLE_VALUE;

    /*
     * Duplicate all the handles which will be passed off as stdin, stdout
     * and stderr of the child process. The duplicate handles are set to
     * be inheritable, so the child process can use them.
     */

    if (inputHandle == NULL || inputHandle == INVALID_HANDLE_VALUE) {
	/* 
	 * If handle was not set, stdin should return immediate EOF.
	 * Under Windows95, some applications (both 16 and 32 bit!) 
	 * cannot read from the NUL device; they read from console
	 * instead.
	 */

	if (CreatePipe(&startInfo.hStdInput, &h, &secAtts, 0) != FALSE) {
	    CloseHandle(h);
	}
    } else {
	DuplicateHandle(hProcess, inputHandle, hProcess, &startInfo.hStdInput,
			0, TRUE, DUPLICATE_SAME_ACCESS);
	if (startInfo.hStdInput == INVALID_HANDLE_VALUE) {
	    EXP_LOG("couldn't duplicate input handle: 0x%x", GetLastError());
	    result = GetLastError();
	    goto end;
	}
    }

    if (outputHandle == NULL || outputHandle == INVALID_HANDLE_VALUE) {
	/*
	 * If handle was not set, output should be sent to an infinitely 
	 * deep sink.  Under Windows 95, some 16 bit applications cannot
	 * have stdout redirected to NUL; they send their output to
	 * the console instead.  Some applications, like "more" or "dir /p", 
	 * when outputting multiple pages to the console, also then try and
	 * read from the console to go the next page.
	 */

	if ((TclWinGetPlatformId() == VER_PLATFORM_WIN32_WINDOWS) 
		&& (applType == EXP_APPL_DOS)) {
	    if (CreatePipe(&h, &startInfo.hStdOutput, &secAtts, 0) != FALSE) {
		CloseHandle(h);
	    }
	} else {
	    startInfo.hStdOutput = CreateFileA("NUL:", GENERIC_WRITE, 0,
		    &secAtts, OPEN_ALWAYS, FILE_ATTRIBUTE_NORMAL, NULL);
	}
    } else {
	DuplicateHandle(hProcess, outputHandle, hProcess,
			&startInfo.hStdOutput, 0, TRUE, DUPLICATE_SAME_ACCESS);
	if (startInfo.hStdOutput == INVALID_HANDLE_VALUE) {
	    EXP_LOG("couldn't duplicate output handle: 0x%x", GetLastError());
	    result = GetLastError();
	    goto end;
	}
    }

    if (errorHandle == NULL || errorHandle == INVALID_HANDLE_VALUE) {
	/*
	 * If handle was not set, errors should be sent to an infinitely
	 * deep sink.
	 */

	startInfo.hStdError = CreateFileA("NUL:", GENERIC_WRITE, 0,
		&secAtts, OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL, NULL);
    } else {
	DuplicateHandle(hProcess, errorHandle, hProcess,
			&startInfo.hStdError, 0, TRUE, DUPLICATE_SAME_ACCESS);
	if (startInfo.hStdError == INVALID_HANDLE_VALUE) {
	    EXP_LOG("couldn't duplicate error handle: 0x%x", GetLastError());
	    result = GetLastError();
	    goto end;
	}
    }

    /* 
     * If we do not have a console window, then we must run DOS and
     * WIN32 console mode applications as detached processes. This tells
     * the loader that the child application should not inherit the
     * console, and that it should not create a new console window for
     * the child application.  The child application should get its stdio 
     * from the redirection handles provided by this application, and run
     * in the background.
     *
     * If we are starting a GUI process, they don't automatically get a 
     * console, so it doesn't matter if they are started as foreground or
     * detached processes.  The GUI window will still pop up to the
     * foreground.
     */

    if (TclWinGetPlatformId() == VER_PLATFORM_WIN32_NT) {
	if (!allocConsole && HasConsole()) {
	    createFlags = 0;
	} else if (applType == EXP_APPL_DOS || allocConsole) {
	    /*
	     * Under NT, 16-bit DOS applications will not run unless they
	     * can be attached to a console.  If we are running without a
	     * console, run the 16-bit program as an normal process inside
	     * of a hidden console application, and then run that hidden
	     * console as a detached process.
	     */

	    if (hideConsole) {
		startInfo.wShowWindow = SW_HIDE;
	    } else {
		/* For debugging, show the sub process console */
		startInfo.wShowWindow = SW_SHOW;
	    }
	    startInfo.dwFlags |= STARTF_USESHOWWINDOW;
	    createFlags = CREATE_NEW_CONSOLE;
	    Tcl_DStringAppend(&cmdLine, "cmd.exe /c ", -1);
	} else {
	    createFlags = DETACHED_PROCESS;
	} 
    } else {
	if (!allocConsole && HasConsole()) {
	    createFlags = 0;
	} else if (allocConsole) {
	    createFlags = CREATE_NEW_CONSOLE;
	} else {
	    createFlags = DETACHED_PROCESS;
	}
	
	if (applType == EXP_APPL_DOS) {
	    /*
	     * Under Windows 95, 16-bit DOS applications do not work well 
	     * with pipes:
	     *
	     * 1. EOF on a pipe between a detached 16-bit DOS application 
	     * and another application is not seen at the other
	     * end of the pipe, so the listening process blocks forever on 
	     * reads.  This inablity to detect EOF happens when either a 
	     * 16-bit app or the 32-bit app is the listener.  
	     *
	     * 2. If a 16-bit DOS application (detached or not) blocks when 
	     * writing to a pipe, it will never wake up again, and it
	     * eventually brings the whole system down around it.
	     *
	     * The 16-bit application is run as a normal process inside
	     * of a hidden helper console app, and this helper may be run
	     * as a detached process.  If any of the stdio handles is
	     * a pipe, the helper application accumulates information 
	     * into temp files and forwards it to or from the DOS 
	     * application as appropriate.  This means that DOS apps 
	     * must receive EOF from a stdin pipe before they will actually
	     * begin, and must finish generating stdout or stderr before 
	     * the data will be sent to the next stage of the pipe.
	     *
	     * The helper app should be located in the same directory as
	     * the tcl dll.
	     */

	    if (createFlags != 0) {
		if (hideConsole) {
		    startInfo.wShowWindow = SW_HIDE;
		} else {
		    /* For debugging, show the sub process console */
		    startInfo.wShowWindow = SW_SHOW;
		}
		startInfo.dwFlags |= STARTF_USESHOWWINDOW;
		createFlags = CREATE_NEW_CONSOLE;
	    }
	    // BUG: Fixme!  Where is tclpipXX.dll ?????  Set it!
	    Tcl_DStringAppend(&cmdLine, "tclpip" STRINGIFY(TCL_MAJOR_VERSION) 
		    STRINGIFY(TCL_MINOR_VERSION) ".dll ", -1);
	}
    }
    if (debug) {
	createFlags |= DEBUG_PROCESS;
    }
    if (newProcessGroup) {
	createFlags |= CREATE_NEW_PROCESS_GROUP;
    }

    /* 
     * If we do not have a console window, then we must run DOS and
     * WIN32 console mode applications as detached processes. This tells
     * the loader that the child application should not inherit the
     * console, and that it should not create a new console window for
     * the child application.  The child application should get its stdio 
     * from the redirection handles provided by this application, and run
     * in the background.
     *
     * If we are starting a GUI process, they don't automatically get a 
     * console, so it doesn't matter if they are started as foreground or
     * detached processes.  The GUI window will still pop up to the
     * foreground.
     */

    if (!allocConsole && HasConsole()) {
	createFlags = 0;
    } else if (applType == EXP_APPL_DOS || allocConsole) {
	/*
	 * Under NT, 16-bit DOS applications will not run unless they
	 * can be attached to a console.  If we are running without a
	 * console, run the 16-bit program as an normal process inside
	 * of a hidden console application, and then run that hidden
	 * console as a detached process.
	 */

	if (hideConsole) {
	    startInfo.wShowWindow = SW_HIDE;
	} else {
	    /* For debugging, show the sub process console */
	    startInfo.wShowWindow = SW_SHOW;
	}
	startInfo.dwFlags |= STARTF_USESHOWWINDOW;
	createFlags = CREATE_NEW_CONSOLE;
	if (applType == EXP_APPL_DOS) {
	    Tcl_DStringAppend(&cmdLine, "cmd.exe /c ", -1);
	}
    } else {
	createFlags = DETACHED_PROCESS;
    }
    if (debug) {
	createFlags |= DEBUG_PROCESS;
    }
    if (newProcessGroup) {
	createFlags |= CREATE_NEW_PROCESS_GROUP;
    }
    
    /*
     * cmdLine gets the full command line used to invoke the executable,
     * including the name of the executable itself.  The command line
     * arguments in argv[] are stored in cmdLine separated by spaces. 
     * Special characters in individual arguments from argv[] must be 
     * quoted when being stored in cmdLine.
     *
     * When calling any application, bear in mind that arguments that 
     * specify a path name are not converted.  If an argument contains 
     * forward slashes as path separators, it may or may not be 
     * recognized as a path name, depending on the program.  In general,
     * most applications accept forward slashes only as option 
     * delimiters and backslashes only as paths.
     *
     * Additionally, when calling a 16-bit dos or windows application, 
     * all path names must use the short, cryptic, path format (e.g., 
     * using ab~1.def instead of "a b.default").  
     */

    BuildCommandLine(execPath, argc, argv, &cmdLine);

    if ((b = (*tclWinProcs->createProcessProc)(NULL, 
	    (TCHAR *) Tcl_DStringValue(&cmdLine), NULL, NULL, TRUE, 
	    (DWORD) createFlags, NULL, NULL, &startInfo, &procInfo)) == 0) {
	result = GetLastError();
	goto end;
    }

    /*
     * This wait is used to force the OS to give some time to the DOS
     * process.
     */

    if (applType == EXP_APPL_DOS) {
	WaitForSingleObject(procInfo.hProcess, 50);
    }

    /* 
     * "When an application spawns a process repeatedly, a new thread 
     * instance will be created for each process but the previous 
     * instances may not be cleaned up.  This results in a significant 
     * virtual memory loss each time the process is spawned.  If there 
     * is a WaitForInputIdle() call between CreateProcess() and
     * CloseHandle(), the problem does not occur." PSS ID Number: Q124121
     */

    WaitForInputIdle(procInfo.hProcess, 5000);
    CloseHandle(procInfo.hThread);

    *globalPidPtr = procInfo.dwProcessId;
    *pidPtr = (Tcl_Pid) procInfo.hProcess;
    if (*pidPtr != 0) {
	ProcInfo *procPtr = (ProcInfo *) ckalloc(sizeof(ProcInfo));
	procPtr->hProcess = procInfo.hProcess;
	procPtr->dwProcessId = procInfo.dwProcessId;
	procPtr->nextPtr = procList;
	procList = procPtr;
    }

    end:
    Tcl_DStringFree(&cmdLine);
    if (startInfo.hStdInput != INVALID_HANDLE_VALUE) {
        CloseHandle(startInfo.hStdInput);
    }
    if (startInfo.hStdOutput != INVALID_HANDLE_VALUE) {
        CloseHandle(startInfo.hStdOutput);
    }
    if (startInfo.hStdError != INVALID_HANDLE_VALUE) {
	CloseHandle(startInfo.hStdError);
    }
    return result;
}
