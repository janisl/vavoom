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
#include "cl_local.h"

// MACROS ------------------------------------------------------------------

//FIXME: Read these from a script or set them from PROGS
#define PO_LINE_START 		1 // polyobj line start special
#define PO_LINE_EXPLICIT	5

#define PO_MAXPOLYSEGS 		64

// TYPES -------------------------------------------------------------------

// EXTERNAL FUNCTION PROTOTYPES --------------------------------------------

// PRIVATE FUNCTION PROTOTYPES ---------------------------------------------

// EXTERNAL DATA DECLARATIONS ----------------------------------------------

// PUBLIC DATA DEFINITIONS -------------------------------------------------

// PRIVATE DATA DEFINITIONS ------------------------------------------------

static int 		PolySegCount;
static float 	PolyStartX;
static float 	PolyStartY;

// CODE --------------------------------------------------------------------

//==========================================================================
//
//	UpdatePolySegs
//
//==========================================================================

static void	UpdatePolySegs(polyobj_t *po)
{
	seg_t **segList;
	int count;

	for (count = po->numsegs, segList = po->segs; count; count--, segList++)
	{
		//	Recalc lines's slope type, bounding box, normal and dist
		CalcLine((*segList)->linedef);
		//	Recalc seg's normal and dist
		CalcSeg(*segList);
		R_SegMoved(*segList);
	}
}

//==========================================================================
//
//	MovePolyobj
//
//==========================================================================

static void MovePolyobj(int num, float x, float y)
{
	int count;
	seg_t **segList;
	seg_t **veryTempSeg;
	polyobj_t *po;

	po = &cl_level.polyobjs[num];
	x -= po->startSpot.x;
	y -= po->startSpot.y;
	if (!x && !y)
		return;

	segList = po->segs;

	for (count = po->numsegs; count; count--, segList++)
	{
		for (veryTempSeg = po->segs; veryTempSeg != segList; veryTempSeg++)
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
	}
	po->startSpot.x += x;
	po->startSpot.y += y;
	UpdatePolySegs(po);
}

//==========================================================================
//
//	RotatePolyobj
//
//==========================================================================

static void RotatePolyobj(int num, float angle)
{
	// Get a pointer to the polyobject.
	polyobj_t *po = &cl_level.polyobjs[num];

	// If the angle is the same then we don't need to rotate.
	if (po->angle == angle)
	{
		return;
	}

	// Calculate the rotation sin and cos scalars.
	float msinAngle = msin(angle);
	float mcosAngle = mcos(angle);

	// Loop for the number of segments in the polyobject.
	seg_t **segList = po->segs;
	vertex_t *originalPts = po->originalPts;
	for (int count = po->numsegs; count; count--, segList++, originalPts++)
	{
		// Save off originalPts x and y.
		float tr_x = originalPts->x;
		float tr_y = originalPts->y;

		// Calculate the rotated x and y into (*segList)->v1.
		(*segList)->v1->x = tr_x * mcosAngle - tr_y * msinAngle;
		(*segList)->v1->y = tr_x * msinAngle + tr_y * mcosAngle;

		// Translate (*segList)->v1 to the startSpot
		*(*segList)->v1 += po->startSpot;
	}

	// Update the angle and segments.
	po->angle = angle;
	UpdatePolySegs(po);
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
	for (i = 0; i < cl_level.numsegs; i++)
	{
		if (!cl_level.segs[i].linedef)
			continue;
		if (cl_level.segs[i].v1->x == x && cl_level.segs[i].v1->y == y)
		{
			if (!segList)
			{
				PolySegCount++;
			}
			else
			{
				*segList++ = &cl_level.segs[i];
				cl_level.segs[i].frontsector->linecount = 0;
			}
			IterFindPolySegs(cl_level.segs[i].v2->x, cl_level.segs[i].v2->y, segList);
			return;
		}
	}
	Sys_Error("IterFindPolySegs:  Non-closed Polyobj located.\n");
}


//==========================================================================
//
//	CL_PO_SpawnPolyobj
//
//==========================================================================

