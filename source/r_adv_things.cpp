//**************************************************************************
//**
//**	##   ##    ##    ##   ##   ####     ####   ###     ###
//**	##   ##  ##  ##  ##   ##  ##  ##   ##  ##  ####   ####
//**	 ## ##  ##    ##  ## ##  ##    ## ##    ## ## ## ## ##
//**	 ## ##  ########  ## ##  ##    ## ##    ## ##  ###  ##
//**	  ###   ##    ##   ###    ##  ##   ##  ##  ##       ##
//**	   #    ##    ##    #      ####     ####   ##       ##
//**
//**	$Id: r_things.cpp 4003 2009-03-04 20:08:00Z dj_jl $
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
//**	Refresh of things, i.e. objects represented by sprites.
//**
//** 	Sprite rotation 0 is facing the viewer, rotation 1 is one angle turn
//**  CLOCKWISE around the axis. This is not the same as the angle, which
//**  increases counter clockwise (protractor). There was a lot of stuff
//**  grabbed wrong, so I changed it...
//**
//**************************************************************************

// HEADER FILES ------------------------------------------------------------

#include "gamedefs.h"
#include "r_local.h"
#include "sv_local.h"

// MACROS ------------------------------------------------------------------

// TYPES -------------------------------------------------------------------

enum
{
	SPR_VP_PARALLEL_UPRIGHT,
	SPR_FACING_UPRIGHT,
	SPR_VP_PARALLEL,
	SPR_ORIENTED,
	SPR_VP_PARALLEL_ORIENTED,
	SPR_VP_PARALLEL_UPRIGHT_ORIENTED,
};

// EXTERNAL FUNCTION PROTOTYPES --------------------------------------------

// PUBLIC FUNCTION PROTOTYPES ----------------------------------------------

// PRIVATE FUNCTION PROTOTYPES ---------------------------------------------

// EXTERNAL DATA DECLARATIONS ----------------------------------------------

extern VCvarI		r_chasecam;

// PUBLIC DATA DEFINITIONS -------------------------------------------------

// PRIVATE DATA DEFINITIONS ------------------------------------------------

extern VCvarI			r_draw_mobjs;
extern VCvarI			r_draw_psprites;
extern VCvarI			r_models;
extern VCvarI			r_hide_models;
extern VCvarI			r_view_models;
extern VCvarI			r_model_shadows;
extern VCvarI			r_model_light;
extern VCvarI			r_sort_sprites;
extern VCvarI			r_fix_sprite_offsets;
extern VCvarI			r_sprite_fix_delta;
extern VCvarI			r_drawfuzz;
extern VCvarF			transsouls;
extern VCvarI			croshair;
extern VCvarF			croshair_alpha;

// CODE --------------------------------------------------------------------

//==========================================================================
//
//	VAdvancedRenderLevel::RenderThingAmbient
//
//==========================================================================

void VAdvancedRenderLevel::RenderThingAmbient(VEntity* mobj)
{
	guard(VAdvancedRenderLevel::RenderThingAmbient);
	//	Skip things in subsectors that are not visible.
	int SubIdx = mobj->SubSector - Level->Subsectors;
	if (!(BspVis[SubIdx >> 3] & (1 << (SubIdx & 7))))
	{
		return;
	}
	if (mobj == ViewEnt && (!r_chasecam || ViewEnt != cl->MO))
	{
		//	Don't draw camera actor.
		return;
	}

	if ((mobj->EntityFlags & VEntity::EF_NoSector) ||
		(mobj->EntityFlags & VEntity::EF_Invisible))
	{
		return;
	}
	if (!mobj->State)
	{
		return;
	}

	int RendStyle = mobj->RenderStyle;
	float Alpha = mobj->Alpha;

	if (RendStyle == STYLE_SoulTrans)
	{
		RendStyle = STYLE_Translucent;
		Alpha = transsouls;
	}
	else if (RendStyle == STYLE_OptFuzzy)
	{
		RendStyle = r_drawfuzz ? STYLE_Fuzzy : STYLE_Translucent;
	}

	switch (RendStyle)
	{
	case STYLE_None:
		return;

	case STYLE_Normal:
		Alpha = 1.0;
		break;

	case STYLE_Fuzzy:
		return;

	case STYLE_Add:
		return;
	}
	Alpha = MID(0.0, Alpha, 1.0);

	if (Alpha < 1.0)
	{
		return;
	}

	//	Setup lighting
	vuint32 light;
	if ((mobj->State->Frame & VState::FF_FULLBRIGHT) ||
		(mobj->EntityFlags & (VEntity::EF_FullBright | VEntity::EF_Bright)))
	{
		light = 0xffffffff;
	}
	else
	{
		light = LightPointAmbient(mobj->Origin);
	}

	float TimeFrac = 0;
	if (mobj->State->Time > 0)
	{
		TimeFrac = 1.0 - (mobj->StateTime / mobj->State->Time);
		TimeFrac = MID(0.0, TimeFrac, 1.0);
	}

	DrawEntityModel(mobj, light, 0, 1, false, TimeFrac, RPASS_Ambient);
	unguard;
}

