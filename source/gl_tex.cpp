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
	// 	Sky maps
	if (skymaps)
	{
		skymap_id = (GLuint*)Z_Calloc(numskymaps * 4);
		skymap_sent = (bool*)Z_Calloc(numskymaps);
	}
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
	if (numskymaps)
	{
		glGenTextures(numskymaps, skymap_id);
	}
	glGenTextures(numspritelumps, sprite_id);
	glGenTextures(MAX_TRANSLATED_SPRITES, trspr_id);
	glGenTextures(MAX_PICS, pic_id);
	glGenTextures(NUM_BLOCK_SURFS, lmap_id);
	glGenTextures(MAX_SKIN_CACHE, skin_id);
	glGenTextures(1, &particle_texture);
	memset(texture_sent, 0, numtextures);
	memset(flat_sent, 0, numflats);
	memset(skymap_sent, 0, numskymaps);
	memset(sprite_sent, 0, numspritelumps);
	memset(trspr_sent, 0, MAX_TRANSLATED_SPRITES);
	memset(pic_sent, 0, MAX_PICS);
	memset(skin_name, 0, sizeof(skin_name));

	for (j = 0; j < 8; j++)
	{
		for (i = 0; i < 8; i++)
		{
			pbuf[j][i].r = 255;
			pbuf[j][i].g = 255;
			pbuf[j][i].b = 255;
			pbuf[j][i].a = byte(ptex[j][i] * 255);
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
		if (numskymaps)
		{
			glDeleteTextures(numskymaps, skymap_id);
		}
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

	UploadTexture(texture->width, texture->height, block);
	glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
	glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
	Z_Free(block);
	texture_iw[texnum] = 1.0 / float(texture->width);
	texture_ih[texnum] = 1.0 / float(texture->height);
	texture_sent[texnum] = true;
}

//==========================================================================
//
// 	TOpenGLDrawer::SetTexture
//
//==========================================================================

void TOpenGLDrawer::SetTexture(int tex)
{
	if (tex & TEXF_FLAT)
	{
		SetFlat(tex);
		return;
	}

	tex = R_TextureAnimation(tex);

	glBindTexture(GL_TEXTURE_2D, texture_id[tex]);
	if (!texture_sent[tex])
	{
		GenerateTexture(tex);
	}
	glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, maxfilter);
	glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, mipfilter);
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
	if (tex & TEXF_SKY_MAP)
	{
		tex &= ~TEXF_SKY_MAP;
		glBindTexture(GL_TEXTURE_2D, skymap_id[tex]);
		if (!skymap_sent[tex])
		{
			Mod_LoadSkin(skymaps[tex].name, 0);
			if (SkinBPP == 8)
			{
				rgba_t *buf = (rgba_t*)Z_Malloc(SkinWidth * SkinHeight * 4);
				for (int i = 0; i < SkinWidth * SkinHeight; i++)
				{
					buf[i] = SkinPal[SkinData[i]];
				}
				UploadTexture(SkinWidth, SkinHeight, buf);
				Z_Free(buf);
			}
			else
			{
				UploadTexture(SkinWidth, SkinHeight, (rgba_t *)SkinData);
			}
			glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP);
			glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP);
			Z_Free(SkinData);
			skymap_sent[tex] = true;
			skymaps[tex].width = SkinWidth;
			skymaps[tex].height = SkinHeight;
		}
		tex_iw = 1.0 / skymaps[tex].width;
		tex_ih = 1.0 / skymaps[tex].height;
	}
	else
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
	}
	// No mipmaping for sky
	glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, maxfilter);
	glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, minfilter);
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
	UploadTexture(64, 64, block);
	glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
	glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
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
	num = R_TextureAnimation(num);
	num &= ~TEXF_FLAT;

	glBindTexture(GL_TEXTURE_2D, flat_id[num]);

	if (!flat_sent[num])
	{
		GenerateFlat(num);
	}
	glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, maxfilter);
	glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, mipfilter);
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
	spriteiw[lump] = 1.0 / (float)w;
	spriteih[lump] = 1.0 / (float)h;

	rgba_t *block = (rgba_t*)Z_Calloc(4 * w * h);

	for (int x = 0; x < w; x++)
	{
		column_t *column = (column_t *)((byte *)patch +
			LittleLong(patch->columnofs[x]));

		// step through the posts in a column
		while (column->topdelta != 0xff)
		{
			byte* source = (byte *)column + 3;
			rgba_t* dest = block + x + column->topdelta * w;
			int count = column->length;

			while (count--)
			{
				*dest = pal8_to24[*source];
				source++;
				dest += w;
			}
			column = (column_t *)((byte *)column + column->length + 4);
		}
	}

	// Generate The Texture
	UploadTexture(w, h, block);
	sprite_sent[lump] = true;
	glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP);
	glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP);

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
	trspriw[slot] = 1.0 / (float)w;
	trsprih[slot] = 1.0 / (float)h;

	rgba_t *block = (rgba_t*)Z_Calloc(4 * w * h);
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
			rgba_t* dest = block + x + column->topdelta * w;
			int count = column->length;

			while (count--)
			{
				*dest = pal8_to24[trtab[*source]];
				source++;
				dest += w;
			}
			column = (column_t *)((byte *)column + column->length + 4);
		}
	}

	// Generate The Texture
	UploadTexture(w, h, block);
	glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP);
	glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP);

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
					glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, maxfilter);
					glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, mipfilter);
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
		glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, maxfilter);
		glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, mipfilter);
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
		glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, maxfilter);
		glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, mipfilter);
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

	glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, maxfilter);
	glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, minfilter);
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

	rgba_t *block = (rgba_t*)Z_Calloc(4 * w * h);
	rgba_t *pal = r_palette[pic_list[handle].palnum];
	int black = r_black_color[pic_list[handle].palnum];

	for (int x = 0; x < w; x++)
	{
		column_t *column = (column_t *)((byte *)patch +
			LittleLong(patch->columnofs[x]));

		// step through the posts in a column
		while (column->topdelta != 0xff)
		{
			byte* source = (byte *)column + 3;
			rgba_t* dest = block + x + column->topdelta * w;
			int count = column->length;

			while (count--)
			{
				*dest = pal[*source ? *source : black];
				source++;
				dest += w;
			}
			column = (column_t *)((byte *)column + column->length + 4);
		}
	}

	UploadTextureNoMip(w, h, block);
	Z_Free(block);
	Z_ChangeTag(patch, PU_CACHE);

	glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP);
	glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP);

	pic_iw[handle] = 1.0 / float(w);
	pic_ih[handle] = 1.0 / float(h);
	pic_sent[handle] = true;
}

