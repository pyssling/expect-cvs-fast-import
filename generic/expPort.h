/*
 * expPort.h --
 *
 *	This header file handles porting issues that occur because
 *	of differences between systems.  It reads in platform specific
 *	portability files.
 *
 * RCS: @(#) $Id: tclPort.h,v 1.5 1999/05/25 01:00:27 stanton Exp $
 */

#ifndef _EXPPORT_H__
#define _EXPPORT_H__

#define HAVE_MEMCPY

#ifdef __WIN32__
#   include "../win/expWinPort.h"
#else
#   if defined(MAC_TCL)
#	include "../mac/expPort.h"
#    else
#	include "../unix/expUnixPort.h"
#    endif
#endif

#endif /* _EXPPORT_H__ */
