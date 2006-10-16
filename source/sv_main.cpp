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
#include "progdefs.h"
#include "sv_local.h"

// MACROS ------------------------------------------------------------------

#define TOCENTRE				-128

#define TEXTURE_TOP				0
#define TEXTURE_MIDDLE			1
#define TEXTURE_BOTTOM			2

#define REBORN_DESCRIPTION		"TEMP GAME"

#define MAX_MODELS				512
#define MAX_SPRITES				512
#define MAX_SKINS				256

// TYPES -------------------------------------------------------------------

// EXTERNAL FUNCTION PROTOTYPES --------------------------------------------

void Draw_TeleportIcon();
void CL_Disconnect();
bool SV_ReadClientMessages(int i);
void SV_RunClientCommand(const char *cmd);
void EntInit();

// PUBLIC FUNCTION PROTOTYPES ----------------------------------------------

void SV_DropClient(bool crash);
void SV_ShutdownServer(boolean);

// PRIVATE FUNCTION PROTOTYPES ---------------------------------------------

static void G_DoReborn(int playernum);
static void G_DoCompleted();

// EXTERNAL DATA DECLARATIONS ----------------------------------------------

// PUBLIC DATA DEFINITIONS -------------------------------------------------

IMPLEMENT_CLASS(V, LevelInfo)
IMPLEMENT_CLASS(V, GameInfo)
IMPLEMENT_CLASS(V, BasePlayer)
IMPLEMENT_CLASS(V, ViewEntity)

VCvarI			real_time("real_time", "1");

server_t		sv;
server_static_t	svs;

// increment every time a check is made
int				validcount = 1;

bool			sv_loading = false;
int				sv_load_num_players;

VBasePlayer*	GPlayersBase[MAXPLAYERS];

skill_t         gameskill; 

boolean         paused;

boolean         deathmatch = false;   	// only if started as net death
boolean         netgame;                // only true if packets are broadcast

VEntity**		sv_mobjs;
mobj_base_t*	sv_mo_base;
double*			sv_mo_free_time;

vuint8			sv_reliable_buf[MAX_MSGLEN];
VMessage		sv_reliable(sv_reliable_buf, MAX_MSGLEN);
vuint8			sv_datagram_buf[MAX_DATAGRAM];
VMessage		sv_datagram(sv_datagram_buf, MAX_DATAGRAM);
vuint8			sv_signon_buf[MAX_MSGLEN];
VMessage		sv_signon(sv_signon_buf, MAX_MSGLEN);

VBasePlayer*	sv_player;

VName			sv_next_map;
VName			sv_secret_map;

int 			TimerGame;

boolean			in_secret;
VName			mapaftersecret;

VGameInfo*		GGameInfo;
VLevelInfo*		GLevelInfo;

TArray<VSndSeqInfo>	sv_ActiveSequences;

// PRIVATE DATA DEFINITIONS ------------------------------------------------

static int 		LeavePosition;

static int		RebornPosition;	// Position indicator for cooperative net-play reborn

static bool		completed;

static int		num_stats;
static int		num_string_stats;

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

	svs.max_clients = 1;

	sv_mobjs = new VEntity*[GMaxEntities];
	sv_mo_base = new mobj_base_t[GMaxEntities];
	sv_mo_free_time = new double[GMaxEntities];
	memset(sv_mobjs, 0, sizeof(VEntity*) * GMaxEntities);
	memset(sv_mo_base, 0, sizeof(mobj_base_t) * GMaxEntities);
	memset(sv_mo_free_time, 0, sizeof(double) * GMaxEntities);

	VMemberBase::StaticLoadPackage(NAME_svprogs);

	GGameInfo = (VGameInfo*)VObject::StaticSpawnObject(
		VClass::FindClass("MainGameInfo"));
	GGameInfo->eventInit();

	num_stats = GGameInfo->num_stats;
	num_string_stats = GGameInfo->num_string_stats;

	VClass* PlayerClass = VClass::FindClass("Player");
	for (i = 0; i < MAXPLAYERS; i++)
	{
		GPlayersBase[i] = (VBasePlayer*)VObject::StaticSpawnObject(
			PlayerClass);
		if (num_string_stats && sizeof(VStr) > 4)
			num_stats = (num_stats + 1) & ~1;
		int TotalStats = num_stats + num_string_stats * (sizeof(VStr) / 4);
		GPlayersBase[i]->OldStats = new vint32[TotalStats];
		memset(GPlayersBase[i]->OldStats, 0, sizeof(vint32) * TotalStats);
	}

	GGameInfo->validcount = &validcount;
	GGameInfo->level = &level;
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
			for (int j = 0; j < num_string_stats; j++)
			{
				((VStr*)(GPlayersBase[i]->OldStats + num_stats))[j].Clean();
			}
			delete[] GPlayersBase[i]->OldStats;
			GPlayersBase[i]->ConditionalDestroy();
		}
	}
	delete[] sv_mobjs;
	delete[] sv_mo_base;
	delete[] sv_mo_free_time;
	level.LevelName.Clean();
	
	P_FreeTerrainTypes();
	P_ClearButtons();
	svs.serverinfo.Clean();
	for (int i = 0; i < MAX_SKINS; i++)
	{
		skins[i].Clean();
	}
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
	level.LevelName.Clean();
	memset(&level, 0, sizeof(level));
	memset(sv_mobjs, 0, sizeof(VEntity *) * GMaxEntities);
	memset(sv_mo_base, 0, sizeof(mobj_base_t) * GMaxEntities);
	memset(sv_mo_free_time, 0, sizeof(double) * GMaxEntities);
	sv_signon.Clear();
	sv_reliable.Clear();
	sv_datagram.Clear();
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
	sv_datagram.Clear();
	unguard;
}

//==========================================================================
//
//	SV_SpawnMobj
//
//==========================================================================

