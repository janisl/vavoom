//**************************************************************************
//**
//**	##   ##    ##    ##   ##   ####     ####   ###     ###
//**	##   ##  ##  ##  ##   ##  ##  ##   ##  ##  ####   ####
//**	 ## ##  ##    ##  ## ##  ##    ## ##    ## ## ## ## ##
//**	 ## ##  ########  ## ##  ##    ## ##    ## ##  ###  ##
//**	  ###   ##    ##   ###    ##  ##   ##  ##  ##       ##
//**	   #    ##    ##    #      ####     ####   ##       ##
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
//**	Software driver module for drawing particles
//**
//**************************************************************************

// HEADER FILES ------------------------------------------------------------

#include "d_local.h"

// MACROS ------------------------------------------------------------------

// TYPES -------------------------------------------------------------------

// EXTERNAL FUNCTION PROTOTYPES --------------------------------------------

// PUBLIC FUNCTION PROTOTYPES ----------------------------------------------

// PRIVATE FUNCTION PROTOTYPES ---------------------------------------------

// EXTERNAL DATA DECLARATIONS ----------------------------------------------

// PUBLIC DATA DEFINITIONS -------------------------------------------------

float			xscaleshrink;
float			yscaleshrink;

int				d_particle_right;
int				d_particle_top;

int				d_pix_shift;
int				d_pix_min;
int				d_pix_max;
int				d_y_aspect_shift;

TVec			r_pright;
TVec			r_pup;
TVec			r_ppn;

particle_func_t	D_DrawParticle;

// PRIVATE DATA DEFINITIONS ------------------------------------------------

// CODE --------------------------------------------------------------------

#ifndef USEASM

//==========================================================================
//
//	D_DrawParticle_8
//
//==========================================================================

extern "C" void D_DrawParticle_8(particle_t *pparticle)
{
	TVec	local, transformed;
	float	zi;
	byte	*pdest;
	short	*pz;
	int		i, izi, pix, count, u, v, color;

	// transform point
	local = pparticle->org - vieworg;

	transformed.x = DotProduct(local, r_pright);
	transformed.y = DotProduct(local, r_pup);
	transformed.z = DotProduct(local, r_ppn);

	if (transformed.z < PARTICLE_Z_CLIP)
		return;

	// project the point
	// FIXME: preadjust xcenter and ycenter
	zi = 1.0 / transformed.z;
	u = (int)(centerxfrac + zi * transformed.x + 0.5);
	v = (int)(centeryfrac + zi * transformed.y + 0.5);

	if ((v > d_particle_top) || 
		(u > d_particle_right) ||
		(v < 0) ||
		(u < 0))
	{
		return;
	}

	color =	d_rgbtable[((pparticle->color >> 9) & 0x7c00) +
		((pparticle->color >> 6) & 0x3e0) +
		((pparticle->color >> 3) & 0x1f)];

	pz = zbuffer + ylookup[v] + u;
	pdest = (byte*)scrn + ylookup[v] + u;
	izi = (int)(zi * 0x8000);

	pix = izi >> d_pix_shift;

	if (pix < d_pix_min)
		pix = d_pix_min;
	else if (pix > d_pix_max)
		pix = d_pix_max;

	switch (pix)
	{
	case 1:
		count = 1 << d_y_aspect_shift;

		for ( ; count ; count--, pz -= ScreenWidth, pdest -= ScreenWidth)
		{
			if (pz[0] <= izi)
			{
				pz[0] = izi;
				pdest[0] = color;
			}
		}
		break;

	case 2:
		count = 2 << d_y_aspect_shift;

		for ( ; count ; count--, pz -= ScreenWidth, pdest -= ScreenWidth)
		{
			if (pz[0] <= izi)
			{
				pz[0] = izi;
				pdest[0] = color;
			}

			if (pz[1] <= izi)
			{
				pz[1] = izi;
				pdest[1] = color;
			}
		}
		break;

	case 3:
		count = 3 << d_y_aspect_shift;

		for ( ; count ; count--, pz -= ScreenWidth, pdest -= ScreenWidth)
		{
			if (pz[0] <= izi)
			{
				pz[0] = izi;
				pdest[0] = color;
			}

			if (pz[1] <= izi)
			{
				pz[1] = izi;
				pdest[1] = color;
			}

			if (pz[2] <= izi)
			{
				pz[2] = izi;
				pdest[2] = color;
			}
		}
		break;

	case 4:
		count = 4 << d_y_aspect_shift;

		for ( ; count ; count--, pz -= ScreenWidth, pdest -= ScreenWidth)
		{
			if (pz[0] <= izi)
			{
				pz[0] = izi;
				pdest[0] = color;
			}

			if (pz[1] <= izi)
			{
				pz[1] = izi;
				pdest[1] = color;
			}

			if (pz[2] <= izi)
			{
				pz[2] = izi;
				pdest[2] = color;
			}

			if (pz[3] <= izi)
			{
				pz[3] = izi;
				pdest[3] = color;
			}
		}
		break;

	default:
		count = pix << d_y_aspect_shift;

		for ( ; count ; count--, pz -= ScreenWidth, pdest -= ScreenWidth)
		{
			for (i=0 ; i<pix ; i++)
			{
				if (pz[i] <= izi)
				{
					pz[i] = izi;
					pdest[i] = color;
				}
			}
		}
		break;
	}
}

