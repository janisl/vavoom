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
//**	
//**	LineOfSight/Visibility checks, uses REJECT Lookup Table. This uses
//**  specialized forms of the maputils routines for optimized performance
//**
//**************************************************************************

// HEADER FILES ------------------------------------------------------------

#include "gamedefs.h"
#include "sv_local.h"

// MACROS ------------------------------------------------------------------

// non-line sight doesn't work now
#define LINE_SIGHT

// TYPES -------------------------------------------------------------------

// EXTERNAL FUNCTION PROTOTYPES --------------------------------------------

// PUBLIC FUNCTION PROTOTYPES ----------------------------------------------

// PRIVATE FUNCTION PROTOTYPES ---------------------------------------------

// EXTERNAL DATA DECLARATIONS ----------------------------------------------

// PUBLIC DATA DEFINITIONS -------------------------------------------------

// PRIVATE DATA DEFINITIONS ------------------------------------------------

static TVec			sightstart;
static TVec			sightend;
static TVec			sdelta;
static TPlane		strace;			// from t1 to t2

static TVec			linestart;
static TVec			lineend;

#ifndef LINE_SIGHT
static float		topslope;
static float		bottomslope;  // slopes to top and bottom of target
#endif

// CODE --------------------------------------------------------------------

//==========================================================================
//
//	PlaneSide2
//
//	Returns side 0 (front), 1 (back), or 2 (on).
//
//==========================================================================

static int PlaneSide2(const TVec &point, const TPlane* plane)
{
	float dot = DotProduct(point, plane->normal) - plane->dist;
	return dot ? dot < 0 : 2;
}

//==========================================================================
//
//	CheckPlane
//
//==========================================================================

static bool CheckPlane(const sec_plane_t *plane)
{
	float		org_dist;
	float		hit_dist;

	if (plane->flags & SPF_NOBLOCKSIGHT)
	{
		//	Plane doesn't block sight
		return true;
	}
	org_dist = DotProduct(linestart, plane->normal) - plane->dist;
	if (org_dist < 0.0)
	{
		//	Ignore back side
		return true;
	}
	hit_dist = DotProduct(lineend, plane->normal) - plane->dist;
	if (hit_dist >= 0.0)
	{
		//	Didn't cross plane
		return true;
	}

	//	Crosses plane
	return false;
}

//==========================================================================
//
//	CheckPlanes
//
//==========================================================================

static bool CheckPlanes(sector_t *sec)
{
	sec_region_t	*reg;
	sec_region_t	*startreg;

	startreg = SV_PointInRegion(sec, linestart);
	for (reg = startreg; reg; reg = reg->next)
	{
		if (!CheckPlane(reg->floor))
		{
			//	Hit floor
			return false;
		}
		if (!CheckPlane(reg->ceiling))
		{
			//	Hit ceiling
			return false;
		}
	}
	for (reg = startreg->prev; reg; reg = reg->prev)
	{
		if (!CheckPlane(reg->floor))
		{
			//	Hit floor
			return false;
		}
		if (!CheckPlane(reg->ceiling))
		{
			//	Hit ceiling
			return false;
		}
	}
	return true;
}

//==========================================================================
//
//	CheckLine
//
//==========================================================================

