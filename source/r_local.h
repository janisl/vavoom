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
	float			xoffs;
	float			yoffs;
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

// PUBLIC FUNCTION PROTOTYPES ----------------------------------------------

//
// R_Main
//

//
// R_BSP
//
void R_RenderWorld(void);

//
// R_Things
//
void R_RenderMobjs(void);
void R_DrawPlayerSprites(void);
void R_DrawCroshair(void);
void R_DrawTranslucentPoly(TVec*, int, int, int, int, bool, dword);
void R_DrawTranslucentPolys(void);

//
// R_Sky
//
void R_InitSkyBoxes(void);
void R_InitSky(const mapInfo_t &info);
void R_AnimateSky(void);
void R_DrawSky(void);

//
//	R_Tex
//
void R_PrecacheLevel(void);
float R_TextureHeight(int pic);

//
//	R_Surf
//
void R_UpdateWorld(void);

//
//	R_Light
//
void R_ClearLights(void);
void R_LightFace(surface_t *surf, subsector_t *leaf);
void R_PushDlights(void);
dword R_LightPoint(const TVec &p);

// PUBLIC DATA DECLARATIONS ------------------------------------------------

//
// R_Main
//
extern int      		screenblocks;
extern int				r_visframecount;
extern subsector_t		*r_viewleaf;
extern bool				r_back2front;

extern byte				light_remap[256];
extern TCvarI			r_darken;

extern refdef_t			refdef;

#endif

//**************************************************************************
//
//	$Log$
//	Revision 1.6  2001/11/09 14:22:09  dj_jl
//	R_InitTexture now called from Host_init
//
//	Revision 1.5  2001/10/12 17:31:13  dj_jl
//	no message
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
