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
//**	routines for drawing sets of polygons sharing the same
//**  texture (used for Alias models)
//**
//**************************************************************************

// HEADER FILES ------------------------------------------------------------

#include "d_local.h"

// MACROS ------------------------------------------------------------------

// TODO: put in span spilling to shrink list size
// !!! if this is changed, it must be changed in d_polysa.s too !!!
#define DPS_MAXSPANS			MAXSCREENHEIGHT+1
									// 1 extra for spanpackage that marks end

#define DPS_SPAN_LIST_END		-9999

// TYPES -------------------------------------------------------------------

struct spanpackage_t
{
	void		*pdest;
	short		*pz;
	byte		*ptex;
	int			sfrac;
	int			tfrac;
	int			zi;
	short		count;
	short		r;
	short		g;
	short		b;
};

struct edgetable
{
	int			isflattop;
	int			numleftedges;
	int			*pleftedgevert0;
	int			*pleftedgevert1;
	int			*pleftedgevert2;
	int			numrightedges;
	int			*prightedgevert0;
	int			*prightedgevert1;
	int			*prightedgevert2;
};

struct adivtab_t
{
	int			quotient;
	int			remainder;
};

typedef void (*aliasspanfunc_t)(spanpackage_t*);

// EXTERNAL FUNCTION PROTOTYPES --------------------------------------------

// PUBLIC FUNCTION PROTOTYPES ----------------------------------------------

extern "C" {
void D_DrawNonSubdiv(void);
void D_PolysetSetEdgeTable(void);
void D_RasterizeAliasPolySmooth(void);
void D_PolysetCalcGradients(int);
void D_PolysetSetUpForLineScan(fixed_t, fixed_t, fixed_t, fixed_t);
void D_PolysetScanLeftEdge(int);
void D_PolysetDrawSpans_8(spanpackage_t*);
void D_PolysetDrawSpans_16(spanpackage_t*);
void D_PolysetDrawSpans_32(spanpackage_t*);
void D_PolysetDrawSpansFuzz_8(spanpackage_t*);
void D_PolysetDrawSpansAltFuzz_8(spanpackage_t*);
void D_PolysetDrawSpansFuzz_15(spanpackage_t*);
void D_PolysetDrawSpansFuzz_16(spanpackage_t*);
void D_PolysetDrawSpansFuzz_32(spanpackage_t*);
void D_PolysetDrawSpansRGB_8(spanpackage_t*);
void D_PolysetDrawSpansRGB_16(spanpackage_t*);
void D_PolysetDrawSpansRGB_32(spanpackage_t*);
void D_PolysetDrawSpansRGBFuzz_8(spanpackage_t*);
void D_PolysetDrawSpansRGBAltFuzz_8(spanpackage_t*);
void D_PolysetDrawSpansRGBFuzz_15(spanpackage_t*);
void D_PolysetDrawSpansRGBFuzz_16(spanpackage_t*);
void D_PolysetDrawSpansRGBFuzz_32(spanpackage_t*);
}

// PRIVATE FUNCTION PROTOTYPES ---------------------------------------------

// EXTERNAL DATA DECLARATIONS ----------------------------------------------

// PUBLIC DATA DEFINITIONS -------------------------------------------------

extern "C" {

int				ubasestep, errorterm, erroradjustup, erroradjustdown;

int				r_p0[8], r_p1[8], r_p2[8];

int				d_denom;

int				a_sstepxfrac, a_tstepxfrac, r_rstepx, r_gstepx, r_bstepx;
int				a_ststepxwhole, r_sstepx, r_tstepx, r_rstepy, r_gstepy;
int				r_bstepy, r_sstepy, r_tstepy, r_zistepx, r_zistepy;
int				d_aspancount, d_countextrastep;

spanpackage_t	*d_pedgespanpackage;
static int		ystart;
byte			*d_pdest, *d_ptex;
short			*d_pz;
int				d_sfrac, d_tfrac, d_r, d_g, d_b, d_zi;
int				d_ptexextrastep, d_ptexbasestep;
int				d_pdestextrastep, d_pdestbasestep;
int				d_sfracextrastep, d_sfracbasestep;
int				d_tfracextrastep, d_tfracbasestep;
int				d_rextrastep, d_rbasestep;
int				d_gextrastep, d_gbasestep;
int				d_bextrastep, d_bbasestep;
int				d_ziextrastep, d_zibasestep;
int				d_pzextrastep, d_pzbasestep;

adivtab_t		adivtab[32*32] =
{
#include "adivtab.h"
};

}

// PRIVATE DATA DEFINITIONS ------------------------------------------------

static spanpackage_t	*a_spans;

static edgetable	*pedgetable;

static edgetable	edgetables[12] =
{
	{0, 1, r_p0, r_p2, NULL, 2, r_p0, r_p1, r_p2},
	{0, 2, r_p1, r_p0, r_p2, 1, r_p1, r_p2, NULL},
	{1, 1, r_p0, r_p2, NULL, 1, r_p1, r_p2, NULL},
	{0, 1, r_p1, r_p0, NULL, 2, r_p1, r_p2, r_p0},
	{0, 2, r_p0, r_p2, r_p1, 1, r_p0, r_p1, NULL},
	{0, 1, r_p2, r_p1, NULL, 1, r_p2, r_p0, NULL},
	{0, 1, r_p2, r_p1, NULL, 2, r_p2, r_p0, r_p1},
	{0, 2, r_p2, r_p1, r_p0, 1, r_p2, r_p0, NULL},
	{0, 1, r_p1, r_p0, NULL, 1, r_p1, r_p2, NULL},
	{1, 1, r_p2, r_p1, NULL, 1, r_p0, r_p1, NULL},
	{1, 1, r_p1, r_p0, NULL, 1, r_p2, r_p0, NULL},
	{0, 1, r_p0, r_p2, NULL, 1, r_p0, r_p1, NULL},
};

static aliasspanfunc_t		D_PolysetDrawSpans;

static const aliasspanfunc_t alias_span_drawers[4][2][3] =
{
	{
		{
			D_PolysetDrawSpans_8,
			D_PolysetDrawSpansFuzz_8,
			D_PolysetDrawSpansAltFuzz_8
		},
		{
			D_PolysetDrawSpansRGB_8,
			D_PolysetDrawSpansRGBFuzz_8,
			D_PolysetDrawSpansRGBAltFuzz_8
		}
	},
	{
		{
			D_PolysetDrawSpans_16,
			D_PolysetDrawSpansFuzz_15,
			D_PolysetDrawSpansFuzz_15
		},
		{
			D_PolysetDrawSpansRGB_16,
			D_PolysetDrawSpansRGBFuzz_15,
			D_PolysetDrawSpansRGBFuzz_15
		}
	},
	{
		{
			D_PolysetDrawSpans_16,
			D_PolysetDrawSpansFuzz_16,
			D_PolysetDrawSpansFuzz_16
		},
		{
			D_PolysetDrawSpansRGB_16,
			D_PolysetDrawSpansRGBFuzz_16,
			D_PolysetDrawSpansRGBFuzz_16
		}
	},
	{
		{
			D_PolysetDrawSpans_32,
			D_PolysetDrawSpansFuzz_32,
			D_PolysetDrawSpansFuzz_32
		},
		{
			D_PolysetDrawSpansRGB_32,
			D_PolysetDrawSpansRGBFuzz_32,
			D_PolysetDrawSpansRGBFuzz_32
		}
	}
};

// CODE --------------------------------------------------------------------

//==========================================================================
//
//	D_PolysetSetupDrawer
//
//==========================================================================

void D_PolysetSetupDrawer(int translucency)
{
	int fuzzfunc = 0;

//translucency = 30;
//d_affinetridesc.coloredlight = 1;
	if (translucency)
	{
		int trindex = (translucency - 5) / 10;
		if (trindex < 0)
			trindex = 0;
		else if (trindex > 8)
			trindex = 8;
		if (trindex < 5)
		{
			d_transluc = tinttables[trindex];
			fuzzfunc = 1;
		}
		else
		{
			d_transluc = tinttables[8 - trindex];
			fuzzfunc = 2;
		}

		trindex = translucency * 31 / 100;
		d_dsttranstab = scaletable[trindex];
		d_srctranstab = scaletable[31 - trindex];
	}

	D_PolysetDrawSpans = alias_span_drawers[bppindex]
		[d_affinetridesc.coloredlight][fuzzfunc];
}

//==========================================================================
//
//	D_PolysetDraw
//
//==========================================================================

