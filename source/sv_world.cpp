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
//**	Copyright (C) 1999-2001 JÆnis Legzdi·ý
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
#include "sv_local.h"
#include "moflags.h"

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

// MACROS ------------------------------------------------------------------

// TYPES -------------------------------------------------------------------

// EXTERNAL FUNCTION PROTOTYPES --------------------------------------------

// PUBLIC FUNCTION PROTOTYPES ----------------------------------------------

// PRIVATE FUNCTION PROTOTYPES ---------------------------------------------

// EXTERNAL DATA DECLARATIONS ----------------------------------------------

// PUBLIC DATA DEFINITIONS -------------------------------------------------

// PRIVATE DATA DEFINITIONS ------------------------------------------------

static opening_t	openings[32];

// CODE --------------------------------------------------------------------

//==========================================================================
//
//	SV_LineOpenings
//
//	Sets opentop and openbottom to the window through a two sided line.
//
//==========================================================================

opening_t *SV_LineOpenings(const line_t* linedef, const TVec& point)
{
	opening_t		*op;
	int				opsused;
	sec_region_t	*frontreg;
	sec_region_t	*backreg;
	sec_plane_t		*frontfloor = NULL;
	sec_plane_t		*backfloor = NULL;
	sec_plane_t		*frontceil = NULL;
	sec_plane_t		*backceil = NULL;
	float			frontfloorz;
	float			backfloorz;
	float			frontceilz;
	float			backceilz;

    if (linedef->sidenum[1] == -1)
    {
		// single sided line
		return NULL;
    }

	op = NULL;
	opsused = 0;
	frontreg = linedef->frontsector->botregion;
	backreg = linedef->backsector->botregion;

	while (frontreg && backreg)
	{
		if (!frontreg->floor->flags)
			frontfloor = frontreg->floor;
		if (!backreg->floor->flags)
			backfloor = backreg->floor;
		if (!frontreg->ceiling->flags)
			frontceil = frontreg->ceiling;
		if (!backreg->ceiling->flags)
			backceil = backreg->ceiling;
		if (backreg->ceiling->flags)
		{
			backreg = backreg->next;
			continue;
		}
		if (frontreg->ceiling->flags)
		{
			frontreg = frontreg->next;
			continue;
		}
		frontfloorz = frontfloor->GetPointZ(point);
		backfloorz = backfloor->GetPointZ(point);
		frontceilz = frontceil->GetPointZ(point);
		backceilz = backceil->GetPointZ(point);
		if (frontfloorz >= backceilz)
		{
			backreg = backreg->next;
			continue;
		}
		if (backfloorz >= frontceilz)
		{
			frontreg = frontreg->next;
			continue;
		}
		openings[opsused].next = op;
		op = &openings[opsused];
		opsused++;
		if (frontfloorz > backfloorz)
		{
			op->bottom = frontfloorz;
			op->lowfloor = backfloorz;
			op->floor = frontfloor;
		}
		else
		{
			op->bottom = backfloorz;
			op->lowfloor = frontfloorz;
			op->floor = backfloor;
		}
		if (frontceilz < backceilz)
		{
			op->top = frontceilz;
			op->ceiling = frontceil;
			frontreg = frontreg->next;
		}
		else
		{
			op->top = backceilz;
			op->ceiling = backceil;
			backreg = backreg->next;
		}
		op->range = op->top - op->bottom;
	}
	return op;
}

//==========================================================================
//
//	P_BoxOnLineSide
//
//	Considers the line to be infinite
//	Returns side 0 or 1, -1 if box crosses the line.
//
//==========================================================================

