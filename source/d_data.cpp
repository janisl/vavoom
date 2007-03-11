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
vuint8			d_fadetable[32 * 256];	// Current colourmap
vuint16			d_fadetable16[32 * 256];
vuint16			d_fadetable16r[32 * 256];
vuint16			d_fadetable16g[32 * 256];
vuint16			d_fadetable16b[32 * 256];
vuint32			d_fadetable32[32 * 256];
vuint8			d_fadetable32r[32 * 256];
vuint8			d_fadetable32g[32 * 256];
vuint8			d_fadetable32b[32 * 256];

//
//	Translucency tables
//
vuint8*			tinttables[5];
vuint16			scaletable[32][256];

vuint8*			consbgmap = NULL;

bool			ForcePaletteUpdate;

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
				d_fadetable16r[i] = 0x8000;
				d_fadetable16g[i] = 0x8000;
				d_fadetable16b[i] = 0x8000;
				continue;
			}
			int r = (int)(r_palette[ci].r * frac + 0.5) + fog;
			int g = (int)(r_palette[ci].g * frac + 0.5) + fog;
			int b = (int)(r_palette[ci].b * frac + 0.5) + fog;
			d_fadetable16r[i] = (r << 7) & 0x7c00;
			d_fadetable16g[i] = (g << 2) & 0x03e0;
			d_fadetable16b[i] = (b >> 3) & 0x001f;
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
				d_fadetable16[i] = 0;
				d_fadetable16r[i] = 0;
				d_fadetable16g[i] = 0;
				d_fadetable16b[i] = 0;
				continue;
			}
			int r = gt[pal[ci].r] * colm / 32 + fog;
			int g = gt[pal[ci].g] * colm / 32 + fog;
			int b = gt[pal[ci].b] * colm / 32 + fog;
			d_fadetable16[i] = MakeCol(r, g, b);
			d_fadetable16r[i] = MakeCol(r, 0, 0);
			d_fadetable16g[i] = MakeCol(0, g, 0);
			d_fadetable16b[i] = MakeCol(0, 0, b);
			//	For 16 bit we use colour 0 as transparent
			if (!d_fadetable16[i])
			{
				d_fadetable16[i] = 1;
			}
			if (!d_fadetable16b[i])
			{
				d_fadetable16b[i] = MakeCol(0, 0, 1);
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
				d_fadetable32[i] = 0;
				d_fadetable32r[i] = 0;
				d_fadetable32g[i] = 0;
				d_fadetable32b[i] = 0;
				continue;
			}
			int r = gt[pal[ci].r] * colm / 32 + fog;
			int g = gt[pal[ci].g] * colm / 32 + fog;
			int b = gt[pal[ci].b] * colm / 32 + fog;
			d_fadetable32[i] = MakeCol32(r, g, b);
			d_fadetable32r[i] = r;
			d_fadetable32g[i] = g;
			d_fadetable32b[i] = b;
			//	For 32 bit we use colour 0 as transparent
			if (!d_fadetable32[i])
			{
				d_fadetable32[i] = 1;
			}
			if (!d_fadetable32b[i])
			{
				d_fadetable32b[i] = 1;
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
	memcpy(d_fadetable, colourmaps, 32 * 256);
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

	newshifts = ForcePaletteUpdate;
	ForcePaletteUpdate = false;

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
	if (r_fog)
	{
		VStream* Strm = W_CreateLumpReaderName(NAME_fogmap);
		Strm->Serialise(d_fadetable, Strm->TotalSize());
		delete Strm;
	}
	else
	{
		memcpy(d_fadetable, colourmaps, 32 * 256);
	}

	//	Remap colour 0 to alternate balck colour
	for (int i = 0; i < 32 * 256; i++)
	{
		if (!(i & 0xff))
		{
			d_fadetable[i] = 0;
		}
		else if (!colourmaps[i])
		{
			d_fadetable[i] = r_black_colour;
		}
	}

	if (ScreenBPP == 8)
	{
		CalcRGBTable8();
	}

	ForcePaletteUpdate = true;
	FlushCaches(false);
	unguard;
}
