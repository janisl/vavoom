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

// HEADER FILES ------------------------------------------------------------

#include "gamedefs.h"
#include "sv_local.h"

// MACROS ------------------------------------------------------------------

#define GRAVITY			1225.0

// TYPES -------------------------------------------------------------------

struct state_t
{
	int sprite;
	int frame;
	int model_index;
	int model_frame;
	float time;
	int statenum;
	int nextstate;
	FFunction *function;
};

class VEntity:public VMapObject
{
	DECLARE_CLASS(VEntity, VMapObject, 0)
	NO_DEFAULT_CONSTRUCTOR(VEntity)

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

	//  Params
	float Mass;

	//  Water
	int WaterLevel;
	int WaterType;

	static int FIndex_Remove;
	static int FIndex_Touch;
	static int FIndex_BlockedByLine;
	static int FIndex_ApplyFriction;

	void eventRemove(void)
	{
		svpr.Exec(GetVFunction(FIndex_Remove), (int)this);
	}
	boolean eventTouch(VMapObject *Other)
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

	boolean SetState(int state);

	boolean CheckWater(void);
	boolean CheckPosition(TVec Pos);
	boolean CheckRelPosition(TVec Pos);
	void UpdateVelocity(void);
	void FakeZMovement(void);
	VEntity *CheckOnmobj(void);

	DECLARE_FUNCTION(SetState)
	DECLARE_FUNCTION(PlaySound)
	DECLARE_FUNCTION(PlayFullVolumeSound)
	DECLARE_FUNCTION(CheckWater)
	DECLARE_FUNCTION(CheckPosition)
	DECLARE_FUNCTION(CheckRelPosition)
	DECLARE_FUNCTION(UpdateVelocity)
	DECLARE_FUNCTION(CheckOnmobj)
};

// EXTERNAL FUNCTION PROTOTYPES --------------------------------------------

// PUBLIC FUNCTION PROTOTYPES ----------------------------------------------

// PRIVATE FUNCTION PROTOTYPES ---------------------------------------------

static state_t	*GStates;
static FName	*GSpriteNames;

// EXTERNAL DATA DECLARATIONS ----------------------------------------------

// PUBLIC DATA DEFINITIONS -------------------------------------------------

IMPLEMENT_CLASS(VEntity);

int VEntity::FIndex_Touch;
int VEntity::FIndex_BlockedByLine;
int VEntity::FIndex_ApplyFriction;
int VEntity::FIndex_Remove;

// PRIVATE DATA DEFINITIONS ------------------------------------------------

// CODE --------------------------------------------------------------------

//==========================================================================
//
//  VEntity::SetState
//
//  Returns true if the actor is still present.
//
//==========================================================================

boolean VEntity::SetState(int state)
{
	guard(VEntity::SetState);
	state_t *st;

	do
	{
		if (!state)
		{
			// Remove mobj
			StateNum = 0;
			eventRemove();
			return false;
		}

		st = &GStates[state];
		StateTime = st->time;
		SpriteIndex = st->sprite;
		SpriteName = GSpriteNames[st->sprite];
		SpriteFrame = st->frame;
		if (!bFixedModel)
			ModelIndex = st->model_index;
		ModelFrame = st->model_frame;
		StateNum = st->statenum;
		NextState = st->nextstate;

		// Modified handling.
		// Call action functions when the state is set
		svpr.Exec(st->function, (int)this);

		state = st->nextstate;
	}
	while (!StateTime);
	return true;
	unguard;
}

//==========================================================================
//
//  VEntity::CheckWater
//
//==========================================================================

