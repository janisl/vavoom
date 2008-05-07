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

//	All line specials that are used by the engine.
enum
{
	LNSPEC_PolyStartLine = 1,
	LNSPEC_PolyExplicitLine = 5,
	LNSPEC_LineHorizon = 9,
	LNSPEC_DoorLockedRaise = 13,
	LNSPEC_ACSLockedExecute = 83,
	LNSPEC_ACSLockedExecuteDoor = 85,
	LNSPEC_LineMirror = 182,
	LNSPEC_StaticInit = 190,
	LNSPEC_LineTranslucent = 208,
	LNSPEC_TransferHeights = 209,
};

// TYPES -------------------------------------------------------------------

//==========================================================================
//
//								MAP
//
//==========================================================================

class VRenderLevelPublic;
class VTextureTranslation;
class VAcsLevel;
class VNetContext;

struct sector_t;

class	VThinker;
class		VLevelInfo;
class		VEntity;
class	VBasePlayer;
class	VWorldInfo;
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

enum
{
	SDF_ADDITIVE		= 0x0001,	//	Additive translucency.
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

	// Texture indices.
	// We do not maintain names here.
	int			toptexture;
	int			bottomtexture;
	int			midtexture;

	// Sector the SideDef is facing.
	sector_t	*sector;

	int			LineNum;

	vuint32		Flags;
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
#define SPF_ADDITIVE		8	//	Additive translucency

#define SKY_FROM_SIDE		0x8000

struct sec_plane_t : public TPlane
{
	float		minz;
	float		maxz;

	//	Use for wall texture mapping.
	float		TexZ;

	int			pic;

	float		xoffs;
	float		yoffs;

	float		XScale;
	float		YScale;

	float		Angle;

	float		BaseAngle;
	float		BaseYOffs;

	int			flags;
	float		Alpha;
	float		MirrorAlpha;

	int			LightSourceSector;
	VEntity*	SkyBox;
};

struct sec_params_t
{
	int			lightlevel;
	int			LightColour;
	int			Fade;
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
// phares 3/14/98
//
// Sector list node showing all sectors an object appears in.
//
// There are two threads that flow through these nodes. The first thread
// starts at TouchingThingList in a sector_t and flows through the SNext
// links to find all mobjs that are entirely or partially in the sector.
// The second thread starts at TouchingSectorList in a VEntity and flows
// through the TNext links to find all sectors a thing touches. This is
// useful when applying friction or push effects to sectors. These effects
// can be done as thinkers that act upon all objects touching their sectors.
// As an mobj moves through the world, these nodes are created and
// destroyed, with the links changed appropriately.
//
// For the links, NULL means top or end of list.
//
struct msecnode_t
{
	sector_t*		Sector;	// a sector containing this object
	VEntity*		Thing;	// this object
	msecnode_t*		TPrev;	// prev msecnode_t for this thing
	msecnode_t*		TNext;	// next msecnode_t for this thing
	msecnode_t*		SPrev;	// prev msecnode_t for this sector
	msecnode_t*		SNext;	// next msecnode_t for this sector
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

	int				special;
	int				tag;
	int				HashFirst;
	int				HashNext;

	float			skyheight;

	// stone, metal, heavy, etc...
	int				seqType;

	// mapblock bounding box for height changes
	int				blockbox[4];

	// origin for any sounds played by the sector
	TVec			soundorg;

	// if == validcount, already checked
	int				validcount;

	// list of subsectors in sector
	// used to check if client can see this sector (it needs to be updated)
	subsector_t*	subsectors;

	//	List of things in sector.
	VEntity*		ThingList;
	msecnode_t*		TouchingThingList;

	int				linecount;
	line_t**		lines;  // [linecount] size

	//	Boom's fake floors.
	sector_t*		heightsec;
	fakefloor_t*	fakefloors;			//	Info for rendering.

	//	Flags.
	enum
	{
		SF_HasExtrafloors	= 0x0001,	//	This sector has extrafloors.
		SF_ExtrafloorSource	= 0x0002,	//	This sector is a source of an extrafloor.
		SF_TransferSource	= 0x0004,	//	Source of an heightsec or transfer light.
		SF_FakeFloorOnly	= 0x0008,	//	When used as heightsec in R_FakeFlat, only copies floor
		SF_ClipFakePlanes	= 0x0010,	//	As a heightsec, clip planes to target sector's planes
		SF_NoFakeLight		= 0x0020,	//	heightsec does not change lighting
		SF_IgnoreHeightSec	= 0x0040,	//	heightsec is only for triggering sector actions
		SF_UnderWater		= 0x0080,	//	Sector is underwater
		SF_Silent			= 0x0100,	//	Actors don't make noise in this sector.
	};
	vuint32			SectorFlags;

