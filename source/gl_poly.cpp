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

#include "gl_local.h"

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
//	glVertex
//
//==========================================================================

inline void	glVertex(const TVec &v)
{
	glVertex3f(v.x, v.y, v.z);
}

//==========================================================================
//
//	VOpenGLDrawer::DrawPolygon
//
//==========================================================================

void VOpenGLDrawer::DrawPolygon(surface_t* surf, int)
{
	guard(VOpenGLDrawer::DrawPolygon);
	bool lightmaped = surf->lightmap != NULL ||
		surf->dlightframe == r_dlightframecount;

	if (lightmaped)
	{
		CacheSurface(surf);
		if (mtexable)
		{
			return;
		}
	}

	if (SimpleSurfsTail)
	{
		SimpleSurfsTail->DrawNext = surf;
		SimpleSurfsTail = surf;
	}
	else
	{
		SimpleSurfsHead = surf;
		SimpleSurfsTail = surf;
	}
	surf->DrawNext = NULL;
	unguard;
}

//==========================================================================
//
//	VOpenGLDrawer::DrawSkyPortal
//
//==========================================================================

void VOpenGLDrawer::DrawSkyPortal(surface_t* surf, int)
{
	guard(VOpenGLDrawer::DrawSkyPortal);
	if (SkyPortalsTail)
	{
		SkyPortalsTail->DrawNext = surf;
		SkyPortalsTail = surf;
	}
	else
	{
		SkyPortalsHead = surf;
		SkyPortalsTail = surf;
	}
	surf->DrawNext = NULL;
	unguard;
}

//==========================================================================
//
//	VOpenGLDrawer::DrawHorizonPolygon
//
//==========================================================================

void VOpenGLDrawer::DrawHorizonPolygon(surface_t* surf, int)
{
	guard(VOpenGLDrawer::DrawHorizonPolygon);
	if (HorizonPortalsTail)
	{
		HorizonPortalsTail->DrawNext = surf;
		HorizonPortalsTail = surf;
	}
	else
	{
		HorizonPortalsHead = surf;
		HorizonPortalsTail = surf;
	}
	surf->DrawNext = NULL;
	unguard;
}

//==========================================================================
//
//	VOpenGLDrawer::WorldDrawing
//
//==========================================================================

