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
#include "r_local.h"

// MACROS ------------------------------------------------------------------

#define MAXSPLITVERTS		128
#define ON_EPSILON			0.1
#define subdivide_size		240

#define MAXWVERTS	8
#define WSURFSIZE	(sizeof(surface_t) + sizeof(TVec) * (MAXWVERTS - 1))

//	This is used to compare floats like ints which is faster
#define FASI(var)	(*(int*)&var)

// TYPES -------------------------------------------------------------------

// EXTERNAL FUNCTION PROTOTYPES --------------------------------------------

// PUBLIC FUNCTION PROTOTYPES ----------------------------------------------

// PRIVATE FUNCTION PROTOTYPES ---------------------------------------------

// EXTERNAL DATA DECLARATIONS ----------------------------------------------

extern int			light_mem;

// PUBLIC DATA DEFINITIONS -------------------------------------------------

// PRIVATE DATA DEFINITIONS ------------------------------------------------

static sector_t*	frontsector;
static sector_t*	backsector;

static subsector_t	*r_sub;
static sec_plane_t	*r_floor;
static sec_plane_t	*r_ceiling;
static sec_plane_t	*r_back_floor;
static sec_plane_t	*r_back_ceiling;

static sec_plane_t	sky_plane;
static float		skyheight;

static int			c_subdivides;

static surface_t	*free_wsurfs;
static int			c_seg_div;

// CODE --------------------------------------------------------------------

//**************************************************************************
//**
//**	Sector surfaces
//**
//**************************************************************************

//==========================================================================
//
//	SetupSky
//
//==========================================================================

static void	SetupSky(void)
{
	skyheight = -99999.0;
	for (int i = 0; i < cl_level.numsectors; i++)
	{
		if (cl_level.sectors[i].ceiling.pic == skyflatnum &&
			cl_level.sectors[i].ceiling.maxz > skyheight)
		{
			skyheight = cl_level.sectors[i].ceiling.maxz;
		}
	}
	sky_plane.Set(TVec(0, 0, -1), -skyheight);
	sky_plane.pic = skyflatnum;
}

//==========================================================================
//
//	InitSurfs
//
//==========================================================================

static void InitSurfs(surface_t *surfs, texinfo_t *texinfo, TPlane *plane)
{
	int i;
	float dot;
	float mins;
	float maxs;
	int bmins;
	int bmaxs;

	float soffs = DotProduct(texinfo->texorg, texinfo->saxis);
	float toffs = DotProduct(texinfo->texorg, texinfo->taxis);

	while (surfs)
	{
		if (plane)
		{
			surfs->texinfo = texinfo;
			surfs->plane = plane;
		}

		mins = 99999.0;
		maxs = -99999.0;
		for (i = 0; i < surfs->count; i++)
		{
			dot = DotProduct(surfs->verts[i], texinfo->saxis) - soffs;
			if (dot < mins)
				mins = dot;
			if (dot > maxs)
				maxs = dot;
		}
		bmins = (int)floor(mins / 16);
		bmaxs = (int)ceil(maxs / 16);
		surfs->texturemins[0] = bmins * 16;
		surfs->extents[0] = (bmaxs - bmins) * 16;
		if (surfs->extents[0] > 256)
		{
			Sys_Error("Bad extents");
		}

		mins = 99999.0;
		maxs = -99999.0;
		for (i = 0; i < surfs->count; i++)
		{
			dot = DotProduct(surfs->verts[i], texinfo->taxis) - toffs;
			if (dot < mins)
				mins = dot;
			if (dot > maxs)
				maxs = dot;
		}
		bmins = (int)floor(mins / 16);
		bmaxs = (int)ceil(maxs / 16);
		surfs->texturemins[1] = bmins * 16;
		surfs->extents[1] = (bmaxs - bmins) * 16;
		if (surfs->extents[1] > 256)
		{
			Sys_Error("Bad extents");
		}

		R_LightFace(surfs, r_sub);

		surfs = surfs->next;
	}
}

//==========================================================================
//
//	FlushSurfCaches
//
//==========================================================================

static void FlushSurfCaches(surface_t *surfs)
{
	while (surfs)
	{
		for (int i = 0; i < 4; i++)
		{
			if (surfs->cachespots[i])
			{
				Drawer->FreeSurfCache(surfs->cachespots[i]);
			}
		}
		surfs = surfs->next;
	}
}

//==========================================================================
//
//	SubdivideFace
//
//==========================================================================

static surface_t *SubdivideFace(surface_t *f, const TVec &axis, const TVec *nextaxis)
{
	int i;
	float dot;
	float mins = 99999.0;
	float maxs = -99999.0;

	for (i = 0; i < f->count; i++)
	{
		dot = DotProduct(f->verts[i], axis);
		if (dot < mins)
			mins = dot;
		if (dot > maxs)
			maxs = dot;
	}

	if (maxs - mins <= subdivide_size)
	{
		if (nextaxis)
		{
			f = SubdivideFace(f, *nextaxis, NULL);
		}
		return f;
	}

	c_subdivides++;

	if (f->count > MAXSPLITVERTS)
	{
		Host_Error("f->count > MAXSPLITVERTS\n");
	}

	TPlane plane;

	plane.normal = axis;
	dot = Length(plane.normal);
	Normalize(plane.normal);
	plane.dist = (mins + subdivide_size - 16) / dot;

	float dots[MAXSPLITVERTS + 1];
	int sides[MAXSPLITVERTS + 1];

	for (i = 0; i < f->count; i++)
	{
		dot = DotProduct(f->verts[i], plane.normal) - plane.dist;
		dots[i] = dot;
		if (dot < -ON_EPSILON)
			sides[i] = -1;
		else if (dot > ON_EPSILON)
			sides[i] = 1;
		else
			sides[i] = 0;
	}
	dots[f->count] = dots[0];
	sides[f->count] = sides[0];

	TVec verts1[MAXSPLITVERTS];
	TVec verts2[MAXSPLITVERTS];
	int count1 = 0;
	int count2 = 0;

	for (i = 0; i < f->count; i++)
	{
		if (sides[i] == 0)
		{
			verts1[count1++] = f->verts[i];
			verts2[count2++] = f->verts[i];
			continue;
		}
		if (sides[i] == 1)
		{
			verts1[count1++] = f->verts[i];
		}
		else
		{
			verts2[count2++] = f->verts[i];
		}
		if (sides[i + 1] == 0 || sides[i] == sides[i + 1])
		{
			continue;
		}

		// generate a split point
		TVec mid;
		TVec &p1 = f->verts[i];
		TVec &p2 = f->verts[(i + 1) % f->count];
		
		dot = dots[i] / (dots[i] - dots[i + 1]);
		for (int j = 0; j < 3; j++)
		{
			// avoid round off error when possible
			if (plane.normal[j] == 1)
				mid[j] = plane.dist;
			else if (plane.normal[j] == -1)
				mid[j] = -plane.dist;
			else
				mid[j] = p1[j] + dot * (p2[j] - p1[j]);
		}

		verts1[count1++] = mid;
		verts2[count2++] = mid;
	}

	surface_t *next = f->next;
	Z_Free(f);

	surface_t *back = (surface_t*)Z_Calloc(sizeof(surface_t) +
		(count2 - 1) * sizeof(TVec), PU_LEVEL, 0);
	back->count = count2;
	memcpy(back->verts, verts2, count2 * sizeof(TVec));

	surface_t *front = (surface_t*)Z_Calloc(sizeof(surface_t) +
		(count1 - 1) * sizeof(TVec), PU_LEVEL, 0);
	front->count = count1;
	memcpy(front->verts, verts1, count1 * sizeof(TVec));

	front->next = next;
	back->next = SubdivideFace(front, axis, nextaxis);
	if (nextaxis)
	{
		back = SubdivideFace(back, *nextaxis, NULL);
	}
	return back;
}

