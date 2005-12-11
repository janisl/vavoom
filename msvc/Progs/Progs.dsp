# Microsoft Developer Studio Project File - Name="Progs" - Package Owner=<4>
# Microsoft Developer Studio Generated Build File, Format Version 6.00
# ** DO NOT EDIT **

# TARGTYPE "Win32 (x86) Console Application" 0x0103

CFG=PROGS - WIN32 RELEASE
!MESSAGE This is not a valid makefile. To build this project using NMAKE,
!MESSAGE use the Export Makefile command and run
!MESSAGE 
!MESSAGE NMAKE /f "Progs.mak".
!MESSAGE 
!MESSAGE You can specify a configuration when running NMAKE
!MESSAGE by defining the macro CFG on the command line. For example:
!MESSAGE 
!MESSAGE NMAKE /f "Progs.mak" CFG="PROGS - WIN32 RELEASE"
!MESSAGE 
!MESSAGE Possible choices for configuration are:
!MESSAGE 
!MESSAGE "Progs - Win32 Release" (based on "Win32 (x86) Console Application")
!MESSAGE "Progs - Win32 Debug" (based on "Win32 (x86) Console Application")
!MESSAGE 

# Begin Project
# PROP AllowPerConfigDependencies 0
# PROP Scc_ProjName ""
# PROP Scc_LocalPath ""
CPP=cl.exe
RSC=rc.exe

!IF  "$(CFG)" == "Progs - Win32 Release"

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
# ADD CPP /nologo /MT /W3 /GX /O2 /D "WIN32" /D "NDEBUG" /D "_CONSOLE" /D "_MBCS" /YX /FD /c
# ADD BASE RSC /l 0x426 /d "NDEBUG"
# ADD RSC /l 0x426 /d "NDEBUG"
BSC32=bscmake.exe
# ADD BASE BSC32 /nologo
# ADD BSC32 /nologo
LINK32=link.exe
# ADD BASE LINK32 kernel32.lib user32.lib gdi32.lib winspool.lib comdlg32.lib advapi32.lib shell32.lib ole32.lib oleaut32.lib uuid.lib odbc32.lib odbccp32.lib kernel32.lib user32.lib gdi32.lib winspool.lib comdlg32.lib advapi32.lib shell32.lib ole32.lib oleaut32.lib uuid.lib odbc32.lib odbccp32.lib /nologo /subsystem:console /machine:I386
# ADD LINK32 kernel32.lib user32.lib gdi32.lib winspool.lib comdlg32.lib advapi32.lib shell32.lib ole32.lib oleaut32.lib uuid.lib odbc32.lib odbccp32.lib kernel32.lib user32.lib gdi32.lib winspool.lib comdlg32.lib advapi32.lib shell32.lib ole32.lib oleaut32.lib uuid.lib odbc32.lib odbccp32.lib /nologo /subsystem:console /machine:I386

!ELSEIF  "$(CFG)" == "Progs - Win32 Debug"

# PROP BASE Use_MFC 0
# PROP BASE Use_Debug_Libraries 0
# PROP BASE Output_Dir "Progs___Win32_Debug"
# PROP BASE Intermediate_Dir "Progs___Win32_Debug"
# PROP BASE Target_Dir ""
# PROP Use_MFC 0
# PROP Use_Debug_Libraries 0
# PROP Output_Dir "Progs___Win32_Debug"
# PROP Intermediate_Dir "Progs___Win32_Debug"
# PROP Ignore_Export_Lib 0
# PROP Target_Dir ""
# ADD BASE CPP /nologo /W3 /GX /O2 /D "WIN32" /D "NDEBUG" /D "_CONSOLE" /D "_MBCS" /YX /FD /c
# ADD CPP /nologo /MTd /W3 /GX /O2 /D "WIN32" /D "NDEBUG" /D "_CONSOLE" /D "_MBCS" /YX /FD /c
# ADD BASE RSC /l 0x426 /d "NDEBUG"
# ADD RSC /l 0x426 /d "NDEBUG"
BSC32=bscmake.exe
# ADD BASE BSC32 /nologo
# ADD BSC32 /nologo
LINK32=link.exe
# ADD BASE LINK32 kernel32.lib user32.lib gdi32.lib winspool.lib comdlg32.lib advapi32.lib shell32.lib ole32.lib oleaut32.lib uuid.lib odbc32.lib odbccp32.lib kernel32.lib user32.lib gdi32.lib winspool.lib comdlg32.lib advapi32.lib shell32.lib ole32.lib oleaut32.lib uuid.lib odbc32.lib odbccp32.lib /nologo /subsystem:console /machine:I386
# ADD LINK32 kernel32.lib user32.lib gdi32.lib winspool.lib comdlg32.lib advapi32.lib shell32.lib ole32.lib oleaut32.lib uuid.lib odbc32.lib odbccp32.lib kernel32.lib user32.lib gdi32.lib winspool.lib comdlg32.lib advapi32.lib shell32.lib ole32.lib oleaut32.lib uuid.lib odbc32.lib odbccp32.lib /nologo /subsystem:console /machine:I386

!ENDIF 

# Begin Target

# Name "Progs - Win32 Release"
# Name "Progs - Win32 Debug"
# Begin Group "Common"

# PROP Default_Filter "vc"
# Begin Source File

SOURCE=..\..\progs\common\builtins.vc
# End Source File
# Begin Source File

SOURCE=..\..\progs\common\client.vc
# End Source File
# Begin Source File

SOURCE=..\..\progs\common\clvars.vc
# End Source File
# Begin Source File

SOURCE=..\..\progs\common\defs.vc
# End Source File
# Begin Source File

SOURCE=..\..\progs\common\entity.vc
# End Source File
# Begin Source File

SOURCE=..\..\progs\common\keys.vc
# End Source File
# Begin Source File

SOURCE=..\..\progs\common\protocol.vc
# End Source File
# Begin Source File

SOURCE=..\..\progs\common\server.vc
# End Source File
# Begin Source File

SOURCE=..\..\progs\common\svvars.vc
# End Source File
# Begin Source File

SOURCE=..\..\progs\common\types.vc
# End Source File
# Begin Source File

SOURCE=..\..\progs\common\uitypes.vc
# End Source File
# Begin Source File

