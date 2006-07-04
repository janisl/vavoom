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

#include "d_local.h"

// MACROS ------------------------------------------------------------------

#define SPRITE_CACHE_SIZE	256

// TYPES -------------------------------------------------------------------

struct sprite_cache_t
{
	void*		data;
	vuint32		light;
	int			lump;
	int			tnum;
};

// EXTERNAL FUNCTION PROTOTYPES --------------------------------------------

// PUBLIC FUNCTION PROTOTYPES ----------------------------------------------

// PRIVATE FUNCTION PROTOTYPES ---------------------------------------------

// EXTERNAL DATA DECLARATIONS ----------------------------------------------

// PUBLIC DATA DEFINITIONS -------------------------------------------------

miptexture_t		*miptexture;

// PRIVATE DATA DEFINITIONS ------------------------------------------------

static sprite_cache_t	sprite_cache[SPRITE_CACHE_SIZE];
static int				sprite_cache_count;

// CODE --------------------------------------------------------------------

//==========================================================================
//
//	VSoftwareDrawer::InitTextures
//
//==========================================================================

void VSoftwareDrawer::InitTextures()
{
}

//==========================================================================
//
//	VSoftwareDrawer::FlushTextureCaches
//
//==========================================================================

void VSoftwareDrawer::FlushTextureCaches()
{
	guard(VSoftwareDrawer::FlushTextureCaches);
	int		i;

	for (i = 0; i < SPRITE_CACHE_SIZE; i++)
	{
		if (sprite_cache[i].data)
		{
			Z_Free(sprite_cache[i].data);
			sprite_cache[i].data = NULL;
		}
	}

	for (i = 0; i < GTextureManager.Textures.Num(); i++)
	{
		if (GTextureManager.Textures[i]->Type == TEXTYPE_SkyMap &&
			GTextureManager.Textures[i]->DriverData)
		{
			Z_Free(GTextureManager.Textures[i]->DriverData);
			GTextureManager.Textures[i]->DriverData = NULL;
		}
	}
	unguard;
}

//==========================================================================
//
// 	VSoftwareDrawer::SetTexture
//
//==========================================================================

void VSoftwareDrawer::SetTexture(int tex)
{
	guard(VSoftwareDrawer::SetTexture);
	if ((vuint32)tex >= (vuint32)GTextureManager.Textures.Num())
		Sys_Error("Invalid texture num %d\n", tex);

	if (GTextureManager.Textures[tex]->Type == TEXTYPE_SkyMap)
	{
		if (!GTextureManager.Textures[tex]->DriverData)
		{
			LoadSkyMap(tex);
		}
		cacheblock = (byte*)GTextureManager.Textures[tex]->DriverData;
		cachewidth = GTextureManager.Textures[tex]->GetWidth();
		return;
	}

	if (!GTextureManager.Textures[tex]->DriverData)
	{
		GenerateTexture(tex);
	}

	miptexture = (miptexture_t*)GTextureManager.Textures[tex]->DriverData;
	cacheblock = (byte*)miptexture + miptexture->offsets[0];
	unguard;
}

//==========================================================================
//
//	VSoftwareDrawer::SetSpriteLump
//
//==========================================================================

void VSoftwareDrawer::SetSpriteLump(int lump, vuint32 light, int translation)
{
	guard(VSoftwareDrawer::SetSpriteLump);
	light &= 0xf8f8f8f8;

	int i;
	int avail = -1;
	for (i = 0; i <	SPRITE_CACHE_SIZE; i++)
	{
		if (sprite_cache[i].data)
		{
			if (sprite_cache[i].lump == lump &&
				sprite_cache[i].light == light &&
				sprite_cache[i].tnum == translation)
			{
				cacheblock = (byte*)sprite_cache[i].data;
				cachewidth = GTextureManager.Textures[lump]->GetWidth();
				return;
			}
		}
		else
		{
			if (avail < 0)
				avail = i;
		}
	}
	if (avail < 0)
	{
		Z_Free(sprite_cache[sprite_cache_count].data);
		sprite_cache[sprite_cache_count].data = NULL;
		avail = sprite_cache_count;
		sprite_cache_count = (sprite_cache_count + 1) % SPRITE_CACHE_SIZE;
	}

	GenerateSprite(lump, avail, light, translation);
	cacheblock = (byte*)sprite_cache[avail].data;
	cachewidth = GTextureManager.Textures[lump]->GetWidth();
	unguard;
}

