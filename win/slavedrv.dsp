# Microsoft Developer Studio Project File - Name="slavedrv" - Package Owner=<4>
# Microsoft Developer Studio Generated Build File, Format Version 6.00
# ** DO NOT EDIT **

# TARGTYPE "Win32 (x86) Console Application" 0x0103

CFG=slavedrv - Win32 Debug Unicode
!MESSAGE This is not a valid makefile. To build this project using NMAKE,
!MESSAGE use the Export Makefile command and run
!MESSAGE 
!MESSAGE NMAKE /f "slavedrv.mak".
!MESSAGE 
!MESSAGE You can specify a configuration when running NMAKE
!MESSAGE by defining the macro CFG on the command line. For example:
!MESSAGE 
!MESSAGE NMAKE /f "slavedrv.mak" CFG="slavedrv - Win32 Debug Unicode"
!MESSAGE 
!MESSAGE Possible choices for configuration are:
!MESSAGE 
!MESSAGE "slavedrv - Win32 Release" (based on "Win32 (x86) Console Application")
!MESSAGE "slavedrv - Win32 Debug" (based on "Win32 (x86) Console Application")
!MESSAGE "slavedrv - Win32 Debug Unicode" (based on "Win32 (x86) Console Application")
!MESSAGE 

# Begin Project
# PROP AllowPerConfigDependencies 0
# PROP Scc_ProjName ""
# PROP Scc_LocalPath ""
CPP=cl.exe
RSC=rc.exe

!IF  "$(CFG)" == "slavedrv - Win32 Release"

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
# ADD BASE CPP /nologo /W3 /GX /O2 /D "WIN32" /D "NDEBUG" /D "_CONSOLE" /D "_MBCS" /YX /FD /c
# ADD CPP /nologo /G5 /MD /W3 /GX /O2 /I "D:\tcl_workspace\tcl_head\generic" /I "..\generic" /I "$(IntDir)" /D "NDEBUG" /D "WIN32" /D "_CONSOLE" /D "USE_TCL_STUBS" /D "TCL_THREADS" /D "BUILD_spawndriver" /D "STATIC_BUILD" /YX"exp.h" /FD /c
# ADD BASE RSC /l 0x409 /d "NDEBUG"
# ADD RSC /l 0x409 /i "D:\tcl_workspace\tcl_head\generic" /i "..\generic" /i "$(INTDIR)" /d "NDEBUG"
BSC32=bscmake.exe
# ADD BASE BSC32 /nologo
# ADD BSC32 /nologo
LINK32=link.exe
# ADD BASE LINK32 kernel32.lib user32.lib gdi32.lib winspool.lib comdlg32.lib advapi32.lib shell32.lib ole32.lib oleaut32.lib uuid.lib odbc32.lib odbccp32.lib kernel32.lib user32.lib gdi32.lib winspool.lib comdlg32.lib advapi32.lib shell32.lib ole32.lib oleaut32.lib uuid.lib odbc32.lib odbccp32.lib /nologo /subsystem:console /machine:I386
# ADD LINK32 kernel32.lib user32.lib gdi32.lib /nologo /machine:I386 /libpath:"D:\tcl_workspace\tcl_head\win\release" /subsystem:console,4.00
# SUBTRACT LINK32 /pdb:none