SOURCE=..\..\progs\common\utils.vc
# End Source File
# End Group
# Begin Group "Doom Shared"

# PROP Default_Filter ""
# Begin Source File

SOURCE=..\..\progs\doom\doomdefs.vc
# End Source File
# Begin Source File

SOURCE=..\..\progs\doom\gendefs.vc
# End Source File
# End Group
# Begin Group "Doom Client"

# PROP Default_Filter ""
# Begin Source File

SOURCE=..\..\progs\doom\client\border.vc
# End Source File
# Begin Source File

SOURCE=..\..\progs\doom\client\clmain.vc
# End Source File
# Begin Source File

SOURCE=..\..\progs\doom\client\clprogs.vc

!IF  "$(CFG)" == "Progs - Win32 Release"

# PROP Intermediate_Dir "../../basev/doom1/progs"
# Begin Custom Build - Compiling $(InputPath)...
IntDir=.\../../basev/doom1/progs
InputPath=..\..\progs\doom\client\clprogs.vc
InputName=clprogs

"$(IntDir)/$(InputName).dat" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
	vcc -I../../progs/common $(InputPath) $(IntDir)/$(InputName).dat

# End Custom Build

!ELSEIF  "$(CFG)" == "Progs - Win32 Debug"

# PROP BASE Intermediate_Dir "../../basev/doom1/progs"
# PROP Intermediate_Dir "../../basev/doom1/progs"
# Begin Custom Build - Compiling $(InputPath)...
IntDir=.\../../basev/doom1/progs
InputPath=..\..\progs\doom\client\clprogs.vc
InputName=clprogs

"$(IntDir)/$(InputName).dat" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
	vcc -I../../progs/common $(InputPath) $(IntDir)/$(InputName).dat

# End Custom Build

!ENDIF 

# End Source File
# Begin Source File

SOURCE=..\..\progs\doom\client\effects.vc
# End Source File
# Begin Source File

SOURCE=..\..\progs\doom\client\finale.vc
# End Source File
# Begin Source File

SOURCE=..\..\progs\doom\client\imission.vc
# End Source File
# Begin Source File

SOURCE=..\..\progs\doom\client\m_api.vc
# End Source File
# Begin Source File

SOURCE=..\..\progs\doom\client\m_ctrl.vc
# End Source File
# Begin Source File

SOURCE=..\..\progs\doom\client\m_epi.vc
# End Source File
# Begin Source File

SOURCE=..\..\progs\doom\client\m_help.vc
# End Source File
# Begin Source File

SOURCE=..\..\progs\doom\client\m_join.vc
# End Source File
# Begin Source File

SOURCE=..\..\progs\doom\client\m_load.vc
# End Source File
# Begin Source File

SOURCE=..\..\progs\doom\client\m_main.vc
# End Source File
# Begin Source File

SOURCE=..\..\progs\doom\client\m_menu.vc
# End Source File
# Begin Source File

SOURCE=..\..\progs\doom\client\m_mouse.vc
# End Source File
# Begin Source File

SOURCE=..\..\progs\doom\client\m_multi.vc
# End Source File
# Begin Source File

SOURCE=..\..\progs\doom\client\m_new.vc
# End Source File
# Begin Source File

SOURCE=..\..\progs\doom\client\m_option.vc
# End Source File
# Begin Source File

SOURCE=..\..\progs\doom\client\m_psetup.vc
# End Source File
# Begin Source File

SOURCE=..\..\progs\doom\client\m_quit.vc
# End Source File
# Begin Source File

SOURCE=..\..\progs\doom\client\m_resol.vc
# End Source File
# Begin Source File

SOURCE=..\..\progs\doom\client\m_save.vc
# End Source File
# Begin Source File

SOURCE=..\..\progs\doom\client\m_single.vc
# End Source File
# Begin Source File

SOURCE=..\..\progs\doom\client\m_skill.vc
# End Source File
# Begin Source File

SOURCE=..\..\progs\doom\client\m_sound.vc
# End Source File
# Begin Source File

SOURCE=..\..\progs\doom\client\m_video.vc
# End Source File
# Begin Source File

SOURCE=..\..\progs\doom\client\msgbox.vc
# End Source File
# Begin Source File

SOURCE=..\..\progs\doom\client\statbar.vc
# End Source File
# Begin Source File

SOURCE=..\..\progs\doom\client\title.vc
# End Source File
# End Group
# Begin Group "Doom Server"

# PROP Default_Filter ""
# Begin Source File

SOURCE=..\..\progs\doom\server\ai_subs.vc
# End Source File
# Begin Source File

SOURCE=..\..\progs\doom\server\arach.vc
# End Source File
# Begin Source File

SOURCE=..\..\progs\doom\server\archvile.vc
# End Source File
# Begin Source File

SOURCE=..\..\progs\doom\server\bot.vc
# End Source File
# Begin Source File

SOURCE=..\..\progs\doom\server\brain.vc
# End Source File
# Begin Source File

SOURCE=..\..\progs\doom\server\caco.vc
# End Source File
# Begin Source File

SOURCE=..\..\progs\doom\server\ceilings.vc
# End Source File
# Begin Source File

SOURCE=..\..\progs\doom\server\cheat.vc
# End Source File
# Begin Source File

SOURCE=..\..\progs\doom\server\cyber.vc
# End Source File
# Begin Source File

SOURCE=..\..\progs\doom\server\demon.vc
# End Source File
# Begin Source File

SOURCE=..\..\progs\doom\server\doors.vc
# End Source File
# Begin Source File

SOURCE=..\..\progs\doom\server\enemy.vc
# End Source File
# Begin Source File

SOURCE=..\..\progs\doom\server\floors.vc
# End Source File
# Begin Source File

SOURCE=..\..\progs\doom\server\grlaunch.vc
# End Source File
# Begin Source File

SOURCE=..\..\progs\doom\server\heavy.vc
# End Source File
# Begin Source File

SOURCE=..\..\progs\doom\server\imp.vc
# End Source File
# Begin Source File

SOURCE=..\..\progs\doom\server\inter.vc
# End Source File
# Begin Source File

SOURCE=..\..\progs\doom\server\keen.vc
# End Source File
# Begin Source File

