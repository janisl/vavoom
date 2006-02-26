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

class VGameInfo;
class VLevelInfo;
struct tmtrace_t;

extern VGameInfo*		GGameInfo;
extern VLevelInfo*		GLevelInfo;

#include "player.h"

//==========================================================================
//
//								THINKERS
//
//==========================================================================

//
//	Doubly linked list of actors.
//
class VThinker : public VObject
{
	DECLARE_CLASS(VThinker, VObject, 0)

	VThinker() : XLevel(GLevel), Level(GLevelInfo) {}

	VLevel*			XLevel;		//	Level object.
	VLevelInfo*		Level;		//	Level info object.

	void Serialise(VStream&);

	//	VThinker interface.
	virtual void Tick(float DeltaTime);
};

//==========================================================================
//
//	VLevelInfo
//
//==========================================================================

class VLevelInfo : public VThinker
{
	DECLARE_CLASS(VLevelInfo, VThinker, 0)

	enum { MAX_TID_COUNT = 200 };

	VGameInfo*		Game;

	// Maintain single and multi player starting spots.
	mthing_t		DeathmatchStarts[MAXDEATHMATCHSTARTS];  // Player spawn spots for deathmatch.
	int				NumDeathmatchStarts;
	mthing_t		PlayerStarts[MAX_PLAYER_STARTS * MAXPLAYERS];// Player spawn spots.

	int				TIDList[MAX_TID_COUNT + 1];	// +1 for termination marker
	VEntity*		TIDMobj[MAX_TID_COUNT];

	TVec			trace_start;
	TVec			trace_end;
	TVec			trace_plane_normal;

	TVec			linestart;
	TVec			lineend;

	VLevelInfo()
	{
		Level = this;
		Game = GGameInfo;
	}

	void eventSpawnSpecials()
	{
		svpr.Exec(GetVFunction("SpawnSpecials"), (int)this);
	}
	void eventUpdateSpecials()
	{
		svpr.Exec(GetVFunction("UpdateSpecials"), (int)this);
	}
	void eventAfterUnarchiveThinkers()
	{
		svpr.Exec(GetVFunction("AfterUnarchiveThinkers"), (int)this);
	}
	line_t* eventFindLine(int lineTag, int *searchPosition)
	{
		return (line_t*)svpr.Exec(GetVFunction("FindLine"), (int)this,
			lineTag, (int)searchPosition);
	}
	void eventPolyThrustMobj(VEntity* A, TVec thrustDir, polyobj_t* po)
	{
		svpr.Exec(GetVFunction("PolyThrustMobj"), (int)this, (int)A,
			PassFloat(thrustDir.x), PassFloat(thrustDir.y), PassFloat(thrustDir.z), (int)po);
	}
	bool eventTagBusy(int tag)
	{
		return !!svpr.Exec(GetVFunction("TagBusy"), (int)this, tag);
	}
	bool eventPolyBusy(int polyobj)
	{
		return !!svpr.Exec(GetVFunction("PolyBusy"), (int)this, polyobj);
	}
	int eventThingCount(int type, int tid)
	{
		return svpr.Exec(GetVFunction("ThingCount"), (int)this, type, tid);
	}
	VEntity* eventFindMobjFromTID(int tid, int *searchPosition)
	{
		return (VEntity*)svpr.Exec(GetVFunction("FindMobjFromTID"), (int)this, tid, (int)searchPosition);
	}
	bool eventExecuteActionSpecial(int Special, int Arg1, int Arg2, int Arg3,
		int Arg4, int Arg5, line_t* Line, int Side, VEntity* A)
	{
		return !!svpr.Exec(GetVFunction("ExecuteActionSpecial"), (int)this,
			Special, Arg1, Arg2, Arg3, Arg4, Arg5, (int)Line, Side, (int)A);
	}
	int eventEV_ThingProjectile(int tid, int type, int angle, int speed,
		int vspeed, int gravity, int newtid)
	{
		return svpr.Exec(GetVFunction("EV_ThingProjectile"), (int)this, tid,
			type, angle, speed, vspeed, gravity, newtid);
	}
	void eventStartPlaneWatcher(VEntity* it, line_t* line, int lineSide,
		bool ceiling, int tag, int height, int special, int arg1, int arg2,
		int arg3, int arg4, int arg5)
	{
		svpr.Exec(GetVFunction("StartPlaneWatcher"), (int)this, (int)it,
			(int)line, lineSide, ceiling, tag, height, special, arg1,
			arg2, arg3, arg4, arg5);
	}
	void eventSpawnMapThing(mthing_t* mthing)
	{
		svpr.Exec(GetVFunction("SpawnMapThing"), (int)this, (int)mthing);
	}
};

