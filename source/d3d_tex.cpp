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

#include "d3d_local.h"

// MACROS ------------------------------------------------------------------

#define SAFE_RELEASE_TEXTURE(iface) \
	if (iface) \
	{ \
		((LPDIRECT3DTEXTURE9)iface)->Release(); \
		iface = NULL; \
	}

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
//	VDirect3DDrawer::ToPowerOf2
//
//==========================================================================

int VDirect3DDrawer::ToPowerOf2(int val)
{
	int answer = 1;
	while (answer < val)
		answer <<= 1;
	return answer;
}

//==========================================================================
//
//	VDirect3DDrawer::CreateSurface
//
//==========================================================================

LPDIRECT3DTEXTURE9 VDirect3DDrawer::CreateSurface(int w, int h, int bpp, bool mipmaps)
{
	guard(VDirect3DDrawer::CreateSurface);
	LPDIRECT3DTEXTURE9 surf = NULL;

	HRESULT res = RenderDevice->CreateTexture(w, h, mipmaps ? 0 : 1, 0,
		bpp == 32 ? D3DFMT_A8R8G8B8 : D3DFMT_A1R5G5B5,
		D3DPOOL_MANAGED, &surf, NULL);
	if (res != D3D_OK)
	{
		if (res == D3DERR_INVALIDCALL)
			GCon->Log("Invalid call");
		else if (res == D3DERR_OUTOFVIDEOMEMORY)
			GCon->Log("Out of vid mem");
		else if (res == E_OUTOFMEMORY)
			GCon->Log("Out of mem");
		else
			GCon->Logf("Unknown error %d", res);
		Sys_Error("Create texture failed\n");
	}
	return surf;
	unguard;
}

//==========================================================================
//
//	VDirect3DDrawer::InitTextures
//
//==========================================================================

void VDirect3DDrawer::InitTextures()
{
	guard(VDirect3DDrawer::InitTextures);
	//	Lightmaps, seperate from other surfaces so CreateSurface doesn't
	// release them
	light_surf = (LPDIRECT3DTEXTURE9*)Z_Calloc(NUM_BLOCK_SURFS * 4);
	add_surf = (LPDIRECT3DTEXTURE9*)Z_Calloc(NUM_BLOCK_SURFS * 4);
	unguard;
}

//==========================================================================
//
//	VDirect3DDrawer::FlushTextures
//
//==========================================================================

void VDirect3DDrawer::FlushTextures()
{
	guard(VDirect3DDrawer::FlushTextures);
	for (int i = 0; i < GTextureManager.GetNumTextures(); i++)
	{
		FlushTexture(GTextureManager[i]);
	}
	unguard;
}

//==========================================================================
//
//	VDirect3DDrawer::ReleaseTextures
//
//==========================================================================

void VDirect3DDrawer::ReleaseTextures()
{
	guard(VDirect3DDrawer::ReleaseTextures);
	FlushTextures();
	for (int i = 0; i < NUM_BLOCK_SURFS; i++)
	{
		SAFE_RELEASE(light_surf[i]);
		SAFE_RELEASE(add_surf[i]);
	}
	SAFE_RELEASE(particle_texture);
	unguard;
}

//==========================================================================
//
//	VDirect3DDrawer::FlushTexture
//
//==========================================================================

void VDirect3DDrawer::FlushTexture(VTexture* Tex)
{
	guard(VDirect3DDrawer::FlushTexture);
	SAFE_RELEASE_TEXTURE(Tex->DriverData);
	for (int j = 0; j < Tex->DriverTranslated.Num(); j++)
	{
		SAFE_RELEASE_TEXTURE(Tex->DriverTranslated[j].Data);
	}
	Tex->DriverTranslated.Clear();
	unguard;
}

//==========================================================================
//
// 	VDirect3DDrawer::SetTexture
//
//==========================================================================

void VDirect3DDrawer::SetTexture(VTexture* Tex, int CMap)
{
	guard(VDirect3DDrawer::SetTexture);
	SetSpriteLump(Tex, NULL, CMap);
	RenderDevice->SetSamplerState(0, D3DSAMP_ADDRESSU, D3DTADDRESS_WRAP);
	RenderDevice->SetSamplerState(0, D3DSAMP_ADDRESSV, D3DTADDRESS_WRAP);
	unguard;
}