//==========================================================================
//
//	CreateSecSurface
//
//==========================================================================

static sec_surface_t *CreateSecSurface(subsector_t *sub, sec_plane_t *splane)
{
	sec_surface_t	*ssurf;
	surface_t		*surf;

	ssurf = (sec_surface_t*)Z_Calloc(sizeof(sec_surface_t), PU_LEVEL, 0);
	surf = (surface_t*)Z_Calloc(sizeof(surface_t) +
		(sub->numlines - 1) * sizeof(TVec), PU_LEVEL, 0);

	if (splane->pic == skyflatnum && splane->normal.z < 0.0)
	{
		splane = &sky_plane;
	}
	ssurf->secplane = splane;
	ssurf->dist = splane->dist;

	if (fabs(splane->normal.z) > 0.7)
	{
		ssurf->texinfo.saxis = TVec(1, 0, 0);
		ssurf->texinfo.taxis = TVec(0, -1, 0);
		ssurf->texinfo.texorg = TVec(-splane->xoffs, splane->yoffs, 0);
	}
	else
	{
		ssurf->texinfo.taxis = TVec(0, 0, -1);
		ssurf->texinfo.saxis = Normalize(CrossProduct(splane->normal, ssurf->texinfo.taxis));
		ssurf->texinfo.texorg = -splane->xoffs * ssurf->texinfo.saxis + -splane->yoffs * ssurf->texinfo.taxis;
	}
	ssurf->texinfo.pic = splane->pic;
	ssurf->texinfo.translucency = splane->translucency ? splane->translucency + 1 : 0;
	ssurf->xoffs = splane->xoffs;
	ssurf->yoffs = splane->yoffs;

	surf->count = sub->numlines;
    seg_t *line = &cl_level.segs[sub->firstline];
	int vlindex = (splane->normal.z < 0);
   	for (int i = 0; i < surf->count; i++)
    {
		TVec &v = *line[vlindex ? surf->count - i - 1 : i].v1;
		TVec &dst = surf->verts[i];
		dst = v;
		dst.z = splane->GetPointZ(dst);
	}

	if (splane->pic == skyflatnum)
	{
		//	Don't subdivide sky
		ssurf->surfs = surf;
	}
	else
	{
		ssurf->surfs = SubdivideFace(surf, ssurf->texinfo.saxis, &ssurf->texinfo.taxis);
		InitSurfs(ssurf->surfs, &ssurf->texinfo, splane);
	}

	return ssurf;
}

//==========================================================================
//
//	UpdateSecSurface
//
//==========================================================================

static void UpdateSecSurface(sec_surface_t *ssurf)
{
	sec_plane_t		*plane = ssurf->secplane;

	if (!plane->pic)
	{
		return;
	}

	if (FASI(ssurf->dist) != FASI(plane->dist))
	{
		ssurf->dist = plane->dist;
		for (surface_t *surf = ssurf->surfs; surf; surf = surf->next)
		{
		   	for (int i = 0; i < surf->count; i++)
    		{
				surf->verts[i].z = plane->GetPointZ(surf->verts[i]);
			}
		}
		FlushSurfCaches(ssurf->surfs);
		InitSurfs(ssurf->surfs, &ssurf->texinfo, NULL);
	}
	if (FASI(ssurf->xoffs) != FASI(plane->xoffs) ||
		FASI(ssurf->yoffs) != FASI(plane->yoffs))
	{
		ssurf->texinfo.texorg = TVec(-plane->xoffs, plane->yoffs, 0);
		ssurf->xoffs = plane->xoffs;
		ssurf->yoffs = plane->yoffs;
		FlushSurfCaches(ssurf->surfs);
		InitSurfs(ssurf->surfs, &ssurf->texinfo, NULL);
	}
	if (ssurf->texinfo.pic != plane->pic)
	{
		ssurf->texinfo.pic = plane->pic;
	}
}

//**************************************************************************
//**
//**	Seg surfaces
//**
//**************************************************************************

//==========================================================================
//
//	NewWSurf
//
//==========================================================================

static surface_t *NewWSurf(void)
{
	if (!free_wsurfs)
	{
		//	Allocate some more surfs
		byte *tmp = (byte*)Z_Malloc(WSURFSIZE * 32, PU_LEVEL, 0);
		for (int i = 0; i < 32; i++)
		{
			((surface_t*)tmp)->next = free_wsurfs;
			free_wsurfs = (surface_t*)tmp;
			tmp += WSURFSIZE;
		}
	}
	surface_t *surf = free_wsurfs;
	free_wsurfs = surf->next;

	memset(surf, 0, WSURFSIZE);

	return surf;
}

//==========================================================================
//
//	FreeWSurfs
//
//==========================================================================

static void	FreeWSurfs(surface_t *surfs)
{
	FlushSurfCaches(surfs);
	while (surfs)
	{
		if (surfs->lightmap)
		{
			Z_Free(surfs->lightmap);
		}
		surface_t *next = surfs->next;
		surfs->next = free_wsurfs;
		free_wsurfs = surfs;
		surfs = next;
	}
}

//==========================================================================
//
//	SubdivideSeg
//
//==========================================================================

