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
	void*					data;
	vuint32					light;
	VTexture*				Tex;
	VTextureTranslation*	tnum;
	int						ColourMap;
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

	for (i = 0; i < GTextureManager.GetNumTextures(); i++)
	{
		if (GTextureManager[i]->Type == TEXTYPE_SkyMap &&
			GTextureManager[i]->DriverData)
		{
			Z_Free(GTextureManager[i]->DriverData);
			GTextureManager[i]->DriverData = NULL;
		}
	}
	unguard;
}

//==========================================================================
//
//	VSoftwareDrawer::FlushTexture
//
//==========================================================================

void VSoftwareDrawer::FlushTexture(VTexture* Tex)
{
	guard(VSoftwareDrawer::FlushTexture);
	if (Tex->DriverData)
	{
		Z_Free(Tex->DriverData);
		Tex->DriverData = NULL;
	}
	for (int j = 0; j < Tex->DriverTranslated.Num(); j++)
	{
		Z_Free(Tex->DriverTranslated[j].Data);
	}
	Tex->DriverTranslated.Clear();
	SCInvalidateTexture(Tex);
	unguard;
}

//==========================================================================
//
// 	VSoftwareDrawer::PrecacheTexture
//
//==========================================================================

void VSoftwareDrawer::PrecacheTexture(VTexture* Tex)
{
	guard(VSoftwareDrawer::PrecacheTexture);
	SetTexture(Tex, 0);
	unguard;
}

//==========================================================================
//
// 	VSoftwareDrawer::SetTexture
//
//==========================================================================

void VSoftwareDrawer::SetTexture(VTexture* Tex, int CMap)
{
	guard(VSoftwareDrawer::SetTexture);
	if (Tex->CheckModified())
	{
		FlushTexture(Tex);
	}

	if (Tex->Type == TEXTYPE_SkyMap)
	{
		if (CMap)
		{
			VTexture::VTransData* TData = Tex->FindDriverTrans(NULL, CMap);
			if (!TData)
			{
				TData = &Tex->DriverTranslated.Alloc();
				TData->Data = NULL;
				TData->Trans = NULL;
				TData->ColourMap = CMap;
				LoadSkyMap(Tex, TData->Data, CMap);
			}
			cacheblock = (vuint8*)TData->Data;
		}
		else
		{
			if (!Tex->DriverData)
			{
				LoadSkyMap(Tex, Tex->DriverData, 0);
			}
			cacheblock = (vuint8*)Tex->DriverData;
		}
		cachewidth = Tex->GetWidth();
		return;
	}

	if (CMap)
	{
		VTexture::VTransData* TData = Tex->FindDriverTrans(NULL, CMap);
		if (!TData)
		{
			TData = &Tex->DriverTranslated.Alloc();
			TData->Data = NULL;
			TData->Trans = NULL;
			TData->ColourMap = CMap;
			GenerateTexture(Tex, TData->Data, CMap);
		}
		miptexture = (miptexture_t*)TData->Data;
	}
	else
	{
		if (!Tex->DriverData)
		{
			GenerateTexture(Tex, Tex->DriverData, 0);
		}
		miptexture = (miptexture_t*)Tex->DriverData;
	}

	cacheblock = (vuint8*)miptexture + miptexture->offsets[0];
	unguard;
}

//==========================================================================
//
//	VSoftwareDrawer::SetSpriteLump
//
//==========================================================================