//==========================================================================
//
//	VAdvancedRenderLevel::RenderMobjsAmbient
//
//==========================================================================

void VAdvancedRenderLevel::RenderMobjsAmbient()
{
	guard(VAdvancedRenderLevel::RenderMobjsAmbient);
	if (!r_draw_mobjs || !r_models)
	{
		return;
	}

	for (TThinkerIterator<VEntity> Ent(Level); Ent; ++Ent)
	{
		RenderThingAmbient(*Ent);
	}
	unguard;
}

//==========================================================================
//
//	VAdvancedRenderLevel::RenderThingTextures
//
//==========================================================================

void VAdvancedRenderLevel::RenderThingTextures(VEntity* mobj)
{
	guard(VAdvancedRenderLevel::RenderThingAmbient);
	//	Skip things in subsectors that are not visible.
	int SubIdx = mobj->SubSector - Level->Subsectors;
	if (!(BspVis[SubIdx >> 3] & (1 << (SubIdx & 7))))
	{
		return;
	}
	if (mobj == ViewEnt && (!r_chasecam || ViewEnt != cl->MO))
	{
		//	Don't draw camera actor.
		return;
	}

	if ((mobj->EntityFlags & VEntity::EF_NoSector) ||
		(mobj->EntityFlags & VEntity::EF_Invisible))
	{
		return;
	}
	if (!mobj->State)
	{
		return;
	}

	int RendStyle = mobj->RenderStyle;
	float Alpha = mobj->Alpha;

	if (RendStyle == STYLE_SoulTrans)
	{
		RendStyle = STYLE_Translucent;
		Alpha = transsouls;
	}
	else if (RendStyle == STYLE_OptFuzzy)
	{
		RendStyle = r_drawfuzz ? STYLE_Fuzzy : STYLE_Translucent;
	}

	switch (RendStyle)
	{
	case STYLE_None:
		return;

	case STYLE_Normal:
		Alpha = 1.0;
		break;

	case STYLE_Fuzzy:
		return;

	case STYLE_Add:
		return;
	}
	Alpha = MID(0.0, Alpha, 1.0);

	if (Alpha < 1.0)
	{
		return;
	}

	float TimeFrac = 0;
	if (mobj->State->Time > 0)
	{
		TimeFrac = 1.0 - (mobj->StateTime / mobj->State->Time);
		TimeFrac = MID(0.0, TimeFrac, 1.0);
	}

	DrawEntityModel(mobj, 0xffffffff, 0, 1, false, TimeFrac, RPASS_Textures);
	unguard;
}

//==========================================================================
//
//	VAdvancedRenderLevel::RenderMobjsTextures
//
//==========================================================================

void VAdvancedRenderLevel::RenderMobjsTextures()
{
	guard(VAdvancedRenderLevel::RenderMobjsTextures);
	if (!r_draw_mobjs || !r_models)
	{
		return;
	}

	for (TThinkerIterator<VEntity> Ent(Level); Ent; ++Ent)
	{
		RenderThingTextures(*Ent);
	}
	unguard;
}

//==========================================================================
//
//	VAdvancedRenderLevel::IsTouchedByLight
//
//==========================================================================

