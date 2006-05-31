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

// EXTERNAL FUNCTION PROTOTYPES --------------------------------------------

// PUBLIC FUNCTION PROTOTYPES ----------------------------------------------

// PRIVATE FUNCTION PROTOTYPES ---------------------------------------------

// EXTERNAL DATA DECLARATIONS ----------------------------------------------

extern VEntity**	sv_mobjs;

// PUBLIC DATA DEFINITIONS -------------------------------------------------

IMPLEMENT_CLASS(V, Entity);

int VEntity::FIndex_Destroyed;
int VEntity::FIndex_Touch;
int VEntity::FIndex_BlockedByLine;
int VEntity::FIndex_ApplyFriction;
int VEntity::FIndex_PushLine;
int VEntity::FIndex_HandleFloorclip;
int VEntity::FIndex_CrossSpecialLine;
int VEntity::FIndex_SectorChanged;
int VEntity::FIndex_RoughCheckThing;
int VEntity::FIndex_GiveInventory;
int VEntity::FIndex_TakeInventory;
int VEntity::FIndex_CheckInventory;
int VEntity::FIndex_GetSigilPieces;

// PRIVATE DATA DEFINITIONS ------------------------------------------------

// CODE --------------------------------------------------------------------

//==========================================================================
//
//  VEntity::SetState
//
//==========================================================================

void VEntity::Serialise(VStream& Strm)
{
	guard(VEntity::Serialise);
	Super::Serialise(Strm);

	if (Strm.IsLoading())
	{
		sv_mobjs[NetID] = this;
		if (EntityFlags & EF_IsPlayer)
		{
			Player->MO = this;
		}
		SubSector = NULL;	//	Must mark as not linked
		LinkToWorld();
	}
	unguard;
}

//==========================================================================
//
//  VEntity::SetState
//
//  Returns true if the actor is still present.
//
//==========================================================================

bool VEntity::SetState(VState* InState)
{
	guard(VEntity::SetState);
	VState *st = InState;
	do
	{
		if (!st)
		{
			// Remove mobj
			State = NULL;
			Remove();
			return false;
		}

		State = st;
		StateTime = st->time;
		SpriteIndex = st->SpriteIndex;
		SpriteName = st->SpriteName;
		SpriteFrame = st->frame;
		if (!(EntityFlags & EF_FixedModel))
			ModelIndex = st->ModelIndex;
		ModelFrame = st->model_frame;
		NextState = st->nextstate;

		// Modified handling.
		// Call action functions when the state is set
		P_PASS_SELF;
		svpr.ExecuteFunction(st->function);

		st = NextState;
	}
	while (!StateTime);
	return true;
	unguard;
}

//==========================================================================
//
//  VEntity::SetInitialState
//
//  Returns true if the actor is still present.
//
//==========================================================================

void VEntity::SetInitialState(VState* InState)
{
	guard(VEntity::SetInitialState);
	State = InState;
	if (InState)
	{
		StateTime = InState->time;
		SpriteIndex = InState->SpriteIndex;
		SpriteName = InState->SpriteName;
		SpriteFrame = InState->frame;
		ModelIndex = InState->ModelIndex;
		ModelFrame = InState->model_frame;
		NextState = InState->nextstate;
	}
	else
	{
		StateTime = -1.0;
		SpriteIndex = 0;
		SpriteName = NAME_None;
		SpriteFrame = 0;
		ModelIndex = 0;
		ModelFrame = 0;
		NextState = NULL;
	}
	unguard;
}

//==========================================================================
//
//	VEntity::FindState
//
//==========================================================================

VState* VEntity::FindState(VName StateName)
{
	guard(VEntity::FindState);
	VState* s = GetClass()->FindState(StateName);
	if (!s)
	{
		Host_Error("Can't find state %s", *StateName);
	}
	return s;
	unguard;
}

//==========================================================================
//
//  VEntity::CheckWater
//
//==========================================================================

bool VEntity::CheckWater()
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
			if (EntityFlags & EF_IsPlayer)
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

static float tmbbox[4];
static TVec pe_pos;	// Pain Elemental position for Lost Soul checks	// phares
static TVec ls_pos;	// Lost Soul position for Lost Soul checks		// phares

//==========================================================================
//																	// phares
// PIT_CrossLine													//   |
//																	//   V
// Checks to see if a PE->LS trajectory line crosses a blocking
// line. Returns false if it does.
//
// tmbbox holds the bounding box of the trajectory. If that box
// does not touch the bounding box of the line in question,
// then the trajectory is not blocked. If the PE is on one side
// of the line and the LS is on the other side, then the
// trajectory is blocked.
//
// Currently this assumes an infinite line, which is not quite
// correct. A more correct solution would be to check for an
// intersection of the trajectory and the line, but that takes
// longer and probably really isn't worth the effort.
//
//==========================================================================

 // killough 3/26/98: make static
