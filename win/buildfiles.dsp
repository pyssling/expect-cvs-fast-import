# Microsoft Developer Studio Project File - Name="buildfiles" - Package Owner=<4>
# Microsoft Developer Studio Generated Build File, Format Version 6.00
# ** DO NOT EDIT **

# TARGTYPE "Win32 (x86) Generic Project" 0x010a

CFG=buildfiles - Win32 Debug
!MESSAGE This is not a valid makefile. To build this project using NMAKE,
!MESSAGE use the Export Makefile command and run
!MESSAGE 
!MESSAGE NMAKE /f "buildfiles.mak".
!MESSAGE 
!MESSAGE You can specify a configuration when running NMAKE
!MESSAGE by defining the macro CFG on the command line. For example:
!MESSAGE 
!MESSAGE NMAKE /f "buildfiles.mak" CFG="buildfiles - Win32 Debug"
!MESSAGE 
!MESSAGE Possible choices for configuration are:
!MESSAGE 
!MESSAGE "buildfiles - Win32 Release" (based on "Win32 (x86) Generic Project")
!MESSAGE "buildfiles - Win32 Debug" (based on "Win32 (x86) Generic Project")
!MESSAGE 

# Begin Project
# PROP AllowPerConfigDependencies 0
# PROP Scc_ProjName ""
# PROP Scc_LocalPath ""
MTL=midl.exe

!IF  "$(CFG)" == "buildfiles - Win32 Release"

# PROP BASE Use_MFC 0
# PROP BASE Use_Debug_Libraries 0
# PROP BASE Output_Dir "buildfiles___Win32_Release"
# PROP BASE Intermediate_Dir "buildfiles___Win32_Release"
# PROP BASE Target_Dir ""
# PROP Use_MFC 0
# PROP Use_Debug_Libraries 0
# PROP Output_Dir "buildfiles___Win32_Release"
# PROP Intermediate_Dir "buildfiles___Win32_Release"
# PROP Target_Dir ""

!ELSEIF  "$(CFG)" == "buildfiles - Win32 Debug"

# PROP BASE Use_MFC 0
# PROP BASE Use_Debug_Libraries 1
# PROP BASE Output_Dir "buildfiles___Win32_Debug"
# PROP BASE Intermediate_Dir "buildfiles___Win32_Debug"
# PROP BASE Target_Dir ""
# PROP Use_MFC 0
# PROP Use_Debug_Libraries 1
# PROP Output_Dir "buildfiles___Win32_Debug"
# PROP Intermediate_Dir "buildfiles___Win32_Debug"
# PROP Target_Dir ""

!ENDIF 

# Begin Target

# Name "buildfiles - Win32 Release"
# Name "buildfiles - Win32 Debug"
# Begin Source File

SOURCE=.\makefile.vc32
# End Source File
# Begin Source File

SOURCE=..\makefile.win
# End Source File
# Begin Source File

SOURCE=.\mkbc32.mif
# End Source File
# Begin Source File

SOURCE=..\mkconfig.mif
# End Source File
# Begin Source File

SOURCE=.\mkfiles.mif
# End Source File
# Begin Source File

SOURCE=.\mkmgw32.mif
# End Source File
# Begin Source File

SOURCE=.\mkprepvc32.mif
# End Source File
# Begin Source File

SOURCE=.\mkvc32.mif
# End Source File
# Begin Source File

SOURCE=.\mkwc32.mif
# End Source File
# End Target
# End Project
