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
//**	clip routines for drawing Alias models directly to the screen
//**
//**************************************************************************

// HEADER FILES ------------------------------------------------------------

#include "d_local.h"
#include "alias.h"

// MACROS ------------------------------------------------------------------

// TYPES -------------------------------------------------------------------

// EXTERNAL FUNCTION PROTOTYPES --------------------------------------------

// PUBLIC FUNCTION PROTOTYPES ----------------------------------------------

// PRIVATE FUNCTION PROTOTYPES ---------------------------------------------

// EXTERNAL DATA DECLARATIONS ----------------------------------------------

// PUBLIC DATA DEFINITIONS -------------------------------------------------

// PRIVATE DATA DEFINITIONS ------------------------------------------------

static finalvert_t		fv[2][8];
static finalstvert_t	stv[2][8];
static auxvert_t		av[8];

// CODE --------------------------------------------------------------------

//==========================================================================
//
//	R_Alias_clip_z
//
//	pfv0 is the unclipped vertex, pfv1 is the z-clipped vertex
//
//==========================================================================

static void R_Alias_clip_z(finalvert_t *pfv0, finalvert_t *pfv1, finalvert_t *out,
	finalstvert_t *pst0, finalstvert_t *pst1, finalstvert_t *stout)
{
	float		scale;
	auxvert_t	*pav0, *pav1, avout;

	pav0 = &av[pfv0 - &fv[0][0]];
	pav1 = &av[pfv1 - &fv[0][0]];

	if (pfv0->v >= pfv1->v)
	{
		scale = (ALIAS_Z_CLIP_PLANE - pav0->fv[2]) /
				(pav1->fv[2] - pav0->fv[2]);
	
		avout.fv[0] = pav0->fv[0] + (pav1->fv[0] - pav0->fv[0]) * scale;
		avout.fv[1] = pav0->fv[1] + (pav1->fv[1] - pav0->fv[1]) * scale;
		avout.fv[2] = ALIAS_Z_CLIP_PLANE;
	
		stout->s = (int)(pst0->s + (pst1->s - pst0->s) * scale);
		stout->t = (int)(pst0->t + (pst1->t - pst0->t) * scale);
		out->r = (int)(pfv0->r + (pfv1->r - pfv0->r) * scale);
		out->g = (int)(pfv0->g + (pfv1->g - pfv0->g) * scale);
		out->b = (int)(pfv0->b + (pfv1->b - pfv0->b) * scale);
	}
	else
	{
		scale = (ALIAS_Z_CLIP_PLANE - pav1->fv[2]) /
				(pav0->fv[2] - pav1->fv[2]);
	
		avout.fv[0] = pav1->fv[0] + (pav0->fv[0] - pav1->fv[0]) * scale;
		avout.fv[1] = pav1->fv[1] + (pav0->fv[1] - pav1->fv[1]) * scale;
		avout.fv[2] = ALIAS_Z_CLIP_PLANE;
	
		stout->s = (int)(pst1->s + (pst0->s - pst1->s) * scale);
		stout->t = (int)(pst1->t + (pst0->t - pst1->t) * scale);
		out->r = (int)(pfv1->r + (pfv0->r - pfv1->r) * scale);
		out->g = (int)(pfv1->g + (pfv0->g - pfv1->g) * scale);
		out->b = (int)(pfv1->b + (pfv0->b - pfv1->b) * scale);
	}

	R_AliasProjectFinalVert(out, &avout);

	if (out->u < 0)
		out->flags |= ALIAS_LEFT_CLIP;
	if (out->v < 0)
		out->flags |= ALIAS_TOP_CLIP;
	if (out->u > viewwidth)
		out->flags |= ALIAS_RIGHT_CLIP;
	if (out->v > viewheight)
		out->flags |= ALIAS_BOTTOM_CLIP;
}

//==========================================================================
//
//	R_Alias_clip_left
//
//==========================================================================

