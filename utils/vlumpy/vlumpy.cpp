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
//**	Copyright (C) 1999-2002 JÆnis Legzdi·ý
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
#include "wadlib.h"
#include "scrlib.h"
#include "imglib.h"

using namespace VavoomUtils;

// MACROS ------------------------------------------------------------------

// TYPES -------------------------------------------------------------------

struct vpic_t
{
	short		width;
	short		height;
	byte		data[4];
};

struct RGB_MAP
{
	byte		data[32][32][32];
};

// EXTERNAL FUNCTION PROTOTYPES --------------------------------------------

// PUBLIC FUNCTION PROTOTYPES ----------------------------------------------

// PRIVATE FUNCTION PROTOTYPES ---------------------------------------------

// EXTERNAL DATA DECLARATIONS ----------------------------------------------

// PUBLIC DATA DEFINITIONS -------------------------------------------------

TOWadFile		outwad;

char			basepath[256];
char			srcpath[256];

RGB_MAP			rgb_table;
bool			rgb_table_created;

char			lumpname[12];
char			destfile[1024];

// PRIVATE DATA DEFINITIONS ------------------------------------------------

// CODE --------------------------------------------------------------------

//==========================================================================
//
//	makecol8
//
//==========================================================================

int makecol8(int r, int g, int b)
{
	return rgb_table.data[r >> 3][g >> 3][b >> 3];
}

//==========================================================================
//
//	GetPixel
//
//==========================================================================

byte GetPixel(int x, int y)
{
	return ImgData[x + y * ImgWidth];
}

//==========================================================================
//
//	fn
//
//==========================================================================

char *fn(const char *name)
{
	static char filename[1024];
	if (name[0] == '/' || name[0] == '\\' || name[1] == ':')
	{
		//	Absolute path
		strcpy(filename, name);
	}
	else
	{
		sprintf(filename, "%s%s", srcpath, name);
	}
	return filename;
}

//==========================================================================
//
//	AddWadFile
//
//==========================================================================

static void AddWadFile(const char *name)
{
	TIWadFile	inwad;

	inwad.Open(name);
	for (int i = 0; i < inwad.numlumps; i++)
	{
		void *data = inwad.GetLump(i);
		outwad.AddLump(inwad.LumpName(i), data, inwad.LumpSize(i));
		Free(data);
	}
	inwad.Close();
}

//==========================================================================
//
//	AddWad
//
//	$WAD filename
//
//==========================================================================

static void AddWad(const char *name)
{
	char *filename = fn(name);
	DefaultExtension(filename, ".wad");
	AddWadFile(filename);
}

//==========================================================================
//
//	AddMap
//
//	$MAP filename
//
//==========================================================================

static void AddMap(const char *name)
{
	char	*filename;

	filename = fn(name);
	DefaultExtension(filename, ".wad");
	AddWadFile(filename);

	StripExtension(filename);
	strcat(filename, ".gwa");
	FILE *ff = fopen(filename, "rb");
	if (ff)
	{
		fclose(ff);
		AddWadFile(filename);
	}
}

//==========================================================================
//
//	LoadImage
//
//	$LOAD image
//
//==========================================================================

static void LoadImage(void)
{
	SC_MustGetString();
	DestroyImage();
	LoadImage(fn(sc_String));
	rgb_table_created = false;
}

//==========================================================================
//
//	my_bestfit_color
//
//==========================================================================

int my_bestfit_color(int r, int g, int b)
{
	int		best_color = 0;
	int		best_dist = 0x1000000;

	for (int i = 1; i < 256; i++)
	{
		int dist = (ImgPal[i].r - r) * (ImgPal[i].r - r) +
			(ImgPal[i].g - g) * (ImgPal[i].g - g) +
			(ImgPal[i].b - b) * (ImgPal[i].b - b);
		if (dist < best_dist)
		{
			best_color = i;
			best_dist = dist;
			if (!dist)
				break;
		}
	}
	return best_color;
}

//==========================================================================
//
//	SetupRGBTable
//
//==========================================================================

