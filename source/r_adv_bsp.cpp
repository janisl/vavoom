//**************************************************************************
//**
//**	##   ##    ##    ##   ##   ####     ####   ###     ###
//**	##   ##  ##  ##  ##   ##  ##  ##   ##  ##  ####   ####
//**	 ## ##  ##    ##  ## ##  ##    ## ##    ## ## ## ## ##
//**	 ## ##  ########  ## ##  ##    ## ##    ## ##  ###  ##
//**	  ###   ##    ##   ###    ##  ##   ##  ##  ##       ##
//**	   #    ##    ##    #      ####     ####   ##       ##
//**
//**	$Id: r_bsp.cpp 4220 2010-04-24 15:24:35Z dj_jl $
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
#include "r_hardware.h"

// MACROS ------------------------------------------------------------------

#define HORIZON_SURF_SIZE	(sizeof(surface_t) + sizeof(TVec) * 3)

// TYPES -------------------------------------------------------------------

// EXTERNAL FUNCTION PROTOTYPES --------------------------------------------

// PUBLIC FUNCTION PROTOTYPES ----------------------------------------------

// PRIVATE FUNCTION PROTOTYPES ---------------------------------------------

// EXTERNAL DATA DECLARATIONS ----------------------------------------------

// PUBLIC DATA DEFINITIONS -------------------------------------------------

extern VCvarI			r_maxmirrors;

// PRIVATE DATA DEFINITIONS ------------------------------------------------

static subsector_t*		r_sub;
static subregion_t*		r_subregion;
static sec_region_t*	r_region;
static bool				MirrorClipSegs;

// CODE --------------------------------------------------------------------

//==========================================================================
//
//	VAdvancedRenderLevel::SetUpFrustumIndexes
//
//==========================================================================

