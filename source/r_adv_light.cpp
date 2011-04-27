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
static sec_region_t*	r_region;
VCvarF					r_lights_radius("r_lights_radius", "4096", CVAR_Archive);

// CODE --------------------------------------------------------------------

//==========================================================================
//
//	VAdvancedRenderLevel::PushDlights
//
//==========================================================================

void VAdvancedRenderLevel::PushDlights()
{
	r_dlightframecount = 1;
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
	int				i;
	int				leafnum;
	linetrace_t		Trace;

	if (FixedLight)
	{
		return FixedLight | (FixedLight << 8) | (FixedLight << 16) | (FixedLight << 24);
	}

	sub = Level->PointInSubsector(p);
	vuint8* dyn_facevis = Level->LeafPVS(sub);
	reg = sub->regions;
	if (reg)
	{
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
	}

	//	Add static lights
	for (i = 0; i < Lights.Num(); i++)
	{
		if (!Lights[i].radius)
		{
			continue;
		}

		// Check potential visibility
		if (!(dyn_facevis[Lights[i].leafnum >> 3] & (1 << (Lights[i].leafnum & 7))))
		{
			continue;
		}

		if (!Level->TraceLine(Trace, Lights[i].origin, p, SPF_NOBLOCKSIGHT))
		{
			// ray was blocked
			continue;
		}

		add = Lights[i].radius - Length(p - Lights[i].origin);
		if (add > 0)
		{
			l += add;
			lr += add * ((Lights[i].colour >> 16) & 0xff) / 255.0;
			lg += add * ((Lights[i].colour >> 8) & 0xff) / 255.0;
			lb += add * (Lights[i].colour & 0xff) / 255.0;
		}
	}

	//	Add dynamic lights
	for (i = 0; i < MAX_DLIGHTS; i++)
	{
		if (DLights[i].die < Level->Time || !DLights[i].radius)
			continue;

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
	float			l, lr, lg, lb, d;
	linetrace_t		Trace;

	if (FixedLight)
	{
		return FixedLight | (FixedLight << 8) | (FixedLight << 16) | (FixedLight << 24);
	}

	sub = Level->PointInSubsector(p);
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
//	VAdvancedRenderLevel::BuildLightVis
//
//==========================================================================

void VAdvancedRenderLevel::BuildLightVis(int bspnum, float* bbox)
{
	guard(VAdvancedRenderLevel::BuildLightVis);
	if (LightClip.ClipIsFull())
	{
		return;
	}

	if (!LightClip.ClipIsBBoxVisible(bbox))
	{
		return;
	}

	if (bspnum == -1)
	{
		int SubNum = 0;
		subsector_t* Sub = &Level->Subsectors[SubNum];
		if (!Sub->sector->linecount)
		{
			//	Skip sectors containing original polyobjs
			return;
		}

		if (!LightClip.ClipCheckSubsector(Sub))
		{
			return;
		}

		LightVis[SubNum >> 3] |= 1 << (SubNum & 7);
		LightClip.ClipAddSubsectorSegs(Sub);
		return;
	}

	// Found a subsector?
	if (!(bspnum & NF_SUBSECTOR))
	{
		node_t* bsp = &Level->Nodes[bspnum];

		// Decide which side the view point is on.
		float Dist = DotProduct(CurrLightPos, bsp->normal) - bsp->dist;
		if (Dist >= CurrLightRadius)
		{
			//	Light is completely on front side.
			BuildLightVis(bsp->children[0], bsp->bbox[0]);
		}
		else if (Dist <= -CurrLightRadius)
		{
			//	Light is completely on back side.
			BuildLightVis(bsp->children[1], bsp->bbox[1]);
		}
		else
		{
			int side = Dist < 0;

			// Recursively divide front space.
			BuildLightVis(bsp->children[side], bsp->bbox[side]);

			// Divide back space.
			BuildLightVis(bsp->children[side ^ 1], bsp->bbox[side ^ 1]);
		}
		return;
	}

	int SubNum = bspnum & (~NF_SUBSECTOR);
	subsector_t* Sub = &Level->Subsectors[SubNum];
	if (!Sub->sector->linecount)
	{
		//	Skip sectors containing original polyobjs
		return;
	}

	if (!LightClip.ClipCheckSubsector(Sub))
	{
		return;
	}

	LightVis[SubNum >> 3] |= 1 << (SubNum & 7);
	LightClip.ClipAddSubsectorSegs(Sub);
	unguard;
}

//==========================================================================
//
//	VAdvancedRenderLevel::DrawShadowSurfaces
//
//==========================================================================

void VAdvancedRenderLevel::DrawShadowSurfaces(surface_t* InSurfs, texinfo_t *texinfo,
	bool CheckSkyBoxAlways)
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

	float a1 = LightClip.PointToClipAngle(*line->v2);
	float a2 = LightClip.PointToClipAngle(*line->v1);
	if (!LightClip.IsRangeVisible(a1, a2))
	{
		return;
	}

	float dist = DotProduct(CurrLightPos, line->normal) - line->dist;
	if (dist < 0 || dist >= CurrLightRadius)
	{
		//	Light is in back side or on plane or too far away
		return;
	}

	line_t *linedef = line->linedef;
	side_t *sidedef = line->sidedef;

	if (!line->backsector)
	{
		// single sided line
		DrawShadowSurfaces(dseg->mid->surfs, &dseg->mid->texinfo, false);
		DrawShadowSurfaces(dseg->topsky->surfs, &dseg->topsky->texinfo, false);
	}
	else
	{
		// two sided line
		DrawShadowSurfaces(dseg->top->surfs, &dseg->top->texinfo, false);
		DrawShadowSurfaces(dseg->topsky->surfs, &dseg->topsky->texinfo, false);
		DrawShadowSurfaces(dseg->bot->surfs, &dseg->bot->texinfo, false);
		DrawShadowSurfaces(dseg->mid->surfs, &dseg->mid->texinfo, false);
		for (segpart_t *sp = dseg->extra; sp; sp = sp->next)
		{
			DrawShadowSurfaces(sp->surfs, &sp->texinfo, false);
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
	if (dist < 0 || dist >= CurrLightRadius)
	{
		//	Light is in back side
		return;
	}

	DrawShadowSurfaces(ssurf->surfs, &ssurf->texinfo, true);
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
		if (!LightClip.ClipCheckSubsector(r_sub))
		{
			return;
		}
		RenderShadowSubRegion(region->next);
	}

	r_region = region->secregion;

	if (r_sub->poly)
	{
/*		if (!LightClip.ClipCheckSubsector(r_sub))
		{
			return;
		}*/
		//	Render the polyobj in the subsector first
		polyCount = r_sub->poly->numsegs;
		polySeg = r_sub->poly->segs;
		while (polyCount--)
		{
			RenderShadowLine((*polySeg)->drawsegs);
			polySeg++;
		}
	}

/*	if (!LightClip.ClipCheckSubsector(r_sub))
	{
		return;
	}*/

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
/*		if (!LightClip.ClipCheckSubsector(r_sub))
		{
			return;
		}*/
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

	if (!(LightVis[num >> 3] & (1 << (num & 7))))
	{
		return;
	}

	if (!Sub->sector->linecount)
	{
		//	Skip sectors containing original polyobjs
		return;
	}

	if (!LightClip.ClipCheckSubsector(Sub))
	{
		return;
	}

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

	if (bspnum == -1)
	{
		RenderShadowSubsector(0);
		return;
	}

	// Found a subsector?
	if (!(bspnum & NF_SUBSECTOR))
	{
		node_t* bsp = &Level->Nodes[bspnum];

		/*if (bsp->VisFrame != r_visframecount)
		{
			return;
		}*/

		// Decide which side the light is on.
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

			// Possibly divide back space
			if (!LightClip.ClipIsBBoxVisible(bsp->bbox[side ^ 1]))
			{
				return;
			}
			RenderShadowBSPNode(bsp->children[side ^ 1], bsp->bbox[side ^ 1]);
		}
		return;
	}

	RenderShadowSubsector(bspnum & (~NF_SUBSECTOR));
	unguard;
}

//==========================================================================
//
//	VAdvancedRenderLevel::DrawLightSurfaces
//
//==========================================================================

void VAdvancedRenderLevel::DrawLightSurfaces(surface_t* InSurfs, texinfo_t *texinfo,
	VEntity* SkyBox, bool CheckSkyBoxAlways)
{
	guard(VAdvancedRenderLevel::DrawLightSurfaces);
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

	if (SkyBox && (SkyBox->EntityFlags & VEntity::EF_FixedModel))
	{
		SkyBox = NULL;
	}
	bool IsStack = SkyBox && SkyBox->eventSkyBoxGetAlways();
	if (texinfo->Tex == GTextureManager[skyflatnum] ||
		(IsStack && CheckSkyBoxAlways))
	{
		return;
	}

	do
	{
		Drawer->DrawSurfaceLight(surfs);
		surfs = surfs->next;
	} while (surfs);
	unguard;
}

//==========================================================================
//
//	VAdvancedRenderLevel::RenderLightLine
//
// 	Clips the given segment and adds any visible pieces to the line list.
//
//==========================================================================

void VAdvancedRenderLevel::RenderLightLine(drawseg_t* dseg)
{
	guard(VAdvancedRenderLevel::RenderLightLine);
	seg_t *line = dseg->seg;

	if (!line->linedef)
	{
		//	Miniseg
		return;
	}

	float a1 = LightClip.PointToClipAngle(*line->v2);
	float a2 = LightClip.PointToClipAngle(*line->v1);
	if (!LightClip.IsRangeVisible(a1, a2))
	{
		return;
	}

/*	float dist = DotProduct(CurrLightPos, line->normal) - line->dist;
	if (dist < 0 || dist >= CurrLightRadius)
	{
		//	Light is in back side or on plane or too far away
		return;
	}*/
	float dist = DotProduct(vieworg, line->normal) - line->dist;
	if (dist <= 0)
	{
		//	Viewer is in back side or on plane
		return;
	}

	line_t *linedef = line->linedef;
	side_t *sidedef = line->sidedef;

	if (!line->backsector)
	{
		// single sided line
		DrawLightSurfaces(dseg->mid->surfs, &dseg->mid->texinfo,
			r_region->ceiling->SkyBox, false);
		DrawLightSurfaces(dseg->topsky->surfs, &dseg->topsky->texinfo,
			r_region->ceiling->SkyBox, false);
	}
	else
	{
		// two sided line
		DrawLightSurfaces(dseg->top->surfs, &dseg->top->texinfo,
			r_region->ceiling->SkyBox, false);
		DrawLightSurfaces(dseg->topsky->surfs, &dseg->topsky->texinfo,
			r_region->ceiling->SkyBox, false);
		DrawLightSurfaces(dseg->bot->surfs, &dseg->bot->texinfo,
			r_region->ceiling->SkyBox, false);
		DrawLightSurfaces(dseg->mid->surfs, &dseg->mid->texinfo,
			r_region->ceiling->SkyBox, false);
		for (segpart_t *sp = dseg->extra; sp; sp = sp->next)
		{
			DrawLightSurfaces(sp->surfs, &sp->texinfo,
				r_region->ceiling->SkyBox, false);
		}
	}
	unguard;
}

//==========================================================================
//
//	VAdvancedRenderLevel::RenderLightSecSurface
//
//==========================================================================

void VAdvancedRenderLevel::RenderLightSecSurface(sec_surface_t* ssurf, VEntity* SkyBox)
{
	guard(VAdvancedRenderLevel::RenderLightSecSurface);
	sec_plane_t& plane = *ssurf->secplane;

	if (!plane.pic)
	{
		return;
	}

/*	float dist = DotProduct(CurrLightPos, plane.normal) - plane.dist;
	if (dist < 0 || dist >= CurrLightRadius)
	{
		//	Light is in back side or on plane or too far away
		return;
	}*/
	float dist = DotProduct(vieworg, plane.normal) - plane.dist;
	if (dist <= 0)
	{
		// Viewer is in back side or on plane
		return;
	}

	DrawLightSurfaces(ssurf->surfs, &ssurf->texinfo, SkyBox, true);
	unguard;
}

//==========================================================================
//
//	VAdvancedRenderLevel::RenderLightSubRegion
//
// 	Determine floor/ceiling planes.
// 	Draw one or more line segments.
//
//==========================================================================

void VAdvancedRenderLevel::RenderLightSubRegion(subregion_t* region)
{
	guard(VAdvancedRenderLevel::RenderLightSubRegion);
	int				count;
	int 			polyCount;
	seg_t**			polySeg;
	float			d;

	d = DotProduct(vieworg, region->floor->secplane->normal) -
		region->floor->secplane->dist;
	if (region->next && d <= 0.0)
	{
		if (!LightClip.ClipCheckSubsector(r_sub))
		{
			return;
		}
		RenderLightSubRegion(region->next);
	}

	r_region = region->secregion;

	if (r_sub->poly)
	{
/*		if (!LightClip.ClipCheckSubsector(r_sub))
		{
			return;
		}*/
		//	Render the polyobj in the subsector first
		polyCount = r_sub->poly->numsegs;
		polySeg = r_sub->poly->segs;
		while (polyCount--)
		{
			RenderLightLine((*polySeg)->drawsegs);
			polySeg++;
		}
	}

	if (!LightClip.ClipCheckSubsector(r_sub))
	{
		return;
	}

	count = r_sub->numlines;
	drawseg_t *ds = region->lines;
	while (count--)
	{
		RenderLightLine(ds);
		ds++;
	}

	RenderLightSecSurface(region->floor, r_region->floor->SkyBox);
	RenderLightSecSurface(region->ceil, r_region->ceiling->SkyBox);

	if (region->next && d > 0.0)
	{
/*		if (!LightClip.ClipCheckSubsector(r_sub))
		{
			return;
		}*/
		RenderLightSubRegion(region->next);
	}
	unguard;
}

//==========================================================================
//
//	VAdvancedRenderLevel::RenderLightSubsector
//
//==========================================================================

void VAdvancedRenderLevel::RenderLightSubsector(int num)
{
	guard(VAdvancedRenderLevel::RenderLightSubsector);
	subsector_t* Sub = &Level->Subsectors[num];
	r_sub = Sub;

	if (!(LightBspVis[num >> 3] & (1 << (num & 7))))
	{
		return;
	}

	if (!Sub->sector->linecount)
	{
		//	Skip sectors containing original polyobjs
		return;
	}

	if (!LightClip.ClipCheckSubsector(Sub))
	{
		return;
	}

	RenderLightSubRegion(Sub->regions);

	//	Add subsector's segs to the clipper. Clipping against mirror
	// is done only for vertical mirror planes.
	LightClip.ClipAddSubsectorSegs(Sub);
	unguard;
}

//==========================================================================
//
//	VAdvancedRenderLevel::RenderLightBSPNode
//
//	Renders all subsectors below a given node, traversing subtree
// recursively. Just call with BSP root.
//
//==========================================================================

void VAdvancedRenderLevel::RenderLightBSPNode(int bspnum, float* bbox)
{
	guard(VAdvancedRenderLevel::RenderLightBSPNode);
	if (LightClip.ClipIsFull())
	{
		return;
	}

	if (!LightClip.ClipIsBBoxVisible(bbox))
	{
		return;
	}

	if (bspnum == -1)
	{
		RenderLightSubsector(0);
		return;
	}

	// Found a subsector?
	if (!(bspnum & NF_SUBSECTOR))
	{
		node_t* bsp = &Level->Nodes[bspnum];

		/*if (bsp->VisFrame != r_visframecount)
		{
			return;
		}*/

		// Decide which side the light is on.
		float Dist = DotProduct(CurrLightPos, bsp->normal) - bsp->dist;
		if (Dist >= CurrLightRadius)
		{
			//	Light is completely on front side.
			RenderLightBSPNode(bsp->children[0], bsp->bbox[0]);
		}
		else if (Dist <= -CurrLightRadius)
		{
			//	Light is completely on back side.
			RenderLightBSPNode(bsp->children[1], bsp->bbox[1]);
		}
		else
		{
			int side = Dist < 0;

			// Recursively divide front space.
			RenderLightBSPNode(bsp->children[side], bsp->bbox[side]);

			// Possibly divide back space
			if (!LightClip.ClipIsBBoxVisible(bsp->bbox[side ^ 1]))
			{
				return;
			}
			RenderLightBSPNode(bsp->children[side ^ 1], bsp->bbox[side ^ 1]);
		}
		return;
	}

	RenderLightSubsector(bspnum & (~NF_SUBSECTOR));
	unguard;
}

//==========================================================================
//
//	VAdvancedRenderLevel::RenderLightShadows
//
//==========================================================================

void VAdvancedRenderLevel::RenderLightShadows(const refdef_t* RD,
	const VViewClipper* Range, TVec& Pos, float Radius, vuint32 Colour)
{
	guard(VAdvancedRenderLevel::RenderLightShadows);
	//	Don't do lights that are too far away.
	if ((Pos - vieworg).Length() > r_lights_radius + Radius)
	{
		return;
	}
	float	dummy_bbox[6] = {-99999, -99999, -99999, 99999, 99999, 99999};

	//	Clip against frustrum.
	for (int i = 0; i < (MirrorClip ? 5 : 4); i++)
	{
		float d = DotProduct(Pos, view_clipplanes[i].normal);
		d -= view_clipplanes[i].dist;
		if (d <= -Radius)
		{
			return;
		}
	}

	CurrLightPos = Pos;
	CurrLightRadius = Radius;
	CurrLightColour = Colour;

	//	Build vis data for light.
	LightClip.ClearClipNodes(CurrLightPos, Level);
	memset(LightVis, 0, VisSize);
	BuildLightVis(Level->NumNodes - 1, dummy_bbox);

	//	Create combined light and view visibility.
	bool HaveIntersect = false;
	for (int i = 0; i < VisSize; i++)
	{
		LightBspVis[i] = BspVis[i] & LightVis[i];
		if (LightBspVis[i])
		{
			HaveIntersect = true;
		}
	}
	if (!HaveIntersect)
	{
		return;
	}

	//	Do shadow volumes.
	Drawer->BeginLightShadowVolumes();
	LightClip.ClearClipNodes(CurrLightPos, Level);
	RenderShadowBSPNode(Level->NumNodes - 1, dummy_bbox);
	Drawer->BeginModelsShadowsPass(CurrLightPos, CurrLightRadius);
	RenderMobjsShadow();

	//	Draw light.
	Drawer->BeginLightPass(CurrLightPos, CurrLightRadius, Colour);
	LightClip.ClearClipNodes(CurrLightPos, Level);
	RenderLightBSPNode(Level->NumNodes - 1, dummy_bbox);
	Drawer->BeginModelsLightPass(CurrLightPos, CurrLightRadius, Colour);
	RenderMobjsLight();
	unguard;
}