!ELSEIF  "$(CFG)" == "slavedrv - Win32 Debug"

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
# ADD BASE CPP /nologo /W3 /Gm /GX /ZI /Od /D "WIN32" /D "_DEBUG" /D "_CONSOLE" /D "_MBCS" /YX /FD /GZ /c
# ADD CPP /nologo /G5 /MDd /W3 /Gm /ZI /Od /I "D:\tcl_workspace\tcl_head\generic" /I "..\generic" /I "$(IntDir)" /D "_DEBUG" /D "WIN32" /D "_CONSOLE" /D "BUILD_exp" /D "USE_TCL_STUBS" /D "TCL_THREADS" /D "BUILD_spawndriver" /D "STATIC_BUILD" /YX"exp.h" /FD /GZ /c
# ADD BASE RSC /l 0x409 d "_DEBUG"
# ADD RSC /l 0x409 /i "D:\tcl_workspace\tcl_head\generic" /i "..\generic" /i "$(INTDIR)" /d "_DEBUG"
BSC32=bscmake.exe
# ADD BASE BSC32 /nologo
# ADD BSC32 /nologo
LINK32=link.exe
# ADD BASE LINK32 kernel32.lib user32.lib gdi32.lib winspool.lib comdlg32.lib advapi32.lib shell32.lib ole32.lib oleaut32.lib uuid.lib odbc32.lib odbccp32.lib kernel32.lib user32.lib gdi32.lib winspool.lib comdlg32.lib advapi32.lib shell32.lib ole32.lib oleaut32.lib uuid.lib odbc32.lib odbccp32.lib /nologo /subsystem:console /debug /machine:I386 /pdbtype:sept
# ADD LINK32 kernel32.lib user32.lib gdi32.lib /nologo /debug /machine:I386 /pdbtype:sept /libpath:"D:\tcl_workspace\tcl_head\win\release" /subsystem:console,4.00
# SUBTRACT LINK32 /pdb:none

!ELSEIF  "$(CFG)" == "slavedrv - Win32 Debug Unicode"

# PROP BASE Use_MFC 0
# PROP BASE Use_Debug_Libraries 1
# PROP BASE Output_Dir "DebugU"
# PROP BASE Intermediate_Dir "DebugU"
# PROP BASE Ignore_Export_Lib 0
# PROP BASE Target_Dir ""
# PROP Use_MFC 0
# PROP Use_Debug_Libraries 1
# PROP Output_Dir "DebugU"
# PROP Intermediate_Dir "DebugU"
# PROP Ignore_Export_Lib 0
# PROP Target_Dir ""
# ADD BASE CPP /nologo /G5 /MDd /W3 /Gm /ZI /Od /I "$(TCL_SRCDIR)\generic" /I "..\generic" /I "$(IntDir)" /D "_DEBUG" /D "WIN32" /D "_UNICODE" /D "UNICODE" /D "_CONSOLE" /D "BUILD_exp" /D "USE_TCL_STUBS" /D "TCL_THREADS" /D "BUILD_spawndriver" /D "STATIC_BUILD" /YX"exp.h" /FD /GZ /c
# ADD CPP /nologo /G5 /MDd /W3 /Gm /ZI /Od /I "D:\tcl_workspace\tcl_head\generic" /I "..\generic" /I "$(IntDir)" /D "_DEBUG" /D "WIN32" /D "_UNICODE" /D "UNICODE" /D "_CONSOLE" /D "BUILD_exp" /D "USE_TCL_STUBS" /D "TCL_THREADS" /D "BUILD_spawndriver" /D "STATIC_BUILD" /YX"exp.h" /FD /GZ /c
# ADD BASE RSC /l 0x409 /i "$(TCL_SRCDIR)\generic" /i "..\generic" /i "$(INTDIR)" /d "_DEBUG"
# ADD RSC /l 0x409 /i "D:\tcl_workspace\tcl_head\generic" /i "..\generic" /i "$(INTDIR)" /d "_DEBUG"
BSC32=bscmake.exe
# ADD BASE BSC32 /nologo
# ADD BSC32 /nologo
LINK32=link.exe
# ADD BASE LINK32 kernel32.lib user32.lib gdi32.lib /nologo /debug /machine:I386 /pdbtype:sept /libpath:"d:\tcl_workspace\tcl_head\win\release" /subsystem:console,4.00
# SUBTRACT BASE LINK32 /pdb:none
# ADD LINK32 kernel32.lib user32.lib gdi32.lib /nologo /debug /machine:I386 /pdbtype:sept /libpath:"D:\tcl_workspace\tcl_head\win\release" /subsystem:console,4.00
# SUBTRACT LINK32 /pdb:none

