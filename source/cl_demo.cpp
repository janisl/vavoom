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
//**	Whenever cl->time gets past the last received message, another message
//**  is read from the demo file.
//**
//**************************************************************************

// HEADER FILES ------------------------------------------------------------

#include "gamedefs.h"
#include "cl_local.h"

// MACROS ------------------------------------------------------------------

// TYPES -------------------------------------------------------------------

// EXTERNAL FUNCTION PROTOTYPES --------------------------------------------

void CL_Disconnect();

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
	guard(CL_FinishTimeDemo);
	int		frames;
	float	time;
	
	cls.timedemo = false;
	
	// the first frame didn't count
	frames = (host_framecount - cls.td_startframe) - 1;
	time = realtime - cls.td_starttime;
	if (!time)
		time = 1;
	GCon->Logf("%d frames %f seconds %f fps", frames, time, frames / time);
	unguard;
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
	guard(CL_StopPlayback);
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
	GClGame->eventDemoPlaybackStopped();
	unguard;
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
	guard(CL_WriteDemoMessage);
	*cls.demofile << net_msg.CurSize;
	*cls.demofile << cl->viewangles;
	cls.demofile->Serialise(net_msg.Data, net_msg.CurSize);
	cls.demofile->Flush();
	unguard;
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
	guard(CL_GetMessage);
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
			else if ( /* cl->time > 0 && */ cl->time <= cl_level.time)
			{
				return 0;		// don't need another message yet
			}
		}

		// get the next message
		*cls.demofile << net_msg.CurSize;
//		VectorCopy (cl->mviewangles[0], cl->mviewangles[1]);
		*cls.demofile << cl->viewangles;

		if (net_msg.CurSize > MAX_MSGLEN)
			Sys_Error("Demo message > MAX_MSGLEN");
		cls.demofile->Serialise(net_msg.Data, net_msg.CurSize);
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
			GCon->Log("<-- server to client keepalive");
		else
			break;
	}
	while (1);

	if (cls.demorecording)
	{
		CL_WriteDemoMessage();
	}
	
	return r;
	unguard;
}

//==========================================================================
//
//	CL_StopRecording
//
//==========================================================================

void CL_StopRecording(void)
{
	guard(CL_StopRecording);
	// write a disconnect message to the demo file
	net_msg.Clear();
	net_msg << (byte)svc_disconnect;
	CL_WriteDemoMessage();

	// finish up
	delete cls.demofile;
	cls.demofile = NULL;
	cls.demorecording = false;
	GCon->Log("Completed demo");
	unguard;
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
	guard(COMMAND StopDemo);
	if (Source != SRC_Command)
	{
		return;
	}

	if (!cls.demorecording)
	{
		GCon->Log("Not recording a demo.");
		return;
	}
	CL_StopRecording();
	unguard;
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
	guard(COMMAND Record);
	if (Source != SRC_Command)
	{
		return;
	}

	int c = Args.Num();
	if (c != 2 && c != 3)
	{
		GCon->Log("record <demoname> [<map>]");
		return;
	}

	if (strstr(*Args[1], ".."))
	{
		GCon->Log("Relative pathnames are not allowed.");
		return;
	}

	if (c == 2 && cls.state == ca_connected)
	{
		GCon->Log("Can not record - already connected to server");
		GCon->Log("Client demo recording must be started before connecting");
		return;
	}

	VStr name = VStr("demos/") + Args[1].DefaultExtension(".dem");

	//
	// start the map up
	//
	if (c > 2)
	{
		VCommand::ExecuteString(VStr("map ") + Args[2], SRC_Command);
	}
	
	//
	// open the demo file
	//

	GCon->Logf("recording to %s.", *name);
	cls.demofile = FL_OpenFileWrite(name);
	if (!cls.demofile)
	{
		GCon->Log("ERROR: couldn't open.");
		return;
	}

	cls.demofile->Serialise(const_cast<char*>("VDEM"), 4);

	cls.demorecording = true;
	unguard;
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
	guard(COMMAND PlayDemo);
	char	magic[8];

	if (Source != SRC_Command)
	{
		return;
	}

	if (Args.Num() != 2)
	{
		GCon->Log("play <demoname> : plays a demo");
		return;
	}

	//
	// disconnect from server
	//
	CL_Disconnect();
	
	//
	// open the demo file
	//
	VStr name = VStr("demos/") + Args[1].DefaultExtension(".dem");

	GCon->Logf("Playing demo from %s.", *name);
	cls.demofile = FL_OpenFileRead(name);
	if (!cls.demofile)
	{
		GCon->Log("ERROR: couldn't open.");
		return;
	}

	cls.demofile->Serialise(magic, 4);
	magic[4] = 0;
	if (strcmp(magic, "VDEM"))
	{
		delete cls.demofile;
		cls.demofile = NULL;
		GCon->Log("ERROR: not a Vavoom demo.");
		return;
	}

	cls.demoplayback = true;
	cls.state = ca_connected;
	GClGame->eventDemoPlaybackStarted();
	unguard;
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
	guard(COMMAND TimeDemo);
	if (Source != SRC_Command)
	{
		return;
	}

	if (Args.Num() != 2)
	{
		GCon->Log("timedemo <demoname> : gets demo speeds");
		return;
	}

	PlayDemo_f.Run();

	// cls.td_starttime will be grabbed at the second frame of the demo, so
	// all the loading time doesn't get counted
	
	cls.timedemo = true;
	cls.td_startframe = host_framecount;
	cls.td_lastframe = -1;		// get a new message this frame
	unguard;
}

//**************************************************************************
//
//	$Log$
//	Revision 1.17  2006/04/05 17:23:37  dj_jl
//	More dynamic string usage in console command class.
//	Added class for handling command line arguments.
//
//	Revision 1.16  2006/03/04 16:01:34  dj_jl
//	File system API now uses strings.
//	
//	Revision 1.15  2006/02/20 22:52:56  dj_jl
//	Changed client state to a class.
//	
//	Revision 1.14  2006/02/09 22:35:54  dj_jl
//	Moved all client game code to classes.
//	
//	Revision 1.13  2005/12/25 19:20:02  dj_jl
//	Moved title screen into a class.
//	
//	Revision 1.12  2004/12/03 16:15:46  dj_jl
//	Implemented support for extended ACS format scripts, functions, libraries and more.
//	
//	Revision 1.11  2002/08/05 17:20:00  dj_jl
//	Added guarding.
//	
//	Revision 1.10  2002/07/23 16:29:55  dj_jl
//	Replaced console streams with output device class.
//	
//	Revision 1.9  2002/05/29 16:54:33  dj_jl
//	Added const cast.
//	
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
