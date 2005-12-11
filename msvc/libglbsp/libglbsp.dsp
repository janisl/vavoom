# Microsoft Developer Studio Project File - Name="libglbsp" - Package Owner=<4>
# Microsoft Developer Studio Generated Build File, Format Version 6.00
# ** DO NOT EDIT **

# TARGTYPE "Win32 (x86) Static Library" 0x0104

CFG=libglbsp - Win32 Debug
!MESSAGE This is not a valid makefile. To build this project using NMAKE,
!MESSAGE use the Export Makefile command and run
!MESSAGE 
!MESSAGE NMAKE /f "libglbsp.mak".
!MESSAGE 
!MESSAGE You can specify a configuration when running NMAKE
!MESSAGE by defining the macro CFG on the command line. For example:
!MESSAGE 
!MESSAGE NMAKE /f "libglbsp.mak" CFG="libglbsp - Win32 Debug"
!MESSAGE 
!MESSAGE Possible choices for configuration are:
!MESSAGE 
!MESSAGE "libglbsp - Win32 Release" (based on "Win32 (x86) Static Library")
!MESSAGE "libglbsp - Win32 Debug" (based on "Win32 (x86) Static Library")
!MESSAGE 

# Begin Project
# PROP AllowPerConfigDependencies 0
# PROP Scc_ProjName ""
# PROP Scc_LocalPath ""
CPP=cl.exe
RSC=rc.exe

!IF  "$(CFG)" == "libglbsp - Win32 Release"

# PROP BASE Use_MFC 0
# PROP BASE Use_Debug_Libraries 0
# PROP BASE Output_Dir "Release"
# PROP BASE Intermediate_Dir "Release"
# PROP BASE Target_Dir ""
# PROP Use_MFC 0
# PROP Use_Debug_Libraries 0
# PROP Output_Dir "Release"
# PROP Intermediate_Dir "Release"
# PROP Target_Dir ""
# ADD BASE CPP /nologo /W3 /GX /O2 /D "WIN32" /D "NDEBUG" /D "_MBCS" /D "_LIB" /YX /FD /c
# ADD CPP /nologo /MT /W3 /GX /O2 /D "WIN32" /D "NDEBUG" /D "_MBCS" /D "_LIB" /YX /FD /c
# ADD BASE RSC /l 0x426 /d "NDEBUG"
# ADD RSC /l 0x426 /d "NDEBUG"
BSC32=bscmake.exe
# ADD BASE BSC32 /nologo
# ADD BSC32 /nologo
LIB32=link.exe -lib
# ADD BASE LIB32 /nologo
# ADD LIB32 /nologo

!ELSEIF  "$(CFG)" == "libglbsp - Win32 Debug"

# PROP BASE Use_MFC 0
# PROP BASE Use_Debug_Libraries 1
# PROP BASE Output_Dir "Debug"
# PROP BASE Intermediate_Dir "Debug"
# PROP BASE Target_Dir ""
# PROP Use_MFC 0
# PROP Use_Debug_Libraries 1
# PROP Output_Dir "Debug"
# PROP Intermediate_Dir "Debug"
# PROP Target_Dir ""
# ADD BASE CPP /nologo /W3 /Gm /GX /ZI /Od /D "WIN32" /D "_DEBUG" /D "_MBCS" /D "_LIB" /YX /FD /GZ /c
# ADD CPP /nologo /MTd /W3 /Gm /GX /ZI /Od /D "WIN32" /D "_DEBUG" /D "_MBCS" /D "_LIB" /YX /FD /GZ /c
# ADD BASE RSC /l 0x426 /d "_DEBUG"
# ADD RSC /l 0x426 /d "_DEBUG"
BSC32=bscmake.exe
# ADD BASE BSC32 /nologo
# ADD BSC32 /nologo
LIB32=link.exe -lib
# ADD BASE LIB32 /nologo
# ADD LIB32 /nologo

!ENDIF 

# Begin Target

# Name "libglbsp - Win32 Release"
# Name "libglbsp - Win32 Debug"
# Begin Group "Source Files"

# PROP Default_Filter "cpp;c;cxx;rc;def;r;odl;idl;hpj;bat"
# Begin Source File

SOURCE=..\..\utils\glbsp\analyze.c
# ADD CPP /W3 /D "GLBSP_PLUGIN"
# End Source File
# Begin Source File

SOURCE=..\..\utils\glbsp\blockmap.c
# ADD CPP /W3 /D "GLBSP_PLUGIN"
# End Source File
# Begin Source File

SOURCE=..\..\utils\glbsp\glbsp.c
# ADD CPP /W3 /D "GLBSP_PLUGIN"
# End Source File
# Begin Source File

SOURCE=..\..\utils\glbsp\level.c
# ADD CPP /W3 /D "GLBSP_PLUGIN"
# End Source File
# Begin Source File

SOURCE=..\..\utils\glbsp\node.c
# ADD CPP /W3 /D "GLBSP_PLUGIN"
# End Source File
# Begin Source File

SOURCE=..\..\utils\glbsp\reject.c
# ADD CPP /W3 /D "GLBSP_PLUGIN"
# End Source File
# Begin Source File

SOURCE=..\..\utils\glbsp\seg.c
# ADD CPP /W3 /D "GLBSP_PLUGIN"
# End Source File
# Begin Source File

SOURCE=..\..\utils\glbsp\system.c
# ADD CPP /W3 /D "GLBSP_PLUGIN"
# End Source File
# Begin Source File

SOURCE=..\..\utils\glbsp\util.c
# ADD CPP /W3 /D "GLBSP_PLUGIN"
# End Source File
# Begin Source File

SOURCE=..\..\utils\glbsp\wad.c
# ADD CPP /W3 /D "GLBSP_PLUGIN"
# End Source File
# End Group
# Begin Group "Header Files"

# PROP Default_Filter "h;hpp;hxx;hm;inl"
# Begin Source File

SOURCE=..\..\utils\glbsp\blockmap.h
# End Source File
# Begin Source File

SOURCE=..\..\utils\glbsp\glbsp.h
# End Source File
# Begin Source File

SOURCE=..\..\utils\glbsp\level.h
# End Source File
# Begin Source File

SOURCE=..\..\utils\glbsp\node.h
# End Source File
# Begin Source File

SOURCE=..\..\utils\glbsp\reject.h
# End Source File
# Begin Source File

SOURCE=..\..\utils\glbsp\seg.h
# End Source File
# Begin Source File

SOURCE=..\..\utils\glbsp\structs.h
# End Source File
# Begin Source File

SOURCE=..\..\utils\glbsp\system.h
# End Source File
# Begin Source File

SOURCE=..\..\utils\glbsp\util.h
# End Source File
# Begin Source File

SOURCE=..\..\utils\glbsp\wad.h
# End Source File
# End Group
# End Target
# End Project