SOURCE=..\..\progs\doom\server\knight.vc
# End Source File
# Begin Source File

SOURCE=..\..\progs\doom\server\lights.vc
# End Source File
# Begin Source File

SOURCE=..\..\progs\doom\server\lostsoul.vc
# End Source File
# Begin Source File

SOURCE=..\..\progs\doom\server\mancubus.vc
# End Source File
# Begin Source File

SOURCE=..\..\progs\doom\server\map.vc
# End Source File
# Begin Source File

SOURCE=..\..\progs\doom\server\mobj.vc
# End Source File
# Begin Source File

SOURCE=..\..\progs\doom\server\mobjinfo.vc
# End Source File
# Begin Source File

SOURCE=..\..\progs\doom\server\move.vc
# End Source File
# Begin Source File

SOURCE=..\..\progs\doom\server\painelem.vc
# End Source File
# Begin Source File

SOURCE=..\..\progs\doom\server\physics.vc
# End Source File
# Begin Source File

SOURCE=..\..\progs\doom\server\plats.vc
# End Source File
# Begin Source File

SOURCE=..\..\progs\doom\server\polyobj.vc
# End Source File
# Begin Source File

SOURCE=..\..\progs\doom\server\revenant.vc
# End Source File
# Begin Source File

SOURCE=..\..\progs\doom\server\savegame.vc
# End Source File
# Begin Source File

SOURCE=..\..\progs\doom\server\shotguy.vc
# End Source File
# Begin Source File

SOURCE=..\..\progs\doom\server\spec.vc
# End Source File
# Begin Source File

SOURCE=..\..\progs\doom\server\specutl.vc
# End Source File
# Begin Source File

SOURCE=..\..\progs\doom\server\spider.vc
# End Source File
# Begin Source File

SOURCE=..\..\progs\doom\server\states.vc
# End Source File
# Begin Source File

SOURCE=..\..\progs\doom\server\strings.vc
# End Source File
# Begin Source File

SOURCE=..\..\progs\doom\server\svdefs.vc
# End Source File
# Begin Source File

SOURCE=..\..\progs\doom\server\svmain.vc
# End Source File
# Begin Source File

SOURCE=..\..\progs\doom\server\svprogs.vc

!IF  "$(CFG)" == "Progs - Win32 Release"

# PROP Intermediate_Dir "../../basev/doom1/progs"
# Begin Custom Build - Compiling $(InputPath)...
IntDir=.\../../basev/doom1/progs
InputPath=..\..\progs\doom\server\svprogs.vc
InputName=svprogs

"$(IntDir)/$(InputName).dat" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
	vcc -I../../progs/common $(InputPath) $(IntDir)/$(InputName).dat

# End Custom Build

!ELSEIF  "$(CFG)" == "Progs - Win32 Debug"

# PROP BASE Intermediate_Dir "../../basev/doom1/progs"
# PROP Intermediate_Dir "../../basev/doom1/progs"
# Begin Custom Build - Compiling $(InputPath)...
IntDir=.\../../basev/doom1/progs
InputPath=..\..\progs\doom\server\svprogs.vc
InputName=svprogs

"$(IntDir)/$(InputName).dat" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
	vcc -I../../progs/common $(InputPath) $(IntDir)/$(InputName).dat

# End Custom Build

!ENDIF 

# End Source File
# Begin Source File

SOURCE=..\..\progs\doom\server\teleport.vc
# End Source File
# Begin Source File

SOURCE=..\..\progs\doom\server\user.vc
# End Source File
# Begin Source File

SOURCE=..\..\progs\doom\server\usersubs.vc
# End Source File
# Begin Source File

SOURCE=..\..\progs\doom\server\weapons.vc
# End Source File
# Begin Source File

SOURCE=..\..\progs\doom\server\world.vc
# End Source File
# Begin Source File

SOURCE=..\..\progs\doom\server\wpninfo.vc
# End Source File
# Begin Source File

SOURCE=..\..\progs\doom\server\xlat.vc
# End Source File
# Begin Source File

SOURCE=..\..\progs\doom\server\zombiemn.vc
# End Source File
# End Group
# Begin Group "Doom2"

# PROP Default_Filter ""
# Begin Source File

SOURCE=..\..\progs\doom2\clprogs.vc

!IF  "$(CFG)" == "Progs - Win32 Release"

# PROP Intermediate_Dir "../../basev/doom2/progs"
# Begin Custom Build - Compiling $(InputPath)...
IntDir=.\../../basev/doom2/progs
InputPath=..\..\progs\doom2\clprogs.vc
InputName=clprogs

"$(IntDir)/$(InputName).dat" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
	vcc -I../../progs/common $(InputPath) $(IntDir)/$(InputName).dat

# End Custom Build

!ELSEIF  "$(CFG)" == "Progs - Win32 Debug"

# PROP BASE Intermediate_Dir "../../basev/doom2/progs"
# PROP Intermediate_Dir "../../basev/doom2/progs"
# Begin Custom Build - Compiling $(InputPath)...
IntDir=.\../../basev/doom2/progs
InputPath=..\..\progs\doom2\clprogs.vc
InputName=clprogs

"$(IntDir)/$(InputName).dat" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
	vcc -I../../progs/common $(InputPath) $(IntDir)/$(InputName).dat

# End Custom Build

!ENDIF 

# End Source File
# Begin Source File

SOURCE=..\..\progs\doom2\svprogs.vc

!IF  "$(CFG)" == "Progs - Win32 Release"

# PROP Intermediate_Dir "../../basev/doom2/progs"
# Begin Custom Build - Compiling $(InputPath)...
IntDir=.\../../basev/doom2/progs
InputPath=..\..\progs\doom2\svprogs.vc
InputName=svprogs

"$(IntDir)/$(InputName).dat" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
	vcc -I../../progs/common $(InputPath) $(IntDir)/$(InputName).dat

# End Custom Build

!ELSEIF  "$(CFG)" == "Progs - Win32 Debug"

# PROP BASE Intermediate_Dir "../../basev/doom2/progs"
# PROP Intermediate_Dir "../../basev/doom2/progs"
# Begin Custom Build - Compiling $(InputPath)...
IntDir=.\../../basev/doom2/progs
InputPath=..\..\progs\doom2\svprogs.vc
InputName=svprogs

