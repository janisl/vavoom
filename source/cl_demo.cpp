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
//**
//**	DEMO CODE
//**
//**	When a demo is playing back, all NET_SendMessages are skipped, and
//**  NET_GetMessages are read from the demo file.
//**
//**	Whenever cl.time gets past the last received message, another message
//**  is read from the demo file.
//**
//**************************************************************************

// HEADER FILES ------------------------------------------------------------

#include "gamedefs.h"
//#define USEZLIB
#ifdef USEZLIB
#include <zlib.h>
#endif

// MACROS ------------------------------------------------------------------

// TYPES -------------------------------------------------------------------

// EXTERNAL FUNCTION PROTOTYPES --------------------------------------------

void CL_Disconnect(void);

// PUBLIC FUNCTION PROTOTYPES ----------------------------------------------

// PRIVATE FUNCTION PROTOTYPES ---------------------------------------------

// EXTERNAL DATA DECLARATIONS ----------------------------------------------

// PUBLIC DATA DEFINITIONS -------------------------------------------------

// PRIVATE DATA DEFINITIONS ------------------------------------------------

#ifdef USEZLIB
static gzFile		gzdemofile;
#endif

// CODE --------------------------------------------------------------------

//==========================================================================
//
//	CL_FinishTimeDemo
//
//==========================================================================

void CL_FinishTimeDemo(void)
{
	int		frames;
	float	time;
	
	cls.timedemo = false;
	
	// the first frame didn't count
	frames = (host_framecount - cls.td_startframe) - 1;
	time = realtime - cls.td_starttime;
	if (!time)
		time = 1;
	con << frames << " frames " << time << " seconds " << (frames/time) << " fps\n";
}

//==========================================================================
//
//	CL_StopPlayback
//
//	Called when a demo file runs out, or the user starts a game
//
//==========================================================================

void CL_StopPlayback(void)
{
	if (!cls.demoplayback)
	{
		return;
	}

#ifdef USEZLIB
	gzclose(gzdemofile);
	gzdemofile = NULL;
#else
	fclose(cls.demofile);
#endif
	cls.demoplayback = false;
	cls.demofile = NULL;
	cls.state = ca_disconnected;

	if (cls.timedemo)
	{
		CL_FinishTimeDemo();
	}
}

//==========================================================================
//
//	CL_WriteDemoMessage
//
//	Dumps the current net message, prefixed by the length and view angles
//
//==========================================================================

void CL_WriteDemoMessage(void)
{
	int			len;
	angle_t		a;

#ifdef USEZLIB
	len = LittleLong(net_msg.CurSize);
	gzwrite(gzdemofile, &len, 4);
	a = LittleLong(cl.viewangles.pitch);
	gzwrite(gzdemofile, &a, 4);
	a = LittleLong(cl.viewangles.yaw);
	gzwrite(gzdemofile, &a, 4);
	a = LittleLong(cl.viewangles.roll);
	gzwrite(gzdemofile, &a, 4);
	gzwrite(gzdemofile, net_msg.Data, net_msg.CurSize);
#else
	len = LittleLong(net_msg.CurSize);
	fwrite(&len, 4, 1, cls.demofile);
	a = LittleLong(cl.viewangles.pitch);
	fwrite(&a, 4, 1, cls.demofile);
	a = LittleLong(cl.viewangles.yaw);
	fwrite(&a, 4, 1, cls.demofile);
	a = LittleLong(cl.viewangles.roll);
	fwrite(&a, 4, 1, cls.demofile);
	fwrite(net_msg.Data, net_msg.CurSize, 1, cls.demofile);
	fflush(cls.demofile);
#endif
}

//==========================================================================
//
//	CL_GetMassage
//
//	Handles recording and playback of demos, on top of NET_ code
//
//==========================================================================

