# Microsoft Developer Studio Project File - Name="Vavoom95" - Package Owner=<4>
# Microsoft Developer Studio Generated Build File, Format Version 6.00
# ** DO NOT EDIT **

# TARGTYPE "Win32 (x86) Application" 0x0101

CFG=Vavoom95 - Win32 Debug
!MESSAGE This is not a valid makefile. To build this project using NMAKE,
!MESSAGE use the Export Makefile command and run
!MESSAGE 
!MESSAGE NMAKE /f "Vavoom95.mak".
!MESSAGE 
!MESSAGE You can specify a configuration when running NMAKE
!MESSAGE by defining the macro CFG on the command line. For example:
!MESSAGE 
!MESSAGE NMAKE /f "Vavoom95.mak" CFG="Vavoom95 - Win32 Debug"
!MESSAGE 
!MESSAGE Possible choices for configuration are:
!MESSAGE 
!MESSAGE "Vavoom95 - Win32 Release" (based on "Win32 (x86) Application")
!MESSAGE "Vavoom95 - Win32 Debug" (based on "Win32 (x86) Application")
!MESSAGE 

# Begin Project
# PROP AllowPerConfigDependencies 0
# PROP Scc_ProjName ""
# PROP Scc_LocalPath ""
CPP=cl.exe
MTL=midl.exe
RSC=rc.exe

!IF  "$(CFG)" == "Vavoom95 - Win32 Release"

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
# ADD BASE CPP /nologo /W3 /GX /O2 /D "WIN32" /D "NDEBUG" /D "_WINDOWS" /D "_MBCS" /YX /FD /c
# ADD CPP /nologo /MT /W4 /GX /O2 /D "NDEBUG" /D "WIN32" /D "_WINDOWS" /D "_MBCS" /D "FLAC__NO_DLL" /YX /FD /c
# ADD BASE MTL /nologo /D "NDEBUG" /mktyplib203 /win32
# ADD MTL /nologo /D "NDEBUG" /mktyplib203 /win32
# ADD BASE RSC /l 0x426 /d "NDEBUG"
# ADD RSC /l 0x426 /d "NDEBUG"
BSC32=bscmake.exe
# ADD BASE BSC32 /nologo
# ADD BSC32 /nologo
LINK32=link.exe
# ADD BASE LINK32 kernel32.lib user32.lib gdi32.lib winspool.lib comdlg32.lib advapi32.lib shell32.lib ole32.lib oleaut32.lib uuid.lib odbc32.lib odbccp32.lib /nologo /subsystem:windows /machine:I386
# ADD LINK32 kernel32.lib user32.lib gdi32.lib ole32.lib winmm.lib opengl32.lib openal32.lib wsock32.lib libpng.lib zlib.lib libmad.lib vorbis_static.lib ogg_static.lib mikmod.lib libFLAC_static.lib libFLAC++_static.lib /nologo /subsystem:windows /machine:I386 /nodefaultlib:"libc.lib" /nodefaultlib:"msvcrt.lib" /nodefaultlib:"libcd.lib" /nodefaultlib:"libcmtd.lib" /nodefaultlib:"msvcrtd.lib" /libpath:"C:\Gamma\Microsoft Visual C++ Toolkit 2003\lib"
# SUBTRACT LINK32 /pdb:none

!ELSEIF  "$(CFG)" == "Vavoom95 - Win32 Debug"

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
# ADD BASE CPP /nologo /W3 /Gm /GX /ZI /Od /D "WIN32" /D "_DEBUG" /D "_WINDOWS" /D "_MBCS" /YX /FD /GZ /c
# ADD CPP /nologo /MTd /W4 /Gm /GX /ZI /Od /D "_DEBUG" /D "WIN32" /D "_WINDOWS" /D "_MBCS" /D "FLAC__NO_DLL" /YX /FD /GZ /c
# ADD BASE MTL /nologo /D "_DEBUG" /mktyplib203 /win32
# ADD MTL /nologo /D "_DEBUG" /mktyplib203 /win32
# ADD BASE RSC /l 0x426 /d "_DEBUG"
# ADD RSC /l 0x426 /d "_DEBUG"
BSC32=bscmake.exe
# ADD BASE BSC32 /nologo
# ADD BSC32 /nologo
LINK32=link.exe
# ADD BASE LINK32 kernel32.lib user32.lib gdi32.lib winspool.lib comdlg32.lib advapi32.lib shell32.lib ole32.lib oleaut32.lib uuid.lib odbc32.lib odbccp32.lib /nologo /subsystem:windows /debug /machine:I386 /pdbtype:sept
# ADD LINK32 kernel32.lib user32.lib gdi32.lib ole32.lib winmm.lib opengl32.lib openal32.lib wsock32.lib libpng.lib zlib.lib libmad.lib vorbis_static.lib ogg_static.lib mikmod.lib libFLAC_static.lib libFLAC++_static.lib /nologo /subsystem:windows /debug /machine:I386
# SUBTRACT LINK32 /pdb:none

!ENDIF 

# Begin Target

# Name "Vavoom95 - Win32 Release"
# Name "Vavoom95 - Win32 Debug"
# Begin Group "Source Files"

# PROP Default_Filter "cpp;c;cxx;rc;def;r;odl;idl;hpj;bat"
# Begin Source File

SOURCE=..\..\source\cheats.cpp
# ADD CPP /W4
# End Source File
# Begin Source File

SOURCE=..\..\source\cmd.cpp
# ADD CPP /W4
# End Source File
# Begin Source File

SOURCE=..\..\source\crc.cpp
# ADD CPP /W4
# End Source File
# Begin Source File

SOURCE=..\..\source\cvar.cpp
# ADD CPP /W4
# End Source File
# Begin Source File

SOURCE=..\..\source\debug.cpp
# ADD CPP /W4
# End Source File
# Begin Source File

SOURCE=..\..\source\files.cpp
# ADD CPP /W4
# End Source File
# Begin Source File

SOURCE=..\..\source\host.cpp
# ADD CPP /W4
# End Source File
# Begin Source File

SOURCE=..\..\source\infostr.cpp
# ADD CPP /W4
# End Source File
# Begin Source File

SOURCE=..\..\source\level.cpp
# End Source File
# Begin Source File

SOURCE=..\..\source\mapinfo.cpp
# ADD CPP /W4
# End Source File
# Begin Source File

SOURCE=..\..\source\maths.cpp
# ADD CPP /W4
# End Source File
# Begin Source File

SOURCE=..\..\source\message.cpp
# ADD CPP /W4
# End Source File
# Begin Source File

SOURCE=..\..\source\misc.cpp
# ADD CPP /W4
# End Source File
# Begin Source File

SOURCE=..\..\source\name.cpp
# ADD CPP /W4
# End Source File
# Begin Source File

SOURCE=..\..\source\p_setup.cpp
# ADD CPP /W4
# End Source File
# Begin Source File

SOURCE=..\..\source\pr_cmds.cpp
# ADD CPP /W4
# End Source File
# Begin Source File

SOURCE=..\..\source\pr_exec.cpp
# ADD CPP /W4
# End Source File
# Begin Source File

SOURCE=..\..\source\sc_man.cpp
# ADD CPP /W4
# End Source File
# Begin Source File

SOURCE=..\..\source\sizebuf.cpp
# ADD CPP /W4
# End Source File
# Begin Source File

