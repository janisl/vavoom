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
//**	Copyright (C) 1999-2001 JÆnis Legzdi·ý
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

#include "fgfxdefs.h"
#include "fmd2defs.h"

// MACROS ------------------------------------------------------------------

#define MAX_PICS		512

#define MAX_PALETTES	8

//	Flag in texture number indicating that this texture is a flat. This
// allows to use wall textures on floors / ceilings and flats on walls
#define TEXF_FLAT		0x8000
#define TEXF_SKY_MAP	0x10000		// External sky graphic

// TYPES -------------------------------------------------------------------

//	A maptexturedef_t describes a rectangular texture, which is composed of
// one or more mappatch_t structures that arrange graphic patches

struct texpatch_t
{
    // Block origin (allways UL),
    // which has allready accounted
    // for the internal origin of the patch.
    short		originx;
    short		originy;
    int			patch;
};

struct texdef_t
{
    // Keep name for switch changing, etc.
    char		name[10];
    short		width;
    short		height;
    // All the patches[patchcount]
    //are drawn back to front into the cached texture.
    short		patchcount;
    texpatch_t	patches[1];
};

struct skymap_t
{
	char name[128];
	int width;
	int height;
};

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

struct pic_info_t
{
	char	name[MAX_VPATH];
	int		type;
	int		palnum;
	int		width;
	int		height;
	int		xoffset;
	int		yoffset;
};

// PUBLIC FUNCTION PROTOTYPES ----------------------------------------------

void R_DrawViewBorder(void);

bool R_BuildLightMap(surface_t*, int);

int R_TextureAnimation(int);

void *Mod_Extradata(model_t *mod);	// handles caching
void Mod_LoadSkin(const char *name, void **bufptr);

// PUBLIC DATA DECLARATIONS ------------------------------------------------

extern int				numtextures;
extern texdef_t**		textures;
extern int				numflats;
extern int*				flatlumps;
extern int				numspritelumps;
extern int*				spritelumps;
extern int*				spritewidth;
extern int*				spriteheight;
extern int*				spriteoffset;
extern int*				spritetopoffset;
extern byte*			translationtables;
extern pic_info_t		pic_list[MAX_PICS];
extern skymap_t			*skymaps;
extern int				numskymaps;

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

extern TCvarI			r_fog;
extern TCvarF			r_fog_r;
extern TCvarF			r_fog_g;
extern TCvarF			r_fog_b;
extern TCvarF			r_fog_start;
extern TCvarF			r_fog_end;
extern TCvarF			r_fog_density;

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

extern rgba_t			r_palette[MAX_PALETTES][256];
extern byte				r_black_color[MAX_PALETTES];

extern int				SkinWidth;
extern int				SkinHeight;
extern int				SkinBPP;
extern byte				*SkinData;
extern rgba_t			SkinPal[256];

extern int				usegamma;
extern byte				gammatable[5][256];

#endif

//**************************************************************************
//
//	$Log$
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