//==========================================================================
//
//	TOpenGLDrawer::GeneratePicFromRaw
//
//==========================================================================

void TOpenGLDrawer::GeneratePicFromRaw(int handle)
{
	int lump = W_GetNumForName(pic_list[handle].name);
	int len = W_LumpLength(lump);
	byte *raw = (byte*)W_CacheLumpNum(lump, PU_STATIC);
	int h = len / 320;
	rgba_t *block = (rgba_t*)Z_Calloc(4 * len);
	rgba_t *pal = r_palette[pic_list[handle].palnum];
	int black = r_black_color[pic_list[handle].palnum];

	byte *src = raw;
	rgba_t *dst = block;
	for (int i = 0; i < len; i++, src++, dst++)
	{
		*dst = pal[*src ? *src : black];
	}

	UploadTextureNoMip(320, h, block);
	Z_Free(block);
	Z_ChangeTag(raw, PU_CACHE);

	if (h < 200)
	{
		glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
		glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
	}
	else
	{
		glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP);
		glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP);
	}

	pic_iw[handle] = 1.0 / float(320);
	pic_ih[handle] = 1.0 / float(h);
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
		if (SkinBPP == 8)
		{
			rgba_t *buf = (rgba_t*)Z_Malloc(SkinWidth * SkinHeight * 4);
			for (i = 0; i < SkinWidth * SkinHeight; i++)
			{
				buf[i] = SkinPal[SkinData[i]];
			}
			UploadTexture(SkinWidth, SkinHeight, buf);
			Z_Free(buf);
		}
		else
		{
			UploadTexture(SkinWidth, SkinHeight, (rgba_t *)SkinData);
		}
		glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
		glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
		Z_Free(SkinData);
	}
	glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, maxfilter);
	glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, mipfilter);
}

//==========================================================================
//
//	TOpenGLDrawer::ResampleTexture
//
//	Resizes	texture.
//	This is a simplified version of gluScaleImage from sources of MESA 3.0
//
//==========================================================================