void D_PolysetDraw(void)
{
	spanpackage_t	spans[DPS_MAXSPANS + 1 +
			((CACHE_SIZE - 1) / sizeof(spanpackage_t)) + 1];
						// one extra because of cache line pretouching

	a_spans = (spanpackage_t *)
			(((long)&spans[0] + CACHE_SIZE - 1) & ~(CACHE_SIZE - 1));

	D_DrawNonSubdiv();
}

//==========================================================================
//
//	D_DrawNonSubdiv
//
//==========================================================================

#ifndef USEASM

extern "C" void D_DrawNonSubdiv(void)
{
	mtriangle_t		*ptri;
	finalvert_t		*pfv, *index0, *index1, *index2;
	finalstvert_t	*pstv, *stindex0, *stindex1, *stindex2;
	int				i;
	int				lnumtriangles;

	pfv = d_affinetridesc.pfinalverts;
	pstv = d_affinetridesc.pstverts;
	ptri = d_affinetridesc.ptriangles;
	lnumtriangles = d_affinetridesc.numtriangles;

	for (i = 0; i < lnumtriangles; i++, ptri++)
	{
		index0 = pfv + ptri->vertindex[0];
		index1 = pfv + ptri->vertindex[1];
		index2 = pfv + ptri->vertindex[2];

		d_denom = (index0->v - index1->v) * (index0->u - index2->u)
				- (index0->u - index1->u) * (index0->v - index2->v);

		if (d_denom <= 0)
		{
			continue;
		}

		stindex0 = pstv + ptri->stvertindex[0];
		stindex1 = pstv + ptri->stvertindex[1];
		stindex2 = pstv + ptri->stvertindex[2];

		r_p0[0] = index0->u;
		r_p0[1] = index0->v;
		r_p0[2] = stindex0->s;
		r_p0[3] = stindex0->t;
		r_p0[4] = index0->zi;
		r_p0[5] = index0->r;
		r_p0[6] = index0->g;
		r_p0[7] = index0->b;

		r_p1[0] = index1->u;
		r_p1[1] = index1->v;
		r_p1[2] = stindex1->s;
		r_p1[3] = stindex1->t;
		r_p1[4] = index1->zi;
		r_p1[5] = index1->r;
		r_p1[6] = index1->g;
		r_p1[7] = index1->b;

		r_p2[0] = index2->u;
		r_p2[1] = index2->v;
		r_p2[2] = stindex2->s;
		r_p2[3] = stindex2->t;
		r_p2[4] = index2->zi;
		r_p2[5] = index2->r;
		r_p2[6] = index2->g;
		r_p2[7] = index2->b;

		D_PolysetSetEdgeTable();
		D_RasterizeAliasPolySmooth();
	}
}

#endif

//==========================================================================
//
//	D_PolysetSetEdgeTable
//
//==========================================================================

extern "C" void D_PolysetSetEdgeTable(void)
{
	int			edgetableindex;

	edgetableindex = 0;	// assume the vertices are already in
						//  top to bottom order

//
// determine which edges are right & left, and the order in which
// to rasterize them
//
	if (r_p0[1] <= r_p1[1])
	{
		if (r_p0[1] == r_p1[1])
		{
			if (r_p0[1] > r_p2[1])
				pedgetable = &edgetables[2];
			else
				pedgetable = &edgetables[5];

			return;
		}
		else
		{
			edgetableindex = 1;
		}
	}

	if (r_p0[1] == r_p2[1])
	{
		if (edgetableindex)
			pedgetable = &edgetables[8];
		else
			pedgetable = &edgetables[9];

		return;
	}
	else if (r_p1[1] == r_p2[1])
	{
		if (edgetableindex)
			pedgetable = &edgetables[10];
		else
			pedgetable = &edgetables[11];

		return;
	}

	if (r_p0[1] < r_p2[1])
		edgetableindex += 2;

	if (r_p1[1] < r_p2[1])
		edgetableindex += 4;

	pedgetable = &edgetables[edgetableindex];
}

//==========================================================================
//
//	D_RasterizeAliasPolySmooth
//
//==========================================================================

