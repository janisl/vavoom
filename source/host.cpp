//**************************************************************************
//**
//**	##   ##    ##    ##   ##   ####     ####   ###     ###
//**	##   ##  ##  ##  ##   ##  ##  ##   ##  ##  ####   ####
//**	 ## ##  ##    ##  ## ##  ##    ## ##    ## ## ## ## ##
//**	 ## ##  ########  ## ##  ##    ## ##    ## ##  ###  ##
//**	  ###   ##    ##   ###    ##  ##   ##  ##  ##       ##
//**	   #    ##    ##    #      ####     ####   ##       ##
//**
//**	$Id$
//**
//**	Copyright (C) 1999-2002 JÆnis Legzdi·ý
//**
//**	This program is free software; you can redistribute it and/or
//**  modify it under the terms of the GNU General Public License
//**  as published by the Free Software Foundation; either version 2
//**  of the License, or (at your option) any later version.
//**
//**	This program is distributed in the hope that it will be useful,
//**  but WITHOUT ANY WARRANTY; without even the implied warranty of
//**  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
//**  GNU General Public License for more details.
//**
//**************************************************************************

// HEADER FILES ------------------------------------------------------------

#include "gamedefs.h"
#include "cl_local.h"

void CL_Init();
void SV_Init();
void CL_SendMove();
void ServerFrame(int realtics);
void CL_ReadFromServer();
void SV_ShutdownServer(boolean crash);
void CL_Disconnect();
char *P_TranslateMap(int map);

// MACROS ------------------------------------------------------------------

// State updates, number of tics / second.
#define TICRATE		35

// TYPES -------------------------------------------------------------------

class EndGame : public VavoomError
{
public:
	explicit EndGame(const char *txt) : VavoomError(txt) { }
};

// EXTERNAL FUNCTION PROTOTYPES --------------------------------------------

int Z_FreeMemory();
void G_DoLoadGame();

// PUBLIC FUNCTION PROTOTYPES ----------------------------------------------

// PRIVATE FUNCTION PROTOTYPES ---------------------------------------------

// EXTERNAL DATA DECLARATIONS ----------------------------------------------

// PUBLIC DATA DEFINITIONS -------------------------------------------------

#ifdef DEVELOPER
VCvarI			developer("developer", "1", CVAR_Archive);
#else
VCvarI			developer("developer", "0", CVAR_Archive);
#endif

int				host_frametics;
double			host_frametime;
double			host_time;
double			realtime;
double			oldrealtime;
int				host_framecount;

boolean			host_initialized = false;

// PRIVATE DATA DEFINITIONS ------------------------------------------------

static VCvarF	host_framerate("framerate", "0");

static double	last_time;

static VCvarI	respawnparm("RespawnMonsters", "0");	// checkparm of -respawn
static VCvarI	randomclass("RandomClass", "0");		// checkparm of -randclass
static VCvarI	fastparm("Fast", "0");				// checkparm of -fast

static VCvarI	show_time("show_time", "0");

static VCvarS	configfile("configfile", "config.cfg", CVAR_Archive);

static char		*host_error_string;

// CODE --------------------------------------------------------------------

//==========================================================================
//
//	Host_Init
//
//==========================================================================

void Host_Init()
{
	guard(Host_Init);
	//  Memory must be initialised before anything else
	void*	base;
	int		size;
	base = Sys_ZoneBase(&size);
	Z_Init(base, size);

#if defined(__unix__) && !defined(DJGPP) && !defined(_WIN32)
	const char* HomeDir = getenv("HOME");
	if (HomeDir)
	{
		Sys_CreateDirectory(va("%s/.vavoom", HomeDir));
		OpenDebugFile(va("%s/.vavoom/debug.txt", getenv("HOME")));
	}
	else
	{
		OpenDebugFile("basev/debug.txt");
	}
#else
	OpenDebugFile("basev/debug.txt");
#endif

	// init subsystems

	M_InitByteOrder();

#ifdef CLIENT
	cls.state = ca_disconnected;
#endif

	const char* p = GArgs.CheckValue("-maxents");
	if (p)
	{
		GMaxEntities = atoi(p);
	}

	VName::StaticInit();
	VObject::StaticInit();

	VCvar::Init();
	VCommand::Init();

	FL_Init();

	PR_Init();

	S_InitScript();
	SN_InitSequenceScript();
	R_InitTexture();

#ifdef CLIENT
	IN_Init();
	S_Init();
	SCR_Init();
	T_Init();
	CT_Init();
	C_Init();
	V_Init();

	CL_Init();

	MN_Init();
	AM_Init();
	SB_Init();
#endif

	InitMapInfo();

#ifdef CLIENT
	R_Init();
#endif

#ifdef SERVER
	SV_Init();
#endif

	NET_Init();

	Z_FreeMemory();

	GCmdBuf.Exec();

#ifndef CLIENT
	if (!sv.active)
	{
		GCmdBuf << "MaxPlayers 4\n";
		GCmdBuf << "Map " << P_TranslateMap(1) << "\n";
	}
#endif

	host_initialized = true;
	unguard;
}

