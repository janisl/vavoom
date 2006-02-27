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
//**	Copyright (C) 1999-2002 JÆnis Legzdi·ý
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

// MACROS ------------------------------------------------------------------

#define MAX_SPRITE_MODELS	512
#define MAX_TRANS_SPRITES	256

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

//
//  Sprites are patches with a special naming convention
// so they can be recognized by R_InitSprites.
//  The base name is NNNNFx or NNNNFxFx, with
// x indicating the rotation, x = 0, 1-7.
//  The sprite and frame specified by a thing_t
// is range checked at run time.
//  A sprite is a patch_t that is assumed to represent
// a three dimensional object and may have multiple
// rotations pre drawn.
//  Horizontal flipping is used to save space,
// thus NNNNF2F5 defines a mirrored patch.
//  Some sprites will only have one picture used
// for all views: NNNNF0
//
struct spriteframe_t
{
	// If false use 0 for any position.
	// Note: as eight entries are available,
	//  we might as well insert the same name eight times.
	boolean     rotate;

    // Lump to use for view angles 0-7.
	short		lump[8];

	// Flip bit (1 = flip) to use for view angles 0-7.
	bool		flip[8];
};

//
// 	A sprite definition:
// a number of animation frames.
//
struct spritedef_t
{
	int					numframes;
	spriteframe_t		*spriteframes;
};

struct trans_sprite_t
{
	TVec		*dv;
	surface_t	*surf;
	int			count;
	int			lump;
	TVec		normal;
	float		pdist;
	TVec		saxis;
	TVec		taxis;
	TVec		texorg;
	int			translucency;
	union
	{
		int		translation;
		char	*skin;
	};
	int			type;
	float		dist;
	dword		light;
};

// EXTERNAL FUNCTION PROTOTYPES --------------------------------------------

// PUBLIC FUNCTION PROTOTYPES ----------------------------------------------

// PRIVATE FUNCTION PROTOTYPES ---------------------------------------------

// EXTERNAL DATA DECLARATIONS ----------------------------------------------

// PUBLIC DATA DEFINITIONS -------------------------------------------------

TVec				r_saxis;
TVec				r_taxis;
TVec				r_texorg;

// PRIVATE DATA DEFINITIONS ------------------------------------------------

//	Temporary variables for sprite installing
static spriteframe_t	sprtemp[30];
static int				maxframe;
static const char		*spritename;

//	variables used to look up
// and range check thing_t sprites patches
static spritedef_t		sprites[MAX_SPRITE_MODELS];

static TCvarI			r_draw_mobjs("r_draw_mobjs", "1", CVAR_ARCHIVE);
static TCvarI			r_draw_psprites("r_draw_psprites", "1", CVAR_ARCHIVE);
static TCvarI			r_models("r_models", "1", CVAR_ARCHIVE);
static TCvarI			r_view_models("r_view_models", "1", CVAR_ARCHIVE);
static TCvarI			r_sort_sprites("r_sort_sprites", "0");
static TCvarI			r_fix_sprite_offsets("r_fix_sprite_offsets", "1", CVAR_ARCHIVE);
static TCvarI			r_sprite_fix_delta("r_sprite_fix_delta", "-3", CVAR_ARCHIVE);
static TCvarI			croshair("croshair", "0", CVAR_ARCHIVE);
static TCvarI			croshair_trans("croshair_trans", "0", CVAR_ARCHIVE);

static trans_sprite_t	trans_sprites[MAX_TRANS_SPRITES];
static TVec				trans_sprite_verts[4 * MAX_TRANS_SPRITES];

// CODE --------------------------------------------------------------------

//==========================================================================
//
//	InstallSpriteLump
//
//	Local function for R_InitSprites.
//
//==========================================================================

