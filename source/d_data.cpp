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

// HEADER FILES ------------------------------------------------------------

#include "d_local.h"

// MACROS ------------------------------------------------------------------

// TYPES -------------------------------------------------------------------

// EXTERNAL FUNCTION PROTOTYPES --------------------------------------------

// PUBLIC FUNCTION PROTOTYPES ----------------------------------------------

// PRIVATE FUNCTION PROTOTYPES ---------------------------------------------

// EXTERNAL DATA DECLARATIONS ----------------------------------------------

extern byte		gammatable[5][256];

// PUBLIC DATA DEFINITIONS -------------------------------------------------

//
//	Colormaps
//
byte			*colormaps;	// Standard colormap
byte			*fadetable;	// Current level's colormap
word			*fadetable16;
dword			*fadetable32;
word			*fadetable16r;
word			*fadetable16g;
word			*fadetable16b;
byte			*fadetable32r;
byte			*fadetable32g;
byte			*fadetable32b;

//
//	Translucency tables
//
byte 			*tinttables[5];
word			scaletable[32][256];

byte			*d_rgbtable;

// PRIVATE DATA DEFINITIONS ------------------------------------------------

static rgb_t	host_basepal[256];

// CODE --------------------------------------------------------------------

//==========================================================================
//
//	CalcRGBTable8
//
//==========================================================================

static void CalcRGBTable8(void)
{
	rgb_t *pal = (rgb_t*)W_CacheLumpName("playpal", PU_CACHE);
	int i = 0;
	for (int tn = 0; tn < 32; tn++)
	{
		float frac = 1.0 - tn / 32.0;
		int fog = r_fog ? 128 * tn / 32 : 0;
		for (int ci = 0; ci < 256; ci++, i++)
		{
			if (!(i & 0xff))
			{
				fadetable16r[i] = 0x8000;
				fadetable16g[i] = 0x8000;
				fadetable16b[i] = 0x8000;
				continue;
			}
			int r = (int)(pal[ci].r * frac + 0.5) + fog;
			int g = (int)(pal[ci].g * frac + 0.5) + fog;
			int b = (int)(pal[ci].b * frac + 0.5) + fog;
			fadetable16r[i] = (r << 7) & 0x7c00;
			fadetable16g[i] = (g << 2) & 0x03e0;
			fadetable16b[i] = (b >> 3) & 0x001f;
		}
	}
}

//==========================================================================
//
//	CalcCol16Table
//
//==========================================================================

static void CalcCol16Table(void)
{
	rgb_t *pal = (rgb_t*)W_CacheLumpName("playpal", PU_CACHE);
	byte *gt = gammatable[usegamma];
	for (int i = 0; i < 256; i++)
	{
		pal8_to16[i] = MakeCol(gt[pal[i].r], gt[pal[i].g], gt[pal[i].b]);
	}
}

//==========================================================================
//
//	CalcFadetable16
//
//==========================================================================

static void CalcFadetable16(rgb_t *pal)
{
	byte *gt = gammatable[usegamma];
	int i = 0;
	for (int tn = 0; tn < 32; tn++)
	{
		int colm = 32 - tn;
		int fog = r_fog ? 128 * tn / 32 : 0;
		for (int ci = 0; ci < 256; ci++, i++)
		{
			if (!(i & 0xff))
			{
				fadetable16[i] = 0;
				fadetable16r[i] = 0;
				fadetable16g[i] = 0;
				fadetable16b[i] = 0;
				continue;
			}
			int r = gt[pal[ci].r] * colm / 32 + fog;
			int g = gt[pal[ci].g] * colm / 32 + fog;
			int b = gt[pal[ci].b] * colm / 32 + fog;
			fadetable16[i] = MakeCol(r, g, b);
			fadetable16r[i] = MakeCol(r, 0, 0);
			fadetable16g[i] = MakeCol(0, g, 0);
			fadetable16b[i] = MakeCol(0, 0, b);
			//	For 16 bit we use color 0 as transparent
			if (!fadetable16[i])
			{
				fadetable16[i] = 1;
			}
			if (!fadetable16b[i])
			{
				fadetable16b[i] = MakeCol(0, 0, 1);
			}
		}
	}

	D_FlushCaches(true);
	D_FlushTextureCaches();
}

