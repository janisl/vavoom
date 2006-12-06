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
//**	BSP traversal, handling of LineSegs for rendering.
//**
//**************************************************************************

// HEADER FILES ------------------------------------------------------------

#include "gamedefs.h"
#include "r_local.h"

// MACROS ------------------------------------------------------------------

// TYPES -------------------------------------------------------------------

struct VClipNode
{
	float		From;
	float		To;
	VClipNode*	Prev;
	VClipNode*	Next;
};

// EXTERNAL FUNCTION PROTOTYPES --------------------------------------------

// PUBLIC FUNCTION PROTOTYPES ----------------------------------------------

// PRIVATE FUNCTION PROTOTYPES ---------------------------------------------

// EXTERNAL DATA DECLARATIONS ----------------------------------------------

// PUBLIC DATA DEFINITIONS -------------------------------------------------

TVec				r_normal;
float				r_dist;

surface_t*			r_surface;

// PRIVATE DATA DEFINITIONS ------------------------------------------------

static int				r_frustum_indexes[4][6];

static subsector_t*		r_sub;
static sec_region_t*	r_region;

static bool				sky_is_visible;

static VClipNode*		FreeClipNodes;
static VClipNode*		ClipHead;
static VClipNode*		ClipTail;

// CODE --------------------------------------------------------------------

//==========================================================================
//
//	NewClipNode
//
//==========================================================================

static VClipNode* NewClipNode()
{
	VClipNode* Ret = FreeClipNodes;
	if (Ret)
	{
		FreeClipNodes = Ret->Next;
	}
	else
	{
		Ret = new VClipNode();
	}
	return Ret;
}

//==========================================================================
//
//	RemoveClipNode
//
//==========================================================================

static void RemoveClipNode(VClipNode* Node)
{
	if (Node->Next)
	{
		Node->Next->Prev = Node->Prev;
	}
	if (Node->Prev)
	{
		Node->Prev->Next = Node->Next;
	}
	if (Node == ClipHead)
	{
		ClipHead = Node->Next;
	}
	if (Node == ClipTail)
	{
		ClipTail = Node->Prev;
	}
	Node->Next = FreeClipNodes;
	FreeClipNodes = Node;
}

//==========================================================================
//
//	ClearClipNodes
//
//==========================================================================

static void ClearClipNodes()
{
	if (ClipHead)
	{
		ClipTail->Next = FreeClipNodes;
		FreeClipNodes = ClipHead;
	}
	ClipHead = NULL;
	ClipTail = NULL;
}

//==========================================================================
//
//	R_ClipShutdown
//
//==========================================================================

void R_ClipShutdown()
{
	ClearClipNodes();
	VClipNode* Node = FreeClipNodes;
	while (Node)
	{
		VClipNode* Next = Node->Next;
		delete Node;
		Node = Next;
	}
}

//==========================================================================
//
//	DoAddClipRange
//
//==========================================================================

static void DoAddClipRange(float From, float To)
{
	guard(DoAddClipRange);
	if (!ClipHead)
	{
		ClipHead = NewClipNode();
		ClipTail = ClipHead;
		ClipHead->From = From;
		ClipHead->To = To;
		ClipHead->Prev = NULL;
		ClipHead->Next = NULL;
		return;
	}

	for (VClipNode* Node = ClipHead; Node; Node = Node->Next)
	{
		if (Node->To < From)
		{
			//	Before this range.
			continue;
		}

		if (To < Node->From)
		{
			//	Insert a new clip range before current one.
			VClipNode* N = NewClipNode();
			N->From = From;
			N->To = To;
			N->Prev = Node->Prev;
			N->Next = Node;
			if (Node->Prev)
			{
				Node->Prev->Next = N;
			}
			else
			{
				ClipHead = N;
			}
			Node->Prev = N;
			return;
		}

		if (Node->From <= From && Node->To >= To)
		{
			//	It contains this range.
			return;
		}

		if (From < Node->From)
		{
			//	Extend start of the current range.
			Node->From = From;
		}
		if (To <= Node->To)
		{
			//	End is included, so we are done here.
			return;
		}

		//	Merge with following nodes if needed.
		while (Node->Next && Node->Next->From <= To)
		{
			Node->To = Node->Next->To;
			RemoveClipNode(Node->Next);
		}
		if (To > Node->To)
		{
			//	Extend end.
			Node->To = To;
		}
		//	We are done here.
		return;
	}

	//	If we are here it means it's a new range at the end.
	VClipNode* NewTail = NewClipNode();
	NewTail->From = From;
	NewTail->To = To;
	NewTail->Prev = ClipTail;
	NewTail->Next = NULL;
	ClipTail->Next = NewTail;
	ClipTail = NewTail;
	unguard;
}

