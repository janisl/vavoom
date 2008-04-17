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

VPortal::VPortal(class VRenderLevel* ARLev)
: RLev(ARLev)
{
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

bool VPortal::MatchMirror(seg_t*) const
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

	VRenderLevel::trans_sprite_t TransSprites[VRenderLevel::MAX_TRANS_SPRITES];

	if (NeedsDepthBuffer())
	{
		//	Set up BSP visibility table and translated sprites. This has to
		// be done only for portals that do rendering of view.
		RLev->BspVis = new vuint8[RLev->VisSize];

		memset(TransSprites, 0, sizeof(TransSprites));
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
	delete[] RLev->BspVis;
	RLev->BspVis = SavedBspVis;
	RLev->trans_sprites = SavedTransSprites;
	RLev->TransformFrustum();
	Drawer->SetupViewOrg();

	Drawer->EndPortal(this, UseStencil);
}

//==========================================================================
//
//	VPortal::SetUpRanges
//
//==========================================================================

void VPortal::SetUpRanges(VViewClipper& Range)
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
			Range.AddClipRange(a1, a2);
		}
		else
		{
			//	Subsector
			for (int j = 0; j < Surfs[i]->count; j++)
			{
				TVec v1;
				TVec v2;
				if (Surfs[i]->plane->normal.z < 0)
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
				if (DotProduct(vieworg, P.normal) - P.dist < 0.01)
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
	return Sky == ASky;
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
	return Viewport == AEnt;
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

	RLev->RenderScene(&refdef, NULL);
	unguard;
}

//==========================================================================
//
//	VSectorStackPortal::MatchSkyBox
//
//==========================================================================

bool VSectorStackPortal::MatchSkyBox(VEntity* AEnt) const
{
	return Viewport == AEnt;
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
	VPortal::SetUpRanges(Range);

	RLev->ViewEnt = Viewport;
	VEntity* Mate = Viewport->eventSkyBoxGetMate();
	vieworg.x = vieworg.x + Viewport->Origin.x - Mate->Origin.x;
	vieworg.y = vieworg.y + Viewport->Origin.y - Mate->Origin.y;

	RLev->RenderScene(&refdef, &Range);
	unguard;
}

//==========================================================================
//
//	VMirrorPortal::MatchSkyBox
//
//==========================================================================

bool VMirrorPortal::MatchMirror(seg_t* ASeg) const
{
	return Seg == ASeg;
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

	float Dist = DotProduct(vieworg, Seg->normal) - Seg->dist;
	vieworg -= 2 * Dist * Seg->normal;

	Dist = DotProduct(viewforward, Seg->normal);
	viewforward -= 2 * Dist * Seg->normal;
	Dist = DotProduct(viewright, Seg->normal);
	viewright -= 2 * Dist * Seg->normal;
	Dist = DotProduct(viewup, Seg->normal);
	viewup -= 2 * Dist * Seg->normal;
	VectorsAngles(viewforward, -viewright, viewup, viewangles);
	MirrorFlip = true;

	RLev->RenderScene(&refdef, NULL);
	MirrorFlip = false;
	unguard;
}