SOURCE=..\..\source\template.cpp
# PROP Exclude_From_Build 1
# End Source File
# Begin Source File

SOURCE=..\..\source\vclass.cpp
# ADD CPP /W4
# End Source File
# Begin Source File

SOURCE=..\..\source\vobject.cpp
# ADD CPP /W4
# End Source File
# Begin Source File

SOURCE=..\..\source\wad.cpp
# ADD CPP /W4
# End Source File
# Begin Source File

SOURCE=..\..\source\zone.cpp
# ADD CPP /W4
# End Source File
# End Group
# Begin Group "Header Files"

# PROP Default_Filter "h;hpp;hxx;hm;inl"
# Begin Source File

SOURCE=..\..\source\archive.h
# End Source File
# Begin Source File

SOURCE=..\..\source\array.h
# End Source File
# Begin Source File

SOURCE=..\..\source\asm_i386.h
# End Source File
# Begin Source File

SOURCE=..\..\source\build.h
# End Source File
# Begin Source File

SOURCE=..\..\source\cmd.h
# End Source File
# Begin Source File

SOURCE=..\..\source\common.h
# End Source File
# Begin Source File

SOURCE=..\..\source\crc.h
# End Source File
# Begin Source File

SOURCE=..\..\source\cvar.h
# End Source File
# Begin Source File

SOURCE=..\..\source\debug.h
# End Source File
# Begin Source File

SOURCE=..\..\source\files.h
# End Source File
# Begin Source File

SOURCE=..\..\source\fmapdefs.h
# End Source File
# Begin Source File

SOURCE=..\..\source\fwaddefs.h
# End Source File
# Begin Source File

SOURCE=..\..\source\gamedefs.h
# End Source File
# Begin Source File

SOURCE=..\..\source\host.h
# End Source File
# Begin Source File

SOURCE=..\..\source\infostr.h
# End Source File
# Begin Source File

SOURCE=..\..\source\level.h
# End Source File
# Begin Source File

SOURCE=..\..\source\mapinfo.h
# End Source File
# Begin Source File

SOURCE=..\..\source\maths.h
# End Source File
# Begin Source File

SOURCE=..\..\source\message.h
# End Source File
# Begin Source File

SOURCE=..\..\source\misc.h
# End Source File
# Begin Source File

SOURCE=..\..\source\name.h
# End Source File
# Begin Source File

SOURCE=..\..\source\names.h
# End Source File
# Begin Source File

SOURCE=..\..\source\progdefs.h
# End Source File
# Begin Source File

SOURCE=..\..\source\progs.h
# End Source File
# Begin Source File

SOURCE=..\..\source\protocol.h
# End Source File
# Begin Source File

SOURCE=..\..\source\save.h
# End Source File
# Begin Source File

SOURCE=..\..\source\scripts.h
# End Source File
# Begin Source File

SOURCE=..\..\source\sizebuf.h
# End Source File
# Begin Source File

SOURCE=..\..\source\template.h
# End Source File
# Begin Source File

SOURCE=..\..\source\vclass.h
# End Source File
# Begin Source File

SOURCE=..\..\source\vector.h
# End Source File
# Begin Source File

SOURCE=..\..\source\vobject.h
# End Source File
# Begin Source File

SOURCE=..\..\source\wad.h
# End Source File
# Begin Source File

SOURCE=..\..\source\waddefs.h
# End Source File
# Begin Source File

SOURCE=..\..\source\zone.h
# End Source File
# End Group
# Begin Group "Resource Files"

# PROP Default_Filter "ico;cur;bmp;dlg;rc2;rct;bin;rgs;gif;jpg;jpeg;jpe"
# Begin Source File

SOURCE=..\..\source\vavoom.ico
# End Source File
# Begin Source File

SOURCE=..\..\source\vavoom.rc
# End Source File
# Begin Source File

SOURCE=..\..\source\vavoom_2.ico
# End Source File
# Begin Source File

SOURCE=..\..\source\vavoom_3.ico
# End Source File
# End Group
# Begin Group "Assembler Files"

# PROP Default_Filter "s"
# Begin Source File

SOURCE=..\..\source\pr_execa.s

!IF  "$(CFG)" == "Vavoom95 - Win32 Release"

USERDEP__PR_EX="../../source/asm_i386.h"	
# Begin Custom Build - Assembling $(InputPath)...
IntDir=.\Release
InputPath=..\..\source\pr_execa.s
InputName=pr_execa

"$(IntDir)\$(InputName).obj" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
	gcc -c -x assembler-with-cpp $(InputPath) -o $(IntDir)\$(InputName).obj

# End Custom Build

!ELSEIF  "$(CFG)" == "Vavoom95 - Win32 Debug"

USERDEP__PR_EX="../../source/asm_i386.h"	
# Begin Custom Build - Assembling $(InputPath)...
IntDir=.\Debug
InputPath=..\..\source\pr_execa.s
InputName=pr_execa

"$(IntDir)\$(InputName).obj" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
	gcc -c -x assembler-with-cpp $(InputPath) -o $(IntDir)\$(InputName).obj

# End Custom Build

!ENDIF 

# End Source File
# Begin Source File

SOURCE=..\..\source\template.s
# End Source File
# End Group
# Begin Group "Makefiles"

# PROP Default_Filter ""
# Begin Source File

SOURCE=..\..\Makefile
# End Source File
# Begin Source File

SOURCE=..\..\vavm95sv.mak
# End Source File
# Begin Source File

SOURCE=..\..\Vavoom95.mak
# End Source File
# End Group
# Begin Group "Rendering"

# PROP Default_Filter ""
# Begin Group "Software"

# PROP Default_Filter ""
# Begin Source File

SOURCE=..\..\source\d16_part.s

!IF  "$(CFG)" == "Vavoom95 - Win32 Release"

USERDEP__D16_P="../../source/asm_i386.h"	
# Begin Custom Build - Assembling $(InputPath)...
IntDir=.\Release
InputPath=..\..\source\d16_part.s
InputName=d16_part

"$(IntDir)\$(InputName).obj" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
	gcc -c -x assembler-with-cpp $(InputPath) -o $(IntDir)\$(InputName).obj

# End Custom Build

!ELSEIF  "$(CFG)" == "Vavoom95 - Win32 Debug"

USERDEP__D16_P="../../source/asm_i386.h"	
# Begin Custom Build - Assembling $(InputPath)...
IntDir=.\Debug
InputPath=..\..\source\d16_part.s
InputName=d16_part

"$(IntDir)\$(InputName).obj" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
	gcc -c -x assembler-with-cpp $(InputPath) -o $(IntDir)\$(InputName).obj

# End Custom Build

!ENDIF 

# End Source File
# Begin Source File

SOURCE=..\..\source\d16_poly.s

!IF  "$(CFG)" == "Vavoom95 - Win32 Release"

USERDEP__D16_PO="../../source/asm_i386.h"	
# Begin Custom Build - Assembling $(InputPath)...
IntDir=.\Release
InputPath=..\..\source\d16_poly.s
InputName=d16_poly

"$(IntDir)\$(InputName).obj" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
	gcc -c -x assembler-with-cpp $(InputPath) -o $(IntDir)\$(InputName).obj

# End Custom Build

!ELSEIF  "$(CFG)" == "Vavoom95 - Win32 Debug"