boolean VEntity::CheckWater(void)
{
	guard(VEntity::CheckWater);
	TVec point;
	int cont;

	point = Origin;
	point.z += 1.0;

	WaterLevel = 0;
	WaterType = 0;
	cont = SV_PointContents(Sector, point);
	if (cont > 0)
	{
		WaterType = cont;
		WaterLevel = 1;
		point.z = Origin.z + Height * 0.5;
		cont = SV_PointContents(Sector, point);
		if (cont > 0)
		{
			WaterLevel = 2;
			if (bIsPlayer)
			{
				point = Player->ViewOrg;
				cont = SV_PointContents(Sector, point);
				if (cont > 0)
					WaterLevel = 3;
			}
		}
	}
	return WaterLevel > 1;
	unguard;
}

//**************************************************************************
//
//	CHECK ABSOLUTE POSITION
//
//**************************************************************************

struct cptrace_t
{
	VEntity *Thing;
	TVec Pos;
	float bbox[4];
	float FloorZ;
	float CeilingZ;
	float DropOffZ;
	sec_plane_t *Floor;
	sec_plane_t *Ceiling;
};

cptrace_t cptrace;

//==========================================================================
//
//	PIT_CheckThing
//
//==========================================================================

boolean PIT_CheckThing(VMapObject *Other)
{
	float blockdist;

	if (!Other->bSolid)
		return true;

	blockdist = Other->Radius + cptrace.Thing->Radius;

	if (fabs(Other->Origin.x - cptrace.Pos.x) >= blockdist ||
		fabs(Other->Origin.y - cptrace.Pos.y) >= blockdist)
	{
		// didn't hit it
		return true;
	}

	// don't clip against self
	if (Other == cptrace.Thing)
		return true;

	if (!cptrace.Thing->bNoPassMobj)
	{
		// check if a mobj passed over/under another object
/*!		if ((cptrace.Thing.Class == Imp || cptrace.Thing.Class == Wizard) &&
			(Other.Class == Imp || Other.Class == Wizard))
		{
			// don't let imps/wizards fly over other imps/wizards
			return false;
		}*/
/*!		if (cptrace.Thing.Class == Bishop && Other.Class == Bishop)
		{
			// don't let bishops fly over other bishops
			return false;
		}*/
		if (cptrace.Pos.z >= Other->Origin.z + Other->Height)
		{
			return true;
		}
		if (cptrace.Pos.z + cptrace.Thing->Height < Other->Origin.z)
		{
			// under thing
			return true;
		}
	}

	return false;
}

//==========================================================================
//
//	PIT_CheckLine
//
//  Adjusts cptrace.FoorZ and cptrace.CeilingZ as lines are contacted
//
//==========================================================================

boolean PIT_CheckLine(line_t * ld)
{
	TVec hit_point;
	opening_t *open;

	if (cptrace.bbox[BOXRIGHT] <= ld->bbox[BOXLEFT] ||
		cptrace.bbox[BOXLEFT] >= ld->bbox[BOXRIGHT] ||
		cptrace.bbox[BOXTOP] <= ld->bbox[BOXBOTTOM] ||
		cptrace.bbox[BOXBOTTOM] >= ld->bbox[BOXTOP])
		return true;

	if (P_BoxOnLineSide(&cptrace.bbox[0], ld) != -1)
		return true;

	// A line has been hit
	if (!ld->backsector)
	{
		// One sided line
		return false;
	}

	if (cptrace.Thing->bCheckLineBlocking && ld->flags & ML_BLOCKING)
	{
		// Explicitly blocking everything
		return false;
	}

	if (cptrace.Thing->bCheckLineBlockMonsters && ld->flags & ML_BLOCKMONSTERS)
	{
		// Block monsters only
		return false;
	}

	// set openrange, opentop, openbottom
	hit_point = cptrace.Pos - (DotProduct(cptrace.Pos, ld->normal) -
		ld->dist) * ld->normal;
	open = SV_LineOpenings(ld, hit_point);
	open = SV_FindOpening(open, cptrace.Pos.z, 
		cptrace.Pos.z + cptrace.Thing->Height);

	if (open)
	{
		// adjust floor / ceiling heights
		if (!(open->ceiling->flags & SPF_NOBLOCKING)
			&& open->top < cptrace.CeilingZ)
		{
			cptrace.Ceiling = open->ceiling;
			cptrace.CeilingZ = open->top;
		}

		if (!(open->floor->flags & SPF_NOBLOCKING) && open->bottom > cptrace.FloorZ)
		{
			cptrace.Floor = open->floor;
			cptrace.FloorZ = open->bottom;
		}

		if (open->lowfloor < cptrace.DropOffZ)
			cptrace.DropOffZ = open->lowfloor;
	}
	else
	{
		cptrace.CeilingZ = cptrace.FloorZ;
	}

	return true;
}

