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
	int nextstate;
	FFunction *function;
	FName statename;
};

// EXTERNAL FUNCTION PROTOTYPES --------------------------------------------

// PUBLIC FUNCTION PROTOTYPES ----------------------------------------------

// PRIVATE FUNCTION PROTOTYPES ---------------------------------------------

static state_t	*GStates;
static FName	*GSpriteNames;

// EXTERNAL DATA DECLARATIONS ----------------------------------------------

// PUBLIC DATA DEFINITIONS -------------------------------------------------

IMPLEMENT_CLASS(VEntity);

int VEntity::FIndex_Destroyed;
int VEntity::FIndex_Touch;
int VEntity::FIndex_BlockedByLine;
int VEntity::FIndex_ApplyFriction;
int VEntity::FIndex_PushLine;
int VEntity::FIndex_HandleFloorclip;
int VEntity::FIndex_CrossSpecialLine;

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
			Remove();
			return false;
		}

		StateNum = state;
		st = &GStates[state];
		StateTime = st->time;
		SpriteIndex = st->sprite;
		SpriteName = GSpriteNames[st->sprite];
		SpriteFrame = st->frame;
		if (!bFixedModel)
			ModelIndex = st->model_index;
		ModelFrame = st->model_frame;
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

static cptrace_t cptrace;

//==========================================================================
//
//	PIT_CheckThing
//
//==========================================================================

static boolean PIT_CheckThing(VEntity *Other)
{
	guardSlow(PIT_CheckThing);
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
	unguardSlow;
}

//==========================================================================
//
//	PIT_CheckLine
//
//  Adjusts cptrace.FoorZ and cptrace.CeilingZ as lines are contacted
//
//==========================================================================

static boolean PIT_CheckLine(line_t * ld)
{
	guardSlow(PIT_CheckLine);
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
	unguardSlow;
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

	boolean FloatOk;	// if true, move would be ok if
						// within tmtrace.FloorZ - tmtrace.CeilingZ

	// keep track of the line that lowers the ceiling,
	// so missiles don't explode against sky hack walls
	line_t *CeilingLine;

	// keep track of special lines as they are hit,
	// but don't process them until the move is proven valid
	#define MAXSPECIALCROSS	32
	line_t *SpecHit[MAXSPECIALCROSS];
	int NumSpecHit;

	VEntity *BlockingMobj;
};

static tmtrace_t tmtrace;

//==========================================================================
//
//	PIT_CheckRelThing
//
//==========================================================================

static boolean PIT_CheckRelThing(VEntity *Other)
{
	guardSlow(PIT_CheckRelThing);
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
		if (tmtrace.End.z >= Other->Origin.z + Other->Height)
		{
			return true;	// overhead
		}
		if (tmtrace.End.z + tmtrace.Thing->Height <= Other->Origin.z)
		{
			return true;	// underneath
		}
	}

	tmtrace.BlockingMobj = Other;
	return tmtrace.Thing->eventTouch(Other);
	unguardSlow;
}

//==========================================================================
//
//	PIT_CheckRelLine
//
//  Adjusts tmtrace.FloorZ and tmtrace.CeilingZ as lines are contacted
//
//==========================================================================

static boolean PIT_CheckRelLine(line_t * ld)
{
	guardSlow(PIT_CheckRelLine);
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
			//GCon->Log(NAME_Dev, "Spechit overflow");
		}
		else
		{
			tmtrace.SpecHit[tmtrace.NumSpecHit] = ld;
			tmtrace.NumSpecHit++;
		}
	}

	return true;
	unguardSlow;
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
//  VEntity::TryMove
//
//  Attempt to move to a new position, crossing special lines.
//
//==========================================================================

