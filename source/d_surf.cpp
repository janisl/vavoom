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
//**
//**	surface-related refresh code
//**
//**************************************************************************

// HEADER FILES ------------------------------------------------------------

#include "d_local.h"

// MACROS ------------------------------------------------------------------

// TYPES -------------------------------------------------------------------

// EXTERNAL FUNCTION PROTOTYPES --------------------------------------------

// PUBLIC FUNCTION PROTOTYPES ----------------------------------------------

// PRIVATE FUNCTION PROTOTYPES ---------------------------------------------

extern "C" {
void D_DrawSurfaceBlock8_mip0(void);
void D_DrawSurfaceBlock8_mip1(void);
void D_DrawSurfaceBlock8_mip2(void);
void D_DrawSurfaceBlock8_mip3(void);
void D_DrawSurfaceBlock16_mip0(void);
void D_DrawSurfaceBlock16_mip1(void);
void D_DrawSurfaceBlock16_mip2(void);
void D_DrawSurfaceBlock16_mip3(void);
void D_DrawSurfaceBlock32_mip0(void);
void D_DrawSurfaceBlock32_mip1(void);
void D_DrawSurfaceBlock32_mip2(void);
void D_DrawSurfaceBlock32_mip3(void);
void D_DrawSurfaceBlock8RGB_mip0(void);
void D_DrawSurfaceBlock8RGB_mip1(void);
void D_DrawSurfaceBlock8RGB_mip2(void);
void D_DrawSurfaceBlock8RGB_mip3(void);
void D_DrawSurfaceBlock16RGB_mip0(void);
void D_DrawSurfaceBlock16RGB_mip1(void);
void D_DrawSurfaceBlock16RGB_mip2(void);
void D_DrawSurfaceBlock16RGB_mip3(void);
void D_DrawSurfaceBlock32RGB_mip0(void);
void D_DrawSurfaceBlock32RGB_mip1(void);
void D_DrawSurfaceBlock32RGB_mip2(void);
void D_DrawSurfaceBlock32RGB_mip3(void);

void D_DrawSkySurf_8(void);
void D_DrawSkySurf_16(void);
void D_DrawSkySurf_32(void);
void D_DrawDoubleSkySurf_8(void);
void D_DrawDoubleSkySurf_16(void);
void D_DrawDoubleSkySurf_32(void);
}

// EXTERNAL DATA DECLARATIONS ----------------------------------------------

// PUBLIC DATA DEFINITIONS -------------------------------------------------

// PRIVATE DATA DEFINITIONS ------------------------------------------------

extern "C" {
int				blocksize, sourcetstep, blockdivshift;
int				lightdelta, lightdeltastep;
int				lightleft,  lightright,  lightleftstep,  lightrightstep;
int				lightrleft, lightrright, lightrleftstep, lightrrightstep;
int				lightgleft, lightgright, lightgleftstep, lightgrightstep;
int				lightbleft, lightbright, lightbleftstep, lightbrightstep;
dword			blockdivmask;
void			*prowdestbase;
byte			*pbasesource;
int				surfrowbytes;	// used by ASM files
dword			*r_lightptr;
dword			*r_lightptrr;
dword			*r_lightptrg;
dword			*r_lightptrb;
int				r_stepback;
int				r_lightwidth;
int				r_numhblocks, r_numvblocks;
byte			*r_source, *r_sourcemax;
}

miptexture_t	*dsky_mt1;
miptexture_t	*dsky_mt2;
int				dsky_offs1;
int				dsky_offs2;
int				dsky_cachewidth;
int				dsky_cacheheight;
byte			*dsky_cachedest;

static void	(*surfmiptable8[4])(void) =
{
	D_DrawSurfaceBlock8_mip0,
	D_DrawSurfaceBlock8_mip1,
	D_DrawSurfaceBlock8_mip2,
	D_DrawSurfaceBlock8_mip3
};
static void	(*surfmiptable16[4])(void) =
{
	D_DrawSurfaceBlock16_mip0,
	D_DrawSurfaceBlock16_mip1,
	D_DrawSurfaceBlock16_mip2,
	D_DrawSurfaceBlock16_mip3
};
static void	(*surfmiptable32[4])(void) =
{
	D_DrawSurfaceBlock32_mip0,
	D_DrawSurfaceBlock32_mip1,
	D_DrawSurfaceBlock32_mip2,
	D_DrawSurfaceBlock32_mip3
};
static void	(*surfmiptable8RGB[4])(void) =
{
	D_DrawSurfaceBlock8RGB_mip0,
	D_DrawSurfaceBlock8RGB_mip1,
	D_DrawSurfaceBlock8RGB_mip2,
	D_DrawSurfaceBlock8RGB_mip3
};
static void	(*surfmiptable16RGB[4])(void) =
{
	D_DrawSurfaceBlock16RGB_mip0,
	D_DrawSurfaceBlock16RGB_mip1,
	D_DrawSurfaceBlock16RGB_mip2,
	D_DrawSurfaceBlock16RGB_mip3
};
static void	(*surfmiptable32RGB[4])(void) =
{
	D_DrawSurfaceBlock32RGB_mip0,
	D_DrawSurfaceBlock32RGB_mip1,
	D_DrawSurfaceBlock32RGB_mip2,
	D_DrawSurfaceBlock32RGB_mip3
};

int					c_surf;

static TCvarI d_colored_lights("d_colored_lights", "1", CVAR_ARCHIVE);

// CODE --------------------------------------------------------------------

//==========================================================================
//
//	D_CacheSurface
//
//==========================================================================

surfcache_t *D_CacheSurface(surface_t *surface, int miplevel)
{
	surfcache_t     *cache;
	float           surfscale;
	int				texture;

	//
	// if the surface is animating or flashing, flush the cache
	//
	texture = R_TextureAnimation(surface->texinfo->pic);
	
	//
	// see if the cache holds apropriate data
	//
	cache = surface->cachespots[miplevel];

	if (cache && ((!cache->dlight && surface->dlightframe != r_dlightframecount)
			|| cache->dlight == r_dlightframecount)
			&& cache->texture == texture
			&& cache->lightlevel == (int)surface->lightlevel)
		return cache;

	//
	// determine shape of surface
	//
	surfscale = 1.0 / (1 << miplevel);
	int surfwidth = surface->extents[0] >> miplevel;
	int surfheight = surface->extents[1] >> miplevel;
	
	//
	// allocate memory if needed
	//
	if (!cache)     // if a texture just animated, don't reallocate it
	{
		cache = D_SCAlloc(surfwidth, surfheight);
		surface->cachespots[miplevel] = cache;
		cache->owner = &surface->cachespots[miplevel];
		cache->mipscale = surfscale;
	}
	
	if (surface->dlightframe == r_dlightframecount)
		cache->dlight = r_dlightframecount;
	else
		cache->dlight = 0;

	cache->texture = texture;
	cache->lightlevel = (int)surface->lightlevel;

	//
	// draw and light the surface texture
	//

	c_surf++;

	byte			*basetptr;
	int				smax, tmax, twidth;
	int				u;
	int				soffset, basetoffset, texwidth;
	int				horzblockstep;
	byte			*pcolumndest;
	void			(*pblockdrawer)(void);
	miptexture_t	*mt;
	bool			colored;

	// calculate the lightings
	colored = R_BuildLightMap(surface, 3);
	if (!d_colored_lights)
	{
		colored = false;
	}
	
	surfrowbytes = surfwidth * PixelBytes;

	Drawer->SetTexture(texture);
	mt = miptexture;
	
	r_source = (byte *)mt + mt->offsets[miplevel];
	
	// the fractional light values should range from 0 to (VID_GRADES - 1) << 16
	// from a source range of 0 - 255
	
	texwidth = mt->width >> miplevel;

	blocksize = 16 >> miplevel;
	blockdivshift = 4 - miplevel;
	blockdivmask = (1 << blockdivshift) - 1;
	
	r_lightwidth = (surface->extents[0] >> 4) + 1;

	r_numhblocks = surfwidth >> blockdivshift;
	r_numvblocks = surfheight >> blockdivshift;

	//==============================

	if (PixelBytes == 1)
	{
		if (colored)
			pblockdrawer = surfmiptable8RGB[miplevel];
		else
			pblockdrawer = surfmiptable8[miplevel];
		// TODO: only needs to be set when there is a display settings change
		horzblockstep = blocksize;
	}
	else if (PixelBytes == 2)
	{
		if (colored)
			pblockdrawer = surfmiptable16RGB[miplevel];
		else
			pblockdrawer = surfmiptable16[miplevel];
		// TODO: only needs to be set when there is a display settings change
		horzblockstep = blocksize << 1;
	}
	else
	{
		if (colored)
			pblockdrawer = surfmiptable32RGB[miplevel];
		else
			pblockdrawer = surfmiptable32[miplevel];
		// TODO: only needs to be set when there is a display settings change
		horzblockstep = blocksize << 2;
	}

	smax = mt->width >> miplevel;
	twidth = texwidth;
	tmax = mt->height >> miplevel;
	sourcetstep = texwidth;
	r_stepback = tmax * twidth;

	r_sourcemax = r_source + (tmax * smax);

	soffset = surface->texturemins[0];
	basetoffset = surface->texturemins[1];

	// << 16 components are to guarantee positive values for %
	soffset = ((soffset >> miplevel) + (smax << 16)) % smax;
	basetptr = &r_source[((((basetoffset >> miplevel) 
		+ (tmax << 16)) % tmax) * twidth)];

	pcolumndest = (byte*)cache->data;

	for (u = 0; u < r_numhblocks; u++)
	{
		r_lightptr = blocklights + u;
		r_lightptrr = blocklightsr + u;
		r_lightptrg = blocklightsg + u;
		r_lightptrb = blocklightsb + u;

		prowdestbase = pcolumndest;

		pbasesource = basetptr + soffset;

		(*pblockdrawer)();

		soffset = soffset + blocksize;
		if (soffset >= smax)
			soffset = 0;

		pcolumndest += horzblockstep;
	}

	return surface->cachespots[miplevel];
}

