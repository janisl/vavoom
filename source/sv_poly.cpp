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

// HEADER FILES ------------------------------------------------------------

#include "gamedefs.h"
#include "sv_local.h"
#include "moflags.h"

// MACROS ------------------------------------------------------------------

//FIXME: Read these from a script or set them from PROGS
#define PO_LINE_START 		1 // polyobj line start special
#define PO_LINE_EXPLICIT	5

#define PO_MAXPOLYSEGS 		64

// TYPES -------------------------------------------------------------------

// EXTERNAL FUNCTION PROTOTYPES --------------------------------------------

// PRIVATE FUNCTION PROTOTYPES ---------------------------------------------

static void ThrustMobj(VMapObject *mobj, seg_t *seg, polyobj_t *po);
static void UnLinkPolyobj(polyobj_t *po);
static void LinkPolyobj(polyobj_t *po);
static boolean CheckMobjBlocking(seg_t *seg, polyobj_t *po);
static void InitBlockMap(void);
static void IterFindPolySegs(float x, float y, seg_t **segList);

// EXTERNAL DATA DECLARATIONS ----------------------------------------------

// PUBLIC DATA DEFINITIONS -------------------------------------------------

// PRIVATE DATA DEFINITIONS ------------------------------------------------

static int 		PolySegCount;
static float	PolyStartX;
static float	PolyStartY;
static int		NumAnchorPoints = 0;
static struct AnchorPoint_t
{
	float		x;
	float		y;
    int			tag;
} *AnchorPoints;

// CODE --------------------------------------------------------------------

//==========================================================================
//
// PO_GetPolyobj
//
//==========================================================================

polyobj_t *PO_GetPolyobj(int polyNum)
{
	int i;

	for (i = 0; i < level.numpolyobjs; i++)
	{
		if (level.polyobjs[i].tag == polyNum)
		{
			return &level.polyobjs[i];
		}
	}
	return NULL;
}

//==========================================================================
//
// PO_GetPolyobjMirror
//
//==========================================================================

int PO_GetPolyobjMirror(int poly)
{
	int i;

	for(i = 0; i < level.numpolyobjs; i++)
	{
		if(level.polyobjs[i].tag == poly)
		{
			return((*level.polyobjs[i].segs)->linedef->arg2);
		}
	}
	return 0;
}

//==========================================================================
//
//	ThrustMobj
//
//==========================================================================

static void ThrustMobj(VMapObject *mobj, seg_t *seg, polyobj_t *po)
{
	svpr.Exec("PolyThrustMobj", (int)mobj,
		seg->normal.x, seg->normal.y, seg->normal.z, (int)po);
}


//==========================================================================
//
//	UpdatePolySegs
//
//==========================================================================

static void	UpdatePolySegs(polyobj_t *po)
{
	seg_t **segList;
	int count;

	validcount++;
	for (count = po->numsegs, segList = po->segs; count; count--, segList++)
	{
		if ((*segList)->linedef->validcount != validcount)
		{
			//	Recalc lines's slope type, bounding box, normal and dist
			CalcLine((*segList)->linedef);
			(*segList)->linedef->validcount = validcount;
		}
		//	Recalc seg's normal and dist
		CalcSeg(*segList);
	}
}

//==========================================================================
//
// PO_MovePolyobj
//
//==========================================================================

boolean PO_MovePolyobj(int num, float x, float y)
{
	int count;
	seg_t **segList;
	seg_t **veryTempSeg;
	polyobj_t *po;
	vertex_t *prevPts;
	boolean blocked;

	if (!(po = PO_GetPolyobj(num)))
	{
		Sys_Error("PO_MovePolyobj:  Invalid polyobj number: %d\n", num);
	}

	UnLinkPolyobj(po);

	segList = po->segs;
	prevPts = po->prevPts;
	blocked = false;

	for (count = po->numsegs; count; count--, segList++, prevPts++)
	{
		for (veryTempSeg = po->segs; veryTempSeg != segList;
			veryTempSeg++)
		{
			if ((*veryTempSeg)->v1 == (*segList)->v1)
			{
				break;
			}
		}
		if (veryTempSeg == segList)
		{
			(*segList)->v1->x += x;
			(*segList)->v1->y += y;
		}
		(*prevPts).x += x; // previous points are unique for each seg
		(*prevPts).y += y;
	}
	UpdatePolySegs(po);
	segList = po->segs;
	for (count = po->numsegs; count; count--, segList++)
	{
		if (CheckMobjBlocking(*segList, po))
		{
			blocked = true;
		}
	}
	if (blocked)
	{
		count = po->numsegs;
		segList = po->segs;
		prevPts = po->prevPts;
		while (count--)
		{
			for (veryTempSeg = po->segs; veryTempSeg != segList;
				veryTempSeg++)
			{
				if ((*veryTempSeg)->v1 == (*segList)->v1)
				{
					break;
				}
			}
			if (veryTempSeg == segList)
			{
				(*segList)->v1->x -= x;
				(*segList)->v1->y -= y;
			}
			(*prevPts).x -= x;
			(*prevPts).y -= y;
			segList++;
			prevPts++;
		}
		UpdatePolySegs(po);
		LinkPolyobj(po);
		return false;
	}
	po->startSpot.x += x;
	po->startSpot.y += y;
	LinkPolyobj(po);
	return true;
}