static surface_t *SubdivideSeg(surface_t *surf, const TVec &axis, const TVec *nextaxis)
{
	int i;
	float dot;
	float mins = 99999.0;
	float maxs = -99999.0;

	for (i = 0; i < surf->count; i++)
	{
		dot = DotProduct(surf->verts[i], axis);
		if (dot < mins)
			mins = dot;
		if (dot > maxs)
			maxs = dot;
	}

	if (maxs - mins <= subdivide_size)
	{
		if (nextaxis)
		{
			surf = SubdivideSeg(surf, *nextaxis, NULL);
		}
		return surf;
	}

	c_seg_div++;

	TPlane plane;

	plane.normal = axis;
	dot = Length(plane.normal);
	Normalize(plane.normal);
	plane.dist = (mins + subdivide_size - 16) / dot;

	float dots[MAXWVERTS + 1];
	int sides[MAXWVERTS + 1];

	for (i = 0; i < surf->count; i++)
	{
		dot = DotProduct(surf->verts[i], plane.normal) - plane.dist;
		dots[i] = dot;
		if (dot < -ON_EPSILON)
			sides[i] = -1;
		else if (dot > ON_EPSILON)
			sides[i] = 1;
		else
			sides[i] = 0;
	}
	dots[surf->count] = dots[0];
	sides[surf->count] = sides[0];

	TVec verts1[MAXWVERTS];
	TVec verts2[MAXWVERTS];
	int count1 = 0;
	int count2 = 0;

	for (i = 0; i < surf->count; i++)
	{
		if (sides[i] == 0)
		{
			verts1[count1++] = surf->verts[i];
			verts2[count2++] = surf->verts[i];
			continue;
		}
		if (sides[i] == 1)
		{
			verts1[count1++] = surf->verts[i];
		}
		else
		{
			verts2[count2++] = surf->verts[i];
		}
		if (sides[i + 1] == 0 || sides[i] == sides[i + 1])
		{
			continue;
		}

		// generate a split point
		TVec mid;
		TVec &p1 = surf->verts[i];
		TVec &p2 = surf->verts[(i + 1) % surf->count];
		
		dot = dots[i] / (dots[i] - dots[i + 1]);
		for (int j = 0; j < 3; j++)
		{
			// avoid round off error when possible
			if (plane.normal[j] == 1)
				mid[j] = plane.dist;
			else if (plane.normal[j] == -1)
				mid[j] = -plane.dist;
			else
				mid[j] = p1[j] + dot * (p2[j] - p1[j]);
		}

		verts1[count1++] = mid;
		verts2[count2++] = mid;
	}

	surf->count = count2;
	memcpy(surf->verts, verts2, count2 * sizeof(TVec));

	surface_t *news = NewWSurf();
	news->count = count1;
	memcpy(news->verts, verts1, count1 * sizeof(TVec));

	news->next = surf->next;
	surf->next = SubdivideSeg(news, axis, nextaxis);
	if (nextaxis)
	{
		surf = SubdivideSeg(surf, *nextaxis, NULL);
	}
	return surf;
}

//==========================================================================
//
//	CreateWSurfs
//
//==========================================================================

static surface_t *CreateWSurfs(TVec *wv, texinfo_t *texinfo, seg_t *seg)
{
	if (wv[1].z <= wv[0].z && wv[2].z <= wv[3].z)
	{
		return NULL;
	}

	if (!texinfo->pic)
	{
		return NULL;
	}

	surface_t *surf;

	if (texinfo->pic == skyflatnum)
	{
		//	Never split sky surfaces
		surf = NewWSurf();
		surf->next = NULL;
		surf->count = 4;
		memcpy(surf->verts, wv, 4 * sizeof(TVec));
		return surf;
	}

	surf = NewWSurf();
	surf->next = NULL;
	surf->count = 4;
	memcpy(surf->verts, wv, 4 * sizeof(TVec));
	surf = SubdivideSeg(surf, texinfo->saxis, &texinfo->taxis);
	InitSurfs(surf, texinfo, seg);
	return surf;
}

//==========================================================================
//
//	CountSegParts
//
//==========================================================================

static int CountSegParts(seg_t *seg)
{
	if (!seg->linedef)
	{
		//	Miniseg
		return 0;
	}
	int			count;
	if (!seg->backsector)
	{
		count = 2;
	}
	else
	{
		count = 4;
		for (sec_region_t *reg = seg->backsector->topregion;
			reg->prev; reg = reg->prev)
		{
			count++;
		}
	}
	return count;
}

//==========================================================================
//
//	CreateSegParts
//
//==========================================================================

static segpart_t	*pspart;