//==========================================================================
//
//	CalcCol32Table
//
//==========================================================================

static void CalcCol32Table(void)
{
	rgb_t *pal = (rgb_t*)W_CacheLumpName("playpal", PU_CACHE);
	byte *gt = gammatable[usegamma];
	for (int i = 0; i < 256; i++)
	{
		pal2rgb[i] = MakeCol(gt[pal[i].r], gt[pal[i].g], gt[pal[i].b]);
	}
}

//==========================================================================
//
//	CalcFadetable32
//
//==========================================================================

static void CalcFadetable32(rgb_t *pal)
{
	byte *gt = gammatable[usegamma];
	int i = 0;
	for (int tn = 0; tn < 32; tn++)
	{
		int colm = 32 - tn;
		int fog = r_fog ? 128 * tn / 32 : 0;
		for (int ci = 0; ci < 256; ci++, i++)
		{
			if (!(i & 0xff))
			{
				fadetable32[i] = 0;
				fadetable32r[i] = 0;
				fadetable32g[i] = 0;
				fadetable32b[i] = 0;
				continue;
			}
			int r = gt[pal[ci].r] * colm / 32 + fog;
			int g = gt[pal[ci].g] * colm / 32 + fog;
			int b = gt[pal[ci].b] * colm / 32 + fog;
			fadetable32[i] = MakeCol32(r, g, b);
			fadetable32r[i] = r;
			fadetable32g[i] = g;
			fadetable32b[i] = b;
			//	For 32 bit we use color 0 as transparent
			if (!fadetable32[i])
			{
				fadetable32[i] = 1;
			}
			if (!fadetable32b[i])
			{
				fadetable32b[i] = 1;
			}
		}
	}

	D_FlushCaches(true);
	D_FlushTextureCaches();
}

//==========================================================================
//
//	TSoftwareDrawer::SetPalette
//
//==========================================================================

void TSoftwareDrawer::SetPalette(int num)
{
	if (num < 0 || num >= 32)
	{
		cond << "Invalid palette num " << num << endl;
		num = 0;
	}
	byte *pal = (byte*)W_CacheLumpName("playpal", PU_CACHE) + num * 768;
	if (ScreenBPP == 8)
	{
		SetPalette8(pal);
		CalcRGBTable8();
	}
	else if (PixelBytes == 2)
	{
		CalcCol16Table();
		CalcFadetable16((rgb_t*)pal);
	}
	else
	{
		CalcCol32Table();
		CalcFadetable32((rgb_t*)pal);
	}

	//	Save palette
	memcpy(host_basepal, pal, 768);
	//	Must recalculate any cshifts
	memset(cl.prev_cshifts, 0, sizeof(cl.prev_cshifts));
}

//==========================================================================
//
//	InitColormaps
//
//==========================================================================

static void InitColormaps(void)
{
    // Load in the light tables,
    colormaps = (byte*)W_CacheLumpName("COLORMAP", PU_STATIC);
    fadetable = colormaps;
	fadetable16 = (word*)Z_Malloc(32 * 256 * 2);
	fadetable16r = (word*)Z_Malloc(32 * 256 * 2);
	fadetable16g = (word*)Z_Malloc(32 * 256 * 2);
	fadetable16b = (word*)Z_Malloc(32 * 256 * 2);
	fadetable32 = (dword*)Z_Malloc(32 * 256 * 4);
	fadetable32r = (byte*)Z_Malloc(32 * 256);
	fadetable32g = (byte*)Z_Malloc(32 * 256);
	fadetable32b = (byte*)Z_Malloc(32 * 256);
}

//==========================================================================
//
//	InitTranslucencyTables
//
//==========================================================================

static void InitTranslucencyTables(void)
{
    tinttables[0] = (byte*)W_CacheLumpName("TRANSP10", PU_STATIC);
    tinttables[1] = (byte*)W_CacheLumpName("TRANSP20", PU_STATIC);
    tinttables[2] = (byte*)W_CacheLumpName("TRANSP30", PU_STATIC);
    tinttables[3] = (byte*)W_CacheLumpName("TRANSP40", PU_STATIC);
    tinttables[4] = (byte*)W_CacheLumpName("TRANSP50", PU_STATIC);

	for (int t = 0; t < 32; t++)
	{
		for (int i = 0; i < 256; i++)
		{
			scaletable[t][i] = (i << 8) * t / 31;
		}
	}
}

