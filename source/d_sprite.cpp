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
//**	Copyright (C) 1999-2006 Jānis Legzdiņš
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
//**	software top-level rasterization driver module for drawing sprites
//**	
//**************************************************************************

// HEADER FILES ------------------------------------------------------------

#include "d_local.h"
int D_MipLevelForScale (float scale);
extern float	scale_for_mip;

// MACROS ------------------------------------------------------------------

//	Theoretically cliping can give only 4 new vertexes. In practice due to
// roundof errors we can get more extra vertexes
#define NUM_EXTRA_VERTS	16
#define MAX_STACK_VERTS	64

// TYPES -------------------------------------------------------------------

// EXTERNAL FUNCTION PROTOTYPES --------------------------------------------

// PUBLIC FUNCTION PROTOTYPES ----------------------------------------------

// PRIVATE FUNCTION PROTOTYPES ---------------------------------------------

// EXTERNAL DATA DECLARATIONS ----------------------------------------------

// PUBLIC DATA DEFINITIONS -------------------------------------------------

// PRIVATE DATA DEFINITIONS ------------------------------------------------

static int				minindex;
static int				maxindex;

static sspan_t			*sprite_spans;

static fixed_t			spr_texturemins[2];

static float			r_nearzi;

static TVec				*p_vert;
static int				r_emited;
static int				max_emited;

// CODE --------------------------------------------------------------------

#if !USE_ASM_I386

//==========================================================================
//
//	D_DrawSpriteSpans_8
//
//==========================================================================

extern "C" void D_DrawSpriteSpans_8(sspan_t *pspan)
{
	int			count, spancount, izi, izistep;
	byte		*pbase, *pdest;
	fixed_t		s, t, snext, tnext, sstep, tstep;
	float		sdivz, tdivz, zi, z, du, dv, spancountminus1;
	float		sdivz8stepu, tdivz8stepu, zi8stepu;
	byte		btemp;
	short		*pz;

	sstep = 0;	// keep compiler happy
	tstep = 0;	// ditto

	pbase = (byte*)cacheblock;

	sdivz8stepu = d_sdivzstepu * 8;
	tdivz8stepu = d_tdivzstepu * 8;
	zi8stepu = d_zistepu * 8;

	// we count on FP exceptions being turned off to avoid range problems
	izistep = (int)(d_zistepu * 0x8000 * 0x10000);

	do
	{
		pdest = (byte*)scrn + ylookup[pspan->v] + pspan->u;
		pz = zbuffer + ylookup[pspan->v] + pspan->u;

		count = pspan->count;

		if (count <= 0)
			goto NextSpan;

		// calculate the initial s/z, t/z, 1/z, s, and t and clamp
		du = (float)pspan->u;
		dv = (float)pspan->v;

		sdivz = d_sdivzorigin + dv*d_sdivzstepv + du*d_sdivzstepu;
		tdivz = d_tdivzorigin + dv*d_tdivzstepv + du*d_tdivzstepu;
		zi = d_ziorigin + dv*d_zistepv + du*d_zistepu;
		z = (float)0x10000 / zi;	// prescale to 16.16 fixed-point
		// we count on FP exceptions being turned off to avoid range problems
		izi = (int)(zi * 0x8000 * 0x10000);

		s = (int)(sdivz * z) + sadjust;
		if (s > bbextents)
			s = bbextents;
		else if (s < 0)
			s = 0;

		t = (int)(tdivz * z) + tadjust;
		if (t > bbextentt)
			t = bbextentt;
		else if (t < 0)
			t = 0;

		do
		{
			// calculate s and t at the far end of the span
			if (count >= 8)
				spancount = 8;
			else
				spancount = count;

			count -= spancount;

			if (count)
			{
				// calculate s/z, t/z, zi->fixed s and t at far end of span,
				// calculate s and t steps across span by shifting
				sdivz += sdivz8stepu;
				tdivz += tdivz8stepu;
				zi += zi8stepu;
				z = (float)0x10000 / zi;	// prescale to 16.16 fixed-point

				snext = (int)(sdivz * z) + sadjust;
				if (snext > bbextents)
					snext = bbextents;
				else if (snext < 8)
					snext = 8;	// prevent round-off error on <0 steps from
								// from causing overstepping & running off
								// the edge of the texture

				tnext = (int)(tdivz * z) + tadjust;
				if (tnext > bbextentt)
					tnext = bbextentt;
				else if (tnext < 8)
					tnext = 8;	// guard against round-off error on <0 steps

				sstep = (snext - s) >> 3;
				tstep = (tnext - t) >> 3;
			}
			else
			{
				// calculate s/z, t/z, zi->fixed s and t at last pixel in
				// span (so can't step off polygon), clamp, calculate s and
				// t steps across span by division, biasing steps low so we
				// don't run off the texture
				spancountminus1 = (float)(spancount - 1);
				sdivz += d_sdivzstepu * spancountminus1;
				tdivz += d_tdivzstepu * spancountminus1;
				zi += d_zistepu * spancountminus1;
				z = (float)0x10000 / zi;	// prescale to 16.16 fixed-point
				snext = (int)(sdivz * z) + sadjust;
				if (snext > bbextents)
					snext = bbextents;
				else if (snext < 8)
					snext = 8;	// prevent round-off error on <0 steps from
								// from causing overstepping & running off
								// the edge of the texture

				tnext = (int)(tdivz * z) + tadjust;
				if (tnext > bbextentt)
					tnext = bbextentt;
				else if (tnext < 8)
					tnext = 8;	// guard against round-off error on <0 steps

				if (spancount > 1)
				{
					sstep = (snext - s) / (spancount - 1);
					tstep = (tnext - t) / (spancount - 1);
				}
			}

			do
			{
				btemp = pbase[(s >> 16) + (t >> 16) * cachewidth];
				if (btemp)
				{
					if (*pz <= (izi >> 16))
					{
						*pz = izi >> 16;
						*pdest = btemp;
					}
				}

				izi += izistep;
				pdest++;
				pz++;
				s += sstep;
				t += tstep;
			} while (--spancount > 0);

			s = snext;
			t = tnext;

		} while (count > 0);

NextSpan:
		pspan++;

	} while (pspan->count != DS_SPAN_LIST_END);
}

//==========================================================================
//
//	D_DrawSpriteSpans_16
//
//==========================================================================

extern "C" void D_DrawSpriteSpans_16(sspan_t *pspan)
{
	int			count, spancount, izi, izistep;
	word		*pbase, *pdest;
	fixed_t		s, t, snext, tnext, sstep, tstep;
	float		sdivz, tdivz, zi, z, du, dv, spancountminus1;
	float		sdivz8stepu, tdivz8stepu, zi8stepu;
	word		btemp;
	short		*pz;

	sstep = 0;	// keep compiler happy
	tstep = 0;	// ditto

	pbase = (word*)cacheblock;

	sdivz8stepu = d_sdivzstepu * 8;
	tdivz8stepu = d_tdivzstepu * 8;
	zi8stepu = d_zistepu * 8;

	// we count on FP exceptions being turned off to avoid range problems
	izistep = (int)(d_zistepu * 0x8000 * 0x10000);

	do
	{
		pdest = (word*)scrn + ylookup[pspan->v] + pspan->u;
		pz = zbuffer + ylookup[pspan->v] + pspan->u;

		count = pspan->count;

		if (count <= 0)
			goto NextSpan;

		// calculate the initial s/z, t/z, 1/z, s, and t and clamp
		du = (float)pspan->u;
		dv = (float)pspan->v;

		sdivz = d_sdivzorigin + dv*d_sdivzstepv + du*d_sdivzstepu;
		tdivz = d_tdivzorigin + dv*d_tdivzstepv + du*d_tdivzstepu;
		zi = d_ziorigin + dv*d_zistepv + du*d_zistepu;
		z = (float)0x10000 / zi;	// prescale to 16.16 fixed-point
		// we count on FP exceptions being turned off to avoid range problems
		izi = (int)(zi * 0x8000 * 0x10000);

		s = (int)(sdivz * z) + sadjust;
		if (s > bbextents)
			s = bbextents;
		else if (s < 0)
			s = 0;

		t = (int)(tdivz * z) + tadjust;
		if (t > bbextentt)
			t = bbextentt;
		else if (t < 0)
			t = 0;

		do
		{
			// calculate s and t at the far end of the span
			if (count >= 8)
				spancount = 8;
			else
				spancount = count;

			count -= spancount;

			if (count)
			{
				// calculate s/z, t/z, zi->fixed s and t at far end of span,
				// calculate s and t steps across span by shifting
				sdivz += sdivz8stepu;
				tdivz += tdivz8stepu;
				zi += zi8stepu;
				z = (float)0x10000 / zi;	// prescale to 16.16 fixed-point

				snext = (int)(sdivz * z) + sadjust;
				if (snext > bbextents)
					snext = bbextents;
				else if (snext < 8)
					snext = 8;	// prevent round-off error on <0 steps from
								// from causing overstepping & running off
								// the edge of the texture

				tnext = (int)(tdivz * z) + tadjust;
				if (tnext > bbextentt)
					tnext = bbextentt;
				else if (tnext < 8)
					tnext = 8;	// guard against round-off error on <0 steps

				sstep = (snext - s) >> 3;
				tstep = (tnext - t) >> 3;
			}
			else
			{
				// calculate s/z, t/z, zi->fixed s and t at last pixel in
				// span (so can't step off polygon), clamp, calculate s and
				// t steps across span by division, biasing steps low so we
				// don't run off the texture
				spancountminus1 = (float)(spancount - 1);
				sdivz += d_sdivzstepu * spancountminus1;
				tdivz += d_tdivzstepu * spancountminus1;
				zi += d_zistepu * spancountminus1;
				z = (float)0x10000 / zi;	// prescale to 16.16 fixed-point
				snext = (int)(sdivz * z) + sadjust;
				if (snext > bbextents)
					snext = bbextents;
				else if (snext < 8)
					snext = 8;	// prevent round-off error on <0 steps from
								// from causing overstepping & running off
								// the edge of the texture

				tnext = (int)(tdivz * z) + tadjust;
				if (tnext > bbextentt)
					tnext = bbextentt;
				else if (tnext < 8)
					tnext = 8;	// guard against round-off error on <0 steps

				if (spancount > 1)
				{
					sstep = (snext - s) / (spancount - 1);
					tstep = (tnext - t) / (spancount - 1);
				}
			}

			do
			{
				btemp = pbase[(s >> 16) + (t >> 16) * cachewidth];
				if (btemp)
				{
					if (*pz <= (izi >> 16))
					{
						*pz = izi >> 16;
						*pdest = btemp;
					}
				}

				izi += izistep;
				pdest++;
				pz++;
				s += sstep;
				t += tstep;
			} while (--spancount > 0);

			s = snext;
			t = tnext;

		} while (count > 0);

NextSpan:
		pspan++;

	} while (pspan->count != DS_SPAN_LIST_END);
}

