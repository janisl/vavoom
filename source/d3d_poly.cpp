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

#include "d3d_local.h"

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

static byte ptex[8][8] =
{
	{ 0, 0, 0, 0, 0, 0, 0, 0 },
	{ 0, 0, 0, 0, 0, 0, 0, 0 },
	{ 0, 0, 0, 1, 1, 0, 0, 0 },
	{ 0, 0, 1, 1, 1, 1, 0, 0 },
	{ 0, 0, 1, 1, 1, 1, 0, 0 },
	{ 0, 0, 0, 1, 1, 0, 0, 0 },
	{ 0, 0, 0, 0, 0, 0, 0, 0 },
	{ 0, 0, 0, 0, 0, 0, 0, 0 },
};

// CODE --------------------------------------------------------------------

//==========================================================================
//
//	MatrixMultiply
//
//==========================================================================

void MatrixMultiply(MyD3DMatrix &out, const MyD3DMatrix& a, const MyD3DMatrix& b)
{
	MyD3DMatrix ret;
	for (int i = 0; i < 4; i++)
	{
		for (int j = 0; j < 4; j++)
		{
			ret(i, j) = 0.0f;
			for (int k = 0; k < 4; k++)
			{
				ret(i, j) += a(i, k) * b(k, j);
			}
		}
	}
	out = ret;
}

//==========================================================================
//
//	VDirect3DDrawer::FlushCaches
//
//==========================================================================