void VOpenGLDrawer::WorldDrawing()
{
	guard(VOpenGLDrawer::WorldDrawing);
	int			lb, i;
	surfcache_t	*cache;
	GLfloat		s, t;
	GLfloat		lights, lightt;
	surface_t	*surf;
	texinfo_t	*tex;

	//	First draw horizons.
	for (surf = HorizonPortalsHead; surf; surf = surf->DrawNext)
	{
		DoHorizonPolygon(surf);
	}

	//	For sky areas we just write to the depth buffer to prevent drawing
	// polygons behind the sky.
	if (SkyPortalsHead)
	{
		glDisable(GL_TEXTURE_2D);
		glColorMask(GL_FALSE, GL_FALSE, GL_FALSE, GL_FALSE);
		for (surf = SkyPortalsHead; surf; surf = surf->DrawNext)
		{
			glBegin(GL_POLYGON);
			for (i = 0; i < surf->count; i++)
			{
				glVertex(surf->verts[i]);
			}
			glEnd();
		}
		glColorMask(GL_TRUE, GL_TRUE, GL_TRUE, GL_TRUE);
		glEnable(GL_TEXTURE_2D);
	}

	//	Draw surfaces.
	for (surf = SimpleSurfsHead; surf; surf = surf->DrawNext)
	{
		texinfo_t* tex = surf->texinfo;
		SetTexture(tex->Tex, tex->ColourMap);

		if (surf->lightmap != NULL ||
			surf->dlightframe == r_dlightframecount)
		{
			glColor4f(1, 1, 1, 1);
		}
		else
		{
			float lev = float(surf->Light >> 24) / 255.0;
			glColor4f(((surf->Light >> 16) & 255) * lev / 255.0,
				((surf->Light >> 8) & 255) * lev / 255.0,
				(surf->Light & 255) * lev / 255.0, 1.0);
		}
		SetFade(surf->Fade);

		glBegin(GL_POLYGON);
		for (i = 0; i < surf->count; i++)
		{
			glTexCoord2f((DotProduct(surf->verts[i], tex->saxis) + tex->soffs) * tex_iw,
				(DotProduct(surf->verts[i], tex->taxis) + tex->toffs) * tex_ih);
			glVertex(surf->verts[i]);
		}
		glEnd();
	}

	if (mtexable)
	{
		glTexEnvf(GL_TEXTURE_ENV, GL_TEXTURE_ENV_MODE, GL_REPLACE);
		SelectTexture(1);
		glEnable(GL_TEXTURE_2D);
		glTexEnvf(GL_TEXTURE_ENV, GL_TEXTURE_ENV_MODE, GL_BLEND);
		SelectTexture(0);
		glColor4f(1, 1, 1, 1);

		for (lb = 0; lb < NUM_BLOCK_SURFS; lb++)
		{
			if (!light_chain[lb])
			{
				continue;
			}

			SelectTexture(1);
			glBindTexture(GL_TEXTURE_2D, lmap_id[lb]);
			glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
			glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

			if (block_changed[lb])
			{
				block_changed[lb] = false;
				glTexImage2D(GL_TEXTURE_2D, 0, 4, BLOCK_WIDTH, BLOCK_HEIGHT,
					0, GL_RGBA, GL_UNSIGNED_BYTE, light_block[lb]);
			}
			SelectTexture(0);

			for (cache = light_chain[lb]; cache; cache = cache->chain)
			{
				surf = cache->surf;
				tex = surf->texinfo;

				SetTexture(tex->Tex, tex->ColourMap);
				SetFade(surf->Fade);
				glBegin(GL_POLYGON);
				for (i = 0; i < surf->count; i++)
				{
					s = DotProduct(surf->verts[i], tex->saxis) + tex->soffs;
					t = DotProduct(surf->verts[i], tex->taxis) + tex->toffs;
					lights = (s - surf->texturemins[0] +
						cache->s * 16 + 8) / (BLOCK_WIDTH * 16);
					lightt = (t - surf->texturemins[1] +
						cache->t * 16 + 8) / (BLOCK_HEIGHT * 16);
					MultiTexCoord(0, s * tex_iw, t * tex_ih);
					MultiTexCoord(1, lights, lightt);
					glVertex(surf->verts[i]);
				}
				glEnd();
			}
		}

		SelectTexture(1);
		glDisable(GL_TEXTURE_2D);
		SelectTexture(0);
		glTexEnvf(GL_TEXTURE_ENV, GL_TEXTURE_ENV_MODE, GL_MODULATE);
	}
	else
	{
		glDepthMask(0);		// don't bother writing Z
		glBlendFunc(GL_ZERO, GL_ONE_MINUS_SRC_COLOR);
		glEnable(GL_BLEND);
		glColor4f(1, 1, 1, 1);

		for (lb = 0; lb < NUM_BLOCK_SURFS; lb++)
		{
			if (!light_chain[lb])
			{
				continue;
			}

			glBindTexture(GL_TEXTURE_2D, lmap_id[lb]);
			glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
			glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

			if (block_changed[lb])
			{
				block_changed[lb] = false;
				glTexImage2D(GL_TEXTURE_2D, 0, 4, BLOCK_WIDTH, BLOCK_HEIGHT,
					0, GL_RGBA, GL_UNSIGNED_BYTE, light_block[lb]);
			}

			for (cache = light_chain[lb]; cache; cache = cache->chain)
			{
				surf = cache->surf;
				tex = surf->texinfo;

				SetFade(surf->Fade);
				glBegin(GL_POLYGON);
				for (i = 0; i < surf->count; i++)
				{
					s = (DotProduct(surf->verts[i], tex->saxis) + tex->soffs -
						surf->texturemins[0] + cache->s * 16 + 8) / (BLOCK_WIDTH * 16);
					t = (DotProduct(surf->verts[i], tex->taxis) + tex->toffs -
						surf->texturemins[1] + cache->t * 16 + 8) / (BLOCK_HEIGHT * 16);
					glTexCoord2f(s, t);
					glVertex(surf->verts[i]);
				}
				glEnd();
			}
		}

		glDisable(GL_BLEND);
		glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
		glDepthMask(1);		// back to normal Z buffering
	}

	//
	//	Add specular lights
	//
	if (specular_highlights)
	{
		glDepthMask(0);		// don't bother writing Z
		glBlendFunc(GL_ONE, GL_ONE);
		glEnable(GL_BLEND);
		glColor4f(1, 1, 1, 1);
		SetFade(0);

		for (lb = 0; lb < NUM_BLOCK_SURFS; lb++)
		{
			if (!add_chain[lb])
			{
				continue;
			}

			glBindTexture(GL_TEXTURE_2D, addmap_id[lb]);
			glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
			glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

			if (add_changed[lb])
			{
				add_changed[lb] = false;
				glTexImage2D(GL_TEXTURE_2D, 0, 4, BLOCK_WIDTH, BLOCK_HEIGHT,
					0, GL_RGBA, GL_UNSIGNED_BYTE, add_block[lb]);
			}

			for (cache = add_chain[lb]; cache; cache = cache->addchain)
			{
				surf = cache->surf;
				tex = surf->texinfo;

				glBegin(GL_POLYGON);
				for (i = 0; i < surf->count; i++)
				{
					s = (DotProduct(surf->verts[i], tex->saxis) + tex->soffs -
						surf->texturemins[0] + cache->s * 16 + 8) / (BLOCK_WIDTH * 16);
					t = (DotProduct(surf->verts[i], tex->taxis) + tex->toffs -
						surf->texturemins[1] + cache->t * 16 + 8) / (BLOCK_HEIGHT * 16);
					glTexCoord2f(s, t);
					glVertex(surf->verts[i]);
				}
				glEnd();
			}
		}

		glDisable(GL_BLEND);
		glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
		glDepthMask(1);		// back to normal Z buffering
	}
	unguard;
}

