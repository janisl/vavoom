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

#include "d3d_local.h"

// MACROS ------------------------------------------------------------------

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
//	TDirect3DDrawer::ToPowerOf2
//
//==========================================================================

int TDirect3DDrawer::ToPowerOf2(int val)
{
	int answer = 1;
	while (answer < val)
		answer <<= 1;
	return answer;
}

//==========================================================================
//
//	TDirect3DDrawer::CreateSurface
//
//==========================================================================

LPDIRECTDRAWSURFACE7 TDirect3DDrawer::CreateSurface(int w, int h, int bpp)
{
	DDSURFACEDESC2			ddsd;
	LPDIRECTDRAWSURFACE7	surf = NULL;
	int i;

	memset(&ddsd, 0, sizeof(ddsd));
	ddsd.dwSize = sizeof(ddsd);
	ddsd.dwFlags = DDSD_WIDTH | DDSD_HEIGHT | DDSD_CAPS | DDSD_PIXELFORMAT;
	ddsd.ddsCaps.dwCaps = DDSCAPS_TEXTURE | SurfaceMemFlag;
	ddsd.dwWidth  = w;
	ddsd.dwHeight = h;
	if (bpp == 32)
		memcpy(&ddsd.ddpfPixelFormat, &PixelFormat32, sizeof(DDPIXELFORMAT));
	else
		memcpy(&ddsd.ddpfPixelFormat, &PixelFormat, sizeof(DDPIXELFORMAT));

	do
	{
		if (DDraw->CreateSurface(&ddsd, &surf, NULL) == DD_OK)
		{
			return surf;
		}

		tscount++;
		for (i = 0; i < numsurfaces; i++)
		{
			int index = (i + tscount) % numsurfaces;
			if (texturesurfaces[index])
			{
				SAFE_RELEASE(texturesurfaces[index]);
				break;
			}
		}
	} while (i < numsurfaces);

	cond << "Not enough video memory\n";
	return NULL;
}

//==========================================================================
//
//	TDirect3DDrawer::LockSurface
//
//==========================================================================

word *TDirect3DDrawer::LockSurface(LPDIRECTDRAWSURFACE7 surf)
{
	DDSURFACEDESC2			ddsd;
	memset(&ddsd, 0, sizeof(ddsd));
	ddsd.dwSize = sizeof(ddsd);
	ddsd.dwFlags = DDSD_WIDTH | DDSD_HEIGHT | DDSD_LPSURFACE;
	if (FAILED(surf->Lock(NULL, &ddsd, DDLOCK_WAIT, NULL)))
	{
		cond << "Failed to lock surface\n";
		return NULL;
	}
	return (word*)ddsd.lpSurface;
}

//==========================================================================
//
//	TDirect3DDrawer::InitTextures
//
//==========================================================================

void TDirect3DDrawer::InitTextures(void)
{
	numsurfaces = numtextures + numflats + numspritelumps +
		MAX_TRANSLATED_SPRITES + MAX_PICS + MAX_SKIN_CACHE;
	texturesurfaces = (LPDIRECTDRAWSURFACE7*)Z_Calloc(numsurfaces * 4);
	//	Textures
	texturedata = texturesurfaces;
	//	Flats
	flatdata = texturedata + numtextures;
	//	Sprite lumps
	spritedata = flatdata + numflats;
	trsprdata = spritedata + numspritelumps;
	//	2D graphics
	picdata = trsprdata + MAX_TRANSLATED_SPRITES;
	//	Skins
	skin_data = picdata + MAX_PICS;

	//	Lightmaps, seperate from other surfaces so CreateSurface doesn't
	// release them
	light_surf = (LPDIRECTDRAWSURFACE7*)Z_Calloc(NUM_BLOCK_SURFS * 4);

	textureiw = (float*)Z_Calloc(numtextures * 4);
	textureih = (float*)Z_Calloc(numtextures * 4);
	spriteiw = (float*)Z_Calloc(numspritelumps * 4);
	spriteih = (float*)Z_Calloc(numspritelumps * 4);
}

//==========================================================================
//
//	TDirect3DDrawer::ReleaseTextures
//
//==========================================================================

