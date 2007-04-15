//**************************************************************************
//**
//**	##   ##    ##    ##   ##   ####     ####   ###     ###
//**	##   ##  ##  ##  ##   ##  ##  ##   ##  ##  ####   ####
//**	 ## ##  ##    ##  ## ##  ##    ## ##    ## ## ## ## ##
//**	 ## ##  ########  ## ##  ##    ## ##    ## ##  ###  ##
//**	  ###   ##    ##   ###    ##  ##   ##  ##  ##       ##
//**	   #    ##    ##    #      ####     ####   ##       ##
//**
//**	$Id: template.cpp 1583 2006-06-27 19:05:42Z dj_jl $
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
//	VWarpTexture::VWarpTexture
//
//==========================================================================

VWarpTexture::VWarpTexture(VTexture* ASrcTex)
: SrcTex(ASrcTex)
, Pixels(NULL)
, GenTime(0)
, WarpXScale(1.0)
, WarpYScale(1.0)
, XSin1(NULL)
, XSin2(NULL)
, YSin1(NULL)
, YSin2(NULL)
{
	Width = SrcTex->GetWidth();
	Height = SrcTex->GetHeight();
	SOffset = SrcTex->SOffset;
	TOffset = SrcTex->TOffset;
	SScale = SrcTex->SScale;
	TScale = SrcTex->TScale;
	WarpType = 1;
}

//==========================================================================
//
//	VWarpTexture::~VWarpTexture
//
//==========================================================================

VWarpTexture::~VWarpTexture()
{
	guard(VWarpTexture::~VWarpTexture);
	if (Pixels)
	{
		delete[] Pixels;
	}
	if (SrcTex)
	{
		delete SrcTex;
	}
	if (XSin1)
	{
		delete[] XSin1;
	}
	if (XSin2)
	{
		delete[] XSin2;
	}
	if (YSin1)
	{
		delete[] YSin1;
	}
	if (YSin2)
	{
		delete[] YSin2;
	}
	unguard;
}

//==========================================================================
//
//	VWarpTexture::SetFrontSkyLayer
//
//==========================================================================

void VWarpTexture::SetFrontSkyLayer()
{
	guardSlow(VWarpTexture::SetFrontSkyLayer);
	SrcTex->SetFrontSkyLayer();
	unguardSlow;
}

//==========================================================================
//
//	VWarpTexture::CheckModified
//
//==========================================================================

bool VWarpTexture::CheckModified()
{
#ifdef CLIENT
	return GenTime != GClLevel->Time;
#else
	return false;
#endif
}

//==========================================================================
//
//	VWarpTexture::GetPixels
//
//==========================================================================

vuint8* VWarpTexture::GetPixels()
{
	guard(VWarpTexture::GetPixels);
#ifdef CLIENT
	if (Pixels && GenTime == GClLevel->Time)
	{
		return Pixels;
	}

	const vuint8* SrcPixels = SrcTex->GetPixels();
	Format = SrcTex->Format;

	GenTime = GClLevel->Time;
	Pixels8BitValid = false;

	if (!XSin1)
	{
		XSin1 = new float[Width];
		YSin1 = new float[Height];
	}

	//	Precalculate sine values.
	for (int x = 0; x < Width; x++)
	{
		XSin1[x] = msin(GenTime * 44 + x / WarpXScale * 5.625 + 95.625) *
			8 * WarpYScale + 8 * WarpYScale * Height;
	}
	for (int y = 0; y < Height; y++)
	{
		YSin1[y] = msin(GenTime * 50 + y / WarpYScale * 5.625) *
			8 * WarpXScale + 8 * WarpXScale * Width;
	}

	if (Format == TEXFMT_8 || Format == TEXFMT_8Pal)
	{
		if (!Pixels)
		{
			Pixels = new vuint8[Width * Height];
		}

		vuint8* Dst = Pixels;
		for (int y = 0; y < Height; y++)
		{
			for (int x = 0; x < Width; x++)
			{
				*Dst++ = SrcPixels[(((int)YSin1[y] + x) % Width) +
					(((int)XSin1[x] + y) % Height) * Width];
			}
		}
	}
	else
	{
		if (!Pixels)
		{
			Pixels = new vuint8[Width * Height * 4];
		}

		vuint32* Dst = (vuint32*)Pixels;
		for (int y = 0; y < Height; y++)
		{
			for (int x = 0; x < Width; x++)
			{
				*Dst++ = ((vuint32*)SrcPixels)[(((int)YSin1[y] + x) % Width) +
					(((int)XSin1[x] + y) % Height) * Width];
			}
		}
	}

	return Pixels;
#else
	return NULL;
#endif
	unguard;
}

