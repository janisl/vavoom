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
//	VAdvancedRenderLevel::DrawTranslucentPoly
//
//==========================================================================

void VAdvancedRenderLevel::DrawTranslucentPoly(surface_t* surf, TVec* sv, int count,
	int lump, float Alpha, bool Additive, int translation, bool type,
	vuint32 light, vuint32 Fade, const TVec& normal, float pdist,
	const TVec& saxis, const TVec& taxis, const TVec& texorg)
{
	guard(VAdvancedRenderLevel::DrawTranslucentPoly);
	int i;

	TVec mid(0, 0, 0);
	for (i = 0; i < count; i++)
	{
		mid += sv[i];
	}
	mid /= count;
	float dist = fabs(DotProduct(mid - vieworg, viewforward));
//	float dist = Length(mid - vieworg);
	int found = -1;
	float best_dist = -1;
	for (i = 0; i < MAX_TRANS_SPRITES; i++)
	{
		trans_sprite_t &spr = trans_sprites[i];
		if (!spr.Alpha)
		{
			if (type)
				memcpy(spr.Verts, sv, sizeof(TVec) * 4);
			spr.dist = dist;
			spr.lump = lump;
			spr.normal = normal;
			spr.pdist = pdist;
			spr.saxis = saxis;
			spr.taxis = taxis;
			spr.texorg = texorg;
			spr.surf = surf;
			spr.Alpha = Alpha;
			spr.Additive = Additive;
			spr.translation = translation;
			spr.type = type;
			spr.light = light;
			spr.Fade = Fade;
			return;
		}
		if (spr.dist > best_dist)
		{
			found = i;
			best_dist = spr.dist;
		}
	}
	if (best_dist > dist)
	{
		//	All slots are full, draw and replace a far away sprite
		trans_sprite_t &spr = trans_sprites[found];
		if (spr.type == 2)
		{
			DrawEntityModel(spr.Ent, spr.light, spr.Fade, spr.Alpha,
				spr.Additive, spr.TimeFrac, RPASS_NonShadow);
		}
		else if (spr.type)
		{
			Drawer->DrawSpritePolygon(spr.Verts, GTextureManager[spr.lump],
				spr.Alpha, spr.Additive, GetTranslation(spr.translation),
				ColourMap, spr.light, spr.Fade, spr.normal, spr.pdist,
				spr.saxis, spr.taxis, spr.texorg);
		}
		else
		{
			check(spr.surf);
			Drawer->DrawMaskedPolygon(spr.surf, spr.Alpha, spr.Additive);
		}
		if (type)
			memcpy(spr.Verts, sv, sizeof(TVec) * 4);
		spr.dist = dist;
		spr.lump = lump;
		spr.normal = normal;
		spr.pdist = pdist;
		spr.saxis = saxis;
		spr.taxis = taxis;
		spr.texorg = texorg;
		spr.surf = surf;
		spr.Alpha = Alpha;
		spr.Additive = Additive;
		spr.translation = translation;
		spr.type = type;
		spr.light = light;
		spr.Fade = Fade;
		return;
	}

	//	All slots are full and are nearer to current sprite so draw it
	if (type)
	{
		Drawer->DrawSpritePolygon(sv, GTextureManager[lump], Alpha,
			Additive, GetTranslation(translation), ColourMap, light, Fade,
			normal, pdist, saxis, taxis, texorg);
	}
	else
	{
		check(surf);
		Drawer->DrawMaskedPolygon(surf, Alpha, Additive);
	}
	unguard;
}

//==========================================================================
//
//	VAdvancedRenderLevel::RenderSprite
//
//==========================================================================

