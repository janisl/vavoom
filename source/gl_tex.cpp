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
//**	Revision 1.2  2001/07/27 14:27:54  dj_jl
//**	Update with Id-s and Log-s, some fixes
//**
//**************************************************************************

// HEADER FILES ------------------------------------------------------------

#include "gl_local.h"

// MACROS ------------------------------------------------------------------

// TYPES -------------------------------------------------------------------

// EXTERNAL FUNCTION PROTOTYPES --------------------------------------------

// PUBLIC FUNCTION PROTOTYPES ----------------------------------------------

// PRIVATE FUNCTION PROTOTYPES ---------------------------------------------

// EXTERNAL DATA DECLARATIONS ----------------------------------------------

// PUBLIC DATA DEFINITIONS -------------------------------------------------

// PRIVATE DATA DEFINITIONS ------------------------------------------------

static byte ptex[8][8] =
{
	{ 0, 0, 0, 0, 0, 0, 0, 0 },
	{ 0, 0, 0, 0, 0, 0, 0, 0 },
	{ 0, 0, 0, 1, 1, 0, 0, 0 },
	{ 0, 0, 1, 1, 1, 1, 0, 0 },
	{ 0, 0, 1, 1, 1, 1, 0, 0 },
	{ 0, 0, 0, 1, 1, 0, 0, 0 },
	{ 0, 0, 0, 0, 0, 0, 0, 0 },
	{ 0, 0, 0, 0, 0, 0, 0, 0 },
};

// CODE --------------------------------------------------------------------

//==========================================================================
//
//	TOpenGLDrawer::InitData
//
//==========================================================================

void TOpenGLDrawer::InitData(void)
{
	byte *pal = (byte*)W_CacheLumpName("PLAYPAL", PU_CACHE);
	for (int i = 0; i < 256; i++)
	{
		pal8_to24[i].r = *pal++;
		pal8_to24[i].g = *pal++;
		pal8_to24[i].b = *pal++;
		pal8_to24[i].a = 255;
	}
}

//==========================================================================
//
//	TOpenGLDrawer::InitTextures
//
//==========================================================================

void TOpenGLDrawer::InitTextures(void)
{
	//	Textures
	texture_id = (GLuint*)Z_Calloc(numtextures * 4);
	texture_sent = (bool*)Z_Calloc(numtextures);
	texture_iw = (float*)Z_Calloc(numtextures * 4);
	texture_ih = (float*)Z_Calloc(numtextures * 4);
	// 	Flats
	flat_id = (GLuint*)Z_Calloc(numflats * 4);
	flat_sent = (bool*)Z_Calloc(numflats);
	// 	Sprite lumps
    sprite_id = (GLuint*)Z_Calloc(numspritelumps * 4);
    sprite_sent = (bool*)Z_Calloc(numspritelumps);
    spriteiw = (float*)Z_Calloc(numspritelumps * 4);
    spriteih = (float*)Z_Calloc(numspritelumps * 4);
}

//==========================================================================
//
//	TOpenGLDrawer::GenerateTextures
//
//==========================================================================

void TOpenGLDrawer::GenerateTextures(void)
{
	int			i, j;
	rgba_t		pbuf[8][8];

	glGenTextures(numtextures, texture_id);
	glGenTextures(numflats, flat_id);
	glGenTextures(numspritelumps, sprite_id);
	glGenTextures(MAX_TRANSLATED_SPRITES, trspr_id);
	glGenTextures(MAX_PICS, pic_id);
	glGenTextures(NUM_BLOCK_SURFS, lmap_id);
	glGenTextures(MAX_SKIN_CACHE, skin_id);
	glGenTextures(1, &particle_texture);
	memset(texture_sent, 0, numtextures);
	memset(flat_sent, 0, numflats);
	memset(sprite_sent, 0, numspritelumps);
	memset(trspr_id, 0, MAX_TRANSLATED_SPRITES);
	memset(pic_sent, 0, MAX_PICS);
	memset(skin_name, 0, sizeof(skin_name));

	for (j = 0; j < 8; j++)
	{
		for (i = 0; i < 8; i++)
		{
			pbuf[j][i].r = 255;
			pbuf[j][i].g = 255;
			pbuf[j][i].b = 255;
			pbuf[j][i].a = ptex[j][i] * 255;
		}
	}
	glBindTexture(GL_TEXTURE_2D, particle_texture);
	glTexImage2D(GL_TEXTURE_2D, 0, 4, 8, 8, 0, GL_RGBA, GL_UNSIGNED_BYTE, pbuf);

	texturesGenerated = true;
}

