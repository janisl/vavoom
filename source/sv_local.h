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

#include "player.h"

//==========================================================================
//
//								THINKERS
//
//==========================================================================

//
//	Doubly linked list of actors.
//
class VThinker:public VObject
{
	DECLARE_ABSTRACT_CLASS(VThinker, VObject, 0)
	NO_DEFAULT_CONSTRUCTOR(VThinker)
};

//==========================================================================
//
//								MAPOBJ DATA
//
// 	NOTES: VMapObject
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
// positioning of any sound effited by the VMapObject.
//
// 	The play simulation uses the blocklinks, x,y,z, radius, height to
// determine when mobj_ts are touching each other, touching lines in the map,
// or hit by trace lines (gunshots, lines of sight, etc). The VMapObject->flags
// element has various bit flags used by the simulation.
//
// 	Every VMapObject is linked into a single sector based on its origin
// coordinates. The subsector_t is found with R_PointInSubsector(x,y), and
// the sector_t can be found with subsector->sector. The sector links are
// only used by the rendering code, the play simulation does not care about
// them at all.
//
// 	Any VMapObject that needs to be acted upon by something else in the play
// world (block movement, be shot, etc) will also need to be linked into the
// blockmap. If the thing has the MF_NOBLOCK flag set, it will not use the
// block links. It can still interact with other things, but only as the
// instigator (missiles will run into other things, but nothing can run into
// a missile). Each block in the grid is 128*128 units, and knows about every
// line_t that it contains a piece of, and every interactable VMapObject that has
// its origin contained.
//
// 	A valid VMapObject is a VMapObject that has the proper subsector_t filled in for
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

struct player_t;

// Map Object definition.
class VMapObject:public VThinker
{
	DECLARE_CLASS(VMapObject, VThinker, 0)

	VMapObject(void);
	virtual ~VMapObject(void);

	// Info for drawing: position.
	TVec			origin;

	// Momentums, used to update position.
	TVec			velocity;

	//More drawing info: to determine current sprite.
	TAVec			angles;	// orientation
	int				spritetype;
	int				sprite;	// used to find patch_t and flip value
	int				frame;	// might be ORed with FF_FULLBRIGHT

	int				model_index;
	int				alias_frame;
	int				alias_skinnum;

	int				translucency;
	int				translation;

	float			floorclip;		// value to use for floor clipping

	int				effects;

	subsector_t*	subsector;

	// Interaction info, by BLOCKMAP.
	// Links in blocks (if needed).
	VMapObject*		bnext;
	VMapObject*		bprev;

	// The closest interval over all contacted Sectors.
	float			floorz;
	float			ceilingz;

	//	Closest floor and ceiling, source of floorz and ceilingz
	sec_plane_t		*floor;
	sec_plane_t		*ceiling;

	// If == validcount, already checked.
	int				validcount;

	//	Flags
	boolean			bSolid;			// Blocks.
	boolean			bNoClient;		// don't update to clients
									// (invisible but touchable)
	boolean			bNoBlockmap;	// don't use the blocklinks
									// (inert but displayable)
	int				health;

	// For movement checking.
	float			radius;
	float			height;

	// Additional info record for player avatars only.
	// Only valid if type == MT_PLAYER
	player_t		*player;

	int				tid;			// thing identifier
	int				special;		// special
	int				args[5];		// special arguments

	int				netID;
};

//==========================================================================
//
//	sv_acs
//
//	Action code scripts
//
//==========================================================================

#define MAX_ACS_SCRIPT_VARS	10
#define MAX_ACS_MAP_VARS 	32
#define MAX_ACS_WORLD_VARS 	64
#define ACS_STACK_DEPTH 	32
#define MAX_ACS_STORE 		20

enum aste_t
{
	ASTE_INACTIVE,
	ASTE_RUNNING,
	ASTE_SUSPENDED,
	ASTE_WAITINGFORTAG,
	ASTE_WAITINGFORPOLY,
	ASTE_WAITINGFORSCRIPT,
	ASTE_TERMINATING
};

struct acsInfo_t
{
	int 	number;
	int 	*address;
	int 	argCount;
	aste_t 	state;
	int 	waitValue;
};

class VACS:public VThinker
{
	DECLARE_CLASS(VACS, VThinker, 0)
	NO_DEFAULT_CONSTRUCTOR(VACS)

	VMapObject 		*activator;
	line_t 		*line;
	int 		side;
	int 		number;
	int 		infoIndex;
	int 		delayCount;
	int 		stack[ACS_STACK_DEPTH];
	int			stackPtr;
	int 		vars[MAX_ACS_SCRIPT_VARS];
	int 		*ip;
};

