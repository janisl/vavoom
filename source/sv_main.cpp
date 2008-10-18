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
#include "sv_local.h"

// MACROS ------------------------------------------------------------------

#define TOCENTRE				-128

// TYPES -------------------------------------------------------------------

// EXTERNAL FUNCTION PROTOTYPES --------------------------------------------

// PUBLIC FUNCTION PROTOTYPES ----------------------------------------------

// PRIVATE FUNCTION PROTOTYPES ---------------------------------------------

static void G_DoReborn(int playernum);
static void G_DoCompleted();

// EXTERNAL DATA DECLARATIONS ----------------------------------------------

// PUBLIC DATA DEFINITIONS -------------------------------------------------

VCvarI			real_time("real_time", "1");

server_t		sv;
server_static_t	svs;

// increment every time a check is made
int				validcount = 1;

bool			sv_loading = false;
int				sv_load_num_players;
bool			run_open_scripts;

VBasePlayer*	GPlayersBase[MAXPLAYERS];

bool			paused;

vuint8			deathmatch = false;   	// only if started as net death
bool			netgame;                // only true if packets are broadcast

int 			TimerGame;

VLevelInfo*		GLevelInfo;

int 			LeavePosition;

bool			completed;

// PRIVATE DATA DEFINITIONS ------------------------------------------------

static int		RebornPosition;	// Position indicator for cooperative net-play reborn

static bool		mapteleport_issued;

static VCvarI	TimeLimit("TimeLimit", "0");
static VCvarI	DeathMatch("DeathMatch", "0", CVAR_ServerInfo);
static VCvarI  	NoMonsters("NoMonsters", "0");
static VCvarI	Skill("Skill", "2");
static VCvarI	sv_cheats("sv_cheats", "0", CVAR_ServerInfo | CVAR_Latch);
static VCvarI	split_frame("split_frame", "1", CVAR_Archive);
static VCvarI	sv_maxmove("sv_maxmove", "400", CVAR_Archive);
static VCvarI	use_standalone("use_standalone", "1", CVAR_Archive);

static VServerNetContext*	ServerNetContext;

// CODE --------------------------------------------------------------------

//==========================================================================
//
//	SV_Init
//
//==========================================================================

void SV_Init()
{
	guard(SV_Init);
	int		i;

	svs.max_clients = 1;

	VMemberBase::StaticLoadPackage(NAME_game, TLocation());

	GGameInfo = (VGameInfo*)VObject::StaticSpawnObject(
		VClass::FindClass("MainGameInfo"));
	GGameInfo->eventInit();

	ProcessDecorateScripts();

	ProcessDehackedFiles();

	for (int i = 0; i < VClass::GSpriteNames.Num(); i++)
	{
		R_InstallSprite(*VClass::GSpriteNames[i], i);
	}

	ServerNetContext = new VServerNetContext();

	VClass* PlayerClass = VClass::FindClass("Player");
	for (i = 0; i < MAXPLAYERS; i++)
	{
		GPlayersBase[i] = (VBasePlayer*)VObject::StaticSpawnObject(
			PlayerClass);
	}

	GGameInfo->validcount = &validcount;
	GGameInfo->skyflatnum = skyflatnum;
	VEntity::InitFuncIndexes();

	P_InitSwitchList();
	P_InitTerrainTypes();
	InitLockDefs();
	unguard;
}

//==========================================================================
//
//	SV_Shutdown
//
//==========================================================================

void SV_Shutdown()
{
	guard(SV_Shutdown);
	SV_ShutdownServer(false);
	if (GGameInfo)
		GGameInfo->ConditionalDestroy();
	for (int i = 0; i < MAXPLAYERS; i++)
	{
		if (GPlayersBase[i])
		{
			delete GPlayersBase[i]->Net;
			GPlayersBase[i]->ConditionalDestroy();
		}
	}

	P_FreeTerrainTypes();
	ShutdownLockDefs();
	svs.serverinfo.Clean();

	delete ServerNetContext;
	unguard;
}

//==========================================================================
//
//	SV_Clear
//
//==========================================================================

void SV_Clear()
{
	guard(SV_Clear);
	if (GLevel)
	{
		GLevel->ConditionalDestroy();
		GLevel = NULL;
		VObject::CollectGarbage();
	}
	memset(&sv, 0, sizeof(sv));
#ifdef CLIENT
	// Make sure all sounds are stopped.
	GAudio->StopAllSound();
#endif
	unguard;
}

//==========================================================================
//
//	VLevel::SpawnThinker
//
//==========================================================================

VThinker* VLevel::SpawnThinker(VClass* AClass, const TVec& AOrigin,
	const TAVec& AAngles, mthing_t* mthing, bool AllowReplace)
{
	guard(VLevel::SpawnThinker);
	VClass* Class = AllowReplace ? AClass->GetReplacement() : AClass;
	VThinker* Ret = (VThinker*)StaticSpawnObject(Class);
	AddThinker(Ret);

	if (this == GLevel && Class->IsChildOf(VEntity::StaticClass()))
	{
		((VEntity*)Ret)->Origin = AOrigin;
		((VEntity*)Ret)->Angles = AAngles;
		((VEntity*)Ret)->eventOnMapSpawn(mthing);
		if (GLevelInfo->LevelInfoFlags2 & VLevelInfo::LIF2_BegunPlay)
		{
			((VEntity*)Ret)->eventBeginPlay();
		}
	}
	return Ret;
	unguard;
}