extern "C" void D_RasterizeAliasPolySmooth(void)
{
	int				initialleftheight, initialrightheight;
	int				*plefttop, *prighttop, *pleftbottom, *prightbottom;
	int				working_rstepx, working_gstepx, working_bstepx, originalcount;

	plefttop = pedgetable->pleftedgevert0;
	prighttop = pedgetable->prightedgevert0;

	pleftbottom = pedgetable->pleftedgevert1;
	prightbottom = pedgetable->prightedgevert1;

	initialleftheight = -(pleftbottom[1] - plefttop[1]);
	initialrightheight = -(prightbottom[1] - prighttop[1]);

//
// set the s, t, and light gradients, which are consistent across the triangle
// because being a triangle, things are affine
//
	D_PolysetCalcGradients(d_affinetridesc.skinwidth);

//
// rasterize the polygon
//

//
// scan out the top (and possibly only) part of the left edge
//
	d_pedgespanpackage = a_spans;

	ystart = plefttop[1];
	d_aspancount = plefttop[0] - prighttop[0];

	d_ptex = (byte *)d_affinetridesc.pskin + (plefttop[2] >> 16) +
			(plefttop[3] >> 16) * d_affinetridesc.skinwidth;
#ifdef USEASM
	d_sfrac = (plefttop[2] & 0xFFFF) << 16;
	d_tfrac = (plefttop[3] & 0xFFFF) << 16;
#else
	d_sfrac = plefttop[2] & 0xFFFF;
	d_tfrac = plefttop[3] & 0xFFFF;
#endif
	d_zi = plefttop[4];
	d_r = plefttop[5];
	d_g = plefttop[6];
	d_b = plefttop[7];

	d_pdest = (byte*)scrn + (ylookup[ystart] + plefttop[0]) * PixelBytes;
	d_pz = zbuffer + ylookup[ystart] + plefttop[0];

	if (initialleftheight == 1)
	{
		d_pedgespanpackage->pdest = d_pdest;
		d_pedgespanpackage->pz = d_pz;
		d_pedgespanpackage->count = d_aspancount;
		d_pedgespanpackage->ptex = d_ptex;

		d_pedgespanpackage->sfrac = d_sfrac;
		d_pedgespanpackage->tfrac = d_tfrac;

	// FIXME: need to clamp l, s, t, at both ends?
		d_pedgespanpackage->r = d_r;
		d_pedgespanpackage->g = d_g;
		d_pedgespanpackage->b = d_b;
		d_pedgespanpackage->zi = d_zi;

		d_pedgespanpackage++;
	}
	else
	{
		D_PolysetSetUpForLineScan(plefttop[0], plefttop[1],
							  pleftbottom[0], pleftbottom[1]);

		d_pzbasestep = (ScreenWidth + ubasestep) << 1;
		d_pzextrastep = d_pzbasestep + 2;

		d_pdestbasestep = (ScreenWidth + ubasestep) * PixelBytes;
		d_pdestextrastep = d_pdestbasestep + PixelBytes;

	// TODO: can reuse partial expressions here

	// for negative steps in x along left edge, bias toward overflow rather than
	// underflow (sort of turning the floor () we did in the gradient calcs into
	// ceil (), but plus a little bit)
		if (ubasestep < 0)
		{
			working_rstepx = r_rstepx - 1;
			working_gstepx = r_gstepx - 1;
			working_bstepx = r_bstepx - 1;
		}
		else
		{
			working_rstepx = r_rstepx;
			working_gstepx = r_gstepx;
			working_bstepx = r_bstepx;
		}

		d_countextrastep = ubasestep + 1;
		d_ptexbasestep = ((r_sstepy + r_sstepx * ubasestep) >> 16) +
				((r_tstepy + r_tstepx * ubasestep) >> 16) *
				d_affinetridesc.skinwidth;
#ifdef USEASM
		d_sfracbasestep = (r_sstepy + r_sstepx * ubasestep) << 16;
		d_tfracbasestep = (r_tstepy + r_tstepx * ubasestep) << 16;
#else
		d_sfracbasestep = (r_sstepy + r_sstepx * ubasestep) & 0xFFFF;
		d_tfracbasestep = (r_tstepy + r_tstepx * ubasestep) & 0xFFFF;
#endif
		d_rbasestep = r_rstepy + working_rstepx * ubasestep;
		d_gbasestep = r_gstepy + working_gstepx * ubasestep;
		d_bbasestep = r_bstepy + working_bstepx * ubasestep;
		d_zibasestep = r_zistepy + r_zistepx * ubasestep;

		d_ptexextrastep = ((r_sstepy + r_sstepx * d_countextrastep) >> 16) +
				((r_tstepy + r_tstepx * d_countextrastep) >> 16) *
				d_affinetridesc.skinwidth;
#ifdef USEASM
		d_sfracextrastep = (r_sstepy + r_sstepx*d_countextrastep) << 16;
		d_tfracextrastep = (r_tstepy + r_tstepx*d_countextrastep) << 16;
#else
		d_sfracextrastep = (r_sstepy + r_sstepx*d_countextrastep) & 0xFFFF;
		d_tfracextrastep = (r_tstepy + r_tstepx*d_countextrastep) & 0xFFFF;
#endif
		d_rextrastep = d_rbasestep + working_rstepx;
		d_gextrastep = d_gbasestep + working_gstepx;
		d_bextrastep = d_bbasestep + working_bstepx;
		d_ziextrastep = d_zibasestep + r_zistepx;

		D_PolysetScanLeftEdge(initialleftheight);
	}

//
// scan out the bottom part of the left edge, if it exists
//
	if (pedgetable->numleftedges == 2)
	{
		int		height;

		plefttop = pleftbottom;
		pleftbottom = pedgetable->pleftedgevert2;

		height = -(pleftbottom[1] - plefttop[1]);

// TODO: make this a function; modularize this function in general

		ystart = plefttop[1];
		d_aspancount = plefttop[0] - prighttop[0];
		d_ptex = (byte *)d_affinetridesc.pskin + (plefttop[2] >> 16) +
				(plefttop[3] >> 16) * d_affinetridesc.skinwidth;
		d_sfrac = 0;
		d_tfrac = 0;
		d_zi = plefttop[4];
		d_r = plefttop[5];
		d_g = plefttop[6];
		d_b = plefttop[7];

		d_pdest = (byte *)scrn + (ylookup[ystart] + plefttop[0]) * PixelBytes;
		d_pz = zbuffer + ylookup[ystart] + plefttop[0];

		if (height == 1)
		{
			d_pedgespanpackage->pdest = d_pdest;
			d_pedgespanpackage->pz = d_pz;
			d_pedgespanpackage->count = d_aspancount;
			d_pedgespanpackage->ptex = d_ptex;

			d_pedgespanpackage->sfrac = d_sfrac;
			d_pedgespanpackage->tfrac = d_tfrac;

		// FIXME: need to clamp l, s, t, at both ends?
			d_pedgespanpackage->r = d_r;
			d_pedgespanpackage->g = d_g;
			d_pedgespanpackage->b = d_b;
			d_pedgespanpackage->zi = d_zi;

			d_pedgespanpackage++;
		}
		else
		{
			D_PolysetSetUpForLineScan(plefttop[0], plefttop[1],
								  pleftbottom[0], pleftbottom[1]);

			d_pdestbasestep = (ScreenWidth + ubasestep) * PixelBytes;
			d_pdestextrastep = d_pdestbasestep + PixelBytes;

			d_pzbasestep = (ScreenWidth + ubasestep) << 1;
			d_pzextrastep = d_pzbasestep + 2;

			if (ubasestep < 0)
			{
				working_rstepx = r_rstepx - 1;
				working_gstepx = r_gstepx - 1;
				working_bstepx = r_bstepx - 1;
			}
			else
			{
				working_rstepx = r_rstepx;
				working_gstepx = r_gstepx;
				working_bstepx = r_bstepx;
			}

			d_countextrastep = ubasestep + 1;
			d_ptexbasestep = ((r_sstepy + r_sstepx * ubasestep) >> 16) +
					((r_tstepy + r_tstepx * ubasestep) >> 16) *
					d_affinetridesc.skinwidth;
#ifdef USEASM
			d_sfracbasestep = (r_sstepy + r_sstepx * ubasestep) << 16;
			d_tfracbasestep = (r_tstepy + r_tstepx * ubasestep) << 16;
#else
			d_sfracbasestep = (r_sstepy + r_sstepx * ubasestep) & 0xFFFF;
			d_tfracbasestep = (r_tstepy + r_tstepx * ubasestep) & 0xFFFF;
#endif
			d_rbasestep = r_rstepy + working_rstepx * ubasestep;
			d_gbasestep = r_gstepy + working_gstepx * ubasestep;
			d_bbasestep = r_bstepy + working_bstepx * ubasestep;
			d_zibasestep = r_zistepy + r_zistepx * ubasestep;

			d_ptexextrastep = ((r_sstepy + r_sstepx * d_countextrastep) >> 16) +
					((r_tstepy + r_tstepx * d_countextrastep) >> 16) *
					d_affinetridesc.skinwidth;
#ifdef USEASM
			d_sfracextrastep = ((r_sstepy+r_sstepx*d_countextrastep) & 0xFFFF)<<16;
			d_tfracextrastep = ((r_tstepy+r_tstepx*d_countextrastep) & 0xFFFF)<<16;
#else
			d_sfracextrastep = (r_sstepy+r_sstepx*d_countextrastep) & 0xFFFF;
			d_tfracextrastep = (r_tstepy+r_tstepx*d_countextrastep) & 0xFFFF;
#endif
			d_rextrastep = d_rbasestep + working_rstepx;
			d_gextrastep = d_gbasestep + working_gstepx;
			d_bextrastep = d_bbasestep + working_bstepx;
			d_ziextrastep = d_zibasestep + r_zistepx;

			D_PolysetScanLeftEdge(height);
		}
	}

// scan out the top (and possibly only) part of the right edge, updating the
// count field
	d_pedgespanpackage = a_spans;

	D_PolysetSetUpForLineScan(prighttop[0], prighttop[1],
						  prightbottom[0], prightbottom[1]);
	d_aspancount = 0;
	d_countextrastep = ubasestep + 1;
	originalcount = a_spans[initialrightheight].count;
	a_spans[initialrightheight].count = DPS_SPAN_LIST_END; // mark end of the spanpackages
	D_PolysetDrawSpans(a_spans);

// scan out the bottom part of the right edge, if it exists
	if (pedgetable->numrightedges == 2)
	{
		int				height;
		spanpackage_t	*pstart;

		pstart = a_spans + initialrightheight;
		pstart->count = originalcount;

		d_aspancount = prightbottom[0] - prighttop[0];

		prighttop = prightbottom;
		prightbottom = pedgetable->prightedgevert2;

		height = -(prightbottom[1] - prighttop[1]);

		D_PolysetSetUpForLineScan(prighttop[0], prighttop[1],
							  prightbottom[0], prightbottom[1]);

		d_countextrastep = ubasestep + 1;
		a_spans[initialrightheight + height].count = DPS_SPAN_LIST_END;
											// mark end of the spanpackages
		D_PolysetDrawSpans(pstart);
	}
}

#ifndef USEASM

//==========================================================================
//
//	D_PolysetCalcGradients
//
//==========================================================================

extern "C" void D_PolysetCalcGradients(int skinwidth)
{
	float	p01_minus_p21, p11_minus_p21, p00_minus_p20, p10_minus_p20;
	float	denominv, t0, t1;

	p00_minus_p20 = r_p0[0] - r_p2[0];
	p01_minus_p21 = r_p0[1] - r_p2[1];
	p10_minus_p20 = r_p1[0] - r_p2[0];
	p11_minus_p21 = r_p1[1] - r_p2[1];

	denominv = 1.0 / (float)d_denom;

	t0 = r_p0[2] - r_p2[2];
	t1 = r_p1[2] - r_p2[2];
	r_sstepx = (int)((t1 * p01_minus_p21 - t0 * p11_minus_p21) * denominv);
	r_sstepy = (int)((t1 * p00_minus_p20 - t0 * p10_minus_p20) * denominv);

	t0 = r_p0[3] - r_p2[3];
	t1 = r_p1[3] - r_p2[3];
	r_tstepx = (int)((t1 * p01_minus_p21 - t0 * p11_minus_p21) * denominv);
	r_tstepy = (int)((t1 * p00_minus_p20 - t0 * p10_minus_p20) * denominv);

	t0 = r_p0[4] - r_p2[4];
	t1 = r_p1[4] - r_p2[4];
	r_zistepx = (int)((t1 * p01_minus_p21 - t0 * p11_minus_p21) * denominv);
	r_zistepy = (int)((t1 * p00_minus_p20 - t0 * p10_minus_p20) * denominv);

// ceil () for light so positive steps are exaggerated, negative steps
// diminished,  pushing us away from underflow toward overflow. Underflow is
// very visible, overflow is very unlikely, because of ambient lighting
	t0 = r_p0[5] - r_p2[5];
	t1 = r_p1[5] - r_p2[5];
	r_rstepx = (int)ceil((t1 * p01_minus_p21 - t0 * p11_minus_p21) * denominv);
	r_rstepy = (int)ceil((t1 * p00_minus_p20 - t0 * p10_minus_p20) * denominv);

	t0 = r_p0[6] - r_p2[6];
	t1 = r_p1[6] - r_p2[6];
	r_gstepx = (int)ceil((t1 * p01_minus_p21 - t0 * p11_minus_p21) * denominv);
	r_gstepy = (int)ceil((t1 * p00_minus_p20 - t0 * p10_minus_p20) * denominv);

	t0 = r_p0[7] - r_p2[7];
	t1 = r_p1[7] - r_p2[7];
	r_bstepx = (int)ceil((t1 * p01_minus_p21 - t0 * p11_minus_p21) * denominv);
	r_bstepy = (int)ceil((t1 * p00_minus_p20 - t0 * p10_minus_p20) * denominv);

	a_sstepxfrac = r_sstepx & 0xFFFF;
	a_tstepxfrac = r_tstepx & 0xFFFF;

	a_ststepxwhole = skinwidth * (r_tstepx >> 16) + (r_sstepx >> 16);
}

