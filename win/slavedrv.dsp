# Microsoft Developer Studio Project File - Name="slavedrv" - Package Owner=<4>
# Microsoft Developer Studio Generated Build File, Format Version 6.00
# ** DO NOT EDIT **

# TARGTYPE "Win32 (x86) External Target" 0x0106

CFG=slavedrv - Win32 Debug
!MESSAGE This is not a valid makefile. To build this project using NMAKE,
!MESSAGE use the Export Makefile command and run
!MESSAGE 
!MESSAGE NMAKE /f "slavedrv.mak".
!MESSAGE 
!MESSAGE You can specify a configuration when running NMAKE
!MESSAGE by defining the macro CFG on the command line. For example:
!MESSAGE 
!MESSAGE NMAKE /f "slavedrv.mak" CFG="slavedrv - Win32 Debug"
!MESSAGE 
!MESSAGE Possible choices for configuration are:
!MESSAGE 
!MESSAGE "slavedrv - Win32 Release" (based on "Win32 (x86) External Target")
!MESSAGE "slavedrv - Win32 Debug" (based on "Win32 (x86) External Target")
!MESSAGE 

# Begin Project
# PROP AllowPerConfigDependencies 0
# PROP Scc_ProjName ""
# PROP Scc_LocalPath ""

!IF  "$(CFG)" == "slavedrv - Win32 Release"

# PROP BASE Use_MFC 0
# PROP BASE Use_Debug_Libraries 0
# PROP BASE Output_Dir ""
# PROP BASE Intermediate_Dir ""
# PROP BASE Cmd_Line "NMAKE /f slavedrv.mak"
# PROP BASE Rebuild_Opt "/a"
# PROP BASE Target_File "slavedrv.exe"
# PROP BASE Bsc_Name "slavedrv.bsc"
# PROP BASE Target_Dir ""
# PROP Use_MFC 0
# PROP Use_Debug_Libraries 0
# PROP Output_Dir ""
# PROP Intermediate_Dir ""
# PROP Cmd_Line "nmake -nologo -f makefile.vc32 slavedrv"
# PROP Rebuild_Opt "/a"
# PROP Target_File "Release/slavedrv.exe"
# PROP Bsc_Name ""
# PROP Target_Dir ""

!ELSEIF  "$(CFG)" == "slavedrv - Win32 Debug"

# PROP BASE Use_MFC 0
# PROP BASE Use_Debug_Libraries 1
# PROP BASE Output_Dir ""
# PROP BASE Intermediate_Dir ""
# PROP BASE Cmd_Line "NMAKE /f slavedrv.mak"
# PROP BASE Rebuild_Opt "/a"
# PROP BASE Target_File "slavedrv.exe"
# PROP BASE Bsc_Name "slavedrv.bsc"
# PROP BASE Target_Dir ""
# PROP Use_MFC 0
# PROP Use_Debug_Libraries 1
# PROP Output_Dir ""
# PROP Intermediate_Dir ""
# PROP Cmd_Line "nmake -nologo -f makefile.vc32 slavedrv DEBUG=1"
# PROP Rebuild_Opt "/a"
# PROP Target_File "Debug/slavedrv.exe"
# PROP Bsc_Name ""
# PROP Target_Dir ""

!ENDIF 

# Begin Target

# Name "slavedrv - Win32 Release"
# Name "slavedrv - Win32 Debug"

!IF  "$(CFG)" == "slavedrv - Win32 Release"

!ELSEIF  "$(CFG)" == "slavedrv - Win32 Debug"

!ENDIF 

# Begin Group "Resource Files"

# PROP Default_Filter ""
# Begin Source File

SOURCE=.\spawndrv.rc
# End Source File
# Begin Source File

SOURCE=.\spawndrvmc.mc
# End Source File
# End Group
# Begin Source File

SOURCE=.\expWin.h
# End Source File
# Begin Source File

SOURCE=.\expWinDynloadTclStubs.c
# End Source File
# Begin Source File

SOURCE=.\expWinLog.c
# End Source File
# Begin Source File

SOURCE=.\expWinProcess.c
# End Source File
# Begin Source File

SOURCE=.\expWinSlave.h
# End Source File
# Begin Source File

SOURCE=.\expWinSlaveDbg.c
# End Source File
# Begin Source File

SOURCE=.\expWinSlaveDrv.c
# End Source File
# Begin Source File

SOURCE=.\expWinSlaveKey.c
# End Source File
# End Target
# End Project
