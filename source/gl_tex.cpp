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
//	VOpenGLDrawer::InitData
//
//==========================================================================

void VOpenGLDrawer::InitData()
{
}

//==========================================================================
//
//	VOpenGLDrawer::InitTextures
//
//==========================================================================

void VOpenGLDrawer::InitTextures()
{
}

//==========================================================================
//
//	VOpenGLDrawer::GenerateTextures
//
//==========================================================================

void VOpenGLDrawer::GenerateTextures()
{
	guard(VOpenGLDrawer::GenerateTextures);
	int			i, j;
	rgba_t		pbuf[8][8];

	glGenTextures(MAX_TRANSLATED_SPRITES, trspr_id);
	glGenTextures(NUM_BLOCK_SURFS, lmap_id);
	glGenTextures(NUM_BLOCK_SURFS, addmap_id);
	glGenTextures(1, &particle_texture);

	memset(trspr_sent, 0, MAX_TRANSLATED_SPRITES);

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
	unguard;
}

//==========================================================================
//
//	VOpenGLDrawer::FlushTextures
//
//==========================================================================

void VOpenGLDrawer::FlushTextures()
{
	guard(VOpenGLDrawer::FlushTextures);
	for (int i = 0; i < GTextureManager.Textures.Num(); i++)
	{
		if (GTextureManager.Textures[i]->DriverHandle)
		{
			glDeleteTextures(1, (GLuint*)&GTextureManager.Textures[i]->DriverHandle);
			GTextureManager.Textures[i]->DriverHandle = 0;
		}
	}
	memset(trspr_sent, 0, MAX_TRANSLATED_SPRITES);
	unguard;
}

//==========================================================================
//
//	VOpenGLDrawer::DeleteTextures
//
//==========================================================================

void VOpenGLDrawer::DeleteTextures()
{
	guard(VOpenGLDrawer::DeleteTextures);
	if (texturesGenerated)
	{
		for (int i = 0; i < GTextureManager.Textures.Num(); i++)
		{
			if (GTextureManager.Textures[i]->DriverHandle)
			{
				glDeleteTextures(1, (GLuint*)&GTextureManager.Textures[i]->DriverHandle);
				GTextureManager.Textures[i]->DriverHandle = 0;
			}
		}
		glDeleteTextures(MAX_TRANSLATED_SPRITES, trspr_id);
		glDeleteTextures(NUM_BLOCK_SURFS, lmap_id);
		glDeleteTextures(NUM_BLOCK_SURFS, addmap_id);
		glDeleteTextures(1, &particle_texture);
		texturesGenerated = false;
	}
	unguard;
}

//==========================================================================
//
//	VOpenGLDrawer::ToPowerOf2
//
//==========================================================================

int VOpenGLDrawer::ToPowerOf2(int val)
{
	int answer = 1;
	while (answer < val)
		answer <<= 1;
	return answer;
}

//==========================================================================
//
// 	VOpenGLDrawer::SetTexture
//
//==========================================================================

void VOpenGLDrawer::SetTexture(int tex)
{
	guard(VOpenGLDrawer::SetTexture);
	tex = GTextureManager.TextureAnimation(tex);

	if (!GTextureManager.Textures[tex]->DriverHandle)
	{
		GenerateTexture(tex);
	}
	else
	{
		glBindTexture(GL_TEXTURE_2D, GTextureManager.Textures[tex]->DriverHandle);
	}
	glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, maxfilter);
	glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, mipfilter);
	tex_iw = 1.0 / GTextureManager.Textures[tex]->GetWidth();
	tex_ih = 1.0 / GTextureManager.Textures[tex]->GetHeight();
	unguard;
}

//==========================================================================
//
//	VOpenGLDrawer::SetSpriteLump
//
//==========================================================================

void VOpenGLDrawer::SetSpriteLump(int lump, int translation)
{
	guard(VOpenGLDrawer::SetSpriteLump);
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
					tex_iw = 1.0 / GTextureManager.Textures[lump]->GetWidth();
					tex_ih = 1.0 / GTextureManager.Textures[lump]->GetHeight();
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
		tex_iw = 1.0 / GTextureManager.Textures[lump]->GetWidth();
		tex_ih = 1.0 / GTextureManager.Textures[lump]->GetHeight();
	}
	else
	{
		if (!GTextureManager.Textures[lump]->DriverHandle)
		{
			GenerateTexture(lump);
		}
		else
		{
			glBindTexture(GL_TEXTURE_2D, GTextureManager.Textures[lump]->DriverHandle);
		}
		glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, maxfilter);
		glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, mipfilter);
		tex_iw = 1.0 / GTextureManager.Textures[lump]->GetWidth();
		tex_ih = 1.0 / GTextureManager.Textures[lump]->GetHeight();
	}
	unguard;
}