//==========================================================================
//
//	VGameInfo
//
//==========================================================================

class VGameInfo : public VObject
{
	DECLARE_CLASS(VGameInfo, VObject, 0)

	int				netgame;
	int				deathmatch;
	int				gameskill;
	int				respawn;
	int				nomonsters;
	int				fastparm;

	int*			validcount;
	int				skyflatnum;

	VBasePlayer*	Players[MAXPLAYERS]; // Bookkeeping on players - state.

	level_t*		level;

	tmtrace_t*		tmtrace;

	int				RebornPosition;

	float			frametime;

	int				num_stats;

	float			FloatBobOffsets[64];
	int				PhaseTable[64];

	VGameInfo()
	{}

	void eventInit()
	{
		svpr.Exec(GetVFunction("Init"), (int)this);
	}
	void eventInitNewGame(int skill)
	{
		svpr.Exec(GetVFunction("InitNewGame"), (int)this, skill);
	}
	VLevelInfo* eventCreateLevelInfo()
	{
		return (VLevelInfo*)svpr.Exec(GetVFunction("CreateLevelInfo"), (int)this);
	}
	void eventTranslateLevel(VLevel* InLevel)
	{
		svpr.Exec(GetVFunction("TranslateLevel"), (int)this, (int)InLevel);
	}
	void eventSpawnWorld(VLevel* InLevel)
	{
		svpr.Exec(GetVFunction("SpawnWorld"), (int)this, (int)InLevel);
	}
};

//==========================================================================
//
//								ENTITY DATA
//
// 	NOTES: VEntity
//
// 	mobj_ts are used to tell the refresh where to draw an image, tell the
// world simulation when objects are contacted, and tell the sound driver
// how to position a sound.
//
// 	The refresh uses the next and prev links to follow lists of things in
// sectors as they are being drawn. The sprite, frame, and angle elements
// determine which patch_t is used to draw the sprite if it is visible.
// The sprite and frame values are allmost allways set from state_t
// structures. The statescr.exe utility generates the states.h and states.c
// files that contain the sprite/frame numbers from the statescr.txt source
// file. The xyz origin point represents a point at the bottom middle of the
// sprite (between the feet of a biped). This is the default origin position
// for patch_ts grabbed with lumpy.exe. A walking creature will have its z
// equal to the floor it is standing on.
//
// 	The sound code uses the x,y, and subsector fields to do stereo
// positioning of any sound effited by the VEntity.
//
// 	The play simulation uses the blocklinks, x,y,z, radius, height to
// determine when mobj_ts are touching each other, touching lines in the map,
// or hit by trace lines (gunshots, lines of sight, etc). The VEntity->flags
// element has various bit flags used by the simulation.
//
// 	Every VEntity is linked into a single sector based on its origin
// coordinates. The subsector_t is found with R_PointInSubsector(x,y), and
// the sector_t can be found with subsector->sector. The sector links are
// only used by the rendering code, the play simulation does not care about
// them at all.
//
// 	Any VEntity that needs to be acted upon by something else in the play
// world (block movement, be shot, etc) will also need to be linked into the
// blockmap. If the thing has the MF_NOBLOCK flag set, it will not use the
// block links. It can still interact with other things, but only as the
// instigator (missiles will run into other things, but nothing can run into
// a missile). Each block in the grid is 128*128 units, and knows about every
// line_t that it contains a piece of, and every interactable VEntity that has
// its origin contained.
//
// 	A valid VEntity is a VEntity that has the proper subsector_t filled in for
// its xy coordinates and is linked into the sector from which the subsector
// was made, or has the MF_NOSECTOR flag set (the subsector_t needs to be
// valid even if MF_NOSECTOR is set), and is linked into a blockmap block or
// has the MF_NOBLOCKMAP flag set. Links should only be modified by the
// P_[Un]SetThingPosition() functions. Do not change the MF_NO? flags while
// a thing is valid.
//
// 	Any questions?
//
//==========================================================================