//==========================================================================
//
//  VEntity::CheckPosition
//
//  This is purely informative, nothing is modified
// 
// in:
//  a mobj_t (can be valid or invalid)
//  a position to be checked
//   (doesn't need to be related to the mobj_t->x,y)
//
//==========================================================================

boolean VEntity::CheckPosition(TVec Pos)
{
	guard(VEntity::CheckPosition);
	int xl;
	int xh;
	int yl;
	int yh;
	int bx;
	int by;
	subsector_t *newsubsec;
	sec_region_t *gap;
	sec_region_t *reg;

	cptrace.Thing = this;

	cptrace.Pos = Pos;

	cptrace.bbox[BOXTOP] = Pos.y + Radius;
	cptrace.bbox[BOXBOTTOM] = Pos.y - Radius;
	cptrace.bbox[BOXRIGHT] = Pos.x + Radius;
	cptrace.bbox[BOXLEFT] = Pos.x - Radius;

	newsubsec = SV_PointInSubsector(Pos.x, Pos.y);

	// The base floor / ceiling is from the subsector that contains the point.
	// Any contacted lines the step closer together will adjust them.
	gap = SV_FindThingGap(newsubsec->sector->botregion, Pos,
		Pos.z, Pos.z + Height);
	reg = gap;
	while (reg->prev && reg->floor->flags & SPF_NOBLOCKING)
		reg = reg->prev;
	cptrace.Floor = reg->floor;
	cptrace.FloorZ = reg->floor->GetPointZ(Pos);
	cptrace.DropOffZ = cptrace.FloorZ;
	reg = gap;
	while (reg->next && reg->ceiling->flags & SPF_NOBLOCKING)
		reg = reg->next;
	cptrace.Ceiling = reg->ceiling;
	cptrace.CeilingZ = reg->ceiling->GetPointZ(Pos);

	validcount++;

	// Check things first, possibly picking things up.
	// The bounding box is extended by MAXRADIUS
	// because mobj_ts are grouped into mapblocks
	// based on their origin point, and can overlap
	// into adjacent blocks by up to MAXRADIUS units.
	xl = MapBlock(cptrace.bbox[BOXLEFT] - level.bmaporgx - MAXRADIUS);
	xh = MapBlock(cptrace.bbox[BOXRIGHT] - level.bmaporgx + MAXRADIUS);
	yl = MapBlock(cptrace.bbox[BOXBOTTOM] - level.bmaporgy - MAXRADIUS);
	yh = MapBlock(cptrace.bbox[BOXTOP] - level.bmaporgy + MAXRADIUS);

	for (bx = xl; bx <= xh; bx++)
		for (by = yl; by <= yh; by++)
			if (!SV_BlockThingsIterator(bx, by, PIT_CheckThing, NULL))
				return false;

	// check lines
	xl = MapBlock(cptrace.bbox[BOXLEFT] - level.bmaporgx);
	xh = MapBlock(cptrace.bbox[BOXRIGHT] - level.bmaporgx);
	yl = MapBlock(cptrace.bbox[BOXBOTTOM] - level.bmaporgy);
	yh = MapBlock(cptrace.bbox[BOXTOP] - level.bmaporgy);

	for (bx = xl; bx <= xh; bx++)
		for (by = yl; by <= yh; by++)
			if (!SV_BlockLinesIterator(bx, by, PIT_CheckLine))
				return false;

	return true;
	unguard;
}

