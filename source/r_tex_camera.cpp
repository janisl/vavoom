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
, bNeedsUpdate(true)
, bUpdated(false)
{
	Name = AName;
	Type = TEXTYPE_Wall;
	Format = TEXFMT_RGBA;
	Width = AWidth;
	Height = AHeight;
	bIsCameraTexture = true;
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
//	VCameraTexture::CheckModified
//
//==========================================================================

bool VCameraTexture::CheckModified()
{
	guard(VCameraTexture::CheckModified);
	if (bUpdated)
	{
		bUpdated = false;
		return true;
	}
	return false;
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
	bNeedsUpdate = true;
	//	If already got pixels, then just return them.
	if (Pixels)
	{
		return Pixels;
	}

	//	Allocate image data.
	Pixels = new vuint8[Width * Height * 4];

	rgba_t* pDst = (rgba_t*)Pixels;
	for (int i = 0; i < Height; i++)
	{
		for (int j = 0; j < Width; j++)
		{
			if (j < Width / 2)
			{
				pDst->r = 0;
				pDst->g = 0;
				pDst->b = 0;
				pDst->a = 255;
			}
			else
			{
				pDst->r = 255;
				pDst->g = 255;
				pDst->b = 255;
				pDst->a = 255;
			}
			pDst++;
		}
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

//==========================================================================
//
//	VCameraTexture::CopyImage
//
//==========================================================================

#ifdef CLIENT
void VCameraTexture::CopyImage()
{
	guard(VCameraTexture::CopyImage);
	if (!Pixels)
	{
		Pixels = new vuint8[Width * Height * 4];
	}

	Drawer->ReadBackScreen(Width, Height, (rgba_t*)Pixels);
	bNeedsUpdate = false;
	bUpdated = true;
	Pixels8BitValid = false;
	unguard;
}
#endif

//==========================================================================
//
//	VCameraTexture::GetHighResolutionTexture
//
//==========================================================================

VTexture* VCameraTexture::GetHighResolutionTexture()
{
	return NULL;
}
