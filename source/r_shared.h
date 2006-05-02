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
//**
//**	Defines shared by refresh and drawer
//**
//**************************************************************************

#ifndef _R_SHARED_H
#define _R_SHARED_H

// HEADER FILES ------------------------------------------------------------

#include "fmd2defs.h"

// MACROS ------------------------------------------------------------------

// TYPES -------------------------------------------------------------------

class TClipPlane : public TPlane
{
public:
	TClipPlane		*next;

	int				clipflag;
	TVec			enter;
	TVec			exit;
	boolean			entered;
	boolean			exited;
};

struct texinfo_t
{
	TVec			saxis;
	float			soffs;
	TVec			taxis;
	float			toffs;
	int				pic;
	//	0 for solid surfaces
	// translucency + 1 for masked surfaces
	int				translucency;
};

struct surface_t
{
	surface_t		*next;
	texinfo_t		*texinfo;
	TPlane			*plane;
	dword			Light;		//	Light level and color.
	byte			*lightmap;
	rgb_t			*lightmap_rgb;
	int				dlightframe;
	int				dlightbits;
	int				count;
	short			texturemins[2];
	short			extents[2];
	surfcache_t		*cachespots[4];
	TVec			verts[1];
};

enum
{
	TEXFMT_8,		//	Paletised texture in main palette.
	TEXFMT_8Pal,	//	Paletised texture with custom palette.
	TEXFMT_RGBA,	//	Truecolor texture.
};

class VTexture
{
public:
	int			Type;
	int			Format;
	VName		Name;
	int			Width;
	int			Height;
	int			SOffset;
	int			TOffset;
	bool		bNoRemap0;
	float		SScale;				//	Scaling
	float		TScale;
	int			TextureTranslation;	// Animation
	union
	{
		dword	DriverHandle;
		void*	DriverData;
	};
protected:
	vuint8*		Pixels8Bit;
	VTexture*	HiResTexture;

public:
	VTexture();
	virtual ~VTexture();

	int GetWidth() { if (Width == -1) GetDimensions(); return Width; }
	int GetHeight() { if (Width == -1) GetDimensions(); return Height; }
	virtual void GetDimensions();
	virtual void SetFrontSkyLayer();
	virtual vuint8* GetPixels() = 0;
	virtual vuint8* GetPixels8();
	virtual rgba_t* GetPalette();
	virtual void Unload() = 0;
	VTexture* GetHighResolutionTexture();

protected:
	void FixupPalette(vuint8* Pixels, rgba_t* Palette);
};

// PUBLIC FUNCTION PROTOTYPES ----------------------------------------------

void R_DrawViewBorder(void);

bool R_BuildLightMap(surface_t*, int);

void *Mod_Extradata(VModel* mod);	// handles caching

// PUBLIC DATA DECLARATIONS ------------------------------------------------

extern vuint8*			translationtables;

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

extern int				extralight;
extern int				fixedlight;

extern "C" {
extern TClipPlane		view_clipplanes[4];
}

extern TVec				r_normal;
extern float			r_dist;

extern TVec				r_saxis;
extern TVec				r_taxis;
extern TVec				r_texorg;

extern surface_t		*r_surface;

extern int				r_dlightframecount;
extern bool				r_light_add;
extern dword			blocklights[18 * 18];
extern dword			blocklightsr[18 * 18];
extern dword			blocklightsg[18 * 18];
extern dword			blocklightsb[18 * 18];
extern dword			blockaddlightsr[18 * 18];
extern dword			blockaddlightsg[18 * 18];
extern dword			blockaddlightsb[18 * 18];

extern rgba_t			r_palette[256];
extern byte				r_black_colour;

extern int				usegamma;
extern byte				gammatable[5][256];

extern float			PixelAspect;

#endif