//==========================================================================
//
//	FloorDivMod
//
//	Returns mathematically correct (floor-based) quotient and remainder for
// numer and denom, both of which should contain no fractional part. The
// quotient must fit in 32 bits.
//
//==========================================================================

static void FloorDivMod(double numer, double denom, int *quotient, int *rem)
{
	int		q, r;
	double	x;

#ifdef PARANOID
	if (denom <= 0.0)
		Sys_Error ("FloorDivMod: bad denominator %f\n", denom);

//	if ((floor(numer) != numer) || (floor(denom) != denom))
//		Sys_Error ("FloorDivMod: non-integer numer or denom %f %f\n",
//				numer, denom);
#endif

	if (numer >= 0.0)
	{

		x = floor(numer / denom);
		q = (int)x;
		r = (int)floor(numer - (x * denom));
	}
	else
	{
	//
	// perform operations with positive values, and fix mod to make floor-based
	//
		x = floor(-numer / denom);
		q = -(int)x;
		r = (int)floor(-numer - (x * denom));
		if (r != 0)
		{
			q--;
			r = (int)denom - r;
		}
	}

	*quotient = q;
	*rem = r;
}

//==========================================================================
//
//	D_PolysetSetUpForLineScan
//
//==========================================================================

extern "C" void D_PolysetSetUpForLineScan(fixed_t startvertu,
	fixed_t startvertv, fixed_t endvertu, fixed_t endvertv)
{
	double		dm, dn;
	int			tm, tn;
	const adivtab_t	*ptemp;

	errorterm = -1;

	tm = endvertu - startvertu;
	tn = -(endvertv - startvertv);

	if (((tm <= 16) && (tm >= -15)) &&
		((tn <= 16) && (tn >= -15)))
	{
		ptemp = &adivtab[((tm+15) << 5) + (tn+15)];
		ubasestep = ptemp->quotient;
		erroradjustup = ptemp->remainder;
		erroradjustdown = tn;
	}
	else
	{
		dm = (double)tm;
		dn = (double)tn;

		FloorDivMod(dm, dn, &ubasestep, &erroradjustup);

		erroradjustdown = tn;
	}
}

//==========================================================================
//
//	D_PolysetScanLeftEdge
//
//==========================================================================

extern "C" void D_PolysetScanLeftEdge(int height)
{
	do
	{
		d_pedgespanpackage->pdest = d_pdest;
		d_pedgespanpackage->pz = d_pz;
		d_pedgespanpackage->count = d_aspancount;
		d_pedgespanpackage->ptex = d_ptex;

		d_pedgespanpackage->sfrac = d_sfrac;
		d_pedgespanpackage->tfrac = d_tfrac;

	// FIXME: need to clamp l, s, t, at both ends?
		d_pedgespanpackage->r = d_r;
		d_pedgespanpackage->g = d_g;
		d_pedgespanpackage->b = d_b;
		d_pedgespanpackage->zi = d_zi;

		d_pedgespanpackage++;

		errorterm += erroradjustup;
		if (errorterm >= 0)
		{
			d_pdest += d_pdestextrastep;
			d_pz = (short*)((byte*)d_pz + d_pzextrastep);
			d_aspancount += d_countextrastep;
			d_ptex += d_ptexextrastep;
			d_sfrac += d_sfracextrastep;
			d_ptex += d_sfrac >> 16;

			d_sfrac &= 0xFFFF;
			d_tfrac += d_tfracextrastep;
			if (d_tfrac & 0x10000)
			{
				d_ptex += d_affinetridesc.skinwidth;
				d_tfrac &= 0xFFFF;
			}
			d_r += d_rextrastep;
			d_g += d_gextrastep;
			d_b += d_bextrastep;
			d_zi += d_ziextrastep;
			errorterm -= erroradjustdown;
		}
		else
		{
			d_pdest += d_pdestbasestep;
			d_pz = (short*)((byte*)d_pz + d_pzbasestep);
			d_aspancount += ubasestep;
			d_ptex += d_ptexbasestep;
			d_sfrac += d_sfracbasestep;
			d_ptex += d_sfrac >> 16;
			d_sfrac &= 0xFFFF;
			d_tfrac += d_tfracbasestep;
			if (d_tfrac & 0x10000)
			{
				d_ptex += d_affinetridesc.skinwidth;
				d_tfrac &= 0xFFFF;
			}
			d_r += d_rbasestep;
			d_g += d_gbasestep;
			d_b += d_bbasestep;
			d_zi += d_zibasestep;
		}
	} while (--height);
}

//==========================================================================
//
//	D_PolysetDrawSpans_8
//
//==========================================================================

extern "C" void D_PolysetDrawSpans_8(spanpackage_t *pspanpackage)
{
	int		lcount;
	byte	*lpdest;
	byte	*lptex;
	int		lsfrac, ltfrac;
	int		llight;
	int		lzi;
	short	*lpz;

	do
	{
		lcount = d_aspancount - pspanpackage->count;

		errorterm += erroradjustup;
		if (errorterm >= 0)
		{
			d_aspancount += d_countextrastep;
			errorterm -= erroradjustdown;
		}
		else
		{
			d_aspancount += ubasestep;
		}

		if (lcount)
		{
			lpdest = (byte*)pspanpackage->pdest;
			lptex = pspanpackage->ptex;
			lpz = pspanpackage->pz;
			lsfrac = pspanpackage->sfrac;
			ltfrac = pspanpackage->tfrac;
			llight = pspanpackage->r;
			lzi = pspanpackage->zi;

			do
			{
				if ((lzi >> 16) >= *lpz)
				{
					*lpdest = fadetable[*lptex + (llight & 0xFF00)];
					*lpz = lzi >> 16;
				}
				lpdest++;
				lzi += r_zistepx;
				lpz++;
				llight += r_rstepx;
				lptex += a_ststepxwhole;
				lsfrac += a_sstepxfrac;
				lptex += lsfrac >> 16;
				lsfrac &= 0xFFFF;
				ltfrac += a_tstepxfrac;
				if (ltfrac & 0x10000)
				{
					lptex += d_affinetridesc.skinwidth;
					ltfrac &= 0xFFFF;
				}
			} while (--lcount);
		}

		pspanpackage++;
	} while (pspanpackage->count != DPS_SPAN_LIST_END);
}

//==========================================================================
//
//	D_PolysetDrawSpans_16
//
//==========================================================================

extern "C" void D_PolysetDrawSpans_16(spanpackage_t *pspanpackage)
{
	int		lcount;
	word	*lpdest;
	byte	*lptex;
	int		lsfrac, ltfrac;
	int		llight;
	int		lzi;
	short	*lpz;

	do
	{
		lcount = d_aspancount - pspanpackage->count;

		errorterm += erroradjustup;
		if (errorterm >= 0)
		{
			d_aspancount += d_countextrastep;
			errorterm -= erroradjustdown;
		}
		else
		{
			d_aspancount += ubasestep;
		}

		if (lcount)
		{
			lpdest = (word*)pspanpackage->pdest;
			lptex = pspanpackage->ptex;
			lpz = pspanpackage->pz;
			lsfrac = pspanpackage->sfrac;
			ltfrac = pspanpackage->tfrac;
			llight = pspanpackage->r;
			lzi = pspanpackage->zi;

			do
			{
				if ((lzi >> 16) >= *lpz)
				{
					*lpdest = fadetable16[*lptex + (llight & 0xFF00)];
					*lpz = lzi >> 16;
				}
				lpdest++;
				lzi += r_zistepx;
				lpz++;
				llight += r_rstepx;
				lptex += a_ststepxwhole;
				lsfrac += a_sstepxfrac;
				lptex += lsfrac >> 16;
				lsfrac &= 0xFFFF;
				ltfrac += a_tstepxfrac;
				if (ltfrac & 0x10000)
				{
					lptex += d_affinetridesc.skinwidth;
					ltfrac &= 0xFFFF;
				}
			} while (--lcount);
		}

		pspanpackage++;
	} while (pspanpackage->count != DPS_SPAN_LIST_END);
}