static boolean CheckLine(seg_t* seg)
{
	line_t*			line;
	int				s1;
	int				s2;
	sector_t*		front;
	sector_t*		back;
	float			opentop;
	float			openbottom;
	float			frac;
#ifndef LINE_SIGHT
	float			slope;
#endif
	float			num;
	float			den;
	TVec			hit_point;

	line = seg->linedef;
	if (!line)
		return true;

	// allready checked other side?
	if (line->validcount == validcount)
		return true;

	line->validcount = validcount;

	s1 = PlaneSide2(*line->v1, &strace);
	s2 = PlaneSide2(*line->v2, &strace);

	// line isn't crossed?
	if (s1 == s2)
		return true;

	s1 = PlaneSide2(sightstart, line);
	s2 = PlaneSide2(sightend, line);

	// line isn't crossed?
	if (s1 == s2)
		return true;

	// stop because it is not two sided anyway
	if (!(line->flags & ML_TWOSIDED))
		return false;

	// crosses a two sided line
	if (s1 == 0)
	{
		front = line->frontsector;
		back = line->backsector;
	}
	else
	{
		back = line->frontsector;
		front = line->backsector;
	}

	// Intercept vector.
	// Don't need to check if den == 0, because then planes are paralel
	// (they will never cross) or it's the same plane (also rejected)
	den = DotProduct(sdelta, line->normal);
	num = line->dist - DotProduct(sightstart, line->normal);
	frac = num / den;
	hit_point = sightstart + frac * sdelta;

#ifdef LINE_SIGHT
	lineend = hit_point;
	if (!CheckPlanes(front))
	{
		return false;
	}
	linestart = lineend;
	float frontfloorz = front->floor.GetPointZ(hit_point);
	float frontceilz = front->ceiling.GetPointZ(hit_point);
	float backfloorz = back->floor.GetPointZ(hit_point);
	float backceilz = back->ceiling.GetPointZ(hit_point);

	// no wall to block sight with?
	if (frontfloorz == backfloorz && frontceilz == backceilz)
		return true;

	// possible occluder
	// because of ceiling height differences
	if (frontceilz < backceilz)
		opentop = frontceilz;
	else
		opentop = backceilz;

	// because of ceiling height differences
	if (frontfloorz > backfloorz)
		openbottom = frontfloorz;
	else
		openbottom = backfloorz;

	// quick test for totally closed doors
	if (openbottom >= opentop)
		return false;		// stop

	if (hit_point.z >= openbottom && hit_point.z <= opentop)
	{
		return true;
	}
	return false;		// stop
#else
	float frontfloorz = front->floor.GetPointZ(hit_point);
	float frontceilz = front->ceiling.GetPointZ(hit_point);
	float backfloorz = back->floor.GetPointZ(hit_point);
	float backceilz = back->ceiling.GetPointZ(hit_point);

	// no wall to block sight with?
	if (frontfloorz == backfloorz && frontceilz == backceilz)
		return true;

	// possible occluder
	// because of ceiling height differences
	if (frontceilz < backceilz)
		opentop = frontceilz;
	else
		opentop = backceilz;

	// because of ceiling height differences
	if (frontfloorz > backfloorz)
		openbottom = frontfloorz;
	else
		openbottom = backfloorz;

	// quick test for totally closed doors
	if (openbottom >= opentop)
		return false;		// stop

	if (frontfloorz != backfloorz)
	{
		slope = (openbottom - sightstart.z) / frac;
		if (slope > bottomslope)
			bottomslope = slope;
	}

	if (frontceilz != backceilz)
	{
		slope = (opentop - sightstart.z) / frac;
		if (slope < topslope)
			topslope = slope;
	}

	if (topslope <= bottomslope)
		return false;		// stop

	return true;
#endif
}

//==========================================================================
//
//	CrossSubsector
//
//	Returns true if strace crosses the given subsector successfully.
//
//==========================================================================

static boolean CrossSubsector(int num)
{
	subsector_t*	sub;
	int				count;
	seg_t*			seg;
	int 			polyCount;
	seg_t**			polySeg;

#ifdef PARANOID
	if (num >= level.numsubsectors)
		Sys_Error("CrossSubsector: ss %i with numss = %i", num, level.numsubsectors);
#endif

	sub = &level.subsectors[num];

	if (sub->poly)
	{
		// Check the polyobj in the subsector first
		polyCount = sub->poly->numsegs;
		polySeg = sub->poly->segs;
		while (polyCount--)
		{
			if (!CheckLine(*polySeg++))
			{
				return false;
			}
		}
	}

	// check lines
	count = sub->numlines;
	seg = &level.segs[sub->firstline];

	for ( ; count ; seg++, count--)
	{
		if (!CheckLine(seg))
		{
			return false;
		}
	}
	// passed the subsector ok
	return true;		
}

//==========================================================================
//
//	CrossBSPNode
//
//	Returns true if strace crosses the given node successfully.
//
//==========================================================================