//==========================================================================
//
//	VOpenGLDrawer::DoHorizonPolygon
//
//==========================================================================

void VOpenGLDrawer::DoHorizonPolygon(surface_t* Surf)
{
	guard(VOpenGLDrawer::DoHorizonPolygon);
	float Dist = 4096.0;
	TVec v[4];
	if (Surf->HorizonPlane->normal.z > 0.0)
	{
		v[0] = Surf->verts[0];
		v[3] = Surf->verts[3];
		TVec HDir = -Surf->plane->normal;

		TVec Dir1 = Normalise(vieworg - Surf->verts[1]);
		TVec Dir2 = Normalise(vieworg - Surf->verts[2]);
		float Mul1 = 1.0 / DotProduct(HDir, Dir1);
		v[1] = Surf->verts[1] + Dir1 * Mul1 * Dist;
		float Mul2 = 1.0 / DotProduct(HDir, Dir2);
		v[2] = Surf->verts[2] + Dir2 * Mul2 * Dist;
		if (v[1].z < v[0].z)
		{
			v[1] = Surf->verts[1] + Dir1 * Mul1 * Dist * (Surf->verts[1].z -
				Surf->verts[0].z) / (Surf->verts[1].z - v[1].z);
			v[2] = Surf->verts[2] + Dir2 * Mul2 * Dist * (Surf->verts[2].z -
				Surf->verts[3].z) / (Surf->verts[2].z - v[2].z);
		}
	}
	else
	{
		v[1] = Surf->verts[1];
		v[2] = Surf->verts[2];
		TVec HDir = -Surf->plane->normal;

		TVec Dir1 = Normalise(vieworg - Surf->verts[0]);
		TVec Dir2 = Normalise(vieworg - Surf->verts[3]);
		float Mul1 = 1.0 / DotProduct(HDir, Dir1);
		v[0] = Surf->verts[0] + Dir1 * Mul1 * Dist;
		float Mul2 = 1.0 / DotProduct(HDir, Dir2);
		v[3] = Surf->verts[3] + Dir2 * Mul2 * Dist;
		if (v[1].z < v[0].z)
		{
			v[0] = Surf->verts[0] + Dir1 * Mul1 * Dist * (Surf->verts[1].z -
				Surf->verts[0].z) / (v[0].z - Surf->verts[0].z);
			v[3] = Surf->verts[3] + Dir2 * Mul2 * Dist * (Surf->verts[2].z -
				Surf->verts[3].z) / (v[3].z - Surf->verts[3].z);
		}
	}

	texinfo_t* Tex = Surf->texinfo;
	SetTexture(Tex->Tex, Tex->ColourMap);

	float lev = float(Surf->Light >> 24) / 255.0;
	glColor4f(((Surf->Light >> 16) & 255) * lev / 255.0,
		((Surf->Light >> 8) & 255) * lev / 255.0,
		(Surf->Light & 255) * lev / 255.0, 1.0);
	SetFade(Surf->Fade);

	//	Draw it
	glDepthMask(GL_FALSE);
	glBegin(GL_POLYGON);
	for (int i = 0; i < 4; i++)
	{
		glTexCoord2f((DotProduct(v[i], Tex->saxis) + Tex->soffs) * tex_iw,
			(DotProduct(v[i], Tex->taxis) + Tex->toffs) * tex_ih);
		glVertex(v[i]);
	}
	glEnd();
	glDepthMask(GL_TRUE);

	//	Write to the depth buffer.
	glDisable(GL_TEXTURE_2D);
	glColorMask(GL_FALSE, GL_FALSE, GL_FALSE, GL_FALSE);
	glBegin(GL_POLYGON);
	for (int i = 0; i < Surf->count; i++)
	{
		glVertex(Surf->verts[i]);
	}
	glEnd();
	glColorMask(GL_TRUE, GL_TRUE, GL_TRUE, GL_TRUE);
	glEnable(GL_TEXTURE_2D);
	unguard;
}

