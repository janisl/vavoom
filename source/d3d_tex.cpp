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

void VDirect3DDrawer::InitTextures(void)
{
	guard(VDirect3DDrawer::InitTextures);
	numsurfaces = numtextures + numflats + numskymaps + numspritelumps +
		MAX_TRANSLATED_SPRITES + MAX_PICS + MAX_SKIN_CACHE;
#if DIRECT3D_VERSION >= 0x0800
	texturesurfaces = (LPDIRECT3DTEXTURE8*)Z_Calloc(numsurfaces * 4);
#else
	texturesurfaces = (LPDIRECTDRAWSURFACE7*)Z_Calloc(numsurfaces * 4);
#endif
	//	Textures
	texturedata = texturesurfaces;
	//	Flats
	flatdata = texturedata + numtextures;
	//  Sky maps
	skymapdata = flatdata + numflats;
	//	Sprite lumps
	spritedata = skymapdata + numskymaps;
	trsprdata = spritedata + numspritelumps;
	//	2D graphics
	picdata = trsprdata + MAX_TRANSLATED_SPRITES;
	//	Skins
	skin_data = picdata + MAX_PICS;

	//	Lightmaps, seperate from other surfaces so CreateSurface doesn't
	// release them
#if DIRECT3D_VERSION >= 0x0800
	light_surf = (LPDIRECT3DTEXTURE8*)Z_Calloc(NUM_BLOCK_SURFS * 4);
	add_surf = (LPDIRECT3DTEXTURE8*)Z_Calloc(NUM_BLOCK_SURFS * 4);
#else
	light_surf = (LPDIRECTDRAWSURFACE7*)Z_Calloc(NUM_BLOCK_SURFS * 4);
	add_surf = (LPDIRECTDRAWSURFACE7*)Z_Calloc(NUM_BLOCK_SURFS * 4);
#endif

	textureiw = (float*)Z_Calloc(numtextures * 4);
	textureih = (float*)Z_Calloc(numtextures * 4);
	spriteiw = (float*)Z_Calloc(numspritelumps * 4);
	spriteih = (float*)Z_Calloc(numspritelumps * 4);
	unguard;
}

//==========================================================================
//
//	VDirect3DDrawer::FlushTextures
//
//==========================================================================

void VDirect3DDrawer::FlushTextures(void)
{
	guard(VDirect3DDrawer::FlushTextures);
	for (int i = 0; i < numsurfaces; i++)
	{
		SAFE_RELEASE(texturesurfaces[i]);
	}
	unguard;
}

//==========================================================================
//
//	VDirect3DDrawer::ReleaseTextures
//
//==========================================================================