//==========================================================================
//
//	D_CacheSkySurface
//
//==========================================================================

surfcache_t *D_CacheSkySurface(surface_t *surface, int texture1,
	int texture2, float offs1, float offs2)
{
	surfcache_t     *cache;

	//
	// if the surface is animating flush the cache
	//
	texture1 = R_TextureAnimation(texture1);
	texture2 = R_TextureAnimation(texture2);
	
	//
	// see if the cache holds apropriate data
	//
	cache = surface->cachespots[0];

	if (cache && cache->texture == texture1 &&
		*(float *)&cache->dlight == offs1 &&
		cache->lightlevel == texture2 && cache->mipscale == offs2)
	{
		return cache;
	}

	//
	// determine shape of surface
	//
	int surfwidth = surface->extents[0];
	int surfheight = surface->extents[1];
	
	//
	// allocate memory if needed
	//
	if (!cache)     // if a texture just animated, don't reallocate it
	{
		cache = D_SCAlloc(surfwidth, surfheight);
		surface->cachespots[0] = cache;
		cache->owner = &surface->cachespots[0];
	}
	
	cache->texture = texture1;
	*(float *)&cache->dlight = offs1;
	cache->lightlevel = texture2;
	cache->mipscale = offs2;

	//
	// draw the surface texture
	//

//	c_surf++;

	dsky_cachewidth = surfwidth;
	dsky_cacheheight = surfheight;
	dsky_cachedest = (byte*)cache->data;

	Drawer->SetSkyTexture(texture1, false);
	dsky_mt1 = miptexture;
	dsky_offs1 = (-(int)offs1) & (dsky_mt1->width - 1);

	if (texture2)
	{
		// Make sure that texture will be not freed
		Z_ChangeTag(dsky_mt1, PU_STATIC);

		Drawer->SetSkyTexture(texture2, true);
		dsky_mt2 = miptexture;
		dsky_offs2 = (-(int)offs2) & (dsky_mt2->width - 1);

		if (PixelBytes == 1)
		{
			D_DrawDoubleSkySurf_8();
		}
		else if (PixelBytes == 2)
		{
			D_DrawDoubleSkySurf_16();
		}
		else
		{
			D_DrawDoubleSkySurf_32();
		}
	}
	else
	{
		if (PixelBytes == 1)
		{
			D_DrawSkySurf_8();
		}
		else if (PixelBytes == 2)
		{
			D_DrawSkySurf_16();
		}
		else
		{
			D_DrawSkySurf_32();
		}
	}

	return surface->cachespots[0];
}

#ifndef USEASM

//==========================================================================
//
//	D_DrawSurfaceBlock8_mip0
//
//==========================================================================

void D_DrawSurfaceBlock8_mip0(void)
{
	int			v, i, b, lightstep, lighttemp, light;
	byte		pix, *psource, *prowdest;

	psource = pbasesource;
	prowdest = (byte*)prowdestbase;

	for (v = 0; v < r_numvblocks; v++)
	{
		// FIXME: make these locals?
		// FIXME: use delta rather than both right and left, like ASM?
		lightleft = r_lightptr[0];
		lightright = r_lightptr[1];
		r_lightptr += r_lightwidth;
		lightleftstep = (r_lightptr[0] - lightleft) >> 4;
		lightrightstep = (r_lightptr[1] - lightright) >> 4;

		for (i = 0; i < 16; i++)
		{
			lighttemp = lightleft - lightright;
			lightstep = lighttemp >> 4;

			light = lightright;

			for (b = 15; b >= 0; b--)
			{
				pix = psource[b];
				prowdest[b] = fadetable[(light & 0xFF00) + pix];
				light += lightstep;
			}
	
			psource += sourcetstep;
			lightright += lightrightstep;
			lightleft += lightleftstep;
			prowdest += surfrowbytes;
		}

		if (psource >= r_sourcemax)
			psource -= r_stepback;
	}
}

//==========================================================================
//
//	D_DrawSurfaceBlock8_mip1
//
//==========================================================================

void D_DrawSurfaceBlock8_mip1(void)
{
	int			v, i, b, lightstep, lighttemp, light;
	byte		pix, *psource, *prowdest;

	psource = pbasesource;
	prowdest = (byte*)prowdestbase;

	for (v = 0; v < r_numvblocks; v++)
	{
		// FIXME: make these locals?
		// FIXME: use delta rather than both right and left, like ASM?
		lightleft = r_lightptr[0];
		lightright = r_lightptr[1];
		r_lightptr += r_lightwidth;
		lightleftstep = (r_lightptr[0] - lightleft) >> 3;
		lightrightstep = (r_lightptr[1] - lightright) >> 3;

		for (i = 0; i < 8; i++)
		{
			lighttemp = lightleft - lightright;
			lightstep = lighttemp >> 3;

			light = lightright;

			for (b = 7; b >= 0; b--)
			{
				pix = psource[b];
				prowdest[b] = fadetable[(light & 0xFF00) + pix];
				light += lightstep;
			}
	
			psource += sourcetstep;
			lightright += lightrightstep;
			lightleft += lightleftstep;
			prowdest += surfrowbytes;
		}

		if (psource >= r_sourcemax)
			psource -= r_stepback;
	}
}

//==========================================================================
//
//	D_DrawSurfaceBlock8_mip2
//
//==========================================================================

void D_DrawSurfaceBlock8_mip2(void)
{
	int			v, i, b, lightstep, lighttemp, light;
	byte		pix, *psource, *prowdest;

	psource = pbasesource;
	prowdest = (byte*)prowdestbase;

	for (v = 0; v < r_numvblocks; v++)
	{
		// FIXME: make these locals?
		// FIXME: use delta rather than both right and left, like ASM?
		lightleft = r_lightptr[0];
		lightright = r_lightptr[1];
		r_lightptr += r_lightwidth;
		lightleftstep = (r_lightptr[0] - lightleft) >> 2;
		lightrightstep = (r_lightptr[1] - lightright) >> 2;

		for (i = 0; i < 4; i++)
		{
			lighttemp = lightleft - lightright;
			lightstep = lighttemp >> 2;

			light = lightright;

			for (b = 3; b >= 0; b--)
			{
				pix = psource[b];
				prowdest[b] = fadetable[(light & 0xFF00) + pix];
				light += lightstep;
			}
	
			psource += sourcetstep;
			lightright += lightrightstep;
			lightleft += lightleftstep;
			prowdest += surfrowbytes;
		}

		if (psource >= r_sourcemax)
			psource -= r_stepback;
	}
}

//==========================================================================
//
//	D_DrawSurfaceBlock8_mip3
//
//==========================================================================

void D_DrawSurfaceBlock8_mip3(void)
{
	int			v, i, b, lightstep, lighttemp, light;
	byte		pix, *psource, *prowdest;

	psource = pbasesource;
	prowdest = (byte*)prowdestbase;

	for (v = 0; v < r_numvblocks; v++)
	{
		// FIXME: make these locals?
		// FIXME: use delta rather than both right and left, like ASM?
		lightleft = r_lightptr[0];
		lightright = r_lightptr[1];
		r_lightptr += r_lightwidth;
		lightleftstep = (r_lightptr[0] - lightleft) >> 1;
		lightrightstep = (r_lightptr[1] - lightright) >> 1;

		for (i = 0; i < 2; i++)
		{
			lighttemp = lightleft - lightright;
			lightstep = lighttemp >> 1;

			light = lightright;

			for (b = 1; b >= 0; b--)
			{
				pix = psource[b];
				prowdest[b] = fadetable[(light & 0xFF00) + pix];
				light += lightstep;
			}
	
			psource += sourcetstep;
			lightright += lightrightstep;
			lightleft += lightleftstep;
			prowdest += surfrowbytes;
		}

		if (psource >= r_sourcemax)
			psource -= r_stepback;
	}
}