//==========================================================================
//
//	VOpenGLDrawer::BeginSky
//
//==========================================================================

void VOpenGLDrawer::BeginSky()
{
	guard(VOpenGLDrawer::BeginSky);
	glDepthMask(0);

	//	Sky polys are alredy translated
	glPushMatrix();
	glTranslatef(vieworg.x, vieworg.y, vieworg.z);
	unguard;
}

//==========================================================================
//
//	VOpenGLDrawer::DrawSkyPolygon
//
//==========================================================================

void VOpenGLDrawer::DrawSkyPolygon(surface_t* surf, bool bIsSkyBox,
	VTexture* Texture1, float offs1, VTexture* Texture2, float offs2,
	int CMap)
{
	guard(VOpenGLDrawer::DrawSkyPolygon);
	int		i;
	int		sidx[4];

	SetFade(surf->Fade);
	sidx[0] = 0;
	sidx[1] = 1;
	sidx[2] = 2;
	sidx[3] = 3;
	if (!bIsSkyBox)
	{
		if (surf->verts[1].z > 0)
		{
			sidx[1] = 0;
			sidx[2] = 3;
		}
		else
		{
			sidx[0] = 1;
			sidx[3] = 2;
		}
	}
	texinfo_t *tex = surf->texinfo;
	if (mtexable && Texture2->Type != TEXTYPE_Null)
	{
		SetTexture(Texture1, CMap);
		SelectTexture(1);
		glEnable(GL_TEXTURE_2D);
		glTexEnvf(GL_TEXTURE_ENV, GL_TEXTURE_ENV_MODE, GL_DECAL);
		SetTexture(Texture2, CMap);
		SelectTexture(0);

		glColor4f(1, 1, 1, 1);
		glBegin(GL_POLYGON);
		for (i = 0; i < surf->count; i++)
		{
			MultiTexCoord(0, 
				(DotProduct(surf->verts[sidx[i]], tex->saxis) + tex->soffs - offs1) * tex_iw,
				(DotProduct(surf->verts[i], tex->taxis) + tex->toffs) * tex_ih);
			MultiTexCoord(1, 
				(DotProduct(surf->verts[sidx[i]], tex->saxis) + tex->soffs - offs2) * tex_iw,
				(DotProduct(surf->verts[i], tex->taxis) + tex->toffs) * tex_ih);
			glVertex(surf->verts[i]);
		}
		glEnd();

		SelectTexture(1);
		glDisable(GL_TEXTURE_2D);
		SelectTexture(0);
	}
	else
	{
		SetTexture(Texture1, CMap);
		glBegin(GL_POLYGON);
		glColor4f(1, 1, 1, 1);
		for (i = 0; i < surf->count; i++)
		{
			glTexCoord2f(
				(DotProduct(surf->verts[sidx[i]], tex->saxis) + tex->soffs - offs1) * tex_iw,
				(DotProduct(surf->verts[i], tex->taxis) + tex->toffs) * tex_ih);
			glVertex(surf->verts[i]);
		}
		glEnd();

		if (Texture2->Type != TEXTYPE_Null)
		{
			SetTexture(Texture2, CMap);
			glEnable(GL_BLEND);
			glBegin(GL_POLYGON);
			glColor4f(1, 1, 1, 1);
			for (i = 0; i < surf->count; i++)
			{
				glTexCoord2f(
					(DotProduct(surf->verts[sidx[i]], tex->saxis) + tex->soffs - offs2) * tex_iw,
					(DotProduct(surf->verts[i], tex->taxis) + tex->toffs) * tex_ih);
				glVertex(surf->verts[i]);
			}
			glEnd();
			glDisable(GL_BLEND);
		}
	}
	unguard;
}