void SetupRGBTable(void)
{
	if (rgb_table_created)
	{
		return;
	}

	for (int r = 0; r < 32; r++)
	{
		for (int g = 0; g < 32; g++)
		{
			for (int b = 0; b < 32; b++)
			{
				rgb_table.data[r][g][b] = my_bestfit_color(
					(int)(r * 255.0 / 31.0 + 0.5),
					(int)(g * 255.0 / 31.0 + 0.5),
					(int)(b * 255.0 / 31.0 + 0.5));
			}
		}
	}
	rgb_table_created = true;
}

//==========================================================================
//
//	GrabRGBTable
//
//==========================================================================

void GrabRGBTable(void)
{
	byte	tmp[32 * 32 * 32 + 4];

	SetupRGBTable();
	memcpy(tmp, &rgb_table, 32 * 32 * 32);
	tmp[32 * 32 * 32] = 0;
	outwad.AddLump(lumpname, tmp, 32 * 32 * 32 + 1);
}

//==========================================================================
//
//	GrabTranslucencyTable
//
//	lumpname TINTTAB amount
//
//==========================================================================

void GrabTranslucencyTable(void)
{
	byte		table[256 * 256];
	byte		temp[768];
	int			i;
	int			j;
	int			r;
	int			g;
	int			b;
	byte*		p;
	byte*		q;
	int			transluc;

	SC_MustGetNumber();
	transluc = sc_Number;

	SetupRGBTable();

	p = table;
	for (i = 0; i < 256; i++)
	{
		temp[i * 3]     = ImgPal[i].r * transluc / 100;
		temp[i * 3 + 1] = ImgPal[i].g * transluc / 100;
		temp[i * 3 + 2] = ImgPal[i].b * transluc / 100;
	}
	for (i = 0; i < 256; i++)
	{
		r = ImgPal[i].r * (100 - transluc) / 100;
		g = ImgPal[i].g * (100 - transluc) / 100;
		b = ImgPal[i].b * (100 - transluc) / 100;
		q = temp;
		for (j = 0; j < 256; j++)
		{
			*(p++) = makecol8(r + q[0], g + q[1], b + q[2]);
			q += 3;
		}
	}
	outwad.AddLump(lumpname, table, 256 * 256);
}

//==========================================================================
//
//	GrabScaleMap
//
//	lumpname SCALEMAP r g b
//
//==========================================================================

void GrabScaleMap(void)
{
	byte		map[256];
	int			i;
	double		r;
	double		g;
	double		b;

	SC_MustGetFloat();
	r = sc_Float;
	SC_MustGetFloat();
	g = sc_Float;
	SC_MustGetFloat();
	b = sc_Float;

	SetupRGBTable();

	for (i = 0; i < 256; i++)
	{
		double col = (ImgPal[i].r * 0.3 + ImgPal[i].g * 0.5 + ImgPal[i].b * 0.2);
		map[i] = makecol8((int)(r * col), (int)(g * col), (int)(b * col));
	}

	outwad.AddLump(lumpname, map, 256);
}

//==========================================================================
//
//	GrabRaw
//
//	lumpname RAW x y width height
//
//==========================================================================

void GrabRaw(void)
{
	SC_MustGetNumber();
	int x1 = sc_Number;
	SC_MustGetNumber();
	int y1 = sc_Number;
	SC_MustGetNumber();
	int w = sc_Number;
	SC_MustGetNumber();
	int h = sc_Number;
	int x2 = x1 + w;
	int y2 = y1 + h;

	byte *data = (byte*)Malloc(w * h);
	byte *dst = data;
	for (int y = y1; y < y2; y++)
	{
		for (int x = x1; x < x2; x++)
		{
			*dst = GetPixel(x, y);
			dst++;
		}
	}

	outwad.AddLump(lumpname, data, w * h);
	Free(data);
}

//==========================================================================
//
//	GrabPic
//
//	lumpname PIC x y width height
//
//==========================================================================

