# Microsoft Developer Studio Project File - Name="ddf2vc" - Package Owner=<4>
# Microsoft Developer Studio Generated Build File, Format Version 6.00
# ** DO NOT EDIT **

# TARGTYPE "Win32 (x86) Console Application" 0x0103

CFG=ddf2vc - Win32 Debug
!MESSAGE This is not a valid makefile. To build this project using NMAKE,
!MESSAGE use the Export Makefile command and run
!MESSAGE 
!MESSAGE NMAKE /f "ddf2vc.mak".
!MESSAGE 
!MESSAGE You can specify a configuration when running NMAKE
!MESSAGE by defining the macro CFG on the command line. For example:
!MESSAGE 
!MESSAGE NMAKE /f "ddf2vc.mak" CFG="ddf2vc - Win32 Debug"
!MESSAGE 
!MESSAGE Possible choices for configuration are:
!MESSAGE 
!MESSAGE "ddf2vc - Win32 Release" (based on "Win32 (x86) Console Application")
!MESSAGE "ddf2vc - Win32 Debug" (based on "Win32 (x86) Console Application")
!MESSAGE 

# Begin Project
# PROP AllowPerConfigDependencies 0
# PROP Scc_ProjName ""
# PROP Scc_LocalPath ""
CPP=cl.exe
RSC=rc.exe

!IF  "$(CFG)" == "ddf2vc - Win32 Release"

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
# ADD BASE CPP /nologo /W3 /GX /O2 /D "WIN32" /D "NDEBUG" /D "_CONSOLE" /D "_MBCS" /YX /FD /c
# ADD CPP /nologo /W3 /GX /O2 /D "NDEBUG" /D "WIN32" /D "_CONSOLE" /D "_MBCS" /D "_GATESY_" /YX /FD /c
# ADD BASE RSC /l 0x426 /d "NDEBUG"
# ADD RSC /l 0x426 /d "NDEBUG"
BSC32=bscmake.exe
# ADD BASE BSC32 /nologo
# ADD BSC32 /nologo
LINK32=link.exe
# ADD BASE LINK32 kernel32.lib user32.lib gdi32.lib winspool.lib comdlg32.lib advapi32.lib shell32.lib ole32.lib oleaut32.lib uuid.lib odbc32.lib odbccp32.lib  kernel32.lib user32.lib gdi32.lib winspool.lib comdlg32.lib advapi32.lib shell32.lib ole32.lib oleaut32.lib uuid.lib odbc32.lib odbccp32.lib /nologo /subsystem:console /machine:I386
# ADD LINK32 kernel32.lib user32.lib gdi32.lib winspool.lib comdlg32.lib advapi32.lib shell32.lib ole32.lib oleaut32.lib uuid.lib odbc32.lib odbccp32.lib  kernel32.lib user32.lib gdi32.lib winspool.lib comdlg32.lib advapi32.lib shell32.lib ole32.lib oleaut32.lib uuid.lib odbc32.lib odbccp32.lib /nologo /subsystem:console /machine:I386

!ELSEIF  "$(CFG)" == "ddf2vc - Win32 Debug"

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
# ADD BASE CPP /nologo /W3 /Gm /GX /ZI /Od /D "WIN32" /D "_DEBUG" /D "_CONSOLE" /D "_MBCS" /YX /FD /GZ  /c
# ADD CPP /nologo /W3 /Gm /GX /ZI /Od /D "_DEBUG" /D "WIN32" /D "_CONSOLE" /D "_MBCS" /D "_GATESY_" /YX /FD /GZ  /c
# ADD BASE RSC /l 0x426 /d "_DEBUG"
# ADD RSC /l 0x426 /d "_DEBUG"
BSC32=bscmake.exe
# ADD BASE BSC32 /nologo
# ADD BSC32 /nologo
LINK32=link.exe
# ADD BASE LINK32 kernel32.lib user32.lib gdi32.lib winspool.lib comdlg32.lib advapi32.lib shell32.lib ole32.lib oleaut32.lib uuid.lib odbc32.lib odbccp32.lib  kernel32.lib user32.lib gdi32.lib winspool.lib comdlg32.lib advapi32.lib shell32.lib ole32.lib oleaut32.lib uuid.lib odbc32.lib odbccp32.lib /nologo /subsystem:console /debug /machine:I386 /pdbtype:sept
# ADD LINK32 kernel32.lib user32.lib gdi32.lib winspool.lib comdlg32.lib advapi32.lib shell32.lib ole32.lib oleaut32.lib uuid.lib odbc32.lib odbccp32.lib  kernel32.lib user32.lib gdi32.lib winspool.lib comdlg32.lib advapi32.lib shell32.lib ole32.lib oleaut32.lib uuid.lib odbc32.lib odbccp32.lib /nologo /subsystem:console /debug /machine:I386 /pdbtype:sept