void CL_PO_SpawnPolyobj(float x, float y, int tag)
{
	int i;
	int j;
	int psIndex;
	int psIndexOld;
	seg_t *polySegList[PO_MAXPOLYSEGS];
    int index;

	index = cl_level.numpolyobjs++;
	if (cl_level.numpolyobjs == 1)
    {
		cl_level.polyobjs = (polyobj_t*)Z_Malloc(sizeof(polyobj_t), PU_LEVEL, 0);
	}
    else
    {
    	Z_Resize((void**)&cl_level.polyobjs, cl_level.numpolyobjs * sizeof(polyobj_t));
    }
	memset(&cl_level.polyobjs[index], 0, sizeof(polyobj_t));

	cl_level.polyobjs[index].startSpot.x = x;
	cl_level.polyobjs[index].startSpot.y = y;
	for(i = 0; i < cl_level.numsegs; i++)
	{
		if (!cl_level.segs[i].linedef)
			continue;
		if (cl_level.segs[i].linedef->special == PO_LINE_START &&
			cl_level.segs[i].linedef->arg1 == tag)
		{
			if (cl_level.polyobjs[index].segs)
			{
            	//	Immpossible, because it is just cleared out
				Sys_Error("CL_PO_SpawnPolyobj:  Polyobj %d already spawned.\n", tag);
			}
			cl_level.segs[i].linedef->special = 0;
			cl_level.segs[i].linedef->arg1 = 0;
			PolySegCount = 1;
			PolyStartX = cl_level.segs[i].v1->x;
			PolyStartY = cl_level.segs[i].v1->y;
			IterFindPolySegs(cl_level.segs[i].v2->x, cl_level.segs[i].v2->y, NULL);

			cl_level.polyobjs[index].numsegs = PolySegCount;
			cl_level.polyobjs[index].segs = (seg_t**)Z_Malloc(PolySegCount*sizeof(seg_t *),
				PU_LEVEL, 0);
			*(cl_level.polyobjs[index].segs) = &cl_level.segs[i]; // insert the first seg
			cl_level.segs[i].frontsector->linecount = 0;
			IterFindPolySegs(cl_level.segs[i].v2->x, cl_level.segs[i].v2->y,
				cl_level.polyobjs[index].segs + 1);
			cl_level.polyobjs[index].tag = tag;
			cl_level.polyobjs[index].seqType = cl_level.segs[i].linedef->arg3;
			break;
		}
	}
	if (!cl_level.polyobjs[index].segs)
	{
		// didn't find a polyobj through PO_LINE_START
		psIndex = 0;
		cl_level.polyobjs[index].numsegs = 0;
		for(j = 1; j < PO_MAXPOLYSEGS; j++)
		{
			psIndexOld = psIndex;
			for (i = 0; i < cl_level.numsegs; i++)
			{
				if (!cl_level.segs[i].linedef)
					continue;
				if (cl_level.segs[i].linedef->special == PO_LINE_EXPLICIT &&
					cl_level.segs[i].linedef->arg1 == tag)
				{
					if(!cl_level.segs[i].linedef->arg2)
					{
						Sys_Error("CL_PO_SpawnPolyobj:  Explicit line missing order number (probably %d) in poly %d.\n",
							j+1, tag);
					}
					if(cl_level.segs[i].linedef->arg2 == j)
					{
						polySegList[psIndex] = &cl_level.segs[i];
						cl_level.segs[i].frontsector->linecount = 0;
						cl_level.polyobjs[index].numsegs++;
						psIndex++;
						if(psIndex > PO_MAXPOLYSEGS)
						{
							Sys_Error("CL_PO_SpawnPolyobj:  psIndex > PO_MAXPOLYSEGS\n");
						}
					}
				}
			}
			// Clear out any specials for these segs...we cannot clear them out
			// 	in the above loop, since we aren't guaranteed one seg per
			//		linedef.
			for (i = 0; i < cl_level.numsegs; i++)
			{
				if (!cl_level.segs[i].linedef)
					continue;
				if (cl_level.segs[i].linedef->special == PO_LINE_EXPLICIT &&
					cl_level.segs[i].linedef->arg1 == tag && cl_level.segs[i].linedef->arg2 == j)
				{
					cl_level.segs[i].linedef->special = 0;
					cl_level.segs[i].linedef->arg1 = 0;
				}
			}
			if(psIndex == psIndexOld)
			{ // Check if an explicit line order has been skipped
				// A line has been skipped if there are any more explicit
				// lines with the current tag value
				for (i = 0; i < cl_level.numsegs; i++)
				{
					if (!cl_level.segs[i].linedef)
						continue;
					if (cl_level.segs[i].linedef->special == PO_LINE_EXPLICIT &&
						cl_level.segs[i].linedef->arg1 == tag)
					{
						Sys_Error("CL_PO_SpawnPolyobj:  Missing explicit line %d for poly %d\n",
							j, tag);
					}
				}
			}
		}
		if (cl_level.polyobjs[index].numsegs)
		{
			PolySegCount = cl_level.polyobjs[index].numsegs; // PolySegCount used globally
			cl_level.polyobjs[index].tag = tag;
			cl_level.polyobjs[index].segs = (seg_t**)Z_Malloc(cl_level.polyobjs[index].numsegs
				* sizeof(seg_t *), PU_LEVEL, 0);
			for(i = 0; i < cl_level.polyobjs[index].numsegs; i++)
			{
				cl_level.polyobjs[index].segs[i] = polySegList[i];
			}
			cl_level.polyobjs[index].seqType = (*cl_level.polyobjs[index].segs)->linedef->arg4;
		}
		// Next, change the polyobjs first line to point to a mirror
		//		if it exists
		(*cl_level.polyobjs[index].segs)->linedef->arg2 =
			(*cl_level.polyobjs[index].segs)->linedef->arg3;
	}
}