int P_BoxOnLineSide(float* tmbox, line_t* ld)
{
    int		p1 = 0;
    int		p2 = 0;
	
    switch (ld->slopetype)
    {
      case ST_HORIZONTAL:
		p1 = tmbox[BOXTOP] > ld->v1->y;
		p2 = tmbox[BOXBOTTOM] > ld->v1->y;
		if (ld->dir.x < 0)
		{
	    	p1 ^= 1;
	    	p2 ^= 1;
		}
		break;
	
      case ST_VERTICAL:
		p1 = tmbox[BOXRIGHT] < ld->v1->x;
		p2 = tmbox[BOXLEFT] < ld->v1->x;
		if (ld->dir.y < 0)
		{
	    	p1 ^= 1;
	    	p2 ^= 1;
		}
		break;
	
      case ST_POSITIVE:
		p1 = ld->PointOnSide(TVec(tmbox[BOXLEFT], tmbox[BOXTOP], 0));
		p2 = ld->PointOnSide(TVec(tmbox[BOXRIGHT], tmbox[BOXBOTTOM], 0));
		break;
	
      case ST_NEGATIVE:
		p1 = ld->PointOnSide(TVec(tmbox[BOXRIGHT], tmbox[BOXTOP], 0));
		p2 = ld->PointOnSide(TVec(tmbox[BOXLEFT], tmbox[BOXBOTTOM], 0));
		break;
    }

    if (p1 == p2)
		return p1;
    return -1;
}

//**************************************************************************
//
//	THING POSITION SETTING
//
//**************************************************************************

//==========================================================================
//
//	SV_UnlinkFromWorld
//
//	Unlinks a thing from block map and sectors. On each position change,
// BLOCKMAP and other lookups maintaining lists ot things inside these
// structures need to be updated.
//
//==========================================================================

void SV_UnlinkFromWorld(mobj_t* thing)
{
    int		blockx;
    int		blocky;

	if (!thing->subsector)
	{
		return;
	}

    if (!(thing->flags & MF_NOBLOCKMAP))
    {
		// inert things don't need to be in blockmap
		// unlink from block map
		if (thing->bnext)
		    thing->bnext->bprev = thing->bprev;
	
		if (thing->bprev)
		    thing->bprev->bnext = thing->bnext;
		else
		{
		    blockx = MapBlock(thing->origin.x - level.bmaporgx);
		    blocky = MapBlock(thing->origin.y - level.bmaporgy);

		    if (blockx >= 0 && blockx < level.bmapwidth &&
		    	blocky >= 0 && blocky < level.bmapheight)
		    {
				level.blocklinks[blocky * level.bmapwidth + blockx] = thing->bnext;
		    }
		}
    }
	thing->subsector = NULL;
}

//==========================================================================
//
//	SV_LinkToWorld
//
//	Links a thing into both a block and a subsector based on it's x y.
//	Sets thing->subsector properly
//
//==========================================================================

void SV_LinkToWorld(mobj_t* thing)
{
    subsector_t*	ss;
	sec_region_t*	reg;
	sec_region_t*	r;
    int				blockx;
    int				blocky;
    mobj_t**		link;

	if (thing->subsector)
	{
		SV_UnlinkFromWorld(thing);
	}

    // link into subsector
	ss = SV_PointInSubsector(thing->origin.x, thing->origin.y);
	reg = SV_FindThingGap(ss->sector->botregion, thing->origin,
		thing->origin.z, thing->origin.z + thing->height);
	thing->subsector = ss;

	r = reg;
	while (r->floor->flags)
		r = r->prev;
	thing->floor = r->floor;
	thing->floorz = r->floor->GetPointZ(thing->origin);

	r = reg;
	while (r->ceiling->flags)
		r = r->next;
	thing->ceiling = r->ceiling;
	thing->ceilingz = r->ceiling->GetPointZ(thing->origin);

    // link into blockmap
    if (!(thing->flags & MF_NOBLOCKMAP))
    {
		// inert things don't need to be in blockmap
		blockx = MapBlock(thing->origin.x - level.bmaporgx);
		blocky = MapBlock(thing->origin.y - level.bmaporgy);

		if (blockx >= 0 && blockx < level.bmapwidth &&
			blocky >= 0 && blocky < level.bmapheight)
		{
		    link = &level.blocklinks[blocky * level.bmapwidth + blockx];
	    	thing->bprev = NULL;
		    thing->bnext = *link;
		    if (*link)
				(*link)->bprev = thing;

		    *link = thing;
		}
		else
		{
		    // thing is off the map
		    thing->bnext = thing->bprev = NULL;
		}
    }
}

//**************************************************************************
//
//	BLOCK MAP ITERATORS
//
//	For each line/thing in the given mapblock, call the passed PIT_*
// function. If the function returns false, exit with false without checking
// anything else.
//
//**************************************************************************