static void CreateSegParts(drawseg_t* dseg, seg_t *seg)
{
	TVec		wv[4];
	segpart_t	*sp;
	float		hdelta;
	float		offshdelta;

	dseg->seg = seg;
	dseg->next = seg->drawsegs;
	seg->drawsegs = dseg;

	if (!seg->linedef)
	{
		//	Miniseg
		return;
	}

	backsector = seg->backsector;

	side_t *sidedef = seg->sidedef;
	line_t *linedef = seg->linedef;

	TVec segdir = (*seg->v2 - *seg->v1) / seg->length;

	float topz1 = r_ceiling->GetPointZ(*seg->v1);
	float topz2 = r_ceiling->GetPointZ(*seg->v2);
	float botz1 = r_floor->GetPointZ(*seg->v1);
	float botz2 = r_floor->GetPointZ(*seg->v2);

	if (!backsector)
	{
		dseg->mid = pspart++;
		sp = dseg->mid;

		sp->texinfo.saxis = segdir;
		sp->texinfo.taxis = TVec(0, 0, -1);
		sp->texinfo.texorg = *seg->v1 - sp->texinfo.saxis *
			(sidedef->textureoffset + seg->offset);
		sp->texinfo.pic = sidedef->midtexture;

		hdelta = topz2 - topz1;
		offshdelta = hdelta * seg->offset / seg->length;

   	    if (linedef->flags & ML_DONTPEGBOTTOM)
        {
       	    //	bottom of texture at bottom
			sp->texinfo.texorg.z = MIN(botz1, botz2) +
				R_TextureHeight(sidedef->midtexture);
       	}
		else if (linedef->flags & ML_DONTPEGTOP)
		{
           	// top of texture at top of top region
			sp->texinfo.texorg.z = r_sub->sector->topregion->ceiling->GetPointZ(*seg->v1);
		}
   	    else
		{
           	// top of texture at top
			sp->texinfo.texorg.z = topz1;
		}
		sp->texinfo.texorg.z += sidedef->rowoffset - offshdelta;

		wv[0].x = wv[1].x = seg->v1->x;
		wv[0].y = wv[1].y = seg->v1->y;
		wv[2].x = wv[3].x = seg->v2->x;
		wv[2].y = wv[3].y = seg->v2->y;

		wv[0].z = botz1;
		wv[1].z = topz1;
		wv[2].z = topz2;
		wv[3].z = botz2;

		sp->surfs = CreateWSurfs(wv, &sp->texinfo, seg);

		sp->frontTopDist = r_ceiling->dist;
		sp->frontBotDist = r_floor->dist;
		sp->textureoffset = sidedef->textureoffset;
		sp->rowoffset = sidedef->rowoffset;

		dseg->topsky = pspart++;
       	if (r_ceiling->pic == skyflatnum)
		{
			sp = dseg->topsky;

			sp->texinfo.pic = skyflatnum;

			wv[0].x = wv[1].x = seg->v1->x;
			wv[0].y = wv[1].y = seg->v1->y;
			wv[2].x = wv[3].x = seg->v2->x;
			wv[2].y = wv[3].y = seg->v2->y;

			wv[0].z = topz1;
			wv[1].z = wv[2].z = skyheight;
			wv[3].z = topz2;

			sp->surfs = CreateWSurfs(wv, &sp->texinfo, seg);

			sp->frontTopDist = r_ceiling->dist;
		}
	}
	else
	{
		// two sided line

		r_back_floor = &backsector->floor;
		r_back_ceiling = &backsector->ceiling;

		float back_topz1 = r_back_ceiling->GetPointZ(*seg->v1);
		float back_topz2 = r_back_ceiling->GetPointZ(*seg->v2);
		float back_botz1 = r_back_floor->GetPointZ(*seg->v1);
		float back_botz2 = r_back_floor->GetPointZ(*seg->v2);

        // hack to allow height changes in outdoor areas
		if (r_ceiling->pic == skyflatnum &&
			r_back_ceiling->pic == skyflatnum)
		{
			topz1 = back_topz1;
			topz2 = back_topz2;
		}

		// top wall
		dseg->top = pspart++;
		sp = dseg->top;

		sp->texinfo.saxis = segdir;
		sp->texinfo.taxis = TVec(0, 0, -1);
		sp->texinfo.texorg = *seg->v1 - sp->texinfo.saxis *
			(sidedef->textureoffset + seg->offset);
		sp->texinfo.pic = sidedef->toptexture;

		hdelta = topz2 - topz1;
		offshdelta = hdelta * seg->offset / seg->length;
		if (linedef->flags & ML_DONTPEGTOP)
		{
			// top of texture at top
			sp->texinfo.texorg.z = topz1;
		}
		else
		{
			// bottom of texture
			sp->texinfo.texorg.z = back_topz1 +
				R_TextureHeight(sidedef->toptexture);
		}
		sp->texinfo.texorg.z += sidedef->rowoffset - offshdelta;

		wv[0].x = wv[1].x = seg->v1->x;
		wv[0].y = wv[1].y = seg->v1->y;
		wv[2].x = wv[3].x = seg->v2->x;
		wv[2].y = wv[3].y = seg->v2->y;

		wv[0].z = MAX(back_topz1, botz1);
		wv[1].z = topz1;
		wv[2].z = topz2;
		wv[3].z = MAX(back_topz2, botz2);

		sp->surfs = CreateWSurfs(wv, &sp->texinfo, seg);

		sp->frontTopDist = r_ceiling->dist;
		sp->frontBotDist = r_floor->dist;
		sp->backTopDist = r_back_ceiling->dist;
		sp->backBotDist = r_back_floor->dist;
		sp->textureoffset = sidedef->textureoffset;
		sp->rowoffset = sidedef->rowoffset;

		//	Sky abowe top
		dseg->topsky = pspart++;
       	if (r_ceiling->pic == skyflatnum &&
       		r_back_ceiling->pic != skyflatnum)
		{
			sp = dseg->topsky;

			sp->texinfo.pic = skyflatnum;

			wv[0].x = wv[1].x = seg->v1->x;
			wv[0].y = wv[1].y = seg->v1->y;
			wv[2].x = wv[3].x = seg->v2->x;
			wv[2].y = wv[3].y = seg->v2->y;

			wv[0].z = topz1;
			wv[1].z = wv[2].z = skyheight;
			wv[3].z = topz2;

			sp->surfs = CreateWSurfs(wv, &sp->texinfo, seg);

			sp->frontTopDist = r_ceiling->dist;
		}

		// bottom wall
		dseg->bot = pspart++;
		sp = dseg->bot;

		sp->texinfo.saxis = segdir;
		sp->texinfo.taxis = TVec(0, 0, -1);
		sp->texinfo.texorg = *seg->v1 - sp->texinfo.saxis *
			(sidedef->textureoffset + seg->offset);
		sp->texinfo.pic = sidedef->bottomtexture;

		hdelta = back_botz2 - back_botz1;
		offshdelta = hdelta * seg->offset / seg->length;
		if (linedef->flags & ML_DONTPEGBOTTOM)
		{
			// bottom of texture at bottom
			// top of texture at top
			sp->texinfo.texorg.z = topz1;
		}
		else
		{
           	// top of texture at top
			sp->texinfo.texorg.z = back_botz1;
		}
		sp->texinfo.texorg.z += sidedef->rowoffset - offshdelta;

		wv[0].x = wv[1].x = seg->v1->x;
		wv[0].y = wv[1].y = seg->v1->y;
		wv[2].x = wv[3].x = seg->v2->x;
		wv[2].y = wv[3].y = seg->v2->y;

		wv[0].z = botz1;
		wv[1].z = MIN(back_botz1, topz1);
		wv[2].z = MIN(back_botz2, topz2);
		wv[3].z = botz2;

		sp->surfs = CreateWSurfs(wv, &sp->texinfo, seg);

		sp->frontTopDist = r_ceiling->dist;
		sp->frontBotDist = r_floor->dist;
		sp->backTopDist = r_back_ceiling->dist;
		sp->backBotDist = r_back_floor->dist;
		sp->textureoffset = sidedef->textureoffset;
		sp->rowoffset = sidedef->rowoffset;

		float midtopz1 = MIN(topz1, back_topz1);
		float midtopz2 = MIN(topz2, back_topz2);
		float midbotz1 = MAX(botz1, back_botz1);
		float midbotz2 = MAX(botz2, back_botz2);

		dseg->mid = pspart++;
		sp = dseg->mid;

		sp->texinfo.saxis = segdir;
		sp->texinfo.taxis = TVec(0, 0, -1);

		if (sidedef->midtexture)
		{
			// masked midtexture
			float texh = R_TextureHeight(sidedef->midtexture);
			hdelta = midtopz2 - midtopz1;
			offshdelta = hdelta * seg->offset / seg->length;

			float			z_org;

			if (linedef->flags & ML_DONTPEGBOTTOM)
			{
				// bottom of texture at bottom
				// top of texture at top
				z_org = MIN(midbotz1, midbotz2) + texh;
			}
			else
			{
				// top of texture at top
				z_org = MAX(midtopz1, midtopz2);
			}
			z_org += sidedef->rowoffset - offshdelta;

			sp->texinfo.texorg = *seg->v1 - sp->texinfo.saxis *
				(sidedef->textureoffset + seg->offset);
			sp->texinfo.texorg.z = z_org;
			sp->texinfo.pic = sidedef->midtexture;
			sp->texinfo.translucency = linedef->translucency + 1;

			wv[0].x = wv[1].x = seg->v1->x;
			wv[0].y = wv[1].y = seg->v1->y;
			wv[2].x = wv[3].x = seg->v2->x;
			wv[2].y = wv[3].y = seg->v2->y;

			wv[0].z = MAX(midbotz1, z_org - texh);
			wv[1].z = MIN(midtopz1, z_org);
			wv[2].z = MIN(midtopz2, z_org);
			wv[3].z = MAX(midbotz2, z_org - texh);

			sp->surfs = CreateWSurfs(wv, &sp->texinfo, seg);
		}

		sp->frontTopDist = r_ceiling->dist;
		sp->frontBotDist = r_floor->dist;
		sp->backTopDist = r_back_ceiling->dist;
		sp->backBotDist = r_back_floor->dist;
		sp->textureoffset = sidedef->textureoffset;
		sp->rowoffset = sidedef->rowoffset;

		sec_region_t *reg;
		for (reg = backsector->topregion; reg->prev; reg = reg->prev)
		{
			sp = pspart++;
			sp->next = dseg->extra;
			dseg->extra = sp;

			TPlane *extratop = reg->floor;
			TPlane *extrabot = reg->prev->ceiling;
			side_t *extraside = &cl_level.sides[reg->prev->extraline->sidenum[0]];

			float extratopz1 = extratop->GetPointZ(*seg->v1);
			float extratopz2 = extratop->GetPointZ(*seg->v2);
			float extrabotz1 = extrabot->GetPointZ(*seg->v1);
			float extrabotz2 = extrabot->GetPointZ(*seg->v2);

			sp->texinfo.saxis = segdir;
			sp->texinfo.taxis = TVec(0, 0, -1);
			sp->texinfo.texorg = *seg->v1 - sp->texinfo.saxis *
				(sidedef->textureoffset + seg->offset);

			hdelta = extratopz2 - extratopz1;
			offshdelta = hdelta * seg->offset / seg->length;
			sp->texinfo.texorg.z = sidedef->rowoffset - offshdelta + extratopz1;
			sp->texinfo.pic = extraside->midtexture;
			sp->texinfo.translucency = reg->prev->extraline->translucency ? reg->prev->extraline->translucency + 1 : 0;

			wv[0].x = wv[1].x = seg->v1->x;
			wv[0].y = wv[1].y = seg->v1->y;
			wv[2].x = wv[3].x = seg->v2->x;
			wv[2].y = wv[3].y = seg->v2->y;

			wv[0].z = MAX(extrabotz1, botz1);
			wv[1].z = MIN(extratopz1, topz1);
			wv[2].z = MIN(extratopz2, topz2);
			wv[3].z = MAX(extrabotz2, botz2);

			sp->surfs = CreateWSurfs(wv, &sp->texinfo, seg);

			sp->frontTopDist = r_ceiling->dist;
			sp->frontBotDist = r_floor->dist;
			sp->backTopDist = extratop->dist;
			sp->backBotDist = extrabot->dist;
			sp->textureoffset = sidedef->textureoffset;
			sp->rowoffset = sidedef->rowoffset;
		}
	}
}

