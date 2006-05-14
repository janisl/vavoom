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

#include "d3d_local.h"

// MACROS ------------------------------------------------------------------

#if DIRECT3D_VERSION >= 0x0800
#define SAFE_RELEASE_TEXTURE(iface) \
	if (iface) \
	{ \
		((LPDIRECT3DTEXTURE8)iface)->Release(); \
		iface = NULL; \
	}
#else
#define SAFE_RELEASE_TEXTURE(iface) \
	if (iface) \
	{ \
		((LPDIRECTDRAWSURFACE7)iface)->Release(); \
		iface = NULL; \
	}
#endif

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

#if DIRECT3D_VERSION >= 0x0800
LPDIRECT3DTEXTURE8 VDirect3DDrawer::CreateSurface(int w, int h, int bpp, bool mipmaps)
{
	guard(VDirect3DDrawer::CreateSurface);
	LPDIRECT3DTEXTURE8 surf = NULL;

	HRESULT res = RenderDevice->CreateTexture(w, h, mipmaps ? 0 : 1, 0,
		bpp == 32 ? D3DFMT_A8R8G8B8 : D3DFMT_A1R5G5B5,
		D3DPOOL_MANAGED, &surf);
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
#else
LPDIRECTDRAWSURFACE7 VDirect3DDrawer::CreateSurface(int w, int h, int bpp, bool mipmaps)
{
	guard(VDirect3DDrawer::CreateSurface);
	DDSURFACEDESC2			ddsd;
	LPDIRECTDRAWSURFACE7	surf = NULL;
	int i;

	memset(&ddsd, 0, sizeof(ddsd));
	ddsd.dwSize = sizeof(ddsd);
	ddsd.dwFlags = DDSD_WIDTH | DDSD_HEIGHT | DDSD_CAPS | DDSD_PIXELFORMAT;
	ddsd.ddsCaps.dwCaps = DDSCAPS_TEXTURE | SurfaceMemFlag;
	if (mipmaps)
	{
		ddsd.ddsCaps.dwCaps |= DDSCAPS_MIPMAP | DDSCAPS_COMPLEX;
	}
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
		for (i = 0; i < GTextureManager.Textures.Num() + MAX_TRANSLATED_SPRITES; i++)
		{
			int index = (i + tscount) % (GTextureManager.Textures.Num() + MAX_TRANSLATED_SPRITES);
			if (index < GTextureManager.Textures.Num())
			{
				if (GTextureManager.Textures[index]->DriverData)
				{
					SAFE_RELEASE_TEXTURE(GTextureManager.Textures[index]->DriverData);
					break;
				}
			}
			else
			{
				if (trsprdata[index - GTextureManager.Textures.Num()])
				{
					SAFE_RELEASE(trsprdata[index - GTextureManager.Textures.Num()]);
					break;
				}
			}
		}
	} while (i < GTextureManager.Textures.Num() + MAX_TRANSLATED_SPRITES);

	Sys_Error("Failed to create surface");
	return NULL;
	unguard;
}
#endif

//==========================================================================
//
//	VDirect3DDrawer::InitTextures
//
//==========================================================================

