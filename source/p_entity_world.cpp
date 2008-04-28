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
//**	VEntity collision, physics and related methods.
//**
//**************************************************************************

// HEADER FILES ------------------------------------------------------------

#include "gamedefs.h"
#include "sv_local.h"

// MACROS ------------------------------------------------------------------

#define WATER_SINK_FACTOR		3.0
#define WATER_SINK_SPEED		0.5

// TYPES -------------------------------------------------------------------

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

struct tmtrace_t
{
	VEntity *Thing;
	VEntity* StepThing;
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
	// also keep track of the blocking line, for checking
	// against doortracks
	line_t *BlockingLine;

	// keep track of special lines as they are hit,
	// but don't process them until the move is proven valid
	TArray<line_t*>		SpecHit;

	VEntity *BlockingMobj;
};

//	Searches though the surrounding mapblocks for monsters/players
//      distance is in MAPBLOCKUNITS
class VRoughBlockSearchIterator : public VScriptIterator
{
private:
	VEntity*	Self;
	int			Distance;
	VEntity*	Ent;
	VEntity**	EntPtr;

	int			StartX;
	int			StartY;
	int			Count;
	int			CurrentEdge;
	int			BlockIndex;
	int			FirstStop;
	int			SecondStop;
	int			ThirdStop;
	int			FinalStop;

public:
	VRoughBlockSearchIterator(VEntity*, int, VEntity**);
	bool GetNext();
};

// EXTERNAL FUNCTION PROTOTYPES --------------------------------------------

// PUBLIC FUNCTION PROTOTYPES ----------------------------------------------

// PRIVATE FUNCTION PROTOTYPES ---------------------------------------------

// EXTERNAL DATA DECLARATIONS ----------------------------------------------

// PUBLIC DATA DEFINITIONS -------------------------------------------------

// PRIVATE DATA DEFINITIONS ------------------------------------------------

// CODE --------------------------------------------------------------------

//**************************************************************************
//
//	THING POSITION SETTING
//
//**************************************************************************

//=============================================================================
//
//	VEntity::CreateSecNodeList
//
// phares 3/14/98
//
//	Alters/creates the sector_list that shows what sectors the object resides in
//
//=============================================================================

void VEntity::CreateSecNodeList()
{
	guard(VEntity::CreateSecNodeList);
	int xl, xh, yl, yh, bx, by;
	msecnode_t* Node;

	// First, clear out the existing Thing fields. As each node is
	// added or verified as needed, Thing will be set properly. When
	// finished, delete all nodes where Thing is still NULL. These
	// represent the sectors the Thing has vacated.

	Node = XLevel->SectorList;
	while (Node)
	{
		Node->Thing = NULL;
		Node = Node->TNext;
	}

	float tmbbox[4];
	tmbbox[BOXTOP] = Origin.y + Radius;
	tmbbox[BOXBOTTOM] = Origin.y - Radius;
	tmbbox[BOXRIGHT] = Origin.x + Radius;
	tmbbox[BOXLEFT] = Origin.x - Radius;

	validcount++; // used to make sure we only process a line once

	xl = MapBlock(tmbbox[BOXLEFT] - XLevel->BlockMapOrgX);
	xh = MapBlock(tmbbox[BOXRIGHT] - XLevel->BlockMapOrgX);
	yl = MapBlock(tmbbox[BOXBOTTOM] - XLevel->BlockMapOrgY);
	yh = MapBlock(tmbbox[BOXTOP] - XLevel->BlockMapOrgY);

	for (bx = xl; bx <= xh; bx++)
	{
		for (by = yl; by <= yh; by++)
		{
			line_t* ld;
			for (VBlockLinesIterator It(this, bx, by, &ld); It.GetNext(); )
			{
				//	Locates all the sectors the object is in by looking at
				// the lines that cross through it. You have already decided
				// that the object is allowed at this location, so don't
				// bother with checking impassable or blocking lines.
				if (tmbbox[BOXRIGHT] <= ld->bbox[BOXLEFT] ||
					tmbbox[BOXLEFT] >= ld->bbox[BOXRIGHT] ||
					tmbbox[BOXTOP] <= ld->bbox[BOXBOTTOM] ||
					tmbbox[BOXBOTTOM] >= ld->bbox[BOXTOP])
				{
					continue;
				}

				if (P_BoxOnLineSide(tmbbox, ld) != -1)
				{
					continue;
				}

				// This line crosses through the object.

				// Collect the sector(s) from the line and add to the
				// SectorList you're examining. If the Thing ends up being
				// allowed to move to this position, then the sector_list will
				// be attached to the Thing's VEntity at TouchingSectorList.

				XLevel->SectorList = XLevel->AddSecnode(ld->frontsector,
					this, XLevel->SectorList);

				// Don't assume all lines are 2-sided, since some Things like
				// MT_TFOG are allowed regardless of whether their radius
				// takes them beyond an impassable linedef.

				// killough 3/27/98, 4/4/98:
				// Use sidedefs instead of 2s flag to determine two-sidedness.

				if (ld->backsector)
				{
					XLevel->SectorList = XLevel->AddSecnode(ld->backsector,
						this, XLevel->SectorList);
				}
			}
		}
	}

	// Add the sector of the (x,y) point to sector_list.
	XLevel->SectorList = XLevel->AddSecnode(Sector, this, XLevel->SectorList);

	// Now delete any nodes that won't be used. These are the ones where
	// Thing is still NULL.

	Node = XLevel->SectorList;
	while (Node)
	{
		if (Node->Thing == NULL)
		{
			if (Node == XLevel->SectorList)
			{
				XLevel->SectorList = Node->TNext;
			}
			Node = XLevel->DelSecnode(Node);
		}
		else
		{
			Node = Node->TNext;
		}
	}
	unguard;
}

//==========================================================================
//
//	VEntity::UnlinkFromWorld
//
//	Unlinks a thing from block map and sectors. On each position change,
// BLOCKMAP and other lookups maintaining lists ot things inside these
// structures need to be updated.
//
//==========================================================================

void VEntity::UnlinkFromWorld()
{
	guard(SV_UnlinkFromWorld);
	if (!SubSector)
	{
		return;
	}

	if (!(EntityFlags & EF_NoSector))
	{
		// invisible things don't need to be in sector list
		// unlink from subsector
		if (SNext)
		{
			SNext->SPrev = SPrev;
		}
		if (SPrev)
		{
			SPrev->SNext = SNext;
		}
		else
		{
			Sector->ThingList = SNext;
		}
		SNext = NULL;
		SPrev = NULL;

		// phares 3/14/98
		//
		//	Save the sector list pointed to by TouchingSectorList. In
		// LinkToWorld, we'll keep any nodes that represent sectors the Thing
		// still touches. We'll add new ones then, and delete any nodes for
		// sectors the Thing has vacated. Then we'll put it back into
		// TouchingSectorList. It's done this way to avoid a lot of
		// deleting/creating for nodes, when most of the time you just get
		// back what you deleted anyway.
		//
		//	If this Thing is being removed entirely, then the calling routine
		// will clear out the nodes in sector_list.
		//
		XLevel->DelSectorList();
		XLevel->SectorList = TouchingSectorList;
		TouchingSectorList = NULL; //to be restored by LinkToWorld
	}

	if (!(EntityFlags & EF_NoBlockmap))
	{
		//	Inert things don't need to be in blockmap
		//	Unlink from block map
		if (BlockMapNext)
		{
			BlockMapNext->BlockMapPrev = BlockMapPrev;
		}
	
		if (BlockMapPrev)
		{
			BlockMapPrev->BlockMapNext = BlockMapNext;
		}
		else
		{
			int blockx = MapBlock(Origin.x - XLevel->BlockMapOrgX);
			int blocky = MapBlock(Origin.y - XLevel->BlockMapOrgY);

			if (blockx >= 0 && blockx < XLevel->BlockMapWidth &&
				blocky >= 0 && blocky < XLevel->BlockMapHeight)
			{
				check(XLevel->BlockLinks[blocky * XLevel->BlockMapWidth + blockx] == this);
				XLevel->BlockLinks[blocky * XLevel->BlockMapWidth + blockx] =
					BlockMapNext;
			}
		}
	}
	SubSector = NULL;
	Sector = NULL;
	unguardf(("(%s)", GetClass()->GetName()));
}