static void R_Alias_clip_left(finalvert_t *pfv0, finalvert_t *pfv1, finalvert_t *out,
	finalstvert_t *pst0, finalstvert_t *pst1, finalstvert_t *stout)
{
	float		scale;

	if (pfv0->v >= pfv1->v)
	{
		scale = (float)(0 - pfv0->u) /
				(pfv1->u - pfv0->u);
		out->u = (int)(pfv0->u + (pfv1->u - pfv0->u)*scale + 0.5);
		out->v = (int)(pfv0->v + (pfv1->v - pfv0->v)*scale + 0.5);
		stout->s = (int)(pst0->s + (pst1->s - pst0->s)*scale + 0.5);
		stout->t = (int)(pst0->t + (pst1->t - pst0->t)*scale + 0.5);
		out->zi = (int)(pfv0->zi + (pfv1->zi - pfv0->zi)*scale + 0.5);
		out->r = (int)(pfv0->r + (pfv1->r - pfv0->r)*scale + 0.5);
		out->g = (int)(pfv0->g + (pfv1->g - pfv0->g)*scale + 0.5);
		out->b = (int)(pfv0->b + (pfv1->b - pfv0->b)*scale + 0.5);
	}
	else
	{
		scale = (float)(0 - pfv1->u) /
				(pfv0->u - pfv1->u);
		out->u = (int)(pfv1->u + (pfv0->u - pfv1->u)*scale + 0.5);
		out->v = (int)(pfv1->v + (pfv0->v - pfv1->v)*scale + 0.5);
		stout->s = (int)(pst1->s + (pst0->s - pst1->s)*scale + 0.5);
		stout->t = (int)(pst1->t + (pst0->t - pst1->t)*scale + 0.5);
		out->zi = (int)(pfv1->zi + (pfv0->zi - pfv1->zi)*scale + 0.5);
		out->r = (int)(pfv1->r + (pfv0->r - pfv1->r)*scale + 0.5);
		out->g = (int)(pfv1->g + (pfv0->g - pfv1->g)*scale + 0.5);
		out->b = (int)(pfv1->b + (pfv0->b - pfv1->b)*scale + 0.5);
	}
}

//==========================================================================
//
//	R_Alias_clip_right
//
//==========================================================================

static void R_Alias_clip_right(finalvert_t *pfv0, finalvert_t *pfv1, finalvert_t *out,
	finalstvert_t *pst0, finalstvert_t *pst1, finalstvert_t *stout)
{
	float		scale;

	if (pfv0->v >= pfv1->v)
	{
		scale = (float)(viewwidth - pfv0->u) /
				(pfv1->u - pfv0->u);
		out->u = (int)(pfv0->u + (pfv1->u - pfv0->u)*scale + 0.5);
		out->v = (int)(pfv0->v + (pfv1->v - pfv0->v)*scale + 0.5);
		stout->s = (int)(pst0->s + (pst1->s - pst0->s)*scale + 0.5);
		stout->t = (int)(pst0->t + (pst1->t - pst0->t)*scale + 0.5);
		out->zi = (int)(pfv0->zi + (pfv1->zi - pfv0->zi)*scale + 0.5);
		out->r = (int)(pfv0->r + (pfv1->r - pfv0->r)*scale + 0.5);
		out->g = (int)(pfv0->g + (pfv1->g - pfv0->g)*scale + 0.5);
		out->b = (int)(pfv0->b + (pfv1->b - pfv0->b)*scale + 0.5);
	}
	else
	{
		scale = (float)(viewwidth - pfv1->u) /
				(pfv0->u - pfv1->u);
		out->u = (int)(pfv1->u + (pfv0->u - pfv1->u)*scale + 0.5);
		out->v = (int)(pfv1->v + (pfv0->v - pfv1->v)*scale + 0.5);
		stout->s = (int)(pst1->s + (pst0->s - pst1->s)*scale + 0.5);
		stout->t = (int)(pst1->t + (pst0->t - pst1->t)*scale + 0.5);
		out->zi = (int)(pfv1->zi + (pfv0->zi - pfv1->zi)*scale + 0.5);
		out->r = (int)(pfv1->r + (pfv0->r - pfv1->r)*scale + 0.5);
		out->g = (int)(pfv1->g + (pfv0->g - pfv1->g)*scale + 0.5);
		out->b = (int)(pfv1->b + (pfv0->b - pfv1->b)*scale + 0.5);
	}
}

//==========================================================================
//
//	R_Alias_clip_top
//
//==========================================================================