"$(IntDir)/$(InputName).dat" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
	vcc -I../../progs/common $(InputPath) $(IntDir)/$(InputName).dat

# End Custom Build

!ENDIF 

# End Source File
# End Group
# Begin Group "Heretic Shared"

# PROP Default_Filter ""
# Begin Source File

SOURCE=..\..\progs\heretic\herdefs.vc
# End Source File
# End Group
# Begin Group "Heretic Client"

# PROP Default_Filter ""
# Begin Source File

SOURCE=..\..\progs\heretic\client\border.vc
# End Source File
# Begin Source File

SOURCE=..\..\progs\heretic\client\clmain.vc
# End Source File
# Begin Source File

SOURCE=..\..\progs\heretic\client\clprogs.vc

!IF  "$(CFG)" == "Progs - Win32 Release"

# PROP Intermediate_Dir "../../basev/heretic/progs"
# Begin Custom Build - Compiling $(InputPath)...
IntDir=.\../../basev/heretic/progs
InputPath=..\..\progs\heretic\client\clprogs.vc
InputName=clprogs

"$(IntDir)/$(InputName).dat" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
	vcc -I../../progs/common $(InputPath) $(IntDir)/$(InputName).dat

# End Custom Build

!ELSEIF  "$(CFG)" == "Progs - Win32 Debug"

# PROP BASE Intermediate_Dir "../../basev/heretic/progs"
# PROP Intermediate_Dir "../../basev/heretic/progs"
# Begin Custom Build - Compiling $(InputPath)...
IntDir=.\../../basev/heretic/progs
InputPath=..\..\progs\heretic\client\clprogs.vc
InputName=clprogs

"$(IntDir)/$(InputName).dat" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
	vcc -I../../progs/common $(InputPath) $(IntDir)/$(InputName).dat

# End Custom Build

!ENDIF 

# End Source File
# Begin Source File

SOURCE=..\..\progs\heretic\client\effects.vc
# End Source File
# Begin Source File

SOURCE=..\..\progs\heretic\client\finale.vc
# End Source File
# Begin Source File

SOURCE=..\..\progs\heretic\client\imission.vc
# End Source File
# Begin Source File

SOURCE=..\..\progs\heretic\client\m_api.vc
# End Source File
# Begin Source File

SOURCE=..\..\progs\heretic\client\m_ctrl.vc
# End Source File
# Begin Source File

SOURCE=..\..\progs\heretic\client\m_epi.vc
# End Source File
# Begin Source File

SOURCE=..\..\progs\heretic\client\m_help.vc
# End Source File
# Begin Source File

SOURCE=..\..\progs\heretic\client\m_join.vc
# End Source File
# Begin Source File

SOURCE=..\..\progs\heretic\client\m_load.vc
# End Source File
# Begin Source File

SOURCE=..\..\progs\heretic\client\m_main.vc
# End Source File
# Begin Source File

SOURCE=..\..\progs\heretic\client\m_menu.vc
# End Source File
# Begin Source File

SOURCE=..\..\progs\heretic\client\m_mouse.vc
# End Source File
# Begin Source File

SOURCE=..\..\progs\heretic\client\m_multi.vc
# End Source File
# Begin Source File

SOURCE=..\..\progs\heretic\client\m_new.vc
# End Source File
# Begin Source File

SOURCE=..\..\progs\heretic\client\m_option.vc
# End Source File
# Begin Source File

SOURCE=..\..\progs\heretic\client\m_psetup.vc
# End Source File
# Begin Source File

SOURCE=..\..\progs\heretic\client\m_quit.vc
# End Source File
# Begin Source File

SOURCE=..\..\progs\heretic\client\m_resol.vc
# End Source File
# Begin Source File

SOURCE=..\..\progs\heretic\client\m_save.vc
# End Source File
# Begin Source File

SOURCE=..\..\progs\heretic\client\m_single.vc
# End Source File
# Begin Source File

SOURCE=..\..\progs\heretic\client\m_skill.vc
# End Source File
# Begin Source File

SOURCE=..\..\progs\heretic\client\m_sound.vc
# End Source File
# Begin Source File

SOURCE=..\..\progs\heretic\client\m_video.vc
# End Source File
# Begin Source File

SOURCE=..\..\progs\heretic\client\msgbox.vc
# End Source File
# Begin Source File

SOURCE=..\..\progs\heretic\client\statbar.vc
# End Source File
# Begin Source File

SOURCE=..\..\progs\heretic\client\title.vc
# End Source File
# End Group
# Begin Group "Heretic Server"

# PROP Default_Filter ""
# Begin Source File

SOURCE=..\..\progs\heretic\server\action.vc
# End Source File
# Begin Source File

SOURCE=..\..\progs\heretic\server\ai_subs.vc
# End Source File
# Begin Source File

SOURCE=..\..\progs\heretic\server\ambient.vc
# End Source File
# Begin Source File

SOURCE=..\..\progs\heretic\server\beast.vc
# End Source File
# Begin Source File

SOURCE=..\..\progs\heretic\server\bot.vc
# End Source File
# Begin Source File

SOURCE=..\..\progs\heretic\server\ceilings.vc
# End Source File
# Begin Source File

SOURCE=..\..\progs\heretic\server\cheat.vc
# End Source File
# Begin Source File

SOURCE=..\..\progs\heretic\server\chicken.vc
# End Source File
# Begin Source File

SOURCE=..\..\progs\heretic\server\clink.vc
# End Source File
# Begin Source File

SOURCE=..\..\progs\heretic\server\doors.vc
# End Source File
# Begin Source File

SOURCE=..\..\progs\heretic\server\enemy.vc
# End Source File
# Begin Source File

SOURCE=..\..\progs\heretic\server\floors.vc
# End Source File
# Begin Source File

SOURCE=..\..\progs\heretic\server\head.vc
# End Source File
# Begin Source File

SOURCE=..\..\progs\heretic\server\imp.vc
# End Source File
# Begin Source File

SOURCE=..\..\progs\heretic\server\inter.vc
# End Source File
# Begin Source File

SOURCE=..\..\progs\heretic\server\invent.vc
# End Source File
# Begin Source File

SOURCE=..\..\progs\heretic\server\knight.vc
# End Source File
# Begin Source File