static boolean PIT_CrossLine(line_t* ld)
{
	guardSlow(PIT_CrossLine);

	if ((ld->flags & ML_BLOCKING) || (ld->flags & ML_BLOCKMONSTERS) ||
		(ld->flags & ML_BLOCKEVERYTHING))
		if (!(tmbbox[BOXLEFT] > ld->bbox[BOXRIGHT] ||
			  tmbbox[BOXRIGHT] < ld->bbox[BOXLEFT] ||
			  tmbbox[BOXTOP] < ld->bbox[BOXBOTTOM] ||
			  tmbbox[BOXBOTTOM] > ld->bbox[BOXTOP]))
			if (ld->PointOnSide(pe_pos) != ld->PointOnSide(ls_pos))
					return false;  // line blocks trajectory

	return true; // line doesn't block trajectory
	unguardSlow;
}

//==========================================================================
//
//  VEntity::CheckSides
//
// This routine checks for Lost Souls trying to be spawned		// phares
// across 1-sided lines, impassible lines, or "monsters can't	//   |
// cross" lines. Draw an imaginary line between the PE			//   V
// and the new Lost Soul spawn spot. If that line crosses
// a 'blocking' line, then disallow the spawn. Only search
// lines in the blocks of the blockmap where the bounding box
// of the trajectory line resides. Then check bounding box
// of the trajectory vs. the bounding box of each blocking
// line to see if the trajectory and the blocking line cross.
// Then check the PE and LS to see if they're on different
// sides of the blocking line. If so, return true, otherwise
// false.
//
//==========================================================================

