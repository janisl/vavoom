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

struct tgaHeader_t
{
	vuint8		id_length;
	vuint8		pal_type;
	vuint8		img_type;
	vuint16		first_colour;
	vuint16		pal_colours;
	vuint8		pal_entry_size;
	vuint16		left;
	vuint16		top;
	vuint16		width;
	vuint16		height;
	vuint8		bpp;
	vuint8		descriptor_bits;

	friend VStream& operator<<(VStream& Strm, tgaHeader_t& h)
	{
		return Strm << h.id_length << h.pal_type << h.img_type
			<< h.first_colour << h.pal_colours << h.pal_entry_size << h.left
			<< h.top << h.width << h.height << h.bpp << h.descriptor_bits;
	}
};

// EXTERNAL FUNCTION PROTOTYPES --------------------------------------------

// PUBLIC FUNCTION PROTOTYPES ----------------------------------------------

// PRIVATE FUNCTION PROTOTYPES ---------------------------------------------

// EXTERNAL DATA DECLARATIONS ----------------------------------------------

// PUBLIC DATA DEFINITIONS -------------------------------------------------

// PRIVATE DATA DEFINITIONS ------------------------------------------------

// CODE --------------------------------------------------------------------

//==========================================================================
//
//	VTgaTexture::Create
//
//==========================================================================

VTexture* VTgaTexture::Create(VStream& Strm, int LumpNum)
{
	guard(VTgaTexture::Create);
	if (Strm.TotalSize() < 18)
	{
		//	File is too small.
		return NULL;
	}

	tgaHeader_t Hdr;
	Strm.Seek(0);
	Strm << Hdr;

	if ((Hdr.pal_type != 0 && Hdr.pal_type != 1) || Hdr.width <= 0 ||
		Hdr.height <= 0 || Hdr.width > 2048 || Hdr.height > 2048 ||
		(Hdr.pal_type == 0 && Hdr.bpp != 15 && Hdr.bpp != 16 &&
		Hdr.bpp != 24 && Hdr.bpp != 32) ||
		(Hdr.pal_type == 1 && Hdr.bpp != 8) ||
		(Hdr.pal_type == 0 && Hdr.img_type != 2 && Hdr.img_type != 10) ||
		(Hdr.pal_type == 1 && Hdr.img_type != 1 && Hdr.img_type != 3 &&
		Hdr.img_type != 9 && Hdr.img_type != 11) ||
		(Hdr.pal_type == 1 && Hdr.pal_entry_size != 16 &&
		Hdr.pal_entry_size != 24 && Hdr.pal_entry_size != 32) ||
		(Hdr.descriptor_bits & 16) != 0)
	{
		return NULL;
	}
	return new VTgaTexture(LumpNum, Hdr);
	unguard;
}

//==========================================================================
//
//	VTgaTexture::VTgaTexture
//
//==========================================================================

VTgaTexture::VTgaTexture(int ALumpNum, tgaHeader_t& Hdr)
: LumpNum(ALumpNum)
, Pixels(0)
, Palette(0)
{
	Name = W_LumpName(LumpNum);
	Width = Hdr.width;
	Height = Hdr.height;
}

//==========================================================================
//
//	VTgaTexture::~VTgaTexture
//
//==========================================================================