//==========================================================================
//
//	D_DrawSpriteSpans_32
//
//==========================================================================

extern "C" void D_DrawSpriteSpans_32(sspan_t *pspan)
{
	int			count, spancount, izi, izistep;
	vuint32		*pbase, *pdest;
	fixed_t		s, t, snext, tnext, sstep, tstep;
	float		sdivz, tdivz, zi, z, du, dv, spancountminus1;
	float		sdivz8stepu, tdivz8stepu, zi8stepu;
	vuint32		btemp;
	short		*pz;

	sstep = 0;	// keep compiler happy
	tstep = 0;	// ditto

	pbase = (vuint32*)cacheblock;

	sdivz8stepu = d_sdivzstepu * 8;
	tdivz8stepu = d_tdivzstepu * 8;
	zi8stepu = d_zistepu * 8;

	// we count on FP exceptions being turned off to avoid range problems
	izistep = (int)(d_zistepu * 0x8000 * 0x10000);

	do
	{
		pdest = (vuint32*)scrn + ylookup[pspan->v] + pspan->u;
		pz = zbuffer + ylookup[pspan->v] + pspan->u;

		count = pspan->count;

		if (count <= 0)
			goto NextSpan;

		// calculate the initial s/z, t/z, 1/z, s, and t and clamp
		du = (float)pspan->u;
		dv = (float)pspan->v;

		sdivz = d_sdivzorigin + dv*d_sdivzstepv + du*d_sdivzstepu;
		tdivz = d_tdivzorigin + dv*d_tdivzstepv + du*d_tdivzstepu;
		zi = d_ziorigin + dv*d_zistepv + du*d_zistepu;
		z = (float)0x10000 / zi;	// prescale to 16.16 fixed-point
		// we count on FP exceptions being turned off to avoid range problems
		izi = (int)(zi * 0x8000 * 0x10000);

		s = (int)(sdivz * z) + sadjust;
		if (s > bbextents)
			s = bbextents;
		else if (s < 0)
			s = 0;

		t = (int)(tdivz * z) + tadjust;
		if (t > bbextentt)
			t = bbextentt;
		else if (t < 0)
			t = 0;

		do
		{
			// calculate s and t at the far end of the span
			if (count >= 8)
				spancount = 8;
			else
				spancount = count;

			count -= spancount;

			if (count)
			{
				// calculate s/z, t/z, zi->fixed s and t at far end of span,
				// calculate s and t steps across span by shifting
				sdivz += sdivz8stepu;
				tdivz += tdivz8stepu;
				zi += zi8stepu;
				z = (float)0x10000 / zi;	// prescale to 16.16 fixed-point

				snext = (int)(sdivz * z) + sadjust;
				if (snext > bbextents)
					snext = bbextents;
				else if (snext < 8)
					snext = 8;	// prevent round-off error on <0 steps from
								// from causing overstepping & running off
								// the edge of the texture

				tnext = (int)(tdivz * z) + tadjust;
				if (tnext > bbextentt)
					tnext = bbextentt;
				else if (tnext < 8)
					tnext = 8;	// guard against round-off error on <0 steps

				sstep = (snext - s) >> 3;
				tstep = (tnext - t) >> 3;
			}
			else
			{
				// calculate s/z, t/z, zi->fixed s and t at last pixel in
				// span (so can't step off polygon), clamp, calculate s and
				// t steps across span by division, biasing steps low so we
				// don't run off the texture
				spancountminus1 = (float)(spancount - 1);
				sdivz += d_sdivzstepu * spancountminus1;
				tdivz += d_tdivzstepu * spancountminus1;
				zi += d_zistepu * spancountminus1;
				z = (float)0x10000 / zi;	// prescale to 16.16 fixed-point
				snext = (int)(sdivz * z) + sadjust;
				if (snext > bbextents)
					snext = bbextents;
				else if (snext < 8)
					snext = 8;	// prevent round-off error on <0 steps from
								// from causing overstepping & running off
								// the edge of the texture

				tnext = (int)(tdivz * z) + tadjust;
				if (tnext > bbextentt)
					tnext = bbextentt;
				else if (tnext < 8)
					tnext = 8;	// guard against round-off error on <0 steps

				if (spancount > 1)
				{
					sstep = (snext - s) / (spancount - 1);
					tstep = (tnext - t) / (spancount - 1);
				}
			}

			do
			{
				btemp = pbase[(s >> 16) + (t >> 16) * cachewidth];
				if (btemp)
				{
					if (*pz <= (izi >> 16))
					{
						*pz = izi >> 16;
						*pdest = btemp;
					}
				}

				izi += izistep;
				pdest++;
				pz++;
				s += sstep;
				t += tstep;
			} while (--spancount > 0);

			s = snext;
			t = tnext;

		} while (count > 0);

NextSpan:
		pspan++;

	} while (pspan->count != DS_SPAN_LIST_END);
}

#endif

//==========================================================================
//
//	D_DrawFuzzSpriteSpans_8
//
//==========================================================================

extern "C" void D_DrawFuzzSpriteSpans_8(sspan_t *pspan)
{
	int			count, spancount, izi, izistep;
	byte		*pbase, *pdest;
	fixed_t		s, t, snext, tnext, sstep, tstep;
	float		sdivz, tdivz, zi, z, du, dv, spancountminus1;
	float		sdivz8stepu, tdivz8stepu, zi8stepu;
	byte		btemp;
	short		*pz;

	sstep = 0;	// keep compiler happy
	tstep = 0;	// ditto

	pbase = (byte*)cacheblock;

	sdivz8stepu = d_sdivzstepu * 8;
	tdivz8stepu = d_tdivzstepu * 8;
	zi8stepu = d_zistepu * 8;

	// we count on FP exceptions being turned off to avoid range problems
	izistep = (int)(d_zistepu * 0x8000 * 0x10000);

	do
	{
		pdest = (byte*)scrn + ylookup[pspan->v] + pspan->u;
		pz = zbuffer + ylookup[pspan->v] + pspan->u;

		count = pspan->count;

		if (count <= 0)
			goto NextSpan;

		// calculate the initial s/z, t/z, 1/z, s, and t and clamp
		du = (float)pspan->u;
		dv = (float)pspan->v;

		sdivz = d_sdivzorigin + dv*d_sdivzstepv + du*d_sdivzstepu;
		tdivz = d_tdivzorigin + dv*d_tdivzstepv + du*d_tdivzstepu;
		zi = d_ziorigin + dv*d_zistepv + du*d_zistepu;
		z = (float)0x10000 / zi;	// prescale to 16.16 fixed-point
		// we count on FP exceptions being turned off to avoid range problems
		izi = (int)(zi * 0x8000 * 0x10000);

		s = (int)(sdivz * z) + sadjust;
		if (s > bbextents)
			s = bbextents;
		else if (s < 0)
			s = 0;

		t = (int)(tdivz * z) + tadjust;
		if (t > bbextentt)
			t = bbextentt;
		else if (t < 0)
			t = 0;

		do
		{
			// calculate s and t at the far end of the span
			if (count >= 8)
				spancount = 8;
			else
				spancount = count;

			count -= spancount;

			if (count)
			{
				// calculate s/z, t/z, zi->fixed s and t at far end of span,
				// calculate s and t steps across span by shifting
				sdivz += sdivz8stepu;
				tdivz += tdivz8stepu;
				zi += zi8stepu;
				z = (float)0x10000 / zi;	// prescale to 16.16 fixed-point

				snext = (int)(sdivz * z) + sadjust;
				if (snext > bbextents)
					snext = bbextents;
				else if (snext < 8)
					snext = 8;	// prevent round-off error on <0 steps from
								// from causing overstepping & running off
								// the edge of the texture

				tnext = (int)(tdivz * z) + tadjust;
				if (tnext > bbextentt)
					tnext = bbextentt;
				else if (tnext < 8)
					tnext = 8;	// guard against round-off error on <0 steps

				sstep = (snext - s) >> 3;
				tstep = (tnext - t) >> 3;
			}
			else
			{
				// calculate s/z, t/z, zi->fixed s and t at last pixel in
				// span (so can't step off polygon), clamp, calculate s and
				// t steps across span by division, biasing steps low so we
				// don't run off the texture
				spancountminus1 = (float)(spancount - 1);
				sdivz += d_sdivzstepu * spancountminus1;
				tdivz += d_tdivzstepu * spancountminus1;
				zi += d_zistepu * spancountminus1;
				z = (float)0x10000 / zi;	// prescale to 16.16 fixed-point
				snext = (int)(sdivz * z) + sadjust;
				if (snext > bbextents)
					snext = bbextents;
				else if (snext < 8)
					snext = 8;	// prevent round-off error on <0 steps from
								// from causing overstepping & running off
								// the edge of the texture

				tnext = (int)(tdivz * z) + tadjust;
				if (tnext > bbextentt)
					tnext = bbextentt;
				else if (tnext < 8)
					tnext = 8;	// guard against round-off error on <0 steps

				if (spancount > 1)
				{
					sstep = (snext - s) / (spancount - 1);
					tstep = (tnext - t) / (spancount - 1);
				}
			}

			do
			{
				btemp = pbase[(s >> 16) + (t >> 16) * cachewidth];
				if (btemp)
				{
					if (*pz <= (izi >> 16))
					{
						*pdest = d_transluc[*pdest + (btemp << 8)];
					}
				}

				izi += izistep;
				pdest++;
				pz++;
				s += sstep;
				t += tstep;
			} while (--spancount > 0);

			s = snext;
			t = tnext;

		} while (count > 0);

NextSpan:
		pspan++;

	} while (pspan->count != DS_SPAN_LIST_END);
}

