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

class VRenderLevelPublic;
class VTextureTranslation;
class VAcsLevel;
class VNetContext;

struct sector_t;
struct mapInfo_t;
struct fakefloor_t;
struct seg_t;
struct subsector_t;
struct node_t;
struct drawseg_t;
struct subregion_t;

class	VThinker;
class		VLevelInfo;
class		VEntity;
class	VBasePlayer;
class	VWorldInfo;
class	VGameInfo;

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

//==========================================================================
//
//	Vertex
//
//==========================================================================

//
//	Your plain vanilla vertex.
//	Note: transformed values not buffered locally,
// like some DOOM-alikes ("wt", "WebView") did.
//
typedef TVec vertex_t;

//==========================================================================
//
//	LineDef
//
//==========================================================================

//
//	Move clipping aid for LineDefs.
//
enum
{
	ST_HORIZONTAL,
	ST_VERTICAL,
	ST_POSITIVE,
	ST_NEGATIVE
};

//  If a texture is pegged, the texture will have the end exposed to air held
// constant at the top or bottom of the texture (stairs or pulled down
// things) and will move with a height change of one of the neighbor sectors.
// Unpegged textures allways have the first row of the texture at the top
// pixel of the line for both top and bottom textures (use next to windows).

//
// LineDef attributes.
//
enum
{
	ML_BLOCKING				= 0x00000001,	// Solid, is an obstacle.
	ML_BLOCKMONSTERS		= 0x00000002,	// Blocks monsters only.
	ML_TWOSIDED				= 0x00000004,	// Backside will not be present at all
	ML_DONTPEGTOP			= 0x00000008,	// upper texture unpegged
	ML_DONTPEGBOTTOM		= 0x00000010,	// lower texture unpegged
	ML_SECRET				= 0x00000020,	// don't map as two sided: IT'S A SECRET!
	ML_SOUNDBLOCK			= 0x00000040,	// don't let sound cross two of these
	ML_DONTDRAW				= 0x00000080,	// don't draw on the automap
	ML_MAPPED				= 0x00000100,	// set if already drawn in automap
	ML_REPEAT_SPECIAL		= 0x00000200,	// special is repeatable
	ML_ADDITIVE				= 0x00000400,	//	Additive translucency.
	ML_MONSTERSCANACTIVATE	= 0x00002000,	//	Monsters (as well as players) can activate the line
	ML_BLOCKPLAYERS			= 0x00004000,	//	Blocks players only.
	ML_BLOCKEVERYTHING		= 0x00008000,	//	Line blocks everything.
	ML_ZONEBOUNDARY			= 0x00010000,	//	Boundary of reverb zones.
	ML_RAILING				= 0x00020000,
	ML_BLOCK_FLOATERS		= 0x00040000,
	ML_CLIP_MIDTEX			= 0x00080000,	// Automatic for every Strife line
	ML_WRAP_MIDTEX			= 0x00100000,
	ML_FIRSTSIDEONLY		= 0x00800000,	// Actiavte only when crossed from front side.

	ML_SPAC_SHIFT			= 10,
	ML_SPAC_MASK			= 0x00001c00,
};

//
// Special activation types
//
enum
{
	SPAC_Cross		= 0x0001,	// when player crosses line
	SPAC_Use		= 0x0002,	// when player uses line
	SPAC_MCross		= 0x0004,	// when monster crosses line
	SPAC_Impact		= 0x0008,	// when projectile hits line
	SPAC_Push		= 0x0010,	// when player pushes line
	SPAC_PCross		= 0x0020,	// when projectile crosses line
	SPAC_UseThrough	= 0x0040,	// SPAC_USE, but passes it through
	//	SPAC_PTouch is remapped as SPAC_Impact | SPAC_PCross
	SPAC_AnyCross	= 0x0080,
	SPAC_MUse		= 0x0100,	// when monster uses line
	SPAC_MPush		= 0x0200,	// when monster pushes line
};

struct line_t : public TPlane
{
	// Vertices, from v1 to v2.
	vertex_t*	v1;
	vertex_t*	v2;

	// Precalculated v2 - v1 for side checking.
	TVec		dir;

	// Animation related.
	int			flags;
	int			SpacFlags;

	// Visual appearance: SideDefs.
	//  sidenum[1] will be -1 if one sided
	int			sidenum[2];

	// Neat. Another bounding box, for the extent
	//  of the LineDef.
	float		bbox[4];