void VAdvancedRenderLevel::RenderSprite(VEntity* thing, vuint32 light, vuint32 Fade,
	float Alpha, bool Additive)
{
	guard(VAdvancedRenderLevel::RenderSprite);
	int spr_type = thing->SpriteType;

	TVec sprorigin = thing->Origin;
	sprorigin.z -= thing->FloorClip;
	TVec sprforward;
	TVec sprright;
	TVec sprup;

	float		dot;
	TVec		tvec;
	float		sr;
	float		cr;

	switch (spr_type)
	{
	case SPR_VP_PARALLEL_UPRIGHT:
		//	Generate the sprite's axes, with sprup straight up in worldspace,
		// and sprright parallel to the viewplane. This will not work if the
		// view direction is very close to straight up or down, because the
		// cross product will be between two nearly parallel vectors and
		// starts to approach an undefined state, so we don't draw if the two
		// vectors are less than 1 degree apart
		dot = viewforward.z;	//	same as DotProduct(viewforward, sprup)
								// because sprup is 0, 0, 1
		if ((dot > 0.999848) || (dot < -0.999848))	// cos(1 degree) = 0.999848
			return;

		sprup = TVec(0, 0, 1);
		//	CrossProduct(sprup, viewforward)
		sprright = Normalise(TVec(viewforward.y, -viewforward.x, 0));
		//	CrossProduct(sprright, sprup)
		sprforward = TVec(-sprright.y, sprright.x, 0);
		break;

	case SPR_FACING_UPRIGHT:
		//	Generate the sprite's axes, with sprup straight up in worldspace,
		// and sprright perpendicular to sprorigin. This will not work if the
		// view direction is very close to straight up or down, because the
		// cross product will be between two nearly parallel vectors and
		// starts to approach an undefined state, so we don't draw if the two
		// vectors are less than 1 degree apart
		tvec = Normalise(sprorigin - vieworg);
		dot = tvec.z;	//	same as DotProduct (tvec, sprup) because
						// sprup is 0, 0, 1
		if ((dot > 0.999848) || (dot < -0.999848))	// cos(1 degree) = 0.999848
			return;
		sprup = TVec(0, 0, 1);
		//	CrossProduct(sprup, -sprorigin)
		sprright = Normalise(TVec(tvec.y, -tvec.x, 0));
		//	CrossProduct(sprright, sprup)
		sprforward = TVec(-sprright.y, sprright.x, 0);
		break;

	case SPR_VP_PARALLEL:
		//	Generate the sprite's axes, completely parallel to the viewplane.
		// There are no problem situations, because the sprite is always in
		// the same position relative to the viewer
		sprup = viewup;
		sprright = viewright;
		sprforward = viewforward;
		break;

	case SPR_ORIENTED:
		//	Generate the sprite's axes, according to the sprite's world
		// orientation
		AngleVectors(thing->Angles, sprforward, sprright, sprup);
		break;

	case SPR_VP_PARALLEL_ORIENTED:
		//	Generate the sprite's axes, parallel to the viewplane, but
		// rotated in that plane around the centre according to the sprite
		// entity's roll angle. So sprforward stays the same, but sprright
		// and sprup rotate
		sr = msin(thing->Angles.roll);
		cr = mcos(thing->Angles.roll);

		sprforward = viewforward;
		sprright = TVec(viewright.x * cr + viewup.x * sr, viewright.y * cr +
			viewup.y * sr, viewright.z * cr + viewup.z * sr);
		sprup = TVec(viewright.x * -sr + viewup.x * cr, viewright.y * -sr +
			viewup.y * cr, viewright.z * -sr + viewup.z * cr);
		break;

	case SPR_VP_PARALLEL_UPRIGHT_ORIENTED:
		//	Generate the sprite's axes, with sprup straight up in worldspace,
		// and sprright parallel to the viewplane and then rotated in that
		// plane around the centre according to the sprite entity's roll
		// angle. So sprforward stays the same, but sprright and sprup rotate
		// This will not work if the view direction is very close to straight
		// up or down, because the cross product will be between two nearly
		// parallel vectors and starts to approach an undefined state, so we
		// don't draw if the two vectors are less than 1 degree apart
		dot = viewforward.z;	//	same as DotProduct(viewforward, sprup)
								// because sprup is 0, 0, 1
		if ((dot > 0.999848) || (dot < -0.999848))	// cos(1 degree) = 0.999848
			return;

		sr = msin(thing->Angles.roll);
		cr = mcos(thing->Angles.roll);

		//	CrossProduct(TVec(0, 0, 1), viewforward)
		tvec = Normalise(TVec(viewforward.y, -viewforward.x, 0));
		//	CrossProduct(tvec, TVec(0, 0, 1))
		sprforward = TVec(-tvec.y, tvec.x, 0);
		//	Rotate
		sprright = TVec(tvec.x * cr, tvec.y * cr, tvec.z * cr + sr);
		sprup = TVec(tvec.x * -sr, tvec.y * -sr, tvec.z * -sr + cr);
		break;

	default:
		Sys_Error("RenderSprite: Bad sprite type %d", spr_type);
	}

	spritedef_t*	sprdef;
	spriteframe_t*	sprframe;

	VState* DispState = (thing->EntityFlags & VEntity::EF_UseDispState) ?
		thing->DispState : thing->State;
	int SpriteIndex = DispState->SpriteIndex;
	if (thing->FixedSpriteName != NAME_None)
	{
		SpriteIndex = VClass::FindSprite(thing->FixedSpriteName);
	}

	// decide which patch to use for sprite relative to player
	if ((unsigned)SpriteIndex >= MAX_SPRITE_MODELS)
	{
#ifdef PARANOID
		GCon->Logf(NAME_Dev, "Invalid sprite number %d", SpriteIndex);
#endif
		return;
	}
	sprdef = &sprites[SpriteIndex];
	if ((DispState->Frame & VState::FF_FRAMEMASK) >= sprdef->numframes)
	{
#ifdef PARANOID
		GCon->Logf(NAME_Dev, "Invalid sprite frame %d : %d",
			SpriteIndex, DispState->Frame);
#endif
		return;
	}
	sprframe = &sprdef->spriteframes[DispState->Frame & VState::FF_FRAMEMASK];

	int			lump;
	bool		flip;

	if (sprframe->rotate)
	{
		// choose a different rotation based on player view
		//FIXME must use sprforward here?
		float ang = matan(thing->Origin.y - vieworg.y,
			thing->Origin.x - vieworg.x);
		if (sprframe->lump[0] == sprframe->lump[1])
		{
			ang = AngleMod(ang - thing->Angles.yaw + 180.0 + 45.0 / 2.0);
		}
		else
		{
			ang = AngleMod(ang - thing->Angles.yaw + 180.0 + 45.0 / 4.0);
		}
		vuint32 rot = (vuint32)(ang * 16 / 360.0) & 15;
		lump = sprframe->lump[rot];
		flip = sprframe->flip[rot];
	}
	else
	{
		// use single rotation for all views
		lump = sprframe->lump[0];
		flip = sprframe->flip[0];
	}
	if (lump <= 0)
	{
#ifdef PARANOID
		GCon->Logf(NAME_Dev, "Sprite frame %d : %d, not present",
			SpriteIndex, DispState->Frame);
#endif
		// Sprite lump is not present
		return;
	}
	VTexture* Tex = GTextureManager[lump];
	int TexWidth = Tex->GetWidth();
	int TexHeight = Tex->GetHeight();
	int TexSOffset = Tex->SOffset;
	int TexTOffset = Tex->TOffset;

	TVec	sv[4];

	TVec start = -TexSOffset * sprright * thing->ScaleX;
	TVec end = (TexWidth - TexSOffset) * sprright * thing->ScaleX;

	if (r_fix_sprite_offsets && TexTOffset < TexHeight &&
		2 * TexTOffset + r_sprite_fix_delta >= TexHeight)
	{
		TexTOffset = TexHeight;
	}
	TVec topdelta = TexTOffset * sprup * thing->ScaleY;
	TVec botdelta = (TexTOffset - TexHeight) * sprup * thing->ScaleY;

	sv[0] = sprorigin + start + botdelta;
	sv[1] = sprorigin + start + topdelta;
	sv[2] = sprorigin + end + topdelta;
	sv[3] = sprorigin + end + botdelta;

	if (Alpha < 1.0 || Additive || r_sort_sprites)
	{
		DrawTranslucentPoly(NULL, sv, 4, lump, Alpha, Additive,
			thing->Translation, true, light, Fade, -sprforward, DotProduct(
			sprorigin, -sprforward), (flip ? -sprright : sprright) /
			thing->ScaleX, -sprup / thing->ScaleY, flip ? sv[2] : sv[1]);
	}
	else
	{
		Drawer->DrawSpritePolygon(sv, GTextureManager[lump], Alpha,
			Additive, GetTranslation(thing->Translation), ColourMap, light,
			Fade, -sprforward, DotProduct(sprorigin, -sprforward),
			(flip ? -sprright : sprright) / thing->ScaleX,
			-sprup / thing->ScaleY, flip ? sv[2] : sv[1]);
	}
	unguard;
}

