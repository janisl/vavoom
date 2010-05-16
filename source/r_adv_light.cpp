//**************************************************************************
//**
//**	##   ##    ##    ##   ##   ####     ####   ###     ###
//**	##   ##  ##  ##  ##   ##  ##  ##   ##  ##  ####   ####
//**	 ## ##  ##    ##  ## ##  ##    ## ##    ## ## ## ## ##
//**	 ## ##  ########  ## ##  ##    ## ##    ## ##  ###  ##
//**	  ###   ##    ##   ###    ##  ##   ##  ##  ##       ##
//**	   #    ##    ##    #      ####     ####   ##       ##
//**
//**	$Id: r_light.cpp 4220 2010-04-24 15:24:35Z dj_jl $
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

// TYPES -------------------------------------------------------------------

// EXTERNAL FUNCTION PROTOTYPES --------------------------------------------

// PUBLIC FUNCTION PROTOTYPES ----------------------------------------------

// PRIVATE FUNCTION PROTOTYPES ---------------------------------------------

// EXTERNAL DATA DECLARATIONS ----------------------------------------------

// PUBLIC DATA DEFINITIONS -------------------------------------------------

extern VCvarI				r_darken;

// PRIVATE DATA DEFINITIONS ------------------------------------------------

static subsector_t*		r_sub;
static subregion_t*		r_subregion;
static sec_region_t*	r_region;

// CODE --------------------------------------------------------------------

//==========================================================================
//
//	VAdvancedRenderLevel::AddStaticLight
//
//==========================================================================

void VAdvancedRenderLevel::AddStaticLight(const TVec &origin, float radius,
	vuint32 colour)
{
	guard(VAdvancedRenderLevel::AddStaticLight);
	light_t& L = Lights.Alloc();
	L.origin = origin;
	L.radius = radius;
	L.colour = colour;
	L.leafnum = Level->PointInSubsector(origin) - Level->Subsectors;
	unguard;
}

//==========================================================================
//
//	VAdvancedRenderLevel::AllocDlight
//
//==========================================================================

dlight_t* VAdvancedRenderLevel::AllocDlight(VThinker* Owner)
{
	guard(VAdvancedRenderLevel::AllocDlight);
	int			i;
	dlight_t*	dl;

	// first look for an exact key match
	if (Owner)
	{
		dl = DLights;
		for (i = 0; i < MAX_DLIGHTS; i++, dl++)
		{
			if (dl->Owner == Owner)
			{
				memset(dl, 0, sizeof(*dl));
				dl->Owner = Owner;
				return dl;
			}
		}
	}

	// then look for anything else
	dl = DLights;
	for (i = 0; i < MAX_DLIGHTS; i++, dl++)
	{
		if (dl->die < Level->Time)
		{
			memset(dl, 0, sizeof(*dl));
			dl->Owner = Owner;
			return dl;
		}
	}

	int bestnum = 0;
	float bestdist = 0.0;
	for (i = 0; i < MAX_DLIGHTS; i++, dl++)
	{
		float dist = Length(dl->origin - cl->ViewOrg);
		if (dist > bestdist)
		{
			bestnum = i;
			bestdist = dist;
		}
	}
	dl = &DLights[bestnum];
	memset(dl, 0, sizeof(*dl));
	dl->Owner = Owner;
	return dl;
	unguard;
}

//==========================================================================
//
//	VAdvancedRenderLevel::DecayLights
//
//==========================================================================

void VAdvancedRenderLevel::DecayLights(float time)
{
	guard(VAdvancedRenderLevel::DecayLights);
	dlight_t* dl = DLights;
	for (int i = 0; i < MAX_DLIGHTS; i++, dl++)
	{
		if (dl->die < Level->Time || !dl->radius)
			continue;

		dl->radius -= time * dl->decay;
		if (dl->radius < 0)
			dl->radius = 0;
	}
	unguard;
}

