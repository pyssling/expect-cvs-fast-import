/* ----------------------------------------------------------------------
 * expWinConsoleDetour.cpp --
 *
 *	Console detour core implimentation.  At first, we will use the
 *	"Detours" library provided by Microsoft Research for testing.
 *	We will have to "heavily customize it" later to remove all
 *	copywrite before we can distribute this.
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
 * RCS: @(#) $Id: expWinConsoleDetour.cpp,v 1.1.2.1 2002/07/03 02:46:40 davygrvy Exp $
 * ----------------------------------------------------------------------
 */

#include "expWinSlave.hpp"

DETOUR_TRAMPOLINE(
	BOOL WINAPI Real_AllocConsole(VOID),
	AllocConsole
	);

DETOUR_TRAMPOLINE(
	BOOL WINAPI Real_Beep(DWORD dwFreq, DWORD dwDuration),
	Beep
	);

DETOUR_TRAMPOLINE(
	HANDLE WINAPI Real_CreateConsoleScreenBuffer(
		DWORD dwDesiredAccess,
		DWORD dwShareMode,
		CONST SECURITY_ATTRIBUTES *lpSecurityAttributes,
		DWORD dwFlags,
		LPVOID lpScreenBufferData),
	CreateConsoleScreenBuffer
	);

DETOUR_TRAMPOLINE(
	BOOL WINAPI Real_FillConsoleOutputAttribute(
		HANDLE hConsoleOutput,
		WORD wAttribute,
		DWORD nLength,
		COORD  dwWriteCoord,
		LPDWORD lpNumberOfAttrsWritten),
	FillConsoleOutputAttribute
	);

DETOUR_TRAMPOLINE(
	BOOL WINAPI Real_FillConsoleOutputCharacterA(
		HANDLE hConsoleOutput,
		CHAR cCharacter,
		DWORD  nLength,
		COORD  dwWriteCoord,
		LPDWORD lpNumberOfCharsWritten),
	FillConsoleOutputCharacterA
	);

DETOUR_TRAMPOLINE(
	BOOL WINAPI Real_FillConsoleOutputCharacterW(
		HANDLE hConsoleOutput,
		WCHAR cCharacter,
		DWORD  nLength,
		COORD  dwWriteCoord,
		LPDWORD lpNumberOfCharsWritten),
	FillConsoleOutputCharacterW
	);

DETOUR_TRAMPOLINE(
	BOOL WINAPI Real_FreeConsole(VOID),
	FreeConsole
	);

DETOUR_TRAMPOLINE(
	HANDLE WINAPI Real_GetStdHandle(DWORD nStdHandle),
	GetStdHandle
	);

//DETOUR_TRAMPOLINE(BOOL WINAPI Real_OpenConsoleW(???), OpenConsoleW);

DETOUR_TRAMPOLINE(
	BOOL WINAPI Real_ScrollConsoleScreenBufferA(
		HANDLE hConsoleOutput,
		CONST SMALL_RECT *lpScrollRectangle,
		CONST SMALL_RECT *lpClipRectangle,
		COORD dwDestinationOrigin,
		CONST CHAR_INFO *lpFill),
	ScrollConsoleScreenBufferA
	);

DETOUR_TRAMPOLINE(
	BOOL WINAPI Real_ScrollConsoleScreenBufferW(
		HANDLE hConsoleOutput,
		CONST SMALL_RECT *lpScrollRectangle,
		CONST SMALL_RECT *lpClipRectangle,
		COORD dwDestinationOrigin,
		CONST CHAR_INFO *lpFill),
	ScrollConsoleScreenBufferW
	);

DETOUR_TRAMPOLINE(
	BOOL WINAPI Real_SetConsoleActiveScreenBuffer(
		HANDLE hConsoleOutput),
	SetConsoleActiveScreenBuffer
	);

DETOUR_TRAMPOLINE(
	BOOL WINAPI Real_SetConsoleCP(UINT wCodePageID),
	SetConsoleCP
	);