//==========================================================================
//
//	SV_BlockLinesIterator
//
//	The validcount flags are used to avoid checking lines that are marked in
// multiple mapblocks, so increment validcount before the first call to
// P_BlockLinesIterator, then make one or more calls to it.
//
//==========================================================================

boolean SV_BlockLinesIterator(int x, int y, boolean(*func)(line_t*), int prfunc)
{
    int			offset;
    short*		list;
    line_t*		ld;
	int 		i;
	polyblock_t	*polyLink;
	seg_t		**tempSeg;

    if (x < 0 || y < 0 || x >= level.bmapwidth || y >= level.bmapheight)
    {
		return true;
    }
    
    offset = y * level.bmapwidth + x;

    //	Check polyobj blockmap
	polyLink = level.PolyBlockMap[offset];
	while (polyLink)
	{
		if (polyLink->polyobj)
		{
			if (polyLink->polyobj->validcount != validcount)
			{
				polyLink->polyobj->validcount = validcount;
				tempSeg = polyLink->polyobj->segs;
				for (i = 0; i < polyLink->polyobj->numsegs; i++, tempSeg++)
				{
					if ((*tempSeg)->linedef->validcount == validcount)
					{
						continue;
					}
					(*tempSeg)->linedef->validcount = validcount;
					if (func && !func((*tempSeg)->linedef))
					{
						return false;
					}
					if (prfunc && !svpr.Exec(prfunc, (int)(*tempSeg)->linedef))
					{
						return false;
					}
				}
			}
		}
		polyLink = polyLink->next;
	}

    offset = *(level.blockmap + offset);

    for (list = level.blockmaplump + offset; *list != -1; list++)
    {
		ld = &level.lines[*list];

		if (ld->validcount == validcount)
	    	continue; 	// line has already been checked

		ld->validcount = validcount;
		
		if (func && !func(ld))
	    	return false;
		if (prfunc && !svpr.Exec(prfunc, (int)ld))
	    	return false;
    }
    return true;	// everything was checked
}

//==========================================================================
//
//	SV_BlockThingsIterator
//
//==========================================================================

boolean SV_BlockThingsIterator(int x, int y, boolean(*func)(mobj_t*), int prfunc)
{
    mobj_t*		mobj;
	
    if (x < 0 || y < 0 || x >= level.bmapwidth || y >= level.bmapheight)
    {
		return true;
    }
    

    for (mobj = level.blocklinks[y*level.bmapwidth+x]; mobj; mobj = mobj->bnext)
    {
		if (func && !func(mobj))
		    return false;
		if (prfunc && !svpr.Exec(prfunc, (int)mobj))
		    return false;
    }
    return true;
}

//**************************************************************************
//
//	INTERCEPT ROUTINES
//
//**************************************************************************

#define MAXINTERCEPTS	128

static intercept_t	intercepts[MAXINTERCEPTS];
static intercept_t	*intercept_p;
static boolean 		earlyout;

static TPlane	 	trace_plane;
static TVec			trace_org;
static TVec			trace_dest;
static TVec			trace_delta;
static TVec			trace_dir;
static float		trace_len;

//==========================================================================
//
//	PIT_AddLineIntercepts
//
//	Looks for lines in the given block that intercept the given trace to add
// to the intercepts list.
//	A line is crossed if its endpoints are on opposite sides of the trace.
// Returns true if earlyout and a solid line hit.
//
//==========================================================================

static boolean PIT_AddLineIntercepts(line_t* ld)
{
	float dot1 = DotProduct(*ld->v1, trace_plane.normal) - trace_plane.dist;
	float dot2 = DotProduct(*ld->v2, trace_plane.normal) - trace_plane.dist;
    
	if (dot1 * dot2 >= 0)
	{
		return true;	// line isn't crossed
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
		return true;
	}
	num = ld->dist - DotProduct(trace_org, ld->normal);
	frac = num / den;

	if (frac < 0)
	{
		return true;	// behind source
	}
	
	// try to early out the check
	if (earlyout && frac < 1.0 && !ld->backsector)
	{
		return false;	// stop checking
	}

	if (intercept_p - intercepts >= MAXINTERCEPTS)
	{
		cond << "Intercepts overflow\n";
	}
	else
	{
		intercept_p->frac = frac;
		intercept_p->isaline = true;
		intercept_p->line = ld;
		intercept_p++;
	}

	return true;	// continue
}