int CL_GetMessage(void)
{
	int			r;
	angle_t		a;

	if (cls.demoplayback)
	{
		// decide if it is time to grab the next message
		if (cls.signon == SIGNONS)	// allways grab until fully connected
		{
			if (cls.timedemo)
			{
				if (host_framecount == cls.td_lastframe)
				{
					return 0;		// allready read this frame's message
				}
				cls.td_lastframe = host_framecount;
				// if this is the second frame, grab the real  cls.td_starttime
				// so the bogus time on the first frame doesn't count
				if (host_framecount == cls.td_startframe + 1)
				{
					cls.td_starttime = realtime;
				}
			}
			else if ( /* cl.time > 0 && */ cl.time <= cl_level.time)
			{
				return 0;		// don't need another message yet
			}
		}

		// get the next message
#ifdef USEZLIB
		gzread(gzdemofile, &net_msg.CurSize, 4);
//		VectorCopy (cl.mviewangles[0], cl.mviewangles[1]);
		gzread(gzdemofile, &a, 4);
		cl.viewangles.pitch = LittleLong(a);
		gzread(gzdemofile, &a, 4);
		cl.viewangles.yaw = LittleLong(a);
		gzread(gzdemofile, &a, 4);
		cl.viewangles.roll = LittleLong(a);
#else
		fread(&net_msg.CurSize, 4, 1, cls.demofile);
//		VectorCopy (cl.mviewangles[0], cl.mviewangles[1]);
		fread(&a, 4, 1, cls.demofile);
		cl.viewangles.pitch = LittleLong(a);
		fread(&a, 4, 1, cls.demofile);
		cl.viewangles.yaw = LittleLong(a);
		fread(&a, 4, 1, cls.demofile);
		cl.viewangles.roll = LittleLong(a);
#endif

		net_msg.CurSize = LittleLong(net_msg.CurSize);
		if (net_msg.CurSize > MAX_MSGLEN)
			Sys_Error("Demo message > MAX_MSGLEN");
#ifdef USEZLIB
		r = gzread(gzdemofile, net_msg.Data, net_msg.CurSize);
#else
		r = fread(net_msg.Data, net_msg.CurSize, 1, cls.demofile);
#endif
		if (r != 1)
		{
			CL_StopPlayback();
			return 0;
		}
	
		return 1;
	}

	while (1)
	{
		r = NET_GetMessage(cls.netcon);

		if (r != 1 && r != 2)
			return r;
	
		// discard nop keepalive message
		if (net_msg.CurSize == 1 && net_msg.Data[0] == svc_nop)
			con << "<-- server to client keepalive\n";
		else
			break;
	}

	if (cls.demorecording)
	{
		CL_WriteDemoMessage();
	}
	
	return r;
}

//==========================================================================
//
//	CL_StopRecording
//
//==========================================================================

void CL_StopRecording(void)
{
	// write a disconnect message to the demo file
	net_msg.Clear();
	net_msg << (byte)svc_disconnect;
	CL_WriteDemoMessage();

	// finish up
#ifdef USEZLIB
	gzclose(gzdemofile);
	gzdemofile = NULL;
#else
	fclose(cls.demofile);
#endif
	cls.demofile = NULL;
	cls.demorecording = false;
	con << "Completed demo\n";
}

//==========================================================================
//
//	COMMAND StopDemo
//
//	stop recording a demo
//
//==========================================================================

COMMAND(StopDemo)
{
	if (cmd_source != src_command)
	{
		return;
	}

	if (!cls.demorecording)
	{
		con << "Not recording a demo.\n";
		return;
	}
	CL_StopRecording();
}

//==========================================================================
//
//	COMMAND Record
//
//	record <demoname> <map>
//
//==========================================================================