//==========================================================================
//
//	RotatePt
//
//==========================================================================

static void RotatePt(float an, float *x, float *y, float startSpotX, float startSpotY)
{
	float	tr_x, tr_y;
	float 	gxt, gyt;

	tr_x = *x;
	tr_y = *y;

	gxt = tr_x * mcos(an);
	gyt = tr_y * msin(an);
	*x = (gxt - gyt) + startSpotX;

	gxt = tr_x * msin(an);
	gyt = tr_y * mcos(an);
	*y = (gyt + gxt) + startSpotY;
}

//==========================================================================
//
// PO_RotatePolyobj
//
//==========================================================================

boolean PO_RotatePolyobj(int num, float angle)
{
	int count;
	seg_t **segList;
	vertex_t *originalPts;
	vertex_t *prevPts;
	float an;
	polyobj_t *po;
	boolean blocked;

	if (!(po = PO_GetPolyobj(num)))
	{
		Sys_Error("PO_RotatePolyobj:  Invalid polyobj number: %d\n", num);
	}
	an = po->angle + angle;

	UnLinkPolyobj(po);

	segList = po->segs;
	originalPts = po->originalPts;
	prevPts = po->prevPts;

	for (count = po->numsegs; count; count--, segList++, originalPts++,
		prevPts++)
	{
		prevPts->x = (*segList)->v1->x;
		prevPts->y = (*segList)->v1->y;
		(*segList)->v1->x = originalPts->x;
		(*segList)->v1->y = originalPts->y;
		RotatePt(an, &(*segList)->v1->x, &(*segList)->v1->y,
			po->startSpot.x, po->startSpot.y);
	}
	UpdatePolySegs(po);
	segList = po->segs;
	blocked = false;
	for (count = po->numsegs; count; count--, segList++)
	{
		if (CheckMobjBlocking(*segList, po))
		{
			blocked = true;
		}
	}
	if (blocked)
	{
		segList = po->segs;
		prevPts = po->prevPts;
		for (count = po->numsegs; count; count--, segList++, prevPts++)
		{
			(*segList)->v1->x = prevPts->x;
			(*segList)->v1->y = prevPts->y;
		}
		UpdatePolySegs(po);
		LinkPolyobj(po);
		return false;
	}
	po->angle = AngleMod(po->angle + angle);
	LinkPolyobj(po);
	return true;
}

//==========================================================================
//
// UnLinkPolyobj
//
//==========================================================================

static void UnLinkPolyobj(polyobj_t *po)
{
	polyblock_t *link;
	int i, j;
	int index;

	// remove the polyobj from each blockmap section
	for (j = po->bbox[BOXBOTTOM]; j <= po->bbox[BOXTOP]; j++)
	{
		index = j * level.bmapwidth;
		for(i = po->bbox[BOXLEFT]; i <= po->bbox[BOXRIGHT]; i++)
		{
			if (i >= 0 && i < level.bmapwidth && j >= 0 && j < level.bmapheight)
			{
				link = level.PolyBlockMap[index+i];
				while (link != NULL && link->polyobj != po)
				{
					link = link->next;
				}
				if (link == NULL)
				{ // polyobj not located in the link cell
					continue;
				}
				link->polyobj = NULL;
			}
		}
	}
}

//==========================================================================
//
// LinkPolyobj
//
//==========================================================================

