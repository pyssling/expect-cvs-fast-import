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

MessageIdTypedef = DWORD
OutputBase = 16

SeverityNames = (
    Success=0x0:STATUS_SEVERITY_SUCCESS
    Informational=0x1:STATUS_SEVERITY_INFORMATIONAL
    Warning=0x2:STATUS_SEVERITY_WARNING
    Fatal=0x3:STATUS_SEVERITY_FATAL
)

FacilityNames = (
    Catagories=0x0
    System=0x0:FACILITY_SYSTEM
    Stubs=0x1:FACILITY_STUBS
    Io=0x2:FACILITY_IO
    Mailbox=0x3:FACILITY_MAILBOX
    NamedPipe=0x4:FACILITY_NAMEDPIPE
    WinSock=0x5:FACILITY_WINSOCK
    DbgTrap=0x6:FACILITY_DBGTRAP
    MasterSlave_Protocol=0x7:FACILITY_MSPROTO
)

LanguageNames=(English=0x409:MSG00409)

MessageId=0x1
Severity=Success
Facility=Catagories
Language=English
Stubs
.

MessageId=0x2
Severity=Success
Facility=Catagories
Language=English
General I/O
.

MessageId=0x3
Severity=Success
Facility=Catagories
Language=English
MailBoxing IPC
.

MessageId=0x4
Severity=Success
Facility=Catagories
Language=English
NamedPipe IPC
.

MessageId=0x5
Severity=Success
Facility=Catagories
Language=English
WinSock IPC
.

MessageId=0x6
Severity=Success
Facility=Catagories
Language=English
Console API traps
.

MessageId=0x7
Severity=Success
Facility=Catagories
Language=English
Master/Slave protocol
.

MessageId=0x1
Severity=Error
Facility=System
SymbolicName=MSG_BYPASS
Language=English
%1
.



MessageId=0x1
Severity=Error
Facility=Io
SymbolicName=MSG_IO_ARGSWRONG
Language=English
%1 : %2 (%3,%4): No commandline arguements to slavedrv.exe.  No work to do.
.

MessageId=0x2
Severity=Warning
Facility=Io
SymbolicName=MSG_IO_BADSHUTDOWN
Language=English
%1 : %2 (%3,%4): Unclean shutdown: %5
.

MessageId=0x2
Severity=Error
Facility=Io
SymbolicName=MSG_IO_UNEXPECTED
Language=English
%1 : %2 (%3,%4): Unexpected error: %5
.



MessageId=0x0
Severity=Error
Facility=Mailbox
SymbolicName=MSG_MB_CANTOPENCLIENT1
Language=English
%1 : %2 (%3,%4): Can't open client-side IPC mailbox for named mailbox "%5".
.

MessageId=0x1
Severity=Error
Facility=Mailbox
SymbolicName=MSG_MB_CANTOPENCLIENT2
Language=English
%1 : %2 (%3,%4): Can't open client-side IPC mailbox for named mailbox "%5".  Got system error %6
.



MessageId=0x1
Severity=Fatal
Facility=Stubs
SymbolicName=MSG_STUBS_TCLDLLCANTFIND
Language=English
%1 : %2 (%3,%4): Tcl is not available.  The Tcl Dll as specified in the environment variable EXP_TCLDLL as "%5" could not be loaded by LoadLibrary().  Check that EXP_TCLDLL has the correct filename and is the fullpath. ex: "C:\Program Files\Tcl\bin\tcl84.dll"
.

MessageId=0x2
Severity=Fatal
Facility=Stubs
SymbolicName=MSG_STUBS_NOCREATEINTERP
Language=English
%1 : %2 (%3,%4): Tcl API function, Tcl_CreateInterp(), not found in "%5".
.

MessageId=0x3
Severity=Fatal
Facility=Stubs
SymbolicName=MSG_STUBS_ENVARNOTSET
Language=English
%1 : %2 (%3,%4): EXP_TCLDLL was not found in the environment.  It is required for slavedrv.exe to know where the Tcl DLL is to use its services.  The channel driver in the Expect extension should be setting this before launching spawndrv.exe.  This executable was not intended to be used outside of the Expect extension.
.

