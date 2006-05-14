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

static void CalcRGBTable8()
{
	guard(CalcRGBTable8);
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
			int r = (int)(r_palette[ci].r * frac + 0.5) + fog;
			int g = (int)(r_palette[ci].g * frac + 0.5) + fog;
			int b = (int)(r_palette[ci].b * frac + 0.5) + fog;
			fadetable16r[i] = (r << 7) & 0x7c00;
			fadetable16g[i] = (g << 2) & 0x03e0;
			fadetable16b[i] = (b >> 3) & 0x001f;
		}
	}
	unguard;
}

//==========================================================================
//
//	CalcCol16Table
//
//==========================================================================

static void CalcCol16Table()
{
	guard(CalcCol16Table);
	byte *gt = gammatable[usegamma];
	for (int i = 0; i < 256; i++)
	{
		pal8_to16[i] = MakeCol(gt[r_palette[i].r], gt[r_palette[i].g],
			gt[r_palette[i].b]);
	}
	unguard;
}

//==========================================================================
//
//	CalcFadetable16
//
//==========================================================================

static void CalcFadetable16(rgb_t *pal)
{
	guard(CalcFadetable16);
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
	unguard;
}

//==========================================================================
//
//	CalcCol32Table
//
//==========================================================================

static void CalcCol32Table()
{
	guard(CalcCol32Table);
	byte *gt = gammatable[usegamma];
	for (int i = 0; i < 256; i++)
	{
		pal2rgb[i] = MakeCol(gt[r_palette[i].r], gt[r_palette[i].g],
			gt[r_palette[i].b]);
	}
	unguard;
}

//==========================================================================
//
//	CalcFadetable32
//
//==========================================================================

static void CalcFadetable32(rgb_t *pal)
{
	guard(CalcFadetable32);
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
	unguard;
}

//==========================================================================
//
//	VSoftwareDrawer::SetPalette
//
//==========================================================================

void VSoftwareDrawer::SetPalette(int InNum)
{
	guard(VSoftwareDrawer::SetPalette);
	int num = InNum;
	if (num < 0 || num >= 32)
	{
		GCon->Logf(NAME_Dev, "Invalid palette num %d", num);
		num = 0;
	}
	vuint8 *pal = r_playpal + num * 768;
	if (ScreenBPP == 8)
	{
		SetPalette8(pal);
		CalcRGBTable8();
	}
	else if (PixelBytes == 2)
	{
		CalcCol16Table();
		CalcFadetable16((rgb_t*)pal);
		FlushCaches(true);
		FlushTextureCaches();
	}
	else
	{
		CalcCol32Table();
		CalcFadetable32((rgb_t*)pal);
		FlushCaches(true);
		FlushTextureCaches();
	}

	//	Save palette
	memcpy(host_basepal, pal, 768);
	//	Must recalculate any cshifts
	memset(cl->prev_cshifts, 0, sizeof(cl->prev_cshifts));
	unguard;
}

//==========================================================================
//
//	InitColourmaps
//
//==========================================================================

static void InitColourmaps()
{
	guard(InitColourmaps);
	// Load in the light tables,
	colormaps = (byte*)W_CacheLumpName(NAME_colormap);
	fadetable = colormaps;
	fadetable16 = (word*)Z_Malloc(32 * 256 * 2);
	fadetable16r = (word*)Z_Malloc(32 * 256 * 2);
	fadetable16g = (word*)Z_Malloc(32 * 256 * 2);
	fadetable16b = (word*)Z_Malloc(32 * 256 * 2);
	fadetable32 = (dword*)Z_Malloc(32 * 256 * 4);
	fadetable32r = (byte*)Z_Malloc(32 * 256);
	fadetable32g = (byte*)Z_Malloc(32 * 256);
	fadetable32b = (byte*)Z_Malloc(32 * 256);
	unguard;
}

//==========================================================================
//
//	InitTranslucencyTables
//
//==========================================================================

static void InitTranslucencyTables()
{
	guard(InitTranslucencyTables);
	tinttables[0] = (byte*)W_CacheLumpName(NAME_transp10);
	tinttables[1] = (byte*)W_CacheLumpName(NAME_transp20);
	tinttables[2] = (byte*)W_CacheLumpName(NAME_transp30);
	tinttables[3] = (byte*)W_CacheLumpName(NAME_transp40);
	tinttables[4] = (byte*)W_CacheLumpName(NAME_transp50);

	for (int t = 0; t < 32; t++)
	{
		for (int i = 0; i < 256; i++)
		{
			scaletable[t][i] = (i << 8) * t / 31;
		}
	}
	unguard;
}

//==========================================================================
//
//	VSoftwareDrawer::InitData
//
//==========================================================================

void VSoftwareDrawer::InitData()
{
	guard(VSoftwareDrawer::InitData);
	d_rgbtable = (byte*)W_CacheLumpName(NAME_rgbtable);
	InitColourmaps();
	InitTranslucencyTables();
	unguard;
}

//==========================================================================
//
//	VSoftwareDrawer::UpdatePalette
//
//==========================================================================

void VSoftwareDrawer::UpdatePalette()
{
	guard(VSoftwareDrawer::UpdatePalette);
	int		i, j;
	bool	newshifts;
	byte	*basepal, *newpal;
	byte	pal[768];
	int		r,g,b;
	int		dstr, dstg, dstb, perc;

	newshifts = false;
	
	for (i = 0; i < NUM_CSHIFTS; i++)
	{
		if (cl->cshifts[i] != cl->prev_cshifts[i])
		{
			newshifts = true;
			cl->prev_cshifts[i] = cl->cshifts[i];
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
			perc = (cl->cshifts[j] >> 24) & 0xff;
			dstr = (cl->cshifts[j] >> 16) & 0xff;
			dstg = (cl->cshifts[j] >> 8) & 0xff;
			dstb = cl->cshifts[j] & 0xff;
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
		FlushCaches(true);
		FlushTextureCaches();
	}
	else
	{
		CalcCol32Table();
		CalcFadetable32((rgb_t*)pal);
		FlushCaches(true);
		FlushTextureCaches();
	}
	unguard;
}

//==========================================================================
//
//	VSoftwareDrawer::NewMap
//
//==========================================================================

void VSoftwareDrawer::NewMap()
{
	guard(VSoftwareDrawer::NewMap);
	if (fadetable != colormaps)
	{
		Z_Free(fadetable);
	}

	if (r_fog)
	{
		fadetable = (byte*)W_CacheLumpName(NAME_fogmap);
	}
	else
	{
		fadetable = colormaps;
	}

	//	Remap colour 0 to alternate balck colour
	for (int i = 0; i < 32 * 256; i++)
	{
		if (!(i & 0xff))
		{
			fadetable[i] = 0;
		}
		else if (!colormaps[i])
		{
			fadetable[i] = r_black_colour;
		}
	}

	if (ScreenBPP == 8)
	{
		CalcRGBTable8();
	}

	FlushCaches(false);
	unguard;
}
