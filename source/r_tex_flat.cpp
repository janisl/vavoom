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
//	VFlatTexture::VFlatTexture
//
//==========================================================================

VTexture* VFlatTexture::Create(VStream&, int LumpNum)
{
	guard(VFlatTexture::Create);
	return new VFlatTexture(LumpNum);
	unguard;
}

//==========================================================================
//
//	VFlatTexture::VFlatTexture
//
//==========================================================================

VFlatTexture::VFlatTexture(int InLumpNum)
: Pixels(0)
{
	guard(VFlatTexture::VFlatTexture);
	SourceLump = InLumpNum;
	Type = TEXTYPE_Flat;
	Format = TEXFMT_8;
	Name = W_LumpName(SourceLump);
	Width = 64;
	//	Check for larger flats.
	while (W_LumpLength(SourceLump) >= Width * Width * 4)
	{
		Width *= 2;
	}
	Height = Width;
	//	Scale large flats to 64x64.
	if (Width > 64)
	{
		SScale = Width / 64;
		TScale = Width / 64;
	}
	unguard;
}

//==========================================================================
//
//	VFlatTexture::~VFlatTexture
//
//==========================================================================

VFlatTexture::~VFlatTexture()
{
	guard(VFlatTexture::~VFlatTexture);
	if (Pixels)
	{
		delete[] Pixels;
	}
	unguard;
}

//==========================================================================
//
//	VFlatTexture::GetPixels
//
//==========================================================================

vuint8* VFlatTexture::GetPixels()
{
	guard(VFlatTexture::GetPixels);
	//	If already got pixels, then just return them.
	if (Pixels)
	{
		return Pixels;
	}

	//	Allocate memory buffer.
	Pixels = new vuint8[Width * Height];

	//	A flat must be at least 64x64, if it's smaller, then ignore it.
	if (W_LumpLength(SourceLump) < 64 * 64)
	{
		memset(Pixels, 0, 64 * 64);
		return Pixels;
	}

	//	Read data.
	VStream* Strm = W_CreateLumpReaderNum(SourceLump);
	for (int i = 0; i < Width * Height; i++)
	{
		*Strm << Pixels[i];
		if (!Pixels[i])
			Pixels[i] = r_black_colour;
	}
	delete Strm;

	return Pixels;
	unguard;
}

//==========================================================================
//
//	VFlatTexture::Unload
//
//==========================================================================

void VFlatTexture::Unload()
{
	guard(VFlatTexture::Unload);
	if (Pixels)
	{
		delete[] Pixels;
		Pixels = NULL;
	}
	unguard;
}
