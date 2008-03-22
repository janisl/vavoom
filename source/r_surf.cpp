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

static subsector_t*	r_sub;
static sec_plane_t*	r_floor;
static sec_plane_t*	r_ceiling;

static segpart_t*	pspart;

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

inline float TextureSScale(VTexture* pic)
{
	return pic->SScale;
}

//==========================================================================
//
//	TextureTScale
//
//==========================================================================

inline float TextureTScale(VTexture* pic)
{
	return pic->TScale;
}

//==========================================================================
//
//	TextureOffsetSScale
//
//==========================================================================

inline float TextureOffsetSScale(VTexture* pic)
{
	if (pic->bWorldPanning)
		return pic->SScale;
	return 1.0;
}

//==========================================================================
//
//	TextureOffsetTScale
//
//==========================================================================

inline float TextureOffsetTScale(VTexture* pic)
{
	if (pic->bWorldPanning)
		return pic->TScale;
	return 1.0;
}

//**************************************************************************
//**
//**	Sector surfaces
//**
//**************************************************************************

//==========================================================================
//
//	VRenderLevel::SetupSky
//
//==========================================================================

void VRenderLevel::SetupSky()
{
	guard(VRenderLevel::SetupSky);
	skyheight = -99999.0;
	for (int i = 0; i < Level->NumSectors; i++)
	{
		if (Level->Sectors[i].ceiling.pic == skyflatnum &&
			Level->Sectors[i].ceiling.maxz > skyheight)
		{
			skyheight = Level->Sectors[i].ceiling.maxz;
		}
	}
	//	Make it a bit higher to avoid clipping of the sprites.
	skyheight += 256;
	sky_plane.Set(TVec(0, 0, -1), -skyheight);
	sky_plane.pic = skyflatnum;
	sky_plane.Alpha = 1.1;
	unguard;
}

//==========================================================================
//
//	VRenderLevel::InitSurfs
//
//==========================================================================

void VRenderLevel::InitSurfs(surface_t* ASurfs, texinfo_t *texinfo,
	TPlane *plane, subsector_t* sub)
{
	guard(VRenderLevel::InitSurfs);
	surface_t* surfs = ASurfs;
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

		LightFace(surfs, sub);

		surfs = surfs->next;
	}
	unguard;
}

//==========================================================================
//
//	VRenderLevel::FlushSurfCaches
//
//==========================================================================