//==========================================================================
//
//	AddClipRange
//
//==========================================================================

static void AddClipRange(float From, float To)
{
	guard(AddClipRange);
	if (From > To)
	{
		DoAddClipRange(0.0, To);
		DoAddClipRange(From, 360.0);
	}
	else
	{
		DoAddClipRange(From, To);
	}
	unguard;
}

//==========================================================================
//
//	DoIsRangeVisible
//
//==========================================================================

static bool DoIsRangeVisible(float From, float To)
{
	guard(DoIsRangeVisible);
	for (VClipNode* N = ClipHead; N; N = N->Next)
	{
		if (From >= N->From && To <= N->To)
		{
			return false;
		}
	}
	return true;
	unguard;
}

//==========================================================================
//
//	IsRangeVisible
//
//==========================================================================

static bool IsRangeVisible(float From, float To)
{
	guard(IsRangeVisible);
	if (From > To)
	{
		return DoIsRangeVisible(0.0, To) || DoIsRangeVisible(From, 360.0);
	}
	else
	{
		return DoIsRangeVisible(From, To);
	}
	unguard;
}

//==========================================================================
//
//	ClipIsFull
//
//==========================================================================

static bool ClipIsFull()
{
	guard(ClipIsFull);
	return ClipHead && ClipHead->From == 0.0 && ClipHead->To == 360.0;
	unguard;
}

//==========================================================================
//
//	PointToClipAngle
//
//==========================================================================

static float PointToClipAngle(const TVec& Pt)
{
	float Ret = matan(Pt.y - vieworg.y, Pt.x - vieworg.x);
	if (Ret < 0)
		Ret += 360.0;
	return Ret;
}

//==========================================================================
//
//	ClipIsBBoxVisible
//
//==========================================================================

static bool ClipIsBBoxVisible(float* BBox)
{
	guard(ClipIsBBoxVisible);
	if (!ClipHead)
	{
		//	No clip nodes yet.
		return true;
	}
	if (BBox[0] <= vieworg.x && BBox[3] >= vieworg.x &&
		BBox[1] <= vieworg.y && BBox[4] >= vieworg.y)
	{
		//	Viewer is inside the box.
		return true;
	}

	TVec v1;
	TVec v2;
	if (BBox[0] > vieworg.x)
	{
		if (BBox[1] > vieworg.y)
		{
			v1.x = BBox[3];
			v1.y = BBox[1];
			v2.x = BBox[0];
			v2.y = BBox[4];
		}
		else if (BBox[4] < vieworg.y)
		{
			v1.x = BBox[0];
			v1.y = BBox[1];
			v2.x = BBox[3];
			v2.y = BBox[4];
		}
		else
		{
			v1.x = BBox[0];
			v1.y = BBox[1];
			v2.x = BBox[0];
			v2.y = BBox[4];
		}
	}
	else if (BBox[3] < vieworg.x)
	{
		if (BBox[1] > vieworg.y)
		{
			v1.x = BBox[3];
			v1.y = BBox[4];
			v2.x = BBox[0];
			v2.y = BBox[1];
		}
		else if (BBox[4] < vieworg.y)
		{
			v1.x = BBox[0];
			v1.y = BBox[4];
			v2.x = BBox[3];
			v2.y = BBox[1];
		}
		else
		{
			v1.x = BBox[3];
			v1.y = BBox[4];
			v2.x = BBox[3];
			v2.y = BBox[1];
		}
	}
	else
	{
		if (BBox[1] > vieworg.y)
		{
			v1.x = BBox[3];
			v1.y = BBox[1];
			v2.x = BBox[0];
			v2.y = BBox[1];
		}
		else
		{
			v1.x = BBox[0];
			v1.y = BBox[4];
			v2.x = BBox[3];
			v2.y = BBox[4];
		}
	}
	return IsRangeVisible(PointToClipAngle(v1), PointToClipAngle(v2));
	unguard;
}

//==========================================================================
//
//	ClipAddSubsectorSegs
//
//==========================================================================

static void ClipAddSubsectorSegs(subsector_t* Sub)
{
	guard(ClipAddSubsectorSegs);
	for (int i = 0; i < Sub->numlines; i++)
	{
		seg_t* line = &GClLevel->Segs[Sub->firstline + i];
		if (line->backsector || !line->linedef)
		{
			//	Miniseg or two-sided line.
			continue;
		}

		float dist = DotProduct(vieworg, line->normal) - line->dist;
		if (dist <= 0)
		{
			//	Viewer is in back side or on plane
			continue;
		}

		AddClipRange(PointToClipAngle(*line->v2),
			PointToClipAngle(*line->v1));
	}
	unguard;
}