//==========================================================================
//
//	PIT_AddThingIntercepts
//
//==========================================================================

static boolean PIT_AddThingIntercepts(mobj_t* thing)
{
	float dot = DotProduct(thing->origin, trace_plane.normal) - trace_plane.dist;
	if (dot >= thing->radius || dot <= -thing->radius)
	{
		return true;		// line isn't crossed
	}

	float dist = DotProduct((thing->origin - trace_org), trace_dir);
//	dist -= sqrt(thing->radius * thing->radius - dot * dot);
    if (dist < 0)
	{
		return true;		// behind source
	}
	float frac = dist / trace_len;

	if (intercept_p - intercepts >= MAXINTERCEPTS)
	{
		cond << "Intercepts overflow\n";
	}
	else
	{
		intercept_p->frac = frac;
		intercept_p->isaline = false;
		intercept_p->thing = thing;
		intercept_p++;
	}

    return true;		// keep going
}

//==========================================================================
//
//	SV_PathTraverse
//
//	Traces a line from x1,y1 to x2,y2, calling the traverser function for
// each. Returns true if the traverser function returns true for all lines.
//
//==========================================================================

boolean SV_PathTraverse(float x1, float y1, float x2, float y2,
		int flags, boolean(*trav)(intercept_t *), int prtrav)
{
	int			xt1;
	int			yt1;
	int			xt2;
	int			yt2;
    
	float		xstep;
	float		ystep;
    
	float		partial;
    
	float		xintercept;
	float		yintercept;
    
    int			mapx;
    int			mapy;
    
    int			mapxstep;
    int			mapystep;

    int			count;
		
    earlyout = flags & PT_EARLYOUT;
		
    validcount++;
    intercept_p = intercepts;
	memset(intercepts, 0, sizeof(intercepts));

	if (((FX(x1 - level.bmaporgx)) & (MAPBLOCKSIZE - 1)) == 0)
//	if (fmod(x1 - level.bmaporgx, MAPBLOCKSIZE) == 0.0)
		x1 += 1.0;	// don't side exactly on a line
    
	if (((FX(y1 - level.bmaporgy)) & (MAPBLOCKSIZE - 1)) == 0)
//	if (fmod(y1 - level.bmaporgy, MAPBLOCKSIZE) == 0.0)
		y1 += 1.0;	// don't side exactly on a line

	trace_org = TVec(x1, y1, 0);
	trace_dest = TVec(x2, y2, 0);
	trace_delta = trace_dest - trace_org;
	trace_dir = Normalize(trace_delta);
	trace_len = Length(trace_delta);

	trace_plane.SetPointDir(trace_org, trace_delta);

	x1 -= level.bmaporgx;
	y1 -= level.bmaporgy;
	xt1 = MapBlock(x1);
	yt1 = MapBlock(y1);

	x2 -= level.bmaporgx;
	y2 -= level.bmaporgy;
	xt2 = MapBlock(x2);
	yt2 = MapBlock(y2);

	if (xt2 > xt1)
	{
		mapxstep = 1;
		partial = 1.0 - FL((FX(x1) >> MAPBTOFRAC) & (FRACUNIT - 1));
//		partial = 1.0 - (x1 / 120.0 - xt1);
		ystep = (y2 - y1) / fabs(x2 - x1);
	}
	else if (xt2 < xt1)
	{
		mapxstep = -1;
		partial = FL((FX(x1) >> MAPBTOFRAC) & (FRACUNIT - 1));
//		partial = x1 / MAPBLOCKSIZE - xt1;
		ystep = (y2 - y1) / fabs(x2 - x1);
	}
	else
	{
		mapxstep = 0;
		partial = 1.0;
		ystep = 256.0;
	}
	yintercept = FL(FX(y1) >> MAPBTOFRAC) + partial * ystep;
//	yintercept = y1 / MAPBLOCKSIZE + partial * ystep;

	if (yt2 > yt1)
	{
		mapystep = 1;
		partial = 1.0 - FL((FX(y1) >> MAPBTOFRAC) & (FRACUNIT - 1));
//		partial = 1.0 - (y1 / MAPBLOCKSIZE - yt1);
		xstep = (x2 - x1) / fabs(y2 - y1);
	}
	else if (yt2 < yt1)
	{
		mapystep = -1;
		partial = FL((FX(y1) >> MAPBTOFRAC) & (FRACUNIT - 1));
//		partial = y1 / MAPBLOCKSIZE - yt1;
		xstep = (x2 - x1) / fabs(y2 - y1);
	}
	else
	{
		mapystep = 0;
		partial = 1.0;
		xstep = 256.0;
	}
	xintercept = FL(FX(x1) >> MAPBTOFRAC) + partial * xstep;
//	xintercept = x1 / MAPBLOCKSIZE + partial * xstep;
    
    // Step through map blocks.
    // Count is present to prevent a round off error
    // from skipping the break.
    mapx = xt1;
    mapy = yt1;
	
    for (count = 0 ; count < 64 ; count++)
    {
		if (flags & PT_ADDLINES)
		{
		    if (!SV_BlockLinesIterator(mapx, mapy, PIT_AddLineIntercepts, 0))
				return false;	// early out
		}
	
		if (flags & PT_ADDTHINGS)
		{
		    if (!SV_BlockThingsIterator(mapx, mapy, PIT_AddThingIntercepts, 0))
				return false;	// early out
		}
		
		if (mapx == xt2 && mapy == yt2)
		{
		    break;
		}
	
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
		
    }

    // go through the sorted list
	float			dist;
    intercept_t*	scan;
    intercept_t*	in;
	
    count = intercept_p - intercepts;
    
    in = 0;			// shut up compiler warning
	
    while (count--)
    {
		dist = 99999.0;
		for (scan = intercepts; scan < intercept_p; scan++)
		{
		    if (scan->frac < dist)
		    {
				dist = scan->frac;
				in = scan;
		    }
		}
	
		if (dist > 1.0)
		    return true;	// checked everything in range

        if (trav && !trav(in))
		    return false;	// don't bother going farther

        if (prtrav && !svpr.Exec(prtrav, (int)in))
		    return false;	// don't bother going farther

		in->frac = 99999.0;
    }
	
    return true;		// everything was traversed
}