void TDirect3DDrawer::ReleaseTextures(void)
{
	int i;
	for (i = 0; i < numsurfaces; i++)
	{
		SAFE_RELEASE(texturesurfaces[i]);
	}
	for (i = 0; i < NUM_BLOCK_SURFS; i++)
	{
		SAFE_RELEASE(light_surf[i]);
	}
	SAFE_RELEASE(particle_texture);
#ifdef BUMP_TEST
	SAFE_RELEASE(bumpTexture);
#endif
}

//==========================================================================
//
//	TDirect3DDrawer::DrawColumnInCache
//
// 	Clip and draw a column from a patch into a flat buffer.
//
//		column - column to draw
//		cache - buffer
//		originx, originy - position of column in the buffer
//		cachewidth, cacheheight - size of the cache
//
//==========================================================================

void TDirect3DDrawer::DrawColumnInCache(column_t* column, word* cache,
	int originx, int originy, int cachewidth, int cacheheight)
{
    int		count;
    int		position;
	byte*	source;
	word*	dest;
	
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
			*dest = pal8_to16[*source];
			source++;
			dest += cachewidth;
    	}
		
		column = (column_t *)((byte *)column + column->length + 4);
    }
}

//==========================================================================
//
//	TDirect3DDrawer::GenerateTexture
//
// 	Using the texture definition, the composite texture is created from the
// patches, and each column is cached.
//
//==========================================================================

void TDirect3DDrawer::GenerateTexture(int texnum)
{
    word*			block;
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
	int				texture_width;
	int				workw;
	int				workh;

    texture = textures[texnum];

	texture_width = texture->width;
	if (texture_width > 256)
	{
		cond << "JL in trouble - texture " << texture->name
			<< " haves width " << texture_width << " greater than 256\n";
		texture_width = 256;
	}
	if (texture_width & (texture_width - 1))
	{
		//	This will happen only with Doom's dummy texture
		con << "Texture width is not a power of 2\n";
		texture_width = 16;
	}
	workw = texture_width;
	if (texture->height & (texture->height - 1))
	{
		//	Texture height is not a power of 2, use 256
		workh = 256;
	}
	else
	{
		workh = texture->height;
	}
	if (square_textures)
	{
		workw = workh = MAX(workw, workh);
	}

	textureiw[texnum] = 1.0 / (float)workw;
	textureih[texnum] = 1.0 / (float)workh;
	texturedata[texnum] = CreateSurface(workw, workh, 16);
	if (!texturedata[texnum])
		return;
	block = LockSurface(texturedata[texnum]);
	memset(block, 0, workw * workh * 2);

    // Composite the columns together.
    patch = texture->patches;

	wtimes = workw / texture_width;
	htimes = workh / texture->height;

    for (i = 0; i < texture->patchcount; i++, patch++)
    {
		realpatch = (patch_t*)W_CacheLumpNum(patch->patch, PU_CACHE);
		x1 = patch->originx;
		x2 = x1 + LittleShort(realpatch->width);

		if (x1 < 0)
	    	x = 0;
		else
	    	x = x1;
	
		if (x2 > texture_width)
	    	x2 = texture_width;

		for ( ; x < x2; x++)
		{
	    	patchcol = (column_t *)((byte *)realpatch
				    + LittleLong(realpatch->columnofs[x - x1]));
			for (int ht = 0; ht < htimes; ht++)
			{
				for (int wt = 0; wt < wtimes; wt++)
				{
			    	DrawColumnInCache(patchcol, block + wt * texture_width +
			    		ht * texture->height * workw, x, patch->originy,
			    		workw, texture->height);
				}
			}
		}
    }

	texturedata[texnum]->Unlock(NULL);
}

//==========================================================================
//
// 	TDirect3DDrawer::SetTexture
//
//==========================================================================

void TDirect3DDrawer::SetTexture(int tex)
{
	if (tex & TEXF_FLAT)
	{
		SetFlat(tex);
		return;
	}

	if (!RenderDevice)
	{
		return;
	}

	tex = R_TextureAnimation(tex);

    if (!texturedata[tex])
		GenerateTexture(tex);

    RenderDevice->SetTexture(0, texturedata[tex]);
	tex_iw = textureiw[tex];
	tex_ih = textureih[tex];
}

//==========================================================================
//
// 	TDirect3DDrawer::SetSkyTexture
//
//==========================================================================

