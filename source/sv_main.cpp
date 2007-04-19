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

#define REBORN_DESCRIPTION		"TEMP GAME"

#define MAX_MODELS				512
#define MAX_SPRITES				512
#define MAX_SKINS				256

// TYPES -------------------------------------------------------------------

// EXTERNAL FUNCTION PROTOTYPES --------------------------------------------

void Draw_TeleportIcon();
void CL_Disconnect();
void SV_ReadClientMessages(int i);
void SV_RunClientCommand(const VStr& cmd);
void EntInit();

// PUBLIC FUNCTION PROTOTYPES ----------------------------------------------

void SV_DropClient(bool crash);
void SV_ShutdownServer(bool);

// PRIVATE FUNCTION PROTOTYPES ---------------------------------------------

static void G_DoReborn(int playernum);
static void G_DoCompleted();

// EXTERNAL DATA DECLARATIONS ----------------------------------------------

// PUBLIC DATA DEFINITIONS -------------------------------------------------

IMPLEMENT_CLASS(V, LevelInfo)
IMPLEMENT_CLASS(V, GameInfo)
IMPLEMENT_CLASS(V, BasePlayer)

VCvarI			real_time("real_time", "1");

server_t		sv;
server_static_t	svs;

// increment every time a check is made
int				validcount = 1;

bool			sv_loading = false;
int				sv_load_num_players;

VBasePlayer*	GPlayersBase[MAXPLAYERS];

skill_t         gameskill; 

bool			paused;

bool			deathmatch = false;   	// only if started as net death
bool			netgame;                // only true if packets are broadcast

VMessageOut*	sv_reliable;
VMessageOut*	sv_datagram;
VMessageOut*	sv_signons;

VBasePlayer*	sv_player;

int 			TimerGame;

VGameInfo*		GGameInfo;
VWorldInfo*		GWorldInfo;
VLevelInfo*		GLevelInfo;

TArray<VSndSeqInfo>	sv_ActiveSequences;

// PRIVATE DATA DEFINITIONS ------------------------------------------------

static int 		LeavePosition;

static int		RebornPosition;	// Position indicator for cooperative net-play reborn

static bool		completed;

static VCvarI	TimeLimit("TimeLimit", "0");
static VCvarI	DeathMatch("DeathMatch", "0", CVAR_ServerInfo);
static VCvarI  	NoMonsters("NoMonsters", "0");
static VCvarI	Skill("Skill", "2");

static VCvarI	sv_cheats("sv_cheats", "0", CVAR_ServerInfo | CVAR_Latch);

static vuint8	*fatpvs;
static VCvarI	show_mobj_overflow("show_mobj_overflow", "0", CVAR_Archive);
static VCvarI	show_update_stats("show_update_stats", "0", 0);

static bool		mapteleport_issued;

static int		nummodels;
static VName	models[MAX_MODELS];
static int		numskins;
static VStr		skins[MAX_SKINS];

static VCvarI	split_frame("split_frame", "1", CVAR_Archive);
static VCvarF	sv_gravity("sv_gravity", "800.0", CVAR_ServerInfo);

static VServerNetContext*	ServerNetContext;

// CODE --------------------------------------------------------------------

//==========================================================================
//
//	VLevelInfo::VLevelInfo
//
//==========================================================================

VLevelInfo::VLevelInfo()
{
	Level = this;
	Game = GGameInfo;
	World = GWorldInfo;
}

//==========================================================================
//
//	VLevelInfo::SetMapInfo
//
//==========================================================================

void VLevelInfo::SetMapInfo(const mapInfo_t& Info)
{
	guard(VLevelInfo::SetMapInfo);
	LevelName = Info.Name;
	LevelNum = Info.LevelNum;
	Cluster = Info.Cluster;

	NextMap = Info.NextMap;
	SecretMap = Info.SecretMap;

	ParTime = Info.ParTime;
	SuckTime = Info.SuckTime;

	Sky1Texture = Info.Sky1Texture;
	Sky2Texture = Info.Sky2Texture;
	Sky1ScrollDelta = Info.Sky1ScrollDelta;
	Sky2ScrollDelta = Info.Sky2ScrollDelta;
	SkyBox = Info.SkyBox;

	FadeTable = Info.FadeTable;

	SongLump = Info.SongLump;
	CDTrack = Info.CDTrack;

	//	Copy flags from mapinfo.
	if (Info.Flags & MAPINFOF_DoubleSky)
		LevelInfoFlags |= LIF_DoubleSky;
	if (Info.Flags & MAPINFOF_Lightning)
		LevelInfoFlags |= LIF_Lightning;
	if (Info.Flags & MAPINFOF_Map07Special)
		LevelInfoFlags |= LIF_Map07Special;
	if (Info.Flags & MAPINFOF_BaronSpecial)
		LevelInfoFlags |= LIF_BaronSpecial;
	if (Info.Flags & MAPINFOF_CyberDemonSpecial)
		LevelInfoFlags |= LIF_CyberDemonSpecial;
	if (Info.Flags & MAPINFOF_SpiderMastermindSpecial)
		LevelInfoFlags |= LIF_SpiderMastermindSpecial;
	if (Info.Flags & MAPINFOF_MinotaurSpecial)
		LevelInfoFlags |= LIF_MinotaurSpecial;
	if (Info.Flags & MAPINFOF_DSparilSpecial)
		LevelInfoFlags |= LIF_DSparilSpecial;
	if (Info.Flags & MAPINFOF_IronLichSpecial)
		LevelInfoFlags |= LIF_IronLichSpecial;
	if (Info.Flags & MAPINFOF_SpecialActionOpenDoor)
		LevelInfoFlags |= LIF_SpecialActionOpenDoor;
	if (Info.Flags & MAPINFOF_SpecialActionLowerFloor)
		LevelInfoFlags |= LIF_SpecialActionLowerFloor;
	if (Info.Flags & MAPINFOF_SpecialActionKillMonsters)
		LevelInfoFlags |= LIF_SpecialActionKillMonsters;
	if (Info.Flags & MAPINFOF_NoIntermission)
		LevelInfoFlags |= LIF_NoIntermission;
	if (Info.Flags & MAPINFOF_NoSoundClipping)
		LevelInfoFlags |= LIF_NoSoundClipping;
	if (Info.Flags & MAPINFOF_AllowMonsterTelefrags)
		LevelInfoFlags |= LIF_AllowMonsterTelefrags;
	if (Info.Flags & MAPINFOF_NoAllies)
		LevelInfoFlags |= LIF_NoAllies;
	if (Info.Flags & MAPINFOF_DeathSlideShow)
		LevelInfoFlags |= LIF_DeathSlideShow;
	if (Info.Flags & MAPINFOF_ForceNoSkyStretch)
		LevelInfoFlags |= LIF_ForceNoSkyStretch;
	if (Info.Flags & MAPINFOF_LookupName)
		LevelInfoFlags |= LIF_LookupName;
	unguard;
}

