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

#ifndef _R_LOCAL_H
#define _R_LOCAL_H

// HEADER FILES ------------------------------------------------------------

#include "cl_local.h"
#include "r_shared.h"

// MACROS ------------------------------------------------------------------

// TYPES -------------------------------------------------------------------

struct segpart_t
{
	segpart_t		*next;
	texinfo_t		texinfo;
	surface_t		*surfs;
	float			frontTopDist;
	float			frontBotDist;
	float			backTopDist;
	float			backBotDist;
	float			textureoffset;
	float			rowoffset;
};

struct drawseg_t
{
	seg_t		*seg;
	drawseg_t	*next;

	segpart_t	*top;
	segpart_t	*mid;
	segpart_t	*bot;
	segpart_t	*topsky;
	segpart_t	*extra;
};

struct sec_surface_t
{
	sec_plane_t		*secplane;
	texinfo_t		texinfo;
	float			dist;
	surface_t		*surfs;
};

struct subregion_t
{
	sec_region_t	*secregion;
	subregion_t		*next;
	sec_plane_t		*floorplane;
	sec_plane_t		*ceilplane;
	sec_surface_t	*floor;
	sec_surface_t	*ceil;
	int				count;
	drawseg_t		*lines;
};

struct fakefloor_t
{
	sec_plane_t		floorplane;
	sec_plane_t		ceilplane;
	sec_params_t	params;
};

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

	VPngTexture(int, int, int);
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

// PUBLIC FUNCTION PROTOTYPES ----------------------------------------------

//
// R_Main
//

//
// R_BSP
//
void R_RenderWorld();
void R_ClipShutdown();

//
// R_Things
//
void R_FreeSpriteData();
void R_RenderMobjs();
void R_DrawPlayerSprites();
void R_DrawCroshair();
void R_DrawTranslucentPoly(surface_t*, TVec*, int, int, float, int, bool,
	vuint32, const TVec&, float, const TVec&, const TVec&, const TVec&);
void R_DrawTranslucentPolys();

//
// R_Sky
//
void R_InitSkyBoxes();
void R_InitSky();
void R_AnimateSky();
void R_DrawSky();
void R_FreeLevelSkyData();

//
//	R_Tex
//
void R_PrecacheLevel();

//
//	R_Surf
//
void R_UpdateWorld();

//
//	R_Light
//
void R_ClearLights();
void R_LightFace(surface_t *surf, subsector_t *leaf);
void R_PushDlights();
vuint32 R_LightPoint(const TVec &p);

//
//	r_model
//
void R_InitModels();
void R_FreeModels();
bool R_DrawAliasModel(const TVec&, const TAVec&, VState*, const char*,
	int, vuint32, float, bool, float);
bool R_DrawEntityModel(VEntity*, vuint32, float, float);
bool R_CheckAliasModelFrame(VEntity*, float);

// PUBLIC DATA DECLARATIONS ------------------------------------------------

//
// R_Main
//
extern int      		screenblocks;
extern int				r_visframecount;
extern subsector_t*		r_viewleaf;

extern byte				light_remap[256];
extern VCvarI			r_darken;

extern refdef_t			refdef;

extern VCvarI			old_aspect;
extern VCvarI			r_hirestex;

#endif
