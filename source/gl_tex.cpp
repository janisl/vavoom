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

#include "gl_local.h"

// MACROS ------------------------------------------------------------------

// TYPES -------------------------------------------------------------------

// EXTERNAL FUNCTION PROTOTYPES --------------------------------------------

// PUBLIC FUNCTION PROTOTYPES ----------------------------------------------

// PRIVATE FUNCTION PROTOTYPES ---------------------------------------------

// EXTERNAL DATA DECLARATIONS ----------------------------------------------

// PUBLIC DATA DEFINITIONS -------------------------------------------------

// PRIVATE DATA DEFINITIONS ------------------------------------------------

static vuint8 ptex[8][8] =
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

	glGenTextures(NUM_BLOCK_SURFS, lmap_id);
	glGenTextures(NUM_BLOCK_SURFS, addmap_id);
	glGenTextures(1, &particle_texture);

	for (j = 0; j < 8; j++)
	{
		for (i = 0; i < 8; i++)
		{
			pbuf[j][i].r = 255;
			pbuf[j][i].g = 255;
			pbuf[j][i].b = 255;
			pbuf[j][i].a = vuint8(ptex[j][i] * 255);
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
		FlushTexture(GTextureManager[i]);
	}
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
		FlushTextures();
		glDeleteTextures(NUM_BLOCK_SURFS, lmap_id);
		glDeleteTextures(NUM_BLOCK_SURFS, addmap_id);
		glDeleteTextures(1, &particle_texture);
		texturesGenerated = false;
	}
	unguard;
}

//==========================================================================
//
//	VOpenGLDrawer::FlushTexture
//
//==========================================================================

void VOpenGLDrawer::FlushTexture(VTexture* Tex)
{
	guard(VOpenGLDrawer::FlushTexture);
	if (Tex->DriverHandle)
	{
		glDeleteTextures(1, (GLuint*)&Tex->DriverHandle);
		Tex->DriverHandle = 0;
	}
	for (int j = 0; j < Tex->DriverTranslated.Num(); j++)
	{
		glDeleteTextures(1, (GLuint*)&Tex->DriverTranslated[j].Handle);
	}
	Tex->DriverTranslated.Clear();
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

void VOpenGLDrawer::SetTexture(VTexture* Tex)
{
	guard(VOpenGLDrawer::SetTexture);
	if (!Tex->DriverHandle || Tex->CheckModified())
	{
		GenerateTexture(Tex, &Tex->DriverHandle, NULL);
	}
	else
	{
		glBindTexture(GL_TEXTURE_2D, Tex->DriverHandle);
	}
	glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, maxfilter);
	glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, mipfilter);
	tex_iw = 1.0 / Tex->GetWidth();
	tex_ih = 1.0 / Tex->GetHeight();
	unguard;
}

//==========================================================================
//
//	VOpenGLDrawer::SetSpriteLump
//
//==========================================================================

void VOpenGLDrawer::SetSpriteLump(VTexture* Tex, VTextureTranslation* Translation)
{
	guard(VOpenGLDrawer::SetSpriteLump);
	if (Tex->CheckModified())
	{
		FlushTexture(Tex);
	}
	if (Translation)
	{
		VTexture::VTransData* TData = Tex->FindDriverTrans(Translation);
		if (TData)
		{
			glBindTexture(GL_TEXTURE_2D, TData->Handle);
		}
		else
		{
			TData = &Tex->DriverTranslated.Alloc();
			TData->Handle = 0;
			TData->Trans = Translation;
			GenerateTexture(Tex, (GLuint*)&TData->Handle, Translation);
		}
	}
	else
	{
		if (!Tex->DriverHandle)
		{
			GenerateTexture(Tex, &Tex->DriverHandle, NULL);
		}
		else
		{
			glBindTexture(GL_TEXTURE_2D, Tex->DriverHandle);
		}
	}

	glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, maxfilter);
	glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, mipfilter);
	tex_iw = 1.0 / Tex->GetWidth();
	tex_ih = 1.0 / Tex->GetHeight();
	unguard;
}

//==========================================================================
//
//	VOpenGLDrawer::SetPic
//
//==========================================================================

