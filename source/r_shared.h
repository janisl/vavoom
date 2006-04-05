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

class TTexture
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

	TTexture();
	virtual ~TTexture();

	int GetWidth() { if (Width == -1) GetDimensions(); return Width; }
	int GetHeight() { if (Width == -1) GetDimensions(); return Height; }
	virtual void GetDimensions();
	virtual void SetFrontSkyLayer();
	virtual byte* GetPixels() = 0;
	virtual byte* GetPixels8();
	virtual rgba_t* GetPalette();
	virtual void MakePurgable() = 0;
	virtual void Unload() = 0;
	rgba_t* GetHighResPixels(int& HRWidth, int& HRHeight);
};

// PUBLIC FUNCTION PROTOTYPES ----------------------------------------------

void R_DrawViewBorder(void);

bool R_BuildLightMap(surface_t*, int);

void *Mod_Extradata(VModel* mod);	// handles caching
void Mod_LoadSkin(const char *name, void **bufptr);

// PUBLIC DATA DECLARATIONS ------------------------------------------------

extern byte*			translationtables;

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

//**************************************************************************
//
//	$Log$
//	Revision 1.21  2006/04/05 17:23:37  dj_jl
//	More dynamic string usage in console command class.
//	Added class for handling command line arguments.
//
//	Revision 1.20  2006/02/27 20:45:26  dj_jl
//	Rewrote names class.
//	
//	Revision 1.19  2006/02/05 14:11:00  dj_jl
//	Fixed conflict with Solaris.
//	
//	Revision 1.18  2005/05/26 16:50:15  dj_jl
//	Created texture manager class
//	
//	Revision 1.17  2002/08/28 16:39:19  dj_jl
//	Implemented sector light color.
//	
//	Revision 1.16  2002/03/28 17:58:02  dj_jl
//	Added support for scaled textures.
//	
//	Revision 1.15  2002/03/20 19:11:21  dj_jl
//	Added guarding.
//	
//	Revision 1.14  2002/01/07 12:16:43  dj_jl
//	Changed copyright year
//	
//	Revision 1.13  2001/12/27 17:36:47  dj_jl
//	Some speedup
//	
//	Revision 1.12  2001/12/18 19:01:34  dj_jl
//	Changes for MSVC asm
//	
//	Revision 1.11  2001/11/09 14:18:40  dj_jl
//	Added specular highlights
//	
//	Revision 1.10  2001/10/27 07:47:52  dj_jl
//	Public gamma variables
//	
//	Revision 1.9  2001/10/18 17:36:31  dj_jl
//	A lots of changes for Alpha 2
//	
//	Revision 1.8  2001/10/04 17:23:29  dj_jl
//	Got rid of some warnings
//	
//	Revision 1.7  2001/09/05 12:21:42  dj_jl
//	Release changes
//	
//	Revision 1.6  2001/08/23 17:47:22  dj_jl
//	Started work on pics with custom palettes
//	
//	Revision 1.5  2001/08/21 17:46:08  dj_jl
//	Added R_TextureAnimation, made SetTexture recognize flats
//	
//	Revision 1.4  2001/08/07 16:46:23  dj_jl
//	Added player models, skins and weapon
//	
//	Revision 1.3  2001/07/31 17:16:31  dj_jl
//	Just moved Log to the end of file
//	
//	Revision 1.2  2001/07/27 14:27:54  dj_jl
//	Update with Id-s and Log-s, some fixes
//
//**************************************************************************
