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
//**
//**	Defines shared by refresh and drawer
//**
//**************************************************************************

#ifndef _R_SHARED_H
#define _R_SHARED_H

// HEADER FILES ------------------------------------------------------------

#include "fmd2defs.h"

// MACROS ------------------------------------------------------------------

//	Colour maps
enum
{
	CM_Default,
	CM_Inverse,
	CM_Gold,
	CM_Red,
	CM_Green,

	CM_Max
};

//  Simulate light fading using dark fog
enum
{
	FADE_LIGHT = 0xff010101
};

// TYPES -------------------------------------------------------------------

class TClipPlane : public TPlane
{
public:
	TClipPlane		*next;

	int				clipflag;
	TVec			enter;
	TVec			exit;
	int				entered;
	int				exited;
};

struct texinfo_t
{
	TVec			saxis;
	float			soffs;
	TVec			taxis;
	float			toffs;
	VTexture*		Tex;
	//	1.1 for solid surfaces
	// Alpha for masked surfaces
	float			Alpha;
	bool			Additive;
	vuint8			ColourMap;
};

struct surface_t
{
	surface_t*		next;
	surface_t*		DrawNext;
	texinfo_t*		texinfo;
	TPlane*			plane;
	sec_plane_t*	HorizonPlane;
	vuint32			Light;		//	Light level and colour.
	vuint32			Fade;
	vuint8*			lightmap;
	rgb_t*			lightmap_rgb;
	int				dlightframe;
	int				dlightbits;
	int				count;
	short			texturemins[2];
	short			extents[2];
	surfcache_t*	cachespots[4];
	TVec			verts[1];
};

//
//	Camera texture.
//
class VCameraTexture : public VTexture
{
public:
	vuint8*		Pixels;
	bool		bNeedsUpdate;
	bool		bUpdated;

	VCameraTexture(VName, int, int);
	~VCameraTexture();
	bool CheckModified();
	vuint8* GetPixels();
	void Unload();
	void CopyImage();
	VTexture* GetHighResolutionTexture();
};

class VRenderLevelShared;

//
//	Base class for portals.
//
class VPortal
{
public:
	VRenderLevelShared*		RLev;
	TArray<surface_t*>		Surfs;
	int						Level;

	VPortal(VRenderLevelShared* ARLev);
	virtual ~VPortal();
	virtual bool NeedsDepthBuffer() const;
	virtual bool IsSky() const;
	virtual bool MatchSky(class VSky*) const;
	virtual bool MatchSkyBox(VEntity*) const;
	virtual bool MatchMirror(TPlane*) const;
	void Draw(bool);
	virtual void DrawContents() = 0;

protected:
	void SetUpRanges(VViewClipper&, bool);
};

struct VMeshFrame
{
	TVec*			Verts;
	TVec*			Normals;
	vuint32			VertsOffset;
	vuint32			NormalsOffset;
};

struct VMeshSTVert
{
	float			S;
	float			T;
};

struct VMeshTri
{
	vuint16				VertIndex[3];
};

struct VMeshModel
{
	VStr				Name;
	mmdl_t*				Data;		// only access through Mod_Extradata
	TArray<VName>		Skins;
	TArray<VMeshFrame>	Frames;
	TArray<TVec>		AllVerts;
	TArray<TVec>		AllNormals;
	TArray<VMeshSTVert>	STVerts;
	TArray<VMeshTri>	Tris;
	bool				Uploaded;
	vuint32				VertsBuffer;
	vuint32				IndexBuffer;
};

// PUBLIC FUNCTION PROTOTYPES ----------------------------------------------

void R_DrawViewBorder();

// PUBLIC DATA DECLARATIONS ------------------------------------------------

//
// POV related.
//
extern "C" {
extern TVec				vieworg;
extern TVec				viewforward;
extern TVec				viewright;
extern TVec				viewup;
} // extern "C"
extern TAVec			viewangles;

extern VCvarI			r_fog;
extern VCvarF			r_fog_r;
extern VCvarF			r_fog_g;
extern VCvarF			r_fog_b;
extern VCvarF			r_fog_start;
extern VCvarF			r_fog_end;
extern VCvarF			r_fog_density;

extern VCvarI			r_vsync;

extern VCvarI			r_fade_light;
extern VCvarF			r_fade_factor;

extern VCvarF			r_sky_bright_factor;

extern "C" {
extern TClipPlane		view_clipplanes[5];
}

extern bool				MirrorFlip;
extern bool				MirrorClip;

extern int				r_dlightframecount;
extern bool				r_light_add;
extern vuint32			blocklights[18 * 18];
extern vuint32			blocklightsr[18 * 18];
extern vuint32			blocklightsg[18 * 18];
extern vuint32			blocklightsb[18 * 18];
extern vuint32			blockaddlightsr[18 * 18];
extern vuint32			blockaddlightsg[18 * 18];
extern vuint32			blockaddlightsb[18 * 18];

extern rgba_t			r_palette[256];
extern vuint8			r_black_colour;

extern "C" {
extern vuint8			r_rgbtable[32 * 32 * 32 + 4];
};

extern int				usegamma;
extern vuint8			gammatable[5][256];

extern float			PixelAspect;

extern VTextureTranslation	ColourMaps[CM_Max];

enum { NUMVERTEXNORMALS = 162 };
extern "C" {
extern float			r_avertexnormals[NUMVERTEXNORMALS][3];
}

//==========================================================================
//
//	R_LookupRBG
//
//==========================================================================

inline int R_LookupRGB(vuint8 r, vuint8 g, vuint8 b)
{
	return r_rgbtable[((r << 7) & 0x7c00) + ((g << 2) & 0x3e0) +
		((b >> 3) & 0x1f)];
}

#endif