	// To aid move clipping.
	int			slopetype;

	// Front and back sector.
	// Note: redundant? Can be retrieved from SideDefs.
	sector_t*	frontsector;
	sector_t*	backsector;

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
	int			HashFirst;
	int			HashNext;
};

//==========================================================================
//
//	SideDef
//
//==========================================================================

enum
{
	SDF_ABSLIGHT		= 0x0001,	//	Light is absolute value.
};

struct side_t
{
	//	Add this to the calculated texture column
	float		TopTextureOffset;
	float		BotTextureOffset;
	float		MidTextureOffset;

	//	Add this to the calculated texture top
	float		TopRowOffset;
	float		BotRowOffset;
	float		MidRowOffset;

	//	Texture indices. We do not maintain names here.
	int			TopTexture;
	int			BottomTexture;
	int			MidTexture;

	//	Sector the SideDef is facing.
	sector_t*	Sector;

	int			LineNum;

	vuint32		Flags;

	int			Light;
};

//==========================================================================
//
//	Sector
//
//==========================================================================

enum
{
	SPF_NOBLOCKING		= 1,	//	Not blocking
	SPF_NOBLOCKSIGHT	= 2,	//	Do not block sight
	SPF_NOBLOCKSHOOT	= 4,	//	Do not block shooting
	SPF_ADDITIVE		= 8,	//	Additive translucency
};

enum
{
	SKY_FROM_SIDE		= 0x8000
};

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
	sec_region_t*	prev;
	sec_region_t*	next;

	//	Planes
	sec_plane_t*	floor;
	sec_plane_t*	ceiling;

	sec_params_t*	params;
	line_t*			extraline;
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
	bool			Visited;// killough 4/4/98, 4/7/98: used in search algorithms
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
		SF_NoFallingDamage	= 0x0200,	//	No falling damage in this sector.
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

	int				Zone;
};

//==========================================================================
//
//	Polyobject
//
//==========================================================================

//
// ===== Polyobj data =====
//
struct polyobj_t
{
	int 			numsegs;
	seg_t**			segs;
	TVec			startSpot;
	vertex_t*		originalPts; 	// used as the base for the rotations
	vertex_t*		prevPts; 		// use to restore the old point values
	float	 		angle;
	int 			tag;			// reference tag assigned in HereticEd
	int				bbox[4];
	int 			validcount;
	enum
	{
		PF_Crush		= 0x01,		// should the polyobj attempt to crush mobjs?
		PF_HurtOnTouch	= 0x02,
	};
	vuint32			PolyFlags;
	int 			seqType;
	subsector_t*	subsector;
	VThinker*		SpecialData;	// pointer a thinker, if the poly is moving
};

//
//
//
struct polyblock_t
{
	polyobj_t*		polyobj;
	polyblock_t*	prev;
	polyblock_t*	next;
};

struct PolyAnchorPoint_t
{
	float			x;
	float			y;
	int				tag;
};

//==========================================================================
//
//	LineSeg
//
//==========================================================================

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

//==========================================================================
//
//	Subsector
//
//==========================================================================

//
// 	A SubSector.
// 	References a Sector. Basically, this is a list of LineSegs, indicating
// the visible walls that define (all or some) sides of a convex BSP leaf.
//
struct subsector_t
{
	sector_t*		sector;
	subsector_t*	seclink;
	int				numlines;
	int				firstline;
	polyobj_t*		poly;

	node_t*			parent;
	int				VisFrame;
	int				SkyVisFrame;

	vuint32			dlightbits;
	int				dlightframe;
	subregion_t*	regions;
};

//==========================================================================
//
//	Node
//
//==========================================================================

//
// Indicate a leaf.
//
enum
{
	NF_SUBSECTOR	= 0x80000000
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

//==========================================================================
//
//	Thing
//
//==========================================================================

//
//	Map thing definition with initialised fields for global use.
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
	int			SkillClassFilter;
	int 		special;
	int 		arg1;
	int 		arg2;
	int 		arg3;
	int 		arg4;
	int 		arg5;
};

//==========================================================================
//
//	Strife conversation scripts
//
//==========================================================================

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

//==========================================================================
//
//	Misc game structs
//
//==========================================================================

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
//	VGameObject
//
//==========================================================================

class VGameObject : public VObject
{
	DECLARE_CLASS(VGameObject, VObject, 0)
	NO_DEFAULT_CONSTRUCTOR(VGameObject)
};
