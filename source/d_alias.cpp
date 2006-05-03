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
//**	routines for setting up to draw alias models
//**
//**************************************************************************

// HEADER FILES ------------------------------------------------------------

#include "d_local.h"

// MACROS ------------------------------------------------------------------

#define VID_CBITS			5
#define VID_GRADES			32

#define LIGHT_MIN			5	//	lowest light value we'll allow, to avoid
								// the need for inner-loop light clamping
#define NUMVERTEXNORMALS	162

// TYPES -------------------------------------------------------------------

struct aedge_t
{
	int			index0;
	int			index1;
};

// EXTERNAL FUNCTION PROTOTYPES --------------------------------------------

// PUBLIC FUNCTION PROTOTYPES ----------------------------------------------

// PRIVATE FUNCTION PROTOTYPES ---------------------------------------------

extern "C" {
void D_ConcatTransforms(float in1[3][4], float in2[3][4], float out[3][4]);
void D_AliasTransformFinalVert(finalvert_t*, auxvert_t*, trivertx_t*);
void D_AliasTransformAndProjectFinalVerts(finalvert_t*);
void D_AliasProjectFinalVert(finalvert_t*, auxvert_t*);
}

// EXTERNAL DATA DECLARATIONS ----------------------------------------------

// PUBLIC DATA DEFINITIONS -------------------------------------------------

affinetridesc_t	d_affinetridesc;

finalvert_t		*pfinalverts;
finalstvert_t	*pfinalstverts;
auxvert_t		*pauxverts;

extern "C" {

trivertx_t		*d_apverts;
int				d_anumverts;

float			aliastransform[3][4];
float			aliasxcenter;
float			aliasycenter;
float			ziscale;

TVec			d_plightvec;
int				d_ambientlightr;
int				d_ambientlightg;
int				d_ambientlightb;
float			d_shadelightr;
float			d_shadelightg;
float			d_shadelightb;

float d_avertexnormals[NUMVERTEXNORMALS][3] =
{
#include "anorms.h"
};

}

// PRIVATE DATA DEFINITIONS ------------------------------------------------

static TVec				modelorg;

static mmdl_t			*pmdl;
static int				a_trivial_accept;

static TVec				alias_forward, alias_right, alias_up;

static int				d_amodels_drawn;

static aedge_t aedges[12] =
{
	{0, 1}, {1, 2}, {2, 3}, {3, 0},
	{4, 5}, {5, 6}, {6, 7}, {7, 4},
	{0, 5}, {1, 4}, {2, 7}, {3, 6}
};

// CODE --------------------------------------------------------------------

//==========================================================================
//
//	VSoftwareDrawer::DrawAliasModel
//
//==========================================================================

void VSoftwareDrawer::DrawAliasModel(const TVec &origin, const TAVec &angles,
	VModel* model, int frame, int skin_index, const char *skin, dword light,
	int translucency, bool is_view_model)
{
	guard(VSoftwareDrawer::DrawAliasModel);
	modelorg = vieworg - origin;

	// see if the bounding box lets us trivially reject, also sets
	// trivial accept status
	if (!AliasCheckBBox(model, angles, frame))
	{
		return;
	}

	finalvert_t		finalverts[MAXALIASVERTS +
						((CACHE_SIZE - 1) / sizeof(finalvert_t)) + 1];
	finalstvert_t	finalstverts[MAXALIASSTVERTS +
						((CACHE_SIZE - 1) / sizeof(finalstvert_t)) + 1];
	auxvert_t		auxverts[MAXALIASVERTS];

	d_amodels_drawn++;

	// cache align
	pfinalverts = (finalvert_t *)
			(((long)&finalverts[0] + CACHE_SIZE - 1) & ~(CACHE_SIZE - 1));
	pfinalstverts = (finalstvert_t *)
			(((long)&finalstverts[0] + CACHE_SIZE - 1) & ~(CACHE_SIZE - 1));
	pauxverts = &auxverts[0];

	pmdl = (mmdl_t*)Mod_Extradata(model);

	AliasSetupSkin(skin_index, skin);
	AliasSetUpTransform(angles, frame, a_trivial_accept);
	AliasSetupLighting(light);
	AliasSetupFrame(frame);

	PolysetSetupDrawer(translucency);

	// hack the depth range to prevent view model from poking into walls
	if (is_view_model)
		ziscale = (float)0x8000 * (float)0x10000 * 3.0;
	else
		ziscale = (float)0x8000 * (float)0x10000;

	if (a_trivial_accept)
		AliasPrepareUnclippedPoints();
	else
		AliasPreparePoints();
	unguard;
}

