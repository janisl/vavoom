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
#include "sv_local.h"
#include "moflags.h"

// MACROS ------------------------------------------------------------------

//#define REAL_TIME

#define TOCENTER 		-128

#define TEXTURE_TOP		0
#define TEXTURE_MIDDLE	1
#define TEXTURE_BOTTOM	2

#define REBORN_DESCRIPTION	"TEMP GAME"

// TYPES -------------------------------------------------------------------

// EXTERNAL FUNCTION PROTOTYPES --------------------------------------------

void Draw_TeleportIcon(void);
void CL_Disconnect(void);
void SV_MapTeleport(char *mapname);
bool SV_ReadClientMessages(int i);

// PUBLIC FUNCTION PROTOTYPES ----------------------------------------------

void SV_DropClient(boolean crash);

// PRIVATE FUNCTION PROTOTYPES ---------------------------------------------

static void G_DoReborn(int playernum);
static void G_DoCompleted(void);

// EXTERNAL DATA DECLARATIONS ----------------------------------------------

// PUBLIC DATA DEFINITIONS -------------------------------------------------

server_t		sv;
server_static_t	svs;
TProgs			svpr;

// increment every time a check is made
int				validcount = 1;

bool			sv_loading = false;

player_t        players[MAXPLAYERS];

skill_t         gameskill; 
 
boolean         paused;

boolean         deathmatch = false;   	// only if started as net death
boolean         netgame;                // only true if packets are broadcast

mobj_t			*sv_mobjs[MAX_MOBJS];
mobj_base_t		sv_mo_base[MAX_MOBJS];
double			sv_mo_free_time[MAX_MOBJS];

byte		sv_reliable_buf[MAX_MSGLEN];
TMessage	sv_reliable(sv_reliable_buf, MAX_MSGLEN);
byte		sv_datagram_buf[MAX_DATAGRAM];
TMessage	sv_datagram(sv_datagram_buf, MAX_DATAGRAM);
byte		sv_signon_buf[MAX_MSGLEN];
TMessage	sv_signon(sv_signon_buf, MAX_MSGLEN);

player_t	*sv_player;

char			sv_next_map[12];
char			sv_secret_map[12];

int 		TimerGame;

int				cid_mobj;
int				cid_acs;

// PRIVATE DATA DEFINITIONS ------------------------------------------------

static int 		LeavePosition;

static boolean	in_secret;
static char		mapaftersecret[12];

static int		RebornPosition;	// Position indicator for cooperative net-play reborn

static bool		completed;

static int		num_stats;

static int		pf_PlayerThink;

static TCvarI	TimeLimit("TimeLimit", "0");
static TCvarI	DeathMatch("DeathMatch", "0", CVAR_SERVERINFO);
static TCvarI  	NoMonsters("NoMonsters", "0");
static TCvarI	Skill("Skill", "2");

static TCvarI	sv_cheats("sv_cheats", "0", CVAR_SERVERINFO | CVAR_LATCH);

static byte		*fatpvs;
static TCvarI	show_mobj_overflow("show_mobj_overflow", "0", CVAR_ARCHIVE);

static bool		mapteleport_issued;

// CODE --------------------------------------------------------------------

//==========================================================================
//
//	SV_Init
//
//==========================================================================

void SV_Init(void)
{
	svs.max_clients = 1;

    svpr.Load("svprogs");

    svpr.SetGlobal("players", (int)players);
    svpr.SetGlobal("validcount", (int)&validcount);
	svpr.SetGlobal("level", (int)&level);
	svpr.SetGlobal("skyflatnum", skyflatnum);

	cid_mobj = svpr.GetClassID("base_mobj_t");
	cid_acs = svpr.GetClassID("ACS");

	num_stats = svpr.GetGlobal("num_stats");
	if (num_stats > 96)
		Sys_Error("Too many stats %d", num_stats);
    pf_PlayerThink = svpr.FuncNumForName("PlayerThink");

	P_InitSwitchList();
	P_InitTerrainTypes();
}

//==========================================================================
//
//	SV_Clear
//
//==========================================================================

void SV_Clear(void)
{
	memset(&sv, 0, sizeof(sv));
	memset(&level, 0, sizeof(level));
	memset(sv_mobjs, 0, sizeof(sv_mobjs));
	memset(sv_mo_base, 0, sizeof(sv_mo_base));
	memset(sv_mo_free_time, 0, sizeof(sv_mo_free_time));
	sv_signon.Clear();
	sv_reliable.Clear();
	sv_datagram.Clear();
#ifdef CLIENT
    // Make sure all sounds are stopped before Z_FreeTags.
    S_StopAllSound();
#endif
	Z_FreeTag(PU_LEVEL);
	Z_FreeTag(PU_LEVSPEC);
}

//==========================================================================
//
//	SV_ClearDatagram
//
//==========================================================================

void SV_ClearDatagram(void)
{
	sv_datagram.Clear();
}

//==========================================================================
//
//	SV_SpawnMobj
//
//==========================================================================

mobj_t *SV_SpawnMobj(int cid)
{
	int			i;
    mobj_t*		mobj;

    mobj = (mobj_t*)svpr.Spawn(cid, PU_LEVSPEC);
    P_AddThinker(mobj);

	//	Client treats first objects as player objects and will use
	// models and skins from player info
	for (i = svs.max_clients + 1; i < MAX_MOBJS; i++)
	{
		if (!sv_mobjs[i] && (sv_mo_free_time[i] < 1.0 ||
			level.time - sv_mo_free_time[i] > 2.0))
		{
			break;
		}
	}
	if (i == MAX_MOBJS)
	{
		Sys_Error("SV_SpawnMobj: Overflow");
	}

	sv_mobjs[i] = mobj;
	mobj->netID = i;

	return mobj;
}