void VDirect3DDrawer::InitTextures()
{
	guard(VDirect3DDrawer::InitTextures);
#if DIRECT3D_VERSION >= 0x0800
	//	Sprite lumps
	trsprdata = (LPDIRECT3DTEXTURE8*)Z_Calloc(MAX_TRANSLATED_SPRITES * 4);

	//	Lightmaps, seperate from other surfaces so CreateSurface doesn't
	// release them
	light_surf = (LPDIRECT3DTEXTURE8*)Z_Calloc(NUM_BLOCK_SURFS * 4);
	add_surf = (LPDIRECT3DTEXTURE8*)Z_Calloc(NUM_BLOCK_SURFS * 4);
#else
	//	Sprite lumps
	trsprdata = (LPDIRECTDRAWSURFACE7*)Z_Calloc(MAX_TRANSLATED_SPRITES * 4);

	//	Lightmaps, seperate from other surfaces so CreateSurface doesn't
	// release them
	light_surf = (LPDIRECTDRAWSURFACE7*)Z_Calloc(NUM_BLOCK_SURFS * 4);
	add_surf = (LPDIRECTDRAWSURFACE7*)Z_Calloc(NUM_BLOCK_SURFS * 4);
#endif
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
	int i;
	for (i = 0; i < GTextureManager.Textures.Num(); i++)
	{
		SAFE_RELEASE_TEXTURE(GTextureManager.Textures[i]->DriverData);
	}
	for (i = 0; i < MAX_TRANSLATED_SPRITES; i++)
	{
		SAFE_RELEASE(trsprdata[i]);
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
	int i;
	for (i = 0; i < GTextureManager.Textures.Num(); i++)
	{
		SAFE_RELEASE_TEXTURE(GTextureManager.Textures[i]->DriverData);
	}
	for (i = 0; i < MAX_TRANSLATED_SPRITES; i++)
	{
		SAFE_RELEASE(trsprdata[i]);
	}
	for (i = 0; i < NUM_BLOCK_SURFS; i++)
	{
		SAFE_RELEASE(light_surf[i]);
		SAFE_RELEASE(add_surf[i]);
	}
	SAFE_RELEASE(particle_texture);
	unguard;
}

//==========================================================================
//
// 	VDirect3DDrawer::SetTexture
//
//==========================================================================

void VDirect3DDrawer::SetTexture(int tex)
{
	guard(VDirect3DDrawer::SetTexture);
	if (!RenderDevice)
	{
		return;
	}

	tex = GTextureManager.TextureAnimation(tex);

	if (!GTextureManager.Textures[tex]->DriverData)
	{
		GenerateTexture(tex);
	}

#if DIRECT3D_VERSION >= 0x0800
	RenderDevice->SetTexture(TexStage, (LPDIRECT3DTEXTURE8)GTextureManager.Textures[tex]->DriverData);
#else
	RenderDevice->SetTexture(TexStage, (LPDIRECTDRAWSURFACE7)GTextureManager.Textures[tex]->DriverData);
#endif
	if (GTextureManager.Textures[tex]->Type == TEXTYPE_SkyMap)
	{
		RenderDevice->SetTextureStageState(TexStage, D3DTSS_ADDRESSU, D3DTADDRESS_CLAMP);
		RenderDevice->SetTextureStageState(TexStage, D3DTSS_ADDRESSV, D3DTADDRESS_CLAMP);
	}
	else
	{
		RenderDevice->SetTextureStageState(TexStage, D3DTSS_ADDRESSU, D3DTADDRESS_WRAP);
		RenderDevice->SetTextureStageState(TexStage, D3DTSS_ADDRESSV, D3DTADDRESS_WRAP);
	}
	tex_iw = 1.0 / GTextureManager.Textures[tex]->GetWidth();
	tex_ih = 1.0 / GTextureManager.Textures[tex]->GetHeight();
	unguard;
}

//==========================================================================
//
//	VDirect3DDrawer::SetSpriteLump
//
//==========================================================================

void VDirect3DDrawer::SetSpriteLump(int lump, int translation)
{
	guard(VDirect3DDrawer::SetSpriteLump);
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
					RenderDevice->SetTextureStageState(0, D3DTSS_ADDRESSU, D3DTADDRESS_CLAMP);
					RenderDevice->SetTextureStageState(0, D3DTSS_ADDRESSV, D3DTADDRESS_CLAMP);
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
			SAFE_RELEASE(trsprdata[0]);
			avail = 0;
		}
		GenerateTranslatedSprite(lump, avail, translation);
		RenderDevice->SetTexture(0, trsprdata[avail]);
		RenderDevice->SetTextureStageState(0, D3DTSS_ADDRESSU, D3DTADDRESS_CLAMP);
		RenderDevice->SetTextureStageState(0, D3DTSS_ADDRESSV, D3DTADDRESS_CLAMP);
		tex_iw = 1.0 / GTextureManager.Textures[lump]->GetWidth();
		tex_ih = 1.0 / GTextureManager.Textures[lump]->GetHeight();
	}
	else
	{
		if (!GTextureManager.Textures[lump]->DriverData)
		{
			GenerateTexture(lump);
		}
#if DIRECT3D_VERSION >= 0x0800
		RenderDevice->SetTexture(0, (LPDIRECT3DTEXTURE8)GTextureManager.Textures[lump]->DriverData);
#else
		RenderDevice->SetTexture(0, (LPDIRECTDRAWSURFACE7)GTextureManager.Textures[lump]->DriverData);
#endif
		RenderDevice->SetTextureStageState(0, D3DTSS_ADDRESSU, D3DTADDRESS_CLAMP);
		RenderDevice->SetTextureStageState(0, D3DTSS_ADDRESSV, D3DTADDRESS_CLAMP);
		tex_iw = 1.0 / GTextureManager.Textures[lump]->GetWidth();
		tex_ih = 1.0 / GTextureManager.Textures[lump]->GetHeight();
	}
	unguard;
}