USERDEP__D16_PO="../../source/asm_i386.h"	
# Begin Custom Build - Assembling $(InputPath)...
IntDir=.\Debug
InputPath=..\..\source\d16_poly.s
InputName=d16_poly

"$(IntDir)\$(InputName).obj" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
	gcc -c -x assembler-with-cpp $(InputPath) -o $(IntDir)\$(InputName).obj

# End Custom Build

!ENDIF 

# End Source File
# Begin Source File

SOURCE=..\..\source\d16_s16.s

!IF  "$(CFG)" == "Vavoom95 - Win32 Release"

USERDEP__D16_S="../../source/asm_i386.h"	
# Begin Custom Build - Assembling $(InputPath)...
IntDir=.\Release
InputPath=..\..\source\d16_s16.s
InputName=d16_s16

"$(IntDir)\$(InputName).obj" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
	gcc -c -x assembler-with-cpp $(InputPath) -o $(IntDir)\$(InputName).obj

# End Custom Build

!ELSEIF  "$(CFG)" == "Vavoom95 - Win32 Debug"

USERDEP__D16_S="../../source/asm_i386.h"	
# Begin Custom Build - Assembling $(InputPath)...
IntDir=.\Debug
InputPath=..\..\source\d16_s16.s
InputName=d16_s16

"$(IntDir)\$(InputName).obj" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
	gcc -c -x assembler-with-cpp $(InputPath) -o $(IntDir)\$(InputName).obj

# End Custom Build

!ENDIF 

# End Source File
# Begin Source File

SOURCE=..\..\source\d16_span.s

!IF  "$(CFG)" == "Vavoom95 - Win32 Release"

USERDEP__D16_SP="../../source/asm_i386.h"	
# Begin Custom Build - Assembling $(InputPath)...
IntDir=.\Release
InputPath=..\..\source\d16_span.s
InputName=d16_span

"$(IntDir)\$(InputName).obj" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
	gcc -c -x assembler-with-cpp $(InputPath) -o $(IntDir)\$(InputName).obj

# End Custom Build

!ELSEIF  "$(CFG)" == "Vavoom95 - Win32 Debug"

USERDEP__D16_SP="../../source/asm_i386.h"	
# Begin Custom Build - Assembling $(InputPath)...
IntDir=.\Debug
InputPath=..\..\source\d16_span.s
InputName=d16_span

"$(IntDir)\$(InputName).obj" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
	gcc -c -x assembler-with-cpp $(InputPath) -o $(IntDir)\$(InputName).obj

# End Custom Build

!ENDIF 

# End Source File
# Begin Source File

SOURCE=..\..\source\d16_spr.s

!IF  "$(CFG)" == "Vavoom95 - Win32 Release"

USERDEP__D16_SPR="../../source/asm_i386.h"	
# Begin Custom Build - Assembling $(InputPath)...
IntDir=.\Release
InputPath=..\..\source\d16_spr.s
InputName=d16_spr

"$(IntDir)\$(InputName).obj" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
	gcc -c -x assembler-with-cpp $(InputPath) -o $(IntDir)\$(InputName).obj

# End Custom Build

!ELSEIF  "$(CFG)" == "Vavoom95 - Win32 Debug"

USERDEP__D16_SPR="../../source/asm_i386.h"	
# Begin Custom Build - Assembling $(InputPath)...
IntDir=.\Debug
InputPath=..\..\source\d16_spr.s
InputName=d16_spr

"$(IntDir)\$(InputName).obj" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
	gcc -c -x assembler-with-cpp $(InputPath) -o $(IntDir)\$(InputName).obj

# End Custom Build

!ENDIF 

# End Source File
# Begin Source File

SOURCE=..\..\source\d16_surf.s

!IF  "$(CFG)" == "Vavoom95 - Win32 Release"

USERDEP__D16_SU="../../source/asm_i386.h"	
# Begin Custom Build - Assembling $(InputPath)...
IntDir=.\Release
InputPath=..\..\source\d16_surf.s
InputName=d16_surf

"$(IntDir)\$(InputName).obj" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
	gcc -c -x assembler-with-cpp $(InputPath) -o $(IntDir)\$(InputName).obj

# End Custom Build

!ELSEIF  "$(CFG)" == "Vavoom95 - Win32 Debug"

USERDEP__D16_SU="../../source/asm_i386.h"	
# Begin Custom Build - Assembling $(InputPath)...
IntDir=.\Debug
InputPath=..\..\source\d16_surf.s
InputName=d16_surf

"$(IntDir)\$(InputName).obj" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
	gcc -c -x assembler-with-cpp $(InputPath) -o $(IntDir)\$(InputName).obj

# End Custom Build

!ENDIF 

# End Source File
# Begin Source File

SOURCE=..\..\source\d32_part.s

!IF  "$(CFG)" == "Vavoom95 - Win32 Release"

USERDEP__D32_P="../../source/asm_i386.h"	
# Begin Custom Build - Assembling $(InputPath)...
IntDir=.\Release
InputPath=..\..\source\d32_part.s
InputName=d32_part

"$(IntDir)\$(InputName).obj" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
	gcc -c -x assembler-with-cpp $(InputPath) -o $(IntDir)\$(InputName).obj

# End Custom Build

!ELSEIF  "$(CFG)" == "Vavoom95 - Win32 Debug"

USERDEP__D32_P="../../source/asm_i386.h"	
# Begin Custom Build - Assembling $(InputPath)...
IntDir=.\Debug
InputPath=..\..\source\d32_part.s
InputName=d32_part

"$(IntDir)\$(InputName).obj" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
	gcc -c -x assembler-with-cpp $(InputPath) -o $(IntDir)\$(InputName).obj

# End Custom Build

!ENDIF 

# End Source File
# Begin Source File

SOURCE=..\..\source\d32_poly.s

!IF  "$(CFG)" == "Vavoom95 - Win32 Release"

USERDEP__D32_PO="../../source/asm_i386.h"	
# Begin Custom Build - Assembling $(InputPath)...
IntDir=.\Release
InputPath=..\..\source\d32_poly.s
InputName=d32_poly

"$(IntDir)\$(InputName).obj" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
	gcc -c -x assembler-with-cpp $(InputPath) -o $(IntDir)\$(InputName).obj

# End Custom Build

!ELSEIF  "$(CFG)" == "Vavoom95 - Win32 Debug"

USERDEP__D32_PO="../../source/asm_i386.h"	
# Begin Custom Build - Assembling $(InputPath)...
IntDir=.\Debug
InputPath=..\..\source\d32_poly.s
InputName=d32_poly

"$(IntDir)\$(InputName).obj" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
	gcc -c -x assembler-with-cpp $(InputPath) -o $(IntDir)\$(InputName).obj

# End Custom Build

!ENDIF 

# End Source File
# Begin Source File

SOURCE=..\..\source\d32_s16.s

!IF  "$(CFG)" == "Vavoom95 - Win32 Release"

USERDEP__D32_S="../../source/asm_i386.h"	
# Begin Custom Build - Assembling $(InputPath)...
IntDir=.\Release
InputPath=..\..\source\d32_s16.s
InputName=d32_s16

"$(IntDir)\$(InputName).obj" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
	gcc -c -x assembler-with-cpp $(InputPath) -o $(IntDir)\$(InputName).obj

# End Custom Build

