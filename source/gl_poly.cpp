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

// precalculated dot products for quantized angles
#define SHADEDOT_QUANT 16
static float	r_avertexnormal_dots[SHADEDOT_QUANT][256] =
#include "anorm_dots.h"
;

// CODE --------------------------------------------------------------------

//==========================================================================
//
//	VOpenGLDrawer::FlushCaches
//
//==========================================================================

void VOpenGLDrawer::FlushCaches(bool free_blocks)
{
	guard(VOpenGLDrawer::FlushCaches);
	int				i;
	surfcache_t		*blines;
	surfcache_t		*block;

	if (free_blocks)
	{
		for (i = 0; i < NUM_BLOCK_SURFS; i++)
		{
			for (blines = cacheblocks[i]; blines; blines = blines->bnext)
			{
				for (block = blines; block; block = block->lnext)
				{
					if (block->owner)
						*block->owner = NULL;
				}
			}
		}
	}

	memset(blockbuf, 0, sizeof(blockbuf));
	freeblocks = NULL;
	for (i = 0; i < NUM_CACHE_BLOCKS; i++)
	{
		blockbuf[i].chain = freeblocks;
		freeblocks = &blockbuf[i];
	}
	for (i = 0; i < NUM_BLOCK_SURFS; i++)
	{
		cacheblocks[i] = freeblocks;
		freeblocks = freeblocks->chain;
		cacheblocks[i]->width = BLOCK_WIDTH;
		cacheblocks[i]->height = BLOCK_HEIGHT;
		cacheblocks[i]->blocknum = i;
	}
	unguard;
}

//==========================================================================
//
//	VOpenGLDrawer::FlushOldCaches
//
//==========================================================================

void VOpenGLDrawer::FlushOldCaches()
{
	guard(VOpenGLDrawer::FlushOldCaches);
	int				i;
	surfcache_t		*blines;
	surfcache_t		*block;

	for (i = 0; i < NUM_BLOCK_SURFS; i++)
	{
		for (blines = cacheblocks[i]; blines; blines = blines->bnext)
		{
			for (block = blines; block; block = block->lnext)
			{
				if (block->owner && cacheframecount != block->lastframe)
				{
					block = FreeBlock(block, false);
				}
			}
			if (!blines->owner && !blines->lprev && !blines->lnext)
			{
				blines = FreeBlock(blines, true);
			}
		}
	}
	if (!freeblocks)
	{
		Sys_Error("No more free blocks");
	}
	unguard;
}

//==========================================================================
//
//	VOpenGLDrawer::AllocBlock
//
//==========================================================================

surfcache_t	*VOpenGLDrawer::AllocBlock(int width, int height)
{
	guard(VOpenGLDrawer::AllocBlock);
	int				i;
	surfcache_t		*blines;
	surfcache_t		*block;
	surfcache_t		*other;

	for (i = 0; i < NUM_BLOCK_SURFS; i++)
	{
		for (blines = cacheblocks[i]; blines; blines = blines->bnext)
		{
			if (blines->height != height)
			{
				continue;
			}
			for (block = blines; block; block = block->lnext)
			{
				if (block->owner)
					continue;
				if (block->width < width)
					continue;
				if (block->width > width)
				{
					if (!freeblocks)
						FlushOldCaches();
					other = freeblocks;
					freeblocks = other->chain;
					other->s = block->s + width;
					other->t = block->t;
					other->width = block->width - width;
					other->height = block->height;
					other->lnext = block->lnext;
					if (other->lnext)
						other->lnext->lprev = other;
					block->lnext = other;
					other->lprev = block;
					block->width = width;
					other->owner = NULL;
					other->blocknum = i;
				}
				return block;
			}
		}
	}

	for (i = 0; i < NUM_BLOCK_SURFS; i++)
	{
		for (blines = cacheblocks[i]; blines; blines = blines->bnext)
		{
			if (blines->height < height)
			{
				continue;
			}
			if (blines->lnext)
			{
				continue;
			}

			block = blines;
			if (block->height > height)
			{
				if (!freeblocks)
					FlushOldCaches();
				other = freeblocks;
				freeblocks = other->chain;
				other->s = 0;
				other->t = block->t + height;
				other->width = block->width;
				other->height = block->height - height;
				other->lnext = NULL;
				other->lprev = NULL;
				other->bnext = block->bnext;
				if (other->bnext)
					other->bnext->bprev = other;
				block->bnext = other;
				other->bprev = block;
				block->height = height;
				other->owner = NULL;
				other->blocknum = i;
			}

			if (!freeblocks)
				FlushOldCaches();
			other = freeblocks;
			freeblocks = other->chain;
			other->s = block->s + width;
			other->t = block->t;
			other->width = block->width - width;
			other->height = block->height;
			other->lnext = NULL;
			block->lnext = other;
			other->lprev = block;
			block->width = width;
			other->owner = NULL;
			other->blocknum = i;

			return block;
		}
	}

	Sys_Error("overflow");
	return NULL;
	unguard;
}

//==========================================================================
//
//	VOpenGLDrawer::FreeBlock
//
//==========================================================================