//**************************************************************************
//
//  MOVEMENT CLIPPING
//
//**************************************************************************

struct tmtrace_t
{
	VEntity *Thing;
	TVec End;
	float BBox[4];
	float FloorZ;
	float CeilingZ;
	float DropOffZ;
	sec_plane_t *Floor;
	sec_plane_t *Ceiling;

	// keep track of the line that lowers the ceiling,
	// so missiles don't explode against sky hack walls
	line_t *CeilingLine;

	// keep track of special lines as they are hit,
	// but don't process them until the move is proven valid
	#define MAXSPECIALCROSS	32
	line_t *SpecHit[MAXSPECIALCROSS];
	int NumSpecHit;

	VMapObject *BlockingMobj;
};

tmtrace_t tmtrace;

//==========================================================================
//
//	PIT_CheckRelThing
//
//==========================================================================

boolean PIT_CheckRelThing(VMapObject *Other)
{
	float blockdist;

	blockdist = Other->Radius + tmtrace.Thing->Radius;

	if (fabs(Other->Origin.x - tmtrace.End.x) >= blockdist ||
		fabs(Other->Origin.y - tmtrace.End.y) >= blockdist)
	{
		// didn't hit it
		return true;
	}

	// don't clip against self
	if (Other == tmtrace.Thing)
		return true;

	//if (!tmtrace.Thing.bNoPassMobj || Actor(Other).bSpecial)
	if (!tmtrace.Thing->bNoPassMobj)
	{
		// check if a mobj passed over/under another object
/*		if ((tmtrace.Thing.Class == Imp || tmtrace.Thing.Class == Wizard)
			&& (Other.Class == Imp || Other.Class == Wizard))
		{
			// don't let imps/wizards fly over other imps/wizards
			return false;
		}*/
/*		if (tmtrace.Thing.Class == Bishop && Other.Class == Bishop)
		{
			// don't let bishops fly over other bishops
			return false;
		}*/
		if (tmtrace.End.z > Other->Origin.z + Other->Height)
		{
			return true;	// overhead
		}
		if (tmtrace.End.z + tmtrace.Thing->Height < Other->Origin.z)
		{
			return true;	// underneath
		}
	}

	tmtrace.BlockingMobj = Other;
	return tmtrace.Thing->eventTouch(Other);
}

//==========================================================================
//
//	PIT_CheckRelLine
//
//  Adjusts tmtrace.FloorZ and tmtrace.CeilingZ as lines are contacted
//
//==========================================================================

