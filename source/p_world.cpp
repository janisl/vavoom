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
//**	Movement/collision utility functions, as used by function in
//**  p_map.c. BLOCKMAP Iterator functions, and some PIT_* functions to use
//**  for iteration.
//**
//**************************************************************************

// HEADER FILES ------------------------------------------------------------

#include "gamedefs.h"

// MACROS ------------------------------------------------------------------

#define FRACBITS		16
#define FRACUNIT		(1<<FRACBITS)

typedef int fixed_t;

#define FL(x)	((float)(x) / (float)FRACUNIT)
#define FX(x)	(fixed_t)((x) * FRACUNIT)

// mapblocks are used to check movement against lines and things
#define MAPBLOCKUNITS	128
#define MAPBLOCKSIZE	(MAPBLOCKUNITS*FRACUNIT)
#define MAPBLOCKSHIFT	(FRACBITS+7)
#define MAPBTOFRAC		(MAPBLOCKSHIFT-FRACBITS)

// TYPES -------------------------------------------------------------------

// EXTERNAL FUNCTION PROTOTYPES --------------------------------------------

// PUBLIC FUNCTION PROTOTYPES ----------------------------------------------

// PRIVATE FUNCTION PROTOTYPES ---------------------------------------------

// EXTERNAL DATA DECLARATIONS ----------------------------------------------

// PUBLIC DATA DEFINITIONS -------------------------------------------------

// PRIVATE DATA DEFINITIONS ------------------------------------------------

// CODE --------------------------------------------------------------------

//==========================================================================
//
//	VBlockLinesIterator::VBlockLinesIterator
//
//==========================================================================

VBlockLinesIterator::VBlockLinesIterator(VThinker* ASelf, int x, int y, line_t** ALinePtr)
: Self(ASelf)
, LinePtr(ALinePtr)
, PolyLink(NULL)
, PolySegIdx(-1)
, List(NULL)
{
	guard(VBlockLinesIterator::VBlockLinesIterator);
	if (x < 0 || x >= Self->XLevel->BlockMapWidth ||
		y < 0 || y >= Self->XLevel->BlockMapHeight)
	{
		//	Off the map
		return;
	}

	int offset = y * Self->XLevel->BlockMapWidth + x;
	PolyLink = Self->XLevel->PolyBlockMap[offset];

	offset = *(Self->XLevel->BlockMap + offset);
	List = Self->XLevel->BlockMapLump + offset + 1;
	unguard;
}

//==========================================================================
//
//	VBlockLinesIterator::GetNext
//
//==========================================================================

bool VBlockLinesIterator::GetNext()
{
	guard(VBlockLinesIterator::GetNext);
	if (!List)
	{
		//	Off the map
		return false;
	}

	//	Check polyobj blockmap
	while (PolyLink)
	{
		if (PolySegIdx >= 0)
		{
			while (PolySegIdx < PolyLink->polyobj->numsegs)
			{
				seg_t* Seg = PolyLink->polyobj->segs[PolySegIdx];
				PolySegIdx++;
				if (Seg->linedef->validcount == validcount)
				{
					continue;
				}
				Seg->linedef->validcount = validcount;
				*LinePtr = Seg->linedef;
				return true;
			}
			PolySegIdx = -1;
		}
		if (PolyLink->polyobj)
		{
			if (PolyLink->polyobj->validcount != validcount)
			{
				PolyLink->polyobj->validcount = validcount;
				PolySegIdx = 0;
				continue;
			}
		}
		PolyLink = PolyLink->next;
	}

	while (*List != -1)
	{
#ifdef PARANOID
		if (*List < 0 || *List >= Self->XLevel->NumLines)
			Host_Error("Broken blockmap - line %d", *List);
#endif
		line_t* Line = &Self->XLevel->Lines[*List];
		List++;

		if (Line->validcount == validcount)
		{
			continue; 	// line has already been checked
		}

		Line->validcount = validcount;
		*LinePtr = Line;
		return true;
	}
	return false;
	unguard;
}

//==========================================================================
//
//	VBlockThingsIterator::VBlockThingsIterator
//
//==========================================================================