class VEntity : public VThinker
{
	DECLARE_CLASS(VEntity, VThinker, 0)
	NO_DEFAULT_CONSTRUCTOR(VEntity)

	// Info for drawing: position.
	TVec			Origin;

	// Momentums, used to update position.
	TVec			Velocity;

	TAVec			Angles;	// orientation

	//More drawing info: to determine current sprite.
	int				SpriteType;
	int				SpriteIndex;	// used to find patch_t and flip value
	int				SpriteFrame;	// might be ORed with FF_FULLBRIGHT

	int				ModelIndex;
	int				ModelFrame;
	int				ModelSkinIndex;
	int				ModelSkinNum;

	int				Translucency;
	int				Translation;

	float			FloorClip;		// value to use for floor clipping

	int				Effects;

	subsector_t*	SubSector;
	sector_t*		Sector;

	// Interaction info, by BLOCKMAP.
	// Links in blocks (if needed).
	VEntity*		BlockMapNext;
	VEntity*		BlockMapPrev;

	// The closest interval over all contacted Sectors.
	float			FloorZ;
	float			CeilingZ;

	//	Closest floor and ceiling, source of floorz and ceilingz
	sec_plane_t		*Floor;
	sec_plane_t		*Ceiling;

	// If == validcount, already checked.
	int				ValidCount;

	//	Flags
	dword			bSolid:1;		// Blocks.
	dword			bHidden:1;		// don't update to clients
									// (invisible but touchable)
	dword			bNoBlockmap:1;	// don't use the blocklinks
									// (inert but displayable)
	dword			bIsPlayer:1;	// Player or player-bot

	int				Health;

	// For movement checking.
	float			Radius;
	float			Height;

	// Additional info record for player avatars only.
	// Only valid if type == MT_PLAYER
	VBasePlayer		*Player;

	int				TID;			// thing identifier
	int				Special;		// special
	int				Args[5];		// special arguments

	int				NetID;

	FName SpriteName;
	float StateTime;	// state tic counter
	int StateNum;
	int NextState;
	dword bFixedModel:1;

	dword bNoGravity:1;		// don't apply gravity every tic
	dword bNoPassMobj:1;	// Disable z block checking.  If on,
							// this flag will prevent the mobj
							// from passing over/under other mobjs.
	dword bColideWithThings:1;
	dword bColideWithWorld:1;
	dword bCheckLineBlocking:1;
	dword bCheckLineBlockMonsters:1;
	dword bDropOff:1;		// allow jumps from high places
	dword bFloat:1;			// allow moves to any height, no gravity
	dword bFly:1;			// fly mode is active
	dword bBlasted:1;		// missile will pass through ghosts
	dword bCantLeaveFloorpic:1;	// stay within a certain floor type
	dword bFloorClip:1;		// if feet are allowed to be clipped
	dword bIgnoreCeilingStep:1;	// continue walk without lowering itself

	//  Params
	float Mass;
	float MaxStepHeight;

	//  Water
	int WaterLevel;
	int WaterType;

	//   For player sounds.
	FName SoundClass;
	FName SoundGender;

	static int FIndex_Destroyed;
	static int FIndex_Touch;
	static int FIndex_BlockedByLine;
	static int FIndex_ApplyFriction;
	static int FIndex_PushLine;
	static int FIndex_HandleFloorclip;
	static int FIndex_CrossSpecialLine;
	static int FIndex_SectorChanged;
	static int FIndex_RoughCheckThing;
	static int FIndex_GiveInventory;
	static int FIndex_TakeInventory;
	static int FIndex_CheckInventory;
	static int FIndex_GetSigilPieces;

	static void InitFuncIndexes(void);

	//	VObject interface.
	void Destroy();
	void Serialise(VStream&);

