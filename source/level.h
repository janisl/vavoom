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
//**
//**	INTERNAL DATA TYPES
//**  used by play and refresh
//**
//**************************************************************************

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
class VThinker:public VObject
{
	DECLARE_CLASS(VThinker, VObject, 0)
public:
	VThinker	*prev;
	VThinker	*next;
	boolean		destroyed;
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
	int			validcount;

	int			translucency;

	int			special;
	int			arg1;
	int			arg2;
	int			arg3;
	int			arg4;
	int			arg5;

	int			user_fields[5];
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

	//	Remember base textures so we can inform new clients about
	// changed textures
	int			base_toptexture;
	int			base_bottomtexture;
	int			base_midtexture;

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

#define SPF_NOBLOCKING		1	//	Not blocking
#define SPF_NOBLOCKSIGHT	2	//	Do not block sight
#define SPF_NOBLOCKSHOOT	4	//	Do not block shooting

struct sec_plane_t : public TPlane
{
	float		minz;
	float		maxz;

	int			pic;
	int			base_pic;

	float		xoffs;
	float		yoffs;

	int			flags;
	int			translucency;
};

struct sec_params_t
{
	int			lightlevel;
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
	int			special;
	int			tag;

	float		base_floorheight;
	float		base_ceilingheight;
	int			base_lightlevel;

	float		skyheight;

	// stone, metal, heavy, etc...
	int			seqType;

	// mapblock bounding box for height changes
	int			blockbox[4];

	// origin for any sounds played by the sector
	TVec		soundorg;

	// if == validcount, already checked
	int			validcount;

	// list of subsectors in sector
	// used to check if client can see this sector (it needs to be updated)
	subsector_t	*subsectors;

	int			linecount;
	line_t		**lines;  // [linecount] size

	int			user_fields[8];
};

//
// ===== Polyobj data =====
//
struct polyobj_t
{
	int 		numsegs;
	seg_t 		**segs;
	TVec		startSpot;
	vertex_t	*originalPts; 	// used as the base for the rotations
	vertex_t 	*prevPts; 		// use to restore the old point values
	float	 	angle;
	int 		tag;			// reference tag assigned in HereticEd
	int			bbox[4];
	int 		validcount;
	boolean 	crush; 			// should the polyobj attempt to crush mobjs?
	int 		seqType;
	subsector_t	*subsector;
	float		base_x;
	float		base_y;
	float		base_angle;
	boolean		changed;
	int			user_fields[3];
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
class mobj_t:public VThinker
{
	DECLARE_CLASS(mobj_t, VThinker, 0)
public:
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

	int				flags;
	int				flags2;			// Heretic flags
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

#define MAX_MOBJS	2048	//	Temporary limit required by client/server

//==========================================================================
//
//									LEVEL
//
//==========================================================================

#define MAXDEATHMATCHSTARTS		16
#define MAX_PLAYER_STARTS 		8

struct base_level_t
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

	float		time;
	int			tictime;

	int			totalkills;
	int			totalitems;
	int			totalsecret;    // for intermission

	char		mapname[12];
	char		nextmap[12];
	char		level_name[32];

	byte		*vis_data;
};

struct sv_level_t:base_level_t
{
	// !!! Valid only during level loading
	int			numthings;
	mthing_t	*things;
	int			behaviorsize;
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
 
	VThinker	thinkers;// both the head and tail of the thinker list
};

struct cl_level_t:base_level_t
{
};

// PUBLIC FUNCTION PROTOTYPES ----------------------------------------------

void CalcLine(line_t *line);
void CalcSeg(seg_t *seg);
void LoadLevel(sv_level_t &lev, const char *mapname);
void LoadLevel(cl_level_t &lev, const char *mapname);
subsector_t* PointInSubsector(const base_level_t &lev, float x, float y);
byte *LeafPVS(const base_level_t &lev, const subsector_t *ss);
sec_region_t *AddExtraFloor(line_t *line, sector_t *dst);

// PUBLIC DATA DECLARATIONS ------------------------------------------------

extern sv_level_t		level;
extern cl_level_t		cl_level;

//**************************************************************************
//
//	$Log$
//	Revision 1.13  2001/12/04 18:14:46  dj_jl
//	Renamed thinker_t to VThinker
//
//	Revision 1.12  2001/12/01 17:43:12  dj_jl
//	Renamed ClassBase to VObject
//	
//	Revision 1.11  2001/10/22 17:25:55  dj_jl
//	Floatification of angles
//	
//	Revision 1.10  2001/10/18 17:36:31  dj_jl
//	A lots of changes for Alpha 2
//	
//	Revision 1.9  2001/10/09 17:29:47  dj_jl
//	Removed some mobj fields not used by engine
//	
//	Revision 1.8  2001/10/08 17:33:01  dj_jl
//	Different client and server level structures
//	
//	Revision 1.7  2001/10/02 17:43:50  dj_jl
//	Added addfields to lines, sectors and polyobjs
//	
//	Revision 1.6  2001/09/24 17:35:24  dj_jl
//	Support for thinker classes
//	
//	Revision 1.5  2001/09/20 16:30:28  dj_jl
//	Started to use object-oriented stuff in progs
//	
//	Revision 1.4  2001/08/02 17:46:38  dj_jl
//	Added sending info about changed textures to new clients
//	
//	Revision 1.3  2001/07/31 17:16:30  dj_jl
//	Just moved Log to the end of file
//	
//	Revision 1.2  2001/07/27 14:27:54  dj_jl
//	Update with Id-s and Log-s, some fixes
//
//**************************************************************************