static void R_Alias_clip_top(finalvert_t *pfv0, finalvert_t *pfv1, finalvert_t *out,
	finalstvert_t *pst0, finalstvert_t *pst1, finalstvert_t *stout)
{
	float		scale;

	if (pfv0->v >= pfv1->v)
	{
		scale = (float)(0 - pfv0->v) /
				(pfv1->v - pfv0->v);
		out->u = (int)(pfv0->u + (pfv1->u - pfv0->u)*scale + 0.5);
		out->v = (int)(pfv0->v + (pfv1->v - pfv0->v)*scale + 0.5);
		stout->s = (int)(pst0->s + (pst1->s - pst0->s)*scale + 0.5);
		stout->t = (int)(pst0->t + (pst1->t - pst0->t)*scale + 0.5);
		out->zi = (int)(pfv0->zi + (pfv1->zi - pfv0->zi)*scale + 0.5);
		out->r = (int)(pfv0->r + (pfv1->r - pfv0->r)*scale + 0.5);
		out->g = (int)(pfv0->g + (pfv1->g - pfv0->g)*scale + 0.5);
		out->b = (int)(pfv0->b + (pfv1->b - pfv0->b)*scale + 0.5);
	}
	else
	{
		scale = (float)(0 - pfv1->v) /
				(pfv0->v - pfv1->v);
		out->u = (int)(pfv1->u + (pfv0->u - pfv1->u)*scale + 0.5);
		out->v = (int)(pfv1->v + (pfv0->v - pfv1->v)*scale + 0.5);
		stout->s = (int)(pst1->s + (pst0->s - pst1->s)*scale + 0.5);
		stout->t = (int)(pst1->t + (pst0->t - pst1->t)*scale + 0.5);
		out->zi = (int)(pfv1->zi + (pfv0->zi - pfv1->zi)*scale + 0.5);
		out->r = (int)(pfv1->r + (pfv0->r - pfv1->r)*scale + 0.5);
		out->g = (int)(pfv1->g + (pfv0->g - pfv1->g)*scale + 0.5);
		out->b = (int)(pfv1->b + (pfv0->b - pfv1->b)*scale + 0.5);
	}
}

//==========================================================================
//
//	R_Alias_clip_bottom
//
//==========================================================================

static void R_Alias_clip_bottom(finalvert_t *pfv0, finalvert_t *pfv1, finalvert_t *out,
	finalstvert_t *pst0, finalstvert_t *pst1, finalstvert_t *stout)
{
	float		scale;

	if (pfv0->v >= pfv1->v)
	{
		scale = (float)(viewheight - pfv0->v) /
				(pfv1->v - pfv0->v);

		out->u = (int)(pfv0->u + (pfv1->u - pfv0->u)*scale + 0.5);
		out->v = (int)(pfv0->v + (pfv1->v - pfv0->v)*scale + 0.5);
		stout->s = (int)(pst0->s + (pst1->s - pst0->s)*scale + 0.5);
		stout->t = (int)(pst0->t + (pst1->t - pst0->t)*scale + 0.5);
		out->zi = (int)(pfv0->zi + (pfv1->zi - pfv0->zi)*scale + 0.5);
		out->r = (int)(pfv0->r + (pfv1->r - pfv0->r)*scale + 0.5);
		out->g = (int)(pfv0->g + (pfv1->g - pfv0->g)*scale + 0.5);
		out->b = (int)(pfv0->b + (pfv1->b - pfv0->b)*scale + 0.5);
	}
	else
	{
		scale = (float)(viewheight - pfv1->v) /
				(pfv0->v - pfv1->v);

		out->u = (int)(pfv1->u + (pfv0->u - pfv1->u)*scale + 0.5);
		out->v = (int)(pfv1->v + (pfv0->v - pfv1->v)*scale + 0.5);
		stout->s = (int)(pst1->s + (pst0->s - pst1->s)*scale + 0.5);
		stout->t = (int)(pst1->t + (pst0->t - pst1->t)*scale + 0.5);
		out->zi = (int)(pfv1->zi + (pfv0->zi - pfv1->zi)*scale + 0.5);
		out->r = (int)(pfv1->r + (pfv0->r - pfv1->r)*scale + 0.5);
		out->g = (int)(pfv1->g + (pfv0->g - pfv1->g)*scale + 0.5);
		out->b = (int)(pfv1->b + (pfv0->b - pfv1->b)*scale + 0.5);
	}
}

//==========================================================================
//
//	R_AliasClip
//
//==========================================================================

static int R_AliasClip(finalvert_t *in, finalvert_t *out, finalstvert_t *stin,
	finalstvert_t *stout, int flag, int count,
	void(*clip)(finalvert_t *pfv0, finalvert_t *pfv1, finalvert_t *out,
	finalstvert_t *pst0, finalstvert_t *pst1, finalstvert_t *stout) )
{
	int			i,j,k;
	int			flags, oldflags;
	
	j = count-1;
	k = 0;
	for (i=0 ; i<count ; j = i, i++)
	{
		oldflags = in[j].flags & flag;
		flags = in[i].flags & flag;

		if (flags && oldflags)
			continue;
		if (oldflags ^ flags)
		{
			clip(&in[j], &in[i], &out[k], &stin[j], &stin[i], &stout[k]);
			out[k].flags = 0;
			if (out[k].u < 0)
				out[k].flags |= ALIAS_LEFT_CLIP;
			if (out[k].v < 0)
				out[k].flags |= ALIAS_TOP_CLIP;
			if (out[k].u > viewwidth)
				out[k].flags |= ALIAS_RIGHT_CLIP;
			if (out[k].v > viewheight)
				out[k].flags |= ALIAS_BOTTOM_CLIP;
			k++;
		}
		if (!flags)
		{
			out[k] = in[i];
			stout[k] = stin[i];
			k++;
		}
	}
	
	return k;
}