void TDirect3DDrawer::SetSkyTexture(int tex, bool double_sky)
{
	if (!RenderDevice)
		return;

	word saved;
	if (double_sky)
	{
		saved = pal8_to16[0];
		pal8_to16[0] = 0;
	}
	SetTexture(tex);
	if (double_sky)
	{
		pal8_to16[0] = saved;
	}
}

//==========================================================================
//
//	TDirect3DDrawer::GenerateFlat
//
//==========================================================================

void TDirect3DDrawer::GenerateFlat(int num)
{
	flatdata[num] = CreateSurface(64, 64, 16);
	if (!flatdata[num])
		return;
	word *block = LockSurface(flatdata[num]);
	byte *data = (byte*)W_CacheLumpNum(flatlumps[num], PU_CACHE);

	byte *src = data;
	word *dst = block;
	for (int i = 0; i < 64 * 64; i++)
	{
		*dst++ = pal8_to16[*src++];
	}

	flatdata[num]->Unlock(NULL);
}

//==========================================================================
//
//	TDirect3DDrawer::SetFlat
//
//==========================================================================

void TDirect3DDrawer::SetFlat(int num)
{
	if (!RenderDevice)
		return;

	num = R_TextureAnimation(num);
	num &= ~TEXF_FLAT;

	if (!flatdata[num])
	{
		GenerateFlat(num);
	}
    RenderDevice->SetTexture(0, flatdata[num]);
	tex_iw = 1.0 / 64.0;
	tex_ih = 1.0 / 64.0;
}

//==========================================================================
//
//	TDirect3DDrawer::GenerateSprite
//
//==========================================================================

void TDirect3DDrawer::GenerateSprite(int lump)
{
    patch_t	*patch = (patch_t*)W_CacheLumpNum(spritelumps[lump], PU_CACHE);

	int w = LittleShort(patch->width);
	int h = LittleShort(patch->height);
	int p2w = ToPowerOf2(w);
	int p2h = ToPowerOf2(h);
	fixed_t xscale;
	if (p2w > maxTexSize)
	{
		xscale = FRACUNIT * w / maxTexSize;
		p2w = maxTexSize;
		if (square_textures)
		{
			p2h = p2w;
		}
		spriteiw[lump] = 1.0 / (float)w;
	}
	else
	{
		xscale = FRACUNIT;
		if (square_textures)
		{
			p2w = p2h = MAX(p2w, p2h);
		}
		spriteiw[lump] = 1.0 / (float)p2w;
	}
	spriteih[lump] = 1.0 / (float)p2h;
	int workw = MIN(w, p2w);

	spritedata[lump] = CreateSurface(p2w, p2h, 16);
	if (!spritedata[lump])
		return;
	word *block = LockSurface(spritedata[lump]);
	memset(block, 0, p2w * p2h * 2);

	for (int x = 0; x < workw; x++)
	{
    	column_t *column = (column_t *)((byte *)patch +
    		LittleLong(patch->columnofs[(x * xscale) >> FRACBITS]));

		// step through the posts in a column
	    while (column->topdelta != 0xff)
	    {
		    byte* source = (byte *)column + 3;
		    word* dest = block + x + column->topdelta * p2w;
			int count = column->length;

	    	while (count--)
	    	{
				*dest = pal8_to16[*source];
				source++;
				dest += p2w;
	    	}
			column = (column_t *)((byte *)column + column->length + 4);
	    }
	}

	spritedata[lump]->Unlock(NULL);
}

//==========================================================================
//
//	TDirect3DDrawer::GenerateTranslatedSprite
//
//==========================================================================

