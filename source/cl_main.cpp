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
#include "network.h"
#include "cl_local.h"
#include "ui.h"

// MACROS ------------------------------------------------------------------

// TYPES -------------------------------------------------------------------

// EXTERNAL FUNCTION PROTOTYPES --------------------------------------------

void SV_ShutdownServer(bool crash);
void CL_Disconnect();

void CL_StopPlayback();
void CL_StopRecording();
void CL_SetUpLocalPlayer(VSocketPublic*);

// PUBLIC FUNCTION PROTOTYPES ----------------------------------------------

// PRIVATE FUNCTION PROTOTYPES ---------------------------------------------

// EXTERNAL DATA DECLARATIONS ----------------------------------------------

extern VLevel*		GClPrevLevel;

// PUBLIC DATA DEFINITIONS -------------------------------------------------

client_static_t		cls;
VBasePlayer*		cl;
VClientNetContext*	ClientNetContext;

VClientGameBase*	GClGame;

VCvarS			cl_name("name", "PLAYER", CVAR_Archive | CVAR_UserInfo);
VCvarI			cl_colour("colour", "0", CVAR_Archive | CVAR_UserInfo);
VCvarI			cl_class("class", "0", CVAR_Archive | CVAR_UserInfo);
VCvarS			cl_model("model", "", CVAR_Archive | CVAR_UserInfo);
VCvarS			cl_skin("skin", "", CVAR_Archive | CVAR_UserInfo);

// PRIVATE DATA DEFINITIONS ------------------------------------------------

IMPLEMENT_CLASS(V, ClientGameBase);

static bool					UserInfoSent;

static VName				CurrentSongLump;
static int					CurrentCDTrack;

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

	ClientNetContext = new VClientNetContext();
	GClGame = (VClientGameBase*)VObject::StaticSpawnObject(
		VClass::FindClass("ClientGame"));
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
	R_AnimateSurfaces();
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
	if (GRoot)
		GRoot->ConditionalDestroy();
	cls.userinfo.Clean();
	im.LeaveName.Clean();
	im.EnterName.Clean();
	im.Text.Clean();
	delete ClientNetContext;
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
		GClLevel->RenderData->DecayLights(host_frametime);
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
	for (TThinkerIterator<VThinker> Th(GClLevel); Th; ++Th)
	{
		Th->eventClientTick(host_frametime);
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

	cl->Net->GetMessages();
	if (cl->Net->State == NETCON_Closed)
	{
		Host_EndGame("Server disconnected");
	}

	if (cls.signon)
	{
		GClLevel->Time += host_frametime;

		CL_UpdateMobjs();
		CL_Ticker();
	}

	if (GClLevel && GClLevel->LevelInfo)
	{
		if (CurrentSongLump != GClLevel->LevelInfo->SongLump ||
			CurrentCDTrack != GClLevel->LevelInfo->CDTrack)
		{
			CurrentSongLump = GClLevel->LevelInfo->SongLump;
			CurrentCDTrack = GClLevel->LevelInfo->CDTrack;
			GAudio->MusicChanged();
		}
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
	if (cls.signon)
	{
		Host_Error("Spawn command already sent");
	}
	if (!UserInfoSent)
	{
		cl->eventServerSetUserInfo(cls.userinfo);
		UserInfoSent = true;
	}
	cl->Net->SendCommand("Spawn\n");
	GCmdBuf << "HideConsole\n";
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
#ifdef SERVER
	if (sv.active)
		return;		// no need if server is local
#endif
	if (cls.demoplayback)
		return;
	// write out a nop
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
		cl->Net->Channels[0]->Close();
		cl->Net->Flush();

		cls.state = ca_disconnected;
#ifdef SERVER
		SV_ShutdownServer(false);
#endif
	}

	if (cl)
	{
		delete cl->Net;
		cl->ViewEnt->ConditionalDestroy();
		cl->ConditionalDestroy();
		cl = NULL;
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

	CL_SetUpLocalPlayer(Sock);
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