	void eventDestroyed()
	{
		svpr.Exec(GetVFunction(FIndex_Destroyed), (int)this);
	}
	boolean eventTouch(VEntity *Other)
	{
		return svpr.Exec(GetVFunction(FIndex_Touch), (int)this, (int)Other);
	}
	void eventBlockedByLine(line_t * ld)
	{
		svpr.Exec(GetVFunction(FIndex_BlockedByLine), (int)this, (int)ld);
	}
	void eventApplyFriction(void)
	{
		svpr.Exec(GetVFunction(FIndex_ApplyFriction), (int)this);
	}
	void eventPushLine(void)
	{
		svpr.Exec(GetVFunction(FIndex_PushLine), (int)this);
	}
	void eventHandleFloorclip(void)
	{
		svpr.Exec(GetVFunction(FIndex_HandleFloorclip), (int)this);
	}
	void eventCrossSpecialLine(line_t *ld, int side)
	{
		svpr.Exec(GetVFunction(FIndex_CrossSpecialLine), (int)this, (int)ld, side);
	}
	bool eventSectorChanged(int CrushChange)
	{
		return !!svpr.Exec(GetVFunction(FIndex_SectorChanged), (int)this, CrushChange);
	}
	bool eventRoughCheckThing(VEntity *Other)
	{
		return !!svpr.Exec(GetVFunction(FIndex_RoughCheckThing), (int)this, (int)Other);
	}
	void eventGiveInventory(FName ItemName, int Amount)
	{
		svpr.Exec(GetVFunction(FIndex_GiveInventory), (int)this, ItemName.GetIndex(), Amount);
	}
	void eventTakeInventory(FName ItemName, int Amount)
	{
		svpr.Exec(GetVFunction(FIndex_TakeInventory), (int)this, ItemName.GetIndex(), Amount);
	}
	int eventCheckInventory(FName ItemName)
	{
		return svpr.Exec(GetVFunction(FIndex_CheckInventory), (int)this, ItemName.GetIndex());
	}
	int eventGetSigilPieces()
	{
		return svpr.Exec(GetVFunction(FIndex_GetSigilPieces), (int)this);
	}

	void Remove(void)
	{
		ConditionalDestroy();
	}

	bool SetState(int state);
	void SetInitialState(int state);
	int FindState(FName StateName);

	boolean CheckWater(void);
	boolean CheckPosition(TVec Pos);
	boolean CheckRelPosition(TVec Pos);
	boolean TryMove(TVec newPos);
	void SlideMove(void);
	void BounceWall(float overbounce);
	void UpdateVelocity(void);
	void FakeZMovement(void);
	VEntity *CheckOnmobj(void);
	VEntity *RoughBlockCheck(int index);
	VEntity *RoughMonsterSearch(int distance);

	void LinkToWorld(void);
	void UnlinkFromWorld(void);
	bool CanSee(VEntity* Other);

	DECLARE_FUNCTION(Remove)
	DECLARE_FUNCTION(SetState)
	DECLARE_FUNCTION(SetInitialState)
	DECLARE_FUNCTION(FindState)
	DECLARE_FUNCTION(PlaySound)
	DECLARE_FUNCTION(PlayFullVolumeSound)
	DECLARE_FUNCTION(StopSound)
	DECLARE_FUNCTION(CheckWater)
	DECLARE_FUNCTION(CheckPosition)
	DECLARE_FUNCTION(CheckRelPosition)
	DECLARE_FUNCTION(TryMove)
	DECLARE_FUNCTION(SlideMove)
	DECLARE_FUNCTION(BounceWall)
	DECLARE_FUNCTION(UpdateVelocity)
	DECLARE_FUNCTION(CheckOnmobj)
	DECLARE_FUNCTION(LinkToWorld)
	DECLARE_FUNCTION(UnlinkFromWorld)
	DECLARE_FUNCTION(CanSee)
	DECLARE_FUNCTION(RoughMonsterSearch)
};

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
    dword		bIsALine:1;
	VEntity		*thing;
	line_t		*line;
};

opening_t *SV_LineOpenings(const line_t* linedef, const TVec& point);

int P_BoxOnLineSide(float* tmbox, line_t* ld);

boolean SV_BlockLinesIterator(int x, int y, boolean(*func)(line_t*));
boolean SV_BlockThingsIterator(int x, int y, boolean(*func)(VEntity*),
	VObject* PrSelf, FFunction *prfunc);
boolean SV_PathTraverse(float x1, float y1, float x2, float y2,
	int flags, boolean(*trav)(intercept_t *), VObject* PrSelf, FFunction *prtrav);

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
int SV_GetModelIndex(const FName &Name);
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
//	FFunction pointers used instead of the function numbers
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