bool VEntity::CheckSides(TVec lsPos)
{
	guard(VEntity::CheckSides);
	int bx,by,xl,xh,yl,yh;

	pe_pos = Origin;
	ls_pos = lsPos;

	// Here is the bounding box of the trajectory
	tmbbox[BOXLEFT] = pe_pos.x < ls_pos.x ? pe_pos.x : ls_pos.x;
	tmbbox[BOXRIGHT] = pe_pos.x > ls_pos.x ? pe_pos.x : ls_pos.x;
	tmbbox[BOXTOP] = pe_pos.y > ls_pos.y ? pe_pos.y : ls_pos.y;
	tmbbox[BOXBOTTOM] = pe_pos.y < ls_pos.y ? pe_pos.y : ls_pos.y;

	// Determine which blocks to look in for blocking lines
	xl = MapBlock(tmbbox[BOXLEFT] - XLevel->BlockMapOrgX);
	xh = MapBlock(tmbbox[BOXRIGHT] - XLevel->BlockMapOrgX);
	yl = MapBlock(tmbbox[BOXBOTTOM] - XLevel->BlockMapOrgY);
	yh = MapBlock(tmbbox[BOXTOP] - XLevel->BlockMapOrgY);

	// xl->xh, yl->yh determine the mapblock set to search
	validcount++; // prevents checking same line twice
	for (bx = xl; bx <= xh; bx++)
		for (by = yl; by <= yh; by++)
			if (!SV_BlockLinesIterator(bx, by, PIT_CrossLine))
				return true;

	return false;
	unguard;
}

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

	if (!(Other->EntityFlags & VEntity::EF_Solid))
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

	if (!(cptrace.Thing->EntityFlags & VEntity::EF_NoPassMobj))
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

	if (!(ld->flags & ML_RAILING))
	{
		if (ld->flags & ML_BLOCKEVERYTHING)
		{
			// Explicitly blocking everything
			return false;
		}

		if (cptrace.Thing->EntityFlags & VEntity::EF_CheckLineBlocking &&
			ld->flags & ML_BLOCKING)
		{
			// Explicitly blocking everything
			return false;
		}

		if (cptrace.Thing->EntityFlags & VEntity::EF_CheckLineBlockMonsters &&
			ld->flags & ML_BLOCKMONSTERS)
		{
			// Block monsters only
			return false;
		}

		if (cptrace.Thing->EntityFlags & VEntity::EF_Float &&
			ld->flags & ML_BLOCK_FLOATERS)
		{
			// Block floaters only
			return false;
		}
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

		if (ld->flags & ML_RAILING)
		{
			cptrace.FloorZ += 32;
		}
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

bool VEntity::CheckPosition(TVec Pos)
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
	xl = MapBlock(cptrace.bbox[BOXLEFT] - XLevel->BlockMapOrgX - MAXRADIUS);
	xh = MapBlock(cptrace.bbox[BOXRIGHT] - XLevel->BlockMapOrgX + MAXRADIUS);
	yl = MapBlock(cptrace.bbox[BOXBOTTOM] - XLevel->BlockMapOrgY - MAXRADIUS);
	yh = MapBlock(cptrace.bbox[BOXTOP] - XLevel->BlockMapOrgY + MAXRADIUS);

	for (bx = xl; bx <= xh; bx++)
		for (by = yl; by <= yh; by++)
			if (!SV_BlockThingsIterator(bx, by, PIT_CheckThing, NULL, NULL))
				return false;

	// check lines
	xl = MapBlock(cptrace.bbox[BOXLEFT] - XLevel->BlockMapOrgX);
	xh = MapBlock(cptrace.bbox[BOXRIGHT] - XLevel->BlockMapOrgX);
	yl = MapBlock(cptrace.bbox[BOXBOTTOM] - XLevel->BlockMapOrgY);
	yh = MapBlock(cptrace.bbox[BOXTOP] - XLevel->BlockMapOrgY);

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

	enum
	{
		TF_FloatOk = 0x01,	// if true, move would be ok if
							// within tmtrace.FloorZ - tmtrace.CeilingZ
	};
	vuint32 TraceFlags;

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

struct avoiddropoff_t
{
	VEntity *thing;
	float angle;
	float floorx;
	float floory;
	float floorz;
	float t_bbox[4];
};

static avoiddropoff_t a;

//=============================================================================
//
// killough 11/98:
//
// Monsters try to move away from tall dropoffs.
//
// In Doom, they were never allowed to hang over dropoffs,
// and would remain stuck if involuntarily forced over one.
// This logic, combined with p_map.c (P_TryMove), allows
// monsters to free themselves without making them tend to
// hang over dropoffs.
//=============================================================================

static boolean PIT_AvoidDropoff(line_t *line)
{
	guardSlow(PIT_AvoidDropoff);
	float front;
	float back;
	sec_region_t* FrontReg;
	sec_region_t* BackReg;

	if (line->backsector && // Ignore one-sided linedefs
		a.t_bbox[BOXRIGHT] > line->bbox[BOXLEFT] &&
		a.t_bbox[BOXLEFT] < line->bbox[BOXRIGHT] &&
		a.t_bbox[BOXTOP] > line->bbox[BOXBOTTOM] && // Linedef must be contacted
		a.t_bbox[BOXBOTTOM] < line->bbox[BOXTOP] &&
		P_BoxOnLineSide(&a.t_bbox[0], line) == -1)
    {
		// New logic for 3D Floors
		FrontReg = SV_FindThingGap(line->frontsector->botregion,
			TVec(a.floorx, a.floory, a.floorz), a.floorz, a.floorz + a.thing->Height);
		BackReg = SV_FindThingGap(line->backsector->botregion,
			TVec(a.floorx, a.floory, a.floorz), a.floorz, a.floorz + a.thing->Height);

		front = FrontReg->floor->GetPointZ(TVec(a.floorx, a.floory, a.floorz));
		back = BackReg->floor->GetPointZ(TVec(a.floorx, a.floory, a.floorz));

		// The monster must contact one of the two floors,
		// and the other must be a tall dropoff.
		if ((back == a.floorz) && (front < a.floorz - a.thing->MaxDropoffHeight))
		{
			// front side dropoff
			a.angle = matan(line->normal.y, line->normal.x);
			return false;
		}
		else if ((front == a.floorz) && (back < a.floorz - a.thing->MaxDropoffHeight))
		{
			// back side dropoff
			a.angle = matan(-line->normal.y, -line->normal.x);
			return false;
		}
		else
			return true;
	}
	return true;
	unguardSlow;
}

//=============================================================================
//
// CheckDropOff
//
// Creates a bounding box for an actor and checks for a dropoff
//
//=============================================================================

float VEntity::CheckDropOff()
{
	guard(VEntity::CheckDropOff);
	int xl;
	int xh;
	int yl;
	int yh;
	int bx;
	int by;
	
	// Try to move away from a dropoff
	a.thing = this;
	a.floorx = Origin.x;
	a.floory = Origin.y;
	a.floorz = Origin.z;

	a.t_bbox[BOXTOP]   = Origin.y + Radius;
	a.t_bbox[BOXBOTTOM]= Origin.y - Radius;
	a.t_bbox[BOXRIGHT] = Origin.x + Radius;
	a.t_bbox[BOXLEFT]  = Origin.x - Radius;

	xl = MapBlock(a.t_bbox[BOXLEFT] - XLevel->BlockMapOrgX);
	xh = MapBlock(a.t_bbox[BOXRIGHT] - XLevel->BlockMapOrgX);
	yl = MapBlock(a.t_bbox[BOXBOTTOM] - XLevel->BlockMapOrgY);
	yh = MapBlock(a.t_bbox[BOXTOP] - XLevel->BlockMapOrgY);

	// check lines
	validcount++;
	for (bx = xl; bx <= xh; bx++)
		for (by = yl; by <= yh; by++)
			if(!SV_BlockLinesIterator(bx, by, PIT_AvoidDropoff)) // all contacted lines
				return a.angle;

	return 0.0;
	unguard;
}

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

	//if (!(tmtrace.Thing->EntityFlags & VEntity::EF_NoPassMobj) || Actor(Other).bSpecial)
	if (!(tmtrace.Thing->EntityFlags & VEntity::EF_NoPassMobj))
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

	if (!(ld->flags & ML_RAILING))
	{
		if (ld->flags & ML_BLOCKEVERYTHING)
		{
			// Explicitly blocking everything
			tmtrace.Thing->eventBlockedByLine(ld);
			return false;
		}

		if (tmtrace.Thing->EntityFlags & VEntity::EF_CheckLineBlocking && ld->flags & ML_BLOCKING)
		{
			// Explicitly blocking everything
			tmtrace.Thing->eventBlockedByLine(ld);
			return false;
		}
	
		if (tmtrace.Thing->EntityFlags & VEntity::EF_CheckLineBlockMonsters && ld->flags & ML_BLOCKMONSTERS)
		{
			// Block monsters only
			tmtrace.Thing->eventBlockedByLine(ld);
			return false;
		}

		if (tmtrace.Thing->EntityFlags & VEntity::EF_Float && ld->flags & ML_BLOCK_FLOATERS)
		{
			// Block floaters only
			tmtrace.Thing->eventBlockedByLine(ld);
			return false;
		}
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

		if (ld->flags & ML_RAILING)
		{
			tmtrace.FloorZ += 32;
		}
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

bool VEntity::CheckRelPosition(TVec Pos)
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
	if (EntityFlags & EF_ColideWithThings)
	{
		xl = MapBlock(tmtrace.BBox[BOXLEFT] - XLevel->BlockMapOrgX - MAXRADIUS);
		xh = MapBlock(tmtrace.BBox[BOXRIGHT] - XLevel->BlockMapOrgX + MAXRADIUS);
		yl = MapBlock(tmtrace.BBox[BOXBOTTOM] - XLevel->BlockMapOrgY - MAXRADIUS);
		yh = MapBlock(tmtrace.BBox[BOXTOP] - XLevel->BlockMapOrgY + MAXRADIUS);

		for (bx = xl; bx <= xh; bx++)
			for (by = yl; by <= yh; by++)
				if (!SV_BlockThingsIterator(bx, by, PIT_CheckRelThing, NULL, NULL))
					return false;

		tmtrace.BlockingMobj = NULL;
	}

	// check lines
	if (EntityFlags & EF_ColideWithWorld)
	{
		xl = MapBlock(tmtrace.BBox[BOXLEFT] - XLevel->BlockMapOrgX);
		xh = MapBlock(tmtrace.BBox[BOXRIGHT] - XLevel->BlockMapOrgX);
		yl = MapBlock(tmtrace.BBox[BOXBOTTOM] - XLevel->BlockMapOrgY);
		yh = MapBlock(tmtrace.BBox[BOXTOP] - XLevel->BlockMapOrgY);

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

bool VEntity::TryMove(TVec newPos)
{
	guard(VEntity::TryMove);
	boolean check;
	TVec oldorg;
	int side;
	int oldside;
	line_t *ld;
	boolean good;

	check = CheckRelPosition(newPos);
	tmtrace.TraceFlags &= ~tmtrace_t::TF_FloatOk;
	if (!check)
	{
		VEntity *O = tmtrace.BlockingMobj;
		if (!O || O->EntityFlags & EF_IsPlayer || !(EntityFlags & EF_IsPlayer) ||
			O->Origin.z + O->Height - Origin.z > MaxStepHeight ||
			O->CeilingZ - (O->Origin.z + O->Height) < Height ||
		   	tmtrace.CeilingZ - (O->Origin.z + O->Height) < Height)
		{
			eventPushLine();
			return false;
		}
	}

	if (EntityFlags & EF_ColideWithWorld)
	{
		if (tmtrace.CeilingZ - tmtrace.FloorZ < Height)
		{
			// Doesn't fit
			eventPushLine();
			return false;
		}

		tmtrace.TraceFlags |= tmtrace_t::TF_FloatOk;

		if (tmtrace.CeilingZ - Origin.z < Height && !(EntityFlags & EF_Fly) &&
			!(EntityFlags & EF_IgnoreCeilingStep))
		{
			// mobj must lower itself to fit
			eventPushLine();
			return false;
		}
		if (EntityFlags & EF_Fly)
		{
			// When flying, slide up or down blocking lines until the actor
			// is not blocked.
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
		else if (Origin.z < tmtrace.FloorZ)
		{
			// Check to make sure there's nothing in the way for the step up
			good = TestMobjZ();
			if(!good)
			{
				eventPushLine();
				return false;
			}
		}
// Only Heretic
//		if (bMissile && tmtrace->FloorZ > Origin.z)
//		{
//			eventPushLine();
//		}
		// killough 3/15/98: Allow certain objects to drop off
		if (!(EntityFlags & EF_DropOff) && !(EntityFlags & EF_Float))
		{
			float floorz = tmtrace.FloorZ;

			// [RH] If the thing is standing on something, use its current z as the floorz.
			// This is so that it does not walk off of things onto a drop off.
			if (EntityFlags & EF_OnMobj)
			{
				floorz = MAX(Origin.z, tmtrace.FloorZ);
			}

			if (!(EntityFlags & EF_AvoidingDropoff))
			{
				if (!(EntityFlags & EF_Blasted) &&
					(floorz - tmtrace.DropOffZ > MaxDropoffHeight))
				{
					// Can't move over a dropoff unless it's been blasted
					return false;
				}
			}
			else
			{
				// special logic to move a monster off a dropoff
				if (DropOffZ - tmtrace.DropOffZ > MaxDropoffHeight)
					return false;
			}
		}
		if (EntityFlags & EF_CantLeaveFloorpic &&
			(tmtrace.Floor->pic != Floor->pic || tmtrace.FloorZ != Origin.z))
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
	DropOffZ = tmtrace.DropOffZ;
	Floor = tmtrace.Floor;
	Ceiling = tmtrace.Ceiling;

	if (EntityFlags & EF_FloorClip)
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
	if (EntityFlags & EF_ColideWithWorld)
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
	unguard;
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
	guard(PTR_SlideTraverse);
	line_t *li;
	TVec hit_point;
	opening_t *open;
	boolean good;

	if (!(in->Flags & intercept_t::IF_IsALine))
		Host_Error("PTR_SlideTraverse: not a line?");

	li = in->line;

	// set openrange, opentop, openbottom
	hit_point = slideorg + in->frac * slidedir;
	open = SV_LineOpenings(li, hit_point);
	open = SV_FindOpening(open, slidemo->Origin.z,
		slidemo->Origin.z + slidemo->Height);

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
		if (open && (slidemo->Origin.z < open->bottom))
		{
			// Check to make sure there's nothing in the way for the step up
			good = slidemo->TestMobjZ();
			if (good)
			{
				return true;
			}
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

	// the line blocks movement,
	// see if it is closer than best so far
	if (in->frac < bestslidefrac)
	{
		bestslidefrac = in->frac;
		bestslideline = li;
	}

	return false;	// stop
	unguard;
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
		PT_ADDLINES, PTR_SlideTraverse, NULL, NULL);
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

void VEntity::SlideMove()
{
	guard(VEntity::SlideMove);
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
	unguard;
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
	guard(PTR_BounceTraverse);
	line_t *li;
	TVec hit_point;
	opening_t *open;

	if (!(in->Flags & intercept_t::IF_IsALine))
		Host_Error("PTR_BounceTraverse: not a line?");

	li = in->line;
	if (li->flags & ML_TWOSIDED)
	{
		hit_point = slideorg + in->frac * slidedir;
		open = SV_LineOpenings(li, hit_point);	// set openrange, opentop, openbottom
		open = SV_FindOpening(open, slidemo->Origin.z,
			slidemo->Origin.z + slidemo->Height);
		if (open && open->range >= slidemo->Height &&	// fits
			slidemo->Origin.z + slidemo->Height <= open->top &&
			slidemo->Origin.z >= open->bottom)	// mobj is not too high
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
	unguard;
}

//============================================================================
//
//  VEntity::BounceWall
//
//============================================================================

void VEntity::BounceWall(float overbounce)
{
	guard(VEntity::BounceWall);
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
	bestslideline = NULL;
	SV_PathTraverse(slideorg.x, slideorg.y,
		slideorg.x + slidedir.x, slideorg.y + slidedir.y,
		PT_ADDLINES, PTR_BounceTraverse, NULL, NULL);
	if (bestslideline)
	{
		Velocity = ClipVelocity(Velocity, bestslideline->normal, overbounce);
	}
	unguard;
}

//==========================================================================
//
//  VEntity::UpdateVelocity
//
//==========================================================================

void VEntity::UpdateVelocity()
{
	guard(VEntity::UpdateVelocity);
/*	if (Origin.z <= FloorZ && !Velocity.x && !Velocity.y &&
		!Velocity.z && !bCountKill && !(EntityFlags & EF_IsPlayer))
	{
		//  No gravity for non-moving things on ground to prevent
		// static objects from sliding on slopes
		return;
	}*/

	//  Don't add gravity if standing on slope with normal.z > 0.7 (aprox
	// 45 degrees)
	if (!(EntityFlags & EF_NoGravity) && (Origin.z > FloorZ || Floor->normal.z < 0.7))
	{
		if (WaterLevel < 2)
		{
			//  Add gravity
			Velocity.z -= Mass / 100.0 * GRAVITY * host_frametime;
		}
		else if (!(EntityFlags & EF_IsPlayer))
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
	guardSlow(PIT_CheckOnmobjZ);
	float blockdist;

	if (!(Other->EntityFlags & VEntity::EF_Solid))
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
	unguardSlow;
}

//=============================================================================
//
// TestMobjZ
//
//  Checks if the new Z position is legal
//
//=============================================================================

bool VEntity::TestMobjZ(bool AlreadySetUp)
{
	guard(VEntity::TestMobjZ);
	int xl, xh, yl, yh, bx, by;

	// Can't hit thing
	if (!(EntityFlags & EF_ColideWithThings))
		return true;

	if (!AlreadySetUp)
	{
		tzmthing = this;
		tzorg = Origin;
	}
	//
	// the bounding box is extended by MAXRADIUS because mobj_ts are grouped
	// into mapblocks based on their origin point, and can overlap into adjacent
	// blocks by up to MAXRADIUS units
	//
	xl = MapBlock(Origin.x - Radius - XLevel->BlockMapOrgX - MAXRADIUS);
	xh = MapBlock(Origin.x + Radius - XLevel->BlockMapOrgX + MAXRADIUS);
	yl = MapBlock(Origin.y - Radius - XLevel->BlockMapOrgY - MAXRADIUS);
	yh = MapBlock(Origin.y + Radius - XLevel->BlockMapOrgY + MAXRADIUS);

	// xl->xh, yl->yh determine the mapblock set to search
	for (bx = xl; bx <= xh; bx++)
		for (by = yl; by <= yh; by++)
			if (!SV_BlockThingsIterator(bx, by, PIT_CheckOnmobjZ, NULL, NULL))
				return false;

	return true;
	unguard;
}

//=============================================================================
//
//  VEntity::FakeZMovement
//
//  Fake the zmovement so that we can check if a move is legal
//
//=============================================================================

void VEntity::FakeZMovement()
{
	guard(VEntity::FakeZMovement);
	//
	//  adjust height
	//
	tzorg.z += Velocity.z * host_frametime;
#if 0
	if (EntityFlags & EF_Float && Target)
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
	if (EntityFlags & EF_IsPlayer && EntityFlags & EF_Fly && !(tzorg.z <= FloorZ) && level->tictime & 2)
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
	unguard;
}

//=============================================================================
//
//  VEntity::CheckOnmobj
//
//  Checks if an object is above another object
//
//=============================================================================

VEntity* VEntity::CheckOnmobj()
{
	guard(VEntity::CheckOnmobj);
	boolean good;

	tzmthing = this;
	tzorg = Origin;
	FakeZMovement();
	good = TestMobjZ(true);

	return good ? NULL : onmobj;
 	unguard;
}

//===========================================================================
//
//  VEntity::RoughBlockCheck
//
//===========================================================================

VEntity* VEntity::RoughBlockCheck(int index)
{
	guard(VEntity::RoughBlockCheck);
	VEntity *link;

	for (link = XLevel->BlockLinks[index]; link; link = link->BlockMapNext)
	{
		if (eventRoughCheckThing(link))
		{
			return link;
		}
	}
	return NULL;
	unguard;
}

//===========================================================================
//
//	VEntity::RoughMonsterSearch
//
//	Searches though the surrounding mapblocks for monsters/players
//      distance is in MAPBLOCKUNITS
//
//===========================================================================

VEntity* VEntity::RoughMonsterSearch(int distance)
{
	guard(VEntity::RoughMonsterSearch);
	int blockX;
	int blockY;
	int startX, startY;
	int blockIndex;
	int firstStop;
	int secondStop;
	int thirdStop;
	int finalStop;
	int count;
	VEntity *newEnemy;

	startX = MapBlock(Origin.x - XLevel->BlockMapOrgX);
	startY = MapBlock(Origin.y - XLevel->BlockMapOrgY);

	if (startX >= 0 && startX < XLevel->BlockMapWidth &&
		startY >= 0 && startY < XLevel->BlockMapHeight)
	{
		newEnemy = RoughBlockCheck(startY * XLevel->BlockMapWidth + startX);
		if (newEnemy)
		{
			// found a target right away
			return newEnemy;
		}
	}
	for (count = 1; count <= distance; count++)
	{
		blockX = startX - count;
		blockY = startY - count;

		if (blockY < 0)
		{
			blockY = 0;
		}
		else if (blockY >= XLevel->BlockMapHeight)
		{
			blockY = XLevel->BlockMapHeight - 1;
		}
		if (blockX < 0)
		{
			blockX = 0;
		}
		else if (blockX >= XLevel->BlockMapWidth)
		{
			blockX = XLevel->BlockMapWidth - 1;
		}
		blockIndex = blockY * XLevel->BlockMapWidth + blockX;
		firstStop = startX + count;
		if (firstStop < 0)
		{
			continue;
		}
		if (firstStop >= XLevel->BlockMapWidth)
		{
			firstStop = XLevel->BlockMapWidth - 1;
		}
		secondStop = startY + count;
		if (secondStop < 0)
		{
			continue;
		}
		if (secondStop >= XLevel->BlockMapHeight)
		{
			secondStop = XLevel->BlockMapHeight - 1;
		}
		thirdStop = secondStop * XLevel->BlockMapWidth + blockX;
		secondStop = secondStop * XLevel->BlockMapWidth + firstStop;
		firstStop += blockY * XLevel->BlockMapWidth;
		finalStop = blockIndex;

		// Trace the first block section (along the top)
		for (; blockIndex <= firstStop; blockIndex++)
		{
			newEnemy = RoughBlockCheck(blockIndex);
			if (newEnemy)
			{
				return newEnemy;
			}
		}
		// Trace the second block section (right edge)
		for (blockIndex--; blockIndex <= secondStop;
			blockIndex += XLevel->BlockMapWidth)
		{
			newEnemy = RoughBlockCheck(blockIndex);
			if (newEnemy)
			{
				return newEnemy;
			}
		}
		// Trace the third block section (bottom edge)
		for (blockIndex -= XLevel->BlockMapWidth; blockIndex >= thirdStop;
			blockIndex--)
		{
			newEnemy = RoughBlockCheck(blockIndex);
			if (newEnemy)
			{
				return newEnemy;
			}
		}
		// Trace the final block section (left edge)
		for (blockIndex++; blockIndex > finalStop;
			blockIndex -= XLevel->BlockMapWidth)
		{
			newEnemy = RoughBlockCheck(blockIndex);
			if (newEnemy)
			{
				return newEnemy;
			}
		}
	}
	return NULL;
	unguard;
}

//==========================================================================
//
//	Entity.Remove
//
//==========================================================================

IMPLEMENT_FUNCTION(VEntity, Remove)
{
	P_GET_SELF;
	Self->Remove();
}

//==========================================================================
//
//	Entity.SetState
//
//==========================================================================

IMPLEMENT_FUNCTION(VEntity, SetState)
{
	P_GET_PTR(VState, state);
	P_GET_SELF;
	PR_Push(Self->SetState(state));
}

IMPLEMENT_FUNCTION(VEntity, SetInitialState)
{
	P_GET_PTR(VState, state);
	P_GET_SELF;
	Self->SetInitialState(state);
}

IMPLEMENT_FUNCTION(VEntity, FindState)
{
	P_GET_NAME(StateName);
	P_GET_SELF;
	RET_PTR(Self->FindState(StateName));
}

//==========================================================================
//
//	Entity.PlaySound
//
//==========================================================================

IMPLEMENT_FUNCTION(VEntity, PlaySound)
{
	P_GET_INT(Channel);
	P_GET_NAME(SoundName);
	P_GET_SELF;
	SV_StartSound(Self, S_ResolveEntitySound(Self->SoundClass,
		Self->SoundGender, SoundName), Channel, 127);
}

//==========================================================================
//
//	Entity.PlayFullVolumeSound
//
//==========================================================================

IMPLEMENT_FUNCTION(VEntity, PlayFullVolumeSound)
{
	P_GET_INT(Channel);
	P_GET_NAME(SoundName);
	P_GET_SELF;
	SV_StartSound(NULL, S_ResolveEntitySound(Self->SoundClass,
		Self->SoundGender, SoundName), Channel, 127);
}

//==========================================================================
//
//	Entity.StopSound
//
//==========================================================================

IMPLEMENT_FUNCTION(VEntity, StopSound)
{
	P_GET_INT(Channel);
	P_GET_SELF;
	SV_StopSound(Self, Channel);
}

//==========================================================================
//
//	Entity.CheckWater
//
//==========================================================================

IMPLEMENT_FUNCTION(VEntity, CheckWater)
{
	P_GET_SELF;
	RET_INT(Self->CheckWater());
}

//==========================================================================
//
//	Entity.CheckDropOff
//
//==========================================================================

IMPLEMENT_FUNCTION(VEntity, CheckDropOff)
{
	P_GET_SELF;
	RET_FLOAT(Self->CheckDropOff());
}

//==========================================================================
//
//	Entity.CheckPosition
//
//==========================================================================

IMPLEMENT_FUNCTION(VEntity, CheckPosition)
{
	P_GET_VEC(Pos);
	P_GET_SELF;
	RET_BOOL(Self->CheckPosition(Pos));
}

//==========================================================================
//
//	Entity.CheckRelPosition
//
//==========================================================================

IMPLEMENT_FUNCTION(VEntity, CheckRelPosition)
{
	P_GET_VEC(Pos);
	P_GET_SELF;
	RET_BOOL(Self->CheckRelPosition(Pos));
}

//==========================================================================
//
//	Entity.CheckSides
//
//==========================================================================

IMPLEMENT_FUNCTION(VEntity, CheckSides)
{
	P_GET_VEC(lsPos);
	P_GET_SELF;
	RET_BOOL(Self->CheckSides(lsPos));
}

//==========================================================================
//
//	Entity.TryMove
//
//==========================================================================

IMPLEMENT_FUNCTION(VEntity, TryMove)
{
	P_GET_VEC(Pos);
	P_GET_SELF;
	RET_BOOL(Self->TryMove(Pos));
}

//==========================================================================
//
//	Entity.TestMobjZ
//
//==========================================================================

IMPLEMENT_FUNCTION(VEntity, TestMobjZ)
{
	P_GET_SELF;
	RET_BOOL(Self->TestMobjZ());
}

//==========================================================================
//
//	Entity.SlideMove
//
//==========================================================================

IMPLEMENT_FUNCTION(VEntity, SlideMove)
{
	P_GET_SELF;
	Self->SlideMove();
}

//==========================================================================
//
//	Entity.SlideMove
//
//==========================================================================

IMPLEMENT_FUNCTION(VEntity, BounceWall)
{
	P_GET_FLOAT(overbounce);
	P_GET_SELF;
	Self->BounceWall(overbounce);
}

//==========================================================================
//
//	Entity.UpdateVelocity
//
//==========================================================================

IMPLEMENT_FUNCTION(VEntity, UpdateVelocity)
{
	P_GET_SELF;
	Self->UpdateVelocity();
}

//===========================================================================
//
//	VEntity.CheckOnmobj
//
//===========================================================================

IMPLEMENT_FUNCTION(VEntity, CheckOnmobj)
{
	P_GET_SELF;
	RET_REF(Self->CheckOnmobj());
}

//===========================================================================
//
//	VEntity.LinkToWorld
//
//===========================================================================

IMPLEMENT_FUNCTION(VEntity, LinkToWorld)
{
	P_GET_SELF;
	Self->LinkToWorld();
}

//===========================================================================
//
//	VEntity.UnlinkFromWorld
//
//===========================================================================

IMPLEMENT_FUNCTION(VEntity, UnlinkFromWorld)
{
	P_GET_SELF;
	Self->UnlinkFromWorld();
}

//===========================================================================
//
//	VEntity.CanSee
//
//===========================================================================

IMPLEMENT_FUNCTION(VEntity, CanSee)
{
	P_GET_REF(VEntity, Other);
	P_GET_SELF;
	RET_BOOL(Self->CanSee(Other));
}

//===========================================================================
//
//	VEntity.RoughMonsterSearch
//
//===========================================================================

IMPLEMENT_FUNCTION(VEntity, RoughMonsterSearch)
{
	P_GET_INT(Distance);
	P_GET_SELF;
	RET_REF(Self->RoughMonsterSearch(Distance));
}

//===========================================================================
//
//  VViewEntity::SetState
//
//===========================================================================

void VViewEntity::SetState(VState* InState)
{
	guard(VViewEntity::SetState);
	VState *state = InState;

	do
	{
		if (!state)
		{
			// Object removed itself.
			State = NULL;
			break;
		}
		State = state;
		StateTime = state->time;	// could be 0
		SpriteIndex = state->SpriteIndex;
		SpriteFrame = state->frame;
		ModelIndex = state->ModelIndex;
		ModelFrame = state->model_frame;
		NextState = state->nextstate;
		// Call action routine.
		P_PASS_SELF;
		svpr.ExecuteFunction(state->function);
		state = NextState;
	}
	while (state && !StateTime);	// An initial state of 0 could cycle through.
	unguard;
}

//==========================================================================
//
//	ViewEntity.SetState
//
//==========================================================================

IMPLEMENT_FUNCTION(VViewEntity, SetState)
{
	P_GET_PTR(VState, state);
	P_GET_SELF;
	Self->SetState(state);
}

//==========================================================================
//
//	VEntity::InitFuncIndexes
//
//==========================================================================

void VEntity::InitFuncIndexes()
{
	guard(VEntity::InitFuncIndexes);
	FIndex_Destroyed = StaticClass()->GetFunctionIndex("Destroyed");
	FIndex_Touch = StaticClass()->GetFunctionIndex("Touch");
	FIndex_BlockedByLine = StaticClass()->GetFunctionIndex("BlockedByLine");
	FIndex_ApplyFriction = StaticClass()->GetFunctionIndex("ApplyFriction");
	FIndex_PushLine = StaticClass()->GetFunctionIndex("PushLine");
	FIndex_HandleFloorclip = StaticClass()->GetFunctionIndex("HandleFloorclip");
	FIndex_CrossSpecialLine = StaticClass()->GetFunctionIndex("CrossSpecialLine");
	FIndex_SectorChanged = StaticClass()->GetFunctionIndex("SectorChanged");
	FIndex_RoughCheckThing = StaticClass()->GetFunctionIndex("RoughCheckThing");
	FIndex_GiveInventory = StaticClass()->GetFunctionIndex("GiveInventory");
	FIndex_TakeInventory = StaticClass()->GetFunctionIndex("TakeInventory");
	FIndex_CheckInventory = StaticClass()->GetFunctionIndex("CheckInventory");
	FIndex_GetSigilPieces = StaticClass()->GetFunctionIndex("GetSigilPieces");
	unguard;
}

//==========================================================================
//
//	EntInit
//
//==========================================================================

void EntInit()
{
	GGameInfo->tmtrace = &tmtrace;
	VEntity::InitFuncIndexes();
}

//==========================================================================
//
//	SV_FindClassFromEditorId
//
//==========================================================================

VClass* SV_FindClassFromEditorId(int Id)
{
	guard(SV_FindClassFromEditorId);
	for (int i = 0; i < VClass::GMobjInfos.Num(); i++)
	{
		if (Id == VClass::GMobjInfos[i].doomednum)
		{
			return VClass::GMobjInfos[i].class_id;
		}
	}
	return NULL;
	unguard;
}

//==========================================================================
//
//	SV_FindClassFromScriptId
//
//==========================================================================

VClass* SV_FindClassFromScriptId(int Id)
{
	guard(SV_FindClassFromScriptId);
	for (int i = 0; i < VClass::GScriptIds.Num(); i++)
	{
		if (Id == VClass::GScriptIds[i].doomednum)
		{
			return VClass::GScriptIds[i].class_id;
		}
	}
	return NULL;
	unguard;
}
