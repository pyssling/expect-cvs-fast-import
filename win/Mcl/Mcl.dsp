# Microsoft Developer Studio Project File - Name="Mcl" - Package Owner=<4>
# Microsoft Developer Studio Generated Build File, Format Version 6.00
# ** DO NOT EDIT **

# TARGTYPE "Win32 (x86) Static Library" 0x0104

CFG=Mcl - Win32 Debug Static
!MESSAGE This is not a valid makefile. To build this project using NMAKE,
!MESSAGE use the Export Makefile command and run
!MESSAGE 
!MESSAGE NMAKE /f "Mcl.mak".
!MESSAGE 
!MESSAGE You can specify a configuration when running NMAKE
!MESSAGE by defining the macro CFG on the command line. For example:
!MESSAGE 
!MESSAGE NMAKE /f "Mcl.mak" CFG="Mcl - Win32 Debug Static"
!MESSAGE 
!MESSAGE Possible choices for configuration are:
!MESSAGE 
!MESSAGE "Mcl - Win32 Release" (based on "Win32 (x86) Static Library")
!MESSAGE "Mcl - Win32 Debug" (based on "Win32 (x86) Static Library")
!MESSAGE "Mcl - Win32 Debug Unicode" (based on "Win32 (x86) Static Library")
!MESSAGE "Mcl - Win32 Release Unicode" (based on "Win32 (x86) Static Library")
!MESSAGE "Mcl - Win32 Release Static" (based on "Win32 (x86) Static Library")
!MESSAGE "Mcl - Win32 Debug Static" (based on "Win32 (x86) Static Library")
!MESSAGE 

# Begin Project
# PROP AllowPerConfigDependencies 0
# PROP Scc_ProjName ""
# PROP Scc_LocalPath ""
CPP=cl.exe
RSC=rc.exe

!IF  "$(CFG)" == "Mcl - Win32 Release"

# PROP BASE Use_MFC 0
# PROP BASE Use_Debug_Libraries 0
# PROP BASE Output_Dir ".\lib"
# PROP BASE Intermediate_Dir ".\ReleaseA"
# PROP BASE Target_Dir ""
# PROP Use_MFC 0
# PROP Use_Debug_Libraries 0
# PROP Output_Dir ".\lib"
# PROP Intermediate_Dir ".\ReleaseA"
# PROP Target_Dir ""
# ADD BASE CPP /nologo /MD /W3 /GX /O2 /I ".\include" /D "WIN32" /D "NDEBUG" /D "_WINDOWS" /D "_MBCS" /YX /FD /c
# ADD CPP /nologo /MD /W3 /GX- /O2 /I ".\include" /D "NDEBUG" /D "_MBCS" /D __CMCL_THROW_EXCEPTIONS__=0 /D "WIN32" /D "_WINDOWS" /YX /FD /c
# ADD BASE RSC /l 0x409
# ADD RSC /l 0x409
BSC32=bscmake.exe
# ADD BASE BSC32 /nologo
# ADD BSC32 /nologo
LIB32=link.exe -lib
# ADD BASE LIB32 /nologo/out:".\lib\mcl.lib"
# ADD LIB32 /nologo

!ELSEIF  "$(CFG)" == "Mcl - Win32 Debug"

# PROP BASE Use_MFC 0
# PROP BASE Use_Debug_Libraries 1
# PROP BASE Output_Dir ".\lib"
# PROP BASE Intermediate_Dir ".\DebugA"
# PROP BASE Target_Dir ""
# PROP Use_MFC 0
# PROP Use_Debug_Libraries 1
# PROP Output_Dir ".\lib"
# PROP Intermediate_Dir ".\DebugA"
# PROP Target_Dir ""
# ADD BASE CPP /nologo /MDd /W3 /GX /Z7 /Od /I ".\include" /D "WIN32" /D "_DEBUG" /D "_WINDOWS" /D "_MBCS" /YX /FD /c
# ADD CPP /nologo /MDd /W3 /GX- /Z7 /Od /I ".\include" /D "_DEBUG" /D "_MBCS" /D __CMCL_THROW_EXCEPTIONS__=0 /D "WIN32" /D "_WINDOWS" /YX /FD /c
# SUBTRACT CPP /Fr
# ADD BASE RSC /l 0x409
# ADD RSC /l 0x409
BSC32=bscmake.exe
# ADD BASE BSC32 /nologo
# ADD BSC32 /nologo
LIB32=link.exe -lib
# ADD BASE LIB32 /nologo /out:".\lib\mcld.lib"
# ADD LIB32 /nologo /out:".\lib\mcld.lib"