!ELSEIF  "$(CFG)" == "Vavoom95 - Win32 Debug"

USERDEP__D32_S="../../source/asm_i386.h"	
# Begin Custom Build - Assembling $(InputPath)...
IntDir=.\Debug
InputPath=..\..\source\d32_s16.s
InputName=d32_s16

"$(IntDir)\$(InputName).obj" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
	gcc -c -x assembler-with-cpp $(InputPath) -o $(IntDir)\$(InputName).obj

# End Custom Build

!ENDIF 

# End Source File
# Begin Source File

SOURCE=..\..\source\d32_span.s

!IF  "$(CFG)" == "Vavoom95 - Win32 Release"

USERDEP__D32_SP="../../source/asm_i386.h"	
# Begin Custom Build - Assembling $(InputPath)...
IntDir=.\Release
InputPath=..\..\source\d32_span.s
InputName=d32_span

"$(IntDir)\$(InputName).obj" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
	gcc -c -x assembler-with-cpp $(InputPath) -o $(IntDir)\$(InputName).obj

# End Custom Build

!ELSEIF  "$(CFG)" == "Vavoom95 - Win32 Debug"

USERDEP__D32_SP="../../source/asm_i386.h"	
# Begin Custom Build - Assembling $(InputPath)...
IntDir=.\Debug
InputPath=..\..\source\d32_span.s
InputName=d32_span

"$(IntDir)\$(InputName).obj" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
	gcc -c -x assembler-with-cpp $(InputPath) -o $(IntDir)\$(InputName).obj

# End Custom Build

!ENDIF 

# End Source File
# Begin Source File

SOURCE=..\..\source\d32_spr.s

!IF  "$(CFG)" == "Vavoom95 - Win32 Release"

USERDEP__D32_SPR="../../source/asm_i386.h"	
# Begin Custom Build - Assembling $(InputPath)...
IntDir=.\Release
InputPath=..\..\source\d32_spr.s
InputName=d32_spr

"$(IntDir)\$(InputName).obj" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
	gcc -c -x assembler-with-cpp $(InputPath) -o $(IntDir)\$(InputName).obj

# End Custom Build

!ELSEIF  "$(CFG)" == "Vavoom95 - Win32 Debug"

USERDEP__D32_SPR="../../source/asm_i386.h"	
# Begin Custom Build - Assembling $(InputPath)...
IntDir=.\Debug
InputPath=..\..\source\d32_spr.s
InputName=d32_spr

"$(IntDir)\$(InputName).obj" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
	gcc -c -x assembler-with-cpp $(InputPath) -o $(IntDir)\$(InputName).obj

# End Custom Build

!ENDIF 

# End Source File
# Begin Source File

SOURCE=..\..\source\d32_surf.s

!IF  "$(CFG)" == "Vavoom95 - Win32 Release"

USERDEP__D32_SU="../../source/asm_i386.h"	
# Begin Custom Build - Assembling $(InputPath)...
IntDir=.\Release
InputPath=..\..\source\d32_surf.s
InputName=d32_surf

"$(IntDir)\$(InputName).obj" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
	gcc -c -x assembler-with-cpp $(InputPath) -o $(IntDir)\$(InputName).obj

# End Custom Build

!ELSEIF  "$(CFG)" == "Vavoom95 - Win32 Debug"

USERDEP__D32_SU="../../source/asm_i386.h"	
# Begin Custom Build - Assembling $(InputPath)...
IntDir=.\Debug
InputPath=..\..\source\d32_surf.s
InputName=d32_surf

"$(IntDir)\$(InputName).obj" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
	gcc -c -x assembler-with-cpp $(InputPath) -o $(IntDir)\$(InputName).obj

# End Custom Build

!ENDIF 

# End Source File
# Begin Source File

SOURCE=..\..\source\d8_part.s

!IF  "$(CFG)" == "Vavoom95 - Win32 Release"

USERDEP__D8_PA="../../source/asm_i386.h"	
# Begin Custom Build - Assembling $(InputPath)...
IntDir=.\Release
InputPath=..\..\source\d8_part.s
InputName=d8_part

"$(IntDir)\$(InputName).obj" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
	gcc -c -x assembler-with-cpp $(InputPath) -o $(IntDir)\$(InputName).obj

# End Custom Build

!ELSEIF  "$(CFG)" == "Vavoom95 - Win32 Debug"

USERDEP__D8_PA="../../source/asm_i386.h"	
# Begin Custom Build - Assembling $(InputPath)...
IntDir=.\Debug
InputPath=..\..\source\d8_part.s
InputName=d8_part

"$(IntDir)\$(InputName).obj" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
	gcc -c -x assembler-with-cpp $(InputPath) -o $(IntDir)\$(InputName).obj

# End Custom Build

!ENDIF 

# End Source File
# Begin Source File

SOURCE=..\..\source\d8_poly.s

!IF  "$(CFG)" == "Vavoom95 - Win32 Release"

USERDEP__D8_PO="../../source/asm_i386.h"	
# Begin Custom Build - Assembling $(InputPath)...
IntDir=.\Release
InputPath=..\..\source\d8_poly.s
InputName=d8_poly

"$(IntDir)\$(InputName).obj" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
	gcc -c -x assembler-with-cpp $(InputPath) -o $(IntDir)\$(InputName).obj

# End Custom Build

!ELSEIF  "$(CFG)" == "Vavoom95 - Win32 Debug"

USERDEP__D8_PO="../../source/asm_i386.h"	
# Begin Custom Build - Assembling $(InputPath)...
IntDir=.\Debug
InputPath=..\..\source\d8_poly.s
InputName=d8_poly

"$(IntDir)\$(InputName).obj" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
	gcc -c -x assembler-with-cpp $(InputPath) -o $(IntDir)\$(InputName).obj

# End Custom Build

!ENDIF 

# End Source File
# Begin Source File

SOURCE=..\..\source\d8_s16.s

!IF  "$(CFG)" == "Vavoom95 - Win32 Release"

USERDEP__D8_S1="../../source/asm_i386.h"	
# Begin Custom Build - Assembling $(InputPath)...
IntDir=.\Release
InputPath=..\..\source\d8_s16.s
InputName=d8_s16

"$(IntDir)\$(InputName).obj" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
	gcc -c -x assembler-with-cpp $(InputPath) -o $(IntDir)\$(InputName).obj

# End Custom Build

!ELSEIF  "$(CFG)" == "Vavoom95 - Win32 Debug"

USERDEP__D8_S1="../../source/asm_i386.h"	
# Begin Custom Build - Assembling $(InputPath)...
IntDir=.\Debug
InputPath=..\..\source\d8_s16.s
InputName=d8_s16

"$(IntDir)\$(InputName).obj" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
	gcc -c -x assembler-with-cpp $(InputPath) -o $(IntDir)\$(InputName).obj

# End Custom Build

!ENDIF 

# End Source File
# Begin Source File

SOURCE=..\..\source\d8_span.s

!IF  "$(CFG)" == "Vavoom95 - Win32 Release"

USERDEP__D8_SP="../../source/asm_i386.h"	
# Begin Custom Build - Assembling $(InputPath)...
IntDir=.\Release
InputPath=..\..\source\d8_span.s
InputName=d8_span

"$(IntDir)\$(InputName).obj" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
	gcc -c -x assembler-with-cpp $(InputPath) -o $(IntDir)\$(InputName).obj

