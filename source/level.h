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

// MAXRADIUS is for precalculated sector block boxes
// the spider demon is larger, but we do not have any moving sectors nearby
#define MAXRADIUS		32.0

// mapblocks are used to check movement against lines and things
#define MapBlock(x)		((int)floor(x) >> 7)

// TYPES -------------------------------------------------------------------

//==========================================================================
//
//								MAP
//
//==========================================================================

class VLevelRenderDataPublic;

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

	float		alpha;

	int			special;
	int			arg1;
	int			arg2;
	int			arg3;
	int			arg4;
	int			arg5;

	int			LineTag;
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
	float		Alpha;

	int			LightSourceSector;
};

struct sec_params_t
{
	int			lightlevel;
	int			LightColour;
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

	// 0 = untraversed, 1,2 = sndlines -1
	vint32		soundtraversed;

	// thing that made a sound (or null)
	VEntity*	SoundTarget;

	// Thinker for reversable actions
	VThinker*	FloorData;
	VThinker*	CeilingData;
	VThinker*	LightingData;
	VThinker*	AffectorData;

	vint32		Damage;

	float		Friction;
	float		MoveFactor;
	float		Gravity;				// Sector gravity (1.0 is normal)
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
	int			changed;
	VThinker*	SpecialData;	// pointer a thinker, if the poly is moving
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

struct PolyAnchorPoint_t
{
	float		x;
	float		y;
	int			tag;
};

//
// BSP node.
//
struct node_t : public TPlane
{
	// Bounding box for each child.
	float		bbox[2][6];

	// If NF_SUBSECTOR its a subsector.
	vuint32		children[2];

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

	vuint32		dlightbits;
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
	vint32		GiveItem;	//	Item given on success
	vint32		NeedItem1;	//	Required item 1
	vint32		NeedItem2;	//	Required item 2
	vint32		NeedItem3;	//	Required item 3
	vint32		NeedAmount1;//	Amount of item 1
	vint32		NeedAmount2;//	Amount of item 2
	vint32		NeedAmount3;//	Amount of item 3
	VStr		Text;		//	Text of the answer
	VStr		TextOK;		//	Message displayed on success
	vint32		Next;		//	Dialog to go on success, negative values
							// to go here immediately
	vint32		Objectives;	//	Mission objectives, LOGxxxx lump
	VStr		TextNo;		//	Message displayed on failure (player doesn't
							// have needed thing, it haves enough health/ammo,
							// item is not ready, quest is not completed)
};

struct FRogueConSpeech
{
	vint32		SpeakerID;	//	Type of the object (MT_xxx)
	vint32		DropItem;	//	Item dropped when killed
	vint32		CheckItem1;	//	Item 1 to check for jump
	vint32		CheckItem2;	//	Item 2 to check for jump
	vint32		CheckItem3;	//	Item 3 to check for jump
	vint32		JumpToConv;	//	Jump to conversation if have certain item(s)
	VStr		Name;		//	Name of the character
	VName		Voice;		//	Voice to play
	VName		BackPic;	//	Picture of the speaker
	VStr		Text;		//	Message
	FRogueConChoice	Choices[5];	//	Choices
};

enum
{
	PT_ADDLINES		= 1,
	PT_ADDTHINGS	= 2,
	PT_EARLYOUT		= 4,
};

struct intercept_t
{
	float		frac;		// along trace line
	enum
	{
		IF_IsALine = 0x01,
	};
	vuint32		Flags;
	VEntity*	thing;
	line_t*		line;
};

struct linetrace_t
{
	TVec		Start;
	TVec		End;
	TVec		Delta;
	TPlane		Plane;			// from t1 to t2
	TVec		LineStart;
	TVec		LineEnd;
	vuint32		PlaneNoBlockFlags;
	TVec		HitPlaneNormal;
	bool		SightEarlyOut;
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
		//	From mapinfo.
		LF_Map07Special					= 0x00000008,
		LF_BaronSpecial					= 0x00000010,
		LF_CyberDemonSpecial			= 0x00000020,
		LF_SpiderMastermindSpecial		= 0x00000040,
		LF_MinotaurSpecial				= 0x00000080,
		LF_DSparilSpecial				= 0x00000100,
		LF_IronLichSpecial				= 0x00000200,
		LF_SpecialActionOpenDoor		= 0x00000400,
		LF_SpecialActionLowerFloor		= 0x00000800,
		LF_SpecialActionKillMonsters	= 0x00001000,
		LF_AllowMonsterTelefrags		= 0x00002000,
		LF_NoAllies						= 0x00004000,
		LF_DeathSlideShow				= 0x00008000,
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

	//	List of all poly-objects on the level
	vint32				NumPolyObjs;
	polyobj_t*			PolyObjs;

	//	Anchor points of poly-objects
	vint32				NumPolyAnchorPoints;
	PolyAnchorPoint_t*	PolyAnchorPoints;

	VThinker*			ThinkerHead;
	VThinker*			ThinkerTail;

	VLevelRenderDataPublic*	RenderData;

	void Serialise(VStream& Strm);
	void ClearReferences();
	void Destroy();

	//	Map loader.
	void LoadMap(VName MapName);

	subsector_t* PointInSubsector(const TVec& point) const;
	vuint8* LeafPVS(const subsector_t* ss) const;