//==========================================================================
//
//	D_DrawAltFuzzSpriteSpans_8
//
//==========================================================================

extern "C" void D_DrawAltFuzzSpriteSpans_8(sspan_t *pspan)
{
	int			count, spancount, izi, izistep;
	byte		*pbase, *pdest;
	fixed_t		s, t, snext, tnext, sstep, tstep;
	float		sdivz, tdivz, zi, z, du, dv, spancountminus1;
	float		sdivz8stepu, tdivz8stepu, zi8stepu;
	byte		btemp;
	short		*pz;

	sstep = 0;	// keep compiler happy
	tstep = 0;	// ditto

	pbase = (byte*)cacheblock;

	sdivz8stepu = d_sdivzstepu * 8;
	tdivz8stepu = d_tdivzstepu * 8;
	zi8stepu = d_zistepu * 8;

	// we count on FP exceptions being turned off to avoid range problems
	izistep = (int)(d_zistepu * 0x8000 * 0x10000);

	do
	{
		pdest = (byte *)scrn + ylookup[pspan->v] + pspan->u;
		pz = zbuffer + ylookup[pspan->v] + pspan->u;

		count = pspan->count;

		if (count <= 0)
			goto NextSpan;

		// calculate the initial s/z, t/z, 1/z, s, and t and clamp
		du = (float)pspan->u;
		dv = (float)pspan->v;

		sdivz = d_sdivzorigin + dv*d_sdivzstepv + du*d_sdivzstepu;
		tdivz = d_tdivzorigin + dv*d_tdivzstepv + du*d_tdivzstepu;
		zi = d_ziorigin + dv*d_zistepv + du*d_zistepu;
		z = (float)0x10000 / zi;	// prescale to 16.16 fixed-point
		// we count on FP exceptions being turned off to avoid range problems
		izi = (int)(zi * 0x8000 * 0x10000);

		s = (int)(sdivz * z) + sadjust;
		if (s > bbextents)
			s = bbextents;
		else if (s < 0)
			s = 0;

		t = (int)(tdivz * z) + tadjust;
		if (t > bbextentt)
			t = bbextentt;
		else if (t < 0)
			t = 0;

		do
		{
			// calculate s and t at the far end of the span
			if (count >= 8)
				spancount = 8;
			else
				spancount = count;

			count -= spancount;

			if (count)
			{
				// calculate s/z, t/z, zi->fixed s and t at far end of span,
				// calculate s and t steps across span by shifting
				sdivz += sdivz8stepu;
				tdivz += tdivz8stepu;
				zi += zi8stepu;
				z = (float)0x10000 / zi;	// prescale to 16.16 fixed-point

				snext = (int)(sdivz * z) + sadjust;
				if (snext > bbextents)
					snext = bbextents;
				else if (snext < 8)
					snext = 8;	// prevent round-off error on <0 steps from
								// from causing overstepping & running off
								// the edge of the texture

				tnext = (int)(tdivz * z) + tadjust;
				if (tnext > bbextentt)
					tnext = bbextentt;
				else if (tnext < 8)
					tnext = 8;	// guard against round-off error on <0 steps

				sstep = (snext - s) >> 3;
				tstep = (tnext - t) >> 3;
			}
			else
			{
				// calculate s/z, t/z, zi->fixed s and t at last pixel in
				// span (so can't step off polygon), clamp, calculate s and
				// t steps across span by division, biasing steps low so we
				// don't run off the texture
				spancountminus1 = (float)(spancount - 1);
				sdivz += d_sdivzstepu * spancountminus1;
				tdivz += d_tdivzstepu * spancountminus1;
				zi += d_zistepu * spancountminus1;
				z = (float)0x10000 / zi;	// prescale to 16.16 fixed-point
				snext = (int)(sdivz * z) + sadjust;
				if (snext > bbextents)
					snext = bbextents;
				else if (snext < 8)
					snext = 8;	// prevent round-off error on <0 steps from
								// from causing overstepping & running off
								// the edge of the texture

				tnext = (int)(tdivz * z) + tadjust;
				if (tnext > bbextentt)
					tnext = bbextentt;
				else if (tnext < 8)
					tnext = 8;	// guard against round-off error on <0 steps

				if (spancount > 1)
				{
					sstep = (snext - s) / (spancount - 1);
					tstep = (tnext - t) / (spancount - 1);
				}
			}

			do
			{
				btemp = pbase[(s >> 16) + (t >> 16) * cachewidth];
				if (btemp)
				{
					if (*pz <= (izi >> 16))
					{
						*pdest = d_transluc[(*pdest << 8) + btemp];
					}
				}

				izi += izistep;
				pdest++;
				pz++;
				s += sstep;
				t += tstep;
			} while (--spancount > 0);

			s = snext;
			t = tnext;

		} while (count > 0);

NextSpan:
		pspan++;

	} while (pspan->count != DS_SPAN_LIST_END);
}

//==========================================================================
//
//	D_DrawFuzzSpriteSpans_15
//
//==========================================================================

#define _MakeCol15(r, g, b)	((((r) << 7) & 0x7c00) | (((g) << 2) & 0x03e0) | (((b) >> 3) & 0x001f))
#define _GetCol15R(col)		(((col) & 0x7c00) >> 7)
#define _GetCol15G(col)		(((col) & 0x03e0) >> 2)
#define _GetCol15B(col)		(((col) & 0x001f) << 3)