//==========================================================================
//
//	VAdvancedRenderLevel::RenderTranslucentAliasModel
//
//==========================================================================

void VAdvancedRenderLevel::RenderTranslucentAliasModel(VEntity* mobj, vuint32 light,
	vuint32 Fade, float Alpha, bool Additive, float TimeFrac)
{
	guard(VAdvancedRenderLevel::RenderTranslucentAliasModel);
	int i;

	float dist = fabs(DotProduct(mobj->Origin - vieworg, viewforward));
	int found = -1;
	float best_dist = -1;
	for (i = 0; i < MAX_TRANS_SPRITES; i++)
	{
		trans_sprite_t &spr = trans_sprites[i];
		if (!spr.Alpha)
		{
			spr.Ent = mobj;
			spr.light = light;
			spr.Fade = Fade;
			spr.Alpha = Alpha;
			spr.Additive = Additive;
			spr.dist = dist;
			spr.type = 2;
			spr.TimeFrac = TimeFrac;
			return;
		}
		if (spr.dist > best_dist)
		{
			found = i;
			best_dist = spr.dist;
		}
	}
	if (best_dist > dist)
	{
		//	All slots are full, draw and replace a far away sprite
		trans_sprite_t &spr = trans_sprites[found];
		if (spr.type == 2)
		{
			DrawEntityModel(spr.Ent, spr.light, spr.Fade, spr.Alpha,
				spr.Additive, spr.TimeFrac, RPASS_NonShadow);
		}
		else if (spr.type)
		{
			Drawer->DrawSpritePolygon(spr.Verts, GTextureManager[spr.lump],
				spr.Alpha, spr.Additive, GetTranslation(spr.translation),
				ColourMap, spr.light, spr.Fade, spr.normal, spr.pdist,
				spr.saxis, spr.taxis, spr.texorg);
		}
		else
		{
			check(spr.surf);
			Drawer->DrawMaskedPolygon(spr.surf, spr.Alpha, spr.Additive);
		}
		spr.Ent = mobj;
		spr.light = light;
		spr.Fade = Fade;
		spr.Alpha = Alpha;
		spr.Additive = Additive;
		spr.dist = dist;
		spr.type = 2;
		spr.TimeFrac = TimeFrac;
		return;
	}
	DrawEntityModel(mobj, light, Fade, Alpha, Additive, TimeFrac, RPASS_NonShadow);
	unguard;
}

