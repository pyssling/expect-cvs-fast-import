# Microsoft Developer Studio Project File - Name="expect" - Package Owner=<4>
# Microsoft Developer Studio Generated Build File, Format Version 6.00
# ** DO NOT EDIT **

# TARGTYPE "Win32 (x86) Dynamic-Link Library" 0x0102

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
!MESSAGE "expect - Win32 Release" (based on "Win32 (x86) Dynamic-Link Library")
!MESSAGE "expect - Win32 Debug" (based on "Win32 (x86) Dynamic-Link Library")
!MESSAGE 

# Begin Project
# PROP AllowPerConfigDependencies 0
# PROP Scc_ProjName ""
# PROP Scc_LocalPath ""
CPP=cl.exe
MTL=midl.exe
RSC=rc.exe

!IF  "$(CFG)" == "expect - Win32 Release"

# PROP BASE Use_MFC 0
# PROP BASE Use_Debug_Libraries 0
# PROP BASE Output_Dir "Release"
# PROP BASE Intermediate_Dir "Release"
# PROP BASE Target_Dir ""
# PROP Use_MFC 0
# PROP Use_Debug_Libraries 0
# PROP Output_Dir "Release"
# PROP Intermediate_Dir "Release"
# PROP Ignore_Export_Lib 0
# PROP Target_Dir ""
# ADD BASE CPP /nologo /MT /W3 /GX /O2 /D "WIN32" /D "NDEBUG" /D "_WINDOWS" /D "_MBCS" /D "_USRDLL" /D "EXPECT_EXPORTS" /YX /FD /c
# ADD CPP /nologo /MD /W3 /GX /O2 /I "." /I "..\generic" /I "d:\tcl_workspace\tcl_head\generic" /I "d:\tcl_workspace\tcl_head\win" /D "NDEBUG" /D "WIN32" /D "_WINDOWS" /D TCL_THREADS=1 /D _WIN32_WINNT=0x0400 /YX /FD /c
# ADD BASE MTL /nologo /D "NDEBUG" /mktyplib203 /win32
# ADD MTL /nologo /D "NDEBUG" /mktyplib203 /win32
# ADD BASE RSC /l 0x409 /d "NDEBUG"
# ADD RSC /l 0x409 /d "NDEBUG"
BSC32=bscmake.exe
# ADD BASE BSC32 /nologo
# ADD BSC32 /nologo
LINK32=link.exe
# ADD BASE LINK32 kernel32.lib user32.lib gdi32.lib winspool.lib comdlg32.lib advapi32.lib shell32.lib ole32.lib oleaut32.lib uuid.lib odbc32.lib odbccp32.lib /nologo /dll /machine:I386
# ADD LINK32 tcl84.lib advapi32.lib user32.lib /nologo /dll /machine:I386 /out:"Release/expect52.dll" /libpath:"d:\tcl_workspace\tcl_head\win\Release"

!ELSEIF  "$(CFG)" == "expect - Win32 Debug"

# PROP BASE Use_MFC 0
# PROP BASE Use_Debug_Libraries 1
# PROP BASE Output_Dir "Debug"
# PROP BASE Intermediate_Dir "Debug"
# PROP BASE Target_Dir ""
# PROP Use_MFC 0
# PROP Use_Debug_Libraries 1
# PROP Output_Dir "Debug"
# PROP Intermediate_Dir "Debug"
# PROP Ignore_Export_Lib 0
# PROP Target_Dir ""
# ADD BASE CPP /nologo /MTd /W3 /Gm /GX /ZI /Od /D "WIN32" /D "_DEBUG" /D "_WINDOWS" /D "_MBCS" /D "_USRDLL" /D "EXPECT_EXPORTS" /YX /FD /GZ /c
# ADD CPP /nologo /G5 /MDd /W3 /Gm /GX /ZI /Od /I "." /I "..\generic" /I "d:\tcl_workspace\tcl_head\generic" /I "d:\tcl_workspace\tcl_head\win" /D "_DEBUG" /D "WIN32" /D "_WINDOWS" /D TCL_THREADS=1 /D _WIN32_WINNT=0x0400 /FR /YX /FD /GZ /c
# ADD BASE MTL /nologo /D "_DEBUG" /mktyplib203 /win32
# ADD MTL /nologo /D "_DEBUG" /mktyplib203 /win32
# ADD BASE RSC /l 0x409 /d "_DEBUG"
# ADD RSC /l 0x409 /d "_DEBUG"
BSC32=bscmake.exe
# ADD BASE BSC32 /nologo
# ADD BSC32 /nologo
LINK32=link.exe
# ADD BASE LINK32 kernel32.lib user32.lib gdi32.lib winspool.lib comdlg32.lib advapi32.lib shell32.lib ole32.lib oleaut32.lib uuid.lib odbc32.lib odbccp32.lib /nologo /dll /debug /machine:I386 /pdbtype:sept
# ADD LINK32 tcl84d.lib advapi32.lib user32.lib /nologo /dll /debug /machine:I386 /out:"Debug/expect52d.dll" /pdbtype:sept /libpath:"d:\tcl_workspace\tcl_head\win\Debug"

!ENDIF 

# Begin Target

# Name "expect - Win32 Release"
# Name "expect - Win32 Debug"
# Begin Group "generic"

# PROP Default_Filter ""
# Begin Source File

SOURCE=..\generic\exp_closetcl.c
# End Source File
# Begin Source File

SOURCE=..\generic\exp_event.c
# End Source File
# Begin Source File

SOURCE=..\generic\exp_glob.c
# End Source File
# Begin Source File

SOURCE=..\generic\exp_inter.c
# End Source File
# Begin Source File

SOURCE=..\generic\exp_log.c
# End Source File
# Begin Source File

SOURCE=..\generic\exp_main_sub.c
# End Source File
# Begin Source File

SOURCE=..\generic\exp_printify.c
# End Source File
# Begin Source File

SOURCE=..\generic\exp_strf.c
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

SOURCE=..\generic\expTrap.c
# End Source File
# Begin Source File

SOURCE=..\generic\getopt.c
# End Source File
# End Group
# Begin Group "win"

# PROP Default_Filter ""
# Begin Source File

SOURCE=.\expect.rc
# PROP Exclude_From_Build 1
# End Source File
# Begin Source File

SOURCE=.\expWinCommand.c
# End Source File
# Begin Source File

SOURCE=.\expWinLog.c
# End Source File
# Begin Source File

SOURCE=.\expWinProcess.c
# End Source File
# Begin Source File

SOURCE=.\expWinSpawnChan.c
# End Source File
# Begin Source File

SOURCE=.\expWinTty.c
# End Source File
# End Group
# End Target
# End Project