static void InstallSpriteLump(int lumpnr, int frame, int rotation, bool flipped)
{
	guard(InstallSpriteLump);
	int			r;

	TTexture* Tex = GTextureManager.Textures[lumpnr];
	if ((dword)frame >= 30 || (dword)rotation > 8)
	{
		Sys_Error("InstallSpriteLump: Bad frame characters in lump %s",
			*Tex->Name);
	}

	if (frame > maxframe)
		maxframe = frame;

	if (rotation == 0)
	{
		// the lump should be used for all rotations
		sprtemp[frame].rotate = false;
		for (r = 0; r < 8; r++)
		{
			sprtemp[frame].lump[r] = lumpnr;
			sprtemp[frame].flip[r] = flipped;
		}
     	return;
	}

	// the lump is only used for one rotation
	if (sprtemp[frame].rotate == false)
	{
		for (r = 0; r < 8; r++)
		{
			sprtemp[frame].lump[r] = -1;
			sprtemp[frame].flip[r] = false;
		}
	}

	sprtemp[frame].rotate = true;

	// make 0 based
	rotation--;

	sprtemp[frame].lump[rotation] = lumpnr;
	sprtemp[frame].flip[rotation] = flipped;
	unguard;
}

//==========================================================================
//
//	R_InstallSprite
//
// 	Builds the sprite rotation matrixes to account for horizontally flipped
// sprites. Will report an error if the lumps are inconsistant.
// 	Sprite lump names are 4 characters for the actor, a letter for the frame,
// and a number for the rotation. A sprite that is flippable will have an
// additional letter/number appended. The rotation character can be 0 to
// signify no rotations.
//
//==========================================================================

void R_InstallSprite(const char *name, int index)
{
	guard(R_InstallSprite);
	int			intname;
	int			frame;
	int			rotation;

	if ((dword)index >= MAX_SPRITE_MODELS)
	{
		Host_Error("Invalid sprite index %d for sprite %s", index, name);
	}
	spritename = name;
	memset(sprtemp, -1, sizeof(sprtemp));
	maxframe = -1;

    // scan all the lump names for each of the names,
    //  noting the highest frame letter.
    // Just compare 4 characters as ints
	intname = *(int*)*VName(spritename, VName::AddLower8);

	// scan the lumps, filling in the frames for whatever is found
	for (int l = 0; l < GTextureManager.Textures.Num(); l++)
	{
		if (GTextureManager.Textures[l]->Type == TEXTYPE_Sprite)
		{
			const char* lumpname = *GTextureManager.Textures[l]->Name;
			if (*(int*)lumpname == intname)
			{
				frame = toupper(lumpname[4]) - 'A';
				rotation = lumpname[5] - '0';
				InstallSpriteLump(l, frame, rotation, false);

				if (lumpname[6])
				{
					frame = toupper(lumpname[6]) - 'A';
					rotation = lumpname[7] - '0';
					InstallSpriteLump(l, frame, rotation, true);
				}
			}
		}
	}

	// check the frames that were found for completeness
	if (maxframe == -1)
	{
		sprites[index].numframes = 0;
		return;
	}

	maxframe++;

	for (frame = 0 ; frame < maxframe ; frame++)
	{
		switch ((int)sprtemp[frame].rotate)
		{
		 case -1:
			// no rotations were found for that frame at all
			Sys_Error("R_InstallSprite: No patches found "
					"for %s frame %c", spritename, frame + 'A');
			break;

		 case 0:
			// only the first rotation is needed
			break;

		 case 1:
			// must have all 8 frames
			for (rotation = 0; rotation < 8; rotation++)
			{
				if (sprtemp[frame].lump[rotation] == -1)
				{
					Sys_Error("R_InstallSprite: Sprite %s frame %c "
							"is missing rotations", spritename, frame + 'A');
				}
			}
			break;
		}
	}

	if (sprites[index].spriteframes)
	{
		Z_Free(sprites[index].spriteframes);
		sprites[index].spriteframes = NULL;
	}
	// allocate space for the frames present and copy sprtemp to it
	sprites[index].numframes = maxframe;
	sprites[index].spriteframes = (spriteframe_t*)
		Z_StrMalloc(maxframe * sizeof(spriteframe_t));
	memcpy(sprites[index].spriteframes, sprtemp, maxframe * sizeof(spriteframe_t));
	unguard;
}

//==========================================================================
//
//	R_AddTranslucentPoly
//
//==========================================================================