//==========================================================================
//
//	D_DrawParticle_15
//
//==========================================================================

extern "C" void D_DrawParticle_15(particle_t *pparticle)
{
	TVec	local, transformed;
	float	zi;
	word	*pdest;
	short	*pz;
	int		i, izi, pix, count, u, v, color;

	// transform point
	local = pparticle->org - vieworg;

	transformed.x = DotProduct(local, r_pright);
	transformed.y = DotProduct(local, r_pup);
	transformed.z = DotProduct(local, r_ppn);

	if (transformed.z < PARTICLE_Z_CLIP)
		return;

	// project the point
	// FIXME: preadjust xcenter and ycenter
	zi = 1.0 / transformed.z;
	u = (int)(centerxfrac + zi * transformed.x + 0.5);
	v = (int)(centeryfrac + zi * transformed.y + 0.5);

	if ((v > d_particle_top) || 
		(u > d_particle_right) ||
		(v < 0) ||
		(u < 0))
	{
		return;
	}

	byte r = (pparticle->color >> 16) & 0xff;
	byte g = (pparticle->color >> 8) & 0xff;
	byte b = pparticle->color & 0xff;
	color =	MakeCol15(r, g, b);

	pz = zbuffer + ylookup[v] + u;
	pdest = (word*)scrn + ylookup[v] + u;
	izi = (int)(zi * 0x8000);

	pix = izi >> d_pix_shift;

	if (pix < d_pix_min)
		pix = d_pix_min;
	else if (pix > d_pix_max)
		pix = d_pix_max;

	switch (pix)
	{
	case 1:
		count = 1 << d_y_aspect_shift;

		for ( ; count ; count--, pz -= ScreenWidth, pdest -= ScreenWidth)
		{
			if (pz[0] <= izi)
			{
				pz[0] = izi;
				pdest[0] = color;
			}
		}
		break;

	case 2:
		count = 2 << d_y_aspect_shift;

		for ( ; count ; count--, pz -= ScreenWidth, pdest -= ScreenWidth)
		{
			if (pz[0] <= izi)
			{
				pz[0] = izi;
				pdest[0] = color;
			}

			if (pz[1] <= izi)
			{
				pz[1] = izi;
				pdest[1] = color;
			}
		}
		break;

	case 3:
		count = 3 << d_y_aspect_shift;

		for ( ; count ; count--, pz -= ScreenWidth, pdest -= ScreenWidth)
		{
			if (pz[0] <= izi)
			{
				pz[0] = izi;
				pdest[0] = color;
			}

			if (pz[1] <= izi)
			{
				pz[1] = izi;
				pdest[1] = color;
			}

			if (pz[2] <= izi)
			{
				pz[2] = izi;
				pdest[2] = color;
			}
		}
		break;

	case 4:
		count = 4 << d_y_aspect_shift;

		for ( ; count ; count--, pz -= ScreenWidth, pdest -= ScreenWidth)
		{
			if (pz[0] <= izi)
			{
				pz[0] = izi;
				pdest[0] = color;
			}

			if (pz[1] <= izi)
			{
				pz[1] = izi;
				pdest[1] = color;
			}

			if (pz[2] <= izi)
			{
				pz[2] = izi;
				pdest[2] = color;
			}

			if (pz[3] <= izi)
			{
				pz[3] = izi;
				pdest[3] = color;
			}
		}
		break;

	default:
		count = pix << d_y_aspect_shift;

		for ( ; count ; count--, pz -= ScreenWidth, pdest -= ScreenWidth)
		{
			for (i=0 ; i<pix ; i++)
			{
				if (pz[i] <= izi)
				{
					pz[i] = izi;
					pdest[i] = color;
				}
			}
		}
		break;
	}
}