extern "C" void D_DrawFuzzSpriteSpans_15(sspan_t *pspan)
{
	int			count, spancount, izi, izistep;
	word		*pbase, *pdest;
	fixed_t		s, t, snext, tnext, sstep, tstep;
	float		sdivz, tdivz, zi, z, du, dv, spancountminus1;
	float		sdivz8stepu, tdivz8stepu, zi8stepu;
	word		btemp;
	short		*pz;

	sstep = 0;	// keep compiler happy
	tstep = 0;	// ditto

	pbase = (word*)cacheblock;

	sdivz8stepu = d_sdivzstepu * 8;
	tdivz8stepu = d_tdivzstepu * 8;
	zi8stepu = d_zistepu * 8;

	// we count on FP exceptions being turned off to avoid range problems
	izistep = (int)(d_zistepu * 0x8000 * 0x10000);

	do
	{
		pdest = (word*)scrn + ylookup[pspan->v] + pspan->u;
		pz = zbuffer + ylookup[pspan->v] + pspan->u;

		count = pspan->count;

		if (count <= 0)
			goto NextSpan;

		// calculate the initial s/z, t/z, 1/z, s, and t and clamp
		du = (float)pspan->u;
		dv = (float)pspan->v;

		sdivz = d_sdivzorigin + dv*d_sdivzstepv + du*d_sdivzstepu;
		tdivz = d_tdivzorigin + dv*d_tdivzstepv + du*d_tdivzstepu;
		zi = d_ziorigin + dv*d_zistepv + du*d_zistepu;
		z = (float)0x10000 / zi;	// prescale to 16.16 fixed-point
		// we count on FP exceptions being turned off to avoid range problems
		izi = (int)(zi * 0x8000 * 0x10000);

		s = (int)(sdivz * z) + sadjust;
		if (s > bbextents)
			s = bbextents;
		else if (s < 0)
			s = 0;

		t = (int)(tdivz * z) + tadjust;
		if (t > bbextentt)
			t = bbextentt;
		else if (t < 0)
			t = 0;

		do
		{
			// calculate s and t at the far end of the span
			if (count >= 8)
				spancount = 8;
			else
				spancount = count;

			count -= spancount;

			if (count)
			{
				// calculate s/z, t/z, zi->fixed s and t at far end of span,
				// calculate s and t steps across span by shifting
				sdivz += sdivz8stepu;
				tdivz += tdivz8stepu;
				zi += zi8stepu;
				z = (float)0x10000 / zi;	// prescale to 16.16 fixed-point

				snext = (int)(sdivz * z) + sadjust;
				if (snext > bbextents)
					snext = bbextents;
				else if (snext < 8)
					snext = 8;	// prevent round-off error on <0 steps from
								// from causing overstepping & running off
								// the edge of the texture

				tnext = (int)(tdivz * z) + tadjust;
				if (tnext > bbextentt)
					tnext = bbextentt;
				else if (tnext < 8)
					tnext = 8;	// guard against round-off error on <0 steps

				sstep = (snext - s) >> 3;
				tstep = (tnext - t) >> 3;
			}
			else
			{
				// calculate s/z, t/z, zi->fixed s and t at last pixel in
				// span (so can't step off polygon), clamp, calculate s and
				// t steps across span by division, biasing steps low so we
				// don't run off the texture
				spancountminus1 = (float)(spancount - 1);
				sdivz += d_sdivzstepu * spancountminus1;
				tdivz += d_tdivzstepu * spancountminus1;
				zi += d_zistepu * spancountminus1;
				z = (float)0x10000 / zi;	// prescale to 16.16 fixed-point
				snext = (int)(sdivz * z) + sadjust;
				if (snext > bbextents)
					snext = bbextents;
				else if (snext < 8)
					snext = 8;	// prevent round-off error on <0 steps from
								// from causing overstepping & running off
								// the edge of the texture

				tnext = (int)(tdivz * z) + tadjust;
				if (tnext > bbextentt)
					tnext = bbextentt;
				else if (tnext < 8)
					tnext = 8;	// guard against round-off error on <0 steps

				if (spancount > 1)
				{
					sstep = (snext - s) / (spancount - 1);
					tstep = (tnext - t) / (spancount - 1);
				}
			}

			do
			{
				btemp = pbase[(s >> 16) + (t >> 16) * cachewidth];
				if (btemp)
				{
					if (*pz <= (izi >> 16))
					{
						byte r1 = _GetCol15R(*pdest);
						byte g1 = _GetCol15G(*pdest);
						byte b1 = _GetCol15B(*pdest);
						byte r2 = _GetCol15R(btemp);
						byte g2 = _GetCol15G(btemp);
						byte b2 = _GetCol15B(btemp);
						byte r = (d_dsttranstab[r1] + d_srctranstab[r2]) >> 8;
						byte g = (d_dsttranstab[g1] + d_srctranstab[g2]) >> 8;
						byte b = (d_dsttranstab[b1] + d_srctranstab[b2]) >> 8;
						*pdest = _MakeCol15(r, g, b);
					}
				}

				izi += izistep;
				pdest++;
				pz++;
				s += sstep;
				t += tstep;
			} while (--spancount > 0);

			s = snext;
			t = tnext;

		} while (count > 0);

NextSpan:
		pspan++;

	} while (pspan->count != DS_SPAN_LIST_END);
}

//==========================================================================
//
//	D_DrawFuzzSpriteSpans_16
//
//==========================================================================

#define _MakeCol16(r, g, b)	((((r) << 8) & 0xf800) | (((g) << 3) & 0x07e0) | (((b) >> 3) & 0x001f))
#define _GetCol16R(col)		(((col) & 0xf800) >> 8)
#define _GetCol16G(col)		(((col) & 0x07e0) >> 3)
#define _GetCol16B(col)		(((col) & 0x001f) << 3)

extern "C" void D_DrawFuzzSpriteSpans_16(sspan_t *pspan)
{
	int			count, spancount, izi, izistep;
	word		*pbase, *pdest;
	fixed_t		s, t, snext, tnext, sstep, tstep;
	float		sdivz, tdivz, zi, z, du, dv, spancountminus1;
	float		sdivz8stepu, tdivz8stepu, zi8stepu;
	word		btemp;
	short		*pz;

	sstep = 0;	// keep compiler happy
	tstep = 0;	// ditto

	pbase = (word*)cacheblock;

	sdivz8stepu = d_sdivzstepu * 8;
	tdivz8stepu = d_tdivzstepu * 8;
	zi8stepu = d_zistepu * 8;

	// we count on FP exceptions being turned off to avoid range problems
	izistep = (int)(d_zistepu * 0x8000 * 0x10000);

	do
	{
		pdest = (word*)scrn + ylookup[pspan->v] + pspan->u;
		pz = zbuffer + ylookup[pspan->v] + pspan->u;

		count = pspan->count;

		if (count <= 0)
			goto NextSpan;

		// calculate the initial s/z, t/z, 1/z, s, and t and clamp
		du = (float)pspan->u;
		dv = (float)pspan->v;

		sdivz = d_sdivzorigin + dv*d_sdivzstepv + du*d_sdivzstepu;
		tdivz = d_tdivzorigin + dv*d_tdivzstepv + du*d_tdivzstepu;
		zi = d_ziorigin + dv*d_zistepv + du*d_zistepu;
		z = (float)0x10000 / zi;	// prescale to 16.16 fixed-point
		// we count on FP exceptions being turned off to avoid range problems
		izi = (int)(zi * 0x8000 * 0x10000);

		s = (int)(sdivz * z) + sadjust;
		if (s > bbextents)
			s = bbextents;
		else if (s < 0)
			s = 0;

		t = (int)(tdivz * z) + tadjust;
		if (t > bbextentt)
			t = bbextentt;
		else if (t < 0)
			t = 0;

		do
		{
			// calculate s and t at the far end of the span
			if (count >= 8)
				spancount = 8;
			else
				spancount = count;

			count -= spancount;

			if (count)
			{
				// calculate s/z, t/z, zi->fixed s and t at far end of span,
				// calculate s and t steps across span by shifting
				sdivz += sdivz8stepu;
				tdivz += tdivz8stepu;
				zi += zi8stepu;
				z = (float)0x10000 / zi;	// prescale to 16.16 fixed-point

				snext = (int)(sdivz * z) + sadjust;
				if (snext > bbextents)
					snext = bbextents;
				else if (snext < 8)
					snext = 8;	// prevent round-off error on <0 steps from
								// from causing overstepping & running off
								// the edge of the texture

				tnext = (int)(tdivz * z) + tadjust;
				if (tnext > bbextentt)
					tnext = bbextentt;
				else if (tnext < 8)
					tnext = 8;	// guard against round-off error on <0 steps

				sstep = (snext - s) >> 3;
				tstep = (tnext - t) >> 3;
			}
			else
			{
				// calculate s/z, t/z, zi->fixed s and t at last pixel in
				// span (so can't step off polygon), clamp, calculate s and
				// t steps across span by division, biasing steps low so we
				// don't run off the texture
				spancountminus1 = (float)(spancount - 1);
				sdivz += d_sdivzstepu * spancountminus1;
				tdivz += d_tdivzstepu * spancountminus1;
				zi += d_zistepu * spancountminus1;
				z = (float)0x10000 / zi;	// prescale to 16.16 fixed-point
				snext = (int)(sdivz * z) + sadjust;
				if (snext > bbextents)
					snext = bbextents;
				else if (snext < 8)
					snext = 8;	// prevent round-off error on <0 steps from
								// from causing overstepping & running off
								// the edge of the texture

				tnext = (int)(tdivz * z) + tadjust;
				if (tnext > bbextentt)
					tnext = bbextentt;
				else if (tnext < 8)
					tnext = 8;	// guard against round-off error on <0 steps

				if (spancount > 1)
				{
					sstep = (snext - s) / (spancount - 1);
					tstep = (tnext - t) / (spancount - 1);
				}
			}

			do
			{
				btemp = pbase[(s >> 16) + (t >> 16) * cachewidth];
				if (btemp)
				{
					if (*pz <= (izi >> 16))
					{
						byte r1 = _GetCol16R(*pdest);
						byte g1 = _GetCol16G(*pdest);
						byte b1 = _GetCol16B(*pdest);
						byte r2 = _GetCol16R(btemp);
						byte g2 = _GetCol16G(btemp);
						byte b2 = _GetCol16B(btemp);
						byte r = (d_dsttranstab[r1] + d_srctranstab[r2]) >> 8;
						byte g = (d_dsttranstab[g1] + d_srctranstab[g2]) >> 8;
						byte b = (d_dsttranstab[b1] + d_srctranstab[b2]) >> 8;
						*pdest = _MakeCol16(r, g, b);
					}
				}

				izi += izistep;
				pdest++;
				pz++;
				s += sstep;
				t += tstep;
			} while (--spancount > 0);

			s = snext;
			t = tnext;

		} while (count > 0);

NextSpan:
		pspan++;

	} while (pspan->count != DS_SPAN_LIST_END);
}

//==========================================================================
//
//	D_DrawFuzzSpriteSpans_32
//
//==========================================================================