void R_DrawTranslucentPoly(TVec *sv, int count, int lump,
	int translucency, int translation, bool type, dword light)
{
	guard(R_DrawTranslucentPoly);
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
		if (!spr.translucency)
		{
			if (count <= 4)
				spr.dv = trans_sprite_verts + 4 * i;
			else
				spr.dv = (TVec*)Z_StrMalloc(sizeof(TVec) * count);
			memcpy(spr.dv, sv, sizeof(TVec) * count);
			spr.count = count;
			spr.dist = dist;
			spr.lump = lump;
			spr.normal = r_normal;
			spr.pdist = r_dist;
			spr.saxis = r_saxis;
			spr.taxis = r_taxis;
			spr.texorg = r_texorg;
			spr.surf = r_surface;
			spr.translucency = translucency + 1;
			spr.translation = translation;
			spr.type = type;
			spr.light = light;
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
		TVec tmp_normal = r_normal;
		float tmp_dist = r_dist;
		TVec tmp_saxis = r_saxis;
		TVec tmp_taxis = r_taxis;
		TVec tmp_texorg = r_texorg;
		surface_t *tmp_surf = r_surface;
		trans_sprite_t &spr = trans_sprites[found];
		r_normal = spr.normal;
		r_dist = spr.pdist;
		r_saxis = spr.saxis;
		r_taxis = spr.taxis;
		r_texorg = spr.texorg;
		r_surface = spr.surf;
		if (spr.type == 2)
		{
			Drawer->DrawAliasModel(spr.dv[0], ((TAVec *)spr.dv)[1],
				(VModel*)spr.surf, spr.lump, spr.count, spr.skin,
				spr.light, spr.translucency - 1, false);
		}
		else if (spr.type)
		{
			Drawer->DrawSpritePolygon(spr.dv, spr.lump,
				spr.translucency - 1, spr.translation, spr.light);
		}
		else
		{
			Drawer->DrawMaskedPolygon(spr.dv, spr.count, spr.lump,
				spr.translucency - 1);
			if (spr.count > 4)
				Z_Free(spr.dv);
		}
		if (count <= 4)
			spr.dv = trans_sprite_verts + 4 * found;
		else
			spr.dv = (TVec*)Z_StrMalloc(sizeof(TVec) * count);
		memcpy(spr.dv, sv, sizeof(TVec) * count);
		spr.count = count;
		spr.dist = dist;
		spr.lump = lump;
		spr.normal = tmp_normal;
		spr.pdist = tmp_dist;
		spr.saxis = tmp_saxis;
		spr.taxis = tmp_taxis;
		spr.texorg = tmp_texorg;
		spr.surf = tmp_surf;
		spr.translucency = translucency + 1;
		spr.translation = translation;
		spr.light = light;
		return;
	}

	//	All slots are full and are nearer to current sprite so draw it
	if (type)
	{
		Drawer->DrawSpritePolygon(sv, lump, translucency,
			translation, light);
	}
	else
	{
		Drawer->DrawMaskedPolygon(sv, count, lump, translucency);
	}
	unguard;
}

//==========================================================================
//
//	RenderSprite
//
//==========================================================================

extern TCvarI		r_chasecam;