//==========================================================================
//
//	TOpenGLDrawer::DeleteTextures
//
//==========================================================================

void TOpenGLDrawer::DeleteTextures(void)
{
	if (texturesGenerated)
	{
		glDeleteTextures(numtextures, texture_id);
		glDeleteTextures(numflats, flat_id);
		glDeleteTextures(numspritelumps, sprite_id);
		glDeleteTextures(MAX_TRANSLATED_SPRITES, trspr_id);
		glDeleteTextures(MAX_PICS, pic_id);
		glDeleteTextures(NUM_BLOCK_SURFS, lmap_id);
		glDeleteTextures(MAX_SKIN_CACHE, skin_id);
		glDeleteTextures(1, &particle_texture);
		texturesGenerated = false;
	}
}

//==========================================================================
//
//	TOpenGLDrawer::ToPowerOf2
//
//==========================================================================

int TOpenGLDrawer::ToPowerOf2(int val)
{
	int answer = 1;
	while (answer < val)
		answer <<= 1;
	return answer;
}

//==========================================================================
//
//	TOpenGLDrawer::DrawColumnInCache
//
// 	Clip and draw a column from a patch into a flat buffer.
//
//		column - column to draw
//		cache - buffer
//		originx, originy - position of column in the buffer
//		cachewidth, cacheheight - size of the cache
//
//==========================================================================

void TOpenGLDrawer::DrawColumnInCache(column_t* column, rgba_t* cache,
	int originx, int originy, int cachewidth, int cacheheight)
{
	int		count;
	int		position;
	byte*	source;
	rgba_t*	dest;

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
			*dest = pal8_to24[*source];
			source++;
			dest += cachewidth;
		}

		column = (column_t *)((byte *)column + column->length + 4);
	}
}

//==========================================================================
//
//	TOpenGLDrawer::GenerateTexture
//
// 	Using the texture definition, the composite texture is created from the
// patches, and each column is cached.
//
//==========================================================================

void TOpenGLDrawer::GenerateTexture(int texnum)
{
	rgba_t*			block;
	texdef_t*		texture;
	texpatch_t*		patch;
	patch_t*		realpatch;
	int				x;
	int				x1;
	int				x2;
	int				i;
	column_t*		patchcol;

	texture = textures[texnum];

	block = (rgba_t*)Z_Calloc(4 * texture->width * texture->height);

	// Composite the columns together.
	patch = texture->patches;

	for (i = 0; i < texture->patchcount; i++, patch++)
	{
		realpatch = (patch_t*)W_CacheLumpNum(patch->patch, PU_CACHE);
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
			DrawColumnInCache(patchcol, block, x, patch->originy,
				texture->width, texture->height);
		}
	}

	gluBuild2DMipmaps(GL_TEXTURE_2D, 4, texture->width, texture->height, GL_RGBA, GL_UNSIGNED_BYTE, block);

	Z_Free(block);
	texture_sent[texnum] = true;
	texture_iw[texnum] = 1.0 / (float)texture->width;
	texture_ih[texnum] = 1.0 / (float)texture->height;
}

//==========================================================================
//
// 	TOpenGLDrawer::SetTexture
//
//==========================================================================

void TOpenGLDrawer::SetTexture(int tex)
{
	tex = texturetranslation[tex];

	glBindTexture(GL_TEXTURE_2D, texture_id[tex]);
	if (!texture_sent[tex])
	{
		GenerateTexture(tex);
	}
	if (tex_linear == 3)
	{
		glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);
		glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
	}
	else if (tex_linear == 2)
	{
		glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_NEAREST);
		glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
	}
	else if (tex_linear)
	{
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
	}
	else
	{
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
	}
	tex_iw = texture_iw[tex];
	tex_ih = texture_ih[tex];
}

//==========================================================================
//
// 	TOpenGLDrawer::SetSkyTexture
//
//==========================================================================