//==========================================================================
//
//	D_DrawSurfaceBlock16_mip0
//
//==========================================================================

void D_DrawSurfaceBlock16_mip0(void)
{
	int			v, i, b, lightstep, lighttemp, light;
	byte		pix, *psource;
	word		*prowdest;

	psource = pbasesource;
	prowdest = (word*)prowdestbase;

	for (v = 0; v < r_numvblocks; v++)
	{
		// FIXME: make these locals?
		// FIXME: use delta rather than both right and left, like ASM?
		lightleft = r_lightptr[0];
		lightright = r_lightptr[1];
		r_lightptr += r_lightwidth;
		lightleftstep = (r_lightptr[0] - lightleft) >> 4;
		lightrightstep = (r_lightptr[1] - lightright) >> 4;

		for (i = 0; i < 16; i++)
		{
			lighttemp = lightleft - lightright;
			lightstep = lighttemp >> 4;

			light = lightright;

			for (b = 15; b >= 0; b--)
			{
				pix = psource[b];
				prowdest[b] = fadetable16[(light & 0xFF00) + pix];
				light += lightstep;
			}
	
			psource += sourcetstep;
			lightright += lightrightstep;
			lightleft += lightleftstep;
			prowdest = (word*)((byte*)prowdest + surfrowbytes);
		}

		if (psource >= r_sourcemax)
			psource -= r_stepback;
	}
}

//==========================================================================
//
//	D_DrawSurfaceBlock16_mip1
//
//==========================================================================

void D_DrawSurfaceBlock16_mip1(void)
{
	int			v, i, b, lightstep, lighttemp, light;
	byte		pix, *psource;
	word		*prowdest;

	psource = pbasesource;
	prowdest = (word*)prowdestbase;

	for (v = 0; v < r_numvblocks; v++)
	{
		// FIXME: make these locals?
		// FIXME: use delta rather than both right and left, like ASM?
		lightleft = r_lightptr[0];
		lightright = r_lightptr[1];
		r_lightptr += r_lightwidth;
		lightleftstep = (r_lightptr[0] - lightleft) >> 3;
		lightrightstep = (r_lightptr[1] - lightright) >> 3;

		for (i = 0; i < 8; i++)
		{
			lighttemp = lightleft - lightright;
			lightstep = lighttemp >> 3;

			light = lightright;

			for (b = 7; b >= 0; b--)
			{
				pix = psource[b];
				prowdest[b] = fadetable16[(light & 0xFF00) + pix];
				light += lightstep;
			}
	
			psource += sourcetstep;
			lightright += lightrightstep;
			lightleft += lightleftstep;
			prowdest = (word*)((byte*)prowdest + surfrowbytes);
		}

		if (psource >= r_sourcemax)
			psource -= r_stepback;
	}
}

//==========================================================================
//
//	D_DrawSurfaceBlock16_mip2
//
//==========================================================================

void D_DrawSurfaceBlock16_mip2(void)
{
	int			v, i, b, lightstep, lighttemp, light;
	byte		pix, *psource;
	word		*prowdest;

	psource = pbasesource;
	prowdest = (word*)prowdestbase;

	for (v = 0; v < r_numvblocks; v++)
	{
		// FIXME: make these locals?
		// FIXME: use delta rather than both right and left, like ASM?
		lightleft = r_lightptr[0];
		lightright = r_lightptr[1];
		r_lightptr += r_lightwidth;
		lightleftstep = (r_lightptr[0] - lightleft) >> 2;
		lightrightstep = (r_lightptr[1] - lightright) >> 2;

		for (i = 0; i < 4; i++)
		{
			lighttemp = lightleft - lightright;
			lightstep = lighttemp >> 2;

			light = lightright;

			for (b = 3; b >= 0; b--)
			{
				pix = psource[b];
				prowdest[b] = fadetable16[(light & 0xFF00) + pix];
				light += lightstep;
			}
	
			psource += sourcetstep;
			lightright += lightrightstep;
			lightleft += lightleftstep;
			prowdest = (word*)((byte*)prowdest + surfrowbytes);
		}

		if (psource >= r_sourcemax)
			psource -= r_stepback;
	}
}

//==========================================================================
//
//	D_DrawSurfaceBlock16_mip3
//
//==========================================================================

void D_DrawSurfaceBlock16_mip3(void)
{
	int			v, i, b, lightstep, lighttemp, light;
	byte		pix, *psource;
	word		*prowdest;

	psource = pbasesource;
	prowdest = (word*)prowdestbase;

	for (v = 0; v < r_numvblocks; v++)
	{
		// FIXME: make these locals?
		// FIXME: use delta rather than both right and left, like ASM?
		lightleft = r_lightptr[0];
		lightright = r_lightptr[1];
		r_lightptr += r_lightwidth;
		lightleftstep = (r_lightptr[0] - lightleft) >> 1;
		lightrightstep = (r_lightptr[1] - lightright) >> 1;

		for (i = 0; i < 2; i++)
		{
			lighttemp = lightleft - lightright;
			lightstep = lighttemp >> 1;

			light = lightright;

			for (b = 1; b >= 0; b--)
			{
				pix = psource[b];
				prowdest[b] = fadetable16[(light & 0xFF00) + pix];
				light += lightstep;
			}
	
			psource += sourcetstep;
			lightright += lightrightstep;
			lightleft += lightleftstep;
			prowdest = (word*)((byte*)prowdest + surfrowbytes);
		}

		if (psource >= r_sourcemax)
			psource -= r_stepback;
	}
}

//==========================================================================
//
//	D_DrawSurfaceBlock32_mip0
//
//==========================================================================

void D_DrawSurfaceBlock32_mip0(void)
{
	int			v, i, b, lightstep, lighttemp, light;
	byte		pix, *psource;
	dword		*prowdest;

	psource = pbasesource;
	prowdest = (dword*)prowdestbase;

	for (v = 0; v < r_numvblocks; v++)
	{
		// FIXME: make these locals?
		// FIXME: use delta rather than both right and left, like ASM?
		lightleft = r_lightptr[0];
		lightright = r_lightptr[1];
		r_lightptr += r_lightwidth;
		lightleftstep = (r_lightptr[0] - lightleft) >> 4;
		lightrightstep = (r_lightptr[1] - lightright) >> 4;

		for (i = 0; i < 16; i++)
		{
			lighttemp = lightleft - lightright;
			lightstep = lighttemp >> 4;

			light = lightright;

			for (b = 15; b >= 0; b--)
			{
				pix = psource[b];
				prowdest[b] = fadetable32[(light & 0xFF00) + pix];
				light += lightstep;
			}
	
			psource += sourcetstep;
			lightright += lightrightstep;
			lightleft += lightleftstep;
			prowdest = (dword*)((byte*)prowdest + surfrowbytes);
		}

		if (psource >= r_sourcemax)
			psource -= r_stepback;
	}
}

//==========================================================================
//
//	D_DrawSurfaceBlock32_mip1
//
//==========================================================================

void D_DrawSurfaceBlock32_mip1(void)
{
	int			v, i, b, lightstep, lighttemp, light;
	byte		pix, *psource;
	dword		*prowdest;

	psource = pbasesource;
	prowdest = (dword*)prowdestbase;

	for (v = 0; v < r_numvblocks; v++)
	{
		// FIXME: make these locals?
		// FIXME: use delta rather than both right and left, like ASM?
		lightleft = r_lightptr[0];
		lightright = r_lightptr[1];
		r_lightptr += r_lightwidth;
		lightleftstep = (r_lightptr[0] - lightleft) >> 3;
		lightrightstep = (r_lightptr[1] - lightright) >> 3;

		for (i = 0; i < 8; i++)
		{
			lighttemp = lightleft - lightright;
			lightstep = lighttemp >> 3;

			light = lightright;

			for (b = 7; b >= 0; b--)
			{
				pix = psource[b];
				prowdest[b] = fadetable32[(light & 0xFF00) + pix];
				light += lightstep;
			}
	
			psource += sourcetstep;
			lightright += lightrightstep;
			lightleft += lightleftstep;
			prowdest = (dword*)((byte*)prowdest + surfrowbytes);
		}

		if (psource >= r_sourcemax)
			psource -= r_stepback;
	}
}

//==========================================================================
//
//	D_DrawSurfaceBlock32_mip2
//
//==========================================================================