void VSoftwareDrawer::SetSpriteLump(VTexture* Tex, vuint32 light,
	VTextureTranslation* Translation, int CMap)
{
	guard(VSoftwareDrawer::SetSpriteLump);
	light &= 0xf8f8f8f8;

	int i;
	int avail = -1;
	//	If texture has been modified, free all instances of this texture.
	if (Tex->CheckModified())
	{
		for (i = 0; i <	SPRITE_CACHE_SIZE; i++)
		{
			if (sprite_cache[i].data && sprite_cache[i].Tex == Tex)
			{
				Z_Free(sprite_cache[i].data);
				sprite_cache[i].data = NULL;
			}
		}
	}
	for (i = 0; i <	SPRITE_CACHE_SIZE; i++)
	{
		if (sprite_cache[i].data)
		{
			if (sprite_cache[i].Tex == Tex &&
				sprite_cache[i].light == light &&
				sprite_cache[i].tnum == Translation &&
				sprite_cache[i].ColourMap == CMap)
			{
				cacheblock = (vuint8*)sprite_cache[i].data;
				cachewidth = Tex->GetWidth();
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

	GenerateSprite(Tex, avail, light, Translation, CMap);
	cacheblock = (vuint8*)sprite_cache[avail].data;
	cachewidth = Tex->GetWidth();
	unguard;
}

//==========================================================================
//
//	VSoftwareDrawer::SetPic
//
//==========================================================================

vuint8* VSoftwareDrawer::SetPic(VTexture* Tex, VTextureTranslation* Trans,
	int CMap)
{
	guard(VSoftwareDrawer::SetPic);
	if (Tex->CheckModified())
	{
		FlushTexture(Tex);
	}
	if (Trans || CMap)
	{
		VTexture::VTransData* TData = Tex->FindDriverTrans(Trans, CMap);
		if (!TData)
		{
			TData = &Tex->DriverTranslated.Alloc();
			TData->Data = NULL;
			TData->Trans = Trans;
			TData->ColourMap = CMap;
			GeneratePic(Tex, &TData->Data, Trans, CMap);
		}
		cachewidth = Tex->GetWidth();
		return (vuint8*)TData->Data;
	}
	else
	{
		if (!Tex->DriverData)
		{
			GeneratePic(Tex, &Tex->DriverData, Trans, CMap);
		}
		cachewidth = Tex->GetWidth();
		return (vuint8*)Tex->DriverData;
	}
	unguard;
}

//==========================================================================
//
//	VSoftwareDrawer::GenerateTexture
//
//==========================================================================

void VSoftwareDrawer::GenerateTexture(VTexture* Tex, void*& DataPtr, int CMap)
{
	guard(VSoftwareDrawer::GenerateTexture);
	vuint8* block = Tex->GetPixels8();

	int mipw = (Tex->GetWidth() + 15) & ~15;
	int miph = (Tex->GetHeight() + 15) & ~15;
	if (!DataPtr)
	{
		DataPtr = Z_Malloc(sizeof(miptexture_t) +
			mipw * miph / 64 * 85);
	}
	miptexture_t* mip = (miptexture_t*)DataPtr;
	memset(mip, 0, sizeof(miptexture_t) + mipw * miph / 64 * 85);
	mip->width = mipw;
	mip->height = miph;
	vuint8* pSrc = block;
	vuint8* pDst = (vuint8*)mip + sizeof(miptexture_t);
	const vuint8* CMTab = CMap ? ColourMaps[CMap].GetTable() : NULL;
	for (int y = 0; y < miph; y++)
	{
		for (int x = 0; x < mipw; x++)
		{
			pDst[x + y * mipw] = pSrc[x % Tex->GetWidth() +
				(y % Tex->GetHeight()) * Tex->GetWidth()];
			if (CMTab)
			{
				pDst[x + y * mipw] = CMTab[pDst[x + y * mipw]];
			}
		}
	}
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
		vuint8* psrc = (vuint8*)mip + mip->offsets[miplevel - 1];
		vuint8* pdst = (vuint8*)mip + mip->offsets[miplevel];
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

				//	Get colour.
				if (a > 2)
				{
					*pdst = 0;
				}
				else
				{
					*pdst = r_rgbtable[((r << 5) & 0x7c00) +
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

void VSoftwareDrawer::LoadSkyMap(VTexture* Tex, void*& DataPtr, int CMap)
{
	guard(VSoftwareDrawer::LoadSkyMap);
	int j;

	vuint8* Pixels = Tex->GetPixels();
	int NumPixels = Tex->GetWidth() * Tex->GetHeight();
	if (!DataPtr)
	{
		DataPtr = Z_Malloc(NumPixels * PixelBytes);
	}
	if (Tex->Format == TEXFMT_8 || Tex->Format == TEXFMT_8Pal || CMap)
	{
		rgba_t CMappedPal[256];
		const rgba_t* Pal = Tex->GetPalette();

		//	If it's colour-mapped, get 8 bit version and calculate palette.
		if (CMap)
		{
			if (Tex->Format == TEXFMT_RGBA)
			{
				Pixels = Tex->GetPixels8();
				Pal = ColourMaps[CMap].GetPalette();
			}
			else if (Tex->Format == TEXFMT_8Pal)
			{
				const rgba_t* CMPal = ColourMaps[CMap].GetPalette();
				for (int i = 0; i < 256; i++)
				{
					int Col = R_LookupRGB(Pal[i].r, Pal[i].g, Pal[i].b);
					CMappedPal[i] = CMPal[Col];
				}
				Pal = CMappedPal;
			}
			else
			{
				Pal = ColourMaps[CMap].GetPalette();
			}
		}

		// Load paletted skymap
		if (ScreenBPP == 8)
		{
			vuint8 remap[256];

			for (j = 0; j < 256; j++)
			{
				remap[j] = MakeCol8(Pal[j].r, Pal[j].g, Pal[j].b);
			}

			vuint8 *psrc = (vuint8*)Pixels;
			vuint8 *pdst = (vuint8*)DataPtr;
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

			vuint8 *psrc = (vuint8*)Pixels;
			word *pdst = (word*)DataPtr;
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

			vuint8 *psrc = (vuint8 *)Pixels;
			vuint32* pdst = (vuint32*)DataPtr;
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
			vuint8 *dst = (vuint8*)DataPtr;
			for (j = 0; j < NumPixels; j++, src++, dst++)
			{
				*dst = MakeCol8(src->r, src->g, src->b);
			}
		}
		else if (ScreenBPP == 15 || ScreenBPP == 16)
		{
			rgba_t *src = (rgba_t*)Pixels;
			word *dst = (word*)DataPtr;
			for (j = 0; j < NumPixels; j++, src++, dst++)
			{
				*dst = MakeCol16(src->r, src->g, src->b);
			}
		}
		else
		{
			rgba_t *src = (rgba_t*)Pixels;
			vuint32* dst = (vuint32*)DataPtr;
			for (j = 0; j < NumPixels; j++, src++, dst++)
			{
				*dst = MakeCol32(src->r, src->g, src->b);
			}
		}
	}
	unguard;
}

//==========================================================================
//
//	VSoftwareDrawer::GenerateSprite
//
//==========================================================================

void VSoftwareDrawer::GenerateSprite(VTexture* Tex, int slot, vuint32 light,
	VTextureTranslation* Translation, int CMap)
{
	guard(VSoftwareDrawer::GenerateSprite);
	int w = Tex->GetWidth();
	int h = Tex->GetHeight();

	vuint8* SrcBlock = Tex->GetPixels8();

	void *block = (vuint8*)Z_Calloc(w * h * PixelBytes);
	sprite_cache[slot].data = block;
	sprite_cache[slot].light = light;
	sprite_cache[slot].Tex = Tex;
	sprite_cache[slot].tnum = Translation;
	sprite_cache[slot].ColourMap = CMap;

	int lightr = (light >> 19) & 0x1f;
	int lightg = (light >> 11) & 0x1f;
	int lightb = (light >> 3) & 0x1f;
	bool coloured = (lightr != lightg) || (lightr != lightb);

	void *cmap;
	void *cmapr;
	void *cmapg;
	void *cmapb;
	int cmapnum = (31 - (light >> 27));
	if (ScreenBPP == 8)
	{
		cmap = d_fadetable + cmapnum * 256;
		cmapr = d_fadetable16r + (31 - lightr) * 256;
		cmapg = d_fadetable16g + (31 - lightg) * 256;
		cmapb = d_fadetable16b + (31 - lightb) * 256;
	}
	else if (PixelBytes == 2)
	{
		cmap = d_fadetable16 + cmapnum * 256;
		cmapr = d_fadetable16r + (31 - lightr) * 256;
		cmapg = d_fadetable16g + (31 - lightg) * 256;
		cmapb = d_fadetable16b + (31 - lightb) * 256;
	}
	else
	{
		cmap = d_fadetable32 + cmapnum * 256;
		cmapr = d_fadetable32r + (31 - lightr) * 256;
		cmapg = d_fadetable32g + (31 - lightg) * 256;
		cmapb = d_fadetable32b + (31 - lightb) * 256;
	}

	const vuint8* trtab = Translation ? Translation->GetTable() : NULL;
	const vuint8* CMTab = CMap ? ColourMaps[CMap].GetTable() : NULL;

	int count =  w * h;
	vuint8* source = SrcBlock;
	if (ScreenBPP == 8 && coloured)
	{
		vuint8* dest = (vuint8*)block;
		while (count--)
		{
			if (*source)
			{
				int itmp = trtab ? trtab[*source] : *source;
				if (CMTab)
				{
					itmp = CMTab[itmp];
				}
				*dest = r_rgbtable[(((word*)cmapr)[itmp]) |
					(((word*)cmapg)[itmp]) | (((word*)cmapb)[itmp])];
			}
			source++;
			dest++;
		}
	}
	else if (ScreenBPP == 8)
	{
		vuint8* dest = (vuint8*)block;
		while (count--)
		{
			if (*source)
			{
				*dest = ((vuint8*)cmap)[trtab ? trtab[*source] : *source];
				if (CMTab)
				{
					*dest = CMTab[*dest];
				}
			}
			source++;
			dest++;
		}
	}
	else if (PixelBytes == 2 && coloured)
	{
		word* dest = (word*)block;
		while (count--)
		{
			if (*source)
			{
				int itmp = trtab ? trtab[*source] : *source;
				if (CMTab)
				{
					itmp = CMTab[itmp];
				}
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
			{
				int itmp = trtab ? trtab[*source] : *source;
				if (CMTab)
				{
					itmp = CMTab[itmp];
				}
				*dest = ((word*)cmap)[itmp];
			}
			source++;
			dest++;
		}
	}
	else if (coloured)
	{
		vuint32* dest = (vuint32*)block;
		while (count--)
		{
			if (*source)
			{
				int itmp = trtab ? trtab[*source] : *source;
				if (CMTab)
				{
					itmp = CMTab[itmp];
				}
				*dest = MakeCol32(((vuint8*)cmapr)[itmp],
					((vuint8*)cmapg)[itmp], ((vuint8*)cmapb)[itmp]);
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
			{
				int itmp = trtab ? trtab[*source] : *source;
				if (CMTab)
				{
					itmp = CMTab[itmp];
				}
				*dest = ((vuint32*)cmap)[itmp];
			}
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

void VSoftwareDrawer::GeneratePic(VTexture* Tex, void** pData,
	VTextureTranslation* Trans, int CMap)
{
	guard(GeneratePic);
	vuint8* Pixels = Tex->GetPixels8();
	int NumPixels = Tex->GetWidth() * Tex->GetHeight();
	if (!*pData)
	{
		*pData = (vuint8*)Z_Malloc(NumPixels);
	}
	memcpy(*pData, Pixels, NumPixels);
	if (Trans)
	{
		const vuint8* TrTab = Trans->GetTable();
		vuint8* pPix = (vuint8*)*pData;
		for (int i = 0; i < NumPixels; i++, pPix++)
		{
			*pPix = TrTab[*pPix];
		}
	}
	if (CMap)
	{
		const vuint8* TrTab = ColourMaps[CMap].GetTable();
		vuint8* pPix = (vuint8*)*pData;
		for (int i = 0; i < NumPixels; i++, pPix++)
		{
			*pPix = TrTab[*pPix];
		}
	}
	unguard;
}