//==========================================================================
//
//	VDirect3DDrawer::SetSpriteLump
//
//==========================================================================

void VDirect3DDrawer::SetSpriteLump(VTexture* Tex,
	VTextureTranslation* Translation, int CMap)
{
	guard(VDirect3DDrawer::SetSpriteLump);
	if (!RenderDevice)
	{
		return;
	}

	if (Tex->CheckModified())
	{
		FlushTexture(Tex);
	}

	if (Translation || CMap)
	{
		VTexture::VTransData* TData = Tex->FindDriverTrans(Translation, CMap);
		if (!TData)
		{
			TData = &Tex->DriverTranslated.Alloc();
			TData->Data = NULL;
			TData->Trans = Translation;
			TData->ColourMap = CMap;
			GenerateTexture(Tex, &TData->Data, Translation, CMap);
		}
		RenderDevice->SetTexture(0, (LPDIRECT3DTEXTURE9)TData->Data);
	}
	else
	{
		if (!Tex->DriverData)
		{
			GenerateTexture(Tex, &Tex->DriverData, NULL, 0);
		}
		RenderDevice->SetTexture(0, (LPDIRECT3DTEXTURE9)Tex->DriverData);
	}

	RenderDevice->SetSamplerState(0, D3DSAMP_ADDRESSU, D3DTADDRESS_CLAMP);
	RenderDevice->SetSamplerState(0, D3DSAMP_ADDRESSV, D3DTADDRESS_CLAMP);
	tex_iw = 1.0 / Tex->GetWidth();
	tex_ih = 1.0 / Tex->GetHeight();
	unguard;
}

//==========================================================================
//
//	VDirect3DDrawer::SetPic
//
//==========================================================================

void VDirect3DDrawer::SetPic(VTexture* Tex, VTextureTranslation* Trans,
	int CMap)
{
	guard(VDirect3DDrawer::SetPic);
	SetSpriteLump(Tex, Trans, CMap);
	unguard;
}

//==========================================================================
//
//	VDirect3DDrawer::GenerateTexture
//
//==========================================================================

void VDirect3DDrawer::GenerateTexture(VTexture* Tex, void** Data,
	VTextureTranslation* Translation, int CMap)
{
	guard(VDirect3DDrawer::GenerateTexture);
	//	Try to load high resolution version.
	VTexture* SrcTex = Tex->GetHighResolutionTexture();
	if (!SrcTex)
	{
		SrcTex = Tex;
	}

	SAFE_RELEASE_TEXTURE(*Data);
	if (Translation && CMap)
	{
		rgba_t TmpPal[256];
		const vuint8* TrTab = Translation->GetTable();
		const rgba_t* CMPal = ColourMaps[CMap].GetPalette();
		for (int i = 0; i < 256; i++)
		{
			TmpPal[i] = CMPal[TrTab[i]];
		}
		*Data = UploadTexture8(SrcTex->GetWidth(), SrcTex->GetHeight(),
			SrcTex->GetPixels8(), TmpPal);
	}
	else if (Translation)
	{
		*Data = UploadTexture8(SrcTex->GetWidth(), SrcTex->GetHeight(),
			SrcTex->GetPixels8(), Translation->GetPalette());
	}
	else if (CMap)
	{
		*Data = UploadTexture8(SrcTex->GetWidth(), SrcTex->GetHeight(),
			SrcTex->GetPixels8(), ColourMaps[CMap].GetPalette());
	}
	else
	{
		byte* block = SrcTex->GetPixels();
		if (SrcTex->Format == TEXFMT_8 || SrcTex->Format == TEXFMT_8Pal)
		{
			*Data = UploadTexture8(SrcTex->GetWidth(),
				SrcTex->GetHeight(), block, SrcTex->GetPalette());
		}
		else
		{
			*Data = UploadTexture(SrcTex->GetWidth(),
				SrcTex->GetHeight(), (rgba_t*)block);
		}
	}
	unguard;
}

//==========================================================================
//
//	VDirect3DDrawer::UploadTextureImage
//
//==========================================================================