//==========================================================================
//
//	SV_GetMobjBits
//
//==========================================================================

int	SV_GetMobjBits(mobj_t &mobj, mobj_base_t &base)
{
	int		bits = 0;

	if (fabs(base.origin.x - mobj.origin.x) >= 1.0)
		bits |=	MOB_X;
	if (fabs(base.origin.y - mobj.origin.y) >= 1.0)
		bits |=	MOB_Y;
	if (fabs(base.origin.z - (mobj.origin.z - mobj.floorclip)) >= 1.0)
		bits |=	MOB_Z;
	if ((base.angles.yaw & 0xff000000) != (mobj.angles.yaw & 0xff000000))
		bits |=	MOB_ANGLE;
	if ((base.angles.pitch & 0xff000000) != (mobj.angles.pitch & 0xff000000))
		bits |=	MOB_ANGLEP;
	if ((base.angles.roll & 0xff000000) != (mobj.angles.roll & 0xff000000))
		bits |=	MOB_ANGLER;
	if (base.sprite != mobj.sprite || base.spritetype != mobj.spritetype)
		bits |=	MOB_SPRITE;
	if (base.frame != mobj.frame)
		bits |=	MOB_FRAME;
	if (base.translucency != mobj.translucency)
		bits |=	MOB_TRANSLUC;
	if (base.translation != mobj.translation)
		bits |=	MOB_TRANSL;
	if (base.effects != mobj.effects)
		bits |= MOB_EFFECTS;
	if (base.model_index != mobj.model_index)
		bits |= MOB_MODEL;
	if (mobj.model_index && base.alias_frame != mobj.alias_frame)
		bits |= MOB_FRAME;

	return bits;
}

//==========================================================================
//
//	SV_WriteMobj
//
//==========================================================================

void SV_WriteMobj(int bits, mobj_t &mobj, TMessage &msg)
{
	if (bits & MOB_X)
		msg << (word)mobj.origin.x;
	if (bits & MOB_Y)
		msg << (word)mobj.origin.y;
	if (bits & MOB_Z)
		msg << (word)(mobj.origin.z - mobj.floorclip);
	if (bits & MOB_ANGLE)
		msg << (byte)(mobj.angles.yaw >> 24);
	if (bits & MOB_ANGLEP)
		msg << (byte)(mobj.angles.pitch >> 24);
	if (bits & MOB_ANGLER)
		msg << (byte)(mobj.angles.roll >> 24);
	if (bits & MOB_SPRITE)
		msg << (word)(mobj.sprite | (mobj.spritetype << 10));
	if (bits & MOB_FRAME)
		msg << (byte)mobj.frame;
	if (bits & MOB_TRANSLUC)
		msg << (byte)mobj.translucency;
	if (bits & MOB_TRANSL)
		msg << (byte)mobj.translation;
	if (bits & MOB_EFFECTS)
		msg << (byte)mobj.effects;
	if (bits & MOB_MODEL)
		msg << (word)mobj.model_index;
	if (mobj.model_index && (bits & MOB_FRAME))
		msg << (byte)mobj.alias_frame;
	if (bits & MOB_WEAPON)
		msg << (word)mobj.player->weapon_model;
}

//==========================================================================
//
//	SV_RemoveMobj
//
//==========================================================================

void SV_RemoveMobj(mobj_t *mobj)
{
	if (sv_mobjs[mobj->netID] != mobj)
		Sys_Error("Invalid mobj num %d", mobj->netID);

	// unlink from sector and block lists
	SV_UnlinkFromWorld(mobj);

	// stop any playing sound
	SV_StopSound(mobj, 0);
    
	sv_mobjs[mobj->netID] = NULL;
	P_RemoveThinker(mobj);

	sv_mo_free_time[mobj->netID] = level.time;
}

//==========================================================================
//
//	SV_CreateBaseline
//
//==========================================================================

void SV_CreateBaseline(void)
{
	int		i;

	for (i = 0; i < level.numsectors; i++)
	{
		sector_t &sec = level.sectors[i];
		if (sec.floor.translucency)
		{
			sv_signon << (byte)svc_sec_transluc
					<< (word)i
					<< (byte)sec.floor.translucency;
		}
	}

	for (i = 0; i < MAX_MOBJS; i++)
	{
		if (!sv_mobjs[i])
			continue;
		if (sv_mobjs[i]->flags & MF_NOSECTOR)
			continue;

		if (sv_signon.CurSize > sv_signon.MaxSize - 32)
		{
			con << "SV_CreateBaseline: Overflow\n";
			return;
		}

		mobj_t &mobj = *sv_mobjs[i];
		mobj_base_t &base = sv_mo_base[i];

		base.origin.x = mobj.origin.x;
		base.origin.y = mobj.origin.y;
		base.origin.z = mobj.origin.z - mobj.floorclip;
		base.angles.yaw = mobj.angles.yaw;
		base.angles.pitch = mobj.angles.pitch;
		base.angles.roll = mobj.angles.roll;
		base.spritetype = mobj.spritetype;
		base.sprite = mobj.sprite;
		base.frame = mobj.frame;
		base.translucency = mobj.translucency;
		base.translation = mobj.translation;
		base.effects = mobj.effects;
		base.model_index = mobj.model_index;
		base.alias_frame = mobj.alias_frame;

		sv_signon << (byte)svc_spawn_baseline
					<< (word)i
					<< (word)mobj.origin.x
					<< (word)mobj.origin.y
					<< (word)(mobj.origin.z - mobj.floorclip)
					<< (byte)(mobj.angles.yaw >> 24)
					<< (byte)(mobj.angles.pitch >> 24)
					<< (byte)(mobj.angles.roll >> 24)
					<< (word)(mobj.sprite | (mobj.spritetype << 10))
					<< (word)mobj.frame
					<< (byte)mobj.translucency
					<< (byte)mobj.translation
					<< (byte)mobj.effects
					<< (word)mobj.model_index
					<< (byte)mobj.alias_frame;
	}
}