static void LinkPolyobj(polyobj_t *po)
{
	float leftX, rightX;
	float topY, bottomY;
	seg_t **tempSeg;
	polyblock_t **link;
	polyblock_t *tempLink;
	int i, j;

	// calculate the polyobj bbox
	tempSeg = po->segs;
	rightX = leftX = (*tempSeg)->v1->x;
	topY = bottomY = (*tempSeg)->v1->y;

	for (i = 0; i < po->numsegs; i++, tempSeg++)
	{
		if ((*tempSeg)->v1->x > rightX)
		{
			rightX = (*tempSeg)->v1->x;
		}
		if ((*tempSeg)->v1->x < leftX)
		{
			leftX = (*tempSeg)->v1->x;
		}
		if ((*tempSeg)->v1->y > topY)
		{
			topY = (*tempSeg)->v1->y;
		}
		if ((*tempSeg)->v1->y < bottomY)
		{
			bottomY = (*tempSeg)->v1->y;
		}
	}
	po->bbox[BOXRIGHT] = MapBlock(rightX - level.bmaporgx);
	po->bbox[BOXLEFT] = MapBlock(leftX - level.bmaporgx);
	po->bbox[BOXTOP] = MapBlock(topY - level.bmaporgy);
	po->bbox[BOXBOTTOM] = MapBlock(bottomY - level.bmaporgy);
	// add the polyobj to each blockmap section
	for(j = po->bbox[BOXBOTTOM]*level.bmapwidth; j <= po->bbox[BOXTOP]*level.bmapwidth;
		j += level.bmapwidth)
	{
		for(i = po->bbox[BOXLEFT]; i <= po->bbox[BOXRIGHT]; i++)
		{
			if(i >= 0 && i < level.bmapwidth && j >= 0 && j < level.bmapheight*level.bmapwidth)
			{
				link = &level.PolyBlockMap[j+i];
				if(!(*link))
				{ // Create a new link at the current block cell
					*link = (polyblock_t*)Z_Malloc(sizeof(polyblock_t), PU_LEVEL, 0);
					(*link)->next = NULL;
					(*link)->prev = NULL;
					(*link)->polyobj = po;
					continue;
				}
				else
				{
					tempLink = *link;
					while(tempLink->next != NULL && tempLink->polyobj != NULL)
					{
						tempLink = tempLink->next;
					}
				}
				if(tempLink->polyobj == NULL)
				{
					tempLink->polyobj = po;
					continue;
				}
				else
				{
					tempLink->next = (polyblock_t*)Z_Malloc(sizeof(polyblock_t),
						PU_LEVEL, 0);
					tempLink->next->next = NULL;
					tempLink->next->prev = tempLink;
					tempLink->next->polyobj = po;
				}
			}
			// else, don't link the polyobj, since it's off the map
		}
	}
}

//==========================================================================
//
// CheckMobjBlocking
//
//==========================================================================

static boolean CheckMobjBlocking(seg_t *seg, polyobj_t *po)
{
	VMapObject *mobj;
	int i, j;
	int left, right, top, bottom;
	float	tmbbox[4];
	line_t *ld;
	boolean blocked;

	ld = seg->linedef;

	top = MapBlock(ld->bbox[BOXTOP] - level.bmaporgy + MAXRADIUS);
	bottom = MapBlock(ld->bbox[BOXBOTTOM] - level.bmaporgy - MAXRADIUS);
	left = MapBlock(ld->bbox[BOXLEFT] - level.bmaporgx - MAXRADIUS);
	right = MapBlock(ld->bbox[BOXRIGHT] - level.bmaporgx + MAXRADIUS);

	blocked = false;

	bottom = bottom < 0 ? 0 : bottom;
	bottom = bottom >= level.bmapheight ? level.bmapheight-1 : bottom;
	top = top < 0 ? 0 : top;
	top = top >= level.bmapheight  ? level.bmapheight-1 : top;
	left = left < 0 ? 0 : left;
	left = left >= level.bmapwidth ? level.bmapwidth-1 : left;
	right = right < 0 ? 0 : right;
	right = right >= level.bmapwidth ?  level.bmapwidth-1 : right;

	for(j = bottom*level.bmapwidth; j <= top*level.bmapwidth; j += level.bmapwidth)
	{
		for(i = left; i <= right; i++)
		{
			for(mobj = level.blocklinks[j+i]; mobj; mobj = mobj->bnext)
			{
				if(mobj->flags&MF_SOLID || mobj->player)
				{
					tmbbox[BOXTOP] = mobj->origin.y + mobj->radius;
					tmbbox[BOXBOTTOM] = mobj->origin.y - mobj->radius;
					tmbbox[BOXLEFT] = mobj->origin.x - mobj->radius;
					tmbbox[BOXRIGHT] = mobj->origin.x + mobj->radius;

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
					ThrustMobj(mobj, seg, po);
					blocked = true;
				}
			}
		}
	}
	return blocked;
}