!ELSEIF  "$(CFG)" == "Mcl - Win32 Debug Unicode"

# PROP BASE Use_MFC 0
# PROP BASE Use_Debug_Libraries 1
# PROP BASE Output_Dir ".\lib"
# PROP BASE Intermediate_Dir ".\DebugU"
# PROP BASE Target_Dir ""
# PROP Use_MFC 0
# PROP Use_Debug_Libraries 1
# PROP Output_Dir ".\lib"
# PROP Intermediate_Dir ".\DebugU"
# PROP Target_Dir ""
# ADD BASE CPP /nologo /MDd /W3 /GX /Z7 /Od /I ".\include" /D "WIN32" /D "_DEBUG" /D "_WINDOWS" /D "UNICODE" /D "_UNICODE" /YX /FD /c
# ADD CPP /nologo /MDd /W3 /GX- /Z7 /Od /I ".\include" /D "_DEBUG" /D "UNICODE" /D "_UNICODE" /D "WIN32" /D "_WINDOWS" /D __CMCL_THROW_EXCEPTIONS__=0 /YX /FD /c
# ADD BASE RSC /l 0x409
# ADD RSC /l 0x409
BSC32=bscmake.exe
# ADD BASE BSC32 /nologo
# ADD BSC32 /nologo
LIB32=link.exe -lib
# ADD BASE LIB32 /nologo /out:".\lib\mclud.lib"
# ADD LIB32 /nologo /out:".\lib\mclud.lib"

!ELSEIF  "$(CFG)" == "Mcl - Win32 Release Unicode"

# PROP BASE Use_MFC 0
# PROP BASE Use_Debug_Libraries 0
# PROP BASE Output_Dir ".\lib"
# PROP BASE Intermediate_Dir ".\ReleaseU"
# PROP BASE Target_Dir ""
# PROP Use_MFC 0
# PROP Use_Debug_Libraries 0
# PROP Output_Dir ".\lib"
# PROP Intermediate_Dir ".\ReleaseU"
# PROP Target_Dir ""
# ADD BASE CPP /nologo /MD /W3 /GX /O2 /I ".\include" /D "WIN32" /D "NDEBUG" /D "_WINDOWS" /D "UNICODE" /D "_UNICODE" /YX /FD /c
# ADD CPP /nologo /MD /W3 /GX- /O2 /I ".\include" /D "NDEBUG" /D "UNICODE" /D "_UNICODE" /D "WIN32" /D "_WINDOWS" /D __CMCL_THROW_EXCEPTIONS__=0 /YX /FD /c
# ADD BASE RSC /l 0x409
# ADD RSC /l 0x409
BSC32=bscmake.exe
# ADD BASE BSC32 /nologo
# ADD BSC32 /nologo
LIB32=link.exe -lib
# ADD BASE LIB32 /nologo /out:".\lib\mclu.lib"
# ADD LIB32 /nologo /out:".\lib\mclu.lib"

!ELSEIF  "$(CFG)" == "Mcl - Win32 Release Static"

# PROP BASE Use_MFC 0
# PROP BASE Use_Debug_Libraries 0
# PROP BASE Output_Dir "Mcl___Win32_Release_Static"
# PROP BASE Intermediate_Dir "Mcl___Win32_Release_Static"
# PROP BASE Target_Dir ""
# PROP Use_MFC 0
# PROP Use_Debug_Libraries 0
# PROP Output_Dir ".\lib"
# PROP Intermediate_Dir ".\ReleaseAS"
# PROP Target_Dir ""
# ADD BASE CPP /nologo /MD /W3 /GX /O2 /I ".\include" /D "WIN32" /D "NDEBUG" /D "_WINDOWS" /D "_MBCS" /YX /FD /c
# ADD CPP /nologo /G5 /MT /W3 /GX- /O2 /I ".\include" /D "NDEBUG" /D "_MBCS" /D "WIN32" /D "_WINDOWS" /D __CMCL_THROW_EXCEPTIONS__=0 /YX /FD /c
# ADD BASE RSC /l 0x409
# ADD RSC /l 0x409
BSC32=bscmake.exe
# ADD BASE BSC32 /nologo
# ADD BSC32 /nologo
LIB32=link.exe -lib
# ADD BASE LIB32 /nologo
# ADD LIB32 /nologo /out:".\lib\mcls.lib"

!ELSEIF  "$(CFG)" == "Mcl - Win32 Debug Static"

