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
//**	Copyright (C) 1999-2001 JÆnis Legzdi·ý
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

//#define PROGS_PROFILE

void CL_Init(void);
void SV_Init(void);
void CL_SendMove(void);
void ServerFrame(int realtics);
void CL_ReadFromServer(void);
void SV_ShutdownServer(boolean crash);
void CL_Disconnect(void);
void InitMapInfo(void);

// MACROS ------------------------------------------------------------------

#define DEBUGFILENAME	"basev/debug.txt"

// TYPES -------------------------------------------------------------------

typedef struct
{
	char *name;
	void (*func)(char **args, int tag);
	int requiredArgs;
	int tag;
} execOpt_t;

// EXTERNAL FUNCTION PROTOTYPES --------------------------------------------

int Z_FreeMemory(void);
void G_DoLoadGame(void);
void Cmd_WriteAlias(FILE *f);

// PUBLIC FUNCTION PROTOTYPES ----------------------------------------------

// PRIVATE FUNCTION PROTOTYPES ---------------------------------------------

static void 	HandleArgs(void);
static void 	ExecOptionSCRIPTS(char **args, int tag);
static void 	ExecOptionDEVMAPS(char **args, int tag);

// EXTERNAL DATA DECLARATIONS ----------------------------------------------

extern const char	*wadfiles[];

// PUBLIC DATA DEFINITIONS -------------------------------------------------

#ifdef DEVELOPER
TCvarI			developer("developer", "1", CVAR_ARCHIVE);
#else
TCvarI			developer("developer", "0", CVAR_ARCHIVE);
#endif

int				host_frametics;
double			host_frametime;
double			host_time;
double			realtime;
double			oldrealtime;
int				host_framecount;

Game_t			Game;

boolean 		DevMaps;				// true = Map development mode
char*			DevMapsDir = "";		// development maps directory

boolean			singletics = false; 	// debug flag to cancel adaptiveness

boolean			host_initialized = false;

jmp_buf			host_abort;

// PRIVATE DATA DEFINITIONS ------------------------------------------------

static TCvarF	host_framerate("framerate", "0");

static double	last_time;

static TCvarI	respawnparm("RespawnMonsters", "0");	// checkparm of -respawn
static TCvarI	randomclass("RandomClass", "0");		// checkparm of -randclass
static TCvarI	fastparm("Fast", "0");				// checkparm of -fast

static TCvarI	show_time("show_time", "0");

static TCvarS	configfile("configfile", "config.cfg", CVAR_ARCHIVE);

static execOpt_t ExecOptions[] =
{
	{ "-scripts", 	ExecOptionSCRIPTS, 1, 0 },
	{ "-devmaps", 	ExecOptionDEVMAPS, 1, 0 },
	{ NULL, NULL, 0, 0 } // Terminator
};

// CODE --------------------------------------------------------------------

//==========================================================================
//
//  Cmd_SaveConfiguration
//
//  Saves game variables
//
//==========================================================================

#ifdef CLIENT
void Cmd_SaveConfiguration(void)
{
    FILE*	f;
	char	path[MAX_OSPATH];

    if (!host_initialized)
    	return;

	sprintf(path, "%s/%s", fl_gamedir, configfile.string);
    f = fopen(path, "w");
    if (!f && strcmp(configfile, "config.cfg"))
	{
		configfile = "config.cfg";
		sprintf(path, "%s/%s", fl_gamedir, configfile.string);
	    f = fopen(path, "w");
	}
    if (!f)
    {
    	con << "Cmd_SaveConfiguration: Failed to open config file \"" <<
	    	path << "\"\n";
		return; // can't write the file, but don't complain
	}

    fprintf(f, "//Vavoom configuration file\n");
	Cvar_Write(f);
	Cmd_WriteAlias(f);
	IN_WriteBindings(f);

    fclose(f);
}
#endif

//==========================================================================
//
//  Version_f
//
//==========================================================================

COMMAND(Version)
{
#ifdef RANGECHECK
	con << "VAVOOM version " << VERSION << " +R.\n";
#else
	con << "VAVOOM version " << VERSION << ".\n";
#endif
	con << "Compiled "__DATE__" "__TIME__".\n";
}

