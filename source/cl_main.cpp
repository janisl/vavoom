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

void StopDemoLoop(void);

// MACROS ------------------------------------------------------------------

// TYPES -------------------------------------------------------------------

// EXTERNAL FUNCTION PROTOTYPES --------------------------------------------

void SV_ShutdownServer(boolean crash);
void CL_Disconnect(void);

void CL_ParseServerMessage(void);
int CL_GetMessage(void);
void CL_StopPlayback(void);
void CL_StopRecording(void);

// PUBLIC FUNCTION PROTOTYPES ----------------------------------------------

// PRIVATE FUNCTION PROTOTYPES ---------------------------------------------

// EXTERNAL DATA DECLARATIONS ----------------------------------------------

// PUBLIC DATA DEFINITIONS -------------------------------------------------

client_static_t		cls;
client_state_t		cl;
TProgs				clpr;

boolean			precache = true;        // if true, load all graphics at start
 
TCvarS			cl_name("name", "PLAYER", CVAR_ARCHIVE | CVAR_USERINFO);
TCvarI			cl_color("color", "0", CVAR_ARCHIVE | CVAR_USERINFO);
TCvarI			cl_class("class", "0", CVAR_ARCHIVE | CVAR_USERINFO);

dlight_t		cl_dlights[MAX_DLIGHTS];

// PRIVATE DATA DEFINITIONS ------------------------------------------------

// CODE --------------------------------------------------------------------

//==========================================================================
//
//	CL_Init
//
//==========================================================================

void CL_Init(void)
{
	clpr.Load("clprogs");
	clpr.SetGlobal("cl", (int)&cl);
	clpr.SetGlobal("level", (int)&cl_level);

	cls.message.Alloc(NET_MAXMESSAGE);
}

//==========================================================================
//
//	CL_Ticker
//
//==========================================================================

void CL_Ticker(void)
{
    // do main actions
    switch (cl.intermission)
    { 
      case 0:
		SB_Ticker();
		AM_Ticker();
		break;

      case 1:
		IM_Ticker();
		break;
			 
      case 2:
		F_Ticker();
		break;
    }
}

//==========================================================================
//
//	CL_AllocDlight
//
//==========================================================================

dlight_t *CL_AllocDlight(int key)
{
	int			i;
	dlight_t	*dl;

	// first look for an exact key match
	if (key)
	{
		dl = cl_dlights;
		for (i = 0; i < MAX_DLIGHTS; i++, dl++)
		{
			if (dl->key == key)
			{
				memset(dl, 0, sizeof(*dl));
				dl->key = key;
				return dl;
			}
		}
	}

	// then look for anything else
	dl = cl_dlights;
	for (i = 0; i < MAX_DLIGHTS; i++, dl++)
	{
		if (dl->die < cl.time)
		{
			memset(dl, 0, sizeof(*dl));
			dl->key = key;
			return dl;
		}
	}

#if 1
	int bestnum = 0;
	float bestdist = 0.0;
	for (i = 0; i < MAX_DLIGHTS; i++, dl++)
	{
		float dist = Length(dl->origin - cl.vieworg);
		if (dist > bestdist)
		{
			bestnum = i;
			bestdist = dist;
		}
	}
	dl = &cl_dlights[bestnum];
#else
	dl = &cl_dlights[0];
#endif
	memset(dl, 0, sizeof(*dl));
	dl->key = key;
	return dl;
}

//==========================================================================
//
//	CL_DecayLights
//
//==========================================================================

void CL_DecayLights(void)
{
	int			i;
	dlight_t	*dl;
	float		time;
	
//	time = cl.time - cl.oldtime;
	time = host_frametime;

	dl = cl_dlights;
	for (i = 0; i < MAX_DLIGHTS; i++, dl++)
	{
		if (dl->die < cl.time || !dl->radius)
			continue;
		
		dl->radius -= time * dl->decay;
		if (dl->radius < 0)
			dl->radius = 0;
	}
}

//==========================================================================
//
//	CL_UpdateMobjs
//
//==========================================================================

void CL_UpdateMobjs(void)
{
	int pfunc = clpr.FuncNumForName("CL_UpdateMobj");
	for (int i = 0; i < MAX_MOBJS; i++)
	{
		if (cl_mobjs[i].in_use)
		{
			clpr.Exec(pfunc, (int)&cl_mobjs[i], i);
		}
	}
}

//==========================================================================
//
//	CL_ReadFromServer
//
//	Read all incoming data from the server
//
//==========================================================================

void CL_ReadFromServer(void)
{
	int		ret;

	if (cls.state != ca_connected)
		return;

//	cl.oldtime = cl.time;
	cl.time += host_frametime;
	
	do
    {
		ret = CL_GetMessage();
		if (ret == -1)
		{
			Host_Error("CL_ReadFromServer: lost server connection");
		}
		if (ret)
		{
//			cl.last_received_message = realtime;
			CL_ParseServerMessage();
		}
	} while (ret && cls.state == ca_connected);

	CL_UpdateMobjs();
   	CL_Ticker();
}

//==========================================================================
//
//	CL_SignonReply
//
//==========================================================================