//==========================================================================
//
//	VDirect3DDrawer::SetPic
//
//==========================================================================

void VDirect3DDrawer::SetPic(int handle)
{
	guard(VDirect3DDrawer::SetPic);
	if (!RenderDevice)
	{
		return;
	}

	handle = GTextureManager.TextureAnimation(handle);

	if (!GTextureManager.Textures[handle]->DriverData)
	{
		GenerateTexture(handle);
	}

#if DIRECT3D_VERSION >= 0x0800
	RenderDevice->SetTexture(TexStage, (LPDIRECT3DTEXTURE8)GTextureManager.Textures[handle]->DriverData);
#else
	RenderDevice->SetTexture(TexStage, (LPDIRECTDRAWSURFACE7)GTextureManager.Textures[handle]->DriverData);
#endif
	RenderDevice->SetTextureStageState(TexStage, D3DTSS_ADDRESSU, D3DTADDRESS_CLAMP);
	RenderDevice->SetTextureStageState(TexStage, D3DTSS_ADDRESSV, D3DTADDRESS_CLAMP);
	tex_iw = 1.0 / GTextureManager.Textures[handle]->GetWidth();
	tex_ih = 1.0 / GTextureManager.Textures[handle]->GetHeight();
	unguard;
}

//==========================================================================
//
//	VDirect3DDrawer::GenerateTexture
//
//==========================================================================

void VDirect3DDrawer::GenerateTexture(int texnum)
{
	guard(VDirect3DDrawer::GenerateTexture);
	VTexture* Tex = GTextureManager.Textures[texnum];

	VTexture* SrcTex = Tex->GetHighResolutionTexture();
	if (!SrcTex)
	{
		SrcTex = Tex;
	}

	byte* block = SrcTex->GetPixels();
	if (SrcTex->Format == TEXFMT_8 || SrcTex->Format == TEXFMT_8Pal)
	{
		Tex->DriverData = UploadTexture8(SrcTex->GetWidth(),
			SrcTex->GetHeight(), block, SrcTex->GetPalette());
	}
	else
	{
		Tex->DriverData = UploadTexture(SrcTex->GetWidth(),
			SrcTex->GetHeight(), (rgba_t*)block);
	}
	SrcTex->Unload();
	unguard;
}

//==========================================================================
//
//	VDirect3DDrawer::GenerateTranslatedSprite
//
//==========================================================================

void VDirect3DDrawer::GenerateTranslatedSprite(int lump, int slot,
	int translation)
{
	guard(VDirect3DDrawer::GenerateTranslatedSprite);
	VTexture* Tex = GTextureManager.Textures[lump];

	trsprlump[slot] = lump;
	trsprtnum[slot] = translation;

	byte* Pixels = Tex->GetPixels8();
	byte* block = (byte*)Z_Malloc(Tex->GetWidth() * Tex->GetHeight());
	byte *trtab = translationtables + translation * 256;
	for (int i = 0; i < Tex->GetWidth() * Tex->GetHeight(); i++)
	{
		block[i] = trtab[Pixels[i]];
	}

	trsprdata[slot] = UploadTexture8(Tex->GetWidth(), Tex->GetHeight(),
		block, Tex->GetPalette());
	Z_Free(block);
	Tex->Unload();
	unguard;
}

//==========================================================================
//
//	VDirect3DDrawer::UploadTextureImage
//
//==========================================================================

