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

#ifndef _SV_LOCAL_H
#define _SV_LOCAL_H

#define MAXHEALTH				100
#define DEFAULT_GRAVITY			1225.0

struct tmtrace_t;

extern VGameInfo*		GGameInfo;
extern VLevelInfo*		GLevelInfo;

//==========================================================================
//
//	sv_acs
//
//	Action code scripts
//
//==========================================================================

#define MAX_ACS_SCRIPT_VARS	20
#define MAX_ACS_MAP_VARS	128
#define MAX_ACS_WORLD_VARS	256
#define MAX_ACS_GLOBAL_VARS	64
#define MAX_ACS_STORE		20

//	Script types
enum
{
	SCRIPT_Closed		= 0,
	SCRIPT_Open			= 1,
	SCRIPT_Respawn		= 2,
	SCRIPT_Death		= 3,
	SCRIPT_Enter		= 4,
	SCRIPT_Pickup		= 5,
	SCRIPT_BlueReturn	= 6,
	SCRIPT_RedReturn	= 7,
	SCRIPT_WhiteReturn	= 8,
	SCRIPT_Lightning	= 12,
	SCRIPT_Unloading	= 13,
	SCRIPT_Disconnect	= 14,
};

class FACSGrowingArray
{
private:
	int		Size;
	int*	Data;
public:
	void Redim(int NewSize);
	void SetElemVal(int Index, int Value);
	int GetElemVal(int Index);
	void Serialise(VStream& Strm);
};

struct acsstore_t
{
	char	map[12];	// Target map
	int 	script;		// Script number on target map
	int 	args[4];	// Padded to 4 for alignment
};

void P_LoadACScripts(int Lump);
void P_UnloadACScripts();
void P_StartTypedACScripts(int Type);
bool P_StartACS(int number, int map, int arg1, int arg2, int arg3,
	VEntity* activator, line_t* line, int side, bool Always,
	bool WantResult);
bool P_TerminateACS(int number, int map);
bool P_SuspendACS(int number, int map);
void P_ACSInitNewGame();
void P_CheckACSStore();
void P_SerialiseScripts(VStream& Strm);

extern int				WorldVars[MAX_ACS_WORLD_VARS];
extern int				GlobalVars[MAX_ACS_GLOBAL_VARS];
extern FACSGrowingArray	WorldArrays[MAX_ACS_WORLD_VARS];
extern FACSGrowingArray	GlobalArrays[MAX_ACS_GLOBAL_VARS];
extern acsstore_t		ACSStore[MAX_ACS_STORE + 1]; // +1 for termination marker

//==========================================================================
//
//	sv_world
//
//	Map utilites
//
//==========================================================================

// MAXRADIUS is for precalculated sector block boxes
// the spider demon is larger, but we do not have any moving sectors nearby
#define MAXRADIUS		32.0

// mapblocks are used to check movement against lines and things
#define MapBlock(x)		((int)floor(x) >> 7)

#define PT_ADDLINES		1
#define PT_ADDTHINGS	2
#define PT_EARLYOUT		4

struct opening_t
{
	float		top;
	float		bottom;
	float		range;
	float		lowfloor;
	sec_plane_t	*floor;
	sec_plane_t	*ceiling;
	opening_t	*next;
};

struct intercept_t
{
	float		frac;		// along trace line
	enum
	{
		IF_IsALine = 0x01,
	};
	vuint32		Flags;
	VEntity		*thing;
	line_t		*line;
};

opening_t *SV_LineOpenings(const line_t* linedef, const TVec& point);

int P_BoxOnLineSide(float* tmbox, line_t* ld);

boolean SV_BlockLinesIterator(int x, int y, boolean(*func)(line_t*));
boolean SV_BlockThingsIterator(int x, int y, boolean(*func)(VEntity*),
	VObject* PrSelf, VMethod *prfunc);
boolean SV_PathTraverse(float x1, float y1, float x2, float y2,
	int flags, boolean(*trav)(intercept_t *), VObject* PrSelf, VMethod *prtrav);

sec_region_t *SV_FindThingGap(sec_region_t *gaps, const TVec &point, float z1, float z2);
opening_t *SV_FindOpening(opening_t *gaps, float z1, float z2);
sec_region_t *SV_PointInRegion(sector_t *sector, const TVec &p);
int SV_PointContents(const sector_t *sector, const TVec &p);
bool P_ChangeSector(sector_t * sector, int crunch);

//==========================================================================
//
//	sv_switch
//
//	Switches
//
//==========================================================================