void TDirect3DDrawer::GenerateTranslatedSprite(int lump, int slot, int translation)
{
    patch_t	*patch = (patch_t*)W_CacheLumpNum(spritelumps[lump], PU_CACHE);

	int w = LittleShort(patch->width);
	int h = LittleShort(patch->height);
	int p2w = ToPowerOf2(w);
	int p2h = ToPowerOf2(h);
	fixed_t xscale;
	if (p2w > maxTexSize)
	{
		xscale = FRACUNIT * w / maxTexSize;
		p2w = maxTexSize;
		if (square_textures)
		{
			p2h = p2w;
		}
		trspriw[slot] = 1.0 / (float)w;
	}
	else
	{
		xscale = FRACUNIT;
		if (square_textures)
		{
			p2w = p2h = MAX(p2w, p2h);
		}
		trspriw[slot] = 1.0 / (float)p2w;
	}
	trsprih[slot] = 1.0 / (float)p2h;
	int workw = MIN(w, p2w);

	trsprdata[slot] = CreateSurface(p2w, p2h, 16);
	if (!trsprdata[slot])
		return;
	word *block = LockSurface(trsprdata[slot]);
	memset(block, 0, p2w * p2h * 2);
	trsprlump[slot] = lump;
	trsprtnum[slot] = translation;

	byte *trtab = translationtables + translation * 256;

	for (int x = 0; x < workw; x++)
	{
    	column_t *column = (column_t *)((byte *)patch +
    		LittleLong(patch->columnofs[(x * xscale) >> FRACBITS]));

		// step through the posts in a column
	    while (column->topdelta != 0xff)
	    {
		    byte* source = (byte *)column + 3;
		    word* dest = block + x + column->topdelta * p2w;
			int count = column->length;

	    	while (count--)
	    	{
				*dest = pal8_to16[trtab[*source]];
				source++;
				dest += p2w;
	    	}
			column = (column_t *)((byte *)column + column->length + 4);
	    }
	}

	trsprdata[slot]->Unlock(NULL);
}

//==========================================================================
//
//	TDirect3DDrawer::SetSpriteLump
//
//==========================================================================