//==========================================================================
//
//	SV_WriteViewData
//
//==========================================================================

void SV_WriteViewData(VBasePlayer &player)
{
	guard(SV_WriteViewData);
	player.ViewEntAlpha = player.MO->Alpha;

	//	Update bam_angles (after teleportation)
	if (player.PlayerFlags & VBasePlayer::PF_FixAngle)
	{
		player.PlayerFlags &= ~VBasePlayer::PF_FixAngle;
		player.eventClientSetAngles(player.ViewAngles);
	}
	unguard;
}

//==========================================================================
//
//	IsRelevant
//
//==========================================================================

static bool IsRelevant(VThinker* Th, VNetConnection* Connection)
{
	guardSlow(IsRelevant);
	if (Th->ThinkerFlags & VThinker::TF_AlwaysRelevant)
		return true;
	VEntity* Ent = Cast<VEntity>(Th);
	if (!Ent)
		return false;
	if (Ent->GetTopOwner() == Connection->Owner->MO)
		return true;
	if (Ent->EntityFlags & VEntity::EF_NoSector)
		return false;
	if (Ent->EntityFlags & VEntity::EF_Invisible)
		return false;
	if (!Connection->CheckFatPVS(Ent->SubSector))
		return false;
	return true;
	unguardSlow;
}

//==========================================================================
//
//	SV_UpdateLevel
//
//==========================================================================

static void SV_UpdateLevel(VBasePlayer* Player)
{
	guard(SV_UpdateLevel);
	int		i;

	Player->Net->SetUpFatPVS();

	((VLevelChannel*)Player->Net->Channels[CHANIDX_Level])->Update();

	//	Mark all entity channels as not updated in this frame.
	for (i = Player->Net->OpenChannels.Num() - 1; i >= 0; i--)
	{
		VChannel* Chan = Player->Net->OpenChannels[i];
		if (Chan->Type == CHANNEL_Thinker)
		{
			((VThinkerChannel*)Chan)->UpdatedThisFrame = false;
		}
	}

	//	Update mobjs in sight
	for (TThinkerIterator<VThinker> Th(GLevel); Th; ++Th)
	{
		if (!IsRelevant(*Th, Player->Net))
		{
			continue;
		}
		VThinkerChannel* Chan = Player->Net->ThinkerChannels.FindPtr(*Th);
		if (!Chan)
		{
			Chan = (VThinkerChannel*)Player->Net->CreateChannel(
				CHANNEL_Thinker, -1);
			if (!Chan)
			{
				continue;
			}
			Chan->SetThinker(*Th);
		}
		Chan->Update();
	}

	//	Close entity channels that were not updated in this frame.
	for (i = Player->Net->OpenChannels.Num() - 1; i >= 0; i--)
	{
		VChannel* Chan = Player->Net->OpenChannels[i];
		if (Chan->Type == CHANNEL_Thinker &&
			!((VThinkerChannel*)Chan)->UpdatedThisFrame)
		{
			Chan->Close();
		}
	}
	unguard;
}

//==========================================================================
//
//	SV_SendClientMessages
//
//==========================================================================

void SV_SendClientMessages()
{
	guard(SV_SendClientMessages);
	//	Update player replication infos.
	for (int i = 0; i < svs.max_clients; i++)
	{
		if (!GGameInfo->Players[i])
		{
			continue;
		}

		VBasePlayer* Player = GGameInfo->Players[i];

		VPlayerReplicationInfo* RepInfo = Player->PlayerReplicationInfo;
		RepInfo->PlayerName = Player->PlayerName;
		RepInfo->UserInfo = Player->UserInfo;
		RepInfo->TranslStart = Player->TranslStart;
		RepInfo->TranslEnd = Player->TranslEnd;
		RepInfo->Colour = Player->Colour;
		RepInfo->Frags = Player->Frags;
		RepInfo->Deaths = Player->Deaths;
		RepInfo->KillCount = Player->KillCount;
		RepInfo->ItemCount = Player->ItemCount;
		RepInfo->SecretCount = Player->SecretCount;
	}

	for (int i = 0; i < svs.max_clients; i++)
	{
		VBasePlayer* Player = GGameInfo->Players[i];
		if (!Player)
		{
			continue;
		}

		// Don't update level if the player isn't totally in the game yet
		if (Player->Net && Player->Net->Channels[0] &&
			(Player->PlayerFlags & VBasePlayer::PF_Spawned))
		{
			if (Player->Net->NeedsUpdate)
			{
				Player->MO->EntityFlags |= VEntity::EF_NetLocalPlayer;

				SV_WriteViewData(*Player);

				SV_UpdateLevel(Player);

				Player->MO->EntityFlags &= ~VEntity::EF_NetLocalPlayer;
			}

			((VPlayerChannel*)Player->Net->Channels[CHANIDX_Player])->Update();
		}
		else if (host_standalone && (Player->PlayerFlags & VBasePlayer::PF_Spawned))
		{
			Player->MO->EntityFlags |= VEntity::EF_NetLocalPlayer;
			SV_WriteViewData(*Player);
		}

		if (Player->Net)
		{
			Player->Net->Tick();
			if (Player->Net->State == NETCON_Closed)
			{
				SV_DropClient(Player, true);
			}
		}
	}
	unguard;
}