//==========================================================================
//
//	TSoftwareDrawer::InitData
//
//==========================================================================

void TSoftwareDrawer::InitData(void)
{
	d_rgbtable = (byte*)W_CacheLumpName("RGBTABLE", PU_STATIC);
    InitColormaps();
	InitTranslucencyTables();
}

//==========================================================================
//
//	TSoftwareDrawer::UpdatePalette
//
//==========================================================================

void TSoftwareDrawer::UpdatePalette(void)
{
	int		i, j;
	bool	newshifts;
	byte	*basepal, *newpal;
	byte	pal[768];
	int		r,g,b;
	int		dstr, dstg, dstb, perc;

	newshifts = false;
	
	for (i = 0; i < NUM_CSHIFTS; i++)
	{
		if (cl.cshifts[i] != cl.prev_cshifts[i])
		{
			newshifts = true;
			cl.prev_cshifts[i] = cl.cshifts[i];
		}
	}
	
	if (!newshifts)
	{
		return;
	}
			
	basepal = (byte*)host_basepal;
	newpal = pal;
	
	for (i = 0; i < 256; i++)
	{
		r = basepal[0];
		g = basepal[1];
		b = basepal[2];
		basepal += 3;
	
		for (j = 0; j < NUM_CSHIFTS; j++)
		{
			perc = (cl.cshifts[j] >> 24) & 0xff;
			dstr = (cl.cshifts[j] >> 16) & 0xff;
			dstg = (cl.cshifts[j] >> 8) & 0xff;
			dstb = cl.cshifts[j] & 0xff;
			r += (perc * (dstr - r)) >> 8;
			g += (perc * (dstg - g)) >> 8;
			b += (perc * (dstb - b)) >> 8;
		}
		
		newpal[0] = r;
		newpal[1] = g;
		newpal[2] = b;
		newpal += 3;
	}

	if (ScreenBPP == 8)
	{
		SetPalette8(pal);
	}
	else if (PixelBytes == 2)
	{
		CalcCol16Table();
		CalcFadetable16((rgb_t*)pal);
	}
	else
	{
		CalcCol32Table();
		CalcFadetable32((rgb_t*)pal);
	}
}

//==========================================================================
//
//	TSoftwareDrawer::NewMap
//
//==========================================================================

void TSoftwareDrawer::NewMap(void)
{
	if (fadetable != colormaps)
	{
		Z_ChangeTag(fadetable, PU_CACHE);
	}

	if (r_fog)
	{
		fadetable = (byte*)W_CacheLumpName("FOGMAP", PU_STATIC);
	}
	else
	{
		fadetable = colormaps;
	}

	//	Remap color 0 to alternate balck color
	for (int i = 0; i < 32 * 256; i++)
	{
		if (!(i & 0xff))
		{
			fadetable[i] = 0;
		}
		else if (!colormaps[i])
		{
			fadetable[i] = r_black_color[0];
		}
	}

	if (ScreenBPP == 8)
	{
		CalcRGBTable8();
	}

	D_FlushCaches(false);
}

//**************************************************************************
//
//	$Log$
//	Revision 1.8  2002/01/07 12:16:42  dj_jl
//	Changed copyright year
//
//	Revision 1.7  2001/11/02 18:35:54  dj_jl
//	Sky optimizations
//	
//	Revision 1.6  2001/10/18 17:36:31  dj_jl
//	A lots of changes for Alpha 2
//	
//	Revision 1.5  2001/08/23 17:47:22  dj_jl
//	Started work on pics with custom palettes
//	
//	Revision 1.4  2001/08/15 17:27:17  dj_jl
//	Truecolor translucency with lookup table
//	
//	Revision 1.3  2001/07/31 17:16:30  dj_jl
//	Just moved Log to the end of file
//	
//	Revision 1.2  2001/07/27 14:27:54  dj_jl
//	Update with Id-s and Log-s, some fixes
//
//**************************************************************************