void TOpenGLDrawer::SetSkyTexture(int tex, bool double_sky)
{
	rgba_t saved;
	if (double_sky)
	{
		saved = pal8_to24[0];
		pal8_to24[0].a = 0;
	}
	SetTexture(tex);
	if (double_sky)
	{
		pal8_to24[0] = saved;
	}
	// No mipmaping for sky
	if (tex_linear)
	{
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
	}
	else
	{
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
	}
}

//==========================================================================
//
//	TOpenGLDrawer::GenerateFlat
//
//==========================================================================

void TOpenGLDrawer::GenerateFlat(int num)
{
	rgba_t *block = (rgba_t*)Z_Malloc(4 * 64 * 64);
	byte *data = (byte*)W_CacheLumpNum(flatlumps[num], PU_CACHE);
	for (int i = 0; i < 64 * 64; i++)
	{
		block[i] = pal8_to24[data[i]];
	}
	gluBuild2DMipmaps(GL_TEXTURE_2D, 4, 64, 64, GL_RGBA, GL_UNSIGNED_BYTE, block);
	Z_Free(block);
	flat_sent[num] = true;
}

//==========================================================================
//
//	TOpenGLDrawer::SetFlat
//
//==========================================================================

void TOpenGLDrawer::SetFlat(int num)
{
	num &= ~TEXF_FLAT;
	num = flattranslation[num];

	glBindTexture(GL_TEXTURE_2D, flat_id[num]);

	if (!flat_sent[num])
	{
		GenerateFlat(num);
	}
	if (tex_linear == 3)
	{
		glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);
		glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
	}
	else if (tex_linear == 2)
	{
		glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_NEAREST);
		glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
	}
	else if (tex_linear)
	{
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
	}
	else
	{
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
	}
	tex_iw = 1.0 / 64.0;
	tex_ih = 1.0 / 64.0;
}

//==========================================================================
//
//	TOpenGLDrawer::GenerateSprite
//
//==========================================================================

void TOpenGLDrawer::GenerateSprite(int lump)
{
    patch_t	*patch = (patch_t*)W_CacheLumpNum(spritelumps[lump], PU_STATIC);

	int w = LittleShort(patch->width);
	int h = LittleShort(patch->height);
	int p2w = ToPowerOf2(w);
	int p2h = ToPowerOf2(h);
	spriteiw[lump] = 1.0 / (float)p2w;
	spriteih[lump] = 1.0 / (float)p2h;

    rgba_t *block = (rgba_t*)Z_Calloc(4 * p2w * p2h);

	for (int x = 0; x < w; x++)
	{
    	column_t *column = (column_t *)((byte *)patch +
    		LittleLong(patch->columnofs[x]));

		// step through the posts in a column
	    while (column->topdelta != 0xff)
	    {
		    byte* source = (byte *)column + 3;
		    rgba_t* dest = block + x + column->topdelta * p2w;
			int count = column->length;

	    	while (count--)
	    	{
				*dest = pal8_to24[*source];
				source++;
				dest += p2w;
	    	}
			column = (column_t *)((byte *)column + column->length + 4);
	    }
	}

	// Generate The Texture
	gluBuild2DMipmaps(GL_TEXTURE_2D, 4, p2w, p2h, GL_RGBA, GL_UNSIGNED_BYTE, block);
	sprite_sent[lump] = true;

	Z_Free(block);
	Z_ChangeTag(patch, PU_CACHE);
}

//==========================================================================
//
//	TOpenGLDrawer::GenerateTranslatedSprite
//
//==========================================================================

void TOpenGLDrawer::GenerateTranslatedSprite(int lump, int slot, int translation)
{
    patch_t	*patch = (patch_t*)W_CacheLumpNum(spritelumps[lump], PU_STATIC);

	int w = LittleShort(patch->width);
	int h = LittleShort(patch->height);
	int p2w = ToPowerOf2(w);
	int p2h = ToPowerOf2(h);
	trspriw[lump] = 1.0 / (float)p2w;
	trsprih[lump] = 1.0 / (float)p2h;

    rgba_t *block = (rgba_t*)Z_Calloc(4 * p2w * p2h);
	trspr_lump[slot] = lump;
	trspr_tnum[slot] = translation;
	trspr_sent[slot] = true;

	byte *trtab = translationtables + translation * 256;

	for (int x = 0; x < w; x++)
	{
    	column_t *column = (column_t *)((byte *)patch +
    		LittleLong(patch->columnofs[x]));

		// step through the posts in a column
	    while (column->topdelta != 0xff)
	    {
		    byte* source = (byte *)column + 3;
		    rgba_t* dest = block + x + column->topdelta * p2w;
			int count = column->length;

	    	while (count--)
	    	{
				*dest = pal8_to24[trtab[*source]];
				source++;
				dest += p2w;
	    	}
			column = (column_t *)((byte *)column + column->length + 4);
	    }
	}

	// Generate The Texture
	gluBuild2DMipmaps(GL_TEXTURE_2D, 4, p2w, p2h, GL_RGBA, GL_UNSIGNED_BYTE, block);

	Z_Free(block);
	Z_ChangeTag(patch, PU_CACHE);
}