//==========================================================================
//
//	VEntity::LinkToWorld
//
//	Links a thing into both a block and a subsector based on it's x y.
//	Sets thing->subsector properly
//
//==========================================================================

void VEntity::LinkToWorld()
{
	guard(VEntity::LinkToWorld);
	subsector_t*	ss;
	sec_region_t*	reg;
	sec_region_t*	r;

	if (SubSector)
	{
		UnlinkFromWorld();
	}

	// link into subsector
	ss = XLevel->PointInSubsector(Origin);
	reg = SV_FindThingGap(ss->sector->botregion, Origin,
		Origin.z, Origin.z + Height);
	SubSector = ss;
	Sector = ss->sector;

	r = reg;
	while (r->floor->flags && r->prev)
	{
		r = r->prev;
	}
	Floor = r->floor;
	FloorZ = r->floor->GetPointZ(Origin);

	r = reg;
	while (r->ceiling->flags && r->next)
	{
		r = r->next;
	}
	Ceiling = r->ceiling;
	CeilingZ = r->ceiling->GetPointZ(Origin);

	// link into sector
	if (!(EntityFlags & EF_NoSector))
	{
		// invisible things don't go into the sector links
		VEntity** Link = &Sector->ThingList;
		SPrev = NULL;
		SNext = *Link;
		if (*Link)
		{
			(*Link)->SPrev = this;
		}
		*Link = this;

		// phares 3/16/98
		//
		// If sector_list isn't NULL, it has a collection of sector
		// nodes that were just removed from this Thing.
		//
		// Collect the sectors the object will live in by looking at
		// the existing sector_list and adding new nodes and deleting
		// obsolete ones.
		//
		// When a node is deleted, its sector links (the links starting
		// at sector_t->touching_thinglist) are broken. When a node is
		// added, new sector links are created.
		CreateSecNodeList();
		TouchingSectorList = XLevel->SectorList;	// Attach to thing
		XLevel->SectorList = NULL;		// clear for next time
	}
	else
	{
		XLevel->DelSectorList();
	}

	// link into blockmap
	if (!(EntityFlags & EF_NoBlockmap))
	{
		// inert things don't need to be in blockmap
		int blockx = MapBlock(Origin.x - XLevel->BlockMapOrgX);
		int blocky = MapBlock(Origin.y - XLevel->BlockMapOrgY);

		if (blockx >= 0 && blockx < XLevel->BlockMapWidth &&
			blocky >= 0 && blocky < XLevel->BlockMapHeight)
		{
			VEntity** link = &XLevel->BlockLinks[
				blocky * XLevel->BlockMapWidth + blockx];
			BlockMapPrev = NULL;
			BlockMapNext = *link;
			if (*link)
			{
				(*link)->BlockMapPrev = this;
			}

			*link = this;
		}
		else
		{
			// thing is off the map
			BlockMapNext = BlockMapPrev = NULL;
		}
	}
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
				{
					WaterLevel = 3;
				}
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

//==========================================================================
//
//	VEntity::PIT_CheckThing
//
//==========================================================================

bool VEntity::PIT_CheckThing(void* arg, VEntity *Other)
{
	guardSlow(VEntity::PIT_CheckThing);
	float blockdist;
	cptrace_t cptrace = *(cptrace_t*)arg;

	// can't hit thing
	if (!(Other->EntityFlags & EF_Solid))
	{
		return true;
	}

	// don't clip against self
	if (Other == cptrace.Thing)
	{
		return true;
	}

	blockdist = Other->Radius + cptrace.Thing->Radius;

	if (fabs(Other->Origin.x - cptrace.Pos.x) >= blockdist ||
		fabs(Other->Origin.y - cptrace.Pos.y) >= blockdist)
	{
		// didn't hit it
		return true;
	}

	if ((cptrace.Thing->EntityFlags & EF_PassMobj) ||
		(cptrace.Thing->EntityFlags & EF_Missile) ||
		(Other->EntityFlags & EF_ActLikeBridge))
	{
		//	Prevent some objects from overlapping
		if (cptrace.Thing->EntityFlags & Other->EntityFlags & EF_DontOverlap)
		{
			return false;
		}
		// check if a mobj passed over/under another object
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
//	VEntity::PIT_CheckLine
//
//  Adjusts cptrace.FoorZ and cptrace.CeilingZ as lines are contacted
//
//==========================================================================

bool VEntity::PIT_CheckLine(void* arg, line_t * ld)
{
	guardSlow(VEntity::PIT_CheckLine);
	TVec hit_point;
	opening_t *open;
	cptrace_t& cptrace = *(cptrace_t*)arg;

	if (cptrace.bbox[BOXRIGHT] <= ld->bbox[BOXLEFT] ||
		cptrace.bbox[BOXLEFT] >= ld->bbox[BOXRIGHT] ||
		cptrace.bbox[BOXTOP] <= ld->bbox[BOXBOTTOM] ||
		cptrace.bbox[BOXBOTTOM] >= ld->bbox[BOXTOP])
	{
		return true;
	}

	if (P_BoxOnLineSide(&cptrace.bbox[0], ld) != -1)
	{
		return true;
	}

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
	open = SV_LineOpenings(ld, hit_point, SPF_NOBLOCKING);
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
		{
			cptrace.DropOffZ = open->lowfloor;
		}

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
	cptrace_t cptrace;

	cptrace.Thing = this;

	cptrace.Pos = Pos;

	cptrace.bbox[BOXTOP] = Pos.y + Radius;
	cptrace.bbox[BOXBOTTOM] = Pos.y - Radius;
	cptrace.bbox[BOXRIGHT] = Pos.x + Radius;
	cptrace.bbox[BOXLEFT] = Pos.x - Radius;

	newsubsec = XLevel->PointInSubsector(Pos);

	// The base floor / ceiling is from the subsector that contains the point.
	// Any contacted lines the step closer together will adjust them.
	gap = SV_FindThingGap(newsubsec->sector->botregion, Pos,
		Pos.z, Pos.z + Height);
	reg = gap;
	while (reg->prev && reg->floor->flags & SPF_NOBLOCKING)
	{
		reg = reg->prev;
	}
	cptrace.Floor = reg->floor;
	cptrace.FloorZ = reg->floor->GetPointZ(Pos);
	cptrace.DropOffZ = cptrace.FloorZ;
	reg = gap;
	while (reg->next && reg->ceiling->flags & SPF_NOBLOCKING)
	{
		reg = reg->next;
	}
	cptrace.Ceiling = reg->ceiling;
	cptrace.CeilingZ = reg->ceiling->GetPointZ(Pos);

	validcount++;

	if (EntityFlags & EF_ColideWithThings)
	{
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
		{
			for (by = yl; by <= yh; by++)
			{
				VEntity* Ent;
				for (VBlockThingsIterator It(this, bx, by, &Ent); It.GetNext();)
				{
					if (!PIT_CheckThing(&cptrace, Ent))
					{
						return false;
					}
				}
			}
		}
	}

	if (EntityFlags & EF_ColideWithWorld)
	{
		// check lines
		xl = MapBlock(cptrace.bbox[BOXLEFT] - XLevel->BlockMapOrgX);
		xh = MapBlock(cptrace.bbox[BOXRIGHT] - XLevel->BlockMapOrgX);
		yl = MapBlock(cptrace.bbox[BOXBOTTOM] - XLevel->BlockMapOrgY);
		yh = MapBlock(cptrace.bbox[BOXTOP] - XLevel->BlockMapOrgY);

		for (bx = xl; bx <= xh; bx++)
		{
			for (by = yl; by <= yh; by++)
			{
				line_t*		ld;
				for (VBlockLinesIterator It(this, bx, by, &ld); It.GetNext(); )
				{
					if (!PIT_CheckLine(&cptrace, ld))
					{
						return false;
					}
				}
			}
		}
	}

	return true;
	unguard;
}

//**************************************************************************
//
//  MOVEMENT CLIPPING
//
//**************************************************************************

//==========================================================================
//
//	VEntity::PIT_CheckRelThing
//
//==========================================================================

bool VEntity::PIT_CheckRelThing(void* arg, VEntity *Other)
{
	guardSlow(VEntity::PIT_CheckRelThing);
	float blockdist;
	tmtrace_t& tmtrace = *(tmtrace_t*)arg;

	// don't clip against self
	if (Other == tmtrace.Thing)
	{
		return true;
	}

	blockdist = Other->Radius + tmtrace.Thing->Radius;

	if (fabs(Other->Origin.x - tmtrace.End.x) >= blockdist ||
		fabs(Other->Origin.y - tmtrace.End.y) >= blockdist)
	{
		// didn't hit it
		return true;
	}

	tmtrace.BlockingMobj = Other;
	if ((!(tmtrace.Thing->EntityFlags & EF_Float) ||
		!(tmtrace.Thing->EntityFlags & EF_Missile) ||
		!(tmtrace.Thing->EntityFlags & EF_NoGravity)) &&
		(Other->EntityFlags & EF_Solid) &&
		(Other->EntityFlags & EF_ActLikeBridge))
	{
		// allow actors to walk on other actors as well as floors
		if (Other->Origin.z + Other->Height >= tmtrace.FloorZ &&
			Other->Origin.z + Other->Height <= tmtrace.Thing->Origin.z +
			tmtrace.Thing->MaxStepHeight)
		{
			tmtrace.StepThing = Other;
			tmtrace.FloorZ = Other->Origin.z + Other->Height;
		}
	}
	//if (!(tmtrace.Thing->EntityFlags & VEntity::EF_NoPassMobj) || Actor(Other).bSpecial)
	if ((tmtrace.Thing->EntityFlags & EF_PassMobj) ||
		(tmtrace.Thing->EntityFlags & EF_Missile) ||
		(Other->EntityFlags & EF_ActLikeBridge))
	{
		//	Prevent some objects from overlapping
		if (tmtrace.Thing->EntityFlags & Other->EntityFlags & EF_DontOverlap)
		{
			return false;
		}
		// check if a mobj passed over/under another object
		if (tmtrace.End.z + 0.00001 >= Other->Origin.z + Other->Height)
		{
			return true;	// overhead
		}
		if (tmtrace.End.z + tmtrace.Thing->Height <= Other->Origin.z + 0.00001)
		{
			return true;	// underneath
		}
	}

	return tmtrace.Thing->eventTouch(Other);
	unguardSlow;
}

//==========================================================================
//
//	VEntity::PIT_CheckRelLine
//
//  Adjusts tmtrace.FloorZ and tmtrace.CeilingZ as lines are contacted
//
//==========================================================================

bool VEntity::PIT_CheckRelLine(void* arg, line_t * ld)
{
	guardSlow(VEntity::PIT_CheckRelLine);
	TVec hit_point;
	opening_t *open;
	tmtrace_t& tmtrace = *(tmtrace_t*)arg;

	if (tmtrace.BBox[BOXRIGHT] <= ld->bbox[BOXLEFT] ||
		tmtrace.BBox[BOXLEFT] >= ld->bbox[BOXRIGHT] ||
		tmtrace.BBox[BOXTOP] <= ld->bbox[BOXBOTTOM] ||
		tmtrace.BBox[BOXBOTTOM] >= ld->bbox[BOXTOP])
	{
		return true;
	}

	if (P_BoxOnLineSide(&tmtrace.BBox[0], ld) != -1)
	{
		return true;
	}

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
		// mark the line as blocking line
		tmtrace.BlockingLine = ld;
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
	open = SV_LineOpenings(ld, hit_point, SPF_NOBLOCKING);
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
		{
			tmtrace.DropOffZ = open->lowfloor;
		}

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
		tmtrace.SpecHit.Append(ld);
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
//  VEntity *BlockingMobj = pointer to thing that blocked position (NULL if not
//   blocked, or blocked by a line).
//
//==========================================================================

bool VEntity::CheckRelPosition(tmtrace_t& tmtrace, TVec Pos)
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
	VEntity* thingblocker;
	VEntity* fakedblocker;

	tmtrace.Thing = this;

	tmtrace.End = Pos;

	tmtrace.BBox[BOXTOP] = Pos.y + Radius;
	tmtrace.BBox[BOXBOTTOM] = Pos.y - Radius;
	tmtrace.BBox[BOXRIGHT] = Pos.x + Radius;
	tmtrace.BBox[BOXLEFT] = Pos.x - Radius;

	newsubsec = XLevel->PointInSubsector(Pos);
	tmtrace.CeilingLine = NULL;

	// The base floor / ceiling is from the subsector
	// that contains the point.
	// Any contacted lines the step closer together
	// will adjust them.
	gap = SV_FindThingGap(newsubsec->sector->botregion, tmtrace.End,
		tmtrace.End.z, tmtrace.End.z + Height);
	reg = gap;
	while (reg->prev && reg->floor->flags & SPF_NOBLOCKING)
	{
		reg = reg->prev;
	}
	tmtrace.Floor = reg->floor;
	tmtrace.FloorZ = reg->floor->GetPointZ(tmtrace.End);
	tmtrace.DropOffZ = tmtrace.FloorZ;
	reg = gap;
	while (reg->next && reg->ceiling->flags & SPF_NOBLOCKING)
	{
		reg = reg->next;
	}
	tmtrace.Ceiling = reg->ceiling;
	tmtrace.CeilingZ = reg->ceiling->GetPointZ(tmtrace.End);

	validcount++;
	tmtrace.SpecHit.Clear();

	tmtrace.BlockingMobj = NULL;
	tmtrace.StepThing = NULL;
	thingblocker = NULL;
	fakedblocker = NULL;

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
		{
			for (by = yl; by <= yh; by++)
			{
				VEntity* Ent;
				for (VBlockThingsIterator It(this, bx, by, &Ent); It.GetNext();)
				{
					if (!PIT_CheckRelThing(&tmtrace, Ent))
					{
						// continue checking for other things in to see if we hit something
						if (tmtrace.BlockingMobj == NULL)
						{
							// slammed into something
							return false;
						}
						else if (!tmtrace.BlockingMobj->Player &&
							!(EntityFlags & VEntity::EF_Float) &&
							!(EntityFlags & VEntity::EF_Missile) &&
							tmtrace.BlockingMobj->Origin.z + tmtrace.BlockingMobj->Height - Origin.z <= MaxStepHeight)
						{
							if (!thingblocker || tmtrace.BlockingMobj->Origin.z > thingblocker->Origin.z)
							{
								thingblocker = tmtrace.BlockingMobj;
							}
							tmtrace.BlockingMobj = NULL;
						}
						else if (Player && Origin.z + Height - tmtrace.BlockingMobj->Origin.z <= MaxStepHeight)
						{
							if (thingblocker)
							{ // something to step up on, set it as
							  // the blocker so that we don't step up
								return false;
							}
							// nothing is blocking, but this object potentially could
							// if there is something else to step on
							fakedblocker = tmtrace.BlockingMobj;
							tmtrace.BlockingMobj = NULL;
						}
						else
						{ // blocking
							return false;
						}
					}
				}
			}
		}
	}

	float thingdropoffz = tmtrace.FloorZ;
	tmtrace.FloorZ = tmtrace.DropOffZ;
	tmtrace.BlockingMobj = NULL;
	// check lines
	validcount++;

	if (EntityFlags & EF_ColideWithWorld)
	{
		xl = MapBlock(tmtrace.BBox[BOXLEFT] - XLevel->BlockMapOrgX);
		xh = MapBlock(tmtrace.BBox[BOXRIGHT] - XLevel->BlockMapOrgX);
		yl = MapBlock(tmtrace.BBox[BOXBOTTOM] - XLevel->BlockMapOrgY);
		yh = MapBlock(tmtrace.BBox[BOXTOP] - XLevel->BlockMapOrgY);

		for (bx = xl; bx <= xh; bx++)
		{
			for (by = yl; by <= yh; by++)
			{
				line_t*		ld;
				for (VBlockLinesIterator It(this, bx, by, &ld); It.GetNext(); )
				{
					if (!PIT_CheckRelLine(&tmtrace, ld))
					{
						return false;
					}
				}
			}
		}

		if (tmtrace.CeilingZ - tmtrace.FloorZ < Height)
		{
			return false;
		}
	}

	if (tmtrace.StepThing != NULL)
	{
		tmtrace.DropOffZ = thingdropoffz;
	}

	tmtrace.BlockingMobj = thingblocker;
	if (tmtrace.BlockingMobj)
	{
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

bool VEntity::TryMove(tmtrace_t& tmtrace, TVec newPos, bool AllowDropOff)
{
	guard(VEntity::TryMove);
	bool check;
	TVec oldorg;
	int side;
	int oldside;
	line_t *ld;
	sector_t* OldSec = Sector;

	check = CheckRelPosition(tmtrace, newPos);
	tmtrace.TraceFlags &= ~tmtrace_t::TF_FloatOk;
	if (!check)
	{
		VEntity *O = tmtrace.BlockingMobj;
		if (!O || O->EntityFlags & EF_IsPlayer || !(EntityFlags & EF_IsPlayer) ||
			O->Origin.z + O->Height - Origin.z > MaxStepHeight ||
			O->CeilingZ - (O->Origin.z + O->Height) < Height ||
			tmtrace.CeilingZ - (O->Origin.z + O->Height) < Height)
		{
			eventPushLine(&tmtrace);
			return false;
		}
	}

	if (EntityFlags & EF_ColideWithWorld)
	{
		if (tmtrace.CeilingZ - tmtrace.FloorZ < Height)
		{
			// Doesn't fit
			eventPushLine(&tmtrace);
			return false;
		}

		tmtrace.TraceFlags |= tmtrace_t::TF_FloatOk;

		if (tmtrace.CeilingZ - Origin.z < Height && !(EntityFlags & EF_Fly) &&
			!(EntityFlags & EF_IgnoreCeilingStep))
		{
			// mobj must lower itself to fit
			eventPushLine(&tmtrace);
			return false;
		}
		if (EntityFlags & EF_Fly)
		{
			// When flying, slide up or down blocking lines until the actor
			// is not blocked.
			if (Origin.z + Height > tmtrace.CeilingZ)
			{
				Velocity.z = -8.0 * 35.0;
				eventPushLine(&tmtrace);
				return false;
			}
			else if (Origin.z < tmtrace.FloorZ
				&& tmtrace.FloorZ - tmtrace.DropOffZ > MaxStepHeight)
			{
				Velocity.z = 8.0 * 35.0;
				eventPushLine(&tmtrace);
				return false;
			}
		}
		if (!(EntityFlags & EF_IgnoreFloorStep))
		{
			if (tmtrace.FloorZ - Origin.z > MaxStepHeight)
			{
				// Too big a step up
				eventPushLine(&tmtrace);
				return false;
			}
			if (Origin.z < tmtrace.FloorZ)
			{
				// Check to make sure there's nothing in the way for the step up
				if (TestMobjZ(TVec(newPos.x, newPos.y, tmtrace.FloorZ)))
				{
					eventPushLine(&tmtrace);
					return false;
				}
			}
		}
// Only Heretic
//		if (bMissile && tmtrace->FloorZ > Origin.z)
//		{
//			eventPushLine();
//		}
		// killough 3/15/98: Allow certain objects to drop off
		if ((!AllowDropOff && !(EntityFlags & EF_DropOff) &&
			!(EntityFlags & EF_Float) && !(EntityFlags & EF_Missile)) ||
			(EntityFlags & EF_NoDropOff))
		{
			if (!(EntityFlags & EF_AvoidingDropoff))
			{
				float floorz = tmtrace.FloorZ;
				// [RH] If the thing is standing on something, use its current z as the floorz.
				// This is so that it does not walk off of things onto a drop off.
				if (EntityFlags & EF_OnMobj)
				{
					floorz = MAX(Origin.z, tmtrace.FloorZ);
				}

				if ((floorz - tmtrace.DropOffZ > MaxDropoffHeight) &&
					!(EntityFlags & EF_Blasted))
				{
					// Can't move over a dropoff unless it's been blasted
					return false;
				}
			}
			else
			{
				// special logic to move a monster off a dropoff
				// this intentionally does not check for standing on things.
				if (FloorZ - tmtrace.FloorZ > MaxDropoffHeight ||
					DropOffZ - tmtrace.DropOffZ > MaxDropoffHeight)
				{
					return false;
				}
			}
		}
		if (EntityFlags & EF_CantLeaveFloorpic &&
			(tmtrace.Floor->pic != Floor->pic || tmtrace.FloorZ != Origin.z))
		{
			// must stay within a sector of a certain floor type
			return false;
		}
	}

	bool OldAboveFakeFloor = false;
	bool OldAboveFakeCeiling = false;
	if (Sector->heightsec)
	{
		float EyeZ = Player ? Player->ViewOrg.z : Origin.z + Height * 0.5;
		OldAboveFakeFloor = EyeZ > Sector->heightsec->floor.GetPointZ(Origin);
		OldAboveFakeCeiling = EyeZ > Sector->heightsec->ceiling.GetPointZ(
			Origin);
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
		while (tmtrace.SpecHit.Num() > 0)
		{
			// see if the line was crossed
			ld = tmtrace.SpecHit[tmtrace.SpecHit.Num() - 1];
			tmtrace.SpecHit.SetNum(tmtrace.SpecHit.Num() - 1);
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

	eventCheckForSectorActions(OldSec, OldAboveFakeFloor,
		OldAboveFakeCeiling);

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

//==========================================================================
//
//  VEntity::ClipVelocity
//
//  Slide off of the impacting object
//
//==========================================================================

TVec VEntity::ClipVelocity(const TVec& in, const TVec& normal, float overbounce)
{
	return in - normal * (DotProduct(in, normal) * overbounce);
}

//==========================================================================
//
//  VEntity::SlidePathTraverse
//
//==========================================================================

void VEntity::SlidePathTraverse(float& BestSlideFrac, line_t*& BestSlideLine,
	float x, float y, float StepVelScale)
{
	guard(VEntity::SlidePathTraverse);
	TVec SlideOrg(x, y, Origin.z);
	TVec SlideDir = Velocity * StepVelScale;
	intercept_t*	in;
	for (VPathTraverse It(this, &in, x, y, x + SlideDir.x,
		y + SlideDir.y, PT_ADDLINES); It.GetNext(); )
	{
		if (!(in->Flags & intercept_t::IF_IsALine))
		{
			Host_Error("PTR_SlideTraverse: not a line?");
		}

		line_t* li = in->line;

		// set openrange, opentop, openbottom
		TVec hit_point = SlideOrg + in->frac * SlideDir;
		opening_t* open = SV_LineOpenings(li, hit_point, SPF_NOBLOCKING);
		open = SV_FindOpening(open, Origin.z, Origin.z + Height);

		if (li->flags & ML_TWOSIDED)
		{
			// set openrange, opentop, openbottom
			hit_point = SlideOrg + in->frac * SlideDir;
			open = SV_LineOpenings(li, hit_point, SPF_NOBLOCKING);
			open = SV_FindOpening(open, Origin.z, Origin.z + Height);

			if (open && (open->range >= Height) &&	//  fits
				(open->top - Origin.z >= Height) &&	// mobj is not too high
				(open->bottom - Origin.z <= MaxStepHeight))	// not too big a step up
			{
				// this line doesn't block movement
				if (Origin.z < open->bottom)
				{
					// Check to make sure there's nothing in the way for the step up
					TVec CheckOrg = Origin;
					CheckOrg.z = open->bottom;
					if (!TestMobjZ(CheckOrg))
					{
						continue;
					}
				}
				else
				{
					continue;
				}
			}
		}
		else
		{
			if (li->PointOnSide(Origin))
			{
				// don't hit the back side
				continue;
			}
		}

		// the line blocks movement,
		// see if it is closer than best so far
		if (in->frac < BestSlideFrac)
		{
			BestSlideFrac = in->frac;
			BestSlideLine = li;
		}

		break;	// stop
	}
	unguard;
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

void VEntity::SlideMove(float StepVelScale)
{
	guard(VEntity::SlideMove);
	float leadx;
	float leady;
	float trailx;
	float traily;
	float newx;
	float newy;
	int hitcount;
	tmtrace_t tmtrace;

	hitcount = 0;

	float XMove = Velocity.x * StepVelScale;
	float YMove = Velocity.y * StepVelScale;
	do
	{
		if (++hitcount == 3)
		{
			// don't loop forever
			if (!TryMove(tmtrace, TVec(Origin.x, Origin.y + YMove, Origin.z),
				true))
			{
				TryMove(tmtrace, TVec(Origin.x + XMove, Origin.y, Origin.z),
					true);
			}
			return;
		}

		// trace along the three leading corners
		if (XMove > 0.0)
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

		float BestSlideFrac = 1.00001f;
		line_t* BestSlideLine = NULL;

		SlidePathTraverse(BestSlideFrac, BestSlideLine, leadx, leady, StepVelScale);
		SlidePathTraverse(BestSlideFrac, BestSlideLine, trailx, leady, StepVelScale);
		SlidePathTraverse(BestSlideFrac, BestSlideLine, leadx, traily, StepVelScale);

		// move up to the wall
		if (BestSlideFrac == 1.00001f)
		{
			// the move must have hit the middle, so stairstep
			if (!TryMove(tmtrace, TVec(Origin.x, Origin.y + YMove, Origin.z),
				true))
			{
				TryMove(tmtrace, TVec(Origin.x + XMove, Origin.y, Origin.z),
					true);
			}
			return;
		}

		// fudge a bit to make sure it doesn't hit
		BestSlideFrac -= 0.03125;
		if (BestSlideFrac > 0.0)
		{
			newx = XMove * BestSlideFrac;
			newy = YMove * BestSlideFrac;

			if (!TryMove(tmtrace, TVec(Origin.x + newx, Origin.y + newy,
				Origin.z), true))
			{
				if (!TryMove(tmtrace, TVec(Origin.x, Origin.y + YMove,
					Origin.z), true))
				{
					TryMove(tmtrace, TVec(Origin.x + XMove, Origin.y,
						Origin.z), true);
				}
				return;
			}
		}

		// Now continue along the wall.
		// First calculate remainder.
		BestSlideFrac = 1.0 - (BestSlideFrac + 0.03125);

		if (BestSlideFrac > 1.0)
		{
			BestSlideFrac = 1.0;
		}

		if (BestSlideFrac <= 0.0)
		{
			return;
		}

		// clip the moves
		Velocity = ClipVelocity(Velocity * BestSlideFrac,
			BestSlideLine->normal, 1.0);
		XMove = Velocity.x * StepVelScale;
		YMove = Velocity.y * StepVelScale;
	}
	while (!TryMove(tmtrace, TVec(Origin.x + XMove, Origin.y + YMove,
		Origin.z), true));
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
//  VEntity::BounceWall
//
//============================================================================

void VEntity::BounceWall(float overbounce)
{
	guard(VEntity::BounceWall);
	TVec SlideOrg;
	if (Velocity.x > 0.0)
	{
		SlideOrg.x = Origin.x + Radius;
	}
	else
	{
		SlideOrg.x = Origin.x - Radius;
	}
	if (Velocity.y > 0.0)
	{
		SlideOrg.y = Origin.y + Radius;
	}
	else
	{
		SlideOrg.y = Origin.y - Radius;
	}
	SlideOrg.z = Origin.z;
	TVec SlideDir = Velocity * host_frametime;
	line_t* BestSlideLine = NULL;
	intercept_t* in;
	for (VPathTraverse It(this, &in, SlideOrg.x, SlideOrg.y, SlideOrg.x +
		SlideDir.x, SlideOrg.y + SlideDir.y, PT_ADDLINES); It.GetNext(); )
	{
		if (!(in->Flags & intercept_t::IF_IsALine))
		{
			Host_Error("PTR_BounceTraverse: not a line?");
		}

		line_t* li = in->line;
		if (li->flags & ML_TWOSIDED)
		{
			TVec hit_point = SlideOrg + in->frac * SlideDir;
			// set openrange, opentop, openbottom
			opening_t* open = SV_LineOpenings(li, hit_point, SPF_NOBLOCKING);
			open = SV_FindOpening(open, Origin.z, Origin.z + Height);
			if (open && open->range >= Height &&	// fits
				Origin.z + Height <= open->top &&
				Origin.z >= open->bottom)	// mobj is not too high
			{
				continue;	// this line doesn't block movement
			}
		}
		else
		{
			if (li->PointOnSide(Origin))
			{
				continue;	// don't hit the back side
			}
		}

		BestSlideLine = li;
		break;	// don't bother going farther
	}
	if (BestSlideLine)
	{
		Velocity = ClipVelocity(Velocity, BestSlideLine->normal, overbounce);
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
	float startvelz, sinkspeed;

/*	if (Origin.z <= FloorZ && !Velocity.x && !Velocity.y &&
		!Velocity.z && !bCountKill && !(EntityFlags & EF_IsPlayer))
	{
		//  No gravity for non-moving things on ground to prevent
		// static objects from sliding on slopes
		return;
	}*/

	//  Don't add gravity if standing on slope with normal.z > 0.7 (aprox
	// 45 degrees)
	if (!(EntityFlags & EF_NoGravity) && (Origin.z > FloorZ ||
		Floor->normal.z < 0.7))
	{
		//	Add gravity
		if (WaterLevel < 2)
		{
			Velocity.z -= Gravity * Level->Gravity * Sector->Gravity *
				host_frametime;
		}
		else if (!(EntityFlags & EF_IsPlayer) || Health <= 0)
		{
			// Water Gravity
			Velocity.z -= Gravity * Level->Gravity * Sector->Gravity / 10.0 *
				host_frametime;
			startvelz = Velocity.z;

			if (EntityFlags & EF_Corpse)
				sinkspeed = -WATER_SINK_SPEED / 3.0;
			else
				sinkspeed = -WATER_SINK_SPEED;

			if (Velocity.z < sinkspeed)
			{
				if (startvelz < sinkspeed)
					Velocity.z = startvelz;
				else
					Velocity.z = sinkspeed;
			}
			else
			{
				Velocity.z = startvelz + (Velocity.z - startvelz) *
					WATER_SINK_FACTOR;
			}
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

//=============================================================================
//
// TestMobjZ
//
//  Checks if the new Z position is legal
//
//=============================================================================

VEntity* VEntity::TestMobjZ(const TVec& TryOrg)
{
	guard(VEntity::TestMobjZ);
	int xl, xh, yl, yh, bx, by;

	// Can't hit thing
	if (!(EntityFlags & EF_ColideWithThings))
	{
		return NULL;
	}

	//
	// the bounding box is extended by MAXRADIUS because mobj_ts are grouped
	// into mapblocks based on their origin point, and can overlap into adjacent
	// blocks by up to MAXRADIUS units
	//
	xl = MapBlock(TryOrg.x - Radius - XLevel->BlockMapOrgX - MAXRADIUS);
	xh = MapBlock(TryOrg.x + Radius - XLevel->BlockMapOrgX + MAXRADIUS);
	yl = MapBlock(TryOrg.y - Radius - XLevel->BlockMapOrgY - MAXRADIUS);
	yh = MapBlock(TryOrg.y + Radius - XLevel->BlockMapOrgY + MAXRADIUS);

	// xl->xh, yl->yh determine the mapblock set to search
	for (bx = xl; bx <= xh; bx++)
	{
		for (by = yl; by <= yh; by++)
		{
			VEntity* Other;
			for (VBlockThingsIterator It(this, bx, by, &Other); It.GetNext(); )
			{
				if (!(Other->EntityFlags & EF_Solid))
				{
					// Can't hit thing
					continue;
				}
				if (Other == this)
				{
					// Don't clip against self
					continue;
				}
				if (TryOrg.z > Other->Origin.z + Other->Height)
				{
					continue;
				}
				if (TryOrg.z + Height < Other->Origin.z)
				{
					// under thing
					continue;
				}
				float blockdist = Other->Radius + Radius;
				if (fabs(Other->Origin.x - TryOrg.x) >= blockdist ||
					fabs(Other->Origin.y - TryOrg.y) >= blockdist)
				{
					// Didn't hit thing
					continue;
				}
				return Other;
			}
		}
	}

	return NULL;
	unguard;
}

//=============================================================================
//
//  VEntity::FakeZMovement
//
//  Fake the zmovement so that we can check if a move is legal
//
//=============================================================================

TVec VEntity::FakeZMovement()
{
	guard(VEntity::FakeZMovement);
	TVec Ret;
	eventCalcFakeZMovement(Ret, host_frametime);

	//
	//  clip movement
	//
	if (Ret.z <= FloorZ)
	{
		// Hit the floor
		Ret.z = FloorZ;
	}
	if (Ret.z + Height > CeilingZ)
	{
		// hit the ceiling
		Ret.z = CeilingZ - Height;
	}
	return Ret;
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
	return TestMobjZ(FakeZMovement());
	unguard;
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

	// Here is the bounding box of the trajectory
	float tmbbox[4];
	tmbbox[BOXLEFT] = MIN(Origin.x, lsPos.x);
	tmbbox[BOXRIGHT] = MAX(Origin.x, lsPos.x);
	tmbbox[BOXTOP] = MAX(Origin.y, lsPos.y);
	tmbbox[BOXBOTTOM] = MIN(Origin.y, lsPos.y);

	// Determine which blocks to look in for blocking lines
	xl = MapBlock(tmbbox[BOXLEFT] - XLevel->BlockMapOrgX);
	xh = MapBlock(tmbbox[BOXRIGHT] - XLevel->BlockMapOrgX);
	yl = MapBlock(tmbbox[BOXBOTTOM] - XLevel->BlockMapOrgY);
	yh = MapBlock(tmbbox[BOXTOP] - XLevel->BlockMapOrgY);

	// xl->xh, yl->yh determine the mapblock set to search
	validcount++; // prevents checking same line twice
	for (bx = xl; bx <= xh; bx++)
	{
		for (by = yl; by <= yh; by++)
		{
			line_t* ld;
			for (VBlockLinesIterator It(this, bx, by, &ld); It.GetNext(); )
			{
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

				if (ld->flags & (ML_BLOCKING | ML_BLOCKMONSTERS |
					ML_BLOCKEVERYTHING))
				{
					if (tmbbox[BOXLEFT] <= ld->bbox[BOXRIGHT] &&
						tmbbox[BOXRIGHT] >= ld->bbox[BOXLEFT] &&
						tmbbox[BOXTOP] >= ld->bbox[BOXBOTTOM] &&
						tmbbox[BOXBOTTOM] <= ld->bbox[BOXTOP])
					{
						if (ld->PointOnSide(Origin) != ld->PointOnSide(lsPos))
						{
								return true;  // line blocks trajectory
						}
					}
				}

				// line doesn't block trajectory
			}
		}
	}

	return false;
	unguard;
}

//=============================================================================
//
//	CheckDropOff
//
//	killough 11/98:
//
//	Monsters try to move away from tall dropoffs.
//
//	In Doom, they were never allowed to hang over dropoffs, and would remain
// stuck if involuntarily forced over one. This logic, combined with P_TryMove,
// allows monsters to free themselves without making them tend to hang over
// dropoffs.
//
//=============================================================================

void VEntity::CheckDropOff(float& DeltaX, float& DeltaY)
{
	guard(VEntity::CheckDropOff);
	float t_bbox[4];
	int xl;
	int xh;
	int yl;
	int yh;
	int bx;
	int by;

	// Try to move away from a dropoff
	DeltaX = 0;
	DeltaY = 0;

	t_bbox[BOXTOP]   = Origin.y + Radius;
	t_bbox[BOXBOTTOM]= Origin.y - Radius;
	t_bbox[BOXRIGHT] = Origin.x + Radius;
	t_bbox[BOXLEFT]  = Origin.x - Radius;

	xl = MapBlock(t_bbox[BOXLEFT] - XLevel->BlockMapOrgX);
	xh = MapBlock(t_bbox[BOXRIGHT] - XLevel->BlockMapOrgX);
	yl = MapBlock(t_bbox[BOXBOTTOM] - XLevel->BlockMapOrgY);
	yh = MapBlock(t_bbox[BOXTOP] - XLevel->BlockMapOrgY);

	// check lines
	validcount++;
	for (bx = xl; bx <= xh; bx++)
	{
		for (by = yl; by <= yh; by++)
		{
			line_t* line;
			for (VBlockLinesIterator It(this, bx, by, &line); It.GetNext(); )
			{
				// Ignore one-sided linedefs
				if (!line->backsector)
				{
					continue;
				}
				// Linedef must be contacted
				if (t_bbox[BOXRIGHT] > line->bbox[BOXLEFT] &&
					t_bbox[BOXLEFT] < line->bbox[BOXRIGHT] &&
					t_bbox[BOXTOP] > line->bbox[BOXBOTTOM] && 
					t_bbox[BOXBOTTOM] < line->bbox[BOXTOP] &&
					P_BoxOnLineSide(t_bbox, line) == -1)
				{
					// New logic for 3D Floors
					sec_region_t*FrontReg = SV_FindThingGap(
						line->frontsector->botregion,
						Origin, Origin.z, Origin.z + Height);
					sec_region_t*BackReg = SV_FindThingGap(
						line->backsector->botregion,
						Origin, Origin.z, Origin.z + Height);
					float front = FrontReg->floor->GetPointZ(Origin);
					float back = BackReg->floor->GetPointZ(Origin);

					// The monster must contact one of the two floors,
					// and the other must be a tall dropoff.
					TVec Dir;
					if (back == Origin.z &&
						front < Origin.z - MaxDropoffHeight)
					{
						// front side dropoff
						Dir = -line->normal;
					}
					else if (front == Origin.z &&
						back < Origin.z - MaxDropoffHeight)
					{
						// back side dropoff
						Dir = line->normal;
					}
					else
					{
						continue;
					}
					// Move away from dropoff at a standard speed.
					// Multiple contacted linedefs are cumulative
					// (e.g. hanging over corner)
					DeltaX += Dir.x * 32.0;
					DeltaY += Dir.y * 32.0;
				}
			}
		}
	}
	unguard;
}

//==========================================================================
//
//	VEntity::CanSee
//
//	LineOfSight/Visibility checks, uses REJECT Lookup Table. This uses
// specialised forms of the maputils routines for optimized performance
//	Returns true if a straight line between t1 and t2 is unobstructed.
//
//==========================================================================

bool VEntity::CanSee(VEntity* Other)
{
	guard(P_CheckSight);
	int			s1;
	int			s2;
	int			pnum;
	linetrace_t	Trace;

	if (!Other)
	{
		return false;
	}
	if ((GetFlags() & _OF_Destroyed) || (Other->GetFlags() & _OF_Destroyed))
	{
		return false;
	}

	//	Determine subsector entries in GL_PVS table.
	//	First check for trivial rejection.
	byte *vis = XLevel->LeafPVS(SubSector);
	s2 = Other->SubSector - XLevel->Subsectors;
	if (!(vis[s2 >> 3] & (1 << (s2 & 7))))
	{
		// can't possibly be connected
		return false;
	}

	if (XLevel->RejectMatrix)
	{
		//	Determine subsector entries in REJECT table.
		//	We must do this because REJECT can have some special effects like
		// "safe sectors"
		s1 = Sector - XLevel->Sectors;
		s2 = Other->Sector - XLevel->Sectors;
		pnum = s1 * XLevel->NumSectors + s2;
		// Check in REJECT table.
		if (XLevel->RejectMatrix[pnum >> 3] & (1 << (pnum & 7)))
		{
			// can't possibly be connected
			return false;
		}
	}

	// An unobstructed LOS is possible.
	// Now look from eyes of t1 to any part of t2.
	Trace.Start = Origin;
	Trace.Start.z += Height * 0.75;
	Trace.End = Other->Origin;
	Trace.End.z += Other->Height * 0.5;

	//	Check middle
	if (XLevel->TraceLine(Trace, Trace.Start, Trace.End, SPF_NOBLOCKSIGHT))
	{
		return true;
	}
	if (Trace.SightEarlyOut)
	{
		return false;
	}

	//	Check head
	Trace.End = Other->Origin;
	Trace.End.z += Other->Height;
	if (XLevel->TraceLine(Trace, Trace.Start, Trace.End, SPF_NOBLOCKSIGHT))
	{
		return true;
	}

	//	Check feats
	Trace.End = Other->Origin;
	Trace.End.z -= Other->FloorClip;
	return XLevel->TraceLine(Trace, Trace.Start, Trace.End, SPF_NOBLOCKSIGHT);
	unguard;
}

//==========================================================================
//
//	VRoughBlockSearchIterator
//
//==========================================================================

VRoughBlockSearchIterator::VRoughBlockSearchIterator(VEntity* ASelf,
	int ADistance, VEntity** AEntPtr)
: Self(ASelf)
, Distance(ADistance)
, Ent(NULL)
, EntPtr(AEntPtr)
, Count(1)
, CurrentEdge(-1)
{
	guard(VRoughBlockSearchIterator::VRoughBlockSearchIterator);
	StartX = MapBlock(Self->Origin.x - Self->XLevel->BlockMapOrgX);
	StartY = MapBlock(Self->Origin.y - Self->XLevel->BlockMapOrgY);

	//	Start with current block
	if (StartX >= 0 && StartX < Self->XLevel->BlockMapWidth &&
		StartY >= 0 && StartY < Self->XLevel->BlockMapHeight)
	{
		Ent = Self->XLevel->BlockLinks[StartY * Self->XLevel->BlockMapWidth +
			StartX];
	}
	unguard;
}

//==========================================================================
//
//	VRoughBlockSearchIterator::GetNext
//
//==========================================================================

bool VRoughBlockSearchIterator::GetNext()
{
	guard(VRoughBlockSearchIterator::GetNext);
	int		BlockX;
	int		BlockY;

	while (1)
	{
		if (Ent)
		{
			*EntPtr = Ent;
			Ent = Ent->BlockMapNext;
			return true;
		}

		switch (CurrentEdge)
		{
		case 0:
			// Trace the first block section (along the top)
			if (BlockIndex <= FirstStop)
			{
				Ent = Self->XLevel->BlockLinks[BlockIndex];
				BlockIndex++;
			}
			else
			{
				CurrentEdge = 1;
				BlockIndex--;
			}
			break;

		case 1:
			// Trace the second block section (right edge)
			if (BlockIndex <= SecondStop)
			{
				Ent = Self->XLevel->BlockLinks[BlockIndex];
				BlockIndex += Self->XLevel->BlockMapWidth;
			}
			else
			{
				CurrentEdge = 2;
				BlockIndex -= Self->XLevel->BlockMapWidth;
			}
			break;

		case 2:
			// Trace the third block section (bottom edge)
			if (BlockIndex >= ThirdStop)
			{
				Ent = Self->XLevel->BlockLinks[BlockIndex];
				BlockIndex--;
			}
			else
			{
				CurrentEdge = 3;
				BlockIndex++;
			}
			break;

		case 3:
			// Trace the final block section (left edge)
			if (BlockIndex > FinalStop)
			{
				Ent = Self->XLevel->BlockLinks[BlockIndex];
				BlockIndex -= Self->XLevel->BlockMapWidth;
			}
			else
			{
				CurrentEdge = -1;
			}
			break;

		default:
			if (Count > Distance)
			{
				//	We are done
				return false;
			}
			BlockX = StartX - Count;
			BlockY = StartY - Count;

			if (BlockY < 0)
			{
				BlockY = 0;
			}
			else if (BlockY >= Self->XLevel->BlockMapHeight)
			{
				BlockY = Self->XLevel->BlockMapHeight - 1;
			}
			if (BlockX < 0)
			{
				BlockX = 0;
			}
			else if (BlockX >= Self->XLevel->BlockMapWidth)
			{
				BlockX = Self->XLevel->BlockMapWidth - 1;
			}
			BlockIndex = BlockY * Self->XLevel->BlockMapWidth + BlockX;
			FirstStop = StartX + Count;
			if (FirstStop < 0)
			{
				Count++;
				break;
			}
			if (FirstStop >= Self->XLevel->BlockMapWidth)
			{
				FirstStop = Self->XLevel->BlockMapWidth - 1;
			}
			SecondStop = StartY + Count;
			if (SecondStop < 0)
			{
				Count++;
				break;
			}
			if (SecondStop >= Self->XLevel->BlockMapHeight)
			{
				SecondStop = Self->XLevel->BlockMapHeight - 1;
			}
			ThirdStop = SecondStop * Self->XLevel->BlockMapWidth + BlockX;
			SecondStop = SecondStop * Self->XLevel->BlockMapWidth + FirstStop;
			FirstStop += BlockY * Self->XLevel->BlockMapWidth;
			FinalStop = BlockIndex;
			Count++;
			CurrentEdge = 0;
			break;
		}
	}
	return false;
	unguard;
}

//==========================================================================
//
//	Script natives
//
//==========================================================================

IMPLEMENT_FUNCTION(VEntity, CheckWater)
{
	P_GET_SELF;
	RET_INT(Self->CheckWater());
}

IMPLEMENT_FUNCTION(VEntity, CheckDropOff)
{
	P_GET_PTR(float, DeltaX);
	P_GET_PTR(float, DeltaY);
	P_GET_SELF;
	Self->CheckDropOff(*DeltaX, *DeltaY);
}

IMPLEMENT_FUNCTION(VEntity, CheckPosition)
{
	P_GET_VEC(Pos);
	P_GET_SELF;
	RET_BOOL(Self->CheckPosition(Pos));
}

IMPLEMENT_FUNCTION(VEntity, CheckRelPosition)
{
	P_GET_VEC(Pos);
	P_GET_PTR(tmtrace_t, tmtrace);
	P_GET_SELF;
	RET_BOOL(Self->CheckRelPosition(*tmtrace, Pos));
}

IMPLEMENT_FUNCTION(VEntity, CheckSides)
{
	P_GET_VEC(lsPos);
	P_GET_SELF;
	RET_BOOL(Self->CheckSides(lsPos));
}

IMPLEMENT_FUNCTION(VEntity, TryMove)
{
	P_GET_BOOL(AllowDropOff);
	P_GET_VEC(Pos);
	P_GET_SELF;
	tmtrace_t tmtrace;
	RET_BOOL(Self->TryMove(tmtrace, Pos, AllowDropOff));
}

IMPLEMENT_FUNCTION(VEntity, TryMoveEx)
{
	P_GET_BOOL(AllowDropOff);
	P_GET_VEC(Pos);
	P_GET_PTR(tmtrace_t, tmtrace);
	P_GET_SELF;
	RET_BOOL(Self->TryMove(*tmtrace, Pos, AllowDropOff));
}

IMPLEMENT_FUNCTION(VEntity, TestMobjZ)
{
	P_GET_SELF;
	RET_BOOL(!Self->TestMobjZ(Self->Origin));
}

IMPLEMENT_FUNCTION(VEntity, SlideMove)
{
	P_GET_FLOAT(StepVelScale);
	P_GET_SELF;
	Self->SlideMove(StepVelScale);
}

IMPLEMENT_FUNCTION(VEntity, BounceWall)
{
	P_GET_FLOAT(overbounce);
	P_GET_SELF;
	Self->BounceWall(overbounce);
}

IMPLEMENT_FUNCTION(VEntity, UpdateVelocity)
{
	P_GET_SELF;
	Self->UpdateVelocity();
}

IMPLEMENT_FUNCTION(VEntity, CheckOnmobj)
{
	P_GET_SELF;
	RET_REF(Self->CheckOnmobj());
}

IMPLEMENT_FUNCTION(VEntity, LinkToWorld)
{
	P_GET_SELF;
	Self->LinkToWorld();
}

IMPLEMENT_FUNCTION(VEntity, UnlinkFromWorld)
{
	P_GET_SELF;
	Self->UnlinkFromWorld();
}

IMPLEMENT_FUNCTION(VEntity, CanSee)
{
	P_GET_REF(VEntity, Other);
	P_GET_SELF;
	RET_BOOL(Self->CanSee(Other));
}

IMPLEMENT_FUNCTION(VEntity, RoughBlockSearch)
{
	P_GET_INT(Distance);
	P_GET_PTR(VEntity*, EntPtr);
	P_GET_SELF;
	RET_PTR(new VRoughBlockSearchIterator(Self, Distance, EntPtr));
}