VEntity *SV_SpawnMobj(VClass *Class)
{
	guard(SV_SpawnMobj);
	VEntity *Ent;
	int i;

	Ent = (VEntity*)VObject::StaticSpawnObject(Class);
	GLevel->AddThinker(Ent);

	//	Client treats first objects as player objects and will use
	// models and skins from player info
	for (i = svs.max_clients + 1; i < GMaxEntities; i++)
	{
		if (!sv_mobjs[i] && (sv_mo_free_time[i] < 1.0 ||
			level.time - sv_mo_free_time[i] > 2.0))
		{
			break;
		}
	}
	if (i == GMaxEntities)
	{
		Sys_Error("SV_SpawnMobj: Overflow");
	}
	sv_mobjs[i] = Ent;
	Ent->NetID = i;

	if (GLevelInfo->LevelInfoFlags & VLevelInfo::LIF_BegunPlay)
	{
		Ent->eventBeginPlay();
	}

	return Ent;
	unguard;
}

int c_bigClass;
int c_bigState;
//==========================================================================
//
//	SV_GetMobjBits
//
//==========================================================================

int SV_GetMobjBits(VEntity &mobj, mobj_base_t &base)
{
	guard(SV_GetMobjBits);
	int		bits = 0;

	if (fabs(base.Origin.x - mobj.Origin.x) >= 1.0)
		bits |= MOB_X;
	if (fabs(base.Origin.y - mobj.Origin.y) >= 1.0)
		bits |= MOB_Y;
	if (fabs(base.Origin.z - (mobj.Origin.z - mobj.FloorClip)) >= 1.0)
		bits |= MOB_Z;
	if (AngleToByte(base.Angles.yaw) != AngleToByte(mobj.Angles.yaw))
		bits |= MOB_ANGLE;
	if (AngleToByte(base.Angles.pitch) != AngleToByte(mobj.Angles.pitch))
		bits |= MOB_ANGLEP;
	if (AngleToByte(base.Angles.roll) != AngleToByte(mobj.Angles.roll))
		bits |= MOB_ANGLER;
	if (base.SpriteType != mobj.SpriteType)
		bits |= MOB_SPRITE;
	if (mobj.EntityFlags & VEntity::EF_FullBright)
		bits |= MOB_FULL_BRIGHT;
	if (base.Translucency != mobj.Translucency)
		bits |= MOB_TRANSLUC;
	if (base.Translation != mobj.Translation)
		bits |= MOB_TRANSL;
	if (base.Effects != mobj.Effects)
		bits |= MOB_EFFECTS;
	if ((mobj.EntityFlags & VEntity::EF_FixedModel) &&
		mobj.FixedModelIndex != mobj.State->ModelIndex)
		bits |= MOB_MODEL;
	if (mobj.State->ModelIndex && mobj.ModelSkinNum)
		bits |= MOB_SKIN_NUM;
	else if (mobj.State->ModelIndex && mobj.ModelSkinIndex)
		bits |= MOB_SKIN_IDX;
	if (base.Class != mobj.GetClass())
	{
		bits |= MOB_CLASS;
		if (mobj.GetClass()->NetId > 0xff)
		{
			bits |= MOB_BIG_CLASS;
			c_bigClass++;
		}
	}
	if (base.State != mobj.State)
	{
		bits |= MOB_STATE;
		if (mobj.State->NetId > 0xff)
		{
			bits |= MOB_BIG_STATE;
			c_bigState++;
		}
	}

	return bits;
	unguard;
}

//==========================================================================
//
//	SV_WriteMobj
//
//==========================================================================

void SV_WriteMobj(int bits, VEntity &mobj, VMessage &msg)
{
	guard(SV_WriteMobj);
	if (bits & MOB_BIG_CLASS)
		msg << (vuint16)mobj.GetClass()->NetId;
	else if (bits & MOB_CLASS)
		msg << (vuint8)mobj.GetClass()->NetId;
	if (bits & MOB_BIG_STATE)
		msg << (vuint16)mobj.State->NetId;
	else if (bits & MOB_STATE)
		msg << (vuint8)mobj.State->NetId;
	if (bits & MOB_X)
		msg << (vuint16)mobj.Origin.x;
	if (bits & MOB_Y)
		msg << (vuint16)mobj.Origin.y;
	if (bits & MOB_Z)
		msg << (vuint16)(mobj.Origin.z - mobj.FloorClip);
	if (bits & MOB_ANGLE)
		msg << (vuint8)(AngleToByte(mobj.Angles.yaw));
	if (bits & MOB_ANGLEP)
		msg << (vuint8)(AngleToByte(mobj.Angles.pitch));
	if (bits & MOB_ANGLER)
		msg << (vuint8)(AngleToByte(mobj.Angles.roll));
	if (bits & MOB_SPRITE)
		msg << (vuint8)mobj.SpriteType;
	if (bits & MOB_TRANSLUC)
		msg << (vuint8)mobj.Translucency;
	if (bits & MOB_TRANSL)
		msg << (vuint8)mobj.Translation;
	if (bits & MOB_EFFECTS)
		msg << (vuint8)mobj.Effects;
	if (bits & MOB_MODEL)
		msg << (vuint16)mobj.FixedModelIndex;
	if (bits & MOB_SKIN_NUM)
		msg << (vuint8)mobj.ModelSkinNum;
	else if (bits & MOB_SKIN_IDX)
		msg << (vuint8)mobj.ModelSkinIndex;
	if (bits & MOB_WEAPON)
		msg << (vuint16)mobj.Player->WeaponModel;
	unguard;
}

//==========================================================================
//
//	SV_UpdateMobj
//
//==========================================================================

void SV_UpdateMobj(int i, VMessage &msg)
{
	guard(SV_UpdateMobj);
	int bits;
	int sendnum;

	if (sv_mobjs[i]->EntityFlags & VEntity::EF_IsPlayer)
	{
		sendnum = SV_GetPlayerNum(sv_mobjs[i]->Player) + 1;
	}
	else
	{
		sendnum = i;
	}

	bits = SV_GetMobjBits(*sv_mobjs[i], sv_mo_base[sendnum]);

	if (sv_mobjs[i]->EntityFlags & VEntity::EF_IsPlayer)
	{
		//	Clear look angles, because they must not affect model orientation
		bits &= ~(MOB_ANGLEP | MOB_ANGLER);
		if (sv_mobjs[i]->Player->WeaponModel)
		{
			bits |= MOB_WEAPON;
		}
	}
	if (sendnum > 0xff)
		bits |= MOB_BIG_NUM;
	if (bits > 0xff)
		bits |= MOB_MORE_BITS;
	if (bits > 0xffff)
		bits |= MOB_MORE_BITS2;

	msg << (vuint8)svc_update_mobj;
	if (bits & MOB_MORE_BITS)
		msg << (vuint16)bits;
	else
		msg << (vuint8)bits;
	if (bits & MOB_MORE_BITS2)
		msg << (vuint8)(bits >> 16);
	if (bits & MOB_BIG_NUM)
		msg << (vuint16)sendnum;
	else
		msg << (vuint8)sendnum;

	SV_WriteMobj(bits, *sv_mobjs[i], msg);
	return;
	unguard;
}

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
		if (sv_mobjs[NetID] != this)
			Sys_Error("Invalid entity num %d", NetID);

		eventDestroyed();

		// unlink from sector and block lists
		UnlinkFromWorld();

		// stop any playing sound
		SV_StopSound(this, 0);

		sv_mobjs[NetID] = NULL;
		sv_mo_free_time[NetID] = level.time;
	}

	Super::Destroy();
	unguard;
}