//========================================================================
//
//	CheckForSkip
//
//	Check to see if any player hit a key
//
//========================================================================

static void CheckForSkip()
{
	int   			i;
	VBasePlayer		*player;
	static bool		triedToSkip;
	bool			skip = false;

	for (i = 0; i < MAXPLAYERS; i++)
	{
		player = GGameInfo->Players[i];
		if (player)
		{
			if (player->Buttons & BT_ATTACK)
			{
				if (!(player->PlayerFlags & VBasePlayer::PF_AttackDown))
				{
					skip = true;
				}
				player->PlayerFlags |= VBasePlayer::PF_AttackDown;
			}
			else
			{
				player->PlayerFlags &= ~VBasePlayer::PF_AttackDown;
			}
			if (player->Buttons & BT_USE)
			{
				if (!(player->PlayerFlags & VBasePlayer::PF_UseDown))
				{
					skip = true;
				}
				player->PlayerFlags |= VBasePlayer::PF_UseDown;
			}
			else
			{
				player->PlayerFlags &= ~VBasePlayer::PF_UseDown;
			}
		}
	}

	if (deathmatch && sv.intertime < 140)
	{
		// wait for 4 seconds before allowing a skip
		if (skip)
		{
			triedToSkip = true;
			skip = false;
		}
	}
	else
	{
		if (triedToSkip)
		{
			skip = true;
			triedToSkip = false;
		}
	}
	if (skip)
	{
		for (int i = 0; i < svs.max_clients; i++)
			if (GGameInfo->Players[i])
				GGameInfo->Players[i]->eventClientSkipIntermission();
	}
}

//==========================================================================
//
//	SV_RunClients
//
//==========================================================================

void SV_RunClients()
{
	guard(SV_RunClients);
	// get commands
	for (int i = 0; i < MAXPLAYERS; i++)
	{
		VBasePlayer* Player = GGameInfo->Players[i];
		if (!Player)
		{
			continue;
		}

		if ((Player->PlayerFlags & VBasePlayer::PF_IsBot) &&
			!(Player->PlayerFlags & VBasePlayer::PF_Spawned))
		{
			Player->SpawnClient();
		}

		// do player reborns if needed
		if (Player->PlayerState == PST_REBORN)
		{
			G_DoReborn(i);
		}

		if (Player->Net)
		{
			Player->Net->NeedsUpdate = false;
			Player->Net->GetMessages();
		}

		// pause if in menu or console and at least one tic has been run
#ifdef CLIENT
		if (Player->PlayerFlags & VBasePlayer::PF_Spawned &&
			!sv.intermission && !paused &&
			(netgame || !(MN_Active() || C_Active())))
#else
		if (Player->PlayerFlags & VBasePlayer::PF_Spawned &&
			!sv.intermission && !paused)
#endif
		{
			Player->ForwardMove = Player->ClientForwardMove;
			Player->SideMove = Player->ClientSideMove;
			// Don't move faster than maxmove
			if (Player->ForwardMove > sv_maxmove)
			{
				Player->ForwardMove = sv_maxmove;
			}
			else if (Player->ForwardMove < -sv_maxmove)
			{
				Player->ForwardMove = -sv_maxmove;
			}
			if (Player->SideMove > sv_maxmove)
			{
				Player->SideMove = sv_maxmove;
			}
			else if (Player->SideMove < -sv_maxmove)
			{
				Player->SideMove = -sv_maxmove;
			}
			//	Check for disabled freelook and jumping
			if (GLevelInfo->LevelInfoFlags & VLevelInfo::LIF_NoFreelook)
			{
				Player->ViewAngles.pitch = 0;
			}
			if (GLevelInfo->LevelInfoFlags & VLevelInfo::LIF_NoJump)
			{
				Player->Buttons &= ~BT_JUMP;
			}
			Player->eventPlayerTick(host_frametime);
		}
	}

	if (sv.intermission)
	{
		CheckForSkip();
		sv.intertime++;
	}
	unguard;
}

//==========================================================================
//
//	SV_Ticker
//
//==========================================================================

void SV_Ticker()
{
	guard(SV_Ticker);
	float	saved_frametime;
	int		exec_times;

	saved_frametime = host_frametime;
	exec_times = 1;
	if (!real_time)
	{
		// Rounded a little bit up to prevent "slow motion"
		host_frametime = 0.028572f;//1.0 / 35.0;
	}
	else if (split_frame)
	{
		while (host_frametime / exec_times > 1.0 / 35.0)
			exec_times++;
	}

	GGameInfo->frametime = host_frametime;
	SV_RunClients();

	if (sv_loading)
		return;

	// do main actions
	if (!sv.intermission)
	{
		host_frametime /= exec_times;
		GGameInfo->frametime = host_frametime;
		for (int i = 0; i < exec_times && !completed; i++)
		{
			// pause if in menu or console
#ifdef CLIENT
			if (host_titlemap || (!paused && (netgame || !(MN_Active() ||
				C_Active()))))
#else
			if (host_titlemap || !paused)
#endif
			{
				//	LEVEL TIMER
				if (TimerGame)
				{
					if (!--TimerGame)
					{
						LeavePosition = 0;
						completed = true;
					}
				}
				if (i)
					VObject::CollectGarbage();
				P_Ticker();
			}
		}
	}

	if (completed)
	{
		G_DoCompleted();
	}

	host_frametime = saved_frametime;
	unguard;
}

