/*
    Follow the mouse.
*/

#define WIN32_LEAN_AND_MEAN
#include <windows.h>
#include <wincon.h>
#include "./Mcl/include/cmcl.h"

#define RECORD_SIZE 15
INPUT_RECORD Records[RECORD_SIZE];
HANDLE ConIn, ConOut;
CMclEvent stop;

void HandleKey(KEY_EVENT_RECORD *ker);
void HandleMouse(MOUSE_EVENT_RECORD *mer);
void HandleWindow(WINDOW_BUFFER_SIZE_RECORD *wbsr);
BOOL isDiff(MOUSE_EVENT_RECORD *mer1, MOUSE_EVENT_RECORD *mer2);
BOOL WINAPI ConsoleHandler(DWORD code);

int main (void)
{
    DWORD dwRead, i;
    CONSOLE_CURSOR_INFO cci = {100, 1};
    MOUSE_EVENT_RECORD lastMouse;
    CMclWaitableCollection groupedHandles;
    DWORD dwRet;
    HANDLE oldBuffer;

    ConIn = CreateFile("CONIN$", GENERIC_READ|GENERIC_WRITE,
	    FILE_SHARE_READ|FILE_SHARE_WRITE, 0L, OPEN_EXISTING, 0, 0L);
    oldBuffer = CreateFile("CONOUT$", GENERIC_READ|GENERIC_WRITE,
	    FILE_SHARE_READ|FILE_SHARE_WRITE, 0L, OPEN_EXISTING, 0, 0L);
    ConOut = CreateConsoleScreenBuffer(GENERIC_READ|GENERIC_WRITE,
	    FILE_SHARE_READ|FILE_SHARE_WRITE, 0L, CONSOLE_TEXTMODE_BUFFER, 0L);

    if (!ConIn || !oldBuffer || !ConOut) {
	WriteFile(GetStdHandle(STD_ERROR_HANDLE), "need console! die, die!\n", 24, &dwRet, 0L);
	return -1;
    }

    SetConsoleCtrlHandler(ConsoleHandler, TRUE);
    SetConsoleActiveScreenBuffer(ConOut);
    SetConsoleMode(ConIn, ENABLE_WINDOW_INPUT|ENABLE_MOUSE_INPUT);
    SetConsoleCursorInfo(ConOut, &cci);

    // Index 0, highest priority goes first.
    groupedHandles.AddObject(stop);

    // Console handles are waitable objects.
    // Index 1
    groupedHandles.AddObject(ConIn);

again:
    // Wait for any (either) to signal.
    dwRet = groupedHandles.Wait(FALSE, INFINITE);

    if (!CMclWaitSucceeded(dwRet, groupedHandles.GetCount()) ||
	    CMclWaitSucceededIndex(dwRet) == 0) {
	// stop event or some other error, so die.
	CloseHandle(ConIn);
	CloseHandle(ConOut);
	SetConsoleActiveScreenBuffer(oldBuffer);
	return 0;
    }

    ReadConsoleInput(ConIn, Records, RECORD_SIZE, &dwRead);
    for (i = 0; i < dwRead; i++) {
	switch (Records[i].EventType) {
	case KEY_EVENT:
	    HandleKey(&Records[i].Event.KeyEvent);
	    break;
	case MOUSE_EVENT:
	    if (isDiff(&lastMouse, &Records[i].Event.MouseEvent)) {
		lastMouse = Records[i].Event.MouseEvent;
		HandleMouse(&Records[i].Event.MouseEvent);
	    }
	    break;
	case WINDOW_BUFFER_SIZE_EVENT:
	    HandleWindow(&Records[i].Event.WindowBufferSizeEvent);
	    break;
	case MENU_EVENT:
	case FOCUS_EVENT:
	    // internal use. DNU.
	    break;
	}
    }

    goto again;
}

void
HandleKey(KEY_EVENT_RECORD *ker)
{
    // exit on esc.
    if (ker->wVirtualKeyCode == VK_ESCAPE) stop.Set();
}

void
HandleMouse(MOUSE_EVENT_RECORD *mer)
{
    SetConsoleCursorPosition(ConOut, mer->dwMousePosition);
}

void
HandleWindow(WINDOW_BUFFER_SIZE_RECORD *wbsr)
{
}

BOOL
isDiff(MOUSE_EVENT_RECORD *mer1, MOUSE_EVENT_RECORD *mer2)
{
    if (
	mer1->dwMousePosition.X ^ mer2->dwMousePosition.X ||
	mer1->dwMousePosition.Y ^ mer2->dwMousePosition.Y ||
	mer1->dwButtonState     ^ mer2->dwButtonState ||
	mer1->dwControlKeyState ^ mer2->dwControlKeyState ||
	mer1->dwEventFlags      ^ mer2->dwEventFlags
    ) {
	return TRUE;
    } else {
	return FALSE;
    }

}

BOOL WINAPI
ConsoleHandler (DWORD code)
{
    // this routine is called by a kernel supplied thread.

    // no matter what condition comes in, shutdown.
    stop.Set();
    return TRUE;
}