#define _MakeCol32(r, g, b)	((((r) << 16) & 0xff0000) | (((g) << 8) & 0xff00) | ((b) & 0xff))
#define _GetCol32R(col)		(((col) & 0xff0000) >> 16)
#define _GetCol32G(col)		(((col) & 0xff00) >> 8)
#define _GetCol32B(col)		((col) & 0xff)

extern "C" void D_DrawFuzzSpriteSpans_32(sspan_t *pspan)
{
	int			count, spancount, izi, izistep;
	vuint32		*pbase, *pdest;
	fixed_t		s, t, snext, tnext, sstep, tstep;
	float		sdivz, tdivz, zi, z, du, dv, spancountminus1;
	float		sdivz8stepu, tdivz8stepu, zi8stepu;
	vuint32		btemp;
	short		*pz;

	sstep = 0;	// keep compiler happy
	tstep = 0;	// ditto

	pbase = (vuint32*)cacheblock;

	sdivz8stepu = d_sdivzstepu * 8;
	tdivz8stepu = d_tdivzstepu * 8;
	zi8stepu = d_zistepu * 8;

	// we count on FP exceptions being turned off to avoid range problems
	izistep = (int)(d_zistepu * 0x8000 * 0x10000);

	do
	{
		pdest = (vuint32*)scrn + ylookup[pspan->v] + pspan->u;
		pz = zbuffer + ylookup[pspan->v] + pspan->u;

		count = pspan->count;

		if (count <= 0)
			goto NextSpan;

		// calculate the initial s/z, t/z, 1/z, s, and t and clamp
		du = (float)pspan->u;
		dv = (float)pspan->v;

		sdivz = d_sdivzorigin + dv*d_sdivzstepv + du*d_sdivzstepu;
		tdivz = d_tdivzorigin + dv*d_tdivzstepv + du*d_tdivzstepu;
		zi = d_ziorigin + dv*d_zistepv + du*d_zistepu;
		z = (float)0x10000 / zi;	// prescale to 16.16 fixed-point
		// we count on FP exceptions being turned off to avoid range problems
		izi = (int)(zi * 0x8000 * 0x10000);

		s = (int)(sdivz * z) + sadjust;
		if (s > bbextents)
			s = bbextents;
		else if (s < 0)
			s = 0;

		t = (int)(tdivz * z) + tadjust;
		if (t > bbextentt)
			t = bbextentt;
		else if (t < 0)
			t = 0;

		do
		{
			// calculate s and t at the far end of the span
			if (count >= 8)
				spancount = 8;
			else
				spancount = count;

			count -= spancount;

			if (count)
			{
				// calculate s/z, t/z, zi->fixed s and t at far end of span,
				// calculate s and t steps across span by shifting
				sdivz += sdivz8stepu;
				tdivz += tdivz8stepu;
				zi += zi8stepu;
				z = (float)0x10000 / zi;	// prescale to 16.16 fixed-point

				snext = (int)(sdivz * z) + sadjust;
				if (snext > bbextents)
					snext = bbextents;
				else if (snext < 8)
					snext = 8;	// prevent round-off error on <0 steps from
								// from causing overstepping & running off
								// the edge of the texture

				tnext = (int)(tdivz * z) + tadjust;
				if (tnext > bbextentt)
					tnext = bbextentt;
				else if (tnext < 8)
					tnext = 8;	// guard against round-off error on <0 steps

				sstep = (snext - s) >> 3;
				tstep = (tnext - t) >> 3;
			}
			else
			{
				// calculate s/z, t/z, zi->fixed s and t at last pixel in
				// span (so can't step off polygon), clamp, calculate s and
				// t steps across span by division, biasing steps low so we
				// don't run off the texture
				spancountminus1 = (float)(spancount - 1);
				sdivz += d_sdivzstepu * spancountminus1;
				tdivz += d_tdivzstepu * spancountminus1;
				zi += d_zistepu * spancountminus1;
				z = (float)0x10000 / zi;	// prescale to 16.16 fixed-point
				snext = (int)(sdivz * z) + sadjust;
				if (snext > bbextents)
					snext = bbextents;
				else if (snext < 8)
					snext = 8;	// prevent round-off error on <0 steps from
								// from causing overstepping & running off
								// the edge of the texture

				tnext = (int)(tdivz * z) + tadjust;
				if (tnext > bbextentt)
					tnext = bbextentt;
				else if (tnext < 8)
					tnext = 8;	// guard against round-off error on <0 steps

				if (spancount > 1)
				{
					sstep = (snext - s) / (spancount - 1);
					tstep = (tnext - t) / (spancount - 1);
				}
			}

			do
			{
				btemp = pbase[(s >> 16) + (t >> 16) * cachewidth];
				if (btemp)
				{
					if (*pz <= (izi >> 16))
					{
						byte r1 = _GetCol32R(*pdest);
						byte g1 = _GetCol32G(*pdest);
						byte b1 = _GetCol32B(*pdest);
						byte r2 = _GetCol32R(btemp);
						byte g2 = _GetCol32G(btemp);
						byte b2 = _GetCol32B(btemp);
						byte r = (d_dsttranstab[r1] + d_srctranstab[r2]) >> 8;
						byte g = (d_dsttranstab[g1] + d_srctranstab[g2]) >> 8;
						byte b = (d_dsttranstab[b1] + d_srctranstab[b2]) >> 8;
						*pdest = _MakeCol32(r, g, b);
					}
				}

				izi += izistep;
				pdest++;
				pz++;
				s += sstep;
				t += tstep;
			} while (--spancount > 0);

			s = snext;
			t = tnext;

		} while (count > 0);

NextSpan:
		pspan++;

	} while (pspan->count != DS_SPAN_LIST_END);
}

//==========================================================================
//
//	D_DrawAdditiveSpriteSpans_15
//
//==========================================================================

extern "C" void D_DrawAdditiveSpriteSpans_15(sspan_t *pspan)
{
	int			count, spancount, izi, izistep;
	word		*pbase, *pdest;
	fixed_t		s, t, snext, tnext, sstep, tstep;
	float		sdivz, tdivz, zi, z, du, dv, spancountminus1;
	float		sdivz8stepu, tdivz8stepu, zi8stepu;
	word		btemp;
	short		*pz;

	sstep = 0;	// keep compiler happy
	tstep = 0;	// ditto

	pbase = (word*)cacheblock;

	sdivz8stepu = d_sdivzstepu * 8;
	tdivz8stepu = d_tdivzstepu * 8;
	zi8stepu = d_zistepu * 8;

	// we count on FP exceptions being turned off to avoid range problems
	izistep = (int)(d_zistepu * 0x8000 * 0x10000);

	do
	{
		pdest = (word*)scrn + ylookup[pspan->v] + pspan->u;
		pz = zbuffer + ylookup[pspan->v] + pspan->u;

		count = pspan->count;

		if (count <= 0)
			goto NextSpan;

		// calculate the initial s/z, t/z, 1/z, s, and t and clamp
		du = (float)pspan->u;
		dv = (float)pspan->v;

		sdivz = d_sdivzorigin + dv*d_sdivzstepv + du*d_sdivzstepu;
		tdivz = d_tdivzorigin + dv*d_tdivzstepv + du*d_tdivzstepu;
		zi = d_ziorigin + dv*d_zistepv + du*d_zistepu;
		z = (float)0x10000 / zi;	// prescale to 16.16 fixed-point
		// we count on FP exceptions being turned off to avoid range problems
		izi = (int)(zi * 0x8000 * 0x10000);

		s = (int)(sdivz * z) + sadjust;
		if (s > bbextents)
			s = bbextents;
		else if (s < 0)
			s = 0;

		t = (int)(tdivz * z) + tadjust;
		if (t > bbextentt)
			t = bbextentt;
		else if (t < 0)
			t = 0;

		do
		{
			// calculate s and t at the far end of the span
			if (count >= 8)
				spancount = 8;
			else
				spancount = count;

			count -= spancount;

			if (count)
			{
				// calculate s/z, t/z, zi->fixed s and t at far end of span,
				// calculate s and t steps across span by shifting
				sdivz += sdivz8stepu;
				tdivz += tdivz8stepu;
				zi += zi8stepu;
				z = (float)0x10000 / zi;	// prescale to 16.16 fixed-point

				snext = (int)(sdivz * z) + sadjust;
				if (snext > bbextents)
					snext = bbextents;
				else if (snext < 8)
					snext = 8;	// prevent round-off error on <0 steps from
								// from causing overstepping & running off
								// the edge of the texture

				tnext = (int)(tdivz * z) + tadjust;
				if (tnext > bbextentt)
					tnext = bbextentt;
				else if (tnext < 8)
					tnext = 8;	// guard against round-off error on <0 steps

				sstep = (snext - s) >> 3;
				tstep = (tnext - t) >> 3;
			}
			else
			{
				// calculate s/z, t/z, zi->fixed s and t at last pixel in
				// span (so can't step off polygon), clamp, calculate s and
				// t steps across span by division, biasing steps low so we
				// don't run off the texture
				spancountminus1 = (float)(spancount - 1);
				sdivz += d_sdivzstepu * spancountminus1;
				tdivz += d_tdivzstepu * spancountminus1;
				zi += d_zistepu * spancountminus1;
				z = (float)0x10000 / zi;	// prescale to 16.16 fixed-point
				snext = (int)(sdivz * z) + sadjust;
				if (snext > bbextents)
					snext = bbextents;
				else if (snext < 8)
					snext = 8;	// prevent round-off error on <0 steps from
								// from causing overstepping & running off
								// the edge of the texture

				tnext = (int)(tdivz * z) + tadjust;
				if (tnext > bbextentt)
					tnext = bbextentt;
				else if (tnext < 8)
					tnext = 8;	// guard against round-off error on <0 steps

				if (spancount > 1)
				{
					sstep = (snext - s) / (spancount - 1);
					tstep = (tnext - t) / (spancount - 1);
				}
			}

			do
			{
				btemp = pbase[(s >> 16) + (t >> 16) * cachewidth];
				if (btemp)
				{
					if (*pz <= (izi >> 16))
					{
						byte r1 = _GetCol15R(*pdest);
						byte g1 = _GetCol15G(*pdest);
						byte b1 = _GetCol15B(*pdest);
						byte r2 = _GetCol15R(btemp);
						byte g2 = _GetCol15G(btemp);
						byte b2 = _GetCol15B(btemp);
						int r = r1 + (d_srctranstab[r2] >> 8);
						if (r > 255)
						{
							r = 255;
						}
						int g = g1 + (d_srctranstab[g2] >> 8);
						if (g > 255)
						{
							g = 255;
						}
						int b = b1 + (d_srctranstab[b2] >> 8);
						if (b > 255)
						{
							b = 255;
						}
						*pdest = _MakeCol15(r, g, b);
					}
				}

				izi += izistep;
				pdest++;
				pz++;
				s += sstep;
				t += tstep;
			} while (--spancount > 0);

			s = snext;
			t = tnext;

		} while (count > 0);

NextSpan:
		pspan++;

	} while (pspan->count != DS_SPAN_LIST_END);
}