//==========================================================================
//
//	VSoftwareDrawer::AliasCheckBBox
//
//==========================================================================

bool VSoftwareDrawer::AliasCheckBBox(VModel* model, const TAVec &angles, int frame)
{
	int					i, flags, numv;
	float				zi, basepts[8][3], v0, v1, frac;
	finalvert_t			*pv0, *pv1, viewpts[16];
	auxvert_t			*pa0, *pa1, viewaux[16];
	boolean				zclipped, zfullyclipped;
	unsigned			anyclip, allclip;
	int					minz;
	
// expand, rotate, and translate points into worldspace

	a_trivial_accept = 0;
	pmdl = (mmdl_t *)Mod_Extradata(model);

// construct the base bounding box for this frame
	AliasSetUpTransform(angles, frame, 0);

// x worldspace coordinates
	basepts[0][0] = basepts[1][0] = basepts[2][0] = basepts[3][0] = 0;
	basepts[4][0] = basepts[5][0] = basepts[6][0] = basepts[7][0] = 255;

// y worldspace coordinates
	basepts[0][1] = basepts[3][1] = basepts[5][1] = basepts[6][1] = 0;
	basepts[1][1] = basepts[2][1] = basepts[4][1] = basepts[7][1] = 255;

// z worldspace coordinates
	basepts[0][2] = basepts[1][2] = basepts[4][2] = basepts[5][2] = 0;
	basepts[2][2] = basepts[3][2] = basepts[6][2] = basepts[7][2] = 255;

	zclipped = false;
	zfullyclipped = true;

	minz = 9999;
	for (i = 0; i < 8; i++)
	{
		viewaux[i].fv[0] = DotProduct(basepts[i], aliastransform[0]) + aliastransform[0][3];
		viewaux[i].fv[1] = DotProduct(basepts[i], aliastransform[1]) + aliastransform[1][3];
		viewaux[i].fv[2] = DotProduct(basepts[i], aliastransform[2]) + aliastransform[2][3];

		if (viewaux[i].fv[2] < ALIAS_Z_CLIP_PLANE)
		{
		// we must clip points that are closer than the near clip plane
			viewpts[i].flags = ALIAS_Z_CLIP;
			zclipped = true;
		}
		else
		{
			if (viewaux[i].fv[2] < minz)
				minz = (int)viewaux[i].fv[2];
			viewpts[i].flags = 0;
			zfullyclipped = false;
		}
	}

	
	if (zfullyclipped)
	{
		return false;	// everything was near-z-clipped
	}

	numv = 8;

	if (zclipped)
	{
	// organize points by edges, use edges to get new points (possible trivial
	// reject)
		for (i=0 ; i<12 ; i++)
		{
		// edge endpoints
			pv0 = &viewpts[aedges[i].index0];
			pv1 = &viewpts[aedges[i].index1];
			pa0 = &viewaux[aedges[i].index0];
			pa1 = &viewaux[aedges[i].index1];

		// if one end is clipped and the other isn't, make a new point
			if (pv0->flags ^ pv1->flags)
			{
				frac = (ALIAS_Z_CLIP_PLANE - pa0->fv[2]) /
					   (pa1->fv[2] - pa0->fv[2]);
				viewaux[numv].fv[0] = pa0->fv[0] +
						(pa1->fv[0] - pa0->fv[0]) * frac;
				viewaux[numv].fv[1] = pa0->fv[1] +
						(pa1->fv[1] - pa0->fv[1]) * frac;
				viewaux[numv].fv[2] = ALIAS_Z_CLIP_PLANE;
				viewpts[numv].flags = 0;
				numv++;
			}
		}
	}

// project the vertices that remain after clipping
	anyclip = 0;
	allclip = ALIAS_XY_CLIP_MASK;

// TODO: probably should do this loop in ASM, especially if we use floats
	for (i=0 ; i<numv ; i++)
	{
	// we don't need to bother with vertices that were z-clipped
		if (viewpts[i].flags & ALIAS_Z_CLIP)
			continue;

		zi = 1.0 / viewaux[i].fv[2];

	// FIXME: do with chop mode in ASM, or convert to float
		v0 = (viewaux[i].fv[0] * xprojection * zi) + aliasxcenter;
		v1 = (viewaux[i].fv[1] * yprojection * zi) + aliasycenter;

		flags = 0;

		if (v0 < 0)
			flags |= ALIAS_LEFT_CLIP;
		if (v1 < 0)
			flags |= ALIAS_TOP_CLIP;
		if (v0 > viewwidth)
			flags |= ALIAS_RIGHT_CLIP;
		if (v1 > viewheight)
			flags |= ALIAS_BOTTOM_CLIP;

		anyclip |= flags;
		allclip &= flags;
	}

	if (allclip)
		return false;	// trivial reject off one side

	a_trivial_accept = !anyclip & !zclipped;

	return true;
}

