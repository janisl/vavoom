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

// HEADER FILES ------------------------------------------------------------

#include "gamedefs.h"
#include "sv_local.h"

// MACROS ------------------------------------------------------------------

#define TOCENTER 		-128

#define TEXTURE_TOP		0
#define TEXTURE_MIDDLE	1
#define TEXTURE_BOTTOM	2

#define REBORN_DESCRIPTION	"TEMP GAME"

#define MAX_MODELS		512
#define MAX_SPRITES		512
#define MAX_SKINS		256

// TYPES -------------------------------------------------------------------

// EXTERNAL FUNCTION PROTOTYPES --------------------------------------------

void Draw_TeleportIcon(void);
void CL_Disconnect(void);
void SV_MapTeleport(char *mapname);
bool SV_ReadClientMessages(int i);
void SV_DestroyAllThinkers(void);
void SV_RunClientCommand(const char *cmd);
void EntInit(void);

// PUBLIC FUNCTION PROTOTYPES ----------------------------------------------

void SV_DropClient(boolean crash);

// PRIVATE FUNCTION PROTOTYPES ---------------------------------------------

static void G_DoReborn(int playernum);
static void G_DoCompleted(void);

// EXTERNAL DATA DECLARATIONS ----------------------------------------------

// PUBLIC DATA DEFINITIONS -------------------------------------------------

IMPLEMENT_CLASS(VViewEntity)

TCvarI			real_time("real_time", "1");

server_t		sv;
server_static_t	svs;
TProgs			svpr;
server_vars_t	svvars;

// increment every time a check is made
int				validcount = 1;

bool			sv_loading = false;

VBasePlayer*	GPlayersBase[MAXPLAYERS];

skill_t         gameskill; 
 
boolean         paused;

boolean         deathmatch = false;   	// only if started as net death
boolean         netgame;                // only true if packets are broadcast

VEntity**		sv_mobjs;
mobj_base_t*	sv_mo_base;
double*			sv_mo_free_time;

byte			sv_reliable_buf[MAX_MSGLEN];
TMessage		sv_reliable(sv_reliable_buf, MAX_MSGLEN);
byte			sv_datagram_buf[MAX_DATAGRAM];
TMessage		sv_datagram(sv_datagram_buf, MAX_DATAGRAM);
byte			sv_signon_buf[MAX_MSGLEN];
TMessage		sv_signon(sv_signon_buf, MAX_MSGLEN);

VBasePlayer*	sv_player;

char			sv_next_map[12];
char			sv_secret_map[12];

int 			TimerGame;

boolean			in_secret;
char			mapaftersecret[12];

// PRIVATE DATA DEFINITIONS ------------------------------------------------

static int 		LeavePosition;

static int		RebornPosition;	// Position indicator for cooperative net-play reborn

static bool		completed;

static int		num_stats;

static FFunction *pf_PlayerThink;

static TCvarI	TimeLimit("TimeLimit", "0");
static TCvarI	DeathMatch("DeathMatch", "0", CVAR_SERVERINFO);
static TCvarI  	NoMonsters("NoMonsters", "0");
static TCvarI	Skill("Skill", "2");

static TCvarI	sv_cheats("sv_cheats", "0", CVAR_SERVERINFO | CVAR_LATCH);

static byte		*fatpvs;
static TCvarI	show_mobj_overflow("show_mobj_overflow", "0", CVAR_ARCHIVE);

static bool		mapteleport_issued;

static int		numsprites;
static char		sprites[MAX_SPRITES][MAX_VPATH];
static int		nummodels;
static char		models[MAX_MODELS][MAX_VPATH];
static int		numskins;
static char		skins[MAX_SKINS][MAX_VPATH];

static int		pg_frametime;

// CODE --------------------------------------------------------------------

//==========================================================================
//
//	SV_Init
//
//==========================================================================

void SV_Init(void)
{
	guard(SV_Init);
	int		i;

	svs.max_clients = 1;

	sv_mobjs = Z_CNew<VEntity *>(GMaxEntities);
	sv_mo_base = Z_CNew<mobj_base_t>(GMaxEntities);
	sv_mo_free_time = Z_CNew<double>(GMaxEntities);

	svpr.Load("svprogs");

	VClass* PlayerClass = VClass::FindClass("Player");
	for (i = 0; i < MAXPLAYERS; i++)
	{
		GPlayersBase[i] = (VBasePlayer*)VObject::StaticSpawnObject(
			PlayerClass, NULL, PU_STATIC);
	}

	svpr.SetGlobal("svvars", (int)&svvars);
	svpr.SetGlobal("validcount", (int)&validcount);
	svpr.SetGlobal("level", (int)&level);
	svpr.SetGlobal("skyflatnum", skyflatnum);
	EntInit();

	pg_frametime = svpr.GlobalNumForName("frametime");

	num_stats = svpr.GetGlobal("num_stats");
	if (num_stats > 96)
		Sys_Error("Too many stats %d", num_stats);
	pf_PlayerThink = svpr.FuncForName("PlayerThink");

	P_InitSwitchList();
	P_InitTerrainTypes();
	unguard;
}

//==========================================================================
//
//	SV_Clear
//
//==========================================================================

void SV_Clear(void)
{
	guard(SV_Clear);
	SV_DestroyAllThinkers();
	memset(&sv, 0, sizeof(sv));
	memset(&level, 0, sizeof(level));
	memset(sv_mobjs, 0, sizeof(VEntity *) * GMaxEntities);
	memset(sv_mo_base, 0, sizeof(mobj_base_t) * GMaxEntities);
	memset(sv_mo_free_time, 0, sizeof(double) * GMaxEntities);
	sv_signon.Clear();
	sv_reliable.Clear();
	sv_datagram.Clear();
#ifdef CLIENT
	// Make sure all sounds are stopped before Z_FreeTags.
	S_StopAllSound();
#endif
	Z_FreeTag(PU_LEVEL);
	unguard;
}

//==========================================================================
//
//	SV_ClearDatagram
//
//==========================================================================

void SV_ClearDatagram(void)
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

	Ent = (VEntity*)VObject::StaticSpawnObject(Class, NULL, PU_LEVSPEC);

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
	return Ent;
	unguard;
}

//==========================================================================
//
//	SV_GetMobjBits
//
//==========================================================================

