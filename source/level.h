//**************************************************************************
//**
//**	##   ##    ##    ##   ##   ####     ####   ###     ###
//**	##   ##  ##  ##  ##   ##  ##  ##   ##  ##  ####   ####
//**	 ## ##  ##    ##  ## ##  ##    ## ##    ## ## ## ## ##
//**	 ## ##  ########  ## ##  ##    ## ##    ## ##  ###  ##
//**	  ###   ##    ##   ###    ##  ##   ##  ##  ##       ##
//**	   #    ##    ##    #      ####     ####   ##       ##
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
//**
//**	INTERNAL DATA TYPES
//**  used by play and refresh
//**
//**************************************************************************

#ifndef _LEVEL_H
#define _LEVEL_H

// HEADER FILES ------------------------------------------------------------

// MACROS ------------------------------------------------------------------

#define MAXPLAYERS		8

//
// 	Frame flags:
// 	handles maximum brightness (torches, muzzle flare, light sources)
//
#define FF_FULLBRIGHT	0x80	// flag in thing->frame
#define FF_FRAMEMASK	0x7f

// TYPES -------------------------------------------------------------------

//==========================================================================
//
//								THINKERS
//
//==========================================================================

//
//	Doubly linked list of actors.
//
struct thinker_t;

//
//	think_t is a function pointer to a routine to handle an actor
//
typedef void (*think_t)(thinker_t*);

struct thinker_t
{
	thinker_t	*prev;
    thinker_t	*next;
	think_t		function;
};

//==========================================================================
//
//								MAP
//
//==========================================================================

struct sector_t;
struct mobj_t;

//
//	Your plain vanilla vertex.
// 	Note: transformed values not buffered locally,
// like some DOOM-alikes ("wt", "WebView") did.
//
typedef TVec vertex_t;

//
//	Level special
//
struct special_t : public thinker_t
{
	int			funcnum;
	int			user_fields[128];
};

//
//  Bounding box
//
enum
{
    BOXTOP,
    BOXBOTTOM,
    BOXLEFT,
    BOXRIGHT
};

//
// Move clipping aid for LineDefs.
//
enum slopetype_t
{
    ST_HORIZONTAL,
    ST_VERTICAL,
    ST_POSITIVE,
    ST_NEGATIVE
};

//
// LineDef
//
struct line_t : public TPlane
{
    // Vertices, from v1 to v2.
    vertex_t	*v1;
    vertex_t	*v2;

    // Precalculated v2 - v1 for side checking.
	TVec		dir;

    // Animation related.
    int			flags;

    // Visual appearance: SideDefs.
    //  sidenum[1] will be -1 if one sided
    int			sidenum[2];

    // Neat. Another bounding box, for the extent
    //  of the LineDef.
	float		bbox[4];

    // To aid move clipping.
    slopetype_t slopetype;

    // Front and back sector.
    // Note: redundant? Can be retrieved from SideDefs.
    sector_t	*frontsector;
    sector_t	*backsector;

    // if == validcount, already checked
    int         validcount;

    // thinker_t for reversable actions
    special_t	*specialdata;

	int			translucency;

	int			special;
    int			arg1;
    int			arg2;
    int			arg3;
    int			arg4;
    int			arg5;
};

//
// The SideDef.
//
struct side_t
{
    // add this to the calculated texture column
	float		textureoffset;

    // add this to the calculated texture top
	float		rowoffset;

	float		base_textureoffset;
	float		base_rowoffset;

    // Texture indices.
    // We do not maintain names here.
    int			toptexture;
    int			bottomtexture;
    int			midtexture;

    // Sector the SideDef is facing.
    sector_t	*sector;
};

struct subsector_t;

//
// The LineSeg.
//
struct drawseg_t;
struct seg_t : public TPlane
{
    vertex_t	*v1;
    vertex_t	*v2;

	float		offset;
	float		length;

    side_t		*sidedef;
    line_t		*linedef;

    // Sector references.
    // Could be retrieved from linedef, too.
    // backsector is NULL for one sided lines
    sector_t	*frontsector;
    sector_t	*backsector;

	//	Side of line (for light calculations)
	int			side;

	drawseg_t	*drawsegs;
};

//
//	Each sector has a degenmobj_t in its center for sound origin purposes.
//
struct degenmobj_t : public thinker_t
{
    // Info for drawing: position.
	TVec		origin;
};

#define SPF_NOBLOCKING		1	//	Not blocking
#define SPF_NOBLOCKSIGHT	2	//	Do not block sight
#define SPF_NOBLOCKSHOOT	4	//	Do not block shooting

struct sec_plane_t : public TPlane
{
	float		minz;
	float		maxz;

	int			pic;
	angle_t		angle;

	float		xoffs;
	float		yoffs;

	float		xscale;
	float		yscale;

	float		light;
	int			flags;
	int			translucency;
};