//==========================================================================
//
//	VSoftwareDrawer::AliasSetUpTransform
//
//==========================================================================

void VSoftwareDrawer::AliasSetUpTransform(const TAVec &angles, int frame, int trivial_accept)
{
	int				i;
	float			rotationmatrix[3][4], t2matrix[3][4];
	static float	tmatrix[3][4];
	static float	viewmatrix[3][4];
	mframe_t		*pframedesc;

// TODO: should really be stored with the entity instead of being reconstructed
// TODO: should use a look-up table
// TODO: could cache lazily, stored in the entity

	AngleVectors(angles, alias_forward, alias_right, alias_up);

// TODO: don't repeat this check when drawing?
	if ((frame >= pmdl->numframes) || (frame < 0))
	{
		GCon->Logf(NAME_Dev, "No such frame %d", frame);
		frame = 0;
	}

	pframedesc = (mframe_t *)((byte*)pmdl + pmdl->ofsframes +
		frame * pmdl->framesize);

	tmatrix[0][0] = pframedesc->scale[0];
	tmatrix[1][1] = pframedesc->scale[1];
	tmatrix[2][2] = pframedesc->scale[2];

	tmatrix[0][3] = pframedesc->scale_origin[0];
	tmatrix[1][3] = pframedesc->scale_origin[1];
	tmatrix[2][3] = pframedesc->scale_origin[2];

// TODO: can do this with simple matrix rearrangement

	for (i = 0; i < 3; i++)
	{
		t2matrix[i][0] = alias_forward[i];
		t2matrix[i][1] = -alias_right[i];
		t2matrix[i][2] = alias_up[i];
	}

	t2matrix[0][3] = -modelorg[0];
	t2matrix[1][3] = -modelorg[1];
	t2matrix[2][3] = -modelorg[2];

// FIXME: can do more efficiently than full concatenation
	D_ConcatTransforms(t2matrix, tmatrix, rotationmatrix);

// TODO: should be global, set when vright, etc., set
	for (i = 0; i < 3; i++)
	{
		viewmatrix[0][i] = viewright[i];
		viewmatrix[1][i] = viewup[i];
		viewmatrix[2][i] = viewforward[i];
	}

//	viewmatrix[0][3] = 0;
//	viewmatrix[1][3] = 0;
//	viewmatrix[2][3] = 0;

	D_ConcatTransforms(viewmatrix, rotationmatrix, aliastransform);

// do the scaling up of x and y to screen coordinates as part of the transform
// for the unclipped case (it would mess up clipping in the clipped case).
// Also scale down z, so 1/z is scaled 31 bits for free, and scale down x and y
// correspondingly so the projected x and y come out right
// FIXME: make this work for clipped case too?
	if (trivial_accept)
	{
		for (i=0 ; i<4 ; i++)
		{
			aliastransform[0][i] *= xprojection *
					(1.0 / ((float)0x8000 * 0x10000));
			aliastransform[1][i] *= yprojection *
					(1.0 / ((float)0x8000 * 0x10000));
			aliastransform[2][i] *= 1.0 / ((float)0x8000 * 0x10000);

		}
	}
}

//==========================================================================
//
//	VSoftwareDrawer::AliasSetupSkin
//
//==========================================================================

