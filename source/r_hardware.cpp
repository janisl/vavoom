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
float			VHardwareDrawer::r_avertexnormals[
	VHardwareDrawer::NUMVERTEXNORMALS][3] =
{
#include "anorms.h"
};

// PRIVATE DATA DEFINITIONS ------------------------------------------------

// CODE --------------------------------------------------------------------

//==========================================================================
//
//	VHardwareDrawer::InitData
//
//==========================================================================

void VHardwareDrawer::InitData()
{
}

//==========================================================================
//
//	VHardwareDrawer::VHardwareDrawer
//
//==========================================================================

VHardwareDrawer::VHardwareDrawer()
: RendLev(NULL)
, freeblocks(NULL)
, cacheframecount(0)
, SimpleSurfsHead(NULL)
, SimpleSurfsTail(NULL)
, SkyPortalsHead(NULL)
, SkyPortalsTail(NULL)
, PortalDepth(0)
{
	memset(light_block, 0, sizeof(light_block));
	memset(block_changed, 0, sizeof(block_changed));
	memset(light_chain, 0, sizeof(light_chain));
	memset(add_block, 0, sizeof(add_block));
	memset(add_changed, 0, sizeof(add_changed));
	memset(add_chain, 0, sizeof(add_chain));
	memset(cacheblocks, 0, sizeof(cacheblocks));
	memset(blockbuf, 0, sizeof(blockbuf));
}

//==========================================================================
//
//	VHardwareDrawer::FlushCaches
//
//==========================================================================