//==========================================================================
//
//	Host_GetConsoleCommands
//
//	Add them exactly as if they had been typed at the console
//
//==========================================================================

static void Host_GetConsoleCommands()
{
	guard(Host_GetConsoleCommands);
	char	*cmd;

#ifdef CLIENT
	if (cls.state != ca_dedicated)
		return;
#endif

	for (cmd = Sys_ConsoleInput(); cmd; cmd = Sys_ConsoleInput())
	{
		GCmdBuf << cmd << "\n";
	}
	unguard;
}

//==========================================================================
//
//	FilterTime
//
//	Returns false if the time is too short to run a frame
//
//==========================================================================

extern VCvarI real_time;

static bool FilterTime()
{
	guard(FilterTime);
	double curr_time = Sys_Time();
	float time = curr_time - last_time;
	last_time = curr_time;

	realtime += time;

	if (real_time)
	{
		if (realtime - oldrealtime < 1.0 / 72.0)
			return false;		// framerate is too high
	}
	else
	{
		if (realtime - oldrealtime < 1.0 / 35.0)
			return false;		// framerate is too high
	}

	host_frametime = realtime - oldrealtime;

	if (host_framerate > 0)
	{
		host_frametime = host_framerate;
	}
	else
	{	// don't allow really long or short frames
		if (host_frametime > 0.1)
			host_frametime = 0.1;
		if (host_frametime < 0.001)
			host_frametime = 0.001;
	}
	
	int			thistime;
	static int	lasttime;

	thistime = (int)(realtime * TICRATE);
	host_frametics = thistime - lasttime;
	if (!real_time && host_frametics < 1)
		return false;		//	No tics to run
	if (host_frametics > 3)
		host_frametics = 3;	//	Don't run too slow
	oldrealtime = realtime;
	lasttime = thistime;

	return true;
	unguard;
}

//==========================================================================
//
//  Host_Frame
//
//	Runs all active servers
//
//==========================================================================

void Host_Frame()
{
	guard(Host_Frame);
	static double time1 = 0;
	static double time2 = 0;
	static double time3 = 0;
	int pass1, pass2, pass3;

	try
	{
		//	Keep the random time dependent
		rand();

#ifdef PARANOID
		Z_CheckHeap();
#endif

		//	Decide the simulation time
		if (!FilterTime())
		{
			//	Don't run too fast, or packets will flood out
			return;
		}

#ifdef CLIENT
		//	Get new key, mice and joystick events
		IN_ProcessEvents();
#endif

		//	Check for commands typed to the host
		Host_GetConsoleCommands();

		//	Process console commands
		GCmdBuf.Exec();

		NET_Poll();

#ifdef CLIENT
		//	Make intentions
		CL_SendMove();
#endif

#ifdef SERVER
		if (sv.active)
		{
			//	Server operations
			ServerFrame(host_frametics);
		}
#endif

		host_time += host_frametime;

#ifdef CLIENT
		//	Fetch results from server
		CL_ReadFromServer();

		//	Collect all garbage
		VObject::CollectGarbage();

		//	Update video
		if (show_time)
			time1 = Sys_Time();

		SCR_Update();

		if (show_time)
			time2 = Sys_Time();

		if (cls.signon == SIGNONS)
		{
			CL_DecayLights();
		}

		//	Update audio
		S_UpdateSounds();
#endif

		if (show_time)
		{
			pass1 = (int)((time1 - time3) * 1000);
			time3 = Sys_Time();
			pass2 = (int)((time2 - time1) * 1000);
			pass3 = (int)((time3 - time2) * 1000);
			GCon->Logf("%d tot %d server %d gfx %d snd",
				pass1 + pass2 + pass3, pass1, pass2, pass3);
		}

		host_framecount++;
	}
	catch (RecoverableError &e)
	{
		GCon->Logf("Host_Error: %s", e.message);

		//	Reset progs virtual machine
		PR_OnAbort();
		//	Make sure, that we use primary wad files
		W_CloseAuxiliary();

#ifdef SERVER
		SV_ShutdownServer(false);
#endif

#ifdef CLIENT
		if (cls.state == ca_dedicated)
			Sys_Error("Host_Error: %s\n", e.message);	// dedicated servers exit

		CL_Disconnect();
		GClGame->eventOnHostError();
		C_StartFull();
#else
		Sys_Error("Host_Error: %s\n", e.message);	// dedicated servers exit
#endif
	}
	catch (EndGame &e)
	{
		GCon->Logf(NAME_Dev, "Host_EndGame: %s", e.message);

		//	Reset progs virtual machine
		PR_OnAbort();
		//	Make sure, that we use primary wad files
		W_CloseAuxiliary();

#ifdef SERVER
		SV_ShutdownServer(false);
#endif

#ifdef CLIENT
		if (cls.state == ca_dedicated)
			Sys_Error("Host_EndGame: %s\n", e.message);	// dedicated servers exit
	
		CL_Disconnect();
		GClGame->eventOnHostEndGame();
#else
		Sys_Error("Host_EndGame: %s\n", e.message);	// dedicated servers exit
#endif
	}
	unguard;
}

