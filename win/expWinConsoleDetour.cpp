/* ----------------------------------------------------------------------
 * expWinConsoleDetour.cpp --
 *
 *	Console detour core implimentation.  At first, we will use the
 *	"Detours" library provided by Microsoft Research for testing.
 *	We will have to "heavily customize it to our special needs"
 *	later on to remove all copywrite before we can distribute this.
 *
 *	see: http://research.microsoft.com/sn/detours/
 *
 * ----------------------------------------------------------------------
 *
 * Written by: Don Libes, libes@cme.nist.gov, NIST, 12/3/90
 * 
 * Design and implementation of this program was paid for by U.S. tax
 * dollars.  Therefore it is public domain.  However, the author and
 * NIST would appreciate credit if this program or parts of it are used.
 * 
 * Copyright (c) 1997 Mitel Corporation
 *	work by Gordon Chaffee <chaffee@bmrc.berkeley.edu> for the NT
 *	port.
 *
 * Copyright (c) 2001-2002 Telindustrie, LLC
 *	work by David Gravereaux <davygrvy@pobox.com> for any Win32 OS.
 *
 * ----------------------------------------------------------------------
 * URLs:    http://expect.nist.gov/
 *	    http://expect.sf.net/
 *	    http://bmrc.berkeley.edu/people/chaffee/expectnt.html
 * ----------------------------------------------------------------------
 * RCS: @(#) $Id: expWinConsoleDetour.cpp,v 1.1.2.2 2002/07/03 04:25:36 davygrvy Exp $
 * ----------------------------------------------------------------------
 */

#include "expWinSlave.hpp"

DETOUR_TRAMPOLINE(
	BOOL WINAPI Yours_AllocConsole(VOID),
	AllocConsole
	);

DETOUR_TRAMPOLINE(
	BOOL WINAPI Yours_Beep(DWORD dwFreq, DWORD dwDuration),
	Beep
	);

DETOUR_TRAMPOLINE(
	HANDLE WINAPI Yours_CreateConsoleScreenBuffer(
		DWORD dwDesiredAccess,
		DWORD dwShareMode,
		CONST SECURITY_ATTRIBUTES *lpSecurityAttributes,
		DWORD dwFlags,
		LPVOID lpScreenBufferData),
	CreateConsoleScreenBuffer
	);

DETOUR_TRAMPOLINE(
	BOOL WINAPI Yours_FillConsoleOutputAttribute(
		HANDLE hConsoleOutput,
		WORD wAttribute,
		DWORD nLength,
		COORD  dwWriteCoord,
		LPDWORD lpNumberOfAttrsWritten),
	FillConsoleOutputAttribute
	);

DETOUR_TRAMPOLINE(
	BOOL WINAPI Yours_FillConsoleOutputCharacterA(
		HANDLE hConsoleOutput,
		CHAR cCharacter,
		DWORD  nLength,
		COORD  dwWriteCoord,
		LPDWORD lpNumberOfCharsWritten),
	FillConsoleOutputCharacterA
	);

DETOUR_TRAMPOLINE(
	BOOL WINAPI Yours_FillConsoleOutputCharacterW(
		HANDLE hConsoleOutput,
		WCHAR cCharacter,
		DWORD  nLength,
		COORD  dwWriteCoord,
		LPDWORD lpNumberOfCharsWritten),
	FillConsoleOutputCharacterW
	);

DETOUR_TRAMPOLINE(
	BOOL WINAPI Yours_FreeConsole(VOID),
	FreeConsole
	);

DETOUR_TRAMPOLINE(
	HANDLE WINAPI Yours_GetStdHandle(DWORD nStdHandle),
	GetStdHandle
	);

//DETOUR_TRAMPOLINE(BOOL WINAPI Yours_OpenConsoleW(???), OpenConsoleW);

DETOUR_TRAMPOLINE(
	BOOL WINAPI Yours_ScrollConsoleScreenBufferA(
		HANDLE hConsoleOutput,
		CONST SMALL_RECT *lpScrollRectangle,
		CONST SMALL_RECT *lpClipRectangle,
		COORD dwDestinationOrigin,
		CONST CHAR_INFO *lpFill),
	ScrollConsoleScreenBufferA
	);