//==========================================================================
//
//	SV_FindThingGap
//
// Find the best gap that the thing could fit in, given a certain Z
// position (z1 is foot, z2 is head).  Assuming at least two gaps exist,
// the best gap is chosen as follows:
//
// 1. if the thing fits in one of the gaps without moving vertically,
//    then choose that gap.
//
// 2. if there is only *one* gap which the thing could fit in, then
//    choose that gap.
//
// 3. if there is multiple gaps which the thing could fit in, choose
//    the gap whose floor is closest to the thing's current Z.
//
// 4. if there is no gaps which the thing could fit in, do the same.
//
// Returns the gap number, or -1 if there are no gaps at all.
//
//==========================================================================

sec_region_t *SV_FindThingGap(sec_region_t *gaps, const TVec &point, float z1, float z2)
{
	float dist;

	int fit_num = 0;
	sec_region_t *fit_last = NULL;

	sec_region_t *fit_closest = NULL;
	float fit_mindist = 200000.0;

	sec_region_t *nofit_closest = NULL;
	float nofit_mindist = 200000.0;

	// check for trivial gaps...
	if (!gaps)
	{
		return NULL;
	}
	if (!gaps->next)
	{
		return gaps;
	}

	sec_plane_t *floor = NULL;
	sec_plane_t *ceil = NULL;

	// There are 2 or more gaps. Now it gets interesting :-)
	while (gaps)
	{
		float	f;
		float	c;

		if (!gaps->floor->flags)
			floor = gaps->floor;
		if (!gaps->ceiling->flags)
			ceil = gaps->ceiling;
		if (gaps->ceiling->flags)
		{
			gaps = gaps->next;
			continue;
		}
		f = floor->GetPointZ(point);
		c = ceil->GetPointZ(point);
		if (z1 >= f && z2 <= c)
		{ // [1]
			return gaps;
		}

		dist = fabs(z1 - f);

		if (z2 - z1 <= c - f)
		{ // [2]
			fit_num++;

			fit_last = gaps;
			if (dist < fit_mindist)
			{ // [3]
				fit_mindist = dist;
				fit_closest = gaps;
			}
		}
		else
		{
			if (dist < nofit_mindist)
			{ // [4]
				nofit_mindist = dist;
				nofit_closest = gaps;
			}
		}
		gaps = gaps->next;
	}

	if (fit_num == 1)
		return fit_last;
	if (fit_num > 1)
		return fit_closest;
	return nofit_closest;
}

