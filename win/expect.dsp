# Microsoft Developer Studio Project File - Name="expect" - Package Owner=<4>
# Microsoft Developer Studio Generated Build File, Format Version 6.00
# ** DO NOT EDIT **

# TARGTYPE "Win32 (x86) External Target" 0x0106

CFG=expect - Win32 Debug
!MESSAGE This is not a valid makefile. To build this project using NMAKE,
!MESSAGE use the Export Makefile command and run
!MESSAGE 
!MESSAGE NMAKE /f "expect.mak".
!MESSAGE 
!MESSAGE You can specify a configuration when running NMAKE
!MESSAGE by defining the macro CFG on the command line. For example:
!MESSAGE 
!MESSAGE NMAKE /f "expect.mak" CFG="expect - Win32 Debug"
!MESSAGE 
!MESSAGE Possible choices for configuration are:
!MESSAGE 
!MESSAGE "expect - Win32 Release" (based on "Win32 (x86) External Target")
!MESSAGE "expect - Win32 Debug" (based on "Win32 (x86) External Target")
!MESSAGE 

# Begin Project
# PROP AllowPerConfigDependencies 0
# PROP Scc_ProjName ""
# PROP Scc_LocalPath ""

!IF  "$(CFG)" == "expect - Win32 Release"

# PROP BASE Use_MFC 0
# PROP BASE Use_Debug_Libraries 0
# PROP BASE Output_Dir "Release"
# PROP BASE Intermediate_Dir "Release"
# PROP BASE Cmd_Line "NMAKE /f expect.mak"
# PROP BASE Rebuild_Opt "/a"
# PROP BASE Target_File "expect.exe"
# PROP BASE Bsc_Name "expect.bsc"
# PROP BASE Target_Dir ""
# PROP Use_MFC 0
# PROP Use_Debug_Libraries 0
# PROP Output_Dir "Release"
# PROP Intermediate_Dir "Release"
# PROP Cmd_Line "nmake -nologo -f makefile.vc32 expect"
# PROP Rebuild_Opt "-a"
# PROP Target_File "Release\expect532.dll"
# PROP Bsc_Name ""
# PROP Target_Dir ""

!ELSEIF  "$(CFG)" == "expect - Win32 Debug"

# PROP BASE Use_MFC 0
# PROP BASE Use_Debug_Libraries 1
# PROP BASE Output_Dir "Debug"
# PROP BASE Intermediate_Dir "Debug"
# PROP BASE Cmd_Line "NMAKE /f expect.mak"
# PROP BASE Rebuild_Opt "/a"
# PROP BASE Target_File "expect.exe"
# PROP BASE Bsc_Name "expect.bsc"
# PROP BASE Target_Dir ""
# PROP Use_MFC 0
# PROP Use_Debug_Libraries 1
# PROP Output_Dir "Debug"
# PROP Intermediate_Dir "Debug"
# PROP Cmd_Line "nmake -nologo -f makefile.vc32 expect DEBUG=1"
# PROP Rebuild_Opt "-a"
# PROP Target_File "Debug\expect532d.dll"
# PROP Bsc_Name ""
# PROP Target_Dir ""

!ENDIF 

# Begin Target

# Name "expect - Win32 Release"
# Name "expect - Win32 Debug"

!IF  "$(CFG)" == "expect - Win32 Release"

!ELSEIF  "$(CFG)" == "expect - Win32 Debug"

!ENDIF 

# Begin Group "compat"

# PROP Default_Filter ""
# Begin Source File

SOURCE=..\compat\exp_memmove.c
# End Source File
# Begin Source File

SOURCE=..\compat\exp_select.c
# End Source File
# Begin Source File

SOURCE=..\compat\exp_strf.c
# End Source File
# End Group
# Begin Group "generic"

# PROP Default_Filter ""
# Begin Group "headers"

# PROP Default_Filter "h"
# Begin Source File

SOURCE=..\generic\exp.h
# End Source File
# Begin Source File

SOURCE=..\generic\exp_command.h
# End Source File
# Begin Source File

SOURCE=..\generic\exp_log.h
# End Source File
# Begin Source File

SOURCE=..\generic\exp_printify.h
# End Source File
# Begin Source File

SOURCE=..\generic\expDecls.h
# End Source File
# Begin Source File

SOURCE=..\generic\expect.h
# End Source File
# Begin Source File

SOURCE=..\generic\expect_comm.h
# End Source File
# Begin Source File

SOURCE=..\generic\expect_tcl.h
# End Source File
# Begin Source File

SOURCE=..\generic\expInt.h
# End Source File
# Begin Source File

SOURCE=..\generic\expIntDecls.h
# End Source File
# Begin Source File

SOURCE=..\generic\expPlatDecls.h
# End Source File
# Begin Source File

SOURCE=..\generic\getopt.h
# End Source File
# End Group
# Begin Source File

SOURCE=..\generic\exp.decls
# End Source File
# Begin Source File

SOURCE=..\generic\exp_clib.c
# End Source File
# Begin Source File

SOURCE=..\generic\exp_closetcl.c
# End Source File
# Begin Source File

SOURCE=..\generic\exp_command.c
# End Source File
# Begin Source File

SOURCE=..\generic\exp_glob.c
# End Source File
# Begin Source File

SOURCE=..\generic\exp_log.c
# End Source File
# Begin Source File

SOURCE=..\generic\exp_main_sub.c
# End Source File
# Begin Source File

SOURCE=..\generic\exp_port.h
# End Source File
# Begin Source File

SOURCE=..\generic\exp_printify.c
# End Source File
# Begin Source File

SOURCE=..\generic\expChan.c
# End Source File
# Begin Source File

SOURCE=..\generic\expCommand.c
# End Source File
# Begin Source File

SOURCE=..\generic\expect.c
# End Source File
# Begin Source File

SOURCE=..\generic\expSpawnChan.c
# End Source File
# Begin Source File

SOURCE=..\generic\expStubInit.c
# End Source File
# Begin Source File

SOURCE=..\generic\expTrap.c
# End Source File
# Begin Source File

SOURCE=..\generic\getopt.c
# End Source File
# End Group
# Begin Group "win"

# PROP Default_Filter ""
# Begin Group "win-headers"

# PROP Default_Filter ""
# Begin Source File

SOURCE=.\expWin.h
# End Source File
# Begin Source File

SOURCE=.\expWinPort.h
# End Source File
# Begin Source File

SOURCE=.\expWinTty.h
# End Source File
# End Group
# Begin Source File

SOURCE=.\expWinCLib.c
# End Source File
# Begin Source File

SOURCE=.\expWinCommand.c
# End Source File
# Begin Source File

SOURCE=.\expWinLog.c
# End Source File
# Begin Source File

SOURCE=.\expWinSpawnChan.c
# End Source File
# Begin Source File

SOURCE=.\expWinTrap.c
# End Source File
# Begin Source File

SOURCE=.\expWinTty.c
# End Source File
# End Group
# End Target
# End Project
