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
//**	Portals.
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

// PRIVATE DATA DEFINITIONS ------------------------------------------------

// CODE --------------------------------------------------------------------

//==========================================================================
//
//	VPortal::VPortal
//
//==========================================================================

VPortal::VPortal(VRenderLevelShared* ARLev)
: RLev(ARLev)
{
	Level = RLev->PortalLevel + 1;
}

//==========================================================================
//
//	VPortal::~VPortal
//
//==========================================================================

VPortal::~VPortal()
{
}

//==========================================================================
//
//	VPortal::NeedsDepthBuffer
//
//==========================================================================

bool VPortal::NeedsDepthBuffer() const
{
	return true;
}

//==========================================================================
//
//	VPortal::IsSky
//
//==========================================================================

bool VPortal::IsSky() const
{
	return false;
}

//==========================================================================
//
//	VPortal::MatchSky
//
//==========================================================================

bool VPortal::MatchSky(VSky*) const
{
	return false;
}

//==========================================================================
//
//	VPortal::MatchSkyBox
//
//==========================================================================

bool VPortal::MatchSkyBox(VEntity*) const
{
	return false;
}

//==========================================================================
//
//	VPortal::MatchMirror
//
//==========================================================================

bool VPortal::MatchMirror(TPlane*) const
{
	return false;
}

//==========================================================================
//
//	VPortal::Draw
//
//==========================================================================

void VPortal::Draw(bool UseStencil)
{
	guard(VPortal::Draw);
	if (!Drawer->StartPortal(this, UseStencil))
	{
		//	All portal polygons are clipped away.
		return;
	}

	//	Save renderer settings.
	TVec SavedViewOrg = vieworg;
	TAVec SavedViewAngles = viewangles;
	TVec SavedViewForward = viewforward;
	TVec SavedViewRight = viewright;
	TVec SavedViewUp = viewup;
	VEntity* SavedViewEnt = RLev->ViewEnt;
	int SavedExtraLight = RLev->ExtraLight;
	int SavedFixedLight = RLev->FixedLight;
	vuint8* SavedBspVis = RLev->BspVis;
	VRenderLevel::trans_sprite_t* SavedTransSprites = RLev->trans_sprites;
	bool SavedMirrorClip = MirrorClip;
	TClipPlane SavedClip = view_clipplanes[4];
	TClipPlane* SavedClipLink = view_clipplanes[3].next;

	VRenderLevel::trans_sprite_t *TransSprites =
		(VRenderLevel::trans_sprite_t *)Z_Calloc(sizeof(VRenderLevel::trans_sprite_t) * VRenderLevel::MAX_TRANS_SPRITES);

	if (NeedsDepthBuffer())
	{
		//	Set up BSP visibility table and translated sprites. This has to
		// be done only for portals that do rendering of view.
		RLev->BspVis = new vuint8[RLev->VisSize];

		memset(TransSprites, 0, sizeof(VRenderLevel::trans_sprite_t) * VRenderLevel::MAX_TRANS_SPRITES);
		RLev->trans_sprites = TransSprites;
	}

	DrawContents();

	//	Restore render settings.
	vieworg = SavedViewOrg;
	viewangles = SavedViewAngles;
	viewforward = SavedViewForward;
	viewright = SavedViewRight;
	viewup = SavedViewUp;
	RLev->ViewEnt = SavedViewEnt;
	RLev->ExtraLight = SavedExtraLight;
	RLev->FixedLight = SavedFixedLight;
	if (NeedsDepthBuffer())
	{
		delete[] RLev->BspVis;
		RLev->BspVis = NULL;
	}
	RLev->BspVis = SavedBspVis;
	RLev->trans_sprites = SavedTransSprites;
	MirrorClip = SavedMirrorClip;
	view_clipplanes[4] = SavedClip;
	view_clipplanes[3].next = SavedClipLink;
	RLev->TransformFrustum();
	Drawer->SetupViewOrg();

	Drawer->EndPortal(this, UseStencil);

	Z_Free(TransSprites);
	unguard;
}