//==========================================================================
//
//	SV_FindOpening
//
// Find the best gap that the thing could fit in, given a certain Z
// position (z1 is foot, z2 is head).  Assuming at least two gaps exist,
// the best gap is chosen as follows:
//
// 1. if the thing fits in one of the gaps without moving vertically,
//    then choose that gap.
//
// 2. if there is only *one* gap which the thing could fit in, then
//    choose that gap.
//
// 3. if there is multiple gaps which the thing could fit in, choose
//    the gap whose floor is closest to the thing's current Z.
//
// 4. if there is no gaps which the thing could fit in, do the same.
//
// Returns the gap number, or -1 if there are no gaps at all.
//
//==========================================================================

opening_t *SV_FindOpening(opening_t *gaps, float z1, float z2)
{
	float dist;

	int fit_num = 0;
	opening_t *fit_last = NULL;

	opening_t *fit_closest = NULL;
	float fit_mindist = 99999.0;

	opening_t *nofit_closest = NULL;
	float nofit_mindist = 99999.0;

	// check for trivial gaps...
	if (!gaps)
	{
		return NULL;
	}
	if (!gaps->next)
	{
		return gaps;
	}

	// There are 2 or more gaps. Now it gets interesting :-)
	while (gaps)
	{
		float	f;
		float	c;

		f = gaps->bottom;
		c = gaps->top;
		if (z1 >= f && z2 <= c)
		{ // [1]
			return gaps;
		}

		dist = fabs(z1 - f);

		if (z2 - z1 <= c - f)
		{ // [2]
			fit_num++;

			fit_last = gaps;
			if (dist < fit_mindist)
			{ // [3]
				fit_mindist = dist;
				fit_closest = gaps;
			}
		}
		else
		{
			if (dist < nofit_mindist)
			{ // [4]
				nofit_mindist = dist;
				nofit_closest = gaps;
			}
		}
		gaps = gaps->next;
	}

	if (fit_num == 1)
		return fit_last;
	if (fit_num > 1)
		return fit_closest;
	return nofit_closest;
}

//==========================================================================
//
//	SV_PointInRegions
//
//==========================================================================

sec_region_t *SV_PointInRegion(sector_t *sector, const TVec &p)
{
	sec_region_t *reg;

	// logic: find matching region, otherwise return highest one.
	for (reg = sector->botregion; reg && reg->next; reg = reg->next)
	{
		if (p.z < reg->ceiling->GetPointZ(p))
			break;
	}

	return reg;
}

//==========================================================================
//
//	SV_PointContents
//
//==========================================================================

int SV_PointContents(const sector_t *sector, const TVec &p)
{
	sec_region_t	*reg;

	for (reg = sector->botregion; reg; reg = reg->next)
	{
		if (p.z <= reg->ceiling->GetPointZ(p) &&
			p.z >= reg->floor->GetPointZ(p))
		{
			return reg->params->contents;
		}
	}
	return -1;
}

//**************************************************************************
//
//	$Log$
//	Revision 1.6  2001/10/27 07:49:29  dj_jl
//	Fixed map block stuff
//
//	Revision 1.5  2001/10/22 17:25:55  dj_jl
//	Floatification of angles
//	
//	Revision 1.4  2001/10/08 17:34:57  dj_jl
//	A lots of small changes and cleanups
//	
//	Revision 1.3  2001/07/31 17:16:31  dj_jl
//	Just moved Log to the end of file
//	
//	Revision 1.2  2001/07/27 14:27:54  dj_jl
//	Update with Id-s and Log-s, some fixes
//
//**************************************************************************