boolean VEntity::TryMove(TVec newPos)
{
	boolean check;
	TVec oldorg;
	int side;
	int oldside;
	line_t *ld;

	check = CheckRelPosition(newPos);
	tmtrace.FloatOk = false;
	if (!check)
	{
		VEntity *O = tmtrace.BlockingMobj;
		if (!O || O->bIsPlayer || !bIsPlayer || 
			O->Origin.z + O->Height - Origin.z > MaxStepHeight ||
			O->CeilingZ - (O->Origin.z + O->Height) < Height ||
		   	tmtrace.CeilingZ - (O->Origin.z + O->Height) < Height)
		{
			eventPushLine();
			return false;
		}
	}

	if (bColideWithWorld)
	{
		if (tmtrace.CeilingZ - tmtrace.FloorZ < Height)
		{
			// Doesn't fit
			eventPushLine();
			return false;
		}

		tmtrace.FloatOk = true;

		if (tmtrace.CeilingZ - Origin.z < Height && !bFly
//			&& Class != LightningCeiling
		)
		{
			// mobj must lower itself to fit
			eventPushLine();
			return false;
		}
		if (bFly)
		{
			if (Origin.z + Height > tmtrace.CeilingZ)
			{
				Velocity.z = -8.0 * 35.0;
				eventPushLine();
				return false;
			}
			else if (Origin.z < tmtrace.FloorZ
				&& tmtrace.FloorZ - tmtrace.DropOffZ > MaxStepHeight)
			{
				Velocity.z = 8.0 * 35.0;
				eventPushLine();
				return false;
			}
		}
		if (tmtrace.FloorZ - Origin.z > MaxStepHeight)
		{
			// Too big a step up
			eventPushLine();
			return false;
		}
// Only Heretic
//		if (bMissile && tmtrace->FloorZ > Origin.z)
//		{
//			eventPushLine();
//		}
		if (!bDropOff && !bFloat && !bBlasted &&
			(tmtrace.FloorZ - tmtrace.DropOffZ > MaxStepHeight))
		{
			// Can't move over a dropoff unless it's been blasted
			return false;
		}
		if (bCantLeaveFloorpic && (tmtrace.Floor->pic != Floor->pic
				|| tmtrace.FloorZ != Origin.z))
		{
			// must stay within a sector of a certain floor type
			return false;
		}
	}

	// the move is ok,
	// so link the thing into its new position
	UnlinkFromWorld();

	oldorg = Origin;
	Origin = newPos;

	LinkToWorld();
	FloorZ = tmtrace.FloorZ;
	CeilingZ = tmtrace.CeilingZ;
	Floor = tmtrace.Floor;
	Ceiling = tmtrace.Ceiling;

	if (bFloorClip)
	{
		eventHandleFloorclip();
	}
	else
	{
		FloorClip = 0.0;
	}

	//
	// if any special lines were hit, do the effect
	//
	if (bColideWithWorld)
	{
		while (tmtrace.NumSpecHit > 0)
		{
			tmtrace.NumSpecHit--;
			// see if the line was crossed
			ld = tmtrace.SpecHit[tmtrace.NumSpecHit];
			side = ld->PointOnSide(Origin);
			oldside = ld->PointOnSide(oldorg);
			if (side != oldside)
			{
				if (ld->special)
				{
					eventCrossSpecialLine(ld, oldside);
				}
			}
		}
	}

	return true;
}

//**************************************************************************
//
//  SLIDE MOVE
//
//  Allows the player to slide along any angled walls.
//
//**************************************************************************

static float bestslidefrac;
static line_t *bestslideline;

static VEntity *slidemo;
static TVec slideorg;
static TVec slidedir;

//==========================================================================
//
//  ClipVelocity
//
//  Slide off of the impacting object
//
//==========================================================================

static TVec ClipVelocity(TVec in, TVec normal, float overbounce)
{
	return in - normal * (DotProduct(in, normal) * overbounce);
}

//==========================================================================
//
//  PTR_SlideTraverse
//
//==========================================================================

static boolean PTR_SlideTraverse(intercept_t * in)
{
	line_t *li;
	TVec hit_point;
	opening_t *open;

	if (!in->isaline)
		Host_Error("PTR_SlideTraverse: not a line?");

	li = in->line;

	if (li->flags & ML_TWOSIDED)
	{
		// set openrange, opentop, openbottom
		hit_point = slideorg + in->frac * slidedir;
		open = SV_LineOpenings(li, hit_point);
		open = SV_FindOpening(open, slidemo->Origin.z,
			slidemo->Origin.z + slidemo->Height);

		if (open && (open->range >= slidemo->Height) &&	//  fits
			(open->top - slidemo->Origin.z >= slidemo->Height) &&	// mobj is not too high
			(open->bottom - slidemo->Origin.z <= slidemo->MaxStepHeight))	// not too big a step up
		{
			// this line doesn't block movement
			return true;
		}
	}
	else
	{
		if (li->PointOnSide(slidemo->Origin))
		{
			// don't hit the back side
			return true;
		}
	}

	// the line does block movement,
	// see if it is closer than best so far
	if (in->frac < bestslidefrac)
	{
		bestslidefrac = in->frac;
		bestslideline = li;
	}

	return false;	// stop
}

