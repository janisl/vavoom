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

static surface_t*	free_wsurfs;
static int			c_seg_div;
static void*		AllocatedWSurfBlocks;
static subregion_t*	AllocatedSubRegions;
static drawseg_t*	AllocatedDrawSegs;
static segpart_t*	AllocatedSegParts;

// CODE --------------------------------------------------------------------

//**************************************************************************
//
//	Scaling
//
//**************************************************************************

//==========================================================================
//
//	TextureSScale
//
//==========================================================================

inline float TextureSScale(int pic)
{
	return GTextureManager.Textures[pic]->SScale;
}

//==========================================================================
//
//	TextureTScale
//
//==========================================================================

inline float TextureTScale(int pic)
{
	return GTextureManager.Textures[pic]->TScale;
}

//==========================================================================
//
//	TextureOffsetSScale
//
//==========================================================================

inline float TextureOffsetSScale(int pic)
{
	if (GTextureManager.Textures[pic]->bWorldPanning)
		return GTextureManager.Textures[pic]->SScale;
	return 1.0;
}

//==========================================================================
//
//	TextureOffsetTScale
//
//==========================================================================

inline float TextureOffsetTScale(int pic)
{
	if (GTextureManager.Textures[pic]->bWorldPanning)
		return GTextureManager.Textures[pic]->TScale;
	return 1.0;
}

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

static void	SetupSky()
{
	guard(SetupSky);
	skyheight = -99999.0;
	for (int i = 0; i < GClLevel->NumSectors; i++)
	{
		if (GClLevel->Sectors[i].ceiling.pic == skyflatnum &&
			GClLevel->Sectors[i].ceiling.maxz > skyheight)
		{
			skyheight = GClLevel->Sectors[i].ceiling.maxz;
		}
	}
	sky_plane.Set(TVec(0, 0, -1), -skyheight);
	sky_plane.pic = skyflatnum;
	unguard;
}

//==========================================================================
//
//	InitSurfs
//
//==========================================================================

static void InitSurfs(surface_t* InSurfs, texinfo_t *texinfo, TPlane *plane)
{
	guard(InitSurfs);
	surface_t* surfs = InSurfs;
	int i;
	float dot;
	float mins;
	float maxs;
	int bmins;
	int bmaxs;

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
			dot = DotProduct(surfs->verts[i], texinfo->saxis) + texinfo->soffs;
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
			dot = DotProduct(surfs->verts[i], texinfo->taxis) + texinfo->toffs;
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
	unguard;
}

//==========================================================================
//
//	FlushSurfCaches
//
//==========================================================================

static void FlushSurfCaches(surface_t* InSurfs)
{
	guard(FlushSurfCaches);
	surface_t* surfs = InSurfs;
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
	unguard;
}

//==========================================================================
//
//	SubdivideFace
//
//==========================================================================

static surface_t* SubdivideFace(surface_t* InF, const TVec &axis,
	const TVec *nextaxis)
{
	guard(SubdivideFace);
	surface_t* f = InF;
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
	plane.normal = Normalise(plane.normal);
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
		(count2 - 1) * sizeof(TVec));
	back->count = count2;
	memcpy(back->verts, verts2, count2 * sizeof(TVec));

	surface_t *front = (surface_t*)Z_Calloc(sizeof(surface_t) +
		(count1 - 1) * sizeof(TVec));
	front->count = count1;
	memcpy(front->verts, verts1, count1 * sizeof(TVec));

	front->next = next;
	back->next = SubdivideFace(front, axis, nextaxis);
	if (nextaxis)
	{
		back = SubdivideFace(back, *nextaxis, NULL);
	}
	return back;
	unguard;
}

//==========================================================================
//
//	CreateSecSurface
//
//==========================================================================

static sec_surface_t *CreateSecSurface(subsector_t* sub, sec_plane_t* InSplane)
{
	guard(CreateSecSurface);
	sec_plane_t* splane = InSplane;
	sec_surface_t	*ssurf;
	surface_t		*surf;

	ssurf = new sec_surface_t;
	memset(ssurf, 0, sizeof(sec_surface_t));
	surf = (surface_t*)Z_Calloc(sizeof(surface_t) +
		(sub->numlines - 1) * sizeof(TVec));

	if (splane->pic == skyflatnum && splane->normal.z < 0.0)
	{
		splane = &sky_plane;
	}
	ssurf->secplane = splane;
	ssurf->dist = splane->dist;

	if (fabs(splane->normal.z) > 0.7)
	{
		ssurf->texinfo.saxis = TVec(1, 0, 0) * TextureSScale(splane->pic);
		ssurf->texinfo.taxis = TVec(0, -1, 0) * TextureTScale(splane->pic);
	}
	else
	{
		ssurf->texinfo.taxis = TVec(0, 0, -1) * TextureTScale(splane->pic);
		ssurf->texinfo.saxis = Normalise(CrossProduct(splane->normal, 
			ssurf->texinfo.taxis)) * TextureSScale(splane->pic);
	}
	ssurf->texinfo.soffs = splane->xoffs;
	ssurf->texinfo.toffs = splane->yoffs;
	ssurf->texinfo.pic = splane->pic;
	ssurf->texinfo.translucency = splane->translucency ? splane->translucency + 1 : 0;

	surf->count = sub->numlines;
	seg_t *line = &GClLevel->Segs[sub->firstline];
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
	unguard;
}

