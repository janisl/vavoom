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
//								MAP
//
//==========================================================================

struct sector_t;
class VThinker;
class VEntity;

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
	int			LightColor;
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

	//	Flags.
	dword		bHasExtrafloors:1;		//	This sector has extrafloors.
	dword		bExtrafloorSource:1;	//	This sector is a source of an extrafloor.

	int			user_fields[6];
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
	dword	 	bCrush:1; 			// should the polyobj attempt to crush mobjs?
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
	int			VisFrame;
	int			SkyVisFrame;
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
	int			VisFrame;
	int			SkyVisFrame;

	dword		dlightbits;
	int			dlightframe;
	subregion_t	*regions;
};

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

//
//	Strife conversation scripts
//

struct FRogueConChoice
{
	int			Unknown1;
	int			Unknown2;
	int			Unknown3;
	int			Unknown4;
	int			Unknown5;
	int			Unknown6;
	int			Unknown7;
	char		Text[32];	//	Text of the answer
	char		TextOK[80];	//	Message displayed on success
	int			Next;		//	Dialog to go on success, negative values to go
							// here immediately
	int			Objectives;	//	Mission objectives, LOGxxxx lump
	char		TextNo[80];	//	Message displayed on failure (player doesn't
							// have needed thing, it haves enough health/ammo,
							// item is not ready, quest is not completed)
};

struct FRogueConSpeech
{
	int			SpeakerID;	//	Type of the object (MT_xxx)
	int			Unknown1;
	int			Unknown2;
	int			Unknown3;
	int			Unknown4;
	int			Unknown5;
	char		Name[16];	//	Name of the character
	char		Voice[8];	//	Voice to play
	char		BackPic[8];	//	Picture of the speaker
	char		Text[320];	//	Message
	FRogueConChoice	Choices[5];	//	Choices
};

//==========================================================================
//
//									LEVEL
//
//==========================================================================

#define MAXDEATHMATCHSTARTS		16
#define MAX_PLAYER_STARTS 		16

class VLevel : public VObject
{
	DECLARE_CLASS(VLevel, VObject, 0)
	NO_DEFAULT_CONSTRUCTOR(VLevel)

	//	Flags.
	dword bForServer:1;		//	True if this level belongs to the server.
	dword bExtended:1;		//	True if level was in Hexen format.

	//
	//	MAP related Lookup tables.
	//	Store VERTEXES, LINEDEFS, SIDEDEFS, etc.
	//

	int				NumVertexes;
	vertex_t		*Vertexes;

	int				NumSectors;
	sector_t		*Sectors;

	int				NumSides;
	side_t			*Sides;

	int				NumLines;
	line_t			*Lines;

	int				NumSegs;
	seg_t			*Segs;

	int				NumSubsectors;
	subsector_t		*Subsectors;

	int				NumNodes;
	node_t			*Nodes;

	byte			*VisData;
	byte			*NoVis;

	// !!! Used only during level loading
	int				NumThings;
	mthing_t		*Things;

	//
	//	BLOCKMAP
	//	Created from axis aligned bounding box of the map, a rectangular
	// array of blocks of size ...
	// Used to speed up collision detection by spatial subdivision in 2D.
	//
	short			*BlockMapLump;	// offsets in blockmap are from here
	word			*BlockMap;		// int for larger maps
	int				BlockMapWidth;	// Blockmap size.
	int				BlockMapHeight;	// size in mapblocks
	float			BlockMapOrgX;	// origin of block map
	float			BlockMapOrgY;
	VEntity			**BlockLinks;	// for thing chains
	polyblock_t		**PolyBlockMap;

	//
	//	REJECT
	//	For fast sight rejection.
	//	Speeds up enemy AI by skipping detailed LineOf Sight calculation.
	// 	Without special effect, this could be used as a PVS lookup as well.
	//
	byte			*RejectMatrix;

	//	Strife conversations.
	int				NumGenericSpeeches;
	FRogueConSpeech	*GenericSpeeches;

	int				NumLevelSpeeches;
	FRogueConSpeech	*LevelSpeeches;

	//	List of all poly-objects on the level.
	int 			NumPolyObjs;
	polyobj_t		*PolyObjs;

	//	Map loader.
	void LoadMap(const char *MapName);