//==========================================================================
//
//	Host_Shutdown
//
//	Return to default system state
//
//==========================================================================

void Host_Shutdown(void)
{
	static boolean		shutting_down = false;

	if (shutting_down)
    {
    	con << "Recursive shutdown\n";
    	return;
    }
    shutting_down = true;

	NET_Shutdown();
#ifdef CLIENT
    IN_Shutdown();
	V_Shutdown();
	S_Shutdown();
#endif
	Sys_Shutdown();

	PR_Traceback();

#ifdef PROGS_PROFILE
	con << "\nClient progs profile:\n\n";
	clpr.DumpProfile();
	con << "\nServer progs profile:\n\n";
	svpr.DumpProfile();
#endif
}

//==========================================================================
//
//	Host_Init
//
//==========================================================================

void Host_Init(void)
{
	if (setjmp(host_abort))
	{
		Sys_Error("Error during startup");
	}

	OpenDebugFile(DEBUGFILENAME);

    // init subsystems

	M_InitByteOrder();

#ifdef CLIENT
	cls.state = ca_disconnected;
#endif

	//  Memory must be initialised before anything else
	void*	base;
    int		size;
	base = Sys_ZoneBase(&size);
	Z_Init(base, size);

	Cmd_Init();

	FL_Init();

    HandleArgs();

#ifdef CLIENT
    C_Init();
	V_Init();
#endif

	W_InitMultipleFiles(wadfiles);

#ifdef CLIENT
	IN_Init();
#endif

    S_Init();
#ifdef CLIENT
	SCR_Init();
    T_Init();
	CT_Init();
#endif

	PR_Init();

#ifdef CLIENT
	CL_Init();
#endif
#ifdef SERVER
	SV_Init();
#endif

#ifdef CLIENT
	MN_Init();
    AM_Init();
	SB_Init();
#endif

    R_Init();

	InitMapInfo();
#ifdef SERVER
    P_Init();
#endif

	NET_Init();

	Z_FreeMemory();

	CmdBuf.Exec();

#ifndef CLIENT
	if (!sv.active)
	{
		CmdBuf << "MaxPlayers 4\n";
		if (Game == Doom || Game == Heretic)
			CmdBuf << "Map E1M1\n";
		else
			CmdBuf << "Map MAP01\n";
	}
#endif

	host_initialized = true;
	cond << "Host_Init done\n";
}

//==========================================================================
//
//	Host_GetConsoleCommands
//
//	Add them exactly as if they had been typed at the console
//
//==========================================================================

static void Host_GetConsoleCommands(void)
{
	char	*cmd;

#ifdef CLIENT
	if (cls.state != ca_dedicated)
		return;
#endif

	while ((cmd = Sys_ConsoleInput()))
	{
		CmdBuf << cmd;
	}
}

//==========================================================================
//
//	FilterTime
//
//	Returns false if the time is too short to run a frame
//
//==========================================================================

static bool FilterTime(void)
{
	double curr_time = Sys_Time();
	float time = curr_time - last_time;
	last_time = curr_time;

	realtime += time;

//	if (realtime - oldrealtime < 1.0 / 72.0)
	if (realtime - oldrealtime < 1.0 / 35.0)
		return false;		// framerate is too high

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
	if (host_frametics < 1)
		return false;		//	No tics to run
	if (host_frametics > 3)
		host_frametics = 3;	//	Don't run too slow
	oldrealtime = realtime;
	lasttime = thistime;

	return true;
}

//==========================================================================
//
//  Host_Frame
//
//	Runs all active servers
//
//==========================================================================

void Host_Frame(void)
{
	static double		time1 = 0;
	static double		time2 = 0;
	static double		time3 = 0;
	int			pass1, pass2, pass3;

	if (setjmp(host_abort))
	{
		//	Something bad happened, or the server disconnected
		return;
	}

	//	Keep the random time dependent
	rand();

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
	CmdBuf.Exec();

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
		con << (pass1 + pass2 + pass3) << " tot "
			<< pass1 << " server " << pass2 << " gfx " << pass3 << " snd\n";
	}

	host_framecount++;
}