//==========================================================================
//
//	D_PolysetDrawSpans_32
//
//==========================================================================

extern "C" void D_PolysetDrawSpans_32(spanpackage_t *pspanpackage)
{
	int		lcount;
	dword	*lpdest;
	byte	*lptex;
	int		lsfrac, ltfrac;
	int		llight;
	int		lzi;
	short	*lpz;

	do
	{
		lcount = d_aspancount - pspanpackage->count;

		errorterm += erroradjustup;
		if (errorterm >= 0)
		{
			d_aspancount += d_countextrastep;
			errorterm -= erroradjustdown;
		}
		else
		{
			d_aspancount += ubasestep;
		}

		if (lcount)
		{
			lpdest = (dword*)pspanpackage->pdest;
			lptex = pspanpackage->ptex;
			lpz = pspanpackage->pz;
			lsfrac = pspanpackage->sfrac;
			ltfrac = pspanpackage->tfrac;
			llight = pspanpackage->r;
			lzi = pspanpackage->zi;

			do
			{
				if ((lzi >> 16) >= *lpz)
				{
					*lpdest = fadetable32[*lptex + (llight & 0xFF00)];
					*lpz = lzi >> 16;
				}
				lpdest++;
				lzi += r_zistepx;
				lpz++;
				llight += r_rstepx;
				lptex += a_ststepxwhole;
				lsfrac += a_sstepxfrac;
				lptex += lsfrac >> 16;
				lsfrac &= 0xFFFF;
				ltfrac += a_tstepxfrac;
				if (ltfrac & 0x10000)
				{
					lptex += d_affinetridesc.skinwidth;
					ltfrac &= 0xFFFF;
				}
			} while (--lcount);
		}

		pspanpackage++;
	} while (pspanpackage->count != DPS_SPAN_LIST_END);
}

//==========================================================================
//
//	D_PolysetDrawSpansFuzz_8
//
//==========================================================================

extern "C" void D_PolysetDrawSpansFuzz_8(spanpackage_t *pspanpackage)
{
	int		lcount;
	byte	*lpdest;
	byte	*lptex;
	int		lsfrac, ltfrac;
	int		llight;
	int		lzi;
	short	*lpz;

	do
	{
		lcount = d_aspancount - pspanpackage->count;

		errorterm += erroradjustup;
		if (errorterm >= 0)
		{
			d_aspancount += d_countextrastep;
			errorterm -= erroradjustdown;
		}
		else
		{
			d_aspancount += ubasestep;
		}

		if (lcount)
		{
			lpdest = (byte*)pspanpackage->pdest;
			lptex = pspanpackage->ptex;
			lpz = pspanpackage->pz;
			lsfrac = pspanpackage->sfrac;
			ltfrac = pspanpackage->tfrac;
			llight = pspanpackage->r;
			lzi = pspanpackage->zi;

			do
			{
				if ((lzi >> 16) >= *lpz)
				{
					*lpdest = d_transluc[*lpdest +
						(fadetable[*lptex + (llight & 0xFF00)] << 8)];
					*lpz = lzi >> 16;
				}
				lpdest++;
				lzi += r_zistepx;
				lpz++;
				llight += r_rstepx;
				lptex += a_ststepxwhole;
				lsfrac += a_sstepxfrac;
				lptex += lsfrac >> 16;
				lsfrac &= 0xFFFF;
				ltfrac += a_tstepxfrac;
				if (ltfrac & 0x10000)
				{
					lptex += d_affinetridesc.skinwidth;
					ltfrac &= 0xFFFF;
				}
			} while (--lcount);
		}

		pspanpackage++;
	} while (pspanpackage->count != DPS_SPAN_LIST_END);
}

//==========================================================================
//
//	D_PolysetDrawSpansAltFuzz_8
//
//==========================================================================

extern "C" void D_PolysetDrawSpansAltFuzz_8(spanpackage_t *pspanpackage)
{
	int		lcount;
	byte	*lpdest;
	byte	*lptex;
	int		lsfrac, ltfrac;
	int		llight;
	int		lzi;
	short	*lpz;

	do
	{
		lcount = d_aspancount - pspanpackage->count;

		errorterm += erroradjustup;
		if (errorterm >= 0)
		{
			d_aspancount += d_countextrastep;
			errorterm -= erroradjustdown;
		}
		else
		{
			d_aspancount += ubasestep;
		}

		if (lcount)
		{
			lpdest = (byte*)pspanpackage->pdest;
			lptex = pspanpackage->ptex;
			lpz = pspanpackage->pz;
			lsfrac = pspanpackage->sfrac;
			ltfrac = pspanpackage->tfrac;
			llight = pspanpackage->r;
			lzi = pspanpackage->zi;

			do
			{
				if ((lzi >> 16) >= *lpz)
				{
					*lpdest = d_transluc[(*lpdest << 8) +
						fadetable[*lptex + (llight & 0xFF00)]];
					*lpz = lzi >> 16;
				}
				lpdest++;
				lzi += r_zistepx;
				lpz++;
				llight += r_rstepx;
				lptex += a_ststepxwhole;
				lsfrac += a_sstepxfrac;
				lptex += lsfrac >> 16;
				lsfrac &= 0xFFFF;
				ltfrac += a_tstepxfrac;
				if (ltfrac & 0x10000)
				{
					lptex += d_affinetridesc.skinwidth;
					ltfrac &= 0xFFFF;
				}
			} while (--lcount);
		}

		pspanpackage++;
	} while (pspanpackage->count != DPS_SPAN_LIST_END);
}

#endif

//==========================================================================
//
//	D_PolysetDrawSpansFuzz_15
//
//==========================================================================

extern "C" void D_PolysetDrawSpansFuzz_15(spanpackage_t *pspanpackage)
{
	int		lcount;
	word	*lpdest;
	byte	*lptex;
	int		lsfrac, ltfrac;
	int		llight;
	int		lzi;
	short	*lpz;

	do
	{
		lcount = d_aspancount - pspanpackage->count;

		errorterm += erroradjustup;
		if (errorterm >= 0)
		{
			d_aspancount += d_countextrastep;
			errorterm -= erroradjustdown;
		}
		else
		{
			d_aspancount += ubasestep;
		}

		if (lcount)
		{
			lpdest = (word*)pspanpackage->pdest;
			lptex = pspanpackage->ptex;
			lpz = pspanpackage->pz;
#ifdef USEASM
			lsfrac = (dword)pspanpackage->sfrac >> 16;
			ltfrac = (dword)pspanpackage->tfrac >> 16;
#else
			lsfrac = pspanpackage->sfrac;
			ltfrac = pspanpackage->tfrac;
#endif
			llight = pspanpackage->r;
			lzi = pspanpackage->zi;

			do
			{
				if ((lzi >> 16) >= *lpz)
				{
					int btemp = fadetable16[*lptex + (llight & 0xFF00)];
					byte r1 = GetCol15R(*lpdest);
					byte g1 = GetCol15G(*lpdest);
					byte b1 = GetCol15B(*lpdest);
					byte r2 = GetCol15R(btemp);
					byte g2 = GetCol15G(btemp);
					byte b2 = GetCol15B(btemp);
					byte r = (d_dsttranstab[r1] + d_srctranstab[r2]) >> 8;
					byte g = (d_dsttranstab[g1] + d_srctranstab[g2]) >> 8;
					byte b = (d_dsttranstab[b1] + d_srctranstab[b2]) >> 8;
					*lpdest = MakeCol15(r, g, b);
					*lpz = lzi >> 16;
				}
				lpdest++;
				lzi += r_zistepx;
				lpz++;
				llight += r_rstepx;
				lptex += a_ststepxwhole;
#ifdef USEASM
				lsfrac += (dword)a_sstepxfrac >> 16;
#else
				lsfrac += a_sstepxfrac;
#endif
				lptex += lsfrac >> 16;
				lsfrac &= 0xFFFF;
#ifdef USEASM
				ltfrac += (dword)a_tstepxfrac >> 16;
#else
				ltfrac += a_tstepxfrac;
#endif
				if (ltfrac & 0x10000)
				{
					lptex += d_affinetridesc.skinwidth;
					ltfrac &= 0xFFFF;
				}
			} while (--lcount);
		}

		pspanpackage++;
	} while (pspanpackage->count != DPS_SPAN_LIST_END);
}