# End Custom Build

!ELSEIF  "$(CFG)" == "Vavoom95 - Win32 Debug"

USERDEP__D8_SP="../../source/asm_i386.h"	
# Begin Custom Build - Assembling $(InputPath)...
IntDir=.\Debug
InputPath=..\..\source\d8_span.s
InputName=d8_span

"$(IntDir)\$(InputName).obj" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
	gcc -c -x assembler-with-cpp $(InputPath) -o $(IntDir)\$(InputName).obj

# End Custom Build

!ENDIF 

# End Source File
# Begin Source File

SOURCE=..\..\source\d8_spr.s

!IF  "$(CFG)" == "Vavoom95 - Win32 Release"

USERDEP__D8_SPR="../../source/asm_i386.h"	
# Begin Custom Build - Assembling $(InputPath)...
IntDir=.\Release
InputPath=..\..\source\d8_spr.s
InputName=d8_spr

"$(IntDir)\$(InputName).obj" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
	gcc -c -x assembler-with-cpp $(InputPath) -o $(IntDir)\$(InputName).obj

# End Custom Build

!ELSEIF  "$(CFG)" == "Vavoom95 - Win32 Debug"

USERDEP__D8_SPR="../../source/asm_i386.h"	
# Begin Custom Build - Assembling $(InputPath)...
IntDir=.\Debug
InputPath=..\..\source\d8_spr.s
InputName=d8_spr

"$(IntDir)\$(InputName).obj" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
	gcc -c -x assembler-with-cpp $(InputPath) -o $(IntDir)\$(InputName).obj

# End Custom Build

!ENDIF 

# End Source File
# Begin Source File

SOURCE=..\..\source\d8_surf.s

!IF  "$(CFG)" == "Vavoom95 - Win32 Release"

USERDEP__D8_SU="../../source/asm_i386.h"	
# Begin Custom Build - Assembling $(InputPath)...
IntDir=.\Release
InputPath=..\..\source\d8_surf.s
InputName=d8_surf

"$(IntDir)\$(InputName).obj" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
	gcc -c -x assembler-with-cpp $(InputPath) -o $(IntDir)\$(InputName).obj

# End Custom Build

!ELSEIF  "$(CFG)" == "Vavoom95 - Win32 Debug"

USERDEP__D8_SU="../../source/asm_i386.h"	
# Begin Custom Build - Assembling $(InputPath)...
IntDir=.\Debug
InputPath=..\..\source\d8_surf.s
InputName=d8_surf

"$(IntDir)\$(InputName).obj" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
	gcc -c -x assembler-with-cpp $(InputPath) -o $(IntDir)\$(InputName).obj

# End Custom Build

!ENDIF 

# End Source File
# Begin Source File

SOURCE=..\..\source\d_aclip.cpp
# ADD CPP /W4
# End Source File
# Begin Source File

SOURCE=..\..\source\d_aclipa.s

!IF  "$(CFG)" == "Vavoom95 - Win32 Release"

USERDEP__D_ACL="../../source/asm_i386.h"	
# Begin Custom Build - Assembling $(InputPath)...
IntDir=.\Release
InputPath=..\..\source\d_aclipa.s
InputName=d_aclipa

"$(IntDir)\$(InputName).obj" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
	gcc -c -x assembler-with-cpp $(InputPath) -o $(IntDir)\$(InputName).obj

# End Custom Build

!ELSEIF  "$(CFG)" == "Vavoom95 - Win32 Debug"

USERDEP__D_ACL="../../source/asm_i386.h"	
# Begin Custom Build - Assembling $(InputPath)...
IntDir=.\Debug
InputPath=..\..\source\d_aclipa.s
InputName=d_aclipa

"$(IntDir)\$(InputName).obj" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
	gcc -c -x assembler-with-cpp $(InputPath) -o $(IntDir)\$(InputName).obj

# End Custom Build

!ENDIF 

# End Source File
# Begin Source File

SOURCE=..\..\source\d_alias.cpp
# ADD CPP /W4
# End Source File
# Begin Source File

SOURCE=..\..\source\d_aliasa.s

!IF  "$(CFG)" == "Vavoom95 - Win32 Release"

USERDEP__D_ALI="../../source/asm_i386.h"	
# Begin Custom Build - Assembling $(InputPath)...
IntDir=.\Release
InputPath=..\..\source\d_aliasa.s
InputName=d_aliasa

"$(IntDir)\$(InputName).obj" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
	gcc -c -x assembler-with-cpp $(InputPath) -o $(IntDir)\$(InputName).obj

# End Custom Build

!ELSEIF  "$(CFG)" == "Vavoom95 - Win32 Debug"

USERDEP__D_ALI="../../source/asm_i386.h"	
# Begin Custom Build - Assembling $(InputPath)...
IntDir=.\Debug
InputPath=..\..\source\d_aliasa.s
InputName=d_aliasa

"$(IntDir)\$(InputName).obj" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
	gcc -c -x assembler-with-cpp $(InputPath) -o $(IntDir)\$(InputName).obj

# End Custom Build

!ENDIF 

# End Source File
# Begin Source File

SOURCE=..\..\source\d_alleg.cpp
# PROP Exclude_From_Build 1
# End Source File
# Begin Source File

SOURCE=..\..\source\d_data.cpp
# ADD CPP /W4
# End Source File
# Begin Source File

SOURCE=..\..\source\d_draw.cpp
# ADD CPP /W4
# End Source File
# Begin Source File

SOURCE=..\..\source\d_edge.cpp
# ADD CPP /W4
# End Source File
# Begin Source File

SOURCE=..\..\source\d_edgea.s

!IF  "$(CFG)" == "Vavoom95 - Win32 Release"

USERDEP__D_EDG="../../source/asm_i386.h"	
# Begin Custom Build - Assembling $(InputPath)...
IntDir=.\Release
InputPath=..\..\source\d_edgea.s
InputName=d_edgea

"$(IntDir)\$(InputName).obj" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
	gcc -c -x assembler-with-cpp $(InputPath) -o $(IntDir)\$(InputName).obj

# End Custom Build

!ELSEIF  "$(CFG)" == "Vavoom95 - Win32 Debug"

USERDEP__D_EDG="../../source/asm_i386.h"	
# Begin Custom Build - Assembling $(InputPath)...
IntDir=.\Debug
InputPath=..\..\source\d_edgea.s
InputName=d_edgea

"$(IntDir)\$(InputName).obj" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
	gcc -c -x assembler-with-cpp $(InputPath) -o $(IntDir)\$(InputName).obj

# End Custom Build

!ENDIF 

# End Source File
# Begin Source File

SOURCE=..\..\source\d_local.h
# End Source File
# Begin Source File

SOURCE=..\..\source\d_main.cpp
# ADD CPP /W4
# End Source File
# Begin Source File

SOURCE=..\..\source\d_part.cpp
# ADD CPP /W4
# End Source File
# Begin Source File

SOURCE=..\..\source\d_polysa.s

!IF  "$(CFG)" == "Vavoom95 - Win32 Release"

USERDEP__D_POL="../../source/asm_i386.h"	
# Begin Custom Build - Assembling $(InputPath)...
IntDir=.\Release
InputPath=..\..\source\d_polysa.s
InputName=d_polysa

