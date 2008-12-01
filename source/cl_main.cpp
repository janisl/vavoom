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
#include "sv_local.h"

// MACROS ------------------------------------------------------------------

// TYPES -------------------------------------------------------------------

// EXTERNAL FUNCTION PROTOTYPES --------------------------------------------

void CL_StopPlayback();
void CL_StopRecording();
void CL_SetUpNetClient(VSocketPublic*);
void SV_ConnectClient(VBasePlayer*);
void CL_Clear();
void CL_ReadFromServerInfo();

// PUBLIC FUNCTION PROTOTYPES ----------------------------------------------

// PRIVATE FUNCTION PROTOTYPES ---------------------------------------------

// EXTERNAL DATA DECLARATIONS ----------------------------------------------

// PUBLIC DATA DEFINITIONS -------------------------------------------------

client_static_t		cls;
VBasePlayer*		cl;
VClientNetContext*	ClientNetContext;

VClientGameBase*	GClGame;

VCvarS			cl_name("name", "PLAYER", CVAR_Archive | CVAR_UserInfo);
VCvarI			cl_colour("colour", "0", CVAR_Archive | CVAR_UserInfo);
VCvarI			cl_class("class", "0", CVAR_Archive | CVAR_UserInfo);
VCvarS			cl_model("model", "", CVAR_Archive | CVAR_UserInfo);

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
	VMemberBase::StaticLoadPackage(NAME_cgame, TLocation());
	TLocation::ClearSourceFiles();

	ClientNetContext = new VClientNetContext();
	GClGame = (VClientGameBase*)VObject::StaticSpawnObject(
		VClass::FindClass("ClientGame"));
	GClGame->Game = GGameInfo;
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
	if (GClGame)
		GClGame->ConditionalDestroy();
	if (GRoot)
		GRoot->ConditionalDestroy();
	cls.userinfo.Clean();
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
	if (!cl)
	{
		return;
	}

	if (cl->Net)
	{
		cl->Net->GetMessages();
		if (cl->Net->State == NETCON_Closed)
		{
			Host_EndGame("Server disconnected");
		}
	}

	if (cls.signon)
	{
		if (GGameInfo->NetMode == NM_Client)
		{
			GClLevel->Time += host_frametime;
			GClLevel->TicTime = (int)(GClLevel->Time * 35.0);
		}

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
	if (GGameInfo->NetMode == NM_TitleMap ||
		GGameInfo->NetMode == NM_Standalone ||
		GGameInfo->NetMode == NM_ListenServer)
	{
		cl->SpawnClient();
	}
	else
	{
		cl->Net->SendCommand("PreSpawn\n");
	}
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
	if (GGameInfo->NetMode != NM_Client)
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
		if (cl)
		{
			delete cl->Net;
			cl->ConditionalDestroy();
		}
		cl = NULL;
	}
	else if (cl)
	{
		if (cls.demorecording)
		{
			CL_StopRecording();
		}

		if (cl->Net)
		{
			GCon->Log(NAME_Dev, "Sending clc_disconnect");
			cl->Net->Channels[0]->Close();
			cl->Net->Flush();
		}

		if (GGameInfo->NetMode == NM_Client)
		{
			delete cl->Net;
			cl->ConditionalDestroy();
		}
		cl = NULL;

#ifdef SERVER
		SV_ShutdownServer(false);
#endif
	}

	if (GClLevel && GClLevel != GLevel)
	{
		delete GClLevel;
	}
	GClLevel = NULL;
	if (GGameInfo->NetMode == NM_Client)
	{
		GGameInfo->NetMode = NM_None;
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
//	Host should be a net address to be passed on
//
//==========================================================================

void CL_EstablishConnection(const char* host)
{
	guard(CL_EstablishConnection);
	if (GGameInfo->NetMode == NM_DedicatedServer)
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

	CL_SetUpNetClient(Sock);
	GCon->Logf(NAME_Dev, "CL_EstablishConnection: connected to %s", host);
	GGameInfo->NetMode = NM_Client;

	UserInfoSent = false;

	GClGame->eventConnected();
	cls.signon = 0;				// need all the signon messages before playing

	MN_DeactivateMenu();

	if (GGameInfo->NetMode == NM_TitleMap ||
		GGameInfo->NetMode == NM_Standalone ||
		GGameInfo->NetMode == NM_ListenServer)
	{
		CL_SetUpStandaloneClient();
	}
	unguard;
}

//==========================================================================
//
//	CL_SetUpLocalPlayer
//
//	Host should be either "local" or a net address to be passed on
//
//==========================================================================

void CL_SetUpLocalPlayer()
{
	guard(CL_SetUpLocalPlayer);
	if (GGameInfo->NetMode == NM_DedicatedServer)
	{
		return;
	}

	VBasePlayer* Player = GPlayersBase[0];
	SV_ConnectClient(Player);
	svs.num_connected++;

	cl = Player;
	cl->ClGame = GClGame;
	GClGame->cl = cl;

	if (GDemoRecordingContext)
	{
		GDemoRecordingContext->ClientConnections[0]->Owner = cl;
		((VPlayerChannel*)GDemoRecordingContext->ClientConnections[
			0]->Channels[CHANIDX_Player])->SetPlayer(cl);
	}

	UserInfoSent = false;

	GClGame->eventConnected();
	cls.signon = 0;				// need all the signon messages before playing

	MN_DeactivateMenu();

	CL_SetUpStandaloneClient();
	unguard;
}

//==========================================================================
//
//	CL_SetUpStandaloneClient
//
//==========================================================================

void CL_SetUpStandaloneClient()
{
	guard(CL_SetUpStandaloneClient);
	CL_Clear();

	GClGame->serverinfo = svs.serverinfo;
	CL_ReadFromServerInfo();

	GClGame->maxclients = svs.max_clients;
	GClGame->deathmatch = deathmatch;

	const mapInfo_t& LInfo = P_GetMapInfo(*GLevel->MapName);
	GCon->Log("---------------------------------------");
	GCon->Log(LInfo.GetName());
	GCon->Log("");

	GClLevel = GLevel;
	GClGame->GLevel = GClLevel;

	R_Start(GClLevel);
	GAudio->Start();

	SB_Start();

	for (int i = 0; i < GClLevel->NumStaticLights; i++)
	{
		rep_light_t& L = GClLevel->StaticLights[i];
		GClLevel->RenderData->AddStaticLight(L.Origin, L.Radius, L.Colour);
	}
	GClLevel->RenderData->PreRender();

	CL_SignonReply();

	cls.signon = 1;
	GCon->Log(NAME_Dev, "Client level loaded");
	unguard;
}

//==========================================================================
//
//	CL_SendMove
//
//==========================================================================

void CL_SendMove()
{
	guard(CL_SendMove);
	if (!cl)
	{
		return;
	}

	if (cls.demoplayback || GGameInfo->NetMode == NM_TitleMap)
	{
		return;
	}
	
	if (cls.signon)
	{
		cl->HandleInput();
		if (cl->Net)
		{
			((VPlayerChannel*)cl->Net->Channels[CHANIDX_Player])->Update();
		}
	}

	if (cl->Net)
	{
		cl->Net->Tick();
	}
	unguard;
}

//==========================================================================
//
//	CL_Responder
//
//	Get info needed to make ticcmd_ts for the players.
// 
//==========================================================================

bool CL_Responder(event_t* ev)
{
	guard(CL_Responder);
	if (GGameInfo->NetMode == NM_TitleMap)
	{
		return false;
	}

	if (cl)
	{
		return cl->Responder(ev);
	}
	return false;
	unguard;
}

//==========================================================================
//
//	CL_Clear
//
//==========================================================================

void CL_Clear()
{
	guard(CL_Clear);
	GClGame->serverinfo.Clean();
	GClGame->intermission = 0;
	if (cl)
	{
		cl->ClearInput();
	}
#ifdef SERVER
	if (GGameInfo->NetMode == NM_None || GGameInfo->NetMode == NM_Client)
#endif
	{
		// Make sure all sounds are stopped.
		GAudio->StopAllSound();
	}
	cls.signon = 0;
	unguard;
}

//==========================================================================
//
//	CL_ReadFromServerInfo
//
//==========================================================================

void CL_ReadFromServerInfo()
{
	guard(CL_ReadFromServerInfo);
	VCvar::SetCheating(!!atoi(*Info_ValueForKey(GClGame->serverinfo, "sv_cheats")));
	unguard;
}

//==========================================================================
//
//	CL_DoLoadLevel
//
//==========================================================================

void CL_ParseServerInfo(VMessageIn& msg)
{
	guard(CL_ParseServerInfo);
	CL_Clear();

	msg << GClGame->serverinfo;
	CL_ReadFromServerInfo();

	VStr TmpStr;
	msg << TmpStr;
	VName MapName = *TmpStr;

	GClGame->maxclients = msg.ReadInt(MAXPLAYERS + 1);
	GClGame->deathmatch = msg.ReadInt(256);

	const mapInfo_t& LInfo = P_GetMapInfo(MapName);
	GCon->Log("---------------------------------------");
	GCon->Log(LInfo.GetName());
	GCon->Log("");

	CL_LoadLevel(MapName);
	GClLevel->NetContext = ClientNetContext;

	((VLevelChannel*)cl->Net->Channels[CHANIDX_Level])->SetLevel(GClLevel);

	R_Start(GClLevel);
	GAudio->Start();

	SB_Start();

	GCon->Log(NAME_Dev, "Client level loaded");
	unguard;
}

//==========================================================================
//
//	VClientNetContext::GetLevel
//
//==========================================================================

VLevel* VClientNetContext::GetLevel()
{
	return GClLevel;
}

//==========================================================================
//
//	CL_SetUpNetClient
//
//==========================================================================

void CL_SetUpNetClient(VSocketPublic* Sock)
{
	guard(CL_SetUpNetClient);
	//	Create player structure.
	cl = (VBasePlayer*)VObject::StaticSpawnObject(
		VClass::FindClass("Player"));
	cl->PlayerFlags |= VBasePlayer::PF_IsClient;
	cl->ClGame = GClGame;
	GClGame->cl = cl;

	if (cls.demoplayback)
	{
		cl->Net = new VDemoPlaybackNetConnection(ClientNetContext, cl);
	}
	else if (cls.demorecording)
	{
		cl->Net = new VDemoRecordingNetConnection(Sock, ClientNetContext, cl);
	}
	else
	{
		cl->Net = new VNetConnection(Sock, ClientNetContext, cl);
	}
	ClientNetContext->ServerConnection = cl->Net;
	((VPlayerChannel*)cl->Net->Channels[CHANIDX_Player])->SetPlayer(cl);
	unguard;
}

//==========================================================================
//
//	CL_FinishTimeDemo
//
//==========================================================================

void CL_FinishTimeDemo()
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

void CL_StopPlayback()
{
	guard(CL_StopPlayback);
	if (!cls.demoplayback)
	{
		return;
	}

	delete cls.demofile;
	cls.demoplayback = false;
	cls.demofile = NULL;

	if (cls.timedemo)
	{
		CL_FinishTimeDemo();
	}
	GClGame->eventDemoPlaybackStopped();
	unguard;
}

//==========================================================================
//
//	CL_StopRecording
//
//==========================================================================

void CL_StopRecording()
{
	guard(CL_StopRecording);
	// finish up
	delete cls.demofile;
	cls.demofile = NULL;
	cls.demorecording = false;
	if (GDemoRecordingContext)
	{
		delete GDemoRecordingContext;
		GDemoRecordingContext = NULL;
	}
	GCon->Log("Completed demo");
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

	if (c == 2 && cl)
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
		VCommand::ExecuteString(VStr("map ") + Args[2], SRC_Command, NULL);
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

	if (GGameInfo->NetMode == NM_Standalone ||
		GGameInfo->NetMode == NM_ListenServer)
	{
		GDemoRecordingContext = new VServerNetContext();
		VSocketPublic* Sock = new VDemoRecordingSocket();
		VNetConnection* Conn = new VNetConnection(Sock,
			GDemoRecordingContext, NULL);
		Conn->AutoAck = true;
		GDemoRecordingContext->ClientConnections.Append(Conn);
		Conn->ObjMap->SetUpClassLookup();
		VObjectMapChannel* Chan = (VObjectMapChannel*)Conn->CreateChannel(
			CHANNEL_ObjectMap, -1);
		while (!Conn->ObjMapSent)
		{
			Conn->Tick();
		}
		Conn->SendServerInfo();
	}
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
	if (VStr::Cmp(magic, "VDEM"))
	{
		delete cls.demofile;
		cls.demofile = NULL;
		GCon->Log("ERROR: not a Vavoom demo.");
		return;
	}

	//
	// disconnect from server
	//
	CL_Disconnect();

	cls.demoplayback = true;
	CL_SetUpNetClient(NULL);
	GGameInfo->NetMode = NM_Client;
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
