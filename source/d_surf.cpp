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
}

// EXTERNAL DATA DECLARATIONS ----------------------------------------------

// PUBLIC DATA DEFINITIONS -------------------------------------------------

// PRIVATE DATA DEFINITIONS ------------------------------------------------

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
	texture = D_TextureAnimation(surface->texinfo->pic);
	
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
	
	surfrowbytes = surfwidth * PixelBytes;

	if (texture & TEXF_FLAT)
	{
		Drawer->SetFlat(texture);
	}
	else
	{
		Drawer->SetTexture(texture);
	}
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

//**************************************************************************
//
//	$Log$
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