void VAdvancedRenderLevel::SetUpFrustumIndexes()
{
	guard(VAdvancedRenderLevel::SetUpFrustumIndexes);
	for (int i = 0; i < 4; i++)
	{
		int *pindex = FrustumIndexes[i];
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
//	VAdvancedRenderLevel::DrawSurfaces
//
//==========================================================================

void VAdvancedRenderLevel::DrawSurfaces(surface_t* InSurfs, texinfo_t *texinfo,
	int clipflags, VEntity* SkyBox, int LightSourceSector, int SideLight,
	bool AbsSideLight, bool CheckSkyBoxAlways)
{
	guard(VAdvancedRenderLevel::DrawSurfaces);
	surface_t* surfs = InSurfs;
	if (!surfs)
	{
		return;
	}

	if (texinfo->Tex->Type == TEXTYPE_Null)
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

	if (SkyBox && (SkyBox->EntityFlags & VEntity::EF_FixedModel))
	{
		SkyBox = NULL;
	}
	bool IsStack = SkyBox && SkyBox->eventSkyBoxGetAlways();
	if (texinfo->Tex == GTextureManager[skyflatnum] ||
		IsStack && CheckSkyBoxAlways)
	{
		VSky* Sky = NULL;
		if (!SkyBox && r_sub->sector->Sky & SKY_FROM_SIDE)
		{
			int Tex;
			bool Flip;
			if (r_sub->sector->Sky == SKY_FROM_SIDE)
			{
				Tex = Level->LevelInfo->Sky2Texture;
				Flip = true;
			}
			else
			{
				side_t* Side = &Level->Sides[(r_sub->sector->Sky &
					(SKY_FROM_SIDE - 1)) - 1];
				Tex = Side->TopTexture;
				Flip = !!Level->Lines[Side->LineNum].arg3;
			}
			if (GTextureManager[Tex]->Type != TEXTYPE_Null)
			{
				for (int i = 0; i < SideSkies.Num(); i++)
				{
					if (SideSkies[i]->SideTex == Tex &&
						SideSkies[i]->SideFlip == Flip)
					{
						Sky = SideSkies[i];
						break;
					}
				}
				if (!Sky)
				{
					Sky = new VSky;
					Sky->Init(Tex, Tex, 0, 0, false,
						!!(Level->LevelInfo->LevelInfoFlags &
						VLevelInfo::LIF_ForceNoSkyStretch), Flip, false);
					SideSkies.Append(Sky);
				}
			}
		}
		if (!Sky && !SkyBox)
		{
			InitSky();
			Sky = &BaseSky;
		}

		VPortal* Portal = NULL;
		if (SkyBox)
		{
			for (int i = 0; i < Portals.Num(); i++)
			{
				if (Portals[i] && Portals[i]->MatchSkyBox(SkyBox))
				{
					Portal = Portals[i];
					break;
				}
			}
			if (!Portal)
			{
				if (IsStack)
				{
					Portal = new VSectorStackPortal(this, SkyBox);
				}
				else
				{
					Portal = new VSkyBoxPortal(this, SkyBox);
				}
				Portals.Append(Portal);
			}
		}
		else
		{
			for (int i = 0; i < Portals.Num(); i++)
			{
				if (Portals[i] && Portals[i]->MatchSky(Sky))
				{
					Portal = Portals[i];
					break;
				}
			}
			if (!Portal)
			{
				Portal = new VSkyPortal(this, Sky);
				Portals.Append(Portal);
			}
		}
		do
		{
			Portal->Surfs.Append(surfs);
			if (IsStack && CheckSkyBoxAlways &&
				SkyBox->eventSkyBoxGetPlaneAlpha())
			{
				surfs->Light = (lLev << 24) | LightParams->LightColour;
				surfs->Fade = Fade;
				surfs->dlightframe = r_sub->dlightframe;
				surfs->dlightbits = r_sub->dlightbits;
				DrawTranslucentPoly(surfs, surfs->verts, surfs->count,
					0, SkyBox->eventSkyBoxGetPlaneAlpha(), false, 0,
					false, 0, 0, TVec(), 0, TVec(), TVec(), TVec());
			}

			if (!Drawer->HasStencil)
			{
				if (PortalLevel == 0)
				{
					world_surf_t& S = WorldSurfs.Alloc();
					S.Surf = surfs;
					S.ClipFlags = clipflags;
					S.Type = 1;
				}
				else
				{
					Drawer->DrawSkyPortal(surfs, clipflags);
				}
			}

			surfs = surfs->next;
		} while (surfs);
		return;
	}

	do
	{
		surfs->Light = (lLev << 24) | LightParams->LightColour;
		surfs->Fade = Fade;
		surfs->dlightframe = r_sub->dlightframe;
		surfs->dlightbits = r_sub->dlightbits;

		if (texinfo->Alpha > 1.0)
		{
			if (PortalLevel == 0)
			{
				world_surf_t& S = WorldSurfs.Alloc();
				S.Surf = surfs;
				S.ClipFlags = clipflags;
				S.Type = 0;
			}
			else
			{
				Drawer->DrawPolygon(surfs, clipflags);
			}
		}
		else
		{
			DrawTranslucentPoly(surfs, surfs->verts, surfs->count,
				0, texinfo->Alpha, texinfo->Additive, 0, false, 0, 0,
				TVec(), 0, TVec(), TVec(), TVec());
		}
		surfs = surfs->next;
	} while (surfs);
	unguard;
}

//==========================================================================
//
//	VAdvancedRenderLevel::RenderHorizon
//
//==========================================================================

void VAdvancedRenderLevel::RenderHorizon(drawseg_t* dseg, int clipflags)
{
	guard(VAdvancedRenderLevel::RenderHorizon);
	seg_t* Seg = dseg->seg;

	if (!dseg->HorizonTop)
	{
		dseg->HorizonTop = (surface_t*)Z_Malloc(HORIZON_SURF_SIZE);
		dseg->HorizonBot = (surface_t*)Z_Malloc(HORIZON_SURF_SIZE);
		memset(dseg->HorizonTop, 0, HORIZON_SURF_SIZE);
		memset(dseg->HorizonBot, 0, HORIZON_SURF_SIZE);
	}

	//	Horizon is not supported in sectors with slopes, so just use TexZ.
	float TopZ = r_region->ceiling->TexZ;
	float BotZ = r_region->floor->TexZ;
	float HorizonZ = vieworg.z;

	//	Handle top part.
	if (TopZ > HorizonZ)
	{
		sec_surface_t* Ceil = r_subregion->ceil;

		//	Calculate light and fade.
		sec_params_t* LightParams = Ceil->secplane->LightSourceSector != -1 ?
			&Level->Sectors[Ceil->secplane->LightSourceSector].params :
			r_region->params;
		int lLev = FixedLight ? FixedLight :
			MIN(255, LightParams->lightlevel + ExtraLight);
		if (r_darken)
		{
			lLev = light_remap[lLev];
		}
		vuint32 Fade = GetFade(r_region);

		surface_t* Surf = dseg->HorizonTop;
		Surf->plane = dseg->seg;
		Surf->texinfo = &Ceil->texinfo;
		Surf->HorizonPlane = Ceil->secplane;
		Surf->Light = (lLev << 24) | LightParams->LightColour;
		Surf->Fade = Fade;
		Surf->count = 4;
		Surf->verts[0] = *Seg->v1;
		Surf->verts[0].z = MAX(BotZ, HorizonZ);
		Surf->verts[1] = *Seg->v1;
		Surf->verts[1].z = TopZ;
		Surf->verts[2] = *Seg->v2;
		Surf->verts[2].z = TopZ;
		Surf->verts[3] = *Seg->v2;
		Surf->verts[3].z = MAX(BotZ, HorizonZ);
		if (Ceil->secplane->pic == skyflatnum)
		{
			//	If it's a sky, render it as a regular sky surface.
			DrawSurfaces(Surf, &Ceil->texinfo, clipflags,
				r_region->ceiling->SkyBox, -1, Seg->sidedef->Light,
				!!(Seg->sidedef->Flags & SDF_ABSLIGHT), false);
		}
		else
		{
			if (PortalLevel == 0)
			{
				world_surf_t& S = WorldSurfs.Alloc();
				S.Surf = Surf;
				S.ClipFlags = clipflags;
				S.Type = 2;
			}
			else
			{
				Drawer->DrawHorizonPolygon(Surf, clipflags);
			}
		}
	}

	//	Handle bottom part.
	if (BotZ < HorizonZ)
	{
		sec_surface_t* Floor = r_subregion->floor;

		//	Calculate light and fade.
		sec_params_t* LightParams = Floor->secplane->LightSourceSector != -1 ?
			&Level->Sectors[Floor->secplane->LightSourceSector].params :
			r_region->params;
		int lLev = FixedLight ? FixedLight :
			MIN(255, LightParams->lightlevel + ExtraLight);
		if (r_darken)
		{
			lLev = light_remap[lLev];
		}
		vuint32 Fade = GetFade(r_region);

		surface_t* Surf = dseg->HorizonBot;
		Surf->plane = dseg->seg;
		Surf->texinfo = &Floor->texinfo;
		Surf->HorizonPlane = Floor->secplane;
		Surf->Light = (lLev << 24) | LightParams->LightColour;
		Surf->Fade = Fade;
		Surf->count = 4;
		Surf->verts[0] = *Seg->v1;
		Surf->verts[0].z = BotZ;
		Surf->verts[1] = *Seg->v1;
		Surf->verts[1].z = MIN(TopZ, HorizonZ);
		Surf->verts[2] = *Seg->v2;
		Surf->verts[2].z = MIN(TopZ, HorizonZ);
		Surf->verts[3] = *Seg->v2;
		Surf->verts[3].z = BotZ;
		if (Floor->secplane->pic == skyflatnum)
		{
			//	If it's a sky, render it as a regular sky surface.
			DrawSurfaces(Surf, &Floor->texinfo, clipflags,
				r_region->floor->SkyBox, -1, Seg->sidedef->Light,
				!!(Seg->sidedef->Flags & SDF_ABSLIGHT), false);
		}
		else
		{
			if (PortalLevel == 0)
			{
				world_surf_t& S = WorldSurfs.Alloc();
				S.Surf = Surf;
				S.ClipFlags = clipflags;
				S.Type = 2;
			}
			else
			{
				Drawer->DrawHorizonPolygon(Surf, clipflags);
			}
		}
	}
	unguard;
}

//==========================================================================
//
//	VAdvancedRenderLevel::RenderMirror
//
//==========================================================================

void VAdvancedRenderLevel::RenderMirror(drawseg_t* dseg, int clipflags)
{
	guard(VAdvancedRenderLevel::RenderMirror);
	seg_t* Seg = dseg->seg;

	if (Drawer->HasStencil && MirrorLevel < r_maxmirrors)
	{
		VPortal* Portal = NULL;
		for (int i = 0; i < Portals.Num(); i++)
		{
			if (Portals[i] && Portals[i]->MatchMirror(Seg))
			{
				Portal = Portals[i];
				break;
			}
		}
		if (!Portal)
		{
			Portal = new VMirrorPortal(this, Seg);
			Portals.Append(Portal);
		}

		surface_t* surfs = dseg->mid->surfs;
		do
		{
			Portal->Surfs.Append(surfs);
			surfs = surfs->next;
		} while (surfs);
	}
	else
	{
		DrawSurfaces(dseg->mid->surfs, &dseg->mid->texinfo, clipflags,
			r_region->ceiling->SkyBox, -1, Seg->sidedef->Light,
			!!(Seg->sidedef->Flags & SDF_ABSLIGHT), false);
	}
	unguard;
}

//==========================================================================
//
//	VAdvancedRenderLevel::RenderLine
//
// 	Clips the given segment and adds any visible pieces to the line list.
//
//==========================================================================

void VAdvancedRenderLevel::RenderLine(drawseg_t* dseg, int clipflags)
{
	guard(VAdvancedRenderLevel::RenderLine);
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

	float a1 = ViewClip.PointToClipAngle(*line->v2);
	float a2 = ViewClip.PointToClipAngle(*line->v1);
	if (!ViewClip.IsRangeVisible(a1, a2))
	{
		return;
	}

	if (MirrorClipSegs)
	{
		//	Clip away segs that are behind mirror.
		float Dist1 = DotProduct(*line->v1, view_clipplanes[4].normal) -
			view_clipplanes[4].dist;
		float Dist2 = DotProduct(*line->v2, view_clipplanes[4].normal) -
			view_clipplanes[4].dist;
		if (Dist1 <= 0 && Dist2 <= 0)
		{
			//	Behind mirror.
			return;
		}
	}

	line_t *linedef = line->linedef;
	side_t *sidedef = line->sidedef;

	//FIXME this marks all lines
	// mark the segment as visible for auto map
	linedef->flags |= ML_MAPPED;

	if (!line->backsector)
	{
		// single sided line
		if (line->linedef->special == LNSPEC_LineHorizon)
		{
			RenderHorizon(dseg, clipflags);
		}
		else if (line->linedef->special == LNSPEC_LineMirror)
		{
			RenderMirror(dseg, clipflags);
		}
		else
		{
			DrawSurfaces(dseg->mid->surfs, &dseg->mid->texinfo, clipflags,
				r_region->ceiling->SkyBox, -1, sidedef->Light,
				!!(sidedef->Flags & SDF_ABSLIGHT), false);
		}
		DrawSurfaces(dseg->topsky->surfs, &dseg->topsky->texinfo, clipflags,
			r_region->ceiling->SkyBox, -1, sidedef->Light,
			!!(sidedef->Flags & SDF_ABSLIGHT), false);
	}
	else
	{
		// two sided line
		DrawSurfaces(dseg->top->surfs, &dseg->top->texinfo, clipflags,
			r_region->ceiling->SkyBox, -1, sidedef->Light,
			!!(sidedef->Flags & SDF_ABSLIGHT), false);
		DrawSurfaces(dseg->topsky->surfs, &dseg->topsky->texinfo, clipflags,
			r_region->ceiling->SkyBox, -1, sidedef->Light,
			!!(sidedef->Flags & SDF_ABSLIGHT), false);
		DrawSurfaces(dseg->bot->surfs, &dseg->bot->texinfo, clipflags,
			r_region->ceiling->SkyBox, -1, sidedef->Light,
			!!(sidedef->Flags & SDF_ABSLIGHT), false);
		DrawSurfaces(dseg->mid->surfs, &dseg->mid->texinfo, clipflags,
			r_region->ceiling->SkyBox, -1, sidedef->Light,
			!!(sidedef->Flags & SDF_ABSLIGHT), false);
		for (segpart_t *sp = dseg->extra; sp; sp = sp->next)
		{
			DrawSurfaces(sp->surfs, &sp->texinfo, clipflags,
				r_region->ceiling->SkyBox, -1, sidedef->Light,
				!!(sidedef->Flags & SDF_ABSLIGHT), false);
		}
	}
	unguard;
}

//==========================================================================
//
//	VAdvancedRenderLevel::RenderSecSurface
//
//==========================================================================

void VAdvancedRenderLevel::RenderSecSurface(sec_surface_t* ssurf, int clipflags,
	VEntity* SkyBox)
{
	guard(VAdvancedRenderLevel::RenderSecSurface);
	sec_plane_t& plane = *ssurf->secplane;

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

	if (plane.MirrorAlpha < 1.0 && Drawer->HasStencil &&
		MirrorLevel < r_maxmirrors)
	{
		VPortal* Portal = NULL;
		for (int i = 0; i < Portals.Num(); i++)
		{
			if (Portals[i] && Portals[i]->MatchMirror(&plane))
			{
				Portal = Portals[i];
				break;
			}
		}
		if (!Portal)
		{
			Portal = new VMirrorPortal(this, &plane);
			Portals.Append(Portal);
		}

		surface_t* surfs = ssurf->surfs;
		do
		{
			Portal->Surfs.Append(surfs);
			surfs = surfs->next;
		} while (surfs);

		if (plane.MirrorAlpha <= 0.0)
		{
			return;
		}
		ssurf->texinfo.Alpha = plane.MirrorAlpha;
	}

	DrawSurfaces(ssurf->surfs, &ssurf->texinfo, clipflags, SkyBox,
		plane.LightSourceSector, 0, false, true);
	unguard;
}

//==========================================================================
//
//	VAdvancedRenderLevel::RenderSubRegion
//
// 	Determine floor/ceiling planes.
// 	Draw one or more line segments.
//
//==========================================================================

void VAdvancedRenderLevel::RenderSubRegion(subregion_t* region, int clipflags)
{
	guard(VAdvancedRenderLevel::RenderSubRegion);
	int				count;
	int 			polyCount;
	seg_t**			polySeg;
	float			d;

	d = DotProduct(vieworg, region->floor->secplane->normal) -
		region->floor->secplane->dist;
	if (region->next && d <= 0.0)
	{
		RenderSubRegion(region->next, clipflags);
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

	RenderSecSurface(region->floor, clipflags, r_region->floor->SkyBox);
	RenderSecSurface(region->ceil, clipflags, r_region->ceiling->SkyBox);

	if (region->next && d > 0.0)
	{
		RenderSubRegion(region->next, clipflags);
	}
	unguard;
}

//==========================================================================
//
//	VAdvancedRenderLevel::RenderSubsector
//
//==========================================================================

void VAdvancedRenderLevel::RenderSubsector(int num, int clipflags)
{
	guard(VAdvancedRenderLevel::RenderSubsector);
	subsector_t* Sub = &Level->Subsectors[num];
	r_sub = Sub;

	if (Sub->VisFrame != r_visframecount)
	{
		return;
	}

	if (!Sub->sector->linecount)
	{
		//	Skip sectors containing original polyobjs
		return;
	}

	if (!ViewClip.ClipCheckSubsector(Sub))
	{
		return;
	}

	BspVis[num >> 3] |= 1 << (num & 7);

	RenderSubRegion(Sub->regions, clipflags);

	//	Add subsector's segs to the clipper. Clipping against mirror
	// is done only for vertical mirror planes.
	ViewClip.ClipAddSubsectorSegs(Sub, MirrorClipSegs ? &view_clipplanes[4] :
		NULL);
	unguard;
}

//==========================================================================
//
//	VAdvancedRenderLevel::RenderBSPNode
//
//	Renders all subsectors below a given node, traversing subtree
// recursively. Just call with BSP root.
//
//==========================================================================

void VAdvancedRenderLevel::RenderBSPNode(int bspnum, float* bbox, int AClipflags)
{
	guard(VAdvancedRenderLevel::RenderBSPNode);
	if (ViewClip.ClipIsFull())
	{
		return;
	}
	int clipflags = AClipflags;
	// cull the clipping planes if not trivial accept
	if (clipflags)
	{
		for (int i = 0; i < 5; i++)
		{
			if (!(clipflags & view_clipplanes[i].clipflag))
			{
				continue;	// don't need to clip against it
			}

			// generate accept and reject points

			int *pindex = FrustumIndexes[i];

			TVec rejectpt;

			rejectpt[0] = bbox[pindex[0]];
			rejectpt[1] = bbox[pindex[1]];
			rejectpt[2] = bbox[pindex[2]];

			float d;

			d = DotProduct(rejectpt, view_clipplanes[i].normal);
			d -= view_clipplanes[i].dist;

			if (d <= 0)
			{
				return;
			}

			TVec acceptpt;

			acceptpt[0] = bbox[pindex[3+0]];
			acceptpt[1] = bbox[pindex[3+1]];
			acceptpt[2] = bbox[pindex[3+2]];

			d = DotProduct(acceptpt, view_clipplanes[i].normal);
			d -= view_clipplanes[i].dist;

			if (d >= 0)
			{
				clipflags ^= view_clipplanes[i].clipflag;	// node is entirely on screen
			}
		}
	}

	if (!ViewClip.ClipIsBBoxVisible(bbox))
	{
		return;
	}

	// Found a subsector?
	if (bspnum & NF_SUBSECTOR)
	{
		if (bspnum == -1)
		{
			RenderSubsector(0, clipflags);
		}
		else
		{
			RenderSubsector(bspnum & (~NF_SUBSECTOR), clipflags);
		}
		return;
	}

	node_t* bsp = &Level->Nodes[bspnum];

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
		((VAdvDrawer*)Drawer)->RenderSurfaceShadowVolume(surfs, CurrLightPos, CurrLightRadius);
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

	//BspVis[num >> 3] |= 1 << (num & 7);

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

	((VAdvDrawer*)Drawer)->BeginLightShadowVolumes();
	LightClip.ClearClipNodes(CurrLightPos, Level);
	RenderShadowBSPNode(Level->NumNodes - 1, dummy_bbox);
	((VAdvDrawer*)Drawer)->DrawLightShadowsPass(CurrLightPos, CurrLightRadius, Colour);
	((VAdvDrawer*)Drawer)->BeginModelsLightPass(CurrLightPos, CurrLightRadius, Colour);
	RenderMobjsLight();
	unguard;
}

//==========================================================================
//
//	VAdvancedRenderLevel::RenderWorld
//
//==========================================================================

void VAdvancedRenderLevel::RenderWorld(const refdef_t* rd, const VViewClipper* Range)
{
	guard(VAdvancedRenderLevel::RenderWorld);
	float	dummy_bbox[6] = {-99999, -99999, -99999, 99999, 99999, 99999};

	SetUpFrustumIndexes();
	ViewClip.ClearClipNodes(vieworg, Level);
	ViewClip.ClipInitFrustrumRange(viewangles, viewforward, viewright, viewup,
		rd->fovx, rd->fovy);
	if (Range)
	{
		//	Range contains a valid range, so we must clip away holes in it.
		ViewClip.ClipToRanges(*Range);
	}
	memset(BspVis, 0, VisSize);
	if (PortalLevel == 0)
	{
		WorldSurfs.Resize(4096);
	}
	MirrorClipSegs = MirrorClip && !view_clipplanes[4].normal.z;

	// head node is the last node output
	RenderBSPNode(Level->NumNodes - 1, dummy_bbox, MirrorClip ? 31 : 15);

	if (PortalLevel == 0)
	{
		guard(Best sky);
		//	Draw the most complex sky portal behind the scene first, without
		// the need to use stencil buffer.
		VPortal* BestSky = NULL;
		int BestSkyIndex = -1;
		for (int i = 0; i < Portals.Num(); i++)
		{
			if (Portals[i] && Portals[i]->IsSky() &&
				(!BestSky || BestSky->Surfs.Num() < Portals[i]->Surfs.Num()))
			{
				BestSky = Portals[i];
				BestSkyIndex = i;
			}
		}
		if (BestSky)
		{
			PortalLevel = 1;
			BestSky->Draw(false);
			delete BestSky;
			Portals.RemoveIndex(BestSkyIndex);
			PortalLevel = 0;
		}
		unguard;

		guard(World surfaces);
		for (int i = 0; i < WorldSurfs.Num(); i++)
		{
			switch (WorldSurfs[i].Type)
			{
			case 0:
				Drawer->DrawPolygon(WorldSurfs[i].Surf,
					WorldSurfs[i].ClipFlags);
				break;
			case 1:
				Drawer->DrawSkyPortal(WorldSurfs[i].Surf,
					WorldSurfs[i].ClipFlags);
				break;
			case 2:
				Drawer->DrawHorizonPolygon(WorldSurfs[i].Surf,
					WorldSurfs[i].ClipFlags);
				break;
			}
		}
		WorldSurfs.Clear();
		unguard;
	}

	((VAdvDrawer*)Drawer)->DrawWorldAmbientPass();

	guard(Portals);
	PortalLevel++;
	if (Drawer->HasStencil)
	{
		for (int i = 0; i < Portals.Num(); i++)
		{
			if (Portals[i] && Portals[i]->Level == PortalLevel)
			{
				Portals[i]->Draw(true);
			}
		}
	}
	for (int i = 0; i < Portals.Num(); i++)
	{
		if (Portals[i] && Portals[i]->Level == PortalLevel)
		{
			delete Portals[i];
			Portals[i] = NULL;
		}
	}
	PortalLevel--;
	if (PortalLevel == 0)
	{
		Portals.Clear();
	}
	unguard;
	unguard;
}