//==========================================================================
//
//	SV_Init
//
//==========================================================================

void SV_Init()
{
	guard(SV_Init);
	int		i;

	ServerNetContext = new VServerNetContext();
	sv_reliable = new VMessageOut(OUT_MESSAGE_SIZE);
	sv_datagram = new VMessageOut(OUT_MESSAGE_SIZE);
	sv_signons = new VMessageOut(OUT_MESSAGE_SIZE);
	sv_signons->Next = NULL;

	svs.max_clients = 1;

	VMemberBase::StaticLoadPackage(NAME_svprogs);

	ProcessDehackedFiles();

	GGameInfo = (VGameInfo*)VObject::StaticSpawnObject(
		VClass::FindClass("MainGameInfo"));
	GGameInfo->eventInit();

	VClass* PlayerClass = VClass::FindClass("Player");
	for (i = 0; i < MAXPLAYERS; i++)
	{
		GPlayersBase[i] = (VBasePlayer*)VObject::StaticSpawnObject(
			PlayerClass);
	}

	GGameInfo->validcount = &validcount;
	GGameInfo->skyflatnum = skyflatnum;
	EntInit();

	P_InitSwitchList();
	P_InitTerrainTypes();
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
	svs.serverinfo.Clean();
	for (int i = 0; i < MAX_SKINS; i++)
	{
		skins[i].Clean();
	}

	delete sv_reliable;
	delete sv_datagram;
	for (VMessageOut* Msg = sv_signons; Msg; )
	{
		VMessageOut* Next = Msg->Next;
		delete Msg;
		Msg = Next;
	}
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
	for (VMessageOut* Msg = sv_signons; Msg; )
	{
		VMessageOut* Next = Msg->Next;
		delete Msg;
		Msg = Next;
	}
	sv_signons = new VMessageOut(OUT_MESSAGE_SIZE);
	sv_signons->Next = NULL;
	sv_reliable->Clear();
	sv_datagram->Clear();
	sv_ActiveSequences.Clear();
#ifdef CLIENT
	// Make sure all sounds are stopped.
	GAudio->StopAllSound();
#endif
	unguard;
}

//==========================================================================
//
//	SV_ClearDatagram
//
//==========================================================================

void SV_ClearDatagram()
{
	guard(SV_ClearDatagram);
	sv_datagram->Clear();
	unguard;
}

//==========================================================================
//
//	SV_AddEntity
//
//==========================================================================

void SV_AddEntity(VEntity* Ent)
{
	int Id = 0;
	bool Used = false;
	do
	{
		Id++;
		Used = false;
		for (TThinkerIterator<VEntity> Other(GLevel); Other; ++Other)
		{
			if (Other->NetID == Id)
			{
				Used = true;
				break;
			}
		}
	}
	while (Used);
	Ent->NetID = Id;
}

//==========================================================================
//
//	VLevel::SpawnThinker
//
//==========================================================================

VThinker* VLevel::SpawnThinker(VClass* Class, const TVec& AOrigin,
	const TAVec& AAngles, mthing_t* mthing)
{
	guard(VLevel::SpawnThinker);
	VThinker* Ret = (VThinker*)StaticSpawnObject(Class);
	AddThinker(Ret);

	if (this == GLevel && Class->IsChildOf(VEntity::StaticClass()))
	{
		SV_AddEntity((VEntity*)Ret);
		if (GLevelInfo->LevelInfoFlags & VLevelInfo::LIF_BegunPlay)
		{
			((VEntity*)Ret)->eventBeginPlay();
		}
		((VEntity*)Ret)->Origin = AOrigin;
		((VEntity*)Ret)->Angles = AAngles;
		((VEntity*)Ret)->eventOnMapSpawn(mthing);
	}

	return Ret;
	unguard;
}

int c_bigClass;
int c_bigState;
//==========================================================================
//
//	VEntity::Destroy
//
//==========================================================================

void VEntity::Destroy()
{
	guard(VEntity::Destroy);
	if (XLevel == GLevel && GLevel)
	{
		eventDestroyed();

		// unlink from sector and block lists
		UnlinkFromWorld();

		// stop any playing sound
		SV_StopSound(this, 0);
	}

	Super::Destroy();
	unguard;
}

//==========================================================================
//
//	GetOriginNum
//
//==========================================================================

int GetOriginNum(const VEntity *mobj)
{
	if (!mobj)
	{
		return 0;
	}
	return mobj->NetID;
}

//==========================================================================
//
//	SV_StartSound
//
//==========================================================================

void SV_StartSound(const TVec &origin, int origin_id, int sound_id,
	int channel, float volume, float Attenuation)
{
	guard(SV_StartSound);
	for (int i = 0; i < MAXPLAYERS; i++)
	{
		if (!GGameInfo->Players[i])
			continue;
		if (!(GGameInfo->Players[i]->PlayerFlags & VBasePlayer::PF_Spawned))
			continue;
		GGameInfo->Players[i]->eventClientStartSound(sound_id, origin,
			origin_id, channel, volume, Attenuation);
	}
	unguard;
}

//==========================================================================
//
//	SV_StopSound
//
//==========================================================================

void SV_StopSound(int origin_id, int channel)
{
	guard(SV_StopSound);
	if (!sv_datagram->CheckSpaceBits(3 << 3))
		return;

	for (int i = 0; i < MAXPLAYERS; i++)
	{
		if (!GGameInfo->Players[i])
			continue;
		if (!(GGameInfo->Players[i]->PlayerFlags & VBasePlayer::PF_Spawned))
			continue;
		GGameInfo->Players[i]->eventClientStopSound(origin_id, channel);
	}
	unguard;
}

//==========================================================================
//
//	SV_StartSound
//
//==========================================================================

void SV_StartSound(const VEntity* Ent, int SoundId, int Channel,
	float Volume, float Attenuation)
{
	guard(SV_StartSound);
	if (Ent)
	{
		SV_StartSound(Ent->Origin, GetOriginNum(Ent), SoundId,
			Channel, Volume, Attenuation);
	}
	else
	{
		SV_StartSound(TVec(0, 0, 0), 0, SoundId, Channel, Volume,
			Attenuation);
	}
	unguard;
}

//==========================================================================
//
//	SV_StartLocalSound
//
//==========================================================================

void SV_StartLocalSound(const VEntity* Ent, int SoundId, int Channel,
	float Volume, float Attenuation)
{
	guard(SV_StartLocalSound);
	if (Ent && Ent->Player)
	{
		Ent->Player->eventClientStartSound(SoundId, TVec(0, 0, 0), 0, Channel,
			Volume, Attenuation);
	}
	unguard;
}