DETOUR_TRAMPOLINE(
	BOOL WINAPI Yours_ScrollConsoleScreenBufferW(
		HANDLE hConsoleOutput,
		CONST SMALL_RECT *lpScrollRectangle,
		CONST SMALL_RECT *lpClipRectangle,
		COORD dwDestinationOrigin,
		CONST CHAR_INFO *lpFill),
	ScrollConsoleScreenBufferW
	);

DETOUR_TRAMPOLINE(
	BOOL WINAPI Yours_SetConsoleActiveScreenBuffer(
		HANDLE hConsoleOutput),
	SetConsoleActiveScreenBuffer
	);

DETOUR_TRAMPOLINE(
	BOOL WINAPI Yours_SetConsoleCP(UINT wCodePageID),
	SetConsoleCP
	);

DETOUR_TRAMPOLINE(
	BOOL WINAPI Yours_SetConsoleCursorInfo(
		HANDLE hConsoleOutput,
		CONST CONSOLE_CURSOR_INFO *lpConsoleCursorInfo),
	SetConsoleCursorInfo
	);

DETOUR_TRAMPOLINE(
	BOOL WINAPI Yours_SetConsoleCursorPosition(
		HANDLE hConsoleOutput,
		COORD dwCursorPosition),
	SetConsoleCursorPosition
	);

DETOUR_TRAMPOLINE(
	BOOL WINAPI Yours_SetConsoleMode(
		HANDLE hConsoleHandle,
		DWORD dwMode),
	SetConsoleMode
	);

DETOUR_TRAMPOLINE(
	BOOL WINAPI Yours_SetConsoleOutputCP(UINT wCodePageID),
	SetConsoleOutputCP
	);

DETOUR_TRAMPOLINE(
	BOOL WINAPI Yours_SetConsoleTextAttribute(
		HANDLE hConsoleOutput,
		WORD wAttributes),
	SetConsoleTextAttribute
	);

DETOUR_TRAMPOLINE(
	BOOL WINAPI Yours_SetConsoleWindowInfo(
		HANDLE hConsoleOutput,
		BOOL bAbsolute,
		CONST SMALL_RECT *lpConsoleWindow),
	SetConsoleWindowInfo
	);

DETOUR_TRAMPOLINE(
	BOOL WINAPI Yours_WriteConsoleA(
		HANDLE hConsoleOutput,
		CONST VOID *lpBuffer,
		DWORD nNumberOfCharsToWrite,
		LPDWORD lpNumberOfCharsWritten,
		LPVOID lpReserved),
	WriteConsoleA
	);

DETOUR_TRAMPOLINE(
	BOOL WINAPI Yours_WriteConsoleW(
		HANDLE hConsoleOutput,
		CONST VOID *lpBuffer,
		DWORD nNumberOfCharsToWrite,
		LPDWORD lpNumberOfCharsWritten,
		LPVOID lpReserved),
	WriteConsoleW
	);

DETOUR_TRAMPOLINE(
	BOOL WINAPI Yours_WriteConsoleOutputA(
		HANDLE hConsoleOutput,
		CONST CHAR_INFO *lpBuffer,
		COORD dwBufferSize,
		COORD dwBufferCoord,
		PSMALL_RECT lpWriteRegion),
	WriteConsoleOutputA
	);

DETOUR_TRAMPOLINE(
	BOOL WINAPI Yours_WriteConsoleOutputW(
		HANDLE hConsoleOutput,
		CONST CHAR_INFO *lpBuffer,
		COORD dwBufferSize,
		COORD dwBufferCoord,
		PSMALL_RECT lpWriteRegion),
	WriteConsoleOutputW
	);

DETOUR_TRAMPOLINE(
	BOOL WINAPI Yours_WriteConsoleOutputCharacterA(
		HANDLE hConsoleOutput,
		LPCSTR lpCharacter,
		DWORD nLength,
		COORD dwWriteCoord,
		LPDWORD lpNumberOfCharsWritten),
	WriteConsoleOutputCharacterA
	);