//==========================================================================
//
//  SlidePathTraverse
//
//==========================================================================

static void SlidePathTraverse(float x, float y)
{
	slideorg = TVec(x, y, slidemo->Origin.z);
	slidedir = slidemo->Velocity * host_frametime;
	SV_PathTraverse(x, y, x + slidedir.x, y + slidedir.y,
		PT_ADDLINES, PTR_SlideTraverse, NULL);
}

//==========================================================================
//
//  VEntity::SlideMove
//
//  The momx / momy move is bad, so try to slide along a wall.
//  Find the first line hit, move flush to it, and slide along it.
//  This is a kludgy mess.
//
//==========================================================================

void VEntity::SlideMove(void)
{
	float leadx;
	float leady;
	float trailx;
	float traily;
	float newx;
	float newy;
	int hitcount;

	slidemo = this;
	hitcount = 0;

	do
	{
		if (++hitcount == 3)
		{
			// don't loop forever
			if (!TryMove(TVec(Origin.x, Origin.y + Velocity.y * host_frametime, Origin.z)))
				TryMove(TVec(Origin.x + Velocity.x * host_frametime, Origin.y, Origin.z));
			return;
		}

		// trace along the three leading corners
		if (Velocity.x > 0.0)
		{
			leadx = Origin.x + Radius;
			trailx = Origin.x - Radius;
		}
		else
		{
			leadx = Origin.x - Radius;
			trailx = Origin.x + Radius;
		}

		if (Velocity.y > 0.0)
		{
			leady = Origin.y + Radius;
			traily = Origin.y - Radius;
		}
		else
		{
			leady = Origin.y - Radius;
			traily = Origin.y + Radius;
		}

		bestslidefrac = 1.00001f;

		SlidePathTraverse(leadx, leady);
		SlidePathTraverse(trailx, leady);
		SlidePathTraverse(leadx, traily);

		// move up to the wall
		if (bestslidefrac == 1.00001f)
		{
			// the move most have hit the middle, so stairstep
			if (!TryMove(TVec(Origin.x, Origin.y + Velocity.y * host_frametime, Origin.z)))
				TryMove(TVec(Origin.x + Velocity.x * host_frametime, Origin.y, Origin.z));
			return;
		}

		// fudge a bit to make sure it doesn't hit
		bestslidefrac -= 0.03125;
		if (bestslidefrac > 0.0)
		{
			newx = Velocity.x * host_frametime * bestslidefrac;
			newy = Velocity.y * host_frametime * bestslidefrac;

			if (!TryMove(TVec(Origin.x + newx, Origin.y + newy, Origin.z)))
			{
				if (!TryMove(TVec(Origin.x, Origin.y + Velocity.y * host_frametime, Origin.z)))
					TryMove(TVec(Origin.x + Velocity.x * host_frametime, Origin.y, Origin.z));
				return;
			}
		}

		// Now continue along the wall.
		// First calculate remainder.
		bestslidefrac = 1.0 - (bestslidefrac + 0.03125);

		if (bestslidefrac > 1.0)
			bestslidefrac = 1.0;

		if (bestslidefrac <= 0.0)
			return;

		// clip the moves
		Velocity = ClipVelocity(Velocity * bestslidefrac,
			bestslideline->normal, 1.0);

	}
	while (!TryMove(TVec(Origin.x + Velocity.x * host_frametime,
			Origin.y + Velocity.y * host_frametime, Origin.z)));
}

//**************************************************************************
//
//  BOUNCING
//
//  Bounce missile against walls
//
//**************************************************************************

//============================================================================
//
//  PTR_BounceTraverse
//
//============================================================================

static boolean PTR_BounceTraverse(intercept_t * in)
{
	line_t *li;
	TVec hit_point;
	opening_t *open;

	if (!in->isaline)
		Host_Error("PTR_BounceTraverse: not a line?");

	li = in->line;
	if (li->flags & ML_TWOSIDED)
	{
		hit_point = slideorg + in->frac * slidedir;
		open = SV_LineOpenings(li, hit_point);	// set openrange, opentop, openbottom
		open = SV_FindOpening(open, slidemo->Origin.z,
			slidemo->Origin.z + slidemo->Height);
		if (open && open->range >= slidemo->Height &&	// fits
			open->top - slidemo->Origin.z >= slidemo->Height)	// mobj is not too high
		{
			return true;	// this line doesn't block movement
		}
	}
	else
	{
		if (li->PointOnSide(slidemo->Origin))
		{
			return true;	// don't hit the back side
		}
	}

	bestslideline = li;
	return false;	// stop
}