void TOpenGLDrawer::ResampleTexture(int widthin, int heightin,
	const byte *datain, int widthout, int heightout, byte *dataout)
{
	int i, j, k;
	float sx, sy;

	if (widthout > 1)
		sx = float(widthin - 1) / float(widthout - 1);
	else
		sx = float(widthin - 1);
	if (heightout > 1)
		sy = float(heightin - 1) / float(heightout - 1);
	else
		sy = float(heightin - 1);

//#define POINT_SAMPLE
#ifdef POINT_SAMPLE
	for (i = 0; i < heightout; i++)
	{
		int ii = int(i * sy);
		for (j = 0; j < widthout; j++)
		{
			int jj = int(j * sx);

			const byte *src = datain + (ii * widthin + jj) * 4;
			byte *dst = dataout + (i * widthout + j) * 4;

			for (k = 0; k < 4; k++)
			{
				*dst++ = *src++;
			}
		}
	}
#else
	if (sx <= 1.0 && sy <= 1.0)
	{
		/* magnify both width and height:  use weighted sample of 4 pixels */
		int i0, i1, j0, j1;
		float alpha, beta;
		const byte *src00, *src01, *src10, *src11;
		float s1, s2;
		byte *dst;

		for (i = 0; i < heightout; i++)
		{
			i0 = int(i * sy);
			i1 = i0 + 1;
			if (i1 >= heightin) i1 = heightin-1;
			alpha = i * sy - i0;
			for (j = 0; j < widthout; j++)
			{
				j0 = int(j * sx);
				j1 = j0 + 1;
				if (j1 >= widthin) j1 = widthin-1;
				beta = j * sx - j0;

				/* compute weighted average of pixels in rect (i0,j0)-(i1,j1) */
				src00 = datain + (i0 * widthin + j0) * 4;
				src01 = datain + (i0 * widthin + j1) * 4;
				src10 = datain + (i1 * widthin + j0) * 4;
				src11 = datain + (i1 * widthin + j1) * 4;

				dst = dataout + (i * widthout + j) * 4;

				for (k = 0; k < 4; k++)
				{
					s1 = *src00++ * (1.0-beta) + *src01++ * beta;
					s2 = *src10++ * (1.0-beta) + *src11++ * beta;
					*dst++ = byte(s1 * (1.0-alpha) + s2 * alpha);
				}
			}
		}
	}
	else
	{
		/* shrink width and/or height:  use an unweighted box filter */
		int i0, i1;
		int j0, j1;
		int ii, jj;
		int sum;
		byte *dst;

		for (i = 0; i < heightout; i++)
		{
			i0 = int(i * sy);
			i1 = i0 + 1;
			if (i1 >= heightin) i1 = heightin-1;
			for (j = 0; j < widthout; j++)
			{
				j0 = int(j * sx);
				j1 = j0 + 1;
				if (j1 >= widthin) j1 = widthin-1;

				dst = dataout + (i * widthout + j) * 4;

				/* compute average of pixels in the rectangle (i0,j0)-(i1,j1) */
				for (k = 0; k < 4; k++)
				{
					sum = 0;
					for (ii = i0; ii <= i1; ii++)
					{
						for (jj = j0; jj <= j1; jj++)
						{
							sum += *(datain + (ii * widthin + jj) * 4 + k);
						}
					}
					sum /= (j1 - j0 + 1) * (i1 - i0 + 1);
					*dst++ = byte(sum);
				}
			}
		}
	}
#endif
}

//==========================================================================
//
//	TOpenGLDrawer::MipMap
//
//	Scales image down for next mipmap level, operates in place
//
//==========================================================================

void TOpenGLDrawer::MipMap(int width, int height, byte *in)
{
	int		i, j;
	byte	*out = in;

	if (width == 1 || height == 1)
	{
		//	Special case when only one dimension is scaled
		int total = width * height / 2;
		for (i = 0; i < total; i++, in += 8, out += 4)
		{
			out[0] = byte((in[0] + in[4]) >> 1);
			out[1] = byte((in[1] + in[5]) >> 1);
			out[2] = byte((in[2] + in[6]) >> 1);
			out[3] = byte((in[3] + in[7]) >> 1);
		}
		return;
	}

	//	Scale down in both dimensions
	width <<= 2;
	height >>= 1;
	for (i = 0; i < height; i++, in += width)
	{
		for (j = 0; j < width; j += 8, in += 8, out += 4)
		{
			out[0] = byte((in[0] + in[4] + in[width + 0] + in[width + 4]) >> 2);
			out[1] = byte((in[1] + in[5] + in[width + 1] + in[width + 5]) >> 2);
			out[2] = byte((in[2] + in[6] + in[width + 2] + in[width + 6]) >> 2);
			out[3] = byte((in[3] + in[7] + in[width + 3] + in[width + 7]) >> 2);
		}
	}
}