	// 0 = untraversed, 1,2 = sndlines -1
	vint32			soundtraversed;

	// thing that made a sound (or null)
	VEntity*		SoundTarget;

	// Thinker for reversable actions
	VThinker*		FloorData;
	VThinker*		CeilingData;
	VThinker*		LightingData;
	VThinker*		AffectorData;

	//	Sector action triggers.
	VEntity*		ActionList;

	vint32			Damage;

	float			Friction;
	float			MoveFactor;
	float			Gravity;				// Sector gravity (1.0 is normal)

	int				Sky;
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
		PF_Crush		= 0x01,		// should the polyobj attempt to crush mobjs?
		PF_HurtOnTouch	= 0x02,
	};
	vuint32		PolyFlags;
	int 		seqType;
	subsector_t	*subsector;
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

struct VStateCall
{
	VEntity*	Item;
	VState*		State;
	vuint8		Result;
};

//==========================================================================
//
//	Structures for level network replication
//
//==========================================================================

struct rep_line_t
{
	float		alpha;
};

struct rep_side_t
{
	float		textureoffset;
	float		rowoffset;
	int			toptexture;
	int			bottomtexture;
	int			midtexture;
	vuint32		Flags;
};

struct rep_sector_t
{
	int			floor_pic;
	float		floor_dist;
	float		floor_xoffs;
	float		floor_yoffs;
	float		floor_XScale;
	float		floor_YScale;
	float		floor_Angle;
	float		floor_BaseAngle;
	float		floor_BaseYOffs;
	VEntity*	floor_SkyBox;
	float		floor_MirrorAlpha;
	int			ceil_pic;
	float		ceil_dist;
	float		ceil_xoffs;
	float		ceil_yoffs;
	float		ceil_XScale;
	float		ceil_YScale;
	float		ceil_Angle;
	float		ceil_BaseAngle;
	float		ceil_BaseYOffs;
	VEntity*	ceil_SkyBox;
	float		ceil_MirrorAlpha;
	int			lightlevel;
	int			Fade;
	int			Sky;
};

struct rep_polyobj_t
{
	TVec		startSpot;
	float	 	angle;
};

struct rep_light_t
{
	TVec		Origin;
	float		Radius;
	vuint32		Colour;
};

struct VSndSeqInfo
{
	VName			Name;
	vint32			OriginId;
	TVec			Origin;
	vint32			ModeNum;
	TArray<VName>	Choices;
};

struct VCameraTextureInfo
{
	VEntity*		Camera;
	int				TexNum;
	int				FOV;
};

//==========================================================================
//
//									LEVEL
//
//==========================================================================

enum
{
	MAX_LEVEL_TRANSLATIONS	= 0xffff,
	MAX_BODY_QUEUE_TRANSLATIONS = 0xff,
};

class VLevel : public VObject
{
	DECLARE_CLASS(VLevel, VObject, 0)
	NO_DEFAULT_CONSTRUCTOR(VLevel)

	VName				MapName;

	//	Flags.
	enum
	{
		LF_ForServer	= 0x01,	//	True if this level belongs to the server.
		LF_Extended		= 0x02,	//	True if level was in Hexen format.
		LF_GLNodesV5	= 0x04,	//	True if using version 5 GL nodes.
	};
	vuint32				LevelFlags;

	//
	//	MAP related Lookup tables.
	//	Store VERTEXES, LINEDEFS, SIDEDEFS, etc.
	//

	vint32				NumVertexes;
	vertex_t*			Vertexes;

	vint32				NumSectors;
	sector_t*			Sectors;

	vint32				NumSides;
	side_t*				Sides;

	vint32				NumLines;
	line_t*				Lines;

	vint32				NumSegs;
	seg_t*				Segs;

	vint32				NumSubsectors;
	subsector_t*		Subsectors;

	vint32				NumNodes;
	node_t*				Nodes;

	vuint8*				VisData;
	vuint8*				NoVis;

	// !!! Used only during level loading
	vint32				NumThings;
	mthing_t*			Things;