struct acsstore_t
{
	char	map[12];	// Target map
	int 	script;		// Script number on target map
	int 	args[4];	// Padded to 4 for alignment
};

void P_LoadACScripts(boolean spawn_thinkers);
boolean P_StartACS(int number, int map, int *args, VMapObject *activator,
	line_t *line, int side);
boolean P_TerminateACS(int number, int map);
boolean P_SuspendACS(int number, int map);
void SV_InterpretACS(VACS *script);
void P_TagFinished(int tag);
void P_PolyobjFinished(int po);
void P_ACSInitNewGame(void);
void P_CheckACSStore(void);

extern int 			ACScriptCount;
extern byte 		*ActionCodeBase;
extern acsInfo_t 	*ACSInfo;
extern int 			MapVars[MAX_ACS_MAP_VARS];
extern int 			WorldVars[MAX_ACS_WORLD_VARS];
extern acsstore_t 	ACSStore[MAX_ACS_STORE+1]; // +1 for termination marker

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
    boolean		isaline;
	VMapObject		*thing;
	line_t		*line;
};

opening_t *SV_LineOpenings(const line_t* linedef, const TVec& point);

int P_BoxOnLineSide(float* tmbox, line_t* ld);

void SV_UnlinkFromWorld(VMapObject* thing);
void SV_LinkToWorld(VMapObject* thing);

boolean SV_BlockLinesIterator(int x, int y, boolean(*func)(line_t*),
	FFunction *prfunc);
boolean SV_BlockThingsIterator(int x, int y, boolean(*func)(VMapObject*),
	FFunction *prfunc);
boolean SV_PathTraverse(float x1, float y1, float x2, float y2,
	int flags, boolean(*trav)(intercept_t *), FFunction *prtrav);

sec_region_t *SV_FindThingGap(sec_region_t *gaps, const TVec &point, float z1, float z2);
opening_t *SV_FindOpening(opening_t *gaps, float z1, float z2);
sec_region_t *SV_PointInRegion(sector_t *sector, const TVec &p);
int SV_PointContents(const sector_t *sector, const TVec &p);

//==========================================================================
//
//	sv_sight
//
//	Sight checking
//
//==========================================================================

boolean P_CheckSight(VMapObject* t1, VMapObject* t2);

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
	TVec		origin;			// position
	TAVec		angles;			// orientation
	int			spritetype;		// sprite orientation type
	int			sprite;			// used to find patch_t and flip value
	int			frame;			// might be ORed with FF_FULLBRIGHT
	int			model_index;
	int			alias_frame;
	int			translucency;	// translucency, in percents
	int			translation;	// player color translation
	int			effects;		// dynamic lights, trails
};

void SV_StartSound(const VMapObject *, int, int, int);
void SV_StopSound(const VMapObject *, int);
void SV_SectorStartSound(const sector_t *, int, int, int);
void SV_SectorStopSound(const sector_t *, int);
void SV_SectorStartSequence(const sector_t *, const char *);
void SV_SectorStopSequence(const sector_t *);
void SV_PolyobjStartSequence(const polyobj_t *, const char *);
void SV_PolyobjStopSequence(const polyobj_t *);
void SV_BroadcastPrintf(const char *s, ...);
void SV_ClientPrintf(player_t *player, const char *s, ...);
void SV_ClientCenterPrintf(player_t *player, const char *s, ...);
void SV_SetFloorPic(int i, int texture);
void SV_SetCeilPic(int i, int texture);
void SV_SetLineTexture(int side, int position, int texture);
void SV_SetLineTransluc(line_t *line, int trans);
const char *SV_GetMapName(int num);
int SV_FindModel(const char *name);
int SV_FindSkin(const char *name);

void SV_ReadMove(void);

extern player_t		*sv_player;

//==========================================================================
//
//	????
//
//==========================================================================

// Difficulty/skill settings/filters.
typedef enum
{
    sk_baby,
    sk_easy,
    sk_medium,
    sk_hard,
    sk_nightmare
} skill_t;

void G_ExitLevel(void);
void G_SecretExitLevel(void);
void G_Completed(int map, int position);
void G_TeleportNewMap(int map, int position);
void G_WorldDone(void);
void G_PlayerReborn(int player);
void G_StartNewInit(void);

extern player_t			players[MAXPLAYERS]; // Bookkeeping on players - state.

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
	return PointInSubsector(level, x, y);
}

#endif

//**************************************************************************
//
//	$Log$
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