//==========================================================================
//
//	VOpenGLDrawer::EndSky
//
//==========================================================================

void VOpenGLDrawer::EndSky()
{
	guard(VOpenGLDrawer::EndSky);
	glPopMatrix();
	glDepthMask(1);
	unguard;
}

//==========================================================================
//
//	VOpenGLDrawer::DrawMaskedPolygon
//
//==========================================================================

void VOpenGLDrawer::DrawMaskedPolygon(surface_t* surf, float Alpha,
	bool Additive)
{
	guard(VOpenGLDrawer::DrawMaskedPolygon);
	texinfo_t* tex = surf->texinfo;
	SetTexture(tex->Tex, tex->ColourMap);
	glEnable(GL_ALPHA_TEST);
	if (blend_sprites || Additive || Alpha < 1.0)
	{
		glAlphaFunc(GL_GREATER, 0.0);
		glEnable(GL_BLEND);
	}
	if (Additive)
	{
		glBlendFunc(GL_SRC_ALPHA, GL_ONE);
	}

	if (surf->lightmap != NULL ||
		surf->dlightframe == r_dlightframecount)
	{
		RendLev->BuildLightMap(surf, 0);
		int w = (surf->extents[0] >> 4) + 1;
		int h = (surf->extents[1] >> 4) + 1;
		int size = w * h;
		int r = 0;
		int g = 0;
		int b = 0;
		for (int i = 0; i < size; i++)
		{
			r += 255 * 256 - blocklightsr[i];
			g += 255 * 256 - blocklightsg[i];
			b += 255 * 256 - blocklightsb[i];
		}
		double iscale = 1.0 / (size * 255 * 256);
		glColor4f(r * iscale, g * iscale, b * iscale, Alpha);
	}
	else
	{
		float lev = float(surf->Light >> 24) / 255.0;
		glColor4f(((surf->Light >> 16) & 255) * lev / 255.0,
			((surf->Light >> 8) & 255) * lev / 255.0,
			(surf->Light & 255) * lev / 255.0, Alpha);
	}
	SetFade(surf->Fade);

	glBegin(GL_POLYGON);
	for (int i = 0; i < surf->count; i++)
	{
		glTexCoord2f((DotProduct(surf->verts[i], tex->saxis) + tex->soffs) * tex_iw,
			(DotProduct(surf->verts[i], tex->taxis) + tex->toffs) * tex_ih);
		glVertex(surf->verts[i]);
	}
	glEnd();

	if (blend_sprites || Additive || Alpha < 1.0)
	{
		glAlphaFunc(GL_GREATER, 0.666);
		glDisable(GL_BLEND);
	}
	if (Additive)
	{
		glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
	}
	glDisable(GL_ALPHA_TEST);
	unguard;
}

//==========================================================================
//
//	VOpenGLDrawer::DrawSpritePolygon
//
//==========================================================================