//==========================================================================
//
//	R_AliasClipTriangle
//
//==========================================================================

void R_AliasClipTriangle(mtriangle_t *ptri)
{
	int				i, k, pingpong;
	mtriangle_t		mtri;
	unsigned		clipflags;

	// copy vertexes
	fv[0][0] = pfinalverts[ptri->vertindex[0]];
	fv[0][1] = pfinalverts[ptri->vertindex[1]];
	fv[0][2] = pfinalverts[ptri->vertindex[2]];
	stv[0][0] = pfinalstverts[ptri->stvertindex[0]];
	stv[0][1] = pfinalstverts[ptri->stvertindex[1]];
	stv[0][2] = pfinalstverts[ptri->stvertindex[2]];

	// clip
	clipflags = fv[0][0].flags | fv[0][1].flags | fv[0][2].flags;

	if (clipflags & ALIAS_Z_CLIP)
	{
		for (i=0 ; i<3 ; i++)
			av[i] = pauxverts[ptri->vertindex[i]];

		k = R_AliasClip(fv[0], fv[1], stv[0], stv[1], ALIAS_Z_CLIP, 3, R_Alias_clip_z);
		if (k == 0)
			return;

		pingpong = 1;
		clipflags = fv[1][0].flags | fv[1][1].flags | fv[1][2].flags;
	}
	else
	{
		pingpong = 0;
		k = 3;
	}

	if (clipflags & ALIAS_LEFT_CLIP)
	{
		k = R_AliasClip(fv[pingpong], fv[pingpong ^ 1], stv[pingpong],
			stv[pingpong ^ 1], ALIAS_LEFT_CLIP, k, R_Alias_clip_left);
		if (k == 0)
			return;

		pingpong ^= 1;
	}

	if (clipflags & ALIAS_RIGHT_CLIP)
	{
		k = R_AliasClip(fv[pingpong], fv[pingpong ^ 1], stv[pingpong],
			stv[pingpong ^ 1], ALIAS_RIGHT_CLIP, k, R_Alias_clip_right);
		if (k == 0)
			return;

		pingpong ^= 1;
	}

	if (clipflags & ALIAS_BOTTOM_CLIP)
	{
		k = R_AliasClip(fv[pingpong], fv[pingpong ^ 1], stv[pingpong],
			stv[pingpong ^ 1], ALIAS_BOTTOM_CLIP, k, R_Alias_clip_bottom);
		if (k == 0)
			return;

		pingpong ^= 1;
	}

	if (clipflags & ALIAS_TOP_CLIP)
	{
		k = R_AliasClip(fv[pingpong], fv[pingpong ^ 1], stv[pingpong],
			stv[pingpong ^ 1], ALIAS_TOP_CLIP, k, R_Alias_clip_top);
		if (k == 0)
			return;

		pingpong ^= 1;
	}

	for (i=0 ; i<k ; i++)
	{
		if (fv[pingpong][i].u < 0)
			fv[pingpong][i].u = 0;
		else if (fv[pingpong][i].u > viewwidth)
			fv[pingpong][i].u = viewwidth;

		if (fv[pingpong][i].v < 0)
			fv[pingpong][i].v = 0;
		else if (fv[pingpong][i].v > viewheight)
			fv[pingpong][i].v = viewheight;

		fv[pingpong][i].flags = 0;
	}

	// draw triangles
	r_affinetridesc.ptriangles = &mtri;
	r_affinetridesc.pfinalverts = fv[pingpong];
	r_affinetridesc.pstverts = stv[pingpong];

	// FIXME: do all at once as trifan?
	mtri.vertindex[0] = 0;
	mtri.stvertindex[0] = 0;
	for (i=1 ; i<k-1 ; i++)
	{
		mtri.vertindex[1] = i;
		mtri.vertindex[2] = i+1;
		mtri.stvertindex[1] = i;
		mtri.stvertindex[2] = i+1;
		D_PolysetDraw();
	}
}

//**************************************************************************
//
//	$Log$
//	Revision 1.4  2001/08/02 17:45:37  dj_jl
//	Added support for colored lit and translucent models
//
//	Revision 1.3  2001/07/31 17:16:30  dj_jl
//	Just moved Log to the end of file
//	
//	Revision 1.2  2001/07/27 14:27:54  dj_jl
//	Update with Id-s and Log-s, some fixes
//
//**************************************************************************
