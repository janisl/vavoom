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
static bool			SightEarlyOut;

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

static bool CheckLine(seg_t* seg)
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
	{
		SightEarlyOut = true;
		return false;
	}

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

static bool CrossSubsector(int num)
{
	subsector_t*	sub;
	int				count;
	seg_t*			seg;
	int 			polyCount;
	seg_t**			polySeg;

	sub = &GLevel->Subsectors[num];

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
	seg = &GLevel->Segs[sub->firstline];

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

static bool CrossBSPNode(int bspnum)
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

	bsp = &GLevel->Nodes[bspnum];

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
	if (!CrossBSPNode(GLevel->NumNodes - 1))
	{
		return false;
	}
	lineend = sightend;
	return CheckPlanes(sec);
}

//==========================================================================
//
//	VEntity::CanSee
//
//	Returns true if a straight line between t1 and t2 is unobstructed.
//
//==========================================================================

bool VEntity::CanSee(VEntity* Other)
{
	guard(P_CheckSight);
	int			s1;
	int			s2;
	int			pnum;

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

	// An unobstructed LOS is possible.
	// Now look from eyes of t1 to any part of t2.
	validcount++;

	sightstart = Origin;
	sightstart.z += Height * 0.75;
	sightend = Other->Origin;
	sightend.z += Other->Height * 0.5;

#ifdef LINE_SIGHT
	SightEarlyOut = false;
	//	Check middle
	if (SightTraceLine(Other->Sector))
	{
		return true;
	}
	if (SightEarlyOut)
	{
		return false;
	}

	//	Check head
	validcount++;
	sightend = Other->Origin;
	sightend.z += Other->Height;
	if (SightTraceLine(Other->Sector))
	{
		return true;
	}

	//	Check feats
	validcount++;
	sightend = Other->Origin;
	sightend.z -= Other->FloorClip;
	return SightTraceLine(Other->Sector);
#else
	topslope =    Other->Origin.z + Other->Height - sightstart.z;
	bottomslope = Other->Origin.z - sightstart.z;

	return SightTraceLine(Other->Sector);
#endif
	unguard;
}