//==========================================================================
//
//	UpdateRowOffset
//
//==========================================================================

static void	UpdateRowOffset(segpart_t *sp, float rowoffset)
{
	sp->texinfo.texorg.z += rowoffset - sp->rowoffset;
	sp->rowoffset = rowoffset;
	FlushSurfCaches(sp->surfs);
	InitSurfs(sp->surfs, &sp->texinfo, NULL);
}

//==========================================================================
//
//	UpdateTextureOffset
//
//==========================================================================

static void UpdateTextureOffset(segpart_t *sp, seg_t *seg, float textureoffset)
{
	TVec tmp = *seg->v1 - sp->texinfo.saxis * (textureoffset + seg->offset);
	sp->texinfo.texorg.x = tmp.x;
	sp->texinfo.texorg.y = tmp.y;
	sp->textureoffset = textureoffset;
	FlushSurfCaches(sp->surfs);
	InitSurfs(sp->surfs, &sp->texinfo, NULL);
}

//==========================================================================
//
//	UpdateDrawSeg
//
//==========================================================================

static void UpdateDrawSeg(drawseg_t* dseg)
{
	seg_t *seg = dseg->seg;
	segpart_t *sp;
	TVec wv[4];

	if (!seg->linedef)
	{
		//	Miniseg
		return;
	}

	backsector = seg->backsector;

	side_t *sidedef = seg->sidedef;
	line_t *linedef = seg->linedef;

	if (!backsector)
	{
		sp = dseg->mid;
		if (FASI(sp->frontTopDist) != FASI(r_ceiling->dist) ||
			FASI(sp->frontBotDist) != FASI(r_floor->dist) ||
			sp->texinfo.pic != sidedef->midtexture)
		{
			float topz1 = r_ceiling->GetPointZ(*seg->v1);
			float topz2 = r_ceiling->GetPointZ(*seg->v2);
			float botz1 = r_floor->GetPointZ(*seg->v1);
			float botz2 = r_floor->GetPointZ(*seg->v2);

			FreeWSurfs(sp->surfs);
			sp->surfs = NULL;

			float hdelta = topz2 - topz1;
			float offshdelta = hdelta * seg->offset / seg->length;

	   	    if (linedef->flags & ML_DONTPEGBOTTOM)
        	{
    	   	    //	bottom of texture at bottom
				sp->texinfo.texorg.z = MIN(botz1, botz2) +
					R_TextureHeight(sidedef->midtexture);
       		}
   		    else
			{
           		// top of texture at top
				sp->texinfo.texorg.z = topz1;
			}
			sp->texinfo.texorg.z += sidedef->rowoffset - offshdelta;
			sp->texinfo.pic = sidedef->midtexture;

			wv[0].x = wv[1].x = seg->v1->x;
			wv[0].y = wv[1].y = seg->v1->y;
			wv[2].x = wv[3].x = seg->v2->x;
			wv[2].y = wv[3].y = seg->v2->y;

			wv[0].z = botz1;
			wv[1].z = topz1;
			wv[2].z = topz2;
			wv[3].z = botz2;

			sp->surfs = CreateWSurfs(wv, &sp->texinfo, seg);

			sp->frontTopDist = r_ceiling->dist;
			sp->frontBotDist = r_floor->dist;
			sp->rowoffset = sidedef->rowoffset;
		}
		else if (FASI(sp->rowoffset) != FASI(sidedef->rowoffset))
		{
			UpdateRowOffset(sp, sidedef->rowoffset);
		}
		if (FASI(sp->textureoffset) != FASI(sidedef->textureoffset))
		{
			UpdateTextureOffset(sp, seg, sidedef->textureoffset);
		}

		sp = dseg->topsky;
       	if (r_ceiling->pic == skyflatnum &&
			FASI(sp->frontTopDist) != FASI(r_ceiling->dist))
		{
			float topz1 = r_ceiling->GetPointZ(*seg->v1);
			float topz2 = r_ceiling->GetPointZ(*seg->v2);

			FreeWSurfs(sp->surfs);
			sp->surfs = NULL;

			wv[0].x = wv[1].x = seg->v1->x;
			wv[0].y = wv[1].y = seg->v1->y;
			wv[2].x = wv[3].x = seg->v2->x;
			wv[2].y = wv[3].y = seg->v2->y;

			wv[0].z = topz1;
			wv[1].z = wv[2].z = skyheight;
			wv[3].z = topz2;

			sp->surfs = CreateWSurfs(wv, &sp->texinfo, seg);

			sp->frontTopDist = r_ceiling->dist;
		}
	}
	else
	{
		r_back_floor = &backsector->floor;
		r_back_ceiling = &backsector->ceiling;

		// top wall
		sp = dseg->top;
		if (FASI(sp->frontTopDist) != FASI(r_ceiling->dist) ||
			FASI(sp->frontBotDist) != FASI(r_floor->dist) ||
			FASI(sp->backTopDist) != FASI(r_back_ceiling->dist) ||
			sp->texinfo.pic != sidedef->toptexture)
		{
			float topz1 = r_ceiling->GetPointZ(*seg->v1);
			float topz2 = r_ceiling->GetPointZ(*seg->v2);
			float botz1 = r_floor->GetPointZ(*seg->v1);
			float botz2 = r_floor->GetPointZ(*seg->v2);

			float back_topz1 = r_back_ceiling->GetPointZ(*seg->v1);
			float back_topz2 = r_back_ceiling->GetPointZ(*seg->v2);

	        // hack to allow height changes in outdoor areas
			if (r_ceiling->pic == skyflatnum &&
				r_back_ceiling->pic == skyflatnum)
			{
				topz1 = back_topz1;
				topz2 = back_topz2;
			}

			FreeWSurfs(sp->surfs);
			sp->surfs = NULL;

			float hdelta = topz2 - topz1;
			float offshdelta = hdelta * seg->offset / seg->length;
			if (linedef->flags & ML_DONTPEGTOP)
			{
				// top of texture at top
				sp->texinfo.texorg.z = topz1;
			}
			else
			{
				// bottom of texture
				sp->texinfo.texorg.z = back_topz1 +
					R_TextureHeight(sidedef->toptexture);
			}
			sp->texinfo.texorg.z += sidedef->rowoffset - offshdelta;
			sp->texinfo.pic = sidedef->toptexture;

			wv[0].x = wv[1].x = seg->v1->x;
			wv[0].y = wv[1].y = seg->v1->y;
			wv[2].x = wv[3].x = seg->v2->x;
			wv[2].y = wv[3].y = seg->v2->y;

			wv[0].z = MAX(back_topz1, botz1);
			wv[1].z = topz1;
			wv[2].z = topz2;
			wv[3].z = MAX(back_topz2, botz2);

			sp->surfs = CreateWSurfs(wv, &sp->texinfo, seg);

			sp->frontTopDist = r_ceiling->dist;
			sp->frontBotDist = r_floor->dist;
			sp->backTopDist = r_back_ceiling->dist;
			sp->rowoffset = sidedef->rowoffset;
		}
		else if (FASI(sp->rowoffset) != FASI(sidedef->rowoffset))
		{
			UpdateRowOffset(sp, sidedef->rowoffset);
		}
		if (FASI(sp->textureoffset) != FASI(sidedef->textureoffset))
		{
			UpdateTextureOffset(sp, seg, sidedef->textureoffset);
		}

		//	Sky abowe top
		sp = dseg->topsky;
		if (r_ceiling->pic == skyflatnum &&
			r_back_ceiling->pic != skyflatnum &&
       		FASI(sp->frontTopDist) != FASI(r_ceiling->dist))
		{
			float topz1 = r_ceiling->GetPointZ(*seg->v1);
			float topz2 = r_ceiling->GetPointZ(*seg->v2);

			FreeWSurfs(sp->surfs);
			sp->surfs = NULL;

			wv[0].x = wv[1].x = seg->v1->x;
			wv[0].y = wv[1].y = seg->v1->y;
			wv[2].x = wv[3].x = seg->v2->x;
			wv[2].y = wv[3].y = seg->v2->y;

			wv[0].z = topz1;
			wv[1].z = wv[2].z = skyheight;
			wv[3].z = topz2;

			sp->surfs = CreateWSurfs(wv, &sp->texinfo, seg);

			sp->frontTopDist = r_ceiling->dist;
		}

		// bottom wall
		sp = dseg->bot;
		if (FASI(sp->frontTopDist) != FASI(r_ceiling->dist) ||
			FASI(sp->frontBotDist) != FASI(r_floor->dist) ||
			FASI(sp->backBotDist) != FASI(r_back_floor->dist))
		{
			float topz1 = r_ceiling->GetPointZ(*seg->v1);
			float topz2 = r_ceiling->GetPointZ(*seg->v2);
			float botz1 = r_floor->GetPointZ(*seg->v1);
			float botz2 = r_floor->GetPointZ(*seg->v2);

			float back_botz1 = r_back_floor->GetPointZ(*seg->v1);
			float back_botz2 = r_back_floor->GetPointZ(*seg->v2);

	        // hack to allow height changes in outdoor areas
			if (r_ceiling->pic == skyflatnum &&
				r_back_ceiling->pic == skyflatnum)
			{
				topz1 = r_back_ceiling->GetPointZ(*seg->v1);
				topz2 = r_back_ceiling->GetPointZ(*seg->v2);
			}

			FreeWSurfs(sp->surfs);
			sp->surfs = NULL;

			float hdelta = back_botz2 - back_botz1;
			float offshdelta = hdelta * seg->offset / seg->length;
			if (linedef->flags & ML_DONTPEGBOTTOM)
			{
				// bottom of texture at bottom
				// top of texture at top
				sp->texinfo.texorg.z = topz1;
			}
			else
			{
           		// top of texture at top
				sp->texinfo.texorg.z = back_botz1;
			}
			sp->texinfo.texorg.z += sidedef->rowoffset - offshdelta;

			wv[0].x = wv[1].x = seg->v1->x;
			wv[0].y = wv[1].y = seg->v1->y;
			wv[2].x = wv[3].x = seg->v2->x;
			wv[2].y = wv[3].y = seg->v2->y;

			wv[0].z = botz1;
			wv[1].z = MIN(back_botz1, topz1);
			wv[2].z = MIN(back_botz2, topz2);
			wv[3].z = botz2;

			sp->surfs = CreateWSurfs(wv, &sp->texinfo, seg);

			sp->frontTopDist = r_ceiling->dist;
			sp->frontBotDist = r_floor->dist;
			sp->backBotDist = r_back_floor->dist;
			sp->rowoffset = sidedef->rowoffset;
		}
		else if (FASI(sp->rowoffset) != FASI(sidedef->rowoffset))
		{
			UpdateRowOffset(sp, sidedef->rowoffset);
		}
		if (FASI(sp->textureoffset) != FASI(sidedef->textureoffset))
		{
			UpdateTextureOffset(sp, seg, sidedef->textureoffset);
		}
		if (sp->texinfo.pic != sidedef->bottomtexture)
		{
			sp->texinfo.pic = sidedef->bottomtexture;
		}

		// masked midtexture
		sp = dseg->mid;
		if (FASI(sp->frontTopDist) != FASI(r_ceiling->dist) ||
			FASI(sp->frontBotDist) != FASI(r_floor->dist) ||
			FASI(sp->backTopDist) != FASI(r_back_ceiling->dist) ||
			FASI(sp->backBotDist) != FASI(r_back_floor->dist) ||
			FASI(sp->rowoffset) != FASI(sidedef->rowoffset) ||
			sp->texinfo.pic != sidedef->midtexture)
		{
			FreeWSurfs(sp->surfs);
			sp->surfs = NULL;

			sp->texinfo.pic = sidedef->midtexture;
			if (sidedef->midtexture)
			{
				float topz1 = r_ceiling->GetPointZ(*seg->v1);
				float topz2 = r_ceiling->GetPointZ(*seg->v2);
				float botz1 = r_floor->GetPointZ(*seg->v1);
				float botz2 = r_floor->GetPointZ(*seg->v2);

				float back_topz1 = r_back_ceiling->GetPointZ(*seg->v1);
				float back_topz2 = r_back_ceiling->GetPointZ(*seg->v2);
				float back_botz1 = r_back_floor->GetPointZ(*seg->v1);
				float back_botz2 = r_back_floor->GetPointZ(*seg->v2);

		        // hack to allow height changes in outdoor areas
				if (r_ceiling->pic == skyflatnum &&
					r_back_ceiling->pic == skyflatnum)
				{
					topz1 = back_topz1;
					topz2 = back_topz2;
				}

				float midtopz1 = MIN(topz1, back_topz1);
				float midtopz2 = MIN(topz2, back_topz2);
				float midbotz1 = MAX(botz1, back_botz1);
				float midbotz2 = MAX(botz2, back_botz2);

				float texh = R_TextureHeight(sidedef->midtexture);
				float hdelta = midtopz2 - midtopz1;
				float offshdelta = hdelta * seg->offset / seg->length;

				float			z_org;

				if (linedef->flags & ML_DONTPEGBOTTOM)
				{
					// bottom of texture at bottom
					// top of texture at top
					z_org = MIN(midbotz1, midbotz2) + texh;
				}
				else
				{
					// top of texture at top
					z_org = MAX(midtopz1, midtopz2);
				}
				z_org += sidedef->rowoffset - offshdelta;

				sp->texinfo.texorg.z = z_org;

				wv[0].x = wv[1].x = seg->v1->x;
				wv[0].y = wv[1].y = seg->v1->y;
				wv[2].x = wv[3].x = seg->v2->x;
				wv[2].y = wv[3].y = seg->v2->y;

				wv[0].z = MAX(midbotz1, z_org - texh);
				wv[1].z = MIN(midtopz1, z_org);
				wv[2].z = MIN(midtopz2, z_org);
				wv[3].z = MAX(midbotz2, z_org - texh);

				sp->surfs = CreateWSurfs(wv, &sp->texinfo, seg);
			}

			sp->frontTopDist = r_ceiling->dist;
			sp->frontBotDist = r_floor->dist;
			sp->backTopDist = r_back_ceiling->dist;
			sp->backBotDist = r_back_floor->dist;
			sp->rowoffset = sidedef->rowoffset;
		}
		if (FASI(sp->textureoffset) != FASI(sidedef->textureoffset))
		{
			UpdateTextureOffset(sp, seg, sidedef->textureoffset);
		}

		sec_region_t *reg;
		segpart_t *sp = dseg->extra;
		for (reg = backsector->botregion; reg->next; reg = reg->next)
		{
			TPlane *extratop = reg->next->floor;
			TPlane *extrabot = reg->ceiling;
			side_t *extraside = &cl_level.sides[reg->extraline->sidenum[0]];

			sp->texinfo.pic = extraside->midtexture;
			if (FASI(sp->frontTopDist) != FASI(r_ceiling->dist) ||
				FASI(sp->frontBotDist) != FASI(r_floor->dist) ||
				FASI(sp->backTopDist) != FASI(extratop->dist) ||
				FASI(sp->backBotDist) != FASI(extrabot->dist))
			{
				float topz1 = r_ceiling->GetPointZ(*seg->v1);
				float topz2 = r_ceiling->GetPointZ(*seg->v2);
				float botz1 = r_floor->GetPointZ(*seg->v1);
				float botz2 = r_floor->GetPointZ(*seg->v2);

				float extratopz1 = extratop->GetPointZ(*seg->v1);
				float extratopz2 = extratop->GetPointZ(*seg->v2);
				float extrabotz1 = extrabot->GetPointZ(*seg->v1);
				float extrabotz2 = extrabot->GetPointZ(*seg->v2);

				FreeWSurfs(sp->surfs);
				sp->surfs = NULL;

				float hdelta = extratopz2 - extratopz1;
				float offshdelta = hdelta * seg->offset / seg->length;
				sp->texinfo.texorg.z = sidedef->rowoffset - offshdelta + extratopz1;

				wv[0].x = wv[1].x = seg->v1->x;
				wv[0].y = wv[1].y = seg->v1->y;
				wv[2].x = wv[3].x = seg->v2->x;
				wv[2].y = wv[3].y = seg->v2->y;

				wv[0].z = MAX(extrabotz1, botz1);
				wv[1].z = MIN(extratopz1, topz1);
				wv[2].z = MIN(extratopz2, topz2);
				wv[3].z = MAX(extrabotz2, botz2);

				sp->surfs = CreateWSurfs(wv, &sp->texinfo, seg);

				sp->frontTopDist = r_ceiling->dist;
				sp->frontBotDist = r_floor->dist;
				sp->backTopDist = extratop->dist;
				sp->backBotDist = extrabot->dist;
				sp->rowoffset = sidedef->rowoffset;
			}
			else if (FASI(sp->rowoffset) != FASI(sidedef->rowoffset))
			{
				UpdateRowOffset(sp, sidedef->rowoffset);
			}
			if (FASI(sp->textureoffset) != FASI(sidedef->textureoffset))
			{
				UpdateTextureOffset(sp, seg, sidedef->textureoffset);
			}
			sp = sp->next;
		}
	}
}

