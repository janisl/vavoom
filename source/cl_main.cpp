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

void SV_ShutdownServer(bool crash);
void CL_Disconnect();

void CL_StopPlayback();
void CL_StopRecording();

// PUBLIC FUNCTION PROTOTYPES ----------------------------------------------

// PRIVATE FUNCTION PROTOTYPES ---------------------------------------------

// EXTERNAL DATA DECLARATIONS ----------------------------------------------

extern VStr			skin_list[256];
extern VLevel*		GClPrevLevel;

// PUBLIC DATA DEFINITIONS -------------------------------------------------

client_static_t		cls;
VBasePlayer*		cl;

VClientGameBase*	GClGame;

VCvarS			cl_name("name", "PLAYER", CVAR_Archive | CVAR_UserInfo);
VCvarI			cl_colour("colour", "0", CVAR_Archive | CVAR_UserInfo);
VCvarI			cl_class("class", "0", CVAR_Archive | CVAR_UserInfo);
VCvarS			cl_model("model", "", CVAR_Archive | CVAR_UserInfo);
VCvarS			cl_skin("skin", "", CVAR_Archive | CVAR_UserInfo);

// PRIVATE DATA DEFINITIONS ------------------------------------------------

IMPLEMENT_CLASS(V, ClientGameBase);

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

	GClGame = (VClientGameBase*)VObject::StaticSpawnObject(
		VClass::FindClass("ClientGame"));
	cl = (VBasePlayer*)VObject::StaticSpawnObject(
		VClass::FindClass("Player"));
	cl->ViewEnt = Spawn<VEntity>();
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
	switch (GClGame->intermission)
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
	if (cl)
	{
		//	Disconnect.
		CL_Disconnect();
	}

	//	Free up memory.
	if (GClLevel)
		GClLevel->ConditionalDestroy();
	if (GClPrevLevel)
		GClPrevLevel->ConditionalDestroy();
	if (GClGame)
		GClGame->ConditionalDestroy();
	if (cl)
	{
		cl->ViewEnt->ConditionalDestroy();
		delete cl->Net;
		cl->ConditionalDestroy();
	}
	if (GRoot)
		GRoot->ConditionalDestroy();
	for (int i = 0; i < 256; i++)
	{
		skin_list[i].Clean();
	}
	cls.userinfo.Clean();
	im.LeaveName.Clean();
	im.EnterName.Clean();
	im.Text.Clean();
	cl_level.LevelName.Clean();
	for (int i = 0; i < MAXPLAYERS; i++)
	{
		scores[i].name.Clean();
		scores[i].userinfo.Clean();
	}
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
	if (GClLevel)
	{
		GClLevel->RenderData->DecayLights(GClGame->time - GClGame->oldtime);
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
	for (TThinkerIterator<VEntity> Ent(GClLevel); Ent; ++Ent)
	{
		GClGame->eventUpdateMobj(*Ent, Ent->NetID, host_frametime);
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
	if (cls.state != ca_connected)
		return;

	GClGame->oldtime = GClGame->time;
	GClGame->time += host_frametime;

	cl->Net->GetMessages();
	if (cl->Net->State == NETCON_Closed)
	{
		Host_Error("CL_ReadFromServer: lost server connection");
	}

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
		cl->Net->Message << (byte)clc_stringcmd << "PreSpawn\n";
		break;

	case 2:
		GClLevel->InitPolyobjs();
		GClLevel->RenderData->PreRender();
		if (!UserInfoSent)
		{
			cl->Net->Message << (byte)clc_player_info << cls.userinfo;
			UserInfoSent = true;
		}
		cl->Net->Message << (byte)clc_stringcmd << "Spawn\n";
		break;

	case 3:
		cl->Net->Message << (byte)clc_stringcmd << "Begin\n";
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

#ifdef SERVER
	if (sv.active)
		return;		// no need if server is local
#endif
	if (cls.demoplayback)
		return;

	// read messages from server, should just be nops
	do
	{
		TArray<vuint8> Data;
		ret = cl->Net->GetRawPacket(Data);
		switch (ret)
		{
		default:
			Host_Error("CL_KeepaliveMessage: CL_GetMessage failed");
		case 0:
			break;	// nothing waiting
		case 1:
			{
				VMessageIn Msg(Data.Ptr(), Data.Num());
				if (Msg.ReadBit())
					break;
				if (Msg.ReadBit())
					Host_Error("CL_KeepaliveMessage: received a message");
				if (Msg.ReadByte() != svc_nop)
				{
					Host_Error("CL_KeepaliveMessage: datagram wasn't a nop");
				}
			}
			break;
		}
	} while (ret);

	// check time
	time = Sys_Time();
	if (time - lastmsg < 5.0)
		return;
	lastmsg = time;

	// write out a nop
	GCon->Log("--> client to server keepalive");

	cl->Net->Message << (byte)clc_nop;
	cl->Net->Channels[0]->SendMessage(&cl->Net->Message);
	cl->Net->Message.Clear();
	cl->Net->Flush();
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
	if (GClGame->ClientFlags & VClientGameBase::CF_Paused)
	{
		GClGame->ClientFlags &= ~VClientGameBase::CF_Paused;
		GAudio->ResumeSound();
	} 
	
	// stop sounds (especially looping!)
	GAudio->StopAllSound();
	
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
		if (cl->Net->Message.GetNumBits())
		{
			GCon->Log(NAME_Dev, "Buffer contains data");
		}
		cl->Net->Message.Clear();
		cl->Net->Message << (byte)clc_disconnect;
		cl->Net->Channels[0]->SendMessage(&cl->Net->Message);
		cl->Net->Message.Clear();
		cl->Net->Flush();
		delete cl->Net;
		cl->Net = NULL;

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

	VSocketPublic* Sock = GNet->Connect(host);
	if (!Sock)
	{
		GCon->Log("Failed to connect to the server");
		return;
	}
	cl->Net = new VClientPlayerNetInfo(Sock);
	cl->Net->Chan->SetPlayer(cl);
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