//==========================================================================
//
// InitBlockMap
//
//==========================================================================

static void InitBlockMap(void)
{
	int		i;

	level.PolyBlockMap = (polyblock_t**)Z_Malloc(level.bmapwidth *
		level.bmapheight * sizeof(polyblock_t *), PU_LEVEL, 0);
	memset(level.PolyBlockMap, 0, level.bmapwidth * level.bmapheight *
		sizeof(polyblock_t *));

	for (i = 0; i < level.numpolyobjs; i++)
	{
		LinkPolyobj(&level.polyobjs[i]);
	}
}

//==========================================================================
//
//	IterFindPolySegs
//
//	Passing NULL for segList will cause IterFindPolySegs to count the number
// of segs in the polyobj
//
//==========================================================================

static void IterFindPolySegs(float x, float y, seg_t **segList)
{
	int i;

	if (x == PolyStartX && y == PolyStartY)
	{
		return;
	}
	for (i = 0; i < level.numsegs; i++)
	{
		if (!level.segs[i].linedef)
			continue;
		if (level.segs[i].v1->x == x && level.segs[i].v1->y == y)
		{
			if (!segList)
			{
				PolySegCount++;
			}
			else
			{
				*segList++ = &level.segs[i];
			}
			IterFindPolySegs(level.segs[i].v2->x, level.segs[i].v2->y, segList);
			return;
		}
	}
	Sys_Error("IterFindPolySegs:  Non-closed Polyobj located.\n");
}


//==========================================================================
//
//	PO_SpawnPolyobj
//
//==========================================================================