DETOUR_TRAMPOLINE(
	BOOL WINAPI Real_SetConsoleCursorInfo(
		HANDLE hConsoleOutput,
		CONST CONSOLE_CURSOR_INFO *lpConsoleCursorInfo),
	SetConsoleCursorInfo
	);

DETOUR_TRAMPOLINE(
	BOOL WINAPI Real_SetConsoleCursorPosition(
		HANDLE hConsoleOutput,
		COORD dwCursorPosition),
	SetConsoleCursorPosition
	);

DETOUR_TRAMPOLINE(
	BOOL WINAPI Real_SetConsoleMode(
		HANDLE hConsoleHandle,
		DWORD dwMode),
	SetConsoleMode
	);

DETOUR_TRAMPOLINE(
	BOOL WINAPI Real_SetConsoleOutputCP(UINT wCodePageID),
	SetConsoleOutputCP
	);

DETOUR_TRAMPOLINE(
	BOOL WINAPI Real_SetConsoleTextAttribute(
		HANDLE hConsoleOutput,
		WORD wAttributes),
	SetConsoleTextAttribute
	);

DETOUR_TRAMPOLINE(
	BOOL WINAPI Real_SetConsoleWindowInfo(
		HANDLE hConsoleOutput,
		BOOL bAbsolute,
		CONST SMALL_RECT *lpConsoleWindow),
	SetConsoleWindowInfo
	);

DETOUR_TRAMPOLINE(
	BOOL WINAPI Real_WriteConsoleA(
		HANDLE hConsoleOutput,
		CONST VOID *lpBuffer,
		DWORD nNumberOfCharsToWrite,
		LPDWORD lpNumberOfCharsWritten,
		LPVOID lpReserved),
	WriteConsoleA
	);

DETOUR_TRAMPOLINE(
	BOOL WINAPI Real_WriteConsoleW(
		HANDLE hConsoleOutput,
		CONST VOID *lpBuffer,
		DWORD nNumberOfCharsToWrite,
		LPDWORD lpNumberOfCharsWritten,
		LPVOID lpReserved),
	WriteConsoleW
	);

DETOUR_TRAMPOLINE(
	BOOL WINAPI Real_WriteConsoleOutputA(
		HANDLE hConsoleOutput,
		CONST CHAR_INFO *lpBuffer,
		COORD dwBufferSize,
		COORD dwBufferCoord,
		PSMALL_RECT lpWriteRegion),
	WriteConsoleOutputA
	);

DETOUR_TRAMPOLINE(
	BOOL WINAPI Real_WriteConsoleOutputW(
		HANDLE hConsoleOutput,
		CONST CHAR_INFO *lpBuffer,
		COORD dwBufferSize,
		COORD dwBufferCoord,
		PSMALL_RECT lpWriteRegion),
	WriteConsoleOutputW
	);

DETOUR_TRAMPOLINE(
	BOOL WINAPI Real_WriteConsoleOutputCharacterA(
		HANDLE hConsoleOutput,
		LPCSTR lpCharacter,
		DWORD nLength,
		COORD dwWriteCoord,
		LPDWORD lpNumberOfCharsWritten),
	WriteConsoleOutputCharacterA
	);

DETOUR_TRAMPOLINE(
	BOOL WINAPI Real_WriteConsoleOutputCharacterW(
		HANDLE hConsoleOutput,
		LPCWSTR lpCharacter,
		DWORD nLength,
		COORD dwWriteCoord,
		LPDWORD lpNumberOfCharsWritten),
	WriteConsoleOutputCharacterW
	);

DETOUR_TRAMPOLINE(
	BOOL WINAPI Real_WriteFile(
		HANDLE hFile,
		LPCVOID lpBuffer,
		DWORD nNumberOfBytesToWrite,
		LPDWORD lpNumberOfBytesWritten,
		LPOVERLAPPED lpOverlapped),
	WriteFile
	);

DETOUR_TRAMPOLINE(
	BOOL WINAPI Real_IsWindowVisible(HWND hWnd),
	IsWindowVisible
	);


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