//==========================================================================
//
//	D_PolysetDrawSpansFuzz_16
//
//==========================================================================

extern "C" void D_PolysetDrawSpansFuzz_16(spanpackage_t *pspanpackage)
{
	int		lcount;
	word	*lpdest;
	byte	*lptex;
	int		lsfrac, ltfrac;
	int		llight;
	int		lzi;
	short	*lpz;

	do
	{
		lcount = d_aspancount - pspanpackage->count;

		errorterm += erroradjustup;
		if (errorterm >= 0)
		{
			d_aspancount += d_countextrastep;
			errorterm -= erroradjustdown;
		}
		else
		{
			d_aspancount += ubasestep;
		}

		if (lcount)
		{
			lpdest = (word*)pspanpackage->pdest;
			lptex = pspanpackage->ptex;
			lpz = pspanpackage->pz;
#ifdef USEASM
			lsfrac = (dword)pspanpackage->sfrac >> 16;
			ltfrac = (dword)pspanpackage->tfrac >> 16;
#else
			lsfrac = pspanpackage->sfrac;
			ltfrac = pspanpackage->tfrac;
#endif
			llight = pspanpackage->r;
			lzi = pspanpackage->zi;

			do
			{
				if ((lzi >> 16) >= *lpz)
				{
					dword btemp = fadetable16[*lptex + (llight & 0xFF00)];
					byte r1 = GetCol16R(*lpdest);
					byte g1 = GetCol16G(*lpdest);
					byte b1 = GetCol16B(*lpdest);
					byte r2 = GetCol16R(btemp);
					byte g2 = GetCol16G(btemp);
					byte b2 = GetCol16B(btemp);
					byte r = (d_dsttranstab[r1] + d_srctranstab[r2]) >> 8;
					byte g = (d_dsttranstab[g1] + d_srctranstab[g2]) >> 8;
					byte b = (d_dsttranstab[b1] + d_srctranstab[b2]) >> 8;
					*lpdest = MakeCol16(r, g, b);
					*lpz = lzi >> 16;
				}
				lpdest++;
				lzi += r_zistepx;
				lpz++;
				llight += r_rstepx;
				lptex += a_ststepxwhole;
#ifdef USEASM
				lsfrac += (dword)a_sstepxfrac >> 16;
#else
				lsfrac += a_sstepxfrac;
#endif
				lptex += lsfrac >> 16;
				lsfrac &= 0xFFFF;
#ifdef USEASM
				ltfrac += (dword)a_tstepxfrac >> 16;
#else
				ltfrac += a_tstepxfrac;
#endif
				if (ltfrac & 0x10000)
				{
					lptex += d_affinetridesc.skinwidth;
					ltfrac &= 0xFFFF;
				}
			} while (--lcount);
		}

		pspanpackage++;
	} while (pspanpackage->count != DPS_SPAN_LIST_END);
}

#ifndef USEASM

//==========================================================================
//
//	D_PolysetDrawSpansFuzz_32
//
//==========================================================================

extern "C" void D_PolysetDrawSpansFuzz_32(spanpackage_t *pspanpackage)
{
	int		lcount;
	dword	*lpdest;
	byte	*lptex;
	int		lsfrac, ltfrac;
	int		llight;
	int		lzi;
	short	*lpz;

	do
	{
		lcount = d_aspancount - pspanpackage->count;

		errorterm += erroradjustup;
		if (errorterm >= 0)
		{
			d_aspancount += d_countextrastep;
			errorterm -= erroradjustdown;
		}
		else
		{
			d_aspancount += ubasestep;
		}

		if (lcount)
		{
			lpdest = (dword*)pspanpackage->pdest;
			lptex = pspanpackage->ptex;
			lpz = pspanpackage->pz;
			lsfrac = pspanpackage->sfrac;
			ltfrac = pspanpackage->tfrac;
			llight = pspanpackage->r;
			lzi = pspanpackage->zi;

			do
			{
				if ((lzi >> 16) >= *lpz)
				{
					dword btemp = fadetable32[*lptex + (llight & 0xFF00)];
					byte r1 = GetCol32R(*lpdest);
					byte g1 = GetCol32G(*lpdest);
					byte b1 = GetCol32B(*lpdest);
					byte r2 = GetCol32R(btemp);
					byte g2 = GetCol32G(btemp);
					byte b2 = GetCol32B(btemp);
					byte r = (d_dsttranstab[r1] + d_srctranstab[r2]) >> 8;
					byte g = (d_dsttranstab[g1] + d_srctranstab[g2]) >> 8;
					byte b = (d_dsttranstab[b1] + d_srctranstab[b2]) >> 8;
					*lpdest = MakeCol32(r, g, b);
					*lpz = lzi >> 16;
				}
				lpdest++;
				lzi += r_zistepx;
				lpz++;
				llight += r_rstepx;
				lptex += a_ststepxwhole;
				lsfrac += a_sstepxfrac;
				lptex += lsfrac >> 16;
				lsfrac &= 0xFFFF;
				ltfrac += a_tstepxfrac;
				if (ltfrac & 0x10000)
				{
					lptex += d_affinetridesc.skinwidth;
					ltfrac &= 0xFFFF;
				}
			} while (--lcount);
		}

		pspanpackage++;
	} while (pspanpackage->count != DPS_SPAN_LIST_END);
}

//==========================================================================
//
//	D_PolysetDrawSpansRGB_8
//
//==========================================================================

extern "C" void D_PolysetDrawSpansRGB_8(spanpackage_t *pspanpackage)
{
	int		lcount;
	byte	*lpdest;
	byte	*lptex;
	int		lsfrac, ltfrac;
	int		lr;
	int		lg;
	int		lb;
	int		lzi;
	short	*lpz;

	do
	{
		lcount = d_aspancount - pspanpackage->count;

		errorterm += erroradjustup;
		if (errorterm >= 0)
		{
			d_aspancount += d_countextrastep;
			errorterm -= erroradjustdown;
		}
		else
		{
			d_aspancount += ubasestep;
		}

		if (lcount)
		{
			lpdest = (byte*)pspanpackage->pdest;
			lptex = pspanpackage->ptex;
			lpz = pspanpackage->pz;
			lsfrac = pspanpackage->sfrac;
			ltfrac = pspanpackage->tfrac;
			lr = pspanpackage->r;
			lg = pspanpackage->g;
			lb = pspanpackage->b;
			lzi = pspanpackage->zi;

			do
			{
				if ((lzi >> 16) >= *lpz)
				{
					int pix = *lptex;
					*lpdest = d_rgbtable[
						fadetable16r[pix + (lr & 0xFF00)] |
						fadetable16g[pix + (lg & 0xFF00)] |
						fadetable16b[pix + (lb & 0xFF00)]];
					*lpz = lzi >> 16;
				}
				lpdest++;
				lzi += r_zistepx;
				lpz++;
				lr += r_rstepx;
				lg += r_gstepx;
				lb += r_bstepx;
				lptex += a_ststepxwhole;
				lsfrac += a_sstepxfrac;
				lptex += lsfrac >> 16;
				lsfrac &= 0xFFFF;
				ltfrac += a_tstepxfrac;
				if (ltfrac & 0x10000)
				{
					lptex += d_affinetridesc.skinwidth;
					ltfrac &= 0xFFFF;
				}
			} while (--lcount);
		}

		pspanpackage++;
	} while (pspanpackage->count != DPS_SPAN_LIST_END);
}

//==========================================================================
//
//	D_PolysetDrawSpansRGB_16
//
//==========================================================================

