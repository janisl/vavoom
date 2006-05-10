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

class	VThinker;
class		VLevelInfo;
class		VEntity;
class	VBasePlayer;
class	VGameInfo;

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

	int			LightSourceSector;
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
struct fakefloor_t;
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

	//	Boom's fake floors.
	sector_t*	heightsec;
	fakefloor_t*	fakefloors;			//	Info for rendering.

	//	Flags.
	enum
	{
		SF_HasExtrafloors	= 0x0001,	//	This sector has extrafloors.
		SF_ExtrafloorSource	= 0x0002,	//	This sector is a source of an extrafloor.
		SF_FakeFloorOnly	= 0x0004,	//	When used as heightsec in R_FakeFlat, only copies floor
		SF_ClipFakePlanes	= 0x0008,	//	As a heightsec, clip planes to target sector's planes
		SF_NoFakeLight		= 0x0010,	//	heightsec does not change lighting
		SF_IgnoreHeightSec	= 0x0020,	//	heightsec is only for triggering sector actions
		SF_UnderWater		= 0x0040,	//	Sector is underwater
	};
	vuint32		SectorFlags;

	int			user_fields[14];
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
	enum
	{
		PF_Crush	= 0x01,		// should the polyobj attempt to crush mobjs?
	};
	vuint32		PolyFlags;
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
	dword		children[2];

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
	enum
	{
		LF_ForServer	= 0x01,	//	True if this level belongs to the server.
		LF_Extended		= 0x02,	//	True if level was in Hexen format.
		LF_GLNodesV5	= 0x04,	//	True if using version 5 GL nodes.
	};
	vuint32			LevelFlags;

	//
	//	MAP related Lookup tables.
	//	Store VERTEXES, LINEDEFS, SIDEDEFS, etc.
	//

	vint32			NumVertexes;
	vertex_t*		Vertexes;

	vint32			NumSectors;
	sector_t*		Sectors;

	vint32			NumSides;
	side_t*			Sides;

	vint32			NumLines;
	line_t*			Lines;

	vint32			NumSegs;
	seg_t*			Segs;

	vint32			NumSubsectors;
	subsector_t*	Subsectors;

	vint32			NumNodes;
	node_t*			Nodes;

	vuint8*			VisData;
	vuint8*			NoVis;

	// !!! Used only during level loading
	vint32			NumThings;
	mthing_t*		Things;

	//
	//	BLOCKMAP
	//	Created from axis aligned bounding box of the map, a rectangular
	// array of blocks of size ...
	// Used to speed up collision detection by spatial subdivision in 2D.
	//
	vint16*			BlockMapLump;	// offsets in blockmap are from here
	vuint16*		BlockMap;		// int for larger maps
	vint32			BlockMapWidth;	// Blockmap size.
	vint32			BlockMapHeight;	// size in mapblocks
	float			BlockMapOrgX;	// origin of block map
	float			BlockMapOrgY;
	VEntity**		BlockLinks;		// for thing chains
	polyblock_t**	PolyBlockMap;

	//
	//	REJECT
	//	For fast sight rejection.
	//	Speeds up enemy AI by skipping detailed LineOf Sight calculation.
	// 	Without special effect, this could be used as a PVS lookup as well.
	//
	vuint8*			RejectMatrix;

	//	Strife conversations.
	vint32				NumGenericSpeeches;
	FRogueConSpeech*	GenericSpeeches;

	vint32				NumLevelSpeeches;
	FRogueConSpeech*	LevelSpeeches;

	//	List of all poly-objects on the level.
	vint32			NumPolyObjs;
	polyobj_t*		PolyObjs;

	VThinker*		ThinkerHead;
	VThinker*		ThinkerTail;

	void Serialise(VStream& Strm);
	void Destroy();

	//	Map loader.
	void LoadMap(VName MapName);

	subsector_t* PointInSubsector(const TVec& point) const;
	vuint8* LeafPVS(const subsector_t* ss) const;

	void AddThinker(VThinker*);
	void RemoveThinker(VThinker*);
	void DestroyAllThinkers();

private:
	//	Map loaders.
	void LoadVertexes(int Lump, int GLLump, int& NumBaseVerts);
	void LoadSectors(int Lump);
	void LoadSideDefsPass1(int Lump);
	void LoadSideDefsPass2(int Lump);
	void LoadLineDefs1(int Lump, int NumBaseVerts);
	void LoadLineDefs2(int Lump, int NumBaseVerts);
	void LoadGLSegs(int Lump, int NumBaseVerts);
	void LoadSubsectors(int Lump);
	void LoadNodes(int Lump);
	void LoadPVS(int Lump);
	void LoadBlockMap(int Lump);
	void LoadReject(int Lump);
	void LoadThings1(int Lump);
	void LoadThings2(int Lump);

	//	Map loading helpers.
	int FindGLNodes(VName name) const;
	int TexNumForName(const char* name, int Type, bool CMap = false) const;
	void SetupLineSides(line_t* ld) const;

	//	Post-loading routines.
	void GroupLines() const;
	void LinkNode(int BSPNum, node_t* pParent) const;
	void ClearBox(float* box) const;
	void AddToBox(float* box, float x, float y) const;

	//	Loader of the Strife conversations.
	void LoadRogueConScript(VName LumpName, FRogueConSpeech*& SpeechList,
		int& NumSpeeches) const;

	DECLARE_FUNCTION(PointInSector)
};

struct level_t
{
	float		time;
	int			tictime;

	int			totalkills;
	int			totalitems;
	int			totalsecret;    // for intermission
	int			currentkills;
	int			currentitems;
	int			currentsecret;

	char		mapname[12];
	int			levelnum;
	int			cluster;
	int			partime;
	char		level_name[32];

	int			sky1Texture;
	int			sky2Texture;
	float		sky1ScrollDelta;
	float		sky2ScrollDelta;
	boolean		doubleSky;
	boolean		lightning;
	char		skybox[32];
	char		fadetable[12];

	int			cdTrack;
	char		songLump[12];
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

extern level_t			level;
extern level_t			cl_level;

extern VLevel*			GLevel;
extern VLevel*			GClLevel;