//==========================================================================
//
//	VAdvancedRenderLevel::RenderAliasModel
//
//==========================================================================

bool VAdvancedRenderLevel::RenderAliasModel(VEntity* mobj, vuint32 light,
	vuint32 Fade, float Alpha, bool Additive)
{
	guard(VAdvancedRenderLevel::RenderAliasModel);
	if (!r_models)
	{
		return false;
	}

	float TimeFrac = 0;
	if (mobj->State->Time > 0)
	{
		TimeFrac = 1.0 - (mobj->StateTime / mobj->State->Time);
		TimeFrac = MID(0.0, TimeFrac, 1.0);
	}

	//	Draw it
	if (Alpha < 1.0 || Additive)
	{
		if (!CheckAliasModelFrame(mobj, TimeFrac))
		{
			return false;
		}
		RenderTranslucentAliasModel(mobj, light, Fade, Alpha, Additive,
			TimeFrac);
		return true;
	}
	else
	{
		return DrawEntityModel(mobj, light, Fade, 1.0, false, TimeFrac,
			RPASS_NonShadow);
	}
	unguard;
}

//==========================================================================
//
//	VAdvancedRenderLevel::RenderThing
//
//==========================================================================

void VAdvancedRenderLevel::RenderThing(VEntity* mobj)
{
	guard(VAdvancedRenderLevel::RenderThing);
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
	bool Additive = false;

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
		Alpha = 0.1;
		break;

	case STYLE_Add:
		Additive = true;
		break;
	}
	Alpha = MID(0.0, Alpha, 1.0);

	if (!Alpha)
	{
		// Never make a vissprite when MF2_DONTDRAW is flagged.
		return;
	}

	//	Setup lighting
	vuint32 light;
	if (RendStyle == STYLE_Fuzzy)
	{
		light = 0;
	}
	else if ((mobj->State->Frame & VState::FF_FULLBRIGHT) ||
		(mobj->EntityFlags & (VEntity::EF_FullBright | VEntity::EF_Bright)))
	{
		light = 0xffffffff;
	}
	else
	{
		light = LightPoint(mobj->Origin);
	}
	vuint32 Fade = GetFade(SV_PointInRegion(mobj->Sector, mobj->Origin));

	//	Try to draw a model. If it's a script and it doesn't
	// specify model for this frame, draw sprite instead.
	if (!RenderAliasModel(mobj, light, Fade, Alpha, Additive))
	{
		RenderSprite(mobj, light, Fade, Alpha, Additive);
	}
	unguard;
}

//==========================================================================
//
//	VAdvancedRenderLevel::RenderMobjs
//
//==========================================================================