//==========================================================================
//
//	D_DrawParticle_16
//
//==========================================================================

extern "C" void D_DrawParticle_16(particle_t *pparticle)
{
	TVec	local, transformed;
	float	zi;
	word	*pdest;
	short	*pz;
	int		i, izi, pix, count, u, v, color;

	// transform point
	local = pparticle->org - vieworg;

	transformed.x = DotProduct(local, r_pright);
	transformed.y = DotProduct(local, r_pup);
	transformed.z = DotProduct(local, r_ppn);

	if (transformed.z < PARTICLE_Z_CLIP)
		return;

	// project the point
	// FIXME: preadjust xcenter and ycenter
	zi = 1.0 / transformed.z;
	u = (int)(centerxfrac + zi * transformed.x + 0.5);
	v = (int)(centeryfrac + zi * transformed.y + 0.5);

	if ((v > d_particle_top) || 
		(u > d_particle_right) ||
		(v < 0) ||
		(u < 0))
	{
		return;
	}

	byte r = (pparticle->color >> 16) & 0xff;
	byte g = (pparticle->color >> 8) & 0xff;
	byte b = pparticle->color & 0xff;
	color =	MakeCol16(r, g, b);

	pz = zbuffer + ylookup[v] + u;
	pdest = (word*)scrn + ylookup[v] + u;
	izi = (int)(zi * 0x8000);

	pix = izi >> d_pix_shift;

	if (pix < d_pix_min)
		pix = d_pix_min;
	else if (pix > d_pix_max)
		pix = d_pix_max;

	switch (pix)
	{
	case 1:
		count = 1 << d_y_aspect_shift;

		for ( ; count ; count--, pz -= ScreenWidth, pdest -= ScreenWidth)
		{
			if (pz[0] <= izi)
			{
				pz[0] = izi;
				pdest[0] = color;
			}
		}
		break;

	case 2:
		count = 2 << d_y_aspect_shift;

		for ( ; count ; count--, pz -= ScreenWidth, pdest -= ScreenWidth)
		{
			if (pz[0] <= izi)
			{
				pz[0] = izi;
				pdest[0] = color;
			}

			if (pz[1] <= izi)
			{
				pz[1] = izi;
				pdest[1] = color;
			}
		}
		break;

	case 3:
		count = 3 << d_y_aspect_shift;

		for ( ; count ; count--, pz -= ScreenWidth, pdest -= ScreenWidth)
		{
			if (pz[0] <= izi)
			{
				pz[0] = izi;
				pdest[0] = color;
			}

			if (pz[1] <= izi)
			{
				pz[1] = izi;
				pdest[1] = color;
			}

			if (pz[2] <= izi)
			{
				pz[2] = izi;
				pdest[2] = color;
			}
		}
		break;

	case 4:
		count = 4 << d_y_aspect_shift;

		for ( ; count ; count--, pz -= ScreenWidth, pdest -= ScreenWidth)
		{
			if (pz[0] <= izi)
			{
				pz[0] = izi;
				pdest[0] = color;
			}

			if (pz[1] <= izi)
			{
				pz[1] = izi;
				pdest[1] = color;
			}

			if (pz[2] <= izi)
			{
				pz[2] = izi;
				pdest[2] = color;
			}

			if (pz[3] <= izi)
			{
				pz[3] = izi;
				pdest[3] = color;
			}
		}
		break;

	default:
		count = pix << d_y_aspect_shift;

		for ( ; count ; count--, pz -= ScreenWidth, pdest -= ScreenWidth)
		{
			for (i=0 ; i<pix ; i++)
			{
				if (pz[i] <= izi)
				{
					pz[i] = izi;
					pdest[i] = color;
				}
			}
		}
		break;
	}
}

//==========================================================================
//
//	D_DrawParticle_32
//
//==========================================================================