surfcache_t *VOpenGLDrawer::FreeBlock(surfcache_t *block, bool check_lines)
{
	guard(VOpenGLDrawer::FreeBlock);
	surfcache_t		*other;

	if (block->owner)
	{
		*block->owner = NULL;
		block->owner = NULL;
	}
	if (block->lnext && !block->lnext->owner)
	{
		other = block->lnext;
		block->width += other->width;
		block->lnext = other->lnext;
		if (block->lnext)
			block->lnext->lprev = block;
		other->chain = freeblocks;
		freeblocks = other;
	}
	if (block->lprev && !block->lprev->owner)
	{
		other = block;
		block = block->lprev;
		block->width += other->width;
		block->lnext = other->lnext;
		if (block->lnext)
			block->lnext->lprev = block;
		other->chain = freeblocks;
		freeblocks = other;
	}

	if (block->lprev || block->lnext || !check_lines)
	{
		return block;
	}

	if (block->bnext && !block->bnext->lnext)
	{
		other = block->bnext;
		block->height += other->height;
		block->bnext = other->bnext;
		if (block->bnext)
			block->bnext->bprev = block;
		other->chain = freeblocks;
		freeblocks = other;
	}
	if (block->bprev && !block->bprev->lnext)
	{
		other = block;
		block = block->bprev;
		block->height += other->height;
		block->bnext = other->bnext;
		if (block->bnext)
			block->bnext->bprev = block;
		other->chain = freeblocks;
		freeblocks = other;
	}
	return block;
	unguard;
}

//==========================================================================
//
//	VOpenGLDrawer::FreeSurfCache
//
//==========================================================================

void VOpenGLDrawer::FreeSurfCache(surfcache_t *block)
{
	FreeBlock(block, true);
}

//==========================================================================
//
//	VOpenGLDrawer::CacheSurface
//
//==========================================================================

void VOpenGLDrawer::CacheSurface(surface_t *surface)
{
	guard(VOpenGLDrawer::CacheSurface);
	surfcache_t     *cache;
	int				smax, tmax;
	int				i, j, bnum;

	//
	// see if the cache holds apropriate data
	//
	cache = surface->cachespots[0];

	if (cache && !cache->dlight && surface->dlightframe != r_dlightframecount
			&& cache->Light == surface->Light)
	{
		bnum = cache->blocknum;
		cache->chain = light_chain[bnum];
		light_chain[bnum] = cache;
		cache->lastframe = cacheframecount;
		return;
	}

	//
	// determine shape of surface
	//
	smax = (surface->extents[0] >> 4) + 1;
	tmax = (surface->extents[1] >> 4) + 1;
	
	//
	// allocate memory if needed
	//
	if (!cache)     // if a texture just animated, don't reallocate it
	{
		cache = AllocBlock(smax, tmax);
		surface->cachespots[0] = cache;
		cache->owner = &surface->cachespots[0];
		cache->surf = surface;
	}
	
	if (surface->dlightframe == r_dlightframecount)
		cache->dlight = 1;
	else
		cache->dlight = 0;
	cache->Light = surface->Light;

	// calculate the lightings
	RendLev->BuildLightMap(surface, 0);
	bnum = cache->blocknum;
	block_changed[bnum] = true;

	for (j = 0; j < tmax; j++)
	{
		for (i = 0; i < smax; i++)
		{
			rgba_t &lb = light_block[bnum][(j + cache->t) * BLOCK_WIDTH +
				i + cache->s];
			lb.r = byte(blocklightsr[j * smax + i] >> 8);
			lb.g = byte(blocklightsg[j * smax + i] >> 8);
			lb.b = byte(blocklightsb[j * smax + i] >> 8);
			lb.a = 255;
		}
	}
	cache->chain = light_chain[bnum];
	light_chain[bnum] = cache;
	cache->lastframe = cacheframecount;

	// specular highlights
	for (j = 0; j < tmax; j++)
	{
		for (i = 0; i < smax; i++)
		{
			rgba_t &lb = add_block[bnum][(j + cache->t) * BLOCK_WIDTH +
				i + cache->s];
			lb.r = byte(blockaddlightsr[j * smax + i] >> 8);
			lb.g = byte(blockaddlightsg[j * smax + i] >> 8);
			lb.b = byte(blockaddlightsb[j * smax + i] >> 8);
			lb.a = 255;
		}
	}
	if (r_light_add)
	{
		cache->addchain = add_chain[bnum];
		add_chain[bnum] = cache;
		add_changed[bnum] = true;
	}
	unguard;
}

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
		SetTexture(tex->Tex);

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

				SetTexture(tex->Tex);
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

	SetFade(0);
	unguard;
}

//==========================================================================
//
//	VOpenGLDrawer::DrawSkyPolygon
//
//==========================================================================

void VOpenGLDrawer::DrawSkyPolygon(surface_t* surf, bool bIsSkyBox,
	VTexture* Texture1, float offs1, VTexture* Texture2, float offs2)
{
	guard(VOpenGLDrawer::DrawSkyPolygon);
	int		i;
	int		sidx[4];

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
		SetTexture(Texture1);
		SelectTexture(1);
		glEnable(GL_TEXTURE_2D);
		glTexEnvf(GL_TEXTURE_ENV, GL_TEXTURE_ENV_MODE, GL_DECAL);
		SetTexture(Texture2);
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
		SetTexture(Texture1);
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
			SetTexture(Texture2);
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
	SetTexture(tex->Tex);
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
	bool Additive, VTextureTranslation* Translation, vuint32 light,
	vuint32 Fade, const TVec&, float, const TVec& saxis, const TVec& taxis,
	const TVec& texorg)
{
	guard(VOpenGLDrawer::DrawSpritePolygon);
	TVec	texpt;

	SetSpriteLump(Tex, Translation);

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
	VTexture* Skin, VTextureTranslation* Trans, vuint32 light, vuint32 Fade,
	float Alpha, bool Additive, bool is_view_model)
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

	SetPic(Skin, Trans);

	glShadeModel(GL_SMOOTH);
	glEnable(GL_BLEND);
	if (Additive)
	{
		glBlendFunc(GL_SRC_ALPHA, GL_ONE);
	}

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