//==========================================================================
//
//	VAdvancedRenderLevel::LightPoint
//
//==========================================================================

vuint32 VAdvancedRenderLevel::LightPoint(const TVec &p)
{
	guard(VAdvancedRenderLevel::LightPoint);
	subsector_t		*sub;
	subregion_t		*reg;
	float			l, lr, lg, lb, d, add;
	int				i, s, t, ds, dt;
	surface_t		*surf;
	int				ltmp;
	rgb_t			*rgbtmp;
	int				leafnum;
	linetrace_t		Trace;

	if (FixedLight)
	{
		return FixedLight | (FixedLight << 8) | (FixedLight << 16) | (FixedLight << 24);
	}

	sub = Level->PointInSubsector(p);
	vuint8* dyn_facevis = Level->LeafPVS(sub);
	reg = sub->regions;
	while (reg->next)
	{
		d = DotProduct(p, reg->floor->secplane->normal) - reg->floor->secplane->dist;

		if (d >= 0.0)
		{
			break;
		}

		reg = reg->next;
	}

	//	Region's base light
	l = reg->secregion->params->lightlevel + ExtraLight;
	if (r_darken)
	{
		l = light_remap[MIN(255, (int)l)];
	}
	l = MIN(255, l);
	int SecLightColour = reg->secregion->params->LightColour;
	lr = ((SecLightColour >> 16) & 255) * l / 255.0;
	lg = ((SecLightColour >> 8) & 255) * l / 255.0;
	lb = (SecLightColour & 255) * l / 255.0;

	//	Add static lights
	for (i = 0; i < Lights.Num(); i++)
	{
		leafnum = Level->PointInSubsector(Lights[i].origin) -
			Level->Subsectors;

		// Check potential visibility
		if (!(dyn_facevis[leafnum >> 3] & (1 << (leafnum & 7))))
		{
			continue;
		}

		add = Lights[i].radius - Length(p - Lights[i].origin);
		if (add <= 0)
		{
			continue;
		}

		if (!Level->TraceLine(Trace, Lights[i].origin, p, SPF_NOBLOCKSIGHT))
		{
			// ray was blocked
			continue;
		}

		l += add;
		lr += add * ((Lights[i].colour >> 16) & 0xff) / 255.0;
		lg += add * ((Lights[i].colour >> 8) & 0xff) / 255.0;
		lb += add * (Lights[i].colour & 0xff) / 255.0;
	}

	//	Add dynamic lights
	for (i = 0; i < MAX_DLIGHTS; i++)
	{
		leafnum = Level->PointInSubsector(DLights[i].origin) -
			Level->Subsectors;

		// Check potential visibility
		if (!(dyn_facevis[leafnum >> 3] & (1 << (leafnum & 7))))
		{
			continue;
		}

		add = (DLights[i].radius - DLights[i].minlight) - Length(p - DLights[i].origin);

		if (add > 0)
		{
			l += add;
			lr += add * ((DLights[i].colour >> 16) & 0xff) / 255.0;
			lg += add * ((DLights[i].colour >> 8) & 0xff) / 255.0;
			lb += add * (DLights[i].colour & 0xff) / 255.0;
		}
	}

	if (l > 255)
		l = 255;
	if (lr > 255)
		lr = 255;
	if (lg > 255)
		lg = 255;
	if (lb > 255)
		lb = 255;

	return ((int)l << 24) | ((int)lr << 16) | ((int)lg << 8) | ((int)lb);
	unguard;
}

//==========================================================================
//
//	VAdvancedRenderLevel::LightPointAmbient
//
//==========================================================================