//==========================================================================
//
//	TOpenGLDrawer::UploadTexture
//
//==========================================================================

void TOpenGLDrawer::UploadTexture(int width, int height, rgba_t *data)
{
	int		w, h;
	byte	*image;
	int		level;
	byte	stackbuf[256 * 128 * 4];

	w = ToPowerOf2(width);
	if (w > maxTexSize)
	{
		w = maxTexSize;
	}
	h = ToPowerOf2(height);
	if (h > maxTexSize)
	{
		h = maxTexSize;
	}

	if (w * h * 4 <= int(sizeof(stackbuf)))
	{
		image = stackbuf;
	}
	else
	{
		image = (byte*)Z_Malloc(w * h * 4, PU_HIGH, 0);
	}
	if (w != width || h != height)
	{
		/* must rescale image to get "top" mipmap texture image */
		ResampleTexture(width, height, (byte*)data, w, h, image);
	}
	else
	{
		memcpy(image, data, w * h * 4);
	}
	glTexImage2D(GL_TEXTURE_2D, 0, 4, w, h, 0, GL_RGBA, GL_UNSIGNED_BYTE, image);

	for (level = 1; w > 1 || h > 1; level++)
	{
		MipMap(w, h, image);
		if (w > 1)
			w >>= 1;
		if (h > 1)
			h >>= 1;
		glTexImage2D(GL_TEXTURE_2D, level, 4, w, h, 0, GL_RGBA,
			GL_UNSIGNED_BYTE, image);
	}

	if (image != stackbuf)
	{
		Z_Free(image);
	}
}

//==========================================================================
//
//	TOpenGLDrawer::UploadTextureNoMip
//
//==========================================================================

void TOpenGLDrawer::UploadTextureNoMip(int width, int height, rgba_t *data)
{
	int		w, h;
	byte	*image;
	byte	stackbuf[64 * 1024];

	w = ToPowerOf2(width);
	if (w > maxTexSize)
	{
		w = maxTexSize;
	}
	h = ToPowerOf2(height);
	if (h > maxTexSize)
	{
		h = maxTexSize;
	}

	if (w != width || h != height)
	{
		/* must rescale image to get "top" mipmap texture image */
		if (w * h * 4 <= int(sizeof(stackbuf)))
		{
			image = stackbuf;
		}
		else
		{
			image = (byte*)Z_Malloc(w * h * 4, PU_HIGH, 0);
		}
		ResampleTexture(width, height, (byte*)data, w, h, image);
		glTexImage2D(GL_TEXTURE_2D, 0, 4, w, h, 0, GL_RGBA, GL_UNSIGNED_BYTE, image);
		if (image != stackbuf)
		{
			Z_Free(image);
		}
	}
	else
	{
		glTexImage2D(GL_TEXTURE_2D, 0, 4, w, h, 0, GL_RGBA, GL_UNSIGNED_BYTE, data);
	}
}

//**************************************************************************
//
//	$Log$
//	Revision 1.12  2001/10/18 17:36:31  dj_jl
//	A lots of changes for Alpha 2
//
//	Revision 1.11  2001/10/04 17:23:29  dj_jl
//	Got rid of some warnings
//	
//	Revision 1.10  2001/09/24 17:36:21  dj_jl
//	Added clamping
//	
//	Revision 1.9  2001/09/20 15:59:43  dj_jl
//	Fixed resampling when one dimansion doesn't change
//	
//	Revision 1.8  2001/08/30 17:37:39  dj_jl
//	Using linear texture resampling
//	
//	Revision 1.7  2001/08/24 17:05:44  dj_jl
//	Beautification
//	
//	Revision 1.6  2001/08/23 17:51:12  dj_jl
//	My own mipmap creation code, glu not used anymore
//	
//	Revision 1.5  2001/08/21 17:46:08  dj_jl
//	Added R_TextureAnimation, made SetTexture recognize flats
//	
//	Revision 1.4  2001/08/01 17:30:31  dj_jl
//	Fixed translated sprites
//	
//	Revision 1.3  2001/07/31 17:16:30  dj_jl
//	Just moved Log to the end of file
//	
//	Revision 1.2  2001/07/27 14:27:54  dj_jl
//	Update with Id-s and Log-s, some fixes
//
//**************************************************************************