void PO_SpawnPolyobj(float x, float y, int tag, int crush)
{
	int i;
	int j;
	int psIndex;
	int psIndexOld;
	seg_t *polySegList[PO_MAXPOLYSEGS];
    int index;

	sv_signon << (byte)svc_poly_spawn
				<< (word)x
				<< (word)y
				<< (byte)tag;

	index = level.numpolyobjs++;
	if (level.numpolyobjs == 1)
    {
		level.polyobjs = (polyobj_t*)Z_Malloc(sizeof(polyobj_t), PU_LEVEL, 0);
	}
    else
    {
    	Z_Resize((void**)&level.polyobjs, level.numpolyobjs * sizeof(polyobj_t));
    }
	memset(&level.polyobjs[index], 0, sizeof(polyobj_t));

	level.polyobjs[index].startSpot.x = x;
	level.polyobjs[index].startSpot.y = y;
	for (i = 0; i < level.numsegs; i++)
	{
		if (!level.segs[i].linedef)
			continue;
		if (level.segs[i].linedef->special == PO_LINE_START &&
			level.segs[i].linedef->arg1 == tag)
		{
			if (level.polyobjs[index].segs)
			{
            	//	Immpossible, because it is just cleared out
				Sys_Error("PO_SpawnPolyobj:  Polyobj %d already spawned.\n", tag);
			}
			level.segs[i].linedef->special = 0;
			level.segs[i].linedef->arg1 = 0;
			PolySegCount = 1;
			PolyStartX = level.segs[i].v1->x;
			PolyStartY = level.segs[i].v1->y;
			IterFindPolySegs(level.segs[i].v2->x, level.segs[i].v2->y, NULL);

			level.polyobjs[index].numsegs = PolySegCount;
			level.polyobjs[index].segs = (seg_t**)Z_Malloc(PolySegCount*sizeof(seg_t *),
				PU_LEVEL, 0);
			*(level.polyobjs[index].segs) = &level.segs[i]; // insert the first seg
			IterFindPolySegs(level.segs[i].v2->x, level.segs[i].v2->y,
				level.polyobjs[index].segs + 1);
			level.polyobjs[index].crush = crush;
			level.polyobjs[index].tag = tag;
			level.polyobjs[index].seqType = level.segs[i].linedef->arg3;
//			if(level.polyobjs[index].seqType < 0
//				|| level.polyobjs[index].seqType >= SEQTYPE_NUMSEQ)
//			{
//				level.polyobjs[index].seqType = 0;
//			}
			break;
		}
	}
	if (!level.polyobjs[index].segs)
	{
		// didn't find a polyobj through PO_LINE_START
		psIndex = 0;
		level.polyobjs[index].numsegs = 0;
		for(j = 1; j < PO_MAXPOLYSEGS; j++)
		{
			psIndexOld = psIndex;
			for (i = 0; i < level.numsegs; i++)
			{
				if (!level.segs[i].linedef)
					continue;
				if (level.segs[i].linedef->special == PO_LINE_EXPLICIT &&
					level.segs[i].linedef->arg1 == tag)
				{
					if(!level.segs[i].linedef->arg2)
					{
						Sys_Error("PO_SpawnPolyobj:  Explicit line missing order number (probably %d) in poly %d.\n",
							j+1, tag);
					}
					if(level.segs[i].linedef->arg2 == j)
					{
						polySegList[psIndex] = &level.segs[i];
						level.polyobjs[index].numsegs++;
						psIndex++;
						if(psIndex > PO_MAXPOLYSEGS)
						{
							Sys_Error("PO_SpawnPolyobj:  psIndex > PO_MAXPOLYSEGS\n");
						}
					}
				}
			}
			// Clear out any specials for these segs...we cannot clear them out
			// 	in the above loop, since we aren't guaranteed one seg per
			//		linedef.
			for (i = 0; i < level.numsegs; i++)
			{
				if (!level.segs[i].linedef)
					continue;
				if(level.segs[i].linedef->special == PO_LINE_EXPLICIT &&
					level.segs[i].linedef->arg1 == tag && level.segs[i].linedef->arg2 == j)
				{
					level.segs[i].linedef->special = 0;
					level.segs[i].linedef->arg1 = 0;
				}
			}
			if(psIndex == psIndexOld)
			{ // Check if an explicit line order has been skipped
				// A line has been skipped if there are any more explicit
				// lines with the current tag value
				for (i = 0; i < level.numsegs; i++)
				{
					if (!level.segs[i].linedef)
						continue;
					if (level.segs[i].linedef->special == PO_LINE_EXPLICIT &&
						level.segs[i].linedef->arg1 == tag)
					{
						Sys_Error("PO_SpawnPolyobj:  Missing explicit line %d for poly %d\n",
							j, tag);
					}
				}
			}
		}
		if(level.polyobjs[index].numsegs)
		{
			PolySegCount = level.polyobjs[index].numsegs; // PolySegCount used globally
			level.polyobjs[index].crush = crush;
			level.polyobjs[index].tag = tag;
			level.polyobjs[index].segs = (seg_t**)Z_Malloc(level.polyobjs[index].numsegs
				* sizeof(seg_t *), PU_LEVEL, 0);
			for(i = 0; i < level.polyobjs[index].numsegs; i++)
			{
				level.polyobjs[index].segs[i] = polySegList[i];
			}
			level.polyobjs[index].seqType = (*level.polyobjs[index].segs)->linedef->arg4;
		}
		// Next, change the polyobjs first line to point to a mirror
		//		if it exists
		(*level.polyobjs[index].segs)->linedef->arg2 =
			(*level.polyobjs[index].segs)->linedef->arg3;
	}
}

//==========================================================================
//
//	PO_AddAnchorPoint
//
//==========================================================================

void PO_AddAnchorPoint(float x, float y, int tag)
{
	int		index;

    index = NumAnchorPoints++;
	if (NumAnchorPoints == 1)
    {
    	AnchorPoints = (AnchorPoint_t*)Z_Malloc(sizeof(*AnchorPoints), PU_LEVEL, 0);
	}
    else
    {
    	Z_Resize((void**)&AnchorPoints, NumAnchorPoints * sizeof(*AnchorPoints));
    }
	AnchorPoints[index].x = x;
    AnchorPoints[index].y = y;
    AnchorPoints[index].tag = tag;
}

//==========================================================================
//
//	TranslateToStartSpot
//
//==========================================================================

