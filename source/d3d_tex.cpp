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
// 	VDirect3DDrawer::PrecacheTexture
//
//==========================================================================

void VDirect3DDrawer::PrecacheTexture(VTexture* Tex)
{
	guard(VDirect3DDrawer::PrecacheTexture);
	SetTexture(Tex, 0);
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
	if (RenderDevice)
	{
		RenderDevice->SetSamplerState(0, D3DSAMP_ADDRESSU, D3DTADDRESS_WRAP);
		RenderDevice->SetSamplerState(0, D3DSAMP_ADDRESSV, D3DTADDRESS_WRAP);
	}
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
		vuint8* block = SrcTex->GetPixels();
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
//	VDirect3DDrawer::UploadTexture8
//
//==========================================================================

LPDIRECT3DTEXTURE9 VDirect3DDrawer::UploadTexture8(int Width, int Height,
	const vuint8* Data, const rgba_t* Pal)
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
	vuint8					*image;
	vuint8					stackbuf[256 * 128 * 4];
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
		image = (vuint8*)Z_Malloc(w * h * 4);
	}
	if (w != width || h != height)
	{
		//	Must rescale image to get "top" mipmap texture image
		ResampleTexture(width, height, (vuint8*)data, w, h, image);
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
