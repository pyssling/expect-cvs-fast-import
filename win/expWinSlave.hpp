/* ----------------------------------------------------------------------------
 * expWinSlave.hpp --
 *
 *	Useful definitions used by the slave driver application but not
 *	useful for anybody else.
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
 * RCS: @(#) $Id: expWinSlave.hpp,v 1.1.4.12 2002/06/23 09:22:45 davygrvy Exp $
 * ----------------------------------------------------------------------------
 */
#ifndef INC_expWinSlave_hpp__
#define INC_expWinSlave_hpp__

#include <windows.h>

void DynloadTclStubs (void);
void ShutdownTcl (void);


#include "Mcl/include/CMcl.h"
#include "slavedrvmc.h"
#include "expWinUtils.hpp"
#include "expWinMessage.hpp"
#include "expWinTestClient.hpp"
#include "expWinConsoleDebugger.hpp"
#include "expWinSlaveTrap.hpp"

void MapToKeys (Message *msg, CMclQueue<Message *> &mQ);


#endif // INC_expWinSlave_hpp__