//==========================================================================
//
//	TOpenGLDrawer::SetSpriteLump
//
//==========================================================================

void TOpenGLDrawer::SetSpriteLump(int lump, int translation)
{
	if (translation)
	{
		int i;
		int avail = -1;
		for (i = 0; i <	MAX_TRANSLATED_SPRITES; i++)
		{
			if (trspr_sent[i])
			{
				if (trspr_lump[i] == lump && trspr_tnum[i] == translation)
				{
					glBindTexture(GL_TEXTURE_2D, trspr_id[i]);
					if (tex_linear == 3)
					{
						glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);
						glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
					}
					else if (tex_linear == 2)
					{
						glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_NEAREST);
						glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
					}
					else if (tex_linear)
					{
						glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
						glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
					}
					else
					{
						glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
						glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
					}
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
			avail = 0;
		}
		glBindTexture(GL_TEXTURE_2D, trspr_id[avail]);
		GenerateTranslatedSprite(lump, avail, translation);
		if (tex_linear == 3)
		{
			glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);
			glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
		}
		else if (tex_linear == 2)
		{
			glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_NEAREST);
			glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
		}
		else if (tex_linear)
		{
			glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
			glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
		}
		else
		{
			glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
			glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
		}
		tex_iw = trspriw[avail];
		tex_ih = trsprih[avail];
	}
	else
	{
		glBindTexture(GL_TEXTURE_2D, sprite_id[lump]);
		if (!sprite_sent[lump])
		{
			GenerateSprite(lump);
		}
		if (tex_linear == 3)
		{
			glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);
			glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
		}
		else if (tex_linear == 2)
		{
			glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_NEAREST);
			glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
		}
		else if (tex_linear)
		{
			glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
			glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
		}
		else
		{
			glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
			glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
		}
		tex_iw = spriteiw[lump];
		tex_ih = spriteih[lump];
	}
}

//==========================================================================
//
//	TOpenGLDrawer::SetPic
//
//==========================================================================

void TOpenGLDrawer::SetPic(int handle)
{
	glBindTexture(GL_TEXTURE_2D, pic_id[handle]);

	if (!pic_sent[handle])
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

	tex_iw = pic_iw[handle];
	tex_ih = pic_ih[handle];

	if (tex_linear)
	{
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
	}
	else
	{
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
	}
}

//==========================================================================
//
//	TOpenGLDrawer::GeneratePicFromPatch
//
//==========================================================================

void TOpenGLDrawer::GeneratePicFromPatch(int handle)
{
	patch_t *patch = (patch_t*)W_CacheLumpName(pic_list[handle].name, PU_STATIC);
	int w = LittleShort(patch->width);
	int h = LittleShort(patch->height);
	int p2w = ToPowerOf2(w);
	int p2h = ToPowerOf2(h);
	fixed_t xscale;
	if (p2w > maxTexSize)
	{
		xscale = FRACUNIT * w / maxTexSize;
		p2w = maxTexSize;
		pic_iw[handle] = 1.0 / (float)w;
	}
	else
	{
		xscale = FRACUNIT;
		pic_iw[handle] = 1.0 / (float)p2w;
	}
	pic_ih[handle] = 1.0 / (float)p2h;
	int workw = MIN(w, p2w);

    rgba_t *block = (rgba_t*)Z_Calloc(4 * p2w * p2h);

	for (int x = 0; x < workw; x++)
	{
    	column_t *column = (column_t *)((byte *)patch +
    		LittleLong(patch->columnofs[(x * xscale) >> FRACBITS]));

		// step through the posts in a column
	    while (column->topdelta != 0xff)
	    {
		    byte* source = (byte *)column + 3;
		    rgba_t* dest = block + x + column->topdelta * p2w;
			int count = column->length;

	    	while (count--)
	    	{
				*dest = pal8_to24[*source];
				source++;
				dest += p2w;
	    	}
			column = (column_t *)((byte *)column + column->length + 4);
	    }
	}

	glTexImage2D(GL_TEXTURE_2D, 0, 4, p2w, p2h, 0, GL_RGBA, GL_UNSIGNED_BYTE, block);
	pic_sent[handle] = true;
	Z_Free(block);
	Z_ChangeTag(patch, PU_CACHE);
}

