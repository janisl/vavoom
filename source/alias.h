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

// HEADER FILES ------------------------------------------------------------

// MACROS ------------------------------------------------------------------

#define ALIAS_Z_CLIP_PLANE	5

// flags in finalvert_t.flags
#define ALIAS_LEFT_CLIP				0x0001
#define ALIAS_TOP_CLIP				0x0002
#define ALIAS_RIGHT_CLIP			0x0004
#define ALIAS_BOTTOM_CLIP			0x0008
#define ALIAS_Z_CLIP				0x0010
// !!! if this is changed, it must be changed in d_ifacea.h too !!!
//#define ALIAS_ONSEAM				0x0020	// also defined in modelgen.h;
											//  must be kept in sync
#define ALIAS_XY_CLIP_MASK			0x000F

// TYPES -------------------------------------------------------------------

struct finalvert_t
{
	int		u;
	int		v;
	int		l;
	int		zi;
	int		flags;
	float	reserved1;
	float	reserved2;
	float	reserved3;
};

struct finalstvert_t
{
	int			s;
	int			t;
};

struct affinetridesc_t
{
	void				*pskin;
	int					skinwidth;
	int					skinheight;
	mtriangle_t			*ptriangles;
	finalvert_t			*pfinalverts;
	finalstvert_t		*pstverts;
	int					numtriangles;
};

// viewmodel lighting
struct alight_t
{
	int			ambientlight;
	int			shadelight;
	float		*plightvec;
};

struct auxvert_t
{
	float	fv[3];		// viewspace x, y
};

// PUBLIC FUNCTION PROTOTYPES ----------------------------------------------

void D_PolysetDraw(void);
void R_AliasClipTriangle(mtriangle_t *ptri);
void R_AliasProjectFinalVert(finalvert_t *fv, auxvert_t *av);

// PUBLIC DATA DECLARATIONS ------------------------------------------------

extern int			ubasestep, errorterm, erroradjustup, erroradjustdown;
extern float		r_aliastransition, r_resfudge;
extern TVec			modelorg;

extern affinetridesc_t		r_affinetridesc;
extern finalvert_t			*pfinalverts;
extern finalstvert_t		*pfinalstverts;
extern auxvert_t			*pauxverts;

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