SOURCE=..\..\progs\heretic\server\lights.vc
# End Source File
# Begin Source File

SOURCE=..\..\progs\heretic\server\map.vc
# End Source File
# Begin Source File

SOURCE=..\..\progs\heretic\server\minotaur.vc
# End Source File
# Begin Source File

SOURCE=..\..\progs\heretic\server\missile.vc
# End Source File
# Begin Source File

SOURCE=..\..\progs\heretic\server\mobj.vc
# End Source File
# Begin Source File

SOURCE=..\..\progs\heretic\server\mobjinfo.vc
# End Source File
# Begin Source File

SOURCE=..\..\progs\heretic\server\move.vc
# End Source File
# Begin Source File

SOURCE=..\..\progs\heretic\server\mummy.vc
# End Source File
# Begin Source File

SOURCE=..\..\progs\heretic\server\physics.vc
# End Source File
# Begin Source File

SOURCE=..\..\progs\heretic\server\plats.vc
# End Source File
# Begin Source File

SOURCE=..\..\progs\heretic\server\pod.vc
# End Source File
# Begin Source File

SOURCE=..\..\progs\heretic\server\polyobj.vc
# End Source File
# Begin Source File

SOURCE=..\..\progs\heretic\server\savegame.vc
# End Source File
# Begin Source File

SOURCE=..\..\progs\heretic\server\snake.vc
# End Source File
# Begin Source File

SOURCE=..\..\progs\heretic\server\sorcerer.vc
# End Source File
# Begin Source File

SOURCE=..\..\progs\heretic\server\spec.vc
# End Source File
# Begin Source File

SOURCE=..\..\progs\heretic\server\specutl.vc
# End Source File
# Begin Source File

SOURCE=..\..\progs\heretic\server\states.vc
# End Source File
# Begin Source File

SOURCE=..\..\progs\heretic\server\svdefs.vc
# End Source File
# Begin Source File

SOURCE=..\..\progs\heretic\server\svmain.vc
# End Source File
# Begin Source File

SOURCE=..\..\progs\heretic\server\svprogs.vc

!IF  "$(CFG)" == "Progs - Win32 Release"

# PROP Intermediate_Dir "../../basev/heretic/progs"
# Begin Custom Build - Compiling $(InputPath)...
IntDir=.\../../basev/heretic/progs
InputPath=..\..\progs\heretic\server\svprogs.vc
InputName=svprogs

"$(IntDir)/$(InputName).dat" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
	vcc -I../../progs/common $(InputPath) $(IntDir)/$(InputName).dat

# End Custom Build

!ELSEIF  "$(CFG)" == "Progs - Win32 Debug"

# PROP BASE Intermediate_Dir "../../basev/heretic/progs"
# PROP Intermediate_Dir "../../basev/heretic/progs"
# Begin Custom Build - Compiling $(InputPath)...
IntDir=.\../../basev/heretic/progs
InputPath=..\..\progs\heretic\server\svprogs.vc
InputName=svprogs

"$(IntDir)/$(InputName).dat" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
	vcc -I../../progs/common $(InputPath) $(IntDir)/$(InputName).dat

# End Custom Build

!ENDIF 

# End Source File
# Begin Source File

SOURCE=..\..\progs\heretic\server\teleglit.vc
# End Source File
# Begin Source File

SOURCE=..\..\progs\heretic\server\teleport.vc
# End Source File
# Begin Source File

SOURCE=..\..\progs\heretic\server\user.vc
# End Source File
# Begin Source File

SOURCE=..\..\progs\heretic\server\usersubs.vc
# End Source File
# Begin Source File

SOURCE=..\..\progs\heretic\server\volcano.vc
# End Source File
# Begin Source File

SOURCE=..\..\progs\heretic\server\weapon.vc
# End Source File
# Begin Source File

SOURCE=..\..\progs\heretic\server\wizard.vc
# End Source File
# Begin Source File

SOURCE=..\..\progs\heretic\server\world.vc
# End Source File
# Begin Source File

SOURCE=..\..\progs\heretic\server\wpninfo.vc
# End Source File
# Begin Source File

SOURCE=..\..\progs\heretic\server\xlat.vc
# End Source File
# End Group
# Begin Group "Hexen Shared"

# PROP Default_Filter ""
# Begin Source File

SOURCE=..\..\progs\hexen\hexndefs.vc
# End Source File
# End Group
# Begin Group "Hexen Client"

# PROP Default_Filter ""
# Begin Source File

SOURCE=..\..\progs\hexen\client\border.vc
# End Source File
# Begin Source File

SOURCE=..\..\progs\hexen\client\clmain.vc
# End Source File
# Begin Source File

SOURCE=..\..\progs\hexen\client\clprogs.vc

!IF  "$(CFG)" == "Progs - Win32 Release"

# PROP Intermediate_Dir "../../basev/hexen/progs"
# Begin Custom Build - Compiling $(InputPath)...
IntDir=.\../../basev/hexen/progs
InputPath=..\..\progs\hexen\client\clprogs.vc
InputName=clprogs

"$(IntDir)/$(InputName).dat" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
	vcc -I../../progs/common $(InputPath) $(IntDir)/$(InputName).dat

# End Custom Build

!ELSEIF  "$(CFG)" == "Progs - Win32 Debug"

# PROP BASE Intermediate_Dir "../../basev/hexen/progs"
# PROP Intermediate_Dir "../../basev/hexen/progs"
# Begin Custom Build - Compiling $(InputPath)...
IntDir=.\../../basev/hexen/progs
InputPath=..\..\progs\hexen\client\clprogs.vc
InputName=clprogs

"$(IntDir)/$(InputName).dat" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
	vcc -I../../progs/common $(InputPath) $(IntDir)/$(InputName).dat

# End Custom Build

!ENDIF 

# End Source File
# Begin Source File

SOURCE=..\..\progs\hexen\client\effects.vc
# End Source File
# Begin Source File

SOURCE=..\..\progs\hexen\client\finale.vc
# End Source File
# Begin Source File

SOURCE=..\..\progs\hexen\client\imission.vc
# End Source File
# Begin Source File

SOURCE=..\..\progs\hexen\client\m_api.vc
# End Source File
# Begin Source File