//==========================================================================
//
//	UpdateSecSurface
//
//==========================================================================

static void UpdateSecSurface(sec_surface_t *ssurf, sec_plane_t* RealPlane)
{
	guard(UpdateSecSurface);
	sec_plane_t		*plane = ssurf->secplane;

	if (!plane->pic)
	{
		return;
	}

	if (plane != RealPlane)
	{
		//	Check for sky changes.
		if (plane->pic == skyflatnum && RealPlane->pic != skyflatnum)
		{
			ssurf->secplane = RealPlane;
			plane = RealPlane;
			if (!ssurf->surfs->extents[0])
			{
				ssurf->surfs = SubdivideFace(ssurf->surfs,
					ssurf->texinfo.saxis, &ssurf->texinfo.taxis);
				InitSurfs(ssurf->surfs, &ssurf->texinfo, plane);
			}
		}
		else if (plane->pic != skyflatnum && RealPlane->pic == skyflatnum)
		{
			ssurf->secplane = &sky_plane;
			plane = &sky_plane;
		}
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
	if (FASI(ssurf->texinfo.soffs) != FASI(plane->xoffs) ||
		FASI(ssurf->texinfo.toffs) != FASI(plane->yoffs))
	{
		ssurf->texinfo.soffs = plane->xoffs;
		ssurf->texinfo.toffs = plane->yoffs;
		FlushSurfCaches(ssurf->surfs);
		InitSurfs(ssurf->surfs, &ssurf->texinfo, NULL);
	}
	if (ssurf->texinfo.pic != plane->pic)
	{
		ssurf->texinfo.pic = plane->pic;
	}
	unguard;
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

static surface_t *NewWSurf()
{
	guard(NewWSurf);
	if (!free_wsurfs)
	{
		//	Allocate some more surfs
		byte* tmp = (byte*)Z_Malloc(WSURFSIZE * 32 + sizeof(void*));
		*(void**)tmp = AllocatedWSurfBlocks;
		AllocatedWSurfBlocks = tmp;
		tmp += sizeof(void*);
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
	unguard;
}

//==========================================================================
//
//	FreeWSurfs
//
//==========================================================================

static void	FreeWSurfs(surface_t* InSurfs)
{
	guard(FreeWSurfs);
	surface_t* surfs = InSurfs;
	FlushSurfCaches(surfs);
	while (surfs)
	{
		if (surfs->lightmap)
		{
			Z_Free(surfs->lightmap);
		}
		if (surfs->lightmap_rgb)
		{
			Z_Free(surfs->lightmap_rgb);
		}
		surface_t *next = surfs->next;
		surfs->next = free_wsurfs;
		free_wsurfs = surfs;
		surfs = next;
	}
	unguard;
}

//==========================================================================
//
//	SubdivideSeg
//
//==========================================================================

static surface_t *SubdivideSeg(surface_t* InSurf, const TVec &axis, const TVec *nextaxis)
{
	guard(SubdivideSeg);
	surface_t* surf = InSurf;
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
	plane.normal = Normalise(plane.normal);
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
	unguard;
}

//==========================================================================
//
//	CreateWSurfs
//
//==========================================================================

static surface_t *CreateWSurfs(TVec *wv, texinfo_t *texinfo, seg_t *seg)
{
	guard(CreateWSurfs);
	if (wv[1].z <= wv[0].z && wv[2].z <= wv[3].z)
	{
		return NULL;
	}

	if (GTextureManager.Textures[texinfo->pic]->Type == TEXTYPE_Null)
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
	unguard;
}

//==========================================================================
//
//	CountSegParts
//
//==========================================================================

static int CountSegParts(seg_t *seg)
{
	guard(CountSegParts);
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
	unguard;
}

//==========================================================================
//
//	CreateSegParts
//
//==========================================================================

static segpart_t	*pspart;

static void CreateSegParts(drawseg_t* dseg, seg_t *seg)
{
	guard(CreateSegParts);
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

		sp->texinfo.saxis = segdir * TextureSScale(sidedef->midtexture);
		sp->texinfo.taxis = TVec(0, 0, -1) * TextureTScale(sidedef->midtexture);
		sp->texinfo.soffs = -DotProduct(*seg->v1, sp->texinfo.saxis) +
			seg->offset * TextureSScale(sidedef->midtexture) +
			sidedef->textureoffset * TextureOffsetSScale(sidedef->midtexture);
		sp->texinfo.pic = sidedef->midtexture;

		hdelta = topz2 - topz1;
		offshdelta = hdelta * seg->offset / seg->length;

		if (linedef->flags & ML_DONTPEGBOTTOM)
		{
			//	bottom of texture at bottom
			sp->texinfo.toffs = MIN(botz1, botz2) +
				GTextureManager.TextureHeight(sidedef->midtexture);
		}
		else if (linedef->flags & ML_DONTPEGTOP)
		{
			// top of texture at top of top region
			sp->texinfo.toffs =
				r_sub->sector->topregion->ceiling->GetPointZ(*seg->v1);
		}
		else
		{
			// top of texture at top
			sp->texinfo.toffs = topz1;
		}
		sp->texinfo.toffs -= offshdelta;
		sp->texinfo.toffs *= TextureTScale(sidedef->midtexture);
		sp->texinfo.toffs += sidedef->rowoffset * TextureOffsetTScale(sidedef->midtexture);

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

		sp->texinfo.saxis = segdir * TextureSScale(sidedef->toptexture);
		sp->texinfo.taxis = TVec(0, 0, -1) * TextureTScale(sidedef->toptexture);
		sp->texinfo.soffs = -DotProduct(*seg->v1, sp->texinfo.saxis) +
			seg->offset * TextureSScale(sidedef->toptexture) +
			sidedef->textureoffset * TextureOffsetSScale(sidedef->toptexture);
		sp->texinfo.pic = sidedef->toptexture;

		hdelta = topz2 - topz1;
		offshdelta = hdelta * seg->offset / seg->length;
		if (linedef->flags & ML_DONTPEGTOP)
		{
			// top of texture at top
			sp->texinfo.toffs = topz1;
		}
		else
		{
			// bottom of texture
			sp->texinfo.toffs = back_topz1 +
				GTextureManager.TextureHeight(sidedef->toptexture);
		}
		sp->texinfo.toffs -= offshdelta;
		sp->texinfo.toffs *= TextureTScale(sidedef->toptexture);
		sp->texinfo.toffs += sidedef->rowoffset * TextureOffsetTScale(sidedef->toptexture);

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

		sp->texinfo.saxis = segdir * TextureSScale(sidedef->bottomtexture);
		sp->texinfo.taxis = TVec(0, 0, -1) * TextureTScale(sidedef->bottomtexture);
		sp->texinfo.soffs = -DotProduct(*seg->v1, sp->texinfo.saxis) +
			seg->offset * TextureSScale(sidedef->bottomtexture) +
			sidedef->textureoffset * TextureOffsetSScale(sidedef->bottomtexture);
		sp->texinfo.pic = sidedef->bottomtexture;

		hdelta = back_botz2 - back_botz1;
		offshdelta = hdelta * seg->offset / seg->length;
		if (linedef->flags & ML_DONTPEGBOTTOM)
		{
			// bottom of texture at bottom
			// top of texture at top
			sp->texinfo.toffs = topz1;
		}
		else
		{
			// top of texture at top
			sp->texinfo.toffs = back_botz1;
		}
		sp->texinfo.toffs -= offshdelta;
		sp->texinfo.toffs *= TextureTScale(sidedef->bottomtexture);
		sp->texinfo.toffs += sidedef->rowoffset * TextureOffsetTScale(sidedef->bottomtexture);

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

		if (sidedef->midtexture)
		{
			// masked midtexture
			float texh = GTextureManager.TextureHeight(sidedef->midtexture);
			hdelta = midtopz2 - midtopz1;
			offshdelta = hdelta * seg->offset / seg->length;

			float			z_org;

			sp->texinfo.saxis = segdir * TextureSScale(sidedef->midtexture);
			sp->texinfo.taxis = TVec(0, 0, -1) * TextureTScale(sidedef->midtexture);
			sp->texinfo.soffs = -DotProduct(*seg->v1, sp->texinfo.saxis) +
				seg->offset * TextureSScale(sidedef->midtexture) +
				sidedef->textureoffset * TextureOffsetSScale(sidedef->midtexture);
			sp->texinfo.translucency = linedef->translucency + 1;

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
			z_org -= offshdelta;

			sp->texinfo.toffs = z_org * TextureTScale(sidedef->midtexture) +
				sidedef->rowoffset * TextureOffsetTScale(sidedef->midtexture);
			sp->texinfo.pic = sidedef->midtexture;

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
			side_t *extraside = &GClLevel->Sides[reg->prev->extraline->sidenum[0]];

			float extratopz1 = extratop->GetPointZ(*seg->v1);
			float extratopz2 = extratop->GetPointZ(*seg->v2);
			float extrabotz1 = extrabot->GetPointZ(*seg->v1);
			float extrabotz2 = extrabot->GetPointZ(*seg->v2);

			sp->texinfo.saxis = segdir * TextureSScale(extraside->midtexture);
			sp->texinfo.taxis = TVec(0, 0, -1) * TextureTScale(extraside->midtexture);
			sp->texinfo.soffs = -DotProduct(*seg->v1, sp->texinfo.saxis) +
				seg->offset * TextureSScale(extraside->midtexture) +
				sidedef->textureoffset * TextureOffsetSScale(extraside->midtexture);

			hdelta = extratopz2 - extratopz1;
			offshdelta = hdelta * seg->offset / seg->length;
			sp->texinfo.toffs = (extratopz1 - offshdelta) *
				TextureTScale(extraside->midtexture) + sidedef->rowoffset *
				TextureOffsetTScale(extraside->midtexture);
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
	unguard;
}

//==========================================================================
//
//	UpdateRowOffset
//
//==========================================================================

static void	UpdateRowOffset(segpart_t *sp, float rowoffset)
{
	guard(UpdateRowOffset);
	sp->texinfo.toffs += (rowoffset - sp->rowoffset) *
		TextureOffsetTScale(sp->texinfo.pic);
	sp->rowoffset = rowoffset;
	FlushSurfCaches(sp->surfs);
	InitSurfs(sp->surfs, &sp->texinfo, NULL);
	unguard;
}

//==========================================================================
//
//	UpdateTextureOffset
//
//==========================================================================

static void UpdateTextureOffset(segpart_t *sp, float textureoffset)
{
	guard(UpdateTextureOffset);
	sp->texinfo.soffs += (textureoffset - sp->textureoffset) *
		TextureOffsetSScale(sp->texinfo.pic);
	sp->textureoffset = textureoffset;
	FlushSurfCaches(sp->surfs);
	InitSurfs(sp->surfs, &sp->texinfo, NULL);
	unguard;
}

//==========================================================================
//
//	UpdateDrawSeg
//
//==========================================================================

static void UpdateDrawSeg(drawseg_t* dseg)
{
	guard(UpdateDrawSeg);
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
				sp->texinfo.toffs = MIN(botz1, botz2) +
					GTextureManager.TextureHeight(sidedef->midtexture);
			}
			else if (linedef->flags & ML_DONTPEGTOP)
			{
				// top of texture at top of top region
				sp->texinfo.toffs =
					r_sub->sector->topregion->ceiling->GetPointZ(*seg->v1);
			}
			else
			{
				// top of texture at top
				sp->texinfo.toffs = topz1;
			}
			sp->texinfo.toffs -= offshdelta;
			sp->texinfo.toffs *= TextureTScale(sidedef->midtexture);
			sp->texinfo.toffs += sidedef->rowoffset * TextureOffsetTScale(sidedef->midtexture);
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
			UpdateTextureOffset(sp, sidedef->textureoffset);
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
				sp->texinfo.toffs = topz1;
			}
			else
			{
				// bottom of texture
				sp->texinfo.toffs = back_topz1 +
					GTextureManager.TextureHeight(sidedef->toptexture);
			}
			sp->texinfo.toffs -= offshdelta;
			sp->texinfo.toffs *= TextureTScale(sidedef->toptexture);
			sp->texinfo.toffs += sidedef->rowoffset * TextureOffsetTScale(sidedef->toptexture);
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
			UpdateTextureOffset(sp, sidedef->textureoffset);
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
				sp->texinfo.toffs = topz1;
			}
			else
			{
				// top of texture at top
				sp->texinfo.toffs = back_botz1;
			}
			sp->texinfo.toffs -= offshdelta;
			sp->texinfo.toffs *= TextureTScale(sidedef->bottomtexture);
			sp->texinfo.toffs += sidedef->rowoffset *
				TextureOffsetTScale(sidedef->bottomtexture);

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
			UpdateTextureOffset(sp, sidedef->textureoffset);
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

				float texh = GTextureManager.TextureHeight(sidedef->midtexture);
				float hdelta = midtopz2 - midtopz1;
				float offshdelta = hdelta * seg->offset / seg->length;

				float			z_org;

				TVec segdir = (*seg->v2 - *seg->v1) / seg->length;

				sp->texinfo.saxis = segdir * TextureSScale(sidedef->midtexture);
				sp->texinfo.taxis = TVec(0, 0, -1) * TextureTScale(sidedef->midtexture);
				sp->texinfo.soffs = -DotProduct(*seg->v1, sp->texinfo.saxis) +
					seg->offset * TextureSScale(sidedef->midtexture) +
					sidedef->textureoffset * TextureOffsetSScale(sidedef->midtexture);
				sp->texinfo.translucency = linedef->translucency + 1;

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

				sp->texinfo.toffs = z_org * TextureTScale(sidedef->midtexture);

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
			else
			{
				sp->texinfo.translucency = 0;
			}

			sp->frontTopDist = r_ceiling->dist;
			sp->frontBotDist = r_floor->dist;
			sp->backTopDist = r_back_ceiling->dist;
			sp->backBotDist = r_back_floor->dist;
			sp->rowoffset = sidedef->rowoffset;
		}
		if (FASI(sp->textureoffset) != FASI(sidedef->textureoffset))
		{
			UpdateTextureOffset(sp, sidedef->textureoffset);
		}

		sec_region_t *reg;
		segpart_t *sp = dseg->extra;
		for (reg = backsector->botregion; reg->next; reg = reg->next)
		{
			TPlane *extratop = reg->next->floor;
			TPlane *extrabot = reg->ceiling;
			side_t *extraside = &GClLevel->Sides[reg->extraline->sidenum[0]];

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
				sp->texinfo.toffs = (extratopz1 - offshdelta) *
					TextureTScale(sidedef->midtexture) + sidedef->rowoffset *
					TextureOffsetTScale(sidedef->midtexture);

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
				UpdateTextureOffset(sp, sidedef->textureoffset);
			}
			sp = sp->next;
		}
	}
	unguard;
}