//==========================================================================
//
//	R_SegMoved
//
//==========================================================================

void R_SegMoved(seg_t *seg)
{
	if (!seg->drawsegs)
	{
		//	Drawsegs not created yet
		return;
	}
	if (!seg->linedef)
	{
		Sys_Error("R_SegMoved: miniseg");
	}
	if (seg->backsector)
	{
		Sys_Error("R_SegMoved: two-sided");
	}

	seg->drawsegs->mid->texinfo.saxis = (*seg->v2 - *seg->v1) / seg->length;
	seg->drawsegs->mid->texinfo.taxis = TVec(0, 0, -1);

	//	Force update
	seg->drawsegs->mid->frontTopDist += 0.346;
	seg->drawsegs->mid->textureoffset += 0.46;
}

//==========================================================================
//
//	R_PreRender
//
//==========================================================================

void R_PreRender(void)
{
	int				i, j;
	int				count, dscount, spcount;
	subregion_t		*sreg;
	subsector_t		*sub;
	drawseg_t		*pds;
	sec_region_t	*reg;

	free_wsurfs = NULL;
	c_subdivides = 0;
	c_seg_div = 0;
	light_mem = 0;
	SetupSky();

	//	Count regions in all subsectors
	count = 0;
	dscount = 0;
	spcount = 0;
	for (i = 0; i < cl_level.numsubsectors; i++)
	{
		sub = &cl_level.subsectors[i];
		if (!sub->sector->linecount)
		{
			//	Skip sectors containing original polyobjs
			continue;
		}
		for (reg = sub->sector->botregion; reg; reg = reg->next)		
		{
			count++;
			dscount += sub->numlines;
			if (sub->poly)
			{
				//	Polyobj
				dscount += sub->poly->numsegs;
			}
			for (j = 0; j < sub->numlines; j++)
			{
				spcount += CountSegParts(&cl_level.segs[sub->firstline + j]);
			}
			if (sub->poly)
			{
				spcount += 2 * sub->poly->numsegs;
			}
		}
	}

	//	Get some memory
	sreg = (subregion_t*)Z_Calloc(count * sizeof(subregion_t), PU_LEVEL, 0);
	pds = (drawseg_t*)Z_Calloc(dscount * sizeof(drawseg_t), PU_LEVEL, 0);
	pspart = (segpart_t*)Z_Calloc(spcount * sizeof(segpart_t), PU_LEVEL, 0);

	//	Add dplanes
	for (i = 0; i < cl_level.numsubsectors; i++)
	{
		if (!(i & 7))
		{
			CL_KeepaliveMessage();
		}
		sub = &cl_level.subsectors[i];
		if (!sub->sector->linecount)
		{
			//	Skip sectors containing original polyobjs
			continue;
		}
		r_sub = sub;
		for (reg = sub->sector->botregion; reg; reg = reg->next)
		{
			sreg->secregion = reg;
			sreg->floorplane = reg->floor;
			sreg->ceilplane = reg->ceiling;
			sreg->floor = CreateSecSurface(sub, reg->floor);
			sreg->ceil = CreateSecSurface(sub, reg->ceiling);

			sreg->count = sub->numlines;
			if (sub->poly)
			{
				//	Polyobj
				sreg->count += sub->poly->numsegs;
			}
			sreg->lines = pds;
			pds += sreg->count;
			frontsector = sub->sector;
			r_floor = reg->floor;
			r_ceiling = reg->ceiling;
			for (j = 0; j < sub->numlines; j++)
			{
				CreateSegParts(&sreg->lines[j], &cl_level.segs[sub->firstline + j]);
			}
			if (sub->poly)
			{
				//	Polyobj
				int polyCount = sub->poly->numsegs;
				seg_t **polySeg = sub->poly->segs;
				while (polyCount--)
				{
					CreateSegParts(&sreg->lines[j], *polySeg);
					polySeg++;
					j++;
				}
			}

			sreg->next = sub->regions;
			sub->regions = sreg;
			sreg++;
		}
	}

	cond << c_subdivides << " subdivides\n";
	cond << c_seg_div << " seg subdivides\n";
	cond << (light_mem / 1024) << "k light mem\n";
}

