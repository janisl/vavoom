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
//	VCameraTexture::VCameraTexture
//
//==========================================================================

VCameraTexture::VCameraTexture(VName AName, int AWidth, int AHeight)
: Pixels(0)
{
	Name = AName;
	Type = TEXTYPE_Wall;
	Format = TEXFMT_8;
	Width = AWidth;
	Height = AHeight;
}

//==========================================================================
//
//	VCameraTexture::~VCameraTexture
//
//==========================================================================

VCameraTexture::~VCameraTexture()
{
	guard(VCameraTexture::~VCameraTexture);
	if (Pixels)
	{
		delete[] Pixels;
	}
	unguard;
}

//==========================================================================
//
//	VCameraTexture::GetPixels
//
//==========================================================================

vuint8* VCameraTexture::GetPixels()
{
	guard(VCameraTexture::GetPixels);
	//	If already got pixels, then just return them.
	if (Pixels)
	{
		return Pixels;
	}

	//	Allocate image data.
	Pixels = new vuint8[Width * Height];

	for (int i = 0; i < Height; i++)
	{
		memset(Pixels + i * Width, r_black_colour, Width / 2);
		memset(Pixels + i * Width + Width / 2, 255, Width / 2);
	}

	return Pixels;
	unguard;
}

//==========================================================================
//
//	VCameraTexture::Unload
//
//==========================================================================

void VCameraTexture::Unload()
{
	guard(VCameraTexture::Unload);
	if (Pixels)
	{
		delete[] Pixels;
		Pixels = NULL;
	}
	unguard;
}