//==========================================================================
//
//	R_SegMoved
//
//==========================================================================

void R_SegMoved(seg_t *seg)
{
	guard(R_SegMoved);
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

	seg->drawsegs->mid->texinfo.saxis = (*seg->v2 - *seg->v1) / seg->length *
		TextureSScale(seg->drawsegs->mid->texinfo.pic);
	seg->drawsegs->mid->texinfo.soffs = -DotProduct(*seg->v1,
		seg->drawsegs->mid->texinfo.saxis) +
		seg->offset * TextureSScale(seg->sidedef->midtexture) +
		seg->sidedef->textureoffset * TextureOffsetSScale(seg->sidedef->midtexture);

	//	Force update
	seg->drawsegs->mid->frontTopDist += 0.346;
	unguard;
}

//==========================================================================
//
//	R_PreRender
//
//==========================================================================

void R_PreRender()
{
	guard(R_PreRender);
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
	for (i = 0; i < GClLevel->NumSubsectors; i++)
	{
		sub = &GClLevel->Subsectors[i];
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
				spcount += CountSegParts(&GClLevel->Segs[sub->firstline + j]);
			}
			if (sub->poly)
			{
				spcount += 2 * sub->poly->numsegs;
			}
		}
	}

	//	Get some memory
	sreg = new subregion_t[count];
	pds = new drawseg_t[dscount];
	pspart = new segpart_t[spcount];
	memset(sreg, 0, sizeof(subregion_t) * count);
	memset(pds, 0, sizeof(drawseg_t) * dscount);
	memset(pspart, 0, sizeof(segpart_t) * spcount);
	AllocatedSubRegions = sreg;
	AllocatedDrawSegs = pds;
	AllocatedSegParts = pspart;

	//	Add dplanes
	for (i = 0; i < GClLevel->NumSubsectors; i++)
	{
		if (!(i & 7))
		{
			CL_KeepaliveMessage();
		}
		sub = &GClLevel->Subsectors[i];
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
				CreateSegParts(&sreg->lines[j], &GClLevel->Segs[sub->firstline + j]);
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

	GCon->Logf(NAME_Dev, "%d subdivides", c_subdivides);
	GCon->Logf(NAME_Dev, "%d seg subdivides", c_seg_div);
	GCon->Logf(NAME_Dev, "%dk light mem", light_mem / 1024);
	unguard;
}