bool VAdvancedRenderLevel::IsTouchedByLight(VEntity* Ent)
{
	guard(VAdvancedRenderLevel::IsTouchedByLight);
	TVec Delta = Ent->Origin - CurrLightPos;
	float Dist = Ent->Radius + CurrLightRadius;
	if (fabs(Delta.x) > Dist || Delta.y > Dist)
	{
		return false;
	}
	if (Delta.z < -CurrLightRadius)
	{
		return false;
	}
	if (Delta.z > CurrLightRadius + Ent->Height)
	{
		return false;
	}
	Delta.z = 0;
	if (Delta.Length() > Dist)
	{
		return false;
	}
	return true;
	unguard;
}

//==========================================================================
//
//	VAdvancedRenderLevel::RenderThingLight
//
//==========================================================================

void VAdvancedRenderLevel::RenderThingLight(VEntity* mobj)
{
	guard(VAdvancedRenderLevel::RenderThingLight);
	//	Skip things in subsectors that are not visible.
	int SubIdx = mobj->SubSector - Level->Subsectors;
	if (!(LightBspVis[SubIdx >> 3] & (1 << (SubIdx & 7))))
	{
		return;
	}
	if (mobj == ViewEnt && (!r_chasecam || ViewEnt != cl->MO))
	{
		//	Don't draw camera actor.
		return;
	}

	if ((mobj->EntityFlags & VEntity::EF_NoSector) ||
		(mobj->EntityFlags & VEntity::EF_Invisible))
	{
		return;
	}
	if (!mobj->State)
	{
		return;
	}

	if (!IsTouchedByLight(mobj))
	{
		return;
	}

	int RendStyle = mobj->RenderStyle;
	float Alpha = mobj->Alpha;

	if (RendStyle == STYLE_SoulTrans)
	{
		RendStyle = STYLE_Translucent;
		Alpha = transsouls;
	}
	else if (RendStyle == STYLE_OptFuzzy)
	{
		RendStyle = r_drawfuzz ? STYLE_Fuzzy : STYLE_Translucent;
	}

	switch (RendStyle)
	{
	case STYLE_None:
		return;

	case STYLE_Normal:
		Alpha = 1.0;
		break;

	case STYLE_Fuzzy:
		return;

	case STYLE_Add:
		return;
	}
	Alpha = MID(0.0, Alpha, 1.0);

	if (Alpha < 1.0)
	{
		return;
	}

	float TimeFrac = 0;
	if (mobj->State->Time > 0)
	{
		TimeFrac = 1.0 - (mobj->StateTime / mobj->State->Time);
		TimeFrac = MID(0.0, TimeFrac, 1.0);
	}

	DrawEntityModel(mobj, 0xffffffff, 0, 1, false, TimeFrac, RPASS_Light);
	unguard;
}

//==========================================================================
//
//	VAdvancedRenderLevel::RenderMobjsLight
//
//==========================================================================

void VAdvancedRenderLevel::RenderMobjsLight()
{
	guard(VAdvancedRenderLevel::RenderMobjsLight);
	if (!r_draw_mobjs || !r_models || !r_model_light)
	{
		return;
	}

	for (TThinkerIterator<VEntity> Ent(Level); Ent; ++Ent)
	{
		RenderThingLight(*Ent);
	}
	unguard;
}

//==========================================================================
//
//	VAdvancedRenderLevel::RenderThingShadow
//
//==========================================================================

