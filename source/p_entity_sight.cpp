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
//**	LineOfSight/Visibility checks, uses REJECT Lookup Table.
//**
//**	This uses specialized forms of the maputils routines for optimized
//**  performance
//**
//**************************************************************************

// HEADER FILES ------------------------------------------------------------

#include "gamedefs.h"
#include "sv_local.h"

// MACROS ------------------------------------------------------------------

//#define USE_BSP

#ifndef USE_BSP

#define FRACBITS		16
#define FRACUNIT		(1<<FRACBITS)

#define FL(x)	((float)(x) / (float)FRACUNIT)
#define FX(x)	(fixed_t)((x) * FRACUNIT)

// mapblocks are used to check movement against lines and things
#define MAPBLOCKUNITS	128
#define MAPBLOCKSIZE	(MAPBLOCKUNITS*FRACUNIT)
#define MAPBLOCKSHIFT	(FRACBITS+7)
#define MAPBTOFRAC		(MAPBLOCKSHIFT-FRACBITS)

// TYPES -------------------------------------------------------------------

typedef int fixed_t;

struct sight_trace_t
{
	TVec				Start;
	TVec				End;
	TVec				Delta;
	TPlane				Plane;
	bool				EarlyOut;
	TVec				LineStart;
	TVec				LineEnd;
	TArray<intercept_t>	Intercepts;
};

// EXTERNAL FUNCTION PROTOTYPES --------------------------------------------

// PUBLIC FUNCTION PROTOTYPES ----------------------------------------------

// PRIVATE FUNCTION PROTOTYPES ---------------------------------------------

// EXTERNAL DATA DECLARATIONS ----------------------------------------------

// PUBLIC DATA DEFINITIONS -------------------------------------------------

// PRIVATE DATA DEFINITIONS ------------------------------------------------

// CODE --------------------------------------------------------------------

//==========================================================================
//
//	SightCheckPlane
//
//==========================================================================

static bool SightCheckPlane(sight_trace_t& Trace, const sec_plane_t* Plane)
{
	guard(SightCheckPlane);
	if (Plane->flags & SPF_NOBLOCKSIGHT)
	{
		//	Plane doesn't block
		return true;
	}

	float OrgDist = DotProduct(Trace.LineStart, Plane->normal) - Plane->dist;
	if (OrgDist < -0.1)
	{
		//	Ignore back side
		return true;
	}

	float HitDist = DotProduct(Trace.LineEnd, Plane->normal) - Plane->dist;
	if (HitDist >= -0.1)
	{
		//	Didn't cross Plane
		return true;
	}

	if (Plane->pic == skyflatnum)
	{
		//	Hit sky, don't clip
		return false;
	}

	//	Crosses Plane
	return false;
	unguard;
}

//==========================================================================
//
//	SightCheckPlanes
//
//==========================================================================

static bool SightCheckPlanes(sight_trace_t& Trace, sector_t* Sec)
{
	guard(SightCheckPlanes);
	if (Sec->topregion == Sec->botregion)
	{
		//	Don't bother with planes if there's no 3D floors.
		return true;
	}

	sec_region_t* StartReg = SV_PointInRegion(Sec, Trace.LineStart);

	if (StartReg != NULL)
	{
		for (sec_region_t* Reg = StartReg; Reg; Reg = Reg->next)
		{
			if (!SightCheckPlane(Trace, Reg->floor))
			{
				//	Hit floor
				return false;
			}
			if (!SightCheckPlane(Trace, Reg->ceiling))
			{
				//	Hit ceiling
				return false;
			}
		}

		for (sec_region_t* Reg = StartReg->prev; Reg != NULL; Reg = Reg->prev)
		{
			if (!SightCheckPlane(Trace, Reg->floor))
			{
				//	Hit floor
				return false;
			}
			if (!SightCheckPlane(Trace, Reg->ceiling))
			{
				//	Hit ceiling
				return false;
			}
		}
	}

	return true;
	unguard;
}

//==========================================================================
//
//	SightTraverse
//
//==========================================================================