void D_DrawSurfaceBlock32_mip2(void)
{
	int			v, i, b, lightstep, lighttemp, light;
	byte		pix, *psource;
	dword		*prowdest;

	psource = pbasesource;
	prowdest = (dword*)prowdestbase;

	for (v = 0; v < r_numvblocks; v++)
	{
		// FIXME: make these locals?
		// FIXME: use delta rather than both right and left, like ASM?
		lightleft = r_lightptr[0];
		lightright = r_lightptr[1];
		r_lightptr += r_lightwidth;
		lightleftstep = (r_lightptr[0] - lightleft) >> 2;
		lightrightstep = (r_lightptr[1] - lightright) >> 2;

		for (i = 0; i < 4; i++)
		{
			lighttemp = lightleft - lightright;
			lightstep = lighttemp >> 2;

			light = lightright;

			for (b = 3; b >= 0; b--)
			{
				pix = psource[b];
				prowdest[b] = fadetable32[(light & 0xFF00) + pix];
				light += lightstep;
			}
	
			psource += sourcetstep;
			lightright += lightrightstep;
			lightleft += lightleftstep;
			prowdest = (dword*)((byte*)prowdest + surfrowbytes);
		}

		if (psource >= r_sourcemax)
			psource -= r_stepback;
	}
}

//==========================================================================
//
//	D_DrawSurfaceBlock32_mip3
//
//==========================================================================

void D_DrawSurfaceBlock32_mip3(void)
{
	int			v, i, b, lightstep, lighttemp, light;
	byte		pix, *psource;
	dword		*prowdest;

	psource = pbasesource;
	prowdest = (dword*)prowdestbase;

	for (v = 0; v < r_numvblocks; v++)
	{
		// FIXME: make these locals?
		// FIXME: use delta rather than both right and left, like ASM?
		lightleft = r_lightptr[0];
		lightright = r_lightptr[1];
		r_lightptr += r_lightwidth;
		lightleftstep = (r_lightptr[0] - lightleft) >> 1;
		lightrightstep = (r_lightptr[1] - lightright) >> 1;

		for (i = 0; i < 2; i++)
		{
			lighttemp = lightleft - lightright;
			lightstep = lighttemp >> 1;

			light = lightright;

			for (b = 1; b >= 0; b--)
			{
				pix = psource[b];
				prowdest[b] = fadetable32[(light & 0xFF00) + pix];
				light += lightstep;
			}
	
			psource += sourcetstep;
			lightright += lightrightstep;
			lightleft += lightleftstep;
			prowdest = (dword*)((byte*)prowdest + surfrowbytes);
		}

		if (psource >= r_sourcemax)
			psource -= r_stepback;
	}
}

//==========================================================================
//
//	D_DrawSurfaceBlock8RGB_mip0
//
//==========================================================================

void D_DrawSurfaceBlock8RGB_mip0(void)
{
	int			v, i, b;
	int			lightrstep, lightrtemp, lightr;
	int			lightgstep, lightgtemp, lightg;
	int			lightbstep, lightbtemp, lightb;
	byte		pix, *psource, *prowdest;

	psource = pbasesource;
	prowdest = (byte*)prowdestbase;

	for (v = 0; v < r_numvblocks; v++)
	{
		// FIXME: make these locals?
		// FIXME: use delta rather than both right and left, like ASM?
		lightrleft = r_lightptrr[0];
		lightrright = r_lightptrr[1];
		r_lightptrr += r_lightwidth;
		lightrleftstep = (r_lightptrr[0] - lightrleft) >> 4;
		lightrrightstep = (r_lightptrr[1] - lightrright) >> 4;

		lightgleft = r_lightptrg[0];
		lightgright = r_lightptrg[1];
		r_lightptrg += r_lightwidth;
		lightgleftstep = (r_lightptrg[0] - lightgleft) >> 4;
		lightgrightstep = (r_lightptrg[1] - lightgright) >> 4;

		lightbleft = r_lightptrb[0];
		lightbright = r_lightptrb[1];
		r_lightptrb += r_lightwidth;
		lightbleftstep = (r_lightptrb[0] - lightbleft) >> 4;
		lightbrightstep = (r_lightptrb[1] - lightbright) >> 4;

		for (i = 0; i < 16; i++)
		{
			lightrtemp = lightrleft - lightrright;
			lightrstep = lightrtemp >> 4;
			lightgtemp = lightgleft - lightgright;
			lightgstep = lightgtemp >> 4;
			lightbtemp = lightbleft - lightbright;
			lightbstep = lightbtemp >> 4;

			lightr = lightrright;
			lightg = lightgright;
			lightb = lightbright;

			for (b = 15; b >= 0; b--)
			{
				pix = psource[b];
				prowdest[b] = d_rgbtable[
					fadetable16r[(lightr & 0xFF00) + pix] |
					fadetable16g[(lightg & 0xFF00) + pix] |
					fadetable16b[(lightb & 0xFF00) + pix]];
				lightr += lightrstep;
				lightg += lightgstep;
				lightb += lightbstep;
			}
	
			psource += sourcetstep;
			lightrright += lightrrightstep;
			lightrleft += lightrleftstep;
			lightgright += lightgrightstep;
			lightgleft += lightgleftstep;
			lightbright += lightbrightstep;
			lightbleft += lightbleftstep;
			prowdest = prowdest + surfrowbytes;
		}

		if (psource >= r_sourcemax)
			psource -= r_stepback;
	}
}

//==========================================================================
//
//	D_DrawSurfaceBlock8RGB_mip1
//
//==========================================================================

void D_DrawSurfaceBlock8RGB_mip1(void)
{
	int			v, i, b;
	int			lightrstep, lightrtemp, lightr;
	int			lightgstep, lightgtemp, lightg;
	int			lightbstep, lightbtemp, lightb;
	byte		pix, *psource, *prowdest;

	psource = pbasesource;
	prowdest = (byte*)prowdestbase;

	for (v = 0; v < r_numvblocks; v++)
	{
		// FIXME: make these locals?
		// FIXME: use delta rather than both right and left, like ASM?
		lightrleft = r_lightptrr[0];
		lightrright = r_lightptrr[1];
		r_lightptrr += r_lightwidth;
		lightrleftstep = (r_lightptrr[0] - lightrleft) >> 3;
		lightrrightstep = (r_lightptrr[1] - lightrright) >> 3;

		lightgleft = r_lightptrg[0];
		lightgright = r_lightptrg[1];
		r_lightptrg += r_lightwidth;
		lightgleftstep = (r_lightptrg[0] - lightgleft) >> 3;
		lightgrightstep = (r_lightptrg[1] - lightgright) >> 3;

		lightbleft = r_lightptrb[0];
		lightbright = r_lightptrb[1];
		r_lightptrb += r_lightwidth;
		lightbleftstep = (r_lightptrb[0] - lightbleft) >> 3;
		lightbrightstep = (r_lightptrb[1] - lightbright) >> 3;

		for (i = 0; i < 8; i++)
		{
			lightrtemp = lightrleft - lightrright;
			lightrstep = lightrtemp >> 3;
			lightgtemp = lightgleft - lightgright;
			lightgstep = lightgtemp >> 3;
			lightbtemp = lightbleft - lightbright;
			lightbstep = lightbtemp >> 3;

			lightr = lightrright;
			lightg = lightgright;
			lightb = lightbright;

			for (b = 7; b >= 0; b--)
			{
				pix = psource[b];
				prowdest[b] = d_rgbtable[
					fadetable16r[(lightr & 0xFF00) + pix] |
					fadetable16g[(lightg & 0xFF00) + pix] |
					fadetable16b[(lightb & 0xFF00) + pix]];
				lightr += lightrstep;
				lightg += lightgstep;
				lightb += lightbstep;
			}
	
			psource += sourcetstep;
			lightrright += lightrrightstep;
			lightrleft += lightrleftstep;
			lightgright += lightgrightstep;
			lightgleft += lightgleftstep;
			lightbright += lightbrightstep;
			lightbleft += lightbleftstep;
			prowdest = prowdest + surfrowbytes;
		}

		if (psource >= r_sourcemax)
			psource -= r_stepback;
	}
}

//==========================================================================
//
//	D_DrawSurfaceBlock8RGB_mip2
//
//==========================================================================