void VAdvancedRenderLevel::RenderThingShadow(VEntity* mobj)
{
	guard(VAdvancedRenderLevel::RenderThingShadow);
	//	Skip things in subsectors that are not visible.
	int SubIdx = mobj->SubSector - Level->Subsectors;
	if (!(LightVis[SubIdx >> 3] & (1 << (SubIdx & 7))))
	{
		return;
	}
	// Draw camera actor for shadow pass
/*	if (mobj == ViewEnt && (!r_chasecam || ViewEnt != cl->MO))
	{
		//	Don't draw camera actor.
		return;
	}*/
	if ((mobj->EntityFlags & VEntity::EF_NoSector) ||
		(mobj->EntityFlags & VEntity::EF_Invisible))
	{
		return;
	}
	if (!mobj->State)
	{
		return;
	}
	if (!IsTouchedByLight(mobj))
	{
		return;
	}

	int RendStyle = mobj->RenderStyle;
	float Alpha = mobj->Alpha;

	if (RendStyle == STYLE_SoulTrans)
	{
		RendStyle = STYLE_Translucent;
		Alpha = transsouls;
	}
	else if (RendStyle == STYLE_OptFuzzy)
	{
		RendStyle = r_drawfuzz ? STYLE_Fuzzy : STYLE_Translucent;
	}

	switch (RendStyle)
	{
	case STYLE_None:
		return;

	case STYLE_Normal:
		Alpha = 1.0;
		break;

	case STYLE_Fuzzy:
		return;

	case STYLE_Add:
		return;
	}
	Alpha = MID(0.0, Alpha, 1.0);

	if (Alpha < 1.0)
	{
		return;
	}

	float TimeFrac = 0;
	if (mobj->State->Time > 0)
	{
		TimeFrac = 1.0 - (mobj->StateTime / mobj->State->Time);
		TimeFrac = MID(0.0, TimeFrac, 1.0);
	}

	DrawEntityModel(mobj, 0xffffffff, 0, 1, false, TimeFrac, RPASS_ShadowVolumes);
	unguard;
}

//==========================================================================
//
//	VAdvancedRenderLevel::RenderMobjsShadow
//
//==========================================================================

void VAdvancedRenderLevel::RenderMobjsShadow()
{
	guard(VAdvancedRenderLevel::RenderMobjsShadow);
	if (!r_draw_mobjs || !r_models || !r_model_shadows)
	{
		return;
	}

	for (TThinkerIterator<VEntity> Ent(Level); Ent; ++Ent)
	{
		RenderThingShadow(*Ent);
	}
	unguard;
}

//==========================================================================
//
//	VAdvancedRenderLevel::RenderThingFog
//
//==========================================================================

void VAdvancedRenderLevel::RenderThingFog(VEntity* mobj)
{
	guard(VAdvancedRenderLevel::RenderThingFog);
	//	Skip things in subsectors that are not visible.
	int SubIdx = mobj->SubSector - Level->Subsectors;
	if (!(BspVis[SubIdx >> 3] & (1 << (SubIdx & 7))))
	{
		return;
	}

	if (mobj == ViewEnt && (!r_chasecam || ViewEnt != cl->MO))
	{
		//	Don't draw camera actor.
		return;
	}

	if ((mobj->EntityFlags & VEntity::EF_NoSector) ||
		(mobj->EntityFlags & VEntity::EF_Invisible))
	{
		return;
	}
	if (!mobj->State)
	{
		return;
	}

	int RendStyle = mobj->RenderStyle;
	float Alpha = mobj->Alpha;

	if (RendStyle == STYLE_SoulTrans)
	{
		RendStyle = STYLE_Translucent;
		Alpha = transsouls;
	}
	else if (RendStyle == STYLE_OptFuzzy)
	{
		RendStyle = r_drawfuzz ? STYLE_Fuzzy : STYLE_Translucent;
	}

	switch (RendStyle)
	{
	case STYLE_None:
		return;

	case STYLE_Normal:
		Alpha = 1.0;
		break;

	case STYLE_Fuzzy:
		return;

	case STYLE_Add:
		return;
	}
	Alpha = MID(0.0, Alpha, 1.0);

	if (Alpha < 1.0)
	{
		return;
	}
	vuint32 Fade = GetFade(SV_PointInRegion(mobj->Sector, mobj->Origin));
	if (!Fade)
	{
		return;
	}

	float TimeFrac = 0;
	if (mobj->State->Time > 0)
	{
		TimeFrac = 1.0 - (mobj->StateTime / mobj->State->Time);
		TimeFrac = MID(0.0, TimeFrac, 1.0);
	}

	DrawEntityModel(mobj, 0xffffffff, Fade, 1, false, TimeFrac, RPASS_Fog);
	unguard;
}

//==========================================================================
//
//	VAdvancedRenderLevel::RenderMobjsFog
//
//==========================================================================

void VAdvancedRenderLevel::RenderMobjsFog()
{
	guard(VAdvancedRenderLevel::RenderMobjsFog);
	if (!r_draw_mobjs || !r_models)
	{
		return;
	}

	for (TThinkerIterator<VEntity> Ent(Level); Ent; ++Ent)
	{
		RenderThingFog(*Ent);
	}
	unguard;
}