void VDirect3DDrawer::ReleaseTextures(void)
{
	guard(VDirect3DDrawer::ReleaseTextures);
	int i;
	for (i = 0; i < numsurfaces; i++)
	{
		SAFE_RELEASE(texturesurfaces[i]);
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
//	VDirect3DDrawer::DrawColumnInCache
//
// 	Clip and draw a column from a patch into a flat buffer.
//
//		column - column to draw
//		cache - buffer
//		originx, originy - position of column in the buffer
//		cachewidth, cacheheight - size of the cache
//
//==========================================================================

void VDirect3DDrawer::DrawColumnInCache(column_t* column, rgba_t* cache,
	int originx, int originy, int cachewidth, int cacheheight, bool dsky)
{
	guard(VDirect3DDrawer::DrawColumnInCache);
    int		count;
    int		position;
	byte*	source;
	rgba_t*	dest;
	int		top = -1;	//	DeepSea tall patches support

	// step through the posts in a column
    while (column->topdelta != 0xff)
    {
		if (column->topdelta <= top)
		{
			top += column->topdelta;
		}
		else
		{
			top = column->topdelta;
		}
		source = (byte *)column + 3;
		count = column->length;
		position = originy + top;

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
			*dest = r_palette[0][*source || dsky ? *source : r_black_color[0]];
			source++;
			dest += cachewidth;
    	}
		
		column = (column_t *)((byte *)column + column->length + 4);
    }
	unguard;
}

//==========================================================================
//
//	VDirect3DDrawer::GenerateTexture
//
// 	Using the texture definition, the composite texture is created from the
// patches, and each column is cached.
//
//==========================================================================

void VDirect3DDrawer::GenerateTexture(int texnum, bool dsky)
{
	guard(VDirect3DDrawer::GenerateTexture);
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

	char HighResName[80];
	sprintf(HighResName, "textures/walls/%s.png", texture->name);
	for (i = 0; HighResName[i]; i++)
		HighResName[i] = tolower(HighResName[i]);
	if (FL_FindFile(HighResName, NULL))
	{
		Mod_LoadSkin(HighResName, 0);
		if (SkinBPP == 8)
		{
			rgba_t *buf = (rgba_t*)Z_Malloc(SkinWidth * SkinHeight * 4);
			for (int i = 0; i < SkinWidth * SkinHeight; i++)
			{
				buf[i] = SkinPal[SkinData[i]];
			}
			texturedata[texnum] = UploadTexture(SkinWidth, SkinHeight, buf);
			Z_Free(buf);
		}
		else
		{
			texturedata[texnum] = UploadTexture(SkinWidth, SkinHeight, (rgba_t *)SkinData);
		}
		Z_Free(SkinData);
		textureiw[texnum] = 1.0 / float(texture->width);
		textureih[texnum] = 1.0 / float(texture->height);
		return;
	}

	block = (rgba_t*)Z_Calloc(texture->width * texture->height * 4);

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
	    		texture->width, texture->height, dsky);
		}
    }

	textureiw[texnum] = 1.0 / (float)texture->width;
	textureih[texnum] = 1.0 / (float)texture->height;
	texturedata[texnum] = UploadTexture(texture->width, texture->height, block);
	Z_Free(block);
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
	{
		GenerateTexture(tex, false);
	}

	RenderDevice->SetTexture(0, texturedata[tex]);
	RenderDevice->SetTextureStageState(0, D3DTSS_ADDRESSU, D3DTADDRESS_WRAP);
	RenderDevice->SetTextureStageState(0, D3DTSS_ADDRESSV, D3DTADDRESS_WRAP);
	tex_iw = textureiw[tex];
	tex_ih = textureih[tex];
	unguard;
}

//==========================================================================
//
// 	VDirect3DDrawer::SetSkyTexture
//
//==========================================================================

void VDirect3DDrawer::SetSkyTexture(int tex, bool double_sky)
{
	guard(VDirect3DDrawer::SetSkyTexture);
	if (!RenderDevice)
	{
		return;
	}

	if (tex & TEXF_SKY_MAP)
	{
		tex &= ~TEXF_SKY_MAP;
		if (!skymapdata[tex])
		{
			Mod_LoadSkin(skymaps[tex].name, 0);
			if (SkinBPP == 8)
			{
				rgba_t *buf = (rgba_t*)Z_Malloc(SkinWidth * SkinHeight * 4);
				byte *src = SkinData;
				rgba_t *dst = buf;
				for (int x = 0; x < SkinWidth * SkinHeight; x++, src++, dst++)
				{
					*dst = SkinPal[*src];
				}
				skymapdata[tex] = UploadTexture(SkinWidth, SkinHeight, buf);
				Z_Free(buf);
			}
			else
			{
				skymapdata[tex] = UploadTexture(SkinWidth, SkinHeight,
					(rgba_t *)SkinData);
			}
			Z_Free(SkinData);
			skymaps[tex].width = SkinWidth;
			skymaps[tex].height = SkinHeight;
		}
		RenderDevice->SetTexture(0, skymapdata[tex]);
		RenderDevice->SetTextureStageState(0, D3DTSS_ADDRESSU, D3DTADDRESS_CLAMP);
		RenderDevice->SetTextureStageState(0, D3DTSS_ADDRESSV, D3DTADDRESS_CLAMP);
		tex_iw = 1.0 / skymaps[tex].width;
		tex_ih = 1.0 / skymaps[tex].height;
	}
	else
	{
		tex = R_TextureAnimation(tex);

		if (!texturedata[tex])
		{
			GenerateTexture(tex, double_sky);
		}

		int tstage = maxMultiTex >= 2 && double_sky ? 1 : 0;
		RenderDevice->SetTexture(tstage, texturedata[tex]);
		RenderDevice->SetTextureStageState(tstage, D3DTSS_ADDRESSU, D3DTADDRESS_WRAP);
		RenderDevice->SetTextureStageState(tstage, D3DTSS_ADDRESSV, D3DTADDRESS_WRAP);
		tex_iw = textureiw[tex];
		tex_ih = textureih[tex];
	}
	unguard;
}

