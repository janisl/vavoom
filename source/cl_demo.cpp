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

void CL_Disconnect(void);

// MACROS ------------------------------------------------------------------

// TYPES -------------------------------------------------------------------

// EXTERNAL FUNCTION PROTOTYPES --------------------------------------------

// PUBLIC FUNCTION PROTOTYPES ----------------------------------------------

// PRIVATE FUNCTION PROTOTYPES ---------------------------------------------

// EXTERNAL DATA DECLARATIONS ----------------------------------------------

extern TCvarI	shareware;

// PUBLIC DATA DEFINITIONS -------------------------------------------------

// PRIVATE DATA DEFINITIONS ------------------------------------------------

// demo loop control
int				demosequence;	// -1 = don't play demos

float			pagetime;
int				page_pic;

// CODE --------------------------------------------------------------------

//==========================================================================
//
//
//
//==========================================================================
namespace DoomTitle
{
//==========================================================================
//
//	AdvanceDemo
//
//	This cycles through the demo sequences.
//
//==========================================================================

void AdvanceDemo(void)
{
	demosequence++;
    if (Game == Doom && ExtendedWAD)
		demosequence %= 7;
    else
		demosequence %= 6;
    
    switch (demosequence)
    {
      case 0:
		page_pic = R_RegisterPic("TITLEPIC", PIC_PATCH);
if (Game == Doom)
{
		pagetime = 170.0 / 35.0;
		S_StartSong("D_INTRO", P_GetCDTitleTrack(), false);
}
else
{
		pagetime = 11.0;
		S_StartSong("D_DM2TTL", P_GetCDTitleTrack(), false);
}
		break;

      case 1:
		CmdBuf << "PlayDemo demo1\n";
		break;

      case 2:
		pagetime = 200.0 / 35.0;
		page_pic = R_RegisterPic("CREDIT", PIC_PATCH);
		break;

      case 3:
		CmdBuf << "PlayDemo demo2\n";
		break;

      case 4:
if (Game == Doom)
{
		pagetime = 200.0 / 35.0;
		if (ExtendedWAD)
		{
			page_pic = R_RegisterPic("CREDIT", PIC_PATCH);
		}
		else
		{
			page_pic = R_RegisterPic("HELP2", PIC_PATCH);
		}
}
else
{
		pagetime = 11.0;
		page_pic = R_RegisterPic("TITLEPIC", PIC_PATCH);
		S_StartSong("D_DM2TTL", P_GetCDTitleTrack(), false);
}
		break;

      case 5:
		CmdBuf << "PlayDemo demo3\n";
		break;

        // THE DEFINITIVE DOOM Special Edition demo
      case 6:
		CmdBuf << "PlayDemo demo4\n";
		break;
    }
}

}

//==========================================================================
//
//
//
//==========================================================================
namespace HereticTitle
{
//==========================================================================
//
//	AdvanceDemo
//
//	This cycles through the demo sequences.
//
//==========================================================================

void AdvanceDemo(void)
{
	demosequence++;
	demosequence %= 6;
    
    switch (demosequence)
    {
      case 0:
		pagetime = 10.0;
		page_pic = R_RegisterPic("TITLE", PIC_RAW);
		S_StartSong("MUS_TITL", P_GetCDTitleTrack(), false);
		break;

      case 1:
		CmdBuf << "PlayDemo demo1\n";
		break;

      case 2:
		pagetime = 200.0 / 35.0;
		page_pic = R_RegisterPic("CREDIT", PIC_RAW);
		break;

      case 3:
		CmdBuf << "PlayDemo demo2\n";
		break;

      case 4:
		pagetime = 200.0 / 35.0;
		if (shareware)
		{
			page_pic = R_RegisterPic("ORDER", PIC_RAW);
		}
		else
		{
			page_pic = R_RegisterPic("CREDIT", PIC_RAW);
		}
		break;

      case 5:
		CmdBuf << "PlayDemo demo3\n";
		break;
    }
}

}

//==========================================================================
//
//
//
//==========================================================================
namespace HexenTitle
{
//==========================================================================
//
//	AdvanceDemo
//
//	This cycles through the demo sequences.
//
//==========================================================================

void AdvanceDemo(void)
{
	demosequence++;
	demosequence %= 6;
    
    switch (demosequence)
    {
      case 0:
		pagetime = 490.0 / 35.0;
		page_pic = R_RegisterPic("TITLE", PIC_RAW);
		S_StartSong("hexen", P_GetCDTitleTrack(), true);
		break;

      case 1:
		CmdBuf << "PlayDemo demo1\n";
		break;

      case 2:
		pagetime = 200.0 / 35.0;
		page_pic = R_RegisterPic("CREDIT", PIC_RAW);
		break;

      case 3:
		CmdBuf << "PlayDemo demo2\n";
		break;

      case 4:
		pagetime = 200.0 / 35.0;
		page_pic = R_RegisterPic("CREDIT", PIC_RAW);
		break;

      case 5:
		CmdBuf << "PlayDemo demo3\n";
		break;
    }
}

}