//==========================================================================
//
//	CL_PO_TranslateToStartSpot
//
//==========================================================================

void CL_PO_TranslateToStartSpot(float originX, float originY, int tag)
{
	seg_t **tempSeg;
	seg_t **veryTempSeg;
	vertex_t *tempPt;
	subsector_t *sub;
	polyobj_t *po;
	float	deltaX;
	float	deltaY;
	vertex_t avg; // used to find a polyobj's center, and hence subsector
	int i;

	po = NULL;
	for (i = 0; i < cl_level.numpolyobjs; i++)
	{
		if (cl_level.polyobjs[i].tag == tag)
		{
			po = &cl_level.polyobjs[i];
			break;
		}
	}
	if (!po)
	{ // didn't match the tag with a polyobj tag
		Sys_Error("TranslateToStartSpot:  Unable to match polyobj tag: %d\n",
			tag);
	}
	if (po->segs == NULL)
	{
		Sys_Error("TranslateToStartSpot:  Anchor point located without a StartSpot point: %d\n", tag);
	}
	po->originalPts = (vertex_t*)Z_Malloc(po->numsegs*sizeof(vertex_t), PU_LEVEL, 0);
	deltaX = originX - po->startSpot.x;
	deltaY = originY - po->startSpot.y;

	tempSeg = po->segs;
	tempPt = po->originalPts;
	avg.x = 0;
	avg.y = 0;

	for (i = 0; i < po->numsegs; i++, tempSeg++, tempPt++)
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
		*tempPt = *(*tempSeg)->v1 - po->startSpot;
	}
	avg.x /= po->numsegs;
	avg.y /= po->numsegs;
	sub = CL_PointInSubsector(avg.x, avg.y);
	if (sub->poly != NULL)
	{
		Sys_Error("CL_PO_TranslateToStartSpot:  Multiple polyobjs in a single subsector.\n");
	}
	sub->poly = po;
	UpdatePolySegs(po);
}

//==========================================================================
//
//	CL_PO_Update
//
//==========================================================================

void CL_PO_Update(int i, float x, float y, float angle)
{
	if (!cl_level.numpolyobjs)
		return;

	RotatePolyobj(i, angle);
	MovePolyobj(i, x, y);
}

//**************************************************************************
//
//	$Log$
//	Revision 1.8  2002/01/15 18:30:43  dj_jl
//	Some fixes and improvements suggested by Malcolm Nixon
//
//	Revision 1.7  2002/01/07 12:16:41  dj_jl
//	Changed copyright year
//	
//	Revision 1.6  2001/10/22 17:25:55  dj_jl
//	Floatification of angles
//	
//	Revision 1.5  2001/10/18 17:36:31  dj_jl
//	A lots of changes for Alpha 2
//	
//	Revision 1.4  2001/09/20 16:27:02  dj_jl
//	Removed degenmobj
//	
//	Revision 1.3  2001/07/31 17:16:30  dj_jl
//	Just moved Log to the end of file
//	
//	Revision 1.2  2001/07/27 14:27:54  dj_jl
//	Update with Id-s and Log-s, some fixes
//
//**************************************************************************
