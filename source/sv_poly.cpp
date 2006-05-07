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

//FIXME: Read these from a script or set them from PROGS
#define PO_LINE_START 		1 // polyobj line start special
#define PO_LINE_EXPLICIT	5

#define PO_MAXPOLYSEGS 		64

// TYPES -------------------------------------------------------------------

// EXTERNAL FUNCTION PROTOTYPES --------------------------------------------

// PRIVATE FUNCTION PROTOTYPES ---------------------------------------------

static void ThrustMobj(VEntity *mobj, seg_t *seg, polyobj_t *po);
static void UnLinkPolyobj(polyobj_t *po);
static void LinkPolyobj(polyobj_t *po);
static boolean CheckMobjBlocking(seg_t *seg, polyobj_t *po);
static void InitBlockMap();

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
	guard(PO_GetPolyobj);
	int i;

	for (i = 0; i < GLevel->NumPolyObjs; i++)
	{
		if (GLevel->PolyObjs[i].tag == polyNum)
		{
			return &GLevel->PolyObjs[i];
		}
	}
	return NULL;
	unguard;
}

//==========================================================================
//
// PO_GetPolyobjMirror
//
//==========================================================================

int PO_GetPolyobjMirror(int poly)
{
	guard(PO_GetPolyobjMirror);
	int i;

	for(i = 0; i < GLevel->NumPolyObjs; i++)
	{
		if (GLevel->PolyObjs[i].tag == poly)
		{
			return((*GLevel->PolyObjs[i].segs)->linedef->arg2);
		}
	}
	return 0;
	unguard;
}

//==========================================================================
//
//	ThrustMobj
//
//==========================================================================

static void ThrustMobj(VEntity *mobj, seg_t *seg, polyobj_t *po)
{
	GLevelInfo->eventPolyThrustMobj(mobj, seg->normal, po);
}

//==========================================================================
//
//	UpdatePolySegs
//
//==========================================================================

static void	UpdatePolySegs(polyobj_t *po)
{
	guard(UpdatePolySegs);
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
	unguard;
}

//==========================================================================
//
// PO_MovePolyobj
//
//==========================================================================

boolean PO_MovePolyobj(int num, float x, float y)
{
	guard(PO_MovePolyobj);
	int count;
	seg_t **segList;
	seg_t **veryTempSeg;
	polyobj_t *po;
	vertex_t *prevPts;
	boolean blocked;

	po = PO_GetPolyobj(num);
	if (!po)
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
	unguard;
}

//==========================================================================
//
// PO_RotatePolyobj
//
//==========================================================================

boolean PO_RotatePolyobj(int num, float angle)
{
	guard(PO_RotatePolyobj);
	int count;

	// Get the polyobject.
	polyobj_t *po = PO_GetPolyobj(num);
	if (!po)
	{
		Sys_Error("PO_RotatePolyobj:  Invalid polyobj number: %d\n", num);
	}

	// Calculate the angle.
	float an = po->angle + angle;
	float msinAn = msin(an);
	float mcosAn = mcos(an);

	UnLinkPolyobj(po);

	seg_t **segList = po->segs;
	vertex_t *originalPts = po->originalPts;
	vertex_t *prevPts = po->prevPts;

	for (count = po->numsegs; count; count--, segList++, originalPts++,
		prevPts++)
	{
		// Save the previous points.
		prevPts->x = (*segList)->v1->x;
		prevPts->y = (*segList)->v1->y;

		// Get the original X and Y values.
		float tr_x = originalPts->x;
		float tr_y = originalPts->y;

		// Calculate the new X and Y values.
		(*segList)->v1->x = (tr_x * mcosAn - tr_y * msinAn) + po->startSpot.x;
		(*segList)->v1->y = (tr_y * mcosAn + tr_x * msinAn) + po->startSpot.y;
	}
	UpdatePolySegs(po);
	segList = po->segs;
	bool blocked = false;
	for (count = po->numsegs; count; count--, segList++)
	{
		if (CheckMobjBlocking(*segList, po))
		{
			blocked = true;
		}
	}

	// If we are blocked then restore the previous points.
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
	unguard;
}

//==========================================================================
//
// UnLinkPolyobj
//
//==========================================================================