//==========================================================================
//
//	GetOriginNum
//
//==========================================================================

int GetOriginNum(const mobj_t *mobj)
{
	if (!mobj)
	{
		return 0;
	}

	if (mobj->player)
	{
		return (mobj->player - players) + 1;
	}

	return mobj->netID;
}

//==========================================================================
//
//	SV_StartSound
//
//==========================================================================

void SV_StartSound(const TVec &origin, int origin_id, int sound_id,
	int channel, int volume)
{
	if (sv_datagram.CurSize + 12 > MAX_DATAGRAM)
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
}

//==========================================================================
//
//	SV_StopSound
//
//==========================================================================

void SV_StopSound(int origin_id, int channel)
{
	if (sv_datagram.CurSize + 3 > MAX_DATAGRAM)
		return;

	sv_datagram << (byte)svc_stop_sound
				<< (word)(origin_id | (channel << 13));
}

//==========================================================================
//
//	SV_StartSound
//
//==========================================================================

void SV_StartSound(const mobj_t * origin, int sound_id, int channel,
	int volume)
{
	if (origin)
	{
		SV_StartSound(origin->origin, GetOriginNum(origin), sound_id,
			channel, volume);
	}
	else
	{
		SV_StartSound(TVec(0, 0, 0), 0, sound_id, channel, volume);
	}
}

//==========================================================================
//
//	SV_StopSound
//
//==========================================================================

void SV_StopSound(const mobj_t *origin, int channel)
{
	SV_StopSound(GetOriginNum(origin), channel);
}

//==========================================================================
//
//	SV_SectorStartSound
//
//==========================================================================

void SV_SectorStartSound(const sector_t *sector, int sound_id, int channel,
	int volume)
{
	if (sector)
	{
		SV_StartSound(sector->soundorg,
			(sector - level.sectors) + MAX_MOBJS,
			sound_id, channel, volume);
	}
	else
	{
		SV_StartSound(TVec(0, 0, 0), 0, sound_id, channel, volume);
	}
}

//==========================================================================
//
//	SV_SectorStopSound
//
//==========================================================================

void SV_SectorStopSound(const sector_t *sector, int channel)
{
	SV_StopSound((sector - level.sectors) + MAX_MOBJS, channel);
}

//==========================================================================
//
//	SV_StartSequence
//
//==========================================================================

void SV_StartSequence(const TVec &origin, int origin_id, const char *name)
{
	if (sv_datagram.CurSize + 32 > MAX_DATAGRAM)
		return;

	sv_datagram << (byte)svc_start_seq
				<< (word)origin_id
				<< (word)origin.x
				<< (word)origin.y
				<< (word)origin.z
				<< name;
}

//==========================================================================
//
//	SV_StopSequence
//
//==========================================================================

void SV_StopSequence(int origin_id)
{
	if (sv_datagram.CurSize + 3 > MAX_DATAGRAM)
		return;

	sv_datagram << (byte)svc_stop_seq
				<< (word)origin_id;
}

//==========================================================================
//
//	SV_SectorStartSequence
//
//==========================================================================

void SV_SectorStartSequence(const sector_t *sector, const char *name)
{
	if (sector)
	{
		SV_StartSequence(sector->soundorg,
			(sector - level.sectors) + MAX_MOBJS, name);
	}
	else
	{
		SV_StartSequence(TVec(0, 0, 0), 0, name);
	}
}

//==========================================================================
//
//	SV_SectorStopSequence
//
//==========================================================================

void SV_SectorStopSequence(const sector_t *sector)
{
	SV_StopSequence((sector - level.sectors) + MAX_MOBJS);
}

//==========================================================================
//
//	SV_PolyobjStartSequence
//
//==========================================================================

void SV_PolyobjStartSequence(const polyobj_t *poly, const char *name)
{
	SV_StartSequence(poly->startSpot,
		(poly - level.polyobjs) + MAX_MOBJS + level.numsectors, name);
}

//==========================================================================
//
//	SV_PolyobjStopSequence
//
//==========================================================================

void SV_PolyobjStopSequence(const polyobj_t *poly)
{
	SV_StopSequence((poly - level.polyobjs) + MAX_MOBJS + level.numsectors);
}

//==========================================================================
//
//	SV_ClientPrintf
//
//==========================================================================

void SV_ClientPrintf(player_t *player, const char *s, ...)
{
	va_list	v;
	char	buf[1024];

	va_start(v, s);
   	vsprintf(buf, s, v);
	va_end(v);

	player->message << (byte)svc_print << buf;
}

//==========================================================================
//
//	SV_ClientCenterPrintf
//
//==========================================================================

void SV_ClientCenterPrintf(player_t *player, const char *s, ...)
{
	va_list	v;
	char	buf[1024];

	va_start(v, s);
   	vsprintf(buf, s, v);
	va_end(v);

	player->message << (byte)svc_center_print << buf;
}

//==========================================================================
//
//	SV_BroadcastPrintf
//
//==========================================================================