//==========================================================================
//
//	VOpenGLDrawer::SetPic
//
//==========================================================================

void VOpenGLDrawer::SetPic(int handle)
{
	guard(VOpenGLDrawer::SetPic);

	if (!GTextureManager.Textures[handle]->DriverHandle)
	{
		GenerateTexture(handle);
	}
	else
	{
		glBindTexture(GL_TEXTURE_2D, GTextureManager.Textures[handle]->DriverHandle);
	}

	tex_iw = 1.0 / GTextureManager.Textures[handle]->GetWidth();
	tex_ih = 1.0 / GTextureManager.Textures[handle]->GetHeight();

	glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, maxfilter);
	glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, mipfilter);
	unguard;
}

//==========================================================================
//
//	VOpenGLDrawer::GenerateTexture
//
//==========================================================================

void VOpenGLDrawer::GenerateTexture(int texnum)
{
	guard(VOpenGLDrawer::GenerateTexture);
	TTexture* Tex = GTextureManager.Textures[texnum];

	glGenTextures(1, (GLuint*)&Tex->DriverHandle);
	glBindTexture(GL_TEXTURE_2D, Tex->DriverHandle);

	//	Try to load high resolution version.
	int HRWidth;
	int HRHeight;
	rgba_t* HRPixels = Tex->GetHighResPixels(HRWidth, HRHeight);
	if (HRPixels)
	{
		//	Use high resolution version.
		UploadTexture(HRWidth, HRHeight, HRPixels);
		Z_Free(HRPixels);
	}
	else
	{
		//	Use normal version.
		byte* block = Tex->GetPixels();
		if (Tex->Format == TEXFMT_8 || Tex->Format == TEXFMT_8Pal)
		{
			UploadTexture8(Tex->GetWidth(), Tex->GetHeight(), block,
				Tex->GetPalette());
		}
		else
		{
			UploadTexture(Tex->GetWidth(), Tex->GetHeight(), (rgba_t*)block);
		}
		Tex->Unload();
	}

	//	Set up texture wrapping.
	if (Tex->Type == TEXTYPE_Wall || Tex->Type == TEXTYPE_Flat ||
		Tex->Type == TEXTYPE_Overload)
	{
		glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
		glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
	}
	else
	{
		glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, ClampToEdge);
		glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, ClampToEdge);
	}
	//	Set up texture anisotropic filtering.
	if (max_anisotropy > 1.0)
	{
		glTexParameterf(GL_TEXTURE_2D, GLenum(GL_TEXTURE_MAX_ANISOTROPY_EXT), max_anisotropy);
	}
	unguard;
}

//==========================================================================
//
//	VOpenGLDrawer::GenerateTranslatedSprite
//
//==========================================================================

void VOpenGLDrawer::GenerateTranslatedSprite(int lump, int slot, int translation)
{
	guard(VOpenGLDrawer::GenerateTranslatedSprite);
	TTexture* Tex = GTextureManager.Textures[lump];

	trspr_lump[slot] = lump;
	trspr_tnum[slot] = translation;
	trspr_sent[slot] = true;

	// Generate The Texture
	byte* Pixels = Tex->GetPixels8();
	byte* block = (byte*)Z_Malloc(Tex->GetWidth() * Tex->GetHeight(), PU_STATIC, 0);
	byte* trtab = translationtables + translation * 256;
	for (int i = 0; i < Tex->GetWidth() * Tex->GetHeight(); i++)
	{
		block[i] = trtab[Pixels[i]];
	}

	UploadTexture8(Tex->GetWidth(), Tex->GetHeight(), block,
		Tex->GetPalette());
	glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, ClampToEdge);
	glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, ClampToEdge);
	//	Set up texture anisotropic filtering.
	if (max_anisotropy > 1.0)
	{
		glTexParameterf(GL_TEXTURE_2D, GLenum(GL_TEXTURE_MAX_ANISOTROPY_EXT), max_anisotropy);
	}

	Z_Free(block);
	Tex->Unload();
	unguard;
}