//==========================================================================
//
//	UpdateSubRegion
//
//==========================================================================

static void UpdateSubRegion(subregion_t *region)
{
	guard(UpdateSubRegion);
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

	UpdateSecSurface(region->floor, region->floorplane);
	UpdateSecSurface(region->ceil, region->ceilplane);

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
	unguard;
}

//==========================================================================
//
//	UpdateSubsector
//
//==========================================================================

static void UpdateSubsector(int num, float *bbox)
{
	guard(UpdateSubsector);
	r_sub = &GClLevel->Subsectors[num];
	frontsector = r_sub->sector;

	if (r_sub->VisFrame != r_visframecount)
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
	unguard;
}

//==========================================================================
//
//	UpdateBSPNode
//
//==========================================================================

static void UpdateBSPNode(int bspnum, float *bbox)
{
	guard(UpdateBSPNode);
	// Found a subsector?
	if (bspnum & NF_SUBSECTOR)
	{
		if (bspnum == -1)
			UpdateSubsector(0, bbox);
		else
			UpdateSubsector(bspnum & (~NF_SUBSECTOR), bbox);
		return;
	}

	node_t* bsp = &GClLevel->Nodes[bspnum];

	if (bsp->VisFrame != r_visframecount)
	{
		return;
	}

	UpdateBSPNode(bsp->children[0], bsp->bbox[0]);
	UpdateBSPNode(bsp->children[1], bsp->bbox[1]);
	bbox[2] = MIN(bsp->bbox[0][2], bsp->bbox[1][2]);
	bbox[5] = MAX(bsp->bbox[0][5], bsp->bbox[1][5]);
	unguard;
}