//==========================================================================
//
//	TOpenGLDrawer::GeneratePicFromRaw
//
//==========================================================================

void TOpenGLDrawer::GeneratePicFromRaw(int handle)
{
	int p2w = 512;
	int p2h = 256;
	fixed_t xscale;
	if (p2w > maxTexSize)
	{
		p2w = maxTexSize;
		xscale = FRACUNIT * 320 / maxTexSize;
		pic_iw[handle] = 1.0 / 320.0;
	}
	else
	{
		xscale = FRACUNIT;
		pic_iw[handle] = 1.0 / 512.0;
	}
	pic_ih[handle] = 1.0 / 256.0;
	int workw = MIN(320, p2w);

	int lump = W_GetNumForName(pic_list[handle].name);
	byte *raw = (byte*)W_CacheLumpNum(lump, PU_STATIC);
	int realh = W_LumpLength(lump) / 320;
    rgba_t *block = (rgba_t*)Z_Calloc(4 * p2w * p2h);

	for (int y = 0; y < realh; y++)
	{
		byte *src = raw + 320 * y;
		rgba_t *dst = block + p2w * y;
		for (int x = 0; x < workw; x++)
		{
			dst[x] = pal8_to24[src[(x * xscale) >> FRACBITS]];
		}
		if (workw == 320)
		{
			//	For automap warping
			dst[320] = pal8_to24[src[0]];
			dst[511] = pal8_to24[src[319]];
		}
	}
	if (realh != 200)
	{
		//	Automap background, copy top and bottom so wrapping in
		// bilinear filtering looks good
		memcpy(block + p2w * realh, block, p2w * 4);
		memcpy(block + p2w * (p2h - 1), block + p2w * (realh - 1), p2w * 4);
	}

	glTexImage2D(GL_TEXTURE_2D, 0, 4, p2w, p2h, 0, GL_RGBA, GL_UNSIGNED_BYTE, block);
	Z_Free(block);
	Z_ChangeTag(raw, PU_CACHE);

	pic_sent[handle] = true;
}

//==========================================================================
//
//	TOpenGLDrawer::SetSkin
//
//==========================================================================

void TOpenGLDrawer::SetSkin(const char *name)
{
	int			i;
	int			avail;

	avail = -1;
	for (i = 0; i < MAX_SKIN_CACHE; i++)
	{
		if (skin_name[i][0])
		{
			if (!strcmp(skin_name[i], name))
			{
				glBindTexture(GL_TEXTURE_2D, skin_id[i]);
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
		}
		i = avail;
		glBindTexture(GL_TEXTURE_2D, skin_id[i]);
		strcpy(skin_name[i], name);
		Mod_LoadSkin(name, 0);
		rgba_t *buf = (rgba_t*)Z_Malloc(SkinWidth * SkinHeight * 4);
		for (i = 0; i < SkinWidth * SkinHeight; i++)
		{
			buf[i] = pal8_to24[SkinData[i]];
		}
		gluBuild2DMipmaps(GL_TEXTURE_2D, 4, SkinWidth, SkinHeight, GL_RGBA, GL_UNSIGNED_BYTE, buf);
		Z_Free(buf);
		Z_Free(SkinData);
	}
	if (tex_linear == 3)
	{
		glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);
		glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
	}
	else if (tex_linear == 2)
	{
		glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_NEAREST);
		glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
	}
	else if (tex_linear)
	{
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
	}
	else
	{
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
	}
}

