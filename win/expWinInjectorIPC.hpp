/* ----------------------------------------------------------------------------
 * expWinInjectorMain.cpp --
 *
 *	Console event injector DLL that's loaded into the slave's address space
 *	used by the ConsoleDebugger class for "writing" to the slave.
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
 * RCS: @(#) $Id: expWinInjectorMain.cpp,v 1.1.2.13 2002/06/25 08:40:50 davygrvy Exp $
 * ----------------------------------------------------------------------------
 */

#ifndef INC_expWinInjectorIPC_hpp__
#define INC_expWinInjectorIPC_hpp__

#define IPC_NUMSLOTS 80
#define IPC_SLOTSIZE sizeof(INPUT_RECORD)

#endif