//==========================================================================
//
//	VDirect3DDrawer::GenerateFlat
//
//==========================================================================

void VDirect3DDrawer::GenerateFlat(int num)
{
	guard(VDirect3DDrawer::GenerateFlat);
	char HighResName[80];
	sprintf(HighResName, "textures/flats/%s.png", W_LumpName(flatlumps[num]));
	for (int j = 0; HighResName[j]; j++)
		HighResName[j] = tolower(HighResName[j]);
	if (FL_FindFile(HighResName, NULL))
	{
		Mod_LoadSkin(HighResName, 0);
		if (SkinBPP == 8)
		{
			rgba_t *buf = (rgba_t*)Z_Malloc(SkinWidth * SkinHeight * 4);
			for (int i = 0; i < SkinWidth * SkinHeight; i++)
			{
				buf[i] = SkinPal[SkinData[i]];
			}
			flatdata[num] = UploadTexture(SkinWidth, SkinHeight, buf);
			Z_Free(buf);
		}
		else
		{
			flatdata[num] = UploadTexture(SkinWidth, SkinHeight, (rgba_t *)SkinData);
		}
		Z_Free(SkinData);
		return;
	}

	rgba_t *block = (rgba_t*)Z_Malloc(64 * 64 * 4, PU_HIGH, 0);
	byte *data = (byte*)W_CacheLumpNum(flatlumps[num], PU_CACHE);

	byte *src = data;
	rgba_t *dst = block;
	for (int i = 0; i < 64 * 64; i++, src++)
	{
		*dst++ = r_palette[0][*src ? *src : r_black_color[0]];
	}

	flatdata[num] = UploadTexture(64, 64, block);
	Z_Free(block);
	unguard;
}

//==========================================================================
//
//	VDirect3DDrawer::SetFlat
//
//==========================================================================

void VDirect3DDrawer::SetFlat(int num)
{
	guard(VDirect3DDrawer::SetFlat);
	if (!RenderDevice)
		return;

	num = R_TextureAnimation(num);
	num &= ~TEXF_FLAT;

	if (!flatdata[num])
	{
		GenerateFlat(num);
	}
    RenderDevice->SetTexture(0, flatdata[num]);
	RenderDevice->SetTextureStageState(0, D3DTSS_ADDRESSU, D3DTADDRESS_WRAP);
	RenderDevice->SetTextureStageState(0, D3DTSS_ADDRESSV, D3DTADDRESS_WRAP);
	tex_iw = 1.0 / 64.0;
	tex_ih = 1.0 / 64.0;
	unguard;
}

//==========================================================================
//
//	VDirect3DDrawer::GenerateSprite
//
//==========================================================================

void VDirect3DDrawer::GenerateSprite(int lump)
{
	guard(VDirect3DDrawer::GenerateSprite);
    patch_t	*patch = (patch_t*)W_CacheLumpNum(spritelumps[lump], PU_STATIC);

	int w = LittleShort(patch->width);
	int h = LittleShort(patch->height);
	spriteiw[lump] = 1.0 / (float)w;
	spriteih[lump] = 1.0 / (float)h;

	rgba_t *block = (rgba_t*)Z_Calloc(w * h * 4, PU_HIGH, 0);

	for (int x = 0; x < w; x++)
	{
    	column_t *column = (column_t *)((byte *)patch +
    		LittleLong(patch->columnofs[x]));

		// step through the posts in a column
		int top = -1;	//	DeepSea tall patches support
	    while (column->topdelta != 0xff)
	    {
			if (column->topdelta <= top)
			{
				top += column->topdelta;
			}
			else
			{
				top = column->topdelta;
			}
		    byte* source = (byte *)column + 3;
		    rgba_t* dest = block + x + top * w;
			int count = column->length;

	    	while (count--)
	    	{
				*dest = r_palette[0][*source ? *source : r_black_color[0]];
				source++;
				dest += w;
	    	}
			column = (column_t *)((byte *)column + column->length + 4);
	    }
	}

	spritedata[lump] = UploadTexture(w, h, block);
	Z_Free(block);
	Z_ChangeTag(patch, PU_CACHE);
	unguard;
}