SOURCE=..\..\progs\hexen\client\m_class.vc
# End Source File
# Begin Source File

SOURCE=..\..\progs\hexen\client\m_ctrl.vc
# End Source File
# Begin Source File

SOURCE=..\..\progs\hexen\client\m_help.vc
# End Source File
# Begin Source File

SOURCE=..\..\progs\hexen\client\m_join.vc
# End Source File
# Begin Source File

SOURCE=..\..\progs\hexen\client\m_load.vc
# End Source File
# Begin Source File

SOURCE=..\..\progs\hexen\client\m_main.vc
# End Source File
# Begin Source File

SOURCE=..\..\progs\hexen\client\m_menu.vc
# End Source File
# Begin Source File

SOURCE=..\..\progs\hexen\client\m_mouse.vc
# End Source File
# Begin Source File

SOURCE=..\..\progs\hexen\client\m_multi.vc
# End Source File
# Begin Source File

SOURCE=..\..\progs\hexen\client\m_new.vc
# End Source File
# Begin Source File

SOURCE=..\..\progs\hexen\client\m_option.vc
# End Source File
# Begin Source File

SOURCE=..\..\progs\hexen\client\m_psetup.vc
# End Source File
# Begin Source File

SOURCE=..\..\progs\hexen\client\m_quit.vc
# End Source File
# Begin Source File

SOURCE=..\..\progs\hexen\client\m_resol.vc
# End Source File
# Begin Source File

SOURCE=..\..\progs\hexen\client\m_save.vc
# End Source File
# Begin Source File

SOURCE=..\..\progs\hexen\client\m_single.vc
# End Source File
# Begin Source File

SOURCE=..\..\progs\hexen\client\m_skill.vc
# End Source File
# Begin Source File

SOURCE=..\..\progs\hexen\client\m_sound.vc
# End Source File
# Begin Source File

SOURCE=..\..\progs\hexen\client\m_video.vc
# End Source File
# Begin Source File

SOURCE=..\..\progs\hexen\client\msgbox.vc
# End Source File
# Begin Source File

SOURCE=..\..\progs\hexen\client\statbar.vc
# End Source File
# Begin Source File

SOURCE=..\..\progs\hexen\client\title.vc
# End Source File
# End Group
# Begin Group "Hexen Server"

# PROP Default_Filter ""
# Begin Source File

SOURCE=..\..\progs\hexen\server\action.vc
# End Source File
# Begin Source File

SOURCE=..\..\progs\hexen\server\ai_subs.vc
# End Source File
# Begin Source File

SOURCE=..\..\progs\hexen\server\batspawn.vc
# End Source File
# Begin Source File

SOURCE=..\..\progs\hexen\server\bell.vc
# End Source File
# Begin Source File

SOURCE=..\..\progs\hexen\server\bishop.vc
# End Source File
# Begin Source File

SOURCE=..\..\progs\hexen\server\bridge.vc
# End Source File
# Begin Source File

SOURCE=..\..\progs\hexen\server\cboss.vc
# End Source File
# Begin Source File

SOURCE=..\..\progs\hexen\server\ceilings.vc
# End Source File
# Begin Source File

SOURCE=..\..\progs\hexen\server\centaur.vc
# End Source File
# Begin Source File

SOURCE=..\..\progs\hexen\server\cheat.vc
# End Source File
# Begin Source File

SOURCE=..\..\progs\hexen\server\demon.vc
# End Source File
# Begin Source File

SOURCE=..\..\progs\hexen\server\doors.vc
# End Source File
# Begin Source File

SOURCE=..\..\progs\hexen\server\dragon.vc
# End Source File
# Begin Source File

SOURCE=..\..\progs\hexen\server\enemy.vc
# End Source File
# Begin Source File

SOURCE=..\..\progs\hexen\server\ettin.vc
# End Source File
# Begin Source File

SOURCE=..\..\progs\hexen\server\fired.vc
# End Source File
# Begin Source File

SOURCE=..\..\progs\hexen\server\floors.vc
# End Source File
# Begin Source File

SOURCE=..\..\progs\hexen\server\fog.vc
# End Source File
# Begin Source File

SOURCE=..\..\progs\hexen\server\iceguy.vc
# End Source File
# Begin Source File

SOURCE=..\..\progs\hexen\server\inter.vc
# End Source File
# Begin Source File

SOURCE=..\..\progs\hexen\server\invent.vc
# End Source File
# Begin Source File

SOURCE=..\..\progs\hexen\server\korax.vc
# End Source File
# Begin Source File

SOURCE=..\..\progs\hexen\server\lights.vc
# End Source File
# Begin Source File

SOURCE=..\..\progs\hexen\server\map.vc
# End Source File
# Begin Source File

SOURCE=..\..\progs\hexen\server\minotaur.vc
# End Source File
# Begin Source File

SOURCE=..\..\progs\hexen\server\missile.vc
# End Source File
# Begin Source File

SOURCE=..\..\progs\hexen\server\mobj.vc
# End Source File
# Begin Source File

SOURCE=..\..\progs\hexen\server\mobjinfo.vc
# End Source File
# Begin Source File

SOURCE=..\..\progs\hexen\server\move.vc
# End Source File
# Begin Source File

SOURCE=..\..\progs\hexen\server\physics.vc
# End Source File
# Begin Source File

SOURCE=..\..\progs\hexen\server\pig.vc
# End Source File
# Begin Source File

SOURCE=..\..\progs\hexen\server\plats.vc
# End Source File
# Begin Source File

SOURCE=..\..\progs\hexen\server\poison.vc
# End Source File
# Begin Source File

SOURCE=..\..\progs\hexen\server\polyobj.vc
# End Source File
# Begin Source File

SOURCE=..\..\progs\hexen\server\pottery.vc
# End Source File
# Begin Source File

SOURCE=..\..\progs\hexen\server\quake.vc
# End Source File
# Begin Source File

SOURCE=..\..\progs\hexen\server\savegame.vc
# End Source File
# Begin Source File

SOURCE=..\..\progs\hexen\server\scroll.vc
# End Source File
# Begin Source File

SOURCE=..\..\progs\hexen\server\serpent.vc
# End Source File
# Begin Source File

SOURCE=..\..\progs\hexen\server\sorcerer.vc
# End Source File
# Begin Source File