void VHardwareDrawer::FlushCaches(bool free_blocks)
{
	guard(VHardwareDrawer::FlushCaches);
	int				i;
	surfcache_t*	blines;
	surfcache_t*	block;

	if (free_blocks)
	{
		for (i = 0; i < NUM_BLOCK_SURFS; i++)
		{
			for (blines = cacheblocks[i]; blines; blines = blines->bnext)
			{
				for (block = blines; block; block = block->lnext)
				{
					if (block->owner)
					{
						*block->owner = NULL;
					}
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
//	VHardwareDrawer::FlushOldCaches
//
//==========================================================================

void VHardwareDrawer::FlushOldCaches()
{
	guard(VHardwareDrawer::FlushOldCaches);
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
//	VHardwareDrawer::AllocBlock
//
//==========================================================================

surfcache_t* VHardwareDrawer::AllocBlock(int width, int height)
{
	guard(VHardwareDrawer::AllocBlock);
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
//	VHardwareDrawer::FreeBlock
//
//==========================================================================

surfcache_t* VHardwareDrawer::FreeBlock(surfcache_t *block, bool check_lines)
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
//	VHardwareDrawer::FreeSurfCache
//
//==========================================================================

void VHardwareDrawer::FreeSurfCache(surfcache_t *block)
{
	guard(VHardwareDrawer::FreeSurfCache);
	FreeBlock(block, true);
	unguard;
}

//==========================================================================
//
//	VHardwareDrawer::CacheSurface
//
//==========================================================================

void VHardwareDrawer::CacheSurface(surface_t *surface)
{
	guard(VHardwareDrawer::CacheSurface);
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

//==========================================================================
//
//	VHardwareDrawer::NewMap
//
//==========================================================================

void VHardwareDrawer::NewMap()
{
	guard(VHardwareDrawer::NewMap);
	FlushCaches(false);
	unguard;
}

//==========================================================================
//
//	VHardwareDrawer::ToPowerOf2
//
//==========================================================================

int VHardwareDrawer::ToPowerOf2(int val)
{
	int answer = 1;
	while (answer < val)
	{
		answer <<= 1;
	}
	return answer;
}

//==========================================================================
//
//	VHardwareDrawer::AdjustGamma
//
//==========================================================================

void VHardwareDrawer::AdjustGamma(rgba_t* data, int size)
{
	guard(VHardwareDrawer::AdjustGamma);
	vuint8* gt = gammatable[usegamma];
	for (int i = 0; i < size; i++)
	{
		data[i].r = gt[data[i].r];
		data[i].g = gt[data[i].g];
		data[i].b = gt[data[i].b];
	}
	unguard;
}

//==========================================================================
//
//	VHardwareDrawer::ResampleTexture
//
//	Resizes	texture.
//	This is a simplified version of gluScaleImage from sources of MESA 3.0
//
//==========================================================================

void VHardwareDrawer::ResampleTexture(int widthin, int heightin,
	const vuint8* datain, int widthout, int heightout, vuint8* dataout)
{
	guard(VHardwareDrawer::ResampleTexture);
	int i, j, k;
	float sx, sy;

	if (widthout > 1)
	{
		sx = float(widthin - 1) / float(widthout - 1);
	}
	else
	{
		sx = float(widthin - 1);
	}
	if (heightout > 1)
	{
		sy = float(heightin - 1) / float(heightout - 1);
	}
	else
	{
		sy = float(heightin - 1);
	}

//#define POINT_SAMPLE
#ifdef POINT_SAMPLE
	for (i = 0; i < heightout; i++)
	{
		int ii = int(i * sy);
		for (j = 0; j < widthout; j++)
		{
			int jj = int(j * sx);

			const vuint8* src = datain + (ii * widthin + jj) * 4;
			vuint8* dst = dataout + (i * widthout + j) * 4;

			for (k = 0; k < 4; k++)
			{
				*dst++ = *src++;
			}
		}
	}
#else
	if (sx <= 1.0 && sy <= 1.0)
	{
		/* magnify both width and height:  use weighted sample of 4 pixels */
		int i0, i1, j0, j1;
		float alpha, beta;
		const vuint8 *src00, *src01, *src10, *src11;
		float s1, s2;
		vuint8* dst;

		for (i = 0; i < heightout; i++)
		{
			i0 = int(i * sy);
			i1 = i0 + 1;
			if (i1 >= heightin) i1 = heightin-1;
			alpha = i * sy - i0;
			for (j = 0; j < widthout; j++)
			{
				j0 = int(j * sx);
				j1 = j0 + 1;
				if (j1 >= widthin) j1 = widthin-1;
				beta = j * sx - j0;

				/* compute weighted average of pixels in rect (i0,j0)-(i1,j1) */
				src00 = datain + (i0 * widthin + j0) * 4;
				src01 = datain + (i0 * widthin + j1) * 4;
				src10 = datain + (i1 * widthin + j0) * 4;
				src11 = datain + (i1 * widthin + j1) * 4;

				dst = dataout + (i * widthout + j) * 4;

				for (k = 0; k < 4; k++)
				{
					s1 = *src00++ * (1.0-beta) + *src01++ * beta;
					s2 = *src10++ * (1.0-beta) + *src11++ * beta;
					*dst++ = vuint8(s1 * (1.0-alpha) + s2 * alpha);
				}
			}
		}
	}
	else
	{
		/* shrink width and/or height:  use an unweighted box filter */
		int i0, i1;
		int j0, j1;
		int ii, jj;
		int sum;
		vuint8* dst;

		for (i = 0; i < heightout; i++)
		{
			i0 = int(i * sy);
			i1 = i0 + 1;
			if (i1 >= heightin) i1 = heightin-1;
			for (j = 0; j < widthout; j++)
			{
				j0 = int(j * sx);
				j1 = j0 + 1;
				if (j1 >= widthin) j1 = widthin-1;

				dst = dataout + (i * widthout + j) * 4;

				/* compute average of pixels in the rectangle (i0,j0)-(i1,j1) */
				for (k = 0; k < 4; k++)
				{
					sum = 0;
					for (ii = i0; ii <= i1; ii++)
					{
						for (jj = j0; jj <= j1; jj++)
						{
							sum += *(datain + (ii * widthin + jj) * 4 + k);
						}
					}
					sum /= (j1 - j0 + 1) * (i1 - i0 + 1);
					*dst++ = vuint8(sum);
				}
			}
		}
	}
#endif
	unguard;
}

//==========================================================================
//
//	VHardwareDrawer::MipMap
//
//	Scales image down for next mipmap level, operates in place
//
//==========================================================================

void VHardwareDrawer::MipMap(int width, int height, vuint8* InIn)
{
	guard(VHardwareDrawer::MipMap);
	vuint8* in = InIn;
	int		i, j;
	vuint8* out = in;

	if (width == 1 || height == 1)
	{
		//	Special case when only one dimension is scaled
		int total = width * height / 2;
		for (i = 0; i < total; i++, in += 8, out += 4)
		{
			out[0] = vuint8((in[0] + in[4]) >> 1);
			out[1] = vuint8((in[1] + in[5]) >> 1);
			out[2] = vuint8((in[2] + in[6]) >> 1);
			out[3] = vuint8((in[3] + in[7]) >> 1);
		}
		return;
	}

	//	Scale down in both dimensions
	width <<= 2;
	height >>= 1;
	for (i = 0; i < height; i++, in += width)
	{
		for (j = 0; j < width; j += 8, in += 8, out += 4)
		{
			out[0] = vuint8((in[0] + in[4] + in[width + 0] + in[width + 4]) >> 2);
			out[1] = vuint8((in[1] + in[5] + in[width + 1] + in[width + 5]) >> 2);
			out[2] = vuint8((in[2] + in[6] + in[width + 2] + in[width + 6]) >> 2);
			out[3] = vuint8((in[3] + in[7] + in[width + 3] + in[width + 7]) >> 2);
		}
	}
	unguard;
}