void SV_BroadcastPrintf(const char *s, ...)
{
	va_list	v;
	char	buf[1024];

	va_start(v, s);
   	vsprintf(buf, s, v);
	va_end(v);

	for (int i = 0; i < svs.max_clients; i++)
		if (players[i].active)
			players[i].message << (byte)svc_print << buf;
}

//==========================================================================
//
//	SV_WriteViewData
//
//==========================================================================

void SV_WriteViewData(player_t &player, TMessage &msg)
{
	int		i;

	msg << (byte)svc_view_data
		<< player.vieworg.x
		<< player.vieworg.y
		<< player.vieworg.z
		<< (byte)player.extralight
		<< (byte)player.fixedcolormap
		<< (byte)player.palette
		<< (byte)player.mo->translucency
		<< (word)player.pspriteSY;
	if (player.psprites[0].statenum)
	{
		msg << (word)player.psprites[0].sprite
			<< (byte)player.psprites[0].frame
			<< (word)player.psprites[0].model_index
			<< (byte)player.psprites[0].alias_frame
			<< (word)player.psprites[0].sx
			<< (word)player.psprites[0].sy;
	}
	else
	{
		msg << (short)-1;
	}
	if (player.psprites[1].statenum)
	{
		msg << (word)player.psprites[1].sprite
			<< (byte)player.psprites[1].frame
			<< (word)player.psprites[1].model_index
			<< (byte)player.psprites[1].alias_frame
			<< (word)player.psprites[1].sx
			<< (word)player.psprites[1].sy;
	}
	else
	{
		msg << (short)-1;
	}

	msg << (byte)player.health
		<< player.items;
	for (i = 0; i < MAXPLAYERS; i++)
		msg << (byte)player.frags[i];

	int bits = 0;
	for (i = 0; i < NUM_CSHIFTS; i++)
		if (player.cshifts[i] & 0xff000000)
			bits |= (1 << i);
	msg << (byte)bits;
	for (i = 0; i < NUM_CSHIFTS; i++)
		if (player.cshifts[i] & 0xff000000)
			msg << player.cshifts[i];

	//	Update angles (after teleportation)
	if (player.fixangle)
	{
		player.fixangle = false;
		msg << (byte)svc_set_angles
			<< (byte)(player.mo->angles.pitch >> 24)
			<< (byte)(player.mo->angles.yaw >> 24)
			<< (byte)(player.mo->angles.roll >> 24);
	}
}

//==========================================================================
//
//	SV_UpdateMobj
//
//==========================================================================