//==========================================================================
//
//	Host_EndGame
//
//==========================================================================

void Host_EndGame(const char *message, ...)
{
	va_list argptr;
	char string[1024];
	
	va_start(argptr,message);
	vsprintf(string,message,argptr);
	va_end(argptr);

	throw EndGame(string);
}

//==========================================================================
//
//	Host_Error
//
//	This shuts down both the client and server
//
//==========================================================================

void Host_Error(const char *error, ...)
{
	va_list argptr;
	char string[1024];
	
	va_start(argptr, error);
	vsprintf(string, error, argptr);
	va_end(argptr);

	throw RecoverableError(string);
}

//==========================================================================
//
//  Version_f
//
//==========================================================================

COMMAND(Version)
{
	GCon->Log("VAVOOM version " VERSION_TEXT ".");
	GCon->Log("Compiled "__DATE__" "__TIME__".");
}

//==========================================================================
//
//	Host_SaveConfiguration
//
//  Saves game variables
//
//==========================================================================

#ifdef CLIENT
void Host_SaveConfiguration()
{
	if (!host_initialized)
		return;

	FILE *f;
	if (fl_savedir)
	{
		FL_CreatePath(fl_savedir + "/" + fl_gamedir);
		f = fopen(*(fl_savedir + "/" + fl_gamedir + "/" +
			(const char*)configfile), "w");
	}
	else
	{
		FL_CreatePath(fl_basedir + "/" + fl_gamedir);
		f = fopen(*(fl_basedir + "/" + fl_gamedir + "/" +
			(const char*)configfile), "w");
	}
	if (!f)
	{
		GCon->Logf("Host_SaveConfiguration: Failed to open config file \"%s\"",
			(const char*)configfile);
		return; // can't write the file, but don't complain
	}

	fprintf(f, "// Generated by Vavoom\n");
	fprintf(f, "//\n// Bindings\n//\n");
	IN_WriteBindings(f);
	fprintf(f, "//\n// Aliases\n//\n");
	VCommand::WriteAlias(f);
	fprintf(f, "//\n// Variables\n//\n");
	VCvar::WriteVariables(f);

	fclose(f);
}
#endif

//==========================================================================
//
//  Quit
//
//==========================================================================

COMMAND(Quit)
{
#ifdef CLIENT
	CL_Disconnect();
#endif
#ifdef SERVER
	SV_ShutdownServer(false);
#endif
#ifdef CLIENT
	// Save game configyration
	Host_SaveConfiguration();
#endif
	Sys_Quit();
}

//==========================================================================
//
//	Host_CoreDump
//
//==========================================================================

void Host_CoreDump(const char *fmt, ...)
{
	static bool first = true;

	if (!host_error_string)
	{
		host_error_string = new char[32];
		strcpy(host_error_string, "Stack trace: ");
		first = true;

		PR_Traceback();
	}

	va_list argptr;
	char string[1024];
	
	va_start(argptr, fmt);
	vsprintf(string, fmt, argptr);
	va_end(argptr);

	dprintf("- %s\n", string);

	char *new_string = new char[strlen(host_error_string) + strlen(string) + 6];
	strcpy(new_string, host_error_string);
	if (first)
		first = false;
	else
		strcat(new_string, " <- ");
	strcat(new_string, string);
	delete host_error_string;
	host_error_string = new_string;
}

//==========================================================================
//
//	Host_GetCoreDump
//
//==========================================================================

const char *Host_GetCoreDump()
{
	return host_error_string ? host_error_string : "";
}

//==========================================================================
//
//	Host_Shutdown
//
//	Return to default system state
//
//==========================================================================