boolean PIT_CheckRelLine(line_t * ld)
{
	TVec hit_point;
	opening_t *open;

	if (tmtrace.BBox[BOXRIGHT] <= ld->bbox[BOXLEFT] ||
		tmtrace.BBox[BOXLEFT] >= ld->bbox[BOXRIGHT] ||
		tmtrace.BBox[BOXTOP] <= ld->bbox[BOXBOTTOM] ||
		tmtrace.BBox[BOXBOTTOM] >= ld->bbox[BOXTOP])
		return true;

	if (P_BoxOnLineSide(&tmtrace.BBox[0], ld) != -1)
		return true;

	// A line has been hit

	// The moving thing's destination position will cross
	// the given line.
	// If this should not be allowed, return false.
	// If the line is special, keep track of it
	// to process later if the move is proven ok.
	// NOTE: specials are NOT sorted by order,
	// so two special lines that are only 8 pixels apart
	// could be crossed in either order.

	if (!ld->backsector)
	{
		// One sided line
		tmtrace.Thing->eventBlockedByLine(ld);
		return false;
	}

	if (tmtrace.Thing->bCheckLineBlocking && ld->flags & ML_BLOCKING)
	{
		// Explicitly blocking everything
		tmtrace.Thing->eventBlockedByLine(ld);
		return false;
	}

	if (tmtrace.Thing->bCheckLineBlockMonsters && ld->flags & ML_BLOCKMONSTERS)
	{
		// Block monsters only
		tmtrace.Thing->eventBlockedByLine(ld);
		return false;
	}

	// set openrange, opentop, openbottom
	hit_point = tmtrace.End - (DotProduct(tmtrace.End, ld->normal) -
		ld->dist) * ld->normal;
	open = SV_LineOpenings(ld, hit_point);
	open = SV_FindOpening(open, tmtrace.End.z,
		tmtrace.End.z + tmtrace.Thing->Height);

	if (open)
	{
		// adjust floor / ceiling heights
		if (!(open->ceiling->flags & SPF_NOBLOCKING) &&
			open->top < tmtrace.CeilingZ)
		{
			tmtrace.Ceiling = open->ceiling;
			tmtrace.CeilingZ = open->top;
			tmtrace.CeilingLine = ld;
		}

		if (!(open->floor->flags & SPF_NOBLOCKING) &&
			open->bottom > tmtrace.FloorZ)
		{
			tmtrace.Floor = open->floor;
			tmtrace.FloorZ = open->bottom;
		}

		if (open->lowfloor < tmtrace.DropOffZ)
			tmtrace.DropOffZ = open->lowfloor;
	}
	else
	{
		tmtrace.CeilingZ = tmtrace.FloorZ;
	}

	// if contacted a special line, add it to the list
	if (ld->special)
	{
		if (tmtrace.NumSpecHit >= MAXSPECIALCROSS)
		{
			//cond << "Spechit overflow\n";
		}
		else
		{
			tmtrace.SpecHit[tmtrace.NumSpecHit] = ld;
			tmtrace.NumSpecHit++;
		}
	}

	return true;
}

//==========================================================================
//
//  VEntity::CheckRelPosition
//
//  This is purely informative, nothing is modified
// (except things picked up).
// 
// in:
//  a mobj_t (can be valid or invalid)
//  a position to be checked
//   (doesn't need to be related to the mobj_t->x,y)
//
// during:
//  special things are touched if MF_PICKUP
//  early out on solid lines?
//
// out:
//  newsubsec
//  floorz
//  ceilingz
//  tmdropoffz
//   the lowest point contacted
//   (monsters won't move to a dropoff)
//  speciallines[]
//  numspeciallines
//
//==========================================================================