void GrabPic(void)
{
	SC_MustGetNumber();
	int x1 = sc_Number;
	SC_MustGetNumber();
	int y1 = sc_Number;
	SC_MustGetNumber();
	int w = sc_Number;
	SC_MustGetNumber();
	int h = sc_Number;
	int x2 = x1 + w;
	int y2 = y1 + h;

	vpic_t *pic = (vpic_t*)Malloc(4 + w * h);
	pic->width = LittleShort(w);
	pic->height = LittleShort(h);
	byte *dst = pic->data;
	for (int y = y1; y < y2; y++)
	{
		for (int x = x1; x < x2; x++)
		{
			*dst = GetPixel(x, y);
			dst++;
		}
	}

	outwad.AddLump(lumpname, pic, 4 + w * h);
	Free(pic);
}

//==========================================================================
//
//	ParseScript
//
//==========================================================================

void ParseScript(const char *name)
{
	ExtractFilePath(name, basepath);
	strcpy(srcpath, basepath);

	strcpy(destfile, name);
	DefaultExtension(destfile, ".ls");
	SC_Open(destfile);
	StripExtension(destfile);

	bool OutputOpened = false;
	bool GrabMode = false;

	while (SC_GetString())
	{
		if (SC_Compare("$dest"))
		{
			if (OutputOpened)
				SC_ScriptError("Output already opened");
			SC_MustGetString();
			strcpy(destfile, fn(sc_String));
			continue;
		}

		if (SC_Compare("$srcdir"))
		{
			SC_MustGetString();
			sprintf(srcpath, "%s%s", basepath, sc_String);
			if (srcpath[strlen(srcpath) - 1] != '/')
			{
				strcat(srcpath, "/");
			}
			continue;
		}

		if (SC_Compare("$load"))
		{
			LoadImage();
			GrabMode = true;
			continue;
		}

		if (SC_Compare("$files"))
		{
			GrabMode = false;
			continue;
		}

		if (!OutputOpened)
		{
			DefaultExtension(destfile, ".wad");
			outwad.Open(destfile, "PWAD");
			OutputOpened = true;
		}

		if (SC_Compare("$wad"))
		{
			SC_MustGetString();
			AddWad(sc_String);
			continue;
		}

		if (SC_Compare("$map"))
		{
			SC_MustGetString();
			AddMap(sc_String);
			continue;
		}

		if (SC_Compare("$label"))
		{
			SC_MustGetString();
			outwad.AddLump(sc_String, NULL, 0);
			continue;
		}

		if (GrabMode)
		{
			if (strlen(sc_String) > 8)
			{
				SC_ScriptError("Lump name is too long.");
			}
			memset(lumpname, 0, sizeof(lumpname));
			strcpy(lumpname, sc_String);

			SC_MustGetString();
			if (SC_Compare("rgbtable"))
			{
				GrabRGBTable();
			}
			else if (SC_Compare("tinttab"))
			{
				GrabTranslucencyTable();
			}
			else if (SC_Compare("scalemap"))
			{
				GrabScaleMap();
			}
			else if (SC_Compare("raw"))
			{
				GrabRaw();
			}
			else if (SC_Compare("pic"))
			{
				GrabPic();
			}
			else
			{
				SC_ScriptError(va("Unknown command %s", sc_String));
			}
		}
		else
		{
			ExtractFileBase(sc_String, lumpname);
			if (strlen(lumpname) > 8)
			{
				SC_ScriptError("File name too long");
			}
			void *data;
			int size = LoadFile(fn(sc_String), &data);
			outwad.AddLump(lumpname, data, size);
			Free(data);
		}
	}

	DestroyImage();
	if (outwad.handle)
	{
		outwad.Close();
	}
	SC_Close();
}

//==========================================================================
//
//	main
//
//==========================================================================

int main(int argc, char *argv[])
{
	if (argc < 2)
	{
		cerr << "Usage: vlumpy <script1> [<script2> ...]\n";
		return 1;
	}

	for (int i = 1; i < argc; i++)
	{
		ParseScript(argv[i]);
	}
	return 0;
}

//**************************************************************************
//
//	$Log$
//	Revision 1.5  2002/01/07 12:31:36  dj_jl
//	Changed copyright year
//
//	Revision 1.4  2001/09/24 17:31:02  dj_jl
//	Beautification
//	
//	Revision 1.3  2001/08/31 17:19:53  dj_jl
//	Beautification
//
//	Revision 1.2  2001/07/27 14:27:56  dj_jl
//	Update with Id-s and Log-s, some fixes
//
//**************************************************************************
