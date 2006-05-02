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

// PUBLIC FUNCTION PROTOTYPES ----------------------------------------------

//
// R_Main
//

//
// R_BSP
//
void R_RenderWorld();

//
// R_Things
//
void R_RenderMobjs();
void R_DrawPlayerSprites();
void R_DrawCroshair();
void R_DrawTranslucentPoly(TVec*, int, int, int, int, bool, dword);
void R_DrawTranslucentPolys();

//
// R_Sky
//
void R_InitSkyBoxes();
void R_InitSky();
void R_AnimateSky();
void R_DrawSky();

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
dword R_LightPoint(const TVec &p);

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

#endif