void VDirect3DDrawer::UploadTextureImage(LPDIRECT3DTEXTURE9 tex, int level,
	int width, int height, const rgba_t* data)
{
	guard(VDirect3DDrawer::UploadTextureImage);
	LPDIRECT3DSURFACE9 surf;
	tex->GetSurfaceLevel(level, &surf);

	D3DLOCKED_RECT lrect;
	if (FAILED(surf->LockRect(&lrect, NULL, 0)))
	{
		GCon->Logf(NAME_Dev, "Failed to lock surface");
		return;
	}

	D3DSURFACE_DESC desc;
	surf->GetDesc(&desc);

	const rgba_t *in = data;
	if (desc.Format == D3DFMT_A1R5G5B5)
	{
		word *out = (word*)lrect.pBits;
		for (int i = 0; i < width * height; i++, in++, out++)
		{
			*out = MakeCol16(in->r, in->g, in->b, in->a);
		}
	}
	else if (desc.Format == D3DFMT_A8R8G8B8)
	{
		vuint32* out = (vuint32*)lrect.pBits;
		for (int i = 0; i < width * height; i++, in++, out++)
		{
			*out = MakeCol32(in->r, in->g, in->b, in->a);
		}
	}
	surf->UnlockRect();
	surf->Release();
	unguard;
}

//==========================================================================
//
//	VDirect3DDrawer::AdjustGamma
//
//==========================================================================

void VDirect3DDrawer::AdjustGamma(rgba_t *data, int size)
{
	guard(VDirect3DDrawer::AdjustGamma);
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
//	VDirect3DDrawer::ResampleTexture
//
//	Resizes	texture.
//	This is a simplified version of gluScaleImage from sources of MESA 3.0
//
//==========================================================================

void VDirect3DDrawer::ResampleTexture(int widthin, int heightin,
	const byte *datain, int widthout, int heightout, byte *dataout)
{
	guard(VDirect3DDrawer::ResampleTexture);
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
//	VDirect3DDrawer::MipMap
//
//	Scales image down for next mipmap level, operates in place
//
//==========================================================================

void VDirect3DDrawer::MipMap(int width, int height, byte *in)
{
	guard(VDirect3DDrawer::MipMap);
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
//	VDirect3DDrawer::UploadTexture8
//
//==========================================================================

LPDIRECT3DTEXTURE9 VDirect3DDrawer::UploadTexture8(int Width, int Height,
	const byte* Data, const rgba_t* Pal)
{
	rgba_t* NewData = (rgba_t*)Z_Calloc(Width * Height * 4);
	for (int i = 0; i < Width * Height; i++)
	{
		if (Data[i])
			NewData[i] = Pal[Data[i]];
	}
	LPDIRECT3DTEXTURE9 Ret;
	Ret = UploadTexture(Width, Height, NewData);
	Z_Free(NewData);
	return Ret;
}

//==========================================================================
//
//	VDirect3DDrawer::UploadTexture
//
//==========================================================================

LPDIRECT3DTEXTURE9 VDirect3DDrawer::UploadTexture(int width, int height, const rgba_t *data)
{
	guard(VDirect3DDrawer::UploadTexture);
	int						w, h;
	byte					*image;
	byte					stackbuf[256 * 128 * 4];
	LPDIRECT3DTEXTURE9		surf;
	UINT					level;

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
	if (square_textures)
	{
		w = h = MAX(w, h);
	}

	if (w * h * 4 <= int(sizeof(stackbuf)))
	{
		image = stackbuf;
	}
	else
	{
		image = (byte*)Z_Malloc(w * h * 4);
	}
	if (w != width || h != height)
	{
		//	Must rescale image to get "top" mipmap texture image
		ResampleTexture(width, height, (byte*)data, w, h, image);
	}
	else
	{
		memcpy(image, data, w * h * 4);
	}
	AdjustGamma((rgba_t*)image, w * h);
	surf = CreateSurface(w, h, 16, true);
	UploadTextureImage(surf, 0, w, h, (rgba_t*)image);

	for (level = 1; level < surf->GetLevelCount(); level++)
	{
		MipMap(w, h, image);
		if (w > 1)
			w >>= 1;
		if (h > 1)
			h >>= 1;
		UploadTextureImage(surf, level, w, h, (rgba_t*)image);
	}

	if (image != stackbuf)
	{
		Z_Free(image);
	}
	return surf;
	unguard;
}