//==========================================================================
//
//	UpdateSubRegion
//
//==========================================================================

static void UpdateSubRegion(subregion_t *region)
{
    int				count;
	int 			polyCount;
	seg_t**			polySeg;

	r_floor = region->floorplane;
	r_ceiling = region->ceilplane;

    count = r_sub->numlines;
    drawseg_t *ds = region->lines;
    while (count--)
    {
		UpdateDrawSeg(ds);
		ds++;
    }

	UpdateSecSurface(region->floor);
	UpdateSecSurface(region->ceil);

	if (r_sub->poly)
	{
		//	Update the polyobj
		polyCount = r_sub->poly->numsegs;
		polySeg = r_sub->poly->segs;
		while (polyCount--)
		{
			UpdateDrawSeg((*polySeg)->drawsegs);
			polySeg++;
		}
	}

	if (region->next)
	{
		UpdateSubRegion(region->next);
	}
}

//==========================================================================
//
//	UpdateSubsector
//
//==========================================================================

static void UpdateSubsector(int num, float *bbox)
{
//FIXME do this in node loading
#ifdef PARANOID
    if (num >= cl_level.numsubsectors)
		Sys_Error("UpdateSubsector: ss %i with numss = %i", num, cl_level.numsubsectors);
#endif

    r_sub = &cl_level.subsectors[num];
    frontsector = r_sub->sector;

	if (r_sub->visframe != r_visframecount)
 	{
		return;
	}

	if (!frontsector->linecount)
	{
		//	Skip sectors containing original polyobjs
		return;
	}

	bbox[2] = frontsector->floor.minz;
	if (frontsector->ceiling.pic == skyflatnum)
		bbox[5] = skyheight;
	else
		bbox[5] = frontsector->ceiling.maxz;

	UpdateSubRegion(r_sub->regions);
}