vuint32 VAdvancedRenderLevel::LightPointAmbient(const TVec &p)
{
	guard(VAdvancedRenderLevel::LightPointAmbient);
	subsector_t		*sub;
	subregion_t		*reg;
	float			l, lr, lg, lb, d, add;
	int				i, s, t, ds, dt;
	surface_t		*surf;
	int				ltmp;
	rgb_t			*rgbtmp;
	int				leafnum;
	linetrace_t		Trace;

	if (FixedLight)
	{
		return FixedLight | (FixedLight << 8) | (FixedLight << 16) | (FixedLight << 24);
	}

	sub = Level->PointInSubsector(p);
	vuint8* dyn_facevis = Level->LeafPVS(sub);
	reg = sub->regions;
	while (reg->next)
	{
		d = DotProduct(p, reg->floor->secplane->normal) - reg->floor->secplane->dist;

		if (d >= 0.0)
		{
			break;
		}

		reg = reg->next;
	}

	//	Region's base light
	l = reg->secregion->params->lightlevel + ExtraLight;
	if (r_darken)
	{
		l = light_remap[MIN(255, (int)l)];
	}
	l = MIN(255, l);
	int SecLightColour = reg->secregion->params->LightColour;
	lr = ((SecLightColour >> 16) & 255) * l / 255.0;
	lg = ((SecLightColour >> 8) & 255) * l / 255.0;
	lb = (SecLightColour & 255) * l / 255.0;

	return ((int)l << 24) | ((int)lr << 16) | ((int)lg << 8) | ((int)lb);
	unguard;
}

//==========================================================================
//
//	VAdvancedRenderLevel::BuildLightMap
//
//==========================================================================

bool VAdvancedRenderLevel::BuildLightMap(surface_t *surf, int shift)
{
	return true;
}

//==========================================================================
//
//	VAdvancedRenderLevel::DrawShadowSurfaces
//
//==========================================================================

void VAdvancedRenderLevel::DrawShadowSurfaces(surface_t* InSurfs, texinfo_t *texinfo,
	VEntity* SkyBox, int LightSourceSector, int SideLight,
	bool AbsSideLight, bool CheckSkyBoxAlways)
{
	guard(VAdvancedRenderLevel::DrawShadowSurfaces);
	surface_t* surfs = InSurfs;
	if (!surfs)
	{
		return;
	}

	if (texinfo->Tex->Type == TEXTYPE_Null)
	{
		return;
	}
	if (texinfo->Alpha < 1.0)
	{
		return;
	}

	sec_params_t* LightParams = LightSourceSector == -1 ? r_region->params :
		&Level->Sectors[LightSourceSector].params;
	int lLev = (AbsSideLight ? 0 : LightParams->lightlevel) + SideLight;
	lLev = FixedLight ? FixedLight : lLev + ExtraLight;
	lLev = MID(0, lLev, 255);
	if (r_darken)
	{
		lLev = light_remap[lLev];
	}
	vuint32 Fade = GetFade(r_region);

	do
	{
		Drawer->RenderSurfaceShadowVolume(surfs, CurrLightPos, CurrLightRadius);
		surfs = surfs->next;
	} while (surfs);
	unguard;
}

//==========================================================================
//
//	VAdvancedRenderLevel::RenderShadowLine
//
// 	Clips the given segment and adds any visible pieces to the line list.
//
//==========================================================================

