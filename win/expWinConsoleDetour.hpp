/* ----------------------------------------------------------------------------
 * expWinConsoleDebugger.hpp --
 *
 *	ConsoleDetour class declared here.
 *
 * ----------------------------------------------------------------------------
 *
 * Written by: Don Libes, libes@cme.nist.gov, NIST, 12/3/90
 * 
 * Design and implementation of this program was paid for by U.S. tax
 * dollars.  Therefore it is public domain.  However, the author and NIST
 * would appreciate credit if this program or parts of it are used.
 * 
 * Copyright (c) 1997 Mitel Corporation
 *	work by Gordon Chaffee <chaffee@bmrc.berkeley.edu> for the WinNT port.
 *
 * Copyright (c) 2001-2002 Telindustrie, LLC
 *	work by David Gravereaux <davygrvy@pobox.com> for any Win32 OS.
 *
 * ----------------------------------------------------------------------------
 * URLs:    http://expect.nist.gov/
 *	    http://expect.sf.net/
 *	    http://bmrc.berkeley.edu/people/chaffee/expectnt.html
 * ----------------------------------------------------------------------------
 * RCS: @(#) $Id: expWinConsoleDetour.hpp,v 1.1.2.1 2002/07/03 02:46:40 davygrvy Exp $
 * ----------------------------------------------------------------------------
 */

#ifndef INC_expWinConsoleDetour_hpp__
#define INC_expWinConsoleDetour_hpp__

#include "expWinSlave.hpp"
#include "./Detours/include/detours.h"

class ConsoleDetour : public CMclThreadHandler, ArgMaker
{
public:
    ConsoleDetour(int _argc, char * const *_argv, CMclQueue<Message *> &_mQ);
    ~ConsoleDetour();

    void WriteRecord (INPUT_RECORD *ir);
    void EnterInteract (HANDLE OutConsole);
    void ExitInteract (void);

private:
    virtual unsigned ThreadHandlerProc(void);

    // Thread-safe message queue used for communication back to the parent.
    //
    CMclQueue<Message *> &mQ;
};

#endif  // #ifndef INC_expWinConsoleDetour_hpp__