//==========================================================================
//
//	SV_StopSound
//
//==========================================================================

void SV_StopSound(const VEntity *origin, int channel)
{
	guard(SV_StopSound);
	SV_StopSound(GetOriginNum(origin), channel);
	unguard;
}

//==========================================================================
//
//	SV_SectorStartSound
//
//==========================================================================

void SV_SectorStartSound(const sector_t* Sector, int SoundId, int Channel,
	float Volume, float Attenuation)
{
	guard(SV_SectorStartSound);
	if (Sector)
	{
		SV_StartSound(Sector->soundorg,
			(Sector - GLevel->Sectors) + GMaxEntities,
			SoundId, Channel, Volume, Attenuation);
	}
	else
	{
		SV_StartSound(TVec(0, 0, 0), 0, SoundId, Channel, Volume,
			Attenuation);
	}
	unguard;
}

//==========================================================================
//
//	SV_SectorStopSound
//
//==========================================================================

void SV_SectorStopSound(const sector_t *sector, int channel)
{
	guard(SV_SectorStopSound);
	SV_StopSound((sector - GLevel->Sectors) + GMaxEntities, channel);
	unguard;
}

//==========================================================================
//
//	SV_StartSequence
//
//==========================================================================

void SV_StartSequence(const TVec& Origin, vint32 OriginId, VName Name,
	vint32 ModeNum)
{
	guard(SV_StartSequence);
	//	Remove any existing sequences of this origin
	for (int i = 0; i < sv_ActiveSequences.Num(); i++)
	{
		if (sv_ActiveSequences[i].OriginId == OriginId)
		{
			sv_ActiveSequences.RemoveIndex(i);
			i--;
		}
	}
	VSndSeqInfo& Seq = sv_ActiveSequences.Alloc();
	Seq.Name = Name;
	Seq.OriginId = OriginId;
	Seq.Origin = Origin;
	Seq.ModeNum = ModeNum;

	for (int i = 0; i < MAXPLAYERS; i++)
	{
		if (!GGameInfo->Players[i])
			continue;
		if (!(GGameInfo->Players[i]->PlayerFlags & VBasePlayer::PF_Spawned))
			continue;
		GGameInfo->Players[i]->eventClientStartSequence(Origin, OriginId,
			Name, ModeNum);
	}
	unguard;
}

//==========================================================================
//
//	SV_AddSequenceChoice
//
//==========================================================================

void SV_AddSequenceChoice(int origin_id, VName Choice)
{
	guard(SV_AddSequenceChoice);
	//	Remove it from server's sequences list.
	for (int i = 0; i < sv_ActiveSequences.Num(); i++)
	{
		if (sv_ActiveSequences[i].OriginId == origin_id)
		{
			sv_ActiveSequences[i].Choices.Append(Choice);
		}
	}

	for (int i = 0; i < MAXPLAYERS; i++)
	{
		if (!GGameInfo->Players[i])
			continue;
		if (!(GGameInfo->Players[i]->PlayerFlags & VBasePlayer::PF_Spawned))
			continue;
		GGameInfo->Players[i]->eventClientAddSequenceChoice(origin_id,
			Choice);
	}
	unguard;
}

//==========================================================================
//
//	SV_StopSequence
//
//==========================================================================

void SV_StopSequence(int origin_id)
{
	guard(SV_StopSequence);
	//	Remove it from server's sequences list.
	for (int i = 0; i < sv_ActiveSequences.Num(); i++)
	{
		if (sv_ActiveSequences[i].OriginId == origin_id)
		{
			sv_ActiveSequences.RemoveIndex(i);
			i--;
		}
	}

	for (int i = 0; i < MAXPLAYERS; i++)
	{
		if (!GGameInfo->Players[i])
			continue;
		if (!(GGameInfo->Players[i]->PlayerFlags & VBasePlayer::PF_Spawned))
			continue;
		GGameInfo->Players[i]->eventClientStopSequence(origin_id);
	}
	unguard;
}

//==========================================================================
//
//	SV_SectorStartSequence
//
//==========================================================================

void SV_SectorStartSequence(const sector_t* Sector, VName Name,
	int ModeNum)
{
	guard(SV_SectorStartSequence);
	if (Sector)
	{
		SV_StartSequence(Sector->soundorg, (Sector - GLevel->Sectors) +
			GMaxEntities, Name, ModeNum);
	}
	else
	{
		SV_StartSequence(TVec(0, 0, 0), 0, Name, ModeNum);
	}
	unguard;
}

//==========================================================================
//
//	SV_SectorStopSequence
//
//==========================================================================

void SV_SectorStopSequence(const sector_t *sector)
{
	guard(SV_SectorStopSequence);
	SV_StopSequence((sector - GLevel->Sectors) + GMaxEntities);
	unguard;
}

//==========================================================================
//
//	SV_PolyobjStartSequence
//
//==========================================================================

void SV_PolyobjStartSequence(const polyobj_t* Poly, VName Name,
	int ModeNum)
{
	guard(SV_PolyobjStartSequence);
	SV_StartSequence(Poly->startSpot, (Poly - GLevel->PolyObjs) +
		GMaxEntities + GLevel->NumSectors, Name, ModeNum);
	unguard;
}

//==========================================================================
//
//	SV_PolyobjStopSequence
//
//==========================================================================

void SV_PolyobjStopSequence(const polyobj_t *poly)
{
	guard(SV_PolyobjStopSequence);
	SV_StopSequence((poly - GLevel->PolyObjs) + GMaxEntities + GLevel->NumSectors);
	unguard;
}

//==========================================================================
//
//	SV_EntityStartSequence
//
//==========================================================================

void SV_EntityStartSequence(const VEntity* Ent, VName Name, int ModeNum)
{
	guard(SV_EntityStartSequence);
	SV_StartSequence(Ent->Origin, GetOriginNum(Ent), Name, ModeNum);
	unguard;
}

//==========================================================================
//
//	SV_EntityAddSequenceChoice
//
//==========================================================================

void SV_EntityAddSequenceChoice(const VEntity* Ent, VName Choice)
{
	guard(SV_EntityAddSequenceChoice);
	SV_AddSequenceChoice(GetOriginNum(Ent), Choice);
	unguard;
}

//==========================================================================
//
//	SV_EntityStopSequence
//
//==========================================================================

void SV_EntityStopSequence(const VEntity* Ent)
{
	guard(SV_EntityStopSequence);
	SV_StopSequence(GetOriginNum(Ent));
	unguard;
}

//==========================================================================
//
//	SV_ClientPrintf
//
//==========================================================================

void SV_ClientPrintf(VBasePlayer *player, const char *s, ...)
{
	guard(SV_ClientPrintf);
	va_list	v;
	char	buf[1024];

	va_start(v, s);
	vsprintf(buf, s, v);
	va_end(v);

	player->Net->Message << (vuint8)svc_print << buf;
	unguard;
}