boolean VEntity::CheckRelPosition(TVec Pos)
{
	guard(VEntity::CheckRelPosition);
	int xl;
	int xh;
	int yl;
	int yh;
	int bx;
	int by;
	subsector_t *newsubsec;
	sec_region_t *gap;
	sec_region_t *reg;

	tmtrace.Thing = this;

	tmtrace.End = Pos;

	tmtrace.BBox[BOXTOP] = Pos.y + Radius;
	tmtrace.BBox[BOXBOTTOM] = Pos.y - Radius;
	tmtrace.BBox[BOXRIGHT] = Pos.x + Radius;
	tmtrace.BBox[BOXLEFT] = Pos.x - Radius;

	newsubsec = SV_PointInSubsector(Pos.x, Pos.y);
	tmtrace.CeilingLine = NULL;

	// The base floor / ceiling is from the subsector
	// that contains the point.
	// Any contacted lines the step closer together
	// will adjust them.
	gap = SV_FindThingGap(newsubsec->sector->botregion, tmtrace.End,
		tmtrace.End.z, tmtrace.End.z + Height);
	reg = gap;
	while (reg->prev && reg->floor->flags & SPF_NOBLOCKING)
		reg = reg->prev;
	tmtrace.Floor = reg->floor;
	tmtrace.FloorZ = reg->floor->GetPointZ(tmtrace.End);
	tmtrace.DropOffZ = tmtrace.FloorZ;
	reg = gap;
	while (reg->next && reg->ceiling->flags & SPF_NOBLOCKING)
		reg = reg->next;
	tmtrace.Ceiling = reg->ceiling;
	tmtrace.CeilingZ = reg->ceiling->GetPointZ(tmtrace.End);

	validcount++;
	tmtrace.NumSpecHit = 0;

	tmtrace.BlockingMobj = NULL;

	// Check things first, possibly picking things up.
	// The bounding box is extended by MAXRADIUS
	// because mobj_ts are grouped into mapblocks
	// based on their origin point, and can overlap
	// into adjacent blocks by up to MAXRADIUS units.
	if (bColideWithThings)
	{
		xl = MapBlock(tmtrace.BBox[BOXLEFT] - level.bmaporgx - MAXRADIUS);
		xh = MapBlock(tmtrace.BBox[BOXRIGHT] - level.bmaporgx + MAXRADIUS);
		yl = MapBlock(tmtrace.BBox[BOXBOTTOM] - level.bmaporgy - MAXRADIUS);
		yh = MapBlock(tmtrace.BBox[BOXTOP] - level.bmaporgy + MAXRADIUS);

		for (bx = xl; bx <= xh; bx++)
			for (by = yl; by <= yh; by++)
				if (!SV_BlockThingsIterator(bx, by, PIT_CheckRelThing, NULL))
					return false;

		tmtrace.BlockingMobj = NULL;
	}

	// check lines
	if (bColideWithWorld)
	{
		xl = MapBlock(tmtrace.BBox[BOXLEFT] - level.bmaporgx);
		xh = MapBlock(tmtrace.BBox[BOXRIGHT] - level.bmaporgx);
		yl = MapBlock(tmtrace.BBox[BOXBOTTOM] - level.bmaporgy);
		yh = MapBlock(tmtrace.BBox[BOXTOP] - level.bmaporgy);

		for (bx = xl; bx <= xh; bx++)
			for (by = yl; by <= yh; by++)
				if (!SV_BlockLinesIterator(bx, by, PIT_CheckRelLine))
					return false;
	}

	return true;
	unguard;
}

//==========================================================================
//
//  VEntity::UpdateVelocity
//
//==========================================================================

void VEntity::UpdateVelocity(void)
{
	guard(VEntity::UpdateVelocity);
/*	if (Origin.z <= FloorZ && !Velocity.x && !Velocity.y &&
		!Velocity.z && !bCountKill && !bIsPlayer)
	{
		//  No gravity for non-moving things on ground to prevent
		// static objects from sliding on slopes
		return;
	}*/

	//  Don't add gravity if standing on slope with normal.z > 0.7 (aprox
	// 45 degrees)
	if (!bNoGravity && (Origin.z > FloorZ || Floor->normal.z < 0.7))
	{
		if (WaterLevel < 2)
		{
			//  Add gravity
			Velocity.z -= Mass / 100.0 * GRAVITY * host_frametime;
		}
		else if (!bIsPlayer)
		{
			//  Add gravity
			Velocity.z = -Mass / 100.0 * 60.0;
		}
	}

	// Friction
	if (Velocity.x || Velocity.y || Velocity.z)
	{
		eventApplyFriction();
	}
	unguard;
}

//**************************************************************************
//
//  TEST ON MOBJ
//
//**************************************************************************

VEntity *tzmthing;
TVec tzorg;
VEntity *onmobj;	//generic global onmobj...used for landing on pods/players

//==========================================================================
//
//  PIT_CheckOnmobjZ
//
//==========================================================================

boolean PIT_CheckOnmobjZ(VMapObject *Other)
{
	float blockdist;

	if (!Other->bSolid)
	{
		// Can't hit thing
		return true;
	}
	blockdist = Other->Radius + tzmthing->Radius;
	if (fabs(Other->Origin.x - tzorg.x) >= blockdist ||
		fabs(Other->Origin.y - tzorg.y) >= blockdist)
	{
		// Didn't hit thing
		return true;
	}
	if (Other == tzmthing)
	{
		// Don't clip against self
		return true;
	}
	if (tzorg.z > Other->Origin.z + Other->Height)
	{
		return true;
	}
	if (tzorg.z + tzmthing->Height < Other->Origin.z)
	{
		// under thing
		return true;
	}
	onmobj = (VEntity *)Other;
	return false;
}