//==========================================================================
//
//	VPortal::SetUpRanges
//
//==========================================================================

void VPortal::SetUpRanges(VViewClipper& Range, bool Revert)
{
	guard(VPortal::SetUpRanges);
	Range.ClearClipNodes(vieworg, RLev->Level);
	for (int i = 0; i < Surfs.Num(); i++)
	{
		if (Surfs[i]->plane->normal.z == 0)
		{
			//	Wall
			seg_t* Seg = (seg_t*)Surfs[i]->plane;
			check(Seg >= RLev->Level->Segs);
			check(Seg < RLev->Level->Segs + RLev->Level->NumSegs);
			float a1 = Range.PointToClipAngle(*Seg->v2);
			float a2 = Range.PointToClipAngle(*Seg->v1);
			if (Revert)
			{
				Range.AddClipRange(a2, a1);
			}
			else
			{
				Range.AddClipRange(a1, a2);
			}
		}
		else
		{
			//	Subsector
			for (int j = 0; j < Surfs[i]->count; j++)
			{
				TVec v1;
				TVec v2;
				if ((Surfs[i]->plane->normal.z < 0) != Revert)
				{
					v1 = Surfs[i]->verts[j < Surfs[i]->count - 1 ? j + 1 : 0];
					v2 = Surfs[i]->verts[j];
				}
				else
				{
					v1 = Surfs[i]->verts[j];
					v2 = Surfs[i]->verts[j < Surfs[i]->count - 1 ? j + 1 : 0];
				}
				TVec Dir = v2 - v1;
				Dir.z = 0;
				if (Dir.x > -0.01 && Dir.x < 0.01 && Dir.y > -0.01 &&
					Dir.y < 0.01)
				{
					//	Too short.
					continue;
				}
				TPlane P;
				P.SetPointDir(v1, Dir);
				if ((DotProduct(vieworg, P.normal) - P.dist < 0.01) != Revert)
				{
					//	View origin is on the back side.
					continue;
				}
				float a1 = Range.PointToClipAngle(v2);
				float a2 = Range.PointToClipAngle(v1);
				Range.AddClipRange(a1, a2);
			}
		}
	}
	unguard;
}

//==========================================================================
//
//	VSkyPortal::NeedsDepthBuffer
//
//==========================================================================

bool VSkyPortal::NeedsDepthBuffer() const
{
	return false;
}

//==========================================================================
//
//	VSkyPortal::IsSky
//
//==========================================================================

bool VSkyPortal::IsSky() const
{
	return true;
}

//==========================================================================
//
//	VSkyPortal::MatchSky
//
//==========================================================================

bool VSkyPortal::MatchSky(VSky* ASky) const
{
	return Level == RLev->PortalLevel + 1 && Sky == ASky;
}

//==========================================================================
//
//	VSkyPortal::DrawContents
//
//==========================================================================

void VSkyPortal::DrawContents()
{
	guard(VSkyPortal::DrawContents);
	vieworg = TVec(0, 0, 0);
	RLev->TransformFrustum();
	Drawer->SetupViewOrg();

	Sky->Draw(RLev->ColourMap);

	Drawer->WorldDrawing();
	unguard;
}

//==========================================================================
//
//	VSkyBoxPortal::IsSky
//
//==========================================================================

bool VSkyBoxPortal::IsSky() const
{
	return true;
}

//==========================================================================
//
//	VSkyBoxPortal::MatchSkyBox
//
//==========================================================================

bool VSkyBoxPortal::MatchSkyBox(VEntity* AEnt) const
{
	return Level == RLev->PortalLevel + 1 && Viewport == AEnt;
}

//==========================================================================
//
//	VSkyBoxPortal::DrawContents
//
//==========================================================================

