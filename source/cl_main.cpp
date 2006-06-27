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
//**	Copyright (C) 1999-2006 Jānis Legzdiņš
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
#include "ui.h"

// MACROS ------------------------------------------------------------------

// TYPES -------------------------------------------------------------------

// EXTERNAL FUNCTION PROTOTYPES --------------------------------------------

void SV_ShutdownServer(boolean crash);
void CL_Disconnect();

void CL_ParseServerMessage(VMessage&);
int CL_GetMessage();
void CL_StopPlayback();
void CL_StopRecording();

// PUBLIC FUNCTION PROTOTYPES ----------------------------------------------

// PRIVATE FUNCTION PROTOTYPES ---------------------------------------------

// EXTERNAL DATA DECLARATIONS ----------------------------------------------

extern VStr			skin_list[256];

// PUBLIC DATA DEFINITIONS -------------------------------------------------

client_static_t		cls;
VClientState*		cl;

VClientGameBase*	GClGame;

VCvarS			cl_name("name", "PLAYER", CVAR_Archive | CVAR_UserInfo);
VCvarI			cl_color("color", "0", CVAR_Archive | CVAR_UserInfo);
VCvarI			cl_class("class", "0", CVAR_Archive | CVAR_UserInfo);
VCvarS			cl_model("model", "", CVAR_Archive | CVAR_UserInfo);
VCvarS			cl_skin("skin", "", CVAR_Archive | CVAR_UserInfo);

dlight_t		cl_dlights[MAX_DLIGHTS];

// PRIVATE DATA DEFINITIONS ------------------------------------------------

IMPLEMENT_CLASS(V, ClientGameBase);
IMPLEMENT_CLASS(V, ClientState);

static bool UserInfoSent;

// CODE --------------------------------------------------------------------

//==========================================================================
//
//	CL_Init
//
//==========================================================================

void CL_Init()
{
	guard(CL_Init);
	VMemberBase::StaticLoadPackage(NAME_clprogs);

	cl_mobjs = new VEntity*[GMaxEntities];
	cl_mo_base = new clmobjbase_t[GMaxEntities];
	memset(cl_mobjs, 0, sizeof(VEntity*) * GMaxEntities);
	memset(cl_mo_base, 0, sizeof(clmobjbase_t) * GMaxEntities);

	cls.message.Alloc(NET_MAXMESSAGE);

	GClGame = (VClientGameBase*)VObject::StaticSpawnObject(
		VClass::FindClass("ClientGame"));
	cl = (VClientState*)VObject::StaticSpawnObject(
		VClass::FindClass("MainClientState"));
	GClGame->cl = cl;
	GClGame->level = &cl_level;
	unguard;
}

//==========================================================================
//
//	CL_Ticker
//
//==========================================================================

void CL_Ticker()
{
	guard(CL_Ticker);
	// do main actions
	switch (cl->intermission)
	{
	case 0:
		SB_Ticker();
		AM_Ticker();
		break;
	}
	unguard;
}

//==========================================================================
//
//	CL_Shutdown
//
//==========================================================================

void CL_Shutdown()
{
	guard(CL_Shutdown);
	//	Disconnect.
	CL_Disconnect();

	//	Free up memory.
	for (int i = 0; i < GMaxEntities; i++)
		if (cl_mobjs[i])
			cl_mobjs[i]->ConditionalDestroy();
	for (int i = 0; i < MAXPLAYERS; i++)
		if (cl_weapon_mobjs[i])
			cl_weapon_mobjs[i]->ConditionalDestroy();
	Z_Free(cl_mobjs);
	Z_Free(cl_mo_base);
	cls.message.Free();
	if (GClLevel)
		GClLevel->ConditionalDestroy();
	GClGame->ConditionalDestroy();
	cl->ConditionalDestroy();
	GRoot->ConditionalDestroy();
	for (int i = 0; i < 256; i++)
	{
		skin_list[i].Clean();
	}
	unguard;
}

//==========================================================================
//
//	CL_AllocDlight
//
//==========================================================================

dlight_t *CL_AllocDlight(int key)
{
	guard(CL_AllocDlight);
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
		if (dl->die < cl->time)
		{
			memset(dl, 0, sizeof(*dl));
			dl->key = key;
			return dl;
		}
	}

	int bestnum = 0;
	float bestdist = 0.0;
	for (i = 0; i < MAX_DLIGHTS; i++, dl++)
	{
		float dist = Length(dl->origin - cl->vieworg);
		if (dist > bestdist)
		{
			bestnum = i;
			bestdist = dist;
		}
	}
	dl = &cl_dlights[bestnum];
	memset(dl, 0, sizeof(*dl));
	dl->key = key;
	return dl;
	unguard;
}

//==========================================================================
//
//	CL_DecayLights
//
//==========================================================================

void CL_DecayLights()
{
	guard(CL_DecayLights);
	int			i;
	dlight_t	*dl;
	float		time;

	time = cl->time - cl->oldtime;

	dl = cl_dlights;
	for (i = 0; i < MAX_DLIGHTS; i++, dl++)
	{
		if (dl->die < cl->time || !dl->radius)
			continue;

		dl->radius -= time * dl->decay;
		if (dl->radius < 0)
			dl->radius = 0;
	}
	unguard;
}

//==========================================================================
//
//	CL_UpdateMobjs
//
//==========================================================================

void CL_UpdateMobjs()
{
	guard(CL_UpdateMobjs);
	for (int i = 0; i < GMaxEntities; i++)
	{
		if (cl_mobjs[i]->InUse)
		{
			GClGame->eventUpdateMobj(cl_mobjs[i], i, host_frametime);
		}
	}
	unguard;
}