void VSoftwareDrawer::AliasSetupSkin(int skin_index, const char *skin)
{
	int SkinID;
	if (skin && *skin)
	{
		SkinID = GTextureManager.AddFileTexture(VName(skin), TEXTYPE_Skin);
	}
	else
	{
		mskin_t* pskins = (mskin_t *)((byte *)pmdl + pmdl->ofsskins);
		if (skin_index < 0 || skin_index >= pmdl->numskins)
			SkinID = GTextureManager.AddFileTexture(VName(pskins[0].name), TEXTYPE_Skin);
		else
			SkinID = GTextureManager.AddFileTexture(VName(pskins[skin_index].name), TEXTYPE_Skin);
	}
	d_affinetridesc.pskin = SetPic(SkinID);
	d_affinetridesc.skinwidth = pmdl->skinwidth;
	d_affinetridesc.skinheight = pmdl->skinheight;
}

//==========================================================================
//
//	VSoftwareDrawer::AliasSetupLighting
//
//==========================================================================

void VSoftwareDrawer::AliasSetupLighting(dword light)
{
	//	Guarantee that no vertex will ever be lit below LIGHT_MIN, so we
	// don't have to clamp off the bottom
	d_ambientlightr = (light >> 17) & 0x7f;

	if (d_ambientlightr < LIGHT_MIN)
		d_ambientlightr = LIGHT_MIN;

	d_ambientlightr = (255 - d_ambientlightr) << VID_CBITS;

	if (d_ambientlightr < LIGHT_MIN)
		d_ambientlightr = LIGHT_MIN;

	d_shadelightr = (light >> 17) & 0x7f;

	if (d_shadelightr < 0)
		d_shadelightr = 0;

	d_shadelightr *= VID_GRADES;


	d_ambientlightg = (light >> 9) & 0x7f;

	if (d_ambientlightg < LIGHT_MIN)
		d_ambientlightg = LIGHT_MIN;

	d_ambientlightg = (255 - d_ambientlightg) << VID_CBITS;

	if (d_ambientlightg < LIGHT_MIN)
		d_ambientlightg = LIGHT_MIN;

	d_shadelightg = (light >> 9) & 0x7f;

	if (d_shadelightg < 0)
		d_shadelightg = 0;

	d_shadelightg *= VID_GRADES;


	d_ambientlightb = (light >> 1) & 0x7f;

	if (d_ambientlightb < LIGHT_MIN)
		d_ambientlightb = LIGHT_MIN;

	d_ambientlightb = (255 - d_ambientlightb) << VID_CBITS;

	if (d_ambientlightb < LIGHT_MIN)
		d_ambientlightb = LIGHT_MIN;

	d_shadelightb = (light >> 1) & 0x7f;

	if (d_shadelightb < 0)
		d_shadelightb = 0;

	d_shadelightb *= VID_GRADES;


	d_affinetridesc.coloredlight = (d_ambientlightr != d_ambientlightg) ||
		(d_ambientlightr != d_ambientlightb);

// FIXME: remove and do real lighting
	TVec		lightvec(-1, 0, 0);

// rotate the lighting vector into the model's frame of reference
	d_plightvec[0] = DotProduct(lightvec, alias_forward);
	d_plightvec[1] = DotProduct(lightvec, alias_right);
	d_plightvec[2] = DotProduct(lightvec, alias_up);
}

//==========================================================================
//
//	VSoftwareDrawer::AliasSetupFrame
//
//	set	d_apverts
//
//==========================================================================

void VSoftwareDrawer::AliasSetupFrame(int frame)
{
	if ((frame >= pmdl->numframes) || (frame < 0))
	{
		GCon->Logf(NAME_Dev, "D_AliasSetupFrame: no such frame %d", frame);
		frame = 0;
	}

	d_apverts = (trivertx_t *)((byte*)pmdl + pmdl->ofsframes +
		frame * pmdl->framesize + sizeof(mframe_t));
}

//==========================================================================
//
//	VSoftwareDrawer::AliasPrepareUnclippedPoints
//
//==========================================================================

void VSoftwareDrawer::AliasPrepareUnclippedPoints()
{
	mstvert_t	*pstverts;
	finalvert_t	*fv;

	pstverts = (mstvert_t *)((byte *)pmdl + pmdl->ofsstverts);
	for (int i = 0; i < pmdl->numstverts; i++)
	{
		pfinalstverts[i].s = pstverts[i].s << 16;
		pfinalstverts[i].t = pstverts[i].t << 16;
	}
	d_anumverts = pmdl->numverts;
// FIXME: just use pfinalverts directly?
	fv = pfinalverts;

	D_AliasTransformAndProjectFinalVerts (fv);

	d_affinetridesc.pfinalverts = pfinalverts;
	d_affinetridesc.pstverts = pfinalstverts;
	d_affinetridesc.ptriangles = (mtriangle_t *)
		((byte *)pmdl + pmdl->ofstris);
	d_affinetridesc.numtriangles = pmdl->numtris;

	PolysetDraw();
}