void VAdvancedRenderLevel::RenderMobjs()
{
	guard(VAdvancedRenderLevel::RenderMobjs);
	if (!r_draw_mobjs)
	{
		return;
	}

	for (TThinkerIterator<VEntity> Ent(Level); Ent; ++Ent)
	{
		RenderThing(*Ent);
	}
	unguard;
}

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

//==========================================================================
//
//	VAdvancedRenderLevel::DrawTranslucentPolys
//
//==========================================================================

void VAdvancedRenderLevel::DrawTranslucentPolys()
{
	guard(VAdvancedRenderLevel::DrawTranslucentPolys);
	int i, found;
	do
	{
		found = -1;
		float best_dist = -1;
		for (i = 0; i < MAX_TRANS_SPRITES; i++)
		{
			trans_sprite_t &spr = trans_sprites[i];
			if (!spr.Alpha)
			{
				continue;
			}
			if (spr.dist > best_dist)
			{
				found = i;
				best_dist = spr.dist;
			}
		}
		if (found != -1)
		{
			trans_sprite_t &spr = trans_sprites[found];
			if (spr.type == 2)
			{
				DrawEntityModel(spr.Ent, spr.light, spr.Fade, spr.Alpha,
					spr.Additive, spr.TimeFrac, RPASS_NonShadow);
			}
			else if (spr.type)
			{
				Drawer->DrawSpritePolygon(spr.Verts, GTextureManager[spr.lump],
					spr.Alpha, spr.Additive, GetTranslation(spr.translation),
					ColourMap, spr.light, spr.Fade, spr.normal, spr.pdist,
					spr.saxis, spr.taxis, spr.texorg);
			}
			else
			{
				check(spr.surf);
				Drawer->DrawMaskedPolygon(spr.surf, spr.Alpha, spr.Additive);
			}
			spr.Alpha = 0;
		}
	} while (found != -1);
	unguard;
}

//==========================================================================
//
//	VAdvancedRenderLevel::RenderPSprite
//
//==========================================================================

void VAdvancedRenderLevel::RenderPSprite(VViewState* VSt, float PSP_DIST,
	vuint32 light, vuint32 Fade, float Alpha, bool Additive)
{
	guard(VAdvancedRenderLevel::RenderPSprite);
	spritedef_t*		sprdef;
	spriteframe_t*		sprframe;
	int					lump;
	bool				flip;

	// decide which patch to use
	if ((vuint32)VSt->State->SpriteIndex >= MAX_SPRITE_MODELS)
	{
#ifdef PARANOID
		GCon->Logf("R_ProjectSprite: invalid sprite number %d",
			VSt->State->SpriteIndex);
#endif
		return;
	}
	sprdef = &sprites[VSt->State->SpriteIndex];
	if ((VSt->State->Frame & VState::FF_FRAMEMASK) >= sprdef->numframes)
	{
#ifdef PARANOID
		GCon->Logf("R_ProjectSprite: invalid sprite frame %d : %d",
			VSt->State->SpriteIndex, VSt->State->Frame);
#endif
		return;
	}
	sprframe = &sprdef->spriteframes[VSt->State->Frame & VState::FF_FRAMEMASK];

	lump = sprframe->lump[0];
	flip = sprframe->flip[0];
	VTexture* Tex = GTextureManager[lump];

	int TexWidth = Tex->GetWidth();
	int TexHeight = Tex->GetHeight();
	int TexSOffset = Tex->SOffset;
	int TexTOffset = Tex->TOffset;

	TVec	dv[4];

	float PSP_DISTI = 1.0 / PSP_DIST;
	TVec sprorigin = vieworg + PSP_DIST * viewforward;

	float sprx = 160.0 - VSt->SX + TexSOffset;
	float spry = 100.0 - VSt->SY + TexTOffset;

	spry -= cl->PSpriteSY;

	//	1 / 160 = 0.00625
	TVec start = sprorigin - (sprx * PSP_DIST * 0.00625) * viewright;
	TVec end = start + (TexWidth * PSP_DIST * 0.00625) * viewright;

	//	1 / 160.0 * 120 / 100 =	0.0075
	TVec topdelta = (spry * PSP_DIST * 0.0075) * viewup;
	TVec botdelta = topdelta - (TexHeight * PSP_DIST * 0.0075) * viewup;
	if (old_aspect)
	{
		topdelta *= 100.0 / 120.0;
		botdelta *= 100.0 / 120.0;
	}

	dv[0] = start + botdelta;
	dv[1] = start + topdelta;
	dv[2] = end + topdelta;
	dv[3] = end + botdelta;

	TVec saxis;
	TVec taxis;
	TVec texorg;
	if (flip)
	{
		saxis = -(viewright * 160 * PSP_DISTI);
		texorg = dv[2];
	}
	else
	{
		saxis = viewright * 160 * PSP_DISTI;
		texorg = dv[1];
	}
	if (old_aspect)
		taxis = -(viewup * 160 * PSP_DISTI);
	else
		taxis = -(viewup * 100 * 4 / 3 * PSP_DISTI);

	Drawer->DrawSpritePolygon(dv, GTextureManager[lump], Alpha, Additive,
		0, ColourMap, light, Fade, -viewforward,
		DotProduct(dv[0], -viewforward), saxis, taxis, texorg);
	unguard;
}