//============================================================================
//
//  VEntity::BounceWall
//
//============================================================================

void VEntity::BounceWall(float overbounce)
{
	slidemo = this;
	if (Velocity.x > 0.0)
	{
		slideorg.x = Origin.x + Radius;
	}
	else
	{
		slideorg.x = Origin.x - Radius;
	}
	if (Velocity.y > 0.0)
	{
		slideorg.y = Origin.y + Radius;
	}
	else
	{
		slideorg.y = Origin.y - Radius;
	}
	slideorg.z = Origin.z;
	slidedir = Velocity * host_frametime;
	SV_PathTraverse(slideorg.x, slideorg.y,
		slideorg.x + slidedir.x, slideorg.y + slidedir.y,
		PT_ADDLINES, PTR_BounceTraverse, NULL);
	Velocity = ClipVelocity(Velocity, bestslideline->normal, overbounce);
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

static VEntity *tzmthing;
static TVec tzorg;
static VEntity *onmobj;	//generic global onmobj...used for landing on pods/players

//==========================================================================
//
//  PIT_CheckOnmobjZ
//
//==========================================================================

static boolean PIT_CheckOnmobjZ(VEntity *Other)
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
	onmobj = Other;
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
//	Entity.Remove
//
//==========================================================================

IMPLEMENT_FUNCTION(VEntity, Remove)
{
	VEntity *Self = (VEntity *)PR_Pop();
	Self->Remove();
}

//==========================================================================
//
//	Entity.SetState
//
//==========================================================================

IMPLEMENT_FUNCTION(VEntity, SetState)
{
	int state = PR_Pop();
	VEntity *Self = (VEntity *)PR_Pop();
	PR_Push(Self->SetState(state));
}

//==========================================================================
//
//	Entity.PlaySound
//
//==========================================================================

IMPLEMENT_FUNCTION(VEntity, PlaySound)
{
	int Channel = PR_Pop();
	FName SoundName = PR_PopName();
	VEntity *Self = (VEntity *)PR_Pop();
	SV_StartSound(Self, S_GetSoundID(SoundName), Channel, 127);
}

//==========================================================================
//
//	Entity.PlayFullVolumeSound
//
//==========================================================================

IMPLEMENT_FUNCTION(VEntity, PlayFullVolumeSound)
{
	int Channel = PR_Pop();
	FName SoundName = PR_PopName();
	VEntity *Self = (VEntity *)PR_Pop();
	SV_StartSound(NULL, S_GetSoundID(SoundName), Channel, 127);
}

//==========================================================================
//
//	Entity.StopSound
//
//==========================================================================

IMPLEMENT_FUNCTION(VEntity, StopSound)
{
	int Channel = PR_Pop();
	VEntity *Self = (VEntity *)PR_Pop();
	SV_StopSound(Self, Channel);
}

//==========================================================================
//
//	Entity.CheckWater
//
//==========================================================================

IMPLEMENT_FUNCTION(VEntity, CheckWater)
{
	VEntity *Self = (VEntity *)PR_Pop();
	PR_Push(Self->CheckWater());
}

//==========================================================================
//
//	Entity.CheckPosition
//
//==========================================================================

IMPLEMENT_FUNCTION(VEntity, CheckPosition)
{
	TVec Pos = PR_Popv();
	VEntity *Self = (VEntity *)PR_Pop();
	PR_Push(Self->CheckPosition(Pos));
}

//==========================================================================
//
//	Entity.CheckRelPosition
//
//==========================================================================

IMPLEMENT_FUNCTION(VEntity, CheckRelPosition)
{
	TVec Pos = PR_Popv();
	VEntity *Self = (VEntity *)PR_Pop();
	PR_Push(Self->CheckRelPosition(Pos));
}

//==========================================================================
//
//	Entity.TryMove
//
//==========================================================================

IMPLEMENT_FUNCTION(VEntity, TryMove)
{
	TVec Pos = PR_Popv();
	VEntity *Self = (VEntity *)PR_Pop();
	PR_Push(Self->TryMove(Pos));
}

//==========================================================================
//
//	Entity.SlideMove
//
//==========================================================================

IMPLEMENT_FUNCTION(VEntity, SlideMove)
{
	VEntity *Self = (VEntity *)PR_Pop();
	Self->SlideMove();
}

//==========================================================================
//
//	Entity.SlideMove
//
//==========================================================================

IMPLEMENT_FUNCTION(VEntity, BounceWall)
{
	float overbounce = PR_Popf();
	VEntity	*Self = (VEntity *)PR_Pop();
	Self->BounceWall(overbounce);
}

//==========================================================================
//
//	Entity.UpdateVelocity
//
//==========================================================================

IMPLEMENT_FUNCTION(VEntity, UpdateVelocity)
{
	VEntity *Self = (VEntity *)PR_Pop();
	Self->UpdateVelocity();
}

//===========================================================================
//
//	VEntity.CheckOnmobj
//
//===========================================================================

IMPLEMENT_FUNCTION(VEntity, CheckOnmobj)
{
	VEntity *Self = (VEntity *)PR_Pop();
	PR_Push((int)Self->CheckOnmobj());
}

//===========================================================================
//
//	VEntity.LinkToWorld
//
//===========================================================================

IMPLEMENT_FUNCTION(VEntity, LinkToWorld)
{
	VEntity *Self = (VEntity *)PR_Pop();
	Self->LinkToWorld();
}

//===========================================================================
//
//	VEntity.UnlinkFromWorld
//
//===========================================================================

IMPLEMENT_FUNCTION(VEntity, UnlinkFromWorld)
{
	VEntity *Self = (VEntity *)PR_Pop();
	Self->UnlinkFromWorld();
}

//===========================================================================
//
//	VEntity.CanSee
//
//===========================================================================

IMPLEMENT_FUNCTION(VEntity, CanSee)
{
	VEntity *Other = (VEntity *)PR_Pop();
	VEntity *Self = (VEntity *)PR_Pop();
	PR_Push(Self->CanSee(Other));
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
		StateNum = stnum;
		state = &GStates[stnum];
		StateTime = state->time;	// could be 0
		SpriteIndex = state->sprite;
		SpriteFrame = state->frame;
		ModelIndex = state->model_index;
		ModelFrame = state->model_frame;
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
	int state = PR_Pop();
	VViewEntity *Self = (VViewEntity *)PR_Pop();
	Self->SetState(state);
}

//==========================================================================
//
//	VEntity::InitFuncIndexes
//
//==========================================================================

void VEntity::InitFuncIndexes(void)
{
	FIndex_Destroyed = StaticClass()->GetFunctionIndex("Destroyed");
	FIndex_Touch = StaticClass()->GetFunctionIndex("Touch");
	FIndex_BlockedByLine = StaticClass()->GetFunctionIndex("BlockedByLine");
	FIndex_ApplyFriction = StaticClass()->GetFunctionIndex("ApplyFriction");
	FIndex_PushLine = StaticClass()->GetFunctionIndex("PushLine");
	FIndex_HandleFloorclip = StaticClass()->GetFunctionIndex("HandleFloorclip");
	FIndex_CrossSpecialLine = StaticClass()->GetFunctionIndex("CrossSpecialLine");
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
	VEntity::InitFuncIndexes();
}

//==========================================================================
//
//
//
//==========================================================================

//**************************************************************************
//
//	$Log$
//	Revision 1.9  2002/08/28 16:41:09  dj_jl
//	Merged VMapObject with VEntity, some natives.
//
//	Revision 1.8  2002/07/23 16:29:56  dj_jl
//	Replaced console streams with output device class.
//	
//	Revision 1.7  2002/07/13 07:48:08  dj_jl
//	Moved some global functions to Entity class.
//	
//	Revision 1.6  2002/06/22 07:08:45  dj_jl
//	Made sliding and bouncing functions native.
//	
//	Revision 1.5  2002/06/14 15:40:09  dj_jl
//	Added state name to the state.
//	
//	Revision 1.4  2002/04/27 17:01:08  dj_jl
//	Fixed clipping when walking over/under other things.
//	
//	Revision 1.3  2002/04/11 16:46:06  dj_jl
//	Made TryMove native.
//	
//	Revision 1.2  2002/03/16 17:55:12  dj_jl
//	Some small changes.
//	
//	Revision 1.1  2002/03/09 18:06:25  dj_jl
//	Made Entity class and most of it's functions native
//	
//**************************************************************************
