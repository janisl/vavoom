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
//**************************************************************************

// HEADER FILES ------------------------------------------------------------

#include "d_local.h"

// MACROS ------------------------------------------------------------------

#define SPRITE_CACHE_SIZE			256

// TYPES -------------------------------------------------------------------

struct sprite_cache_t
{
	void*		data;
	dword		light;
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

static miptexture_t**	texturedata;
static byte**			skytexturedata;	//FIXME this is a waste of memory
static miptexture_t**	flatdata;

static sprite_cache_t	sprite_cache[SPRITE_CACHE_SIZE];
static int				sprite_cache_count;

// CODE --------------------------------------------------------------------

//==========================================================================
//
//	TSoftwareDrawer::InitTextures
//
//==========================================================================

void TSoftwareDrawer::InitTextures(void)
{
	//	Textures
    texturedata = (miptexture_t**)Z_Calloc(numtextures * 4);
    skytexturedata = (byte**)Z_Calloc(numtextures * 4);
	//	Flats
    flatdata = (miptexture_t**)Z_Calloc((numflats) * 4, PU_STATIC, 0);
}

//==========================================================================
//
//	D_FlushSpriteCache
//
//==========================================================================

void D_FlushSpriteCache(void)
{
	int		i;

	for (i = 0; i < SPRITE_CACHE_SIZE; i++)
	{
		if (sprite_cache[i].data)
		{
			Z_Free(sprite_cache[i].data);
		}
	}
}

int			mip_r, mip_g, mip_b, mip_a;
rgb_t		*mip_pal;

void MipPixel(int pix)
{
	if (!pix)
	{
		mip_a++;
	}
	else
	{
		mip_r += mip_pal[pix].r;
		mip_g += mip_pal[pix].g;
		mip_b += mip_pal[pix].b;
	}
}

byte MipColor(void)
{
	if (mip_a > 2)
	{
		return 0;
	}
	return d_rgbtable[((mip_r << 5) & 0x7c00) +
		(mip_g & 0x3e0) + ((mip_b >> 5) & 0x1f)];
}

//==========================================================================
//
//	MakeMips
//
//==========================================================================

static void	MakeMips(miptexture_t *mip)
{
	byte		*psrc, *pdst;
	int			i, j, miplevel, mipw, miph, srcrow;

	mip_pal = (rgb_t*)W_CacheLumpName("playpal", PU_CACHE);

	mip->offsets[0] = sizeof(miptexture_t);
	mip->offsets[1] = mip->offsets[0] + mip->width * mip->height;
	mip->offsets[2] = mip->offsets[1] + mip->width * mip->height / 4;
	mip->offsets[3] = mip->offsets[2] + mip->width * mip->height / 16;

	for (miplevel = 1; miplevel < 4; miplevel++)
	{
		mipw = mip->width >> miplevel;
 		miph = mip->height >> miplevel;
		srcrow = mip->width >> (miplevel - 1);
		psrc = (byte*)mip + mip->offsets[miplevel - 1];
		pdst = (byte*)mip + mip->offsets[miplevel];
		for (i = 0; i < miph; i++)
		{
			for (j = 0; j < mipw; j++)
			{
				mip_a = 0;
				mip_r = 0;
 				mip_g = 0;
 				mip_b = 0;
				MipPixel(psrc[0]);
				MipPixel(psrc[1]);
				MipPixel(psrc[srcrow]);
				MipPixel(psrc[srcrow + 1]);
				*pdst = MipColor();
				psrc += 2;
				pdst++;
			}
			psrc += srcrow;
		}
	}
}

//==========================================================================
//
//	DrawColumnInCache
//
// 	Clip and draw a column from a patch into a flat buffer.
//
//		column - column to draw
//		cache - buffer
//		originx, originy - position of column in the buffer
//		cachewidth, cacheheight - size of the cache
//
//==========================================================================

static void DrawColumnInCache(column_t* column, byte* cache,
	int originx, int originy, int cachewidth, int cacheheight, bool dsky)
{
    int		count;
    int		position;
    byte*	source;
    byte*	dest;
	
	// step through the posts in a column
    while (column->topdelta != 0xff)
    {
		source = (byte *)column + 3;
		count = column->length;
		position = originy + column->topdelta;

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
			*dest = *source || dsky ? *source : black_color;
			source++;
			dest += cachewidth;
    	}
		
		column = (column_t *)((byte *)column + column->length + 4);
    }
}

//==========================================================================
//
//	GenerateTexture
//
// 	Using the texture definition, the composite texture is created from the
// patches, and each column is cached.
//
//==========================================================================

static void GenerateTexture(int texnum)
{
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
			    		x, patch->originy, mipw, texture->height, false);
				}
			}
		}
    }

	MakeMips(mip);

    // Now that the texture has been built in column cache,
    //  it is purgable from zone memory.
    Z_ChangeTag(mip, PU_CACHE);
}

//==========================================================================
//
// 	TSoftwareDrawer::SetTexture
//
//==========================================================================

void TSoftwareDrawer::SetTexture(int tex)
{
	if ((dword)tex >= (dword)numtextures)
		Sys_Error("Invalid texture num %d\n", tex);

    if (!texturedata[tex])
		GenerateTexture(tex);

    miptexture = texturedata[tex];
    cacheblock = (byte*)miptexture + miptexture->offsets[0];
}

//==========================================================================
//
//	GenerateSkyTexture
//
//==========================================================================

