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

#include "gamedefs.h"
#include "r_local.h"
#include "r_hardware.h"

// MACROS ------------------------------------------------------------------

// TYPES -------------------------------------------------------------------

// EXTERNAL FUNCTION PROTOTYPES --------------------------------------------

// PUBLIC FUNCTION PROTOTYPES ----------------------------------------------

// PRIVATE FUNCTION PROTOTYPES ---------------------------------------------

// EXTERNAL DATA DECLARATIONS ----------------------------------------------

// PUBLIC DATA DEFINITIONS -------------------------------------------------

// precalculated dot products for quantized angles
float			VHardwareDrawer::r_avertexnormal_dots[
	VHardwareDrawer::SHADEDOT_QUANT][256] =
#include "anorm_dots.h"
;

// PRIVATE DATA DEFINITIONS ------------------------------------------------

// CODE --------------------------------------------------------------------

//==========================================================================
//
//	VHardwareDrawer::VHardwareDrawer
//
//==========================================================================

VHardwareDrawer::VHardwareDrawer()
: RendLev(NULL)
, SimpleSurfsHead(NULL)
, SimpleSurfsTail(NULL)
, SkyPortalsHead(NULL)
, SkyPortalsTail(NULL)
, PortalDepth(0)
{
}

//==========================================================================
//
//	VRenderLevelShared::FlushCaches
//
//==========================================================================

void VRenderLevelShared::FlushCaches()
{
	guard(VRenderLevelShared::FlushCaches);
	memset(blockbuf, 0, sizeof(blockbuf));
	freeblocks = NULL;
	for (int i = 0; i < NUM_CACHE_BLOCKS; i++)
	{
		blockbuf[i].chain = freeblocks;
		freeblocks = &blockbuf[i];
	}
	for (int i = 0; i < NUM_BLOCK_SURFS; i++)
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
//	VRenderLevelShared::FlushOldCaches
//
//==========================================================================

void VRenderLevelShared::FlushOldCaches()
{
	guard(VRenderLevelShared::FlushOldCaches);
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
//	VRenderLevelShared::AllocBlock
//
//==========================================================================

surfcache_t* VRenderLevelShared::AllocBlock(int width, int height)
{
	guard(VRenderLevelShared::AllocBlock);
	int				i;
	surfcache_t*	blines;
	surfcache_t*	block;
	surfcache_t*	other;

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
				{
					continue;
				}
				if (block->width < width)
				{
					continue;
				}
				if (block->width > width)
				{
					if (!freeblocks)
					{
						FlushOldCaches();
					}
					other = freeblocks;
					freeblocks = other->chain;
					other->s = block->s + width;
					other->t = block->t;
					other->width = block->width - width;
					other->height = block->height;
					other->lnext = block->lnext;
					if (other->lnext)
					{
						other->lnext->lprev = other;
					}
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
				{
					FlushOldCaches();
				}
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
				{
					other->bnext->bprev = other;
				}
				block->bnext = other;
				other->bprev = block;
				block->height = height;
				other->owner = NULL;
				other->blocknum = i;
			}

			if (!freeblocks)
			{
				FlushOldCaches();
			}
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
//	VRenderLevelShared::FreeBlock
//
//==========================================================================

surfcache_t* VRenderLevelShared::FreeBlock(surfcache_t *block, bool check_lines)
{
	guard(VHardwareDrawer::FreeBlock);
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
		{
			block->lnext->lprev = block;
		}
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
		{
			block->lnext->lprev = block;
		}
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
		{
			block->bnext->bprev = block;
		}
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
		{
			block->bnext->bprev = block;
		}
		other->chain = freeblocks;
		freeblocks = other;
	}
	return block;
	unguard;
}

//==========================================================================
//
//	VRenderLevelShared::FreeSurfCache
//
//==========================================================================

void VRenderLevelShared::FreeSurfCache(surfcache_t *block)
{
	guard(VRenderLevelShared::FreeSurfCache);
	FreeBlock(block, true);
	unguard;
}

//==========================================================================
//
//	VRenderLevelShared::CacheSurface
//
//==========================================================================

void VRenderLevelShared::CacheSurface(surface_t *surface)
{
	guard(VRenderLevelShared::CacheSurface);
	surfcache_t     *cache;
	int				smax, tmax;
	int				i, j, bnum;

	//
	// see if the cache holds apropriate data
	//
	cache = surface->CacheSurf;

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
		surface->CacheSurf = cache;
		cache->owner = &surface->CacheSurf;
		cache->surf = surface;
	}
	
	if (surface->dlightframe == r_dlightframecount)
		cache->dlight = 1;
	else
		cache->dlight = 0;
	cache->Light = surface->Light;

	// calculate the lightings
	BuildLightMap(surface, 0);
	bnum = cache->blocknum;
	block_changed[bnum] = true;

	for (j = 0; j < tmax; j++)
	{
		for (i = 0; i < smax; i++)
		{
			rgba_t &lb = light_block[bnum][(j + cache->t) * BLOCK_WIDTH +
				i + cache->s];
			lb.r = 255 - byte(blocklightsr[j * smax + i] >> 8);
			lb.g = 255 - byte(blocklightsg[j * smax + i] >> 8);
			lb.b = 255 - byte(blocklightsb[j * smax + i] >> 8);
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