//=============================================================================
//
//  VEntity::FakeZMovement
//
//  Fake the zmovement so that we can check if a move is legal
//
//=============================================================================

void VEntity::FakeZMovement(void)
{
	//
	//  adjust height
	//
	tzorg.z += Velocity.z * host_frametime;
#if 0
	if (bFloat && Target)
	{
		// float down towards enemy if too close
		if (!bSkullFly && !bInFloat)
		{
			float dist = MobjDist2(self, Target);
			float delta = Target.Origin.z + Height / 2.0 - tzorg.z;
			if (delta < 0.0 && dist < -(delta * 3.0))
				tzorg.z -= FLOATSPEED * frametime;
			else if (delta > 0.0 && dist < (delta * 3.0))
				tzorg.z += FLOATSPEED * frametime;
		}
	}
	if (bIsPlayer && bFly && !(tzorg.z <= FloorZ) && level->tictime & 2)
	{
		tzorg.z += sin(90.0 * 35.0 / 20.0 * level->time);
	}
#endif

	//
	//  clip movement
	//
	if (tzorg.z <= FloorZ)
	{
		// Hit the floor
		tzorg.z = FloorZ;
	}
	if (tzorg.z + Height > CeilingZ)
	{
		// hit the ceiling
		tzorg.z = CeilingZ - Height;
	}
}

//=============================================================================
//
//  VEntity::CheckOnmobj
//
//  Checks if the new Z position is legal
//
//=============================================================================

VEntity *VEntity::CheckOnmobj(void)
{
	int xl, xh, yl, yh, bx, by;

	if (!bColideWithThings)
		return NULL;

	tzmthing = this;
	tzorg = Origin;
	FakeZMovement();

	//
	// check things first, possibly picking things up
	// the bounding box is extended by MAXRADIUS because mobj_ts are grouped
	// into mapblocks based on their origin point, and can overlap into adjacent
	// blocks by up to MAXRADIUS units
	//
	xl = MapBlock(Origin.x - Radius - level.bmaporgx - MAXRADIUS);
	xh = MapBlock(Origin.x + Radius - level.bmaporgx + MAXRADIUS);
	yl = MapBlock(Origin.y - Radius - level.bmaporgy - MAXRADIUS);
	yh = MapBlock(Origin.y + Radius - level.bmaporgy + MAXRADIUS);

	for (bx = xl; bx <= xh; bx++)
	{
		for (by = yl; by <= yh; by++)
		{
			if (!SV_BlockThingsIterator(bx, by, PIT_CheckOnmobjZ, NULL))
			{
				return onmobj;
			}
		}
	}
	return NULL;
}

//==========================================================================
//
//	Entity.SetState
//
//==========================================================================

IMPLEMENT_FUNCTION(VEntity, SetState)
{
	VEntity	*Self;
	int		state;

	state = PR_Pop();
	Self = (VEntity *)PR_Pop();
	PR_Push(Self->SetState(state));
}

//==========================================================================
//
//	Entity.PlaySound
//
//==========================================================================

IMPLEMENT_FUNCTION(VEntity, PlaySound)
{
	VEntity	*Self;
	FName	SoundName;
	int		Channel;

	Channel = PR_Pop();
	SoundName = PR_PopName();
	Self = (VEntity *)PR_Pop();
	SV_StartSound(Self, S_GetSoundID(SoundName), Channel, 127);
}

//==========================================================================
//
//	Entity.PlayFullVolumeSound
//
//==========================================================================

IMPLEMENT_FUNCTION(VEntity, PlayFullVolumeSound)
{
	VEntity	*Self;
	FName	SoundName;
	int		Channel;

	Channel = PR_Pop();
	SoundName = PR_PopName();
	Self = (VEntity *)PR_Pop();
	SV_StartSound(NULL, S_GetSoundID(SoundName), Channel, 127);
}