//==========================================================================
//
//	SV_ChangeSky
//
//==========================================================================

void SV_ChangeSky(const char* Sky1, const char* Sky2)
{
	guard(SV_ChangeSky);
	GLevelInfo->Sky1Texture = GTextureManager.NumForName(VName(Sky1,
		VName::AddLower8), TEXTYPE_Wall, true, false);
	GLevelInfo->Sky2Texture = GTextureManager.NumForName(VName(Sky2,
		VName::AddLower8), TEXTYPE_Wall, true, false);
	unguard;
}

//==========================================================================
//
//	SV_ChangeMusic
//
//==========================================================================

void SV_ChangeMusic(const char* SongName)
{
	guard(SV_ChangeMusic);
	GLevelInfo->SongLump = VName(SongName, VName::AddLower8);
	unguard;
}

//==========================================================================
//
//	SV_ChangeLocalMusic
//
//==========================================================================

void SV_ChangeLocalMusic(VBasePlayer *player, const char* SongName)
{
	guard(SV_ChangeLocalMusic);
	player->eventClientChangeMusic(SongName, 0);
	unguard;
}

//==========================================================================
//
//	CheckRedirects
//
//==========================================================================

static VName CheckRedirects(VName Map)
{
	guard(CheckRedirects);
	const mapInfo_t& Info = P_GetMapInfo(Map);
	if (Info.RedirectType == NAME_None || Info.RedirectMap == NAME_None)
	{
		//	No redirect for this map.
		return Map;
	}

	//	Check all players.
	for (int i = 0; i < MAXPLAYERS; i++)
	{
		VBasePlayer* P = GGameInfo->Players[i];
		if (!P || !(P->PlayerFlags & VBasePlayer::PF_Spawned))
		{
			continue;
		}
		if (P->MO->eventCheckInventory(Info.RedirectType) > 0)
		{
			return CheckRedirects(Info.RedirectMap);
		}
	}

	//	None of the players have required item, no redirect.
	return Map;
	unguard;
}

//==========================================================================
//
//	G_DoCompleted
//
//==========================================================================

static void G_DoCompleted()
{
	int			i;

	completed = false;
	if (sv.intermission)
	{
		return;
	}
	if (!netgame && (!GGameInfo->Players[0] ||
		!(GGameInfo->Players[0]->PlayerFlags & VBasePlayer::PF_Spawned)))
	{
		//FIXME Some ACS left from previous visit of the level
		return;
	}
	sv.intermission = 1;
	sv.intertime = 0;
	GLevelInfo->CompletitionTime = GLevel->Time;

	GLevel->Acs->StartTypedACScripts(SCRIPT_Unloading, 0, 0, 0, NULL, false,
		true);

	GLevelInfo->NextMap = CheckRedirects(GLevelInfo->NextMap);

	const mapInfo_t& old_info = P_GetMapInfo(GLevel->MapName);
	const mapInfo_t& new_info = P_GetMapInfo(GLevelInfo->NextMap);
	const VClusterDef* ClusterD = P_GetClusterDef(old_info.Cluster);
	bool HubChange = !old_info.Cluster || !(ClusterD->Flags & CLUSTERF_Hub) ||
		old_info.Cluster != new_info.Cluster;

	for (i = 0; i < MAXPLAYERS; i++)
	{
		if (GGameInfo->Players[i])
		{
			GGameInfo->Players[i]->eventPlayerExitMap(HubChange);
			if (deathmatch || HubChange)
			{
				GGameInfo->Players[i]->eventClientIntermission(
					GLevelInfo->NextMap);
			}
		}
	}

	if (!deathmatch && !HubChange)
	{
		GCmdBuf << "TeleportNewMap\n";
	}
}

//==========================================================================
//
//	COMMAND	TeleportNewMap
//
//==========================================================================

COMMAND(TeleportNewMap)
{
	guard(COMMAND TeleportNewMap);
	if (Source == SRC_Command)
	{
#ifdef CLIENT
		ForwardToServer();
#endif
		return;
	}

	if (!sv.active)
	{
		return;
	}

	if (Args.Num() == 3)
	{
		GLevelInfo->NextMap = VName(*Args[1], VName::AddLower8);
		LeavePosition = atoi(*Args[2]);
	}
	else if (sv.intermission != 1)
	{
		return;
	}

	if (!deathmatch)
	{
		if (VStr(GLevelInfo->NextMap).StartsWith("EndGame"))
		{
			for (int i = 0; i < svs.max_clients; i++)
				if (GGameInfo->Players[i])
					GGameInfo->Players[i]->eventClientFinale(*GLevelInfo->NextMap);
			sv.intermission = 2;
			return;
		}
	}

#ifdef CLIENT
	Draw_TeleportIcon();
#endif
	RebornPosition = LeavePosition;
	GGameInfo->RebornPosition = RebornPosition;
	mapteleport_issued = true;
	unguard;
}

//==========================================================================
//
//	G_DoReborn
//
//==========================================================================

static void G_DoReborn(int playernum)
{
	if (!GGameInfo->Players[playernum] ||
		!(GGameInfo->Players[playernum]->PlayerFlags & VBasePlayer::PF_Spawned))
		return;
	if (!netgame && !deathmatch)// For fun now
	{
		GCmdBuf << "Restart\n";
		GGameInfo->Players[playernum]->PlayerState = PST_LIVE;
	}
	else
	{
		GGameInfo->Players[playernum]->eventNetGameReborn();
	}
}