void VOpenGLDrawer::DrawSpritePolygon(TVec *cv, VTexture* Tex, float Alpha,
	bool Additive, VTextureTranslation* Translation, int CMap, vuint32 light,
	vuint32 Fade, const TVec&, float, const TVec& saxis, const TVec& taxis,
	const TVec& texorg)
{
	guard(VOpenGLDrawer::DrawSpritePolygon);
	TVec	texpt;

	SetSpriteLump(Tex, Translation, CMap);

	if (blend_sprites || Additive || Alpha < 1.0)
	{
		glAlphaFunc(GL_GREATER, 0.0);
		glEnable(GL_BLEND);
	}
	if (Additive)
	{
		glBlendFunc(GL_SRC_ALPHA, GL_ONE);
	}
	glEnable(GL_ALPHA_TEST);

	vuint32 alpha = (int)(255 * Alpha);
	SetColour((light & 0x00ffffff) | (alpha << 24));
	SetFade(Fade);

	glBegin(GL_QUADS);

	texpt = cv[0] - texorg;
	glTexCoord2f(DotProduct(texpt, saxis) * tex_iw,
		DotProduct(texpt, taxis) * tex_ih);
	glVertex(cv[0]);

	texpt = cv[1] - texorg;
	glTexCoord2f(DotProduct(texpt, saxis) * tex_iw,
		DotProduct(texpt, taxis) * tex_ih);
	glVertex(cv[1]);

	texpt = cv[2] - texorg;
	glTexCoord2f(DotProduct(texpt, saxis) * tex_iw,
		DotProduct(texpt, taxis) * tex_ih);
	glVertex(cv[2]);

	texpt = cv[3] - texorg;
	glTexCoord2f(DotProduct(texpt, saxis) * tex_iw,
		DotProduct(texpt, taxis) * tex_ih);
	glVertex(cv[3]);

	glEnd();

	if (blend_sprites || Additive || Alpha < 1.0)
	{
		glAlphaFunc(GL_GREATER, 0.666);
		glDisable(GL_BLEND);
	}
	if (Additive)
	{
		glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
	}
	glDisable(GL_ALPHA_TEST);
	unguard;
}

//==========================================================================
//
//	VOpenGLDrawer::DrawAliasModel
//
//==========================================================================

void VOpenGLDrawer::DrawAliasModel(const TVec &origin, const TAVec &angles,
	const TVec& Offset, const TVec& Scale, mmdl_t* pmdl, int frame,
	VTexture* Skin, VTextureTranslation* Trans, int CMap, vuint32 light,
	vuint32 Fade, float Alpha, bool Additive, bool is_view_model)
{
	guard(VOpenGLDrawer::DrawAliasModel);
	mframe_t	*framedesc;
	float 		l;
	int			index;
	trivertx_t	*verts;
	int			*order;
	int			count;
	float		shadelightr;
	float		shadelightg;
	float		shadelightb;
	float		*shadedots;

	if (is_view_model)
	{
		// hack the depth range to prevent view model from poking into walls
		glDepthRange(0.0, 0.3);
	}

	//
	// get lighting information
	//
	shadelightr = ((light >> 16) & 0xff) / 510.0;
	shadelightg = ((light >> 8) & 0xff) / 510.0;
	shadelightb = (light & 0xff) / 510.0;
	shadedots = r_avertexnormal_dots[((int)(angles.yaw * (SHADEDOT_QUANT / 360.0))) & (SHADEDOT_QUANT - 1)];
	if (!model_lighting)
	{
		SetColour((light & 0x00ffffff) | (int(255 * Alpha) << 24));
	}
	SetFade(Fade);

	//
	// draw all the triangles
	//

	glPushMatrix();
	glTranslatef(origin.x, origin.y, origin.z);

	glRotatef(angles.yaw,  0, 0, 1);
	glRotatef(angles.pitch,  0, 1, 0);
	glRotatef(angles.roll,  1, 0, 0);

	glScalef(Scale.x, Scale.y, Scale.z);
	glTranslatef(Offset.x, Offset.y, Offset.z);

	framedesc = (mframe_t*)((byte *)pmdl + pmdl->ofsframes + frame * pmdl->framesize);

	glTranslatef(framedesc->scale_origin[0], framedesc->scale_origin[1], framedesc->scale_origin[2]);
	glScalef(framedesc->scale[0], framedesc->scale[1], framedesc->scale[2]);

	SetPic(Skin, Trans, CMap);

	glShadeModel(GL_SMOOTH);
	glEnable(GL_BLEND);
	if (Additive)
	{
		glBlendFunc(GL_SRC_ALPHA, GL_ONE);
	}
	glAlphaFunc(GL_GREATER, 0.0);
	glEnable(GL_ALPHA_TEST);

	verts = (trivertx_t *)(framedesc + 1);
	order = (int *)((byte *)pmdl + pmdl->ofscmds);

	while (*order)
	{
		// get the vertex count and primitive type
		count = *order++;
		if (count < 0)
		{
			count = -count;
			glBegin(GL_TRIANGLE_FAN);
		}
		else
		{
			glBegin(GL_TRIANGLE_STRIP);
		}

		do
		{
			// texture coordinates come from the draw list
			glTexCoord2f(((float *)order)[0], ((float *)order)[1]);
			order += 2;

			// normals and vertexes come from the frame list
			index = *order++;
			if (model_lighting)
			{
				l = shadedots[verts[index].lightnormalindex];
				glColor4f(l * shadelightr, l * shadelightg, l * shadelightb, Alpha);
			}
			glVertex3f(verts[index].v[0], verts[index].v[1], verts[index].v[2]);
		} while (--count);

		glEnd();
	}

	glShadeModel(GL_FLAT);
	glDisable(GL_BLEND);
	if (Additive)
	{
		glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
	}
	glAlphaFunc(GL_GREATER, 0.666);
	glDisable(GL_ALPHA_TEST);

	glPopMatrix();
	if (is_view_model)
	{
		glDepthRange(0.0, 1.0);
	}
	unguard;
}