static void RenderSprite(clmobj_t *thing)
{
	guard(RenderSprite);
	if (thing == &cl_mobjs[cl->clientnum + 1] && !r_chasecam)
	{
		//	Don't draw client's mobj
		return;
	}

	if (thing->translucency >= 95)
	{
		// Never make a vissprite when MF2_DONTDRAW is flagged.
		return;
	}

	int spr_type = thing->spritetype;

	TVec sprorigin = thing->origin;
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
		AngleVectors(thing->angles, sprforward, sprright, sprup);
		break;

	 case SPR_VP_PARALLEL_ORIENTED:
		//	Generate the sprite's axes, parallel to the viewplane, but
		// rotated in that plane around the center according to the sprite
		// entity's roll angle. So sprforward stays the same, but sprright
		// and sprup rotate
		sr = msin(thing->angles.roll);
		cr = mcos(thing->angles.roll);

		sprforward = viewforward;
		sprright = TVec(viewright.x * cr + viewup.x * sr, viewright.y * cr +
			viewup.y * sr, viewright.z * cr + viewup.z * sr);
		sprup = TVec(viewright.x * -sr + viewup.x * cr, viewright.y * -sr +
			viewup.y * cr, viewright.z * -sr + viewup.z * cr);
		break;

	 case SPR_VP_PARALLEL_UPRIGHT_ORIENTED:
		//	Generate the sprite's axes, with sprup straight up in worldspace,
		// and sprright parallel to the viewplane and then rotated in that
		// plane around the center according to the sprite entity's roll
		// angle. So sprforward stays the same, but sprright and sprup rotate
		// This will not work if the view direction is very close to straight
		// up or down, because the cross product will be between two nearly
		// parallel vectors and starts to approach an undefined state, so we
		// don't draw if the two vectors are less than 1 degree apart
		dot = viewforward.z;	//	same as DotProduct(viewforward, sprup)
								// because sprup is 0, 0, 1
		if ((dot > 0.999848) || (dot < -0.999848))	// cos(1 degree) = 0.999848
			return;

		sr = msin(thing->angles.roll);
		cr = mcos(thing->angles.roll);

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

	// decide which patch to use for sprite relative to player
	if ((unsigned)thing->sprite >= MAX_SPRITE_MODELS)
	{
#ifdef PARANOID
		GCon->Logf(NAME_Dev, "Invalid sprite number %d", thing->sprite);
#endif
		return;
	}
	sprdef = &sprites[thing->sprite];
	if ((thing->frame & FF_FRAMEMASK) >= sprdef->numframes)
	{
#ifdef PARANOID
		GCon->Logf(NAME_Dev, "Invalid sprite frame %d : %d",
			thing->sprite, thing->frame);
#endif
		return;
	}
	sprframe = &sprdef->spriteframes[thing->frame & FF_FRAMEMASK];

	int			lump;
	boolean		flip;

	if (sprframe->rotate)
	{
		// choose a different rotation based on player view
		//FIXME must use sprforward here?
		float ang = matan(thing->origin.y - vieworg.y,
			thing->origin.x - vieworg.x);
		ang = AngleMod(ang - thing->angles.yaw + 180.0 + 45.0 / 2.0);
		dword rot = (dword)(ang * 8 / 360.0) & 7;
		lump = sprframe->lump[rot];
		flip = (boolean)sprframe->flip[rot];
	}
	else
	{
		// use single rotation for all views
		lump = sprframe->lump[0];
		flip = (boolean)sprframe->flip[0];
	}

	TTexture* Tex = GTextureManager.Textures[lump];
	int TexWidth = Tex->GetWidth();
	int TexHeight = Tex->GetHeight();
	int TexSOffset = Tex->SOffset;
	int TexTOffset = Tex->TOffset;

	TVec	sv[4];

	TVec start = -TexSOffset * sprright;
	TVec end = (TexWidth - TexSOffset) * sprright;

	if (r_fix_sprite_offsets && TexTOffset < TexHeight &&
		2 * TexTOffset + r_sprite_fix_delta >= TexHeight)
	{
		TexTOffset = TexHeight;
	}
	TVec topdelta = TexTOffset * sprup;
	TVec botdelta = (TexTOffset - TexHeight) * sprup;

	sv[0] = sprorigin + start + botdelta;
	sv[1] = sprorigin + start + topdelta;
	sv[2] = sprorigin + end + topdelta;
	sv[3] = sprorigin + end + botdelta;

	r_normal = -sprforward;
	r_dist = DotProduct(sprorigin, r_normal);

	if (flip)
	{
		r_saxis = -sprright;
		r_texorg = sv[2];
	}
	else
	{
		r_saxis = sprright;
		r_texorg = sv[1];
	}
	r_taxis = -sprup;

	dword light;
	if (thing->frame & FF_FULLBRIGHT || fixedlight)
	{
		light = 0xffffffff;
	}
	else
	{
		light = R_LightPoint(sprorigin);
	}

	if (thing->translucency > 0 || r_sort_sprites)
	{
		R_DrawTranslucentPoly(sv, 4, lump,
			thing->translucency, thing->translation, true, light);
	}
	else
	{
		Drawer->DrawSpritePolygon(sv, lump, thing->translucency,
			thing->translation, light);
	}
	unguard;
}

