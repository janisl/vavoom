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
//**	Copyright (C) 1999-2001 JÆnis Legzdi·ý
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
//	TDirect3DDrawer::FlushCaches
//
//==========================================================================

void TDirect3DDrawer::FlushCaches(bool free_blocks)
{
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
}

//==========================================================================
//
//	TDirect3DDrawer::AllocBlock
//
//==========================================================================

surfcache_t	*TDirect3DDrawer::AllocBlock(int width, int height)
{
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
						Sys_Error("No more free blocks");
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
					Sys_Error("No more free blocks");
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
				Sys_Error("No more free blocks");
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
}

//==========================================================================
//
//	TDirect3DDrawer::FreeSurfCache
//
//==========================================================================

void TDirect3DDrawer::FreeSurfCache(surfcache_t *block)
{
	surfcache_t		*other;

	*block->owner = NULL;
	block->owner = NULL;
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

	if (block->lprev || block->lnext)
	{
		return;
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
}

//==========================================================================
//
//	TDirect3DDrawer::CacheSurface
//
//==========================================================================

void TDirect3DDrawer::CacheSurface(surface_t *surface)
{
	surfcache_t     *cache;
	int				smax, tmax;
	int				i, j, bnum;

	//
	// see if the cache holds apropriate data
	//
	cache = surface->cachespots[0];

	if (cache && !cache->dlight && surface->dlightframe != r_dlightframecount
			&& cache->lightlevel == surface->lightlevel)
	{
		bnum = cache->blocknum;
		cache->chain = light_chain[bnum];
		light_chain[bnum] = cache;
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
	cache->lightlevel = surface->lightlevel;

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
}

//==========================================================================
//
//	TDirect3DDrawer::DrawPolygon
//
//==========================================================================

void TDirect3DDrawer::DrawPolygon(TVec *cv, int count, int texture, int)
{
	MyD3DVertex		out[256];
	int				i, l;
	bool			lightmaped;
	surface_t		*surf = r_surface;

	lightmaped = surf->lightmap != NULL ||
		surf->dlightframe == r_dlightframecount;

	if (lightmaped)
	{
		CacheSurface(surf);
		if (maxMultiTex >= 2)
		{
			return;
		}
		l = 0xffffffff;
	}
	else
	{
		l =	0xff000000 | (surf->lightlevel << 16) |
			(surf->lightlevel << 8) | surf->lightlevel;
	}

	SetTexture(texture);

	for (i = 0; i < count; i++)
	{
		TVec texpt = cv[i] - r_texorg;
		out[i] = MyD3DVertex(cv[i], l,
			DotProduct(texpt, r_saxis) * tex_iw,
			DotProduct(texpt, r_taxis) * tex_ih);
	}

#if DIRECT3D_VERSION >= 0x0800
	RenderDevice->DrawPrimitiveUP(D3DPT_TRIANGLEFAN, count - 2, out, sizeof(MyD3DVertex));
#else
	RenderDevice->DrawPrimitive(D3DPT_TRIANGLEFAN, MYD3D_VERTEX_FORMAT, out, count, 0);
#endif
}

//==========================================================================
//
//	TDirect3DDrawer::WorldDrawing
//
//==========================================================================

void TDirect3DDrawer::WorldDrawing(void)
{
	MyD3DVertex		out[256];
	int				lb, i;
	surfcache_t		*cache;
	float			s, t, lights, lightt;
	surface_t		*surf;
	texinfo_t		*tex;

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
					TVec texpt = surf->verts[i] - tex->texorg;
					s = DotProduct(texpt, tex->saxis);
					t = DotProduct(texpt, tex->taxis);
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
					TVec texpt = surf->verts[i] - tex->texorg;
					s = DotProduct(texpt, tex->saxis);
					t = DotProduct(texpt, tex->taxis);
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
}

//==========================================================================
//
//	TDirect3DDrawer::BeginSky
//
//==========================================================================

void TDirect3DDrawer::BeginSky(void)
{
#if DIRECT3D_VERSION >= 0x0800
	viewData.MinZ = 0.99;
#else
	viewData.dvMinZ = 0.99;
#endif
    RenderDevice->SetViewport(&viewData);

	if (r_use_fog)
	{
		RenderDevice->SetRenderState(D3DRENDERSTATE_FOGENABLE, FALSE);
	}
}

//==========================================================================
//
//	TDirect3DDrawer::DrawSkyPolygon
//
//==========================================================================

void TDirect3DDrawer::DrawSkyPolygon(TVec *cv, int count,
	int texture1, float offs1, int texture2, float offs2)
{
	MyD3DVertex		out[256];
	int				i;

	SetSkyTexture(texture1, false);
	for (i = 0; i < count; i++)
	{
		TVec v = cv[i] + vieworg;
		TVec texpt = cv[i] - r_texorg;
		out[i] = MyD3DVertex(v, 0xffffffff,
			(DotProduct(texpt, r_saxis) - offs1) * tex_iw,
			DotProduct(texpt, r_taxis) * tex_ih);
	}
#if DIRECT3D_VERSION >= 0x0800
	RenderDevice->DrawPrimitiveUP(D3DPT_TRIANGLEFAN, count - 2, out, sizeof(MyD3DVertex));
#else
	RenderDevice->DrawPrimitive(D3DPT_TRIANGLEFAN, MYD3D_VERTEX_FORMAT, out, count, 0);
#endif

	if (texture2)
	{
		SetSkyTexture(texture2, true);
		for (i = 0; i < count; i++)
		{
			TVec v = cv[i] + vieworg;
			TVec texpt = cv[i] - r_texorg;
			out[i] = MyD3DVertex(v, 0xffffffff,
				(DotProduct(texpt, r_saxis) - offs2) * tex_iw,
				DotProduct(texpt, r_taxis) * tex_ih);
		}
		RenderDevice->SetRenderState(D3DRENDERSTATE_ALPHABLENDENABLE, TRUE);
#if DIRECT3D_VERSION >= 0x0800
		RenderDevice->DrawPrimitiveUP(D3DPT_TRIANGLEFAN, count - 2, out, sizeof(MyD3DVertex));
#else
		RenderDevice->DrawPrimitive(D3DPT_TRIANGLEFAN, MYD3D_VERTEX_FORMAT, out, count, 0);
#endif
		RenderDevice->SetRenderState(D3DRENDERSTATE_ALPHABLENDENABLE, FALSE);
	}
}

//==========================================================================
//
//	TDirect3DDrawer::EndSky
//
//==========================================================================

void TDirect3DDrawer::EndSky(void)
{
	if (r_use_fog)
	{
		RenderDevice->SetRenderState(D3DRENDERSTATE_FOGENABLE, TRUE);
	}

#if DIRECT3D_VERSION >= 0x0800
	viewData.MinZ = 0;
#else
	viewData.dvMinZ = 0;
#endif
    RenderDevice->SetViewport(&viewData);
}

//==========================================================================
//
//	TDirect3DDrawer::DrawMaskedPolygon
//
//==========================================================================

void TDirect3DDrawer::DrawMaskedPolygon(TVec *cv, int count,
	int texture, int translucency)
{
	MyD3DVertex	out[256];
	int			i, r, g, b, alpha, w, h, size, l;
	surface_t	*surf = r_surface;

	SetTexture(texture);

	R_BuildLightMap(surf, 0);
	w = (surf->extents[0] >> 4) + 1;
	h = (surf->extents[1] >> 4) + 1;
	size = w * h;
	r = 0;
	g = 0;
	b = 0;
	for (i = 0; i < size; i++)
	{
		r += 255 * 256 - blocklightsr[i];
		g += 255 * 256 - blocklightsg[i];
		b += 255 * 256 - blocklightsb[i];
	}
	r = r / (size * 256);
	g = g / (size * 256);
	b = b / (size * 256);
	alpha = (100 - translucency) * 255 / 100;
	l = (alpha << 24) | (r << 16) | (g << 8) | b;

	for (i = 0; i < count; i++)
	{
		TVec texpt = cv[i] - r_texorg;
		out[i] = MyD3DVertex(cv[i], l,
			DotProduct(texpt, r_saxis) * tex_iw,
			DotProduct(texpt, r_taxis) * tex_ih);
	}

	RenderDevice->SetRenderState(D3DRENDERSTATE_ALPHATESTENABLE, TRUE);
	if (blend_sprites || translucency)
	{
		RenderDevice->SetRenderState(D3DRENDERSTATE_ALPHABLENDENABLE, TRUE);
		RenderDevice->SetRenderState(D3DRENDERSTATE_ALPHAREF, 0);
	}

#if DIRECT3D_VERSION >= 0x0800
	RenderDevice->DrawPrimitiveUP(D3DPT_TRIANGLEFAN, count - 2, out, sizeof(MyD3DVertex));
#else
	RenderDevice->DrawPrimitive(D3DPT_TRIANGLEFAN, MYD3D_VERTEX_FORMAT, out, count, 0);
#endif

	RenderDevice->SetRenderState(D3DRENDERSTATE_ALPHATESTENABLE, FALSE);
	if (blend_sprites || translucency)
	{
		RenderDevice->SetRenderState(D3DRENDERSTATE_ALPHABLENDENABLE, FALSE);
		RenderDevice->SetRenderState(D3DRENDERSTATE_ALPHAREF, 170);
	}
}

//==========================================================================
//
//	TDirect3DDrawer::DrawSpritePolygon
//
//==========================================================================

void TDirect3DDrawer::DrawSpritePolygon(TVec *cv, int lump,
	int translucency, int translation, dword light)
{
	MyD3DVertex		out[4];
	int				i;

	SetSpriteLump(lump, translation);

	int l = (((100 - translucency) * 255 / 100) << 24) | (light & 0x00ffffff);
	for (i = 0; i < 4; i++)
	{
		TVec texpt = cv[i] - r_texorg;
		out[i] = MyD3DVertex(cv[i], l,
			DotProduct(texpt, r_saxis) * tex_iw,
			DotProduct(texpt, r_taxis) * tex_ih);
	}

	if (blend_sprites || translucency)
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

	if (blend_sprites || translucency)
	{
		RenderDevice->SetRenderState(D3DRENDERSTATE_ALPHABLENDENABLE, FALSE);
		RenderDevice->SetRenderState(D3DRENDERSTATE_ALPHAREF, 170);
	}
	RenderDevice->SetRenderState(D3DRENDERSTATE_ALPHATESTENABLE, FALSE);
}

//==========================================================================
//
//	TDirect3DDrawer::DrawAliasModel
//
//==========================================================================

void TDirect3DDrawer::DrawAliasModel(const TVec &origin, const TAVec &angles,
	model_t *model, int frame, const char *skin, dword light, int translucency,
	bool is_view_model)
{
	mmdl_t				*pmdl;
	mframe_t			*pframedesc;
	mskin_t				*pskindesc;
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
	dword				alpha;

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
	shadedots = r_avertexnormal_dots[((int)(BAM2DEG(angles.yaw) * (SHADEDOT_QUANT / 360.0))) & (SHADEDOT_QUANT - 1)];
	alpha = ((100 - translucency) * 255 / 100) << 24;
	
	//
	// locate the proper data
	//
	pmdl = (mmdl_t *)Mod_Extradata(model);

	// Hack to make sure that skin loading doesn't free model
	Z_ChangeTag(pmdl, PU_STATIC);

	//
	// draw all the triangles
	//

	if ((frame >= pmdl->numframes) || (frame < 0))
	{
		cond << "no such frame " << frame << endl;
		frame = 0;
	}
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
	matTmp(0, 0) = pframedesc->scale[0];
	matTmp(1, 1) = pframedesc->scale[1];
	matTmp(2, 2) = pframedesc->scale[2];
	matTmp(3, 0) = pframedesc->scale_origin[0];
	matTmp(3, 1) = pframedesc->scale_origin[1];
	matTmp(3, 2) = pframedesc->scale_origin[2];

	matWorld = matTmp * matWorld;
	RenderDevice->SetTransform(D3DTRANSFORMSTATE_WORLD, &matWorld);

	if (skin && *skin)
	{
		SetSkin(skin);
	}
	else
	{
		pskindesc = (mskin_t *)((byte *)pmdl + pmdl->ofsskins);
		SetSkin(pskindesc->name);
	}

	RenderDevice->SetRenderState(D3DRENDERSTATE_SHADEMODE, D3DSHADE_GOURAUD);
	if (translucency)
	{
		RenderDevice->SetRenderState(D3DRENDERSTATE_ALPHABLENDENABLE, TRUE);
	}

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
			l = shadedots[verts[index].lightnormalindex];
			out[i].color = alpha |
				(((int)(l * shadelightr * 0xff) << 16) & 0x00ff0000) |
				(((int)(l * shadelightg * 0xff) <<  8) & 0x0000ff00) |
				(((int)(l * shadelightb * 0xff)      ) & 0x000000ff);

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
	if (translucency)
	{
		RenderDevice->SetRenderState(D3DRENDERSTATE_ALPHABLENDENABLE, FALSE);
	}

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

	// Make it cachable again
	Z_ChangeTag(pmdl, PU_CACHE);
}

//==========================================================================
//
//	TDirect3DDrawer::StartParticles
//
//==========================================================================

void TDirect3DDrawer::StartParticles(void)
{
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
}

//==========================================================================
//
//	TDirect3DDrawer::DrawParticle
//
//==========================================================================

void TDirect3DDrawer::DrawParticle(particle_t *p)
{
	MyD3DVertex out[4];
	out[0] = MyD3DVertex(p->org - viewright + viewup, p->color, 0, 0);
	out[1] = MyD3DVertex(p->org + viewright + viewup, p->color, 1, 0);
	out[2] = MyD3DVertex(p->org + viewright - viewup, p->color, 1, 1);
	out[3] = MyD3DVertex(p->org - viewright - viewup, p->color, 0, 1);
#if DIRECT3D_VERSION >= 0x0800
	RenderDevice->DrawPrimitiveUP(D3DPT_TRIANGLEFAN, 2, out, sizeof(MyD3DVertex));
#else
	RenderDevice->DrawPrimitive(D3DPT_TRIANGLEFAN, MYD3D_VERTEX_FORMAT, out, 4, 0);
#endif
}

//==========================================================================
//
//	TDirect3DDrawer::EndParticles
//
//==========================================================================

void TDirect3DDrawer::EndParticles(void)
{
	RenderDevice->SetRenderState(D3DRENDERSTATE_ALPHABLENDENABLE, FALSE);
	RenderDevice->SetRenderState(D3DRENDERSTATE_ALPHATESTENABLE, FALSE);
	RenderDevice->SetRenderState(D3DRENDERSTATE_ALPHAREF, 170);
}

//**************************************************************************
//
//	$Log$
//	Revision 1.14  2001/10/12 17:28:26  dj_jl
//	Blending of sprite borders
//
//	Revision 1.13  2001/10/09 17:21:39  dj_jl
//	Added sky begining and ending functions
//	
//	Revision 1.12  2001/10/04 17:22:05  dj_jl
//	My overloaded matrix, beautification
//	
//	Revision 1.11  2001/09/14 16:48:22  dj_jl
//	Switched to DirectX 8
//	
//	Revision 1.10  2001/09/05 12:21:42  dj_jl
//	Release changes
//	
//	Revision 1.9  2001/08/29 17:47:55  dj_jl
//	Added texture filtering variables
//	
//	Revision 1.8  2001/08/24 17:03:57  dj_jl
//	Added mipmapping, removed bumpmap test code
//	
//	Revision 1.7  2001/08/21 17:46:08  dj_jl
//	Added R_TextureAnimation, made SetTexture recognize flats
//	
//	Revision 1.6  2001/08/07 16:46:23  dj_jl
//	Added player models, skins and weapon
//	
//	Revision 1.5  2001/08/04 17:29:11  dj_jl
//	Added depth hack for weapon models
//	
//	Revision 1.4  2001/08/01 17:36:11  dj_jl
//	Added alpha test to the particle drawing
//	
//	Revision 1.3  2001/07/31 17:16:30  dj_jl
//	Just moved Log to the end of file
//	
//	Revision 1.2  2001/07/27 14:27:54  dj_jl
//	Update with Id-s and Log-s, some fixes
//
//**************************************************************************