void Host_Shutdown()
{
	static boolean		shutting_down = false;

	if (shutting_down)
	{
		GCon->Log("Recursive shutdown");
		return;
	}
	shutting_down = true;

#define SAFE_SHUTDOWN(name, args) \
	try { name args; } catch (...) { GCon->Log(#name" failed"); }

	SAFE_SHUTDOWN(NET_Shutdown, ())
#ifdef CLIENT
	SAFE_SHUTDOWN(IN_Shutdown, ())
	SAFE_SHUTDOWN(V_Shutdown, ())
	SAFE_SHUTDOWN(S_Shutdown, ())
#endif
	SAFE_SHUTDOWN(Sys_Shutdown, ())

	SAFE_SHUTDOWN(VObject::StaticExit, ())
	SAFE_SHUTDOWN(VName::StaticExit, ())
}

//**************************************************************************
//
//	$Log$
//	Revision 1.36  2006/04/12 18:53:19  dj_jl
//	Dedicated server fix.
//
//	Revision 1.35  2006/04/05 17:23:37  dj_jl
//	More dynamic string usage in console command class.
//	Added class for handling command line arguments.
//	
//	Revision 1.34  2006/03/29 22:32:27  dj_jl
//	Changed console variables and command buffer to use dynamic strings.
//	
//	Revision 1.33  2006/03/04 16:01:34  dj_jl
//	File system API now uses strings.
//	
//	Revision 1.32  2006/02/27 20:45:26  dj_jl
//	Rewrote names class.
//	
//	Revision 1.31  2006/02/09 22:35:54  dj_jl
//	Moved all client game code to classes.
//	
//	Revision 1.30  2006/01/29 20:41:30  dj_jl
//	On Unix systems use ~/.vavoom for generated files.
//	
//	Revision 1.29  2005/11/20 12:38:50  dj_jl
//	Implemented support for sound sequence extensions.
//	
//	Revision 1.28  2005/11/05 14:57:36  dj_jl
//	Putting Strife shareware voices in correct namespace.
//	
//	Revision 1.27  2005/05/26 16:54:21  dj_jl
//	Created texture manager class
//	
//	Revision 1.26  2003/03/08 12:08:04  dj_jl
//	Beautification.
//	
//	Revision 1.25  2002/08/28 16:42:04  dj_jl
//	Configurable entity limit.
//	
//	Revision 1.24  2002/07/23 16:29:56  dj_jl
//	Replaced console streams with output device class.
//	
//	Revision 1.23  2002/07/23 13:10:37  dj_jl
//	Some fixes for switching to floating-point time.
//	
//	Revision 1.22  2002/05/29 16:53:27  dj_jl
//	Got rid of a warning.
//	
//	Revision 1.21  2002/05/18 16:56:34  dj_jl
//	Added FArchive and FOutputDevice classes.
//	
//	Revision 1.20  2002/04/11 16:40:57  dj_jl
//	Safe core dumps.
//	
//	Revision 1.19  2002/02/22 18:09:51  dj_jl
//	Some improvements, beautification.
//	
//	Revision 1.18  2002/01/07 12:16:42  dj_jl
//	Changed copyright year
//	
//	Revision 1.17  2002/01/04 18:22:13  dj_jl
//	Beautification
//	
//	Revision 1.16  2002/01/03 18:38:25  dj_jl
//	Added guard macros and core dumps
//	
//	Revision 1.15  2001/12/27 17:37:42  dj_jl
//	Added garbage collection
//	
//	Revision 1.14  2001/12/18 19:05:03  dj_jl
//	Made VCvar a pure C++ class
//	
//	Revision 1.13  2001/12/12 19:28:49  dj_jl
//	Some little changes, beautification
//	
//	Revision 1.12  2001/11/09 14:22:09  dj_jl
//	R_InitTexture now called from Host_init
//	
//	Revision 1.11  2001/10/12 17:31:13  dj_jl
//	no message
//	
//	Revision 1.10  2001/10/08 17:26:17  dj_jl
//	Started to use exceptions
//	
//	Revision 1.9  2001/10/04 17:20:25  dj_jl
//	Saving config using streams
//	
//	Revision 1.8  2001/09/24 17:35:24  dj_jl
//	Support for thinker classes
//	
//	Revision 1.7  2001/08/31 17:28:00  dj_jl
//	Removed RANGECHECK
//	
//	Revision 1.6  2001/08/30 17:46:21  dj_jl
//	Removed game dependency
//	
//	Revision 1.5  2001/08/21 17:41:33  dj_jl
//	Removed -devmaps option
//	
//	Revision 1.4  2001/08/04 17:25:14  dj_jl
//	Moved title / demo loop to progs
//	Removed shareware / ExtendedWAD from engine
//	
//	Revision 1.3  2001/07/31 17:07:41  dj_jl
//	Changes for filesystem and localising demo loop
//	
//	Revision 1.2  2001/07/27 14:27:54  dj_jl
//	Update with Id-s and Log-s, some fixes
//
//**************************************************************************