//==========================================================================
//
//	D_DrawAdditiveSpriteSpans_16
//
//==========================================================================

extern "C" void D_DrawAdditiveSpriteSpans_16(sspan_t *pspan)
{
	int			count, spancount, izi, izistep;
	word		*pbase, *pdest;
	fixed_t		s, t, snext, tnext, sstep, tstep;
	float		sdivz, tdivz, zi, z, du, dv, spancountminus1;
	float		sdivz8stepu, tdivz8stepu, zi8stepu;
	word		btemp;
	short		*pz;

	sstep = 0;	// keep compiler happy
	tstep = 0;	// ditto

	pbase = (word*)cacheblock;

	sdivz8stepu = d_sdivzstepu * 8;
	tdivz8stepu = d_tdivzstepu * 8;
	zi8stepu = d_zistepu * 8;

	// we count on FP exceptions being turned off to avoid range problems
	izistep = (int)(d_zistepu * 0x8000 * 0x10000);

	do
	{
		pdest = (word*)scrn + ylookup[pspan->v] + pspan->u;
		pz = zbuffer + ylookup[pspan->v] + pspan->u;

		count = pspan->count;

		if (count <= 0)
			goto NextSpan;

		// calculate the initial s/z, t/z, 1/z, s, and t and clamp
		du = (float)pspan->u;
		dv = (float)pspan->v;

		sdivz = d_sdivzorigin + dv*d_sdivzstepv + du*d_sdivzstepu;
		tdivz = d_tdivzorigin + dv*d_tdivzstepv + du*d_tdivzstepu;
		zi = d_ziorigin + dv*d_zistepv + du*d_zistepu;
		z = (float)0x10000 / zi;	// prescale to 16.16 fixed-point
		// we count on FP exceptions being turned off to avoid range problems
		izi = (int)(zi * 0x8000 * 0x10000);

		s = (int)(sdivz * z) + sadjust;
		if (s > bbextents)
			s = bbextents;
		else if (s < 0)
			s = 0;

		t = (int)(tdivz * z) + tadjust;
		if (t > bbextentt)
			t = bbextentt;
		else if (t < 0)
			t = 0;

		do
		{
			// calculate s and t at the far end of the span
			if (count >= 8)
				spancount = 8;
			else
				spancount = count;

			count -= spancount;

			if (count)
			{
				// calculate s/z, t/z, zi->fixed s and t at far end of span,
				// calculate s and t steps across span by shifting
				sdivz += sdivz8stepu;
				tdivz += tdivz8stepu;
				zi += zi8stepu;
				z = (float)0x10000 / zi;	// prescale to 16.16 fixed-point

				snext = (int)(sdivz * z) + sadjust;
				if (snext > bbextents)
					snext = bbextents;
				else if (snext < 8)
					snext = 8;	// prevent round-off error on <0 steps from
								// from causing overstepping & running off
								// the edge of the texture

				tnext = (int)(tdivz * z) + tadjust;
				if (tnext > bbextentt)
					tnext = bbextentt;
				else if (tnext < 8)
					tnext = 8;	// guard against round-off error on <0 steps

				sstep = (snext - s) >> 3;
				tstep = (tnext - t) >> 3;
			}
			else
			{
				// calculate s/z, t/z, zi->fixed s and t at last pixel in
				// span (so can't step off polygon), clamp, calculate s and
				// t steps across span by division, biasing steps low so we
				// don't run off the texture
				spancountminus1 = (float)(spancount - 1);
				sdivz += d_sdivzstepu * spancountminus1;
				tdivz += d_tdivzstepu * spancountminus1;
				zi += d_zistepu * spancountminus1;
				z = (float)0x10000 / zi;	// prescale to 16.16 fixed-point
				snext = (int)(sdivz * z) + sadjust;
				if (snext > bbextents)
					snext = bbextents;
				else if (snext < 8)
					snext = 8;	// prevent round-off error on <0 steps from
								// from causing overstepping & running off
								// the edge of the texture

				tnext = (int)(tdivz * z) + tadjust;
				if (tnext > bbextentt)
					tnext = bbextentt;
				else if (tnext < 8)
					tnext = 8;	// guard against round-off error on <0 steps

				if (spancount > 1)
				{
					sstep = (snext - s) / (spancount - 1);
					tstep = (tnext - t) / (spancount - 1);
				}
			}

			do
			{
				btemp = pbase[(s >> 16) + (t >> 16) * cachewidth];
				if (btemp)
				{
					if (*pz <= (izi >> 16))
					{
						byte r1 = _GetCol16R(*pdest);
						byte g1 = _GetCol16G(*pdest);
						byte b1 = _GetCol16B(*pdest);
						byte r2 = _GetCol16R(btemp);
						byte g2 = _GetCol16G(btemp);
						byte b2 = _GetCol16B(btemp);
						int r = r1 + (d_srctranstab[r2] >> 8);
						if (r > 255)
						{
							r = 255;
						}
						int g = g1 + (d_srctranstab[g2] >> 8);
						if (g > 255)
						{
							g = 255;
						}
						int b = b1 + (d_srctranstab[b2] >> 8);
						if (b > 255)
						{
							b = 255;
						}
						*pdest = _MakeCol16(r, g, b);
					}
				}

				izi += izistep;
				pdest++;
				pz++;
				s += sstep;
				t += tstep;
			} while (--spancount > 0);

			s = snext;
			t = tnext;

		} while (count > 0);

NextSpan:
		pspan++;

	} while (pspan->count != DS_SPAN_LIST_END);
}

//==========================================================================
//
//	D_DrawAdditiveSpriteSpans_32
//
//==========================================================================