"$(IntDir)\$(InputName).obj" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
	gcc -c -x assembler-with-cpp $(InputPath) -o $(IntDir)\$(InputName).obj

# End Custom Build

!ELSEIF  "$(CFG)" == "Vavoom95 - Win32 Debug"

USERDEP__D_POL="../../source/asm_i386.h"	
# Begin Custom Build - Assembling $(InputPath)...
IntDir=.\Debug
InputPath=..\..\source\d_polysa.s
InputName=d_polysa

"$(IntDir)\$(InputName).obj" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
	gcc -c -x assembler-with-cpp $(InputPath) -o $(IntDir)\$(InputName).obj

# End Custom Build

!ENDIF 

# End Source File
# Begin Source File

SOURCE=..\..\source\d_polyse.cpp
# ADD CPP /W4
# End Source File
# Begin Source File

SOURCE=..\..\source\d_scache.cpp
# ADD CPP /W4
# End Source File
# Begin Source File

SOURCE=..\..\source\d_sdl.cpp
# PROP Exclude_From_Build 1
# End Source File
# Begin Source File

SOURCE=..\..\source\d_span.cpp
# ADD CPP /W4
# End Source File
# Begin Source File

SOURCE=..\..\source\d_sprite.cpp
# ADD CPP /W4
# End Source File
# Begin Source File

SOURCE=..\..\source\d_surf.cpp
# ADD CPP /W4
# End Source File
# Begin Source File

SOURCE=..\..\source\d_tex.cpp
# ADD CPP /W4
# End Source File
# Begin Source File

SOURCE=..\..\source\d_varsa.s

!IF  "$(CFG)" == "Vavoom95 - Win32 Release"

USERDEP__D_VAR="../../source/asm_i386.h"	
# Begin Custom Build - Assembling $(InputPath)...
IntDir=.\Release
InputPath=..\..\source\d_varsa.s
InputName=d_varsa

"$(IntDir)\$(InputName).obj" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
	gcc -c -x assembler-with-cpp $(InputPath) -o $(IntDir)\$(InputName).obj

# End Custom Build

!ELSEIF  "$(CFG)" == "Vavoom95 - Win32 Debug"

USERDEP__D_VAR="../../source/asm_i386.h"	
# Begin Custom Build - Assembling $(InputPath)...
IntDir=.\Debug
InputPath=..\..\source\d_varsa.s
InputName=d_varsa

"$(IntDir)\$(InputName).obj" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
	gcc -c -x assembler-with-cpp $(InputPath) -o $(IntDir)\$(InputName).obj

# End Custom Build

!ENDIF 

# End Source File
# Begin Source File

SOURCE=..\..\source\d_win32.cpp
# ADD CPP /W4
# End Source File
# Begin Source File

SOURCE=..\..\source\d_zspan.s

!IF  "$(CFG)" == "Vavoom95 - Win32 Release"

USERDEP__D_ZSP="../../source/asm_i386.h"	
# Begin Custom Build - Assembling $(InputPath)...
IntDir=.\Release
InputPath=..\..\source\d_zspan.s
InputName=d_zspan

"$(IntDir)\$(InputName).obj" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
	gcc -c -x assembler-with-cpp $(InputPath) -o $(IntDir)\$(InputName).obj

# End Custom Build

!ELSEIF  "$(CFG)" == "Vavoom95 - Win32 Debug"

USERDEP__D_ZSP="../../source/asm_i386.h"	
# Begin Custom Build - Assembling $(InputPath)...
IntDir=.\Debug
InputPath=..\..\source\d_zspan.s
InputName=d_zspan

"$(IntDir)\$(InputName).obj" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
	gcc -c -x assembler-with-cpp $(InputPath) -o $(IntDir)\$(InputName).obj

# End Custom Build

!ENDIF 

# End Source File
# End Group
# Begin Group "OpenGL"

# PROP Default_Filter ""
# Begin Source File

SOURCE=..\..\source\gl_alleg.cpp
# PROP Exclude_From_Build 1
# End Source File
# Begin Source File

SOURCE=..\..\source\gl_draw.cpp
# ADD CPP /W4
# End Source File
# Begin Source File

SOURCE=..\..\source\gl_local.h
# End Source File
# Begin Source File

SOURCE=..\..\source\gl_main.cpp
# ADD CPP /W4
# End Source File
# Begin Source File

SOURCE=..\..\source\gl_poly.cpp
# ADD CPP /W4
# End Source File
# Begin Source File

SOURCE=..\..\source\gl_sdl.cpp
# PROP Exclude_From_Build 1
# End Source File
# Begin Source File

SOURCE=..\..\source\gl_tex.cpp
# ADD CPP /W4
# End Source File
# Begin Source File

SOURCE=..\..\source\gl_win32.cpp
# ADD CPP /W4
# End Source File
# Begin Source File

SOURCE=..\..\source\gl_x.cpp
# PROP Exclude_From_Build 1
# End Source File
# End Group
# Begin Group "Direct3D"

# PROP Default_Filter ""
# Begin Source File

SOURCE=..\..\source\d3d_draw.cpp
# ADD CPP /W4
# End Source File
# Begin Source File

SOURCE=..\..\source\d3d_info.cpp
# ADD CPP /W4
# End Source File
# Begin Source File

SOURCE=..\..\source\d3d_local.h
# End Source File
# Begin Source File

SOURCE=..\..\source\d3d_main.cpp
# ADD CPP /W4
# End Source File
# Begin Source File

SOURCE=..\..\source\d3d_poly.cpp
# ADD CPP /W4
# End Source File
# Begin Source File

SOURCE=..\..\source\d3d_tex.cpp
# ADD CPP /W4
# End Source File
# End Group
# Begin Source File

SOURCE=..\..\source\adivtab.h
# End Source File
# Begin Source File

SOURCE=..\..\source\anorm_dots.h
# End Source File
# Begin Source File

SOURCE=..\..\source\anorms.h
# End Source File
# Begin Source File

SOURCE=..\..\source\drawer.h
# End Source File
# Begin Source File

SOURCE=..\..\source\fgfxdefs.h
# End Source File
# Begin Source File

SOURCE=..\..\source\fmd2defs.h
# End Source File
# Begin Source File

SOURCE=..\..\source\ftexdefs.h
# End Source File
# Begin Source File

SOURCE=..\..\source\r_bsp.cpp
# ADD CPP /W4
# End Source File
# Begin Source File

SOURCE=..\..\source\r_light.cpp
# ADD CPP /W4
# End Source File
# Begin Source File

SOURCE=..\..\source\r_local.h
# End Source File
# Begin Source File

SOURCE=..\..\source\r_main.cpp
# ADD CPP /W4
# End Source File
# Begin Source File

SOURCE=..\..\source\r_model.cpp
# ADD CPP /W4
# End Source File
# Begin Source File

SOURCE=..\..\source\r_shared.h
# End Source File
# Begin Source File

SOURCE=..\..\source\r_sky.cpp
# ADD CPP /W4
# End Source File
# Begin Source File

SOURCE=..\..\source\r_surf.cpp
# ADD CPP /W4
# End Source File
# Begin Source File

SOURCE=..\..\source\r_tex.cpp
# ADD CPP /W4
# End Source File
# Begin Source File

SOURCE=..\..\source\r_things.cpp
# ADD CPP /W4
# End Source File
# Begin Source File

