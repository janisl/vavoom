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

#define MAXPLAYERS		8

// MAXRADIUS is for precalculated sector block boxes
// the spider demon is larger, but we do not have any moving sectors nearby
#define MAXRADIUS		32.0

// mapblocks are used to check movement against lines and things
#define MapBlock(x)		((int)floor(x) >> 7)

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
	float		TopTextureOffset;
	float		BotTextureOffset;
	float		MidTextureOffset;
	float		TopRowOffset;
	float		BotRowOffset;
	float		MidRowOffset;
	int			TopTexture;
	int			BottomTexture;
	int			MidTexture;
	vuint32		Flags;
	int			Light;
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

class VLevel : public VGameObject
{
	DECLARE_CLASS(VLevel, VGameObject, 0)
	NO_DEFAULT_CONSTRUCTOR(VLevel)

	friend class VUdmfParser;

	VName				MapName;

	//	Flags.
	enum
	{
		LF_ForServer	= 0x01,	//	True if this level belongs to the server.
		LF_Extended		= 0x02,	//	True if level was in Hexen format.
		LF_GLNodesV5	= 0x04,	//	True if using version 5 GL nodes.
		LF_TextMap		= 0x08,	//	UDMF format map.
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

	//	Sound environments for sector zones.
	vint32				NumZones;
	vint32*				Zones;

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
	void TickWorld(float);

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

	void SetCameraToTexture(VEntity*, VName, int);

	msecnode_t* AddSecnode(sector_t*, VEntity*, msecnode_t*);
	msecnode_t* DelSecnode(msecnode_t*);
	void DelSectorList();

	int FindSectorFromTag(int, int);
	line_t* FindLine(int, int*);

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
	void LoadSideDefs(int);
	void LoadLineDefs1(int, int, const mapInfo_t&);
	void LoadLineDefs2(int, int, const mapInfo_t&);
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
	void LoadTextMap(int, const mapInfo_t&);

	//	Map loading helpers.
	int FindGLNodes(VName) const;
	int TexNumForName(const char*, int, bool = false) const;
	int TexNumOrColour(const char*, int, bool&, vuint32&) const;
	void CreateSides();
	void FinaliseLines();
	void CreateRepBase();
	void CreateBlockMap();
	void BuildNodes();
	void HashSectors();
	void HashLines();

	//	Post-loading routines.
	void GroupLines() const;
	void LinkNode(int, node_t*) const;
	void ClearBox(float*) const;
	void AddToBox(float*, float, float) const;
	void FloodZones();
	void FloodZone(sector_t*, int);

	//	Loader of the Strife conversations.
	void LoadRogueConScript(VName, int, FRogueConSpeech*&, int&) const;

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
	DECLARE_FUNCTION(FindLine)

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

void CalcLine(line_t *line);
void CalcSeg(seg_t *seg);
void SV_LoadLevel(VName MapName);
void CL_LoadLevel(VName MapName);
sec_region_t *AddExtraFloor(line_t *line, sector_t *dst);
void SwapPlanes(sector_t *);
void CalcSecMinMaxs(sector_t *sector);

extern VLevel*			GLevel;
extern VLevel*			GClLevel;