//==========================================================================
//
//	UpdateBSPNode
//
//==========================================================================

static void UpdateBSPNode(int bspnum, float *bbox)
{
    // Found a subsector?
    if (bspnum & NF_SUBSECTOR)
    {
		if (bspnum == -1)
		    UpdateSubsector(0, bbox);
		else
		    UpdateSubsector(bspnum & (~NF_SUBSECTOR), bbox);
		return;
    }
		
	node_t* bsp = &cl_level.nodes[bspnum];
    
	if (bsp->visframe != r_visframecount)
 	{
		return;
	}

	UpdateBSPNode(bsp->children[0], bsp->bbox[0]);
    UpdateBSPNode(bsp->children[1], bsp->bbox[1]);
	bbox[2] = MIN(bsp->bbox[0][2], bsp->bbox[1][2]);
	bbox[5] = MAX(bsp->bbox[0][5], bsp->bbox[1][5]);
}

//==========================================================================
//
//	R_UpdateWorld
//
//==========================================================================

void R_UpdateWorld(void)
{
	float	dummy_bbox[6] = {-99999, -99999, -99999, 99999, 9999, 99999};

	UpdateBSPNode(cl_level.numnodes - 1, dummy_bbox);	// head node is the last node output
}

//**************************************************************************
//
//	$Log$
//	Revision 1.7  2002/01/07 12:16:43  dj_jl
//	Changed copyright year
//
//	Revision 1.6  2001/12/01 17:51:46  dj_jl
//	Little changes to compile with MSVC
//	
//	Revision 1.5  2001/10/08 17:34:57  dj_jl
//	A lots of small changes and cleanups
//	
//	Revision 1.4  2001/08/30 17:36:47  dj_jl
//	Faster compares
//	
//	Revision 1.3  2001/07/31 17:16:31  dj_jl
//	Just moved Log to the end of file
//	
//	Revision 1.2  2001/07/27 14:27:54  dj_jl
//	Update with Id-s and Log-s, some fixes
//
//**************************************************************************