VTgaTexture::~VTgaTexture()
{
	guard(VTgaTexture::~VTgaTexture);
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
//	VTgaTexture::GetPixels
//
//==========================================================================

vuint8* VTgaTexture::GetPixels()
{
	guard(VTgaTexture::GetPixels);
	//	If we already have loaded pixels, return them.
	if (Pixels)
	{
		return Pixels;
	}

	//	Load texture.
	int			count;
	int			c;

	VStream* Strm = LumpNum >= 0 ? W_CreateLumpReaderNum(LumpNum) :
		FL_OpenFileRead(*Name);
	if (!Strm)
	{
		Sys_Error("Couldn't find file %s", *Name);
	}

	tgaHeader_t hdr;
	*Strm << hdr;

	Width = hdr.width;
	Height = hdr.height;

	Strm->Seek(Strm->Tell() + hdr.id_length);

	if (hdr.pal_type == 1)
	{
		Palette = new rgba_t[256];
		for (int i = 0; i < hdr.pal_colours; i++)
		{
			vuint16 col;
			switch (hdr.pal_entry_size)
			{
			case 16:
				*Strm << col;
				Palette[i].r = (col & 0x1F) << 3;
				Palette[i].g = ((col >> 5) & 0x1F) << 3;
				Palette[i].b = ((col >> 10) & 0x1F) << 3;
				Palette[i].a = 255;
				break;
			case 24:
				*Strm << Palette[i].b
					<< Palette[i].g
					<< Palette[i].r;
				Palette[i].a = 255;
				break;
			case 32:
				*Strm << Palette[i].b
					<< Palette[i].g
					<< Palette[i].r
					<< Palette[i].a;
				break;
			}
		}
	}

	/* Image type:
	*    0 = no image data
	*    1 = uncompressed colour mapped
	*    2 = uncompressed true colour
	*    3 = grayscale
	*    9 = RLE colour mapped
	*   10 = RLE true colour
	*   11 = RLE grayscale
	*/

	if (hdr.img_type == 1 || hdr.img_type == 3 ||
		hdr.img_type == 9 || hdr.img_type == 11)
	{
		Format = TEXFMT_8Pal;
		Pixels = new vuint8[Width * Height];
	}
	else
	{
		Format = TEXFMT_RGBA;
		Pixels = new vuint8[Width * Height * 4];
	}

	if (hdr.img_type == 1 && hdr.bpp == 8 && hdr.pal_type == 1)
	{
		// 8-bit, uncompressed
		for (int y = Height; y; y--)
		{
			int yc = hdr.descriptor_bits & 0x20 ? Height - y : y - 1;
			vuint8* dst = Pixels + yc * Width;

			Strm->Serialise(dst, Width);
		}
	}
	else if (hdr.img_type == 2 && hdr.pal_type == 0 && hdr.bpp == 16)
	{
		// 16-bit uncompressed
		for (int y = Height; y; y--)
		{
			int yc = hdr.descriptor_bits & 0x20 ? Height - y : y - 1;
			rgba_t *dst = (rgba_t*)(Pixels + yc * Width * 4);

			for (int x = 0; x < Width; x++, dst++)
			{
				vuint16 col;
				*Strm << col;
				dst->r = ((col >> 10) & 0x1F) << 3;
				dst->g = ((col >> 5) & 0x1F) << 3;
				dst->b = (col & 0x1F) << 3;
				dst->a = 255;
			}
		}
	}
	else if (hdr.img_type == 2 && hdr.pal_type == 0 && hdr.bpp == 24)
	{
		// 24-bit uncompressed
		for (int y = Height; y; y--)
		{
			int yc = hdr.descriptor_bits & 0x20 ? Height - y : y - 1;
			rgba_t *dst = (rgba_t*)(Pixels + yc * Width * 4);

			for (int x = 0; x < Width; x++, dst++)
			{
				*Strm << dst->b
					<< dst->g
					<< dst->r;
				dst->a = 255;
			}
		}
	}
	else if (hdr.img_type == 2 && hdr.pal_type == 0 && hdr.bpp == 32)
	{
		// 32-bit uncompressed
		for (int y = Height; y; y--)
		{
			int yc = hdr.descriptor_bits & 0x20 ? Height - y : y - 1;
			rgba_t *dst = (rgba_t*)(Pixels + yc * Width * 4);

			for (int x = 0; x < Width; x++, dst++)
			{
				*Strm << dst->b
					<< dst->g
					<< dst->r
					<< dst->a;
			}
		}
	}
	else if (hdr.img_type == 3 && hdr.bpp == 8 && hdr.pal_type == 1)
	{
		// Grayscale uncompressed
		for (int i = 0; i < 256; i++)
		{
			Palette[i].r = i;
			Palette[i].g = i;
			Palette[i].b = i;
			Palette[i].a = 255;
		}
		for (int y = Height; y; y--)
		{
			int yc = hdr.descriptor_bits & 0x20 ? Height - y : y - 1;
			byte *dst = Pixels + yc * Width;

			Strm->Serialise(dst, Width);
		}
	}
	else if (hdr.img_type == 9 && hdr.bpp == 8 && hdr.pal_type == 1)
	{
		// 8-bit RLE compressed
		for (int y = Height; y; y--)
		{
			int yc = hdr.descriptor_bits & 0x20 ? Height - y : y - 1;
			vuint8* dst = Pixels + yc * Width;
			c = 0;

			do
			{
				count = Streamer<vuint8>(*Strm);
				if (count & 0x80)
				{
					count = (count & 0x7F) + 1;
					c += count;
					vuint8 col;
					*Strm << col;
					while (count--)
						*(dst++) = col;
				}
				else
				{
					count++;
					c += count;
					Strm->Serialise(dst, count);
					dst += count;
				}
			}
			while (c < Width);
		}
	}
	else if (hdr.img_type == 10 && hdr.pal_type == 0 && hdr.bpp == 16)
	{
		// 16-bit RLE compressed
		for (int y = Height; y; y--)
		{
			int yc = hdr.descriptor_bits & 0x20 ? Height - y : y - 1;
			rgba_t *dst = (rgba_t*)(Pixels + yc * Width * 4);
			c = 0;

			do
			{
				count = Streamer<vuint8>(*Strm);
				if (count & 0x80)
				{
					count = (count & 0x7F) + 1;
					c += count;
					vuint16 col;
					*Strm << col;
					while (count--)
					{
						dst->r = ((col >> 10) & 0x1F) << 3;
						dst->g = ((col >> 5) & 0x1F) << 3;
						dst->b = (col & 0x1F) << 3;
						dst->a = 255;
						dst++;
					}
				}
				else
				{
					count++;
					c += count;
					while (count--)
					{
						vuint16 col;
						*Strm << col;
						dst->r = ((col >> 10) & 0x1F) << 3;
						dst->g = ((col >> 5) & 0x1F) << 3;
						dst->b = (col & 0x1F) << 3;
						dst->a = 255;
						dst++;
					}
				}
			}
			while (c < Width);
		}
	}
	else if (hdr.img_type == 10 && hdr.pal_type == 0 && hdr.bpp == 24)
	{
		// 24-bit REL compressed
		for (int y = Height; y; y--)
		{
			int yc = hdr.descriptor_bits & 0x20 ? Height - y : y - 1;
			rgba_t *dst = (rgba_t*)(Pixels + yc * Width * 4);
			c = 0;

			do
			{
				count = Streamer<vuint8>(*Strm);
				if (count & 0x80)
				{
					count = (count & 0x7F) + 1;
					c += count;
					rgba_t col;
					*Strm << col.b << col.g << col.r;
					col.a = 255;
					while (count--)
					{
						*dst = col;
						dst++;
					}
				}
				else
				{
					count++;
					c += count;
					while (count--)
					{
						*Strm << dst->b
							<< dst->g
							<< dst->r;
						dst->a = 255;
						dst++;
					}
				}
			}
			while (c < Width);
		}
	}
	else if (hdr.img_type == 10 && hdr.pal_type == 0 && hdr.bpp == 32)
	{
		// 32-bit RLE compressed
		for (int y = Height; y; y--)
		{
			int yc = hdr.descriptor_bits & 0x20 ? Height - y : y - 1;
			rgba_t* dst = (rgba_t*)(Pixels + yc * Width * 4);
			c = 0;

			do
			{
				count = Streamer<vuint8>(*Strm);
				if (count & 0x80)
				{
					count = (count & 0x7F) + 1;
					c += count;
					rgba_t col;
					*Strm << col.b << col.g << col.r << col.a;
					while (count--)
					{
						*dst = col;
						dst++;
					}
				}
				else
				{
					count++;
					c += count;
					while (count--)
					{
						*Strm << dst->b
							<< dst->g
							<< dst->r
							<< dst->a;
						dst++;
					}
				}
			}
			while (c < Width);
		}
	}
	else if (hdr.img_type == 11 && hdr.bpp == 8 && hdr.pal_type == 1)
	{
		// Grayscale RLE compressed
		for (int i = 0; i < 256; i++)
		{
			Palette[i].r = i;
			Palette[i].g = i;
			Palette[i].b = i;
			Palette[i].a = 255;
		}
		for (int y = Height; y; y--)
		{
			int yc = hdr.descriptor_bits & 0x20 ? Height - y : y - 1;
			byte *dst = Pixels + yc * Width;
			c = 0;

			do
			{
				count = Streamer<vuint8>(*Strm);
				if (count & 0x80)
				{
					count = (count & 0x7F) + 1;
					c += count;
					vuint8 col;
					*Strm << col;
					while (count--)
						*(dst++) = col;
				}
				else
				{
					count++;
					c += count;
					Strm->Serialise(dst, count);
					dst += count;
				}
			}
			while (c < Width);
		}
	}
	else
	{
		Sys_Error("Nonsupported tga format");
	}

	delete Strm;

	//	For 8-bit textures remap colour 0.
	if (Format == TEXFMT_8Pal)
	{
		FixupPalette(Pixels, Palette);
	}
	return Pixels;
	unguard;
}

//==========================================================================
//
//	VTgaTexture::GetPalette
//
//==========================================================================

rgba_t* VTgaTexture::GetPalette()
{
	guardSlow(VTgaTexture::GetPalette);
	return Palette;
	unguardSlow;
}

//==========================================================================
//
//	VTgaTexture::Unload
//
//==========================================================================

void VTgaTexture::Unload()
{
	guard(VTgaTexture::Unload);
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
//	WriteTGA
//
//==========================================================================

#ifdef CLIENT
void WriteTGA(char* filename, void* data, int width, int height, int bpp,
	bool bot2top)
{
	guard(WriteTGA);
	VStream* Strm = FL_OpenFileWrite(filename);
	if (!Strm)
	{
		GCon->Log("Couldn't write tga");
		return;
	}

	tgaHeader_t hdr;
	hdr.id_length = 0;
	hdr.pal_type = (bpp == 8) ? 1 : 0;
	hdr.img_type = (bpp == 8) ? 1 : 2;
	hdr.first_colour = 0;
	hdr.pal_colours = (bpp == 8) ? 256 : 0;
	hdr.pal_entry_size = (bpp == 8) ? 24 : 0;
	hdr.left = 0;
	hdr.top = 0;
	hdr.width = width;
	hdr.height = height;
	hdr.bpp = bpp;
	hdr.descriptor_bits = bot2top ? 0 : 0x20;
	*Strm << hdr;

	if (bpp == 8)
	{
		for (int i = 0; i < 256; i++)
		{
			*Strm << r_palette[i].b
				<< r_palette[i].g
				<< r_palette[i].r;
		}
	}

	if (bpp == 8)
	{
		Strm->Serialise(data, width * height);
	}
	else if (bpp == 24)
	{
		rgb_t *src = (rgb_t *)data;
		for (int i = 0; i < width * height; i++, src++)
		{
			*Strm << src->b 
				<< src->g 
				<< src->r;
		}
	}

	delete Strm;
	unguard;
}
#endif
