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

#include "zip.h"
#include <time.h>
#include "cmdlib.h"
#include "wadlib.h"
#include "scrlib.h"
#include "imglib.h"

using namespace VavoomUtils;

// MACROS ------------------------------------------------------------------

// TYPES -------------------------------------------------------------------

// posts are runs of non masked source pixels
struct post_t
{
	vuint8		topdelta;       // -1 is the last post in a column
	vuint8		length;         // length data bytes follows
};

// column_t is a list of 0 or more post_t, (byte)-1 terminated
typedef post_t column_t;

//  Patches.
//  A patch holds one or more columns.
//  Patches are used for sprites and all masked pictures, and we compose
// textures from the TEXTURE1/2 lists of patches.
struct patch_t
{
	short		width;          // bounding box size
	short		height;
	short		leftoffset;     // pixels to the left of origin
	short		topoffset;      // pixels below the origin
	int			columnofs[8];   // only [width] used
	// the [0] is &columnofs[width]
};

struct vpic_t
{
	char		magic[4];
	short		width;
	short		height;
	vuint8		bpp;
	vuint8		reserved[7];
};

struct RGB_MAP
{
	vuint8		data[32][32][32];
};

struct fot1_header_t
{
	char		Id[4];
	vuint16		Width;
	vuint16		Height;
};

// EXTERNAL FUNCTION PROTOTYPES --------------------------------------------

// PUBLIC FUNCTION PROTOTYPES ----------------------------------------------

// PRIVATE FUNCTION PROTOTYPES ---------------------------------------------

// EXTERNAL DATA DECLARATIONS ----------------------------------------------

// PUBLIC DATA DEFINITIONS -------------------------------------------------

TOWadFile		outwad;
zipFile			Zip;

char			basepath[256];
char			srcpath[256];
char			destpath[256];

RGB_MAP			rgb_table;
bool			rgb_table_created;

char			lumpname[256];
char			destfile[1024];

// PRIVATE DATA DEFINITIONS ------------------------------------------------

// CODE --------------------------------------------------------------------

//==========================================================================
//
//	AddToZip
//
//==========================================================================

void AddToZip(const char* Name, void* Data, size_t Size)
{
	zip_fileinfo zi;
	memset(&zi, 0, sizeof(zi));

	//	Set file time to current time.
	time_t CurTime = 0;
	time(&CurTime);
	struct tm* LTime = localtime(&CurTime);
	if (LTime)
	{
		zi.tmz_date.tm_sec  = LTime->tm_sec;
		zi.tmz_date.tm_min  = LTime->tm_min;
		zi.tmz_date.tm_hour = LTime->tm_hour;
		zi.tmz_date.tm_mday = LTime->tm_mday;
		zi.tmz_date.tm_mon  = LTime->tm_mon ;
		zi.tmz_date.tm_year = LTime->tm_year;
	}

	//	Open file.
	if (zipOpenNewFileInZip(Zip, Name, &zi, NULL, 0, NULL, 0, NULL,
		Z_DEFLATED, Z_BEST_COMPRESSION) != ZIP_OK)
	{
		Error("Failed to open file in ZIP");
	}

	//	Write to it
	if (zipWriteInFileInZip(Zip, Data, Size) != ZIP_OK)
	{
		Error("Failed to write file in ZIP");
	}

	//	Close it.
	if (zipCloseFileInZip(Zip) != ZIP_OK)
	{
		Error("Failed to close file in ZIP");
	}
}

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

vuint8 GetPixel(int x, int y)
{
	return ImgData[x + y * ImgWidth];
}

//==========================================================================
//
//	GetPixelRGB
//
//==========================================================================