//==========================================================================
//
//	VDirect3DDrawer::GenerateTranslatedSprite
//
//==========================================================================

void VDirect3DDrawer::GenerateTranslatedSprite(int lump, int slot, int translation)
{
	guard(VDirect3DDrawer::GenerateTranslatedSprite);
    patch_t	*patch = (patch_t*)W_CacheLumpNum(spritelumps[lump], PU_STATIC);

	int w = LittleShort(patch->width);
	int h = LittleShort(patch->height);
	trspriw[slot] = 1.0 / (float)w;
	trsprih[slot] = 1.0 / (float)h;

	rgba_t *block = (rgba_t*)Z_Calloc(w * h * 4, PU_HIGH, 0);
	trsprlump[slot] = lump;
	trsprtnum[slot] = translation;

	byte *trtab = translationtables + translation * 256;

	for (int x = 0; x < w; x++)
	{
    	column_t *column = (column_t *)((byte *)patch +
    		LittleLong(patch->columnofs[x]));

		// step through the posts in a column
		int top = -1;	//	DeepSea tall patches support
	    while (column->topdelta != 0xff)
	    {
			if (column->topdelta <= top)
			{
				top += column->topdelta;
			}
			else
			{
				top = column->topdelta;
			}
		    byte* source = (byte *)column + 3;
		    rgba_t* dest = block + x + top * w;
			int count = column->length;

	    	while (count--)
	    	{
				int col = trtab[*source];
				*dest = r_palette[0][col ? col : r_black_color[0]];
				source++;
				dest += w;
	    	}
			column = (column_t *)((byte *)column + column->length + 4);
	    }
	}

	trsprdata[slot] = UploadTexture(w, h, block);
	Z_Free(block);
	Z_ChangeTag(patch, PU_CACHE);
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
		RenderDevice->SetTextureStageState(0, D3DTSS_ADDRESSU, D3DTADDRESS_CLAMP);
		RenderDevice->SetTextureStageState(0, D3DTSS_ADDRESSV, D3DTADDRESS_CLAMP);
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
		RenderDevice->SetTextureStageState(0, D3DTSS_ADDRESSU, D3DTADDRESS_CLAMP);
		RenderDevice->SetTextureStageState(0, D3DTSS_ADDRESSV, D3DTADDRESS_CLAMP);
		tex_iw = spriteiw[lump];
		tex_ih = spriteih[lump];
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
	RenderDevice->SetTextureStageState(0, D3DTSS_ADDRESSU, D3DTADDRESS_CLAMP);
	RenderDevice->SetTextureStageState(0, D3DTSS_ADDRESSV, D3DTADDRESS_CLAMP);
	tex_iw = piciw[handle];
	tex_ih = picih[handle];
	unguard;
}

//==========================================================================
//
//	VDirect3DDrawer::GeneratePicFromPatch
//
//==========================================================================