//==========================================================================
//
//	CL_ReadFromServer
//
//	Read all incoming data from the server
//
//==========================================================================

void CL_ReadFromServer()
{
	guard(CL_ReadFromServer);
	int		ret;

	if (cls.state != ca_connected)
		return;

	cl->oldtime = cl->time;
	cl->time += host_frametime;
	
	do
	{
		ret = CL_GetMessage();
		if (ret == -1)
		{
			Host_Error("CL_ReadFromServer: lost server connection");
		}
		if (ret)
		{
//			cl->last_received_message = realtime;
			CL_ParseServerMessage(GNet->NetMsg);
		}
	} while (ret && cls.state == ca_connected);

	if (cls.signon == SIGNONS)
	{
		CL_UpdateMobjs();
		CL_Ticker();
	}
	unguard;
}

//==========================================================================
//
//	CL_SignonReply
//
//==========================================================================

void CL_SignonReply()
{
	guard(CL_SignonReply);
	switch (cls.signon)
	{
	 case 1:
		cls.message << (byte)clc_stringcmd << "PreSpawn\n";
		break;

	 case 2:
		R_PreRender();
		if (!UserInfoSent)
		{
			cls.message << (byte)clc_player_info << cls.userinfo;
			UserInfoSent = true;
		}
		cls.message << (byte)clc_stringcmd << "Spawn\n";
		break;

	 case 3:
		cls.message << (byte)clc_stringcmd << "Begin\n";
		break;
	}
	unguard;
}

//==========================================================================
//
//	CL_KeepaliveMessage
//
//	When the client is taking a long time to load stuff, send keepalive
// messages so the server doesn't disconnect.
//
//==========================================================================

void CL_KeepaliveMessage()
{
	guard(CL_KeepaliveMessage);
	float			time;
	static float	lastmsg;
	int				ret;
	VMessage		old;
	byte			olddata[8192];

#ifdef SERVER
	if (sv.active)
		return;		// no need if server is local
#endif
	if (cls.demoplayback)
		return;

	// read messages from server, should just be nops
	old = GNet->NetMsg;
	memcpy(olddata, GNet->NetMsg.Data, GNet->NetMsg.CurSize);
	
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
			if (GNet->NetMsg.ReadByte() != svc_nop)
				Host_Error("CL_KeepaliveMessage: datagram wasn't a nop");
			break;
		}
	} while (ret);

	GNet->NetMsg = old;
	memcpy(GNet->NetMsg.Data, olddata, GNet->NetMsg.CurSize);

	// check time
	time = Sys_Time();
	if (time - lastmsg < 5.0)
		return;
	lastmsg = time;

	// write out a nop
	GCon->Log("--> client to server keepalive");

	cls.message << (byte)clc_nop;
	cls.netcon->SendMessage(&cls.message);
	cls.message.Clear();
	unguard;
}

//==========================================================================
//
//	CL_Disconnect
//
//	Sends a disconnect message to the server
//	This is also called on Host_Error, so it shouldn't cause any errors
//
//==========================================================================

void CL_Disconnect()
{
	guard(CL_Disconnect);
    if (cl->ClientFlags & VClientState::CF_Paused)
    { 
		cl->ClientFlags &= ~VClientState::CF_Paused;
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

		GCon->Log(NAME_Dev, "Sending clc_disconnect");
		if (cls.message.CurSize)
		{
			GCon->Log(NAME_Dev, "Buffer contains data");
		}
		cls.message.Clear();
		cls.message << (byte)clc_disconnect;
		cls.netcon->SendUnreliableMessage(&cls.message);
		cls.message.Clear();
		cls.netcon->Close();
		cls.netcon = NULL;

		cls.state = ca_disconnected;
#ifdef SERVER
		SV_ShutdownServer(false);
#endif
	}

	cls.demoplayback = false;
	cls.timedemo = false;
	cls.signon = 0;
	GClGame->eventDisconnected();
	unguard;
}

//==========================================================================
//
//	CL_EstablishConnection
//
//	Host should be either "local" or a net address to be passed on
//
//==========================================================================

void CL_EstablishConnection(const char* host)
{
	guard(CL_EstablishConnection);
	if (cls.state == ca_dedicated)
	{
		return;
	}

	if (cls.demoplayback)
	{
		return;
	}

	CL_Disconnect();

	cls.netcon = GNet->Connect(host);
	if (!cls.netcon)
	{
		GCon->Log("Failed to connect to the server");
		return;
	}
	GCon->Logf(NAME_Dev, "CL_EstablishConnection: connected to %s", host);

	UserInfoSent = false;

	GClGame->eventConnected();
	cls.state = ca_connected;
	cls.signon = 0;				// need all the signon messages before playing

	MN_DeactivateMenu();
	unguard;
}

//==========================================================================
//
//	COMMAND Connect
//
//==========================================================================

COMMAND(Connect)
{
	CL_EstablishConnection(Args.Num() > 1 ? *Args[1] : "");
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
}

#ifndef SERVER

//==========================================================================
//
//	COMMAND Pause
//
//==========================================================================

COMMAND(Pause)
{
	ForwardToServer();
}

//==========================================================================
//
//  Stats_f
//
//==========================================================================

COMMAND(Stats)
{
	ForwardToServer();
}

//==========================================================================
//
//	COMMAND TeleportNewMap
//
//==========================================================================

COMMAND(TeleportNewMap)
{
	ForwardToServer();
}

//==========================================================================
//
//	COMMAND	Say
//
//==========================================================================

COMMAND(Say)
{
	ForwardToServer();
}

#endif