//==========================================================================
//
//
//
//==========================================================================
namespace StrifeTitle
{
//==========================================================================
//
//	AdvanceDemo
//
//	This cycles through the demo sequences.
//
//==========================================================================

void AdvanceDemo(void)
{
	demosequence++;
	demosequence %= 4;
    
    switch (demosequence)
    {
      case 0:
		pagetime = 170.0 / 35.0;
		page_pic = R_RegisterPic("TITLEPIC", PIC_PATCH);
  		S_StartSong("D_STRIFE", P_GetCDTitleTrack(), false);
		break;

      case 1:
		CmdBuf << "PlayDemo demo1\n";
		break;

      case 2:
		pagetime = 200.0 / 35.0;
		page_pic = R_RegisterPic("CREDIT", PIC_PATCH);
		break;

      case 3:
		CmdBuf << "PlayDemo demo2\n";
		break;
    }
}

}

//==========================================================================
//
//	AdvanceDemo
//
//	This cycles through the demo sequences.
//
//==========================================================================

void AdvanceDemo(void)
{
	switch (Game)
	{
	 case Doom:
	 case Doom2: DoomTitle::AdvanceDemo(); break;
	 case Heretic: HereticTitle::AdvanceDemo(); break;
	 case Hexen: HexenTitle::AdvanceDemo(); break;
	 case Strife: StrifeTitle::AdvanceDemo(); break;
	}
}

//==========================================================================
//
//	COMMAND StartDemos
//
//==========================================================================

COMMAND(StartDemos)
{
#ifdef SERVER
	if (sv.active)
		return;
#endif
    demosequence = -1;
	AdvanceDemo();
}

//==========================================================================
//
//	G_ForceTitle
//
//==========================================================================

void G_ForceTitle(void)
{
	CL_Disconnect();

   	switch (Game)
	{
	 case Doom:
		pagetime = 5.0;
		page_pic = R_RegisterPic("TITLEPIC", PIC_PATCH);
		break;
	 case Doom2:
		pagetime = 11.0;
		page_pic = R_RegisterPic("TITLEPIC", PIC_PATCH);
		break;
	 case Heretic:
		pagetime = 10.0;
		page_pic = R_RegisterPic("TITLE", PIC_RAW);
		break;
	 case Hexen:
		pagetime = 14.0;
		page_pic = R_RegisterPic("TITLE", PIC_RAW);
		break;
	 case Strife:
		pagetime = 11.0;
		page_pic = R_RegisterPic("TITLEPIC", PIC_PATCH);
		break;
	}
}

//==========================================================================
//
//	G_PageDrawer
//
//==========================================================================

void G_PageDrawer(void)
{
	pagetime -= host_frametime;
    if (demosequence != -1 && !MN_Active() && !consolestate &&
    	!messageToPrint && pagetime < 0.0)
	{
		AdvanceDemo();
	}

	R_DrawPic(0, 0, page_pic);
	if (Game > Doom2)
    {
		if (demosequence == 1)
		{
			R_DrawPic(4, 160, R_RegisterPic("ADVISOR", PIC_PATCH));
		}
	}
}

//==========================================================================
//
//	OnHostEndGame
//
//==========================================================================

void OnHostEndGame(void)
{
	if (demosequence != -1)
		AdvanceDemo();
	G_ForceTitle();
}

//==========================================================================
//
//	OnHostError
//
//==========================================================================

void OnHostError(void)
{
	demosequence = -1;
	G_ForceTitle();
}

//==========================================================================
//
//	StopDemoLoop
//
//==========================================================================

void StopDemoLoop(void)
{
	demosequence = -1;			// not in the demo loop now
}

//==========================================================================
//
//	OnDemoFailed
//
//==========================================================================

void OnDemoFailed(void)
{
#if 0
	demosequence = -1;		// stop demo loop
#else
	AdvanceDemo();
#endif
}

//**************************************************************************
//
//
//
//**************************************************************************

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

	fclose(cls.demofile);
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
		fread(&net_msg.CurSize, 4, 1, cls.demofile);
//		VectorCopy (cl.mviewangles[0], cl.mviewangles[1]);
		fread(&a, 4, 1, cls.demofile);
		cl.viewangles.pitch = LittleLong(a);
		fread(&a, 4, 1, cls.demofile);
		cl.viewangles.yaw = LittleLong(a);
		fread(&a, 4, 1, cls.demofile);
		cl.viewangles.roll = LittleLong(a);
		
		net_msg.CurSize = LittleLong(net_msg.CurSize);
		if (net_msg.CurSize > MAX_MSGLEN)
			Sys_Error("Demo message > MAX_MSGLEN");
		r = fread(net_msg.Data, net_msg.CurSize, 1, cls.demofile);
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
	fclose(cls.demofile);
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

	sprintf(name, "%s/%s", fl_gamedir, Argv(1));

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
	cls.demofile = fopen(name, "wb");
	if (!cls.demofile)
	{
		con << "ERROR: couldn't open.\n";
		return;
	}

	fwrite("VDEM", 1, 4, cls.demofile);

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
	sprintf(name, "%s/%s", fl_gamedir, Argv(1));
	FL_DefaultExtension(name, ".dem");

	con << "Playing demo from " << name << ".\n";
	cls.demofile = fopen(name, "rb");
	if (!cls.demofile)
	{
		con << "ERROR: couldn't open.\n";
		OnDemoFailed();
		return;
	}

	fread(magic, 1, 4, cls.demofile);
	magic[4] = 0;
	if (strcmp(magic, "VDEM"))
	{
		fclose(cls.demofile);
		con << "ERROR: not a Vavoom demo.\n";
		OnDemoFailed();
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
//	Revision 1.3  2001/07/31 17:10:21  dj_jl
//	Localizing demo loop
//
//	Revision 1.2  2001/07/27 14:27:54  dj_jl
//	Update with Id-s and Log-s, some fixes
//
//**************************************************************************