SOURCE=..\..\progs\hexen\server\spec.vc
# End Source File
# Begin Source File

SOURCE=..\..\progs\hexen\server\specutl.vc
# End Source File
# Begin Source File

SOURCE=..\..\progs\hexen\server\states.vc
# End Source File
# Begin Source File

SOURCE=..\..\progs\hexen\server\svdefs.vc
# End Source File
# Begin Source File

SOURCE=..\..\progs\hexen\server\svmain.vc
# End Source File
# Begin Source File

SOURCE=..\..\progs\hexen\server\svprogs.vc

!IF  "$(CFG)" == "Progs - Win32 Release"

# PROP Intermediate_Dir "../../basev/hexen/progs"
# Begin Custom Build - Compiling $(InputPath)...
IntDir=.\../../basev/hexen/progs
InputPath=..\..\progs\hexen\server\svprogs.vc
InputName=svprogs

"$(IntDir)/$(InputName).dat" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
	vcc -I../../progs/common $(InputPath) $(IntDir)/$(InputName).dat

# End Custom Build

!ELSEIF  "$(CFG)" == "Progs - Win32 Debug"

# PROP BASE Intermediate_Dir "../../basev/hexen/progs"
# PROP Intermediate_Dir "../../basev/hexen/progs"
# Begin Custom Build - Compiling $(InputPath)...
IntDir=.\../../basev/hexen/progs
InputPath=..\..\progs\hexen\server\svprogs.vc
InputName=svprogs

"$(IntDir)/$(InputName).dat" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
	vcc -I../../progs/common $(InputPath) $(IntDir)/$(InputName).dat

# End Custom Build

!ENDIF 

# End Source File
# Begin Source File

SOURCE=..\..\progs\hexen\server\teleport.vc
# End Source File
# Begin Source File

SOURCE=..\..\progs\hexen\server\telo.vc
# End Source File
# Begin Source File

SOURCE=..\..\progs\hexen\server\things.vc
# End Source File
# Begin Source File

SOURCE=..\..\progs\hexen\server\thrust.vc
# End Source File
# Begin Source File

SOURCE=..\..\progs\hexen\server\tree.vc
# End Source File
# Begin Source File

SOURCE=..\..\progs\hexen\server\user.vc
# End Source File
# Begin Source File

SOURCE=..\..\progs\hexen\server\usersubs.vc
# End Source File
# Begin Source File

SOURCE=..\..\progs\hexen\server\w_cleric.vc
# End Source File
# Begin Source File

SOURCE=..\..\progs\hexen\server\w_fightr.vc
# End Source File
# Begin Source File

SOURCE=..\..\progs\hexen\server\w_mage.vc
# End Source File
# Begin Source File

SOURCE=..\..\progs\hexen\server\w_pig.vc
# End Source File
# Begin Source File

SOURCE=..\..\progs\hexen\server\weapons.vc
# End Source File
# Begin Source File

SOURCE=..\..\progs\hexen\server\world.vc
# End Source File
# Begin Source File

SOURCE=..\..\progs\hexen\server\wpninfo.vc
# End Source File
# Begin Source File

SOURCE=..\..\progs\hexen\server\wraith.vc
# End Source File
# End Group
# Begin Group "Strife Shared"

# PROP Default_Filter ""
# Begin Source File

SOURCE=..\..\progs\strife\strfdefs.vc
# End Source File
# End Group
# Begin Group "Strife Client"

# PROP Default_Filter ""
# Begin Source File

SOURCE=..\..\progs\strife\client\border.vc
# End Source File
# Begin Source File

SOURCE=..\..\progs\strife\client\clmain.vc
# End Source File
# Begin Source File

SOURCE=..\..\progs\strife\client\clprogs.vc

!IF  "$(CFG)" == "Progs - Win32 Release"

# PROP Intermediate_Dir "../../basev/strife/progs"
# Begin Custom Build - Compiling $(InputPath)...
IntDir=.\../../basev/strife/progs
InputPath=..\..\progs\strife\client\clprogs.vc
InputName=clprogs

"$(IntDir)/$(InputName).dat" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
	vcc -I../../progs/common $(InputPath) $(IntDir)/$(InputName).dat

# End Custom Build

!ELSEIF  "$(CFG)" == "Progs - Win32 Debug"

# PROP BASE Intermediate_Dir "../../basev/strife/progs"
# PROP Intermediate_Dir "../../basev/strife/progs"
# Begin Custom Build - Compiling $(InputPath)...
IntDir=.\../../basev/strife/progs
InputPath=..\..\progs\strife\client\clprogs.vc
InputName=clprogs

"$(IntDir)/$(InputName).dat" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
	vcc -I../../progs/common $(InputPath) $(IntDir)/$(InputName).dat

# End Custom Build

!ENDIF 

# End Source File
# Begin Source File

SOURCE=..\..\progs\strife\client\effects.vc
# End Source File
# Begin Source File

SOURCE=..\..\progs\strife\client\finale.vc
# End Source File
# Begin Source File

SOURCE=..\..\progs\strife\client\imission.vc
# End Source File
# Begin Source File

SOURCE=..\..\progs\strife\client\m_api.vc
# End Source File
# Begin Source File

SOURCE=..\..\progs\strife\client\m_ctrl.vc
# End Source File
# Begin Source File

SOURCE=..\..\progs\strife\client\m_help.vc
# End Source File
# Begin Source File

SOURCE=..\..\progs\strife\client\m_join.vc
# End Source File
# Begin Source File

SOURCE=..\..\progs\strife\client\m_load.vc
# End Source File
# Begin Source File

SOURCE=..\..\progs\strife\client\m_main.vc
# End Source File
# Begin Source File

SOURCE=..\..\progs\strife\client\m_menu.vc
# End Source File
# Begin Source File

SOURCE=..\..\progs\strife\client\m_mouse.vc
# End Source File
# Begin Source File

SOURCE=..\..\progs\strife\client\m_multi.vc
# End Source File
# Begin Source File

SOURCE=..\..\progs\strife\client\m_new.vc
# End Source File
# Begin Source File

SOURCE=..\..\progs\strife\client\m_option.vc
# End Source File
# Begin Source File