SOURCE=..\..\source\render.h
# End Source File
# End Group
# Begin Group "Networking"

# PROP Default_Filter ""
# Begin Source File

SOURCE=..\..\source\comport.h
# End Source File
# Begin Source File

SOURCE=..\..\source\mpdosock.h
# End Source File
# Begin Source File

SOURCE=..\..\source\mplib.c
# PROP Exclude_From_Build 1
# End Source File
# Begin Source File

SOURCE=..\..\source\mplpc.c
# PROP Exclude_From_Build 1
# End Source File
# Begin Source File

SOURCE=..\..\source\net_bsd.cpp
# PROP Exclude_From_Build 1
# End Source File
# Begin Source File

SOURCE=..\..\source\net_bw.cpp
# PROP Exclude_From_Build 1
# End Source File
# Begin Source File

SOURCE=..\..\source\net_bw.h
# End Source File
# Begin Source File

SOURCE=..\..\source\net_comm.cpp
# PROP Exclude_From_Build 1
# End Source File
# Begin Source File

SOURCE=..\..\source\net_dgrm.cpp
# ADD CPP /W4
# End Source File
# Begin Source File

SOURCE=..\..\source\net_dgrm.h
# End Source File
# Begin Source File

SOURCE=..\..\source\net_dos.cpp
# PROP Exclude_From_Build 1
# End Source File
# Begin Source File

SOURCE=..\..\source\net_ipx.cpp
# PROP Exclude_From_Build 1
# End Source File
# Begin Source File

SOURCE=..\..\source\net_ipx.h
# End Source File
# Begin Source File

SOURCE=..\..\source\net_loc.h
# End Source File
# Begin Source File

SOURCE=..\..\source\net_loop.cpp
# ADD CPP /W4
# End Source File
# Begin Source File

SOURCE=..\..\source\net_loop.h
# End Source File
# Begin Source File

SOURCE=..\..\source\net_main.cpp
# ADD CPP /W4
# End Source File
# Begin Source File

SOURCE=..\..\source\net_mp.cpp
# PROP Exclude_From_Build 1
# End Source File
# Begin Source File

SOURCE=..\..\source\net_mp.h
# End Source File
# Begin Source File

SOURCE=..\..\source\net_none.cpp
# PROP Exclude_From_Build 1
# End Source File
# Begin Source File

SOURCE=..\..\source\net_null.cpp
# ADD CPP /W4
# End Source File
# Begin Source File

SOURCE=..\..\source\net_null.h
# End Source File
# Begin Source File

SOURCE=..\..\source\net_ser.cpp
# PROP Exclude_From_Build 1
# End Source File
# Begin Source File

SOURCE=..\..\source\net_ser.h
# End Source File
# Begin Source File

SOURCE=..\..\source\net_udp.cpp
# PROP Exclude_From_Build 1
# End Source File
# Begin Source File

SOURCE=..\..\source\net_udp.h
# End Source File
# Begin Source File

SOURCE=..\..\source\net_win.cpp
# ADD CPP /W4
# End Source File
# Begin Source File

SOURCE=..\..\source\net_wins.cpp
# ADD CPP /W4
# End Source File
# Begin Source File

SOURCE=..\..\source\net_wins.h
# End Source File
# Begin Source File

SOURCE=..\..\source\net_wipx.cpp
# ADD CPP /W4
# End Source File
# Begin Source File

SOURCE=..\..\source\net_wipx.h
# End Source File
# Begin Source File

SOURCE=..\..\source\network.h
# End Source File
# End Group
# Begin Group "Sound"

# PROP Default_Filter ""
# Begin Source File

SOURCE=..\..\source\cd_dos.cpp
# PROP Exclude_From_Build 1
# End Source File
# Begin Source File

SOURCE=..\..\source\cd_linux.cpp
# PROP Exclude_From_Build 1
# End Source File
# Begin Source File

SOURCE=..\..\source\cd_win32.cpp
# ADD CPP /W4
# End Source File
# Begin Source File

SOURCE=..\..\source\eax.h
# End Source File
# Begin Source File

SOURCE=..\..\source\s_al.cpp
# End Source File
# Begin Source File

SOURCE=..\..\source\s_alleg.cpp
# PROP Exclude_From_Build 1
# End Source File
# Begin Source File

SOURCE=..\..\source\s_allegm.cpp
# PROP Exclude_From_Build 1
# End Source File
# Begin Source File

SOURCE=..\..\source\s_data.cpp
# ADD CPP /W4
# End Source File
# Begin Source File

SOURCE=..\..\source\s_eaxutl.cpp
# End Source File
# Begin Source File

SOURCE=..\..\source\s_flac.cpp
# End Source File
# Begin Source File

SOURCE=..\..\source\s_local.h
# End Source File
# Begin Source File

SOURCE=..\..\source\s_mikmod.cpp
# End Source File
# Begin Source File

SOURCE=..\..\source\s_mp3.cpp
# End Source File
# Begin Source File

SOURCE=..\..\source\s_sdl.cpp
# PROP Exclude_From_Build 1
# End Source File
# Begin Source File

SOURCE=..\..\source\s_sdlm.cpp
# PROP Exclude_From_Build 1
# End Source File
# Begin Source File

SOURCE=..\..\source\s_sound.cpp
# ADD CPP /W4
# End Source File
# Begin Source File

SOURCE=..\..\source\s_tmidty.cpp
# End Source File
# Begin Source File

SOURCE=..\..\source\s_vorbis.cpp
# End Source File
# Begin Source File

SOURCE=..\..\source\s_wav.cpp
# End Source File
# Begin Source File

SOURCE=..\..\source\s_win32.cpp
# ADD CPP /W4
# End Source File
# Begin Source File

SOURCE=..\..\source\s_win32m.cpp
# ADD CPP /W4
# End Source File
# Begin Source File

SOURCE=..\..\source\sn_sonix.cpp
# ADD CPP /W4
# End Source File
# Begin Source File

SOURCE=..\..\source\sound.h
# End Source File
# End Group
# Begin Group "User Interface"

# PROP Default_Filter ""
# Begin Source File

SOURCE=..\..\source\am_map.cpp

!IF  "$(CFG)" == "Vavoom95 - Win32 Release"

# ADD CPP /W4 /GR-

!ELSEIF  "$(CFG)" == "Vavoom95 - Win32 Debug"

# ADD CPP /W4

!ENDIF 

# End Source File
# Begin Source File

SOURCE=..\..\source\automap.h
# End Source File
# Begin Source File

SOURCE=..\..\source\chat.cpp
# ADD CPP /W4
# End Source File
# Begin Source File

SOURCE=..\..\source\chat.h
# End Source File
# Begin Source File

SOURCE=..\..\source\console.cpp
# ADD CPP /W4
# End Source File
# Begin Source File

SOURCE=..\..\source\console.h
# End Source File
# Begin Source File

SOURCE=..\..\source\finale.cpp
# ADD CPP /W4
# End Source File
# Begin Source File

SOURCE=..\..\source\finale.h
# End Source File
# Begin Source File

SOURCE=..\..\source\iline.cpp
# ADD CPP /W4
# End Source File
# Begin Source File

SOURCE=..\..\source\iline.h
# End Source File
# Begin Source File

SOURCE=..\..\source\imission.cpp
# ADD CPP /W4
# End Source File
# Begin Source File