//==========================================================================
//
//	VOpenGLDrawer::StartParticles
//
//==========================================================================

void VOpenGLDrawer::StartParticles()
{
	guard(VOpenGLDrawer::StartParticles);
	glEnable(GL_BLEND);
	glEnable(GL_ALPHA_TEST);
	glAlphaFunc(GL_GREATER, 0.0);
	if (pointparmsable)
	{
		GLfloat parms[3] = { 0.0, 1.0, 0.0 };
		p_PointParameterfv(GLenum(GL_DISTANCE_ATTENUATION_EXT), parms);
		p_PointParameterf(GLenum(GL_POINT_FADE_THRESHOLD_SIZE_EXT), 1.0);
		glDisable(GL_TEXTURE_2D);
		glEnable(GL_POINT_SMOOTH);
		glBegin(GL_POINTS);
	}
	else
	{
		glBindTexture(GL_TEXTURE_2D, particle_texture);
		glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, maxfilter);
		glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, minfilter);
		glBegin(GL_QUADS);
	}
	unguard;
}

//==========================================================================
//
//	VOpenGLDrawer::DrawParticle
//
//==========================================================================

void VOpenGLDrawer::DrawParticle(particle_t *p)
{
	guard(VOpenGLDrawer::DrawParticle);
	SetColour(p->colour);
	if (pointparmsable)
	{
		glVertex(p->org);
	}
	else
	{
		glTexCoord2f(0, 0); glVertex(p->org - viewright * p->Size + viewup * p->Size);
		glTexCoord2f(1, 0); glVertex(p->org + viewright * p->Size + viewup * p->Size);
		glTexCoord2f(1, 1); glVertex(p->org + viewright * p->Size - viewup * p->Size);
		glTexCoord2f(0, 1); glVertex(p->org - viewright * p->Size - viewup * p->Size);
	}
	unguard;
}

//==========================================================================
//
//	VOpenGLDrawer::EndParticles
//
//==========================================================================

void VOpenGLDrawer::EndParticles()
{
	guard(VOpenGLDrawer::EndParticles);
	glEnd();
	glDisable(GL_BLEND);
	glDisable(GL_ALPHA_TEST);
	glAlphaFunc(GL_GREATER, 0.666);
	if (pointparmsable)
	{
		glDisable(GL_POINT_SMOOTH);
		glEnable(GL_TEXTURE_2D);
	}
	unguard;
}