void VRenderLevel::FlushSurfCaches(surface_t* InSurfs)
{
	guard(VRenderLevel::FlushSurfCaches);
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
//	VRenderLevel::SubdivideFace
//
//==========================================================================

surface_t* VRenderLevel::SubdivideFace(surface_t* InF, const TVec &axis,
	const TVec *nextaxis)
{
	guard(VRenderLevel::SubdivideFace);
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
//	VRenderLevel::CreateSecSurface
//
//==========================================================================

sec_surface_t* VRenderLevel::CreateSecSurface(subsector_t* sub,
	sec_plane_t* InSplane)
{
	guard(VRenderLevel::CreateSecSurface);
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

	VTexture* Tex = GTextureManager(splane->pic);
	if (fabs(splane->normal.z) > 0.1)
	{
		ssurf->texinfo.saxis = TVec(mcos(splane->BaseAngle - splane->Angle),
			msin(splane->BaseAngle - splane->Angle), 0) * TextureSScale(Tex) *
			splane->XScale;
		ssurf->texinfo.taxis = TVec(msin(splane->BaseAngle - splane->Angle),
			-mcos(splane->BaseAngle - splane->Angle), 0) * TextureTScale(Tex) *
			splane->YScale;
	}
	else
	{
		ssurf->texinfo.taxis = TVec(0, 0, -1) * TextureTScale(Tex) *
			splane->YScale;
		ssurf->texinfo.saxis = Normalise(CrossProduct(splane->normal,
			ssurf->texinfo.taxis)) * TextureSScale(Tex) * splane->XScale;
	}
	ssurf->texinfo.soffs = splane->xoffs;
	ssurf->texinfo.toffs = splane->yoffs + splane->BaseYOffs;
	ssurf->texinfo.Tex = Tex;
	ssurf->texinfo.Alpha = splane->Alpha < 1.0 ? splane->Alpha : 1.1;
	ssurf->texinfo.Additive = false;
	ssurf->XScale = splane->XScale;
	ssurf->YScale = splane->YScale;
	ssurf->Angle = splane->BaseAngle - splane->Angle;

	surf->count = sub->numlines;
	seg_t *line = &Level->Segs[sub->firstline];
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
		surf->texinfo = &ssurf->texinfo;
		surf->plane = splane;
	}
	else
	{
		ssurf->surfs = SubdivideFace(surf, ssurf->texinfo.saxis,
			&ssurf->texinfo.taxis);
		InitSurfs(ssurf->surfs, &ssurf->texinfo, splane, sub);
	}

	return ssurf;
	unguard;
}

//==========================================================================
//
//	VRenderLevel::UpdateSecSurface
//
//==========================================================================

void VRenderLevel::UpdateSecSurface(sec_surface_t *ssurf,
	sec_plane_t* RealPlane, subsector_t* sub)
{
	guard(VRenderLevel::UpdateSecSurface);
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
				InitSurfs(ssurf->surfs, &ssurf->texinfo, plane, sub);
			}
		}
		else if (plane->pic != skyflatnum && RealPlane->pic == skyflatnum)
		{
			ssurf->secplane = &sky_plane;
			plane = &sky_plane;
		}
	}

	if (ssurf->texinfo.Tex != GTextureManager(plane->pic))
	{
		ssurf->texinfo.Tex = GTextureManager(plane->pic);
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
		if (plane->pic != skyflatnum)
		{
			FlushSurfCaches(ssurf->surfs);
			InitSurfs(ssurf->surfs, &ssurf->texinfo, NULL, sub);
		}
	}
	if (FASI(ssurf->XScale) != FASI(plane->XScale) ||
		FASI(ssurf->YScale) != FASI(plane->YScale) ||
		ssurf->Angle != plane->BaseAngle - plane->Angle)
	{
		if (fabs(plane->normal.z) > 0.1)
		{
			ssurf->texinfo.saxis = TVec(mcos(plane->BaseAngle - plane->Angle),
				msin(plane->BaseAngle - plane->Angle), 0) *
				TextureSScale(ssurf->texinfo.Tex) * plane->XScale;
			ssurf->texinfo.taxis = TVec(msin(plane->BaseAngle - plane->Angle),
				-mcos(plane->BaseAngle - plane->Angle), 0) *
				TextureTScale(ssurf->texinfo.Tex) * plane->YScale;
		}
		else
		{
			ssurf->texinfo.taxis = TVec(0, 0, -1) *
				TextureTScale(ssurf->texinfo.Tex) * plane->YScale;
			ssurf->texinfo.saxis = Normalise(CrossProduct(plane->normal,
				ssurf->texinfo.taxis)) * TextureSScale(ssurf->texinfo.Tex) *
				plane->XScale;
		}
		ssurf->texinfo.soffs = plane->xoffs;
		ssurf->texinfo.toffs = plane->yoffs + plane->BaseYOffs;
		ssurf->XScale = plane->XScale;
		ssurf->YScale = plane->YScale;
		ssurf->Angle = plane->BaseAngle - plane->Angle;
		if (plane->pic != skyflatnum)
		{
			FreeSurfaces(ssurf->surfs);
			surface_t* surf = (surface_t*)Z_Calloc(sizeof(surface_t) +
				(sub->numlines - 1) * sizeof(TVec));
			surf->count = sub->numlines;
			seg_t* line = &Level->Segs[sub->firstline];
			int vlindex = (plane->normal.z < 0);
			for (int i = 0; i < surf->count; i++)
			{
				TVec &v = *line[vlindex ? surf->count - i - 1 : i].v1;
				TVec &dst = surf->verts[i];
				dst = v;
				dst.z = plane->GetPointZ(dst);
			}
			ssurf->surfs = SubdivideFace(surf, ssurf->texinfo.saxis,
				&ssurf->texinfo.taxis);
			InitSurfs(ssurf->surfs, &ssurf->texinfo, plane, sub);
		}
	}
	else if (FASI(ssurf->texinfo.soffs) != FASI(plane->xoffs) ||
		ssurf->texinfo.toffs != plane->yoffs + plane->BaseYOffs)
	{
		ssurf->texinfo.soffs = plane->xoffs;
		ssurf->texinfo.toffs = plane->yoffs + plane->BaseYOffs;
		if (plane->pic != skyflatnum)
		{
			FlushSurfCaches(ssurf->surfs);
			InitSurfs(ssurf->surfs, &ssurf->texinfo, NULL, sub);
		}
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
//	VRenderLevel::NewWSurf
//
//==========================================================================

surface_t* VRenderLevel::NewWSurf()
{
	guard(VRenderLevel::NewWSurf);
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
//	VRenderLevel::FreeWSurfs
//
//==========================================================================

void VRenderLevel::FreeWSurfs(surface_t* InSurfs)
{
	guard(VRenderLevel::FreeWSurfs);
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
//	VRenderLevel::SubdivideSeg
//
//==========================================================================

surface_t* VRenderLevel::SubdivideSeg(surface_t* InSurf,
	const TVec &axis, const TVec *nextaxis)
{
	guard(VRenderLevel::SubdivideSeg);
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
//	VRenderLevel::CreateWSurfs
//
//==========================================================================

surface_t* VRenderLevel::CreateWSurfs(TVec* wv, texinfo_t* texinfo,
	seg_t* seg, subsector_t* sub)
{
	guard(VRenderLevel::CreateWSurfs);
	if (wv[1].z <= wv[0].z && wv[2].z <= wv[3].z)
	{
		return NULL;
	}

	if (texinfo->Tex->Type == TEXTYPE_Null)
	{
		return NULL;
	}

	surface_t *surf;

	if (texinfo->Tex == GTextureManager[skyflatnum])
	{
		//	Never split sky surfaces
		surf = NewWSurf();
		surf->next = NULL;
		surf->count = 4;
		surf->texinfo = texinfo;
		surf->plane = seg;
		memcpy(surf->verts, wv, 4 * sizeof(TVec));
		return surf;
	}

	surf = NewWSurf();
	surf->next = NULL;
	surf->count = 4;
	memcpy(surf->verts, wv, 4 * sizeof(TVec));
	surf = SubdivideSeg(surf, texinfo->saxis, &texinfo->taxis);
	InitSurfs(surf, texinfo, seg, sub);
	return surf;
	unguard;
}

//==========================================================================
//
//	VRenderLevel::CountSegParts
//
//==========================================================================

int VRenderLevel::CountSegParts(seg_t* seg)
{
	guard(VRenderLevel::CountSegParts);
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
//	VRenderLevel::CreateSegParts
//
//==========================================================================

void VRenderLevel::CreateSegParts(drawseg_t* dseg, seg_t *seg)
{
	guard(VRenderLevel::CreateSegParts);
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

	side_t *sidedef = seg->sidedef;
	line_t *linedef = seg->linedef;

	TVec segdir = (*seg->v2 - *seg->v1) / seg->length;

	float topz1 = r_ceiling->GetPointZ(*seg->v1);
	float topz2 = r_ceiling->GetPointZ(*seg->v2);
	float botz1 = r_floor->GetPointZ(*seg->v1);
	float botz2 = r_floor->GetPointZ(*seg->v2);

	if (!seg->backsector)
	{
		dseg->mid = pspart++;
		sp = dseg->mid;

		VTexture* MTex = GTextureManager(sidedef->midtexture);
		sp->texinfo.saxis = segdir * TextureSScale(MTex);
		sp->texinfo.taxis = TVec(0, 0, -1) * TextureTScale(MTex);
		sp->texinfo.soffs = -DotProduct(*seg->v1, sp->texinfo.saxis) +
			seg->offset * TextureSScale(MTex) +
			sidedef->textureoffset * TextureOffsetSScale(MTex);
		sp->texinfo.Tex = MTex;
		sp->texinfo.Alpha = 1.1;
		sp->texinfo.Additive = false;

		hdelta = topz2 - topz1;
		offshdelta = hdelta * seg->offset / seg->length;

		if (linedef->flags & ML_DONTPEGBOTTOM)
		{
			//	bottom of texture at bottom
			sp->texinfo.toffs = MIN(botz1, botz2) + MTex->GetScaledHeight();
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
		sp->texinfo.toffs *= TextureTScale(MTex);
		sp->texinfo.toffs += sidedef->rowoffset * TextureOffsetTScale(MTex);

		wv[0].x = wv[1].x = seg->v1->x;
		wv[0].y = wv[1].y = seg->v1->y;
		wv[2].x = wv[3].x = seg->v2->x;
		wv[2].y = wv[3].y = seg->v2->y;

		wv[0].z = botz1;
		wv[1].z = topz1;
		wv[2].z = topz2;
		wv[3].z = botz2;

		sp->surfs = CreateWSurfs(wv, &sp->texinfo, seg, r_sub);

		sp->frontTopDist = r_ceiling->dist;
		sp->frontBotDist = r_floor->dist;
		sp->textureoffset = sidedef->textureoffset;
		sp->rowoffset = sidedef->rowoffset;

		//	Sky above line.
		dseg->topsky = pspart++;
		sp = dseg->topsky;
		sp->texinfo.Tex = GTextureManager[skyflatnum];
		sp->texinfo.Alpha = 1.1;
		sp->texinfo.Additive = false;
		if (r_ceiling->pic == skyflatnum)
		{
			wv[0].x = wv[1].x = seg->v1->x;
			wv[0].y = wv[1].y = seg->v1->y;
			wv[2].x = wv[3].x = seg->v2->x;
			wv[2].y = wv[3].y = seg->v2->y;

			wv[0].z = topz1;
			wv[1].z = wv[2].z = skyheight;
			wv[3].z = topz2;

			sp->surfs = CreateWSurfs(wv, &sp->texinfo, seg, r_sub);

			sp->frontTopDist = r_ceiling->dist;
		}
	}
	else
	{
		// two sided line

		sec_plane_t* back_floor = &seg->backsector->floor;
		sec_plane_t* back_ceiling = &seg->backsector->ceiling;

		float back_topz1 = back_ceiling->GetPointZ(*seg->v1);
		float back_topz2 = back_ceiling->GetPointZ(*seg->v2);
		float back_botz1 = back_floor->GetPointZ(*seg->v1);
		float back_botz2 = back_floor->GetPointZ(*seg->v2);

		// hack to allow height changes in outdoor areas
		float top_topz1 = topz1;
		float top_topz2 = topz2;
		if (r_ceiling->pic == skyflatnum &&
			back_ceiling->pic == skyflatnum)
		{
			top_topz1 = back_topz1;
			top_topz2 = back_topz2;
		}

		// top wall
		dseg->top = pspart++;
		sp = dseg->top;

		VTexture* TTex = GTextureManager(sidedef->toptexture);
		sp->texinfo.saxis = segdir * TextureSScale(TTex);
		sp->texinfo.taxis = TVec(0, 0, -1) * TextureTScale(TTex);
		sp->texinfo.soffs = -DotProduct(*seg->v1, sp->texinfo.saxis) +
			seg->offset * TextureSScale(TTex) +
			sidedef->textureoffset * TextureOffsetSScale(TTex);
		sp->texinfo.Tex = TTex;
		sp->texinfo.Alpha = 1.1;
		sp->texinfo.Additive = false;

		hdelta = top_topz2 - top_topz1;
		offshdelta = hdelta * seg->offset / seg->length;
		if (linedef->flags & ML_DONTPEGTOP)
		{
			// top of texture at top
			sp->texinfo.toffs = top_topz1;
		}
		else
		{
			// bottom of texture
			sp->texinfo.toffs = back_topz1 + TTex->GetScaledHeight();
		}
		sp->texinfo.toffs -= offshdelta;
		sp->texinfo.toffs *= TextureTScale(TTex);
		sp->texinfo.toffs += sidedef->rowoffset * TextureOffsetTScale(TTex);

		wv[0].x = wv[1].x = seg->v1->x;
		wv[0].y = wv[1].y = seg->v1->y;
		wv[2].x = wv[3].x = seg->v2->x;
		wv[2].y = wv[3].y = seg->v2->y;

		wv[0].z = MAX(back_topz1, botz1);
		wv[1].z = top_topz1;
		wv[2].z = top_topz2;
		wv[3].z = MAX(back_topz2, botz2);

		sp->surfs = CreateWSurfs(wv, &sp->texinfo, seg, r_sub);

		sp->frontTopDist = r_ceiling->dist;
		sp->frontBotDist = r_floor->dist;
		sp->backTopDist = back_ceiling->dist;
		sp->backBotDist = back_floor->dist;
		sp->textureoffset = sidedef->textureoffset;
		sp->rowoffset = sidedef->rowoffset;

		//	Sky abowe top
		dseg->topsky = pspart++;
		if (r_ceiling->pic == skyflatnum &&
			back_ceiling->pic != skyflatnum)
		{
			sp = dseg->topsky;

			sp->texinfo.Tex = GTextureManager[skyflatnum];
			sp->texinfo.Alpha = 1.1;
			sp->texinfo.Additive = false;

			wv[0].x = wv[1].x = seg->v1->x;
			wv[0].y = wv[1].y = seg->v1->y;
			wv[2].x = wv[3].x = seg->v2->x;
			wv[2].y = wv[3].y = seg->v2->y;

			wv[0].z = topz1;
			wv[1].z = wv[2].z = skyheight;
			wv[3].z = topz2;

			sp->surfs = CreateWSurfs(wv, &sp->texinfo, seg, r_sub);

			sp->frontTopDist = r_ceiling->dist;
		}

		// bottom wall
		dseg->bot = pspart++;
		sp = dseg->bot;

		VTexture* BTex = GTextureManager(sidedef->bottomtexture);
		sp->texinfo.saxis = segdir * TextureSScale(BTex);
		sp->texinfo.taxis = TVec(0, 0, -1) * TextureTScale(BTex);
		sp->texinfo.soffs = -DotProduct(*seg->v1, sp->texinfo.saxis) +
			seg->offset * TextureSScale(BTex) +
			sidedef->textureoffset * TextureOffsetSScale(BTex);
		sp->texinfo.Tex = BTex;
		sp->texinfo.Alpha = 1.1;
		sp->texinfo.Additive = false;

		hdelta = back_botz2 - back_botz1;
		offshdelta = hdelta * seg->offset / seg->length;
		if (linedef->flags & ML_DONTPEGBOTTOM)
		{
			// bottom of texture at bottom
			// top of texture at top
			sp->texinfo.toffs = top_topz1;
		}
		else
		{
			// top of texture at top
			sp->texinfo.toffs = back_botz1;
		}
		sp->texinfo.toffs -= offshdelta;
		sp->texinfo.toffs *= TextureTScale(BTex);
		sp->texinfo.toffs += sidedef->rowoffset * TextureOffsetTScale(BTex);

		wv[0].x = wv[1].x = seg->v1->x;
		wv[0].y = wv[1].y = seg->v1->y;
		wv[2].x = wv[3].x = seg->v2->x;
		wv[2].y = wv[3].y = seg->v2->y;

		wv[0].z = botz1;
		wv[1].z = MIN(back_botz1, topz1);
		wv[2].z = MIN(back_botz2, topz2);
		wv[3].z = botz2;

		sp->surfs = CreateWSurfs(wv, &sp->texinfo, seg, r_sub);

		sp->frontTopDist = r_ceiling->dist;
		sp->frontBotDist = r_floor->dist;
		sp->backTopDist = back_ceiling->dist;
		sp->backBotDist = back_floor->dist;
		sp->textureoffset = sidedef->textureoffset;
		sp->rowoffset = sidedef->rowoffset;

		float midtopz1 = MIN(topz1, back_topz1);
		float midtopz2 = MIN(topz2, back_topz2);
		float midbotz1 = MAX(botz1, back_botz1);
		float midbotz2 = MAX(botz2, back_botz2);

		dseg->mid = pspart++;
		sp = dseg->mid;

		VTexture* MTex = GTextureManager(sidedef->midtexture);
		sp->texinfo.Tex = MTex;
		if (MTex->Type != TEXTYPE_Null)
		{
			// masked midtexture
			float texh = MTex->GetScaledHeight();
			hdelta = midtopz2 - midtopz1;
			offshdelta = hdelta * seg->offset / seg->length;

			float			z_org;

			sp->texinfo.saxis = segdir * TextureSScale(MTex);
			sp->texinfo.taxis = TVec(0, 0, -1) * TextureTScale(MTex);
			sp->texinfo.soffs = -DotProduct(*seg->v1, sp->texinfo.saxis) +
				seg->offset * TextureSScale(MTex) +
				sidedef->textureoffset * TextureOffsetSScale(MTex);
			sp->texinfo.Alpha = linedef->alpha;
			sp->texinfo.Additive = !!(sidedef->Flags & SDF_ADDITIVE);

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
			z_org += sidedef->rowoffset * (!MTex->bWorldPanning ?
				1.0 : 1.0 / MTex->TScale) - offshdelta;

			sp->texinfo.toffs = z_org * TextureTScale(MTex);

			wv[0].x = wv[1].x = seg->v1->x;
			wv[0].y = wv[1].y = seg->v1->y;
			wv[2].x = wv[3].x = seg->v2->x;
			wv[2].y = wv[3].y = seg->v2->y;

			wv[0].z = MAX(midbotz1, z_org - texh);
			wv[1].z = MIN(midtopz1, z_org);
			wv[2].z = MIN(midtopz2, z_org);
			wv[3].z = MAX(midbotz2, z_org - texh);

			sp->surfs = CreateWSurfs(wv, &sp->texinfo, seg, r_sub);
		}

		sp->frontTopDist = r_ceiling->dist;
		sp->frontBotDist = r_floor->dist;
		sp->backTopDist = back_ceiling->dist;
		sp->backBotDist = back_floor->dist;
		sp->textureoffset = sidedef->textureoffset;
		sp->rowoffset = sidedef->rowoffset;

		sec_region_t *reg;
		for (reg = seg->backsector->topregion; reg->prev; reg = reg->prev)
		{
			sp = pspart++;
			sp->next = dseg->extra;
			dseg->extra = sp;

			TPlane *extratop = reg->floor;
			TPlane *extrabot = reg->prev->ceiling;
			side_t *extraside = &Level->Sides[reg->prev->extraline->sidenum[0]];

			float extratopz1 = extratop->GetPointZ(*seg->v1);
			float extratopz2 = extratop->GetPointZ(*seg->v2);
			float extrabotz1 = extrabot->GetPointZ(*seg->v1);
			float extrabotz2 = extrabot->GetPointZ(*seg->v2);

			VTexture* MTex = GTextureManager(extraside->midtexture);
			sp->texinfo.saxis = segdir * TextureSScale(MTex);
			sp->texinfo.taxis = TVec(0, 0, -1) * TextureTScale(MTex);
			sp->texinfo.soffs = -DotProduct(*seg->v1, sp->texinfo.saxis) +
				seg->offset * TextureSScale(MTex) +
				sidedef->textureoffset * TextureOffsetSScale(MTex);

			hdelta = extratopz2 - extratopz1;
			offshdelta = hdelta * seg->offset / seg->length;
			sp->texinfo.toffs = (extratopz1 - offshdelta) *
				TextureTScale(MTex) + sidedef->rowoffset *
				TextureOffsetTScale(MTex);
			sp->texinfo.Tex = MTex;
			sp->texinfo.Alpha = reg->prev->extraline->alpha < 1.0 ?
				reg->prev->extraline->alpha : 1.1;
			sp->texinfo.Additive = !!(extraside->Flags & SDF_ADDITIVE);

			wv[0].x = wv[1].x = seg->v1->x;
			wv[0].y = wv[1].y = seg->v1->y;
			wv[2].x = wv[3].x = seg->v2->x;
			wv[2].y = wv[3].y = seg->v2->y;

			wv[0].z = MAX(extrabotz1, botz1);
			wv[1].z = MIN(extratopz1, topz1);
			wv[2].z = MIN(extratopz2, topz2);
			wv[3].z = MAX(extrabotz2, botz2);

			sp->surfs = CreateWSurfs(wv, &sp->texinfo, seg, r_sub);

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
//	VRenderLevel::UpdateRowOffset
//
//==========================================================================

void VRenderLevel::UpdateRowOffset(segpart_t *sp, float rowoffset)
{
	guard(VRenderLevel::UpdateRowOffset);
	sp->texinfo.toffs += (rowoffset - sp->rowoffset) *
		TextureOffsetTScale(sp->texinfo.Tex);
	sp->rowoffset = rowoffset;
	FlushSurfCaches(sp->surfs);
	InitSurfs(sp->surfs, &sp->texinfo, NULL, r_sub);
	unguard;
}

//==========================================================================
//
//	VRenderLevel::UpdateTextureOffset
//
//==========================================================================

void VRenderLevel::UpdateTextureOffset(segpart_t* sp, float textureoffset)
{
	guard(VRenderLevel::UpdateTextureOffset);
	sp->texinfo.soffs += (textureoffset - sp->textureoffset) *
		TextureOffsetSScale(sp->texinfo.Tex);
	sp->textureoffset = textureoffset;
	FlushSurfCaches(sp->surfs);
	InitSurfs(sp->surfs, &sp->texinfo, NULL, r_sub);
	unguard;
}

//==========================================================================
//
//	VRenderLevel::UpdateDrawSeg
//
//==========================================================================

void VRenderLevel::UpdateDrawSeg(drawseg_t* dseg)
{
	guard(VRenderLevel::UpdateDrawSeg);
	seg_t *seg = dseg->seg;
	segpart_t *sp;
	TVec wv[4];

	if (!seg->linedef)
	{
		//	Miniseg
		return;
	}

	side_t* sidedef = seg->sidedef;
	line_t* linedef = seg->linedef;

	if (!seg->backsector)
	{
		sp = dseg->mid;
		VTexture* MTex = GTextureManager(sidedef->midtexture);
		if (FASI(sp->frontTopDist) != FASI(r_ceiling->dist) ||
			FASI(sp->frontBotDist) != FASI(r_floor->dist) ||
			sp->texinfo.Tex->SScale != MTex->SScale ||
			sp->texinfo.Tex->TScale != MTex->TScale ||
			sp->texinfo.Tex->GetHeight() != MTex->GetHeight())
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
					MTex->GetScaledHeight();
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
			sp->texinfo.toffs *= TextureTScale(MTex);
			sp->texinfo.toffs += sidedef->rowoffset * TextureOffsetTScale(MTex);
			sp->texinfo.Tex = MTex;

			wv[0].x = wv[1].x = seg->v1->x;
			wv[0].y = wv[1].y = seg->v1->y;
			wv[2].x = wv[3].x = seg->v2->x;
			wv[2].y = wv[3].y = seg->v2->y;

			wv[0].z = botz1;
			wv[1].z = topz1;
			wv[2].z = topz2;
			wv[3].z = botz2;

			sp->surfs = CreateWSurfs(wv, &sp->texinfo, seg, r_sub);

			sp->frontTopDist = r_ceiling->dist;
			sp->frontBotDist = r_floor->dist;
			sp->rowoffset = sidedef->rowoffset;
		}
		else if (FASI(sp->rowoffset) != FASI(sidedef->rowoffset))
		{
			sp->texinfo.Tex = MTex;
			UpdateRowOffset(sp, sidedef->rowoffset);
		}
		else
		{
			sp->texinfo.Tex = MTex;
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

			sp->surfs = CreateWSurfs(wv, &sp->texinfo, seg, r_sub);

			sp->frontTopDist = r_ceiling->dist;
		}
	}
	else
	{
		sec_plane_t* back_floor = &seg->backsector->floor;
		sec_plane_t* back_ceiling = &seg->backsector->ceiling;

		// top wall
		sp = dseg->top;
		VTexture* TTex = GTextureManager(sidedef->toptexture);
		if (FASI(sp->frontTopDist) != FASI(r_ceiling->dist) ||
			FASI(sp->frontBotDist) != FASI(r_floor->dist) ||
			FASI(sp->backTopDist) != FASI(back_ceiling->dist) ||
			sp->texinfo.Tex->SScale != TTex->SScale ||
			sp->texinfo.Tex->TScale != TTex->TScale)
		{
			float topz1 = r_ceiling->GetPointZ(*seg->v1);
			float topz2 = r_ceiling->GetPointZ(*seg->v2);
			float botz1 = r_floor->GetPointZ(*seg->v1);
			float botz2 = r_floor->GetPointZ(*seg->v2);

			float back_topz1 = back_ceiling->GetPointZ(*seg->v1);
			float back_topz2 = back_ceiling->GetPointZ(*seg->v2);

			// hack to allow height changes in outdoor areas
			float top_topz1 = topz1;
			float top_topz2 = topz2;
			if (r_ceiling->pic == skyflatnum &&
				back_ceiling->pic == skyflatnum)
			{
				top_topz1 = back_topz1;
				top_topz2 = back_topz2;
			}

			FreeWSurfs(sp->surfs);
			sp->surfs = NULL;

			float hdelta = top_topz2 - top_topz1;
			float offshdelta = hdelta * seg->offset / seg->length;
			if (linedef->flags & ML_DONTPEGTOP)
			{
				// top of texture at top
				sp->texinfo.toffs = top_topz1;
			}
			else
			{
				// bottom of texture
				sp->texinfo.toffs = back_topz1 + TTex->GetScaledHeight();
			}
			sp->texinfo.toffs -= offshdelta;
			sp->texinfo.toffs *= TextureTScale(TTex);
			sp->texinfo.toffs += sidedef->rowoffset * TextureOffsetTScale(TTex);
			sp->texinfo.Tex = TTex;

			wv[0].x = wv[1].x = seg->v1->x;
			wv[0].y = wv[1].y = seg->v1->y;
			wv[2].x = wv[3].x = seg->v2->x;
			wv[2].y = wv[3].y = seg->v2->y;

			wv[0].z = MAX(back_topz1, botz1);
			wv[1].z = top_topz1;
			wv[2].z = top_topz2;
			wv[3].z = MAX(back_topz2, botz2);

			sp->surfs = CreateWSurfs(wv, &sp->texinfo, seg, r_sub);

			sp->frontTopDist = r_ceiling->dist;
			sp->frontBotDist = r_floor->dist;
			sp->backTopDist = back_ceiling->dist;
			sp->rowoffset = sidedef->rowoffset;
		}
		else if (FASI(sp->rowoffset) != FASI(sidedef->rowoffset))
		{
			sp->texinfo.Tex = TTex;
			UpdateRowOffset(sp, sidedef->rowoffset);
		}
		else
		{
			sp->texinfo.Tex = TTex;
		}
		if (FASI(sp->textureoffset) != FASI(sidedef->textureoffset))
		{
			UpdateTextureOffset(sp, sidedef->textureoffset);
		}

		//	Sky abowe top
		sp = dseg->topsky;
		if (r_ceiling->pic == skyflatnum &&
			back_ceiling->pic != skyflatnum &&
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

			sp->surfs = CreateWSurfs(wv, &sp->texinfo, seg, r_sub);

			sp->frontTopDist = r_ceiling->dist;
		}

		// bottom wall
		sp = dseg->bot;
		VTexture* BTex = GTextureManager(sidedef->bottomtexture);
		sp->texinfo.Tex = BTex;
		if (FASI(sp->frontTopDist) != FASI(r_ceiling->dist) ||
			FASI(sp->frontBotDist) != FASI(r_floor->dist) ||
			FASI(sp->backBotDist) != FASI(back_floor->dist))
		{
			float topz1 = r_ceiling->GetPointZ(*seg->v1);
			float topz2 = r_ceiling->GetPointZ(*seg->v2);
			float botz1 = r_floor->GetPointZ(*seg->v1);
			float botz2 = r_floor->GetPointZ(*seg->v2);

			float back_botz1 = back_floor->GetPointZ(*seg->v1);
			float back_botz2 = back_floor->GetPointZ(*seg->v2);

			// hack to allow height changes in outdoor areas
			if (r_ceiling->pic == skyflatnum &&
				back_ceiling->pic == skyflatnum)
			{
				topz1 = back_ceiling->GetPointZ(*seg->v1);
				topz2 = back_ceiling->GetPointZ(*seg->v2);
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
			sp->texinfo.toffs *= TextureTScale(BTex);
			sp->texinfo.toffs += sidedef->rowoffset *
				TextureOffsetTScale(BTex);

			wv[0].x = wv[1].x = seg->v1->x;
			wv[0].y = wv[1].y = seg->v1->y;
			wv[2].x = wv[3].x = seg->v2->x;
			wv[2].y = wv[3].y = seg->v2->y;

			wv[0].z = botz1;
			wv[1].z = MIN(back_botz1, topz1);
			wv[2].z = MIN(back_botz2, topz2);
			wv[3].z = botz2;

			sp->surfs = CreateWSurfs(wv, &sp->texinfo, seg, r_sub);

			sp->frontTopDist = r_ceiling->dist;
			sp->frontBotDist = r_floor->dist;
			sp->backBotDist = back_floor->dist;
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

		// masked midtexture
		sp = dseg->mid;
		VTexture* MTex = GTextureManager(sidedef->midtexture);
		if (FASI(sp->frontTopDist) != FASI(r_ceiling->dist) ||
			FASI(sp->frontBotDist) != FASI(r_floor->dist) ||
			FASI(sp->backTopDist) != FASI(back_ceiling->dist) ||
			FASI(sp->backBotDist) != FASI(back_floor->dist) ||
			FASI(sp->rowoffset) != FASI(sidedef->rowoffset) ||
			sp->texinfo.Tex->SScale != MTex->SScale ||
			sp->texinfo.Tex->TScale != MTex->TScale ||
			sp->texinfo.Tex->GetHeight() != MTex->GetHeight() ||
			(sp->texinfo.Tex->Type == TEXTYPE_Null) != (MTex->Type == TEXTYPE_Null))
		{
			FreeWSurfs(sp->surfs);
			sp->surfs = NULL;

			sp->texinfo.Tex = MTex;
			if (sidedef->midtexture)
			{
				float topz1 = r_ceiling->GetPointZ(*seg->v1);
				float topz2 = r_ceiling->GetPointZ(*seg->v2);
				float botz1 = r_floor->GetPointZ(*seg->v1);
				float botz2 = r_floor->GetPointZ(*seg->v2);

				float back_topz1 = back_ceiling->GetPointZ(*seg->v1);
				float back_topz2 = back_ceiling->GetPointZ(*seg->v2);
				float back_botz1 = back_floor->GetPointZ(*seg->v1);
				float back_botz2 = back_floor->GetPointZ(*seg->v2);

				float midtopz1 = MIN(topz1, back_topz1);
				float midtopz2 = MIN(topz2, back_topz2);
				float midbotz1 = MAX(botz1, back_botz1);
				float midbotz2 = MAX(botz2, back_botz2);

				float texh = MTex->GetScaledHeight();
				float hdelta = midtopz2 - midtopz1;
				float offshdelta = hdelta * seg->offset / seg->length;

				float			z_org;

				TVec segdir = (*seg->v2 - *seg->v1) / seg->length;

				sp->texinfo.saxis = segdir * TextureSScale(MTex);
				sp->texinfo.taxis = TVec(0, 0, -1) * TextureTScale(MTex);
				sp->texinfo.soffs = -DotProduct(*seg->v1, sp->texinfo.saxis) +
					seg->offset * TextureSScale(MTex) +
					sidedef->textureoffset * TextureOffsetSScale(MTex);
				sp->texinfo.Alpha = linedef->alpha;
				sp->texinfo.Additive = !!(sidedef->Flags & SDF_ADDITIVE);

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
				z_org += sidedef->rowoffset * (!MTex->bWorldPanning ?
					1.0 : 1.0 / MTex->TScale) - offshdelta;

				sp->texinfo.toffs = z_org * TextureTScale(MTex);

				wv[0].x = wv[1].x = seg->v1->x;
				wv[0].y = wv[1].y = seg->v1->y;
				wv[2].x = wv[3].x = seg->v2->x;
				wv[2].y = wv[3].y = seg->v2->y;

				wv[0].z = MAX(midbotz1, z_org - texh);
				wv[1].z = MIN(midtopz1, z_org);
				wv[2].z = MIN(midtopz2, z_org);
				wv[3].z = MAX(midbotz2, z_org - texh);

				sp->surfs = CreateWSurfs(wv, &sp->texinfo, seg, r_sub);
			}
			else
			{
				sp->texinfo.Alpha = 1.1;
				sp->texinfo.Additive = false;
			}

			sp->frontTopDist = r_ceiling->dist;
			sp->frontBotDist = r_floor->dist;
			sp->backTopDist = back_ceiling->dist;
			sp->backBotDist = back_floor->dist;
			sp->rowoffset = sidedef->rowoffset;
		}
		else
		{
			sp->texinfo.Tex = MTex;
			if (sidedef->midtexture)
			{
				sp->texinfo.Alpha = linedef->alpha;
				sp->texinfo.Additive = !!(sidedef->Flags & SDF_ADDITIVE);
			}
		}
		if (FASI(sp->textureoffset) != FASI(sidedef->textureoffset))
		{
			UpdateTextureOffset(sp, sidedef->textureoffset);
		}

		sec_region_t *reg;
		segpart_t *sp = dseg->extra;
		for (reg = seg->backsector->botregion; reg->next; reg = reg->next)
		{
			TPlane *extratop = reg->next->floor;
			TPlane *extrabot = reg->ceiling;
			side_t *extraside = &Level->Sides[reg->extraline->sidenum[0]];

			VTexture* ETex = GTextureManager(extraside->midtexture);
			sp->texinfo.Tex = ETex;
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
					TextureTScale(ETex) + sidedef->rowoffset *
					TextureOffsetTScale(ETex);

				wv[0].x = wv[1].x = seg->v1->x;
				wv[0].y = wv[1].y = seg->v1->y;
				wv[2].x = wv[3].x = seg->v2->x;
				wv[2].y = wv[3].y = seg->v2->y;

				wv[0].z = MAX(extrabotz1, botz1);
				wv[1].z = MIN(extratopz1, topz1);
				wv[2].z = MIN(extratopz2, topz2);
				wv[3].z = MAX(extrabotz2, botz2);

				sp->surfs = CreateWSurfs(wv, &sp->texinfo, seg, r_sub);

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
//	VRenderLevel::SegMoved
//
//==========================================================================

void VRenderLevel::SegMoved(seg_t* seg)
{
	guard(VRenderLevel::SegMoved);
	if (!seg->drawsegs)
	{
		//	Drawsegs not created yet
		return;
	}
	if (!seg->linedef)
	{
		Sys_Error("R_SegMoved: miniseg");
	}

	VTexture* Tex = seg->drawsegs->mid->texinfo.Tex;
	seg->drawsegs->mid->texinfo.saxis = (*seg->v2 - *seg->v1) / seg->length *
		TextureSScale(Tex);
	seg->drawsegs->mid->texinfo.soffs = -DotProduct(*seg->v1,
		seg->drawsegs->mid->texinfo.saxis) +
		seg->offset * TextureSScale(Tex) +
		seg->sidedef->textureoffset * TextureOffsetSScale(Tex);

	//	Force update
	seg->drawsegs->mid->frontTopDist += 0.346;
	unguard;
}

//==========================================================================
//
//	VRenderLevel::PreRender
//
//==========================================================================

void VRenderLevel::PreRender()
{
	guard(VRenderLevel::PreRender);
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

	//	Set up fake floors.
	for (i = 0; i < Level->NumSectors; i++)
	{
		if (Level->Sectors[i].heightsec)
		{
			SetupFakeFloors(&Level->Sectors[i]);
		}
	}

	//	Count regions in all subsectors
	count = 0;
	dscount = 0;
	spcount = 0;
	for (i = 0; i < Level->NumSubsectors; i++)
	{
		sub = &Level->Subsectors[i];
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
				spcount += CountSegParts(&Level->Segs[sub->firstline + j]);
			}
			if (sub->poly)
			{
				int polyCount = sub->poly->numsegs;
				seg_t **polySeg = sub->poly->segs;
				while (polyCount--)
				{
					spcount += CountSegParts(*polySeg);
					polySeg++;
				}
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
	for (i = 0; i < Level->NumSubsectors; i++)
	{
		if (!(i & 7))
		{
			CL_KeepaliveMessage();
		}
		sub = &Level->Subsectors[i];
		if (!sub->sector->linecount)
		{
			//	Skip sectors containing original polyobjs
			continue;
		}
		r_sub = sub;
		for (reg = sub->sector->botregion; reg; reg = reg->next)
		{
			r_floor = reg->floor;
			r_ceiling = reg->ceiling;
			if (sub->sector->fakefloors)
			{
				if (r_floor == &sub->sector->floor)
				{
					r_floor = &sub->sector->fakefloors->floorplane;
				}
				if (r_ceiling == &sub->sector->ceiling)
				{
					r_ceiling = &sub->sector->fakefloors->ceilplane;
				}
			}

			sreg->secregion = reg;
			sreg->floorplane = r_floor;
			sreg->ceilplane = r_ceiling;
			sreg->floor = CreateSecSurface(sub, r_floor);
			sreg->ceil = CreateSecSurface(sub, r_ceiling);

			sreg->count = sub->numlines;
			if (sub->poly)
			{
				//	Polyobj
				sreg->count += sub->poly->numsegs;
			}
			sreg->lines = pds;
			pds += sreg->count;
			for (j = 0; j < sub->numlines; j++)
			{
				CreateSegParts(&sreg->lines[j], &Level->Segs[sub->firstline + j]);
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
//	VRenderLevel::UpdateSubRegion
//
//==========================================================================

void VRenderLevel::UpdateSubRegion(subregion_t* region)
{
	guard(VRenderLevel::UpdateSubRegion);
	int				count;
	int 			polyCount;
	seg_t**			polySeg;

	r_floor = region->floorplane;
	r_ceiling = region->ceilplane;
	if (r_sub->sector->fakefloors)
	{
		if (r_floor == &r_sub->sector->floor)
		{
			r_floor = &r_sub->sector->fakefloors->floorplane;
		}
		if (r_ceiling == &r_sub->sector->ceiling)
		{
			r_ceiling = &r_sub->sector->fakefloors->ceilplane;
		}
	}

	count = r_sub->numlines;
	drawseg_t *ds = region->lines;
	while (count--)
	{
		UpdateDrawSeg(ds);
		ds++;
	}

	UpdateSecSurface(region->floor, region->floorplane, r_sub);
	UpdateSecSurface(region->ceil, region->ceilplane, r_sub);

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
//	VRenderLevel::UpdateSubsector
//
//==========================================================================

void VRenderLevel::UpdateSubsector(int num, float *bbox)
{
	guard(VRenderLevel::UpdateSubsector);
	r_sub = &Level->Subsectors[num];

	if (r_sub->VisFrame != r_visframecount)
	{
		return;
	}

	if (!r_sub->sector->linecount)
	{
		//	Skip sectors containing original polyobjs
		return;
	}

	bbox[2] = r_sub->sector->floor.minz;
	if (r_sub->sector->ceiling.pic == skyflatnum)
		bbox[5] = skyheight;
	else
		bbox[5] = r_sub->sector->ceiling.maxz;

	UpdateSubRegion(r_sub->regions);
	unguard;
}

//==========================================================================
//
//	VRenderLevel::UpdateBSPNode
//
//==========================================================================

void VRenderLevel::UpdateBSPNode(int bspnum, float* bbox)
{
	guard(VRenderLevel::UpdateBSPNode);
	// Found a subsector?
	if (bspnum & NF_SUBSECTOR)
	{
		if (bspnum == -1)
			UpdateSubsector(0, bbox);
		else
			UpdateSubsector(bspnum & (~NF_SUBSECTOR), bbox);
		return;
	}

	node_t* bsp = &Level->Nodes[bspnum];

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
//	VRenderLevel::CopyPlaneIfValid
//
//==========================================================================

bool VRenderLevel::CopyPlaneIfValid(sec_plane_t* dest,
	const sec_plane_t* source, const sec_plane_t* opp)
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
//	VRenderLevel::UpdateFakeFlats
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

void VRenderLevel::UpdateFakeFlats(sector_t* sec)
{
	guard(VRenderLevel::UpdateFakeFlats);
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
		ff->floorplane.XScale		= s->floor.XScale;
		ff->floorplane.YScale		= s->floor.YScale;
		ff->floorplane.Angle		= s->floor.Angle;
		ff->floorplane.BaseAngle	= s->floor.BaseAngle;
		ff->floorplane.BaseYOffs	= s->floor.BaseYOffs;

		ff->ceilplane.normal		= -s->floor.normal;
		ff->ceilplane.dist			= -s->floor.dist/* + 1*/;
		if (s->ceiling.pic == skyflatnum)
		{
			ff->floorplane.normal	= -ff->ceilplane.normal;
			ff->floorplane.dist		= -ff->ceilplane.dist/* + 1*/;
			ff->ceilplane.pic		= ff->floorplane.pic;
			ff->ceilplane.xoffs		= ff->floorplane.xoffs;
			ff->ceilplane.yoffs		= ff->floorplane.yoffs;
			ff->ceilplane.XScale	= ff->floorplane.XScale;
			ff->ceilplane.YScale	= ff->floorplane.YScale;
			ff->ceilplane.Angle		= ff->floorplane.Angle;
			ff->ceilplane.BaseAngle	= ff->floorplane.BaseAngle;
			ff->ceilplane.BaseYOffs	= ff->floorplane.BaseYOffs;
		}
		else
		{
			ff->ceilplane.pic		= diffTex ? s->floor.pic : s->ceiling.pic;
			ff->ceilplane.xoffs		= s->ceiling.xoffs;
			ff->ceilplane.yoffs		= s->ceiling.yoffs;
			ff->ceilplane.XScale	= s->ceiling.XScale;
			ff->ceilplane.YScale	= s->ceiling.YScale;
			ff->ceilplane.Angle		= s->ceiling.Angle;
			ff->ceilplane.BaseAngle	= s->ceiling.BaseAngle;
			ff->ceilplane.BaseYOffs	= s->ceiling.BaseYOffs;
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
		ff->floorplane.XScale	= ff->ceilplane.XScale		= s->ceiling.XScale;
		ff->floorplane.YScale	= ff->ceilplane.YScale		= s->ceiling.YScale;
		ff->floorplane.Angle	= ff->ceilplane.Angle		= s->ceiling.Angle;
		ff->floorplane.BaseAngle	= ff->ceilplane.BaseAngle	= s->ceiling.BaseAngle;
		ff->floorplane.BaseYOffs	= ff->ceilplane.BaseYOffs	= s->ceiling.BaseYOffs;

		if (s->floor.pic != skyflatnum)
		{
			ff->ceilplane.normal	= sec->ceiling.normal;
			ff->ceilplane.dist		= sec->ceiling.dist;
			ff->floorplane.pic		= s->floor.pic;
			ff->floorplane.xoffs	= s->floor.xoffs;
			ff->floorplane.yoffs	= s->floor.yoffs;
			ff->floorplane.XScale	= s->floor.XScale;
			ff->floorplane.YScale	= s->floor.YScale;
			ff->floorplane.Angle	= s->floor.Angle;
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
//	VRenderLevel::UpdateWorld
//
//==========================================================================

void VRenderLevel::UpdateWorld()
{
	guard(VRenderLevel::UpdateWorld);
	float	dummy_bbox[6] = {-99999, -99999, -99999, 99999, 9999, 99999};

	//	Update fake sectors.
	for (int i = 0; i < Level->NumSectors; i++)
	{
		if (Level->Sectors[i].heightsec &&
			!(Level->Sectors[i].heightsec->SectorFlags & sector_t::SF_IgnoreHeightSec))
		{
			UpdateFakeFlats(&Level->Sectors[i]);
		}
	}

	UpdateBSPNode(Level->NumNodes - 1, dummy_bbox);	// head node is the last node output
	unguard;
}

//==========================================================================
//
//	VRenderLevel::SetupFakeFloors
//
//==========================================================================

void VRenderLevel::SetupFakeFloors(sector_t* Sec)
{
	guard(VRenderLevel::SetupFakeFloors);
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

	Sec->topregion->params = &Sec->fakefloors->params;
	unguard;
}

//==========================================================================
//
//	VRenderLevel::FreeSurfaces
//
//==========================================================================

void VRenderLevel::FreeSurfaces(surface_t* InSurf)
{
	guard(VRenderLevel::FreeSurfaces);
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
//	VRenderLevel::FreeSegParts
//
//==========================================================================

void VRenderLevel::FreeSegParts(segpart_t* ASP)
{
	guard(VRenderLevel::FreeSegParts);
	for (segpart_t* sp = ASP; sp; sp = sp->next)
	{
		FreeWSurfs(sp->surfs);
	}
	unguard;
}
