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

#include "d_local.h"

// MACROS ------------------------------------------------------------------

#define SPRITE_CACHE_SIZE	256
#define	MAX_SKIN_CACHE		256

// TYPES -------------------------------------------------------------------

struct sprite_cache_t
{
	void*		data;
	dword		light;
	int			lump;
	int			tnum;
};

struct skincache_t
{
	char		name[64];
	void		*data;
};

// EXTERNAL FUNCTION PROTOTYPES --------------------------------------------

// PUBLIC FUNCTION PROTOTYPES ----------------------------------------------

// PRIVATE FUNCTION PROTOTYPES ---------------------------------------------

// EXTERNAL DATA DECLARATIONS ----------------------------------------------

// PUBLIC DATA DEFINITIONS -------------------------------------------------

miptexture_t		*miptexture;

// PRIVATE DATA DEFINITIONS ------------------------------------------------

static miptexture_t**	texturedata;
static miptexture_t**	flatdata;
static void**			skymapdata;

static sprite_cache_t	sprite_cache[SPRITE_CACHE_SIZE];
static int				sprite_cache_count;

static skincache_t		skincache[MAX_SKIN_CACHE];

static byte				*picdata[MAX_PICS];
static int				picwidth[MAX_PICS];

// CODE --------------------------------------------------------------------

//==========================================================================
//
//	VSoftwareDrawer::InitTextures
//
//==========================================================================

void VSoftwareDrawer::InitTextures(void)
{
	guard(VSoftwareDrawer::InitTextures);
	//	Textures
	texturedata = Z_CNew<miptexture_t*>(numtextures);
	//	Flats
	flatdata = Z_CNew<miptexture_t*>(numflats);
	//	Skyboxes
	if (numskymaps)
	{
		skymapdata = Z_CNew<void *>(numskymaps);
	}
	unguard;
}

//==========================================================================
//
//	VSoftwareDrawer::FlushTextureCaches
//
//==========================================================================