//==========================================================================
//
//	SV_ClientCentrePrintf
//
//==========================================================================

void SV_ClientCentrePrintf(VBasePlayer *player, const char *s, ...)
{
	guard(SV_ClientCentrePrintf);
	va_list	v;
	char	buf[1024];

	va_start(v, s);
	vsprintf(buf, s, v);
	va_end(v);

	player->Net->Message << (vuint8)svc_centre_print << buf;
	unguard;
}

//==========================================================================
//
//	SV_BroadcastPrintf
//
//==========================================================================

void SV_BroadcastPrintf(const char *s, ...)
{
	guard(SV_BroadcastPrintf);
	va_list	v;
	char	buf[1024];

	va_start(v, s);
	vsprintf(buf, s, v);
	va_end(v);

	for (int i = 0; i < svs.max_clients; i++)
		if (GGameInfo->Players[i])
			GGameInfo->Players[i]->Net->Message << (vuint8)svc_print << buf;
	unguard;
}

//==========================================================================
//
//	SV_BroadcastCentrePrintf
//
//==========================================================================

void SV_BroadcastCentrePrintf(const char *s, ...)
{
	guard(SV_BroadcastCentrePrintf);
	va_list	v;
	char	buf[1024];

	va_start(v, s);
	vsprintf(buf, s, v);
	va_end(v);

	for (int i = 0; i < svs.max_clients; i++)
		if (GGameInfo->Players[i])
			GGameInfo->Players[i]->Net->Message << (vuint8)svc_centre_print << buf;
	unguard;
}

//==========================================================================
//
//	SV_WriteViewData
//
//==========================================================================

void SV_WriteViewData(VBasePlayer &player, VMessageOut& msg)
{
	guard(SV_WriteViewData);
	int		i;

	msg << (vuint8)svc_view_data
		<< player.ViewOrg.x
		<< player.ViewOrg.y
		<< player.ViewOrg.z
		<< (vuint8)(player.MO->Alpha * 255);
	for (i = 0; i < NUMPSPRITES; i++)
	{
		if (player.ViewStates[i].State)
		{
			int TimeFrac = 0;
			if (player.ViewStates[i].StateTime < 0)
				TimeFrac = 255;
			else if (player.ViewStates[i].State->Time > 0)
			{
				TimeFrac = (int)(254.0 * player.ViewStates[i].StateTime /
					player.ViewStates[i].State->Time);
				TimeFrac = MID(0, TimeFrac, 254);
			}
			msg << (vuint16)((VClass*)player.ViewStates[i].State->Outer)->NetId
				<< (vuint16)player.ViewStates[i].State->NetId
				<< (vuint8)TimeFrac
				<< (vuint16)player.ViewStates[i].SX
				<< (vuint16)player.ViewStates[i].SY;
		}
		else
		{
			msg << (vint16)-1;
		}
	}

	//	Update bam_angles (after teleportation)
	if (player.PlayerFlags & VBasePlayer::PF_FixAngle)
	{
		player.PlayerFlags &= ~VBasePlayer::PF_FixAngle;
		msg << (vuint8)svc_set_angles
			<< (vuint8)(AngleToByte(player.ViewAngles.pitch))
			<< (vuint8)(AngleToByte(player.ViewAngles.yaw))
			<< (vuint8)(AngleToByte(player.ViewAngles.roll));
	}
	unguard;
}

//==========================================================================
//
//	SV_CheckFatPVS
//
//==========================================================================

int SV_CheckFatPVS(subsector_t *subsector)
{
	int ss = subsector - GLevel->Subsectors;
	return fatpvs[ss / 8] & (1 << (ss & 7));
}

//==========================================================================
//
//	SV_SecCheckFatPVS
//
//==========================================================================

bool SV_SecCheckFatPVS(sector_t *sec)
{
	for (subsector_t *sub = sec->subsectors; sub; sub = sub->seclink)
	{
		if (SV_CheckFatPVS(sub))
		{
			return true;
		}
	}
	return false;
}

//==========================================================================
//
//	IsRelevant
//
//==========================================================================

static bool IsRelevant(VThinker* Th)
{
	guardSlow(IsRelevant);
	if (Th->ThinkerFlags & VThinker::TF_AlwaysRelevant)
		return true;
	VEntity* Ent = Cast<VEntity>(Th);
	if (!Ent)
		return false;
	if (Ent->EntityFlags & VEntity::EF_Hidden)
		return false;
	if (!SV_CheckFatPVS(Ent->SubSector))
		return false;
	return true;
	unguardSlow;
}

//==========================================================================
//
//	SV_UpdateLevel
//
//==========================================================================

void SV_UpdateLevel(VMessageOut& msg)
{
	guard(SV_UpdateLevel);
	int		i;

	fatpvs = GLevel->LeafPVS(sv_player->MO->SubSector);

	((VLevelChannel*)sv_player->Net->Channels[CHANIDX_Level])->Update();

int StartSize = msg.GetNumBytes();
int NumObjs = 0;
	//	Mark all entity channels as not updated in this frame.
	for (i = sv_player->Net->OpenChannels.Num() - 1; i >= 0; i--)
	{
		VChannel* Chan = sv_player->Net->OpenChannels[i];
		if (Chan->Type == CHANNEL_Thinker)
		{
			((VThinkerChannel*)Chan)->UpdatedThisFrame = false;
		}
	}

	//	Update mobjs in sight
	for (TThinkerIterator<VThinker> Th(GLevel); Th; ++Th)
	{
		if (!IsRelevant(*Th))
			continue;
		VThinkerChannel* Chan = sv_player->Net->ThinkerChannels.FindPtr(*Th);
		if (!Chan)
		{
			Chan = new VThinkerChannel(sv_player->Net, -1);
			Chan->SetThinker(*Th);
		}
		Chan->Update();
		NumObjs++;
	}

	//	Close entity channels that were not updated in this frame.
	for (i = sv_player->Net->OpenChannels.Num() - 1; i >= 0; i--)
	{
		VChannel* Chan = sv_player->Net->OpenChannels[i];
		if (Chan->Type == CHANNEL_Thinker &&
			!((VThinkerChannel*)Chan)->UpdatedThisFrame)
		{
			Chan->Close();
		}
	}

if (show_update_stats)
dprintf("Update size %d (%d) for %d, aver %f big %d %d\n", msg.GetNumBytes(), msg.GetNumBytes() -
		StartSize, NumObjs, float(msg.GetNumBytes() - StartSize) / NumObjs, c_bigClass, c_bigState);
	unguard;
}

//==========================================================================
//
//	SV_SendNop
//
//	Send a nop message without trashing or sending the accumulated client
// message buffer
//
//==========================================================================