//==========================================================================
//
//	NET_SendToAll
//
//==========================================================================

int NET_SendToAll(int blocktime)
{
	guard(NET_SendToAll);
	double		start;
	int			i;
	int			count = 0;
	bool		state1[MAXPLAYERS];
	bool		state2[MAXPLAYERS];

	for (i = 0; i < svs.max_clients; i++)
	{
		VBasePlayer* Player = GGameInfo->Players[i];
		if (Player && Player->Net)
		{
			if (Player->Net->IsLocalConnection())
			{
				state1[i] = false;
				state2[i] = true;
				continue;
			}
			count++;
			state1[i] = false;
			state2[i] = false;
		}
		else
		{
			state1[i] = true;
			state2[i] = true;
		}
	}

	start = Sys_Time();
	while (count)
	{
		count = 0;
		for (i = 0; i < svs.max_clients; i++)
		{
			VBasePlayer* Player = GGameInfo->Players[i];
			if (!state1[i])
			{
				state1[i] = true;
				Player->Net->Channels[0]->Close();
				count++;
				continue;
			}

			if (!state2[i])
			{
				if (Player->Net->State == NETCON_Closed)
				{
					state2[i] = true;
				}
				else
				{
					Player->Net->GetMessages();
					Player->Net->Tick();
				}
				count++;
				continue;
			}
		}
		if ((Sys_Time() - start) > blocktime)
			break;
	}
	return count;
	unguard;
}

//==========================================================================
//
//	SV_SendServerInfo
//
//==========================================================================

void SV_SendServerInfo(VBasePlayer* Player)
{
	guard(SV_SendServerInfo);
	if (!Player->Net)
	{
		return;
	}

	//	This will load level on client side.
	((VLevelChannel*)Player->Net->Channels[CHANIDX_Level])->SetLevel(GLevel);
	((VLevelChannel*)Player->Net->Channels[CHANIDX_Level])->SendNewLevel();
	unguard;
}

//==========================================================================
//
//	SV_SendServerInfoToClients
//
//==========================================================================

void SV_SendServerInfoToClients()
{
	guard(SV_SendServerInfoToClients);
	for (int i = 0; i < svs.max_clients; i++)
	{
		VBasePlayer* Player = GGameInfo->Players[i];
		if (!Player)
		{
			continue;
		}
		Player->Level = GLevelInfo;
		SV_SendServerInfo(Player);
	}
	unguard;
}

//==========================================================================
//
//	SV_SpawnServer
//
//==========================================================================

void SV_SpawnServer(const char *mapname, bool spawn_thinkers, bool titlemap)
{
	guard(SV_SpawnServer);
	int			i;

	GCon->Logf(NAME_Dev, "Spawning server %s", mapname);
	paused = false;
	mapteleport_issued = false;
	run_open_scripts = spawn_thinkers;

	if (sv.active)
	{
		//	Level change
		for (i = 0; i < MAXPLAYERS; i++)
		{
			if (!GGameInfo->Players[i])
				continue;

			GGameInfo->Players[i]->KillCount = 0;
			GGameInfo->Players[i]->SecretCount = 0;
			GGameInfo->Players[i]->ItemCount = 0;

			GGameInfo->Players[i]->PlayerFlags &= ~VBasePlayer::PF_Spawned;
			GGameInfo->Players[i]->MO = NULL;
			GGameInfo->Players[i]->Frags = 0;
			GGameInfo->Players[i]->Deaths = 0;
			if (GGameInfo->Players[i]->PlayerState == PST_DEAD)
				GGameInfo->Players[i]->PlayerState = PST_REBORN;
		}
	}
	else
	{
		//	New game
		GGameInfo->WorldInfo = GGameInfo->eventCreateWorldInfo();

		GGameInfo->WorldInfo->SetSkill(Skill);
		GGameInfo->eventInitNewGame(GGameInfo->WorldInfo->GameSkill);

		host_titlemap = titlemap;
		host_standalone = (svs.max_clients == 1 && use_standalone) ||
			host_titlemap;
	}

	SV_Clear();
	VCvar::Unlatch();

	sv.active = true;

	//	Load it
	SV_LoadLevel(VName(mapname, VName::AddLower8));
	GLevel->NetContext = ServerNetContext;
	GLevel->WorldInfo = GGameInfo->WorldInfo;

	const mapInfo_t& info = P_GetMapInfo(GLevel->MapName);

	netgame = svs.max_clients > 1;
	deathmatch = DeathMatch;

	GGameInfo->netgame = netgame;
	GGameInfo->deathmatch = deathmatch;

	P_InitThinkers();

	if (spawn_thinkers)
	{
		//	Create level info.
		GLevelInfo = (VLevelInfo*)GLevel->SpawnThinker(
			GGameInfo->LevelInfoClass);
		GLevelInfo->Level = GLevelInfo;
		GLevelInfo->Game = GGameInfo;
		GLevelInfo->World = GGameInfo->WorldInfo;
		GLevel->LevelInfo = GLevelInfo;
		GLevelInfo->SetMapInfo(info);

		//	Spawn things.
		for (i = 0; i < GLevel->NumThings; i++)
		{
			GLevelInfo->eventSpawnMapThing(&GLevel->Things[i]);
		}
		if (deathmatch && GLevelInfo->DeathmatchStarts.Num() < 4)
		{
			Host_Error("Level needs more deathmatch start spots");
		}
	}

	if (deathmatch)
	{
		TimerGame = TimeLimit * 35 * 60;
	}
	else
	{
		TimerGame = 0;
	}

	// set up world state

	//
	//	P_SpawnSpecials
	//	After the map has been loaded, scan for specials that spawn thinkers
	//
	if (spawn_thinkers)
	{
		GLevelInfo->eventSpawnSpecials();
	}

	VMemberBase::StaticSetUpNetClasses();

	if (!spawn_thinkers)
	{
//		if (level.thinkerHead)
//		{
//			Sys_Error("Spawned a thinker when it's not allowed");
//		}
		return;
	}

	SV_SendServerInfoToClients();

	//	Call BeginPlay events.
	for (TThinkerIterator<VEntity> Ent(GLevel); Ent; ++Ent)
	{
		Ent->eventBeginPlay();
	}
	GLevelInfo->LevelInfoFlags2 |= VLevelInfo::LIF2_BegunPlay;

	if (!host_standalone)
	{
		P_Ticker();
		P_Ticker();

		//	Start open scripts.
		GLevel->Acs->StartTypedACScripts(SCRIPT_Open, 0, 0, 0, NULL, false,
			false);
	}

	GCon->Log(NAME_Dev, "Server spawned");
	unguard;
}