void VSoftwareDrawer::FlushTextureCaches(void)
{
	guard(VSoftwareDrawer::FlushTextureCaches);
	int		i;

	for (i = 0; i < SPRITE_CACHE_SIZE; i++)
	{
		if (sprite_cache[i].data)
		{
			Z_Free(sprite_cache[i].data);
		}
	}

	for (i = 0; i < numskymaps; i++)
	{
		if (skymapdata[i])
		{
			Z_Free(skymapdata[i]);
		}
	}
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
	//	Get palette.
	rgb_t* pal = (rgb_t*)W_CacheLumpName("playpal", PU_CACHE);

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
					r += pal[psrc[0]].r;
					g += pal[psrc[0]].g;
					b += pal[psrc[0]].b;
				}

				//	Pixel 2.
				if (!psrc[1])
				{
					a++;
				}
				else
				{
					r += pal[psrc[1]].r;
					g += pal[psrc[1]].g;
					b += pal[psrc[1]].b;
				}

				//	Pixel 3.
				if (!psrc[srcrow])
				{
					a++;
				}
				else
				{
					r += pal[psrc[srcrow]].r;
					g += pal[psrc[srcrow]].g;
					b += pal[psrc[srcrow]].b;
				}

				//	Pixel 4.
				if (!psrc[srcrow + 1])
				{
					a++;
				}
				else
				{
					r += pal[psrc[srcrow + 1]].r;
					g += pal[psrc[srcrow + 1]].g;
					b += pal[psrc[srcrow + 1]].b;
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
//	VSoftwareDrawer::DrawColumnInCache
//
// 	Clip and draw a column from a patch into a flat buffer.
//
//		column - column to draw
//		cache - buffer
//		originx, originy - position of column in the buffer
//		cachewidth, cacheheight - size of the cache
//
//==========================================================================

void VSoftwareDrawer::DrawColumnInCache(column_t* column, byte* cache,
	int originx, int originy, int cachewidth, int cacheheight, bool dsky)
{
	guard(VSoftwareDrawer::DrawColumnInCache);
    int		count;
    int		position;
    byte*	source;
    byte*	dest;
	int		top = -1;	//	DeepSea tall patches support
	
	// step through the posts in a column
    while (column->topdelta != 0xff)
    {
		if (column->topdelta <= top)
		{
			top += column->topdelta;
		}
		else
		{
			top = column->topdelta;
		}
		source = (byte *)column + 3;
		count = column->length;
		position = originy + top;

		//	Clip position
		if (position < 0)
		{
	    	count += position;
			source -= position;
	    	position = 0;
		}
		if (position + count > cacheheight)
		{
	    	count = cacheheight - position;
		}
    	dest = cache + originx + position * cachewidth;

    	while (count-- > 0)
    	{
			*dest = *source || dsky ? *source : r_black_color[0];
			source++;
			dest += cachewidth;
    	}
		
		column = (column_t *)((byte *)column + column->length + 4);
    }
	unguard;
}

//==========================================================================
//
//	VSoftwareDrawer::GenerateTexture
//
// 	Using the texture definition, the composite texture is created from the
// patches, and each column is cached.
//
//==========================================================================

void VSoftwareDrawer::GenerateTexture(int texnum, bool double_sky)
{
	guard(VSoftwareDrawer::GenerateTexture);
	miptexture_t	*mip;
    byte*			block;
    texdef_t*		texture;
    texpatch_t*		patch;	
    patch_t*		realpatch;
    int				x;
    int				x1;
    int				x2;
    int				i;
    column_t*		patchcol;
	int				mipw;
	int				miph;
	int				htimes, wtimes, ht, wt;

    texture = textures[texnum];

	mipw = (texture->width + 15) & ~15;
	miph = (texture->height + 15) & ~15;
	mip = (miptexture_t*)Z_Calloc(sizeof(miptexture_t) + mipw * miph / 64 * 85,
		PU_STATIC, (void**)&texturedata[texnum]);
	mip->width = mipw;
	mip->height = miph;
	block = (byte*)mip + sizeof(miptexture_t);

    // Composite the columns together.
    patch = texture->patches;
	wtimes = texture->width < 16 ? 16 / texture->width : 1;
	htimes = texture->height < 16 ? 16 / texture->height : 1;

    for (i = 0; i < texture->patchcount; i++, patch++)
    {
		realpatch = (patch_t*)W_CacheLumpNum(patch->patch, PU_TEMP);
		x1 = patch->originx;
		x2 = x1 + LittleShort(realpatch->width);

		if (x1 < 0)
	    	x = 0;
		else
	    	x = x1;
	
		if (x2 > texture->width)
	    	x2 = texture->width;

		for ( ; x < x2; x++)
		{
	    	patchcol = (column_t *)((byte *)realpatch
				    + LittleLong(realpatch->columnofs[x - x1]));
			for (ht = 0; ht < htimes; ht++)
			{
				for (wt = 0; wt < wtimes; wt++)
				{
		    		DrawColumnInCache(patchcol, block + wt * texture->width +
		    			ht * mipw * texture->height,
			    		x, patch->originy, mipw, texture->height, double_sky);
				}
			}
		}
    }

	MakeMips(mip);

    // Now that the texture has been built in column cache,
    //  it is purgable from zone memory.
    Z_ChangeTag(mip, PU_CACHE);
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
	if (tex & TEXF_FLAT)
	{
		SetFlat(tex);
		return;
	}

	if ((dword)tex >= (dword)numtextures)
		Sys_Error("Invalid texture num %d\n", tex);

	if (!texturedata[tex])
		GenerateTexture(tex, false);

	miptexture = texturedata[tex];
	cacheblock = (byte*)miptexture + miptexture->offsets[0];
	unguard;
}

//==========================================================================
//
//	VSoftwareDrawer::LoadSkyMap
//
//==========================================================================

void VSoftwareDrawer::LoadSkyMap(const char *name, void **dataptr)
{
	guard(VSoftwareDrawer::LoadSkyMap);
	int j;

	Mod_LoadSkin(name, NULL);
	Z_Malloc(SkinWidth * SkinHeight * PixelBytes, PU_STATIC, dataptr);
	if (SkinBPP == 8)
	{
		// Load paletted skymap
		if (ScreenBPP == 8)
		{
			byte remap[256];

			for (j = 0; j < 256; j++)
			{
				remap[j] = MakeCol8(SkinPal[j].r, SkinPal[j].g, SkinPal[j].b);
			}

			byte *psrc = (byte *)SkinData;
			byte *pdst = (byte *)*dataptr;
			for (j = 0; j < SkinWidth * SkinHeight; j++, psrc++, pdst++)
			{
				*pdst = remap[*psrc];
			}
		}
		else if (ScreenBPP == 15 || ScreenBPP == 16)
		{
			word remap[256];

			for (j = 0; j < 256; j++)
			{
				remap[j] = MakeCol16(SkinPal[j].r, SkinPal[j].g, SkinPal[j].b);
			}

			byte *psrc = (byte *)SkinData;
			word *pdst = (word *)*dataptr;
			for (j = 0; j < SkinWidth * SkinHeight; j++, psrc++, pdst++)
			{
				*pdst = remap[*psrc];
			}
		}
		else
		{
			dword remap[256];

			for (j = 0; j < 256; j++)
			{
				remap[j] = MakeCol32(SkinPal[j].r, SkinPal[j].g, SkinPal[j].b);
			}

			byte *psrc = (byte *)SkinData;
			dword *pdst = (dword *)*dataptr;
			for (j = 0; j < SkinWidth * SkinHeight; j++, psrc++, pdst++)
			{
				*pdst = remap[*psrc];
			}
		}
	}
	else
	{
		if (ScreenBPP == 8)
		{
			rgba_t *src = (rgba_t *)SkinData;
			byte *dst = (byte *)*dataptr;
			for (j = 0; j < SkinWidth * SkinHeight; j++, src++, dst++)
			{
				*dst = MakeCol8(src->r, src->g, src->b);
			}
		}
		else if (ScreenBPP == 15 || ScreenBPP == 16)
		{
			rgba_t *src = (rgba_t *)SkinData;
			word *dst = (word *)*dataptr;
			for (j = 0; j < SkinWidth * SkinHeight; j++, src++, dst++)
			{
				*dst = MakeCol16(src->r, src->g, src->b);
			}
		}
		else
		{
			rgba_t *src = (rgba_t *)SkinData;
			dword *dst = (dword *)*dataptr;
			for (j = 0; j < SkinWidth * SkinHeight; j++, src++, dst++)
			{
				*dst = MakeCol32(src->r, src->g, src->b);
			}
		}
	}
	Z_ChangeTag(*dataptr, PU_CACHE);
	Z_Free(SkinData);
	unguard;
}

//==========================================================================
//
// 	VSoftwareDrawer::SetSkyTexture
//
//==========================================================================

void VSoftwareDrawer::SetSkyTexture(int tex, bool double_sky)
{
	guard(VSoftwareDrawer::SetSkyTexture);
	if (tex & TEXF_SKY_MAP)
	{
		tex &= ~TEXF_SKY_MAP;
		if (!skymapdata[tex])
		{
			LoadSkyMap(skymaps[tex].name, &skymapdata[tex]);
			skymaps[tex].width = SkinWidth;
			skymaps[tex].height = SkinHeight;
		}
		cacheblock = (byte *)skymapdata[tex];
		cachewidth = skymaps[tex].width;
		return;
	}

	if (tex & TEXF_FLAT)
	{
		SetFlat(tex);
	}
	else
	{
		if (!texturedata[tex])
			GenerateTexture(tex, double_sky);

		miptexture = texturedata[tex];
	}
	unguard;
}

//==========================================================================
//
//	VSoftwareDrawer::GenerateFlat
//
//==========================================================================

void VSoftwareDrawer::GenerateFlat(int num)
{
	guard(VSoftwareDrawer::GenerateFlat);
	miptexture_t	*mip;
	byte			*block, *data;

	mip = (miptexture_t*)Z_Malloc(85 * 64 + sizeof(miptexture_t),
		PU_STATIC, (void**)&flatdata[num]);

	mip->width = 64;
	mip->height = 64;

	block = (byte*)mip + sizeof(miptexture_t);
	data = (byte*)W_CacheLumpNum(flatlumps[num], PU_TEMP);
	for (int j = 0; j < 64; j++)
	{
		for (int i = 0; i < 64; i++)
		{
			byte pix = data[(j << 6) + i];
			block[(j << 6) + i] = pix ? pix : r_black_color[0];
		}
	}

	MakeMips(mip);
	Z_ChangeTag(mip, PU_CACHE);
	unguard;
}

//==========================================================================
//
//	VSoftwareDrawer::SetFlat
//
//==========================================================================

void VSoftwareDrawer::SetFlat(int num)
{
	guard(VSoftwareDrawer::SetFlat);
	num &= ~TEXF_FLAT;

	if (!flatdata[num])
	{
		GenerateFlat(num);
	}

    miptexture = flatdata[num];
    cacheblock = (byte*)miptexture + miptexture->offsets[0];
	unguard;
}

//==========================================================================
//
//	VSoftwareDrawer::GenerateSprite
//
//==========================================================================

void VSoftwareDrawer::GenerateSprite(int lump, int slot, dword light, int translation)
{
	guard(VSoftwareDrawer::GenerateSprite);
    patch_t	*patch = (patch_t*)W_CacheLumpNum(spritelumps[lump], PU_STATIC);

	int w = LittleShort(patch->width);
	int h = LittleShort(patch->height);

    void *block = (byte*)Z_Calloc(w * h * PixelBytes, PU_CACHE,
    	&sprite_cache[slot].data);
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

	for (int x = 0; x < w; x++)
	{
    	column_t *column = (column_t *)((byte *)patch + LittleLong(patch->columnofs[x]));

		// step through the posts in a column
		int top = -1;	//	DeepSea tall patches support
	    while (column->topdelta != 0xff)
	    {
			if (column->topdelta <= top)
			{
				top += column->topdelta;
			}
			else
			{
				top = column->topdelta;
			}
		    byte* source = (byte *)column + 3;
			int count = column->length;

			if (ScreenBPP == 8 && colored)
			{
			    byte* dest = ((byte*)block) + x + top * w;
		    	while (count--)
	    		{
					int itmp = trtab[*source ? *source : r_black_color[0]];
					*dest = d_rgbtable[(((word*)cmapr)[itmp]) |
						(((word*)cmapg)[itmp]) | (((word*)cmapb)[itmp])];
					source++;
					dest += w;
		    	}
			}
			else if (ScreenBPP == 8)
			{
			    byte* dest = ((byte*)block) + x + top * w;
		    	while (count--)
	    		{
					*dest = ((byte*)cmap)[trtab[*source ? *source : r_black_color[0]]];
					source++;
					dest += w;
		    	}
			}
			else if (PixelBytes == 2 && colored)
			{
			    word* dest = ((word*)block) + x + top * w;
		    	while (count--)
	    		{
					int itmp = trtab[*source ? *source : r_black_color[0]];
					*dest = (((word*)cmapr)[itmp]) |
						(((word*)cmapg)[itmp]) | (((word*)cmapb)[itmp]);
					if (!*dest) *dest = 1;
					source++;
					dest += w;
		    	}
			}
			else if (PixelBytes == 2)
			{
			    word* dest = ((word*)block) + x + top * w;
		    	while (count--)
	    		{
					*dest = ((word*)cmap)[trtab[*source ? *source : r_black_color[0]]];
					source++;
					dest += w;
		    	}
			}
			else if (colored)
			{
			    dword* dest = ((dword*)block) + x + top * w;
		    	while (count--)
	    		{
					int itmp = trtab[*source ? *source : r_black_color[0]];
					*dest = MakeCol32(((byte*)cmapr)[itmp],
						((byte*)cmapg)[itmp], ((byte*)cmapb)[itmp]);
					if (!*dest) *dest = 1;
					source++;
					dest += w;
		    	}
			}
			else
			{
			    dword* dest = ((dword*)block) + x + top * w;
		    	while (count--)
	    		{
					*dest = ((dword*)cmap)[trtab[*source ? *source : r_black_color[0]]];
					source++;
					dest += w;
		    	}
			}
			column = (column_t *)((byte *)column + column->length + 4);
	    }
	}

	Z_ChangeTag(patch, PU_CACHE);
	unguard;
}

//==========================================================================
//
//	VSoftwareDrawer::SetSpriteLump
//
//==========================================================================

void VSoftwareDrawer::SetSpriteLump(int lump, dword light, int translation)
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
				cachewidth = spritewidth[lump];
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
		avail = sprite_cache_count;
		sprite_cache_count = (sprite_cache_count + 1) % SPRITE_CACHE_SIZE;
	}

	GenerateSprite(lump, avail, light, translation);
	cacheblock = (byte*)sprite_cache[avail].data;
	cachewidth = spritewidth[lump];
	unguard;
}

