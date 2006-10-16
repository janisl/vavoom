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

#include "d_local.h"

// MACROS ------------------------------------------------------------------

// TYPES -------------------------------------------------------------------

// EXTERNAL FUNCTION PROTOTYPES --------------------------------------------

// PUBLIC FUNCTION PROTOTYPES ----------------------------------------------

// PRIVATE FUNCTION PROTOTYPES ---------------------------------------------

// EXTERNAL DATA DECLARATIONS ----------------------------------------------

extern vuint8	gammatable[5][256];

// PUBLIC DATA DEFINITIONS -------------------------------------------------

//
//	Colourmaps
//
vuint8*			colourmaps;	// Standard colourmap
vuint8*			fadetable;	// Current level's colourmap
vuint16*		fadetable16;
vuint32*		fadetable32;
vuint16*		fadetable16r;
vuint16*		fadetable16g;
vuint16*		fadetable16b;
vuint8*			fadetable32r;
vuint8*			fadetable32g;
vuint8*			fadetable32b;

//
//	Translucency tables
//
vuint8*			tinttables[5];
vuint16			scaletable[32][256];

vuint8*			d_rgbtable;

vuint8*			consbgmap = NULL;

// PRIVATE DATA DEFINITIONS ------------------------------------------------

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
			//	For 16 bit we use colour 0 as transparent
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
			//	For 32 bit we use colour 0 as transparent
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
//	InitColourmaps
//
//==========================================================================

static void InitColourmaps()
{
	guard(InitColourmaps);
	// Load in the light tables,
	VStream* Strm = W_CreateLumpReaderName(NAME_colormap);
	colourmaps = new vuint8[Strm->TotalSize()];
	Strm->Serialise(colourmaps, Strm->TotalSize());
	delete Strm;
	fadetable = colourmaps;
	fadetable16 = (word*)Z_Malloc(32 * 256 * 2);
	fadetable16r = (word*)Z_Malloc(32 * 256 * 2);
	fadetable16g = (word*)Z_Malloc(32 * 256 * 2);
	fadetable16b = (word*)Z_Malloc(32 * 256 * 2);
	fadetable32 = (vuint32*)Z_Malloc(32 * 256 * 4);
	fadetable32r = (byte*)Z_Malloc(32 * 256);
	fadetable32g = (byte*)Z_Malloc(32 * 256);
	fadetable32b = (byte*)Z_Malloc(32 * 256);
	unguard;
}

//==========================================================================
//
//	CreateTranslucencyTable
//
//==========================================================================

static vuint8* CreateTranslucencyTable(int transluc)
{
	guard(CreateTranslucencyTable);
	vuint8 temp[768];
	for (int i = 0; i < 256; i++)
	{
		temp[i * 3]     = r_palette[i].r * transluc / 100;
		temp[i * 3 + 1] = r_palette[i].g * transluc / 100;
		temp[i * 3 + 2] = r_palette[i].b * transluc / 100;
	}
	vuint8* table = new vuint8[0x10000];
	vuint8* p = table;
	for (int i = 0; i < 256; i++)
	{
		int r = r_palette[i].r * (100 - transluc) / 100;
		int g = r_palette[i].g * (100 - transluc) / 100;
		int b = r_palette[i].b * (100 - transluc) / 100;
		vuint8* q = temp;
		for (int j = 0; j < 256; j++)
		{
			*(p++) = MakeCol8(r + q[0], g + q[1], b + q[2]);
			q += 3;
		}
	}
	return table;
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
	tinttables[0] = CreateTranslucencyTable(10);
	tinttables[1] = CreateTranslucencyTable(20);
	tinttables[2] = CreateTranslucencyTable(30);
	tinttables[3] = CreateTranslucencyTable(40);
	tinttables[4] = CreateTranslucencyTable(50);

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
	d_rgbtable = GTextureManager.GetRgbTable();
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
	byte	*newpal;
	byte	pal[768];
	int		r,g,b;
	int		dstr, dstg, dstb, perc;

	newshifts = false;

	for (i = 0; i < NUM_CSHIFTS; i++)
	{
		if (cl->CShifts[i] != GClGame->prev_cshifts[i])
		{
			newshifts = true;
			GClGame->prev_cshifts[i] = cl->CShifts[i];
		}
	}

	if (!newshifts)
	{
		return;
	}

	rgba_t* basepal = r_palette;
	newpal = pal;
	
	for (i = 0; i < 256; i++)
	{
		r = basepal->r;
		g = basepal->g;
		b = basepal->b;
		basepal++;
	
		for (j = 0; j < NUM_CSHIFTS; j++)
		{
			perc = (cl->CShifts[j] >> 24) & 0xff;
			dstr = (cl->CShifts[j] >> 16) & 0xff;
			dstg = (cl->CShifts[j] >> 8) & 0xff;
			dstb = cl->CShifts[j] & 0xff;
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
	if (fadetable != colourmaps)
	{
		Z_Free(fadetable);
	}

	if (r_fog)
	{
		VStream* Strm = W_CreateLumpReaderName(NAME_fogmap);
		fadetable = new vuint8[Strm->TotalSize()];
		Strm->Serialise(fadetable, Strm->TotalSize());
		delete Strm;
	}
	else
	{
		fadetable = colourmaps;
	}

	//	Remap colour 0 to alternate balck colour
	for (int i = 0; i < 32 * 256; i++)
	{
		if (!(i & 0xff))
		{
			fadetable[i] = 0;
		}
		else if (!colourmaps[i])
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