void VAdvancedRenderLevel::RenderShadowLine(drawseg_t* dseg)
{
	guard(VAdvancedRenderLevel::RenderShadowLine);
	seg_t *line = dseg->seg;

	if (!line->linedef)
	{
		//	Miniseg
		return;
	}

	float dist = DotProduct(CurrLightPos, line->normal) - line->dist;
	if (dist <= 0)
	{
		//	Light is in back side or on plane
		return;
	}

	float a1 = LightClip.PointToClipAngle(*line->v2);
	float a2 = LightClip.PointToClipAngle(*line->v1);
	if (!LightClip.IsRangeVisible(a1, a2))
	{
		return;
	}

	line_t *linedef = line->linedef;
	side_t *sidedef = line->sidedef;

	if (!line->backsector)
	{
		// single sided line
		DrawShadowSurfaces(dseg->mid->surfs, &dseg->mid->texinfo,
			r_region->ceiling->SkyBox, -1, sidedef->Light,
			!!(sidedef->Flags & SDF_ABSLIGHT), false);
		DrawShadowSurfaces(dseg->topsky->surfs, &dseg->topsky->texinfo,
			r_region->ceiling->SkyBox, -1, sidedef->Light,
			!!(sidedef->Flags & SDF_ABSLIGHT), false);
	}
	else
	{
		// two sided line
		DrawShadowSurfaces(dseg->top->surfs, &dseg->top->texinfo,
			r_region->ceiling->SkyBox, -1, sidedef->Light,
			!!(sidedef->Flags & SDF_ABSLIGHT), false);
		DrawShadowSurfaces(dseg->topsky->surfs, &dseg->topsky->texinfo,
			r_region->ceiling->SkyBox, -1, sidedef->Light,
			!!(sidedef->Flags & SDF_ABSLIGHT), false);
		DrawShadowSurfaces(dseg->bot->surfs, &dseg->bot->texinfo,
			r_region->ceiling->SkyBox, -1, sidedef->Light,
			!!(sidedef->Flags & SDF_ABSLIGHT), false);
		DrawShadowSurfaces(dseg->mid->surfs, &dseg->mid->texinfo,
			r_region->ceiling->SkyBox, -1, sidedef->Light,
			!!(sidedef->Flags & SDF_ABSLIGHT), false);
		for (segpart_t *sp = dseg->extra; sp; sp = sp->next)
		{
			DrawShadowSurfaces(sp->surfs, &sp->texinfo,
				r_region->ceiling->SkyBox, -1, sidedef->Light,
				!!(sidedef->Flags & SDF_ABSLIGHT), false);
		}
	}
	unguard;
}

//==========================================================================
//
//	VAdvancedRenderLevel::RenderShadowSecSurface
//
//==========================================================================

void VAdvancedRenderLevel::RenderShadowSecSurface(sec_surface_t* ssurf, VEntity* SkyBox)
{
	guard(VAdvancedRenderLevel::RenderShadowSecSurface);
	sec_plane_t& plane = *ssurf->secplane;

	if (!plane.pic)
	{
		return;
	}

	float dist = DotProduct(CurrLightPos, plane.normal) - plane.dist;
	if (dist <= 0)
	{
		//	Light is in back side or on plane
		return;
	}

	DrawShadowSurfaces(ssurf->surfs, &ssurf->texinfo, SkyBox,
		plane.LightSourceSector, 0, false, true);
	unguard;
}

//==========================================================================
//
//	VAdvancedRenderLevel::RenderShadowSubRegion
//
// 	Determine floor/ceiling planes.
// 	Draw one or more line segments.
//
//==========================================================================

void VAdvancedRenderLevel::RenderShadowSubRegion(subregion_t* region)
{
	guard(VAdvancedRenderLevel::RenderShadowSubRegion);
	int				count;
	int 			polyCount;
	seg_t**			polySeg;
	float			d;

	d = DotProduct(CurrLightPos, region->floor->secplane->normal) -
		region->floor->secplane->dist;
	if (region->next && d <= 0.0)
	{
		RenderShadowSubRegion(region->next);
	}

	r_subregion = region;
	r_region = region->secregion;

	if (r_sub->poly)
	{
		//	Render the polyobj in the subsector first
		polyCount = r_sub->poly->numsegs;
		polySeg = r_sub->poly->segs;
		while (polyCount--)
		{
			RenderShadowLine((*polySeg)->drawsegs);
			polySeg++;
		}
	}

	count = r_sub->numlines;
	drawseg_t *ds = region->lines;
	while (count--)
	{
		RenderShadowLine(ds);
		ds++;
	}

	RenderShadowSecSurface(region->floor, r_region->floor->SkyBox);
	RenderShadowSecSurface(region->ceil, r_region->ceiling->SkyBox);

	if (region->next && d > 0.0)
	{
		RenderShadowSubRegion(region->next);
	}
	unguard;
}