int	SV_GetMobjBits(VEntity &mobj, mobj_base_t &base)
{
	guard(SV_GetMobjBits);
	int		bits = 0;

	if (fabs(base.Origin.x - mobj.Origin.x) >= 1.0)
		bits |=	MOB_X;
	if (fabs(base.Origin.y - mobj.Origin.y) >= 1.0)
		bits |=	MOB_Y;
	if (fabs(base.Origin.z - (mobj.Origin.z - mobj.FloorClip)) >= 1.0)
		bits |=	MOB_Z;
//	if (fabs(base.Angles.yaw - mobj.Angles.yaw) >= 1.0)
	if (AngleToByte(base.Angles.yaw) != AngleToByte(mobj.Angles.yaw))
		bits |=	MOB_ANGLE;
//	if (fabs(base.Angles.pitch - mobj.Angles.pitch) >= 1.0)
	if (AngleToByte(base.Angles.pitch) != AngleToByte(mobj.Angles.pitch))
		bits |=	MOB_ANGLEP;
//	if (fabs(base.Angles.roll - mobj.Angles.roll) >= 1.0)
	if (AngleToByte(base.Angles.roll) != AngleToByte(mobj.Angles.roll))
		bits |=	MOB_ANGLER;
	if (base.SpriteIndex != mobj.SpriteIndex || base.SpriteType != mobj.SpriteType)
		bits |=	MOB_SPRITE;
	if (base.SpriteFrame != mobj.SpriteFrame)
		bits |=	MOB_FRAME;
	if (base.Translucency != mobj.Translucency)
		bits |=	MOB_TRANSLUC;
	if (base.Translation != mobj.Translation)
		bits |=	MOB_TRANSL;
	if (base.Effects != mobj.Effects)
		bits |= MOB_EFFECTS;
	if (base.ModelIndex != mobj.ModelIndex)
		bits |= MOB_MODEL;
	if (mobj.ModelIndex && mobj.ModelSkinNum)
		bits |= MOB_SKIN;
	if (mobj.ModelIndex && base.ModelFrame != mobj.ModelFrame)
		bits |= MOB_FRAME;

	return bits;
	unguard;
}

//==========================================================================
//
//	SV_WriteMobj
//
//==========================================================================

void SV_WriteMobj(int bits, VEntity &mobj, TMessage &msg)
{
	guard(SV_WriteMobj);
	if (bits & MOB_X)
		msg << (word)mobj.Origin.x;
	if (bits & MOB_Y)
		msg << (word)mobj.Origin.y;
	if (bits & MOB_Z)
		msg << (word)(mobj.Origin.z - mobj.FloorClip);
	if (bits & MOB_ANGLE)
		msg << (byte)(AngleToByte(mobj.Angles.yaw));
	if (bits & MOB_ANGLEP)
		msg << (byte)(AngleToByte(mobj.Angles.pitch));
	if (bits & MOB_ANGLER)
		msg << (byte)(AngleToByte(mobj.Angles.roll));
	if (bits & MOB_SPRITE)
		msg << (word)(mobj.SpriteIndex | (mobj.SpriteType << 10));
	if (bits & MOB_FRAME)
		msg << (byte)mobj.SpriteFrame;
	if (bits & MOB_TRANSLUC)
		msg << (byte)mobj.Translucency;
	if (bits & MOB_TRANSL)
		msg << (byte)mobj.Translation;
	if (bits & MOB_EFFECTS)
		msg << (byte)mobj.Effects;
	if (bits & MOB_MODEL)
		msg << (word)mobj.ModelIndex;
	if (bits & MOB_SKIN)
		msg << (byte)mobj.ModelSkinNum;
	if (mobj.ModelIndex && (bits & MOB_FRAME))
		msg << (byte)mobj.ModelFrame;
	if (bits & MOB_WEAPON)
		msg << (word)mobj.Player->WeaponModel;
	unguard;
}

//==========================================================================
//
//	VEntity::Destroy
//
//==========================================================================

void VEntity::Destroy(void)
{
	guard(VEntity::Destroy);
	if (sv_mobjs[NetID] != this)
		Sys_Error("Invalid entity num %d", NetID);

	eventDestroyed();

	// unlink from sector and block lists
	UnlinkFromWorld();

	// stop any playing sound
	SV_StopSound(this, 0);

	sv_mobjs[NetID] = NULL;
	sv_mo_free_time[NetID] = level.time;

	Super::Destroy();
	unguard;
}

//==========================================================================
//
//	SV_CreateBaseline
//
//==========================================================================