//==========================================================================
//
//	CopyPlaneIfValid
//
//==========================================================================

static bool CopyPlaneIfValid(sec_plane_t* dest, const sec_plane_t* source,
	const sec_plane_t* opp)
{
	guard(CopyPlaneIfValid);
	bool copy = false;

	// If the planes do not have matching slopes, then always copy them
	// because clipping would require creating new sectors.
	if (source->normal != dest->normal)
	{
		copy = true;
	}
	else if (opp->normal != -dest->normal)
	{
		if (source->dist < dest->dist)
		{
			copy = true;
		}
	}
	else if (source->dist < dest->dist && source->dist > -opp->dist)
	{
		copy = true;
	}

	if (copy)
	{
		*(TPlane*)dest = *(TPlane*)source;
	}

	return copy;
	unguard;
}

//==========================================================================
//
//	UpdateFakeFlats
//
// killough 3/7/98: Hack floor/ceiling heights for deep water etc.
//
// If player's view height is underneath fake floor, lower the
// drawn ceiling to be just under the floor height, and replace
// the drawn floor and ceiling textures, and light level, with
// the control sector's.
//
// Similar for ceiling, only reflected.
//
// killough 4/11/98, 4/13/98: fix bugs, add 'back' parameter
//
//==========================================================================

void UpdateFakeFlats(sector_t* sec)
{
	guard(UpdateFakeFlats);
	const sector_t *s = sec->heightsec;
	sector_t *heightsec = r_viewleaf->sector->heightsec;
	bool underwater = /*r_fakingunderwater ||*/
		(heightsec && vieworg.z <= heightsec->floor.GetPointZ(vieworg));
	bool doorunderwater = false;
	int diffTex = !!(s->SectorFlags & sector_t::SF_ClipFakePlanes);

	// Replace sector being drawn with a copy to be hacked
	fakefloor_t* ff = sec->fakefloors;
	ff->floorplane = sec->floor;
	ff->ceilplane = sec->ceiling;
	ff->params = sec->params;

	// Replace floor and ceiling height with control sector's heights.
	if (diffTex)
	{
		if (CopyPlaneIfValid(&ff->floorplane, &s->floor, &sec->ceiling))
		{
			ff->floorplane.pic = s->floor.pic;
		}
		else if (s->SectorFlags & sector_t::SF_FakeFloorOnly)
		{
			if (underwater)
			{
//				tempsec->ColourMap = s->ColourMap;
				if (!(s->SectorFlags & sector_t::SF_NoFakeLight))
				{
					ff->params.lightlevel = s->params.lightlevel;

/*					if (floorlightlevel != NULL)
					{
						*floorlightlevel = GetFloorLight (s);
					}

					if (ceilinglightlevel != NULL)
					{
						*ceilinglightlevel = GetFloorLight (s);
					}*/
				}
				return;
			}
			return;
		}
	}
	else
	{
		ff->floorplane.normal = s->floor.normal;
		ff->floorplane.dist = s->floor.dist;
	}

	if (!(s->SectorFlags & sector_t::SF_FakeFloorOnly))
	{
		if (diffTex)
		{
			if (CopyPlaneIfValid(&ff->ceilplane, &s->ceiling, &sec->floor))
			{
				ff->ceilplane.pic = s->ceiling.pic;
			}
		}
		else
		{
			ff->ceilplane.normal = s->ceiling.normal;
			ff->ceilplane.dist = s->ceiling.dist;
		}
	}

//	float refflorz = s->floor.GetPointZ(viewx, viewy);
	float refceilz = s->ceiling.GetPointZ(vieworg);
//	float orgflorz = sec->floor.GetPointZ(viewx, viewy);
	float orgceilz = sec->ceiling.GetPointZ(vieworg);

#if 0//1
	// [RH] Allow viewing underwater areas through doors/windows that
	// are underwater but not in a water sector themselves.
	// Only works if you cannot see the top surface of any deep water
	// sectors at the same time.
	if (back && !r_fakingunderwater && curline->frontsector->heightsec == NULL)
	{
		if (rw_frontcz1 <= s->floorplane.ZatPoint (curline->v1->x, curline->v1->y) &&
			rw_frontcz2 <= s->floorplane.ZatPoint (curline->v2->x, curline->v2->y))
		{
			// Check that the window is actually visible
			for (int z = WallSX1; z < WallSX2; ++z)
			{
				if (floorclip[z] > ceilingclip[z])
				{
					doorunderwater = true;
					r_fakingunderwater = true;
					break;
				}
			}
		}
	}
#endif

	if (underwater || doorunderwater)
	{
		ff->floorplane.normal = sec->floor.normal;
		ff->floorplane.dist = sec->floor.dist;
		ff->ceilplane.normal = -s->floor.normal;
		ff->ceilplane.dist = -s->floor.dist/* + 1*/;
//		ff->ColourMap = s->ColourMap;
	}

	// killough 11/98: prevent sudden light changes from non-water sectors:
	if ((underwater/* && !back*/) || doorunderwater)
	{
		// head-below-floor hack
		ff->floorplane.pic			= diffTex ? sec->floor.pic : s->floor.pic;
		ff->floorplane.xoffs		= s->floor.xoffs;
		ff->floorplane.yoffs		= s->floor.yoffs;
//		tempsec->floor_xscale		= s->floor_xscale;
//		tempsec->floor_yscale		= s->floor_yscale;
//		tempsec->floor_angle		= s->floor_angle;
//		tempsec->base_floor_angle	= s->base_floor_angle;
//		tempsec->base_floor_yoffs	= s->base_floor_yoffs;

		ff->ceilplane.normal		= -s->floor.normal;
		ff->ceilplane.dist			= -s->floor.dist/* + 1*/;
		if (s->ceiling.pic == skyflatnum)
		{
			ff->floorplane.normal	= -ff->ceilplane.normal;
			ff->floorplane.dist		= -ff->ceilplane.dist/* + 1*/;
			ff->ceilplane.pic		= ff->floorplane.pic;
			ff->ceilplane.xoffs		= ff->floorplane.xoffs;
			ff->ceilplane.yoffs		= ff->floorplane.yoffs;
//			ff->ceilplane.xscale	= ff->floorplane.xscale;
//			ff->ceilplane.yscale	= ff->floorplane.yscale;
//			ff->ceilplane.angle		= ff->floorplane.angle;
//			ff->base_ceiling_angle	= ff->base_floor_angle;
//			ff->base_ceiling_yoffs	= ff->base_floor_yoffs;
		}
		else
		{
			ff->ceilplane.pic		= diffTex ? s->floor.pic : s->ceiling.pic;
			ff->ceilplane.xoffs		= s->ceiling.xoffs;
			ff->ceilplane.yoffs		= s->ceiling.yoffs;
//			ff->ceilplane.xscale	= s->ceiling.xscale;
//			ff->ceilplane.yscale	= s->ceiling.yscale;
//			ff->ceilplane.angle		= s->ceiling.angle;
//			ff->base_ceiling_angle	= s->base_ceiling_angle;
//			ff->base_ceiling_yoffs	= s->base_ceiling_yoffs;
		}

		if (!(s->SectorFlags & sector_t::SF_NoFakeLight))
		{
			ff->params.lightlevel = s->params.lightlevel;

/*			if (floorlightlevel != NULL)
			{
				*floorlightlevel = GetFloorLight (s);
			}

			if (ceilinglightlevel != NULL)
			{
				*ceilinglightlevel = GetFloorLight (s);
			}*/
		}
	}
	else if (heightsec && orgceilz > refceilz &&
		!(s->SectorFlags & sector_t::SF_FakeFloorOnly) &&
		vieworg.z >= heightsec->ceiling.GetPointZ(vieworg))
	{
		// Above-ceiling hack
		ff->ceilplane.normal	= s->ceiling.normal;
		ff->ceilplane.dist		= s->ceiling.dist;
		ff->floorplane.normal	= -s->ceiling.normal;
		ff->floorplane.dist		= -s->ceiling.dist/* + 1*/;
//		ff->ColourMap			= s->ColourMap;

		ff->ceilplane.pic = diffTex ? sec->ceiling.pic : s->ceiling.pic;
		ff->floorplane.pic									= s->ceiling.pic;
		ff->floorplane.xoffs	= ff->ceilplane.xoffs		= s->ceiling.xoffs;
		ff->floorplane.yoffs	= ff->ceilplane.yoffs		= s->ceiling.yoffs;
//		ff->floorplane.xscale	= ff->ceilplane.xscale		= s->ceiling.xscale;
//		ff->floorplane.yscale	= ff->ceilplane.yscale		= s->ceiling.yscale;
//		ff->floorplane.angle	= ff->ceilplane.angle		= s->ceiling.angle;
//		ff->base_floor_angle	= ff->base_ceiling_angle	= s->base_ceiling_angle;
//		ff->base_floor_yoffs	= ff->base_ceiling_yoffs	= s->base_ceiling_yoffs;

		if (s->floor.pic != skyflatnum)
		{
			ff->ceilplane.normal	= sec->ceiling.normal;
			ff->ceilplane.dist		= sec->ceiling.dist;
			ff->floorplane.pic		= s->floor.pic;
			ff->floorplane.xoffs	= s->floor.xoffs;
			ff->floorplane.yoffs	= s->floor.yoffs;
//			ff->floorplane.xscale	= s->floor.xscale;
//			ff->floorplane.yscale	= s->floor.yscale;
//			ff->floorplane.angle	= s->floor.angle;
		}

		if (!(s->SectorFlags & sector_t::SF_NoFakeLight))
		{
			ff->params.lightlevel  = s->params.lightlevel;

/*			if (floorlightlevel != NULL)
			{
				*floorlightlevel = GetFloorLight (s);
			}

			if (ceilinglightlevel != NULL)
			{
				*ceilinglightlevel = GetCeilingLight (s);
			}*/
		}
	}
	unguard;
}

