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
		surfs->Light = (lLev << 24) | LightParams->LightColor;
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
	unguard;
}

//==========================================================================
//
//	RenderBSPNode
//
// 	Renders all subsectors below a given node, traversing subtree
// recursively. Just call with BSP root.
//
//==========================================================================

static void RenderBSPNode(int bspnum, float *bbox, int InClipflags)
{
	guard(RenderBSPNode);
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

void R_RenderWorld(void)
{
	guard(R_RenderWorld);
	float	dummy_bbox[6] = {-99999, -99999, -99999, 99999, 9999, 99999};

	R_SetUpFrustumIndexes();

	sky_is_visible = false;

	RenderBSPNode(GClLevel->NumNodes - 1, dummy_bbox, 15);	// head node is the last node output

	if (sky_is_visible)
	{
		R_DrawSky();
	}

	Drawer->WorldDrawing();
	unguard;
}

//**************************************************************************
//
//	$Log$
//	Revision 1.15  2005/06/30 20:20:54  dj_jl
//	Implemented rendering of Boom fake flats.
//
//	Revision 1.14  2005/04/28 07:16:15  dj_jl
//	Fixed some warnings, other minor fixes.
//	
//	Revision 1.13  2005/03/28 07:28:19  dj_jl
//	Transfer lighting and other BOOM stuff.
//	
//	Revision 1.12  2002/09/07 16:31:51  dj_jl
//	Added Level class.
//	
//	Revision 1.11  2002/08/28 16:39:19  dj_jl
//	Implemented sector light color.
//	
//	Revision 1.10  2002/07/13 07:39:08  dj_jl
//	Removed back to front drawing.
//	
//	Revision 1.9  2002/03/28 17:58:02  dj_jl
//	Added support for scaled textures.
//	
//	Revision 1.8  2002/03/20 19:11:21  dj_jl
//	Added guarding.
//	
//	Revision 1.7  2002/01/25 18:08:19  dj_jl
//	Beautification
//	
//	Revision 1.6  2002/01/07 12:16:43  dj_jl
//	Changed copyright year
//	
//	Revision 1.5  2001/10/12 17:31:13  dj_jl
//	no message
//	
//	Revision 1.4  2001/09/12 17:33:39  dj_jl
//	Fixed paranoid errors
//	
//	Revision 1.3  2001/07/31 17:16:31  dj_jl
//	Just moved Log to the end of file
//	
//	Revision 1.2  2001/07/27 14:27:54  dj_jl
//	Update with Id-s and Log-s, some fixes
//
//**************************************************************************
