#include "expWinInt.h"


SpawnPipeClient::SpawnPipeClient(const char *name, CMclQueue<Message> &_mQ)
    : mQ(_mQ)
{
    hStdIn  = GetStdHandle(STD_INPUT_HANDLE);
    hStdOut = GetStdHandle(STD_OUTPUT_HANDLE);
    hStdErr = GetStdHandle(STD_ERROR_HANDLE);
}

void
SpawnPipeClient::Write(Message &what)
{
    DWORD dwWritten;

    switch (what.mode) {
    case Message::TYPE_NORMAL:
	WriteFile(hStdOut, what.bytes, what.length, &dwWritten, 0L);
    case Message::TYPE_ERROR:
	WriteFile(hStdErr, what.bytes, what.length, &dwWritten, 0L);
    }
}