//==========================================================================
//
//	COMMAND PreSpawn
//
//==========================================================================

COMMAND(PreSpawn)
{
	guard(COMMAND PreSpawn);
	if (Source == SRC_Command)
	{
		GCon->Log("PreSpawn is not valid from console");
		return;
	}

	//	Make sure level info is spawned on client side, since there
	// could be some RPCs that depend on it.
	VThinkerChannel* Chan = Player->Net->ThinkerChannels.FindPtr(GLevelInfo);
	if (!Chan)
	{
		Chan = (VThinkerChannel*)Player->Net->CreateChannel(CHANNEL_Thinker,
			-1);
		if (Chan)
		{
			Chan->SetThinker(GLevelInfo);
			Chan->Update();
		}
	}
	unguard;
}

//==========================================================================
//
//	COMMAND Spawn
//
//==========================================================================

COMMAND(Spawn)
{
	guard(COMMAND Spawn);
	if (Source == SRC_Command)
	{
		GCon->Log("Spawn is not valid from console");
		return;
	}

	Player->SpawnClient();
	unguard;
}

//==========================================================================
//
//	SV_DropClient
//
//==========================================================================

void SV_DropClient(VBasePlayer* Player, bool)
{
	guard(SV_DropClient);
	GLevel->Acs->StartTypedACScripts(SCRIPT_Disconnect,
		SV_GetPlayerNum(Player), 0, 0, NULL, true, false);
	if (Player->PlayerFlags & VBasePlayer::PF_Spawned)
	{
		Player->eventDisconnectClient();
	}
	Player->PlayerFlags &= ~VBasePlayer::PF_Active;
	GGameInfo->Players[SV_GetPlayerNum(Player)] = NULL;
	Player->PlayerFlags &= ~VBasePlayer::PF_Spawned;

	Player->PlayerReplicationInfo->DestroyThinker();

	delete Player->Net;
	Player->Net = NULL;

	svs.num_connected--;
	Player->UserInfo = VStr();
	unguard;
}

//==========================================================================
//
//	SV_ShutdownServer
//
//	This only happens at the end of a game, not between levels
//
//==========================================================================

void SV_ShutdownServer(bool crash)
{
	guard(SV_ShutdownServer);
	int			i;
	int			count;

	if (!sv.active)
		return;

	sv.active = false;
	sv_loading = false;

#ifdef CLIENT
	// stop all client sounds immediately
	if (cls.state == ca_connected)
		CL_Disconnect();
#endif

	// make sure all the clients know we're disconnecting
	count = NET_SendToAll(5);
	if (count)
		GCon->Logf("Shutdown server failed for %d clients", count);

	for (i = 0; i < svs.max_clients; i++)
	{
		if (GGameInfo->Players[i])
			SV_DropClient(GGameInfo->Players[i], crash);
	}

	//
	// clear structures
	//
#ifdef CLIENT
	if (GClLevel == GLevel)
	{
		GClLevel = NULL;
	}
#endif
	if (GLevel)
	{
		delete GLevel;
		GLevel = NULL;
	}
	if (GGameInfo->WorldInfo)
	{
		delete GGameInfo->WorldInfo;
		GGameInfo->WorldInfo = NULL;
	}
	for (i = 0; i < MAXPLAYERS; i++)
	{
		//	Save net pointer
		VNetConnection* OldNet = GPlayersBase[i]->Net;
		GPlayersBase[i]->GetClass()->DestructObject(GPlayersBase[i]);
		memset((vuint8*)GPlayersBase[i] + sizeof(VObject), 0,
			GPlayersBase[i]->GetClass()->ClassSize - sizeof(VObject));
		//	Restore pointer
		GPlayersBase[i]->Net = OldNet;
	}
	memset(GGameInfo->Players, 0, sizeof(GGameInfo->Players));
	memset(&sv, 0, sizeof(sv));
	unguard;
}

#ifdef CLIENT

//==========================================================================
//
//	G_DoSingleReborn
//
//	Called by G_Ticker based on gameaction.  Loads a game from the reborn
// save slot.
//
//==========================================================================