!ENDIF 

# Begin Target

# Name "ddf2vc - Win32 Release"
# Name "ddf2vc - Win32 Debug"
# Begin Group "Source Files"

# PROP Default_Filter "cpp;c;cxx;rc;def;r;odl;idl;hpj;bat"
# Begin Source File

SOURCE=..\..\utils\ddf2vc\action.c
# End Source File
# Begin Source File

SOURCE=..\..\utils\ddf2vc\ddf2vc.c
# End Source File
# Begin Source File

SOURCE=..\..\utils\ddf2vc\ddf_anim.c
# End Source File
# Begin Source File

SOURCE=..\..\utils\ddf2vc\ddf_atk.c
# End Source File
# Begin Source File

SOURCE=..\..\utils\ddf2vc\ddf_boom.c
# End Source File
# Begin Source File

SOURCE=..\..\utils\ddf2vc\ddf_colm.c
# End Source File
# Begin Source File

SOURCE=..\..\utils\ddf2vc\ddf_game.c
# End Source File
# Begin Source File

SOURCE=..\..\utils\ddf2vc\ddf_lang.c
# End Source File
# Begin Source File

SOURCE=..\..\utils\ddf2vc\ddf_levl.c
# End Source File
# Begin Source File

SOURCE=..\..\utils\ddf2vc\ddf_line.c
# End Source File
# Begin Source File

SOURCE=..\..\utils\ddf2vc\ddf_main.c
# End Source File
# Begin Source File

SOURCE=..\..\utils\ddf2vc\ddf_mobj.c
# End Source File
# Begin Source File

SOURCE=..\..\utils\ddf2vc\ddf_mus.c
# End Source File
# Begin Source File

SOURCE=..\..\utils\ddf2vc\ddf_sect.c
# End Source File
# Begin Source File

SOURCE=..\..\utils\ddf2vc\ddf_sfx.c
# End Source File
# Begin Source File

SOURCE=..\..\utils\ddf2vc\ddf_stat.c
# End Source File
# Begin Source File

SOURCE=..\..\utils\ddf2vc\ddf_swth.c
# End Source File
# Begin Source File

SOURCE=..\..\utils\ddf2vc\ddf_weap.c
# End Source File
# Begin Source File

SOURCE=..\..\utils\ddf2vc\m_argv.c
# End Source File
# Begin Source File

SOURCE=..\..\utils\ddf2vc\m_crc32.c
# End Source File
# Begin Source File

SOURCE=..\..\utils\ddf2vc\scripts.c
# End Source File
# Begin Source File

SOURCE=..\..\utils\ddf2vc\vc_mobj.c
# End Source File
# Begin Source File

SOURCE=..\..\utils\ddf2vc\vc_weapn.c
# End Source File
# Begin Source File

SOURCE=..\..\utils\ddf2vc\z_zone.c
# End Source File
# End Group
# Begin Group "Header Files"

# PROP Default_Filter "h;hpp;hxx;hm;inl"
# Begin Source File

SOURCE=..\..\utils\ddf2vc\ddf_locl.h
# End Source File
# Begin Source File

SOURCE=..\..\utils\ddf2vc\ddf_main.h
# End Source File
# Begin Source File

SOURCE=..\..\utils\ddf2vc\dm_data.h
# End Source File
# Begin Source File

SOURCE=..\..\utils\ddf2vc\dm_defs.h
# End Source File
# Begin Source File

SOURCE=..\..\utils\ddf2vc\dm_state.h
# End Source File
# Begin Source File

SOURCE=..\..\utils\ddf2vc\dm_type.h
# End Source File
# Begin Source File

SOURCE=..\..\utils\ddf2vc\dstrings.h
# End Source File
# Begin Source File