#if DIRECT3D_VERSION >= 0x0800
void VDirect3DDrawer::UploadTextureImage(LPDIRECT3DTEXTURE8 tex, int level,
	int width, int height, rgba_t *data)
{
	guard(VDirect3DDrawer::UploadTextureImage);
	LPDIRECT3DSURFACE8 surf;
	tex->GetSurfaceLevel(level, &surf);

	D3DLOCKED_RECT lrect;
	if (FAILED(surf->LockRect(&lrect, NULL, 0)))
	{
		GCon->Logf(NAME_Dev, "Failed to lock surface");
		return;
	}

	D3DSURFACE_DESC desc;
	surf->GetDesc(&desc);

	rgba_t *in = data;
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
		dword *out = (dword*)lrect.pBits;
		for (int i = 0; i < width * height; i++, in++, out++)
		{
			*out = MakeCol32(in->r, in->g, in->b, in->a);
		}
	}
	surf->UnlockRect();
	surf->Release();
	unguard;
}
#else
void VDirect3DDrawer::UploadTextureImage(LPDIRECTDRAWSURFACE7 surf,
	int width, int height, rgba_t *data)
{
	guard(VDirect3DDrawer::UploadTextureImage);
	DDSURFACEDESC2 ddsd;
	memset(&ddsd, 0, sizeof(ddsd));
	ddsd.dwSize = sizeof(ddsd);
	ddsd.dwFlags = DDSD_WIDTH | DDSD_HEIGHT | DDSD_PIXELFORMAT | DDSD_LPSURFACE;
	if (FAILED(surf->Lock(NULL, &ddsd, DDLOCK_WAIT, NULL)))
	{
		GCon->Logf(NAME_Dev, "Failed to lock surface");
		return;
	}
	rgba_t *in = data;
	if (ddsd.ddpfPixelFormat.dwRGBBitCount == 16)
	{
		for (int y = 0; y < height; y++)
		{
			word *out = (word*)((byte *)ddsd.lpSurface + y * ddsd.lPitch);
			for (int x = 0; x < width; x++, in++, out++)
			{
				*out = MakeCol16(in->r, in->g, in->b, in->a);
			}
		}
	}
	else if (ddsd.ddpfPixelFormat.dwRGBBitCount == 32)
	{
		for (int y = 0; y < height; y++)
		{
			dword *out = (dword*)((byte *)ddsd.lpSurface + y * ddsd.lPitch);
			for (int x = 0; x < width; x++, in++, out++)
			{
				*out = MakeCol32(in->r, in->g, in->b, in->a);
			}
		}
	}
	surf->Unlock(NULL);
	unguard;
}
#endif

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

#if DIRECT3D_VERSION >= 0x0800
LPDIRECT3DTEXTURE8 VDirect3DDrawer::UploadTexture8(int Width, int Height,
	byte* Data, rgba_t* Pal)
#else
LPDIRECTDRAWSURFACE7 VDirect3DDrawer::UploadTexture8(int Width, int Height,
	byte* Data, rgba_t* Pal)
#endif
{
	rgba_t* NewData = (rgba_t*)Z_Calloc(Width * Height * 4);
	for (int i = 0; i < Width * Height; i++)
	{
		if (Data[i])
			NewData[i] = Pal[Data[i]];
	}
#if DIRECT3D_VERSION >= 0x0800
	LPDIRECT3DTEXTURE8 Ret;
#else
	LPDIRECTDRAWSURFACE7 Ret;
#endif
	Ret = UploadTexture(Width, Height, NewData);
	Z_Free(NewData);
	return Ret;
}

//==========================================================================
//
//	VDirect3DDrawer::UploadTexture
//
//==========================================================================

#if DIRECT3D_VERSION >= 0x0800
LPDIRECT3DTEXTURE8 VDirect3DDrawer::UploadTexture(int width, int height, rgba_t *data)
#else
LPDIRECTDRAWSURFACE7 VDirect3DDrawer::UploadTexture(int width, int height, rgba_t *data)
#endif
{
	guard(VDirect3DDrawer::UploadTexture);
	int						w, h;
	byte					*image;
	byte					stackbuf[256 * 128 * 4];
#if DIRECT3D_VERSION >= 0x0800
	LPDIRECT3DTEXTURE8		surf;
	UINT					level;
#else
	LPDIRECTDRAWSURFACE7	surf;
	LPDIRECTDRAWSURFACE7	mipsurf;
	DDSCAPS2				ddsc;
	HRESULT					ddres;
#endif

	AdjustGamma(data, width * height);

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
	surf = CreateSurface(w, h, 16, true);
#if DIRECT3D_VERSION >= 0x0800
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
#else
	UploadTextureImage(surf, w, h, (rgba_t*)image);

	mipsurf = NULL;
	memset(&ddsc, 0, sizeof(ddsc));
	ddsc.dwCaps = DDSCAPS_TEXTURE | DDSCAPS_MIPMAP;
	while (w > 1 && h > 1)
	{
		MipMap(w, h, image);
		if (w > 1)
			w >>= 1;
		if (h > 1)
			h >>= 1;
		if (mipsurf)
		{
			LPDIRECTDRAWSURFACE7 prevsurf = mipsurf;
			ddres = prevsurf->GetAttachedSurface(&ddsc, &mipsurf);
			prevsurf->Release();
		}
		else
		{
			ddres = surf->GetAttachedSurface(&ddsc, &mipsurf);
		}
		if (ddres != DD_OK)
		{
			Sys_Error("Failed to get attached surface");
		}
		UploadTextureImage(mipsurf, w, h, (rgba_t*)image);
	}
#endif

	if (image != stackbuf)
	{
		Z_Free(image);
	}
	return surf;
	unguard;
}
