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
//	r_model
//
void LoadPNGLump(int LumpNum, void** bufptr);

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
extern subsector_t		*r_viewleaf;

extern byte				light_remap[256];
extern VCvarI			r_darken;

extern refdef_t			refdef;

extern VCvarI			old_aspect;

extern int				SkinWidth;
extern int				SkinHeight;
extern int				SkinBPP;
extern byte*			SkinData;
extern rgba_t			SkinPal[256];

#endif

//**************************************************************************
//
//	$Log$
//	Revision 1.15  2006/04/05 17:23:37  dj_jl
//	More dynamic string usage in console command class.
//	Added class for handling command line arguments.
//
//	Revision 1.14  2005/06/30 20:20:55  dj_jl
//	Implemented rendering of Boom fake flats.
//	
//	Revision 1.13  2005/05/30 18:34:03  dj_jl
//	Added support for IMGZ and PNG lump textures
//	
//	Revision 1.12  2005/05/26 16:50:14  dj_jl
//	Created texture manager class
//	
//	Revision 1.11  2004/12/27 12:23:16  dj_jl
//	Multiple small changes for version 1.16
//	
//	Revision 1.10  2002/08/28 16:39:19  dj_jl
//	Implemented sector light color.
//	
//	Revision 1.9  2002/07/13 07:39:08  dj_jl
//	Removed back to front drawing.
//	
//	Revision 1.8  2002/03/28 17:58:02  dj_jl
//	Added support for scaled textures.
//	
//	Revision 1.7  2002/01/07 12:16:43  dj_jl
//	Changed copyright year
//	
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