void SV_CreateBaseline(void)
{
	guard(SV_CreateBaseline);
	int		i;

	for (i = 0; i < GLevel->NumSectors; i++)
	{
		sector_t &sec = GLevel->Sectors[i];
		if (sec.floor.translucency)
		{
			sv_signon << (byte)svc_sec_transluc
					<< (word)i
					<< (byte)sec.floor.translucency;
		}
	}

	for (i = 0; i < GMaxEntities; i++)
	{
		if (!sv_mobjs[i])
			continue;
		if (sv_mobjs[i]->bHidden)
			continue;

		if (!sv_signon.CheckSpace(32))
		{
			GCon->Log(NAME_Dev, "SV_CreateBaseline: Overflow");
			return;
		}

		VEntity &mobj = *sv_mobjs[i];
		mobj_base_t &base = sv_mo_base[i];

		base.Origin.x = mobj.Origin.x;
		base.Origin.y = mobj.Origin.y;
		base.Origin.z = mobj.Origin.z - mobj.FloorClip;
		base.Angles.yaw = mobj.Angles.yaw;
		base.Angles.pitch = mobj.Angles.pitch;
		base.Angles.roll = mobj.Angles.roll;
		base.SpriteType = mobj.SpriteType;
		base.SpriteIndex = mobj.SpriteIndex;
		base.SpriteFrame = mobj.SpriteFrame;
		base.Translucency = mobj.Translucency;
		base.Translation = mobj.Translation;
		base.Effects = mobj.Effects;
		base.ModelIndex = mobj.ModelIndex;
		base.ModelFrame = mobj.ModelFrame;

		sv_signon << (byte)svc_spawn_baseline
					<< (word)i
					<< (word)mobj.Origin.x
					<< (word)mobj.Origin.y
					<< (word)(mobj.Origin.z - mobj.FloorClip)
					<< (byte)(AngleToByte(mobj.Angles.yaw))
					<< (byte)(AngleToByte(mobj.Angles.pitch))
					<< (byte)(AngleToByte(mobj.Angles.roll))
					<< (word)(mobj.SpriteIndex | (mobj.SpriteType << 10))
					<< (word)mobj.SpriteFrame
					<< (byte)mobj.Translucency
					<< (byte)mobj.Translation
					<< (byte)mobj.Effects
					<< (word)mobj.ModelIndex
					<< (byte)mobj.ModelFrame;
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

	if (mobj->bIsPlayer)
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
	int channel, int volume)
{
	guard(SV_StartSound);
	if (!sv_datagram.CheckSpace(12))
		return;

	sv_datagram << (byte)svc_start_sound
				<< (word)sound_id
				<< (word)(origin_id | (channel << 13));
	if (origin_id)
	{
		sv_datagram << (word)origin.x
					<< (word)origin.y
					<< (word)origin.z;
	}
	sv_datagram << (byte)volume;
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

	sv_datagram << (byte)svc_stop_sound
				<< (word)(origin_id | (channel << 13));
	unguard;
}

//==========================================================================
//
//	SV_StartSound
//
//==========================================================================

void SV_StartSound(const VEntity * origin, int sound_id, int channel,
	int volume)
{
	guard(SV_StartSound);
	if (origin)
	{
		SV_StartSound(origin->Origin, GetOriginNum(origin), sound_id,
			channel, volume);
	}
	else
	{
		SV_StartSound(TVec(0, 0, 0), 0, sound_id, channel, volume);
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

void SV_SectorStartSound(const sector_t *sector, int sound_id, int channel,
	int volume)
{
	guard(SV_SectorStartSound);
	if (sector)
	{
		SV_StartSound(sector->soundorg,
			(sector - GLevel->Sectors) + GMaxEntities,
			sound_id, channel, volume);
	}
	else
	{
		SV_StartSound(TVec(0, 0, 0), 0, sound_id, channel, volume);
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

void SV_StartSequence(const TVec &origin, int origin_id, const char *name)
{
	guard(SV_StartSequence);
	sv_reliable << (byte)svc_start_seq
				<< (word)origin_id
				<< (word)origin.x
				<< (word)origin.y
				<< (word)origin.z
				<< name;
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
	sv_reliable << (byte)svc_stop_seq
				<< (word)origin_id;
	unguard;
}

//==========================================================================
//
//	SV_SectorStartSequence
//
//==========================================================================

void SV_SectorStartSequence(const sector_t *sector, const char *name)
{
	guard(SV_SectorStartSequence);
	if (sector)
	{
		SV_StartSequence(sector->soundorg,
			(sector - GLevel->Sectors) + GMaxEntities, name);
	}
	else
	{
		SV_StartSequence(TVec(0, 0, 0), 0, name);
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

void SV_PolyobjStartSequence(const polyobj_t *poly, const char *name)
{
	guard(SV_PolyobjStartSequence);
	SV_StartSequence(poly->startSpot,
		(poly - GLevel->PolyObjs) + GMaxEntities + GLevel->NumSectors, name);
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

	player->Message << (byte)svc_print << buf;
	unguard;
}

//==========================================================================
//
//	SV_ClientCenterPrintf
//
//==========================================================================

void SV_ClientCenterPrintf(VBasePlayer *player, const char *s, ...)
{
	guard(SV_ClientCenterPrintf);
	va_list	v;
	char	buf[1024];

	va_start(v, s);
   	vsprintf(buf, s, v);
	va_end(v);

	player->Message << (byte)svc_center_print << buf;
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
		if (svvars.Players[i])
			svvars.Players[i]->Message << (byte)svc_print << buf;
	unguard;
}

//==========================================================================
//
//	SV_WriteViewData
//
//==========================================================================

void SV_WriteViewData(VBasePlayer &player, TMessage &msg)
{
	guard(SV_WriteViewData);
	int		i;

	msg << (byte)svc_view_data
		<< player.ViewOrg.x
		<< player.ViewOrg.y
		<< player.ViewOrg.z
		<< (byte)player.ExtraLight
		<< (byte)player.FixedColormap
		<< (byte)player.Palette
		<< (byte)player.MO->Translucency
		<< (word)player.PSpriteSY;
	if (player.ViewEnts[0] && player.ViewEnts[0]->StateNum)
	{
		msg << (word)player.ViewEnts[0]->SpriteIndex
			<< (byte)player.ViewEnts[0]->SpriteFrame
			<< (word)player.ViewEnts[0]->ModelIndex
			<< (byte)player.ViewEnts[0]->ModelFrame
			<< (word)player.ViewEnts[0]->SX
			<< (word)player.ViewEnts[0]->SY;
	}
	else
	{
		msg << (short)-1;
	}
	if (player.ViewEnts[1] && player.ViewEnts[1]->StateNum)
	{
		msg << (word)player.ViewEnts[1]->SpriteIndex
			<< (byte)player.ViewEnts[1]->SpriteFrame
			<< (word)player.ViewEnts[1]->ModelIndex
			<< (byte)player.ViewEnts[1]->ModelFrame
			<< (word)player.ViewEnts[1]->SX
			<< (word)player.ViewEnts[1]->SY;
	}
	else
	{
		msg << (short)-1;
	}

	msg << (byte)player.Health
		<< player.Items
		<< (short)player.Frags;

	int bits = 0;
	for (i = 0; i < NUM_CSHIFTS; i++)
		if (player.CShifts[i] & 0xff000000)
			bits |= (1 << i);
	msg << (byte)bits;
	for (i = 0; i < NUM_CSHIFTS; i++)
		if (player.CShifts[i] & 0xff000000)
			msg << player.CShifts[i];

	//	Update bam_angles (after teleportation)
	if (player.bFixAngle)
	{
		player.bFixAngle = false;
		msg << (byte)svc_set_angles
			<< (byte)(AngleToByte(player.ViewAngles.pitch))
			<< (byte)(AngleToByte(player.ViewAngles.yaw))
			<< (byte)(AngleToByte(player.ViewAngles.roll));
	}
	unguard;
}

//==========================================================================
//
//	SV_UpdateMobj
//
//==========================================================================

void SV_UpdateMobj(int i, TMessage &msg)
{
	guard(SV_UpdateMobj);
	int bits;
	int sendnum;

	if (sv_mobjs[i]->bIsPlayer)
	{
		sendnum = SV_GetPlayerNum(sv_mobjs[i]->Player) + 1;
	}
	else
	{
		sendnum = i;
	}

	bits = SV_GetMobjBits(*sv_mobjs[i], sv_mo_base[sendnum]);

	if (sv_mobjs[i]->bIsPlayer)
	{
		//	Clear look angles, because they must not affect model orientation
		bits &= ~(MOB_ANGLEP | MOB_ANGLER);
		if (sv_mobjs[i]->Player->WeaponModel)
		{
			bits |= MOB_WEAPON;
		}
	}
	if (sendnum > 255)
		bits |=	MOB_BIG_NUM;
	if (bits > 255)
		bits |=	MOB_MORE_BITS;

	msg << (byte)svc_update_mobj;
	if (bits & MOB_MORE_BITS)
		msg << (word)bits;
	else
		msg << (byte)bits;
	if (bits & MOB_BIG_NUM)
		msg << (word)sendnum;
	else
		msg << (byte)sendnum;

	SV_WriteMobj(bits, *sv_mobjs[i], msg);
	return;
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

void SV_UpdateLevel(TMessage &msg)
{
	guard(SV_UpdateLevel);
	int		i;
	int		bits;

	fatpvs = GLevel->LeafPVS(sv_player->MO->SubSector);

	for (i = 0; i < GLevel->NumSectors; i++)
	{
		sector_t	*sec;

		sec = &GLevel->Sectors[i];
		if (!SV_SecCheckFatPVS(sec) && !sec->bExtrafloorSource)
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
		msg << (byte)svc_sec_update
			<< (byte)bits;
		if (bits & SUB_BIG_NUM)
			msg << (word)i;
		else
			msg << (byte)i;
		if (bits & SUB_FLOOR)
			msg << (word)(sec->floor.dist);
		if (bits & SUB_CEIL)
			msg << (word)(sec->ceiling.dist);
		if (bits & SUB_LIGHT)
			msg << (byte)(sec->params.lightlevel >> 2);
		if (bits & SUB_FLOOR_X)
			msg << (byte)(sec->floor.xoffs);
		if (bits & SUB_FLOOR_Y)
			msg << (byte)(sec->floor.yoffs);
		if (bits & SUB_CEIL_X)
			msg << (byte)(sec->ceiling.xoffs);
		if (bits & SUB_CEIL_Y)
			msg << (byte)(sec->ceiling.yoffs);
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

		msg << (byte)svc_side_ofs
			<< (word)i
			<< (word)side->textureoffset
			<< (word)side->rowoffset;
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

		msg << (byte)svc_poly_update
			<< (byte)i
			<< (word)po->startSpot.x
			<< (word)po->startSpot.y
			<< (byte)(AngleToByte(po->angle));
	}

	//	First update players
	for (i = 0; i < GMaxEntities; i++)
	{
		if (!sv_mobjs[i])
			continue;
		if (sv_mobjs[i]->bHidden)
			continue;
		if (!sv_mobjs[i]->bIsPlayer)
			continue;
		if (!msg.CheckSpace(25))
		{
			GCon->Log(NAME_Dev, "UpdateLevel: player overflow");
			return;
		}
		SV_UpdateMobj(i, msg);
	}

	//	Then update non-player mobjs in sight
	int starti = sv_player->MobjUpdateStart;
	for (i = 0; i < GMaxEntities; i++)
	{
		int index = (i + starti) % GMaxEntities;
		if (!sv_mobjs[index])
			continue;
		if (sv_mobjs[index]->bHidden)
			continue;
		if (sv_mobjs[index]->bIsPlayer)
			continue;
		if (!SV_CheckFatPVS(sv_mobjs[index]->SubSector))
			continue;
		if (!msg.CheckSpace(25))
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
			return;
		}
		SV_UpdateMobj(index, msg);
	}
	sv_player->MobjUpdateStart = 0;
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
	TMessage	msg;
	byte		buf[4];

	msg.Data = buf;
	msg.MaxSize = sizeof(buf);
	msg.CurSize = 0;

	msg << (byte)svc_nop;

	if (NET_SendUnreliableMessage(client->NetCon, &msg) == -1)
		SV_DropClient(true);	// if the message couldn't send, kick off
	client->LastMessage = realtime;
	unguard;
}

//==========================================================================
//
//	SV_SendClientDatagram
//
//==========================================================================

void SV_SendClientDatagram(void)
{
	guard(SV_SendClientDatagram);
	byte		buf[4096];
	TMessage	msg(buf, MAX_DATAGRAM);

	if (!netgame)
	{
		//	HACK!!!!!
		//	Make a bigger buffer for single player.
		msg.MaxSize = 4096;
	}
	for (int i = 0; i < svs.max_clients; i++)
	{
		if (!svvars.Players[i])
		{
			continue;
		}

		sv_player = svvars.Players[i];

		if (!sv_player->bSpawned)
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

		if (!sv_player->bNeedsUpdate)
			continue;

		msg.Clear();

		msg << (byte)svc_time
			<< level.time;

		SV_WriteViewData(*sv_player, msg);

		if (msg.CheckSpace(sv_datagram.CurSize))
			msg << sv_datagram;

		SV_UpdateLevel(msg);

		if (NET_SendUnreliableMessage(sv_player->NetCon, &msg) == -1)
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

void SV_SendReliable(void)
{
	guard(SV_SendReliable);
	int		i, j;
	int		*Stats;

	for (i = 0; i < svs.max_clients; i++)
	{
		if (!svvars.Players[i])
			continue;

		svvars.Players[i]->Message << sv_reliable;

		if (!svvars.Players[i]->bSpawned)
			continue;

		Stats = (int*)((byte*)svvars.Players[i] + sizeof(VBasePlayer));
		for (j = 0; j < num_stats; j++)
		{
			if (Stats[j] == svvars.Players[i]->OldStats[j])
			{
				continue;
			}
			int sval = Stats[j];
			if (sval >= 0 && sval < 256)
			{
				svvars.Players[i]->Message << (byte)svc_stats_byte
					<< (byte)j << (byte)sval;
			}
			else if (sval >= MINSHORT && sval <= MAXSHORT)
			{
				svvars.Players[i]->Message << (byte)svc_stats_short
					<< (byte)j << (short)sval;
			}
			else
			{
				svvars.Players[i]->Message << (byte)svc_stats_long
					<< (byte)j << sval;
			}
			svvars.Players[i]->OldStats[j] = sval;
		}
	}

	sv_reliable.Clear();

	for (i = 0; i < svs.max_clients; i++)
	{
		if (!svvars.Players[i])
		{
			continue;
		}

		if (svvars.Players[i]->Message.Overflowed)
		{
			SV_DropClient(true);
			GCon->Log(NAME_Dev, "Client message overflowed");
			continue;
		}

		if (!svvars.Players[i]->Message.CurSize)
		{
			continue;
		}

		if (!NET_CanSendMessage(svvars.Players[i]->NetCon))
		{
			continue;
		}

		if (NET_SendMessage(svvars.Players[i]->NetCon, &svvars.Players[i]->Message) == -1)
		{
			SV_DropClient(true);
			continue;
		}
		svvars.Players[i]->Message.Clear();
		svvars.Players[i]->LastMessage = realtime;
	}
	unguard;
}

//==========================================================================
//
//	SV_SendClientMessages
//
//==========================================================================

void SV_SendClientMessages(void)
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

static void CheckForSkip(void)
{
	int   			i;
	VBasePlayer		*player;
	static boolean	triedToSkip;
	bool			skip = false;

	for (i = 0; i < MAXPLAYERS; i++)
	{
		player = svvars.Players[i];
		if (player)
		{
			if (player->Buttons & BT_ATTACK)
			{
				if (!player->bAttackDown)
				{
					skip = true;
				}
				player->bAttackDown = true;
			}
			else
			{
				player->bAttackDown = false;
			}
			if (player->Buttons & BT_USE)
			{
				if (!player->bUseDown)
				{
					skip = true;
				}
				player->bUseDown = true;
			}
			else
			{
				player->bUseDown = false;
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
		sv_reliable << (byte)svc_skip_intermission;
	}
}

//==========================================================================
//
//	SV_RunClients
//
//==========================================================================

void SV_RunClients(void)
{
	guard(SV_RunClients);
	int			i;

	// get commands
	for (i = 0; i < MAXPLAYERS; i++)
	{
		if (!svvars.Players[i])
		{
			continue;
		}

		// do player reborns if needed
		if (svvars.Players[i]->PlayerState == PST_REBORN)
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
		if (svvars.Players[i]->bSpawned && !sv.intermission && !paused &&
			(netgame || !(MN_Active() || C_Active())))
#else
		if (svvars.Players[i]->bSpawned && !sv.intermission && !paused)
#endif
		{
			svpr.Exec(pf_PlayerThink, (int)svvars.Players[i]);
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

void SV_Ticker(void)
{
	guard(SV_Ticker);
	float	saved_frametime;

	saved_frametime = host_frametime;
	if (!real_time)
	{
		// Rounded a little bit up to prevent "slow motion"
		host_frametime = 0.028572f;//1.0 / 35.0;
	}

	svpr.SetGlobal(pg_frametime, PassFloat(host_frametime));
	SV_RunClients();

	if (sv_loading)
		return;

	// do main actions
	if (!sv.intermission)
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
			P_Ticker();
		}
	}

	if (completed)
	{
		G_DoCompleted();
	}

	if (!real_time)
	{
		host_frametime = saved_frametime;
	}
	unguard;
}

//==========================================================================
//
//	SV_ForceLightning
//
//==========================================================================

void SV_ForceLightning(void)
{
	sv_datagram << (byte)svc_force_lightning;
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
		sv_reliable << (byte)svc_side_mid
					<< (word)side
					<< (word)GLevel->Sides[side].midtexture;
	}
	else if (position == TEXTURE_BOTTOM)
	{
		GLevel->Sides[side].bottomtexture = texture;
		sv_reliable << (byte)svc_side_bot
					<< (word)side
					<< (word)GLevel->Sides[side].bottomtexture;
	}
	else
	{ // TEXTURE_TOP
		GLevel->Sides[side].toptexture = texture;
		sv_reliable << (byte)svc_side_top
					<< (word)side
					<< (word)GLevel->Sides[side].toptexture;
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
	sv_signon	<< (byte)svc_line_transluc
				<< (short)(line - GLevel->Lines)
				<< (byte)trans;
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
	sv_reliable << (byte)svc_sec_floor
				<< (word)i
				<< (word)GLevel->Sectors[i].floor.pic;
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
	sv_reliable << (byte)svc_sec_ceil
				<< (word)i
				<< (word)GLevel->Sectors[i].ceiling.pic;
	unguard;
}

//==========================================================================
//
//	G_DoCompleted
//
//==========================================================================

static void G_DoCompleted(void)
{
	int			i;
	int			j;
	mapInfo_t	old_info;
	mapInfo_t	new_info;

	completed = false;
	if (sv.intermission)
	{
		return;
	}
	if (!netgame && (!svvars.Players[0] || !svvars.Players[0]->bSpawned))
	{
		//FIXME Some ACS left from previous visit of the level
		return;
	}
	sv.intermission = 1;
	sv.intertime = 0;

	P_GetMapInfo(level.mapname, old_info);
	P_GetMapInfo(sv_next_map, new_info);

	for (i = 0; i < MAXPLAYERS; i++)
	{
		if (svvars.Players[i])
		{
			svpr.Exec("G_PlayerExitMap", i,
				!old_info.cluster || old_info.cluster != new_info.cluster);
		}
	}

	if (!deathmatch && old_info.cluster &&
		old_info.cluster == new_info.cluster)
	{
		CmdBuf << "TeleportNewMap\n";
		return;
	}

	sv_reliable << (byte)svc_intermission
				<< sv_next_map;
	for (i = 0; i < MAXPLAYERS; i++)
	{
		if (svvars.Players[i])
		{
			sv_reliable << (byte)true;
			for (j = 0; j < MAXPLAYERS; j++)
				sv_reliable << (byte)svvars.Players[i]->FragsStats[j];
			sv_reliable << (short)svvars.Players[i]->KillCount
						<< (short)svvars.Players[i]->ItemCount
						<< (short)svvars.Players[i]->SecretCount;
		}
		else
		{
			sv_reliable << (byte)false;
			for (j = 0; j < MAXPLAYERS; j++)
				sv_reliable << (byte)0;
			sv_reliable << (short)0
						<< (short)0
						<< (short)0;
		}
	}
}

//==========================================================================
//
//	G_ExitLevel
//
//==========================================================================

void G_ExitLevel(void)
{ 
	guard(G_ExitLevel);
	completed = true;
	if (!deathmatch)
	{
		if (!strcmp(level.mapname, "E1M8") ||
			!strcmp(level.mapname, "E2M8") ||
			!strcmp(level.mapname, "E3M8") ||
			!strcmp(level.mapname, "E4M8") ||
			!strcmp(level.mapname, "E5M8"))
		{
			sv_reliable << (byte)svc_finale;
			sv.intermission = 2;
			return;
		}
	}

	if (in_secret)
	{
		strcpy(sv_next_map, mapaftersecret);
	}
	in_secret = false;
	unguard;
}

//==========================================================================
//
//	G_SecretExitLevel
//
//==========================================================================

void G_SecretExitLevel(void)
{
	guard(G_SecretExitLevel);
	if (!sv_secret_map[0])
	{
		// No secret map, use normal exit
		G_ExitLevel();
		return;
	}

	if (!in_secret)
	{
		strcpy(mapaftersecret, sv_next_map);
	}
	completed = true;

	strcpy(sv_next_map, sv_secret_map); 	// go to secret level

	in_secret = true;
	for (int i = 0; i < MAXPLAYERS; i++)
	{
		if (svvars.Players[i])
		{
			svvars.Players[i]->bDidSecret = true;
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

void G_Completed(int map, int position)
{
	guard(G_Completed);
	if (map == -1 && position == -1)
	{
		if (!deathmatch)
		{
			sv_reliable << (byte)svc_finale;
			sv.intermission = 2;
		   	return;
		}
		map = 1;
		position = 0;
	}
	strcpy(sv_next_map, SV_GetMapName(map));

	LeavePosition = position;
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
	if (cmd_source == src_command)
	{
#ifdef CLIENT
		Cmd_ForwardToServer();
#endif
		return;
	}

	if (!sv.active)
	{
		return;
	}

	if (Argc() == 3)
	{
		strcpy(sv_next_map, Argv(1));
		LeavePosition = atoi(Argv(2));
	}
	else if (sv.intermission != 1)
	{
		return;
	}

#ifdef CLIENT
	Draw_TeleportIcon();
#endif
	RebornPosition = LeavePosition;
	svpr.SetGlobal("RebornPosition", RebornPosition);
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
	if (!svvars.Players[playernum] || !svvars.Players[playernum]->bSpawned)
		return;
	if (!netgame && !deathmatch)// For fun now
	{
		CmdBuf << "Restart\n";
		svvars.Players[playernum]->PlayerState = PST_LIVE;
	}
	else
	{
		svpr.Exec("NetGameReborn", playernum);
	}
}

//==========================================================================
//
//	NET_SendToAll
//
//==========================================================================

int NET_SendToAll(TSizeBuf *data, int blocktime)
{
	guard(NET_SendToAll);
	double		start;
	int			i;
	int			count = 0;
	boolean		state1[MAXPLAYERS];
	boolean		state2[MAXPLAYERS];

	for (i = 0; i < svs.max_clients; i++)
	{
		sv_player = svvars.Players[i];
		if (sv_player && sv_player->NetCon)
		{
			if (sv_player->NetCon->driver == 0)
			{
				NET_SendMessage(sv_player->NetCon, data);
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
			sv_player = svvars.Players[i];
			if (!state1[i])
			{
				if (NET_CanSendMessage(sv_player->NetCon))
				{
					state1[i] = true;
					NET_SendMessage(sv_player->NetCon, data);
				}
				else
				{
					NET_GetMessage(sv_player->NetCon);
				}
				count++;
				continue;
			}

			if (!state2[i])
			{
				if (NET_CanSendMessage(sv_player->NetCon))
				{
					state2[i] = true;
				}
				else
				{
					NET_GetMessage(sv_player->NetCon);
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

static void SV_InitModelLists(void)
{
	int i;
	FName* list;

	numsprites = svpr.GetGlobal("num_sprite_names");
	list = (FName*)svpr.GlobalAddr("sprite_names");
	for (i = 0; i < numsprites; i++)
	{
		strcpy(sprites[i], *list[i]);
	}

	nummodels = svpr.GetGlobal("num_models");
	list = (FName*)svpr.GlobalAddr("models");
	for (i = 1; i < nummodels; i++)
	{
		strcpy(models[i], *list[i]);
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
		if (!stricmp(name, models[i]))
		{
			return i;
		}
	}
	strcpy(models[i], name);
	nummodels++;
	sv_reliable << (byte)svc_model
				<< (short)i
				<< name;
	return i;
	unguard;
}

//==========================================================================
//
//	SV_GetModelIndex
//
//==========================================================================

int SV_GetModelIndex(const FName &Name)
{
	guard(SV_GetModelIndex);
	int i;

	if (Name == NAME_None)
	{
		return 0;
	}
	for (i = 0; i < nummodels; i++)
	{
		if (!stricmp(*Name, models[i]))
		{
			return i;
		}
	}
	strcpy(models[i], *Name);
	nummodels++;
	sv_reliable << (byte)svc_model
				<< (short)i
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
		if (!stricmp(name, skins[i]))
		{
			return i;
		}
	}
	strcpy(skins[i], name);
	numskins++;
	sv_reliable << (byte)svc_skin
				<< (byte)i
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
	TMessage	&msg = player->Message;

	msg << (byte)svc_server_info
		<< (byte)PROTOCOL_VERSION
		<< svs.serverinfo
		<< level.mapname
		<< (byte)SV_GetPlayerNum(player)
		<< (byte)svs.max_clients
		<< (byte)deathmatch
		<< level.totalkills
		<< level.totalitems
		<< level.totalsecret;

	for (i = 0; i < MAXPLAYERS; i++)
	{
		msg << (byte)svc_userinfo
			<< (byte)i
			<< (svvars.Players[i] ? svvars.Players[i]->UserInfo : "");
	}

	msg << (byte)svc_sprites
		<< (short)numsprites;
	for (i = 0; i < numsprites; i++)
	{
		msg << sprites[i];
	}

	for (i = 1; i < nummodels; i++)
	{
		msg << (byte)svc_model
			<< (short)i
			<< models[i];
	}

	for (i = 1; i < numskins; i++)
	{
		msg << (byte)svc_skin
			<< (byte)i
			<< skins[i];
	}

	msg << (byte)svc_signonnum
		<< (byte)1;
	unguard;
}

//==========================================================================
//
//	SV_SpawnServer
//
//==========================================================================

void SV_SpawnServer(char *mapname, boolean spawn_thinkers)
{
	guard(SV_SpawnServer);
	int			i;
	mapInfo_t	info;

	GCon->Logf(NAME_Dev, "Spawning server %s", mapname);
	paused = false;
	mapteleport_issued = false;

	if (sv.active)
	{
		//	Level change
		for (i = 0; i < MAXPLAYERS; i++)
		{
			if (!svvars.Players[i])
				continue;

			svvars.Players[i]->KillCount = 0;
			svvars.Players[i]->SecretCount = 0;
			svvars.Players[i]->ItemCount = 0;

			svvars.Players[i]->bSpawned = false;
			svvars.Players[i]->MO = NULL;
			svvars.Players[i]->Frags = 0;
			memset(svvars.Players[i]->FragsStats, 0, sizeof(svvars.Players[i]->FragsStats));
			if (svvars.Players[i]->PlayerState == PST_DEAD)
				svvars.Players[i]->PlayerState = PST_REBORN;
			svvars.Players[i]->Message.Clear();
		}
	}
	else if (!sv_loading)
	{
		//	New game
		in_secret = false;
	}

	SV_Clear();
	TCvar::Unlatch();

	sv.active = true;

	W_CleanupName(mapname, level.mapname);

	P_GetMapInfo(level.mapname, info);
	strcpy(sv_next_map, info.nextMap);
	strcpy(sv_secret_map, info.secretMap);
	memcpy(sv.mapalias, info.mapalias, sizeof(info.mapalias));

	netgame = svs.max_clients > 1;
	deathmatch = DeathMatch;

	svpr.SetGlobal("gameskill", gameskill);
	svpr.SetGlobal("netgame", netgame);
	svpr.SetGlobal("deathmatch", deathmatch);

	//	Prepare to load level
	svpr.Exec("StartLevelLoading");

	//	Load it
	SV_LoadLevel(level.mapname);

	//	Spawn slopes, extra floors, etc.
	svpr.Exec("SpawnWorld");

	P_InitThinkers();
	FFunction *pf_spawn_map_thing = svpr.FuncForName("P_SpawnMapThing");
	for (i = 0; i < GLevel->NumThings; i++)
	{
		svpr.Exec(pf_spawn_map_thing, (int)&GLevel->Things[i], spawn_thinkers);
	}
	Z_Free(GLevel->Things);
	PO_Init(); // Initialize the polyobjs
	P_LoadACScripts(spawn_thinkers);

	if (deathmatch)
	{
		if (level.numdeathmatchstarts < 4)
		{
			Host_Error("Level needs more deathmatch start spots");
		}
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
	svpr.Exec("P_SpawnSpecials", spawn_thinkers);
	svpr.Exec("EndLevelLoading");

	Z_CheckHeap();

	SV_InitModelLists();
	for (i = 0; i < svs.max_clients; i++)
	{
		if (svvars.Players[i])
		{
			SV_SendServerInfo(svvars.Players[i]);
			if (svvars.Players[i]->bIsBot)
			{
				sv_player = svvars.Players[i];
				SV_RunClientCommand("PreSpawn\n");
				SV_RunClientCommand("Spawn\n");
				SV_RunClientCommand("Begin\n");
			}
		}
	}

	if (!spawn_thinkers)
	{
//		if (level.thinkerHead)
//		{
//			Sys_Error("Spawned a thinker when it's not allowed");
//		}
		return;
	}

	P_Ticker();
	P_Ticker();
	SV_CreateBaseline();

	GCon->Log(NAME_Dev, "Server spawned");
	unguard;
}

//==========================================================================
//
//	SV_GetMapName
//
//==========================================================================

const char *SV_GetMapName(int num)
{
	guard(SV_GetMapName);
	//  Check map aliases
	for (int i = 0; i < MAX_MAP_ALIAS; i++)
	{
		if (sv.mapalias[i].num == num)
		{
			return sv.mapalias[i].name;
		}
	}

	//  Use defalt map name in form MAP##
	static char namebuf[12];

	sprintf(namebuf, "MAP%02d", num);
	return namebuf;
	unguard;
}

//==========================================================================
//
//	SV_WriteChangedTextures
//
//	Writes texture change commands for new clients
//
//==========================================================================

static void SV_WriteChangedTextures(TMessage &msg)
{
	int			i;

	for (i = 0; i < GLevel->NumSides; i++)
	{
		side_t &s = GLevel->Sides[i];
		if (s.midtexture != s.base_midtexture)
		{
			msg << (byte)svc_side_mid
				<< (word)i
				<< (word)s.midtexture;
		}
		if (s.bottomtexture != s.base_bottomtexture)
		{
			msg << (byte)svc_side_bot
				<< (word)i
				<< (word)s.bottomtexture;
		}
		if (s.toptexture != s.base_toptexture)
		{
			msg << (byte)svc_side_top
				<< (word)i
				<< (word)s.toptexture;
		}
	}

	for (i = 0; i < GLevel->NumSectors; i++)
	{
		sector_t &s = GLevel->Sectors[i];
		if (s.floor.pic != s.floor.base_pic)
		{
			msg << (byte)svc_sec_floor
				<< (word)i
				<< (word)s.floor.pic;
		}
		if (s.ceiling.pic != s.ceiling.base_pic)
		{
			msg << (byte)svc_sec_ceil
				<< (word)i
				<< (word)s.ceiling.pic;
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
	if (cmd_source == src_command)
	{
		GCon->Log("PreSpawn is not valid from console");
		return;
	}

	sv_player->Message << sv_signon;
	sv_player->Message << (byte)svc_signonnum << (byte)2;
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
	if (cmd_source == src_command)
	{
		GCon->Log("Spawn is not valid from console");
		return;
	}

	if (!sv_loading)
	{
		if (sv_player->bSpawned)
		{
			GCon->Log(NAME_Dev, "Already spawned");
		}
		if (sv_player->MO)
		{
			GCon->Log(NAME_Dev, "Mobj already spawned");
		}
		svpr.Exec("SpawnClient", SV_GetPlayerNum(sv_player));
	}
	else
	{
		if (!sv_player->MO)
		{
			Host_Error("Player without Mobj\n");
		}
	}
	SV_WriteChangedTextures(sv_player->Message);
	sv_player->Message << (byte)svc_set_angles
						<< (byte)(AngleToByte(sv_player->ViewAngles.pitch))
						<< (byte)(AngleToByte(sv_player->ViewAngles.yaw))
						<< (byte)0;
	sv_player->Message << (byte)svc_signonnum << (byte)3;
	sv_player->bFixAngle = false;
	memset(sv_player->OldStats, 0, sizeof(sv_player->OldStats));
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
	if (cmd_source == src_command)
	{
		GCon->Log("Begin is not valid from console");
		return;
	}

	sv_loading = false;

	sv_player->bSpawned = true;

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

void SV_DropClient(boolean)
{
	guard(SV_DropClient);
	if (sv_player->bSpawned)
	{
		svpr.Exec("DisconnectClient", (int)sv_player);
	}
	sv_player->bActive = false;
	svvars.Players[SV_GetPlayerNum(sv_player)] = NULL;
	sv_player->bSpawned = false;
	NET_Close(sv_player->NetCon);
	sv_player->NetCon = NULL;
	svs.num_connected--;
	sv_player->UserInfo[0] = 0;
	sv_reliable << (byte)svc_userinfo
				<< (byte)SV_GetPlayerNum(sv_player)
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
	byte		buf[128];
	TMessage	msg(buf, 128);
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
			if (sv_player->bActive && sv_player->Message.cursize)
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
	msg << (byte)svc_disconnect;
	count = NET_SendToAll(&msg, 5);
	if (count)
		GCon->Logf("Shutdown server failed for %d clients", count);

	for (i = 0; i < svs.max_clients; i++)
	{
		sv_player = svvars.Players[i];
		if (sv_player)
			SV_DropClient(crash);
	}

	//
	// clear structures
	//
	SV_DestroyAllThinkers();
	for (i = 0; i < MAXPLAYERS; i++)
	{
		memset((byte*)GPlayersBase[i] + sizeof(VObject), 0,
			GPlayersBase[i]->GetClass()->ClassSize - sizeof(VObject));
	}
	memset(svvars.Players, 0, sizeof(svvars.Players));
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
		char		mapname[12];
		strcpy(mapname, level.mapname);
		SV_SpawnServer(mapname, true);
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
	if (cmd_source == src_command)
	{
#ifdef CLIENT
		Cmd_ForwardToServer();
#endif
		return;
	}

	paused ^= 1;
	sv_reliable << (byte)svc_pause << (byte)paused;
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
	if (cmd_source == src_command)
	{
#ifdef CLIENT
		Cmd_ForwardToServer();
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
//	Initializes a client_t for a new net connection.  This will only be
// called once for a player each game, not once for each level change.
//
//==========================================================================

void SV_ConnectClient(VBasePlayer *player)
{
	guard(SV_ConnectClient);
	GCon->Logf(NAME_Dev, "Client %s connected", player->NetCon->address);

	svvars.Players[SV_GetPlayerNum(player)] = player;
	player->bActive = true;

	player->Message.Data = player->MsgBuf;
	player->Message.MaxSize = MAX_MSGLEN;
	player->Message.CurSize = 0;
	player->Message.AllowOverflow = true;		// we can catch it
	player->Message.Overflowed = false;
	player->bSpawned = false;
	if (!sv_loading)
	{
		player->MO = NULL;
		player->PlayerState = PST_REBORN;
		svpr.Exec("PutClientIntoServer", (int)player);
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

void SV_CheckForNewClients(void)
{
	guard(SV_CheckForNewClients);
	qsocket_t	*sock;
	int			i;
		
	//
	// check for new connections
	//
	while (1)
	{
		sock = NET_CheckNewConnections();
		if (!sock)
			break;

		//
		// init a new client structure
		//
		for (i = 0; i < svs.max_clients; i++)
			if (!svvars.Players[i])
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

extern bool net_connect_bot;
void SV_SetUserInfo(const char *info);

void SV_ConnectBot(const char *name)
{
	guard(SV_ConnectBot);
	qsocket_t	*sock;
	int			i;
		
	net_connect_bot = true;
	sock = NET_CheckNewConnections();
	if (!sock)
		return;

	//
	// init a new client structure
	//
	for (i = 0; i < svs.max_clients; i++)
		if (!svvars.Players[i])
			break;
	if (i == svs.max_clients)
		Sys_Error("SV_ConnectBot: no free clients");

	GPlayersBase[i]->NetCon = sock;
	GPlayersBase[i]->bIsBot = true;
	strcpy(GPlayersBase[i]->PlayerName, name);
	SV_ConnectClient(GPlayersBase[i]);
	svs.num_connected++;

	sv_player = svvars.Players[i];
	sv_player->Message.Clear();
	SV_RunClientCommand("PreSpawn\n");
	sv_player->Message.Clear();
	SV_RunClientCommand("Spawn\n");
	sv_player->Message.Clear();
	SV_SetUserInfo(sv_player->UserInfo);
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
	SV_ConnectBot(Argv(1));
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

	if (Argc() != 2)
	{
		GCon->Log("map <mapname> : cange level");
	 	return;
	}
	strcpy(mapname, Argv(1));

	SV_ShutdownServer(false);
#ifdef CLIENT
	CL_Disconnect();
#endif

	SV_InitBaseSlot();
	SV_ClearRebornSlot();
	P_ACSInitNewGame();
	// Default the player start spot group to 0
	RebornPosition = 0;
	svpr.SetGlobal("RebornPosition", RebornPosition);

	if ((int)Skill < sk_baby)
		Skill = sk_baby;
	if ((int)Skill > sk_nightmare)
		Skill = sk_nightmare;

	// Set up a bunch of globals
	gameskill = (skill_t)(int)Skill;
	svpr.Exec("G_InitNew", gameskill);

	SV_SpawnServer(mapname, true);
#ifdef CLIENT
	if (cls.state != ca_dedicated)
		CmdBuf << "Connect local\n";
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

	if (Argc () != 2)
	{
		GCon->Logf("maxplayers is %d", svs.max_clients);
		return;
	}

	if (sv.active)
	{
		GCon->Log("maxplayers can not be changed while a server is running.");
		return;
	}

	n = atoi(Argv(1));
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
		CmdBuf << "listen 0\n";
#endif
		DeathMatch = 0;
		NoMonsters = 0;
	}
	else
	{
#ifdef CLIENT
		CmdBuf << "listen 1\n";
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
	if (cmd_source == src_command)
	{
#ifdef CLIENT
		Cmd_ForwardToServer();
#endif
		return;
	}
	if (Argc() < 2)
		return;

	SV_BroadcastPrintf("%s: %s", sv_player->PlayerName, Args());
	SV_StartSound(NULL, S_GetSoundID("Chat"), 0, 127);
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
	void Serialize(const char* V, EName)
	{
		printf("%s\n", V);
	}
};

FConsoleDevice			Console;

FOutputDevice			*GCon = &Console;

void FOutputDevice::Log(const char* S)
{
	Serialize(S, NAME_Log);
}
void FOutputDevice::Log(EName Type, const char* S)
{
	Serialize(S, Type);
}
void FOutputDevice::Log(const FString& S)
{
	Serialize(*S, NAME_Log);
}
void FOutputDevice::Log(EName Type, const FString& S)
{
	Serialize(*S, Type);
}
void FOutputDevice::Logf(const char* Fmt, ...)
{
	va_list argptr;
	char string[1024];
	
	va_start(argptr, Fmt);
	vsprintf(string, Fmt, argptr);
	va_end(argptr);

	Serialize(string, NAME_Log);
}
void FOutputDevice::Logf(EName Type, const char* Fmt, ...)
{
	va_list argptr;
	char string[1024];
	
	va_start(argptr, Fmt);
	vsprintf(string, Fmt, argptr);
	va_end(argptr);

	Serialize(string, Type);
}

#endif

//**************************************************************************
//
//	$Log$
//	Revision 1.57  2004/02/09 17:28:45  dj_jl
//	Bots fix
//
//	Revision 1.56  2003/11/12 16:47:40  dj_jl
//	Changed player structure into a class
//	
//	Revision 1.55  2003/10/22 06:16:54  dj_jl
//	Secret level info saved in savegame
//	
//	Revision 1.54  2003/07/11 16:45:20  dj_jl
//	Made array of players with pointers
//	
//	Revision 1.53  2003/07/03 18:11:13  dj_jl
//	Moving extrafloors
//	
//	Revision 1.52  2003/03/08 16:02:53  dj_jl
//	A little multiplayer fix.
//	
//	Revision 1.51  2003/03/08 11:33:39  dj_jl
//	Got rid of some warnings.
//	
//	Revision 1.50  2002/09/07 16:31:51  dj_jl
//	Added Level class.
//	
//	Revision 1.49  2002/08/28 16:41:09  dj_jl
//	Merged VMapObject with VEntity, some natives.
//	
//	Revision 1.48  2002/08/08 18:05:20  dj_jl
//	Release fixes.
//	
//	Revision 1.47  2002/08/05 17:21:00  dj_jl
//	Made sound sequences reliable.
//	
//	Revision 1.46  2002/07/23 16:29:56  dj_jl
//	Replaced console streams with output device class.
//	
//	Revision 1.45  2002/07/23 13:10:37  dj_jl
//	Some fixes for switching to floating-point time.
//	
//	Revision 1.44  2002/07/13 07:43:31  dj_jl
//	Fixed net buffer hack.
//	
//	Revision 1.43  2002/06/29 16:00:45  dj_jl
//	Added total frags count.
//	
//	Revision 1.42  2002/06/14 15:37:47  dj_jl
//	Added FOutputDevice code for dedicated server.
//	
//	Revision 1.41  2002/03/28 18:03:24  dj_jl
//	Hack for single player, added SV_GetModelIndex
//	
//	Revision 1.40  2002/03/12 19:21:55  dj_jl
//	No need for linefeed in client-printing
//	
//	Revision 1.39  2002/03/09 18:06:25  dj_jl
//	Made Entity class and most of it's functions native
//	
//	Revision 1.38  2002/03/04 18:27:49  dj_jl
//	Fixes for weapons
//	
//	Revision 1.37  2002/02/22 18:09:52  dj_jl
//	Some improvements, beautification.
//	
//	Revision 1.36  2002/02/16 16:30:36  dj_jl
//	Fixed sending server infor to remote clients
//	
//	Revision 1.35  2002/02/15 19:12:04  dj_jl
//	Property namig style change
//	
//	Revision 1.34  2002/02/14 19:23:58  dj_jl
//	Beautification
//	
//	Revision 1.33  2002/02/06 17:30:36  dj_jl
//	Replaced Actor flags with boolean variables.
//	
//	Revision 1.32  2002/02/02 19:20:41  dj_jl
//	FFunction pointers used instead of the function numbers
//	
//	Revision 1.31  2002/01/28 18:43:48  dj_jl
//	Fixed "floating players"
//	
//	Revision 1.30  2002/01/24 18:15:23  dj_jl
//	Fixed "slow motion" bug
//	
//	Revision 1.29  2002/01/11 18:22:41  dj_jl
//	Started to use names in progs
//	
//	Revision 1.28  2002/01/07 12:16:43  dj_jl
//	Changed copyright year
//	
//	Revision 1.27  2002/01/03 18:38:25  dj_jl
//	Added guard macros and core dumps
//	
//	Revision 1.26  2001/12/28 16:26:39  dj_jl
//	Temporary fix for map teleport
//	
//	Revision 1.25  2001/12/27 17:33:29  dj_jl
//	Removed thinker list
//	
//	Revision 1.24  2001/12/18 19:03:16  dj_jl
//	A lots of work on VObject
//	
//	Revision 1.23  2001/12/12 19:28:49  dj_jl
//	Some little changes, beautification
//	
//	Revision 1.22  2001/12/04 18:16:28  dj_jl
//	Player models and skins handled by server
//	
//	Revision 1.21  2001/12/03 19:23:08  dj_jl
//	Fixes for view angles at respawn
//	
//	Revision 1.20  2001/12/01 17:40:41  dj_jl
//	Added support for bots
//	
//	Revision 1.19  2001/10/27 07:51:27  dj_jl
//	Beautification
//	
//	Revision 1.18  2001/10/22 17:25:55  dj_jl
//	Floatification of angles
//	
//	Revision 1.17  2001/10/18 17:36:31  dj_jl
//	A lots of changes for Alpha 2
//	
//	Revision 1.16  2001/10/12 17:31:13  dj_jl
//	no message
//	
//	Revision 1.15  2001/10/09 17:30:45  dj_jl
//	Improved stats updates
//	
//	Revision 1.14  2001/10/08 17:33:01  dj_jl
//	Different client and server level structures
//	
//	Revision 1.13  2001/10/04 17:18:23  dj_jl
//	Implemented the rest of cvar flags
//	
//	Revision 1.12  2001/09/27 17:03:20  dj_jl
//	Support for multiple mobj classes
//	
//	Revision 1.11  2001/09/24 17:35:24  dj_jl
//	Support for thinker classes
//	
//	Revision 1.10  2001/09/20 16:30:28  dj_jl
//	Started to use object-oriented stuff in progs
//	
//	Revision 1.9  2001/08/30 17:46:21  dj_jl
//	Removed game dependency
//	
//	Revision 1.8  2001/08/21 17:39:22  dj_jl
//	Real string pointers in progs
//	
//	Revision 1.7  2001/08/15 17:08:59  dj_jl
//	Fixed finale
//	
//	Revision 1.6  2001/08/07 16:46:23  dj_jl
//	Added player models, skins and weapon
//	
//	Revision 1.5  2001/08/04 17:32:39  dj_jl
//	Beautification
//	
//	Revision 1.4  2001/08/02 17:46:38  dj_jl
//	Added sending info about changed textures to new clients
//	
//	Revision 1.3  2001/07/31 17:16:31  dj_jl
//	Just moved Log to the end of file
//	
//	Revision 1.2  2001/07/27 14:27:54  dj_jl
//	Update with Id-s and Log-s, some fixes
//
//**************************************************************************