//==========================================================================
//
//	VOpenGLDrawer::AdjustGamma
//
//==========================================================================

void VOpenGLDrawer::AdjustGamma(rgba_t *data, int size)
{
	guard(VOpenGLDrawer::AdjustGamma);
	byte *gt = gammatable[usegamma];
	for (int i = 0; i < size; i++)
	{
		data[i].r = gt[data[i].r];
		data[i].g = gt[data[i].g];
		data[i].b = gt[data[i].b];
	}
	unguard;
}

//==========================================================================
//
//	VOpenGLDrawer::ResampleTexture
//
//	Resizes	texture.
//	This is a simplified version of gluScaleImage from sources of MESA 3.0
//
//==========================================================================

void VOpenGLDrawer::ResampleTexture(int widthin, int heightin,
	const byte *datain, int widthout, int heightout, byte *dataout)
{
	guard(VOpenGLDrawer::ResampleTexture);
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
	unguard;
}

//==========================================================================
//
//	VOpenGLDrawer::MipMap
//
//	Scales image down for next mipmap level, operates in place
//
//==========================================================================

void VOpenGLDrawer::MipMap(int width, int height, byte* InIn)
{
	guard(VOpenGLDrawer::MipMap);
	byte* in = InIn;
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
	unguard;
}

//==========================================================================
//
//	VOpenGLDrawer::UploadTexture8
//
//==========================================================================

void VOpenGLDrawer::UploadTexture8(int Width, int Height, byte* Data,
	rgba_t* Pal)
{
	rgba_t* NewData = (rgba_t*)Z_Calloc(Width * Height * 4, PU_STATIC, 0);
	for (int i = 0; i < Width * Height; i++)
	{
		if (Data[i])
			NewData[i] = Pal[Data[i]];
	}
	UploadTexture(Width, Height, NewData);
	Z_Free(NewData);
}

//==========================================================================
//
//	VOpenGLDrawer::UploadTexture
//
//==========================================================================

void VOpenGLDrawer::UploadTexture(int width, int height, rgba_t *data)
{
	guard(VOpenGLDrawer::UploadTexture);
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
	AdjustGamma((rgba_t*)image, w * h);
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
	unguard;
}

//**************************************************************************
//
//	$Log$
//	Revision 1.28  2006/01/03 19:57:45  dj_jl
//	Fixed anisotropic texture filtering.
//
//	Revision 1.27  2005/05/26 16:50:14  dj_jl
//	Created texture manager class
//	
//	Revision 1.26  2005/05/03 14:57:06  dj_jl
//	Added support for specifying skin index.
//	
//	Revision 1.25  2005/04/28 07:16:15  dj_jl
//	Fixed some warnings, other minor fixes.
//	
//	Revision 1.24  2005/03/28 07:25:40  dj_jl
//	Changed location of hi-res 2D graphics.
//	
//	Revision 1.23  2005/01/24 12:53:54  dj_jl
//	Skybox fixes.
//	
//	Revision 1.22  2004/12/27 12:23:16  dj_jl
//	Multiple small changes for version 1.16
//	
//	Revision 1.21  2004/11/30 07:19:00  dj_jl
//	Support for high resolution textures.
//	
//	Revision 1.20  2004/11/23 12:43:10  dj_jl
//	Wad file lump namespaces.
//	
//	Revision 1.19  2002/07/13 07:38:00  dj_jl
//	Added drawers to the object tree.
//	
//	Revision 1.18  2002/04/11 16:44:44  dj_jl
//	Got rid of some warnings.
//	
//	Revision 1.17  2002/03/20 19:09:53  dj_jl
//	DeepSea tall patches support.
//	
//	Revision 1.16  2002/01/11 18:24:44  dj_jl
//	Added guard macros
//	
//	Revision 1.15  2002/01/07 12:16:42  dj_jl
//	Changed copyright year
//	
//	Revision 1.14  2001/11/09 14:18:40  dj_jl
//	Added specular highlights
//	
//	Revision 1.13  2001/10/27 07:45:01  dj_jl
//	Added gamma controls
//	
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
