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

#include "cmdlib.h"
#include "imglib.h"

namespace VavoomUtils {

// MACROS ------------------------------------------------------------------

// TYPES -------------------------------------------------------------------

#pragma pack(push, 1)

struct pcx_t
{
	char			manufacturer;
	char			version;
	char			encoding;
	char			bits_per_pixel;

	unsigned short	xmin;
	unsigned short	ymin;
	unsigned short	xmax;
	unsigned short	ymax;

	unsigned short	hres;
	unsigned short	vres;

	unsigned char	palette[48];

	char			reserved;
	char			colour_planes;
	unsigned short	bytes_per_line;
	unsigned short	palette_type;

	char			filler[58];
	unsigned char	data;		// unbounded
};

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
};

#pragma pack(pop)

// EXTERNAL FUNCTION PROTOTYPES --------------------------------------------

// PUBLIC FUNCTION PROTOTYPES ----------------------------------------------

// PRIVATE FUNCTION PROTOTYPES ---------------------------------------------

// EXTERNAL DATA DECLARATIONS ----------------------------------------------

// PUBLIC DATA DEFINITIONS -------------------------------------------------

int				ImgWidth;
int				ImgHeight;
int				ImgBPP;
vuint8*			ImgData;
rgb_t			ImgPal[256];

// PRIVATE DATA DEFINITIONS ------------------------------------------------

// CODE --------------------------------------------------------------------

static void LoadPCX(const char *filename)
{
	int			c;
	int			bytes_per_line;
	int			x, y;
	char		ch;
	pcx_t		*pcx;
	vuint8		*data;

	LoadFile(filename, (void**)&pcx);

	if (pcx->bits_per_pixel != 8)
	{
		// we like 8 bit colour planes
		Error("No 8-bit planes\n");
	}
	if (pcx->colour_planes != 1)
	{
		Error("Not 8 bpp\n");
	}

	ImgWidth = LittleShort(pcx->xmax) - LittleShort(pcx->xmin) + 1;
	ImgHeight = LittleShort(pcx->ymax) - LittleShort(pcx->ymin) + 1;
	ImgBPP = 8;

	bytes_per_line = LittleShort(pcx->bytes_per_line);

	ImgData = new vuint8[ImgWidth * ImgHeight];

	data = &pcx->data;

	for (y = 0; y < ImgHeight; y++)
	{
		// decompress RLE encoded PCX data
		x = 0;

		while (x < bytes_per_line)
		{
			ch = *data++;
			if ((ch & 0xC0) == 0xC0)
			{
				c = (ch & 0x3F);
				ch = *data++;
			}
			else
			{
				c = 1;
			}

			while (c--)
			{
				if (x < ImgWidth)
					ImgData[y * ImgWidth + x] = ch;
				x++;
			}
		}
	}

	if (*data == 12)
	{
		data++;
		for (c = 0; c < 256; c++)
		{
			ImgPal[c].r = *data++;
			ImgPal[c].g = *data++;
			ImgPal[c].b = *data++;
		}
	}

	Z_Free(pcx);
}

//==========================================================================
//
//	LoadTGA
//
//==========================================================================