struct sec_params_t
{
    int		    lightlevel;
	int			contents;
};

struct sec_region_t
{
	//	Linked list of regions in bottom to top order
	sec_region_t	*prev;
	sec_region_t	*next;

	//	Planes
	sec_plane_t		*floor;
	sec_plane_t		*ceiling;

	sec_params_t	*params;
	line_t			*extraline;
};

//
//	The SECTORS record, at runtime.
//	Stores things/mobjs.
//
struct sector_t
{
	sec_plane_t		floor;
	sec_plane_t		ceiling;
	sec_params_t	params;

	sec_region_t	*topregion;	//	Highest region
	sec_region_t	*botregion;	//	Lowest region

	float		floorheight;
	float		ceilingheight;
    int		    special;
    int			tag;

	float		base_floorheight;
	float		base_ceilingheight;
    int		    base_lightlevel;

	float		skyheight;

    // 0 = untraversed, 1,2 = sndlines -1
    int         soundtraversed;

    // thing that made a sound (or null)
    mobj_t		*soundtarget;

    // stone, metal, heavy, etc...
	int			seqType;

    // mapblock bounding box for height changes
    int         blockbox[4];

    // origin for any sounds played by the sector
    degenmobj_t soundorg;

    // if == validcount, already checked
    int         validcount;

    // thinker_t for reversable actions
    special_t	*specialdata;

	// list of subsectors in sector
	// used to check if client can see this sector (it needs to be updated)
	subsector_t	*subsectors;

    int			linecount;
    line_t		**lines;  // [linecount] size
};

//
// ===== Polyobj data =====
//
struct polyobj_t
{
	int 		numsegs;
	seg_t 		**segs;
	degenmobj_t startSpot;
	vertex_t 	*originalPts; 	// used as the base for the rotations
	vertex_t 	*prevPts; 		// use to restore the old point values
	angle_t 	angle;
	int 		tag;			// reference tag assigned in HereticEd
	int			bbox[4];
	int 		validcount;
	boolean 	crush; 			// should the polyobj attempt to crush mobjs?
	int 		seqType;
	special_t	*specialdata; 	// pointer a thinker, if the poly is moving
	subsector_t	*subsector;
	float		base_x;
	float		base_y;
	angle_t		base_angle;
	boolean		changed;
};

//
//
//
struct polyblock_t
{
	polyobj_t 	*polyobj;
	polyblock_t	*prev;
	polyblock_t	*next;
};

//
// BSP node.
//
struct node_t : public TPlane
{
    // Bounding box for each child.
	float		bbox[2][6];

    // If NF_SUBSECTOR its a subsector.
    word		children[2];

	node_t		*parent;
	int			visframe;
};

//
// 	A SubSector.
// 	References a Sector. Basically, this is a list of LineSegs, indicating
// the visible walls that define (all or some) sides of a convex BSP leaf.
//
struct subregion_t;
struct subsector_t
{
    sector_t	*sector;
	subsector_t	*seclink;
    int			numlines;
    int			firstline;
	polyobj_t	*poly;

	node_t		*parent;
	int			visframe;

	dword		dlightbits;
	int			dlightframe;
	subregion_t	*regions;
};

//==========================================================================
//
//								MAPOBJ DATA
//
// 	NOTES: mobj_t
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
// positioning of any sound effited by the mobj_t.
//
// 	The play simulation uses the blocklinks, x,y,z, radius, height to
// determine when mobj_ts are touching each other, touching lines in the map,
// or hit by trace lines (gunshots, lines of sight, etc). The mobj_t->flags
// element has various bit flags used by the simulation.
//
// 	Every mobj_t is linked into a single sector based on its origin
// coordinates. The subsector_t is found with R_PointInSubsector(x,y), and
// the sector_t can be found with subsector->sector. The sector links are
// only used by the rendering code, the play simulation does not care about
// them at all.
//
// 	Any mobj_t that needs to be acted upon by something else in the play
// world (block movement, be shot, etc) will also need to be linked into the
// blockmap. If the thing has the MF_NOBLOCK flag set, it will not use the
// block links. It can still interact with other things, but only as the
// instigator (missiles will run into other things, but nothing can run into
// a missile). Each block in the grid is 128*128 units, and knows about every
// line_t that it contains a piece of, and every interactable mobj_t that has
// its origin contained.
//
// 	A valid mobj_t is a mobj_t that has the proper subsector_t filled in for
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

//
// Map thing definition with initialised fields for global use.
//
struct mthing_t
{
	int 		tid;
	float		x;
	float		y;
	float		height;
	int 		angle;
	int			type;
	int			options;
	int 		special;
	int 		arg1;
	int 		arg2;
	int 		arg3;
	int 		arg4;
	int 		arg5;
};

struct player_t;

// Map Object definition.
struct mobj_t : public degenmobj_t
{
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
    mobj_t*			bnext;
    mobj_t*			bprev;
    