//==========================================================================
//
//	VSoftwareDrawer::SetPic
//
//==========================================================================

byte* VSoftwareDrawer::SetPic(int handle)
{
	guard(VSoftwareDrawer::SetPic);
	if (!GTextureManager.Textures[handle]->DriverData)
	{
		GeneratePic(handle);
	}
	cachewidth = GTextureManager.Textures[handle]->GetWidth();
	return (byte*)GTextureManager.Textures[handle]->DriverData;
	unguard;
}

//==========================================================================
//
//	VSoftwareDrawer::GenerateTexture
//
//==========================================================================

void VSoftwareDrawer::GenerateTexture(int texnum)
{
	guard(VSoftwareDrawer::GenerateTexture);
	VTexture* Tex = GTextureManager.Textures[texnum];

	byte* block = Tex->GetPixels8();

	int mipw = (Tex->GetWidth() + 15) & ~15;
	int miph = (Tex->GetHeight() + 15) & ~15;
	miptexture_t* mip = (miptexture_t*)Z_Calloc(sizeof(miptexture_t) +
		mipw * miph / 64 * 85);
	Tex->DriverData = mip;
	mip->width = mipw;
	mip->height = miph;
	byte* pSrc = block;
	byte* pDst = (byte*)mip + sizeof(miptexture_t);
	for (int y = 0; y < miph; y++)
	{
		for (int x = 0; x < mipw; x++)
		{
			pDst[x + y * mipw] = pSrc[x % Tex->GetWidth() +
				(y % Tex->GetHeight()) * Tex->GetWidth()];
		}
	}
	Tex->Unload();
	MakeMips(mip);
	unguard;
}

//==========================================================================
//
//	VSoftwareDrawer::MakeMips
//
//==========================================================================

void VSoftwareDrawer::MakeMips(miptexture_t *mip)
{
	guard(VSoftwareDrawer::MakeMips);
	//	Calc offsets.
	mip->offsets[0] = sizeof(miptexture_t);
	mip->offsets[1] = mip->offsets[0] + mip->width * mip->height;
	mip->offsets[2] = mip->offsets[1] + mip->width * mip->height / 4;
	mip->offsets[3] = mip->offsets[2] + mip->width * mip->height / 16;

	for (int miplevel = 1; miplevel < 4; miplevel++)
	{
		//	Setup data.
		int mipw = mip->width >> miplevel;
 		int miph = mip->height >> miplevel;
		int srcrow = mip->width >> (miplevel - 1);
		byte* psrc = (byte*)mip + mip->offsets[miplevel - 1];
		byte* pdst = (byte*)mip + mip->offsets[miplevel];
		for (int i = 0; i < miph; i++)
		{
			for (int j = 0; j < mipw; j++)
			{
				int a = 0;
				int r = 0;
 				int g = 0;
 				int b = 0;

				//	Pixel 1.
				if (!psrc[0])
				{
					a++;
				}
				else
				{
					r += r_palette[psrc[0]].r;
					g += r_palette[psrc[0]].g;
					b += r_palette[psrc[0]].b;
				}

				//	Pixel 2.
				if (!psrc[1])
				{
					a++;
				}
				else
				{
					r += r_palette[psrc[1]].r;
					g += r_palette[psrc[1]].g;
					b += r_palette[psrc[1]].b;
				}

				//	Pixel 3.
				if (!psrc[srcrow])
				{
					a++;
				}
				else
				{
					r += r_palette[psrc[srcrow]].r;
					g += r_palette[psrc[srcrow]].g;
					b += r_palette[psrc[srcrow]].b;
				}

				//	Pixel 4.
				if (!psrc[srcrow + 1])
				{
					a++;
				}
				else
				{
					r += r_palette[psrc[srcrow + 1]].r;
					g += r_palette[psrc[srcrow + 1]].g;
					b += r_palette[psrc[srcrow + 1]].b;
				}

				//	Get color.
				if (a > 2)
				{
					*pdst = 0;
				}
				else
				{
					*pdst = d_rgbtable[((r << 5) & 0x7c00) +
						(g & 0x3e0) + ((b >> 5) & 0x1f)];
				}

				//	Advance pointers.
				psrc += 2;
				pdst++;
			}
			psrc += srcrow;
		}
	}
	unguard;
}