//==========================================================================
//
//	VSoftwareDrawer::LoadImage
//
//==========================================================================

void VSoftwareDrawer::LoadImage(const char *name, void **dataptr)
{
	guard(D_LoadImage);
	int j;

	Mod_LoadSkin(name, dataptr);
	if (SkinBPP == 8)
	{
		// Remap to game palette
		byte remap[256];
		byte *tmp;

		for (j = 0; j < 256; j++)
		{
			remap[j] = MakeCol8(SkinPal[j].r, SkinPal[j].g, SkinPal[j].b);
		}

		tmp = (byte *)SkinData;
		for (j = 0; j < SkinWidth * SkinHeight; j++, tmp++)
		{
			*tmp = remap[*tmp];
		}
	}
	else
	{
		byte *tmp = (byte *)Z_Malloc(SkinWidth * SkinHeight, PU_STATIC,
			dataptr);
		rgba_t *src = (rgba_t *)SkinData;
		for (j = 0; j < SkinWidth * SkinHeight; j++, tmp++, src++)
		{
			*tmp = MakeCol8(src->r, src->g, src->b);
		}
		Z_Free(SkinData);
	}
	unguard;
}

//==========================================================================
//
//	VSoftwareDrawer::SetSkin
//
//==========================================================================

void* VSoftwareDrawer::SetSkin(const char *name)
{
	guard(SetSkin);
	int i;
	int avail;

	avail = -1;
	for (i = 0; i < MAX_SKIN_CACHE; i++)
	{
		if (skincache[i].data)
		{
			if (!strcmp(skincache[i].name, name))
			{
				break;
			}
		}
		else
		{
			if (avail < 0)
				avail = i;
		}
	}

	if (i == MAX_SKIN_CACHE)
	{
		// Not in cache, load it
		if (avail < 0)
		{
			avail = 0;
			Z_Free(skincache[avail].data);
		}
		i = avail;
		strcpy(skincache[i].name, name);
		LoadImage(name, &skincache[i].data);
	}

	return skincache[i].data;
	unguard;
}