DETOUR_TRAMPOLINE(
	BOOL WINAPI Yours_WriteConsoleOutputCharacterW(
		HANDLE hConsoleOutput,
		LPCWSTR lpCharacter,
		DWORD nLength,
		COORD dwWriteCoord,
		LPDWORD lpNumberOfCharsWritten),
	WriteConsoleOutputCharacterW
	);

DETOUR_TRAMPOLINE(
	BOOL WINAPI Yours_WriteFile(
		HANDLE hFile,
		LPCVOID lpBuffer,
		DWORD nNumberOfBytesToWrite,
		LPDWORD lpNumberOfBytesWritten,
		LPOVERLAPPED lpOverlapped),
	WriteFile
	);

DETOUR_TRAMPOLINE(
	BOOL WINAPI Yours_IsWindowVisible(HWND hWnd),
	IsWindowVisible
	);



BOOL WINAPI
Mine_AllocConsole (VOID)
{
    BOOL rv = FALSE;
    __try {
        rv = Yours_AllocConsole();
    }
    __finally {}
    return rv;
}

BOOL WINAPI
Mine_Beep (
	DWORD dwFreq,
	DWORD dwDuration)
{
    // Do nothing, be quiet.
    return TRUE;
}

HANDLE WINAPI
Mine_CreateConsoleScreenBuffer (
	DWORD dwDesiredAccess,
	DWORD dwShareMode,
	CONST SECURITY_ATTRIBUTES *lpSecurityAttributes,
	DWORD dwFlags,
	LPVOID lpScreenBufferData)
{
    HANDLE rv = INVALID_HANDLE_VALUE;
    __try {
        rv = Yours_CreateConsoleScreenBuffer(dwDesiredAccess, dwShareMode,
		lpSecurityAttributes, dwFlags, lpScreenBufferData);
    }
    __finally {}
    return rv;
}

BOOL WINAPI
Mine_FillConsoleOutputAttribute(
	HANDLE hConsoleOutput,
	WORD wAttribute,
	DWORD nLength,
	COORD  dwWriteCoord,
	LPDWORD lpNumberOfAttrsWritten)
{
    BOOL rv = FALSE;
    __try {
        rv = Yours_FillConsoleOutputAttribute(hConsoleOutput, wAttribute,
		nLength, dwWriteCoord, lpNumberOfAttrsWritten);
    }
    __finally {}
    return rv;
}

BOOL WINAPI
Mine_FillConsoleOutputCharacterA(
	HANDLE hConsoleOutput,
	CHAR cCharacter,
	DWORD  nLength,
	COORD  dwWriteCoord,
	LPDWORD lpNumberOfCharsWritten)
{
    BOOL rv = FALSE;
    __try {
        rv = Yours_FillConsoleOutputCharacterA(hConsoleOutput, cCharacter,
		nLength, dwWriteCoord, lpNumberOfCharsWritten);
    }
    __finally {}
    return rv;
}

BOOL WINAPI
Mine_FillConsoleOutputCharacterW(
	HANDLE hConsoleOutput,
	WCHAR cCharacter,
	DWORD  nLength,
	COORD  dwWriteCoord,
	LPDWORD lpNumberOfCharsWritten)
{
    BOOL rv = FALSE;
    __try {
        rv = Yours_FillConsoleOutputCharacterW(hConsoleOutput, cCharacter,
		nLength, dwWriteCoord, lpNumberOfCharsWritten);
    }
    __finally {}
    return rv;
}

BOOL WINAPI
Mine_FreeConsole(VOID)
{
    BOOL rv = FALSE;
    __try {
        rv = Yours_FreeConsole();
    }
    __finally {}
    return rv;
}

HANDLE WINAPI
Mine_GetStdHandle(DWORD nStdHandle)
{
    HANDLE rv = INVALID_HANDLE_VALUE;
    __try {
        rv = Yours_GetStdHandle(nStdHandle);
    }
    __finally {}
    return rv;
}