//==========================================================================
//
//	Entity.CheckWater
//
//==========================================================================

IMPLEMENT_FUNCTION(VEntity, CheckWater)
{
	VEntity	*Self;

	Self = (VEntity *)PR_Pop();
	PR_Push(Self->CheckWater());
}

//==========================================================================
//
//	Entity.CheckPosition
//
//==========================================================================

IMPLEMENT_FUNCTION(VEntity, CheckPosition)
{
	VEntity	*Self;
	TVec	Pos;

	Pos = PR_Popv();
	Self = (VEntity *)PR_Pop();
	PR_Push(Self->CheckPosition(Pos));
}

//==========================================================================
//
//	Entity.CheckRelPosition
//
//==========================================================================

IMPLEMENT_FUNCTION(VEntity, CheckRelPosition)
{
	VEntity	*Self;
	TVec	Pos;

	Pos = PR_Popv();
	Self = (VEntity *)PR_Pop();
	PR_Push(Self->CheckRelPosition(Pos));
}

//==========================================================================
//
//	Entity.UpdateVelocity
//
//==========================================================================

IMPLEMENT_FUNCTION(VEntity, UpdateVelocity)
{
	VEntity	*Self;

	Self = (VEntity *)PR_Pop();
	Self->UpdateVelocity();
}

//===========================================================================
//
//	VEntity.CheckOnmobj
//
//===========================================================================

IMPLEMENT_FUNCTION(VEntity, CheckOnmobj)
{
	VEntity	*Self;

	Self = (VEntity *)PR_Pop();
	PR_Push((int)Self->CheckOnmobj());
}

//===========================================================================
//
//  VViewEntity::SetState
//
//===========================================================================

void VViewEntity::SetState(int stnum)
{
	guard(VViewEntity::SetState);
	state_t *state;

	do
	{
		if (!stnum)
		{
			// Object removed itself.
			StateNum = 0;
			break;
		}
		state = &GStates[stnum];
		StateTime = state->time;	// could be 0
		SpriteIndex = state->sprite;
		SpriteFrame = state->frame;
		ModelIndex = state->model_index;
		ModelFrame = state->model_frame;
		StateNum = state->statenum;
		NextState = state->nextstate;
		// Call action routine.
		svpr.Exec(state->function, (int)this);
		stnum = NextState;
	}
	while (StateNum && !StateTime);	// An initial state of 0 could cycle through.
	unguard;
}

//==========================================================================
//
//	ViewEntity.SetState
//
//==========================================================================

IMPLEMENT_FUNCTION(VViewEntity, SetState)
{
	VViewEntity	*Self;
	int			state;

	state = PR_Pop();
	Self = (VViewEntity *)PR_Pop();
	Self->SetState(state);
}

//==========================================================================
//
//	EntInit
//
//==========================================================================

void EntInit(void)
{
	svpr.SetGlobal("tmtrace", (int)&tmtrace);
	GStates = (state_t *)svpr.GlobalAddr("states");
	GSpriteNames = (FName *)svpr.GlobalAddr("sprite_names");
	VEntity::FIndex_Remove =
		VEntity::StaticClass()->GetFunctionIndex("Remove");
	VEntity::FIndex_Touch = 
		VEntity::StaticClass()->GetFunctionIndex("Touch");
	VEntity::FIndex_BlockedByLine = 
		VEntity::StaticClass()->GetFunctionIndex("BlockedByLine");
	VEntity::FIndex_ApplyFriction = 
		VEntity::StaticClass()->GetFunctionIndex("ApplyFriction");
}

//==========================================================================
//
//
//
//==========================================================================

//**************************************************************************
//
//	$Log$
//	Revision 1.2  2002/03/16 17:55:12  dj_jl
//	Some small changes.
//
//	Revision 1.1  2002/03/09 18:06:25  dj_jl
//	Made Entity class and most of it's functions native
//	
//**************************************************************************