	void AddThinker(VThinker*);
	void RemoveThinker(VThinker*);
	void DestroyAllThinkers();

	//	Poly-objects.
	void SpawnPolyobj(float, float, int, int);
	void AddPolyAnchorPoint(float, float, int);
	void InitPolyobjs();
	polyobj_t* GetPolyobj(int);
	int GetPolyobjMirror(int);
	bool MovePolyobj(int, float, float);
	bool RotatePolyobj(int, float);

	bool BlockLinesIterator(int, int, bool(*func)(void*, line_t*), void*);
	bool BlockThingsIterator(int, int, bool(*func)(void*, VEntity*),
		void*, VObject*, VMethod*);
	bool PathTraverse(float, float, float, float, int,
		bool(*trav)(void*, intercept_t *), void*, VObject*, VMethod*);
	bool ChangeSector(sector_t*, int);

	bool TraceLine(linetrace_t&, const TVec&, const TVec&, int) const;

	bool IsForServer() const
	{
		return !!(LevelFlags & LF_ForServer);
	}
	bool IsForClient() const
	{
		return !(LevelFlags & LF_ForServer);
	}

private:
	//	Map loaders.
	void LoadVertexes(int, int, int&);
	void LoadSectors(int);
	void LoadSideDefsPass1(int);
	void LoadSideDefsPass2(int);
	void LoadLineDefs1(int, int);
	void LoadLineDefs2(int, int);
	void LoadGLSegs(int, int);
	void LoadSubsectors(int);
	void LoadNodes(int);
	void LoadPVS(int);
	void LoadBlockMap(int);
	void LoadReject(int);
	void LoadThings1(int);
	void LoadThings2(int);

	//	Map loading helpers.
	int FindGLNodes(VName) const;
	int TexNumForName(const char*, int, bool = false) const;
	void SetupLineSides(line_t*) const;

	//	Post-loading routines.
	void GroupLines() const;
	void LinkNode(int, node_t*) const;
	void ClearBox(float*) const;
	void AddToBox(float*, float, float) const;

	//	Loader of the Strife conversations.
	void LoadRogueConScript(VName, FRogueConSpeech*&, int&) const;

	//	Internal poly-object methods
	void IterFindPolySegs(const TVec&, seg_t**, int&, const TVec&);
	void TranslatePolyobjToStartSpot(float, float, int);
	void UpdatePolySegs(polyobj_t*);
	void InitPolyBlockMap();
	void LinkPolyobj(polyobj_t*);
	void UnLinkPolyobj(polyobj_t*);
	bool PolyCheckMobjBlocking(seg_t*, polyobj_t*);

	//	Internal TraceLine methods
	bool CheckPlane(linetrace_t&, const sec_plane_t*) const;
	bool CheckPlanes(linetrace_t&, sector_t*) const;
	bool CheckLine(linetrace_t&, seg_t*) const;
	bool CrossSubsector(linetrace_t&, int) const;
	bool CrossBSPNode(linetrace_t&, int) const;

	DECLARE_FUNCTION(PointInSector)
	DECLARE_FUNCTION(BlockThingsIterator)
	DECLARE_FUNCTION(PathTraverse)
	DECLARE_FUNCTION(TraceLine)
	DECLARE_FUNCTION(ChangeSector)
	DECLARE_FUNCTION(AddExtraFloor)
	DECLARE_FUNCTION(SwapPlanes)

	DECLARE_FUNCTION(SetFloorPic)
	DECLARE_FUNCTION(SetCeilPic)
	DECLARE_FUNCTION(SetLineTexture)
	DECLARE_FUNCTION(SetLineAlpha)
	DECLARE_FUNCTION(SetFloorLightSector)
	DECLARE_FUNCTION(SetCeilingLightSector)
	DECLARE_FUNCTION(SetHeightSector)

	DECLARE_FUNCTION(FindSectorFromTag)

	//	Polyobj functions
	DECLARE_FUNCTION(SpawnPolyobj)
	DECLARE_FUNCTION(AddPolyAnchorPoint)
	DECLARE_FUNCTION(GetPolyobj)
	DECLARE_FUNCTION(GetPolyobjMirror)
	DECLARE_FUNCTION(MovePolyobj)
	DECLARE_FUNCTION(RotatePolyobj)
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

	VName		MapName;
	int			levelnum;
	int			cluster;
	int			partime;
	int			sucktime;
	VStr		LevelName;

	int			sky1Texture;
	int			sky2Texture;
	float		sky1ScrollDelta;
	float		sky2ScrollDelta;
	int			doubleSky;
	int			lightning;
	VName		SkyBox;
	VName		FadeTable;

	int			cdTrack;
	VName		SongLump;
};

// PUBLIC FUNCTION PROTOTYPES ----------------------------------------------

void CalcLine(line_t *line);
void CalcSeg(seg_t *seg);
void SV_LoadLevel(VName MapName);
void CL_LoadLevel(VName MapName);
sec_region_t *AddExtraFloor(line_t *line, sector_t *dst);
void SwapPlanes(sector_t *);
void CalcSecMinMaxs(sector_t *sector);

// PUBLIC DATA DECLARATIONS ------------------------------------------------

extern int				GMaxEntities;

extern level_t			level;
extern level_t			cl_level;

extern VLevel*			GLevel;
extern VLevel*			GClLevel;
