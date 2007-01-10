//**************************************************************************
//**
//**	##   ##    ##    ##   ##   ####     ####   ###     ###
//**	##   ##  ##  ##  ##   ##  ##  ##   ##  ##  ####   ####
//**	 ## ##  ##    ##  ## ##  ##    ## ##    ## ## ## ## ##
//**	 ## ##  ########  ## ##  ##    ## ##    ## ##  ###  ##
//**	  ###   ##    ##   ###    ##  ##   ##  ##  ##       ##
//**	   #    ##    ##    #      ####     ####   ##       ##
//**
//**	$Id: template.cpp 1583 2006-06-27 19:05:42Z dj_jl $
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

#include "gamedefs.h"
#include "r_local.h"

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
//	VMultiPatchTexture::VMultiPatchTexture
//
//==========================================================================

VMultiPatchTexture::VMultiPatchTexture(VStream& Strm, int DirectoryIndex,
	VTexture** PatchLookup, int NumPatchLookup, int FirstTex, bool IsStrife)
: Pixels(0)
{
	guard(VMultiPatchTexture::VMultiPatchTexture);
	Type = TEXTYPE_Wall;
	Format = TEXFMT_8;

	//	Read offset and seek to the starting position.
	Strm.Seek(4 + DirectoryIndex * 4);
	vint32 Offset = Streamer<vint32>(Strm);
	if (Offset < 0 || Offset >= Strm.TotalSize())
	{
		Sys_Error("InitTextures: bad texture directory");
	}
	Strm.Seek(Offset);

	//	Read name.
	char TmpName[12];
	Strm.Serialise(TmpName, 8);
	TmpName[8] = 0;
	Name = VName(TmpName, VName::AddLower8);

	//	In Doom textures were searched from the beginning, so to avoid
	// problems, especially with animated textures, set name to a blank one
	// if this one is a duplicate.
	if (GTextureManager.CheckNumForName(Name, TEXTYPE_Wall, false, false) >= FirstTex)
	{
		Name = NAME_None;
	}

	//	Skip unused value.
	Streamer<vint16>(Strm);	//	Masked, unused.

	//	Read scaling.
	vuint8 TmpSScale = Streamer<vuint8>(Strm);
	vuint8 TmpTScale = Streamer<vuint8>(Strm);
	SScale = TmpSScale ? TmpSScale / 8.0 : 1.0;
	TScale = TmpTScale ? TmpTScale / 8.0 : 1.0;

	//	Read dimensions.
	Width = Streamer<vint16>(Strm);
	Height = Streamer<vint16>(Strm);

	//	Skip unused value.
	if (!IsStrife)
		Streamer<vint32>(Strm);	//	ColumnDirectory, unused.

	//	Create list of patches.
	PatchCount = Streamer<vint16>(Strm);
	Patches = new VTexPatch[PatchCount];
	memset(Patches, 0, sizeof(VTexPatch) * PatchCount);

	//	Read patches.
	VTexPatch* patch = Patches;
	for (int i = 0; i < PatchCount; i++, patch++)
	{
		//	Read origin.
		patch->XOrigin = Streamer<vint16>(Strm);
		patch->YOrigin = Streamer<vint16>(Strm);

		//	Read patch index and find patch texture.
		vint16 PatchIdx = Streamer<vint16>(Strm);
		if (PatchIdx < 0 || PatchIdx >= NumPatchLookup)
		{
			Sys_Error("InitTextures: Bad patch index in texture %s", *Name);
		}
		patch->Tex = PatchLookup[PatchIdx];
		if (!patch->Tex)
		{
			Sys_Error("InitTextures: Missing patch in texture %s", *Name);
		}

		//	Skip unused values.
		if (!IsStrife)
		{
			Streamer<vint16>(Strm);	//	Step dir, unused.
			Streamer<vint16>(Strm);	//	Colour map, unused.
		}
	}

	//	Fix sky texture heights for Heretic, but it can also be used
	// for Doom and Strife
	if (!VStr::NICmp(*Name, "sky", 3) && Height == 128)
	{
		if (Patches[0].Tex->GetHeight() > Height)
		{
			Height = Patches[0].Tex->GetHeight();
		}
	}
	unguard;
}

//==========================================================================
//
//	VMultiPatchTexture::~VMultiPatchTexture
//
//==========================================================================

VMultiPatchTexture::~VMultiPatchTexture()
{
	guard(VMultiPatchTexture::~VMultiPatchTexture);
	if (Patches)
	{
		delete[] Patches;
	}
	if (Pixels)
	{
		delete[] Pixels;
	}
	unguard;
}

//==========================================================================
//
//	VMultiPatchTexture::SetFrontSkyLayer
//
//==========================================================================