void P_InitSwitchList();
void P_ClearButtons();
void P_ChangeSwitchTexture(line_t* line, bool useAgain, VName DefaultSound);
void P_UpdateButtons();

void P_InitTerrainTypes();
int SV_TerrainType(int pic);
void P_FreeTerrainTypes();

//==========================================================================
//
//	sv_tick
//
//	Handling thinkers, running tics
//
//==========================================================================

void P_InitThinkers();
void P_Ticker();

extern int 			TimerGame; // tic countdown for deathmatch

//==========================================================================
//
//	sv_poly
//
//	Polyobjs
//
//==========================================================================

void PO_SpawnPolyobj(float x, float y, int tag, int crush);
void PO_AddAnchorPoint(float x, float y, int tag);
boolean PO_MovePolyobj(int num, float x, float y);
boolean PO_RotatePolyobj(int num, float angle);
void PO_Init();
boolean PO_Busy(int polyobj);
polyobj_t *PO_GetPolyobj(int polyNum);
int PO_GetPolyobjMirror(int poly);
void PO_FreePolyobjData();

//==========================================================================
//
//	sv_main
//
//==========================================================================

//	Mobj baseline type
struct mobj_base_t
{
	TVec		Origin;			// position
	TAVec		Angles;			// orientation
	int			SpriteType;		// sprite orientation type
	int			SpriteIndex;	// used to find patch_t and flip value
	int			SpriteFrame;	// might be ORed with FF_FULLBRIGHT
	int			ModelIndex;
	int			ModelFrame;
	int			Translucency;	// translucency, in percents
	int			Translation;	// player color translation
	int			Effects;		// dynamic lights, trails
};

void SV_StartSound(const VEntity *, int, int, int);
void SV_StartLocalSound(const VEntity *, int, int, int);
void SV_StopSound(const VEntity *, int);
void SV_SectorStartSound(const sector_t *, int, int, int);
void SV_SectorStopSound(const sector_t *, int);
void SV_SectorStartSequence(const sector_t *, const char *);
void SV_SectorStopSequence(const sector_t *);
void SV_PolyobjStartSequence(const polyobj_t *, const char *);
void SV_PolyobjStopSequence(const polyobj_t *);
void SV_BroadcastPrintf(const char *s, ...);
void SV_BroadcastCentrePrintf(const char *s, ...);
void SV_ClientPrintf(VBasePlayer *player, const char *s, ...);
void SV_ClientCenterPrintf(VBasePlayer *player, const char *s, ...);
void SV_SetFloorPic(int i, int texture);
void SV_SetCeilPic(int i, int texture);
void SV_SetLineTexture(int side, int position, int texture);
void SV_SetLineTransluc(line_t *line, int trans);
const char *SV_GetMapName(int num);
int SV_FindModel(const char *name);
int SV_GetModelIndex(const VName &Name);
int SV_FindSkin(const char *name);
void SV_ChangeSky(const char* Sky1, const char* Sky2);
void SV_ChangeMusic(const char* SongName);
void SV_ChangeLocalMusic(VBasePlayer *player, const char* SongName);

void SV_ReadMove();

extern VBasePlayer*		sv_player;

//==========================================================================
//
//	????
//
//==========================================================================

// Difficulty/skill settings/filters.
enum skill_t
{
	sk_baby,
	sk_easy,
	sk_medium,
	sk_hard,
	sk_nightmare
};

void G_ExitLevel(int Position);
void G_SecretExitLevel(int Position);
void G_Completed(int Map, int Position, int SaveAngle);
void G_TeleportNewMap(int map, int position);
void G_WorldDone();
void G_PlayerReborn(int player);
void G_StartNewInit();

int GetMobjNum(VEntity *mobj);
VEntity* SetMobjPtr(int archiveNum);

extern VBasePlayer*		GPlayersBase[MAXPLAYERS]; // Bookkeeping on players - state.

extern skill_t			gameskill;
 
extern boolean			netgame;                // only true if packets are broadcast
extern boolean			deathmatch;   			// only if started as net death

extern boolean			paused;

//==========================================================================
//
//	Inlines
//
//==========================================================================

inline subsector_t* SV_PointInSubsector(float x, float y)
{
	return GLevel->PointInSubsector(TVec(x, y, 0));
}

inline int SV_GetPlayerNum(VBasePlayer* player)
{
	int		i;

	for (i = 0; i < MAXPLAYERS; i++)
	{
		if (player == GPlayersBase[i])
		{
		    return i;
		}
	}
	return 0;
}

#endif
