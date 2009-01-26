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

#ifndef _R_TEX_H
#define _R_TEX_H

#include "drawer.h"
#include "r_shared.h"

//
//	A dummy texture.
//
class VDummyTexture : public VTexture
{
public:
	VDummyTexture();
	vuint8* GetPixels();
	void Unload();
};

//
//	A standard Doom patch.
//
class VPatchTexture : public VTexture
{
private:
	int			LumpNum;
	vuint8*		Pixels;

public:
	static VTexture* Create(VStream&, int);

	VPatchTexture(int, int, int, int, int);
	~VPatchTexture();
	vuint8* GetPixels();
	void Unload();
};

//
//	A texture defined in TEXTURE1/TEXTURE2 lumps.
//	A maptexturedef_t describes a rectangular texture, which is composed of
// one or more mappatch_t structures that arrange graphic patches
//
class VMultiPatchTexture : public VTexture
{
private:
	struct VTexPatch
	{
		//	Block origin (allways UL), which has allready accounted for the
		// internal origin of the patch.
		short		XOrigin;
		short		YOrigin;
		VTexture*	Tex;
	};

	//	All the Patches[PatchCount] are drawn back to front into the cached
	// texture.
	int				PatchCount;
	VTexPatch*		Patches;
	vuint8*			Pixels;

public:
	VMultiPatchTexture(VStream&, int, VTexture**, int, int, bool);
	VMultiPatchTexture(VScriptParser*, int);
	~VMultiPatchTexture();
	void SetFrontSkyLayer();
	vuint8* GetPixels();
	void Unload();
};

//
//	A standard Doom flat.
//
class VFlatTexture : public VTexture
{
private:
	int			LumpNum;
	vuint8*		Pixels;

public:
	static VTexture* Create(VStream&, int);

	VFlatTexture(int);
	~VFlatTexture();
	vuint8* GetPixels();
	void Unload();
};

//
//	Raven's raw screens.
//
class VRawPicTexture : public VTexture
{
private:
	int			LumpNum;
	int			PalLumpNum;
	vuint8*		Pixels;
	rgba_t*		Palette;

public:
	static VTexture* Create(VStream&, int);

	VRawPicTexture(int, int);
	~VRawPicTexture();
	vuint8* GetPixels();
	rgba_t* GetPalette();
	void Unload();
};

//
//	Raven's automap background.
//
class VAutopageTexture : public VTexture
{
private:
	int			LumpNum;
	vuint8*		Pixels;

public:
	static VTexture* Create(VStream&, int);

	VAutopageTexture(int);
	~VAutopageTexture();
	vuint8* GetPixels();
	void Unload();
};

//
//	ZDoom's IMGZ grapnics.
// [RH] Just a format I invented to avoid WinTex's palette remapping
// when I wanted to insert some alpha maps.
//
class VImgzTexture : public VTexture
{
private:
	int			LumpNum;
	vuint8*		Pixels;

public:
	static VTexture* Create(VStream&, int);

	VImgzTexture(int, int, int, int, int);
	~VImgzTexture();
	vuint8* GetPixels();
	void Unload();
};

//
//	A PCX file.
//
class VPcxTexture : public VTexture
{
private:
	int			LumpNum;
	vuint8*		Pixels;
	rgba_t*		Palette;

public:
	static VTexture* Create(VStream&, int);

	VPcxTexture(int, struct pcx_t&);
	~VPcxTexture();
	vuint8* GetPixels();
	rgba_t* GetPalette();
	void Unload();
};

//
//	A TGA file.
//
class VTgaTexture : public VTexture
{
private:
	int			LumpNum;
	vuint8*		Pixels;
	rgba_t*		Palette;

public:
	static VTexture* Create(VStream&, int);

	VTgaTexture(int, struct tgaHeader_t&);
	~VTgaTexture();
	vuint8* GetPixels();
	rgba_t* GetPalette();
	void Unload();
};

//
//	A PNG file.
//
class VPngTexture : public VTexture
{
private:
	int			LumpNum;
	vuint8*		Pixels;

public:
	static VTexture* Create(VStream&, int);

	VPngTexture(int, int, int, int, int);
	~VPngTexture();
	vuint8* GetPixels();
	void Unload();
};

//
//	A JPEG file.
//
class VJpegTexture : public VTexture
{
public:
	int			LumpNum;
	vuint8*		Pixels;

	static VTexture* Create(VStream&, int);

	VJpegTexture(int, int, int);
	~VJpegTexture();
	vuint8* GetPixels();
	void Unload();
};

//
//	A texture that returns a wiggly version of another texture.
//
class VWarpTexture : public VTexture
{
protected:
	VTexture*	SrcTex;
	vuint8*		Pixels;
	float		GenTime;
	float		WarpXScale;
	float		WarpYScale;
	float*		XSin1;
	float*		XSin2;
	float*		YSin1;
	float*		YSin2;

public:
	VWarpTexture(VTexture*);
	~VWarpTexture();
	void SetFrontSkyLayer();
	bool CheckModified();
	vuint8* GetPixels();
	rgba_t* GetPalette();
	VTexture* GetHighResolutionTexture();
	void Unload();
};

//
//	Different style of warping.
//
class VWarp2Texture : public VWarpTexture
{
public:
	VWarp2Texture(VTexture*);
	vuint8* GetPixels();
};

extern VCvarI			r_hirestex;

#endif