static void TranslateToStartSpot(float originX, float originY, int tag)
{
	seg_t **tempSeg;
	seg_t **veryTempSeg;
	vertex_t *tempPt;
	subsector_t *sub;
	polyobj_t *po;
	float deltaX;
	float deltaY;
	vertex_t avg; // used to find a polyobj's center, and hence subsector
	int i;

	sv_signon << (byte)svc_poly_translate
				<< (word)originX
				<< (word)originY
				<< (byte)tag;

	po = NULL;
	for(i = 0; i < level.numpolyobjs; i++)
	{
		if(level.polyobjs[i].tag == tag)
		{
			po = &level.polyobjs[i];
			break;
		}
	}
	if(!po)
	{ // didn't match the tag with a polyobj tag
		Sys_Error("TranslateToStartSpot:  Unable to match polyobj tag: %d\n",
			tag);
	}
	if(po->segs == NULL)
	{
		Sys_Error("TranslateToStartSpot:  Anchor point located without a StartSpot point: %d\n", tag);
	}
	po->originalPts = (vertex_t*)Z_Malloc(po->numsegs*sizeof(vertex_t), PU_LEVEL, 0);
	po->prevPts = (vertex_t*)Z_Malloc(po->numsegs*sizeof(vertex_t), PU_LEVEL, 0);
	deltaX = originX - po->startSpot.x;
	deltaY = originY - po->startSpot.y;

	tempSeg = po->segs;
	tempPt = po->originalPts;
	avg.x = 0;
	avg.y = 0;

	for(i = 0; i < po->numsegs; i++, tempSeg++, tempPt++)
	{
		for (veryTempSeg = po->segs; veryTempSeg != tempSeg; veryTempSeg++)
		{
			if ((*veryTempSeg)->v1 == (*tempSeg)->v1)
			{
				break;
			}
		}
		if (veryTempSeg == tempSeg)
		{ // the point hasn't been translated, yet
			(*tempSeg)->v1->x -= deltaX;
			(*tempSeg)->v1->y -= deltaY;
		}
		avg.x += (*tempSeg)->v1->x;
		avg.y += (*tempSeg)->v1->y;
		// the original Pts are based off the startSpot Pt, and are
		// unique to each seg, not each linedef
		tempPt->x = (*tempSeg)->v1->x - po->startSpot.x;
		tempPt->y = (*tempSeg)->v1->y - po->startSpot.y;
		tempPt->z = 0;
	}
	UpdatePolySegs(po);
	avg.x /= po->numsegs;
	avg.y /= po->numsegs;
	sub = SV_PointInSubsector(avg.x, avg.y);
	if(sub->poly != NULL)
	{
		Sys_Error("PO_TranslateToStartSpot:  Multiple polyobjs in a single subsector.\n");
	}
	sub->poly = po;
	po->subsector = sub;

	po->base_x = po->startSpot.x;
	po->base_y = po->startSpot.y;
	po->base_angle = po->angle;
}

//==========================================================================
//
//	PO_Init
//
//==========================================================================

void PO_Init(void)
{
	int				i;

	for (i=0; i<NumAnchorPoints; i++)
    {
		TranslateToStartSpot(AnchorPoints[i].x, AnchorPoints[i].y,
			AnchorPoints[i].tag);
    }

    NumAnchorPoints = 0;

	// check for a startspot without an anchor point
	for (i = 0; i < level.numpolyobjs; i++)
	{
		if (!level.polyobjs[i].originalPts)
		{
			Sys_Error("PO_Init:  StartSpot located without an Anchor point: %d\n",
				level.polyobjs[i].tag);
		}
	}
	InitBlockMap();
}

//==========================================================================
//
// PO_Busy
//
//==========================================================================

boolean PO_Busy(int polyobj)
{
	return svpr.Exec("PolyBusy", polyobj);
}

//**************************************************************************
//
//	$Log$
//	Revision 1.9  2001/12/18 19:03:16  dj_jl
//	A lots of work on VObject
//
//	Revision 1.8  2001/10/22 17:25:55  dj_jl
//	Floatification of angles
//	
//	Revision 1.7  2001/10/18 17:36:31  dj_jl
//	A lots of changes for Alpha 2
//	
//	Revision 1.6  2001/10/04 17:16:54  dj_jl
//	Removed some unused code
//	
//	Revision 1.5  2001/10/02 17:43:50  dj_jl
//	Added addfields to lines, sectors and polyobjs
//	
//	Revision 1.4  2001/09/20 16:27:02  dj_jl
//	Removed degenmobj
//	
//	Revision 1.3  2001/07/31 17:16:31  dj_jl
//	Just moved Log to the end of file
//	
//	Revision 1.2  2001/07/27 14:27:54  dj_jl
//	Update with Id-s and Log-s, some fixes
//
//**************************************************************************