void VDirect3DDrawer::FlushCaches(bool free_blocks)
{
	guard(VDirect3DDrawer::FlushCaches);
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
//	VDirect3DDrawer::FlushOldCaches
//
//==========================================================================

void VDirect3DDrawer::FlushOldCaches()
{
	guard(VDirect3DDrawer::FlushOldCaches);
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
//	VDirect3DDrawer::AllocBlock
//
//==========================================================================

surfcache_t	*VDirect3DDrawer::AllocBlock(int width, int height)
{
	guard(VDirect3DDrawer::AllocBlock);
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
//	VDirect3DDrawer::FreeBlock
//
//==========================================================================

surfcache_t *VDirect3DDrawer::FreeBlock(surfcache_t *block, bool check_lines)
{
	guard(VDirect3DDrawer::FreeBlock);
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
//	VDirect3DDrawer::FreeSurfCache
//
//==========================================================================

void VDirect3DDrawer::FreeSurfCache(surfcache_t *block)
{
	FreeBlock(block, true);
}

//==========================================================================
//
//	VDirect3DDrawer::CacheSurface
//
//==========================================================================

void VDirect3DDrawer::CacheSurface(surface_t *surface)
{
	guard(VDirect3DDrawer::CacheSurface);
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
	R_BuildLightMap(surface, 0);
	bnum = cache->blocknum;
	block_changed[bnum] = true;

	for (j = 0; j < tmax; j++)
	{
		for (i = 0; i < smax; i++)
		{
			rgba_t &cdst = light_block[bnum][(j + cache->t) * BLOCK_WIDTH + i + cache->s];
			cdst.r = byte(255 - (blocklightsr[j * smax + i] >> 8));
			cdst.g = byte(255 - (blocklightsg[j * smax + i] >> 8));
			cdst.b = byte(255 - (blocklightsb[j * smax + i] >> 8));
			cdst.a = 255;
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
//	VDirect3DDrawer::DrawPolygon
//
//==========================================================================

void VDirect3DDrawer::DrawPolygon(surface_t* surf, int)
{
	guard(VDirect3DDrawer::DrawPolygon);
	bool lightmaped = surf->lightmap != NULL ||
		surf->dlightframe == r_dlightframecount;

	if (lightmaped)
	{
		CacheSurface(surf);
		if (maxMultiTex >= 2)
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
//	VDirect3DDrawer::DrawSkyPortal
//
//==========================================================================

void VDirect3DDrawer::DrawSkyPortal(surface_t* surf, int)
{
	guard(VDirect3DDrawer::DrawSkyPortal);
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
//	VDirect3DDrawer::WorldDrawing
//
//==========================================================================

void VDirect3DDrawer::WorldDrawing()
{
	guard(VDirect3DDrawer::WorldDrawing);
	MyD3DVertex		out[256];
	int				lb, i;
	surfcache_t		*cache;
	float			s, t, lights, lightt;
	surface_t		*surf;
	texinfo_t		*tex;
	vuint32			light;

	//	For sky areas we just write to the depth buffer to prevent drawing
	// polygons behind the sky.
	if (SkyPortalsHead)
	{
		RenderDevice->SetTextureStageState(0, D3DTSS_COLOROP, D3DTOP_DISABLE);
		//FIXME is there another way how to disable colour writes?
		RenderDevice->SetRenderState(D3DRENDERSTATE_ALPHABLENDENABLE, TRUE);
		for (surf = SkyPortalsHead; surf; surf = surf->DrawNext)
		{
			for (i = 0; i < surf->count; i++)
			{
				out[i] = MyD3DVertex(surf->verts[i], 0, 0, 0);
			}
#if DIRECT3D_VERSION >= 0x0800
			RenderDevice->DrawPrimitiveUP(D3DPT_TRIANGLEFAN, surf->count - 2, out, sizeof(MyD3DVertex));
#else
			RenderDevice->DrawPrimitive(D3DPT_TRIANGLEFAN, MYD3D_VERTEX_FORMAT, out, surf->count, 0);
#endif
		}
		RenderDevice->SetRenderState(D3DRENDERSTATE_ALPHABLENDENABLE, FALSE);
		RenderDevice->SetTextureStageState(0, D3DTSS_COLOROP, D3DTOP_MODULATE);
	}

	//	Draw surfaces.
	for (surf = SimpleSurfsHead; surf; surf = surf->DrawNext)
	{
		texinfo_t *tex = surf->texinfo;
		if (surf->lightmap != NULL ||
			surf->dlightframe == r_dlightframecount)
		{
			light = 0xffffffff;
		}
		else
		{
			int lev = surf->Light >> 24;
			int lR = ((surf->Light >> 16) & 255) * lev / 255;
			int lG = ((surf->Light >> 8) & 255) * lev / 255;
			int lB = (surf->Light & 255) * lev / 255;
			light = 0xff000000 | (lR << 16) | (lG << 8) | lB;
		}

		SetTexture(tex->pic);

		for (i = 0; i < surf->count; i++)
		{
			out[i] = MyD3DVertex(surf->verts[i], light,
				(DotProduct(surf->verts[i], tex->saxis) + tex->soffs) * tex_iw,
				(DotProduct(surf->verts[i], tex->taxis) + tex->toffs) * tex_ih);
		}

#if DIRECT3D_VERSION >= 0x0800
		RenderDevice->DrawPrimitiveUP(D3DPT_TRIANGLEFAN, surf->count - 2, out, sizeof(MyD3DVertex));
#else
		RenderDevice->DrawPrimitive(D3DPT_TRIANGLEFAN, MYD3D_VERTEX_FORMAT, out, surf->count, 0);
#endif
	}

	if (maxMultiTex >= 2)
	{
		RenderDevice->SetTextureStageState(0, D3DTSS_COLOROP, D3DTOP_SELECTARG1);
		RenderDevice->SetTextureStageState(1, D3DTSS_COLOROP, D3DTOP_MODULATE);
		RenderDevice->SetTextureStageState(1, D3DTSS_COLORARG1, D3DTA_TEXTURE);
		RenderDevice->SetTextureStageState(1, D3DTSS_COLORARG2, D3DTA_CURRENT);
		RenderDevice->SetTextureStageState(1, D3DTSS_TEXCOORDINDEX, 1);
#if DIRECT3D_VERSION >= 0x0800
		RenderDevice->SetTextureStageState(1, D3DTSS_MAGFILTER, D3DTEXF_LINEAR);
		RenderDevice->SetTextureStageState(1, D3DTSS_MINFILTER, D3DTEXF_LINEAR);
#else
		RenderDevice->SetTextureStageState(1, D3DTSS_MAGFILTER, D3DTFG_LINEAR);
		RenderDevice->SetTextureStageState(1, D3DTSS_MINFILTER, D3DTFN_LINEAR);
#endif

		for (lb = 0; lb < NUM_BLOCK_SURFS; lb++)
		{
			if (!light_chain[lb])
			{
				continue;
			}

			if (!light_surf[lb])
			{
				light_surf[lb] = CreateSurface(BLOCK_WIDTH, BLOCK_HEIGHT, 16, false);
				block_changed[lb] = true;
			}
			if (block_changed[lb])
			{
				block_changed[lb] = false;
#if DIRECT3D_VERSION >= 0x0800
				UploadTextureImage(light_surf[lb], 0, BLOCK_WIDTH, BLOCK_HEIGHT, light_block[lb]);
#else
				UploadTextureImage(light_surf[lb], BLOCK_WIDTH, BLOCK_HEIGHT, light_block[lb]);
#endif
			}

			RenderDevice->SetTexture(1, light_surf[lb]);

			for (cache = light_chain[lb]; cache; cache = cache->chain)
			{
				surf = cache->surf;
				tex = surf->texinfo;
				SetTexture(tex->pic);
				for (i = 0; i < surf->count; i++)
				{
					s = DotProduct(surf->verts[i], tex->saxis) + tex->soffs;
					t = DotProduct(surf->verts[i], tex->taxis) + tex->toffs;
					lights = (s - surf->texturemins[0]) / 16 + cache->s + 0.5;
					lightt = (t - surf->texturemins[1]) / 16 + cache->t + 0.5;
					out[i] = MyD3DVertex(surf->verts[i], 0xffffffff,
						s * tex_iw, t * tex_ih,
						lights / BLOCK_WIDTH, lightt / BLOCK_HEIGHT);
				}
#if DIRECT3D_VERSION >= 0x0800
				RenderDevice->DrawPrimitiveUP(D3DPT_TRIANGLEFAN, surf->count - 2, out, sizeof(MyD3DVertex));
#else
				RenderDevice->DrawPrimitive(D3DPT_TRIANGLEFAN, MYD3D_VERTEX_FORMAT, out, surf->count, 0);
#endif
			}
		}

		RenderDevice->SetTextureStageState(0, D3DTSS_COLOROP, D3DTOP_MODULATE);
		RenderDevice->SetTextureStageState(1, D3DTSS_COLOROP, D3DTOP_DISABLE);
	}
	else
	{
#if DIRECT3D_VERSION >= 0x0800
		RenderDevice->SetTextureStageState(0, D3DTSS_MAGFILTER, D3DTEXF_LINEAR);
		RenderDevice->SetTextureStageState(0, D3DTSS_MINFILTER, D3DTEXF_LINEAR);
		RenderDevice->SetTextureStageState(0, D3DTSS_MIPFILTER, D3DTEXF_NONE);
#else
		RenderDevice->SetTextureStageState(0, D3DTSS_MAGFILTER, D3DTFG_LINEAR);
		RenderDevice->SetTextureStageState(0, D3DTSS_MINFILTER, D3DTFN_LINEAR);
		RenderDevice->SetTextureStageState(0, D3DTSS_MIPFILTER, D3DTFP_NONE);
#endif
		RenderDevice->SetRenderState(D3DRENDERSTATE_SRCBLEND, D3DBLEND_ZERO);
		RenderDevice->SetRenderState(D3DRENDERSTATE_DESTBLEND, D3DBLEND_SRCCOLOR);
		RenderDevice->SetRenderState(D3DRENDERSTATE_ALPHABLENDENABLE, TRUE);
		RenderDevice->SetRenderState(D3DRENDERSTATE_ZWRITEENABLE, FALSE);	// don't bother writing Z

		for (lb = 0; lb < NUM_BLOCK_SURFS; lb++)
		{
			if (!light_chain[lb])
			{
				continue;
			}

			if (!light_surf[lb])
			{
				light_surf[lb] = CreateSurface(BLOCK_WIDTH, BLOCK_HEIGHT, 16, false);
				block_changed[lb] = true;
			}
			if (block_changed[lb])
			{
				block_changed[lb] = false;
#if DIRECT3D_VERSION >= 0x0800
				UploadTextureImage(light_surf[lb], 0, BLOCK_WIDTH, BLOCK_HEIGHT, light_block[lb]);
#else
				UploadTextureImage(light_surf[lb], BLOCK_WIDTH, BLOCK_HEIGHT, light_block[lb]);
#endif
			}

			RenderDevice->SetTexture(0, light_surf[lb]);

			for (cache = light_chain[lb]; cache; cache = cache->chain)
			{
				surf = cache->surf;
				tex = surf->texinfo;
				for (i = 0; i < surf->count; i++)
				{
					s = DotProduct(surf->verts[i], tex->saxis) + tex->soffs;
					t = DotProduct(surf->verts[i], tex->taxis) + tex->toffs;
					lights = (s - surf->texturemins[0]) / 16 + cache->s + 0.5;
					lightt = (t - surf->texturemins[1]) / 16 + cache->t + 0.5;
					out[i] = MyD3DVertex(surf->verts[i], 0xffffffff,
						lights / BLOCK_WIDTH, lightt / BLOCK_HEIGHT);
				}
#if DIRECT3D_VERSION >= 0x0800
				RenderDevice->DrawPrimitiveUP(D3DPT_TRIANGLEFAN, surf->count - 2, out, sizeof(MyD3DVertex));
#else
				RenderDevice->DrawPrimitive(D3DPT_TRIANGLEFAN, MYD3D_VERTEX_FORMAT, out, surf->count, 0);
#endif
			}
		}

		RenderDevice->SetTextureStageState(0, D3DTSS_MAGFILTER, magfilter);
		RenderDevice->SetTextureStageState(0, D3DTSS_MINFILTER, minfilter);
		RenderDevice->SetTextureStageState(0, D3DTSS_MIPFILTER, mipfilter);
		RenderDevice->SetRenderState(D3DRENDERSTATE_ALPHABLENDENABLE, FALSE);
		RenderDevice->SetRenderState(D3DRENDERSTATE_SRCBLEND, D3DBLEND_SRCALPHA);
		RenderDevice->SetRenderState(D3DRENDERSTATE_DESTBLEND, D3DBLEND_INVSRCALPHA);
		RenderDevice->SetRenderState(D3DRENDERSTATE_ZWRITEENABLE, TRUE);		// back to normal Z buffering
	}

	//
	//  Add specular hightlights
	//
	if (specular_highlights)
	{
#if DIRECT3D_VERSION >= 0x0800
		RenderDevice->SetTextureStageState(0, D3DTSS_MAGFILTER, D3DTEXF_LINEAR);
		RenderDevice->SetTextureStageState(0, D3DTSS_MINFILTER, D3DTEXF_LINEAR);
		RenderDevice->SetTextureStageState(0, D3DTSS_MIPFILTER, D3DTEXF_NONE);
#else
		RenderDevice->SetTextureStageState(0, D3DTSS_MAGFILTER, D3DTFG_LINEAR);
		RenderDevice->SetTextureStageState(0, D3DTSS_MINFILTER, D3DTFN_LINEAR);
		RenderDevice->SetTextureStageState(0, D3DTSS_MIPFILTER, D3DTFP_NONE);
#endif
		RenderDevice->SetRenderState(D3DRENDERSTATE_SRCBLEND, D3DBLEND_ONE);
		RenderDevice->SetRenderState(D3DRENDERSTATE_DESTBLEND, D3DBLEND_ONE);
		RenderDevice->SetRenderState(D3DRENDERSTATE_ALPHABLENDENABLE, TRUE);
		RenderDevice->SetRenderState(D3DRENDERSTATE_ZWRITEENABLE, FALSE);	// don't bother writing Z
		if (r_fog)
		{
			RenderDevice->SetRenderState(D3DRENDERSTATE_FOGENABLE, FALSE);
		}

		for (lb = 0; lb < NUM_BLOCK_SURFS; lb++)
		{
			if (!add_chain[lb])
			{
				continue;
			}

			if (!add_surf[lb])
			{
				add_surf[lb] = CreateSurface(BLOCK_WIDTH, BLOCK_HEIGHT, 16, false);
				add_changed[lb] = true;
			}
			if (add_changed[lb])
			{
				add_changed[lb] = false;
#if DIRECT3D_VERSION >= 0x0800
				UploadTextureImage(add_surf[lb], 0, BLOCK_WIDTH, BLOCK_HEIGHT, add_block[lb]);
#else
				UploadTextureImage(add_surf[lb], BLOCK_WIDTH, BLOCK_HEIGHT, add_block[lb]);
#endif
			}

			RenderDevice->SetTexture(0, add_surf[lb]);

			for (cache = add_chain[lb]; cache; cache = cache->addchain)
			{
				surf = cache->surf;
				tex = surf->texinfo;
				for (i = 0; i < surf->count; i++)
				{
					s = DotProduct(surf->verts[i], tex->saxis) + tex->soffs;
					t = DotProduct(surf->verts[i], tex->taxis) + tex->toffs;
					lights = (s - surf->texturemins[0]) / 16 + cache->s + 0.5;
					lightt = (t - surf->texturemins[1]) / 16 + cache->t + 0.5;
					out[i] = MyD3DVertex(surf->verts[i], 0xffffffff,
						lights / BLOCK_WIDTH, lightt / BLOCK_HEIGHT);
				}
#if DIRECT3D_VERSION >= 0x0800
				RenderDevice->DrawPrimitiveUP(D3DPT_TRIANGLEFAN, surf->count - 2, out, sizeof(MyD3DVertex));
#else
				RenderDevice->DrawPrimitive(D3DPT_TRIANGLEFAN, MYD3D_VERTEX_FORMAT, out, surf->count, 0);
#endif
			}
		}

		RenderDevice->SetTextureStageState(0, D3DTSS_MAGFILTER, magfilter);
		RenderDevice->SetTextureStageState(0, D3DTSS_MINFILTER, minfilter);
		RenderDevice->SetTextureStageState(0, D3DTSS_MIPFILTER, mipfilter);
		RenderDevice->SetRenderState(D3DRENDERSTATE_ALPHABLENDENABLE, FALSE);
		RenderDevice->SetRenderState(D3DRENDERSTATE_SRCBLEND, D3DBLEND_SRCALPHA);
		RenderDevice->SetRenderState(D3DRENDERSTATE_DESTBLEND, D3DBLEND_INVSRCALPHA);
		RenderDevice->SetRenderState(D3DRENDERSTATE_ZWRITEENABLE, TRUE);		// back to normal Z buffering
		if (r_fog)
		{
			RenderDevice->SetRenderState(D3DRENDERSTATE_FOGENABLE, TRUE);
		}
	}
	unguard;
}

//==========================================================================
//
//	VDirect3DDrawer::BeginSky
//
//==========================================================================

void VDirect3DDrawer::BeginSky()
{
	guard(VDirect3DDrawer::BeginSky);
	RenderDevice->SetRenderState(D3DRENDERSTATE_ZWRITEENABLE, FALSE);
	if (r_fog)
	{
		RenderDevice->SetRenderState(D3DRENDERSTATE_FOGENABLE, FALSE);
	}
	unguard;
}

//==========================================================================
//
//	VDirect3DDrawer::DrawSkyPolygon
//
//==========================================================================

void VDirect3DDrawer::DrawSkyPolygon(surface_t* surf, bool bIsSkyBox,
	int texture1, float offs1, int texture2, float offs2)
{
	guard(VDirect3DDrawer::DrawSkyPolygon);
	MyD3DVertex		out[256];
	int				i;
	int				sidx[4];

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
	if (maxMultiTex >= 2 && texture2)
	{
		RenderDevice->SetTextureStageState(0, D3DTSS_COLOROP, D3DTOP_SELECTARG1);
		RenderDevice->SetTextureStageState(1, D3DTSS_COLOROP, D3DTOP_BLENDTEXTUREALPHA);
		RenderDevice->SetTextureStageState(1, D3DTSS_COLORARG1, D3DTA_TEXTURE);
		RenderDevice->SetTextureStageState(1, D3DTSS_COLORARG2, D3DTA_CURRENT);
		RenderDevice->SetTextureStageState(1, D3DTSS_TEXCOORDINDEX, 1);

		SetTexture(texture1);
		TexStage = 1;
		SetTexture(texture2);
		TexStage = 0;
		for (i = 0; i < surf->count; i++)
		{
			out[i] = MyD3DVertex(surf->verts[i] + vieworg, 0xffffffff,
				(DotProduct(surf->verts[sidx[i]], tex->saxis) + tex->soffs - offs1) * tex_iw,
				(DotProduct(surf->verts[i], tex->taxis) + tex->toffs) * tex_ih,
				(DotProduct(surf->verts[sidx[i]], tex->saxis) + tex->soffs - offs2) * tex_iw,
				(DotProduct(surf->verts[i], tex->taxis) + tex->toffs) * tex_ih);
		}
#if DIRECT3D_VERSION >= 0x0800
		RenderDevice->DrawPrimitiveUP(D3DPT_TRIANGLEFAN, surf->count - 2, out, sizeof(MyD3DVertex));
#else
		RenderDevice->DrawPrimitive(D3DPT_TRIANGLEFAN, MYD3D_VERTEX_FORMAT, out, surf->count, 0);
#endif

		RenderDevice->SetTextureStageState(0, D3DTSS_COLOROP, D3DTOP_MODULATE);
		RenderDevice->SetTextureStageState(1, D3DTSS_COLOROP, D3DTOP_DISABLE);
	}
	else
	{
		SetTexture(texture1);
		for (i = 0; i < surf->count; i++)
		{
			out[i] = MyD3DVertex(surf->verts[i] + vieworg, 0xffffffff,
				(DotProduct(surf->verts[sidx[i]], tex->saxis) + tex->soffs - offs1) * tex_iw,
				(DotProduct(surf->verts[i], tex->taxis) + tex->toffs) * tex_ih);
		}
#if DIRECT3D_VERSION >= 0x0800
		RenderDevice->DrawPrimitiveUP(D3DPT_TRIANGLEFAN, surf->count - 2, out, sizeof(MyD3DVertex));
#else
		RenderDevice->DrawPrimitive(D3DPT_TRIANGLEFAN, MYD3D_VERTEX_FORMAT, out, surf->count, 0);
#endif

		if (texture2)
		{
			SetTexture(texture2);
			for (i = 0; i < surf->count; i++)
			{
				out[i] = MyD3DVertex(surf->verts[i] + vieworg, 0xffffffff,
					(DotProduct(surf->verts[sidx[i]], tex->saxis) + tex->soffs - offs2) * tex_iw,
					(DotProduct(surf->verts[i], tex->taxis) + tex->toffs) * tex_ih);
			}
			RenderDevice->SetRenderState(D3DRENDERSTATE_ALPHABLENDENABLE, TRUE);
#if DIRECT3D_VERSION >= 0x0800
			RenderDevice->DrawPrimitiveUP(D3DPT_TRIANGLEFAN, surf->count - 2, out, sizeof(MyD3DVertex));
#else
			RenderDevice->DrawPrimitive(D3DPT_TRIANGLEFAN, MYD3D_VERTEX_FORMAT, out, surf->count, 0);
#endif
			RenderDevice->SetRenderState(D3DRENDERSTATE_ALPHABLENDENABLE, FALSE);
		}
	}
	unguard;
}

//==========================================================================
//
//	VDirect3DDrawer::EndSky
//
//==========================================================================

void VDirect3DDrawer::EndSky()
{
	guard(VDirect3DDrawer::EndSky);
	if (r_fog)
	{
		RenderDevice->SetRenderState(D3DRENDERSTATE_FOGENABLE, TRUE);
	}
	RenderDevice->SetRenderState(D3DRENDERSTATE_ZWRITEENABLE, TRUE);
	unguard;
}

//==========================================================================
//
//	VDirect3DDrawer::DrawMaskedPolygon
//
//==========================================================================

void VDirect3DDrawer::DrawMaskedPolygon(surface_t* surf, float Alpha)
{
	guard(VDirect3DDrawer::DrawMaskedPolygon);
	MyD3DVertex	out[256];
	int			l;

	texinfo_t* tex = surf->texinfo;
	SetTexture(tex->pic);

	if (surf->lightmap != NULL ||
		surf->dlightframe == r_dlightframecount)
	{
		R_BuildLightMap(surf, 0);
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
		r = r / (size * 256);
		g = g / (size * 256);
		b = b / (size * 256);
		int alpha = (int)(Alpha * 255);
		l = (alpha << 24) | (r << 16) | (g << 8) | b;
	}
	else
	{
		int lev = surf->Light >> 24;
		int lR = ((surf->Light >> 16) & 255) * lev / 255;
		int lG = ((surf->Light >> 8) & 255) * lev / 255;
		int lB = (surf->Light & 255) * lev / 255;
		int alpha = (int)(Alpha * 255);
		l = (alpha << 24) | (lR << 16) | (lG << 8) | lB;
	}

	for (int i = 0; i < surf->count; i++)
	{
		out[i] = MyD3DVertex(surf->verts[i], l,
			(DotProduct(surf->verts[i], tex->saxis) + tex->soffs) * tex_iw,
			(DotProduct(surf->verts[i], tex->taxis) + tex->toffs) * tex_ih);
	}

	RenderDevice->SetRenderState(D3DRENDERSTATE_ALPHATESTENABLE, TRUE);
	if (blend_sprites || Alpha < 1.0)
	{
		RenderDevice->SetRenderState(D3DRENDERSTATE_ALPHABLENDENABLE, TRUE);
		RenderDevice->SetRenderState(D3DRENDERSTATE_ALPHAREF, 0);
	}

#if DIRECT3D_VERSION >= 0x0800
	RenderDevice->DrawPrimitiveUP(D3DPT_TRIANGLEFAN, surf->count - 2, out, sizeof(MyD3DVertex));
#else
	RenderDevice->DrawPrimitive(D3DPT_TRIANGLEFAN, MYD3D_VERTEX_FORMAT, out, surf->count, 0);
#endif

	RenderDevice->SetRenderState(D3DRENDERSTATE_ALPHATESTENABLE, FALSE);
	if (blend_sprites || Alpha < 1.0)
	{
		RenderDevice->SetRenderState(D3DRENDERSTATE_ALPHABLENDENABLE, FALSE);
		RenderDevice->SetRenderState(D3DRENDERSTATE_ALPHAREF, 170);
	}
	unguard;
}

//==========================================================================
//
//	VDirect3DDrawer::DrawSpritePolygon
//
//==========================================================================

void VDirect3DDrawer::DrawSpritePolygon(TVec *cv, int lump, float Alpha,
	int translation, vuint32 light, const TVec&, float, const TVec& saxis,
	const TVec& taxis, const TVec& texorg)
{
	guard(VDirect3DDrawer::DrawSpritePolygon);
	MyD3DVertex		out[4];

	SetSpriteLump(lump, translation);

	int l = ((int)(Alpha * 255) << 24) | (light & 0x00ffffff);
	for (int i = 0; i < 4; i++)
	{
		TVec texpt = cv[i] - texorg;
		out[i] = MyD3DVertex(cv[i], l,
			DotProduct(texpt, saxis) * tex_iw,
			DotProduct(texpt, taxis) * tex_ih);
	}

	if (blend_sprites || Alpha < 1.0)
	{
		RenderDevice->SetRenderState(D3DRENDERSTATE_ALPHABLENDENABLE, TRUE);
		RenderDevice->SetRenderState(D3DRENDERSTATE_ALPHAREF, 0);
	}
	RenderDevice->SetRenderState(D3DRENDERSTATE_ALPHATESTENABLE, TRUE);

#if DIRECT3D_VERSION >= 0x0800
	RenderDevice->DrawPrimitiveUP(D3DPT_TRIANGLEFAN, 2, out, sizeof(MyD3DVertex));
#else
	RenderDevice->DrawPrimitive(D3DPT_TRIANGLEFAN, MYD3D_VERTEX_FORMAT, out, 4, 0);
#endif

	if (blend_sprites || Alpha < 1.0)
	{
		RenderDevice->SetRenderState(D3DRENDERSTATE_ALPHABLENDENABLE, FALSE);
		RenderDevice->SetRenderState(D3DRENDERSTATE_ALPHAREF, 170);
	}
	RenderDevice->SetRenderState(D3DRENDERSTATE_ALPHATESTENABLE, FALSE);
	unguard;
}

//==========================================================================
//
//	VDirect3DDrawer::DrawAliasModel
//
//==========================================================================

void VDirect3DDrawer::DrawAliasModel(const TVec &origin, const TAVec &angles,
	const TVec& Offset, const TVec& Scale, mmdl_t* pmdl, int frame,
	int SkinID, vuint32 light, float Alpha, bool is_view_model)
{
	guard(VDirect3DDrawer::DrawAliasModel);
	mframe_t			*pframedesc;
	float 				l;
	int					index;
	trivertx_t			*verts;
	int					*order;
	int					count;
	float				shadelightr;
	float				shadelightg;
	float				shadelightb;
	float				*shadedots;
	MyD3DVertex			out[256];//FIXME
	D3DPRIMITIVETYPE	primtype;
	int					i;
	MyD3DMatrix			matWorld;
	MyD3DMatrix			matTmp;
	TVec				alias_forward;
	TVec				alias_right;
	TVec				alias_up;
	vuint32				alpha;

	if (is_view_model)
	{
		// hack the depth range to prevent view model from poking into walls
#if DIRECT3D_VERSION >= 0x0800
		viewData.MaxZ = 0.3;
#else
		viewData.dvMaxZ = 0.3;
#endif
		RenderDevice->SetViewport(&viewData);
	}

	//
	// get lighting information
	//
	shadelightr = ((light >> 16) & 0xff) / 512.0;
	shadelightg = ((light >> 8) & 0xff) / 512.0;
	shadelightb = (light & 0xff) / 512.0;
	shadedots = r_avertexnormal_dots[((int)(angles.yaw * (SHADEDOT_QUANT / 360.0))) & (SHADEDOT_QUANT - 1)];
	light &= 0x00ffffff;
	alpha = int(Alpha * 255) << 24;
	
	//
	// draw all the triangles
	//

	pframedesc = (mframe_t*)((byte *)pmdl + pmdl->ofsframes + frame * pmdl->framesize);

	AngleVectors(angles, alias_forward, alias_right, alias_up);

	matWorld = IdentityMatrix;
	for (i = 0; i < 3; i++)
	{
		matWorld(0, i) = alias_forward[i];
		matWorld(1, i) = -alias_right[i];
		matWorld(2, i) = alias_up[i];
	}
	matWorld(3, 0) = origin.x;
	matWorld(3, 1) = origin.y;
	matWorld(3, 2) = origin.z;

	matTmp = IdentityMatrix;
	matTmp(0, 0) = pframedesc->scale[0] * Scale.x;
	matTmp(1, 1) = pframedesc->scale[1] * Scale.y;
	matTmp(2, 2) = pframedesc->scale[2] * Scale.z;
	matTmp(3, 0) = pframedesc->scale_origin[0] * Scale.x + Offset.x;
	matTmp(3, 1) = pframedesc->scale_origin[1] * Scale.y + Offset.x;
	matTmp(3, 2) = pframedesc->scale_origin[2] * Scale.z + Offset.x;

	matWorld = matTmp * matWorld;
	RenderDevice->SetTransform(D3DTRANSFORMSTATE_WORLD, &matWorld);

	SetPic(SkinID);

	RenderDevice->SetRenderState(D3DRENDERSTATE_SHADEMODE, D3DSHADE_GOURAUD);
	RenderDevice->SetRenderState(D3DRENDERSTATE_ALPHABLENDENABLE, TRUE);

	verts = (trivertx_t *)(pframedesc + 1);
	order = (int *)((byte *)pmdl + pmdl->ofscmds);

	while (*order)
	{
		// get the vertex count and primitive type
		count = *order++;
		if (count < 0)
		{
			count = -count;
			primtype = D3DPT_TRIANGLEFAN;
		}
		else
		{
			primtype = D3DPT_TRIANGLESTRIP;
		}

		if (count > 256)
		{
			Sys_Error("Too many command verts");
		}

		for (i = 0; i < count; i++)
		{
			// texture coordinates come from the draw list
			out[i].texs = ((float *)order)[0];
			out[i].text = ((float *)order)[1];
			order += 2;

			// normals and vertexes come from the frame list
			index = *order++;
			if (model_lighting)
			{
				l = shadedots[verts[index].lightnormalindex];
				out[i].colour = alpha |
					(((int)(l * shadelightr * 0xff) << 16) & 0x00ff0000) |
					(((int)(l * shadelightg * 0xff) <<  8) & 0x0000ff00) |
					(((int)(l * shadelightb * 0xff)      ) & 0x000000ff);
			}
			else
			{
				out[i].colour = alpha | light;
			}

			out[i].x = verts[index].v[0];
			out[i].y = verts[index].v[1];
			out[i].z = verts[index].v[2];
		}

#if DIRECT3D_VERSION >= 0x0800
		RenderDevice->DrawPrimitiveUP(primtype, count - 2, out, sizeof(MyD3DVertex));
#else
		RenderDevice->DrawPrimitive(primtype, MYD3D_VERTEX_FORMAT, out, count, 0);
#endif
	}

	RenderDevice->SetRenderState(D3DRENDERSTATE_SHADEMODE, D3DSHADE_FLAT);
	RenderDevice->SetRenderState(D3DRENDERSTATE_ALPHABLENDENABLE, FALSE);

	RenderDevice->SetTransform(D3DTRANSFORMSTATE_WORLD, &IdentityMatrix);
	if (is_view_model)
	{
#if DIRECT3D_VERSION >= 0x0800
		viewData.MaxZ = 1.0;
#else
		viewData.dvMaxZ = 1.0;
#endif
		RenderDevice->SetViewport(&viewData);
	}
	unguard;
}

//==========================================================================
//
//	VDirect3DDrawer::StartParticles
//
//==========================================================================

void VDirect3DDrawer::StartParticles()
{
	guard(VDirect3DDrawer::StartParticles);
	if (!particle_texture)
	{
		rgba_t		pbuf[8][8];

		for (int j = 0; j < 8; j++)
		{
			for (int i = 0; i < 8; i++)
			{
				pbuf[j][i].r = 255;
				pbuf[j][i].g = 255;
				pbuf[j][i].b = 255;
				pbuf[j][i].a = byte(ptex[j][i] * 255);
			}
		}
		particle_texture = CreateSurface(8, 8, 16, false);
#if DIRECT3D_VERSION >= 0x0800
		UploadTextureImage(particle_texture, 0, 8, 8, &pbuf[0][0]);
#else
		UploadTextureImage(particle_texture, 8, 8, &pbuf[0][0]);
#endif
	}
	RenderDevice->SetTexture(0, particle_texture);
	RenderDevice->SetRenderState(D3DRENDERSTATE_ALPHABLENDENABLE, TRUE);
	RenderDevice->SetRenderState(D3DRENDERSTATE_ALPHATESTENABLE, TRUE);
	RenderDevice->SetRenderState(D3DRENDERSTATE_ALPHAREF, 0);
	unguard;
}

//==========================================================================
//
//	VDirect3DDrawer::DrawParticle
//
//==========================================================================

void VDirect3DDrawer::DrawParticle(particle_t *p)
{
	guard(VDirect3DDrawer::DrawParticle);
	MyD3DVertex out[4];
	out[0] = MyD3DVertex(p->org - viewright + viewup, p->colour, 0, 0);
	out[1] = MyD3DVertex(p->org + viewright + viewup, p->colour, 1, 0);
	out[2] = MyD3DVertex(p->org + viewright - viewup, p->colour, 1, 1);
	out[3] = MyD3DVertex(p->org - viewright - viewup, p->colour, 0, 1);
#if DIRECT3D_VERSION >= 0x0800
	RenderDevice->DrawPrimitiveUP(D3DPT_TRIANGLEFAN, 2, out, sizeof(MyD3DVertex));
#else
	RenderDevice->DrawPrimitive(D3DPT_TRIANGLEFAN, MYD3D_VERTEX_FORMAT, out, 4, 0);
#endif
	unguard;
}

//==========================================================================
//
//	VDirect3DDrawer::EndParticles
//
//==========================================================================

void VDirect3DDrawer::EndParticles()
{
	guard(VDirect3DDrawer::EndParticles);
	RenderDevice->SetRenderState(D3DRENDERSTATE_ALPHABLENDENABLE, FALSE);
	RenderDevice->SetRenderState(D3DRENDERSTATE_ALPHATESTENABLE, FALSE);
	RenderDevice->SetRenderState(D3DRENDERSTATE_ALPHAREF, 170);
	unguard;
}