void VSkyBoxPortal::DrawContents()
{
	guard(VSkyBoxPortal::DrawContents);
	//	Set view origin to be sky view origin.
	RLev->ViewEnt = Viewport;
	vieworg = Viewport->Origin;
	viewangles.yaw += Viewport->Angles.yaw;
	AngleVectors(viewangles, viewforward, viewright, viewup);

	//	No light flashes in the sky.
	RLev->ExtraLight = 0;
	if (RLev->ColourMap == CM_Default)
	{
		RLev->FixedLight = 0;
	}

	//	Reuse FixedModel flag to prevent recursion
	Viewport->EntityFlags |= VEntity::EF_FixedModel;

	RLev->RenderScene(&refdef, NULL);

	Viewport->EntityFlags &= ~VEntity::EF_FixedModel;
	unguard;
}

//==========================================================================
//
//	VSectorStackPortal::MatchSkyBox
//
//==========================================================================

bool VSectorStackPortal::MatchSkyBox(VEntity* AEnt) const
{
	return Level == RLev->PortalLevel + 1 && Viewport == AEnt;
}

//==========================================================================
//
//	VSectorStackPortal::DrawContents
//
//==========================================================================

void VSectorStackPortal::DrawContents()
{
	guard(VSectorStackPortal::DrawContents);
	VViewClipper Range;
	VPortal::SetUpRanges(Range, false);

	RLev->ViewEnt = Viewport;
	VEntity* Mate = Viewport->eventSkyBoxGetMate();
	vieworg.x = vieworg.x + Viewport->Origin.x - Mate->Origin.x;
	vieworg.y = vieworg.y + Viewport->Origin.y - Mate->Origin.y;

	//	Reuse FixedModel flag to prevent recursion
	Viewport->EntityFlags |= VEntity::EF_FixedModel;

	RLev->RenderScene(&refdef, &Range);

	Viewport->EntityFlags &= ~VEntity::EF_FixedModel;
	unguard;
}

//==========================================================================
//
//	VMirrorPortal::MatchMirror
//
//==========================================================================

bool VMirrorPortal::MatchMirror(TPlane* APlane) const
{
	return Level == RLev->PortalLevel + 1 &&
		Plane->normal == APlane->normal && Plane->dist == APlane->dist;
}

//==========================================================================
//
//	VMirrorPortal::DrawContents
//
//==========================================================================

void VMirrorPortal::DrawContents()
{
	guard(VMirrorPortal::DrawContents);
	RLev->ViewEnt = NULL;

	RLev->MirrorLevel++;
	MirrorFlip = RLev->MirrorLevel & 1;
	MirrorClip = true;

	float Dist = DotProduct(vieworg, Plane->normal) - Plane->dist;
	vieworg -= 2 * Dist * Plane->normal;

	Dist = DotProduct(viewforward, Plane->normal);
	viewforward -= 2 * Dist * Plane->normal;
	Dist = DotProduct(viewright, Plane->normal);
	viewright -= 2 * Dist * Plane->normal;
	Dist = DotProduct(viewup, Plane->normal);
	viewup -= 2 * Dist * Plane->normal;
	VectorsAngles(viewforward, MirrorFlip ? -viewright : viewright, viewup,
		viewangles);

	VViewClipper Range;
	SetUpRanges(Range, true);

	view_clipplanes[4].normal = Plane->normal;
	view_clipplanes[4].dist = Plane->dist;
	view_clipplanes[3].next = &view_clipplanes[4];
	view_clipplanes[4].next = NULL;
	view_clipplanes[4].clipflag = 0x10;

	int* pindex = RLev->FrustumIndexes[4];
	for (int j = 0; j < 3; j++)
	{
		if (view_clipplanes[4].normal[j] < 0)
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

	RLev->RenderScene(&refdef, &Range);

	RLev->MirrorLevel--;
	MirrorFlip = RLev->MirrorLevel & 1;
	unguard;
}