void CL_SignonReply(void)
{
	switch (cls.signon)
	{
	 case 1:
		cls.message << (byte)clc_stringcmd << "PreSpawn\n";
		break;

	 case 2:
		R_PreRender();
		cls.message << (byte)clc_player_info << cls.userinfo;
		cls.message << (byte)clc_stringcmd << "Spawn\n";
		break;

	 case 3:
		cls.message << (byte)clc_stringcmd << "Begin\n";
		break;
	}
}

//==========================================================================
//
//	CL_KeepaliveMessage
//
//	When the client is taking a long time to load stuff, send keepalive
// messages so the server doesn't disconnect.
//
//==========================================================================

void CL_KeepaliveMessage(void)
{
	float			time;
	static float	lastmsg;
	int				ret;
	TMessage		old;
	byte			olddata[8192];

#ifdef SERVER
	if (sv.active)
		return;		// no need if server is local
#endif
	if (cls.demoplayback)
		return;

	// read messages from server, should just be nops
	old = net_msg;
	memcpy(olddata, net_msg.Data, net_msg.CurSize);
	
	do
	{
		ret = CL_GetMessage();
		switch (ret)
		{
		 default:
			Host_Error("CL_KeepaliveMessage: CL_GetMessage failed");
		 case 0:
			break;	// nothing waiting
		 case 1:
			Host_Error("CL_KeepaliveMessage: received a message");
			break;
		 case 2:
			if (net_msg.ReadByte() != svc_nop)
				Host_Error("CL_KeepaliveMessage: datagram wasn't a nop");
			break;
		}
	} while (ret);

	net_msg = old;
	memcpy(net_msg.Data, olddata, net_msg.CurSize);

	// check time
	time = Sys_Time();
	if (time - lastmsg < 5.0)
		return;
	lastmsg = time;

	// write out a nop
	con << "--> client to server keepalive\n";

	cls.message << (byte)clc_nop;
	NET_SendMessage(cls.netcon, &cls.message);
	cls.message.Clear();
}

//==========================================================================
//
//	CL_Disconnect
//
//	Sends a disconnect message to the server
//	This is also called on Host_Error, so it shouldn't cause any errors
//
//==========================================================================

void CL_Disconnect(void)
{
    if (cl.paused)
    { 
		cl.paused = false;
		S_ResumeSound();
    } 
	
	// stop sounds (especially looping!)
	S_StopAllSound();
	
	// if running a local server, shut it down
	if (cls.demoplayback)
	{
		CL_StopPlayback();
	}
	else if (cls.state == ca_connected)
	{
		if (cls.demorecording)
		{
			CL_StopRecording();
		}

		cond << "Sending clc_disconnect\n";
		if (cls.message.CurSize)
		{
			cond << "Buffer contains data\n";
		}
		cls.message.Clear();
		cls.message << (byte)clc_disconnect;
		NET_SendUnreliableMessage(cls.netcon, &cls.message);
		cls.message.Clear();
		NET_Close(cls.netcon);
		cls.netcon = NULL;

		cls.state = ca_disconnected;
#ifdef SERVER
		SV_ShutdownServer(false);
#endif
	}

	cls.demoplayback = false;
	cls.timedemo = false;
	cls.signon = 0;
}

//==========================================================================
//
//	CL_EstablishConnection
//
//	Host should be either "local" or a net address to be passed on
//
//==========================================================================

void CL_EstablishConnection(char *host)
{
	if (cls.state == ca_dedicated)
	{
		return;
	}

	if (cls.demoplayback)
	{
		return;
	}

	CL_Disconnect();

	cls.netcon = NET_Connect(host);
	if (!cls.netcon)
	{
		con << "Failed to connect to the server\n";
		return;
	}
	cond << "CL_EstablishConnection: connected to " << host << endl;
	
	StopDemoLoop();
	cls.state = ca_connected;
	cls.signon = 0;				// need all the signon messages before playing
//	cls.message.Clear();

	MN_DeactivateMenu();
}

//==========================================================================
//
//	COMMAND Connect
//
//==========================================================================

COMMAND(Connect)
{
	CL_EstablishConnection(Argv(1));
}

//==========================================================================
//
//	COMMAND Reconnect
//
//==========================================================================

COMMAND(Reconnect)
{
	cls.signon = 0;
}

//==========================================================================
//
//	COMMAND Disconnect
//
//==========================================================================

COMMAND(Disconnect)
{
	CL_Disconnect();
#ifdef SERVER
	SV_ShutdownServer(false);
#endif

	CmdBuf << "StartDemos\n";
}

#ifndef SERVER

//==========================================================================
//
//	COMMAND Pause
//
//==========================================================================

COMMAND(Pause)
{
	Cmd_ForwardToServer();
}

//==========================================================================
//
//  Stats_f
//
//==========================================================================

COMMAND(Stats)
{
	Cmd_ForwardToServer();
}

//==========================================================================
//
//	COMMAND TeleportNewMap
//
//==========================================================================

COMMAND(TeleportNewMap)
{
	Cmd_ForwardToServer();
}

//==========================================================================
//
//	COMMAND	Say
//
//==========================================================================

COMMAND(Say)
{
	Cmd_ForwardToServer();
}

#endif

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