void D_DrawSurfaceBlock8RGB_mip2(void)
{
	int			v, i, b;
	int			lightrstep, lightrtemp, lightr;
	int			lightgstep, lightgtemp, lightg;
	int			lightbstep, lightbtemp, lightb;
	byte		pix, *psource, *prowdest;

	psource = pbasesource;
	prowdest = (byte*)prowdestbase;

	for (v = 0; v < r_numvblocks; v++)
	{
		// FIXME: make these locals?
		// FIXME: use delta rather than both right and left, like ASM?
		lightrleft = r_lightptrr[0];
		lightrright = r_lightptrr[1];
		r_lightptrr += r_lightwidth;
		lightrleftstep = (r_lightptrr[0] - lightrleft) >> 2;
		lightrrightstep = (r_lightptrr[1] - lightrright) >> 2;

		lightgleft = r_lightptrg[0];
		lightgright = r_lightptrg[1];
		r_lightptrg += r_lightwidth;
		lightgleftstep = (r_lightptrg[0] - lightgleft) >> 2;
		lightgrightstep = (r_lightptrg[1] - lightgright) >> 2;

		lightbleft = r_lightptrb[0];
		lightbright = r_lightptrb[1];
		r_lightptrb += r_lightwidth;
		lightbleftstep = (r_lightptrb[0] - lightbleft) >> 2;
		lightbrightstep = (r_lightptrb[1] - lightbright) >> 2;

		for (i = 0; i < 4; i++)
		{
			lightrtemp = lightrleft - lightrright;
			lightrstep = lightrtemp >> 2;
			lightgtemp = lightgleft - lightgright;
			lightgstep = lightgtemp >> 2;
			lightbtemp = lightbleft - lightbright;
			lightbstep = lightbtemp >> 2;

			lightr = lightrright;
			lightg = lightgright;
			lightb = lightbright;

			for (b = 3; b >= 0; b--)
			{
				pix = psource[b];
				prowdest[b] = d_rgbtable[
					fadetable16r[(lightr & 0xFF00) + pix] |
					fadetable16g[(lightg & 0xFF00) + pix] |
					fadetable16b[(lightb & 0xFF00) + pix]];
				lightr += lightrstep;
				lightg += lightgstep;
				lightb += lightbstep;
			}
	
			psource += sourcetstep;
			lightrright += lightrrightstep;
			lightrleft += lightrleftstep;
			lightgright += lightgrightstep;
			lightgleft += lightgleftstep;
			lightbright += lightbrightstep;
			lightbleft += lightbleftstep;
			prowdest = prowdest + surfrowbytes;
		}

		if (psource >= r_sourcemax)
			psource -= r_stepback;
	}
}

//==========================================================================
//
//	D_DrawSurfaceBlock8RGB_mip3
//
//==========================================================================

void D_DrawSurfaceBlock8RGB_mip3(void)
{
	int			v, i, b;
	int			lightrstep, lightrtemp, lightr;
	int			lightgstep, lightgtemp, lightg;
	int			lightbstep, lightbtemp, lightb;
	byte		pix, *psource, *prowdest;

	psource = pbasesource;
	prowdest = (byte*)prowdestbase;

	for (v = 0; v < r_numvblocks; v++)
	{
		// FIXME: make these locals?
		// FIXME: use delta rather than both right and left, like ASM?
		lightrleft = r_lightptrr[0];
		lightrright = r_lightptrr[1];
		r_lightptrr += r_lightwidth;
		lightrleftstep = (r_lightptrr[0] - lightrleft) >> 1;
		lightrrightstep = (r_lightptrr[1] - lightrright) >> 1;

		lightgleft = r_lightptrg[0];
		lightgright = r_lightptrg[1];
		r_lightptrg += r_lightwidth;
		lightgleftstep = (r_lightptrg[0] - lightgleft) >> 1;
		lightgrightstep = (r_lightptrg[1] - lightgright) >> 1;

		lightbleft = r_lightptrb[0];
		lightbright = r_lightptrb[1];
		r_lightptrb += r_lightwidth;
		lightbleftstep = (r_lightptrb[0] - lightbleft) >> 1;
		lightbrightstep = (r_lightptrb[1] - lightbright) >> 1;

		for (i = 0; i < 2; i++)
		{
			lightrtemp = lightrleft - lightrright;
			lightrstep = lightrtemp >> 1;
			lightgtemp = lightgleft - lightgright;
			lightgstep = lightgtemp >> 1;
			lightbtemp = lightbleft - lightbright;
			lightbstep = lightbtemp >> 1;

			lightr = lightrright;
			lightg = lightgright;
			lightb = lightbright;

			for (b = 1; b >= 0; b--)
			{
				pix = psource[b];
				prowdest[b] = d_rgbtable[
					fadetable16r[(lightr & 0xFF00) + pix] |
					fadetable16g[(lightg & 0xFF00) + pix] |
					fadetable16b[(lightb & 0xFF00) + pix]];
				lightr += lightrstep;
				lightg += lightgstep;
				lightb += lightbstep;
			}
	
			psource += sourcetstep;
			lightrright += lightrrightstep;
			lightrleft += lightrleftstep;
			lightgright += lightgrightstep;
			lightgleft += lightgleftstep;
			lightbright += lightbrightstep;
			lightbleft += lightbleftstep;
			prowdest = prowdest + surfrowbytes;
		}

		if (psource >= r_sourcemax)
			psource -= r_stepback;
	}
}

//==========================================================================
//
//	D_DrawSurfaceBlock16RGB_mip0
//
//==========================================================================

void D_DrawSurfaceBlock16RGB_mip0(void)
{
	int			v, i, b;
	int			lightrstep, lightrtemp, lightr;
	int			lightgstep, lightgtemp, lightg;
	int			lightbstep, lightbtemp, lightb;
	byte		pix, *psource;
	word		*prowdest;

	psource = pbasesource;
	prowdest = (word*)prowdestbase;

	for (v = 0; v < r_numvblocks; v++)
	{
		// FIXME: make these locals?
		// FIXME: use delta rather than both right and left, like ASM?
		lightrleft = r_lightptrr[0];
		lightrright = r_lightptrr[1];
		r_lightptrr += r_lightwidth;
		lightrleftstep = (r_lightptrr[0] - lightrleft) >> 4;
		lightrrightstep = (r_lightptrr[1] - lightrright) >> 4;

		lightgleft = r_lightptrg[0];
		lightgright = r_lightptrg[1];
		r_lightptrg += r_lightwidth;
		lightgleftstep = (r_lightptrg[0] - lightgleft) >> 4;
		lightgrightstep = (r_lightptrg[1] - lightgright) >> 4;

		lightbleft = r_lightptrb[0];
		lightbright = r_lightptrb[1];
		r_lightptrb += r_lightwidth;
		lightbleftstep = (r_lightptrb[0] - lightbleft) >> 4;
		lightbrightstep = (r_lightptrb[1] - lightbright) >> 4;

		for (i = 0; i < 16; i++)
		{
			lightrtemp = lightrleft - lightrright;
			lightrstep = lightrtemp >> 4;
			lightgtemp = lightgleft - lightgright;
			lightgstep = lightgtemp >> 4;
			lightbtemp = lightbleft - lightbright;
			lightbstep = lightbtemp >> 4;

			lightr = lightrright;
			lightg = lightgright;
			lightb = lightbright;

			for (b = 15; b >= 0; b--)
			{
				pix = psource[b];
				prowdest[b] = fadetable16r[(lightr & 0xFF00) + pix] |
					fadetable16g[(lightg & 0xFF00) + pix] |
					fadetable16b[(lightb & 0xFF00) + pix];
				lightr += lightrstep;
				lightg += lightgstep;
				lightb += lightbstep;
			}
	
			psource += sourcetstep;
			lightrright += lightrrightstep;
			lightrleft += lightrleftstep;
			lightgright += lightgrightstep;
			lightgleft += lightgleftstep;
			lightbright += lightbrightstep;
			lightbleft += lightbleftstep;
			prowdest = (word*)((byte*)prowdest + surfrowbytes);
		}

		if (psource >= r_sourcemax)
			psource -= r_stepback;
	}
}

//==========================================================================
//
//	D_DrawSurfaceBlock16RGB_mip1
//
//==========================================================================

