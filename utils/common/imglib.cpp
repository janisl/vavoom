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
//**	Copyright (C) 1999-2001 JÆnis Legzdi·ý
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
//**	$Log$
//**	Revision 1.3  2001/08/31 17:19:53  dj_jl
//**	Beautification
//**
//**	Revision 1.2  2001/07/27 14:27:54  dj_jl
//**	Update with Id-s and Log-s, some fixes
//**	
//**************************************************************************

// HEADER FILES ------------------------------------------------------------

#include "cmdlib.h"
#include "imglib.h"

// MACROS ------------------------------------------------------------------

// TYPES -------------------------------------------------------------------

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
	char			color_planes;
	unsigned short	bytes_per_line;
	unsigned short	palette_type;

	char			filler[58];
	unsigned char	data;		// unbounded
};

// EXTERNAL FUNCTION PROTOTYPES --------------------------------------------

// PUBLIC FUNCTION PROTOTYPES ----------------------------------------------

// PRIVATE FUNCTION PROTOTYPES ---------------------------------------------

// EXTERNAL DATA DECLARATIONS ----------------------------------------------

// PUBLIC DATA DEFINITIONS -------------------------------------------------

int				ImgWidth;
int				ImgHeight;
byte			*ImgData;
rgb_t			ImgPal[256];

// PRIVATE DATA DEFINITIONS ------------------------------------------------

// CODE --------------------------------------------------------------------

void LoadPCX(const char *filename)
{
	int			c;
	int			bytes_per_line;
	int			x, y;
	char		ch;
	pcx_t		*pcx;
	byte		*data;

	LoadFile(filename, (void**)&pcx);

	if (pcx->bits_per_pixel != 8)
	{
		// we like 8 bit color planes
		Error("No 8-bit planes\n");
	}
	if (pcx->color_planes != 1)
	{
		Error("Not 8 bpp\n");
	}

	ImgWidth = LittleShort(pcx->xmax) - LittleShort(pcx->xmin) + 1;
	ImgHeight = LittleShort(pcx->ymax) - LittleShort(pcx->ymin) + 1;

	bytes_per_line = pcx->bytes_per_line;

	ImgData = new byte[ImgWidth * ImgHeight];

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

	Free(pcx);
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
	else
		Error("Unknown extension");
}

//==========================================================================
//
//	DestroyImage
//
//==========================================================================

void DestroyImage(void)
{
	if (ImgData)
	{
		delete ImgData;
		ImgData = NULL;
	}
}