static bool SightTraverse(sight_trace_t& Trace, intercept_t* in)
{
	guard(SightTraverse);
	line_t* li = in->line;

	int s1 = li->PointOnSide2(Trace.Start);
	sector_t* front;
	if (s1 == 0 || s1 == 2)
	{
		front = li->frontsector;
	}
	else
	{
		front = li->backsector;
	}

	TVec hit_point = Trace.Start + in->frac * Trace.Delta;
	Trace.LineEnd = hit_point;
	if (!SightCheckPlanes(Trace, front))
	{
		return false;
	}
	Trace.LineStart = Trace.LineEnd;

	//
	// crosses a two sided line
	//
	opening_t* open = SV_LineOpenings(li, hit_point, SPF_NOBLOCKSIGHT);
	while (open)
	{
		if (open->bottom <= hit_point.z && open->top >= hit_point.z)
		{
			return true;
		}
		open = open->next;
	}

	return false;	// stop
	unguard;
}

//==========================================================================
//
//	SightTraverseIntercepts
//
//	Returns true if the traverser function returns true for all lines
//
//==========================================================================

static bool SightTraverseIntercepts(sight_trace_t& Trace, VThinker* Self,
	sector_t* EndSector)
{
	guard(SightTraverseIntercepts);
	int				count;
	float			dist;
	intercept_t*	scan;
	intercept_t*	in;

	count = Trace.Intercepts.Num();

	//
	// calculate intercept distance
	//
	scan = Trace.Intercepts.Ptr();
	for (int i = 0; i < Trace.Intercepts.Num(); i++, scan++)
	{
		float den = DotProduct(scan->line->normal, Trace.Delta);
		float num = scan->line->dist -
			DotProduct(Trace.Start, scan->line->normal);
		scan->frac = num / den;
	}

	//
	// go through in order
	//
	in = NULL;			// shut up compiler warning

	while (count--)
	{
		dist = 99999.0;
		scan = Trace.Intercepts.Ptr();
		for (int i = 0; i < Trace.Intercepts.Num(); i++, scan++)
		{
			if (scan->frac < dist)
			{
				dist = scan->frac;
				in = scan;
			}
		}
		if (!SightTraverse(Trace, in))
		{
			return false;			// don't bother going farther
		}
		in->frac = 99999.0;
	}

	Trace.LineEnd = Trace.End;
	return SightCheckPlanes(Trace, EndSector);
	unguard;
}

//==========================================================================
//
//	SightCheckLine
//
//==========================================================================

static bool SightCheckLine(sight_trace_t& Trace, line_t *ld)
{
	if (ld->validcount == validcount)
	{
		return true;
	}

	ld->validcount = validcount;

	float dot1 = DotProduct(*ld->v1, Trace.Plane.normal) - Trace.Plane.dist;
	float dot2 = DotProduct(*ld->v2, Trace.Plane.normal) - Trace.Plane.dist;

	if (dot1 * dot2 >= 0)
	{
		return true;		// line isn't crossed
	}

	dot1 = DotProduct(Trace.Start, ld->normal) - ld->dist;
	dot2 = DotProduct(Trace.End, ld->normal) - ld->dist;

	if (dot1 * dot2 >= 0)
	{
		return true;		// line isn't crossed
	}

	// try to early out the check
	if (!ld->backsector || !(ld->flags & ML_TWOSIDED) || (ld->flags & ML_BLOCKEVERYTHING))
	{
		return false;	// stop checking
	}

	// store the line for later intersection testing
	intercept_t& In = Trace.Intercepts.Alloc();
	In.line = ld;

	return true;
}

//==========================================================================
//
//	SightBlockLinesIterator
//
//==========================================================================

static bool SightBlockLinesIterator(sight_trace_t& Trace, VThinker* Self,
	int x, int y)
{
	guard(SightBlockLinesIterator);
	int				offset;
	vint32*			list;
	polyblock_t*	polyLink;
	seg_t**			segList;
	int 			i;

	offset = y * Self->XLevel->BlockMapWidth + x;

	polyLink = Self->XLevel->PolyBlockMap[offset];
	while (polyLink)
	{
		if (polyLink->polyobj)
		{
			// only check non-empty links
			if (polyLink->polyobj->validcount != validcount)
			{
				segList = polyLink->polyobj->segs;
				for (i = 0; i < polyLink->polyobj->numsegs; i++, segList++)
				{
					if (!SightCheckLine(Trace, (*segList)->linedef))
						return false;
				}
				polyLink->polyobj->validcount = validcount;
			}
		}
		polyLink = polyLink->next;
	}

	offset = *(Self->XLevel->BlockMap + offset);

	for (list = Self->XLevel->BlockMapLump + offset + 1; *list != -1; list++)
	{
		if (!SightCheckLine(Trace, &Self->XLevel->Lines[*list]))
			return false;
	}

	return true;			// everything was checked
	unguard;
}