void D_DrawSurfaceBlock16RGB_mip1(void)
{
	int			v, i, b;
	int			lightrstep, lightrtemp, lightr;
	int			lightgstep, lightgtemp, lightg;
	int			lightbstep, lightbtemp, lightb;
	byte		pix, *psource;
	word		*prowdest;

	psource = pbasesource;
	prowdest = (word*)prowdestbase;

	for (v = 0; v < r_numvblocks; v++)
	{
		// FIXME: make these locals?
		// FIXME: use delta rather than both right and left, like ASM?
		lightrleft = r_lightptrr[0];
		lightrright = r_lightptrr[1];
		r_lightptrr += r_lightwidth;
		lightrleftstep = (r_lightptrr[0] - lightrleft) >> 3;
		lightrrightstep = (r_lightptrr[1] - lightrright) >> 3;

		lightgleft = r_lightptrg[0];
		lightgright = r_lightptrg[1];
		r_lightptrg += r_lightwidth;
		lightgleftstep = (r_lightptrg[0] - lightgleft) >> 3;
		lightgrightstep = (r_lightptrg[1] - lightgright) >> 3;

		lightbleft = r_lightptrb[0];
		lightbright = r_lightptrb[1];
		r_lightptrb += r_lightwidth;
		lightbleftstep = (r_lightptrb[0] - lightbleft) >> 3;
		lightbrightstep = (r_lightptrb[1] - lightbright) >> 3;

		for (i = 0; i < 8; i++)
		{
			lightrtemp = lightrleft - lightrright;
			lightrstep = lightrtemp >> 3;
			lightgtemp = lightgleft - lightgright;
			lightgstep = lightgtemp >> 3;
			lightbtemp = lightbleft - lightbright;
			lightbstep = lightbtemp >> 3;

			lightr = lightrright;
			lightg = lightgright;
			lightb = lightbright;

			for (b = 7; b >= 0; b--)
			{
				pix = psource[b];
				prowdest[b] = fadetable16r[(lightr & 0xFF00) + pix] |
					fadetable16g[(lightg & 0xFF00) + pix] |
					fadetable16b[(lightb & 0xFF00) + pix];
				lightr += lightrstep;
				lightg += lightgstep;
				lightb += lightbstep;
			}
	
			psource += sourcetstep;
			lightrright += lightrrightstep;
			lightrleft += lightrleftstep;
			lightgright += lightgrightstep;
			lightgleft += lightgleftstep;
			lightbright += lightbrightstep;
			lightbleft += lightbleftstep;
			prowdest = (word*)((byte*)prowdest + surfrowbytes);
		}

		if (psource >= r_sourcemax)
			psource -= r_stepback;
	}
}

//==========================================================================
//
//	D_DrawSurfaceBlock16RGB_mip2
//
//==========================================================================

void D_DrawSurfaceBlock16RGB_mip2(void)
{
	int			v, i, b;
	int			lightrstep, lightrtemp, lightr;
	int			lightgstep, lightgtemp, lightg;
	int			lightbstep, lightbtemp, lightb;
	byte		pix, *psource;
	word		*prowdest;

	psource = pbasesource;
	prowdest = (word*)prowdestbase;

	for (v = 0; v < r_numvblocks; v++)
	{
		// FIXME: make these locals?
		// FIXME: use delta rather than both right and left, like ASM?
		lightrleft = r_lightptrr[0];
		lightrright = r_lightptrr[1];
		r_lightptrr += r_lightwidth;
		lightrleftstep = (r_lightptrr[0] - lightrleft) >> 2;
		lightrrightstep = (r_lightptrr[1] - lightrright) >> 2;

		lightgleft = r_lightptrg[0];
		lightgright = r_lightptrg[1];
		r_lightptrg += r_lightwidth;
		lightgleftstep = (r_lightptrg[0] - lightgleft) >> 2;
		lightgrightstep = (r_lightptrg[1] - lightgright) >> 2;

		lightbleft = r_lightptrb[0];
		lightbright = r_lightptrb[1];
		r_lightptrb += r_lightwidth;
		lightbleftstep = (r_lightptrb[0] - lightbleft) >> 2;
		lightbrightstep = (r_lightptrb[1] - lightbright) >> 2;

		for (i = 0; i < 4; i++)
		{
			lightrtemp = lightrleft - lightrright;
			lightrstep = lightrtemp >> 2;
			lightgtemp = lightgleft - lightgright;
			lightgstep = lightgtemp >> 2;
			lightbtemp = lightbleft - lightbright;
			lightbstep = lightbtemp >> 2;

			lightr = lightrright;
			lightg = lightgright;
			lightb = lightbright;

			for (b = 3; b >= 0; b--)
			{
				pix = psource[b];
				prowdest[b] = fadetable16r[(lightr & 0xFF00) + pix] |
					fadetable16g[(lightg & 0xFF00) + pix] |
					fadetable16b[(lightb & 0xFF00) + pix];
				lightr += lightrstep;
				lightg += lightgstep;
				lightb += lightbstep;
			}
	
			psource += sourcetstep;
			lightrright += lightrrightstep;
			lightrleft += lightrleftstep;
			lightgright += lightgrightstep;
			lightgleft += lightgleftstep;
			lightbright += lightbrightstep;
			lightbleft += lightbleftstep;
			prowdest = (word*)((byte*)prowdest + surfrowbytes);
		}

		if (psource >= r_sourcemax)
			psource -= r_stepback;
	}
}

//==========================================================================
//
//	D_DrawSurfaceBlock16RGB_mip3
//
//==========================================================================

void D_DrawSurfaceBlock16RGB_mip3(void)
{
	int			v, i, b;
	int			lightrstep, lightrtemp, lightr;
	int			lightgstep, lightgtemp, lightg;
	int			lightbstep, lightbtemp, lightb;
	byte		pix, *psource;
	word		*prowdest;

	psource = pbasesource;
	prowdest = (word*)prowdestbase;

	for (v = 0; v < r_numvblocks; v++)
	{
		// FIXME: make these locals?
		// FIXME: use delta rather than both right and left, like ASM?
		lightrleft = r_lightptrr[0];
		lightrright = r_lightptrr[1];
		r_lightptrr += r_lightwidth;
		lightrleftstep = (r_lightptrr[0] - lightrleft) >> 1;
		lightrrightstep = (r_lightptrr[1] - lightrright) >> 1;

		lightgleft = r_lightptrg[0];
		lightgright = r_lightptrg[1];
		r_lightptrg += r_lightwidth;
		lightgleftstep = (r_lightptrg[0] - lightgleft) >> 1;
		lightgrightstep = (r_lightptrg[1] - lightgright) >> 1;

		lightbleft = r_lightptrb[0];
		lightbright = r_lightptrb[1];
		r_lightptrb += r_lightwidth;
		lightbleftstep = (r_lightptrb[0] - lightbleft) >> 1;
		lightbrightstep = (r_lightptrb[1] - lightbright) >> 1;

		for (i = 0; i < 2; i++)
		{
			lightrtemp = lightrleft - lightrright;
			lightrstep = lightrtemp >> 1;
			lightgtemp = lightgleft - lightgright;
			lightgstep = lightgtemp >> 1;
			lightbtemp = lightbleft - lightbright;
			lightbstep = lightbtemp >> 1;

			lightr = lightrright;
			lightg = lightgright;
			lightb = lightbright;

			for (b = 1; b >= 0; b--)
			{
				pix = psource[b];
				prowdest[b] = fadetable16r[(lightr & 0xFF00) + pix] |
					fadetable16g[(lightg & 0xFF00) + pix] |
					fadetable16b[(lightb & 0xFF00) + pix];
				lightr += lightrstep;
				lightg += lightgstep;
				lightb += lightbstep;
			}
	
			psource += sourcetstep;
			lightrright += lightrrightstep;
			lightrleft += lightrleftstep;
			lightgright += lightgrightstep;
			lightgleft += lightgleftstep;
			lightbright += lightbrightstep;
			lightbleft += lightbleftstep;
			prowdest = (word*)((byte*)prowdest + surfrowbytes);
		}

		if (psource >= r_sourcemax)
			psource -= r_stepback;
	}
}

//==========================================================================
//
//	D_DrawSurfaceBlock32RGB_mip0
//
//==========================================================================

void D_DrawSurfaceBlock32RGB_mip0(void)
{
	int			v, i, b;
	int			lightrstep, lightrtemp, lightr;
	int			lightgstep, lightgtemp, lightg;
	int			lightbstep, lightbtemp, lightb;
	byte		pix, *psource;
	dword		*prowdest;

	psource = pbasesource;
	prowdest = (dword*)prowdestbase;

	for (v = 0; v < r_numvblocks; v++)
	{
		// FIXME: make these locals?
		// FIXME: use delta rather than both right and left, like ASM?
		lightrleft = r_lightptrr[0];
		lightrright = r_lightptrr[1];
		r_lightptrr += r_lightwidth;
		lightrleftstep = (r_lightptrr[0] - lightrleft) >> 4;
		lightrrightstep = (r_lightptrr[1] - lightrright) >> 4;

		lightgleft = r_lightptrg[0];
		lightgright = r_lightptrg[1];
		r_lightptrg += r_lightwidth;
		lightgleftstep = (r_lightptrg[0] - lightgleft) >> 4;
		lightgrightstep = (r_lightptrg[1] - lightgright) >> 4;

		lightbleft = r_lightptrb[0];
		lightbright = r_lightptrb[1];
		r_lightptrb += r_lightwidth;
		lightbleftstep = (r_lightptrb[0] - lightbleft) >> 4;
		lightbrightstep = (r_lightptrb[1] - lightbright) >> 4;

		for (i = 0; i < 16; i++)
		{
			lightrtemp = lightrleft - lightrright;
			lightrstep = lightrtemp >> 4;
			lightgtemp = lightgleft - lightgright;
			lightgstep = lightgtemp >> 4;
			lightbtemp = lightbleft - lightbright;
			lightbstep = lightbtemp >> 4;

			lightr = lightrright;
			lightg = lightgright;
			lightb = lightbright;

			for (b = 15; b >= 0; b--)
			{
				pix = psource[b];
				prowdest[b] = MakeCol32(
					fadetable32r[(lightr & 0xFF00) + pix],
					fadetable32g[(lightg & 0xFF00) + pix],
					fadetable32b[(lightb & 0xFF00) + pix]);
				lightr += lightrstep;
				lightg += lightgstep;
				lightb += lightbstep;
			}
	
			psource += sourcetstep;
			lightrright += lightrrightstep;
			lightrleft += lightrleftstep;
			lightgright += lightgrightstep;
			lightgleft += lightgleftstep;
			lightbright += lightbrightstep;
			lightbleft += lightbleftstep;
			prowdest = (dword*)((byte*)prowdest + surfrowbytes);
		}

		if (psource >= r_sourcemax)
			psource -= r_stepback;
	}
}

