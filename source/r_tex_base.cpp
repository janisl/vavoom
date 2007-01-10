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
#include "r_local.h"

// MACROS ------------------------------------------------------------------

// TYPES -------------------------------------------------------------------

typedef VTexture* (*VTexCreateFunc)(VStream&, int);

// EXTERNAL FUNCTION PROTOTYPES --------------------------------------------

// PUBLIC FUNCTION PROTOTYPES ----------------------------------------------

// PRIVATE FUNCTION PROTOTYPES ---------------------------------------------

// EXTERNAL DATA DECLARATIONS ----------------------------------------------

// PUBLIC DATA DEFINITIONS -------------------------------------------------

// PRIVATE DATA DEFINITIONS ------------------------------------------------

// CODE --------------------------------------------------------------------

//==========================================================================
//
//	VTexture::CreateTexture
//
//==========================================================================

VTexture* VTexture::CreateTexture(int Type, int LumpNum)
{
	guard(VTexture::CreateTexture);
	static const struct
	{
		VTexCreateFunc	Create;
		int				Type;
	} TexTable[] =
	{
		{ VImgzTexture::Create,		TEXTYPE_Any },
		{ VPngTexture::Create,		TEXTYPE_Any },
		{ VJpegTexture::Create,		TEXTYPE_Any },
		{ VPcxTexture::Create,		TEXTYPE_Any },
		{ VTgaTexture::Create,		TEXTYPE_Any },
		{ VFlatTexture::Create,		TEXTYPE_Flat },
		{ VRawPicTexture::Create,	TEXTYPE_Pic },
		{ VPatchTexture::Create,	TEXTYPE_Any },
		{ VAutopageTexture::Create,	TEXTYPE_Autopage },
	};

	if (LumpNum < 0)
	{
		return NULL;
	}
	VStream* Strm = W_CreateLumpReaderNum(LumpNum);

	for (size_t i = 0; i < ARRAY_COUNT(TexTable); i++)
	{
		if (TexTable[i].Type == Type || TexTable[i].Type == TEXTYPE_Any)
		{
			VTexture* Tex = TexTable[i].Create(*Strm, LumpNum);
			if (Tex)
			{
				Tex->Type = Type;
				delete Strm;
				return Tex;
			}
		}
	}

	delete Strm;
	return NULL;
	unguard;
}

//==========================================================================
//
//	VTexture::VTexture
//
//==========================================================================

VTexture::VTexture()
: Type(TEXTYPE_Any)
, Format(TEXFMT_8)
, Name(NAME_None)
, Width(0)
, Height(0)
, SOffset(0)
, TOffset(0)
, bNoRemap0(false)
, bWorldPanning(false)
, WarpType(0)
, SScale(1)
, TScale(1)
, TextureTranslation(0)
, DriverData(0)
, Pixels8Bit(0)
, HiResTexture(0)
, Pixels8BitValid(false)
{
}

//==========================================================================
//
//	VTexture::~VTexture
//
//==========================================================================

VTexture::~VTexture()
{
	if (Pixels8Bit)
	{
		delete[] Pixels8Bit;
	}
	if (HiResTexture)
	{
		delete HiResTexture;
	}
}

//==========================================================================
//
//	VTexture::SetFrontSkyLayer
//
//==========================================================================

void VTexture::SetFrontSkyLayer()
{
	guardSlow(VTexture::SetFrontSkyLayer);
	bNoRemap0 = true;
	unguardSlow;
}

//==========================================================================
//
//	VTexture::CheckModified
//
//==========================================================================

bool VTexture::CheckModified()
{
	return false;
}

//==========================================================================
//
//	VTexture::GetPixels8
//
//==========================================================================