SOURCE=..\..\progs\strife\client\m_psetup.vc
# End Source File
# Begin Source File

SOURCE=..\..\progs\strife\client\m_quit.vc
# End Source File
# Begin Source File

SOURCE=..\..\progs\strife\client\m_resol.vc
# End Source File
# Begin Source File

SOURCE=..\..\progs\strife\client\m_save.vc
# End Source File
# Begin Source File

SOURCE=..\..\progs\strife\client\m_single.vc
# End Source File
# Begin Source File

SOURCE=..\..\progs\strife\client\m_skill.vc
# End Source File
# Begin Source File

SOURCE=..\..\progs\strife\client\m_sound.vc
# End Source File
# Begin Source File

SOURCE=..\..\progs\strife\client\m_video.vc
# End Source File
# Begin Source File

SOURCE=..\..\progs\strife\client\msgbox.vc
# End Source File
# Begin Source File

SOURCE=..\..\progs\strife\client\statbar.vc
# End Source File
# Begin Source File

SOURCE=..\..\progs\strife\client\title.vc
# End Source File
# End Group
# Begin Group "Strife Server"

# PROP Default_Filter ""
# Begin Source File

SOURCE=..\..\progs\strife\server\adummy.vc
# End Source File
# Begin Source File

SOURCE=..\..\progs\strife\server\ai_subs.vc
# End Source File
# Begin Source File

SOURCE=..\..\progs\strife\server\ceilings.vc
# End Source File
# Begin Source File

SOURCE=..\..\progs\strife\server\cheat.vc
# End Source File
# Begin Source File

SOURCE=..\..\progs\strife\server\doors.vc
# End Source File
# Begin Source File

SOURCE=..\..\progs\strife\server\enemy.vc
# End Source File
# Begin Source File

SOURCE=..\..\progs\strife\server\floors.vc
# End Source File
# Begin Source File

SOURCE=..\..\progs\strife\server\inter.vc
# End Source File
# Begin Source File

SOURCE=..\..\progs\strife\server\lights.vc
# End Source File
# Begin Source File

SOURCE=..\..\progs\strife\server\map.vc
# End Source File
# Begin Source File

SOURCE=..\..\progs\strife\server\mobj.vc
# End Source File
# Begin Source File

SOURCE=..\..\progs\strife\server\mobjinfo.vc
# End Source File
# Begin Source File

SOURCE=..\..\progs\strife\server\move.vc
# End Source File
# Begin Source File

SOURCE=..\..\progs\strife\server\physics.vc
# End Source File
# Begin Source File

SOURCE=..\..\progs\strife\server\plats.vc
# End Source File
# Begin Source File

SOURCE=..\..\progs\strife\server\polyobj.vc
# End Source File
# Begin Source File

SOURCE=..\..\progs\strife\server\savegame.vc
# End Source File
# Begin Source File

SOURCE=..\..\progs\strife\server\spec.vc
# End Source File
# Begin Source File

SOURCE=..\..\progs\strife\server\specutl.vc
# End Source File
# Begin Source File

SOURCE=..\..\progs\strife\server\states.vc
# End Source File
# Begin Source File

SOURCE=..\..\progs\strife\server\svdefs.vc
# End Source File
# Begin Source File

SOURCE=..\..\progs\strife\server\svmain.vc
# End Source File
# Begin Source File

SOURCE=..\..\progs\strife\server\svprogs.vc

!IF  "$(CFG)" == "Progs - Win32 Release"

# PROP Intermediate_Dir "../../basev/strife/progs"
# Begin Custom Build - Compiling $(InputPath)...
IntDir=.\../../basev/strife/progs
InputPath=..\..\progs\strife\server\svprogs.vc
InputName=svprogs

"$(IntDir)/$(InputName).dat" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
	vcc -I../../progs/common $(InputPath) $(IntDir)/$(InputName).dat

# End Custom Build

!ELSEIF  "$(CFG)" == "Progs - Win32 Debug"

# PROP BASE Intermediate_Dir "../../basev/strife/progs"
# PROP Intermediate_Dir "../../basev/strife/progs"
# Begin Custom Build - Compiling $(InputPath)...
IntDir=.\../../basev/strife/progs
InputPath=..\..\progs\strife\server\svprogs.vc
InputName=svprogs

"$(IntDir)/$(InputName).dat" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
	vcc -I../../progs/common $(InputPath) $(IntDir)/$(InputName).dat

# End Custom Build

!ENDIF 

# End Source File
# Begin Source File

SOURCE=..\..\progs\strife\server\teleport.vc
# End Source File
# Begin Source File

SOURCE=..\..\progs\strife\server\user.vc
# End Source File
# Begin Source File

SOURCE=..\..\progs\strife\server\usersubs.vc
# End Source File
# Begin Source File

SOURCE=..\..\progs\strife\server\weapons.vc
# End Source File
# Begin Source File

SOURCE=..\..\progs\strife\server\world.vc
# End Source File
# Begin Source File

SOURCE=..\..\progs\strife\server\wpninfo.vc
# End Source File
# Begin Source File

SOURCE=..\..\progs\strife\server\xlat.vc
# End Source File
# End Group
# Begin Group "Test"

# PROP Default_Filter ""
# Begin Source File

SOURCE=..\..\progs\test\tstprogs.vc

!IF  "$(CFG)" == "Progs - Win32 Release"

# PROP Intermediate_Dir "../../basev/progs"
# Begin Custom Build - Compiling $(InputPath)...
IntDir=.\../../basev/progs
InputPath=..\..\progs\test\tstprogs.vc
InputName=tstprogs

"$(IntDir)/$(InputName).dat" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
	vcc -I../../progs/common $(InputPath) $(IntDir)/$(InputName).dat

# End Custom Build

!ELSEIF  "$(CFG)" == "Progs - Win32 Debug"

# PROP Intermediate_Dir "../../basev/progs"
# Begin Custom Build - Compiling $(InputPath)...
IntDir=.\../../basev/progs
InputPath=..\..\progs\test\tstprogs.vc
InputName=tstprogs

"$(IntDir)/$(InputName).dat" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
	vcc -I../../progs/common $(InputPath) $(IntDir)/$(InputName).dat

# End Custom Build

!ENDIF 

# End Source File
# End Group
# End Target
# End Project