//==========================================================================
//
//	VSoftwareDrawer::LoadSkyMap
//
//==========================================================================

void VSoftwareDrawer::LoadSkyMap(int texnum)
{
	guard(VSoftwareDrawer::LoadSkyMap);
	VTexture* Tex = GTextureManager.Textures[texnum];
	int j;

	byte* Pixels = Tex->GetPixels();
	int NumPixels = Tex->GetWidth() * Tex->GetHeight();
	Tex->DriverData = Z_Malloc(NumPixels * PixelBytes);
	if (Tex->Format == TEXFMT_8 || Tex->Format == TEXFMT_8Pal)
	{
		// Load paletted skymap
		rgba_t* Pal = Tex->GetPalette();
		if (ScreenBPP == 8)
		{
			byte remap[256];

			for (j = 0; j < 256; j++)
			{
				remap[j] = MakeCol8(Pal[j].r, Pal[j].g, Pal[j].b);
			}

			byte *psrc = (byte*)Pixels;
			byte *pdst = (byte*)Tex->DriverData;
			for (j = 0; j < NumPixels; j++, psrc++, pdst++)
			{
				*pdst = remap[*psrc];
			}
		}
		else if (ScreenBPP == 15 || ScreenBPP == 16)
		{
			word remap[256];

			for (j = 0; j < 256; j++)
			{
				remap[j] = MakeCol16(Pal[j].r, Pal[j].g, Pal[j].b);
			}

			byte *psrc = (byte*)Pixels;
			word *pdst = (word*)Tex->DriverData;
			for (j = 0; j < NumPixels; j++, psrc++, pdst++)
			{
				*pdst = remap[*psrc];
			}
		}
		else
		{
			vuint32 remap[256];

			for (j = 0; j < 256; j++)
			{
				remap[j] = MakeCol32(Pal[j].r, Pal[j].g, Pal[j].b);
			}

			byte *psrc = (byte *)Pixels;
			vuint32* pdst = (vuint32*)Tex->DriverData;
			for (j = 0; j < NumPixels; j++, psrc++, pdst++)
			{
				*pdst = remap[*psrc];
			}
		}
	}
	else
	{
		if (ScreenBPP == 8)
		{
			rgba_t *src = (rgba_t*)Pixels;
			byte *dst = (byte*)Tex->DriverData;
			for (j = 0; j < NumPixels; j++, src++, dst++)
			{
				*dst = MakeCol8(src->r, src->g, src->b);
			}
		}
		else if (ScreenBPP == 15 || ScreenBPP == 16)
		{
			rgba_t *src = (rgba_t*)Pixels;
			word *dst = (word*)Tex->DriverData;
			for (j = 0; j < NumPixels; j++, src++, dst++)
			{
				*dst = MakeCol16(src->r, src->g, src->b);
			}
		}
		else
		{
			rgba_t *src = (rgba_t*)Pixels;
			vuint32* dst = (vuint32*)Tex->DriverData;
			for (j = 0; j < NumPixels; j++, src++, dst++)
			{
				*dst = MakeCol32(src->r, src->g, src->b);
			}
		}
	}
	Tex->Unload();
	unguard;
}

//==========================================================================
//
//	VSoftwareDrawer::GenerateSprite
//
//==========================================================================