extern "C" void D_DrawParticle_32(particle_t *pparticle)
{
	TVec	local, transformed;
	float	zi;
	dword	*pdest;
	short	*pz;
	int		i, izi, pix, count, u, v, color;

	// transform point
	local = pparticle->org - vieworg;

	transformed.x = DotProduct(local, r_pright);
	transformed.y = DotProduct(local, r_pup);
	transformed.z = DotProduct(local, r_ppn);

	if (transformed.z < PARTICLE_Z_CLIP)
		return;

	// project the point
	// FIXME: preadjust xcenter and ycenter
	zi = 1.0 / transformed.z;
	u = (int)(centerxfrac + zi * transformed.x + 0.5);
	v = (int)(centeryfrac + zi * transformed.y + 0.5);

	if ((v > d_particle_top) || 
		(u > d_particle_right) ||
		(v < 0) ||
		(u < 0))
	{
		return;
	}

	byte r = (pparticle->color >> 16) & 0xff;
	byte g = (pparticle->color >> 8) & 0xff;
	byte b = pparticle->color & 0xff;
	color =	MakeCol32(r, g, b);

	pz = zbuffer + ylookup[v] + u;
	pdest = (dword*)scrn + ylookup[v] + u;
	izi = (int)(zi * 0x8000);

	pix = izi >> d_pix_shift;

	if (pix < d_pix_min)
		pix = d_pix_min;
	else if (pix > d_pix_max)
		pix = d_pix_max;

	switch (pix)
	{
	case 1:
		count = 1 << d_y_aspect_shift;

		for ( ; count ; count--, pz -= ScreenWidth, pdest -= ScreenWidth)
		{
			if (pz[0] <= izi)
			{
				pz[0] = izi;
				pdest[0] = color;
			}
		}
		break;

	case 2:
		count = 2 << d_y_aspect_shift;

		for ( ; count ; count--, pz -= ScreenWidth, pdest -= ScreenWidth)
		{
			if (pz[0] <= izi)
			{
				pz[0] = izi;
				pdest[0] = color;
			}

			if (pz[1] <= izi)
			{
				pz[1] = izi;
				pdest[1] = color;
			}
		}
		break;

	case 3:
		count = 3 << d_y_aspect_shift;

		for ( ; count ; count--, pz -= ScreenWidth, pdest -= ScreenWidth)
		{
			if (pz[0] <= izi)
			{
				pz[0] = izi;
				pdest[0] = color;
			}

			if (pz[1] <= izi)
			{
				pz[1] = izi;
				pdest[1] = color;
			}

			if (pz[2] <= izi)
			{
				pz[2] = izi;
				pdest[2] = color;
			}
		}
		break;

	case 4:
		count = 4 << d_y_aspect_shift;

		for ( ; count ; count--, pz -= ScreenWidth, pdest -= ScreenWidth)
		{
			if (pz[0] <= izi)
			{
				pz[0] = izi;
				pdest[0] = color;
			}

			if (pz[1] <= izi)
			{
				pz[1] = izi;
				pdest[1] = color;
			}

			if (pz[2] <= izi)
			{
				pz[2] = izi;
				pdest[2] = color;
			}

			if (pz[3] <= izi)
			{
				pz[3] = izi;
				pdest[3] = color;
			}
		}
		break;

	default:
		count = pix << d_y_aspect_shift;

		for ( ; count ; count--, pz -= ScreenWidth, pdest -= ScreenWidth)
		{
			for (i=0 ; i<pix ; i++)
			{
				if (pz[i] <= izi)
				{
					pz[i] = izi;
					pdest[i] = color;
				}
			}
		}
		break;
	}
}

#endif

//==========================================================================
//
//	TSoftwareDrawer::StartParticles
//
//==========================================================================

void TSoftwareDrawer::StartParticles(void)
{
	r_pright = viewright * xscaleshrink;
	r_pup = viewup * yscaleshrink;
	r_ppn = viewforward;
}

//==========================================================================
//
//	TSoftwareDrawer::DrawParticle
//
//==========================================================================

void TSoftwareDrawer::DrawParticle(particle_t *pparticle)
{
	D_DrawParticle(pparticle);
}

//==========================================================================
//
//	TSoftwareDrawer::EndParticles
//
//==========================================================================

void TSoftwareDrawer::EndParticles(void)
{
}