rgba_t &GetPixelRGB(int x, int y)
{
	return ((rgba_t *)ImgData)[x + y * ImgWidth];
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
	else if (destpath[0])
	{
		sprintf(filename, "%s%s", destpath, name);
		FILE* f = fopen(filename, "rb");
		if (f)
		{
			fclose(f);
		}
		else
		{
			sprintf(filename, "%s%s", srcpath, name);
		}
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
	if (Zip)
	{
		Error("$wad cannot be used for ZIP file");
	}

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
	if (Zip)
	{
		Error("$map cannot be used for ZIP file");
	}

	char* filename = fn(name);
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

static void LoadImage()
{
	SC_MustGetString();
	DestroyImage();
	LoadImage(fn(sc_String));
	rgb_table_created = false;
}

//==========================================================================
//
//	my_bestfit_colour
//
//==========================================================================

int my_bestfit_colour(int r, int g, int b)
{
	int		best_colour = 0;
	int		best_dist = 0x1000000;

	for (int i = 1; i < 256; i++)
	{
		int dist = (ImgPal[i].r - r) * (ImgPal[i].r - r) +
			(ImgPal[i].g - g) * (ImgPal[i].g - g) +
			(ImgPal[i].b - b) * (ImgPal[i].b - b);
		if (dist < best_dist)
		{
			best_colour = i;
			best_dist = dist;
			if (!dist)
				break;
		}
	}
	return best_colour;
}

//==========================================================================
//
//	SetupRGBTable
//
//==========================================================================

void SetupRGBTable()
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
				rgb_table.data[r][g][b] = my_bestfit_colour(
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

void GrabRGBTable()
{
	vuint8	tmp[32 * 32 * 32 + 4];

	SetupRGBTable();
	memcpy(tmp, &rgb_table, 32 * 32 * 32);
	tmp[32 * 32 * 32] = 0;
	if (Zip)
	{
		AddToZip(lumpname, tmp, 32 * 32 * 32 + 1);
	}
	else
	{
		outwad.AddLump(lumpname, tmp, 32 * 32 * 32 + 1);
	}
}

//==========================================================================
//
//	GrabTranslucencyTable
//
//	lumpname TINTTAB amount
//
//==========================================================================

void GrabTranslucencyTable()
{
	vuint8		table[256 * 256];
	vuint8		temp[768];
	int			i;
	int			j;
	int			r;
	int			g;
	int			b;
	vuint8*		p;
	vuint8*		q;
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
	if (Zip)
	{
		AddToZip(lumpname, table, 256 * 256);
	}
	else
	{
		outwad.AddLump(lumpname, table, 256 * 256);
	}
}

//==========================================================================
//
//	GrabScaleMap
//
//	lumpname SCALEMAP r g b
//
//==========================================================================

void GrabScaleMap()
{
	vuint8		map[256];
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

	if (Zip)
	{
		AddToZip(lumpname, map, 256);
	}
	else
	{
		outwad.AddLump(lumpname, map, 256);
	}
}

//==========================================================================
//
//	GrabRaw
//
//	lumpname RAW x y width height
//
//==========================================================================

void GrabRaw()
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

	vuint8 *data = (vuint8*)Malloc(w * h);
	vuint8 *dst = data;
	for (int y = y1; y < y2; y++)
	{
		for (int x = x1; x < x2; x++)
		{
			*dst = GetPixel(x, y);
			dst++;
		}
	}

	if (Zip)
	{
		AddToZip(lumpname, data, w * h);
	}
	else
	{
		outwad.AddLump(lumpname, data, w * h);
	}
	Free(data);
}

//==========================================================================
//
//	GrabPatch
//
//	lumpname PATCH x y width height leftoffset topoffset
//
//==========================================================================

void GrabPatch()
{
	SC_MustGetNumber();
	int x1 = sc_Number;
	SC_MustGetNumber();
	int y1 = sc_Number;
	SC_MustGetNumber();
	int w = sc_Number;
	SC_MustGetNumber();
	int h = sc_Number;
	SC_MustGetNumber();
	int leftoffset = sc_Number;
	SC_MustGetNumber();
	int topoffset = sc_Number;

	int TransColour = 0;
	patch_t* Patch = (patch_t*)Malloc(8 + 4 * w + w * h * 4);
	Patch->width = LittleShort(w);
	Patch->height = LittleShort(h);
	Patch->leftoffset = LittleShort(leftoffset);
	Patch->topoffset = LittleShort(topoffset);
	column_t* Col = (column_t*)&Patch->columnofs[w];

	for (int x = 0; x < w; x++)
	{
		Patch->columnofs[x] = LittleLong((vuint8*)Col - (vuint8*)Patch);
		int y = 0;
		int PrevTop = -1;
		while (y < h)
		{
			//	Skip transparent pixels.
			if (GetPixel(x1 + x, y1 + y) == TransColour)
			{
				y++;
				continue;
			}
			//	Grab a post.
			if (y < 255)
			{
				Col->topdelta = y;
			}
			else
			{
				//	Tall patch.
				while (y - PrevTop > 254)
				{
					//	Insert empty post.
					Col->topdelta = 254;
					Col = (column_t*)((vuint8*)Col + 4);
					if (PrevTop < 254)
					{
						PrevTop = 254;
					}
					else
					{
						PrevTop += 254;
					}
				}
				Col->topdelta = y - PrevTop;
			}
			PrevTop = y;
			vuint8* Pixels = (vuint8*)Col + 3;
			while (y < h && Col->length < 255 &&
				GetPixel(x1 + x, y1 + y) != TransColour)
			{
				Pixels[Col->length] = GetPixel(x1 + x, y1 + y);
				Col->length++;
				y++;
			}
			Col = (column_t*)((vuint8*)Col + Col->length + 4);
		}

		//	Add terminating post.
		Col->topdelta = 0xff;
		Col = (column_t*)((vuint8*)Col + 4);
	}

	if (Zip)
	{
		AddToZip(lumpname, Patch, (vuint8*)Col - (vuint8*)Patch);
	}
	else
	{
		outwad.AddLump(lumpname, Patch, (vuint8*)Col - (vuint8*)Patch);
	}
	Free(Patch);
}

//==========================================================================
//
//	GrabPic
//
//	lumpname PIC x y width height
//
//==========================================================================

void GrabPic()
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

	vpic_t *pic = (vpic_t*)Malloc(sizeof(vpic_t) + w * h);
	memcpy(pic->magic, "VPIC", 4);
	pic->width = LittleShort(w);
	pic->height = LittleShort(h);
	pic->bpp = 8;
	vuint8* dst = (vuint8*)(pic + 1);
	for (int y = y1; y < y2; y++)
	{
		for (int x = x1; x < x2; x++)
		{
			*dst = GetPixel(x, y);
			dst++;
		}
	}

	if (Zip)
	{
		AddToZip(lumpname, pic, sizeof(vpic_t) + w * h);
	}
	else
	{
		outwad.AddLump(lumpname, pic, sizeof(vpic_t) + w * h);
	}
	Free(pic);
}

//==========================================================================
//
//	GrabPic15
//
//	lumpname PIC15 x y width height
//
//==========================================================================

void GrabPic15()
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

	ConvertImageTo32Bit();
	vpic_t *pic = (vpic_t*)Malloc(sizeof(vpic_t) + w * h * 2);
	memcpy(pic->magic, "VPIC", 4);
	pic->width = LittleShort(w);
	pic->height = LittleShort(h);
	pic->bpp = 15;
	vuint8* dst = (vuint8*)(pic + 1);
	for (int y = y1; y < y2; y++)
	{
		for (int x = x1; x < x2; x++)
		{
			const rgba_t &p = GetPixelRGB(x, y);
			int c;
			if (p.r == 0 && (p.g & 0xf8) == 0xf8 && (p.b & 0xf8) == 0xf8)
				c = 0x8000;
			else
				c = ((p.r << 7) & 0x7c00) | ((p.g << 2) & 0x03e0) | ((p.b >> 3) & 0x001f);
			dst[0] = vuint8(c);
			dst[1] = vuint8(c >> 8);
			dst += 2;
		}
	}

	if (Zip)
	{
		AddToZip(lumpname, pic, sizeof(vpic_t) + w * h * 2);
	}
	else
	{
		outwad.AddLump(lumpname, pic, sizeof(vpic_t) + w * h * 2);
	}
	Free(pic);
}