//==========================================================================
//
//	VSoftwareDrawer::GeneratePicFromPatch
//
//==========================================================================

void VSoftwareDrawer::GeneratePicFromPatch(int handle)
{
	guard(GeneratePicFromPatch);
	patch_t *patch = (patch_t*)W_CacheLumpName(pic_list[handle].name, PU_TEMP);
	int w = LittleShort(patch->width);
	int h = LittleShort(patch->height);
	byte *block = (byte*)Z_Calloc(w * h, PU_CACHE, (void**)&picdata[handle]);
	picdata[handle] = block;
	picwidth[handle] = w;
	int black = r_black_color[pic_list[handle].palnum];

	for (int x = 0; x < w; x++)
	{
    	column_t *column = (column_t *)((byte *)patch + LittleLong(patch->columnofs[x]));

		// step through the posts in a column
		int top = -1;	//	DeepSea tall patches support
	    while (column->topdelta != 0xff)
	    {
			if (column->topdelta <= top)
			{
				top += column->topdelta;
			}
			else
			{
				top = column->topdelta;
			}
		    byte* source = (byte *)column + 3;
		    byte* dest = block + x + top * w;
			int count = column->length;

	    	while (count--)
	    	{
				*dest = *source ? *source : black;
				source++;
				dest += w;
	    	}
			column = (column_t *)((byte *)column + column->length + 4);
	    }
	}

	if (pic_list[handle].palnum)
	{
		byte remap[256];
		rgba_t *pal = r_palette[pic_list[handle].palnum];

		remap[0] = 0;
		for (int pali = 1; pali < 256; pali++)
		{
			remap[pali] = MakeCol8(pal[pali].r, pal[pali].g, pal[pali].b);
		}
		for (int i = 0; i < w * h; i++)
		{
			block[i] = remap[block[i]];
		}
	}
	unguard;
}