//==========================================================================
//
//	VSoftwareDrawer::AliasPreparePoints
//
//	General clipped case
//
//==========================================================================

void VSoftwareDrawer::AliasPreparePoints()
{
	int			i;
	mstvert_t	*pstverts;
	finalvert_t	*fv;
	auxvert_t	*av;
	mtriangle_t	*ptri;
	finalvert_t	*pfv[3];

	pstverts = (mstvert_t *)((byte *)pmdl + pmdl->ofsstverts);
	for (i = 0; i < pmdl->numstverts; i++)
	{
		pfinalstverts[i].s = pstverts[i].s << 16;
		pfinalstverts[i].t = pstverts[i].t << 16;
	}
	d_anumverts = pmdl->numverts;
 	fv = pfinalverts;
	av = pauxverts;

	for (i = 0; i < d_anumverts; i++, fv++, av++, d_apverts++)
	{
		D_AliasTransformFinalVert (fv, av, d_apverts);
		if (av->fv[2] < ALIAS_Z_CLIP_PLANE)
			fv->flags |= ALIAS_Z_CLIP;
		else
		{
			 D_AliasProjectFinalVert (fv, av);

			if (fv->u < 0)
				fv->flags |= ALIAS_LEFT_CLIP;
			if (fv->v < 0)
				fv->flags |= ALIAS_TOP_CLIP;
			if (fv->u > viewwidth)
				fv->flags |= ALIAS_RIGHT_CLIP;
			if (fv->v > viewheight)
				fv->flags |= ALIAS_BOTTOM_CLIP;
		}
	}

//
// clip and draw all triangles
//
	d_affinetridesc.numtriangles = 1;

	ptri = (mtriangle_t*)((byte*)pmdl + pmdl->ofstris);
	for (i = 0; i < pmdl->numtris; i++, ptri++)
	{
		pfv[0] = &pfinalverts[ptri->vertindex[0]];
		pfv[1] = &pfinalverts[ptri->vertindex[1]];
		pfv[2] = &pfinalverts[ptri->vertindex[2]];

		if (pfv[0]->flags & pfv[1]->flags & pfv[2]->flags)
			continue;		// completely clipped
		
		if (!(pfv[0]->flags | pfv[1]->flags | pfv[2]->flags))
		{	// totally unclipped
			d_affinetridesc.pfinalverts = pfinalverts;
			d_affinetridesc.pstverts = pfinalstverts;
			d_affinetridesc.ptriangles = ptri;
			PolysetDraw();
		}
		else		
		{	// partially clipped
			AliasClipTriangle(ptri);
		}
	}
}

#ifndef USEASM

//==========================================================================
//
//	D_ConcatTransforms
//
//==========================================================================

extern "C" void D_ConcatTransforms(float in1[3][4], float in2[3][4], float out[3][4])
{
	out[0][0] = in1[0][0] * in2[0][0] + in1[0][1] * in2[1][0] +
				in1[0][2] * in2[2][0];
	out[0][1] = in1[0][0] * in2[0][1] + in1[0][1] * in2[1][1] +
				in1[0][2] * in2[2][1];
	out[0][2] = in1[0][0] * in2[0][2] + in1[0][1] * in2[1][2] +
				in1[0][2] * in2[2][2];
	out[0][3] = in1[0][0] * in2[0][3] + in1[0][1] * in2[1][3] +
				in1[0][2] * in2[2][3] + in1[0][3];
	out[1][0] = in1[1][0] * in2[0][0] + in1[1][1] * in2[1][0] +
				in1[1][2] * in2[2][0];
	out[1][1] = in1[1][0] * in2[0][1] + in1[1][1] * in2[1][1] +
				in1[1][2] * in2[2][1];
	out[1][2] = in1[1][0] * in2[0][2] + in1[1][1] * in2[1][2] +
				in1[1][2] * in2[2][2];
	out[1][3] = in1[1][0] * in2[0][3] + in1[1][1] * in2[1][3] +
				in1[1][2] * in2[2][3] + in1[1][3];
	out[2][0] = in1[2][0] * in2[0][0] + in1[2][1] * in2[1][0] +
				in1[2][2] * in2[2][0];
	out[2][1] = in1[2][0] * in2[0][1] + in1[2][1] * in2[1][1] +
				in1[2][2] * in2[2][1];
	out[2][2] = in1[2][0] * in2[0][2] + in1[2][1] * in2[1][2] +
				in1[2][2] * in2[2][2];
	out[2][3] = in1[2][0] * in2[0][3] + in1[2][1] * in2[1][3] +
				in1[2][2] * in2[2][3] + in1[2][3];
}

