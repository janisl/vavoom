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
//	VAutopageTexture::VAutopageTexture
//
//==========================================================================

VTexture* VAutopageTexture::Create(VStream& Strm, int LumpNum)
{
	guard(VAutopageTexture::Create);
	if (Strm.TotalSize() < 320)
	{
		return NULL;
	}

	return new VAutopageTexture(LumpNum);
	unguard;
}

//==========================================================================
//
//	VAutopageTexture::VAutopageTexture
//
//==========================================================================

VAutopageTexture::VAutopageTexture(int ALumpNum)
: LumpNum(ALumpNum)
, Pixels(0)
{
	Name = W_LumpName(LumpNum);
	Width = 320;
	Height = W_LumpLength(LumpNum) / 320;
	Format = TEXFMT_8;
}

//==========================================================================
//
//	VAutopageTexture::~VAutopageTexture
//
//==========================================================================

VAutopageTexture::~VAutopageTexture()
{
	guard(VAutopageTexture::~VAutopageTexture);
	if (Pixels)
	{
		delete[] Pixels;
	}
	unguard;
}

//==========================================================================
//
//	VAutopageTexture::GetPixels
//
//==========================================================================

vuint8* VAutopageTexture::GetPixels()
{
	guard(VAutopageTexture::GetPixels);
	//	If already got pixels, then just return them.
	if (Pixels)
	{
		return Pixels;
	}

	//	Read data.
	VStream* Strm = W_CreateLumpReaderNum(LumpNum);
	int len = Strm->TotalSize();
	Pixels = new vuint8[len];
	vuint8* dst = Pixels;
	for (int i = 0; i < len; i++, dst++)
	{
		*Strm << *dst;
		if (!*dst)
		{
			*dst = r_black_colour;
		}
	}
	delete Strm;

	return Pixels;
	unguard;
}

//==========================================================================
//
//	VAutopageTexture::Unload
//
//==========================================================================

void VAutopageTexture::Unload()
{
	guard(VAutopageTexture::Unload);
	if (Pixels)
	{
		delete[] Pixels;
		Pixels = NULL;
	}
	unguard;
}