static void LoadTGA(const char *filename)
{
	tgaHeader_t *hdr;
	vuint8* data;
	int col;
	int count;
	int c;

	LoadFile(filename, (void**)&hdr);

	ImgWidth = LittleShort(hdr->width);
	ImgHeight = LittleShort(hdr->height);

	data = (vuint8*)(hdr + 1) + hdr->id_length;

	for (int i = 0; i < hdr->pal_colours; i++)
	{
		switch (hdr->pal_entry_size)
		{
		case 16:
			col = *(vuint16*)data;
			ImgPal[i].r = (col & 0x1F) << 3;
			ImgPal[i].g = ((col >> 5) & 0x1F) << 3;
			ImgPal[i].b = ((col >> 10) & 0x1F) << 3;
			//ImgPal[i].a = 255;
			break;
		case 24:
			ImgPal[i].b = data[0];
			ImgPal[i].g = data[1];
			ImgPal[i].r = data[2];
			//ImgPal[i].a = 255;
			break;
		case 32:
			ImgPal[i].b = data[0];
			ImgPal[i].g = data[1];
			ImgPal[i].r = data[2];
			//ImgPal[i].a = data[3];
			break;
		}
		data += (hdr->pal_entry_size >> 3);
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

	if (hdr->img_type == 1 || hdr->img_type == 3 ||
		hdr->img_type == 9 || hdr->img_type == 11)
	{
		ImgBPP = 8;
		ImgData = (vuint8*)Z_Malloc(ImgWidth * ImgHeight);
	}
	else
	{
		ImgBPP = 32;
		ImgData = (vuint8*)Z_Malloc(ImgWidth * ImgHeight * 4);
	}

	if (hdr->img_type == 1 && hdr->bpp == 8 && hdr->pal_type == 1)
	{
		// 8-bit, uncompressed
		for (int y = ImgHeight; y; y--)
		{
			int yc = hdr->descriptor_bits & 0x20 ? ImgHeight - y : y - 1;
			vuint8* dst = ImgData + yc * ImgWidth;

			memcpy(dst, data, ImgWidth);
			data += ImgWidth;
		}
	}
	else if (hdr->img_type == 2 && hdr->pal_type == 0 && hdr->bpp == 16)
	{
		// 16-bit uncompressed
		for (int y = ImgHeight; y; y--)
		{
			int yc = hdr->descriptor_bits & 0x20 ? ImgHeight - y : y - 1;
			rgba_t *dst = (rgba_t*)(ImgData + yc * ImgWidth * 4);

			for (int x = 0; x < ImgWidth; x++, dst++, data += 2)
			{
				col = *(vuint16*)data;
				dst->r = ((col >> 10) & 0x1F) << 3;
				dst->g = ((col >> 5) & 0x1F) << 3;
				dst->b = (col & 0x1F) << 3;
				dst->a = 255;
			}
		}
	}
	else if (hdr->img_type == 2 && hdr->pal_type == 0 && hdr->bpp == 24)
	{
		// 24-bit uncompressed
		for (int y = ImgHeight; y; y--)
		{
			int yc = hdr->descriptor_bits & 0x20 ? ImgHeight - y : y - 1;
			rgba_t *dst = (rgba_t*)(ImgData + yc * ImgWidth * 4);

			for (int x = 0; x < ImgWidth; x++, dst++, data += 3)
			{
				dst->b = data[0];
				dst->g = data[1];
				dst->r = data[2];
				dst->a = 255;
			}
		}
	}
	else if (hdr->img_type == 2 && hdr->pal_type == 0 && hdr->bpp == 32)
	{
		// 32-bit uncompressed
		for (int y = ImgHeight; y; y--)
		{
			int yc = hdr->descriptor_bits & 0x20 ? ImgHeight - y : y - 1;
			rgba_t *dst = (rgba_t*)(ImgData + yc * ImgWidth * 4);

			for (int x = 0; x < ImgWidth; x++, dst++, data += 4)
			{
				dst->b = data[0];
				dst->g = data[1];
				dst->r = data[2];
				dst->a = data[3];
			}
		}
	}
	else if (hdr->img_type == 3 && hdr->bpp == 8 && hdr->pal_type == 1)
	{
		// Grayscale uncompressed
		for (int i = 0; i < 256; i++)
		{
			ImgPal[i].r = i;
			ImgPal[i].g = i;
			ImgPal[i].b = i;
			//ImgPal[i].a = 255;
		}
		for (int y = ImgHeight; y; y--)
		{
			int yc = hdr->descriptor_bits & 0x20 ? ImgHeight - y : y - 1;
			vuint8* dst = ImgData + yc * ImgWidth;

			memcpy(dst, data, ImgWidth);
			data += ImgWidth;
		}
	}
	else if (hdr->img_type == 9 && hdr->bpp == 8 && hdr->pal_type == 1)
	{
		// 8-bit RLE compressed
		for (int y = ImgHeight; y; y--)
		{
			int yc = hdr->descriptor_bits & 0x20 ? ImgHeight - y : y - 1;
			vuint8* dst = ImgData + yc * ImgWidth;
			c = 0;

			do
			{
				count = *data++;
				if (count & 0x80)
				{
					count = (count & 0x7F) + 1;
					c += count;
					while (count--)
						*(dst++) = *data;
					data++;
				}
				else
				{
					count++;
					c += count;
					memcpy(dst, data, count);
					data += count;
					dst += count;
				}
			}
			while (c < ImgWidth);
		}
	}
	else if (hdr->img_type == 10 && hdr->pal_type == 0 && hdr->bpp == 16)
	{
		// 16-bit RLE compressed
		for (int y = ImgHeight; y; y--)
		{
			int yc = hdr->descriptor_bits & 0x20 ? ImgHeight - y : y - 1;
			rgba_t *dst = (rgba_t*)(ImgData + yc * ImgWidth * 4);
			c = 0;

			do
			{
				count = *data++;
				if (count & 0x80)
				{
					count = (count & 0x7F) + 1;
					c += count;
					col = *(vuint16*)data;
					while (count--)
					{
						dst->r = ((col >> 10) & 0x1F) << 3;
						dst->g = ((col >> 5) & 0x1F) << 3;
						dst->b = (col & 0x1F) << 3;
						dst->a = 255;
						dst++;
					}
					data += 2;
				}
				else
				{
					count++;
					c += count;
					while (count--)
					{
						col = *(vuint16*)data;
						dst->r = ((col >> 10) & 0x1F) << 3;
						dst->g = ((col >> 5) & 0x1F) << 3;
						dst->b = (col & 0x1F) << 3;
						dst->a = 255;
						data += 2;
						dst++;
					}
				}
			}
			while (c < ImgWidth);
		}
	}
	else if (hdr->img_type == 10 && hdr->pal_type == 0 && hdr->bpp == 24)
	{
		// 24-bit REL compressed
		for (int y = ImgHeight; y; y--)
		{
			int yc = hdr->descriptor_bits & 0x20 ? ImgHeight - y : y - 1;
			rgba_t *dst = (rgba_t*)(ImgData + yc * ImgWidth * 4);
			c = 0;

			do
			{
				count = *data++;
				if (count & 0x80)
				{
					count = (count & 0x7F) + 1;
					c += count;
					while (count--)
					{
						dst->r = data[2];
						dst->g = data[1];
						dst->b = data[0];
						dst->a = 255;
						dst++;
					}
					data += 3;
				}
				else
				{
					count++;
					c += count;
					while (count--)
					{
						dst->r = data[2];
						dst->g = data[1];
						dst->b = data[0];
						dst->a = 255;
						data += 3;
						dst++;
					}
				}
			}
			while (c < ImgWidth);
		}
	}
	else if (hdr->img_type == 10 && hdr->pal_type == 0 && hdr->bpp == 32)
	{
		// 32-bit RLE compressed
		for (int y = ImgHeight; y; y--)
		{
			int yc = hdr->descriptor_bits & 0x20 ? ImgHeight - y : y - 1;
			rgba_t *dst = (rgba_t*)(ImgData + yc * ImgWidth * 4);
			c = 0;

			do
			{
				count = *data++;
				if (count & 0x80)
				{
					count = (count & 0x7F) + 1;
					c += count;
					while (count--)
					{
						dst->r = data[2];
						dst->g = data[1];
						dst->b = data[0];
						dst->a = data[3];
						dst++;
					}
					data += 4;
				}
				else
				{
					count++;
					c += count;
					while (count--)
					{
						dst->r = data[2];
						dst->g = data[1];
						dst->b = data[0];
						dst->a = data[3];
						data += 4;
						dst++;
					}
				}
			}
			while (c < ImgWidth);
		}
	}
	else if (hdr->img_type == 11 && hdr->bpp == 8 && hdr->pal_type == 1)
	{
		// Grayscale RLE compressed
		for (int i = 0; i < 256; i++)
		{
			ImgPal[i].r = i;
			ImgPal[i].g = i;
			ImgPal[i].b = i;
			//ImgPal[i].a = 255;
		}
		for (int y = ImgHeight; y; y--)
		{
			int yc = hdr->descriptor_bits & 0x20 ? ImgHeight - y : y - 1;
			vuint8* dst = ImgData + yc * ImgWidth;
			c = 0;

			do
			{
				count = *data++;
				if (count & 0x80)
				{
					count = (count & 0x7F) + 1;
					c += count;
					while (count--)
						*(dst++) = *data;
					data++;
				}
				else
				{
					count++;
					c += count;
					memcpy(dst, data, count);
					data += count;
					dst += count;
				}
			}
			while (c < ImgWidth);
		}
	}
	else
	{
		Error("Nonsupported tga format");
	}


	Z_Free(hdr);
}

//==========================================================================
//
//	LoadImage
//
//==========================================================================

void LoadImage(const char *name)
{
	char	ext[8];

	ExtractFileExtension(name, ext);
	if (!strcmp(ext, "pcx"))
		LoadPCX(name);
	else if (!strcmp(ext, "tga"))
		LoadTGA(name);
	else
		Error("Unknown extension");
}

//==========================================================================
//
//	ConvertImageTo32Bit
//
//==========================================================================

void ConvertImageTo32Bit()
{
	if (ImgBPP == 8)
	{
		rgba_t *NewData = (rgba_t *)Z_Malloc(ImgWidth * ImgHeight * 4);
		for (int i = 0; i < ImgWidth * ImgHeight; i++)
		{
			NewData[i].r = ImgPal[ImgData[i]].r;
			NewData[i].g = ImgPal[ImgData[i]].g;
			NewData[i].b = ImgPal[ImgData[i]].b;
			NewData[i].a = 255;
		}
		Z_Free(ImgData);
		ImgData = (vuint8*)NewData;
		ImgBPP = 32;
	}
}

//==========================================================================
//
//	DestroyImage
//
//==========================================================================

void DestroyImage()
{
	if (ImgData)
	{
		delete ImgData;
		ImgData = NULL;
	}
}

} // namespace