void SV_SendNop(VBasePlayer *client)
{
	guard(SV_SendNop);
	VMessageOut	msg(4 << 3);

	msg << (vuint8)svc_nop;

	client->Net->Channels[0]->SendMessage(&msg);
	client->Net->LastMessage = realtime;
	unguard;
}

//==========================================================================
//
//	SV_SendClientDatagram
//
//==========================================================================

void SV_SendClientDatagram()
{
	guard(SV_SendClientDatagram);
	for (int i = 0; i < svs.max_clients; i++)
	{
		if (!GGameInfo->Players[i])
		{
			continue;
		}

		sv_player = GGameInfo->Players[i];

		if (!(sv_player->PlayerFlags & VBasePlayer::PF_Spawned))
		{
			// the player isn't totally in the game yet
			// send small keepalive messages if too much time has passed
			// send a full message when the next signon stage has been requested
			// some other message data (name changes, etc) may accumulate
			// between signon stages
			if (realtime - sv_player->Net->LastMessage > 5)
			{
				SV_SendNop(sv_player);
			}
			continue;
		}

		if (!sv_player->Net->NeedsUpdate)
			continue;

		sv_player->MO->EntityFlags |= VEntity::EF_NetLocalPlayer;

		VMessageOut msg(OUT_MESSAGE_SIZE);

		msg << (vuint8)svc_time
			<< GLevel->Time;

		SV_WriteViewData(*sv_player, msg);

		if (msg.CheckSpaceBits(sv_datagram->GetNumBytes() << 3))
			msg << *sv_datagram;

		SV_UpdateLevel(msg);

		sv_player->Net->Channels[0]->SendMessage(&msg);

		sv_player->MO->EntityFlags &= ~VEntity::EF_NetLocalPlayer;
	}
	unguard;
}

//==========================================================================
//
//	SV_SendReliable
//
//==========================================================================