	subsector_t* PointInSubsector(const TVec &point) const;
	byte *LeafPVS(const subsector_t *ss) const;

private:
	//	Map loaders.
	void LoadVertexes(int Lump, int GLLump);
	void LoadSectors(int Lump);
	void LoadSideDefs(int Lump);
	void LoadLineDefs1(int Lump);
	void LoadLineDefs2(int Lump);
	void LoadGLSegs(int Lump);
	void LoadSubsectors(int Lump);
	void LoadNodes(int Lump);
	void LoadPVS(int Lump);
	void LoadBlockMap(int Lump);
	void LoadThings1(int Lump);
	void LoadThings2(int Lump);

	//	Map loading helpers.
	int FTNumForName(const char *name) const;
	int TFNumForName(const char *name) const;
	void SetupLineSides(line_t *ld) const;

	//	Post-loading routines.
	void GroupLines(void) const;
	void LinkNode(int BSPNum, node_t *pParent) const;
	void ClearBox(float *box) const;
	void AddToBox(float* box, float x, float y) const;

	//	Loader of the Strife conversations.
	void LoadRogueConScript(const char *LumpName,
		FRogueConSpeech *&SpeechList, int &NumSpeeches) const;

	DECLARE_FUNCTION(PointInSector)
};

struct base_level_t
{
	float		time;
	int			tictime;

	int			totalkills;
	int			totalitems;
	int			totalsecret;    // for intermission

	char		mapname[12];
	char		nextmap[12];
	char		level_name[32];
};

struct sv_level_t:base_level_t
{
	// Maintain single and multi player starting spots.
	mthing_t	deathmatchstarts[MAXDEATHMATCHSTARTS];  // Player spawn spots for deathmatch.
	int			numdeathmatchstarts;
	mthing_t	playerstarts[MAX_PLAYER_STARTS * MAXPLAYERS];// Player spawn spots.
};

struct cl_level_t:base_level_t
{
};

// PUBLIC FUNCTION PROTOTYPES ----------------------------------------------

void CalcLine(line_t *line);
void CalcSeg(seg_t *seg);
void SV_LoadLevel(const char *MapName);
void CL_LoadLevel(const char *MapName);
sec_region_t *AddExtraFloor(line_t *line, sector_t *dst);
void SwapPlanes(sector_t *);
void CalcSecMinMaxs(sector_t *sector);

// PUBLIC DATA DECLARATIONS ------------------------------------------------

extern int				GMaxEntities;

extern sv_level_t		level;
extern cl_level_t		cl_level;

extern VLevel*			GLevel;
extern VLevel*			GClLevel;

//**************************************************************************
//
//	$Log$
//	Revision 1.28  2004/12/03 16:15:47  dj_jl
//	Implemented support for extended ACS format scripts, functions, libraries and more.
//
//	Revision 1.27  2003/10/22 06:23:46  dj_jl
//	Increased maximal start spot count
//	
//	Revision 1.26  2003/07/03 18:11:13  dj_jl
//	Moving extrafloors
//	
//	Revision 1.25  2003/03/08 11:33:39  dj_jl
//	Got rid of some warnings.
//	
//	Revision 1.24  2002/09/07 16:31:51  dj_jl
//	Added Level class.
//	
//	Revision 1.23  2002/08/28 16:39:19  dj_jl
//	Implemented sector light color.
//	
//	Revision 1.22  2002/08/24 14:51:50  dj_jl
//	Fixes for large blockmaps.
//	
//	Revision 1.21  2002/07/27 18:10:11  dj_jl
//	Implementing Strife conversations.
//	
//	Revision 1.20  2002/03/28 17:59:54  dj_jl
//	Increased maximal object count.
//	
//	Revision 1.19  2002/01/25 18:08:19  dj_jl
//	Beautification
//	
//	Revision 1.18  2002/01/11 08:09:34  dj_jl
//	Added sector plane swapping
//	
//	Revision 1.17  2002/01/07 12:16:42  dj_jl
//	Changed copyright year
//	
//	Revision 1.16  2001/12/27 17:33:29  dj_jl
//	Removed thinker list
//	
//	Revision 1.15  2001/12/18 19:03:16  dj_jl
//	A lots of work on VObject
//	
//	Revision 1.14  2001/12/12 19:28:49  dj_jl
//	Some little changes, beautification
//	
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
