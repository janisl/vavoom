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

#ifndef _SV_LOCAL_H
#define _SV_LOCAL_H

#define MAXHEALTH		100

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
void P_StartTypedACScripts(int Type);
bool P_StartACS(int number, int map, int arg1, int arg2, int arg3,
	VEntity* activator, line_t* line, int side, bool Always,
	bool WantResult);
boolean P_TerminateACS(int number, int map);
boolean P_SuspendACS(int number, int map);
void P_TagFinished(int tag);
void P_PolyobjFinished(int po);
void P_ACSInitNewGame(void);
void P_CheckACSStore(void);
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

void P_InitSwitchList(void);
void P_ClearButtons(void);
void P_ChangeSwitchTexture(line_t* line, int useAgain);
void P_UpdateButtons(void);

void P_InitTerrainTypes(void);
int SV_TerrainType(int pic);

//==========================================================================
//
//	sv_tick
//
//	Handling thinkers, running tics
//
//==========================================================================

void P_InitThinkers(void);
void P_Ticker(void);

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
void PO_Init(void);
boolean PO_Busy(int polyobj);
polyobj_t *PO_GetPolyobj(int polyNum);
int PO_GetPolyobjMirror(int poly);

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

void SV_ReadMove(void);

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
void G_WorldDone(void);
void G_PlayerReborn(int player);
void G_StartNewInit(void);

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

//**************************************************************************
//
//	$Log$
//	Revision 1.52  2006/03/18 16:51:15  dj_jl
//	Renamed type class names, better code serialisation.
//
//	Revision 1.51  2006/03/12 12:54:49  dj_jl
//	Removed use of bitfields for portability reasons.
//	
//	Revision 1.50  2006/03/06 13:05:51  dj_jl
//	Thunbker list in level, client now uses entity class.
//	
//	Revision 1.49  2006/02/28 18:06:28  dj_jl
//	Put thinkers back in linked list.
//	
//	Revision 1.48  2006/02/27 20:45:26  dj_jl
//	Rewrote names class.
//	
//	Revision 1.47  2006/02/26 20:52:48  dj_jl
//	Proper serialisation of level and players.
//	
//	Revision 1.46  2006/02/25 17:14:19  dj_jl
//	Implemented proper serialisation of the objects.
//	
//	Revision 1.45  2006/02/22 20:33:51  dj_jl
//	Created stream class.
//	
//	Revision 1.44  2006/02/15 23:28:18  dj_jl
//	Moved all server progs global variables to classes.
//	
//	Revision 1.43  2006/02/13 18:34:34  dj_jl
//	Moved all server progs global functions to classes.
//	
//	Revision 1.42  2006/02/05 18:52:44  dj_jl
//	Moved common utils to level info class or built-in.
//	
//	Revision 1.41  2005/12/27 22:24:00  dj_jl
//	Created level info class, moved action special handling to it.
//	
//	Revision 1.40  2005/12/11 21:37:00  dj_jl
//	Made path traversal callbacks class members.
//	
//	Revision 1.39  2005/12/07 22:53:26  dj_jl
//	Moved compiler generated data out of globals.
//	
//	Revision 1.38  2005/11/17 18:53:21  dj_jl
//	Implemented support for sndinfo extensions.
//	
//	Revision 1.37  2005/10/18 21:33:13  dj_jl
//	Fixed ceiling lightninfg.
//	
//	Revision 1.36  2005/05/03 14:57:07  dj_jl
//	Added support for specifying skin index.
//	
//	Revision 1.35  2005/03/16 15:04:44  dj_jl
//	More work on line specials.
//	
//	Revision 1.34  2004/12/27 12:23:16  dj_jl
//	Multiple small changes for version 1.16
//	
//	Revision 1.33  2004/12/22 07:49:13  dj_jl
//	More extended ACS support, more linedef flags.
//	
//	Revision 1.32  2004/12/03 16:15:47  dj_jl
//	Implemented support for extended ACS format scripts, functions, libraries and more.
//	
//	Revision 1.31  2003/11/12 16:47:40  dj_jl
//	Changed player structure into a class
//	
//	Revision 1.30  2003/09/24 16:42:31  dj_jl
//	Fixed rough block checking
//	
//	Revision 1.29  2003/07/11 16:45:20  dj_jl
//	Made array of players with pointers
//	
//	Revision 1.28  2003/03/08 12:10:13  dj_jl
//	API fixes.
//	
//	Revision 1.27  2002/09/07 16:31:51  dj_jl
//	Added Level class.
//	
//	Revision 1.26  2002/08/28 16:41:09  dj_jl
//	Merged VMapObject with VEntity, some natives.
//	
//	Revision 1.25  2002/07/23 13:10:37  dj_jl
//	Some fixes for switching to floating-point time.
//	
//	Revision 1.24  2002/03/28 18:05:25  dj_jl
//	Added SV_GetModelIndex.
//	
//	Revision 1.23  2002/03/09 18:05:34  dj_jl
//	Added support for defining native functions outside pr_cmds
//	
//	Revision 1.22  2002/02/22 18:09:52  dj_jl
//	Some improvements, beautification.
//	
//	Revision 1.21  2002/02/16 16:29:26  dj_jl
//	Added support for bool variables
//	
//	Revision 1.20  2002/02/15 19:12:04  dj_jl
//	Property namig style change
//	
//	Revision 1.19  2002/02/14 19:23:58  dj_jl
//	Beautification
//	
//	Revision 1.18  2002/02/06 17:30:36  dj_jl
//	Replaced Actor flags with boolean variables.
//	
//	Revision 1.17  2002/02/02 19:20:41  dj_jl
//	VMethod pointers used instead of the function numbers
//	
//	Revision 1.16  2002/01/07 12:16:43  dj_jl
//	Changed copyright year
//	
//	Revision 1.15  2001/12/27 17:33:29  dj_jl
//	Removed thinker list
//	
//	Revision 1.14  2001/12/18 19:03:16  dj_jl
//	A lots of work on VObject
//	
//	Revision 1.13  2001/12/04 18:14:46  dj_jl
//	Renamed thinker_t to VThinker
//	
//	Revision 1.12  2001/10/27 07:49:29  dj_jl
//	Fixed map block stuff
//	
//	Revision 1.11  2001/10/22 17:25:55  dj_jl
//	Floatification of angles
//	
//	Revision 1.10  2001/10/18 17:36:31  dj_jl
//	A lots of changes for Alpha 2
//	
//	Revision 1.9  2001/10/12 17:31:13  dj_jl
//	no message
//	
//	Revision 1.8  2001/09/24 17:35:24  dj_jl
//	Support for thinker classes
//	
//	Revision 1.7  2001/09/20 16:30:28  dj_jl
//	Started to use object-oriented stuff in progs
//	
//	Revision 1.6  2001/08/29 17:56:11  dj_jl
//	Added sound channels
//	
//	Revision 1.5  2001/08/07 16:46:23  dj_jl
//	Added player models, skins and weapon
//	
//	Revision 1.4  2001/08/04 17:25:14  dj_jl
//	Moved title / demo loop to progs
//	Removed shareware / ExtendedWAD from engine
//	
//	Revision 1.3  2001/07/31 17:16:31  dj_jl
//	Just moved Log to the end of file
//	
//	Revision 1.2  2001/07/27 14:27:54  dj_jl
//	Update with Id-s and Log-s, some fixes
//
//**************************************************************************
