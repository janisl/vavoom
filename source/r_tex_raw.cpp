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
#include "r_tex.h"

// MACROS ------------------------------------------------------------------

// TYPES -------------------------------------------------------------------

// EXTERNAL FUNCTION PROTOTYPES --------------------------------------------

// PUBLIC FUNCTION PROTOTYPES ----------------------------------------------

// PRIVATE FUNCTION PROTOTYPES ---------------------------------------------

// EXTERNAL DATA DECLARATIONS ----------------------------------------------

// PUBLIC DATA DEFINITIONS -------------------------------------------------

// PRIVATE DATA DEFINITIONS ------------------------------------------------

// CODE --------------------------------------------------------------------

//==========================================================================
//
//	VRawPicTexture::Create
//
//==========================================================================

VTexture* VRawPicTexture::Create(VStream& Strm, int LumpNum)
{
	guard(VRawPicTexture::Create);
	if (Strm.TotalSize() != 64000)
	{
		//	Wrong size.
		return NULL;
	}

	//	Do an extra check to see if it's a valid patch
	vint16		Width;
	vint16		Height;
	vint16		SOffset;
	vint16		TOffset;

	Strm.Seek(0);
	Strm << Width << Height << SOffset << TOffset;
	if (Width > 0 && Height > 0 && Width <= 2048 && Height < 510)
	{
		//	Dimensions seem to be valid. Check column directory to see if
		// it's valid. We expect at least one column to start exactly right
		// after the directory.
		bool GapAtStart = true;
		bool IsValid = true;
		vint32* Offsets = new vint32[Width];
		for (int i = 0; i < Width; i++)
		{
			Strm << Offsets[i];
			if (Offsets[i] == 8 + Width * 4)
			{
				GapAtStart = false;
			}
			else if (Offsets[i] < 8 + Width * 4 ||
				Offsets[i] >= Strm.TotalSize())
			{
				IsValid = false;
				break;
			}
		}
		if (IsValid && !GapAtStart)
		{
			//	Offsets seem to be valid.
			delete[] Offsets;
			Offsets = NULL;
			return NULL;
		}
		delete[] Offsets;
		Offsets = NULL;
	}

	return new VRawPicTexture(LumpNum, -1);
	unguard;
}

//==========================================================================
//
//	VRawPicTexture::VRawPicTexture
//
//==========================================================================

VRawPicTexture::VRawPicTexture(int ALumpNum, int APalLumpNum)
: PalLumpNum(APalLumpNum)
, Pixels(0)
, Palette(0)
{
	SourceLump = ALumpNum;
	Type = TEXTYPE_Pic;
	Name = W_LumpName(SourceLump);
	Width = 320;
	Height = 200;
	Format = PalLumpNum >= 0 ? TEXFMT_8Pal : TEXFMT_8;
}

//==========================================================================
//
//	VRawPicTexture::~VRawPicTexture
//
//==========================================================================

VRawPicTexture::~VRawPicTexture()
{
	guard(VRawPicTexture::~VRawPicTexture);
	if (Pixels)
	{
		delete[] Pixels;
		Pixels = NULL;
	}
	if (Palette)
	{
		delete[] Palette;
		Palette = NULL;
	}
	unguard;
}

//==========================================================================
//
//	VRawPicTexture::GetPixels
//
//==========================================================================

vuint8* VRawPicTexture::GetPixels()
{
	guard(VRawPicTexture::GetPixels);
	//	If already got pixels, then just return them.
	if (Pixels)
	{
		return Pixels;
	}

	Pixels = new vuint8[64000];

	//	Set up palette.
	int black;
	if (PalLumpNum < 0)
	{
		black = r_black_colour;
	}
	else
	{
		//	Load palette and find black colour for remaping.
		Palette = new rgba_t[256];
		VStream* PStrm = W_CreateLumpReaderNum(PalLumpNum);
		int best_dist = 0x10000;
		black = 0;
		for (int i = 0; i < 256; i++)
		{
			*PStrm << Palette[i].r
					<< Palette[i].g
					<< Palette[i].b;
			if (i == 0)
			{
				Palette[i].a = 0;
			}
			else
			{
				Palette[i].a = 255;
				int dist = Palette[i].r * Palette[i].r + Palette[i].g *
					Palette[i].g + Palette[i].b * Palette[i].b;
				if (dist < best_dist)
				{
					black = i;
					best_dist = dist;
				}
			}
		}
		delete PStrm;
		PStrm = NULL;
	}

	//	Read data.
	VStream* Strm = W_CreateLumpReaderNum(SourceLump);
	vuint8* dst = Pixels;
	for (int i = 0; i < 64000; i++, dst++)
	{
		*Strm << *dst;
		if (!*dst)
			*dst = black;
	}
	delete Strm;
	Strm = NULL;

	return Pixels;
	unguard;
}

//==========================================================================
//
//	VRawPicTexture::GetPalette
//
//==========================================================================

rgba_t* VRawPicTexture::GetPalette()
{
	guardSlow(VRawPicTexture::GetPalette);
	return Palette ? Palette : r_palette;
	unguardSlow;
}

//==========================================================================
//
//	VRawPicTexture::Unload
//
//==========================================================================

void VRawPicTexture::Unload()
{
	guard(VRawPicTexture::Unload);
	if (Pixels)
	{
		delete[] Pixels;
		Pixels = NULL;
	}
	if (Palette)
	{
		delete[] Palette;
		Palette = NULL;
	}
	unguard;
}