# PROP BASE Use_MFC 0
# PROP BASE Use_Debug_Libraries 1
# PROP BASE Output_Dir "Mcl___Win32_Debug_Static"
# PROP BASE Intermediate_Dir "Mcl___Win32_Debug_Static"
# PROP BASE Target_Dir ""
# PROP Use_MFC 0
# PROP Use_Debug_Libraries 1
# PROP Output_Dir ".\lib"
# PROP Intermediate_Dir ".\DebugAS"
# PROP Target_Dir ""
# ADD BASE CPP /nologo /MDd /W3 /GX /Z7 /Od /I ".\include" /D "WIN32" /D "_DEBUG" /D "_WINDOWS" /D "_MBCS" /YX /FD /c
# ADD CPP /nologo /G5 /MTd /W3 /GX- /Z7 /Od /I ".\include" /D "_DEBUG" /D "_MBCS" /D "WIN32" /D "_WINDOWS" /D __CMCL_THROW_EXCEPTIONS__=0 /YX /FD /c
# ADD BASE RSC /l 0x409
# ADD RSC /l 0x409
BSC32=bscmake.exe
# ADD BASE BSC32 /nologo
# ADD BSC32 /nologo
LIB32=link.exe -lib
# ADD BASE LIB32 /nologo /out:".\lib\mcld.lib"
# ADD LIB32 /nologo /out:".\lib\mclsd.lib"

!ENDIF 

# Begin Target

# Name "Mcl - Win32 Release"
# Name "Mcl - Win32 Debug"
# Name "Mcl - Win32 Debug Unicode"
# Name "Mcl - Win32 Release Unicode"
# Name "Mcl - Win32 Release Static"
# Name "Mcl - Win32 Debug Static"
# Begin Group "Source Files"

# PROP Default_Filter "cpp;c;cxx;rc;def;r;odl;hpj;bat;for;f90"
# Begin Source File

SOURCE=.\src\CMclAutoLock.cpp
# End Source File
# Begin Source File

SOURCE=.\src\CMclAutoPtr.cpp
# End Source File
# Begin Source File

SOURCE=.\src\CMclCritSec.cpp
# End Source File
# Begin Source File

SOURCE=.\src\CMclEvent.cpp
# End Source File
# Begin Source File

SOURCE=.\src\CMclGlobal.cpp
# End Source File
# Begin Source File

SOURCE=.\src\CMclKernel.cpp
# End Source File
# Begin Source File

SOURCE=.\src\CMclMailbox.cpp
# End Source File
# Begin Source File

SOURCE=.\src\CMclMonitor.cpp
# End Source File
# Begin Source File

SOURCE=.\src\CMclMutex.cpp
# End Source File
# Begin Source File

SOURCE=.\src\CMclSemaphore.cpp
# End Source File
# Begin Source File

SOURCE=.\src\CMclSharedMemory.cpp
# End Source File
# Begin Source File

SOURCE=.\src\CMclThread.cpp
# End Source File
# Begin Source File

SOURCE=.\src\CMclWaitableCollection.cpp
# End Source File
# End Group
# Begin Group "Header Files"

# PROP Default_Filter "h;hpp;hxx;hm;inl;fi;fd"
# Begin Source File

SOURCE=.\include\CMcl.h
# End Source File
# Begin Source File

SOURCE=.\include\CMclAutoLock.h
# End Source File
# Begin Source File

SOURCE=.\include\CMclAutoPtr.h
# End Source File
# Begin Source File

SOURCE=.\include\CMclCritSec.h
# End Source File
# Begin Source File

SOURCE=.\include\CMclEvent.h
# End Source File
# Begin Source File

SOURCE=.\include\CMclGlobal.h
# End Source File
# Begin Source File

SOURCE=.\include\CMclKernel.h
# End Source File
# Begin Source File

SOURCE=.\include\CMclLinkedLists.h
# End Source File
# Begin Source File

SOURCE=.\include\CMclMailbox.h
# End Source File
# Begin Source File

SOURCE=.\include\CMclMonitor.h
# End Source File
# Begin Source File

SOURCE=.\include\CMclMutex.h
# End Source File
# Begin Source File

SOURCE=.\include\CMclSemaphore.h
# End Source File
# Begin Source File

SOURCE=.\include\CMclSharedMemory.h
# End Source File
# Begin Source File

SOURCE=.\include\CMclThread.h
# End Source File
# Begin Source File

SOURCE=.\include\CMclWaitableCollection.h
# End Source File
# Begin Source File

SOURCE=.\include\CMclWaitableObject.h
# End Source File
# End Group
# End Target
# End Project