VBlockThingsIterator::VBlockThingsIterator(VThinker* Self, int x, int y, VEntity** AEntPtr)
: EntPtr(AEntPtr)
{
	guard(VBlockThingsIterator::VBlockThingsIterator);
	if (x < 0 || x >= Self->XLevel->BlockMapWidth ||
		y < 0 || y >= Self->XLevel->BlockMapHeight)
	{
		Ent = NULL;
		return;
	}
	Ent = Self->XLevel->BlockLinks[y * Self->XLevel->BlockMapWidth + x];
	unguard;
}

//==========================================================================
//
//	VBlockThingsIterator::GetNext
//
//==========================================================================

bool VBlockThingsIterator::GetNext()
{
	guard(VBlockThingsIterator::GetNext);
	if (!Ent)
	{
		return false;
	}
	*EntPtr = Ent;
	Ent = Ent->BlockMapNext;
	return true;
	unguard;
}

//==========================================================================
//
//	VRadiusThingsIterator::VRadiusThingsIterator
//
//==========================================================================

VRadiusThingsIterator::VRadiusThingsIterator(VThinker* ASelf,
	VEntity** AEntPtr, TVec Org, float Radius)
: Self(ASelf)
, EntPtr(AEntPtr)
{
	guard(VRadiusThingsIterator::VRadiusThingsIterator);
	xl = MapBlock(Org.x - Radius - Self->XLevel->BlockMapOrgX - MAXRADIUS);
	xh = MapBlock(Org.x + Radius - Self->XLevel->BlockMapOrgX + MAXRADIUS);
	yl = MapBlock(Org.y - Radius - Self->XLevel->BlockMapOrgY - MAXRADIUS);
	yh = MapBlock(Org.y + Radius - Self->XLevel->BlockMapOrgY + MAXRADIUS);
	x = xl;
	y = yl;
	if (x < 0 || x >= Self->XLevel->BlockMapWidth ||
		y < 0 || y >= Self->XLevel->BlockMapHeight)
	{
		Ent = NULL;
	}
	else
	{
		Ent = Self->XLevel->BlockLinks[y * Self->XLevel->BlockMapWidth + x];
	}
	unguard;
}

//==========================================================================
//
//	VRadiusThingsIterator::GetNext
//
//==========================================================================

bool VRadiusThingsIterator::GetNext()
{
	guard(VRadiusThingsIterator::GetNext);
	while (1)
	{
		while (Ent)
		{
			*EntPtr = Ent;
			Ent = Ent->BlockMapNext;
			return true;
		}

		y++;
		if (y > yh)
		{
			x++;
			y = yl;
			if (x > xh)
			{
				return false;
			}
		}

		if (x < 0 || x >= Self->XLevel->BlockMapWidth ||
			y < 0 || y >= Self->XLevel->BlockMapHeight)
		{
			Ent = NULL;
		}
		else
		{
			Ent = Self->XLevel->BlockLinks[y * Self->XLevel->BlockMapWidth + x];
		}
	}
	unguard;
}

//==========================================================================
//
//	VPathTraverse::VPathTraverse
//
//==========================================================================

VPathTraverse::VPathTraverse(VThinker* Self, intercept_t** AInPtr, float InX1,
	float InY1, float x2, float y2, int flags)
: Count(0)
, In(NULL)
, InPtr(AInPtr)
{
	Init(Self, InX1, InY1, x2, y2, flags);
}

//==========================================================================
//
//	VPathTraverse::Init
//
//==========================================================================

