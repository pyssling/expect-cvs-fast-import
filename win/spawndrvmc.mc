;/* ----------------------------------------------------------------------------
; * spawndrvmc.mc --
; *
; *	This file contains the message catalog for use with Win32 error
; *	reporting through ReportEvent() and FormatMessage().
; *
; * Copyright (c) 2001 Telindustrie, LLC
; *
; * Authors: David Gravereaux <davygrvy@pobox.com>
; *
; * See the file "license.terms" for information on usage and redistribution
; * of this file, and for a DISCLAIMER OF ALL WARRANTIES.
; * ----------------------------------------------------------------------------
; */

;//MessageIdTypedef = DWORD
OutputBase = 16

;// 2 bits max!
SeverityNames = (
    Success=0x0:STATUS_SEVERITY_SUCCESS
    Informational=0x1:STATUS_SEVERITY_INFORMATIONAL
    Warning=0x2:STATUS_SEVERITY_WARNING
    Error=0x3:STATUS_SEVERITY_ERROR
)

;// 12 bits max!
FacilityNames = (
    System=0x0:FACILITY_SYSTEM
    Stubs=0x1:FACILITY_STUBS
    Io=0x2:FACILITY_IO
    Mailbox=0x3:FACILITY_MAILBOX
    NamedPipe=0x3:FACILITY_NAMEDPIPE
    WinSock=0x4:FACILITY_WINSOCK
    DbgTrap=0x5:FACILITY_DBGTRAP
)

LanguageNames=(English=0x409:MSG00409)
;//LanguageNames=(Japanese=0x411:MSG00411)


;// Message definitions


MessageId=0x1
Severity=Error
Facility=Stubs
SymbolicName=STUBS_TCLDLL_CANTFIND
Language=English
Tcl is not available.  The Tcl Dll as specified in the envar EXP_TCLDLL as
"%1" could not be loaded by LoadLibrary().  Ensure EXP_TCLDLL has the correct
core and is the fullpath.
.

MessageId=0x2
Severity=Warning
Facility=Io
SymbolicName=MSG_BAD_PARM1
Language=English
Cannot reconnect to the server.
.

MessageId=0x3
Severity=Success
Facility=System
SymbolicName=MSG_STRIKE_ANY_KEY
Language=English
Press any key to continue . . . %0
.

MessageId=0x4
Severity=Error
Facility=System
SymbolicName=MSG_CMD_DELETE
Language=English
File %1 contains %2 which is in error
.

MessageId=0x5
Severity=Success
Facility=System
SymbolicName=MSG_RETRYS
Language=English
There have been %1!d! retrys with %2!d!%% success%! Disconnect from% 
the server and retry later.
.
