/* ----------------------------------------------------------------------------
 * expWinSlaveTrap.hpp --
 *
 *	Declares the SlaveTrap classes which are our "invokation
 *	managers" of the trap method we run in a thread.
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
 * RCS: @(#) $Id: expWinSlaveTrap.hpp,v 1.1.2.4 2002/06/27 03:43:34 davygrvy Exp $
 * ----------------------------------------------------------------------------
 */

#ifndef INC_expWinSlaveTrap_hpp__
#define INC_expWinSlaveTrap_hpp__

class SlaveTrap {
public:
    virtual ~SlaveTrap() {};
    virtual void Write(Message *) = 0;
};

// This uses the debugger method by setting breakpoints on all calls the
// process makes to the console API and running the process within the
// native OS debugger.  See expWinConsoleDebugger.hpp for the details of
// what we run in the thread.
//
class SlaveTrapDbg : public SlaveTrap
{
public:
    SlaveTrapDbg(int argc, char * const argv[], CMclQueue<Message *> &_mQ);
    virtual ~SlaveTrapDbg();
    virtual void Write(Message *);

private:
    ConsoleDebugger *debugger;
    CMclQueue<Message *> &mQ;
    CMclThread *debuggerThread;
};

// This uses the detour method by rewriting the import directory of the
// processes' image (in memory) so as to pass though us.  This method
// doesn't require the debugger APIs.  See expWinConsoleDetour.hpp for
// more detials.
//
class SlaveTrapDetour : public SlaveTrap
{
public:
    SlaveTrapDetour(int argc, char * const argv[], CMclQueue<Message *> &_mQ);
    virtual ~SlaveTrapDetour();
    virtual void Write(Message *);

private:
    CMclQueue<Message *> &mQ;
};

#endif