MessageId=0x4
Severity=Fatal
Facility=Stubs
SymbolicName=MSG_STUBS_INITSTUBS
Language=English
%1 : %2 (%3,%4): Tcl_InitStubs() failed with "%5".
.




MessageId=0x1
Severity=Warning
Facility=MasterSlave_Protocol
SymbolicName=MSG_MS_SLAVENOWRITABLE
Language=English
%1 : %2 (%3,%4): Unable to write to slave: %5
.

MessageId=0x2
Severity=Error
Facility=MasterSlave_Protocol
SymbolicName=MSG_MS_BADSTATE
Language=English
%1 : %2 (%3,%4): Unexpected state
.




MessageId=0x1
Severity=Error
Facility=NamedPipe
SymbolicName=MSG_NP_CANTOPEN
Language=English
%1 : %2 (%3,%4): Can't open argv[1], "%5", for read/write.  CreateFile() returned: %6
.

MessageId=0x2
Severity=Error
Facility=NamedPipe
SymbolicName=MSG_NP_BADTYPE
Language=English
%1 : %2 (%3,%4): NamedPipe specified as, "%5", was found not to be a pipe filetype.
.


MessageId=0x1
Severity=Error
Facility=WinSock
SymbolicName=MSG_WS_CANTSTART
Language=English
%1 : %2 (%3,%4): WinSock system was unable to start.  slavedrv.exe requested version %5.  WSAStartup() returned: %6
.

MessageId=0x2
Severity=Error
Facility=WinSock
SymbolicName=MSG_WS_CANTCREATEMASTERSOCK
Language=English
%1 : %2 (%3,%4): Master socket was unable to be created.  WSASocket() returned: %6
.

MessageId=0x3
Severity=Error
Facility=WinSock
SymbolicName=MSG_WS_CANTCONNECTMASTERSOCK
Language=English
%1 : %2 (%3,%4): Can't connect to local loopback on port %5.  connect() returned: %6
.

MessageId=0x4
Severity=Error
Facility=WinSock
SymbolicName=MSG_WS_PORTOUTOFRANGE
Language=English
%1 : %2 (%3,%4): Local loopback port out-of-range at "%5".  Must be greater than zero and less than 65536.
.




MessageId=0x1
Severity=Error
Facility=DbgTrap
SymbolicName=MSG_DT_CANTGETCONSOLEHANDLE
Language=English
%1 : %2 (%3,%4): Can't open the console handle. CreateFile("%5") returned: %6
.

MessageId=0x2
Severity=Warning
Facility=DbgTrap
SymbolicName=MSG_DT_UNEXPECTEDDBGEVENT
Language=English
%1 : %2 (%3,%4): Unexpected debug event for %5
.

MessageId=0x3
Severity=Warning
Facility=DbgTrap
SymbolicName=MSG_DT_EXCEPTIONDBGEVENT
Language=English
%1 : %2 (%3,%4): Exception code is %5
.

MessageId=0x4
Severity=Warning
Facility=DbgTrap
SymbolicName=MSG_DT_NOVIRT
Language=English
%1 : %2 (%3,%4): Unable to find entry for VirtualAlloc
.

MessageId=0x5
Severity=Warning
Facility=DbgTrap
SymbolicName=MSG_DT_CANTREADSPMEM
Language=English
%1 : %2 (%3,%4): Error reading from subprocess memory
.

MessageId=0x6
Severity=Warning
Facility=DbgTrap
SymbolicName=MSG_DT_CANTWRITESPMEM
Language=English
%1 : %2 (%3,%4): Error writing to subprocess memory
.

MessageId=0x7
Severity=Warning
Facility=DbgTrap
SymbolicName=MSG_DT_SCREENBUF
Language=English
%1 : %2 (%3,%4): Call to GetConsoleScreenBufferInfo() failed with %5, %6
.
