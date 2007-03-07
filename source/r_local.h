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

class VRenderLevel : public VRenderLevelPublic
{
private:
	struct skysurface_t : surface_t
	{
		TVec			__verts[3];
	};

	struct sky_t
	{
		int 			texture1;
		int 			texture2;
		int 			baseTexture1;
		int 			baseTexture2;
		float			columnOffset1;
		float			columnOffset2;
		float			scrollDelta1;
		float			scrollDelta2;
		skysurface_t	surf;
		TPlane			plane;
		texinfo_t		texinfo;
	};

	enum
	{
		VDIVS			= 8,
		HDIVS			= 16
	};

	struct light_t
	{
		TVec			origin;
		float			radius;
		vuint32			colour;
		int				leafnum;
	};

	enum { MAX_DLIGHTS	= 32 };

	enum
	{
		MAX_PARTICLES			= 2048,	// default max # of particles at one
										//  time
		ABSOLUTE_MIN_PARTICLES	= 512,	// no fewer than this no matter what's
										//  on the command line
	};
	
	VLevel*			Level;

	//	Surf variables
	sec_plane_t		sky_plane;
	float			skyheight;

	int				c_subdivides;

	surface_t*		free_wsurfs;
	int				c_seg_div;
	void*			AllocatedWSurfBlocks;
	subregion_t*	AllocatedSubRegions;
	drawseg_t*		AllocatedDrawSegs;
	segpart_t*		AllocatedSegParts;

	//	Sky variables
	bool			LevelHasLightning;
	int				NextLightningFlash;
	int				LightningFlash;
	int*			LightningLightLevels;

	sky_t			sky[HDIVS * VDIVS];
	int				NumSkySurfs;
	bool			bIsSkyBox;

	//	Light variables
	TArray<light_t>	Lights;
	dlight_t		DLights[MAX_DLIGHTS];

	int				NumParticles;
	particle_t*		Particles;
	particle_t*		ActiveParticles;
	particle_t*		FreeParticles;

	//	World render variables
	VViewClipper	ViewClip;
	int				FrustumIndexes[4][6];
	bool			SkyIsVisible;

	//	Surf methods
	void SetupSky();
	void InitSurfs(surface_t*, texinfo_t*, TPlane*, subsector_t*);
	void FlushSurfCaches(surface_t*);
	surface_t* SubdivideFace(surface_t*, const TVec&, const TVec*);
	sec_surface_t* CreateSecSurface(subsector_t*, sec_plane_t*);
	void UpdateSecSurface(sec_surface_t*, sec_plane_t*, subsector_t*);
	surface_t* NewWSurf();
	void FreeWSurfs(surface_t*);
	surface_t* SubdivideSeg(surface_t*, const TVec&, const TVec*);
	surface_t* CreateWSurfs(TVec*, texinfo_t*, seg_t*, subsector_t*);
	int CountSegParts(seg_t*);
	void CreateSegParts(drawseg_t*, seg_t*);
	void UpdateRowOffset(segpart_t*, float);
	void UpdateTextureOffset(segpart_t*, float);
	void UpdateDrawSeg(drawseg_t*);
	void UpdateSubRegion(subregion_t*);
	void UpdateSubsector(int, float*);
	void UpdateBSPNode(int, float*);
	bool CopyPlaneIfValid(sec_plane_t*, const sec_plane_t*,
		const sec_plane_t*);
	void UpdateFakeFlats(sector_t*);
	void FreeSurfaces(surface_t*);
	void FreeSegParts(segpart_t*);

	//	Sky methods
	void InitOldSky();
	void InitSkyBox(VName, VName);
	void InitSky();
	void DoLightningFlash();

	//	Light methods
	static void CalcMinMaxs(surface_t*);
	float CastRay(const TVec&, const TVec&, float);
	static void CalcFaceVectors(surface_t*);
	void CalcPoints(surface_t*);
	void SingleLightFace(light_t*, surface_t*);
	void LightFace(surface_t*, subsector_t*);
	void MarkLights(dlight_t*, int, int);
	void AddDynamicLights(surface_t*);

	void PrecacheLevel();

	void InitParticles();
	void ClearParticles();

	//	World BSP rendering
	void SetUpFrustumIndexes();
	void DrawSurfaces(surface_t*, texinfo_t*, int clipflags, int = -1);
	void RenderLine(drawseg_t*, int);
	void RenderSecSurface(sec_surface_t*, int);
	void RenderSubRegion(subregion_t*, int);
	void RenderSubsector(int, int);
	void RenderBSPNode(int, float*, int);

public:
	VRenderLevel(VLevel*);
	~VRenderLevel();

	void PreRender();
	void SegMoved(seg_t*);
	void UpdateWorld();
	void SetupFakeFloors(sector_t*);

	void SkyChanged();
	void AnimateSky(float);
	void ForceLightning();
	void DrawSky();

	void AddStaticLight(const TVec&, float, vuint32);
	dlight_t* AllocDlight(int);
	void DecayLights(float);
	void PushDlights();
	vuint32 LightPoint(const TVec &p);
	bool BuildLightMap(surface_t*, int);

	particle_t* NewParticle();
	void UpdateParticles(float);
	void DrawParticles();

	void RenderWorld();
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

extern VLevel*			r_Level;

#endif
