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

// EXTERNAL FUNCTION PROTOTYPES --------------------------------------------

// PUBLIC FUNCTION PROTOTYPES ----------------------------------------------

// PRIVATE FUNCTION PROTOTYPES ---------------------------------------------

// EXTERNAL DATA DECLARATIONS ----------------------------------------------

// PUBLIC DATA DEFINITIONS -------------------------------------------------

// PRIVATE DATA DEFINITIONS ------------------------------------------------

// CODE --------------------------------------------------------------------
//==========================================================================
//
//	VImgzTexture::Create
//
//==========================================================================

VTexture* VImgzTexture::Create(VStream& Strm, int LumpNum)
{
	guard(VImgzTexture::Create);
	if (Strm.TotalSize() < 24)
	{
		//	Not enough space for IMGZ header.
		return NULL;
	}

	vuint8 Id[4];
	vuint16 Width;
	vuint16 Height;
	vuint16 SOffset;
	vuint16 TOffset;

	Strm.Seek(0);
	Strm.Serialise(Id, 4);
	if (Id[0] != 'I' || Id[1] != 'M' || Id[2] != 'G' || Id[3] == 'Z')
	{
		return NULL;
	}

	Strm << Width << Height << SOffset << TOffset;
	return new VImgzTexture(LumpNum, Width, Height, SOffset, TOffset);
	unguard;
}

//==========================================================================
//
//	VImgzTexture::VImgzTexture
//
//==========================================================================

VImgzTexture::VImgzTexture(int ALumpNum, int AWidth, int AHeight,
	int ASOffset, int ATOffset)
: LumpNum(ALumpNum)
, Pixels(0)
{
	Name = W_LumpName(LumpNum);
	Format = TEXFMT_8;
	Width = AWidth;
	Height = AHeight;
	SOffset = ASOffset;
	TOffset = ATOffset;
}

//==========================================================================
//
//	VImgzTexture::~VImgzTexture
//
//==========================================================================

VImgzTexture::~VImgzTexture()
{
	guard(VImgzTexture::~VImgzTexture);
	if (Pixels)
	{
		delete[] Pixels;
	}
	unguard;
}

//==========================================================================
//
//	VImgzTexture::GetPixels
//
//==========================================================================

vuint8* VImgzTexture::GetPixels()
{
	guard(VImgzTexture::GetPixels);
	//	If already got pixels, then just return them.
	if (Pixels)
	{
		return Pixels;
	}

	VStream* Strm = W_CreateLumpReaderNum(LumpNum);

	//	Read header.
	Strm->Seek(4);	//	Skip magic.
	Width = Streamer<vuint16>(*Strm);
	Height = Streamer<vuint16>(*Strm);
	SOffset = Streamer<vint16>(*Strm);
	TOffset = Streamer<vint16>(*Strm);
	vuint8 Compression = Streamer<vuint8>(*Strm);
	Strm->Seek(24);	//	Skip reserved space.

	//	Read data.
	Pixels = new vuint8[Width * Height];
	memset(Pixels, 0, Width * Height);
	if (!Compression)
	{
		Strm->Serialise(Pixels, Width * Height);
	}
	else
	{
		//	IMGZ compression is the same RLE used by IFF ILBM files
		vuint8* pDst = Pixels;
		int runlen = 0, setlen = 0;
		vuint8 setval = 0;  // Shut up, GCC

		for (int y = Height; y != 0; --y)
		{
			for (int x = Width; x != 0; )
			{
				if (runlen != 0)
				{
					*Strm << *pDst;
					pDst++;
					x--;
					runlen--;
				}
				else if (setlen != 0)
				{
					*pDst = setval;
					pDst++;
					x--;
					setlen--;
				}
				else
				{
					vint8 code;
					*Strm << code;
					if (code >= 0)
					{
						runlen = code + 1;
					}
					else if (code != -128)
					{
						setlen = (-code) + 1;
						*Strm << setval;
					}
				}
			}
		}
	}
	delete Strm;

	return Pixels;
	unguard;
}

//==========================================================================
//
//	VImgzTexture::Unload
//
//==========================================================================

void VImgzTexture::Unload()
{
	guard(VImgzTexture::Unload);
	if (Pixels)
	{
		delete[] Pixels;
		Pixels = NULL;
	}
	unguard;
}