//==========================================================================
//
//	CompressChar
//
//==========================================================================

vint8* CompressChar(vuint8* Src, vint8* Dst, int Size)
{
	vuint8* SrcEnd = Src + Size;
	do
	{
		if (SrcEnd - Src < 2)
		{
			//	Write last pixel
			*Dst++ = 0;
			*Dst++ = *Src++;
		}
		else if (Src[0] != Src[1] && (SrcEnd - Src < 2 || Src[0] != Src[2]))
		{
			//	Write different pixels.
			int Len = 2;
			while (Len < 128 && Len < SrcEnd - Src && (Len + 2 > SrcEnd - Src ||
				(Src[Len - 1] != Src[Len] && Src[Len - 1] != Src[Len + 1])))
			{
				Len++;
			}
			*Dst++ = Len - 1;
			for (int i = 0; i < Len; i++)
			{
				*Dst++ = *Src++;
			}
		}
		else
		{
			//	Write equal pixels.
			int Len = 2;
			while (Len < 128 && Len < SrcEnd - Src && Src[Len - 1] == Src[Len])
			{
				Len++;
			}
			*Dst++ = -Len + 1;
			*Dst++ = *Src;
			Src += Len;
		}
	}
	while (Src < SrcEnd);
	return Dst;
}

//==========================================================================
//
//	GrabFon1
//
//	lumpname FON1
//
//==========================================================================