//==========================================================================
//
//	VWarpTexture::GetPalette
//
//==========================================================================

rgba_t* VWarpTexture::GetPalette()
{
	guard(VWarpTexture::GetPalette);
	return SrcTex->GetPalette();
	unguard;
}

//==========================================================================
//
//	VWarpTexture::GetHighResolutionTexture
//
//==========================================================================

VTexture* VWarpTexture::GetHighResolutionTexture()
{
	guard(VWarpTexture::GetHighResolutionTexture);
	if (!r_hirestex)
	{
		return NULL;
	}
	//	If high resolution texture is already created, then just return it.
	if (HiResTexture)
	{
		return HiResTexture;
	}

	VTexture* SrcTex = VTexture::GetHighResolutionTexture();
	if (!SrcTex)
	{
		return NULL;
	}

	VWarpTexture* NewTex;
	if (WarpType == 1)
		NewTex = new VWarpTexture(SrcTex);
	else
		NewTex = new VWarp2Texture(SrcTex);
	NewTex->Name = Name;
	NewTex->Type = Type;
	NewTex->WarpXScale = NewTex->GetWidth() / GetWidth();
	NewTex->WarpYScale = NewTex->GetHeight() / GetHeight();
	HiResTexture = NewTex;
	return HiResTexture;
	unguard;
}

//==========================================================================
//
//	VWarpTexture::Unload
//
//==========================================================================

void VWarpTexture::Unload()
{
	guard(VWarpTexture::Unload);
	if (Pixels)
	{
		delete[] Pixels;
		Pixels = NULL;
	}
	SrcTex->Unload();
	unguard;
}

//==========================================================================
//
//	VWarp2Texture::VWarp2Texture
//
//==========================================================================

VWarp2Texture::VWarp2Texture(VTexture* ASrcTex)
: VWarpTexture(ASrcTex)
{
	WarpType = 2;
}

//==========================================================================
//
//	VWarp2Texture::GetPixels
//
//==========================================================================

vuint8* VWarp2Texture::GetPixels()
{
	guard(VWarp2Texture::GetPixels);
#ifdef CLIENT
	if (Pixels && GenTime == GClLevel->Time)
	{
		return Pixels;
	}

	const vuint8* SrcPixels = SrcTex->GetPixels();
	Format = SrcTex->Format;

	GenTime = GClLevel->Time;
	Pixels8BitValid = false;

	if (!XSin1)
	{
		XSin1 = new float[Height];
		XSin2 = new float[Width];
		YSin1 = new float[Height];
		YSin2 = new float[Width];
	}

	//	Precalculate sine values.
	for (int y = 0; y < Height; y++)
	{
		XSin1[y] = msin(y / WarpYScale * 5.625 + GenTime * 313.895 + 39.55) *
			2 * WarpXScale;
		YSin1[y] = y + (2 * Height + msin(y / WarpYScale * 5.625 + GenTime *
			118.337 + 30.76) * 2) * WarpYScale;
	}
	for (int x = 0; x < Width; x++)
	{
		XSin2[x] = x + (2 * Width + msin(x / WarpXScale * 11.25 + GenTime *
			251.116 + 13.18) * 2) * WarpXScale;
		YSin2[x] = msin(x / WarpXScale * 11.25 + GenTime * 251.116 + 52.73) *
			2 * WarpYScale;
	}

	if (Format == TEXFMT_8 || Format == TEXFMT_8Pal)
	{
		if (!Pixels)
		{
			Pixels = new vuint8[Width * Height];
		}

		vuint8* dest = Pixels;
		for (int y = 0; y < Height; y++)
		{
			for (int x = 0; x < Width; x++)
			{
				*dest++ = SrcPixels[((int)(XSin1[y] + XSin2[x]) % Width) +
					((int)(YSin1[y] + YSin2[x]) % Height) * Width];
			}
		}
	}
	else
	{
		if (!Pixels)
		{
			Pixels = new vuint8[Width * Height * 4];
		}

		vuint32* dest = (vuint32*)Pixels;
		for (int y = 0; y < Height; y++)
		{
			for (int x = 0; x < Width; x++)
			{
				int Idx = ((int)(XSin1[y] + XSin2[x]) % Width) * 4 +
					((int)(YSin1[y] + YSin2[x]) % Height) * Width * 4;
				*dest++ = *(vuint32*)(SrcPixels + Idx);
			}
		}
	}

	return Pixels;
#else
	return NULL;
#endif
	unguard;
}