//==========================================================================
//
//	R_UpdateWorld
//
//==========================================================================

void R_UpdateWorld()
{
	guard(R_UpdateWorld);
	float	dummy_bbox[6] = {-99999, -99999, -99999, 99999, 9999, 99999};

	//	Update fake sectors.
	for (int i = 0; i < GClLevel->NumSectors; i++)
	{
		if (GClLevel->Sectors[i].heightsec &&
			!(GClLevel->Sectors[i].heightsec->SectorFlags & sector_t::SF_IgnoreHeightSec))
		{
			UpdateFakeFlats(&GClLevel->Sectors[i]);
		}
	}

	UpdateBSPNode(GClLevel->NumNodes - 1, dummy_bbox);	// head node is the last node output
	unguard;
}

//==========================================================================
//
//	R_SetupFakeFloors
//
//==========================================================================

void R_SetupFakeFloors(sector_t* Sec)
{
	sector_t* HeightSec = Sec->heightsec;

	if (HeightSec->SectorFlags & sector_t::SF_IgnoreHeightSec)
	{
		return;
	}

	Sec->fakefloors = new fakefloor_t;
	memset(Sec->fakefloors, 0, sizeof(fakefloor_t));
	Sec->fakefloors->floorplane = Sec->floor;
	Sec->fakefloors->ceilplane = Sec->ceiling;
	Sec->fakefloors->params = Sec->params;

	Sec->botregion->floor = &Sec->fakefloors->floorplane;
	Sec->topregion->ceiling = &Sec->fakefloors->ceilplane;
	Sec->topregion->params = &Sec->fakefloors->params;
}