//==========================================================================
//
//	SightPathTraverse
//
//	Traces a line from x1,y1 to x2,y2, calling the traverser function for
// each. Returns true if the traverser function returns true for all lines
//
//==========================================================================

static bool SightPathTraverse(sight_trace_t& Trace, VThinker* Self,
	sector_t* EndSector)
{
	guard(SightPathTraverse);
	float		x1 = Trace.Start.x;
	float		y1 = Trace.Start.y;
	float		x2 = Trace.End.x;
	float		y2 = Trace.End.y;
	float		xstep, ystep;
	float		partialx, partialy;
	float		xintercept, yintercept;
	int			mapx, mapy, mapxstep, mapystep;
	int			count;

	validcount++;
	Trace.Intercepts.Clear();

	if (((FX(x1 - Self->XLevel->BlockMapOrgX)) & (MAPBLOCKSIZE - 1)) == 0)
	{
		x1 += 1.0;				// don't side exactly on a line
	}
	if (((FX(y1 - Self->XLevel->BlockMapOrgY)) & (MAPBLOCKSIZE - 1)) == 0)
	{
		y1 += 1.0;				// don't side exactly on a line
	}
	Trace.Delta = Trace.End - Trace.Start;
	Trace.Plane.SetPointDir(Trace.Start, Trace.Delta);
	Trace.EarlyOut = false;
	Trace.LineStart = Trace.Start;

	x1 -= Self->XLevel->BlockMapOrgX;
	y1 -= Self->XLevel->BlockMapOrgY;
	int xt1 = MapBlock(x1);
	int yt1 = MapBlock(y1);

	x2 -= Self->XLevel->BlockMapOrgX;
	y2 -= Self->XLevel->BlockMapOrgY;
	int xt2 = MapBlock(x2);
	int yt2 = MapBlock(y2);

	// points should never be out of bounds, but check once instead of
	// each block
	if (xt1 < 0 || yt1 < 0 || xt1 >= Self->XLevel->BlockMapWidth || yt1 >= Self->XLevel->BlockMapHeight ||
		xt2 < 0 || yt2 < 0 || xt2 >= Self->XLevel->BlockMapWidth || yt2 >= Self->XLevel->BlockMapHeight)
	{
		Trace.EarlyOut = true;
		return false;
	}

	if (xt2 > xt1)
	{
		mapxstep = 1;
		partialx = 1.0 - FL((FX(x1) >> MAPBTOFRAC) & (FRACUNIT - 1));
		ystep = (y2 - y1) / fabs(x2 - x1);
	}
	else if (xt2 < xt1)
	{
		mapxstep = -1;
		partialx = FL((FX(x1) >> MAPBTOFRAC) & (FRACUNIT - 1));
		ystep = (y2 - y1) / fabs(x2 - x1);
	}
	else
	{
		mapxstep = 0;
		partialx = 1.0;
		ystep = 256.0;
	}	
	yintercept = FL(FX(y1) >> MAPBTOFRAC) + partialx * ystep;
	
	if (yt2 > yt1)
	{
		mapystep = 1;
		partialy = 1.0 - FL((FX(y1) >> MAPBTOFRAC) & (FRACUNIT - 1));
		xstep = (x2 - x1) / fabs(y2 - y1);
	}
	else if (yt2 < yt1)
	{
		mapystep = -1;
		partialy = FL((FX(y1) >> MAPBTOFRAC) & (FRACUNIT - 1));
		xstep = (x2 - x1) / fabs(y2 - y1);
	}
	else
	{
		mapystep = 0;
		partialy = 1.0;
		xstep = 256.0;
	}	
	xintercept = FL(FX(x1) >> MAPBTOFRAC) + partialy * xstep;

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
	// from skipping the break
	mapx = xt1;
	mapy = yt1;
	
	for (count = 0; count < 64; count++)
	{
		if (!SightBlockLinesIterator(Trace, Self, mapx, mapy))
		{
			Trace.EarlyOut = true;
			return false;	// early out
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
			if (mapx == xt2)
				mapxstep = 0;
		}
		else if ((int)xintercept == mapx)
		{
			xintercept += xstep;
			mapy += mapystep;
			if (mapy == yt2)
				mapystep = 0;
		}
		else if ((int)yintercept == mapy && (int)xintercept == mapx)
		{
			// The trace is exiting a block through its corner. Not only does the block
			// being entered need to be checked (which will happen when this loop
			// continues), but the other two blocks adjacent to the corner also need to
			// be checked.
			if (!SightBlockLinesIterator(Trace, Self, mapx + mapxstep, mapy) ||
				!SightBlockLinesIterator(Trace, Self, mapx, mapy + mapystep))
			{
				return false;
			}
			xintercept += xstep;
			yintercept += ystep;
			mapx += mapxstep;
			mapy += mapystep;
			if (mapx == xt2)
				mapxstep = 0;
			if (mapy == yt2)
				mapystep = 0;
		}
		else
		{
			count = 64;	// Stop traversing, because somebody screwed up.
		}
	}

	//
	// couldn't early out, so go through the sorted list
	//
	return SightTraverseIntercepts(Trace, Self, EndSector);
	unguard;
}