void GrabFon1()
{
	//	Dimensions of a character.
	int CharW = ImgWidth / 16;
	int CharH = ImgHeight / 16;
	//	Calculate maximum size needed for a single character.
	int MaxCharBytes = CharW * CharH + (CharW * CharH + 127) / 128;

	//	Allocate memory and fill in header.
	fot1_header_t* Font = (fot1_header_t*)Malloc(sizeof(fot1_header_t) +
		MaxCharBytes * 256);
	Font->Id[0] = 'F';
	Font->Id[1] = 'O';
	Font->Id[2] = 'N';
	Font->Id[3] = '1';
	Font->Width = LittleShort(CharW);
	Font->Height = LittleShort(CharH);

	//	Process characters.
	vuint8* CharBuf = (vuint8*)Malloc(CharW * CharH);
	vint8* pDst = (vint8*)(Font + 1);
	for (int i = 0; i < 256; i++)
	{
		int StartX = (i % 16) * CharW;
		int StartY = (i / 16) * CharH;
		for (int ch = 0; ch < CharH; ch++)
		{
			for (int cw = 0; cw < CharW; cw++)
			{
				CharBuf[ch * CharW + cw] = GetPixel(StartX + cw, StartY + ch);
			}
		}
		pDst = CompressChar(CharBuf, pDst, CharW * CharH);
	}

	//	Write lump and free memory.
	if (Zip)
	{
		AddToZip(lumpname, Font, pDst - (vint8*)Font);
	}
	else
	{
		outwad.AddLump(lumpname, Font, pDst - (vint8*)Font);
	}
	Free(Font);
	Free(CharBuf);
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

	SC_Open(name);

	if (!destfile[0])
	{
		strcpy(destfile, name);
		StripExtension(destfile);
		destpath[0] = 0;
	}
	else
	{
		ExtractFilePath(destfile, destpath);
	}

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
			char Ext[8];
			ExtractFileExtension(destfile, Ext);
			if (!stricmp(Ext, "zip") || !stricmp(Ext, "pk3"))
			{
				Zip = zipOpen(destfile, APPEND_STATUS_CREATE);
				if (!Zip)
				{
					Error("Failed to open ZIP file");
				}
			}
			else
			{
				outwad.Open(destfile, "PWAD");
			}
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
			if (Zip)
			{
				Error("$label cannot be used for ZIP file");
			}
			SC_MustGetString();
			outwad.AddLump(sc_String, NULL, 0);
			continue;
		}

		if (GrabMode)
		{
			if (!Zip && strlen(sc_String) > 8)
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
			else if (SC_Compare("patch"))
			{
				GrabPatch();
			}
			else if (SC_Compare("pic"))
			{
				GrabPic();
			}
			else if (SC_Compare("pic15"))
			{
				GrabPic15();
			}
			else if (SC_Compare("fon1"))
			{
				GrabFon1();
			}
			else
			{
				SC_ScriptError(va("Unknown command %s", sc_String));
			}
		}
		else if (Zip)
		{
			strcpy(lumpname, sc_String);
			SC_MustGetString();
			void *data;
			int size = LoadFile(fn(sc_String), &data);
			AddToZip(lumpname, data, size);
			Free(data);
		}
		else
		{
			ExtractFileBase(sc_String, lumpname);
			if (strlen(lumpname) > 8)
			{
				SC_ScriptError("Lump name too long");
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
	if (Zip)
	{
		if (zipClose(Zip, NULL) != ZIP_OK)
		{
			Error("Failed to close ZIP file");
		}
	}
	SC_Close();

	destfile[0] = 0;
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
		fprintf(stderr, "Usage: vlumpy <script1> [<script2> ...]\n");
		return 1;
	}

	try
	{
		for (int i = 1; i < argc; i++)
		{
			if (argv[i][0] == '-' && argv[i][1] == 'D')
			{
				strcpy(destfile, argv[i] + 2);
			}
			else
			{
				ParseScript(argv[i]);
			}
		}
	}
	catch (WadLibError &E)
	{
		Error("%s", E.message);
	}
	return 0;
}