static void GenerateSkyTexture(int texnum, bool double_sky)
{
    byte*			block;
    texdef_t*		texture;
    texpatch_t*		patch;	
    patch_t*		realpatch;
    int				x;
    int				x1;
    int				x2;
    int				i;
    column_t*		patchcol;
	int				wtimes;
	int				htimes;

    texture = textures[texnum];

    block = (byte*)Z_Calloc(1024 * 256, PU_STATIC, (void**)&skytexturedata[texnum]);

    // Composite the columns together.
    patch = texture->patches;

	wtimes = 1024 / texture->width;
	htimes = 256 / texture->height;

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
			for (int ht = 0; ht < htimes; ht++)
			{
				for (int wt = 0; wt < wtimes; wt++)
				{
			    	DrawColumnInCache(patchcol, block + wt * texture->width +
			    		ht * texture->height * 1024, x, patch->originy,
			    		1024, texture->height, double_sky);
				}
			}
		}
    }

    // Now that the texture has been built in column cache,
    //  it is purgable from zone memory.
    Z_ChangeTag(block, PU_CACHE);
}

//==========================================================================
//
// 	TSoftwareDrawer::SetSkyTexture
//
//==========================================================================

void TSoftwareDrawer::SetSkyTexture(int tex, bool double_sky)
{
	tex = texturetranslation[tex];

    if (!skytexturedata[tex])
		GenerateSkyTexture(tex, double_sky);

    cacheblock = skytexturedata[tex];
}

//==========================================================================
//
//	GenerateFlat
//
//==========================================================================

static void GenerateFlat(int num)
{
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
			block[(j << 6) + i] = pix ? pix : black_color;
		}
	}

	MakeMips(mip);
	Z_ChangeTag(mip, PU_CACHE);
}

//==========================================================================
//
//	TSoftwareDrawer::SetFlat
//
//==========================================================================

void TSoftwareDrawer::SetFlat(int num)
{
	num &= ~TEXF_FLAT;

	if (!flatdata[num])
	{
		GenerateFlat(num);
	}

    miptexture = flatdata[num];
    cacheblock = (byte*)miptexture + miptexture->offsets[0];
}

//==========================================================================
//
//	GenerateSprite
//
//==========================================================================

static void	GenerateSprite(int lump, int slot, dword light, int translation)
{
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
	    while (column->topdelta != 0xff)
	    {
		    byte* source = (byte *)column + 3;
			int count = column->length;

			if (ScreenBPP == 8 && colored)
			{
			    byte* dest = ((byte*)block) + x + column->topdelta * w;
		    	while (count--)
	    		{
					int itmp = trtab[*source ? *source : black_color];
					*dest = d_rgbtable[(((word*)cmapr)[itmp]) |
						(((word*)cmapg)[itmp]) | (((word*)cmapb)[itmp])];
					source++;
					dest += w;
		    	}
			}
			else if (ScreenBPP == 8)
			{
			    byte* dest = ((byte*)block) + x + column->topdelta * w;
		    	while (count--)
	    		{
					*dest = ((byte*)cmap)[trtab[*source ? *source : black_color]];
					source++;
					dest += w;
		    	}
			}
			else if (PixelBytes == 2 && colored)
			{
			    word* dest = ((word*)block) + x + column->topdelta * w;
		    	while (count--)
	    		{
					int itmp = trtab[*source ? *source : black_color];
					*dest = (((word*)cmapr)[itmp]) |
						(((word*)cmapg)[itmp]) | (((word*)cmapb)[itmp]);
					if (!*dest) *dest = 1;
					source++;
					dest += w;
		    	}
			}
			else if (PixelBytes == 2)
			{
			    word* dest = ((word*)block) + x + column->topdelta * w;
		    	while (count--)
	    		{
					*dest = ((word*)cmap)[trtab[*source ? *source : black_color]];
					source++;
					dest += w;
		    	}
			}
			else if (colored)
			{
			    dword* dest = ((dword*)block) + x + column->topdelta * w;
		    	while (count--)
	    		{
					int itmp = trtab[*source ? *source : black_color];
					*dest = MakeCol32(((byte*)cmapr)[itmp],
						((byte*)cmapg)[itmp], ((byte*)cmapb)[itmp]);
					if (!*dest) *dest = 1;
					source++;
					dest += w;
		    	}
			}
			else
			{
			    dword* dest = ((dword*)block) + x + column->topdelta * w;
		    	while (count--)
	    		{
					*dest = ((dword*)cmap)[trtab[*source ? *source : black_color]];
					source++;
					dest += w;
		    	}
			}
			column = (column_t *)((byte *)column + column->length + 4);
	    }
	}

	Z_ChangeTag(patch, PU_CACHE);
}

//==========================================================================
//
//	SetSpriteLump
//
//==========================================================================

void SetSpriteLump(int lump, dword light, int translation)
{
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
}

//==========================================================================
//
//	D_TextureAnimation
//
//==========================================================================

int D_TextureAnimation(int tex)
{
	if (tex & TEXF_FLAT)
		return TEXF_FLAT | flattranslation[tex & ~TEXF_FLAT];
	else
		return texturetranslation[tex];
}

//**************************************************************************
//
//	$Log$
//	Revision 1.3  2001/07/31 17:16:30  dj_jl
//	Just moved Log to the end of file
//
//	Revision 1.2  2001/07/27 14:27:54  dj_jl
//	Update with Id-s and Log-s, some fixes
//
//**************************************************************************
