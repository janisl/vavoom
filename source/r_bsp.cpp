//**************************************************************************
//**
//**	##   ##    ##    ##   ##   ####     ####   ###     ###
//**	##   ##  ##  ##  ##   ##  ##  ##   ##  ##  ####   ####
//**	 ## ##  ##    ##  ## ##  ##    ## ##    ## ## ## ## ##
//**	 ## ##  ########  ## ##  ##    ## ##    ## ##  ###  ##
//**	  ###   ##    ##   ###    ##  ##   ##  ##  ##       ##
//**	   #    ##    ##    #      ####     ####   ##       ##
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
//**	BSP traversal, handling of LineSegs for rendering.
//**
//**************************************************************************

// HEADER FILES ------------------------------------------------------------

#include "gamedefs.h"
#include "r_local.h"

// MACROS ------------------------------------------------------------------

// TYPES -------------------------------------------------------------------

// EXTERNAL FUNCTION PROTOTYPES --------------------------------------------

// PUBLIC FUNCTION PROTOTYPES ----------------------------------------------

// PRIVATE FUNCTION PROTOTYPES ---------------------------------------------

// EXTERNAL DATA DECLARATIONS ----------------------------------------------

// PUBLIC DATA DEFINITIONS -------------------------------------------------

TVec				r_normal;
float				r_dist;

TVec				r_saxis;
TVec				r_taxis;
TVec				r_texorg;

surface_t			*r_surface;

// PRIVATE DATA DEFINITIONS ------------------------------------------------

static int			r_frustum_indexes[4][6];

static subsector_t	*r_sub;
static sec_region_t	*r_region;

static bool			sky_is_visible;

// CODE --------------------------------------------------------------------

//==========================================================================
//
//	R_SetUpFrustumIndexes
//
//==========================================================================

static void R_SetUpFrustumIndexes(void)
{
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
}

//==========================================================================
//
//	DrawSurfaces
//
//==========================================================================

static void DrawSurfaces(surface_t *surfs, texinfo_t *texinfo, int clipflags)
{
	if (!surfs)
	{
		return;
	}

	if (texinfo->pic == skyflatnum)
	{
		sky_is_visible = true;
		return;
	}

	r_saxis = texinfo->saxis;
	r_taxis = texinfo->taxis;
	r_texorg = texinfo->texorg;

	do
	{
		if (fixedlight)
		{
			surfs->lightlevel = fixedlight;
		}
		else
		{
			surfs->lightlevel = MIN(255, r_region->params->lightlevel);
		}
		if (r_darken)
		{
			surfs->lightlevel = light_remap[surfs->lightlevel];
		}
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
}

//==========================================================================
//
//	RenderSecSurface
//
//==========================================================================

static void	RenderSecSurface(sec_surface_t *ssurf, int clipflags)
{
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

	DrawSurfaces(ssurf->surfs, &ssurf->texinfo, clipflags);
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
    int				count;
    seg_t*			line;
	int 			polyCount;
	seg_t**			polySeg;
	float			d;

	d = DotProduct(vieworg, region->floor->secplane->normal) - region->floor->secplane->dist;
	if (region->next && ((r_back2front && d > 0.0) || (!r_back2front && d <= 0.0)))
	{
		RenderSubRegion(region->next, clipflags);
	}

	r_region = region->secregion;

	if (r_sub->poly && !r_back2front)
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

    line = &cl_level.segs[r_sub->firstline];

	RenderSecSurface(region->floor, clipflags);
	RenderSecSurface(region->ceil, clipflags);

	if (r_sub->poly && r_back2front)
	{
		//	Render the polyobj
		polyCount = r_sub->poly->numsegs;
		polySeg = r_sub->poly->segs;
		while (polyCount--)
		{
			RenderLine((*polySeg)->drawsegs, clipflags);
			polySeg++;
		}
	}

	if (region->next && ((r_back2front && d <= 0.0) || (!r_back2front && d > 0.0)))
	{
		RenderSubRegion(region->next, clipflags);
	}
}

//==========================================================================
//
//	RenderSubsector
//
//==========================================================================

static void RenderSubsector(int num, int clipflags)
{
//FIXME do this in node loading
#ifdef PARANOID
    if (num >= cl_level.numsubsectors)
		I_Error("RenderSubsector: ss %i with numss = %i", num, cl_level.numsubsectors);
#endif

    r_sub = &cl_level.subsectors[num];

	if (r_sub->visframe != r_visframecount)
 	{
		return;
	}

	if (!r_sub->sector->linecount)
	{
		//	Skip sectors containing original polyobjs
		return;
	}

	RenderSubRegion(r_sub->regions, clipflags);
}

//==========================================================================
//
//	RenderBSPNode
//
// 	Renders all subsectors below a given node, traversing subtree
// recursively. Just call with BSP root.
//
//==========================================================================

static void RenderBSPNode(int bspnum, float *bbox, int clipflags)
{
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

    // Found a subsector?
    if (bspnum & NF_SUBSECTOR)
    {
		if (bspnum == -1)
		    RenderSubsector(0, clipflags);
		else
		    RenderSubsector(bspnum & (~NF_SUBSECTOR), clipflags);
		return;
    }
		
	node_t* bsp = &cl_level.nodes[bspnum];
    
	if (bsp->visframe != r_visframecount)
 	{
		return;
	}

    // Decide which side the view point is on.
	int side = bsp->PointOnSide(vieworg);

	if (r_back2front)
	{
    	// Divide back space.
		RenderBSPNode(bsp->children[side ^ 1], bsp->bbox[side ^ 1], clipflags);

    	// Recursively divide front space.
	    RenderBSPNode(bsp->children[side], bsp->bbox[side], clipflags);
	}
	else
	{
    	// Recursively divide front space.
	    RenderBSPNode(bsp->children[side], bsp->bbox[side], clipflags);

    	// Divide back space.
		RenderBSPNode(bsp->children[side ^ 1], bsp->bbox[side ^ 1], clipflags);
	}
}

//==========================================================================
//
//	R_RenderWorld
//
//==========================================================================

void R_RenderWorld(void)
{
	float	dummy_bbox[6] = {-99999, -99999, -99999, 99999, 9999, 99999};

	R_SetUpFrustumIndexes();

	sky_is_visible = false;

	RenderBSPNode(cl_level.numnodes - 1, dummy_bbox, 15);	// head node is the last node output

	if (sky_is_visible)
	{
		R_DrawSkyBox();
	}

	Drawer->WorldDrawing();
}