COMMAND(Restart)
{
	guard(COMMAND Restart);
	if (netgame || !sv.active)
		return;

	if (SV_RebornSlotAvailable())
	{
		// Use the reborn code if the slot is available
		SV_LoadGame(SV_GetRebornSlot());
	}
	else
	{
		// reload the level from scratch
		SV_SpawnServer(*GLevel->MapName, true, false);
	}
	unguard;
}

#endif

//==========================================================================
//
//	COMMAND Pause
//
//==========================================================================

COMMAND(Pause)
{
	guard(COMMAND Pause);
	if (Source == SRC_Command)
	{
#ifdef CLIENT
		ForwardToServer();
#endif
		return;
	}

	paused ^= 1;
	for (int i = 0; i < svs.max_clients; i++)
		if (GGameInfo->Players[i])
			GGameInfo->Players[i]->eventClientPause(paused);
	unguard;
}

//==========================================================================
//
//  Stats_f
//
//==========================================================================

COMMAND(Stats)
{
	guard(COMMAND Stats);
	if (Source == SRC_Command)
	{
#ifdef CLIENT
		ForwardToServer();
#endif
		return;
	}

	Player->Printf("Kills: %d of %d", Player->KillCount, GLevelInfo->TotalKills);
	Player->Printf("Items: %d of %d", Player->ItemCount, GLevelInfo->TotalItems);
	Player->Printf("Secrets: %d of %d", Player->SecretCount, GLevelInfo->TotalSecret);
	unguard;
}

//==========================================================================
//
//	SV_ConnectClient
//
//	Initialises a client_t for a new net connection.  This will only be
// called once for a player each game, not once for each level change.
//
//==========================================================================

void SV_ConnectClient(VBasePlayer *player)
{
	guard(SV_ConnectClient);
	if (player->Net)
	{
		GCon->Logf(NAME_Dev, "Client %s connected", *player->Net->GetAddress());

		ServerNetContext->ClientConnections.Append(player->Net);
		player->Net->NeedsUpdate = false;
	}

	GGameInfo->Players[SV_GetPlayerNum(player)] = player;
	player->ClientNum = SV_GetPlayerNum(player);
	player->PlayerFlags |= VBasePlayer::PF_Active;

	player->PlayerFlags &= ~VBasePlayer::PF_Spawned;
	player->Level = GLevelInfo;
	if (!sv_loading)
	{
		player->MO = NULL;
		player->PlayerState = PST_REBORN;
		player->eventPutClientIntoServer();
	}
	player->Frags = 0;
	player->Deaths = 0;

	player->PlayerReplicationInfo =
		(VPlayerReplicationInfo*)GLevel->SpawnThinker(GGameInfo->PlayerReplicationInfoClass);
	player->PlayerReplicationInfo->Player = player;
	player->PlayerReplicationInfo->PlayerNum = SV_GetPlayerNum(player);

	SV_SendServerInfo(player);
	unguard;
}

//==========================================================================
//
//	SV_CheckForNewClients
//
//==========================================================================

void SV_CheckForNewClients()
{
	guard(SV_CheckForNewClients);
	VSocketPublic*	sock;
	int				i;
		
	//
	// check for new connections
	//
	while (1)
	{
		sock = GNet->CheckNewConnections();
		if (!sock)
			break;

		//
		// init a new client structure
		//
		for (i = 0; i < svs.max_clients; i++)
			if (!GGameInfo->Players[i])
				break;
		if (i == svs.max_clients)
			Sys_Error("Host_CheckForNewClients: no free clients");

		VBasePlayer* Player = GPlayersBase[i];
		Player->Net = new VNetConnection(sock, ServerNetContext, Player);
		((VPlayerChannel*)Player->Net->Channels[CHANIDX_Player])->SetPlayer(Player);
		SV_ConnectClient(Player);
		svs.num_connected++;
	}
	unguard;
}

//==========================================================================
//
//	SV_ConnectBot
//
//==========================================================================

void SV_ConnectBot(const char *name)
{
	guard(SV_ConnectBot);
	int				i;

	if (!sv.active)
	{
		GCon->Log("Game is not running");
		return;
	}

	if (svs.num_connected >= svs.max_clients)
	{
		GCon->Log("Server is full");
		return;
	}

	//
	// init a new client structure
	//
	for (i = 0; i < svs.max_clients; i++)
		if (!GGameInfo->Players[i])
			break;
	if (i == svs.max_clients)
		Sys_Error("SV_ConnectBot: no free clients");

	VBasePlayer* Player = GPlayersBase[i];
	Player->PlayerFlags |= VBasePlayer::PF_IsBot;
	Player->PlayerName = name;
	SV_ConnectClient(Player);
	svs.num_connected++;
	SV_SetUserInfo(Player, Player->UserInfo);
	Player->SpawnClient();
	unguard;
}

//==========================================================================
//
//	COMMAND AddBot
//
//==========================================================================

COMMAND(AddBot)
{
	SV_ConnectBot(Args.Num() > 1 ? *Args[1] : "");
}

//==========================================================================
//
//  Map
//
//==========================================================================