//==========================================================================
//
//	Host_EndGame
//
//==========================================================================

void Host_EndGame(const char *message, ...)
{
	va_list		argptr;
	char		string[1024];
	
	va_start(argptr,message);
	vsprintf(string,message,argptr);
	va_end(argptr);
	cond << "Host_EndGame: " << string << endl;

	//	Reset progs virtual machine
	PR_OnAbort();
	//	Make sure, that we use primary wad files
	W_CloseAuxiliary();

#ifdef SERVER
	SV_ShutdownServer(false);
#endif

#ifdef CLIENT
	if (cls.state == ca_dedicated)
		Sys_Error("Host_EndGame: %s\n", string);	// dedicated servers exit
	
	CL_Disconnect();
	clpr.Exec("OnHostEndGame");

	longjmp(host_abort, 1);
#else
	Sys_Error("Host_EndGame: %s\n", string);	// dedicated servers exit
#endif
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
	va_list		argptr;
	char		string[1024];
	static boolean inerror = false;
	
	if (inerror)
	{
		Sys_Error("Host_Error: recursively entered");
	}
	inerror = true;
	
//	SCR_EndLoadingPlaque();		// reenable screen updates

	va_start(argptr, error);
	vsprintf(string, error, argptr);
	va_end(argptr);
	con << "Host_Error: " << string << endl;

	//	Reset progs virtual machine
	PR_OnAbort();
	//	Make sure, that we use primary wad files
	W_CloseAuxiliary();

#ifdef SERVER
	SV_ShutdownServer(false);
#endif

#ifdef CLIENT
	if (cls.state == ca_dedicated)
		Sys_Error("Host_Error: %s\n", string);	// dedicated servers exit

	CL_Disconnect();
	clpr.Exec("OnHostError");
	C_StartFull();

	inerror = false;

	longjmp(host_abort, 1);
#else
	Sys_Error("Host_Error: %s\n", string);	// dedicated servers exit
#endif
}

//==========================================================================
//==========================================================================
//
//	Identify game and version, handle args
//
//==========================================================================
//==========================================================================

void FL_AddFile(const char *file);

//==========================================================================
//
//	HandleArgs
//
//==========================================================================

static void HandleArgs(void)
{
 	int			p;
	execOpt_t*	opt;

	// Process command line options
	for (opt = ExecOptions; opt->name != NULL; opt++)
	{
		p = M_CheckParm(opt->name);
		if (p && p < myargc - opt->requiredArgs)
		{
			opt->func(&myargv[p], opt->tag);
		}
	}
}

//==========================================================================
//
//	ExecOptionSCRIPTS
//
//==========================================================================

static void ExecOptionSCRIPTS(char **args, int)
{
	sc_FileScripts = true;
	sc_ScriptsDir = args[1];
}

//==========================================================================
//
// ExecOptionDEVMAPS
//
//==========================================================================

static void ExecOptionDEVMAPS(char **args, int)
{
	DevMaps = true;
	cond << "Map development mode enabled:\n";
	cond << "[config    ] = " << args[1] << endl;
	SC_OpenFile(args[1]);
	SC_MustGetStringName("mapsdir");
	SC_MustGetString();
	cond << "[mapsdir   ] = " << sc_String << endl;
	DevMapsDir = (char*)Z_Malloc(strlen(sc_String) + 1, PU_STATIC, 0);
	strcpy(DevMapsDir, sc_String);
	SC_MustGetStringName("scriptsdir");
	SC_MustGetString();
	cond << "[scriptsdir] = " << sc_String << endl;
	sc_FileScripts = true;
	sc_ScriptsDir = (char*)Z_Malloc(strlen(sc_String) + 1, PU_STATIC, 0);
	strcpy(sc_ScriptsDir, sc_String);
	while (SC_GetString())
	{
		if (SC_Compare("file"))
		{
			SC_MustGetString();
			FL_AddFile(sc_String);
		}
		else
		{
			SC_ScriptError(NULL);
		}
	}
	SC_Close();
}

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
    Cmd_SaveConfiguration();
#endif
	Sys_Quit();
}

//**************************************************************************
//
//	$Log$
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
