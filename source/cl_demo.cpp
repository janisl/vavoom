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

// MACROS ------------------------------------------------------------------

// TYPES -------------------------------------------------------------------

// EXTERNAL FUNCTION PROTOTYPES --------------------------------------------

void CL_Disconnect(void);

// PUBLIC FUNCTION PROTOTYPES ----------------------------------------------

// PRIVATE FUNCTION PROTOTYPES ---------------------------------------------

// EXTERNAL DATA DECLARATIONS ----------------------------------------------

// PUBLIC DATA DEFINITIONS -------------------------------------------------

// PRIVATE DATA DEFINITIONS ------------------------------------------------

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

	delete cls.demofile;
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
	*cls.demofile << net_msg.CurSize;
	*cls.demofile << cl.viewangles;
	cls.demofile->Serialize(net_msg.Data, net_msg.CurSize);
	cls.demofile->Flush();
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
	int r;

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
		*cls.demofile << net_msg.CurSize;
//		VectorCopy (cl.mviewangles[0], cl.mviewangles[1]);
		*cls.demofile << cl.viewangles;

		if (net_msg.CurSize > MAX_MSGLEN)
			Sys_Error("Demo message > MAX_MSGLEN");
		cls.demofile->Serialize(net_msg.Data, net_msg.CurSize);
		if (cls.demofile->IsError())
		{
			CL_StopPlayback();
			return 0;
		}
	
		return 1;
	}

	do
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
	while (1);

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
	delete cls.demofile;
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
	char	name[MAX_VPATH];

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
	sprintf(name, "demos/%s", Argv(1));

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
	cls.demofile = FL_OpenFileWrite(name);
	if (!cls.demofile)
	{
		con << "ERROR: couldn't open.\n";
		return;
	}

	cls.demofile->Serialize("VDEM", 4);

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
	cls.demofile = FL_OpenFileRead(name);
	if (!cls.demofile)
	{
		con << "ERROR: couldn't open.\n";
		return;
	}

	cls.demofile->Serialize(magic, 4);
	magic[4] = 0;
	if (strcmp(magic, "VDEM"))
	{
		delete cls.demofile;
		cls.demofile = NULL;
		con << "ERROR: not a Vavoom demo.\n";
		return;
	}

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
//	Revision 1.8  2002/05/18 16:56:34  dj_jl
//	Added FArchive and FOutputDevice classes.
//
//	Revision 1.7  2002/01/07 12:16:41  dj_jl
//	Changed copyright year
//	
//	Revision 1.6  2001/10/18 17:36:31  dj_jl
//	A lots of changes for Alpha 2
//	
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