//==========================================================================
//
//	D_AliasTransformAndProjectFinalVerts
//
//==========================================================================

extern "C" void D_AliasTransformAndProjectFinalVerts(finalvert_t *fv)
{
	int			i;
	float		lightcos, *plightnormal, zi;
	trivertx_t	*pverts;

	pverts = d_apverts;

	for (i = 0; i < d_anumverts; i++, fv++, pverts++)
	{
		//	transform and project
		TVec v(pverts->v[0], pverts->v[1], pverts->v[2]);
		zi = 1.0 / (DotProduct(v, aliastransform[2]) +
				aliastransform[2][3]);

		//	x, y, and z are scaled down by 1/2**31 in the transform, so 1/z
		// is scaled up by 1/2**31, and the scaling cancels out for x and y
		// in the projection
		fv->zi = (int)zi;

		fv->u = (int)(((DotProduct(v, aliastransform[0]) +
				aliastransform[0][3]) * zi) + aliasxcenter);
		fv->v = (int)(((DotProduct(v, aliastransform[1]) +
				aliastransform[1][3]) * zi) + aliasycenter);

		fv->flags = 0;

		//	lighting
		plightnormal = d_avertexnormals[pverts->lightnormalindex];
		lightcos = DotProduct(plightnormal, d_plightvec);
		int r = d_ambientlightr;
		int g = d_ambientlightg;
		int b = d_ambientlightb;

		if (lightcos < 0)
		{
			r += (int)(d_shadelightr * lightcos);
			//	Clamp; because we limited the minimum ambient and shading
			// light, we don't have to clamp low light, just bright
			if (r < 0)
				r = 0;
			g += (int)(d_shadelightg * lightcos);
			if (g < 0)
				g = 0;
			b += (int)(d_shadelightb * lightcos);
			if (b < 0)
				b = 0;
		}

		fv->r = r;
		fv->g = g;
		fv->b = b;
	}
}

//==========================================================================
//
//	D_AliasTransformFinalVert
//
//==========================================================================

extern "C" void D_AliasTransformFinalVert(finalvert_t *fv, auxvert_t *av,
	trivertx_t *pverts)
{
	float	lightcos, *plightnormal;

	TVec v(pverts->v[0], pverts->v[1], pverts->v[2]);
	av->fv[0] = DotProduct(v, aliastransform[0]) +
			aliastransform[0][3];
	av->fv[1] = DotProduct(v, aliastransform[1]) +
			aliastransform[1][3];
	av->fv[2] = DotProduct(v, aliastransform[2]) +
			aliastransform[2][3];

	fv->flags = 0;

// lighting
	plightnormal = d_avertexnormals[pverts->lightnormalindex];
	lightcos = DotProduct(plightnormal, d_plightvec);
	int r = d_ambientlightr;
	int g = d_ambientlightg;
	int b = d_ambientlightb;

	if (lightcos < 0)
	{
		r += (int)(d_shadelightr * lightcos);
		g += (int)(d_shadelightg * lightcos);
		b += (int)(d_shadelightb * lightcos);

		//	Clamp; because we limited the minimum ambient and shading light,
		// we don't have to clamp low light, just bright
		if (r < 0)
			r = 0;
		if (g < 0)
			g = 0;
		if (b < 0)
			b = 0;
	}

	fv->r = r;
	fv->g = g;
	fv->b = b;
}

//==========================================================================
//
//	D_AliasProjectFinalVert
//
//==========================================================================

extern "C" void D_AliasProjectFinalVert(finalvert_t *fv, auxvert_t *av)
{
	float	zi;

	// project points
	zi = 1.0 / av->fv[2];

	fv->zi = (int)(zi * ziscale);

	fv->u = (int)((av->fv[0] * xprojection * zi) + aliasxcenter);
	fv->v = (int)((av->fv[1] * yprojection * zi) + aliasycenter);
}

#endif