//==========================================================================
//
//	D_DrawSurfaceBlock32RGB_mip1
//
//==========================================================================

void D_DrawSurfaceBlock32RGB_mip1(void)
{
	int			v, i, b;
	int			lightrstep, lightrtemp, lightr;
	int			lightgstep, lightgtemp, lightg;
	int			lightbstep, lightbtemp, lightb;
	byte		pix, *psource;
	dword		*prowdest;

	psource = pbasesource;
	prowdest = (dword*)prowdestbase;

	for (v = 0; v < r_numvblocks; v++)
	{
		// FIXME: make these locals?
		// FIXME: use delta rather than both right and left, like ASM?
		lightrleft = r_lightptrr[0];
		lightrright = r_lightptrr[1];
		r_lightptrr += r_lightwidth;
		lightrleftstep = (r_lightptrr[0] - lightrleft) >> 3;
		lightrrightstep = (r_lightptrr[1] - lightrright) >> 3;

		lightgleft = r_lightptrg[0];
		lightgright = r_lightptrg[1];
		r_lightptrg += r_lightwidth;
		lightgleftstep = (r_lightptrg[0] - lightgleft) >> 3;
		lightgrightstep = (r_lightptrg[1] - lightgright) >> 3;

		lightbleft = r_lightptrb[0];
		lightbright = r_lightptrb[1];
		r_lightptrb += r_lightwidth;
		lightbleftstep = (r_lightptrb[0] - lightbleft) >> 3;
		lightbrightstep = (r_lightptrb[1] - lightbright) >> 3;

		for (i = 0; i < 8; i++)
		{
			lightrtemp = lightrleft - lightrright;
			lightrstep = lightrtemp >> 3;
			lightgtemp = lightgleft - lightgright;
			lightgstep = lightgtemp >> 3;
			lightbtemp = lightbleft - lightbright;
			lightbstep = lightbtemp >> 3;

			lightr = lightrright;
			lightg = lightgright;
			lightb = lightbright;

			for (b = 7; b >= 0; b--)
			{
				pix = psource[b];
				prowdest[b] = MakeCol32(
					fadetable32r[(lightr & 0xFF00) + pix],
					fadetable32g[(lightg & 0xFF00) + pix],
					fadetable32b[(lightb & 0xFF00) + pix]);
				lightr += lightrstep;
				lightg += lightgstep;
				lightb += lightbstep;
			}
	
			psource += sourcetstep;
			lightrright += lightrrightstep;
			lightrleft += lightrleftstep;
			lightgright += lightgrightstep;
			lightgleft += lightgleftstep;
			lightbright += lightbrightstep;
			lightbleft += lightbleftstep;
			prowdest = (dword*)((byte*)prowdest + surfrowbytes);
		}

		if (psource >= r_sourcemax)
			psource -= r_stepback;
	}
}

//==========================================================================
//
//	D_DrawSurfaceBlock32RGB_mip2
//
//==========================================================================

void D_DrawSurfaceBlock32RGB_mip2(void)
{
	int			v, i, b;
	int			lightrstep, lightrtemp, lightr;
	int			lightgstep, lightgtemp, lightg;
	int			lightbstep, lightbtemp, lightb;
	byte		pix, *psource;
	dword		*prowdest;

	psource = pbasesource;
	prowdest = (dword*)prowdestbase;

	for (v = 0; v < r_numvblocks; v++)
	{
		// FIXME: make these locals?
		// FIXME: use delta rather than both right and left, like ASM?
		lightrleft = r_lightptrr[0];
		lightrright = r_lightptrr[1];
		r_lightptrr += r_lightwidth;
		lightrleftstep = (r_lightptrr[0] - lightrleft) >> 2;
		lightrrightstep = (r_lightptrr[1] - lightrright) >> 2;

		lightgleft = r_lightptrg[0];
		lightgright = r_lightptrg[1];
		r_lightptrg += r_lightwidth;
		lightgleftstep = (r_lightptrg[0] - lightgleft) >> 2;
		lightgrightstep = (r_lightptrg[1] - lightgright) >> 2;

		lightbleft = r_lightptrb[0];
		lightbright = r_lightptrb[1];
		r_lightptrb += r_lightwidth;
		lightbleftstep = (r_lightptrb[0] - lightbleft) >> 2;
		lightbrightstep = (r_lightptrb[1] - lightbright) >> 2;

		for (i = 0; i < 4; i++)
		{
			lightrtemp = lightrleft - lightrright;
			lightrstep = lightrtemp >> 2;
			lightgtemp = lightgleft - lightgright;
			lightgstep = lightgtemp >> 2;
			lightbtemp = lightbleft - lightbright;
			lightbstep = lightbtemp >> 2;

			lightr = lightrright;
			lightg = lightgright;
			lightb = lightbright;

			for (b = 3; b >= 0; b--)
			{
				pix = psource[b];
				prowdest[b] = MakeCol32(
					fadetable32r[(lightr & 0xFF00) + pix],
					fadetable32g[(lightg & 0xFF00) + pix],
					fadetable32b[(lightb & 0xFF00) + pix]);
				lightr += lightrstep;
				lightg += lightgstep;
				lightb += lightbstep;
			}
	
			psource += sourcetstep;
			lightrright += lightrrightstep;
			lightrleft += lightrleftstep;
			lightgright += lightgrightstep;
			lightgleft += lightgleftstep;
			lightbright += lightbrightstep;
			lightbleft += lightbleftstep;
			prowdest = (dword*)((byte*)prowdest + surfrowbytes);
		}

		if (psource >= r_sourcemax)
			psource -= r_stepback;
	}
}

//==========================================================================
//
//	D_DrawSurfaceBlock32RGB_mip3
//
//==========================================================================

void D_DrawSurfaceBlock32RGB_mip3(void)
{
	int			v, i, b;
	int			lightrstep, lightrtemp, lightr;
	int			lightgstep, lightgtemp, lightg;
	int			lightbstep, lightbtemp, lightb;
	byte		pix, *psource;
	dword		*prowdest;

	psource = pbasesource;
	prowdest = (dword*)prowdestbase;

	for (v = 0; v < r_numvblocks; v++)
	{
		// FIXME: make these locals?
		// FIXME: use delta rather than both right and left, like ASM?
		lightrleft = r_lightptrr[0];
		lightrright = r_lightptrr[1];
		r_lightptrr += r_lightwidth;
		lightrleftstep = (r_lightptrr[0] - lightrleft) >> 1;
		lightrrightstep = (r_lightptrr[1] - lightrright) >> 1;

		lightgleft = r_lightptrg[0];
		lightgright = r_lightptrg[1];
		r_lightptrg += r_lightwidth;
		lightgleftstep = (r_lightptrg[0] - lightgleft) >> 1;
		lightgrightstep = (r_lightptrg[1] - lightgright) >> 1;

		lightbleft = r_lightptrb[0];
		lightbright = r_lightptrb[1];
		r_lightptrb += r_lightwidth;
		lightbleftstep = (r_lightptrb[0] - lightbleft) >> 1;
		lightbrightstep = (r_lightptrb[1] - lightbright) >> 1;

		for (i = 0; i < 2; i++)
		{
			lightrtemp = lightrleft - lightrright;
			lightrstep = lightrtemp >> 1;
			lightgtemp = lightgleft - lightgright;
			lightgstep = lightgtemp >> 1;
			lightbtemp = lightbleft - lightbright;
			lightbstep = lightbtemp >> 1;

			lightr = lightrright;
			lightg = lightgright;
			lightb = lightbright;

			for (b = 1; b >= 0; b--)
			{
				pix = psource[b];
				prowdest[b] = MakeCol32(
					fadetable32r[(lightr & 0xFF00) + pix],
					fadetable32g[(lightg & 0xFF00) + pix],
					fadetable32b[(lightb & 0xFF00) + pix]);
				lightr += lightrstep;
				lightg += lightgstep;
				lightb += lightbstep;
			}
	
			psource += sourcetstep;
			lightrright += lightrrightstep;
			lightrleft += lightrleftstep;
			lightgright += lightgrightstep;
			lightgleft += lightgleftstep;
			lightbright += lightbrightstep;
			lightbleft += lightbleftstep;
			prowdest = (dword*)((byte*)prowdest + surfrowbytes);
		}

		if (psource >= r_sourcemax)
			psource -= r_stepback;
	}
}

