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
	for (int i = 0; i < GTextureManager.GetNumTextures(); i++)
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

	//	Delete all created shader objects.
	for (int i = CreatedShaderObjects.Num() - 1; i >= 0; i--)
	{
		p_glDeleteObjectARB(CreatedShaderObjects[i]);
	}
	CreatedShaderObjects.Clear();
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
// 	VOpenGLDrawer::PrecacheTexture
//
//==========================================================================

void VOpenGLDrawer::PrecacheTexture(VTexture* Tex)
{
	guard(VOpenGLDrawer::PrecacheTexture);
	SetTexture(Tex, 0);
	unguard;
}

//==========================================================================
//
// 	VOpenGLDrawer::SetTexture
//
//==========================================================================

void VOpenGLDrawer::SetTexture(VTexture* Tex, int CMap)
{
	guard(VOpenGLDrawer::SetTexture);
	SetSpriteLump(Tex, NULL, CMap);
	unguard;
}

//==========================================================================
//
//	VOpenGLDrawer::SetSpriteLump
//
//==========================================================================

void VOpenGLDrawer::SetSpriteLump(VTexture* Tex,
	VTextureTranslation* Translation, int CMap)
{
	guard(VOpenGLDrawer::SetSpriteLump);
	if (Tex->CheckModified())
	{
		FlushTexture(Tex);
	}
	if (Translation || CMap)
	{
		VTexture::VTransData* TData = Tex->FindDriverTrans(Translation, CMap);
		if (TData)
		{
			glBindTexture(GL_TEXTURE_2D, TData->Handle);
		}
		else
		{
			TData = &Tex->DriverTranslated.Alloc();
			TData->Handle = 0;
			TData->Trans = Translation;
			TData->ColourMap = CMap;
			GenerateTexture(Tex, (GLuint*)&TData->Handle, Translation, CMap);
		}
	}
	else
	{
		if (!Tex->DriverHandle)
		{
			GenerateTexture(Tex, &Tex->DriverHandle, NULL, 0);
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

void VOpenGLDrawer::SetPic(VTexture* Tex, VTextureTranslation* Trans,
	int CMap)
{
	guard(VOpenGLDrawer::SetPic);
	SetSpriteLump(Tex, Trans, CMap);
	unguard;
}

//==========================================================================
//
//	VOpenGLDrawer::GenerateTexture
//
//==========================================================================

void VOpenGLDrawer::GenerateTexture(VTexture* Tex, GLuint* pHandle,
	VTextureTranslation* Translation, int CMap)
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
	if (Translation && CMap)
	{
		rgba_t TmpPal[256];
		const vuint8* TrTab = Translation->GetTable();
		const rgba_t* CMPal = ColourMaps[CMap].GetPalette();
		for (int i = 0; i < 256; i++)
		{
			TmpPal[i] = CMPal[TrTab[i]];
		}
		UploadTexture8(SrcTex->GetWidth(), SrcTex->GetHeight(),
			SrcTex->GetPixels8(), TmpPal);
	}
	else if (Translation)
	{
		UploadTexture8(SrcTex->GetWidth(), SrcTex->GetHeight(),
			SrcTex->GetPixels8(), Translation->GetPalette());
	}
	else if (CMap)
	{
		UploadTexture8(SrcTex->GetWidth(), SrcTex->GetHeight(),
			SrcTex->GetPixels8(), ColourMaps[CMap].GetPalette());
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
		/* Smooth transparent edges */
		SmoothEdges((vuint8*)data, width, height, (vuint8*)data);
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