//==========================================================================
//
//	RenderTranslucentAliasModel
//
//==========================================================================

void RenderTranslucentAliasModel(clmobj_t *mobj, dword light)
{
	guard(RenderTranslucentAliasModel);
	int i;

	float dist = fabs(DotProduct(mobj->origin - vieworg, viewforward));
	int found = -1;
	float best_dist = -1;
	for (i = 0; i < MAX_TRANS_SPRITES; i++)
	{
		trans_sprite_t &spr = trans_sprites[i];
		if (!spr.translucency)
		{
			spr.dv = trans_sprite_verts + 4 * i;
			spr.dv[0] = mobj->origin;
			((TAVec *)spr.dv)[1] = mobj->angles;
			spr.surf = (surface_t*)mobj->AliasModel;
			spr.lump = mobj->AliasFrame;
			spr.count = mobj->AliasSkinIndex;
			spr.light = light;
			spr.translucency = mobj->translucency + 1;
			spr.dist = dist;
			spr.type = 2;
			spr.skin = mobj->Skin;
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
		r_normal = spr.normal;
		r_dist = spr.pdist;
		r_saxis = spr.saxis;
		r_taxis = spr.taxis;
		r_texorg = spr.texorg;
		r_surface = spr.surf;
		if (spr.type == 2)
		{
			Drawer->DrawAliasModel(spr.dv[0], ((TAVec *)spr.dv)[1],
				(VModel*)spr.surf, spr.lump, spr.count, spr.skin,
				spr.light, spr.translucency - 1, false);
		}
		else if (spr.type)
		{
			Drawer->DrawSpritePolygon(spr.dv, spr.lump,
				spr.translucency - 1, spr.translation, spr.light);
		}
		else
		{
			Drawer->DrawMaskedPolygon(spr.dv, spr.count, spr.lump,
				spr.translucency - 1);
			if (spr.count > 4)
				Z_Free(spr.dv);
		}
		spr.dv = trans_sprite_verts + 4 * i;
		spr.dv[0] = mobj->origin;
		((TAVec *)spr.dv)[1] = mobj->angles;
		spr.surf = (surface_t*)mobj->AliasModel;
		spr.lump = mobj->AliasFrame;
		spr.count = mobj->AliasSkinIndex;
		spr.light = light;
		spr.translucency = mobj->translucency + 1;
		spr.skin = mobj->Skin;
		spr.dist = dist;
		spr.type = 2;
		return;
	}
	Drawer->DrawAliasModel(mobj->origin, mobj->angles, mobj->AliasModel,
		mobj->AliasFrame, mobj->AliasSkinIndex, mobj->Skin, light,
		mobj->translucency, false);
	unguard;
}

//==========================================================================
//
//	RenderAliasModel
//
//==========================================================================

static void RenderAliasModel(clmobj_t *mobj)
{
	guard(RenderAliasModel);
	if (!r_chasecam && (mobj == &cl_mobjs[cl->clientnum + 1] ||
		mobj == &cl_weapon_mobjs[cl->clientnum + 1]))
	{
		//	Don't draw client's mobj
		return;
	}

	if (mobj->translucency >= 95)
	{
		// Never make a vissprite when MF2_DONTDRAW is flagged.
		return;
	}

	//	Setup lighting
	dword light;
	if (mobj->frame & FF_FULLBRIGHT || fixedlight)
	{
		light = 0xffffffff;
	}
	else
	{
		light = R_LightPoint(mobj->origin);
	}

	//	Draw it
	if (mobj->translucency)
	{
		RenderTranslucentAliasModel(mobj, light);
	}
	else
	{
		Drawer->DrawAliasModel(mobj->origin, mobj->angles, mobj->AliasModel,
			mobj->AliasFrame, mobj->AliasSkinIndex, mobj->Skin, light, 0,
			false);
	}
	unguard;
}

//==========================================================================
//
//	R_RenderMobjs
//
//==========================================================================

void R_RenderMobjs(void)
{
	guard(R_RenderMobjs);
	int i;

	if (!r_draw_mobjs)
	{
		return;
	}

	for (i = 0; i < GMaxEntities; i++)
	{
		if (cl_mobjs[i].in_use)
		{
			if (cl_mobjs[i].AliasModel && r_models)
			{
				RenderAliasModel(&cl_mobjs[i]);
			}
			else
			{
	    	    RenderSprite(&cl_mobjs[i]);
			}
		}
	}

	if (r_models)
	{
		for (i = 0; i < MAXPLAYERS; i++)
		{
			if (cl_weapon_mobjs[i].in_use && cl_weapon_mobjs[i].AliasModel)
			{
				RenderAliasModel(&cl_weapon_mobjs[i]);
			}
		}
	}
	unguard;
}

//==========================================================================
//
//	R_DrawTranslucentPolys
//
//==========================================================================

void R_DrawTranslucentPolys(void)
{
	guard(R_DrawTranslucentPolys);
	int i, found;
	do
	{
		found = -1;
		float best_dist = -1;
		for (i = 0; i < MAX_TRANS_SPRITES; i++)
		{
			trans_sprite_t &spr = trans_sprites[i];
			if (!spr.translucency)
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
			r_normal = spr.normal;
			r_dist = spr.pdist;
			r_saxis = spr.saxis;
			r_taxis = spr.taxis;
			r_texorg = spr.texorg;
			r_surface = spr.surf;
			if (spr.type == 2)
			{
				Drawer->DrawAliasModel(spr.dv[0], ((TAVec *)spr.dv)[1],
					(VModel*)spr.surf, spr.lump, spr.count, spr.skin,
					spr.light, spr.translucency - 1, false);
			}
			else if (spr.type)
			{
				Drawer->DrawSpritePolygon(spr.dv, spr.lump,
					spr.translucency - 1, spr.translation, spr.light);
			}
			else
			{
				Drawer->DrawMaskedPolygon(spr.dv, spr.count, spr.lump,
					spr.translucency - 1);
				if (spr.count > 4)
					Z_Free(spr.dv);
			}
			spr.translucency = 0;
		}
	} while (found != -1);
	unguard;
}

//==========================================================================
//
//	RenderPSprite
//
//==========================================================================

static void RenderPSprite(cl_pspdef_t* psp, float PSP_DIST)
{
	guard(RenderPSprite);
	spritedef_t*		sprdef;
	spriteframe_t*		sprframe;
	int					lump;
	boolean				flip;

	// decide which patch to use
	if ((unsigned)psp->sprite >= MAX_SPRITE_MODELS)
	{
#ifdef PARANOID
		GCon->Logf("R_ProjectSprite: invalid sprite number %d", psp->sprite);
#endif
		return;
	}
	sprdef = &sprites[psp->sprite];
	if ((psp->frame & FF_FRAMEMASK)  >= sprdef->numframes)
	{
#ifdef PARANOID
		GCon->Logf("R_ProjectSprite: invalid sprite frame %d : %d",
			psp->sprite, psp->frame);
#endif
		return;
	}
	sprframe = &sprdef->spriteframes[psp->frame & FF_FRAMEMASK];

	lump = sprframe->lump[0];
	flip = (boolean)sprframe->flip[0];
	TTexture* Tex = GTextureManager.Textures[lump];

	int TexWidth = Tex->GetWidth();
	int TexHeight = Tex->GetHeight();
	int TexSOffset = Tex->SOffset;
	int TexTOffset = Tex->TOffset;

	TVec	dv[4];

	float PSP_DISTI = 1.0 / PSP_DIST;
	TVec sprorigin = vieworg + PSP_DIST * viewforward;

	float sprx = 160.0 - psp->sx + TexSOffset;
	float spry = 100.0 - psp->sy + TexTOffset;
	if (refdef.height == ScreenHeight)
	{
		spry -= cl->pspriteSY;
	}

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

	r_normal = -viewforward;
	r_dist = DotProduct(dv[0], r_normal);

	if (flip)
	{
		r_saxis = -(viewright * 160 * PSP_DISTI);
		r_texorg = dv[2];
	}
	else
	{
		r_saxis = viewright * 160 * PSP_DISTI;
		r_texorg = dv[1];
	}
	if (old_aspect)
		r_taxis = -(viewup * 160 * PSP_DISTI);
	else
		r_taxis = -(viewup * 100 * 4 / 3 * PSP_DISTI);

	dword light;
	if (psp->frame & FF_FULLBRIGHT)
	{
		light = 0xffffffff;
	}
	else
	{
		light = R_LightPoint(sprorigin);
	}

	Drawer->DrawSpritePolygon(dv, lump, cl->translucency, 0, light);
	unguard;
}

//==========================================================================
//
//	RenderViewModel
//
//==========================================================================

static void RenderViewModel(cl_pspdef_t *psp)
{
	guard(RenderViewModel);
	TVec origin = vieworg + (psp->sx - 1.0) * viewright / 8.0 -
		(psp->sy - 32.0) * viewup / 6.0;

	dword light;
	if (psp->frame & FF_FULLBRIGHT)
	{
		light = 0xffffffff;
	}
	else
	{
		light = R_LightPoint(origin);
	}

	Drawer->DrawAliasModel(origin, cl->viewangles, psp->alias_model,
		psp->alias_frame, 0, NULL, light, cl->translucency, true);
	unguard;
}

//==========================================================================
//
// 	R_DrawPlayerSprites
//
//==========================================================================

void R_DrawPlayerSprites(void)
{
	guard(R_DrawPlayerSprites);
    int         i;
    cl_pspdef_t	*psp;

	if (!r_draw_psprites || r_chasecam)
	{
		return;
	}

	if (cl->psprites[0].alias_model && r_view_models)
	{
		RenderViewModel(cl->psprites);
	}
	else
	{
	    // add all active psprites
		for (i = 0, psp = cl->psprites; i < NUMPSPRITES; i++, psp++)
		{
			if (psp->sprite != -1)
			{
				RenderPSprite(psp, 3 - i);
			}
		}
	}
	unguard;
}

//==========================================================================
//
//	R_DrawCroshair
//
//==========================================================================

void R_DrawCroshair(void)
{
	guard(R_DrawCroshair);
	if (croshair)
	{
		if (croshair_trans < 0)		croshair_trans = 0;
		if (croshair_trans > 100)	croshair_trans = 100;

		int			cy;
		if (screenblocks < 11)
			cy = (200 - sb_height) / 2;
		else
			cy = 100;
		int handle = GTextureManager.AddPatch(VName(va("CROSHAI%i",
			(int)croshair), VName::AddLower8), TEXTYPE_Pic);
		R_DrawPic(160, cy, handle, croshair_trans);
	}
	unguard;
}

//==========================================================================
//
//  R_DrawSpritePatch
//
//==========================================================================

void R_DrawSpritePatch(int x, int y, int sprite, int frame, int rot, int translation)
{
	guard(R_DrawSpritePatch);
	boolean			flip;
	int				lump;

	spriteframe_t *sprframe = &sprites[sprite].spriteframes[frame & FF_FRAMEMASK];
	flip = (boolean)sprframe->flip[rot];
	lump = sprframe->lump[rot];
	TTexture* Tex = GTextureManager.Textures[lump];

	Tex->GetWidth();

	float x1 = x - Tex->SOffset;
	float y1 = y - Tex->TOffset;
	float x2 = x1 + Tex->GetWidth();
	float y2 = y1 + Tex->GetHeight();

	x1 *= fScaleX;
	y1 *= fScaleY;
	x2 *= fScaleX;
	y2 *= fScaleY;

	Drawer->DrawSpriteLump(x1, y1, x2, y2, lump, translation, flip);
	unguard;
}

//==========================================================================
//
//	R_DrawModelFrame
//
//==========================================================================

void R_DrawModelFrame(const TVec &origin, float angle, VModel* model,
	int frame, const char *skin)
{
	guard(R_DrawModelFrame);
	viewangles.yaw = 180;
	viewangles.pitch = 0;
	viewangles.roll = 0;
	AngleVectors(viewangles, viewforward, viewright, viewup);
	vieworg = TVec(0, 0, 0);
	fixedlight = 0;

	refdef_t	rd;

    rd.x = 0;
	rd.y = 0;
	rd.width = ScreenWidth;
	rd.height = ScreenHeight;
	rd.fovx = tan(DEG2RAD(90) / 2);
	rd.fovy = rd.fovx * rd.height / rd.width / PixelAspect;
	rd.drawworld = false;

	Drawer->SetupView(&rd);

	TAVec angles;
	angles.yaw = angle;
	angles.pitch = 0;
	angles.roll = 0;
	Drawer->DrawAliasModel(origin, angles, model, frame, 0, skin,
		0xffffffff, 0, false);

	Drawer->EndView();
	unguard;
}

//**************************************************************************
//
//	$Log$
//	Revision 1.26  2006/02/27 20:45:26  dj_jl
//	Rewrote names class.
//
//	Revision 1.25  2006/02/20 22:52:56  dj_jl
//	Changed client state to a class.
//	
//	Revision 1.24  2006/02/05 14:11:00  dj_jl
//	Fixed conflict with Solaris.
//	
//	Revision 1.23  2005/11/02 22:29:26  dj_jl
//	Offset fix enabled by default.
//	
//	Revision 1.22  2005/05/26 16:50:15  dj_jl
//	Created texture manager class
//	
//	Revision 1.21  2005/05/03 14:57:06  dj_jl
//	Added support for specifying skin index.
//	
//	Revision 1.20  2004/12/27 12:23:16  dj_jl
//	Multiple small changes for version 1.16
//	
//	Revision 1.19  2004/11/22 07:33:17  dj_jl
//	Always check for valid sprite numbers and frames.
//	
//	Revision 1.18  2003/12/23 07:15:43  dj_jl
//	Sprite offset fix
//	
//	Revision 1.17  2003/10/23 06:36:47  dj_jl
//	PSprites drawn at different depths
//	
//	Revision 1.16  2002/08/28 16:39:19  dj_jl
//	Implemented sector light color.
//	
//	Revision 1.15  2002/07/13 07:51:49  dj_jl
//	Replacing console's iostream with output device.
//	
//	Revision 1.14  2002/03/28 17:58:02  dj_jl
//	Added support for scaled textures.
//	
//	Revision 1.13  2002/03/20 19:11:21  dj_jl
//	Added guarding.
//	
//	Revision 1.12  2002/01/07 12:16:43  dj_jl
//	Changed copyright year
//	
//	Revision 1.11  2001/10/27 07:46:14  dj_jl
//	Fixed sprite rotations
//	
//	Revision 1.10  2001/10/22 17:25:55  dj_jl
//	Floatification of angles
//	
//	Revision 1.9  2001/10/18 17:36:31  dj_jl
//	A lots of changes for Alpha 2
//	
//	Revision 1.8  2001/08/31 17:28:47  dj_jl
//	Tried to fix weapon model position when dead
//	
//	Revision 1.7  2001/08/15 17:21:47  dj_jl
//	Added model drawing for menu
//	
//	Revision 1.6  2001/08/07 16:46:23  dj_jl
//	Added player models, skins and weapon
//	
//	Revision 1.5  2001/08/04 17:29:11  dj_jl
//	Added depth hack for weapon models
//	
//	Revision 1.4  2001/08/01 17:33:58  dj_jl
//	Fixed drawing of spite lump for player setup menu, beautification
//	
//	Revision 1.3  2001/07/31 17:11:56  dj_jl
//	Some fixes in model rendering
//	
//	Revision 1.2  2001/07/27 14:27:54  dj_jl
//	Update with Id-s and Log-s, some fixes
//
//**************************************************************************