COMMAND(Record)
{
	int		c;
	char	name[512];

	if (cmd_source != src_command)
	{
		return;
	}

	c = Argc();
	if (c != 2 && c != 3)
	{
		con << "record <demoname> [<map>]\n";
		return;
	}

	if (strstr(Argv(1), ".."))
	{
		con << "Relative pathnames are not allowed.\n";
		return;
	}

	if (c == 2 && cls.state == ca_connected)
	{
		con << "Can not record - already connected to server\n"
			<< "Client demo recording must be started before connecting\n";
		return;
	}

	Sys_CreateDirectory(va("%s/demos", fl_gamedir));
	sprintf(name, "%s/demos/%s", fl_gamedir, Argv(1));

	//
	// start the map up
	//
	if (c > 2)
	{
		Cmd_ExecuteString(va("map %s", Argv(2)), src_command);
	}
	
	//
	// open the demo file
	//
	FL_DefaultExtension(name, ".dem");

	con << "recording to " << name << ".\n";
#ifdef USEZLIB
	gzdemofile = gzopen(name, "wb9");
	if (!gzdemofile)
	{
		con << "ERROR: couldn't open.\n";
		return;
	}

	gzwrite(gzdemofile, "VDEM", 4);
#else
	cls.demofile = fopen(name, "wb");
	if (!cls.demofile)
	{
		con << "ERROR: couldn't open.\n";
		return;
	}

	fwrite("VDEM", 1, 4, cls.demofile);
#endif

	cls.demorecording = true;
}

//==========================================================================
//
//	COMMAND PlayDemo
//
//	play [demoname]
//
//==========================================================================

COMMAND(PlayDemo)
{
	char	name[256];
	char	fname[MAX_OSPATH];
	char	magic[8];

	if (cmd_source != src_command)
	{
		return;
	}

	if (Argc() != 2)
	{
		con << "play <demoname> : plays a demo\n";
		return;
	}

	//
	// disconnect from server
	//
	CL_Disconnect();
	
	//
	// open the demo file
	//
	sprintf(name, "demos/%s", Argv(1));
	FL_DefaultExtension(name, ".dem");

	con << "Playing demo from " << name << ".\n";
	if (!FL_FindFile(name, fname))
	{
		con << "ERROR: not found\n";
		return;
	}
#ifdef USEZLIB
	gzdemofile = gzopen(fname, "rb");
	if (!gzdemofile)
	{
		con << "ERROR: couldn't open.\n";
		return;
	}

	gzread(gzdemofile, magic, 4);
	magic[4] = 0;
	if (strcmp(magic, "VDEM"))
	{
		gzclose(gzdemofile);
		con << "ERROR: not a Vavoom demo.\n";
		return;
	}
#else
	cls.demofile = fopen(fname, "rb");
	if (!cls.demofile)
	{
		con << "ERROR: couldn't open.\n";
		return;
	}

	fread(magic, 1, 4, cls.demofile);
	magic[4] = 0;
	if (strcmp(magic, "VDEM"))
	{
		fclose(cls.demofile);
		con << "ERROR: not a Vavoom demo.\n";
		return;
	}
#endif

	cls.demoplayback = true;
	cls.state = ca_connected;
}

//==========================================================================
//
//	COMMAND TimeDemo
//
//	timedemo [demoname]
//
//==========================================================================

COMMAND(TimeDemo)
{
	if (cmd_source != src_command)
	{
		return;
	}

	if (Argc() != 2)
	{
		con << "timedemo <demoname> : gets demo speeds\n";
		return;
	}

	PlayDemo_f.Run();

	// cls.td_starttime will be grabbed at the second frame of the demo, so
	// all the loading time doesn't get counted
	
	cls.timedemo = true;
	cls.td_startframe = host_framecount;
	cls.td_lastframe = -1;		// get a new message this frame
}

//**************************************************************************
//
//	$Log$
//	Revision 1.5  2001/09/05 12:21:42  dj_jl
//	Release changes
//
//	Revision 1.4  2001/08/04 17:25:14  dj_jl
//	Moved title / demo loop to progs
//	Removed shareware / ExtendedWAD from engine
//	
//	Revision 1.3  2001/07/31 17:10:21  dj_jl
//	Localizing demo loop
//	
//	Revision 1.2  2001/07/27 14:27:54  dj_jl
//	Update with Id-s and Log-s, some fixes
//
//**************************************************************************
