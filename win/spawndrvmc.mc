;========================================================================
;  spawndrv.mc --
;
;	This file contains the message catalog for use with error
;	reporting.
;
;========================================================================
; RCS: @(#) $Id: spawndrv.mc,v 1.1.2.1 2001/10/12 00:43:44 davygrvy Exp $
;========================================================================

MessageIdTypedef=DWORD

SeverityNames=(Success=0x0:STATUS_SEVERITY_SUCCESS
               Informational=0x1:STATUS_SEVERITY_INFORMATIONAL
               Warning=0x2:STATUS_SEVERITY_WARNING
               Error=0x3:STATUS_SEVERITY_ERROR
              )

FacilityNames=(System=0x0:FACILITY_SYSTEM
               Runtime=0x2:FACILITY_RUNTIME
               Stubs=0x3:FACILITY_STUBS
               Io=0x4:FACILITY_IO_ERROR_CODE
              )

LanguageNames=(English=0x409:MSG00409)
LanguageNames=(Japanese=0x411:MSG00411)


// Message definitions


MessageId=0x1
Severity=Error
Facility=Runtime
SymbolicName=MSG_BAD_COMMAND
Language=English
You have chosen an incorrect command.
.
Language=Japanese
·s³‚ÈƒRƒ}ƒ"ƒh‚ª'I'ğ‚³‚ê‚Ü‚µ‚½.
.

MessageId=0x2
Severity=Warning
Facility=Io
SymbolicName=MSG_BAD_PARM1
Language=English
Cannot reconnect to the server.
.
Language=Japanese
ƒT[ƒo[‚ÖÄÚ'±‚Å‚«‚Ü‚¹‚ñ.
.

MessageId=0x3
Severity=Success
Facility=System
SymbolicName=MSG_STRIKE_ANY_KEY
Language=English
Press any key to continue . . . %0
.
Language=Japanese
'±‚¯‚é‚É‚Í‰½‚©ƒL[‚ğ‰Ÿ‚µ‚Ä‚­‚¾‚³‚¢ . . . %0
.

MessageId=0x4
Severity=Error
Facility=System
SymbolicName=MSG_CMD_DELETE
Language=English
File %1 contains %2 which is in error
.
Language=Japanese
ƒtƒ@ƒCƒ‹ %1 '†‚ÉŠÜ‚Ü‚ê‚é %2 ‚ÍƒGƒ‰[‚Å‚·
.

MessageId=0x5
Severity=Success
Facility=System
SymbolicName=MSG_RETRYS
Language=English
There have been %1!d! retrys with %2!d!%% success%! Disconnect from% 
the server and retry later.
.
Language=Japanese
%1!d! ‰ñÚ'±‚ğ‚İ‚Ü‚µ‚½‚ª %2!d!%% ‚Ì¬Œ÷—‚Å‚µ‚½%!.
ƒT[ƒo[‚ÆØ'f‚µ‚½ŒãAÄÚ'±‚µ‚Ä‚­‚¾‚³‚¢.
.