extern "C" void D_PolysetDrawSpansRGB_16(spanpackage_t *pspanpackage)
{
	int		lcount;
	word	*lpdest;
	byte	*lptex;
	int		lsfrac, ltfrac;
	int		lr;
	int		lg;
	int		lb;
	int		lzi;
	short	*lpz;

	do
	{
		lcount = d_aspancount - pspanpackage->count;

		errorterm += erroradjustup;
		if (errorterm >= 0)
		{
			d_aspancount += d_countextrastep;
			errorterm -= erroradjustdown;
		}
		else
		{
			d_aspancount += ubasestep;
		}

		if (lcount)
		{
			lpdest = (word*)pspanpackage->pdest;
			lptex = pspanpackage->ptex;
			lpz = pspanpackage->pz;
			lsfrac = pspanpackage->sfrac;
			ltfrac = pspanpackage->tfrac;
			lr = pspanpackage->r;
			lg = pspanpackage->g;
			lb = pspanpackage->b;
			lzi = pspanpackage->zi;

			do
			{
				if ((lzi >> 16) >= *lpz)
				{
					int pix = *lptex;
					*lpdest = fadetable16r[pix + (lr & 0xFF00)] |
						fadetable16g[pix + (lg & 0xFF00)] |
						fadetable16b[pix + (lb & 0xFF00)];
					*lpz = lzi >> 16;
				}
				lpdest++;
				lzi += r_zistepx;
				lpz++;
				lr += r_rstepx;
				lg += r_gstepx;
				lb += r_bstepx;
				lptex += a_ststepxwhole;
				lsfrac += a_sstepxfrac;
				lptex += lsfrac >> 16;
				lsfrac &= 0xFFFF;
				ltfrac += a_tstepxfrac;
				if (ltfrac & 0x10000)
				{
					lptex += d_affinetridesc.skinwidth;
					ltfrac &= 0xFFFF;
				}
			} while (--lcount);
		}

		pspanpackage++;
	} while (pspanpackage->count != DPS_SPAN_LIST_END);
}

//==========================================================================
//
//	D_PolysetDrawSpansRGB_32
//
//==========================================================================

extern "C" void D_PolysetDrawSpansRGB_32(spanpackage_t *pspanpackage)
{
	int		lcount;
	dword	*lpdest;
	byte	*lptex;
	int		lsfrac, ltfrac;
	int		lr;
	int		lg;
	int		lb;
	int		lzi;
	short	*lpz;

	do
	{
		lcount = d_aspancount - pspanpackage->count;

		errorterm += erroradjustup;
		if (errorterm >= 0)
		{
			d_aspancount += d_countextrastep;
			errorterm -= erroradjustdown;
		}
		else
		{
			d_aspancount += ubasestep;
		}

		if (lcount)
		{
			lpdest = (dword*)pspanpackage->pdest;
			lptex = pspanpackage->ptex;
			lpz = pspanpackage->pz;
			lsfrac = pspanpackage->sfrac;
			ltfrac = pspanpackage->tfrac;
			lr = pspanpackage->r;
			lg = pspanpackage->g;
			lb = pspanpackage->b;
			lzi = pspanpackage->zi;

			do
			{
				if ((lzi >> 16) >= *lpz)
				{
					int pix = *lptex;
					*lpdest = MakeCol32(
						fadetable32r[pix + (lr & 0xFF00)],
						fadetable32g[pix + (lg & 0xFF00)],
						fadetable32b[pix + (lb & 0xFF00)]);
					*lpz = lzi >> 16;
				}
				lpdest++;
				lzi += r_zistepx;
				lpz++;
				lr += r_rstepx;
				lg += r_gstepx;
				lb += r_bstepx;
				lptex += a_ststepxwhole;
				lsfrac += a_sstepxfrac;
				lptex += lsfrac >> 16;
				lsfrac &= 0xFFFF;
				ltfrac += a_tstepxfrac;
				if (ltfrac & 0x10000)
				{
					lptex += d_affinetridesc.skinwidth;
					ltfrac &= 0xFFFF;
				}
			} while (--lcount);
		}

		pspanpackage++;
	} while (pspanpackage->count != DPS_SPAN_LIST_END);
}

//==========================================================================
//
//	D_PolysetDrawSpansRGBFuzz_8
//
//==========================================================================

extern "C" void D_PolysetDrawSpansRGBFuzz_8(spanpackage_t *pspanpackage)
{
	int		lcount;
	byte	*lpdest;
	byte	*lptex;
	int		lsfrac, ltfrac;
	int		lr;
	int		lg;
	int		lb;
	int		lzi;
	short	*lpz;

	do
	{
		lcount = d_aspancount - pspanpackage->count;

		errorterm += erroradjustup;
		if (errorterm >= 0)
		{
			d_aspancount += d_countextrastep;
			errorterm -= erroradjustdown;
		}
		else
		{
			d_aspancount += ubasestep;
		}

		if (lcount)
		{
			lpdest = (byte*)pspanpackage->pdest;
			lptex = pspanpackage->ptex;
			lpz = pspanpackage->pz;
			lsfrac = pspanpackage->sfrac;
			ltfrac = pspanpackage->tfrac;
			lr = pspanpackage->r;
			lg = pspanpackage->g;
			lb = pspanpackage->b;
			lzi = pspanpackage->zi;

			do
			{
				if ((lzi >> 16) >= *lpz)
				{
					int pix = *lptex;
					*lpdest = d_transluc[*lpdest + (d_rgbtable[
						fadetable16r[pix + (lr & 0xFF00)] |
						fadetable16g[pix + (lg & 0xFF00)] |
						fadetable16b[pix + (lb & 0xFF00)]] << 8)];
				}
				lpdest++;
				lzi += r_zistepx;
				lpz++;
				lr += r_rstepx;
				lg += r_gstepx;
				lb += r_bstepx;
				lptex += a_ststepxwhole;
				lsfrac += a_sstepxfrac;
				lptex += lsfrac >> 16;
				lsfrac &= 0xFFFF;
				ltfrac += a_tstepxfrac;
				if (ltfrac & 0x10000)
				{
					lptex += d_affinetridesc.skinwidth;
					ltfrac &= 0xFFFF;
				}
			} while (--lcount);
		}

		pspanpackage++;
	} while (pspanpackage->count != DPS_SPAN_LIST_END);
}

//==========================================================================
//
//	D_PolysetDrawSpansRGBAltFuzz_8
//
//==========================================================================

extern "C" void D_PolysetDrawSpansRGBAltFuzz_8(spanpackage_t *pspanpackage)
{
	int		lcount;
	byte	*lpdest;
	byte	*lptex;
	int		lsfrac, ltfrac;
	int		lr;
	int		lg;
	int		lb;
	int		lzi;
	short	*lpz;

	do
	{
		lcount = d_aspancount - pspanpackage->count;

		errorterm += erroradjustup;
		if (errorterm >= 0)
		{
			d_aspancount += d_countextrastep;
			errorterm -= erroradjustdown;
		}
		else
		{
			d_aspancount += ubasestep;
		}

		if (lcount)
		{
			lpdest = (byte*)pspanpackage->pdest;
			lptex = pspanpackage->ptex;
			lpz = pspanpackage->pz;
			lsfrac = pspanpackage->sfrac;
			ltfrac = pspanpackage->tfrac;
			lr = pspanpackage->r;
			lg = pspanpackage->g;
			lb = pspanpackage->b;
			lzi = pspanpackage->zi;

			do
			{
				if ((lzi >> 16) >= *lpz)
				{
					int pix = *lptex;
					*lpdest = d_transluc[(*lpdest << 8) + d_rgbtable[
						fadetable16r[pix + (lr & 0xFF00)] |
						fadetable16g[pix + (lg & 0xFF00)] |
						fadetable16b[pix + (lb & 0xFF00)]]];
					*lpz = lzi >> 16;
				}
				lpdest++;
				lzi += r_zistepx;
				lpz++;
				lr += r_rstepx;
				lg += r_gstepx;
				lb += r_bstepx;
				lptex += a_ststepxwhole;
				lsfrac += a_sstepxfrac;
				lptex += lsfrac >> 16;
				lsfrac &= 0xFFFF;
				ltfrac += a_tstepxfrac;
				if (ltfrac & 0x10000)
				{
					lptex += d_affinetridesc.skinwidth;
					ltfrac &= 0xFFFF;
				}
			} while (--lcount);
		}

		pspanpackage++;
	} while (pspanpackage->count != DPS_SPAN_LIST_END);
}

#endif

//==========================================================================
//
//	D_PolysetDrawSpansRGBFuzz_15
//
//==========================================================================

