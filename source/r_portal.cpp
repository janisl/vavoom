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
	TVec SavedViewOrg = vieworg;
	vieworg = TVec(0, 0, 0);
	RLev->TransformFrustum();
	Drawer->SetupViewOrg();

	Sky->Draw(RLev->ColourMap, true);

	if (!Drawer->HasStencil)
	{
		Drawer->WorldDrawing();
	}
	vieworg = SavedViewOrg;
	RLev->TransformFrustum();
	Drawer->SetupViewOrg();
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
	r_viewleaf = RLev->Level->PointInSubsector(vieworg);

	RLev->TransformFrustum();
	Drawer->SetupViewOrg();

	RLev->BspVis = new vuint8[RLev->VisSize];

	VRenderLevel::trans_sprite_t TransSprites[VRenderLevel::MAX_TRANS_SPRITES];
	memset(TransSprites, 0, sizeof(TransSprites));
	RLev->trans_sprites = TransSprites;

	RLev->MarkLeaves();

	RLev->PushDlights();

	RLev->UpdateWorld(&refdef, NULL);

	RLev->RenderWorld(&refdef, NULL);

	RLev->RenderMobjs();

	RLev->DrawParticles();

	RLev->DrawTranslucentPolys();

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

	VViewClipper Range;
	Range.ClearClipNodes(vieworg, RLev->Level);
	for (int i = 0; i < Surfs.Num(); i++)
	{
		if (fabs(Surfs[i]->plane->normal.z) == 0)
		{
			//	Wall
			seg_t* Seg = (seg_t*)Surfs[i]->plane;
			check(Seg >= RLev->Level->Segs);
			check(Seg < RLev->Level->Segs + RLev->Level->NumSegs);
			Range.AddClipRange(Range.PointToClipAngle(*Seg->v2),
				Range.PointToClipAngle(*Seg->v1));
		}
		else
		{
			//	Subsector
			for (int j = 0; j < Surfs[i]->count; j++)
			{
				TVec v1 = Surfs[i]->verts[j];
				TVec v2 = Surfs[i]->verts[j < Surfs[i]->count - 1 ? j + 1 : 0];
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
				Range.AddClipRange(Range.PointToClipAngle(v2),
					Range.PointToClipAngle(v1));
			}
		}
	}

	RLev->ViewEnt = Viewport;
	VEntity* Mate = Viewport->eventSkyBoxGetMate();
	vieworg.x = vieworg.x + Viewport->Origin.x - Mate->Origin.x;
	vieworg.y = vieworg.y + Viewport->Origin.y - Mate->Origin.y;
	r_viewleaf = RLev->Level->PointInSubsector(vieworg);

	RLev->TransformFrustum();
	Drawer->SetupViewOrg();

	RLev->BspVis = new vuint8[RLev->VisSize];

	VRenderLevel::trans_sprite_t TransSprites[VRenderLevel::MAX_TRANS_SPRITES];
	memset(TransSprites, 0, sizeof(TransSprites));
	RLev->trans_sprites = TransSprites;

	RLev->MarkLeaves();

	RLev->PushDlights();

	RLev->UpdateWorld(&refdef, &Range);

	RLev->RenderWorld(&refdef, &Range);

	RLev->RenderMobjs();

	RLev->DrawParticles();

	RLev->DrawTranslucentPolys();

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
	unguard;
}