COMMAND(Map)
{
	guard(COMMAND Map);
	VStr	mapname;

	if (Args.Num() != 2)
	{
		GCon->Log("map <mapname> : change level");
		return;
	}
	mapname = Args[1];

	SV_ShutdownServer(false);
#ifdef CLIENT
	CL_Disconnect();
#endif

	SV_InitBaseSlot();
	SV_ClearRebornSlot();
	// Default the player start spot group to 0
	RebornPosition = 0;
	GGameInfo->RebornPosition = RebornPosition;

	if ((int)Skill < 0)
	{
		Skill = 0;
	}
	else if ((int)Skill >= P_GetNumSkills())
	{
		Skill = P_GetNumSkills() - 1;
	}

	// Set up a bunch of globals
	GGameInfo->netgame = svs.max_clients > 1;

	SV_SpawnServer(*mapname, true, false);
#ifdef CLIENT
	if (cls.state != ca_dedicated)
		GCmdBuf << "Connect local\n";
#endif
	unguard;
}

//==========================================================================
//
//	Host_StartTitleMap
//
//==========================================================================

bool Host_StartTitleMap()
{
	guard(Host_StartTitleMap);
	if (!FL_FileExists("maps/titlemap.wad") &&
		W_CheckNumForName(NAME_titlemap) < 0)
	{
		return false;
	}

	// Default the player start spot group to 0
	RebornPosition = 0;
	GGameInfo->RebornPosition = RebornPosition;
	GGameInfo->netgame = false;

	SV_SpawnServer("titlemap", true, true);
#ifdef CLIENT
	if (cls.state != ca_dedicated)
		GCmdBuf << "Connect local\n";
#endif
	return true;
	unguard;
}

//==========================================================================
//
//	COMMAND MaxPlayers
//
//==========================================================================

COMMAND(MaxPlayers)
{
	guard(COMMAND MaxPlayers);
	int 	n;

	if (Args.Num() != 2)
	{
		GCon->Logf("maxplayers is %d", svs.max_clients);
		return;
	}

	if (sv.active)
	{
		GCon->Log("maxplayers can not be changed while a server is running.");
		return;
	}

	n = atoi(*Args[1]);
	if (n < 1)
		n = 1;
	if (n > MAXPLAYERS)
	{
		n = MAXPLAYERS;
		GCon->Logf("maxplayers set to %d", n);
	}
	svs.max_clients = n;

	if (n == 1)
	{
#ifdef CLIENT
		GCmdBuf << "listen 0\n";
#endif
		DeathMatch = 0;
		NoMonsters = 0;
	}
	else
	{
#ifdef CLIENT
		GCmdBuf << "listen 1\n";
#endif
		DeathMatch = 2;
		NoMonsters = 1;
	}
	unguard;
}

//==========================================================================
//
//	ServerFrame
//
//==========================================================================

void ServerFrame(int realtics)
{
	guard(ServerFrame);
	SV_CheckForNewClients();

	if (real_time)
	{
		SV_Ticker();
	}
	else
	{
		// run the count dics
		while (realtics--)
		{
			SV_Ticker();
		}
	}

	if (mapteleport_issued)
	{
		SV_MapTeleport(GLevelInfo->NextMap);
	}

	SV_SendClientMessages();
	unguard;
}

//==========================================================================
//
//	SV_FindClassFromEditorId
//
//==========================================================================

VClass* SV_FindClassFromEditorId(int Id, int GameFilter)
{
	guard(SV_FindClassFromEditorId);
	for (int i = VClass::GMobjInfos.Num() - 1; i >= 0; i--)
	{
		if ((!VClass::GMobjInfos[i].GameFilter ||
			(VClass::GMobjInfos[i].GameFilter & GameFilter)) &&
			Id == VClass::GMobjInfos[i].DoomEdNum)
		{
			return VClass::GMobjInfos[i].Class;
		}
	}
	return NULL;
	unguard;
}

//==========================================================================
//
//	SV_FindClassFromScriptId
//
//==========================================================================

VClass* SV_FindClassFromScriptId(int Id, int GameFilter)
{
	guard(SV_FindClassFromScriptId);
	for (int i = VClass::GScriptIds.Num() - 1; i >= 0; i--)
	{
		if ((!VClass::GScriptIds[i].GameFilter ||
			(VClass::GScriptIds[i].GameFilter & GameFilter)) &&
			Id == VClass::GScriptIds[i].DoomEdNum)
		{
			return VClass::GScriptIds[i].Class;
		}
	}
	return NULL;
	unguard;
}

//==========================================================================
//
//	COMMAND Say
//
//==========================================================================

COMMAND(Say)
{
	guard(COMMAND Say);
	if (Source == SRC_Command)
	{
#ifdef CLIENT
		ForwardToServer();
#endif
		return;
	}
	if (Args.Num() < 2)
		return;

	VStr Text = Player->PlayerName;
	Text += ":";
	for (int i = 1; i < Args.Num(); i++)
	{
		Text += " ";
		Text += Args[i];
	}
	GLevelInfo->BroadcastPrint(*Text);
	GLevelInfo->StartSound(TVec(0, 0, 0), 0,
		GSoundManager->GetSoundID("misc/chat"), 0, 1.0, 0, false);
	unguard;
}

//**************************************************************************
//
//	Dedicated server console streams
//
//**************************************************************************

#ifndef CLIENT

class FConsoleDevice : public FOutputDevice
{
public:
	void Serialise(const char* V, EName)
	{
		printf("%s\n", V);
	}
};

FConsoleDevice			Console;

FOutputDevice			*GCon = &Console;

#endif