void TDirect3DDrawer::SetSpriteLump(int lump, int translation)
{
	if (!RenderDevice)
		return;

	if (translation)
	{
		int i;
		int avail = -1;
		for (i = 0; i <	MAX_TRANSLATED_SPRITES; i++)
		{
			if (trsprdata[i])
			{
				if (trsprlump[i] == lump && trsprtnum[i] == translation)
				{
					RenderDevice->SetTexture(0, trsprdata[i]);
					tex_iw = trspriw[i];
					tex_ih = trsprih[i];
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
			Z_Free(trsprdata[0]);
			avail = 0;
		}
		GenerateTranslatedSprite(lump, avail, translation);
		RenderDevice->SetTexture(0, trsprdata[avail]);
		tex_iw = trspriw[avail];
		tex_ih = trsprih[avail];
	}
	else
	{
		if (!spritedata[lump])
		{
			GenerateSprite(lump);
		}
		RenderDevice->SetTexture(0, spritedata[lump]);
		tex_iw = spriteiw[lump];
		tex_ih = spriteih[lump];
	}
}

//==========================================================================
//
//	TDirect3DDrawer::SetPic
//
//==========================================================================

void TDirect3DDrawer::SetPic(int handle)
{
	if (!RenderDevice)
		return;

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

	RenderDevice->SetTexture(0, picdata[handle]);
	tex_iw = piciw[handle];
	tex_ih = picih[handle];
}

//==========================================================================
//
//	TDirect3DDrawer::GeneratePicFromPatch
//
//==========================================================================

void TDirect3DDrawer::GeneratePicFromPatch(int handle)
{
	patch_t *patch = (patch_t*)W_CacheLumpName(pic_list[handle].name, PU_CACHE);
	int w = LittleShort(patch->width);
	int h = LittleShort(patch->height);
	int p2w = ToPowerOf2(w);
	int p2h = ToPowerOf2(h);
	fixed_t xscale;
	if (p2w > maxTexSize)
	{
		xscale = FRACUNIT * w / maxTexSize;
		p2w = maxTexSize;
		if (square_textures)
		{
			p2h = p2w;
		}
		piciw[handle] = 1.0 / (float)w;
	}
	else
	{
		xscale = FRACUNIT;
		if (square_textures)
		{
			p2w = p2h = MAX(p2w, p2h);
		}
		piciw[handle] = 1.0 / (float)p2w;
	}
	picih[handle] = 1.0 / (float)p2h;
	int workw = MIN(w, p2w);

	picdata[handle] = CreateSurface(p2w, p2h, 16);
	if (!picdata[handle])
		return;
    word *block = LockSurface(picdata[handle]);
	memset(block, 0, p2w * p2h * 2);

	for (int x = 0; x < workw; x++)
	{
    	column_t *column = (column_t *)((byte *)patch +
    		LittleLong(patch->columnofs[(x * xscale) >> FRACBITS]));

		// step through the posts in a column
	    while (column->topdelta != 0xff)
	    {
		    byte* source = (byte *)column + 3;
		    word* dest = block + x + column->topdelta * p2w;
			int count = column->length;

	    	while (count--)
	    	{
				*dest = pal8_to16[*source];
				source++;
				dest += p2w;
	    	}
			column = (column_t *)((byte *)column + column->length + 4);
	    }
	}

	picdata[handle]->Unlock(NULL);
}

//==========================================================================
//
//	TDirect3DDrawer::GeneratePicFromRaw
//
//==========================================================================

void TDirect3DDrawer::GeneratePicFromRaw(int handle)
{
	int p2w = 512;
	int p2h = 256;
	fixed_t xscale;
	if (p2w > maxTexSize)
	{
		p2w = maxTexSize;
		xscale = FRACUNIT * 320 / maxTexSize;
		piciw[handle] = 1.0 / 320.0;
	}
	else
	{
		xscale = FRACUNIT;
		piciw[handle] = 1.0 / 512.0;
	}
	if (square_textures)
	{
		p2h = p2w;
	}
	picih[handle] = 1.0 / (float)p2h;
	int workw = MIN(320, p2w);

	int lump = W_GetNumForName(pic_list[handle].name);
	byte* raw = (byte*)W_CacheLumpNum(lump, PU_CACHE);
	int realh = W_LumpLength(lump) / 320;
	picdata[handle] = CreateSurface(p2w, p2h, 16);
	if (!picdata[handle])
		return;
    word *block = LockSurface(picdata[handle]);

	for (int y = 0; y < realh; y++)
	{
		byte *src = raw + 320 * y;
		word *dst = block + p2w * y;
		for (int x = 0; x < workw; x++)
		{
			dst[x] = pal8_to16[src[(x * xscale) >> FRACBITS]];
		}
		if (workw == 320)
		{
			//	For automap warping
			dst[320] = pal8_to16[src[0]];
			dst[511] = pal8_to16[src[319]];
		}
	}
	if (realh != 200)
	{
		//	Automap background, copy top and bottom so wrapping in
		// bilinear filtering looks good
		memcpy(block + p2w * realh, block, p2w * 2);
		memcpy(block + p2w * (p2h - 1), block + p2w * (realh - 1), p2w * 2);
	}

	picdata[handle]->Unlock(NULL);
}

//==========================================================================
//
//	TDirect3DDrawer::SetSkin
//
//==========================================================================

void TDirect3DDrawer::SetSkin(const char *name)
{
	int			i;
	int			avail;

	avail = -1;
	for (i = 0; i < MAX_SKIN_CACHE; i++)
	{
		if (skin_data[i])
		{
			if (!strcmp(skin_name[i], name))
			{
				avail = i;
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
			SAFE_RELEASE(skin_data[0]);
		}
		strcpy(skin_name[avail], name);
		Mod_LoadSkin(name, 0);

		int w = SkinWidth > maxTexSize ? maxTexSize : ToPowerOf2(SkinWidth);
		int h = SkinHeight > maxTexSize ? maxTexSize : ToPowerOf2(SkinHeight);
		if (square_textures)
		{
			w = h = MAX(w, h);
		}

		int sscale = FRACUNIT * SkinWidth / w;
		int tscale = FRACUNIT * SkinHeight / h;

		skin_data[avail] = CreateSurface(w, h, 16);
		word *buf = LockSurface(skin_data[avail]);
		word *dst = buf;
		for (int t = 0; t < h; t++)
		{
			byte *src = SkinData + ((t * tscale) >> FRACBITS) * SkinWidth;
			for (int s = 0; s < w; s++)
			{
				*dst++ = pal8_to16[src[(s * sscale) >> FRACBITS]];
			}
		}
		skin_data[avail]->Unlock(NULL);
		Z_Free(SkinData);
	}

	RenderDevice->SetTexture(0, skin_data[avail]);
}

//**************************************************************************
//
//	$Log$
//	Revision 1.5  2001/08/21 17:46:08  dj_jl
//	Added R_TextureAnimation, made SetTexture recognize flats
//
//	Revision 1.4  2001/08/02 17:47:44  dj_jl
//	Support skins with non-power of 2 dimensions
//	
//	Revision 1.3  2001/07/31 17:16:30  dj_jl
//	Just moved Log to the end of file
//	
//	Revision 1.2  2001/07/27 14:27:54  dj_jl
//	Update with Id-s and Log-s, some fixes
//
//**************************************************************************