void VOpenGLDrawer::SetPic(VTexture* Tex, VTextureTranslation* Trans)
{
	guard(VOpenGLDrawer::SetPic);
	SetSpriteLump(Tex, Trans);
	unguard;
}

//==========================================================================
//
//	VOpenGLDrawer::GenerateTexture
//
//==========================================================================

void VOpenGLDrawer::GenerateTexture(VTexture* Tex, GLuint* pHandle,
	VTextureTranslation* Translation)
{
	guard(VOpenGLDrawer::GenerateTexture);
	if (!*pHandle)
	{
		glGenTextures(1, pHandle);
	}
	glBindTexture(GL_TEXTURE_2D, *pHandle);

	//	Try to load high resolution version.
	VTexture* SrcTex = Tex->GetHighResolutionTexture();
	if (!SrcTex)
	{
		SrcTex = Tex;
	}

	//	Upload data.
	if (Translation)
	{
		UploadTexture8(SrcTex->GetWidth(), SrcTex->GetHeight(),
			SrcTex->GetPixels8(), Translation->GetPalette());
	}
	else
	{
		vuint8* block = SrcTex->GetPixels();
		if (SrcTex->Format == TEXFMT_8 || SrcTex->Format == TEXFMT_8Pal)
		{
			UploadTexture8(SrcTex->GetWidth(), SrcTex->GetHeight(), block,
				SrcTex->GetPalette());
		}
		else
		{
			UploadTexture(SrcTex->GetWidth(), SrcTex->GetHeight(),
				(rgba_t*)block);
		}
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
//	VOpenGLDrawer::AdjustGamma
//
//==========================================================================

void VOpenGLDrawer::AdjustGamma(rgba_t *data, int size)
{
	guard(VOpenGLDrawer::AdjustGamma);
	vuint8* gt = gammatable[usegamma];
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
	const vuint8* datain, int widthout, int heightout, vuint8* dataout)
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

			const vuint8* src = datain + (ii * widthin + jj) * 4;
			vuint8* dst = dataout + (i * widthout + j) * 4;

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
		const vuint8 *src00, *src01, *src10, *src11;
		float s1, s2;
		vuint8* dst;

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
					*dst++ = vuint8(s1 * (1.0-alpha) + s2 * alpha);
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
		vuint8* dst;

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
					*dst++ = vuint8(sum);
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

void VOpenGLDrawer::MipMap(int width, int height, vuint8* InIn)
{
	guard(VOpenGLDrawer::MipMap);
	vuint8* in = InIn;
	int		i, j;
	vuint8* out = in;

	if (width == 1 || height == 1)
	{
		//	Special case when only one dimension is scaled
		int total = width * height / 2;
		for (i = 0; i < total; i++, in += 8, out += 4)
		{
			out[0] = vuint8((in[0] + in[4]) >> 1);
			out[1] = vuint8((in[1] + in[5]) >> 1);
			out[2] = vuint8((in[2] + in[6]) >> 1);
			out[3] = vuint8((in[3] + in[7]) >> 1);
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
			out[0] = vuint8((in[0] + in[4] + in[width + 0] + in[width + 4]) >> 2);
			out[1] = vuint8((in[1] + in[5] + in[width + 1] + in[width + 5]) >> 2);
			out[2] = vuint8((in[2] + in[6] + in[width + 2] + in[width + 6]) >> 2);
			out[3] = vuint8((in[3] + in[7] + in[width + 3] + in[width + 7]) >> 2);
		}
	}
	unguard;
}

//==========================================================================
//
//	VOpenGLDrawer::UploadTexture8
//
//==========================================================================

void VOpenGLDrawer::UploadTexture8(int Width, int Height, const vuint8* Data,
	const rgba_t* Pal)
{
	rgba_t* NewData = (rgba_t*)Z_Calloc(Width * Height * 4);
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

void VOpenGLDrawer::UploadTexture(int width, int height, const rgba_t* data)
{
	guard(VOpenGLDrawer::UploadTexture);
	int		w, h;
	vuint8*	image;
	int		level;
	vuint8	stackbuf[256 * 128 * 4];

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
		image = (vuint8*)Z_Malloc(w * h * 4);
	}
	if (w != width || h != height)
	{
		/* must rescale image to get "top" mipmap texture image */
		ResampleTexture(width, height, (vuint8*)data, w, h, image);
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
