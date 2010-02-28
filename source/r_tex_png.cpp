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

#if defined(CLIENT) || !defined(SERVER)
#include <png.h>
#endif

#include "gamedefs.h"
#include "r_tex.h"

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

VTexture* VPngTexture::Create(VStream& Strm, int LumpNum)
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
	vuint32		CRC;
	Strm.SerialiseBigEndian(&Width, 4);
	Strm.SerialiseBigEndian(&Height, 4);
	Strm << BitDepth << ColourType << Compression << Filter << Interlace;
	Strm << CRC;

	//	Scan other chunks looking for grAb chunk with offsets
	vint32 SOffset = 0;
	vint32 TOffset = 0;
	while (Strm.TotalSize() - Strm.Tell() >= 12)
	{
		vuint32 Len;
		Strm.SerialiseBigEndian(&Len, 4);
		Strm.Serialise(Id, 4);
		if (Id[0] == 'g' && Id[1] == 'r' && Id[2] == 'A' && Id[3] == 'b')
		{
			Strm.SerialiseBigEndian(&SOffset, 4);
			Strm.SerialiseBigEndian(&TOffset, 4);
			if (SOffset < -32768 || SOffset > 32767)
			{
				GCon->Logf("S-offset for PNG texture %s is bad: %d (0x%08x)",
					*W_LumpName(LumpNum), SOffset, SOffset);
				SOffset = 0;
			}
			if (TOffset < -32768 || TOffset > 32767)
			{
				GCon->Logf("T-offset for PNG texture %s is bad: %d (0x%08x)",
					*W_LumpName(LumpNum), TOffset, TOffset);
				TOffset = 0;
			}
		}
		else
		{
			Strm.Seek(Strm.Tell() + Len);
		}
		Strm << CRC;
	}

	return new VPngTexture(LumpNum, Width, Height, SOffset, TOffset);
	unguard;
}

//==========================================================================
//
//	VPngTexture::VPngTexture
//
//==========================================================================

VPngTexture::VPngTexture(int ALumpNum, int AWidth, int AHeight, int ASOffset,
	int ATOffset)
: Pixels(0)
{
	SourceLump = ALumpNum;
	Name = W_LumpName(SourceLump);
	Width = AWidth;
	Height = AHeight;
	SOffset = ASOffset;
	TOffset = ATOffset;
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
	VStream* Strm = W_CreateLumpReaderNum(SourceLump);

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
	unguard;
}

//==========================================================================
//
//	WritePNG
//
//==========================================================================

#ifdef CLIENT
void WritePNG(const VStr& FileName, const void* Data, int Width, int Height,
	int Bpp, bool Bot2top)
{
	guard(WritePNG);
	VStream* Strm = FL_OpenFileWrite(FileName);
	if (!Strm)
	{
		GCon->Log("Couldn't write png");
		return;
	}

	//	Create writing structure.
	png_structp png_ptr = png_create_write_struct(PNG_LIBPNG_VER_STRING,
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

	//	Set up error handling.
	if (setjmp(png_jmpbuf(png_ptr)))
	{
		Sys_Error("Error writing PNG file");
	}

	//	Set my read function.
	png_set_write_fn(png_ptr, Strm, ReadFunc, NULL);

	png_set_IHDR(png_ptr, info_ptr, Width, Height, 8,
		Bpp == 8 ? PNG_COLOR_TYPE_PALETTE : PNG_COLOR_TYPE_RGB,
		PNG_INTERLACE_NONE, PNG_COMPRESSION_TYPE_BASE, PNG_FILTER_TYPE_BASE);
	if (Bpp == 8)
	{
		png_set_PLTE(png_ptr, info_ptr, (png_colorp)r_palette, 256);
	}
	png_write_info(png_ptr, info_ptr);

	TArray<png_bytep> RowPointers;
	RowPointers.SetNum(Height);
	for (int i = 0; i < Height; i++)
	{
		RowPointers[i] = ((byte*)Data) + (Bot2top ? Height - i - 1 : i) *
			Width * (Bpp / 8);
	}
	png_write_image(png_ptr, RowPointers.Ptr());

	png_write_end(png_ptr, NULL);
	png_destroy_write_struct(&png_ptr, &info_ptr);

	Strm->Close();
	delete Strm;
	unguard;
}
#endif