void SV_UpdateMobj(int i, TMessage &msg)
{
	int		bits;
	int		sendnum;

	bits = SV_GetMobjBits(*sv_mobjs[i], sv_mo_base[i]);

	if (sv_mobjs[i]->player)
	{
		sendnum = (sv_mobjs[i]->player - players) + 1;
		//	Clear look angles, because they must not affect model orientation
		bits &= ~(MOB_ANGLEP | MOB_ANGLER);
		if (sv_mobjs[i]->player->weapon_model)
		{
			bits |= MOB_WEAPON;
		}
	}
	else
	{
		sendnum = i;
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
}

//==========================================================================
//
//	SV_CheckFatPVS
//
//==========================================================================

int SV_CheckFatPVS(subsector_t *subsector)
{
	int ss = subsector - level.subsectors;
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
	int		i;
	int		bits;

	fatpvs = LeafPVS(level, sv_player->mo->subsector);

	for (i = 0; i < level.numsectors; i++)
	{
		sector_t	*sec;

		sec = &level.sectors[i];
		if (!SV_SecCheckFatPVS(sec))
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

		if (msg.CurSize + 14 > MAX_DATAGRAM)
		{
			cond << "UpdateLevel: secs overflow\n";
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
	for (i = 0; i < level.numsides; i++)
	{
		side_t		*side;

		side = &level.sides[i];
		if (!SV_SecCheckFatPVS(side->sector))
			continue;

		if (side->base_textureoffset == side->textureoffset &&
			side->base_rowoffset == side->rowoffset)
			continue;

		if (msg.CurSize + 7 > MAX_DATAGRAM)
		{
			cond << "UpdateLevel: sides overflow\n";
			return;
		}

		msg << (byte)svc_side_ofs
			<< (word)i
			<< (word)side->textureoffset
			<< (word)side->rowoffset;
	}

	for (i = 0; i < level.numpolyobjs; i++)
	{
		polyobj_t	*po;

		po = &level.polyobjs[i];
		if (!SV_CheckFatPVS(po->subsector))
			continue;

		if (po->base_x != po->startSpot.x ||
			po->base_y != po->startSpot.y ||
			po->base_angle != po->angle)
			po->changed = true;

		if (!po->changed)
			continue;

		if (msg.CurSize + 7 > MAX_DATAGRAM)
		{
			cond << "UpdateLevel: poly overflow\n";
			return;
		}

		msg << (byte)svc_poly_update
			<< (byte)i
			<< (word)po->startSpot.x
			<< (word)po->startSpot.y
			<< (byte)(po->angle >> 24);
  	}

	//	First update players
	for (i = 0; i < MAX_MOBJS; i++)
	{
		if (!sv_mobjs[i])
			continue;
		if (!sv_mobjs[i]->player)
			continue;
		if (msg.CurSize > 1000)
		{
			cond << "UpdateLevel: player overflow\n";
			return;
		}
		SV_UpdateMobj(i, msg);
	}

	//	Then update non-player mobjs in sight
	int starti = sv_player->mobj_update_start;
	for (i = 0; i < MAX_MOBJS; i++)
	{
		int index = (i + starti) % MAX_MOBJS;
		if (!sv_mobjs[index])
			continue;
		if (sv_mobjs[index]->flags & MF_NOSECTOR)
			continue;
		if (sv_mobjs[index]->player)
			continue;
		if (!SV_CheckFatPVS(sv_mobjs[index]->subsector))
			continue;
		if (msg.CurSize > 1000)
		{
			if (sv_player->mobj_update_start && show_mobj_overflow)
			{
				con << "UpdateLevel: mobj overflow 2\n";
			}
			else if (show_mobj_overflow > 1)
			{
				con << "UpdateLevel: mobj overflow\n";
			}
			//	Next update starts here
			sv_player->mobj_update_start = index;
			return;
		}
		SV_UpdateMobj(index, msg);
	}
	sv_player->mobj_update_start = 0;
}

//==========================================================================
//
//	SV_SendNop
//
//	Send a nop message without trashing or sending the accumulated client
// message buffer
//
//==========================================================================

void SV_SendNop(player_t *client)
{
	TMessage	msg;
	byte		buf[4];
	
	msg.Data = buf;
	msg.MaxSize = sizeof(buf);
	msg.CurSize = 0;

	msg << (byte)svc_nop;

	if (NET_SendUnreliableMessage(client->netcon, &msg) == -1)
		SV_DropClient(true);	// if the message couldn't send, kick off
	client->last_message = realtime;
}

//==========================================================================
//
//	SV_SendClientDatagram
//
//==========================================================================

void SV_SendClientDatagram(void)
{
	byte		buf[MAX_DATAGRAM];
	TMessage	msg(buf, MAX_DATAGRAM);

	for (int i = 0; i < svs.max_clients; i++)
	{
		if (!players[i].active)
		{
			continue;
		}

		sv_player = &players[i];

		if (!sv_player->spawned)
		{
			// the player isn't totally in the game yet
			// send small keepalive messages if too much time has passed
			// send a full message when the next signon stage has been requested
			// some other message data (name changes, etc) may accumulate
			// between signon stages
			if (realtime - sv_player->last_message > 5)
			{
				SV_SendNop(sv_player);
			}
			continue;
		}

		msg.Clear();

		msg << (byte)svc_time
			<< level.tictime;

		SV_WriteViewData(players[i], msg);

		if (msg.CurSize + sv_datagram.CurSize <= MAX_DATAGRAM)
			msg << sv_datagram;

		SV_UpdateLevel(msg);

		if (NET_SendUnreliableMessage(sv_player->netcon, &msg) == -1)
		{
			SV_DropClient(true);
		}
	}
}

//==========================================================================
//
//	SV_SendReliable
//
//==========================================================================

void SV_SendReliable(void)
{
	int		i, j;

	for (i = 0; i < svs.max_clients; i++)
	{
		if (!players[i].active)
			continue;

		players[i].message << sv_reliable;

		if (!players[i].spawned)
			continue;

		for (j = 0; j < num_stats; j++)
		{
			if (players[i].user_fields[j] == players[i].old_stats[j])
			{
				continue;
			}
			int sval = players[i].user_fields[j];
			if (sval >= 0 && sval < 256)
			{
				players[i].message << (byte)svc_stats_byte
					<< (byte)j << (byte)sval;
			}
			else if (sval >= MINSHORT && sval <= MAXSHORT)
			{
				players[i].message << (byte)svc_stats_short
					<< (byte)j << (short)sval;
			}
			else
			{
				players[i].message << (byte)svc_stats_long
					<< (byte)j << sval;
			}
			players[i].old_stats[j] = players[i].user_fields[j];
		}
	}

	sv_reliable.Clear();

	for (i = 0; i < svs.max_clients; i++)
	{
		if (!players[i].active)
		{
			continue;
		}

		if (players[i].message.Overflowed)
		{
			SV_DropClient(true);
			cond << "Client message overflowed\n";
			continue;
		}

		if (!players[i].message.CurSize)
		{
			continue;
		}

		if (!NET_CanSendMessage(players[i].netcon))
		{
			continue;
		}

		if (NET_SendMessage(players[i].netcon, &players[i].message) == -1)
		{
			SV_DropClient(true);
			continue;
		}
		players[i].message.Clear();
		players[i].last_message = realtime;
	}
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
    player_t		*player;
	static boolean	triedToSkip;
	bool			skip = false;

    for (i = 0, player = players; i < MAXPLAYERS; i++, player++)
    {
		if (players[i].active)
		{
		    if (player->buttons & BT_ATTACK)
		    {
				if (!player->attackdown)
				{
				    skip = true;
				}
				player->attackdown = true;
		    }
		    else
			{
				player->attackdown = false;
			}
		    if (player->buttons & BT_USE)
		    {
				if (!player->usedown)
				{
				    skip = true;
				}
				player->usedown = true;
		    }
		    else
			{
				player->usedown = false;
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
	int			i;

    // get commands
    for (i = 0; i < MAXPLAYERS; i++)
    {
		if (!players[i].active)
		{
			continue;
		}

	    // do player reborns if needed
		if (players[i].playerstate == PST_REBORN)
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
		if (players[i].spawned && !sv.intermission && !paused &&
			(netgame || !(MN_Active() || C_Active())))
#else
		if (players[i].spawned && !sv.intermission && !paused)
#endif
	    {
			svpr.Exec(pf_PlayerThink, (int)&players[i]);
		}
    }

	if (sv.intermission)
	{
		CheckForSkip();
		sv.intertime++;
	}
}

//==========================================================================
//
//	SV_Ticker
//
//==========================================================================

void SV_Ticker(void)
{
#ifndef REAL_TIME
	float	saved_frametime = host_frametime;
	host_frametime = 1.0 / 35.0;
#endif

	svpr.SetGlobal("frametime", PassFloat(host_frametime));
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

#ifndef REAL_TIME
	host_frametime = saved_frametime;
#endif
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
	if (position == TEXTURE_MIDDLE)
	{
		level.sides[side].midtexture = texture;
		sv_reliable << (byte)svc_side_mid
					<< (word)side
					<< (word)level.sides[side].midtexture;
	}
	else if (position == TEXTURE_BOTTOM)
	{
		level.sides[side].bottomtexture = texture;
		sv_reliable << (byte)svc_side_bot
					<< (word)side
					<< (word)level.sides[side].bottomtexture;
	}
	else
	{ // TEXTURE_TOP
		level.sides[side].toptexture = texture;
		sv_reliable << (byte)svc_side_top
					<< (word)side
					<< (word)level.sides[side].toptexture;
	}
}

//==========================================================================
//
//	SV_SetLineTransluc
//
//==========================================================================

void SV_SetLineTransluc(line_t *line, int trans)
{
	line->translucency = trans;
	sv_signon	<< (byte)svc_line_transluc
				<< (short)(line - level.lines)
				<< (byte)trans;
}

//==========================================================================
//
//	SV_SetFloorPic
//
//==========================================================================

void SV_SetFloorPic(int i, int texture)
{
	level.sectors[i].floor.pic = texture;
	sv_reliable << (byte)svc_sec_floor
				<< (word)i
				<< (word)level.sectors[i].floor.pic;
}

//==========================================================================
//
//	SV_SetCeilPic
//
//==========================================================================

void SV_SetCeilPic(int i, int texture)
{
	level.sectors[i].ceiling.pic = texture;
	sv_reliable << (byte)svc_sec_ceil
				<< (word)i
				<< (word)level.sectors[i].ceiling.pic;
}

//==========================================================================
//
//	G_DoCompleted
//
//==========================================================================

static void G_DoCompleted(void)
{
    int         i;
    int         j;
	mapInfo_t	old_info;
	mapInfo_t	new_info;

 	completed = false;
	if (sv.intermission)
	{
		return;
	}
	sv.intermission = 1;
	sv.intertime = 0;

	P_GetMapInfo(level.mapname, old_info);
	P_GetMapInfo(sv_next_map, new_info);

	for (i = 0; i < MAXPLAYERS; i++)
	{
		if (players[i].active)
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
		sv_reliable << (byte)players[i].active;
		for (j = 0; j < MAXPLAYERS; j++)
			sv_reliable << (byte)players[i].frags[j];
		sv_reliable << (short)players[i].killcount
					<< (short)players[i].itemcount
					<< (short)players[i].secretcount;
	}
}

//==========================================================================
//
//	G_ExitLevel
//
//==========================================================================

void G_ExitLevel(void)
{ 
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
}

//==========================================================================
//
//	G_SecretExitLevel
//
//==========================================================================

void G_SecretExitLevel(void)
{
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
		if (players[i].active)
		{
			players[i].didsecret = true;
		}
	}
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
}

//==========================================================================
//
//	COMMAND	TeleportNewMap
//
//==========================================================================

COMMAND(TeleportNewMap)
{
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

#ifdef CLIENT
	Draw_TeleportIcon();
#endif
	RebornPosition = LeavePosition;
    svpr.SetGlobal("RebornPosition", RebornPosition);
	mapteleport_issued = true;
}

//==========================================================================
//
//	G_DoReborn
//
//==========================================================================

static void G_DoReborn(int playernum)
{
	if (!players[playernum].spawned)
		return;
    if (!netgame && !deathmatch)// For fun now
	{
		CmdBuf << "Restart\n";
		players[playernum].playerstate = PST_LIVE;
	}
    else 
    {
    	svpr.Exec("NetGameReborn", playernum);
    }
	players[playernum].fixangle = true;
}

//==========================================================================
//
//	NET_SendToAll
//
//==========================================================================

int NET_SendToAll(TSizeBuf *data, int blocktime)
{
	double		start;
	int			i;
	int			count = 0;
	boolean		state1[MAXPLAYERS];
	boolean		state2[MAXPLAYERS];

	for (i = 0, sv_player = players; i < svs.max_clients; i++, sv_player++)
	{
		if (!players[i].netcon)
			continue;
		if (players[i].active)
		{
			if (players[i].netcon->driver == 0)
			{
				NET_SendMessage(players[i].netcon, data);
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
		for (i = 0, sv_player = players; i < svs.max_clients; i++, sv_player++)
		{
			if (!state1[i])
			{
				if (NET_CanSendMessage(sv_player->netcon))
				{
					state1[i] = true;
					NET_SendMessage(sv_player->netcon, data);
				}
				else
				{
					NET_GetMessage(sv_player->netcon);
				}
				count++;
				continue;
			}

			if (!state2[i])
			{
				if (NET_CanSendMessage(sv_player->netcon))
				{
					state2[i] = true;
				}
				else
				{
					NET_GetMessage(sv_player->netcon);
				}
				count++;
				continue;
			}
		}
		if ((Sys_Time() - start) > blocktime)
			break;
	}
	return count;
}

//==========================================================================
//
//	SV_SendServerInfo
//
//==========================================================================

void SV_SendServerInfo(player_t *player)
{
	int			i;
	byte		buf[MAX_MSGLEN];
	TMessage	msg(buf, MAX_MSGLEN);

	msg << (byte)svc_server_info
		<< (byte)PROTOCOL_VERSION
		<< svs.serverinfo
		<< level.mapname
		<< (byte)(player - players)
		<< (byte)svs.max_clients
		<< (byte)deathmatch
		<< level.totalkills
		<< level.totalitems
		<< level.totalsecret;

	for (i = 0; i < MAXPLAYERS; i++)
	{
		msg << (byte)svc_userinfo
			<< (byte)i
			<< players[i].userinfo;
	}

	int numsprites = svpr.GetGlobal("num_sprite_names");
    int num = svpr.GlobalNumForName("sprite_names");
	msg << (byte)svc_sprites
		<< (short)numsprites;
	for (i = 0; i < numsprites; i++)
	{
		msg << (char*)svpr.GetGlobal(num + i);
	}

	int nummodels = svpr.GetGlobal("num_models");
    num = svpr.GlobalNumForName("models");
	msg << (byte)svc_models
		<< (short)nummodels;
	for (i = 1; i < nummodels; i++)
	{
		msg << (char*)svpr.GetGlobal(num + i);
	}

	msg << (byte)svc_signonnum
		<< (byte)1;
	if (NET_SendMessage(player->netcon, &msg) == -1)
	{
		SV_DropClient(true);
		cond << "Send failed\n";
	}
}

//==========================================================================
//
//	SV_SpawnServer
//
//==========================================================================

void SV_SpawnServer(char *mapname, boolean spawn_thinkers)
{
    int			i;
	mapInfo_t	info;

	cond << "Spawning server " << mapname << endl;
	paused = false;
	mapteleport_issued = false;

	if (sv.active)
	{
		//	Level change
		for (i = 0; i < MAXPLAYERS; i++)
		{
			if (!players[i].active)
				continue;

			players[i].killcount = 0;
			players[i].secretcount = 0;
			players[i].itemcount = 0;

			players[i].spawned = false;
			players[i].mo = NULL;
			memset(players[i].frags, 0, sizeof(players[i].frags));
			if (players[i].playerstate == PST_DEAD)
				players[i].playerstate = PST_REBORN;
			players[i].message.Clear();
		}
	}
	else
	{
		//	New game
		in_secret = false;
	}

	SV_Clear();
	Cvar_Unlatch();

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
	LoadLevel(level, level.mapname);

	//	Spawn slopes, extra floors, etc.
	svpr.Exec("SpawnWorld");

    P_InitThinkers();
	int pf_spawn_map_thing = svpr.FuncNumForName("P_SpawnMapThing");
	for (i = 0; i < level.numthings; i++)
	{
		svpr.Exec(pf_spawn_map_thing, (int)&level.things[i], spawn_thinkers);
	}
	Z_Free(level.things);
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

	for (i = 0; i < svs.max_clients; i++)
	{
		if (players[i].active)
			SV_SendServerInfo(&players[i]);
	}

	if (!spawn_thinkers)
	{
	    if (level.thinkers.next != &level.thinkers)
    	{
    		Sys_Error("Spawned a thinker when it's not allowed");
	    }
		return;
	}

	P_Ticker();
	P_Ticker();
	SV_CreateBaseline();

	cond << "Server spawned\n";
}

//==========================================================================
//
//	SV_GetMapName
//
//==========================================================================

const char *SV_GetMapName(int num)
{
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

	for (i = 0; i < level.numsides; i++)
	{
		side_t &s = level.sides[i];
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

	for (i = 0; i < level.numsectors; i++)
	{
		sector_t &s = level.sectors[i];
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
	if (cmd_source == src_command)
	{
		con << "PreSpawn is not valid from console\n";
		return;
	}

	sv_player->message << sv_signon;
	sv_player->message << (byte)svc_signonnum << (byte)2;
}

//==========================================================================
//
//	COMMAND Spawn
//
//==========================================================================

COMMAND(Spawn)
{
	if (cmd_source == src_command)
	{
		con << "Spawn is not valid from console\n";
		return;
	}

	if (!sv_loading)
	{
		if (sv_player->spawned)
		{
			con << "Already spawned\n";
		}
		if (sv_player->mo)
		{
			con << "Mobj already spawned\n";
		}
		svpr.Exec("SpawnClient", sv_player - players);
	}
	else
	{
		if (!sv_player->mo)
		{
			Host_Error("Player without Mobj\n");
		}
	}
	SV_WriteChangedTextures(sv_player->message);
	sv_player->message << (byte)svc_set_angles
						<< (byte)(sv_player->mo->angles.pitch >> 24)
						<< (byte)(sv_player->mo->angles.yaw >> 24)
						<< (byte)0;
	sv_player->message << (byte)svc_signonnum << (byte)3;
	memset(sv_player->old_stats, 0, sizeof(sv_player->old_stats));
}

//==========================================================================
//
//	COMMAND Begin
//
//==========================================================================

COMMAND(Begin)
{
	if (cmd_source == src_command)
	{
		con << "Begin is not valid from console\n";
		return;
	}

	sv_loading = false;

	sv_player->spawned = true;

	// For single play, save immediately into the reborn slot
	if (!netgame)
	{
		SV_SaveGame(SV_GetRebornSlot(), REBORN_DESCRIPTION);
	}
}

//==========================================================================
//
//	SV_DropClient
//
//==========================================================================

void SV_DropClient(boolean)
{
	if (sv_player->spawned)
	{
		svpr.Exec("DisconnectClient", (int)sv_player);
	}
	sv_player->active = false;
	sv_player->spawned = false;
	NET_Close(sv_player->netcon);
	sv_player->netcon = NULL;
	svs.num_connected--;
	sv_player->userinfo[0] = 0;
	sv_reliable << (byte)svc_userinfo
				<< (byte)(sv_player - players)
				<< "";
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
			if (sv_player->active && sv_player->message.cursize)
			{
				if (NET_CanSendMessage (sv_player->netconnection))
				{
					NET_SendMessage(sv_player->netconnection, &sv_player->message);
					SZ_Clear (&sv_player->message);
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
		con << "Shutdown server failed for " << count << " clients\n";

	for (i = 0, sv_player = players; i < svs.max_clients; i++, sv_player++)
		if (sv_player->active)
			SV_DropClient(crash);

	//
	// clear structures
	//
	memset(players, 0, sizeof(players));
	memset(&sv, 0, sizeof(sv));
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
}

#endif

//==========================================================================
//
//	COMMAND Pause
//
//==========================================================================

COMMAND(Pause)
{
	if (cmd_source == src_command)
	{
#ifdef CLIENT
		Cmd_ForwardToServer();
#endif
		return;
	}

	paused ^= 1;
	sv_reliable << (byte)svc_pause << (byte)paused;
}

//==========================================================================
//
//  Stats_f
//
//==========================================================================

COMMAND(Stats)
{
	if (cmd_source == src_command)
	{
#ifdef CLIENT
		Cmd_ForwardToServer();
#endif
		return;
	}

	SV_ClientPrintf(sv_player, "Kills: %d of %d\n", sv_player->killcount, level.totalkills);
	SV_ClientPrintf(sv_player, "Items: %d of %d\n", sv_player->itemcount, level.totalitems);
	SV_ClientPrintf(sv_player, "Secrets: %d of %d\n", sv_player->secretcount, level.totalsecret);
}

//==========================================================================
//
//	SV_ConnectClient
//
//	Initializes a client_t for a new net connection.  This will only be
// called once for a player each game, not once for each level change.
//
//==========================================================================

void SV_ConnectClient(player_t *player)
{
	cond << "Client " << player->netcon->address << " connected\n";

	player->active = true;

	player->message.Data = player->msgbuf;
	player->message.MaxSize = MAX_MSGLEN;
	player->message.CurSize = 0;
	player->message.AllowOverflow = true;		// we can catch it
	player->spawned = false;
	if (!sv_loading)
	{
		player->mo = NULL;
		player->playerstate = PST_REBORN;
	}
	memset(player->frags, 0, sizeof(player->frags));

	SV_SendServerInfo(player);
}

//==========================================================================
//
//	SV_CheckForNewClients
//
//==========================================================================

void SV_CheckForNewClients(void)
{
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
			if (!players[i].active)
				break;
		if (i == svs.max_clients)
			Sys_Error("Host_CheckForNewClients: no free clients");

		players[i].netcon = sock;
		SV_ConnectClient(&players[i]);
		svs.num_connected++;
	}
}

//==========================================================================
//
//  Map
//
//==========================================================================

COMMAND(Map)
{
	char	mapname[12];

	if (Argc() != 2)
	{
		con << "map <mapname> : cange level\n";
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
}

//==========================================================================
//
//	COMMAND MaxPlayers
//
//==========================================================================

COMMAND(MaxPlayers)
{
	int 	n;

	if (Argc () != 2)
	{
		con << "\"maxplayers\" is \"" << svs.max_clients << "\"\n";
		return;
	}

	if (sv.active)
	{
		con << "maxplayers can not be changed while a server is running.\n";
		return;
	}

	n = atoi(Argv(1));
	if (n < 1)
		n = 1;
	if (n > MAXPLAYERS)
	{
		n = MAXPLAYERS;
		con << "\"maxplayers\" set to \"" << n << "\"\n";
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
}

//==========================================================================
//
//	ServerFrame
//
//==========================================================================

void ServerFrame(int realtics)
{
	try
	{
		SV_ClearDatagram();

		SV_CheckForNewClients();

#ifndef REAL_TIME
	    // run the count dics
    	while (realtics--)
#endif
	    {
			SV_Ticker();
		}

		if (mapteleport_issued)
		{
			SV_MapTeleport(sv_next_map);
		}

		SV_SendClientMessages();
	}
	catch (...)
	{
		dprintf("- ServerFrame\n");
		throw;
	}
}

//==========================================================================
//
//	COMMAND Say
//
//==========================================================================

COMMAND(Say)
{
	if (cmd_source == src_command)
	{
#ifdef CLIENT
		Cmd_ForwardToServer();
#endif
		return;
	}
	if (Argc() < 2)
		return;

   	SV_BroadcastPrintf("%s: %s\n", sv_player->name, Args());
	SV_StartSound(NULL, S_GetSoundID("Chat"), 0, 127);
}

//**************************************************************************
//
//	Dedicated server console streams
//
//**************************************************************************

#ifndef CLIENT

class TConBuf : public streambuf
{
 public:
	TConBuf(bool dev_buf)
	{
		dev_only = dev_buf;
	}

	int sync();
	int overflow(int ch);

	bool		dev_only;
};
     
static TConBuf			cbuf(false);
static TConBuf			cdbuf(true);

ostream					con(&cbuf);
ostream					cond(&cdbuf);

//==========================================================================
//
//  TConBuf::sync
//
//==========================================================================

int TConBuf::sync()
{
	return 0;
}
     
//==========================================================================
//
//  TConBuf::overflow
//
//==========================================================================

int TConBuf::overflow(int ch)
{
	if (ch != EOF && (!dev_only || (int)developer))
	{
		dprintf("%c", ch);
   		cout << (char)ch;
	}
	return 0;
}

#endif

//**************************************************************************
//
//	$Log$
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