void VSoftwareDrawer::GenerateSprite(int lump, int slot, vuint32 light,
	int translation)
{
	guard(VSoftwareDrawer::GenerateSprite);
	VTexture* Tex = GTextureManager.Textures[lump];

	int w = Tex->GetWidth();
	int h = Tex->GetHeight();

	byte* SrcBlock = Tex->GetPixels8();

	void *block = (byte*)Z_Calloc(w * h * PixelBytes);
	sprite_cache[slot].data = block;
	sprite_cache[slot].light = light;
	sprite_cache[slot].lump = lump;
	sprite_cache[slot].tnum = translation;

	int lightr = (light >> 19) & 0x1f;
	int lightg = (light >> 11) & 0x1f;
	int lightb = (light >> 3) & 0x1f;
	bool colored = (lightr != lightg) || (lightr != lightb);

	void *cmap;
	void *cmapr;
	void *cmapg;
	void *cmapb;
	int cmapnum = (31 - (light >> 27));
	if (ScreenBPP == 8)
	{
		cmap = fadetable + cmapnum * 256;
		cmapr = fadetable16r + (31 - lightr) * 256;
		cmapg = fadetable16g + (31 - lightg) * 256;
		cmapb = fadetable16b + (31 - lightb) * 256;
	}
	else if (PixelBytes == 2)
	{
		cmap = fadetable16 + cmapnum * 256;
		cmapr = fadetable16r + (31 - lightr) * 256;
		cmapg = fadetable16g + (31 - lightg) * 256;
		cmapb = fadetable16b + (31 - lightb) * 256;
	}
	else
	{
		cmap = fadetable32 + cmapnum * 256;
		cmapr = fadetable32r + (31 - lightr) * 256;
		cmapg = fadetable32g + (31 - lightg) * 256;
		cmapb = fadetable32b + (31 - lightb) * 256;
	}

	byte *trtab = translationtables + translation * 256;

	int count =  w * h;
	byte* source = SrcBlock;
	if (ScreenBPP == 8 && colored)
	{
		byte* dest = (byte*)block;
		while (count--)
		{
			if (*source)
			{
				int itmp = trtab[*source];
				*dest = d_rgbtable[(((word*)cmapr)[itmp]) |
					(((word*)cmapg)[itmp]) | (((word*)cmapb)[itmp])];
			}
			source++;
			dest++;
		}
	}
	else if (ScreenBPP == 8)
	{
		byte* dest = (byte*)block;
		while (count--)
		{
			if (*source)
				*dest = ((byte*)cmap)[trtab[*source]];
			source++;
			dest++;
		}
	}
	else if (PixelBytes == 2 && colored)
	{
		word* dest = (word*)block;
		while (count--)
		{
			if (*source)
			{
				int itmp = trtab[*source];
				*dest = (((word*)cmapr)[itmp]) |
					(((word*)cmapg)[itmp]) | (((word*)cmapb)[itmp]);
				if (!*dest)
					*dest = 1;
			}
			source++;
			dest++;
		}
	}
	else if (PixelBytes == 2)
	{
		word* dest = (word*)block;
		while (count--)
		{
			if (*source)
				*dest = ((word*)cmap)[trtab[*source]];
			source++;
			dest++;
		}
	}
	else if (colored)
	{
		vuint32* dest = (vuint32*)block;
		while (count--)
		{
			if (*source)
			{
				int itmp = trtab[*source];
				*dest = MakeCol32(((byte*)cmapr)[itmp],
					((byte*)cmapg)[itmp], ((byte*)cmapb)[itmp]);
				if (!*dest)
					*dest = 1;
			}
			source++;
			dest++;
		}
	}
	else
	{
		vuint32* dest = (vuint32*)block;
		while (count--)
		{
			if (*source)
				*dest = ((vuint32*)cmap)[trtab[*source]];
			source++;
			dest++;
		}
	}
	unguard;
}

//==========================================================================
//
//	VSoftwareDrawer::GeneratePic
//
//==========================================================================

void VSoftwareDrawer::GeneratePic(int texnum)
{
	guard(GeneratePic);
	VTexture* Tex = GTextureManager.Textures[texnum];
	byte* Pixels = Tex->GetPixels8();
	int NumPixels = Tex->GetWidth() * Tex->GetHeight();
	Tex->DriverData = (byte*)Z_Malloc(NumPixels);
	memcpy(Tex->DriverData, Pixels, NumPixels);
	Tex->Unload();
	unguard;
}