BOOL WINAPI
Mine_ScrollConsoleScreenBufferA(
	HANDLE hConsoleOutput,
	CONST SMALL_RECT *lpScrollRectangle,
	CONST SMALL_RECT *lpClipRectangle,
	COORD dwDestinationOrigin,
	CONST CHAR_INFO *lpFill)
{
    BOOL rv = FALSE;
    __try {
        rv = Yours_ScrollConsoleScreenBufferA(hConsoleOutput, lpScrollRectangle,
		lpClipRectangle, dwDestinationOrigin, lpFill);
    }
    __finally {}
    return rv;
}

BOOL WINAPI
Mine_ScrollConsoleScreenBufferW(
	HANDLE hConsoleOutput,
	CONST SMALL_RECT *lpScrollRectangle,
	CONST SMALL_RECT *lpClipRectangle,
	COORD dwDestinationOrigin,
	CONST CHAR_INFO *lpFill)
{
    BOOL rv = FALSE;
    __try {
        rv = Yours_ScrollConsoleScreenBufferW(hConsoleOutput, lpScrollRectangle,
		lpClipRectangle, dwDestinationOrigin, lpFill);
    }
    __finally {}
    return rv;
}

BOOL WINAPI
Mine_SetConsoleActiveScreenBuffer(HANDLE hConsoleOutput)
{
    BOOL rv = FALSE;
    __try {
        rv = Yours_SetConsoleActiveScreenBuffer(hConsoleOutput);
    }
    __finally {}
    return rv;
}

BOOL WINAPI
Mine_SetConsoleCP(UINT wCodePageID)
{
    BOOL rv = FALSE;
    __try {
        rv = Yours_SetConsoleCP(wCodePageID);
    }
    __finally {}
    return rv;
}

BOOL WINAPI
Mine_SetConsoleCursorInfo(
	HANDLE hConsoleOutput,
	CONST CONSOLE_CURSOR_INFO *lpConsoleCursorInfo)
{
    BOOL rv = FALSE;
    __try {
        rv = Yours_SetConsoleCursorInfo(hConsoleOutput, lpConsoleCursorInfo);
    }
    __finally {}
    return rv;
}

BOOL WINAPI
Mine_SetConsoleCursorPosition(
	HANDLE hConsoleOutput,
	COORD dwCursorPosition)
{
    BOOL rv = FALSE;
    __try {
        rv = Yours_SetConsoleCursorPosition(hConsoleOutput, dwCursorPosition);
    }
    __finally {}
    return rv;
}

BOOL WINAPI
Mine_SetConsoleMode(
	HANDLE hConsoleHandle,
	DWORD dwMode)
{
    BOOL rv = FALSE;
    __try {
        rv = Yours_SetConsoleMode(hConsoleHandle, dwMode);
    }
    __finally {}
    return rv;
}

BOOL WINAPI
Mine_SetConsoleOutputCP(UINT wCodePageID)
{
    BOOL rv = FALSE;
    __try {
        rv = Yours_SetConsoleOutputCP(wCodePageID);
    }
    __finally {}
    return rv;
}

BOOL WINAPI
Mine_SetConsoleTextAttribute(
	HANDLE hConsoleOutput,
	WORD wAttributes)
{
    BOOL rv = FALSE;
    __try {
        rv = Yours_SetConsoleTextAttribute(hConsoleOutput, wAttributes);
    }
    __finally {}
    return rv;
}

BOOL WINAPI
Mine_SetConsoleWindowInfo(
	HANDLE hConsoleOutput,
	BOOL bAbsolute,
	CONST SMALL_RECT *lpConsoleWindow)
{
    BOOL rv = FALSE;
    __try {
        rv = Yours_SetConsoleWindowInfo(hConsoleOutput, bAbsolute,
		lpConsoleWindow);
    }
    __finally {}
    return rv;
}

BOOL WINAPI
Mine_WriteConsoleA(
	HANDLE hConsoleOutput,
	CONST VOID *lpBuffer,
	DWORD nNumberOfCharsToWrite,
	LPDWORD lpNumberOfCharsWritten,
	LPVOID lpReserved)
{
    BOOL rv = FALSE;
    __try {
        rv = Yours_WriteConsoleA(hConsoleOutput, lpBuffer,
		nNumberOfCharsToWrite, lpNumberOfCharsWritten,
		lpReserved);
    }
    __finally {}
    return rv;
}