//==========================================================================
//
//	VSoftwareDrawer::GeneratePicFromRaw
//
//==========================================================================

void VSoftwareDrawer::GeneratePicFromRaw(int handle)
{
	guard(GeneratePicFromRaw);
	picdata[handle] = (byte*)Z_Malloc(320 * 200, PU_CACHE, (void**)&picdata[handle]);
	W_ReadLump(W_GetNumForName(pic_list[handle].name), picdata[handle]);

	byte remap[256];
	if (pic_list[handle].palnum)
	{
		rgba_t *pal = r_palette[pic_list[handle].palnum];

		for (int pali = 0; pali < 256; pali++)
		{
			remap[pali] = MakeCol8(pal[pali].r, pal[pali].g, pal[pali].b);
		}
	}
	else
	{
		remap[0] = r_black_color[pic_list[handle].palnum];
		for (int pali = 1; pali < 256; pali++)
		{
			remap[pali] = pali;
		}
	}

	for (int i = 0; i < 320 * 200; i++)
	{
		picdata[handle][i] = remap[picdata[handle][i]];
	}
	picwidth[handle] = 320;
	unguard;
}

//==========================================================================
//
//	VSoftwareDrawer::SetPic
//
//==========================================================================

byte* VSoftwareDrawer::SetPic(int handle)
{
	if (!picdata[handle])
	{
		switch (pic_list[handle].type)
 		{
	 	 case PIC_PATCH:
			GeneratePicFromPatch(handle);
			break;

		 case PIC_RAW:
			GeneratePicFromRaw(handle);
			break;
		}
	}
	cachewidth = picwidth[handle];
	return picdata[handle];
}

//**************************************************************************
//
//	$Log$
//	Revision 1.11  2002/11/16 17:11:15  dj_jl
//	Improving software driver class.
//
//	Revision 1.10  2002/07/13 07:38:00  dj_jl
//	Added drawers to the object tree.
//	
//	Revision 1.9  2002/03/20 19:09:53  dj_jl
//	DeepSea tall patches support.
//	
//	Revision 1.8  2002/01/07 12:16:42  dj_jl
//	Changed copyright year
//	
//	Revision 1.7  2001/11/02 18:35:55  dj_jl
//	Sky optimizations
//	
//	Revision 1.6  2001/10/18 17:36:31  dj_jl
//	A lots of changes for Alpha 2
//	
//	Revision 1.5  2001/08/23 17:47:22  dj_jl
//	Started work on pics with custom palettes
//	
//	Revision 1.4  2001/08/21 17:46:08  dj_jl
//	Added R_TextureAnimation, made SetTexture recognize flats
//	
//	Revision 1.3  2001/07/31 17:16:30  dj_jl
//	Just moved Log to the end of file
//	
//	Revision 1.2  2001/07/27 14:27:54  dj_jl
//	Update with Id-s and Log-s, some fixes
//
//**************************************************************************