SOURCE=..\..\source\imission.h
# End Source File
# Begin Source File

SOURCE=..\..\source\l_glbsp.cpp
# ADD CPP /W4
# End Source File
# Begin Source File

SOURCE=..\..\source\l_glvis.cpp
# ADD CPP /W4
# End Source File
# Begin Source File

SOURCE=..\..\source\menu.cpp
# ADD CPP /W4
# End Source File
# Begin Source File

SOURCE=..\..\source\menu.h
# End Source File
# Begin Source File

SOURCE=..\..\source\sbar.cpp
# ADD CPP /W4
# End Source File
# Begin Source File

SOURCE=..\..\source\sbar.h
# End Source File
# Begin Source File

SOURCE=..\..\source\screen.cpp
# ADD CPP /W4
# End Source File
# Begin Source File

SOURCE=..\..\source\screen.h
# End Source File
# Begin Source File

SOURCE=..\..\source\text.cpp
# ADD CPP /W4
# End Source File
# Begin Source File

SOURCE=..\..\source\text.h
# End Source File
# Begin Source File

SOURCE=..\..\source\ui.h
# End Source File
# Begin Source File

SOURCE=..\..\source\ui_gc.cpp
# ADD CPP /W4
# End Source File
# Begin Source File

SOURCE=..\..\source\ui_gc.h
# End Source File
# Begin Source File

SOURCE=..\..\source\ui_modal.cpp
# End Source File
# Begin Source File

SOURCE=..\..\source\ui_modal.h
# End Source File
# Begin Source File

SOURCE=..\..\source\ui_root.cpp
# End Source File
# Begin Source File

SOURCE=..\..\source\ui_root.h
# End Source File
# Begin Source File

SOURCE=..\..\source\ui_win.cpp
# ADD CPP /W4
# End Source File
# Begin Source File

SOURCE=..\..\source\ui_win.h
# End Source File
# Begin Source File

SOURCE=..\..\source\video.h
# End Source File
# End Group
# Begin Group "Input"

# PROP Default_Filter ""
# Begin Source File

SOURCE=..\..\source\in_alleg.cpp
# PROP Exclude_From_Build 1
# End Source File
# Begin Source File

SOURCE=..\..\source\in_dos.cpp
# PROP Exclude_From_Build 1
# End Source File
# Begin Source File

SOURCE=..\..\source\in_input.cpp
# ADD CPP /W4
# End Source File
# Begin Source File

SOURCE=..\..\source\in_sdl.cpp
# PROP Exclude_From_Build 1
# End Source File
# Begin Source File

SOURCE=..\..\source\in_win32.cpp
# ADD CPP /W4
# End Source File
# Begin Source File

SOURCE=..\..\source\input.h
# End Source File
# End Group
# Begin Group "Client"

# PROP Default_Filter ""
# Begin Source File

SOURCE=..\..\source\cl_demo.cpp
# ADD CPP /W4
# End Source File
# Begin Source File

SOURCE=..\..\source\cl_input.cpp
# ADD CPP /W4
# End Source File
# Begin Source File

SOURCE=..\..\source\cl_local.h
# End Source File
# Begin Source File

SOURCE=..\..\source\cl_main.cpp
# ADD CPP /W4
# End Source File
# Begin Source File

SOURCE=..\..\source\cl_parse.cpp
# ADD CPP /W4
# End Source File
# Begin Source File

SOURCE=..\..\source\cl_poly.cpp
# ADD CPP /W4
# End Source File
# Begin Source File

SOURCE=..\..\source\cl_trace.cpp
# ADD CPP /W4
# End Source File
# Begin Source File

SOURCE=..\..\source\client.h
# End Source File
# End Group
# Begin Group "Server"

# PROP Default_Filter ""
# Begin Source File

SOURCE=..\..\source\player.h
# End Source File
# Begin Source File

SOURCE=..\..\source\server.h
# End Source File
# Begin Source File

SOURCE=..\..\source\sv_acs.cpp
# ADD CPP /W4
# End Source File
# Begin Source File

SOURCE=..\..\source\sv_ent.cpp
# End Source File
# Begin Source File

SOURCE=..\..\source\sv_local.h
# End Source File
# Begin Source File

SOURCE=..\..\source\sv_main.cpp
# ADD CPP /W4
# End Source File
# Begin Source File

SOURCE=..\..\source\sv_poly.cpp
# ADD CPP /W4
# End Source File
# Begin Source File

SOURCE=..\..\source\sv_save.cpp
# ADD CPP /W4
# End Source File
# Begin Source File

SOURCE=..\..\source\sv_sight.cpp
# ADD CPP /W4
# End Source File
# Begin Source File

SOURCE=..\..\source\sv_swtch.cpp
# ADD CPP /W4
# End Source File
# Begin Source File

SOURCE=..\..\source\sv_tick.cpp
# ADD CPP /W4
# End Source File
# Begin Source File

SOURCE=..\..\source\sv_user.cpp
# ADD CPP /W4
# End Source File
# Begin Source File

SOURCE=..\..\source\sv_world.cpp
# ADD CPP /W4
# End Source File
# End Group
# Begin Group "System"

# PROP Default_Filter ""
# Begin Source File

SOURCE=..\..\source\npxsetup.c
# PROP Exclude_From_Build 1
# End Source File
# Begin Source File

SOURCE=..\..\source\sys_bsd.cpp
# PROP Exclude_From_Build 1
# End Source File
# Begin Source File

SOURCE=..\..\source\sys_dos.cpp
# PROP Exclude_From_Build 1
# End Source File
# Begin Source File

SOURCE=..\..\source\sys_i386.s

!IF  "$(CFG)" == "Vavoom95 - Win32 Release"

USERDEP__SYS_I="../../source/asm_i386.h"	
# Begin Custom Build - Assembling $(InputPath)...
IntDir=.\Release
InputPath=..\..\source\sys_i386.s
InputName=sys_i386

"$(IntDir)\$(InputName).obj" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
	gcc -c -x assembler-with-cpp $(InputPath) -o $(IntDir)\$(InputName).obj

# End Custom Build

!ELSEIF  "$(CFG)" == "Vavoom95 - Win32 Debug"

USERDEP__SYS_I="../../source/asm_i386.h"	
# Begin Custom Build - Assembling $(InputPath)...
IntDir=.\Debug
InputPath=..\..\source\sys_i386.s
InputName=sys_i386

"$(IntDir)\$(InputName).obj" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
	gcc -c -x assembler-with-cpp $(InputPath) -o $(IntDir)\$(InputName).obj

# End Custom Build

!ENDIF 

# End Source File
# Begin Source File

SOURCE=..\..\source\sys_lin.cpp
# PROP Exclude_From_Build 1
# End Source File
# Begin Source File

SOURCE=..\..\source\sys_sdl.cpp
# PROP Exclude_From_Build 1
# End Source File
# Begin Source File

SOURCE=..\..\source\sys_win.cpp
# ADD CPP /W4
# End Source File
# Begin Source File

SOURCE=..\..\source\sys_wind.cpp
# PROP Exclude_From_Build 1
# End Source File
# Begin Source File

SOURCE=..\..\source\system.h
# End Source File
# Begin Source File

SOURCE=..\..\source\winlocal.h
# End Source File
# End Group
# End Target
# End Project