void VPathTraverse::Init(VThinker* Self, float InX1, float InY1, float x2,
	float y2, int flags)
{
	guard(VPathTraverse::Init);
	float x1 = InX1;
	float y1 = InY1;
	int			xt1;
	int			yt1;
	int			xt2;
	int			yt2;

	float		xstep;
	float		ystep;

	float		partialx;
	float		partialy;

	float		xintercept;
	float		yintercept;

	int			mapx;
	int			mapy;

	int			mapxstep;
	int			mapystep;

	validcount++;

	if (((FX(x1 - Self->XLevel->BlockMapOrgX)) & (MAPBLOCKSIZE - 1)) == 0)
//	if (fmod(x1 - BlockMapOrgX, MAPBLOCKSIZE) == 0.0)
		x1 += 1.0;	// don't side exactly on a line

	if (((FX(y1 - Self->XLevel->BlockMapOrgY)) & (MAPBLOCKSIZE - 1)) == 0)
//	if (fmod(y1 - BlockMapOrgY, MAPBLOCKSIZE) == 0.0)
		y1 += 1.0;	// don't side exactly on a line

	trace_org = TVec(x1, y1, 0);
	trace_dest = TVec(x2, y2, 0);
	trace_delta = trace_dest - trace_org;
	trace_dir = Normalise(trace_delta);
	trace_len = Length(trace_delta);

	trace_plane.SetPointDir(trace_org, trace_delta);

	x1 -= Self->XLevel->BlockMapOrgX;
	y1 -= Self->XLevel->BlockMapOrgY;
	xt1 = MapBlock(x1);
	yt1 = MapBlock(y1);

	x2 -= Self->XLevel->BlockMapOrgX;
	y2 -= Self->XLevel->BlockMapOrgY;
	xt2 = MapBlock(x2);
	yt2 = MapBlock(y2);

	if (xt2 > xt1)
	{
		mapxstep = 1;
		partialx = 1.0 - FL((FX(x1) >> MAPBTOFRAC) & (FRACUNIT - 1));
//		partialx = 1.0 - (x1 / 120.0 - xt1);
		ystep = (y2 - y1) / fabs(x2 - x1);
	}
	else if (xt2 < xt1)
	{
		mapxstep = -1;
		partialx = FL((FX(x1) >> MAPBTOFRAC) & (FRACUNIT - 1));
//		partialx = x1 / MAPBLOCKSIZE - xt1;
		ystep = (y2 - y1) / fabs(x2 - x1);
	}
	else
	{
		mapxstep = 0;
		partialx = 1.0;
		ystep = 256.0;
	}
	yintercept = FL(FX(y1) >> MAPBTOFRAC) + partialx * ystep;
//	yintercept = y1 / MAPBLOCKSIZE + partial * ystep;

	if (yt2 > yt1)
	{
		mapystep = 1;
		partialy = 1.0 - FL((FX(y1) >> MAPBTOFRAC) & (FRACUNIT - 1));
//		partialy = 1.0 - (y1 / MAPBLOCKSIZE - yt1);
		xstep = (x2 - x1) / fabs(y2 - y1);
	}
	else if (yt2 < yt1)
	{
		mapystep = -1;
		partialy = FL((FX(y1) >> MAPBTOFRAC) & (FRACUNIT - 1));
//		partialy = y1 / MAPBLOCKSIZE - yt1;
		xstep = (x2 - x1) / fabs(y2 - y1);
	}
	else
	{
		mapystep = 0;
		partialy = 1.0;
		xstep = 256.0;
	}
	xintercept = FL(FX(x1) >> MAPBTOFRAC) + partialy * xstep;
//	xintercept = x1 / MAPBLOCKSIZE + partialy * xstep;

	// [RH] Fix for traces that pass only through blockmap corners. In that case,
	// xintercept and yintercept can both be set ahead of mapx and mapy, so the
	// for loop would never advance anywhere.
	if (fabs(xstep) == 1.0 && fabs(ystep) == 1.0)
	{
		if (ystep < 0.0)
		{
			partialx = 1.0 - partialx;
		}
		if (xstep < 0.0)
		{
			partialy = 1.0 - partialy;
		}
		if (partialx == partialy)
		{
			xintercept = xt1;
			yintercept = yt1;
		}
	}

	// Step through map blocks.
	// Count is present to prevent a round off error
	// from skipping the break.
	mapx = xt1;
	mapy = yt1;

	for (int count = 0 ; count < 100 ; count++)
	{
		if (flags & PT_ADDLINES)
		{
			if (!AddLineIntercepts(Self, mapx, mapy, !!(flags & PT_EARLYOUT)))
			{
				return;	// early out
			}
		}

		if (flags & PT_ADDTHINGS)
		{
			AddThingIntercepts(Self, mapx, mapy);
		}

		if (mapx == xt2 && mapy == yt2)
		{
			break;
		}

		// [RH] Handle corner cases properly instead of pretending they don't exist.
		if ((int)yintercept == mapy)
		{
			yintercept += ystep;
			mapx += mapxstep;
		}
		else if ((int)xintercept == mapx)
		{
			xintercept += xstep;
			mapy += mapystep;
		}
		else if (((int)yintercept == mapy) && ((int)xintercept == mapx))
		{
			// The trace is exiting a block through its corner. Not only does the block
			// being entered need to be checked (which will happen when this loop
			// continues), but the other two blocks adjacent to the corner also need to
			// be checked.
			if (flags & PT_ADDLINES)
			{
				if (!AddLineIntercepts(Self, mapx + mapxstep, mapy, !!(flags & PT_EARLYOUT)) ||
					!AddLineIntercepts(Self, mapx, mapy + mapystep, !!(flags & PT_EARLYOUT)))
					return;	// early out
			}
			
			if (flags & PT_ADDTHINGS)
			{
				AddThingIntercepts(Self, mapx + mapxstep, mapy);
				AddThingIntercepts(Self, mapx, mapy + mapystep);
			}

			xintercept += xstep;
			yintercept += ystep;
			mapx += mapxstep;
			mapy += mapystep;
		}
		else
		{
			count = 100;	// Stop traversing, because somebody screwed up.
		}
	}

	Count = Intercepts.Num();
	unguard;
}