extern "C" void D_PolysetDrawSpansRGBFuzz_15(spanpackage_t *pspanpackage)
{
	int		lcount;
	word	*lpdest;
	byte	*lptex;
	int		lsfrac, ltfrac;
	int		lr;
	int		lg;
	int		lb;
	int		lzi;
	short	*lpz;

	do
	{
		lcount = d_aspancount - pspanpackage->count;

		errorterm += erroradjustup;
		if (errorterm >= 0)
		{
			d_aspancount += d_countextrastep;
			errorterm -= erroradjustdown;
		}
		else
		{
			d_aspancount += ubasestep;
		}

		if (lcount)
		{
			lpdest = (word*)pspanpackage->pdest;
			lptex = pspanpackage->ptex;
			lpz = pspanpackage->pz;
#ifdef USEASM
			lsfrac = (dword)pspanpackage->sfrac >> 16;
			ltfrac = (dword)pspanpackage->tfrac >> 16;
#else
			lsfrac = pspanpackage->sfrac;
			ltfrac = pspanpackage->tfrac;
#endif
			lr = pspanpackage->r;
			lg = pspanpackage->g;
			lb = pspanpackage->b;
			lzi = pspanpackage->zi;

			do
			{
				if ((lzi >> 16) >= *lpz)
				{
					int pix = *lptex;
					int btemp = fadetable16r[pix + (lr & 0xFF00)] |
						fadetable16g[pix + (lg & 0xFF00)] |
						fadetable16b[pix + (lb & 0xFF00)];
					byte r1 = GetCol15R(*lpdest);
					byte g1 = GetCol15G(*lpdest);
					byte b1 = GetCol15B(*lpdest);
					byte r2 = GetCol15R(btemp);
					byte g2 = GetCol15G(btemp);
					byte b2 = GetCol15B(btemp);
					byte r = (d_dsttranstab[r1] + d_srctranstab[r2]) >> 8;
					byte g = (d_dsttranstab[g1] + d_srctranstab[g2]) >> 8;
					byte b = (d_dsttranstab[b1] + d_srctranstab[b2]) >> 8;
					*lpdest = MakeCol15(r, g, b);
					*lpz = lzi >> 16;
				}
				lpdest++;
				lzi += r_zistepx;
				lpz++;
				lr += r_rstepx;
				lg += r_gstepx;
				lb += r_bstepx;
				lptex += a_ststepxwhole;
#ifdef USEASM
				lsfrac += (dword)a_sstepxfrac >> 16;
#else
				lsfrac += a_sstepxfrac;
#endif
				lptex += lsfrac >> 16;
				lsfrac &= 0xFFFF;
#ifdef USEASM
				ltfrac += (dword)a_tstepxfrac >> 16;
#else
				ltfrac += a_tstepxfrac;
#endif
				if (ltfrac & 0x10000)
				{
					lptex += d_affinetridesc.skinwidth;
					ltfrac &= 0xFFFF;
				}
			} while (--lcount);
		}

		pspanpackage++;
	} while (pspanpackage->count != DPS_SPAN_LIST_END);
}

//==========================================================================
//
//	D_PolysetDrawSpansRGBFuzz_16
//
//==========================================================================

extern "C" void D_PolysetDrawSpansRGBFuzz_16(spanpackage_t *pspanpackage)
{
	int		lcount;
	word	*lpdest;
	byte	*lptex;
	int		lsfrac, ltfrac;
	int		lr;
	int		lg;
	int		lb;
	int		lzi;
	short	*lpz;

	do
	{
		lcount = d_aspancount - pspanpackage->count;

		errorterm += erroradjustup;
		if (errorterm >= 0)
		{
			d_aspancount += d_countextrastep;
			errorterm -= erroradjustdown;
		}
		else
		{
			d_aspancount += ubasestep;
		}

		if (lcount)
		{
			lpdest = (word*)pspanpackage->pdest;
			lptex = pspanpackage->ptex;
			lpz = pspanpackage->pz;
#ifdef USEASM
			lsfrac = (dword)pspanpackage->sfrac >> 16;
			ltfrac = (dword)pspanpackage->tfrac >> 16;
#else
			lsfrac = pspanpackage->sfrac;
			ltfrac = pspanpackage->tfrac;
#endif
			lr = pspanpackage->r;
			lg = pspanpackage->g;
			lb = pspanpackage->b;
			lzi = pspanpackage->zi;

			do
			{
				if ((lzi >> 16) >= *lpz)
				{
					int pix = *lptex;
					dword btemp = fadetable16r[pix + (lr & 0xFF00)] |
						fadetable16g[pix + (lg & 0xFF00)] |
						fadetable16b[pix + (lb & 0xFF00)];
					byte r1 = GetCol16R(*lpdest);
					byte g1 = GetCol16G(*lpdest);
					byte b1 = GetCol16B(*lpdest);
					byte r2 = GetCol16R(btemp);
					byte g2 = GetCol16G(btemp);
					byte b2 = GetCol16B(btemp);
					byte r = (d_dsttranstab[r1] + d_srctranstab[r2]) >> 8;
					byte g = (d_dsttranstab[g1] + d_srctranstab[g2]) >> 8;
					byte b = (d_dsttranstab[b1] + d_srctranstab[b2]) >> 8;
					*lpdest = MakeCol16(r, g, b);
					*lpz = lzi >> 16;
				}
				lpdest++;
				lzi += r_zistepx;
				lpz++;
				lr += r_rstepx;
				lg += r_gstepx;
				lb += r_bstepx;
				lptex += a_ststepxwhole;
#ifdef USEASM
				lsfrac += (dword)a_sstepxfrac >> 16;
#else
				lsfrac += a_sstepxfrac;
#endif
				lptex += lsfrac >> 16;
				lsfrac &= 0xFFFF;
#ifdef USEASM
				ltfrac += (dword)a_tstepxfrac >> 16;
#else
				ltfrac += a_tstepxfrac;
#endif
				if (ltfrac & 0x10000)
				{
					lptex += d_affinetridesc.skinwidth;
					ltfrac &= 0xFFFF;
				}
			} while (--lcount);
		}

		pspanpackage++;
	} while (pspanpackage->count != DPS_SPAN_LIST_END);
}

#ifndef USEASM

//==========================================================================
//
//	D_PolysetDrawSpansRGBFuzz_32
//
//==========================================================================

extern "C" void D_PolysetDrawSpansRGBFuzz_32(spanpackage_t *pspanpackage)
{
	int		lcount;
	dword	*lpdest;
	byte	*lptex;
	int		lsfrac, ltfrac;
	int		lr;
	int		lg;
	int		lb;
	int		lzi;
	short	*lpz;

	do
	{
		lcount = d_aspancount - pspanpackage->count;

		errorterm += erroradjustup;
		if (errorterm >= 0)
		{
			d_aspancount += d_countextrastep;
			errorterm -= erroradjustdown;
		}
		else
		{
			d_aspancount += ubasestep;
		}

		if (lcount)
		{
			lpdest = (dword*)pspanpackage->pdest;
			lptex = pspanpackage->ptex;
			lpz = pspanpackage->pz;
			lsfrac = pspanpackage->sfrac;
			ltfrac = pspanpackage->tfrac;
			lr = pspanpackage->r;
			lg = pspanpackage->g;
			lb = pspanpackage->b;
			lzi = pspanpackage->zi;

			do
			{
				if ((lzi >> 16) >= *lpz)
				{
					int pix = *lptex;
					byte r1 = GetCol32R(*lpdest);
					byte g1 = GetCol32G(*lpdest);
					byte b1 = GetCol32B(*lpdest);
					byte r2 = fadetable32r[pix + (lr & 0xFF00)];
					byte g2 = fadetable32g[pix + (lg & 0xFF00)];
					byte b2 = fadetable32b[pix + (lb & 0xFF00)];
					byte r = (d_dsttranstab[r1] + d_srctranstab[r2]) >> 8;
					byte g = (d_dsttranstab[g1] + d_srctranstab[g2]) >> 8;
					byte b = (d_dsttranstab[b1] + d_srctranstab[b2]) >> 8;
					*lpdest = MakeCol32(r, g, b);
					*lpz = lzi >> 16;
				}
				lpdest++;
				lzi += r_zistepx;
				lpz++;
				lr += r_rstepx;
				lg += r_gstepx;
				lb += r_bstepx;
				lptex += a_ststepxwhole;
				lsfrac += a_sstepxfrac;
				lptex += lsfrac >> 16;
				lsfrac &= 0xFFFF;
				ltfrac += a_tstepxfrac;
				if (ltfrac & 0x10000)
				{
					lptex += d_affinetridesc.skinwidth;
					ltfrac &= 0xFFFF;
				}
			} while (--lcount);
		}

		pspanpackage++;
	} while (pspanpackage->count != DPS_SPAN_LIST_END);
}

#endif

//**************************************************************************
//
//	$Log$
//	Revision 1.10  2002/01/07 12:16:42  dj_jl
//	Changed copyright year
//
//	Revision 1.9  2001/12/18 19:01:34  dj_jl
//	Changes for MSVC asm
//	
//	Revision 1.8  2001/10/18 17:36:31  dj_jl
//	A lots of changes for Alpha 2
//	
//	Revision 1.7  2001/09/05 12:56:24  dj_jl
//	Release changes
//	
//	Revision 1.6  2001/09/05 12:21:42  dj_jl
//	Release changes
//	
//	Revision 1.5  2001/08/15 17:12:23  dj_jl
//	Optimized model drawing
//	
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
