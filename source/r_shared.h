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
//**	$Log$
//**	Revision 1.2  2001/07/27 14:27:54  dj_jl
//**	Update with Id-s and Log-s, some fixes
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

// MACROS ------------------------------------------------------------------

#define MAX_PICS	512

//	Flag in texture number indicating that this texture is a flat. This
// allows to use wall textures on floors / ceilings and flats on walls
#define TEXF_FLAT	0x8000

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
	char	name[12];
	int		type;
};

// PUBLIC FUNCTION PROTOTYPES ----------------------------------------------

void R_DrawViewBorder(void);

bool R_BuildLightMap(surface_t *surf, int shift);

// PUBLIC DATA DECLARATIONS ------------------------------------------------

extern int				numtextures;
extern texdef_t**		textures;
extern int*				texturetranslation;
extern int*				flatlumps;
extern int*				flattranslation;
extern int				numspritelumps;
extern int*				spritelumps;
extern int*				spritewidth;
extern int*				spriteheight;
extern int*				spriteoffset;
extern int*				spritetopoffset;
extern byte*			translationtables;
extern pic_info_t		pic_list[MAX_PICS];

//
// POV related.
//
extern TVec				vieworg;
extern TVec				viewforward;
extern TVec				viewright;
extern TVec				viewup;

extern int				viewwidth;
extern int				viewheight;

extern bool				r_use_fog;

extern int				extralight;
extern int				fixedlight;

extern TClipPlane		view_clipplanes[4];

extern TVec				r_normal;
extern float			r_dist;

extern TVec				r_saxis;
extern TVec				r_taxis;
extern TVec				r_texorg;

extern surface_t		*r_surface;

extern int				r_dlightframecount;
extern dword			blocklights[18 * 18];
extern dword			blocklightsr[18 * 18];
extern dword			blocklightsg[18 * 18];
extern dword			blocklightsb[18 * 18];

extern int				SkinWidth;
extern int				SkinHeight;
extern byte				*SkinData;
extern rgb_t			SkinPal[256];

#endif