//==========================================================================
//
//	VAdvancedRenderLevel::RenderShadowSubsector
//
//==========================================================================

void VAdvancedRenderLevel::RenderShadowSubsector(int num)
{
	guard(VAdvancedRenderLevel::RenderShadowSubsector);
	subsector_t* Sub = &Level->Subsectors[num];
	r_sub = Sub;

	/*if (Sub->VisFrame != r_visframecount)
	{
		return;
	}*/

	if (!Sub->sector->linecount)
	{
		//	Skip sectors containing original polyobjs
		return;
	}

	if (!LightClip.ClipCheckSubsector(Sub))
	{
		return;
	}

	LightVis[num >> 3] |= 1 << (num & 7);

	RenderShadowSubRegion(Sub->regions);

	//	Add subsector's segs to the clipper. Clipping against mirror
	// is done only for vertical mirror planes.
	LightClip.ClipAddSubsectorSegs(Sub);
	unguard;
}

//==========================================================================
//
//	VAdvancedRenderLevel::RenderShadowBSPNode
//
//	Renders all subsectors below a given node, traversing subtree
// recursively. Just call with BSP root.
//
//==========================================================================

void VAdvancedRenderLevel::RenderShadowBSPNode(int bspnum, float* bbox)
{
	guard(VAdvancedRenderLevel::RenderShadowBSPNode);
	if (LightClip.ClipIsFull())
	{
		return;
	}

	if (!LightClip.ClipIsBBoxVisible(bbox))
	{
		return;
	}

	// Found a subsector?
	if (bspnum & NF_SUBSECTOR)
	{
		if (bspnum == -1)
		{
			RenderShadowSubsector(0);
		}
		else
		{
			RenderShadowSubsector(bspnum & (~NF_SUBSECTOR));
		}
		return;
	}

	node_t* bsp = &Level->Nodes[bspnum];

	/*if (bsp->VisFrame != r_visframecount)
	{
		return;
	}*/

	// Decide which side the view point is on.
	float Dist = DotProduct(CurrLightPos, bsp->normal) - bsp->dist;
	if (Dist >= CurrLightRadius)
	{
		//	Light is completely on front side.
		RenderShadowBSPNode(bsp->children[0], bsp->bbox[0]);
	}
	else if (Dist <= -CurrLightRadius)
	{
		//	Light is completely on back side.
		RenderShadowBSPNode(bsp->children[1], bsp->bbox[1]);
	}
	else
	{
		int side = Dist < 0;

		// Recursively divide front space.
		RenderShadowBSPNode(bsp->children[side], bsp->bbox[side]);

		// Divide back space.
		RenderShadowBSPNode(bsp->children[side ^ 1], bsp->bbox[side ^ 1]);
	}
	unguard;
}

//==========================================================================
//
//	VAdvancedRenderLevel::RenderLightShadows
//
//==========================================================================

void VAdvancedRenderLevel::RenderLightShadows(TVec& Pos, float Radius, vuint32 Colour)
{
	guard(VAdvancedRenderLevel::RenderLightShadows);
	float	dummy_bbox[6] = {-99999, -99999, -99999, 99999, 99999, 99999};

	CurrLightPos = Pos;
	CurrLightRadius = Radius;
	CurrLightColour = Colour;

	Drawer->BeginLightShadowVolumes();
	LightClip.ClearClipNodes(CurrLightPos, Level);
	memset(LightVis, 0, VisSize);
	RenderShadowBSPNode(Level->NumNodes - 1, dummy_bbox);
	Drawer->BeginModelsShadowsPass(CurrLightPos, CurrLightRadius);
	RenderMobjsShadow();

	Drawer->DrawLightShadowsPass(CurrLightPos, CurrLightRadius, Colour);
	Drawer->BeginModelsLightPass(CurrLightPos, CurrLightRadius, Colour);
	RenderMobjsLight();
	unguard;
}