//==========================================================================
//
//	SV_CreateBaseline
//
//==========================================================================

void SV_CreateBaseline()
{
	guard(SV_CreateBaseline);
	int		i;

	int CurId = 0;
	for (i = 0; i < VMemberBase::GMembers.Num(); i++)
	{
		if (VMemberBase::GMembers[i]->MemberType == MEMBER_Class)
		{
			VClass* C = static_cast<VClass*>(VMemberBase::GMembers[i]);
			if (C->IsChildOf(VThinker::StaticClass()))
			{
				C->NetId = CurId++;
			}
		}
	}

	for (i = 0; i < GLevel->NumSectors; i++)
	{
		sector_t &sec = GLevel->Sectors[i];
		if (sec.floor.translucency)
		{
			sv_signon << (vuint8)svc_sec_transluc
					<< (vuint16)i
					<< (vuint8)sec.floor.translucency;
		}
	}

	for (i = 0; i < GMaxEntities; i++)
	{
		if (!sv_mobjs[i])
			continue;
		if (sv_mobjs[i]->EntityFlags & VEntity::EF_Hidden)
			continue;

		//NOTE Do we really need 10 bytes extra?
		if (!sv_signon.CheckSpace(30))
		{
			GCon->Log(NAME_Dev, "SV_CreateBaseline: Overflow");
			return;
		}

		VEntity &mobj = *sv_mobjs[i];
		mobj_base_t &base = sv_mo_base[i];

		base.Class = mobj.GetClass();
		base.State = mobj.State;
		base.Origin.x = mobj.Origin.x;
		base.Origin.y = mobj.Origin.y;
		base.Origin.z = mobj.Origin.z - mobj.FloorClip;
		base.Angles.yaw = mobj.Angles.yaw;
		base.Angles.pitch = mobj.Angles.pitch;
		base.Angles.roll = mobj.Angles.roll;
		base.SpriteType = mobj.SpriteType;
		base.Translucency = mobj.Translucency;
		base.Translation = mobj.Translation;
		base.Effects = mobj.Effects;

		sv_signon << (vuint8)svc_spawn_baseline
					<< (vuint16)i
					<< (vuint16)mobj.GetClass()->NetId
					<< (vuint16)mobj.State->NetId
					<< (vuint16)mobj.Origin.x
					<< (vuint16)mobj.Origin.y
					<< (vuint16)(mobj.Origin.z - mobj.FloorClip)
					<< (vuint8)(AngleToByte(mobj.Angles.yaw))
					<< (vuint8)(AngleToByte(mobj.Angles.pitch))
					<< (vuint8)(AngleToByte(mobj.Angles.roll))
					<< (vuint8)mobj.SpriteType
					<< (vuint8)mobj.Translucency
					<< (vuint8)mobj.Translation
					<< (vuint8)mobj.Effects;
	}
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

	if (mobj->EntityFlags & VEntity::EF_IsPlayer)
	{
		return SV_GetPlayerNum(mobj->Player) + 1;
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
	if (!sv_datagram.CheckSpace(12))
		return;

	sv_datagram << (vuint8)svc_start_sound
				<< (vuint16)sound_id
				<< (vuint16)(origin_id | (channel << 13));
	if (origin_id)
	{
		sv_datagram << (vuint16)origin.x
					<< (vuint16)origin.y
					<< (vuint16)origin.z;
	}
	sv_datagram << (vuint8)(volume * 127)
		<< (vuint8)(Attenuation * 64);
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
	if (!sv_datagram.CheckSpace(3))
		return;

	sv_datagram << (vuint8)svc_stop_sound
				<< (vuint16)(origin_id | (channel << 13));
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
	guard(SV_StartSound);
	if (Ent && Ent->Player)
	{
		Ent->Player->Message << (vuint8)svc_start_sound
							<< (vuint16)SoundId
							<< (vuint16)(Channel << 13)
							<< (vuint8)(Volume * 127)
							<< (vuint8)(Attenuation * 64);
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

	sv_reliable << (vuint8)svc_start_seq
				<< (vuint16)OriginId
				<< (vuint16)Origin.x
				<< (vuint16)Origin.y
				<< (vuint16)Origin.z
				<< *Name
				<< (vuint8)ModeNum;
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

	sv_reliable << (vuint8)svc_add_seq_choice
				<< (vuint16)origin_id
				<< *Choice;
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

	sv_reliable << (vuint8)svc_stop_seq
				<< (vuint16)origin_id;
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

	player->Message << (vuint8)svc_print << buf;
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

	player->Message << (vuint8)svc_centre_print << buf;
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
			GGameInfo->Players[i]->Message << (vuint8)svc_print << buf;
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
			GGameInfo->Players[i]->Message << (vuint8)svc_centre_print << buf;
	unguard;
}

//==========================================================================
//
//	SV_WriteViewData
//
//==========================================================================

void SV_WriteViewData(VBasePlayer &player, VMessage &msg)
{
	guard(SV_WriteViewData);
	int		i;

	msg << (vuint8)svc_view_data
		<< player.ViewOrg.x
		<< player.ViewOrg.y
		<< player.ViewOrg.z
		<< (vuint8)player.ExtraLight
		<< (vuint8)player.FixedColourmap
		<< (vuint8)player.MO->Translucency
		<< (vuint16)player.PSpriteSY;
	if (player.ViewEnts[0] && player.ViewEnts[0]->State)
	{
		msg << (vuint16)player.ViewEnts[0]->SpriteIndex
			<< (vuint8)player.ViewEnts[0]->SpriteFrame
			<< (vuint16)player.ViewEnts[0]->ModelIndex
			<< (vuint8)player.ViewEnts[0]->ModelFrame
			<< (vuint16)player.ViewEnts[0]->SX
			<< (vuint16)player.ViewEnts[0]->SY;
	}
	else
	{
		msg << (vint16)-1;
	}
	if (player.ViewEnts[1] && player.ViewEnts[1]->State)
	{
		msg << (vuint16)player.ViewEnts[1]->SpriteIndex
			<< (vuint8)player.ViewEnts[1]->SpriteFrame
			<< (vuint16)player.ViewEnts[1]->ModelIndex
			<< (vuint8)player.ViewEnts[1]->ModelFrame
			<< (vuint16)player.ViewEnts[1]->SX
			<< (vuint16)player.ViewEnts[1]->SY;
	}
	else
	{
		msg << (vint16)-1;
	}

	msg << (vuint8)player.Health
		<< player.Items
		<< (vint16)player.Frags;

	int bits = 0;
	for (i = 0; i < NUM_CSHIFTS; i++)
		if (player.CShifts[i] & 0xff000000)
			bits |= (1 << i);
	msg << (vuint8)bits;
	for (i = 0; i < NUM_CSHIFTS; i++)
		if (player.CShifts[i] & 0xff000000)
			msg << player.CShifts[i];

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
//	SV_UpdateLevel
//
//==========================================================================

void SV_UpdateLevel(VMessage &msg)
{
	guard(SV_UpdateLevel);
	int		i;
	int		bits;

	fatpvs = GLevel->LeafPVS(sv_player->MO->SubSector);

	for (i = 0; i < GLevel->NumSectors; i++)
	{
		sector_t	*sec;

		sec = &GLevel->Sectors[i];
		if (!SV_SecCheckFatPVS(sec) && !(sec->SectorFlags & sector_t::SF_ExtrafloorSource))
			continue;

		bits = 0;

		if (fabs(sec->base_floorheight - sec->floor.dist) >= 1.0)
			bits |= SUB_FLOOR;
		if (fabs(sec->base_ceilingheight - sec->ceiling.dist) >= 1.0)
			bits |= SUB_CEIL;
		if (abs(sec->base_lightlevel - sec->params.lightlevel) >= 4)
			bits |= SUB_LIGHT;
		if (sec->floor.xoffs)
			bits |= SUB_FLOOR_X;
		if (sec->floor.yoffs)
			bits |= SUB_FLOOR_Y;
		if (sec->ceiling.xoffs)
			bits |= SUB_CEIL_X;
		if (sec->ceiling.yoffs)
			bits |= SUB_CEIL_Y;

		if (!bits)
			continue;

		if (!msg.CheckSpace(14))
		{
			GCon->Log(NAME_Dev, "UpdateLevel: secs overflow");
			return;
		}
		
		if (i > 255)
			bits |= SUB_BIG_NUM;
		msg << (vuint8)svc_sec_update
			<< (vuint8)bits;
		if (bits & SUB_BIG_NUM)
			msg << (vuint16)i;
		else
			msg << (vuint8)i;
		if (bits & SUB_FLOOR)
			msg << (vuint16)(sec->floor.dist);
		if (bits & SUB_CEIL)
			msg << (vuint16)(sec->ceiling.dist);
		if (bits & SUB_LIGHT)
			msg << (vuint8)(sec->params.lightlevel >> 2);
		if (bits & SUB_FLOOR_X)
			msg << (vuint8)(sec->floor.xoffs);
		if (bits & SUB_FLOOR_Y)
			msg << (vuint8)(sec->floor.yoffs);
		if (bits & SUB_CEIL_X)
			msg << (vuint8)(sec->ceiling.xoffs);
		if (bits & SUB_CEIL_Y)
			msg << (vuint8)(sec->ceiling.yoffs);
	}
	for (i = 0; i < GLevel->NumSides; i++)
	{
		side_t		*side;

		side = &GLevel->Sides[i];
		if (!SV_SecCheckFatPVS(side->sector))
			continue;

		if (side->base_textureoffset == side->textureoffset &&
			side->base_rowoffset == side->rowoffset)
			continue;

		if (!msg.CheckSpace(7))
		{
			GCon->Log(NAME_Dev, "UpdateLevel: sides overflow");
			return;
		}

		msg << (vuint8)svc_side_ofs
			<< (vuint16)i
			<< (vuint16)side->textureoffset
			<< (vuint16)side->rowoffset;
	}

	for (i = 0; i < GLevel->NumPolyObjs; i++)
	{
		polyobj_t	*po;

		po = &GLevel->PolyObjs[i];
		if (!SV_CheckFatPVS(po->subsector))
			continue;

		if (po->base_x != po->startSpot.x ||
			po->base_y != po->startSpot.y ||
			po->base_angle != po->angle)
			po->changed = true;

		if (!po->changed)
			continue;

		if (!msg.CheckSpace(7))
		{
			GCon->Log(NAME_Dev, "UpdateLevel: poly overflow");
			return;
		}

		msg << (vuint8)svc_poly_update
			<< (vuint8)i
			<< (vuint16)floor(po->startSpot.x + 0.5)
			<< (vuint16)floor(po->startSpot.y + 0.5)
			<< (vuint8)(AngleToByte(po->angle));
	}

int StartSize = msg.CurSize;
int NumObjs = 0;
	//	First update players
	for (i = 0; i < GMaxEntities; i++)
	{
		if (!sv_mobjs[i])
			continue;
		if (sv_mobjs[i]->GetFlags() & _OF_DelayedDestroy)
			continue;
		if (sv_mobjs[i]->EntityFlags & VEntity::EF_Hidden)
			continue;
		if (!sv_mobjs[i]->EntityFlags & VEntity::EF_IsPlayer)
			continue;
		if (!msg.CheckSpace(29))
		{
			GCon->Log(NAME_Dev, "UpdateLevel: player overflow");
			return;
		}
		SV_UpdateMobj(i, msg);
		NumObjs++;
	}

	//	Then update non-player mobjs in sight
	int starti = sv_player->MobjUpdateStart;
	for (i = 0; i < GMaxEntities; i++)
	{
		int index = (i + starti) % GMaxEntities;
		if (!sv_mobjs[index])
			continue;
		if (sv_mobjs[index]->GetFlags() & _OF_DelayedDestroy)
			continue;
		if (sv_mobjs[index]->EntityFlags & VEntity::EF_Hidden)
			continue;
		if (sv_mobjs[index]->EntityFlags & VEntity::EF_IsPlayer)
			continue;
		if (!SV_CheckFatPVS(sv_mobjs[index]->SubSector))
			continue;
		if (!msg.CheckSpace(29))
		{
			if (sv_player->MobjUpdateStart && show_mobj_overflow)
			{
				GCon->Log(NAME_Dev, "UpdateLevel: mobj overflow 2");
			}
			else if (show_mobj_overflow > 1)
			{
				GCon->Log(NAME_Dev, "UpdateLevel: mobj overflow");
			}
			//	Next update starts here
			sv_player->MobjUpdateStart = index;
if (show_update_stats)
dprintf("Update size %d (%d) for %d, aver %f big %d %d\n", msg.CurSize, msg.CurSize -
		StartSize, NumObjs, float(msg.CurSize - StartSize) / NumObjs, c_bigClass, c_bigState);
			return;
		}
		SV_UpdateMobj(index, msg);
		NumObjs++;
	}
	sv_player->MobjUpdateStart = 0;
if (show_update_stats)
dprintf("Update size %d (%d) for %d, aver %f big %d %d\n", msg.CurSize, msg.CurSize -
		StartSize, NumObjs, float(msg.CurSize - StartSize) / NumObjs, c_bigClass, c_bigState);
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
	VMessage	msg;
	vuint8		buf[4];

	msg.Data = buf;
	msg.MaxSize = sizeof(buf);
	msg.CurSize = 0;

	msg << (vuint8)svc_nop;

	if (client->NetCon->SendUnreliableMessage(&msg) == -1)
		SV_DropClient(true);	// if the message couldn't send, kick off
	client->LastMessage = realtime;
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
	vuint8		buf[4096];
	VMessage	msg(buf, MAX_DATAGRAM);

	if (!netgame)
	{
		//	HACK!!!!!
		//	Make a bigger buffer for single player.
		msg.MaxSize = 4096;
	}
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
			if (realtime - sv_player->LastMessage > 5)
			{
				SV_SendNop(sv_player);
			}
			continue;
		}

		if (!(sv_player->PlayerFlags & VBasePlayer::PF_NeedsUpdate))
			continue;

		msg.Clear();

		msg << (vuint8)svc_time
			<< level.time;

		SV_WriteViewData(*sv_player, msg);

		if (msg.CheckSpace(sv_datagram.CurSize))
			msg << sv_datagram;

		SV_UpdateLevel(msg);

		if (sv_player->NetCon->SendUnreliableMessage(&msg) == -1)
		{
			SV_DropClient(true);
		}
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
	int		i, j;
	int		*Stats;

	for (i = 0; i < svs.max_clients; i++)
	{
		if (!GGameInfo->Players[i])
			continue;

		GGameInfo->Players[i]->Message << sv_reliable;

		if (!(GGameInfo->Players[i]->PlayerFlags & VBasePlayer::PF_Spawned))
			continue;

		Stats = (vint32*)((vuint8*)GGameInfo->Players[i] + sizeof(VBasePlayer));
		for (j = 0; j < num_stats; j++)
		{
			if (Stats[j] == GGameInfo->Players[i]->OldStats[j])
			{
				continue;
			}
			int sval = Stats[j];
			if (sval >= 0 && sval < 256)
			{
				GGameInfo->Players[i]->Message << (vuint8)svc_stats_byte
					<< (vuint8)j << (vuint8)sval;
			}
			else if (sval >= MINSHORT && sval <= MAXSHORT)
			{
				GGameInfo->Players[i]->Message << (vuint8)svc_stats_short
					<< (vuint8)j << (vint16)sval;
			}
			else
			{
				GGameInfo->Players[i]->Message << (vuint8)svc_stats_long
					<< (vuint8)j << sval;
			}
			GGameInfo->Players[i]->OldStats[j] = sval;
		}
		VStr* StrStats = (VStr*)(Stats + num_stats);
		VStr* OldStrStats = (VStr*)(GGameInfo->Players[i]->OldStats + num_stats);
		for (j = 0; j < num_string_stats; j++)
		{
			if (StrStats[j] == OldStrStats[j])
			{
				continue;
			}
			GGameInfo->Players[i]->Message << (vuint8)svc_stats_string
				<< (vuint8)(num_stats + j) << StrStats[j];
			OldStrStats[j] = StrStats[j];
		}
	}

	sv_reliable.Clear();

	for (i = 0; i < svs.max_clients; i++)
	{
		if (!GGameInfo->Players[i])
		{
			continue;
		}

		if (GGameInfo->Players[i]->Message.Overflowed)
		{
			SV_DropClient(true);
			GCon->Log(NAME_Dev, "Client message overflowed");
			continue;
		}

		if (!GGameInfo->Players[i]->Message.CurSize)
		{
			continue;
		}

		if (!GGameInfo->Players[i]->NetCon->CanSendMessage())
		{
			continue;
		}

		if (GGameInfo->Players[i]->NetCon->SendMessage(
			&GGameInfo->Players[i]->Message) == -1)
		{
			SV_DropClient(true);
			continue;
		}
		GGameInfo->Players[i]->Message.Clear();
		GGameInfo->Players[i]->LastMessage = realtime;
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
	static boolean	triedToSkip;
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
		sv_reliable << (vuint8)svc_skip_intermission;
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
	int			i;

	// get commands
	for (i = 0; i < MAXPLAYERS; i++)
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

		if (!SV_ReadClientMessages(i))
		{
			SV_DropClient(true);
			continue;
		}

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
	sv_datagram << (vuint8)svc_force_lightning;
}

//==========================================================================
//
//	SV_SetLineTexture
//
//==========================================================================

void SV_SetLineTexture(int side, int position, int texture)
{
	guard(SV_SetLineTexture);
	if (position == TEXTURE_MIDDLE)
	{
		GLevel->Sides[side].midtexture = texture;
		sv_reliable << (vuint8)svc_side_mid
					<< (vuint16)side
					<< (vuint16)GLevel->Sides[side].midtexture;
	}
	else if (position == TEXTURE_BOTTOM)
	{
		GLevel->Sides[side].bottomtexture = texture;
		sv_reliable << (vuint8)svc_side_bot
					<< (vuint16)side
					<< (vuint16)GLevel->Sides[side].bottomtexture;
	}
	else
	{ // TEXTURE_TOP
		GLevel->Sides[side].toptexture = texture;
		sv_reliable << (vuint8)svc_side_top
					<< (vuint16)side
					<< (vuint16)GLevel->Sides[side].toptexture;
	}
	unguard;
}

//==========================================================================
//
//	SV_SetLineTransluc
//
//==========================================================================

void SV_SetLineTransluc(line_t *line, int trans)
{
	guard(SV_SetLineTransluc);
	line->translucency = trans;
	sv_signon	<< (vuint8)svc_line_transluc
				<< (vint16)(line - GLevel->Lines)
				<< (vuint8)trans;
	unguard;
}

//==========================================================================
//
//	SV_SetFloorPic
//
//==========================================================================

void SV_SetFloorPic(int i, int texture)
{
	guard(SV_SetFloorPic);
	GLevel->Sectors[i].floor.pic = texture;
	sv_reliable << (vuint8)svc_sec_floor
				<< (vuint16)i
				<< (vuint16)GLevel->Sectors[i].floor.pic;
	unguard;
}

//==========================================================================
//
//	SV_SetCeilPic
//
//==========================================================================

void SV_SetCeilPic(int i, int texture)
{
	guard(SV_SetCeilPic);
	GLevel->Sectors[i].ceiling.pic = texture;
	sv_reliable << (vuint8)svc_sec_ceil
				<< (vuint16)i
				<< (vuint16)GLevel->Sectors[i].ceiling.pic;
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
	level.sky1Texture = GTextureManager.NumForName(VName(Sky1,
		VName::AddLower8), TEXTYPE_Wall, true, false);
	level.sky2Texture = GTextureManager.NumForName(VName(Sky2,
		VName::AddLower8), TEXTYPE_Wall, true, false);
	sv_reliable << (vuint8)svc_change_sky
				<< (vuint16)level.sky1Texture
				<< (vuint16)level.sky2Texture;
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
	level.SongLump = VName(SongName, VName::AddLower8);
	sv_reliable << (vuint8)svc_change_music
				<< *level.SongLump
				<< (vuint8)level.cdTrack;
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
	player->Message << (vuint8)svc_change_music
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
	if (!netgame && (!GGameInfo->Players[0] || !(GGameInfo->Players[0]->PlayerFlags & VBasePlayer::PF_Spawned)))
	{
		//FIXME Some ACS left from previous visit of the level
		return;
	}
	sv.intermission = 1;
	sv.intertime = 0;

	const mapInfo_t& old_info = P_GetMapInfo(level.MapName);
	const mapInfo_t& new_info = P_GetMapInfo(sv_next_map);

	for (i = 0; i < MAXPLAYERS; i++)
	{
		if (GGameInfo->Players[i])
		{
			GGameInfo->Players[i]->eventPlayerExitMap(!old_info.Cluster ||
				old_info.Cluster != new_info.Cluster);
		}
	}

	sv_reliable << (vuint8)svc_intermission
				<< *sv_next_map;
	for (i = 0; i < MAXPLAYERS; i++)
	{
		if (GGameInfo->Players[i])
		{
			sv_reliable << (vuint8)true;
			for (j = 0; j < MAXPLAYERS; j++)
				sv_reliable << (vuint8)GGameInfo->Players[i]->FragsStats[j];
			sv_reliable << (vint16)GGameInfo->Players[i]->KillCount
						<< (vint16)GGameInfo->Players[i]->ItemCount
						<< (vint16)GGameInfo->Players[i]->SecretCount;
		}
		else
		{
			sv_reliable << (vuint8)false;
			for (j = 0; j < MAXPLAYERS; j++)
				sv_reliable << (vuint8)0;
			sv_reliable << (vint16)0
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

	if (in_secret)
	{
		sv_next_map = mapaftersecret;
	}
	in_secret = false;
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
	if (sv_secret_map == NAME_None)
	{
		// No secret map, use normal exit
		G_ExitLevel(Position);
		return;
	}

	if (!in_secret)
	{
		mapaftersecret = sv_next_map;
	}
	LeavePosition = Position;
	completed = true;

	sv_next_map = sv_secret_map; 	// go to secret level

	in_secret = true;
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
			sv_reliable << (vuint8)svc_finale <<
				(VStr(sv_next_map).StartsWith("EndGame") ? *sv_next_map : "");
			sv.intermission = 2;
			return;
		}
		Map = 1;
		Position = 0;
	}
	sv_next_map = P_GetMapNameByLevelNum(Map);

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
		sv_next_map = VName(*Args[1], VName::AddLower8);
		LeavePosition = atoi(*Args[2]);
	}
	else if (sv.intermission != 1)
	{
		return;
	}

	if (!deathmatch)
	{
		if (VStr(sv_next_map).StartsWith("EndGame"))
		{
			sv_reliable << (vuint8)svc_finale << *sv_next_map;
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

int NET_SendToAll(VMessage* data, int blocktime)
{
	guard(NET_SendToAll);
	double		start;
	int			i;
	int			count = 0;
	bool		state1[MAXPLAYERS];
	bool		state2[MAXPLAYERS];

	for (i = 0; i < svs.max_clients; i++)
	{
		sv_player = GGameInfo->Players[i];
		if (sv_player && sv_player->NetCon)
		{
			if (sv_player->NetCon->IsLocalConnection())
			{
				sv_player->NetCon->SendMessage(data);
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
				if (sv_player->NetCon->CanSendMessage())
				{
					state1[i] = true;
					sv_player->NetCon->SendMessage(data);
				}
				else
				{
					sv_player->NetCon->GetMessage();
				}
				count++;
				continue;
			}

			if (!state2[i])
			{
				if (sv_player->NetCon->CanSendMessage())
				{
					state2[i] = true;
				}
				else
				{
					sv_player->NetCon->GetMessage();
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
	nummodels = VClass::GModelNames.Num();
	for (int i = 1; i < nummodels; i++)
	{
		models[i] = VClass::GModelNames[i];
	}

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
	sv_reliable << (vuint8)svc_model
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
	sv_reliable << (vuint8)svc_model
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
	sv_reliable << (vuint8)svc_skin
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
	int			i;
	VMessage	&msg = player->Message;

	msg << (vuint8)svc_server_info
		<< (vuint8)PROTOCOL_VERSION
		<< svs.serverinfo
		<< *level.MapName
		<< level.LevelName
		<< (vuint8)SV_GetPlayerNum(player)
		<< (vuint8)svs.max_clients
		<< (vuint8)deathmatch
		<< level.totalkills
		<< level.totalitems
		<< level.totalsecret
		<< (vuint16)level.sky1Texture
		<< (vuint16)level.sky2Texture
		<< level.sky1ScrollDelta
		<< level.sky2ScrollDelta
		<< (vuint8)level.doubleSky
		<< (vuint8)level.lightning
		<< *level.SkyBox
		<< *level.FadeTable
		<< *level.SongLump
		<< (vuint8)level.cdTrack;

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

	for (i = VClass::GModelNames.Num(); i < nummodels; i++)
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
			GGameInfo->Players[i]->Message.Clear();
		}
	}
	else if (!sv_loading)
	{
		//	New game
		in_secret = false;
	}

	SV_Clear();
	VCvar::Unlatch();

	sv.active = true;

	level.MapName = VName(mapname, VName::AddLower8);

	const mapInfo_t& info = P_GetMapInfo(level.MapName);
	sv_next_map = info.NextMap;
	sv_secret_map = info.SecretMap;

	level.LevelName = info.GetName();
	level.levelnum = info.LevelNum;
	level.cluster = info.Cluster;
	level.partime = 0;//FIXME not used in Vavoom.

	level.sky1Texture = info.Sky1Texture;
	level.sky2Texture = info.Sky2Texture;
	level.sky1ScrollDelta = info.Sky1ScrollDelta;
	level.sky2ScrollDelta = info.Sky2ScrollDelta;
	level.doubleSky = !!(info.Flags & MAPINFOF_DoubleSky);
	level.lightning = !!(info.Flags & MAPINFOF_Lightning);
	level.SkyBox = info.SkyBox;
	level.FadeTable = info.FadeTable;

	level.cdTrack = info.CDTrack;
	level.SongLump = info.SongLump;

	netgame = svs.max_clients > 1;
	deathmatch = DeathMatch;

	GGameInfo->gameskill = gameskill;
	GGameInfo->netgame = netgame;
	GGameInfo->deathmatch = deathmatch;

	//	Load it
	SV_LoadLevel(level.MapName);

	//	Copy special action flags from mapinfo.
	if (info.Flags & MAPINFOF_Map07Special)
		GLevel->LevelFlags |= VLevel::LF_Map07Special;
	if (info.Flags & MAPINFOF_BaronSpecial)
		GLevel->LevelFlags |= VLevel::LF_BaronSpecial;
	if (info.Flags & MAPINFOF_CyberDemonSpecial)
		GLevel->LevelFlags |= VLevel::LF_CyberDemonSpecial;
	if (info.Flags & MAPINFOF_SpiderMastermindSpecial)
		GLevel->LevelFlags |= VLevel::LF_SpiderMastermindSpecial;
	if (info.Flags & MAPINFOF_MinotaurSpecial)
		GLevel->LevelFlags |= VLevel::LF_MinotaurSpecial;
	if (info.Flags & MAPINFOF_DSparilSpecial)
		GLevel->LevelFlags |= VLevel::LF_DSparilSpecial;
	if (info.Flags & MAPINFOF_IronLichSpecial)
		GLevel->LevelFlags |= VLevel::LF_IronLichSpecial;
	if (info.Flags & MAPINFOF_SpecialActionOpenDoor)
		GLevel->LevelFlags |= VLevel::LF_SpecialActionOpenDoor;
	if (info.Flags & MAPINFOF_SpecialActionLowerFloor)
		GLevel->LevelFlags |= VLevel::LF_SpecialActionLowerFloor;
	if (info.Flags & MAPINFOF_SpecialActionKillMonsters)
		GLevel->LevelFlags |= VLevel::LF_SpecialActionKillMonsters;

	//	Spawn slopes, extra floors, etc.
	GGameInfo->eventSpawnWorld(GLevel);

	P_InitThinkers();

	if (spawn_thinkers)
	{
		GLevelInfo = GGameInfo->eventCreateLevelInfo();
		GLevelInfo->Level = GLevelInfo;
		if (info.Gravity)
			GLevelInfo->Gravity = info.Gravity * DEFAULT_GRAVITY / 800.0;
		else
			GLevelInfo->Gravity = sv_gravity * DEFAULT_GRAVITY / 800.0;
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

	//	Init buttons
	P_ClearButtons();

	//
	//	P_SpawnSpecials
	//	After the map has been loaded, scan for specials that spawn thinkers
	//
	if (spawn_thinkers)
	{
		GLevelInfo->eventSpawnSpecials();
	}

	SV_InitModelLists();

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
	for (VThinker* Th = GLevel->ThinkerHead; Th; Th = Th->Next)
	{
		if (Th->IsA(VEntity::StaticClass()) &&
			!(Th->GetFlags() & _OF_DelayedDestroy))
		{
			((VEntity*)Th)->eventBeginPlay();
		}
	}
	GLevelInfo->LevelInfoFlags |= VLevelInfo::LIF_BegunPlay;

	//	Start open scripts.
	P_StartTypedACScripts(SCRIPT_Open);

	P_Ticker();
	P_Ticker();
	SV_CreateBaseline();

	GCon->Log(NAME_Dev, "Server spawned");
	unguard;
}

//==========================================================================
//
//	SV_WriteChangedTextures
//
//	Writes texture change commands for new clients
//
//==========================================================================

static void SV_WriteChangedTextures(VMessage &msg)
{
	int			i;

	for (i = 0; i < GLevel->NumSides; i++)
	{
		side_t &s = GLevel->Sides[i];
		if (s.midtexture != s.base_midtexture)
		{
			msg << (vuint8)svc_side_mid
				<< (vuint16)i
				<< (vuint16)s.midtexture;
		}
		if (s.bottomtexture != s.base_bottomtexture)
		{
			msg << (vuint8)svc_side_bot
				<< (vuint16)i
				<< (vuint16)s.bottomtexture;
		}
		if (s.toptexture != s.base_toptexture)
		{
			msg << (vuint8)svc_side_top
				<< (vuint16)i
				<< (vuint16)s.toptexture;
		}
	}

	for (i = 0; i < GLevel->NumSectors; i++)
	{
		sector_t &s = GLevel->Sectors[i];
		if (s.floor.pic != s.floor.base_pic)
		{
			msg << (vuint8)svc_sec_floor
				<< (vuint16)i
				<< (vuint16)s.floor.pic;
		}
		if (s.ceiling.pic != s.ceiling.base_pic)
		{
			msg << (vuint8)svc_sec_ceil
				<< (vuint16)i
				<< (vuint16)s.ceiling.pic;
		}
	}
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

	sv_player->Message << sv_signon;
	sv_player->Message << (vuint8)svc_signonnum << (vuint8)2;
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
			sv_player->Message << (vuint8)svc_start_seq
				<< (vuint16)sv_ActiveSequences[i].OriginId
				<< (vuint16)sv_ActiveSequences[i].Origin.x
				<< (vuint16)sv_ActiveSequences[i].Origin.y
				<< (vuint16)sv_ActiveSequences[i].Origin.z
				<< *sv_ActiveSequences[i].Name
				<< (vuint8)sv_ActiveSequences[i].ModeNum;
			for (int j = 0; j < sv_ActiveSequences[i].Choices.Num(); j++)
			{
				sv_player->Message << (vuint8)svc_add_seq_choice
					<< (vuint16)sv_ActiveSequences[i].OriginId
					<< *sv_ActiveSequences[i].Choices[j];
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
	SV_WriteChangedTextures(sv_player->Message);
	sv_player->Message << (vuint8)svc_set_angles
						<< (vuint8)(AngleToByte(sv_player->ViewAngles.pitch))
						<< (vuint8)(AngleToByte(sv_player->ViewAngles.yaw))
						<< (vuint8)0;
	sv_player->Message << (vuint8)svc_signonnum << (vuint8)3;
	sv_player->PlayerFlags &= ~VBasePlayer::PF_FixAngle;
	memset(sv_player->OldStats, 0, num_stats * 4);
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
	if (sv_player->NetCon)
	{
		sv_player->NetCon->Close();
	}
	sv_player->NetCon = NULL;
	svs.num_connected--;
	sv_player->UserInfo = VStr();
	sv_reliable << (vuint8)svc_userinfo
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

void SV_ShutdownServer(boolean crash)
{
	guard(SV_ShutdownServer);
	vuint8		buf[128];
	VMessage	msg(buf, 128);
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
	for (i = 0; i < MAXPLAYERS; i++)
	{
		//	Save old stats pointer
		int* OldStats = GPlayersBase[i]->OldStats;
		GPlayersBase[i]->GetClass()->DestructObject(GPlayersBase[i]);
		memset((vuint8*)GPlayersBase[i] + sizeof(VObject), 0,
			GPlayersBase[i]->GetClass()->ClassSize - sizeof(VObject));
		//	Restore pointer
		GPlayersBase[i]->OldStats = OldStats;
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
		SV_SpawnServer(*level.MapName, true);
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
	sv_reliable << (vuint8)svc_pause << (vuint8)paused;
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

	SV_ClientPrintf(sv_player, "Kills: %d of %d", sv_player->KillCount, level.totalkills);
	SV_ClientPrintf(sv_player, "Items: %d of %d", sv_player->ItemCount, level.totalitems);
	SV_ClientPrintf(sv_player, "Secrets: %d of %d", sv_player->SecretCount, level.totalsecret);
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
	GCon->Logf(NAME_Dev, "Client %s connected", *player->NetCon->Address);

	GGameInfo->Players[SV_GetPlayerNum(player)] = player;
	player->PlayerFlags |= VBasePlayer::PF_Active;

	player->Message.Data = player->MsgBuf;
	player->Message.MaxSize = MAX_MSGLEN;
	player->Message.CurSize = 0;
	player->Message.AllowOverflow = true;		// we can catch it
	player->Message.Overflowed = false;
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

		GPlayersBase[i]->NetCon = sock;
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

	GPlayersBase[i]->NetCon = sock;
	GPlayersBase[i]->PlayerFlags |= VBasePlayer::PF_IsBot;
	GPlayersBase[i]->PlayerName = name;
	SV_ConnectClient(GPlayersBase[i]);
	svs.num_connected++;

	sv_player = GGameInfo->Players[i];
	sv_player->Message.Clear();
	SV_RunClientCommand("PreSpawn\n");
	sv_player->Message.Clear();
	SV_SetUserInfo(sv_player->UserInfo);
	SV_RunClientCommand("Spawn\n");
	sv_player->Message.Clear();
	SV_RunClientCommand("Begin\n");
	sv_player->Message.Clear();
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
	P_ACSInitNewGame();
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
		SV_MapTeleport(sv_next_map);
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

void FOutputDevice::Log(const char* S)
{
	Serialise(S, NAME_Log);
}
void FOutputDevice::Log(EName Type, const char* S)
{
	Serialise(S, Type);
}
void FOutputDevice::Log(const VStr& S)
{
	Serialise(*S, NAME_Log);
}
void FOutputDevice::Log(EName Type, const VStr& S)
{
	Serialise(*S, Type);
}
void FOutputDevice::Logf(const char* Fmt, ...)
{
	va_list argptr;
	char string[1024];
	
	va_start(argptr, Fmt);
	vsprintf(string, Fmt, argptr);
	va_end(argptr);

	Serialise(string, NAME_Log);
}
void FOutputDevice::Logf(EName Type, const char* Fmt, ...)
{
	va_list argptr;
	char string[1024];
	
	va_start(argptr, Fmt);
	vsprintf(string, Fmt, argptr);
	va_end(argptr);

	Serialise(string, Type);
}

#endif