//==========================================================================
//
//	R_SetUpFrustumIndexes
//
//==========================================================================

static void R_SetUpFrustumIndexes()
{
	guard(R_SetUpFrustumIndexes);
	for (int i = 0; i < 4; i++)
	{
		int *pindex = r_frustum_indexes[i];
		for (int j = 0; j < 3; j++)
		{
			if (view_clipplanes[i].normal[j] < 0)
			{
				pindex[j] = j;
				pindex[j + 3] = j + 3;
			}
			else
			{
				pindex[j] = j + 3;
				pindex[j + 3] = j;
			}
		}
	}
	unguard;
}

//==========================================================================
//
//	DrawSurfaces
//
//==========================================================================

static void DrawSurfaces(surface_t* InSurfs, texinfo_t *texinfo, int clipflags,
	int LightSourceSector = -1)
{
	guard(DrawSurfaces);
	surface_t* surfs = InSurfs;
	if (!surfs)
	{
		return;
	}

	if (GTextureManager.Textures[texinfo->pic]->Type == TEXTYPE_Null)
	{
		return;
	}

	if (texinfo->pic == skyflatnum)
	{
		sky_is_visible = true;
		return;
	}

	sec_params_t* LightParams = LightSourceSector == -1 ? r_region->params :
		&GClLevel->Sectors[LightSourceSector].params;
	int lLev = fixedlight ? fixedlight :
			MIN(255, LightParams->lightlevel);
	if (r_darken)
	{
		lLev = light_remap[lLev];
	}
	do
	{
		surfs->Light = (lLev << 24) | LightParams->LightColour;
		surfs->dlightframe = r_sub->dlightframe;
		surfs->dlightbits = r_sub->dlightbits;

		r_surface = surfs;
		if (!texinfo->translucency)
		{
			Drawer->DrawPolygon(surfs->verts, surfs->count, texinfo->pic,
				clipflags);
		}
		else
		{
			R_DrawTranslucentPoly(surfs->verts, surfs->count, texinfo->pic,
				texinfo->translucency - 1, 0, false, 0);
		}
		surfs = surfs->next;
	} while (surfs);
	unguard;
}

//==========================================================================
//
//	RenderLine
//
// 	Clips the given segment and adds any visible pieces to the line list.
//
//==========================================================================

static void RenderLine(drawseg_t* dseg, int clipflags)
{
	guard(RenderLine);
	seg_t *line = dseg->seg;

	if (!line->linedef)
	{
		//	Miniseg
		return;
	}

	float dist = DotProduct(vieworg, line->normal) - line->dist;
	if (dist <= 0)
	{
		//	Viewer is in back side or on plane
		return;
	}

	float a1 = PointToClipAngle(*line->v2);
	float a2 = PointToClipAngle(*line->v1);
	if (!IsRangeVisible(a1, a2))
	{
		return;
	}

	line_t *linedef = line->linedef;

	r_normal = line->normal;
	r_dist = line->dist;

	//FIXME this marks all lines
	// mark the segment as visible for auto map
	linedef->flags |= ML_MAPPED;

	if (!line->backsector)
	{
		// single sided line
		DrawSurfaces(dseg->mid->surfs, &dseg->mid->texinfo, clipflags);
		DrawSurfaces(dseg->topsky->surfs, &dseg->topsky->texinfo, clipflags);
	}
	else
	{
		// two sided line
		DrawSurfaces(dseg->top->surfs, &dseg->top->texinfo, clipflags);
		DrawSurfaces(dseg->topsky->surfs, &dseg->topsky->texinfo, clipflags);
		DrawSurfaces(dseg->bot->surfs, &dseg->bot->texinfo, clipflags);
		DrawSurfaces(dseg->mid->surfs, &dseg->mid->texinfo, clipflags);
		for (segpart_t *sp = dseg->extra; sp; sp = sp->next)
		{
			DrawSurfaces(sp->surfs, &sp->texinfo, clipflags);
		}
	}
	unguard;
}

//==========================================================================
//
//	RenderSecSurface
//
//==========================================================================

static void	RenderSecSurface(sec_surface_t *ssurf, int clipflags)
{
	guard(RenderSecSurface);
	sec_plane_t &plane = *ssurf->secplane;

	if (!plane.pic)
	{
		return;
	}

	float dist = DotProduct(vieworg, plane.normal) - plane.dist;
	if (dist <= 0)
	{
		//	Viewer is in back side or on plane
		return;
	}

	r_normal = plane.normal;
	r_dist = plane.dist;

	DrawSurfaces(ssurf->surfs, &ssurf->texinfo, clipflags,
		plane.LightSourceSector);
	unguard;
}