#endif // USEASM

//==========================================================================
//
//	D_DrawSkySurf_8
//
//==========================================================================

void D_DrawSkySurf_8(void)
{
	byte	*basesrc, *src;
	byte	*dst;
	int		texwidth, texheight;
	int		smask;
	int		s, t, texs;

	texwidth = dsky_mt1->width;
	texheight = dsky_mt1->width;
	smask = texwidth - 1;

	basesrc = (byte *)dsky_mt1 + dsky_mt1->offsets[0];
	dst = dsky_cachedest;

	for (t = 0; t < dsky_cacheheight; t++)
	{
		src = &basesrc[(t % texheight) * texwidth];

		texs = dsky_offs1;

		for (s = 0; s < dsky_cachewidth; s++)
		{
			*dst++ = src[texs];
			texs = (texs + 1) & smask;
		}
	}
}

//==========================================================================
//
//	D_DrawSkySurf_16
//
//==========================================================================

void D_DrawSkySurf_16(void)
{
	byte	*basesrc, *src;
	word	*dst;
	int		texwidth, texheight;
	int		smask;
	int		s, t, texs;

	texwidth = dsky_mt1->width;
	texheight = dsky_mt1->width;
	smask = texwidth - 1;

	basesrc = (byte *)dsky_mt1 + dsky_mt1->offsets[0];
	dst = (word *)dsky_cachedest;

	for (t = 0; t < dsky_cacheheight; t++)
	{
		src = &basesrc[(t % texheight) * texwidth];

		texs = dsky_offs1;

		for (s = 0; s < dsky_cachewidth; s++)
		{
			*dst++ = pal8_to16[src[texs]];
			texs = (texs + 1) & smask;
		}
	}
}

//==========================================================================
//
//	D_DrawSkySurf_32
//
//==========================================================================

void D_DrawSkySurf_32(void)
{
	byte	*basesrc, *src;
	dword	*dst;
	int		texwidth, texheight;
	int		smask;
	int		s, t, texs;

	texwidth = dsky_mt1->width;
	texheight = dsky_mt1->width;
	smask = texwidth - 1;

	basesrc = (byte *)dsky_mt1 + dsky_mt1->offsets[0];
	dst = (dword *)dsky_cachedest;

	for (t = 0; t < dsky_cacheheight; t++)
	{
		src = &basesrc[(t % texheight) * texwidth];

		texs = dsky_offs1;

		for (s = 0; s < dsky_cachewidth; s++)
		{
			*dst++ = pal2rgb[src[texs]];
			texs = (texs + 1) & smask;
		}
	}
}

//==========================================================================
//
//	D_DrawDoubleSkySurf_8
//
//==========================================================================

void D_DrawDoubleSkySurf_8(void)
{
	byte	*basesrc1, *basesrc2, *src1, *src2;
	byte	*dst;
	int		texwidth1, texheight1, texwidth2, texheight2;
	int		smask1, smask2;
	int		s, t, texs1, texs2;

	texwidth1 = dsky_mt1->width;
	texheight1 = dsky_mt1->width;

	texwidth2 = dsky_mt2->width;
	texheight2 = dsky_mt2->width;

	basesrc1 = (byte *)dsky_mt1 + dsky_mt1->offsets[0];
	basesrc2 = (byte *)dsky_mt2 + dsky_mt2->offsets[0];

	dst = dsky_cachedest;

	smask1 = texwidth1 - 1;
	smask2 = texwidth2 - 1;

	for (t = 0; t < dsky_cacheheight; t++)
	{
		src1 = &basesrc1[(t % texheight1) * texwidth1];
		src2 = &basesrc2[(t % texheight2) * texwidth2];

		texs1 = dsky_offs1;
		texs2 = dsky_offs2;

		for (s = 0; s < dsky_cachewidth; s++)
		{
			if (src2[texs2])
			{
				*dst++ = src2[texs2];
			}
			else
			{
				*dst++ = src1[texs1];
			}
			texs1 = (texs1 + 1) & smask1;
			texs2 = (texs2 + 1) & smask2;
		}
	}
}

//==========================================================================
//
//	D_DrawDoubleSkySurf_16
//
//==========================================================================

void D_DrawDoubleSkySurf_16(void)
{
	byte	*basesrc1, *basesrc2, *src1, *src2;
	word	*dst;
	int		texwidth1, texheight1, texwidth2, texheight2;
	int		smask1, smask2;
	int		s, t, texs1, texs2;

	texwidth1 = dsky_mt1->width;
	texheight1 = dsky_mt1->width;

	texwidth2 = dsky_mt2->width;
	texheight2 = dsky_mt2->width;

	basesrc1 = (byte *)dsky_mt1 + dsky_mt1->offsets[0];
	basesrc2 = (byte *)dsky_mt2 + dsky_mt2->offsets[0];

	dst = (word *)dsky_cachedest;

	smask1 = texwidth1 - 1;
	smask2 = texwidth2 - 1;

	for (t = 0; t < dsky_cacheheight; t++)
	{
		src1 = &basesrc1[(t % texheight1) * texwidth1];
		src2 = &basesrc2[(t % texheight2) * texwidth2];

		texs1 = dsky_offs1;
		texs2 = dsky_offs2;

		for (s = 0; s < dsky_cachewidth; s++)
		{
			if (src2[texs2])
			{
				*dst++ = pal8_to16[src2[texs2]];
			}
			else
			{
				*dst++ = pal8_to16[src1[texs1]];
			}
			texs1 = (texs1 + 1) & smask1;
			texs2 = (texs2 + 1) & smask2;
		}
	}
}

//==========================================================================
//
//	D_DrawDoubleSkySurf_32
//
//==========================================================================

void D_DrawDoubleSkySurf_32(void)
{
	byte	*basesrc1, *basesrc2, *src1, *src2;
	dword	*dst;
	int		texwidth1, texheight1, texwidth2, texheight2;
	int		smask1, smask2;
	int		s, t, texs1, texs2;

	texwidth1 = dsky_mt1->width;
	texheight1 = dsky_mt1->width;

	texwidth2 = dsky_mt2->width;
	texheight2 = dsky_mt2->width;

	basesrc1 = (byte *)dsky_mt1 + dsky_mt1->offsets[0];
	basesrc2 = (byte *)dsky_mt2 + dsky_mt2->offsets[0];

	dst = (dword *)dsky_cachedest;

	smask1 = texwidth1 - 1;
	smask2 = texwidth2 - 1;

	for (t = 0; t < dsky_cacheheight; t++)
	{
		src1 = &basesrc1[(t % texheight1) * texwidth1];
		src2 = &basesrc2[(t % texheight2) * texwidth2];

		texs1 = dsky_offs1;
		texs2 = dsky_offs2;

		for (s = 0; s < dsky_cachewidth; s++)
		{
			if (src2[texs2])
			{
				*dst++ = pal2rgb[src2[texs2]];
			}
			else
			{
				*dst++ = pal2rgb[src1[texs1]];
			}
			texs1 = (texs1 + 1) & smask1;
			texs2 = (texs2 + 1) & smask2;
		}
	}
}

//**************************************************************************
//
//	$Log$
//	Revision 1.8  2001/12/18 19:01:34  dj_jl
//	Changes for MSVC asm
//
//	Revision 1.7  2001/11/02 18:35:55  dj_jl
//	Sky optimizations
//	
//	Revision 1.6  2001/10/18 17:36:31  dj_jl
//	A lots of changes for Alpha 2
//	
//	Revision 1.5  2001/08/21 17:46:08  dj_jl
//	Added R_TextureAnimation, made SetTexture recognize flats
//	
//	Revision 1.4  2001/08/02 17:41:19  dj_jl
//	Added new asm for 32-bits
//	
//	Revision 1.3  2001/07/31 17:16:30  dj_jl
//	Just moved Log to the end of file
//	
//	Revision 1.2  2001/07/27 14:27:54  dj_jl
//	Update with Id-s and Log-s, some fixes
//
//**************************************************************************