void VMultiPatchTexture::SetFrontSkyLayer()
{
	guard(VMultiPatchTexture::SetFrontSkyLayer);
	for (int i = 0; i < PatchCount; i++)
	{
		Patches[i].Tex->SetFrontSkyLayer();
	}
	bNoRemap0 = true;
	unguard;
}

//==========================================================================
//
//	VMultiPatchTexture::GetPixels
//
// 	Using the texture definition, the composite texture is created from the
// patches, and each column is cached.
//
//==========================================================================

vuint8* VMultiPatchTexture::GetPixels()
{
	guard(VMultiPatchTexture::GetPixels);
	//	If already got pixels, then just return them.
	if (Pixels)
	{
		return Pixels;
	}

	//	Load all patches, if any of them is not in standard palette, then
	// switch to 32 bit mode.
	Format = TEXFMT_8;
	for (int i = 0; i < PatchCount; i++)
	{
		Patches[i].Tex->GetPixels();
		if (Patches[i].Tex->Format != TEXFMT_8)
		{
			Format = TEXFMT_RGBA;
		}
	}

	if (Format == TEXFMT_8)
	{
		Pixels = new vuint8[Width * Height];
		memset(Pixels, 0, Width * Height);

		// Composite the columns together.
		VTexPatch* patch = Patches;
		for (int i = 0; i < PatchCount; i++, patch++)
		{
			VTexture* PatchTex = patch->Tex;
			vuint8* PatchPixels = PatchTex->GetPixels();
			int x1 = patch->XOrigin;
			int x2 = x1 + PatchTex->GetWidth();
			if (x2 > Width)
				x2 = Width;
			int y1 = patch->YOrigin;
			int y2 = y1 + PatchTex->GetHeight();
			if (y2 > Height)
				y2 = Height;
			for (int y = y1 < 0 ? 0 : y1; y < y2; y++)
			{
				for (int x = x1 < 0 ? 0 : x1; x < x2; x++)
				{
					int PIdx = (x - x1) + (y - y1) * PatchTex->GetWidth();
					if (PatchPixels[PIdx])
						Pixels[x + y * Width] = PatchPixels[PIdx];
				}
			}
		}
	}
	else
	{
		Pixels = new vuint8[Width * Height * 4];
		memset(Pixels, 0, Width * Height * 4);

		// Composite the columns together.
		VTexPatch* patch = Patches;
		for (int i = 0; i < PatchCount; i++, patch++)
		{
			VTexture* PatchTex = patch->Tex;
			vuint8* PatchPixels = PatchTex->GetPixels();
			int x1 = patch->XOrigin;
			int x2 = x1 + PatchTex->GetWidth();
			if (x2 > Width)
				x2 = Width;
			int y1 = patch->YOrigin;
			int y2 = y1 + PatchTex->GetHeight();
			if (y2 > Height)
				y2 = Height;
			for (int y = y1 < 0 ? 0 : y1; y < y2; y++)
			{
				for (int x = x1 < 0 ? 0 : x1; x < x2; x++)
				{
					//	Get pixel.
					int PIdx = (x - x1) + (y - y1) * PatchTex->GetWidth();
					rgba_t col;
					switch (PatchTex->Format)
					{
					case TEXFMT_8:
						col = r_palette[PatchPixels[PIdx]];
						break;
					case TEXFMT_8Pal:
						col = PatchTex->GetPalette()[PatchPixels[PIdx]];
						break;
					case TEXFMT_RGBA:
						col = ((rgba_t*)PatchPixels)[PIdx];
						break;
					default:
						//	Shut up compiler
						col.r = 0;
						col.g = 0;
						col.b = 0;
						col.a = 0;
						break;
					}

					//	Add to texture.
					if (col.a == 255)
					{
						((rgba_t*)Pixels)[x + y * Width] = col;
					}
					else if (col.a)
					{
						rgba_t& Dst = ((rgba_t*)Pixels)[x + y * Width];
						float a = col.a / 255.0;
						float ia = (Dst.a / 255.0) * (255.0 - col.a) / 255.0;
						Dst.r = vuint8(Dst.r * ia + col.r * a);
						Dst.g = vuint8(Dst.g * ia + col.g * a);
						Dst.b = vuint8(Dst.b * ia + col.b * a);
						Dst.a = Dst.a + col.a > 255 ? 255 : Dst.a + col.a;
					}
				}
			}
		}
	}

	return Pixels;
	unguard;
}

//==========================================================================
//
//	VMultiPatchTexture::Unload
//
//==========================================================================

void VMultiPatchTexture::Unload()
{
	guard(VMultiPatchTexture::Unload);
	if (Pixels)
	{
		delete[] Pixels;
		Pixels = NULL;
	}
	unguard;
}