//==========================================================================
//
//	FreeSurfaces
//
//==========================================================================

static void FreeSurfaces(surface_t* InSurf)
{
	guard(FreeSurfaces);
	surface_t* next;
	for (surface_t* s = InSurf; s; s = next)
	{
		for (int i = 0; i < 4; i++)
		{
			if (s->cachespots[i])
			{
				Drawer->FreeSurfCache(s->cachespots[i]);
			}
		}
		if (s->lightmap)
		{
			Z_Free(s->lightmap);
		}
		if (s->lightmap_rgb)
		{
			Z_Free(s->lightmap_rgb);
		}
		next = s->next;
		Z_Free(s);
	}
	unguard;
}

//==========================================================================
//
//	FreeSegParts
//
//==========================================================================

static void FreeSegParts(segpart_t* InSP)
{
	guard(FreeSegParts);
	for (segpart_t* sp = InSP; sp; sp = sp->next)
	{
		FreeWSurfs(sp->surfs);
	}
	unguard;
}

//==========================================================================
//
//	R_FreeLevelData
//
//==========================================================================

void R_FreeLevelData()
{
	guard(R_FreeLevelData);
	//	Free fake floor data.
	for (int i = 0; i < GClLevel->NumSectors; i++)
	{
		if (GClLevel->Sectors[i].fakefloors)
		{
			delete GClLevel->Sectors[i].fakefloors;
		}
	}

	for (int i = 0; i < GClLevel->NumSubsectors; i++)
	{
		for (subregion_t* r = GClLevel->Subsectors[i].regions; r; r = r->next)
		{
			FreeSurfaces(r->floor->surfs);
			delete r->floor;
			FreeSurfaces(r->ceil->surfs);
			delete r->ceil;
		}
	}

	//	Free seg parts.
	for (int i = 0; i < GClLevel->NumSegs; i++)
	{
		for (drawseg_t* ds = GClLevel->Segs[i].drawsegs; ds; ds = ds->next)
		{
			FreeSegParts(ds->top);
			FreeSegParts(ds->mid);
			FreeSegParts(ds->bot);
			FreeSegParts(ds->topsky);
			FreeSegParts(ds->extra);
		}
	}
	//	Free allocated wall surface blocks.
	for (void* Block = AllocatedWSurfBlocks; Block;)
	{
		void* Next = *(void**)Block;
		Z_Free(Block);
		Block = Next;
	}
	AllocatedWSurfBlocks = NULL;

	//	Free big blocks.
	delete[] AllocatedSubRegions;
	delete[] AllocatedDrawSegs;
	delete[] AllocatedSegParts;

	R_FreeLevelSkyData();
	unguard;
}