    // The closest interval over all contacted Sectors.
	float			floorz;
	float			ceilingz;

	//	Closest floor and ceiling, source of floorz and ceilingz
	sec_plane_t		*floor;
	sec_plane_t		*ceiling;

    // If == validcount, already checked.
    int				validcount;

    int				type;

	int				damage;			// For missiles
    int				flags;
	int				flags2;			// Heretic flags
    int				health;

    // For movement checking.
	float			radius;
	float			height;

    // Thing being chased/attacked (or NULL),
    // also the originator for missiles.
    mobj_t*			target;

    // Additional info record for player avatars only.
    // Only valid if type == MT_PLAYER
    player_t		*player;

	int				archiveNum;		// Identity during archive
	int				tid;			// thing identifier
	int				special;		// special
	int				args[5];		// special arguments

	int				netID;

	//	128 integers for user defined fields in PROGS
	int				user_fields[128];
};

#define MAX_MOBJS	2048	//	Temporary limit required by client/server

//==========================================================================
//
//									LEVEL
//
//==========================================================================

#define MAXDEATHMATCHSTARTS		16
#define MAX_PLAYER_STARTS 		8

struct mapInfo_t
{
	char	lumpname[12];
	int		cluster;    // Defines what cluster level belongs to
	int		warpTrans;  // Actual map number in case maps are not sequential
	char	nextMap[12];// Map to teleport to upon exit of timed deathmatch
	int		cdTrack;    // CD track to play during level
	char	name[32];   // Name of map
	int		sky1Texture;// Default sky texture
	int		sky2Texture;// Alternate sky displayed in Sky2 sectors
	float	sky1ScrollDelta;// Default sky texture speed
	float	sky2ScrollDelta;// Alternate sky texture speed
	boolean	doubleSky;  // parallax sky: sky2 behind sky1
	boolean	lightning;  // Use of lightning on the level flashes from sky1 to sky2
	char	fadetable[12];// Fade table {fogmap}
	char	songLump[12];// Background music (MUS or MIDI)
};

struct level_t
{
	//
	// MAP related Lookup tables.
	// Store VERTEXES, LINEDEFS, SIDEDEFS, etc.
	//
	// Lookup tables for map data.
	int			numvertexes;
	vertex_t	*vertexes;

	int			numsegs;
	seg_t		*segs;

	int			numsectors;
	sector_t	*sectors;

	int			numsubsectors;
	subsector_t	*subsectors;

	int			numnodes;
	node_t		*nodes;

	int			numlines;
	line_t		*lines;

	int			numsides;
	side_t		*sides;

	int 		numpolyobjs;
	polyobj_t	*polyobjs; // list of all poly-objects on the level

	// !!! Valid only during level loading
	int			numthings;
	mthing_t	*things;
	int			*behavior;

    //
	//	BLOCKMAP
	//	Created from axis aligned bounding box of the map, a rectangular
	// array of blocks of size ...
	// Used to speed up collision detection by spatial subdivision in 2D.
	//
	short		*blockmaplump;	// offsets in blockmap are from here
	short		*blockmap;		// int for larger maps
	int			bmapwidth;      // Blockmap size.
	int			bmapheight;     // size in mapblocks
	float		bmaporgx;       // origin of block map
	float		bmaporgy;
	mobj_t		**blocklinks;	// for thing chains
	polyblock_t **PolyBlockMap;

	//
	//	REJECT
	//	For fast sight rejection.
	//	Speeds up enemy AI by skipping detailed LineOf Sight calculation.
	// 	Without special effect, this could be used as a PVS lookup as well.
	//
	byte		*rejectmatrix;

	// Maintain single and multi player starting spots.
	mthing_t	deathmatchstarts[MAXDEATHMATCHSTARTS];  // Player spawn spots for deathmatch.
	int			numdeathmatchstarts;
	mthing_t	playerstarts[MAX_PLAYER_STARTS * MAXPLAYERS];// Player spawn spots.

	float		time;
	int			tictime;

	int			totalkills;
	int			totalitems;
	int			totalsecret;    // for intermission
 
	thinker_t	thinkers;// both the head and tail of the thinker list

	char		mapname[12];
	char		nextmap[12];
	char		level_name[32];

	byte		*vis_data;
};

// PUBLIC FUNCTION PROTOTYPES ----------------------------------------------

void P_GetMapInfo(const char *map, mapInfo_t &info);
void CalcLine(line_t *line);
void CalcSeg(seg_t *seg);
void LoadLevel(level_t &lev, const char *mapname, bool server_level);
subsector_t* PointInSubsector(const level_t &lev, float x, float y);
byte *LeafPVS(const level_t &lev, const subsector_t *ss);
sec_region_t *AddExtraFloor(line_t *line, sector_t *dst);

// PUBLIC DATA DECLARATIONS ------------------------------------------------

#endif