//==========================================================================
//
//	RenderSubRegion
//
// 	Determine floor/ceiling planes.
// 	Draw one or more line segments.
//
//==========================================================================

static void RenderSubRegion(subregion_t *region, int clipflags)
{
	guard(RenderSubRegion);
	int				count;
	int 			polyCount;
	seg_t**			polySeg;
	float			d;

	d = DotProduct(vieworg, region->floor->secplane->normal) - region->floor->secplane->dist;
	if (region->next && d <= 0.0)
	{
		RenderSubRegion(region->next, clipflags);
	}

	r_region = region->secregion;

	if (r_sub->poly)
	{
		//	Render the polyobj in the subsector first
		polyCount = r_sub->poly->numsegs;
		polySeg = r_sub->poly->segs;
		while (polyCount--)
		{
			RenderLine((*polySeg)->drawsegs, clipflags);
			polySeg++;
		}
	}

	count = r_sub->numlines;
	drawseg_t *ds = region->lines;
	while (count--)
	{
		RenderLine(ds, clipflags);
		ds++;
	}

	RenderSecSurface(region->floor, clipflags);
	RenderSecSurface(region->ceil, clipflags);

	if (region->next && d > 0.0)
	{
		RenderSubRegion(region->next, clipflags);
	}
	unguard;
}

//==========================================================================
//
//	RenderSubsector
//
//==========================================================================

static void RenderSubsector(int num, int clipflags)
{
	guard(RenderSubsector);
	r_sub = &GClLevel->Subsectors[num];

	if (r_sub->VisFrame != r_visframecount)
	{
		return;
	}

	if (!r_sub->sector->linecount)
	{
		//	Skip sectors containing original polyobjs
		return;
	}

	RenderSubRegion(r_sub->regions, clipflags);

	ClipAddSubsectorSegs(r_sub);
	unguard;
}

//==========================================================================
//
//	RenderBSPNode
//
//	Renders all subsectors below a given node, traversing subtree
// recursively. Just call with BSP root.
//
//==========================================================================

static void RenderBSPNode(int bspnum, float *bbox, int InClipflags)
{
	guard(RenderBSPNode);
	if (ClipIsFull())
		return;
	int clipflags = InClipflags;
	// cull the clipping planes if not trivial accept
	if (clipflags)
	{
		for (int i = 0; i < 4; i++)
		{
			if (!(clipflags & view_clipplanes[i].clipflag))
				continue;	// don't need to clip against it

			// generate accept and reject points

			int *pindex = r_frustum_indexes[i];

			TVec rejectpt;

			rejectpt[0] = bbox[pindex[0]];
			rejectpt[1] = bbox[pindex[1]];
			rejectpt[2] = bbox[pindex[2]];

			float d;

			d = DotProduct(rejectpt, view_clipplanes[i].normal);
			d -= view_clipplanes[i].dist;

			if (d <= 0)
				return;

			TVec acceptpt;

			acceptpt[0] = bbox[pindex[3+0]];
			acceptpt[1] = bbox[pindex[3+1]];
			acceptpt[2] = bbox[pindex[3+2]];

			d = DotProduct(acceptpt, view_clipplanes[i].normal);
			d -= view_clipplanes[i].dist;

			if (d >= 0)
				clipflags ^= view_clipplanes[i].clipflag;	// node is entirely on screen
		}
	}

	if (!ClipIsBBoxVisible(bbox))
	{
		return;
	}

	// Found a subsector?
	if (bspnum & NF_SUBSECTOR)
	{
		if (bspnum == -1)
			RenderSubsector(0, clipflags);
		else
			RenderSubsector(bspnum & (~NF_SUBSECTOR), clipflags);
		return;
	}

	node_t* bsp = &GClLevel->Nodes[bspnum];

	if (bsp->VisFrame != r_visframecount)
	{
		return;
	}

	// Decide which side the view point is on.
	int side = bsp->PointOnSide(vieworg);

	// Recursively divide front space.
	RenderBSPNode(bsp->children[side], bsp->bbox[side], clipflags);

	// Divide back space.
	RenderBSPNode(bsp->children[side ^ 1], bsp->bbox[side ^ 1], clipflags);
	unguard;
}

//==========================================================================
//
//	R_RenderWorld
//
//==========================================================================

void R_RenderWorld()
{
	guard(R_RenderWorld);
	float	dummy_bbox[6] = {-99999, -99999, -99999, 99999, 9999, 99999};

	R_SetUpFrustumIndexes();
	ClearClipNodes();

	sky_is_visible = false;

	RenderBSPNode(GClLevel->NumNodes - 1, dummy_bbox, 15);	// head node is the last node output

	if (sky_is_visible)
	{
		R_DrawSky();
	}

	Drawer->WorldDrawing();
	unguard;
}