BOOL WINAPI
Mine_WriteConsoleW(
	HANDLE hConsoleOutput,
	CONST VOID *lpBuffer,
	DWORD nNumberOfCharsToWrite,
	LPDWORD lpNumberOfCharsWritten,
	LPVOID lpReserved)
{
    BOOL rv = FALSE;
    __try {
        rv = Yours_WriteConsoleW(hConsoleOutput, lpBuffer,
		nNumberOfCharsToWrite, lpNumberOfCharsWritten,
		lpReserved);
    }
    __finally {}
    return rv;
}

BOOL WINAPI
Mine_WriteConsoleOutputA(
	HANDLE hConsoleOutput,
	CONST CHAR_INFO *lpBuffer,
	COORD dwBufferSize,
	COORD dwBufferCoord,
	PSMALL_RECT lpWriteRegion)
{
    BOOL rv = FALSE;
    __try {
        rv = Yours_WriteConsoleOutputA(hConsoleOutput, lpBuffer,
		dwBufferSize, dwBufferCoord, lpWriteRegion);
    }
    __finally {}
    return rv;
}

BOOL WINAPI
Mine_WriteConsoleOutputW(
	HANDLE hConsoleOutput,
	CONST CHAR_INFO *lpBuffer,
	COORD dwBufferSize,
	COORD dwBufferCoord,
	PSMALL_RECT lpWriteRegion)
{
    BOOL rv = FALSE;
    __try {
        rv = Yours_WriteConsoleOutputW(hConsoleOutput, lpBuffer,
		dwBufferSize, dwBufferCoord, lpWriteRegion);
    }
    __finally {}
    return rv;
}

BOOL WINAPI
Mine_WriteConsoleOutputCharacterA(
	HANDLE hConsoleOutput,
	LPCSTR lpCharacter,
	DWORD nLength,
	COORD dwWriteCoord,
	LPDWORD lpNumberOfCharsWritten)
{
    BOOL rv = FALSE;
    __try {
        rv = Yours_WriteConsoleOutputCharacterA(hConsoleOutput, lpCharacter,
		nLength, dwWriteCoord, lpNumberOfCharsWritten);
    }
    __finally {}
    return rv;
}

BOOL WINAPI
Mine_WriteConsoleOutputCharacterW(
	HANDLE hConsoleOutput,
	LPCWSTR lpCharacter,
	DWORD nLength,
	COORD dwWriteCoord,
	LPDWORD lpNumberOfCharsWritten)
{
    BOOL rv = FALSE;
    __try {
        rv = Yours_WriteConsoleOutputCharacterW(hConsoleOutput, lpCharacter,
		nLength, dwWriteCoord, lpNumberOfCharsWritten);
    }
    __finally {}
    return rv;
}

BOOL WINAPI
Mine_WriteFile(
	HANDLE hFile,
	LPCVOID lpBuffer,
	DWORD nNumberOfBytesToWrite,
	LPDWORD lpNumberOfBytesWritten,
	LPOVERLAPPED lpOverlapped)
{
    BOOL rv = FALSE;
    __try {
        rv = Yours_WriteFile(hFile, lpBuffer, nNumberOfBytesToWrite,
		lpNumberOfBytesWritten, lpOverlapped);
    }
    __finally {}
    return rv;
}

BOOL WINAPI
Mine_IsWindowVisible(HWND hWnd)
{
    return TRUE;
}




ConsoleDetour::ConsoleDetour (
	int _argc,
	char * const *_argv,
	CMclQueue<Message *> &_mQ)
    : mQ(_mQ)
{
}

ConsoleDetour::~ConsoleDetour ()
{
}

unsigned
ConsoleDetour::ThreadHandlerProc (void)
{
    return 0;
}

void
ConsoleDetour::WriteRecord (INPUT_RECORD *ir)
{
}

void
ConsoleDetour::EnterInteract (HANDLE OutConsole)
{
}

void
ConsoleDetour::ExitInteract (void)
{
}