void SV_SendReliable()
{
	guard(SV_SendReliable);
	for (int i = 0; i < svs.max_clients; i++)
	{
		if (!GGameInfo->Players[i])
			continue;

		GGameInfo->Players[i]->Net->Message << *sv_reliable;

		if (!(GGameInfo->Players[i]->PlayerFlags & VBasePlayer::PF_Spawned))
			continue;

		sv_player = GGameInfo->Players[i];
		((VPlayerChannel*)sv_player->Net->Channels[CHANIDX_Player])->Update();
	}

	sv_reliable->Clear();

	for (int i = 0; i < svs.max_clients; i++)
	{
		VBasePlayer* Player = GGameInfo->Players[i];
		sv_player = Player;
		if (!Player)
		{
			continue;
		}

		if (Player->Net->Message.IsError())
		{
			SV_DropClient(true);
			GCon->Log(NAME_Dev, "Client message overflowed");
			continue;
		}

		if (Player->Net->Message.GetNumBytes())
		{
			Player->Net->Message.bReliable = true;
			Player->Net->Channels[0]->SendMessage(&Player->Net->Message);
			Player->Net->Message.Clear();
			Player->Net->LastMessage = realtime;
		}
	}

	for (int i = 0; i < svs.max_clients; i++)
	{
		sv_player = GGameInfo->Players[i];
		if (!sv_player)
		{
			continue;
		}
		sv_player->Net->Tick();
		if (sv_player->Net->State == NETCON_Closed)
		{
			SV_DropClient(true);
			continue;
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
	SV_SendClientDatagram();

	SV_SendReliable();
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
		*sv_reliable << (vuint8)svc_skip_intermission;
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
		if (!GGameInfo->Players[i])
		{
			continue;
		}

		// do player reborns if needed
		if (GGameInfo->Players[i]->PlayerState == PST_REBORN)
		{
			G_DoReborn(i);
		}

		SV_ReadClientMessages(i);

		// pause if in menu or console and at least one tic has been run
#ifdef CLIENT
		if (GGameInfo->Players[i]->PlayerFlags & VBasePlayer::PF_Spawned &&
			!sv.intermission && !paused &&
			(netgame || !(MN_Active() || C_Active())))
#else
		if (GGameInfo->Players[i]->PlayerFlags & VBasePlayer::PF_Spawned &&
			!sv.intermission && !paused)
#endif
		{
			GGameInfo->Players[i]->eventPlayerTick(host_frametime);
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
			if (!paused && (netgame || !(MN_Active() || C_Active())))
#else
			if (!paused)
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
//	SV_ForceLightning
//
//==========================================================================

void SV_ForceLightning()
{
	*sv_datagram << (vuint8)svc_force_lightning;
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
	player->Net->Message << (vuint8)svc_change_music
						<< SongName
						<< (vuint8)0;
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
	int			j;

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

	const mapInfo_t& old_info = P_GetMapInfo(GLevel->MapName);
	const mapInfo_t& new_info = P_GetMapInfo(GLevelInfo->NextMap);
	const VClusterDef* ClusterD = P_GetClusterDef(old_info.Cluster);

	for (i = 0; i < MAXPLAYERS; i++)
	{
		if (GGameInfo->Players[i])
		{
			GGameInfo->Players[i]->eventPlayerExitMap(!old_info.Cluster ||
				!(ClusterD->Flags & CLUSTERF_Hub) ||
				old_info.Cluster != new_info.Cluster);
		}
	}

	*sv_reliable << (vuint8)svc_intermission
				<< *GLevelInfo->NextMap;
	for (i = 0; i < MAXPLAYERS; i++)
	{
		if (GGameInfo->Players[i])
		{
			*sv_reliable << (vuint8)true;
			for (j = 0; j < MAXPLAYERS; j++)
				*sv_reliable << (vuint8)GGameInfo->Players[i]->FragsStats[j];
			*sv_reliable << (vint16)GGameInfo->Players[i]->KillCount
						<< (vint16)GGameInfo->Players[i]->ItemCount
						<< (vint16)GGameInfo->Players[i]->SecretCount;
		}
		else
		{
			*sv_reliable << (vuint8)false;
			for (j = 0; j < MAXPLAYERS; j++)
				*sv_reliable << (vuint8)0;
			*sv_reliable << (vint16)0
						<< (vint16)0
						<< (vint16)0;
		}
	}
}

//==========================================================================
//
//	G_ExitLevel
//
//==========================================================================

void G_ExitLevel(int Position)
{ 
	guard(G_ExitLevel);
	LeavePosition = Position;
	completed = true;
	unguard;
}

//==========================================================================
//
//	G_SecretExitLevel
//
//==========================================================================

void G_SecretExitLevel(int Position)
{
	guard(G_SecretExitLevel);
	if (GLevelInfo->SecretMap == NAME_None)
	{
		// No secret map, use normal exit
		G_ExitLevel(Position);
		return;
	}

	LeavePosition = Position;
	completed = true;

	GLevelInfo->NextMap = GLevelInfo->SecretMap; 	// go to secret level

	for (int i = 0; i < MAXPLAYERS; i++)
	{
		if (GGameInfo->Players[i])
		{
			GGameInfo->Players[i]->PlayerFlags |= VBasePlayer::PF_DidSecret;
		}
	}
	unguard;
} 

//==========================================================================
//
//	G_Completed
//
//	Starts intermission routine, which is used only during hub exits,
// and DeathMatch games.
//
//==========================================================================

void G_Completed(int InMap, int InPosition, int SaveAngle)
{
	guard(G_Completed);
	int Map = InMap;
	int Position = InPosition;
	if (Map == -1 && Position == -1)
	{
		if (!deathmatch)
		{
			*sv_reliable << (vuint8)svc_finale <<
				(VStr(GLevelInfo->NextMap).StartsWith("EndGame") ?
				*GLevelInfo->NextMap : "");
			sv.intermission = 2;
			return;
		}
		Map = 1;
		Position = 0;
	}
	GLevelInfo->NextMap = P_GetMapNameByLevelNum(Map);

	LeavePosition = Position;
	completed = true;
	unguard;
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
			*sv_reliable << (vuint8)svc_finale << *GLevelInfo->NextMap;
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

int NET_SendToAll(VMessageOut* data, int blocktime)
{
	guard(NET_SendToAll);
	double		start;
	int			i;
	int			count = 0;
	bool		state1[MAXPLAYERS];
	bool		state2[MAXPLAYERS];

	data->bReliable = true;
	for (i = 0; i < svs.max_clients; i++)
	{
		sv_player = GGameInfo->Players[i];
		if (sv_player)
		{
			if (sv_player->Net->IsLocalConnection())
			{
				sv_player->Net->Channels[0]->SendMessage(data);
				state1[i] = true;
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
			sv_player = GGameInfo->Players[i];
			if (!state1[i])
			{
				state1[i] = true;
				sv_player->Net->Channels[0]->SendMessage(data);
				count++;
				continue;
			}

			if (!state2[i])
			{
				if (!sv_player->Net->Channels[0]->OutMsg)
				{
					state2[i] = true;
				}
				else
				{
					sv_player->Net->GetMessages();
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
//	SV_InitModels
//
//==========================================================================

static void SV_InitModelLists()
{
	nummodels = 1;
	numskins = 1;
}

//==========================================================================
//
//	SV_FindModel
//
//==========================================================================

int SV_FindModel(const char *name)
{
	guard(SV_FindModel);
	int i;

	if (!name || !*name)
	{
		return 0;
	}
	for (i = 0; i < nummodels; i++)
	{
		if (models[i] == name)
		{
			return i;
		}
	}
	models[i] = name;
	nummodels++;
	*sv_reliable << (vuint8)svc_model
				<< (vint16)i
				<< name;
	return i;
	unguard;
}

//==========================================================================
//
//	SV_GetModelIndex
//
//==========================================================================

int SV_GetModelIndex(const VName &Name)
{
	guard(SV_GetModelIndex);
	int i;

	if (Name == NAME_None)
	{
		return 0;
	}
	for (i = 0; i < nummodels; i++)
	{
		if (Name == models[i])
		{
			return i;
		}
	}
	models[i] = Name;
	nummodels++;
	*sv_reliable << (vuint8)svc_model
				<< (vint16)i
				<< *Name;
	return i;
	unguard;
}

//==========================================================================
//
//	SV_FindSkin
//
//==========================================================================

int SV_FindSkin(const char *name)
{
	guard(SV_FindSkin);
	int i;

	if (!name || !*name)
	{
		return 0;
	}
	for (i = 0; i < numskins; i++)
	{
		if (skins[i] == name)
		{
			return i;
		}
	}
	skins[i] = name;
	numskins++;
	*sv_reliable << (vuint8)svc_skin
				<< (vuint8)i
				<< name;
	return i;
	unguard;
}

//==========================================================================
//
//	SV_SendServerInfo
//
//==========================================================================

void SV_SendServerInfo(VBasePlayer *player)
{
	guard(SV_SendServerInfo);
	int				i;
	VMessageOut&	msg = player->Net->Message;

	((VLevelChannel*)player->Net->Channels[CHANIDX_Level])->SetLevel(GLevel);

	msg << (vuint8)svc_server_info
		<< (vuint8)PROTOCOL_VERSION
		<< svs.serverinfo
		<< *GLevel->MapName
		<< (vuint8)SV_GetPlayerNum(player)
		<< (vuint8)svs.max_clients
		<< (vuint8)deathmatch;

	for (i = 0; i < MAXPLAYERS; i++)
	{
		msg << (vuint8)svc_userinfo
			<< (vuint8)i
			<< (GGameInfo->Players[i] ? GGameInfo->Players[i]->UserInfo : "");
	}

/*	for (i = 0; i < VMemberBase::GMembers.Num(); i++)
	{
		if (VMemberBase::GMembers[i]->MemberType == MEMBER_Class)
		{
			VClass* C = static_cast<VClass*>(VMemberBase::GMembers[i]);
			if (C->IsChildOf(VThinker::StaticClass()))
			{
				msg << (vuint8)svc_class_name
					<< (vuint16)C->NetId
					<< C->GetName();
			}
		}
	}*/

	for (i = 1; i < nummodels; i++)
	{
		msg << (vuint8)svc_model
			<< (vint16)i
			<< *models[i];
	}

	for (i = 1; i < numskins; i++)
	{
		msg << (vuint8)svc_skin
			<< (vuint8)i
			<< *skins[i];
	}

	msg << (vuint8)svc_signonnum
		<< (vuint8)1;
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
		if (GGameInfo->Players[i])
		{
			GGameInfo->Players[i]->Level = GLevelInfo;
			SV_SendServerInfo(GGameInfo->Players[i]);
			if (GGameInfo->Players[i]->PlayerFlags & VBasePlayer::PF_IsBot)
			{
				sv_player = GGameInfo->Players[i];
				SV_RunClientCommand("PreSpawn\n");
				SV_RunClientCommand("Spawn\n");
				SV_RunClientCommand("Begin\n");
			}
		}
	}
	unguard;
}

//==========================================================================
//
//	SV_SpawnServer
//
//==========================================================================

void SV_SpawnServer(const char *mapname, bool spawn_thinkers)
{
	guard(SV_SpawnServer);
	int			i;

	GCon->Logf(NAME_Dev, "Spawning server %s", mapname);
	paused = false;
	mapteleport_issued = false;

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
			memset(GGameInfo->Players[i]->FragsStats, 0, sizeof(GGameInfo->Players[i]->FragsStats));
			if (GGameInfo->Players[i]->PlayerState == PST_DEAD)
				GGameInfo->Players[i]->PlayerState = PST_REBORN;
			GGameInfo->Players[i]->Net->Message.Clear();
		}
	}
	else
	{
		//	New game
		GWorldInfo = GGameInfo->eventCreateWorldInfo();
	}

	SV_Clear();
	VCvar::Unlatch();

	sv.active = true;

	//	Load it
	SV_LoadLevel(VName(mapname, VName::AddLower8));
	GLevel->WorldInfo = GWorldInfo;

	const mapInfo_t& info = P_GetMapInfo(GLevel->MapName);

	netgame = svs.max_clients > 1;
	deathmatch = DeathMatch;

	GGameInfo->gameskill = gameskill;
	GGameInfo->netgame = netgame;
	GGameInfo->deathmatch = deathmatch;

	P_InitThinkers();

	if (spawn_thinkers)
	{
		//	Create level info.
		GLevelInfo = (VLevelInfo*)GLevel->SpawnThinker(
			GGameInfo->LevelInfoClass);
		GLevelInfo->Level = GLevelInfo;
		GLevel->LevelInfo = GLevelInfo;
		if (info.Gravity)
			GLevelInfo->Gravity = info.Gravity * DEFAULT_GRAVITY / 800.0;
		else
			GLevelInfo->Gravity = sv_gravity * DEFAULT_GRAVITY / 800.0;
		GLevelInfo->SetMapInfo(info);

		//	Spawn things.
		for (i = 0; i < GLevel->NumThings; i++)
		{
			GLevelInfo->eventSpawnMapThing(&GLevel->Things[i]);
		}
		if (deathmatch && GLevelInfo->NumDeathmatchStarts < 4)
		{
			Host_Error("Level needs more deathmatch start spots");
		}
	}
	GLevel->InitPolyobjs(); // Initialise the polyobjs

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

	SV_InitModelLists();

	VMemberBase::SetUpNetClasses();

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
	GLevelInfo->LevelInfoFlags |= VLevelInfo::LIF_BegunPlay;

	//	Start open scripts.
	GLevel->Acs->StartTypedACScripts(SCRIPT_Open);

	P_Ticker();
	P_Ticker();

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

	for (VMessageOut* Msg = sv_signons; Msg; Msg = Msg->Next)
	{
		Msg->bReliable = true;
		sv_player->Net->Channels[0]->SendMessage(Msg);
	}
	sv_player->Net->Message << (vuint8)svc_signonnum << (vuint8)2;
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

	if (!sv_loading)
	{
		if (sv_player->PlayerFlags & VBasePlayer::PF_Spawned)
		{
			GCon->Log(NAME_Dev, "Already spawned");
		}
		if (sv_player->MO)
		{
			GCon->Log(NAME_Dev, "Mobj already spawned");
		}
		sv_player->eventSpawnClient();
		for (int i = 0; i < sv_ActiveSequences.Num(); i++)
		{
			sv_player->eventClientStartSequence(
				sv_ActiveSequences[i].Origin,
				sv_ActiveSequences[i].OriginId,
				sv_ActiveSequences[i].Name,
				sv_ActiveSequences[i].ModeNum);
			for (int j = 0; j < sv_ActiveSequences[i].Choices.Num(); j++)
			{
				sv_player->eventClientAddSequenceChoice(
					sv_ActiveSequences[i].OriginId,
					sv_ActiveSequences[i].Choices[j]);
			}
		}
	}
	else
	{
		if (!sv_player->MO)
		{
			Host_Error("Player without Mobj\n");
		}
	}
	sv_player->Net->Message << (vuint8)svc_set_angles
						<< (vuint8)(AngleToByte(sv_player->ViewAngles.pitch))
						<< (vuint8)(AngleToByte(sv_player->ViewAngles.yaw))
						<< (vuint8)0;
	sv_player->Net->Message << (vuint8)svc_signonnum << (vuint8)3;
	sv_player->PlayerFlags &= ~VBasePlayer::PF_FixAngle;
	((VPlayerChannel*)sv_player->Net->Channels[CHANIDX_Player])->SetPlayer(NULL);
	((VPlayerChannel*)sv_player->Net->Channels[CHANIDX_Player])->SetPlayer(sv_player);
	unguard;
}

//==========================================================================
//
//	COMMAND Begin
//
//==========================================================================

COMMAND(Begin)
{
	guard(COMMAND Begin);
	if (Source == SRC_Command)
	{
		GCon->Log("Begin is not valid from console");
		return;
	}

	if (!netgame || svs.num_connected == sv_load_num_players)
	{
		sv_loading = false;
	}

	sv_player->PlayerFlags |= VBasePlayer::PF_Spawned;

	// For single play, save immediately into the reborn slot
	if (!netgame)
	{
		SV_SaveGame(SV_GetRebornSlot(), REBORN_DESCRIPTION);
	}
	unguard;
}

//==========================================================================
//
//	SV_DropClient
//
//==========================================================================

void SV_DropClient(bool)
{
	guard(SV_DropClient);
	if (sv_player->PlayerFlags & VBasePlayer::PF_Spawned)
	{
		sv_player->eventDisconnectClient();
	}
	sv_player->PlayerFlags &= ~VBasePlayer::PF_Active;
	GGameInfo->Players[SV_GetPlayerNum(sv_player)] = NULL;
	sv_player->PlayerFlags &= ~VBasePlayer::PF_Spawned;

	delete sv_player->Net;
	sv_player->Net = NULL;

	svs.num_connected--;
	sv_player->UserInfo = VStr();
	*sv_reliable << (vuint8)svc_userinfo
				<< (vuint8)SV_GetPlayerNum(sv_player)
				<< "";
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
#if 0
	double	start;

	// flush any pending messages - like the score!!!
	start = Sys_FloatTime();
	do
	{
		count = 0;
		for (i=0, sv_player = svs.clients ; i<svs.maxclients ; i++, sv_player++)
		{
			if (sv_player->PlayerFlags & VBasePlayer::PF_Active && sv_player->Message.cursize)
			{
				if (NET_CanSendMessage (sv_player->netconnection))
				{
					NET_SendMessage(sv_player->netconnection, &sv_player->Message);
					SZ_Clear (&sv_player->Message);
				}
				else
				{
					NET_GetMessage(sv_player->netconnection);
					count++;
				}
			}
		}
		if ((Sys_FloatTime() - start) > 3.0)
			break;
	}
	while (count);
#endif

	// make sure all the clients know we're disconnecting
	VMessageOut msg(128 << 3);
	msg << (vuint8)svc_disconnect;
	count = NET_SendToAll(&msg, 5);
	if (count)
		GCon->Logf("Shutdown server failed for %d clients", count);

	for (i = 0; i < svs.max_clients; i++)
	{
		sv_player = GGameInfo->Players[i];
		if (sv_player)
			SV_DropClient(crash);
	}

	//
	// clear structures
	//
	if (GLevel)
	{
		delete GLevel;
		GLevel = NULL;
	}
	if (GWorldInfo)
	{
		delete GWorldInfo;
		GWorldInfo = NULL;
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
	sv_ActiveSequences.Clear();
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
		SV_SpawnServer(*GLevel->MapName, true);
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
	*sv_reliable << (vuint8)svc_pause << (vuint8)paused;
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

	SV_ClientPrintf(sv_player, "Kills: %d of %d", sv_player->KillCount, GLevelInfo->TotalKills);
	SV_ClientPrintf(sv_player, "Items: %d of %d", sv_player->ItemCount, GLevelInfo->TotalItems);
	SV_ClientPrintf(sv_player, "Secrets: %d of %d", sv_player->SecretCount, GLevelInfo->TotalSecret);
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
	GCon->Logf(NAME_Dev, "Client %s connected", *player->Net->GetAddress());

	GGameInfo->Players[SV_GetPlayerNum(player)] = player;
	player->PlayerFlags |= VBasePlayer::PF_Active;

	player->Net->Message.AllowOverflow = true;		// we can catch it
	player->Net->LastMessage = 0;
	player->Net->NeedsUpdate = false;
	player->PlayerFlags &= ~VBasePlayer::PF_Spawned;
	player->Level = GLevelInfo;
	if (!sv_loading)
	{
		player->MO = NULL;
		player->PlayerState = PST_REBORN;
		player->eventPutClientIntoServer();
	}
	player->Frags = 0;
	memset(player->FragsStats, 0, sizeof(player->FragsStats));

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

		GPlayersBase[i]->Net = new VNetConnection(sock, ServerNetContext);
		((VPlayerChannel*)GPlayersBase[i]->Net->Channels[CHANIDX_Player])->SetPlayer(GPlayersBase[i]);
		SV_ConnectClient(GPlayersBase[i]);
		svs.num_connected++;
	}
	unguard;
}

//==========================================================================
//
//	SV_ConnectBot
//
//==========================================================================

void SV_SetUserInfo(const VStr& info);

void SV_ConnectBot(const char *name)
{
	guard(SV_ConnectBot);
	VSocketPublic*	sock;
	int				i;

	if (svs.num_connected >= svs.max_clients)
	{
		GCon->Log("Server is full");
		return;
	}

	GNet->ConnectBot = true;
	sock = GNet->CheckNewConnections();
	if (!sock)
		return;

	//
	// init a new client structure
	//
	for (i = 0; i < svs.max_clients; i++)
		if (!GGameInfo->Players[i])
			break;
	if (i == svs.max_clients)
		Sys_Error("SV_ConnectBot: no free clients");

	GPlayersBase[i]->PlayerFlags |= VBasePlayer::PF_IsBot;
	GPlayersBase[i]->PlayerName = name;
	GPlayersBase[i]->Net = new VNetConnection(sock, ServerNetContext);
	GPlayersBase[i]->Net->AutoAck = true;
	((VPlayerChannel*)GPlayersBase[i]->Net->Channels[CHANIDX_Player])->SetPlayer(GPlayersBase[i]);
	SV_ConnectClient(GPlayersBase[i]);
	svs.num_connected++;

	sv_player = GGameInfo->Players[i];
	sv_player->Net->Message.Clear();
	SV_RunClientCommand("PreSpawn\n");
	sv_player->Net->Message.Clear();
	SV_SetUserInfo(sv_player->UserInfo);
	SV_RunClientCommand("Spawn\n");
	sv_player->Net->Message.Clear();
	SV_RunClientCommand("Begin\n");
	sv_player->Net->Message.Clear();
	unguard;
}

//==========================================================================
//
//	SV_GetSignon
//
//==========================================================================

VMessageOut* SV_GetSignon(int Len)
{
	guard(SV_GetSignon);
	VMessageOut* Msg = sv_signons;
	while (Msg->Next)
	{
		Msg = Msg->Next;
	}
	if (!Msg->CheckSpaceBits(Len))
	{
		Msg->Next = new VMessageOut(OUT_MESSAGE_SIZE);
		Msg = Msg->Next;
		Msg->Next = NULL;
	}
	return Msg;
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
	char	mapname[12];

	if (Args.Num() != 2)
	{
		GCon->Log("map <mapname> : change level");
		return;
	}
	VStr::Cpy(mapname, *Args[1]);

	SV_ShutdownServer(false);
#ifdef CLIENT
	CL_Disconnect();
#endif

	SV_InitBaseSlot();
	SV_ClearRebornSlot();
	// Default the player start spot group to 0
	RebornPosition = 0;
	GGameInfo->RebornPosition = RebornPosition;

	if ((int)Skill < sk_baby)
		Skill = sk_baby;
	if ((int)Skill > sk_nightmare)
		Skill = sk_nightmare;

	// Set up a bunch of globals
	gameskill = (skill_t)(int)Skill;
	GGameInfo->eventInitNewGame(gameskill);

	SV_SpawnServer(mapname, true);
#ifdef CLIENT
	if (cls.state != ca_dedicated)
		GCmdBuf << "Connect local\n";
#endif
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
	SV_ClearDatagram();

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

	VStr Text = sv_player->PlayerName;
	Text += ":";
	for (int i = 1; i < Args.Num(); i++)
	{
		Text += " ";
		Text += Args[i];
	}
	SV_BroadcastPrintf(*Text);
	SV_StartSound(NULL, GSoundManager->GetSoundID("misc/chat"), 0, 1.0, 0);
	unguard;
}

//==========================================================================
//
//	VLevelInfo natives
//
//==========================================================================

IMPLEMENT_FUNCTION(VLevelInfo, AddStaticLight)
{
	P_GET_FLOAT(Radius);
	P_GET_VEC(Origin);
	P_GET_SELF;
	VMessageOut* Msg = SV_GetSignon(8 << 3);
	*Msg << (vuint8)svc_static_light
		<< (short)Origin.x
		<< (short)Origin.y
		<< (short)Origin.z
		<< (vuint8)(Radius / 8.0);
}

IMPLEMENT_FUNCTION(VLevelInfo, AddStaticLightRGB)
{
	P_GET_INT(Colour);
	P_GET_FLOAT(Radius);
	P_GET_VEC(Origin);
	P_GET_SELF;
	VMessageOut* Msg = SV_GetSignon(12 << 3);
	*Msg << (vuint8)svc_static_light_rgb
		<< (short)Origin.x
		<< (short)Origin.y
		<< (short)Origin.z
		<< (vuint8)(Radius / 8.0)
		<< Colour;
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