void VDirect3DDrawer::GeneratePicFromPatch(int handle)
{
	guard(VDirect3DDrawer::GeneratePicFromPatch);
	int LumpNum = W_CheckNumForName(pic_list[handle].name);
	//	Some inventory pics are inside sprites.
	if (LumpNum < 0)
		LumpNum = W_GetNumForName(pic_list[handle].name, WADNS_Sprites);
	patch_t *patch = (patch_t*)W_CacheLumpNum(LumpNum, PU_STATIC);
	int w = LittleShort(patch->width);
	int h = LittleShort(patch->height);

	char HighResName[80];
	sprintf(HighResName, "textures/pics/%s.png", pic_list[handle].name);
	for (int i = 0; HighResName[i]; i++)
		HighResName[i] = tolower(HighResName[i]);
	if (FL_FindFile(HighResName, NULL))
	{
		Mod_LoadSkin(HighResName, 0);
		if (SkinBPP == 8)
		{
			rgba_t *buf = (rgba_t*)Z_Malloc(SkinWidth * SkinHeight * 4);
			for (int i = 0; i < SkinWidth * SkinHeight; i++)
			{
				buf[i] = SkinPal[SkinData[i]];
			}
			picdata[handle] = UploadTextureNoMip(SkinWidth, SkinHeight, buf);
			Z_Free(buf);
		}
		else
		{
			picdata[handle] = UploadTextureNoMip(SkinWidth, SkinHeight, (rgba_t *)SkinData);
		}
		Z_Free(SkinData);
		piciw[handle] = 1.0 / float(w);
		picih[handle] = 1.0 / float(h);
		Z_ChangeTag(patch, PU_CACHE);
		return;
	}

    rgba_t *block = (rgba_t*)Z_Calloc(w * h * 4, PU_HIGH, 0);
	rgba_t *pal = r_palette[pic_list[handle].palnum];
	int black = r_black_color[pic_list[handle].palnum];

	for (int x = 0; x < w; x++)
	{
    	column_t *column = (column_t *)((byte *)patch +
    		LittleLong(patch->columnofs[x]));

		// step through the posts in a column
		int top = -1;	//	DeepSea tall patches support
	    while (column->topdelta != 0xff)
	    {
			if (column->topdelta <= top)
			{
				top += column->topdelta;
			}
			else
			{
				top = column->topdelta;
			}
		    byte* source = (byte *)column + 3;
		    rgba_t* dest = block + x + top * w;
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

	picdata[handle] = UploadTextureNoMip(w, h, block);
	piciw[handle] = 1.0 / float(w);
	picih[handle] = 1.0 / float(h);
	Z_Free(block);
	Z_ChangeTag(patch, PU_CACHE);
	unguard;
}

//==========================================================================
//
//	VDirect3DDrawer::GeneratePicFromRaw
//
//==========================================================================

void VDirect3DDrawer::GeneratePicFromRaw(int handle)
{
	guard(VDirect3DDrawer::GeneratePicFromRaw);
	int lump = W_GetNumForName(pic_list[handle].name);
	int len = W_LumpLength(lump);
	int h = len / 320;
	int i;

	char HighResName[80];
	sprintf(HighResName, "textures/pics/%s.png", pic_list[handle].name);
	for (i = 0; HighResName[i]; i++)
		HighResName[i] = tolower(HighResName[i]);
	if (FL_FindFile(HighResName, NULL))
	{
		Mod_LoadSkin(HighResName, 0);
		if (SkinBPP == 8)
		{
			rgba_t *buf = (rgba_t*)Z_Malloc(SkinWidth * SkinHeight * 4);
			for (int i = 0; i < SkinWidth * SkinHeight; i++)
			{
				buf[i] = SkinPal[SkinData[i]];
			}
			picdata[handle] = UploadTextureNoMip(SkinWidth, SkinHeight, buf);
			Z_Free(buf);
		}
		else
		{
			picdata[handle] = UploadTextureNoMip(SkinWidth, SkinHeight, (rgba_t *)SkinData);
		}
		Z_Free(SkinData);
		piciw[handle] = 1.0 / 320.0;
		picih[handle] = 1.0 / float(h);
		return;
	}

	byte* raw = (byte*)W_CacheLumpNum(lump, PU_STATIC);
    rgba_t *block = (rgba_t*)Z_Calloc(len * 4, PU_HIGH, 0);
	rgba_t *pal = r_palette[pic_list[handle].palnum];
	int black = r_black_color[pic_list[handle].palnum];

	byte *src = raw;
	rgba_t *dst = block;
	for (i = 0; i < len; i++, src++, dst++)
	{
		*dst = pal[*src ? *src : black];
	}

	piciw[handle] = 1.0 / 320.0;
	picih[handle] = 1.0 / float(h);
	picdata[handle] = UploadTextureNoMip(320, h, block);
	Z_Free(block);
	Z_ChangeTag(raw, PU_CACHE);
	unguard;
}

//==========================================================================
//
//	VDirect3DDrawer::SetSkin
//
//==========================================================================

void VDirect3DDrawer::SetSkin(const char *name)
{
	guard(VDirect3DDrawer::SetSkin);
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
		if (SkinBPP == 8)
		{
			rgba_t *buf = (rgba_t*)Z_Malloc(SkinWidth * SkinHeight * 4, PU_HIGH, 0);
			byte *src = SkinData;
			rgba_t *dst = buf;
			for (int x = 0; x < SkinWidth * SkinHeight; x++, src++, dst++)
			{
				*dst = SkinPal[*src];
			}
			skin_data[avail] = UploadTexture(SkinWidth, SkinHeight, buf);
			Z_Free(buf);
		}
		else
		{
			skin_data[avail] = UploadTexture(SkinWidth, SkinHeight, (rgba_t *)SkinData);
		}
		Z_Free(SkinData);
	}

	RenderDevice->SetTexture(0, skin_data[avail]);
	RenderDevice->SetTextureStageState(0, D3DTSS_ADDRESSU, D3DTADDRESS_WRAP);
	RenderDevice->SetTextureStageState(0, D3DTSS_ADDRESSV, D3DTADDRESS_WRAP);
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
		image = (byte*)Z_Malloc(w * h * 4, PU_HIGH, 0);
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

//==========================================================================
//
//	VDirect3DDrawer::UploadTextureNoMip
//
//==========================================================================

#if DIRECT3D_VERSION >= 0x0800
LPDIRECT3DTEXTURE8 VDirect3DDrawer::UploadTextureNoMip(int width, int height, rgba_t *data)
#else
LPDIRECTDRAWSURFACE7 VDirect3DDrawer::UploadTextureNoMip(int width, int height, rgba_t *data)
#endif
{
	guard(VDirect3DDrawer::UploadTextureNoMip);
	int		w, h;
	byte	*image;
	byte	stackbuf[64 * 1024];
#if DIRECT3D_VERSION >= 0x0800
	LPDIRECT3DTEXTURE8		surf;
#else
	LPDIRECTDRAWSURFACE7	surf;
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

	surf = CreateSurface(w, h, 16, false);
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
#if DIRECT3D_VERSION >= 0x0800
		UploadTextureImage(surf, 0, w, h, (rgba_t*)image);
#else
		UploadTextureImage(surf, w, h, (rgba_t*)image);
#endif
		if (image != stackbuf)
		{
			Z_Free(image);
		}
	}
	else
	{
#if DIRECT3D_VERSION >= 0x0800
		UploadTextureImage(surf, 0, w, h, data);
#else
		UploadTextureImage(surf, w, h, data);
#endif
	}
	return surf;
	unguard;
}

//**************************************************************************
//
//	$Log$
//	Revision 1.25  2005/03/28 07:25:40  dj_jl
//	Changed location of hi-res 2D graphics.
//
//	Revision 1.24  2004/12/27 12:23:16  dj_jl
//	Multiple small changes for version 1.16
//	
//	Revision 1.23  2004/11/30 07:19:00  dj_jl
//	Support for high resolution textures.
//	
//	Revision 1.22  2004/11/23 12:43:10  dj_jl
//	Wad file lump namespaces.
//	
//	Revision 1.21  2002/07/13 07:38:00  dj_jl
//	Added drawers to the object tree.
//	
//	Revision 1.20  2002/03/28 17:55:08  dj_jl
//	Added wrapping/clamping.
//	
//	Revision 1.19  2002/03/20 19:09:53  dj_jl
//	DeepSea tall patches support.
//	
//	Revision 1.18  2002/01/15 18:30:43  dj_jl
//	Some fixes and improvements suggested by Malcolm Nixon
//	
//	Revision 1.17  2002/01/11 18:24:44  dj_jl
//	Added guard macros
//	
//	Revision 1.16  2002/01/07 12:16:41  dj_jl
//	Changed copyright year
//	
//	Revision 1.15  2001/11/09 14:18:40  dj_jl
//	Added specular highlights
//	
//	Revision 1.14  2001/11/02 18:35:54  dj_jl
//	Sky optimizations
//	
//	Revision 1.13  2001/10/27 07:45:01  dj_jl
//	Added gamma controls
//	
//	Revision 1.12  2001/10/18 17:36:31  dj_jl
//	A lots of changes for Alpha 2
//	
//	Revision 1.11  2001/10/04 17:22:05  dj_jl
//	My overloaded matrix, beautification
//	
//	Revision 1.10  2001/09/20 15:59:43  dj_jl
//	Fixed resampling when one dimansion doesn't change
//	
//	Revision 1.9  2001/09/14 16:48:22  dj_jl
//	Switched to DirectX 8
//	
//	Revision 1.8  2001/08/30 17:37:39  dj_jl
//	Using linear texture resampling
//	
//	Revision 1.7  2001/08/24 17:03:57  dj_jl
//	Added mipmapping, removed bumpmap test code
//	
//	Revision 1.6  2001/08/23 17:47:57  dj_jl
//	Started work on mipmapping
//	
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