!ENDIF 

# Begin Target

# Name "slavedrv - Win32 Release"
# Name "slavedrv - Win32 Debug"
# Name "slavedrv - Win32 Debug Unicode"
# Begin Group "Source Files"

# PROP Default_Filter "cpp;c;cxx;rc;def;r;odl;idl;hpj;bat"
# Begin Source File

SOURCE=.\expSlaveDrvMain.c
# End Source File
# Begin Source File

SOURCE=.\expWinDynloadTclStubs.c
# End Source File
# Begin Source File

SOURCE=.\expWinInit.c
# End Source File
# Begin Source File

SOURCE=.\expWinLog.c
# End Source File
# Begin Source File

SOURCE=.\expWinMailboxCli.cpp
# End Source File
# Begin Source File

SOURCE=.\expWinProcess.c
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
# End Group
# Begin Group "Header Files"

# PROP Default_Filter "h;hpp;hxx;hm;inl"
# Begin Source File

SOURCE=..\generic\exp.h
# End Source File
# Begin Source File

SOURCE=..\generic\expInt.h
# End Source File
# Begin Source File

SOURCE=..\generic\expIntDecls.h
# End Source File
# Begin Source File

SOURCE=..\generic\expIntPlatDecls.h
# End Source File
# Begin Source File

SOURCE=..\generic\expPlatDecls.h
# End Source File
# Begin Source File

SOURCE=.\expWinInt.h
# End Source File
# Begin Source File

SOURCE=.\expWinPort.h
# End Source File
# Begin Source File

SOURCE=.\expWinSlave.h
# End Source File
# End Group
# Begin Group "Resource Files"

# PROP Default_Filter "ico;cur;bmp;dlg;rc2;rct;bin;rgs;gif;jpg;jpeg;jpe"
# Begin Source File

SOURCE=.\spawndrv.rc
# End Source File
# Begin Source File

SOURCE=.\spawndrvmc.mc

!IF  "$(CFG)" == "slavedrv - Win32 Release"

# Begin Custom Build - Compiling message catalog...
IntDir=.\Release
InputPath=.\spawndrvmc.mc

BuildCmds= \
	mc -w -h "$(IntDir)" -r "$(IntDir)" $(InputPath)

"$(IntDir)\spawndrvmc.rc" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
   $(BuildCmds)

"$(IntDir)\spawndrvmc.h" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
   $(BuildCmds)

"$(IntDir)\MSG00409.bin" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
   $(BuildCmds)
# End Custom Build

!ELSEIF  "$(CFG)" == "slavedrv - Win32 Debug"

# Begin Custom Build - Compiling message catalog...
IntDir=.\Debug
InputPath=.\spawndrvmc.mc

BuildCmds= \
	mc -w -h "$(IntDir)" -r "$(IntDir)" $(InputPath)

"$(IntDir)\spawndrvmc.rc" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
   $(BuildCmds)

"$(IntDir)\spawndrvmc.h" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
   $(BuildCmds)

"$(IntDir)\MSG00409.bin" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
   $(BuildCmds)
# End Custom Build

!ELSEIF  "$(CFG)" == "slavedrv - Win32 Debug Unicode"

# Begin Custom Build - Compiling message catalog...
IntDir=.\DebugU
InputPath=.\spawndrvmc.mc

BuildCmds= \
	mc -w -h "$(IntDir)" -r "$(IntDir)" $(InputPath)

"$(IntDir)\spawndrvmc.rc" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
   $(BuildCmds)

"$(IntDir)\spawndrvmc.h" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
   $(BuildCmds)

"$(IntDir)\MSG00409.bin" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
   $(BuildCmds)
# End Custom Build

!ENDIF 

# End Source File
# End Group
# End Target
# End Project