//==========================================================================
//
//	VAdvancedRenderLevel::RenderViewModel
//
//==========================================================================

bool VAdvancedRenderLevel::RenderViewModel(VViewState* VSt, vuint32 light,
	vuint32 Fade, float Alpha, bool Additive)
{
	guard(VAdvancedRenderLevel::RenderViewModel);
	if (!r_view_models)
	{
		return false;
	}

	TVec origin = vieworg + (VSt->SX - 1.0) * viewright / 8.0 -
		(VSt->SY - 32.0) * viewup / 6.0;

	float TimeFrac = 0;
	if (VSt->State->Time > 0)
	{
		TimeFrac = 1.0 - (VSt->StateTime / VSt->State->Time);
		TimeFrac = MID(0.0, TimeFrac, 1.0);
	}

	bool Interpolate;
	// Check if we want to interpolate model frames
	if (!r_interpolate_frames)
	{
		Interpolate = false;
	}
	else
	{
		Interpolate = true;
	}
	return DrawAliasModel(origin, cl->ViewAngles, 1.0, 1.0, VSt->State,
		VSt->State->NextState ? VSt->State->NextState : VSt->State, NULL,
		0, light, Fade, Alpha, Additive, true, TimeFrac, Interpolate,
		RPASS_NonShadow);
	unguard;
}

//==========================================================================
//
//	VAdvancedRenderLevel::DrawPlayerSprites
//
//==========================================================================

void VAdvancedRenderLevel::DrawPlayerSprites()
{
	guard(VAdvancedRenderLevel::DrawPlayerSprites);
	if (!r_draw_psprites || r_chasecam)
	{
		return;
	}

	int RendStyle = STYLE_Normal;
	float Alpha = 1.0;
	bool Additive = false;

	cl->MO->eventGetViewEntRenderParams(Alpha, RendStyle);

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
		Alpha = 0.1;
		break;

	case STYLE_Add:
		Additive = true;
		break;
	}
	Alpha = MID(0.0, Alpha, 1.0);

	// add all active psprites
	for (int i = 0; i < NUMPSPRITES; i++)
	{
		if (!cl->ViewStates[i].State)
		{
			continue;
		}

		vuint32 light;
		if (RendStyle == STYLE_Fuzzy)
		{
			light = 0;
		}
		else if (cl->ViewStates[i].State->Frame & VState::FF_FULLBRIGHT)
		{
			light = 0xffffffff;
		}
		else
		{
			light = LightPoint(vieworg);
		}
		vuint32 Fade = GetFade(SV_PointInRegion(r_viewleaf->sector, cl->ViewOrg));

		if (!RenderViewModel(&cl->ViewStates[i], light, Fade, Alpha,
			Additive))
		{
			RenderPSprite(&cl->ViewStates[i], 3 - i, light, Fade, Alpha,
				Additive);
		}
	}
	unguard;
}

//==========================================================================
//
//	VAdvancedRenderLevel::DrawCroshair
//
//==========================================================================

void VAdvancedRenderLevel::DrawCroshair()
{
	guard(VAdvancedRenderLevel::DrawCroshair);
	if (croshair)
	{
		if (croshair_alpha < 0.0)	croshair_alpha = 0.0;
		if (croshair_alpha > 1.0)	croshair_alpha = 1.0;

		int			cy;
		if (screenblocks < 11)
			cy = (480 - sb_height) / 2;
		else
			cy = 240;
		int handle = GTextureManager.AddPatch(VName(va("CROSHAI%i",
			(int)croshair), VName::AddLower8), TEXTYPE_Pic);
		R_DrawPic(320, cy, handle, croshair_alpha);
	}
	unguard;
}