//==========================================================================
//
//	VOpenGLDrawer::StartPortal
//
//==========================================================================

bool VOpenGLDrawer::StartPortal(VPortal* Portal)
{
	guard(VOpenGLDrawer::StartPortal);
	//	Disable drawing
	glDisable(GL_TEXTURE_2D);
	glColorMask(GL_FALSE, GL_FALSE, GL_FALSE, GL_FALSE);
	glDepthMask(GL_FALSE);

	//	Set up stencil test.
	if (!PortalDepth)
	{
		glEnable(GL_STENCIL_TEST);
	}
	glStencilFunc(GL_EQUAL, PortalDepth, ~0);
	glStencilOp(GL_KEEP, GL_KEEP, GL_INCR);

	//	Mark the portal area.
	DrawPortalArea(Portal);

	//	Set up stencil test for portal
	glStencilFunc(GL_EQUAL, PortalDepth + 1, ~0);
	glStencilOp(GL_KEEP, GL_KEEP, GL_KEEP);

	if (Portal->NeedsDepthBuffer())
	{
		glDepthMask(GL_TRUE);
		//	Clear depth buffer
		glDepthRange(1, 1);
		glDepthFunc(GL_ALWAYS);
		DrawPortalArea(Portal);
		glDepthFunc(GL_LEQUAL);
		glDepthRange(0, 1);
	}
	else
	{
		glDepthMask(GL_FALSE);
		glDisable(GL_DEPTH_TEST);
	}

	//	Enable drawing.
	glColorMask(GL_TRUE, GL_TRUE, GL_TRUE, GL_TRUE);
	glEnable(GL_TEXTURE_2D);

	PortalDepth++;
	return true;
	unguard;
}

//==========================================================================
//
//	VOpenGLDrawer::DrawPortalArea
//
//==========================================================================

void VOpenGLDrawer::DrawPortalArea(VPortal* Portal)
{
	guard(VOpenGLDrawer::DrawPortalArea);
	for (int i = 0; i < Portal->Surfs.Num(); i++)
	{
		const surface_t* Surf = Portal->Surfs[i];
		glBegin(GL_POLYGON);
		for (int i = 0; i < Surf->count; i++)
		{
			glVertex(Surf->verts[i]);
		}
		glEnd();
	}
	unguard;
}

//==========================================================================
//
//	VSoftwareDrawer::EndPortal
//
//==========================================================================

void VOpenGLDrawer::EndPortal(VPortal* Portal)
{
    guard(VOpenGLDrawer::EndPortal);
	if (Portal->NeedsDepthBuffer())
	{
		//	Clear depth buffer
		glDepthRange(1, 1);
		glDisable(GL_TEXTURE_2D);
		glColorMask(GL_FALSE, GL_FALSE, GL_FALSE, GL_FALSE);
		glDepthFunc(GL_ALWAYS);
		DrawPortalArea(Portal);
		glDepthFunc(GL_LEQUAL);
		glColorMask(GL_TRUE, GL_TRUE, GL_TRUE, GL_TRUE);
		glEnable(GL_TEXTURE_2D);
		glDepthRange(0, 1);
	}
	else
	{
		glDepthMask(GL_TRUE);
		glEnable(GL_DEPTH_TEST);
	}

	glStencilOp(GL_KEEP, GL_KEEP, GL_DECR);

	//	Draw proper z-buffer for the portal area.
	glDepthFunc(GL_ALWAYS);
	glDisable(GL_TEXTURE_2D);
	glColorMask(GL_FALSE, GL_FALSE, GL_FALSE, GL_FALSE);
	DrawPortalArea(Portal);
	glColorMask(GL_TRUE, GL_TRUE, GL_TRUE, GL_TRUE);
	glEnable(GL_TEXTURE_2D);
	glDepthFunc(GL_LEQUAL);

	glStencilOp(GL_KEEP, GL_KEEP, GL_KEEP);

	PortalDepth--;
	glStencilFunc(GL_EQUAL, PortalDepth, ~0);
	if (!PortalDepth)
	{
		glDisable(GL_STENCIL_TEST);
	}
    unguard;
}