//==========================================================================
//
//	VPathTraverse::AddLineIntercepts
//
//	Looks for lines in the given block that intercept the given trace to add
// to the intercepts list.
//	A line is crossed if its endpoints are on opposite sides of the trace.
// Returns true if earlyout and a solid line hit.
//
//==========================================================================

bool VPathTraverse::AddLineIntercepts(VThinker* Self, int mapx, int mapy,
	bool EarlyOut)
{
	guard(VPathTraverse::AddLineIntercepts);
	line_t*		ld;

	for (VBlockLinesIterator It(Self, mapx, mapy, &ld); It.GetNext(); )
	{
		float dot1 = DotProduct(*ld->v1, trace_plane.normal) - trace_plane.dist;
		float dot2 = DotProduct(*ld->v2, trace_plane.normal) - trace_plane.dist;

		if (dot1 * dot2 >= 0)
		{
			continue;	// line isn't crossed
		}

		// hit the line
		//
		//	Find the fractional intercept point along the trace line.
		//
		float		num;
		float		den;
		float		frac;

		den = DotProduct(ld->normal, trace_delta);
		if (den == 0)
		{
			continue;
		}
		num = ld->dist - DotProduct(trace_org, ld->normal);
		frac = num / den;

		if (frac < 0)
		{
			continue;	// behind source
		}

		// try to early out the check
		if (EarlyOut && frac < 1.0 && !ld->backsector)
		{
			return false;	// stop checking
		}

		intercept_t& In = Intercepts.Alloc();
		In.frac = frac;
		In.Flags = intercept_t::IF_IsALine;
		In.line = ld;
		In.thing = NULL;
	}
	return true;
	unguard;
}

//==========================================================================
//
//	VPathTraverse::AddThingIntercepts
//
//==========================================================================

void VPathTraverse::AddThingIntercepts(VThinker* Self, int mapx, int mapy)
{
	guard(VPathTraverse::AddThingIntercepts);
	VEntity* thing;
	for (VBlockThingsIterator It(Self, mapx, mapy, &thing); It.GetNext();)
	{
		float dot = DotProduct(thing->Origin, trace_plane.normal) - trace_plane.dist;
		if (dot >= thing->Radius || dot <= -thing->Radius)
		{
			continue;		// line isn't crossed
		}

		float dist = DotProduct((thing->Origin - trace_org), trace_dir);
//		dist -= sqrt(thing->radius * thing->radius - dot * dot);
		if (dist < 0)
		{
			continue;		// behind source
		}
		float frac = dist / trace_len;

		intercept_t& In = Intercepts.Alloc();
		In.frac = frac;
		In.Flags = 0;
		In.line = NULL;
		In.thing = thing;
	}
	unguard;
}

//==========================================================================
//
//	VPathTraverse::GetNext
//
//==========================================================================

bool VPathTraverse::GetNext()
{
	guard(VPathTraverse::GetNext);
	if (!Count)
	{
		//	Everything was traversed.
		return false;
	}
	Count--;

	if (In)
	{
		//	Mark previous intercept as checked.
		In->frac = 99999.0;
	}

	//	Go through the sorted list.
	float Dist = 99999.0;
	intercept_t* EndIn = Intercepts.Ptr() + Intercepts.Num();
	for (intercept_t* Scan = Intercepts.Ptr(); Scan < EndIn; Scan++)
	{
		if (Scan->frac < Dist)
		{
			Dist = Scan->frac;
			In = Scan;
		}
	}

	if (Dist > 1.0)
	{
		//	Checked everything in range.
		return false;
	}

	*InPtr = In;
	return true;
	unguard;
}
