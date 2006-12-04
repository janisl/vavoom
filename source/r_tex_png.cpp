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

#if defined(CLIENT) || !defined(SERVER)
#include <png.h>
#endif
#include "gamedefs.h"
#include "r_local.h"

// MACROS ------------------------------------------------------------------

//	This one is missing in older versions of libpng
#ifndef png_jmpbuf
#define png_jmpbuf(png_ptr)		((png_ptr)->jmpbuf)
#endif

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
//	VPngTexture::Create
//
//==========================================================================

VTexture* VPngTexture::Create(VStream& Strm, int LumpNum, VName Name)
{
	guard(VPngTexture::Create);
	if (Strm.TotalSize() < 29)
	{
		//	File is too small.
		return NULL;
	}

	vuint8		Id[8];

	//	Verify signature.
	Strm.Seek(0);
	Strm.Serialise(Id, 8);
	if (Id[0] != 137 || Id[1] != 'P' || Id[2] != 'N' || Id[3] != 'G' ||
		Id[4] != 13 || Id[5] != 10 || Id[6] != 26 || Id[7] != 10)
	{
		//	Not a PNG file.
		return NULL;
	}

	//	Make sure it's followed by an image header.
	Strm.Serialise(Id, 8);
	if (Id[0] != 0 || Id[1] != 0 || Id[2] != 0 || Id[3] != 13 ||
		Id[4] != 'I' || Id[5] != 'H' || Id[6] != 'D' || Id[7] != 'R')
	{
		//	Assume it's a corupted file.
		return NULL;
	}

	//	Read image info.
	vint32		Width;
	vint32		Height;
	vuint8		BitDepth;
	vuint8		ColourType;
	vuint8		Compression;
	vuint8		Filter;
	vuint8		Interlace;
	Strm.SerialiseBigEndian(&Width, 4);
	Strm.SerialiseBigEndian(&Height, 4);
	Strm << BitDepth << ColourType << Compression << Filter << Interlace;

	return new VPngTexture(LumpNum, Name, Width, Height);
	unguard;
}

//==========================================================================
//
//	VPngTexture::VPngTexture
//
//==========================================================================

VPngTexture::VPngTexture(int ALumpNum, VName AName, int AWidth, int AHeight)
: LumpNum(ALumpNum)
, Pixels(0)
, Palette(0)
{
	Name = LumpNum >= 0 ? W_LumpName(LumpNum) : AName;
	Width = AWidth;
	Height = AHeight;
}

//==========================================================================
//
//	VPngTexture::~VPngTexture
//
//==========================================================================

VPngTexture::~VPngTexture()
{
	guard(VPngTexture::~VPngTexture);
	if (Pixels)
	{
		delete[] Pixels;
	}
	if (Palette)
	{
		delete[] Palette;
	}
	unguard;
}

//==========================================================================
//
//	ReadFunc
//
//==========================================================================

#ifdef CLIENT
static void ReadFunc(png_structp png, png_bytep data, png_size_t len)
{
	guard(ReadFunc);
	VStream* Strm = (VStream*)png_get_io_ptr(png);
	Strm->Serialise(data, len);
	unguard;
}
#endif

//==========================================================================
//
//	VPngTexture::GetPixels
//
//==========================================================================

vuint8* VPngTexture::GetPixels()
{
	guard(VPngTexture::GetPixels);
#ifdef CLIENT
	//	If we already have loaded pixels, return them.
	if (Pixels)
	{
		return Pixels;
	}

	//	Create reading structure.
	png_structp png_ptr = png_create_read_struct(PNG_LIBPNG_VER_STRING,
		NULL, NULL, NULL);
	if (!png_ptr)
	{
		Sys_Error("Couldn't create png_ptr");
	}

	//	Create info structure.
	png_infop info_ptr = png_create_info_struct(png_ptr);
	if (!info_ptr)
	{
		Sys_Error("Couldn't create info_ptr");
	}

	//	Create end info structure.
	png_infop end_info = png_create_info_struct(png_ptr);
	if (!end_info)
	{
		Sys_Error("Couldn't create end_info");
	}

	//	Set up error handling.
	if (setjmp(png_jmpbuf(png_ptr)))
	{
		Sys_Error("Error reading PNG file");
	}

	//	Open stream.
	VStream* Strm;
	if (LumpNum != -1)
	{
		Strm = W_CreateLumpReaderNum(LumpNum);
	}
	else
	{
		Strm = FL_OpenFileRead(*Name);
	}

	//	Verify signature.
	png_byte Signature[8];
	Strm->Seek(0);
	Strm->Serialise(Signature, 8);
	if (png_sig_cmp(Signature, 0, 8))
	{
		Sys_Error("%s is not a valid PNG file", *Name);
	}

	//	Set my read function.
	Strm->Seek(0);
	png_set_read_fn(png_ptr, Strm, ReadFunc);

	//	Read image info.
	png_read_info(png_ptr, info_ptr);
	Width = png_get_image_width(png_ptr, info_ptr);
	Height = png_get_image_height(png_ptr, info_ptr);
	int BitDepth = png_get_bit_depth(png_ptr, info_ptr);
	int ColourType = png_get_color_type(png_ptr, info_ptr);

	//	Set up transformations.
	if (ColourType == PNG_COLOR_TYPE_PALETTE)
	{
		png_set_palette_to_rgb(png_ptr);
	}
	if (ColourType == PNG_COLOR_TYPE_GRAY && BitDepth < 8)
	{
		png_set_gray_1_2_4_to_8(png_ptr);
	}
	if (png_get_valid(png_ptr, info_ptr, PNG_INFO_tRNS))
	{
		png_set_tRNS_to_alpha(png_ptr);
	}
	if (BitDepth == 16)
	{
		png_set_strip_16(png_ptr);
	}
	if (ColourType == PNG_COLOR_TYPE_PALETTE ||
		ColourType == PNG_COLOR_TYPE_RGB ||
		ColourType == PNG_COLOR_TYPE_GRAY)
	{
		png_set_filler(png_ptr, 0xff, PNG_FILLER_AFTER);
	}
	if (ColourType == PNG_COLOR_TYPE_GRAY ||
		ColourType == PNG_COLOR_TYPE_GRAY_ALPHA)
	{
		png_set_gray_to_rgb(png_ptr);
	}

	//	Set up unpacking buffer and row pointers.
	Format = TEXFMT_RGBA;
	Pixels = new vuint8[Width * Height * 4];
	png_bytep* RowPtrs = new png_bytep[Height];
	for (int i = 0; i < Height; i++)
	{
		RowPtrs[i] = Pixels + i * Width * 4;
	}
	png_read_image(png_ptr, RowPtrs);

	//	Finish reading.
	png_read_end(png_ptr, end_info);
	png_destroy_read_struct(&png_ptr, &info_ptr, &end_info);

	delete[] RowPtrs;

	//	Free memory.
	delete Strm;
	return Pixels;
#else
	Sys_Error("ReadPixels on dedicated server");
	return NULL;
#endif
	unguard;
}

//==========================================================================
//
//	VPngTexture::GetPalette
//
//==========================================================================

rgba_t* VPngTexture::GetPalette()
{
	guardSlow(VPngTexture::GetPalette);
	return Palette;
	unguardSlow;
}

//==========================================================================
//
//	VPngTexture::Unload
//
//==========================================================================

void VPngTexture::Unload()
{
	guard(VPngTexture::Unload);
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