	//
	//	BLOCKMAP
	//	Created from axis aligned bounding box of the map, a rectangular
	// array of blocks of size ...
	// Used to speed up collision detection by spatial subdivision in 2D.
	//
	vint32*				BlockMapLump;	// offsets in blockmap are from here
	vint32*				BlockMap;		// int for larger maps
	vint32				BlockMapWidth;	// Blockmap size.
	vint32				BlockMapHeight;	// size in mapblocks
	float				BlockMapOrgX;	// origin of block map
	float				BlockMapOrgY;
	VEntity**			BlockLinks;		// for thing chains
	polyblock_t**		PolyBlockMap;

	//
	//	REJECT
	//	For fast sight rejection.
	//	Speeds up enemy AI by skipping detailed LineOf Sight calculation.
	// 	Without special effect, this could be used as a PVS lookup as well.
	//
	vuint8*				RejectMatrix;

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

	VLevelInfo*			LevelInfo;
	VWorldInfo*			WorldInfo;

	VAcsLevel*			Acs;

	VRenderLevelPublic*	RenderData;
	VNetContext*		NetContext;

	rep_line_t*			BaseLines;
	rep_side_t*			BaseSides;
	rep_sector_t*		BaseSectors;
	rep_polyobj_t*		BasePolyObjs;

	vint32				NumStaticLights;
	rep_light_t*		StaticLights;

	TArray<VSndSeqInfo>	ActiveSequences;

	TArray<VCameraTextureInfo>	CameraTextures;

	float				Time;
	int					TicTime;

	msecnode_t*			SectorList;
	// phares 3/21/98
	//
	// Maintain a freelist of msecnode_t's to reduce memory allocs and frees.
	msecnode_t*			HeadSecNode;

	//	Translations controlled by ACS scripts.
	TArray<VTextureTranslation*>	Translations;
	TArray<VTextureTranslation*>	BodyQueueTrans;

	VState*				CallingState;
	VStateCall*			StateCall;

	int					NextSoundOriginID;

	void Serialise(VStream& Strm);
	void ClearReferences();
	void Destroy();

	//	Map loader.
	void LoadMap(VName MapName);

	subsector_t* PointInSubsector(const TVec& point) const;
	vuint8* LeafPVS(const subsector_t* ss) const;

	VThinker* SpawnThinker(VClass*, const TVec& = TVec(0, 0, 0),
		const TAVec& = TAVec(0, 0, 0), mthing_t* = NULL,
		bool AllowReplace = true);
	void AddThinker(VThinker*);
	void RemoveThinker(VThinker*);
	void DestroyAllThinkers();

	//	Poly-objects.
	void SpawnPolyobj(float, float, int, bool, bool);
	void AddPolyAnchorPoint(float, float, int);
	void InitPolyobjs();
	polyobj_t* GetPolyobj(int);
	int GetPolyobjMirror(int);
	bool MovePolyobj(int, float, float);
	bool RotatePolyobj(int, float);

	bool ChangeSector(sector_t*, int);

	bool TraceLine(linetrace_t&, const TVec&, const TVec&, int) const;

	void ClampOffsets();

	void SetCameraToTexture(VEntity*, VName, int);

	msecnode_t* AddSecnode(sector_t*, VEntity*, msecnode_t*);
	msecnode_t* DelSecnode(msecnode_t*);
	void DelSectorList();

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
	void LoadCompressedGLNodes(int);
	void LoadBlockMap(int);
	void LoadReject(int);
	void LoadThings1(int);
	void LoadThings2(int);
	void LoadACScripts(int);

	//	Map loading helpers.
	int FindGLNodes(VName) const;
	int TexNumForName(const char*, int, bool = false) const;
	int TexNumOrColour(const char*, int, bool&, vuint32&) const;
	void SetupLineSides(line_t*) const;
	void CreateRepBase();
	void CreateBlockMap();
	void BuildNodes();
	void HashSectors();

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

	int SetBodyQueueTrans(int, int);

	DECLARE_FUNCTION(PointInSector)
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

	//	ACS functions
	DECLARE_FUNCTION(StartACS)
	DECLARE_FUNCTION(SuspendACS)
	DECLARE_FUNCTION(TerminateACS)
	DECLARE_FUNCTION(StartTypedACScripts)

	DECLARE_FUNCTION(SetBodyQueueTrans)
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

extern VLevel*			GLevel;
extern VLevel*			GClLevel;