SOURCE=..\..\utils\ddf2vc\e_event.h
# End Source File
# Begin Source File

SOURCE=..\..\utils\ddf2vc\e_net.h
# End Source File
# Begin Source File

SOURCE=..\..\utils\ddf2vc\e_player.h
# End Source File
# Begin Source File

SOURCE=..\..\utils\ddf2vc\e_search.h
# End Source File
# Begin Source File

SOURCE=..\..\utils\ddf2vc\e_think.h
# End Source File
# Begin Source File

SOURCE=..\..\utils\ddf2vc\e_ticcmd.h
# End Source File
# Begin Source File

SOURCE=..\..\utils\ddf2vc\djgpp\i_compen.h
# End Source File
# Begin Source File

SOURCE=..\..\utils\ddf2vc\i_defs.h
# End Source File
# Begin Source File

SOURCE=..\..\utils\ddf2vc\i_system.h
# End Source File
# Begin Source File

SOURCE=..\..\utils\ddf2vc\lu_math.h
# End Source File
# Begin Source File

SOURCE=..\..\utils\ddf2vc\m_argv.h
# End Source File
# Begin Source File

SOURCE=..\..\utils\ddf2vc\m_fixed.h
# End Source File
# Begin Source File

SOURCE=..\..\utils\ddf2vc\m_inline.h
# End Source File
# Begin Source File

SOURCE=..\..\utils\ddf2vc\m_math.h
# End Source File
# Begin Source File

SOURCE=..\..\utils\ddf2vc\m_misc.h
# End Source File
# Begin Source File

SOURCE=..\..\utils\ddf2vc\m_random.h
# End Source File
# Begin Source File

SOURCE=..\..\utils\ddf2vc\p_action.h
# End Source File
# Begin Source File

SOURCE=..\..\utils\ddf2vc\p_local.h
# End Source File
# Begin Source File

SOURCE=..\..\utils\ddf2vc\p_mobj.h
# End Source File
# Begin Source File

SOURCE=..\..\utils\ddf2vc\p_spec.h
# End Source File
# Begin Source File

SOURCE=..\..\utils\ddf2vc\p_weapon.h
# End Source File
# Begin Source File

SOURCE=..\..\utils\ddf2vc\r_bsp.h
# End Source File
# Begin Source File

SOURCE=..\..\utils\ddf2vc\r_data.h
# End Source File
# Begin Source File

SOURCE=..\..\utils\ddf2vc\r_defs.h
# End Source File
# Begin Source File

SOURCE=..\..\utils\ddf2vc\r_local.h
# End Source File
# Begin Source File

SOURCE=..\..\utils\ddf2vc\r_main.h
# End Source File
# Begin Source File

SOURCE=..\..\utils\ddf2vc\r_plane.h
# End Source File
# Begin Source File

SOURCE=..\..\utils\ddf2vc\r_segs.h
# End Source File
# Begin Source File

SOURCE=..\..\utils\ddf2vc\r_state.h
# End Source File
# Begin Source File

SOURCE=..\..\utils\ddf2vc\r_things.h
# End Source File
# Begin Source File

SOURCE=..\..\utils\ddf2vc\s_sound.h
# End Source File
# Begin Source File

SOURCE=..\..\utils\ddf2vc\v_res.h
# End Source File
# Begin Source File

SOURCE=..\..\utils\ddf2vc\v_screen.h
# End Source File
# Begin Source File

SOURCE=..\..\utils\ddf2vc\v_video1.h
# End Source File
# Begin Source File

SOURCE=..\..\utils\ddf2vc\vc.h
# End Source File
# Begin Source File

SOURCE=..\..\utils\ddf2vc\w_image.h
# End Source File
# Begin Source File

SOURCE=..\..\utils\ddf2vc\w_textur.h
# End Source File
# Begin Source File

SOURCE=..\..\utils\ddf2vc\w_wad.h
# End Source File
# Begin Source File

SOURCE=..\..\utils\ddf2vc\z_zone.h
# End Source File
# End Group
# Begin Group "Resource Files"

# PROP Default_Filter "ico;cur;bmp;dlg;rc2;rct;bin;rgs;gif;jpg;jpeg;jpe"
# End Group
# End Target
# End Project