//==========================================================================
//
//	SightPathTraverse2
//
//	Rechecks Trace.Intercepts with different ending z value.
//
//==========================================================================

static bool SightPathTraverse2(sight_trace_t& Trace, VThinker* Self,
	sector_t* EndSector)
{
	guard(SightPathTraverse2);
	Trace.Delta = Trace.End - Trace.Start;
	Trace.LineStart = Trace.Start;

	return SightTraverseIntercepts(Trace, Self, EndSector);
	unguard;
}

#endif

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
	guard(VEntity::CanSee);
	int				s1;
	int				s2;
	int				pnum;
#ifdef USE_BSP
	linetrace_t		Trace;
#else
	sight_trace_t	Trace;
#endif

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

	// killough 4/19/98: make fake floors and ceilings block monster view
	if ((Sector->heightsec &&
		((Origin.z + Height <= Sector->heightsec->floor.GetPointZ(Origin.x, Origin.y) &&
		  Other->Origin.z >= Sector->heightsec->floor.GetPointZ(Other->Origin.x, Other->Origin.y)) ||
		  (Origin.z >= Sector->heightsec->ceiling.GetPointZ (Origin.x, Origin.y) &&
		  Other->Origin.z + Height <= Sector->heightsec->ceiling.GetPointZ (Other->Origin.x, Other->Origin.y))))
		||
		(Other->Sector->heightsec &&
		((Other->Origin.z + Other->Height <= Other->Sector->heightsec->floor.GetPointZ (Other->Origin.x, Other->Origin.y) &&
		Origin.z >= Other->Sector->heightsec->floor.GetPointZ (Origin.x, Origin.y)) ||
		  (Other->Origin.z >= Other->Sector->heightsec->ceiling.GetPointZ (Other->Origin.x, Other->Origin.y) &&
		  Origin.z + Other->Height <= Other->Sector->heightsec->ceiling.GetPointZ (Origin.x, Origin.y)))))
	{
		return false;
	}

	// An unobstructed LOS is possible.
	// Now look from eyes of t1 to any part of t2.
	Trace.Start = Origin;
	Trace.Start.z += Height * 0.75;
	Trace.End = Other->Origin;
	Trace.End.z += Other->Height * 0.5;

	//	Check middle
#ifdef USE_BSP
	if (XLevel->TraceLine(Trace, Trace.Start, Trace.End, SPF_NOBLOCKSIGHT))
#else
	if (SightPathTraverse(Trace, this, Other->SubSector->sector))
#endif
	{
		return true;
	}
#ifdef USE_BSP
	if (Trace.SightEarlyOut)
#else
	if (Trace.EarlyOut)
#endif
	{
		return false;
	}

	//	Check head
	Trace.End = Other->Origin;
	Trace.End.z += Other->Height;
#ifdef USE_BSP
	if (XLevel->TraceLine(Trace, Trace.Start, Trace.End, SPF_NOBLOCKSIGHT))
#else
	if (SightPathTraverse2(Trace, this, Other->SubSector->sector))
#endif
	{
		return true;
	}

	//	Check feats
	Trace.End = Other->Origin;
	Trace.End.z -= Other->FloorClip;
#ifdef USE_BSP
	return XLevel->TraceLine(Trace, Trace.Start, Trace.End, SPF_NOBLOCKSIGHT);
#else
	return SightPathTraverse2(Trace, this, Other->SubSector->sector);
#endif
	unguard;
}
