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

#ifndef __CL_LOCAL_H__
#define __CL_LOCAL_H__

// HEADER FILES ------------------------------------------------------------

#include "iline.h"		//	Input line widget
#include "model.h"
#include "drawer.h"

// MACROS ------------------------------------------------------------------

#define	MAX_DLIGHTS		32

// TYPES -------------------------------------------------------------------

struct dlight_t
{
	TVec	origin;		// origin of the light
	float	radius;		// radius - how far light goes
	float	die;		// stop lighting after this time
	float	decay;		// drop this each second
	float	minlight;	// don't add when contributing less
	int		key;		// used to identify owner to reuse the same light
	dword	color;		// for colored lights
};

// Client side Map Object definition.
struct clmobj_t
{
	boolean		in_use;

	TVec		origin;	// position
	TAVec		angles;	// orientation

	int			spritetype;
    int			sprite;	// used to find patch_t and flip value
    int			frame;	// might be ORed with FF_FULLBRIGHT

	int			model_index;
	model_t		*alias_model;
	int			alias_frame;
	int			alias_skinnum;

    int			translucency;
    int			translation;

	float		floorclip;		// value to use for floor clipping

	int			effects;
};

// PUBLIC FUNCTION PROTOTYPES ----------------------------------------------

dlight_t *CL_AllocDlight(int key);
void CL_DecayLights(void);

void CL_KeepaliveMessage(void);

boolean CL_TraceLine(const TVec &start, const TVec &end);

particle_t *R_NewParticle(void);
void R_AddStaticLight(const TVec &origin, float radius, dword color);

void R_InstallSprite(const char *name, int index);

// PUBLIC DATA DECLARATIONS ------------------------------------------------

extern clmobj_t			cl_mobjs[MAX_MOBJS];

extern float			fScaleX;
extern float			fScaleY;
extern float			fScaleXI;
extern float			fScaleYI;

inline subsector_t* CL_PointInSubsector(float x, float y)
{
	return PointInSubsector(cl_level, x, y);
}

extern dlight_t			cl_dlights[MAX_DLIGHTS];

#endif

//**************************************************************************
//
//	$Log$
//	Revision 1.3  2001/07/31 17:16:30  dj_jl
//	Just moved Log to the end of file
//
//	Revision 1.2  2001/07/27 14:27:54  dj_jl
//	Update with Id-s and Log-s, some fixes
//
//**************************************************************************