vuint8* VTexture::GetPixels8()
{
	guard(VTexture::GetPixels8);
	//	If already have converted version, then just return it.
	if (Pixels8Bit && Pixels8BitValid)
	{
		return Pixels8Bit;
	}

	vuint8* Pixels = GetPixels();
	if (Format == TEXFMT_8Pal)
	{
		//	Remap to game palette
		int NumPixels = Width * Height;
		rgba_t* Pal = GetPalette();
		vuint8 Remap[256];
		Remap[0] = 0;
		int i;
		for (i = 1; i < 256; i++)
		{
			Remap[i] = r_rgbtable[((Pal[i].r << 7) & 0x7c00) +
				((Pal[i].g << 2) & 0x3e0) + ((Pal[i].b >> 3) & 0x1f)];
		}

		if (!Pixels8Bit)
		{
			Pixels8Bit = new vuint8[NumPixels];
		}
		vuint8* pSrc = Pixels;
		vuint8* pDst = Pixels8Bit;
		for (i = 0; i < NumPixels; i++, pSrc++, pDst++)
		{
			*pDst = Remap[*pSrc];
		}
		Pixels8BitValid = true;
		return Pixels8Bit;
	}
	else if (Format == TEXFMT_RGBA)
	{
		int NumPixels = Width * Height;
		if (!Pixels8Bit)
		{
			Pixels8Bit = new vuint8[NumPixels];
		}
		rgba_t* pSrc = (rgba_t*)Pixels;
		vuint8* pDst = Pixels8Bit;
		for (int i = 0; i < NumPixels; i++, pSrc++, pDst++)
		{
			if (pSrc->a < 128)
				*pDst = 0;
			else
				*pDst = r_rgbtable[((pSrc->r << 7) & 0x7c00) +
					((pSrc->g << 2) & 0x3e0) + ((pSrc->b >> 3) & 0x1f)];
		}
		Pixels8BitValid = true;
		return Pixels8Bit;
	}
	return Pixels;
	unguard;
}

//==========================================================================
//
//	VTexture::GetPalette
//
//==========================================================================

rgba_t* VTexture::GetPalette()
{
	guardSlow(VTexture::GetPalette);
	return r_palette;
	unguardSlow;
}

//==========================================================================
//
//	VTexture::GetHighResolutionTexture
//
//	Return high-resolution version of this texture, or self if it doesn't
// exist.
//
//==========================================================================

VTexture* VTexture::GetHighResolutionTexture()
{
	guard(VTexture::GetHighResolutionTexture);
#ifdef CLIENT
	if (!r_hirestex)
	{
		return NULL;
	}
	//	If high resolution texture is already created, then just return it.
	if (HiResTexture)
	{
		return HiResTexture;
	}

	//	Determine directory name depending on type.
	const char* DirName;
	switch (Type)
	{
	case TEXTYPE_Wall:
		DirName = "walls";
		break;
	case TEXTYPE_Flat:
		DirName = "flats";
		break;
	case TEXTYPE_Overload:
		DirName = "textures";
		break;
	case TEXTYPE_Sprite:
		DirName = "sprites";
		break;
	case TEXTYPE_Pic:
	case TEXTYPE_Autopage:
		DirName = "graphics";
		break;
	default:
		return NULL;
	}

	//	Try to find it.
	static const char* Exts[] = { "png", "jpg", "tga", NULL };
	int LumpNum = W_FindLumpByFileNameWithExts(VStr("hirestex/") + DirName +
		"/" + *Name, Exts);
	if (LumpNum >= 0)
	{
		//	Create new high-resolution texture.
		HiResTexture = CreateTexture(Type, LumpNum);
		HiResTexture->Name = Name;
		return HiResTexture;
	}
#endif
	//	No hi-res texture found.
	return NULL;
	unguard;
}

//==========================================================================
//
//	VTexture::FixupPalette
//
//==========================================================================

void VTexture::FixupPalette(vuint8* Pixels, rgba_t* Palette)
{
	guard(VTexture::FixupPalette);
	//	Find black colour for remaping.
	int i;
	int black = 0;
	int best_dist = 0x10000;
	for (i = 1; i < 256; i++)
	{
		int dist = Palette[i].r * Palette[i].r + Palette[i].g *
				Palette[i].g + Palette[i].b * Palette[i].b;
		if (dist < best_dist && Palette[i].a == 255)
		{
			black = i;
			best_dist = dist;
		}
	}
	for (i = 0; i < Width * Height; i++)
	{
		if (Palette[Pixels[i]].a == 0)
			Pixels[i] = 0;
		else if (!Pixels[i])
			Pixels[i] = black;
	}
	Palette[0].r = 0;
	Palette[0].g = 0;
	Palette[0].b = 0;
	Palette[0].a = 0;
	unguard;
}

//==========================================================================
//
//	VDummyTexture::VDummyTexture
//
//==========================================================================

VDummyTexture::VDummyTexture()
{
	Type = TEXTYPE_Null;
	Format = TEXFMT_8;
}

//==========================================================================
//
//	VDummyTexture::GetPixels
//
//==========================================================================

vuint8* VDummyTexture::GetPixels()
{
	return NULL;
}

//==========================================================================
//
//	VDummyTexture::Unload
//
//==========================================================================

void VDummyTexture::Unload()
{
}