extern "C" void D_DrawAdditiveSpriteSpans_32(sspan_t *pspan)
{
	int			count, spancount, izi, izistep;
	vuint32		*pbase, *pdest;
	fixed_t		s, t, snext, tnext, sstep, tstep;
	float		sdivz, tdivz, zi, z, du, dv, spancountminus1;
	float		sdivz8stepu, tdivz8stepu, zi8stepu;
	vuint32		btemp;
	short		*pz;

	sstep = 0;	// keep compiler happy
	tstep = 0;	// ditto

	pbase = (vuint32*)cacheblock;

	sdivz8stepu = d_sdivzstepu * 8;
	tdivz8stepu = d_tdivzstepu * 8;
	zi8stepu = d_zistepu * 8;

	// we count on FP exceptions being turned off to avoid range problems
	izistep = (int)(d_zistepu * 0x8000 * 0x10000);

	do
	{
		pdest = (vuint32*)scrn + ylookup[pspan->v] + pspan->u;
		pz = zbuffer + ylookup[pspan->v] + pspan->u;

		count = pspan->count;

		if (count <= 0)
			goto NextSpan;

		// calculate the initial s/z, t/z, 1/z, s, and t and clamp
		du = (float)pspan->u;
		dv = (float)pspan->v;

		sdivz = d_sdivzorigin + dv*d_sdivzstepv + du*d_sdivzstepu;
		tdivz = d_tdivzorigin + dv*d_tdivzstepv + du*d_tdivzstepu;
		zi = d_ziorigin + dv*d_zistepv + du*d_zistepu;
		z = (float)0x10000 / zi;	// prescale to 16.16 fixed-point
		// we count on FP exceptions being turned off to avoid range problems
		izi = (int)(zi * 0x8000 * 0x10000);

		s = (int)(sdivz * z) + sadjust;
		if (s > bbextents)
			s = bbextents;
		else if (s < 0)
			s = 0;

		t = (int)(tdivz * z) + tadjust;
		if (t > bbextentt)
			t = bbextentt;
		else if (t < 0)
			t = 0;

		do
		{
			// calculate s and t at the far end of the span
			if (count >= 8)
				spancount = 8;
			else
				spancount = count;

			count -= spancount;

			if (count)
			{
				// calculate s/z, t/z, zi->fixed s and t at far end of span,
				// calculate s and t steps across span by shifting
				sdivz += sdivz8stepu;
				tdivz += tdivz8stepu;
				zi += zi8stepu;
				z = (float)0x10000 / zi;	// prescale to 16.16 fixed-point

				snext = (int)(sdivz * z) + sadjust;
				if (snext > bbextents)
					snext = bbextents;
				else if (snext < 8)
					snext = 8;	// prevent round-off error on <0 steps from
								// from causing overstepping & running off
								// the edge of the texture

				tnext = (int)(tdivz * z) + tadjust;
				if (tnext > bbextentt)
					tnext = bbextentt;
				else if (tnext < 8)
					tnext = 8;	// guard against round-off error on <0 steps

				sstep = (snext - s) >> 3;
				tstep = (tnext - t) >> 3;
			}
			else
			{
				// calculate s/z, t/z, zi->fixed s and t at last pixel in
				// span (so can't step off polygon), clamp, calculate s and
				// t steps across span by division, biasing steps low so we
				// don't run off the texture
				spancountminus1 = (float)(spancount - 1);
				sdivz += d_sdivzstepu * spancountminus1;
				tdivz += d_tdivzstepu * spancountminus1;
				zi += d_zistepu * spancountminus1;
				z = (float)0x10000 / zi;	// prescale to 16.16 fixed-point
				snext = (int)(sdivz * z) + sadjust;
				if (snext > bbextents)
					snext = bbextents;
				else if (snext < 8)
					snext = 8;	// prevent round-off error on <0 steps from
								// from causing overstepping & running off
								// the edge of the texture

				tnext = (int)(tdivz * z) + tadjust;
				if (tnext > bbextentt)
					tnext = bbextentt;
				else if (tnext < 8)
					tnext = 8;	// guard against round-off error on <0 steps

				if (spancount > 1)
				{
					sstep = (snext - s) / (spancount - 1);
					tstep = (tnext - t) / (spancount - 1);
				}
			}

			do
			{
				btemp = pbase[(s >> 16) + (t >> 16) * cachewidth];
				if (btemp)
				{
					if (*pz <= (izi >> 16))
					{
						byte r1 = _GetCol32R(*pdest);
						byte g1 = _GetCol32G(*pdest);
						byte b1 = _GetCol32B(*pdest);
						byte r2 = _GetCol32R(btemp);
						byte g2 = _GetCol32G(btemp);
						byte b2 = _GetCol32B(btemp);
						int r = r1 + (d_srctranstab[r2] >> 8);
						if (r > 255)
						{
							r = 255;
						}
						int g = g1 + (d_srctranstab[g2] >> 8);
						if (g > 255)
						{
							g = 255;
						}
						int b = b1 + (d_srctranstab[b2] >> 8);
						if (b > 255)
						{
							b = 255;
						}
						*pdest = _MakeCol32(r, g, b);
					}
				}

				izi += izistep;
				pdest++;
				pz++;
				s += sstep;
				t += tstep;
			} while (--spancount > 0);

			s = snext;
			t = tnext;

		} while (count > 0);

NextSpan:
		pspan++;

	} while (pspan->count != DS_SPAN_LIST_END);
}

//==========================================================================
//
//	VSoftwareDrawer::SpriteClipEdge
//
//==========================================================================

void VSoftwareDrawer::SpriteClipEdge(const TVec &v0, const TVec &v1,
	TClipPlane *clip, int clipflags)
{
	if (clip)
	{
		do
		{
			if (!(clipflags & clip->clipflag))
			{
				continue;
			}

			float d0 = DotProduct(v0, clip->normal) - clip->dist;
			float d1 = DotProduct(v1, clip->normal) - clip->dist;
			if (d0 >= 0)
			{
				// point 0 is unclipped
				if (d1 >= 0)
				{
					// both points are unclipped
					continue;
				}

				// only point 1 is clipped

				TVec &clipvert = clip->enter;
				clip->entered = true;

				float f = d0 / (d0 - d1);
				clipvert.x = v0.x + f * (v1.x - v0.x);
				clipvert.y = v0.y + f * (v1.y - v0.y);
				clipvert.z = v0.z + f * (v1.z - v0.z);

				SpriteClipEdge(v0, clipvert, clip->next, clipflags);
				if (clip->exited)
				{
					clip->entered = false;
					clip->exited = false;
					SpriteClipEdge(clipvert, clip->exit, view_clipplanes, clipflags ^ clip->clipflag);
				}
				return;
			}
			else
			{
				// point 0 is clipped

				if (d1 < 0)
				{
					// both points are clipped
					return;
				}

				// only point 0 is clipped

				TVec &clipvert = clip->exit;
				clip->exited = true;

				float f = d0 / (d0 - d1);
				clipvert.x = v0.x + f * (v1.x - v0.x);
				clipvert.y = v0.y + f * (v1.y - v0.y);
				clipvert.z = v0.z + f * (v1.z - v0.z);

				if (clip->entered)
				{
					clip->entered = false;
					clip->exited = false;
					SpriteClipEdge(clip->enter, clipvert, view_clipplanes, clipflags ^ clip->clipflag);
				}
				SpriteClipEdge(clipvert, v1, clip->next, clipflags);
				return;
			}
		} while ((clip = clip->next) != NULL);
	}

	//	add the vertex
	if (r_emited >= max_emited)
		return;

	TVec		tr;

	TransformVector(v1 - vieworg, tr);
	if (tr.z < 0.01)
		tr.z = 0.01;

	float z1 = 1.0f / tr.z;
	if (z1 > r_nearzi)
	{
		r_nearzi = z1;
	}
	tr.x = tr.x * z1 * xprojection + centrexfrac;
	tr.y = tr.y * z1 * yprojection + centreyfrac;

	if (tr.x < -0.5)
		tr.x = -0.5;
	if (tr.x > viewwidth - 0.5)
		tr.x = viewwidth - 0.5;
	if (tr.y < -0.5)
		tr.y = -0.5;
	if (tr.y > viewheight - 0.5)
		tr.y = viewheight - 0.5;

	p_vert[r_emited] = tr;
	r_emited++;
}

//==========================================================================
//
//	VSoftwareDrawer::SpriteScanLeftEdge
//
//==========================================================================

void VSoftwareDrawer::SpriteScanLeftEdge(TVec *vb, int count)
{
	int				i, v, itop, ibottom;
	TVec			*pvert, *pnext;
	sspan_t			*pspan;
	float			du, dv, vtop, vbottom, slope;
	fixed_t			u, u_step;

	pspan = sprite_spans;
	i = minindex;

	vbottom = ceil(vb[i].y);

	do
	{
		pvert = &vb[i];
		if (i + 1 == count)
		{
			pnext = &vb[0];
		}
		else
		{
			pnext = &vb[i + 1];
		}

		vtop = ceil(pnext->y);

		if (vtop > vbottom)
		{
			du = pnext->x - pvert->x;
			dv = pnext->y - pvert->y;
			slope = du / dv;
			u_step = (int)(slope * 0x10000);
			// adjust u to ceil the integer portion
			u = (int)((pvert->x + (vbottom - pvert->y) * slope) * 0x10000) + 0xffff;
			itop = (int)vtop;
			ibottom = (int)vbottom;

			for (v = ibottom; v < itop; v++)
			{
				pspan->u = u >> 16;
				pspan->v = v;
				u += u_step;
				pspan++;
			}
		}

		vbottom = vtop;

		i++;
		if (i == count)
		{
			i = 0;
		}
	} while (i != maxindex);
}

//==========================================================================
//
//	VSoftwareDrawer::SpriteScanRightEdge
//
//==========================================================================

void VSoftwareDrawer::SpriteScanRightEdge(TVec *vb, int count)
{
	int				i, v, itop, ibottom;
	TVec			*pvert, *pnext;
	sspan_t			*pspan;
	float			du, dv, vtop, vbottom, slope;
	fixed_t			u, u_step;

	pspan = sprite_spans;
	i = minindex;
	vbottom = ceil(vb[i].y);

	do
	{
		pvert = &vb[i];
		if (i - 1 < 0)
		{
			pnext = &vb[count - 1];
		}
		else
		{
			pnext = &vb[i - 1];
		}

		vtop = ceil(pnext->y);

		if (vtop > vbottom)
		{
			du = pnext->x - pvert->x;
			dv = pnext->y - pvert->y;
			slope = du / dv;
			u_step = (int)(slope * 0x10000);
			// adjust u to ceil the integer portion
			u = (int)((pvert->x + (vbottom - pvert->y) * slope) * 0x10000) + 0xffff;
			itop = (int)vtop;
			ibottom = (int)vbottom;

			for (v = ibottom; v < itop; v++)
			{
				pspan->count = (u >> 16) - pspan->u;
				u += u_step;
				pspan++;
			}
		}
		vbottom = vtop;
		i--;
		if (i < 0)
		{
			i = count - 1;
		}
	} while (i != maxindex);

	pspan->count = DS_SPAN_LIST_END;	// mark the end of the span list
}