static boolean CrossBSPNode(int bspnum)
{
	node_t*	bsp;
	int		side;

	if (bspnum & NF_SUBSECTOR)
	{
		if (bspnum == -1)
			return CrossSubsector(0);
		else
			return CrossSubsector(bspnum & (~NF_SUBSECTOR));
	}

	bsp = &level.nodes[bspnum];

	// decide which side the start point is on
	side = PlaneSide2(sightstart, bsp);
	if (side == 2)
		side = 0;	// an "on" should cross both sides

	// cross the starting side
	if (!CrossBSPNode(bsp->children[side]))
		return false;

	// the partition plane is crossed here
	if (side == PlaneSide2(sightend, bsp))
	{
		// the line doesn't touch the other side
		return true;
	}

	// cross the ending side		
	return CrossBSPNode(bsp->children[side^1]);
}

//==========================================================================
//
//	SightTraceLine
//
//==========================================================================

static bool	SightTraceLine(sector_t *sec)
{
	sdelta = sightend - sightstart;
	strace.SetPointDir(sightstart, sdelta);

	linestart = sightstart;
	// the head node is the last node output
	if (!CrossBSPNode(level.numnodes - 1))
	{
		return false;
	}
	lineend = sightend;
	return CheckPlanes(sec);
}

//==========================================================================
//
//	P_CheckSight
//
//	Returns true if a straight line between t1 and t2 is unobstructed.
//
//==========================================================================

boolean P_CheckSight(VMapObject* t1, VMapObject* t2)
{
	guard(P_CheckSight);
	int			s1;
	int			s2;
	int			pnum;

	if (!t1 || !t2)
	{
//		cond << "Bad check sight\n";
		return false;
	}
	if ((t1->GetFlags() & OF_Destroyed) || (t2->GetFlags() & OF_Destroyed))
	{
//		cond << "Check sight with destroyed thing\n";
		return false;
	}

	//	Determine subsector entries in GL_PVS table.
	//	First check for trivial rejection.
	byte *vis = LeafPVS(level, t1->SubSector);
	s2 = t2->SubSector - level.subsectors;
	if (!(vis[s2 >> 3] & (1 << (s2 & 7))))
	{
		// can't possibly be connected
		return false;
	}

	//	Determine subsector entries in REJECT table.
	//	We must do this because REJECT can have some special effects like
	// "safe sectors"
	s1 = t1->Sector - level.sectors;
	s2 = t2->Sector - level.sectors;
	pnum = s1 * level.numsectors + s2;
	// Check in REJECT table.
	if (level.rejectmatrix[pnum >> 3] & (1 << (pnum & 7)))
	{
		// can't possibly be connected
		return false;
	}

	// An unobstructed LOS is possible.
	// Now look from eyes of t1 to any part of t2.
	validcount++;

	sightstart = t1->Origin;
	sightstart.z += t1->Height * 0.75;
	sightend = t2->Origin;
	sightend.z += t2->Height * 0.5;

#ifdef LINE_SIGHT
	//	Check middle
	if (SightTraceLine(t2->Sector))
	{
		return true;
	}

	//	Check head
	validcount++;
	sightend = t2->Origin;
	sightend.z += t2->Height;
	if (SightTraceLine(t2->Sector))
	{
		return true;
	}

	//	Check feats
	validcount++;
	sightend = t2->Origin;
	sightend.z -= t2->FloorClip;
	return SightTraceLine(t2->Sector);
#else
	topslope =    t2->Origin.z + t2->Height - sightstart.z;
	bottomslope = t2->Origin.z - sightstart.z;

	return SightTraceLine(t2->Sector);
#endif
	unguard;
}

//**************************************************************************
//
//	$Log$
//	Revision 1.9  2002/03/02 17:31:55  dj_jl
//	Improved sight checking with 3 rays.
//
//	Revision 1.8  2002/02/15 19:12:04  dj_jl
//	Property namig style change
//	
//	Revision 1.7  2002/01/07 12:16:43  dj_jl
//	Changed copyright year
//	
//	Revision 1.6  2002/01/04 18:21:48  dj_jl
//	A little protection against crashes
//	
//	Revision 1.5  2001/12/18 19:03:16  dj_jl
//	A lots of work on VObject
//	
//	Revision 1.4  2001/08/31 17:28:00  dj_jl
//	Removed RANGECHECK
//	
//	Revision 1.3  2001/07/31 17:16:31  dj_jl
//	Just moved Log to the end of file
//	
//	Revision 1.2  2001/07/27 14:27:54  dj_jl
//	Update with Id-s and Log-s, some fixes
//
//**************************************************************************