static void UnLinkPolyobj(polyobj_t *po)
{
	guard(UnLinkPolyobj);
	polyblock_t *link;
	int i, j;
	int index;

	// remove the polyobj from each blockmap section
	for (j = po->bbox[BOXBOTTOM]; j <= po->bbox[BOXTOP]; j++)
	{
		index = j * GLevel->BlockMapWidth;
		for (i = po->bbox[BOXLEFT]; i <= po->bbox[BOXRIGHT]; i++)
		{
			if (i >= 0 && i < GLevel->BlockMapWidth &&
				j >= 0 && j < GLevel->BlockMapHeight)
			{
				link = GLevel->PolyBlockMap[index + i];
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
	unguard;
}

//==========================================================================
//
// LinkPolyobj
//
//==========================================================================

static void LinkPolyobj(polyobj_t *po)
{
	guard(LinkPolyobj);
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
	po->bbox[BOXRIGHT] = MapBlock(rightX - GLevel->BlockMapOrgX);
	po->bbox[BOXLEFT] = MapBlock(leftX - GLevel->BlockMapOrgX);
	po->bbox[BOXTOP] = MapBlock(topY - GLevel->BlockMapOrgY);
	po->bbox[BOXBOTTOM] = MapBlock(bottomY - GLevel->BlockMapOrgY);
	// add the polyobj to each blockmap section
	for (j = po->bbox[BOXBOTTOM] * GLevel->BlockMapWidth;
		j <= po->bbox[BOXTOP] * GLevel->BlockMapWidth;
		j += GLevel->BlockMapWidth)
	{
		for (i = po->bbox[BOXLEFT]; i <= po->bbox[BOXRIGHT]; i++)
		{
			if (i >= 0 && i < GLevel->BlockMapWidth &&
				j >= 0 && j < GLevel->BlockMapHeight * GLevel->BlockMapWidth)
			{
				link = &GLevel->PolyBlockMap[j + i];
				if (!(*link))
				{ 
					// Create a new link at the current block cell
					*link = (polyblock_t*)Z_Malloc(sizeof(polyblock_t), PU_LEVEL, 0);
					(*link)->next = NULL;
					(*link)->prev = NULL;
					(*link)->polyobj = po;
					continue;
				}
				else
				{
					tempLink = *link;
					while (tempLink->next != NULL && tempLink->polyobj != NULL)
					{
						tempLink = tempLink->next;
					}
				}
				if (tempLink->polyobj == NULL)
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
	unguard;
}

//==========================================================================
//
// CheckMobjBlocking
//
//==========================================================================

static boolean CheckMobjBlocking(seg_t *seg, polyobj_t *po)
{
	guard(CheckMobjBlocking);
	VEntity *mobj;
	int i, j;
	int left, right, top, bottom;
	float	tmbbox[4];
	line_t *ld;
	boolean blocked;

	ld = seg->linedef;

	top = MapBlock(ld->bbox[BOXTOP] - GLevel->BlockMapOrgY + MAXRADIUS);
	bottom = MapBlock(ld->bbox[BOXBOTTOM] - GLevel->BlockMapOrgY - MAXRADIUS);
	left = MapBlock(ld->bbox[BOXLEFT] - GLevel->BlockMapOrgX - MAXRADIUS);
	right = MapBlock(ld->bbox[BOXRIGHT] - GLevel->BlockMapOrgX + MAXRADIUS);

	blocked = false;

	bottom = bottom < 0 ? 0 : bottom;
	bottom = bottom >= GLevel->BlockMapHeight ? GLevel->BlockMapHeight - 1 : bottom;
	top = top < 0 ? 0 : top;
	top = top >= GLevel->BlockMapHeight  ? GLevel->BlockMapHeight - 1 : top;
	left = left < 0 ? 0 : left;
	left = left >= GLevel->BlockMapWidth ? GLevel->BlockMapWidth - 1 : left;
	right = right < 0 ? 0 : right;
	right = right >= GLevel->BlockMapWidth ?  GLevel->BlockMapWidth - 1 : right;

	for (j = bottom * GLevel->BlockMapWidth; j <= top * GLevel->BlockMapWidth;
		j += GLevel->BlockMapWidth)
	{
		for (i = left; i <= right; i++)
		{
			for (mobj = GLevel->BlockLinks[j + i]; mobj; mobj = mobj->BlockMapNext)
			{
				if (mobj->EntityFlags & VEntity::EF_Solid || mobj->EntityFlags & VEntity::EF_IsPlayer)
				{
					tmbbox[BOXTOP] = mobj->Origin.y + mobj->Radius;
					tmbbox[BOXBOTTOM] = mobj->Origin.y - mobj->Radius;
					tmbbox[BOXLEFT] = mobj->Origin.x - mobj->Radius;
					tmbbox[BOXRIGHT] = mobj->Origin.x + mobj->Radius;

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
	unguard;
}

//==========================================================================
//
// InitBlockMap
//
//==========================================================================

static void InitBlockMap()
{
	guard(InitBlockMap);
	int		i;

	GLevel->PolyBlockMap = Z_CNew(polyblock_t*, GLevel->BlockMapWidth *
		GLevel->BlockMapHeight, PU_LEVEL, 0);

	for (i = 0; i < GLevel->NumPolyObjs; i++)
	{
		LinkPolyobj(&GLevel->PolyObjs[i]);
	}
	unguard;
}

//==========================================================================
//
//	IterFindPolySegs
//
//	Passing NULL for segList will cause IterFindPolySegs to count the number
// of segs in the polyobj
//
//==========================================================================

static void IterFindPolySegs(TVec From, seg_t **segList)
{
	int i;

	if (From.x == PolyStartX && From.y == PolyStartY)
	{
		return;
	}
	for (i = 0; i < GLevel->NumSegs; i++)
	{
		if (!GLevel->Segs[i].linedef)
			continue;
		if (*GLevel->Segs[i].v1 == From)
		{
			if (!segList)
			{
				PolySegCount++;
			}
			else
			{
				*segList++ = &GLevel->Segs[i];
			}
			IterFindPolySegs(*GLevel->Segs[i].v2, segList);
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
	guard(PO_SpawnPolyobj);
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

	index = GLevel->NumPolyObjs++;
	if (GLevel->NumPolyObjs == 1)
    {
		GLevel->PolyObjs = (polyobj_t*)Z_Malloc(sizeof(polyobj_t), PU_LEVEL, 0);
	}
    else
    {
    	Z_Resize((void**)&GLevel->PolyObjs, GLevel->NumPolyObjs * sizeof(polyobj_t));
    }
	memset(&GLevel->PolyObjs[index], 0, sizeof(polyobj_t));

	GLevel->PolyObjs[index].startSpot.x = x;
	GLevel->PolyObjs[index].startSpot.y = y;
	for (i = 0; i < GLevel->NumSegs; i++)
	{
		if (!GLevel->Segs[i].linedef)
			continue;
		if (GLevel->Segs[i].linedef->special == PO_LINE_START &&
			GLevel->Segs[i].linedef->arg1 == tag)
		{
			if (GLevel->PolyObjs[index].segs)
			{
            	//	Immpossible, because it is just cleared out
				Sys_Error("PO_SpawnPolyobj:  Polyobj %d already spawned.\n", tag);
			}
			GLevel->Segs[i].linedef->special = 0;
			GLevel->Segs[i].linedef->arg1 = 0;
			PolySegCount = 1;
			PolyStartX = GLevel->Segs[i].v1->x;
			PolyStartY = GLevel->Segs[i].v1->y;
			IterFindPolySegs(*GLevel->Segs[i].v2, NULL);

			GLevel->PolyObjs[index].numsegs = PolySegCount;
			GLevel->PolyObjs[index].segs = (seg_t**)Z_Malloc(PolySegCount*sizeof(seg_t *),
				PU_LEVEL, 0);
			*(GLevel->PolyObjs[index].segs) = &GLevel->Segs[i]; // insert the first seg
			IterFindPolySegs(*GLevel->Segs[i].v2, GLevel->PolyObjs[index].segs + 1);
			if (crush)
				GLevel->PolyObjs[index].PolyFlags |= polyobj_t::PF_Crush;
			else
				GLevel->PolyObjs[index].PolyFlags &= ~polyobj_t::PF_Crush;
			GLevel->PolyObjs[index].tag = tag;
			GLevel->PolyObjs[index].seqType = GLevel->Segs[i].linedef->arg3;
//			if (GLevel->PolyObjs[index].seqType < 0 ||
//				GLevel->PolyObjs[index].seqType >= SEQTYPE_NUMSEQ)
//			{
//				GLevel->PolyObjs[index].seqType = 0;
//			}
			break;
		}
	}
	if (!GLevel->PolyObjs[index].segs)
	{
		// didn't find a polyobj through PO_LINE_START
		psIndex = 0;
		GLevel->PolyObjs[index].numsegs = 0;
		for(j = 1; j < PO_MAXPOLYSEGS; j++)
		{
			psIndexOld = psIndex;
			for (i = 0; i < GLevel->NumSegs; i++)
			{
				if (!GLevel->Segs[i].linedef)
					continue;
				if (GLevel->Segs[i].linedef->special == PO_LINE_EXPLICIT &&
					GLevel->Segs[i].linedef->arg1 == tag)
				{
					if (!GLevel->Segs[i].linedef->arg2)
					{
						Sys_Error("PO_SpawnPolyobj:  Explicit line missing order number (probably %d) in poly %d.\n",
							j+1, tag);
					}
					if (GLevel->Segs[i].linedef->arg2 == j)
					{
						polySegList[psIndex] = &GLevel->Segs[i];
						GLevel->PolyObjs[index].numsegs++;
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
			for (i = 0; i < GLevel->NumSegs; i++)
			{
				if (!GLevel->Segs[i].linedef)
					continue;
				if (GLevel->Segs[i].linedef->special == PO_LINE_EXPLICIT &&
					GLevel->Segs[i].linedef->arg1 == tag &&
					GLevel->Segs[i].linedef->arg2 == j)
				{
					GLevel->Segs[i].linedef->special = 0;
					GLevel->Segs[i].linedef->arg1 = 0;
				}
			}
			if(psIndex == psIndexOld)
			{ // Check if an explicit line order has been skipped
				// A line has been skipped if there are any more explicit
				// lines with the current tag value
				for (i = 0; i < GLevel->NumSegs; i++)
				{
					if (!GLevel->Segs[i].linedef)
						continue;
					if (GLevel->Segs[i].linedef->special == PO_LINE_EXPLICIT &&
						GLevel->Segs[i].linedef->arg1 == tag)
					{
						Sys_Error("PO_SpawnPolyobj:  Missing explicit line %d for poly %d\n",
							j, tag);
					}
				}
			}
		}
		if (GLevel->PolyObjs[index].numsegs)
		{
			PolySegCount = GLevel->PolyObjs[index].numsegs; // PolySegCount used globally
			if (crush)
				GLevel->PolyObjs[index].PolyFlags |= polyobj_t::PF_Crush;
			else
				GLevel->PolyObjs[index].PolyFlags &= ~polyobj_t::PF_Crush;
			GLevel->PolyObjs[index].tag = tag;
			GLevel->PolyObjs[index].segs = (seg_t**)Z_Malloc(GLevel->PolyObjs[index].numsegs
				* sizeof(seg_t *), PU_LEVEL, 0);
			for(i = 0; i < GLevel->PolyObjs[index].numsegs; i++)
			{
				GLevel->PolyObjs[index].segs[i] = polySegList[i];
			}
			GLevel->PolyObjs[index].seqType = (*GLevel->PolyObjs[index].segs)->linedef->arg4;
		}
		// Next, change the polyobjs first line to point to a mirror
		//		if it exists
		(*GLevel->PolyObjs[index].segs)->linedef->arg2 =
			(*GLevel->PolyObjs[index].segs)->linedef->arg3;
	}
	unguard;
}

//==========================================================================
//
//	PO_AddAnchorPoint
//
//==========================================================================

void PO_AddAnchorPoint(float x, float y, int tag)
{
	guard(PO_AddAnchorPoint);
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
	unguard;
}

//==========================================================================
//
//	TranslateToStartSpot
//
//==========================================================================

static void TranslateToStartSpot(float originX, float originY, int tag)
{
	guard(TranslateToStartSpot);
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
	for(i = 0; i < GLevel->NumPolyObjs; i++)
	{
		if (GLevel->PolyObjs[i].tag == tag)
		{
			po = &GLevel->PolyObjs[i];
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
	unguard;
}

//==========================================================================
//
//	PO_Init
//
//==========================================================================

void PO_Init()
{
	guard(PO_Init);
	int				i;

	for (i = 0; i < NumAnchorPoints; i++)
	{
		TranslateToStartSpot(AnchorPoints[i].x, AnchorPoints[i].y,
			AnchorPoints[i].tag);
	}

	NumAnchorPoints = 0;

	// check for a startspot without an anchor point
	for (i = 0; i < GLevel->NumPolyObjs; i++)
	{
		if (!GLevel->PolyObjs[i].originalPts)
		{
			Sys_Error("PO_Init:  StartSpot located without an Anchor point: %d\n",
				GLevel->PolyObjs[i].tag);
		}
	}
	InitBlockMap();
	unguard;
}

//==========================================================================
//
// PO_Busy
//
//==========================================================================

boolean PO_Busy(int polyobj)
{
	guard(PO_Busy);
	return GLevelInfo->eventPolyBusy(polyobj);
	unguard;
}

//==========================================================================
//
//	PO_FreePolyobjData
//
//==========================================================================

void PO_FreePolyobjData()
{
	guard(PO_FreePolyobjData);
	if (AnchorPoints)
	{
		Z_Free(AnchorPoints);
	}
	AnchorPoints = NULL;
	unguard;
}