//==========================================================================
//
//	VSoftwareDrawer::SpriteCaclulateGradients
//
//==========================================================================

void VSoftwareDrawer::SpriteCaclulateGradients(VTexture* Tex, const TVec& normal,
	float dist, const TVec& saxis, const TVec& taxis, const TVec& texorg)
{
	TVec		p_normal, p_saxis, p_taxis;
	float		distinv;

	TransformVector(normal, p_normal);
	TransformVector(saxis, p_saxis);
	TransformVector(taxis, p_taxis);

	distinv = 1.0 / (dist - DotProduct(vieworg, normal));

	d_sdivzstepu = p_saxis.x / xprojection;
	d_tdivzstepu = p_taxis.x / xprojection;

	d_sdivzstepv = p_saxis.y / yprojection;
	d_tdivzstepv = p_taxis.y / yprojection;

	d_zistepu = p_normal.x * distinv / xprojection;
	d_zistepv = p_normal.y * distinv / yprojection;

	d_sdivzorigin = p_saxis.z - centrexfrac * d_sdivzstepu -
			centreyfrac * d_sdivzstepv;
	d_tdivzorigin = p_taxis.z - centrexfrac * d_tdivzstepu -
			centreyfrac * d_tdivzstepv;
	d_ziorigin = p_normal.z * distinv - centrexfrac * d_zistepu -
			centreyfrac * d_zistepv;

	sadjust = (fixed_t)(DotProduct(vieworg - texorg, saxis) * 0x10000 +
		0.5) - spr_texturemins[0];
	tadjust = (fixed_t)(DotProduct(vieworg - texorg, taxis) * 0x10000 +
		0.5) - spr_texturemins[1];

	// -1 (-epsilon) so we never wander off the edge of the texture
	bbextents = (Tex->GetWidth() << 16) - 1;
	bbextentt = (Tex->GetHeight() << 16) - 1;
}

//==========================================================================
//
//	VSoftwareDrawer::MaskedSurfCaclulateGradients
//
//==========================================================================

void VSoftwareDrawer::MaskedSurfCaclulateGradients(surface_t *surf)
{
	TVec		p_normal, p_saxis, p_taxis;
	float		distinv, mipscale, t;
	int			miplevel;
	surfcache_t *cache;
	texinfo_t	*tex;

	tex = surf->texinfo;
	miplevel = D_MipLevelForScale(r_nearzi * scale_for_mip);
	mipscale = 1.0 / (float)(1 << miplevel);

	TransformVector(surf->plane->normal, p_normal);
	TransformVector(tex->saxis, p_saxis);
	TransformVector(tex->taxis, p_taxis);

	distinv = 1.0 / (surf->plane->dist - DotProduct(vieworg, surf->plane->normal));

	d_zistepu = p_normal.x * distinv / xprojection;
	d_zistepv = p_normal.y * distinv / yprojection;
	d_ziorigin = p_normal.z * distinv - centrexfrac * d_zistepu -
			centreyfrac * d_zistepv;

	t = mipscale / xprojection;
	d_sdivzstepu = p_saxis.x * t;
	d_tdivzstepu = p_taxis.x * t;

	t = mipscale / yprojection;
	d_sdivzstepv = p_saxis.y * t;
	d_tdivzstepv = p_taxis.y * t;

	d_sdivzorigin = p_saxis.z * mipscale -
		centrexfrac * d_sdivzstepu - centreyfrac * d_sdivzstepv;
	d_tdivzorigin = p_taxis.z * mipscale -
		centrexfrac * d_tdivzstepu - centreyfrac * d_tdivzstepv;

	t = 0x10000 * mipscale;
	sadjust = (fixed_t)(DotProduct(vieworg, tex->saxis) * t + 0.5) -
		((surf->texturemins[0] << 16) >> miplevel)
		+ (fixed_t)(tex->soffs * t);
	tadjust = (fixed_t)(DotProduct(vieworg, tex->taxis) * t + 0.5) -
		((surf->texturemins[1] << 16) >> miplevel)
		+ (fixed_t)(tex->toffs * t);

	// -1 (-epsilon) so we never wander off the edge of the texture
	bbextents = ((surf->extents[0] << 16) >> miplevel) - 1;
	bbextentt = ((surf->extents[1] << 16) >> miplevel) - 1;

	cache = CacheSurface(surf, miplevel);
	cachewidth = cache->width;
	cacheblock = cache->data;
}

//==========================================================================
//
//	VSoftwareDrawer::SpriteDrawPolygon
//
//==========================================================================

void VSoftwareDrawer::SpriteDrawPolygon(TVec *cv, int count, surface_t *surf,
	VTexture* Tex, VTextureTranslation* Translation, int CMap, float Alpha,
	bool Additive, vuint32 light, vuint32 Fade, const TVec& normal,
	float dist, const TVec& saxis, const TVec& taxis, const TVec& texorg)
{
	int			i;
	float		ymin, ymax;
	sspan_t		spans[MAXSCREENHEIGHT + 1];

	for (i = 0; i < 4; i++)
	{
		view_clipplanes[i].entered = false;
		view_clipplanes[i].exited = false;
	}

#ifdef __GNUC__
	max_emited = count + NUM_EXTRA_VERTS;
	TVec verts[max_emited];
#else
	max_emited = MAX_STACK_VERTS;
	TVec verts[MAX_STACK_VERTS];
#endif
	r_emited = 0;
	r_nearzi = 0;
	p_vert = verts;

	for (i = 0; i < count; i++)
	{
		SpriteClipEdge(cv[i ? i - 1 : count - 1], cv[i], view_clipplanes, 15);
	}

	if (r_emited < 3)
	{
		return;
	}

	sprite_spans = spans;

	//	Find the top and bottom vertices, and make sure there's at least one
	// scan to draw
	ymin = 999999.9;
	ymax = -999999.9;

	for (i = 0; i < r_emited; i++)
	{
		if (verts[i].y < ymin)
		{
			ymin = verts[i].y;
			minindex = i;
		}

		if (verts[i].y > ymax)
		{
			ymax = verts[i].y;
			maxindex = i;
		}
	}

	ymin = ceil(ymin);
	ymax = ceil(ymax);

	if (ymin >= ymax)
	{
		// doesn't cross any scans at all
		return;
	}

	if (Additive)
	{
		int trindex = (int)(Alpha * 10 - 0.5);
		if (trindex < 0)
		{
			trindex = 0;
		}
		else if (trindex > 9)
		{
			trindex = 9;
		}
		d_transluc = AdditiveTransTables[trindex];
		spritespanfunc = D_DrawAdditiveSpriteSpans;

		trindex = (int)(Alpha * 31);
		d_dsttranstab = scaletable[31 - trindex];
		d_srctranstab = scaletable[trindex];
	}
	else if (Alpha >= 1.0)
	{
		spritespanfunc = D_DrawSpriteSpans;
	}
	else
	{
		int trindex = (int)((1 - Alpha) * 10 - 0.5);
		if (trindex < 0)
			trindex = 0;
		else if (trindex > 8)
			trindex = 8;
		if (trindex < 5)
		{
			d_transluc = tinttables[trindex];
			spritespanfunc = D_DrawFuzzSpriteSpans;
		}
		else
		{
			d_transluc = tinttables[8 - trindex];
			spritespanfunc = D_DrawAltFuzzSpriteSpans;
		}

		trindex = (int)(Alpha * 31);
		d_dsttranstab = scaletable[31 - trindex];
		d_srctranstab = scaletable[trindex];
	}

	if (surf)
	{
		MaskedSurfCaclulateGradients(surf);
	}
	else
	{
		SpriteCaclulateGradients(Tex, normal, dist, saxis, taxis, texorg);
		SetFade(Fade);
		SetSpriteLump(Tex, light, Translation, CMap);
	}
	SpriteScanLeftEdge(verts, r_emited);
	SpriteScanRightEdge(verts, r_emited);
	spritespanfunc(sprite_spans);
}

//==========================================================================
//
//	VSoftwareDrawer::DrawMaskedPolygon
//
//==========================================================================

void VSoftwareDrawer::DrawMaskedPolygon(surface_t* surf, float Alpha,
	bool Additive)
{
	guard(VSoftwareDrawer::DrawMaskedPolygon);
	SpriteDrawPolygon(surf->verts, surf->count, surf, NULL, 0, 0, Alpha,
		Additive, 0, 0, TVec(), 0, TVec(), TVec(), TVec());
	unguard;
}

//==========================================================================
//
//	VSoftwareDrawer::DrawSpritePolygon
//
//==========================================================================

void VSoftwareDrawer::DrawSpritePolygon(TVec* cv, VTexture* Tex, float Alpha,
	bool Additive, VTextureTranslation* Translation, int CMap, vuint32 light,
	vuint32 Fade, const TVec& normal, float dist, const TVec& saxis,
	const TVec& taxis, const TVec& texorg)
{
	guard(VSoftwareDrawer::DrawSpritePolygon);
	SpriteDrawPolygon(cv, 4, NULL, Tex, Translation, CMap, Alpha, Additive,
		light, Fade, normal, dist, saxis, taxis, texorg);
	unguard;
}
