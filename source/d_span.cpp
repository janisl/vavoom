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
//**	$Log$
//**	Revision 1.2  2001/07/27 14:27:54  dj_jl
//**	Update with Id-s and Log-s, some fixes
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

// PRIVATE DATA DEFINITIONS ------------------------------------------------

// CODE --------------------------------------------------------------------

#ifndef USEASM

//==========================================================================
//
//	D_DrawSpans8_8
//
//==========================================================================

extern "C" void D_DrawSpans8_8(espan_t *pspan)
{
	int				count, spancount;
	byte			*pbase, *pdest;
	fixed_t			s, t, snext, tnext, sstep, tstep;
	float			sdivz, tdivz, zi, z, du, dv, spancountminus1;
	float			sdivz8stepu, tdivz8stepu, zi8stepu;

	sstep = 0;	// keep compiler happy
	tstep = 0;	// ditto

	pbase = (byte *)cacheblock;

	sdivz8stepu = d_sdivzstepu * 8;
	tdivz8stepu = d_tdivzstepu * 8;
	zi8stepu = d_zistepu * 8;

	do
	{
	    pdest = (byte *)scrn + ylookup[pspan->v] + pspan->u;

		count = pspan->count;

		// calculate the initial s/z, t/z, 1/z, s, and t and clamp
		du = (float)pspan->u;
		dv = (float)pspan->v;

		sdivz = d_sdivzorigin + dv*d_sdivzstepv + du*d_sdivzstepu;
		tdivz = d_tdivzorigin + dv*d_tdivzstepv + du*d_tdivzstepu;
		zi = d_ziorigin + dv*d_zistepv + du*d_zistepu;
		z = (float)0x10000 / zi;	// prescale to 16.16 fixed-point

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
					snext = 8;	//	Prevent round-off error on <0 steps from
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
				//	Calculate s/z, t/z, zi->fixed s and t at last pixel in
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
					snext = 8;	//	Prevent round-off error on <0 steps from
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
				*pdest++ = pbase[(s >> 16) + (t >> 16) * cachewidth];
				s += sstep;
				t += tstep;
			} while (--spancount > 0);

			s = snext;
			t = tnext;

		} while (count > 0);

	} while ((pspan = pspan->pnext) != NULL);
}

//==========================================================================
//
//	D_DrawSpans8_16
//
//==========================================================================

extern "C" void D_DrawSpans8_16(espan_t *pspan)
{
	int				count, spancount;
	word			*pbase, *pdest;
	fixed_t			s, t, snext, tnext, sstep, tstep;
	float			sdivz, tdivz, zi, z, du, dv, spancountminus1;
	float			sdivz8stepu, tdivz8stepu, zi8stepu;

	sstep = 0;	// keep compiler happy
	tstep = 0;	// ditto

	pbase = (word*)cacheblock;

	sdivz8stepu = d_sdivzstepu * 8;
	tdivz8stepu = d_tdivzstepu * 8;
	zi8stepu = d_zistepu * 8;

	do
	{
	    pdest = (word*)scrn + ylookup[pspan->v] + pspan->u;

		count = pspan->count;

		// calculate the initial s/z, t/z, 1/z, s, and t and clamp
		du = (float)pspan->u;
		dv = (float)pspan->v;

		sdivz = d_sdivzorigin + dv*d_sdivzstepv + du*d_sdivzstepu;
		tdivz = d_tdivzorigin + dv*d_tdivzstepv + du*d_tdivzstepu;
		zi = d_ziorigin + dv*d_zistepv + du*d_zistepu;
		z = (float)0x10000 / zi;	// prescale to 16.16 fixed-point

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
					snext = 8;	//	Prevent round-off error on <0 steps from
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
				//	Calculate s/z, t/z, zi->fixed s and t at last pixel in
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
					snext = 8;	//	Prevent round-off error on <0 steps from
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
				*pdest++ = pbase[(s >> 16) + (t >> 16) * cachewidth];
				s += sstep;
				t += tstep;
			} while (--spancount > 0);

			s = snext;
			t = tnext;

		} while (count > 0);

	} while ((pspan = pspan->pnext) != NULL);
}

//==========================================================================
//
//	D_DrawSpans8_32
//
//==========================================================================

extern "C" void D_DrawSpans8_32(espan_t *pspan)
{
	int				count, spancount;
	dword			*pbase, *pdest;
	fixed_t			s, t, snext, tnext, sstep, tstep;
	float			sdivz, tdivz, zi, z, du, dv, spancountminus1;
	float			sdivz8stepu, tdivz8stepu, zi8stepu;

	sstep = 0;	// keep compiler happy
	tstep = 0;	// ditto

	pbase = (dword*)cacheblock;

	sdivz8stepu = d_sdivzstepu * 8;
	tdivz8stepu = d_tdivzstepu * 8;
	zi8stepu = d_zistepu * 8;

	do
	{
	    pdest = (dword*)scrn + ylookup[pspan->v] + pspan->u;

		count = pspan->count;

		// calculate the initial s/z, t/z, 1/z, s, and t and clamp
		du = (float)pspan->u;
		dv = (float)pspan->v;

		sdivz = d_sdivzorigin + dv*d_sdivzstepv + du*d_sdivzstepu;
		tdivz = d_tdivzorigin + dv*d_tdivzstepv + du*d_tdivzstepu;
		zi = d_ziorigin + dv*d_zistepv + du*d_zistepu;
		z = (float)0x10000 / zi;	// prescale to 16.16 fixed-point

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
					snext = 8;	//	Prevent round-off error on <0 steps from
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
				//	Calculate s/z, t/z, zi->fixed s and t at last pixel in
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
					snext = 8;	//	Prevent round-off error on <0 steps from
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
				*pdest++ = pbase[(s >> 16) + (t >> 16) * cachewidth];
				s += sstep;
				t += tstep;
			} while (--spancount > 0);

			s = snext;
			t = tnext;

		} while (count > 0);

	} while ((pspan = pspan->pnext) != NULL);
}

//==========================================================================
//
//	D_DrawZSpan
//
//==========================================================================

extern "C" void D_DrawZSpans(espan_t *pspan)
{
	int				count, doublecount, izistep;
	dword			izi;
	word			*pdest;
	unsigned		ltemp;
	double			zi;
	float			du, dv;

	// FIXME: check for clamping/range problems
	// we count on FP exceptions being turned off to avoid range problems
	izistep = (int)(d_zistepu * 0x8000 * 0x10000);

	do
	{
		pdest = zbuffer + ylookup[pspan->v] + pspan->u;

		count = pspan->count;

		// calculate the initial 1/z
		du = (float)pspan->u;
		dv = (float)pspan->v;

		zi = d_ziorigin + dv*d_zistepv + du*d_zistepu;
		// we count on FP exceptions being turned off to avoid range problems
		izi = (int)(zi * 0x8000 * 0x10000);

		if ((long)pdest & 0x02)
		{
			//	Align dest to dword
			*pdest++ = (word)(izi >> 16);
			izi += izistep;
			count--;
		}

		if ((doublecount = count >> 1) > 0)
		{
			//	Process pixels by 2 pixels at time
			do
			{
				ltemp = izi >> 16;
				izi += izistep;
				ltemp |= izi & 0xFFFF0000;
				izi += izistep;
				*(dword *)pdest = ltemp;
				pdest += 2;
			} while (--doublecount > 0);
		}

		//	Process remaining pixel (if there is any)
		if (count & 1)
			*pdest = (word)(izi >> 16);

	} while ((pspan = pspan->pnext) != NULL);
}
#endif	//	USEASM

//#ifndef USEASM
//==========================================================================
//
//	D_DrawSkySpans_8
//
//==========================================================================

extern "C" void D_DrawSkySpans_8(espan_t *pspan)
{
	int				count, spancount;
	byte			*pbase, *pdest;
	fixed_t			s, t, snext, tnext, sstep, tstep;
	float			sdivz, tdivz, zi, z, du, dv, spancountminus1;
	float			sdivz8stepu, tdivz8stepu, zi8stepu;

	sstep = 0;	// keep compiler happy
	tstep = 0;	// ditto

	pbase = (byte *)cacheblock;

	sdivz8stepu = d_sdivzstepu * 8;
	tdivz8stepu = d_tdivzstepu * 8;
	zi8stepu = d_zistepu * 8;

	do
	{
	    pdest = (byte *)scrn + ylookup[pspan->v] + pspan->u;

		count = pspan->count;

		// calculate the initial s/z, t/z, 1/z, s, and t and clamp
		du = (float)pspan->u;
		dv = (float)pspan->v;

		sdivz = d_sdivzorigin + dv*d_sdivzstepv + du*d_sdivzstepu;
		tdivz = d_tdivzorigin + dv*d_tdivzstepv + du*d_tdivzstepu;
		zi = d_ziorigin + dv*d_zistepv + du*d_zistepu;
		z = (float)0x10000 / zi;	// prescale to 16.16 fixed-point

		s = (int)(sdivz * z) + sadjust;
/*		if (s > bbextents)
			s = bbextents;
		else if (s < 0)
			s = 0;*/

		t = (int)(tdivz * z) + tadjust;
/*		if (t > bbextentt)
			t = bbextentt;
		else if (t < 0)
			t = 0;*/

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
/*				if (snext > bbextents)
					snext = bbextents;
				else if (snext < 8)
					snext = 8;	//	Prevent round-off error on <0 steps from
								// from causing overstepping & running off
								// the edge of the texture
*/
				tnext = (int)(tdivz * z) + tadjust;
/*				if (tnext > bbextentt)
					tnext = bbextentt;
				else if (tnext < 8)
					tnext = 8;	// guard against round-off error on <0 steps
*/
				sstep = (snext - s) >> 3;
				tstep = (tnext - t) >> 3;
			}
			else
			{
				//	Calculate s/z, t/z, zi->fixed s and t at last pixel in
				// span (so can't step off polygon), clamp, calculate s and
				// t steps across span by division, biasing steps low so we
				// don't run off the texture
				spancountminus1 = (float)(spancount - 1);
				sdivz += d_sdivzstepu * spancountminus1;
				tdivz += d_tdivzstepu * spancountminus1;
				zi += d_zistepu * spancountminus1;
				z = (float)0x10000 / zi;	// prescale to 16.16 fixed-point
				snext = (int)(sdivz * z) + sadjust;
/*				if (snext > bbextents)
					snext = bbextents;
				else if (snext < 8)
					snext = 8;	//	Prevent round-off error on <0 steps from
								// from causing overstepping & running off
								// the edge of the texture
*/
				tnext = (int)(tdivz * z) + tadjust;
/*				if (tnext > bbextentt)
					tnext = bbextentt;
				else if (tnext < 8)
					tnext = 8;	// guard against round-off error on <0 steps
*/
				if (spancount > 1)
				{
					sstep = (snext - s) / (spancount - 1);
					tstep = (tnext - t) / (spancount - 1);
				}
			}

			do
			{
//				*pdest++ = pbase[(s >> 16) + (t >> 16) * cachewidth];
				*pdest++ = pbase[((t >> 6) & 0x3fc00) | ((s >> 16) & 0x3ff)];
				s += sstep;
				t += tstep;
			} while (--spancount > 0);

			s = snext;
			t = tnext;

		} while (count > 0);

	} while ((pspan = pspan->pnext) != NULL);
}
//#endif

//==========================================================================
//
//	D_DrawDoubleSkySpans_8
//
//==========================================================================

extern "C" void D_DrawDoubleSkySpans_8(espan_t *pspan)
{
	int				count, spancount;
	byte			*pbase, *pdest;
	fixed_t			s, t, snext, tnext, sstep, tstep;
	float			sdivz, tdivz, zi, z, du, dv, spancountminus1;
	float			sdivz8stepu, tdivz8stepu, zi8stepu;
	byte			btemp;

	sstep = 0;	// keep compiler happy
	tstep = 0;	// ditto

	pbase = (byte *)cacheblock;

	sdivz8stepu = d_sdivzstepu * 8;
	tdivz8stepu = d_tdivzstepu * 8;
	zi8stepu = d_zistepu * 8;

	do
	{
	    pdest = (byte *)scrn + ylookup[pspan->v] + pspan->u;

		count = pspan->count;

		// calculate the initial s/z, t/z, 1/z, s, and t and clamp
		du = (float)pspan->u;
		dv = (float)pspan->v;

		sdivz = d_sdivzorigin + dv*d_sdivzstepv + du*d_sdivzstepu;
		tdivz = d_tdivzorigin + dv*d_tdivzstepv + du*d_tdivzstepu;
		zi = d_ziorigin + dv*d_zistepv + du*d_zistepu;
		z = (float)0x10000 / zi;	// prescale to 16.16 fixed-point

		s = (int)(sdivz * z) + sadjust;
/*		if (s > bbextents)
			s = bbextents;
		else if (s < 0)
			s = 0;*/

		t = (int)(tdivz * z) + tadjust;
/*		if (t > bbextentt)
			t = bbextentt;
		else if (t < 0)
			t = 0;*/

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
/*				if (snext > bbextents)
					snext = bbextents;
				else if (snext < 8)
					snext = 8;	//	Prevent round-off error on <0 steps from
								// from causing overstepping & running off
								// the edge of the texture
*/
				tnext = (int)(tdivz * z) + tadjust;
/*				if (tnext > bbextentt)
					tnext = bbextentt;
				else if (tnext < 8)
					tnext = 8;	// guard against round-off error on <0 steps
*/
				sstep = (snext - s) >> 3;
				tstep = (tnext - t) >> 3;
			}
			else
			{
				//	Calculate s/z, t/z, zi->fixed s and t at last pixel in
				// span (so can't step off polygon), clamp, calculate s and
				// t steps across span by division, biasing steps low so we
				// don't run off the texture
				spancountminus1 = (float)(spancount - 1);
				sdivz += d_sdivzstepu * spancountminus1;
				tdivz += d_tdivzstepu * spancountminus1;
				zi += d_zistepu * spancountminus1;
				z = (float)0x10000 / zi;	// prescale to 16.16 fixed-point
				snext = (int)(sdivz * z) + sadjust;
/*				if (snext > bbextents)
					snext = bbextents;
				else if (snext < 8)
					snext = 8;	//	Prevent round-off error on <0 steps from
								// from causing overstepping & running off
								// the edge of the texture
*/
				tnext = (int)(tdivz * z) + tadjust;
/*				if (tnext > bbextentt)
					tnext = bbextentt;
				else if (tnext < 8)
					tnext = 8;	// guard against round-off error on <0 steps
*/
				if (spancount > 1)
				{
					sstep = (snext - s) / (spancount - 1);
					tstep = (tnext - t) / (spancount - 1);
				}
			}

			do
			{
//				btemp = pbase[(s >> 16) + (t >> 16) * cachewidth];
				btemp = pbase[((t >> 6) & 0x3fc00) | ((s >> 16) & 0x3ff)];
				if (btemp)
				{
					*pdest = btemp;
				}
				pdest++;
				s += sstep;
				t += tstep;
			} while (--spancount > 0);

			s = snext;
			t = tnext;

		} while (count > 0);

	} while ((pspan = pspan->pnext) != NULL);
}

//#ifndef USEASM
//==========================================================================
//
//	D_DrawSkySpans_16
//
//==========================================================================

extern "C" void D_DrawSkySpans_16(espan_t *pspan)
{
	int				count, spancount;
	byte			*pbase;
	word			*pdest;
	fixed_t			s, t, snext, tnext, sstep, tstep;
	float			sdivz, tdivz, zi, z, du, dv, spancountminus1;
	float			sdivz8stepu, tdivz8stepu, zi8stepu;

	sstep = 0;	// keep compiler happy
	tstep = 0;	// ditto

	pbase = (byte *)cacheblock;

	sdivz8stepu = d_sdivzstepu * 8;
	tdivz8stepu = d_tdivzstepu * 8;
	zi8stepu = d_zistepu * 8;

	do
	{
	    pdest = (word *)scrn + ylookup[pspan->v] + pspan->u;

		count = pspan->count;

		// calculate the initial s/z, t/z, 1/z, s, and t and clamp
		du = (float)pspan->u;
		dv = (float)pspan->v;

		sdivz = d_sdivzorigin + dv*d_sdivzstepv + du*d_sdivzstepu;
		tdivz = d_tdivzorigin + dv*d_tdivzstepv + du*d_tdivzstepu;
		zi = d_ziorigin + dv*d_zistepv + du*d_zistepu;
		z = (float)0x10000 / zi;	// prescale to 16.16 fixed-point

		s = (int)(sdivz * z) + sadjust;
/*		if (s > bbextents)
			s = bbextents;
		else if (s < 0)
			s = 0;*/

		t = (int)(tdivz * z) + tadjust;
/*		if (t > bbextentt)
			t = bbextentt;
		else if (t < 0)
			t = 0;*/

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
/*				if (snext > bbextents)
					snext = bbextents;
				else if (snext < 8)
					snext = 8;	//	Prevent round-off error on <0 steps from
								// from causing overstepping & running off
								// the edge of the texture
*/
				tnext = (int)(tdivz * z) + tadjust;
/*				if (tnext > bbextentt)
					tnext = bbextentt;
				else if (tnext < 8)
					tnext = 8;	// guard against round-off error on <0 steps
*/
				sstep = (snext - s) >> 3;
				tstep = (tnext - t) >> 3;
			}
			else
			{
				//	Calculate s/z, t/z, zi->fixed s and t at last pixel in
				// span (so can't step off polygon), clamp, calculate s and
				// t steps across span by division, biasing steps low so we
				// don't run off the texture
				spancountminus1 = (float)(spancount - 1);
				sdivz += d_sdivzstepu * spancountminus1;
				tdivz += d_tdivzstepu * spancountminus1;
				zi += d_zistepu * spancountminus1;
				z = (float)0x10000 / zi;	// prescale to 16.16 fixed-point
				snext = (int)(sdivz * z) + sadjust;
/*				if (snext > bbextents)
					snext = bbextents;
				else if (snext < 8)
					snext = 8;	//	Prevent round-off error on <0 steps from
								// from causing overstepping & running off
								// the edge of the texture
*/
				tnext = (int)(tdivz * z) + tadjust;
/*				if (tnext > bbextentt)
					tnext = bbextentt;
				else if (tnext < 8)
					tnext = 8;	// guard against round-off error on <0 steps
*/
				if (spancount > 1)
				{
					sstep = (snext - s) / (spancount - 1);
					tstep = (tnext - t) / (spancount - 1);
				}
			}

			do
			{
//				*pdest++ = pbase[(s >> 16) + (t >> 16) * cachewidth];
				*pdest++ = pal8_to16[pbase[((t >> 6) & 0x3fc00) | ((s >> 16) & 0x3ff)]];
				s += sstep;
				t += tstep;
			} while (--spancount > 0);

			s = snext;
			t = tnext;

		} while (count > 0);

	} while ((pspan = pspan->pnext) != NULL);
}
//#endif

//==========================================================================
//
//	D_DrawDoubleSkySpans_16
//
//==========================================================================

extern "C" void D_DrawDoubleSkySpans_16(espan_t *pspan)
{
	int				count, spancount;
	byte			*pbase;
	word			*pdest;
	fixed_t			s, t, snext, tnext, sstep, tstep;
	float			sdivz, tdivz, zi, z, du, dv, spancountminus1;
	float			sdivz8stepu, tdivz8stepu, zi8stepu;
	byte			btemp;

	sstep = 0;	// keep compiler happy
	tstep = 0;	// ditto

	pbase = (byte *)cacheblock;

	sdivz8stepu = d_sdivzstepu * 8;
	tdivz8stepu = d_tdivzstepu * 8;
	zi8stepu = d_zistepu * 8;

	do
	{
	    pdest = (word *)scrn + ylookup[pspan->v] + pspan->u;

		count = pspan->count;

		// calculate the initial s/z, t/z, 1/z, s, and t and clamp
		du = (float)pspan->u;
		dv = (float)pspan->v;

		sdivz = d_sdivzorigin + dv*d_sdivzstepv + du*d_sdivzstepu;
		tdivz = d_tdivzorigin + dv*d_tdivzstepv + du*d_tdivzstepu;
		zi = d_ziorigin + dv*d_zistepv + du*d_zistepu;
		z = (float)0x10000 / zi;	// prescale to 16.16 fixed-point

		s = (int)(sdivz * z) + sadjust;
/*		if (s > bbextents)
			s = bbextents;
		else if (s < 0)
			s = 0;*/

		t = (int)(tdivz * z) + tadjust;
/*		if (t > bbextentt)
			t = bbextentt;
		else if (t < 0)
			t = 0;*/

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
/*				if (snext > bbextents)
					snext = bbextents;
				else if (snext < 8)
					snext = 8;	//	Prevent round-off error on <0 steps from
								// from causing overstepping & running off
								// the edge of the texture
*/
				tnext = (int)(tdivz * z) + tadjust;
/*				if (tnext > bbextentt)
					tnext = bbextentt;
				else if (tnext < 8)
					tnext = 8;	// guard against round-off error on <0 steps
*/
				sstep = (snext - s) >> 3;
				tstep = (tnext - t) >> 3;
			}
			else
			{
				//	Calculate s/z, t/z, zi->fixed s and t at last pixel in
				// span (so can't step off polygon), clamp, calculate s and
				// t steps across span by division, biasing steps low so we
				// don't run off the texture
				spancountminus1 = (float)(spancount - 1);
				sdivz += d_sdivzstepu * spancountminus1;
				tdivz += d_tdivzstepu * spancountminus1;
				zi += d_zistepu * spancountminus1;
				z = (float)0x10000 / zi;	// prescale to 16.16 fixed-point
				snext = (int)(sdivz * z) + sadjust;
/*				if (snext > bbextents)
					snext = bbextents;
				else if (snext < 8)
					snext = 8;	//	Prevent round-off error on <0 steps from
								// from causing overstepping & running off
								// the edge of the texture
*/
				tnext = (int)(tdivz * z) + tadjust;
/*				if (tnext > bbextentt)
					tnext = bbextentt;
				else if (tnext < 8)
					tnext = 8;	// guard against round-off error on <0 steps
*/
				if (spancount > 1)
				{
					sstep = (snext - s) / (spancount - 1);
					tstep = (tnext - t) / (spancount - 1);
				}
			}

			do
			{
//				btemp = pbase[(s >> 16) + (t >> 16) * cachewidth];
				btemp = pbase[((t >> 6) & 0x3fc00) | ((s >> 16) & 0x3ff)];
				if (btemp)
				{
					*pdest = pal8_to16[btemp];
				}
				pdest++;
				s += sstep;
				t += tstep;
			} while (--spancount > 0);

			s = snext;
			t = tnext;

		} while (count > 0);

	} while ((pspan = pspan->pnext) != NULL);
}

//#ifndef USEASM
//==========================================================================
//
//	D_DrawSkySpans_32
//
//==========================================================================

extern "C" void D_DrawSkySpans_32(espan_t *pspan)
{
	int				count, spancount;
	byte			*pbase;
	dword			*pdest;
	fixed_t			s, t, snext, tnext, sstep, tstep;
	float			sdivz, tdivz, zi, z, du, dv, spancountminus1;
	float			sdivz8stepu, tdivz8stepu, zi8stepu;

	sstep = 0;	// keep compiler happy
	tstep = 0;	// ditto

	pbase = (byte *)cacheblock;

	sdivz8stepu = d_sdivzstepu * 8;
	tdivz8stepu = d_tdivzstepu * 8;
	zi8stepu = d_zistepu * 8;

	do
	{
	    pdest = (dword *)scrn + ylookup[pspan->v] + pspan->u;

		count = pspan->count;

		// calculate the initial s/z, t/z, 1/z, s, and t and clamp
		du = (float)pspan->u;
		dv = (float)pspan->v;

		sdivz = d_sdivzorigin + dv*d_sdivzstepv + du*d_sdivzstepu;
		tdivz = d_tdivzorigin + dv*d_tdivzstepv + du*d_tdivzstepu;
		zi = d_ziorigin + dv*d_zistepv + du*d_zistepu;
		z = (float)0x10000 / zi;	// prescale to 16.16 fixed-point

		s = (int)(sdivz * z) + sadjust;
/*		if (s > bbextents)
			s = bbextents;
		else if (s < 0)
			s = 0;*/

		t = (int)(tdivz * z) + tadjust;
/*		if (t > bbextentt)
			t = bbextentt;
		else if (t < 0)
			t = 0;*/

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
/*				if (snext > bbextents)
					snext = bbextents;
				else if (snext < 8)
					snext = 8;	//	Prevent round-off error on <0 steps from
								// from causing overstepping & running off
								// the edge of the texture
*/
				tnext = (int)(tdivz * z) + tadjust;
/*				if (tnext > bbextentt)
					tnext = bbextentt;
				else if (tnext < 8)
					tnext = 8;	// guard against round-off error on <0 steps
*/
				sstep = (snext - s) >> 3;
				tstep = (tnext - t) >> 3;
			}
			else
			{
				//	Calculate s/z, t/z, zi->fixed s and t at last pixel in
				// span (so can't step off polygon), clamp, calculate s and
				// t steps across span by division, biasing steps low so we
				// don't run off the texture
				spancountminus1 = (float)(spancount - 1);
				sdivz += d_sdivzstepu * spancountminus1;
				tdivz += d_tdivzstepu * spancountminus1;
				zi += d_zistepu * spancountminus1;
				z = (float)0x10000 / zi;	// prescale to 16.16 fixed-point
				snext = (int)(sdivz * z) + sadjust;
/*				if (snext > bbextents)
					snext = bbextents;
				else if (snext < 8)
					snext = 8;	//	Prevent round-off error on <0 steps from
								// from causing overstepping & running off
								// the edge of the texture
*/
				tnext = (int)(tdivz * z) + tadjust;
/*				if (tnext > bbextentt)
					tnext = bbextentt;
				else if (tnext < 8)
					tnext = 8;	// guard against round-off error on <0 steps
*/
				if (spancount > 1)
				{
					sstep = (snext - s) / (spancount - 1);
					tstep = (tnext - t) / (spancount - 1);
				}
			}

			do
			{
//				*pdest++ = pbase[(s >> 16) + (t >> 16) * cachewidth];
				*pdest++ = pal2rgb[pbase[((t >> 6) & 0x3fc00) | ((s >> 16) & 0x3ff)]];
				s += sstep;
				t += tstep;
			} while (--spancount > 0);

			s = snext;
			t = tnext;

		} while (count > 0);

	} while ((pspan = pspan->pnext) != NULL);
}
//#endif

//==========================================================================
//
//	D_DrawDoubleSkySpans_32
//
//==========================================================================

extern "C" void D_DrawDoubleSkySpans_32(espan_t *pspan)
{
	int				count, spancount;
	byte			*pbase;
	dword			*pdest;
	fixed_t			s, t, snext, tnext, sstep, tstep;
	float			sdivz, tdivz, zi, z, du, dv, spancountminus1;
	float			sdivz8stepu, tdivz8stepu, zi8stepu;
	byte			btemp;

	sstep = 0;	// keep compiler happy
	tstep = 0;	// ditto

	pbase = (byte *)cacheblock;

	sdivz8stepu = d_sdivzstepu * 8;
	tdivz8stepu = d_tdivzstepu * 8;
	zi8stepu = d_zistepu * 8;

	do
	{
	    pdest = (dword *)scrn + ylookup[pspan->v] + pspan->u;

		count = pspan->count;

		// calculate the initial s/z, t/z, 1/z, s, and t and clamp
		du = (float)pspan->u;
		dv = (float)pspan->v;

		sdivz = d_sdivzorigin + dv*d_sdivzstepv + du*d_sdivzstepu;
		tdivz = d_tdivzorigin + dv*d_tdivzstepv + du*d_tdivzstepu;
		zi = d_ziorigin + dv*d_zistepv + du*d_zistepu;
		z = (float)0x10000 / zi;	// prescale to 16.16 fixed-point

		s = (int)(sdivz * z) + sadjust;
/*		if (s > bbextents)
			s = bbextents;
		else if (s < 0)
			s = 0;*/

		t = (int)(tdivz * z) + tadjust;
/*		if (t > bbextentt)
			t = bbextentt;
		else if (t < 0)
			t = 0;*/

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
/*				if (snext > bbextents)
					snext = bbextents;
				else if (snext < 8)
					snext = 8;	//	Prevent round-off error on <0 steps from
								// from causing overstepping & running off
								// the edge of the texture
*/
				tnext = (int)(tdivz * z) + tadjust;
/*				if (tnext > bbextentt)
					tnext = bbextentt;
				else if (tnext < 8)
					tnext = 8;	// guard against round-off error on <0 steps
*/
				sstep = (snext - s) >> 3;
				tstep = (tnext - t) >> 3;
			}
			else
			{
				//	Calculate s/z, t/z, zi->fixed s and t at last pixel in
				// span (so can't step off polygon), clamp, calculate s and
				// t steps across span by division, biasing steps low so we
				// don't run off the texture
				spancountminus1 = (float)(spancount - 1);
				sdivz += d_sdivzstepu * spancountminus1;
				tdivz += d_tdivzstepu * spancountminus1;
				zi += d_zistepu * spancountminus1;
				z = (float)0x10000 / zi;	// prescale to 16.16 fixed-point
				snext = (int)(sdivz * z) + sadjust;
/*				if (snext > bbextents)
					snext = bbextents;
				else if (snext < 8)
					snext = 8;	//	Prevent round-off error on <0 steps from
								// from causing overstepping & running off
								// the edge of the texture
*/
				tnext = (int)(tdivz * z) + tadjust;
/*				if (tnext > bbextentt)
					tnext = bbextentt;
				else if (tnext < 8)
					tnext = 8;	// guard against round-off error on <0 steps
*/
				if (spancount > 1)
				{
					sstep = (snext - s) / (spancount - 1);
					tstep = (tnext - t) / (spancount - 1);
				}
			}

			do
			{
//				btemp = pbase[(s >> 16) + (t >> 16) * cachewidth];
				btemp = pbase[((t >> 6) & 0x3fc00) | ((s >> 16) & 0x3ff)];
				if (btemp)
				{
					*pdest = pal2rgb[btemp];
				}
				pdest++;
				s += sstep;
				t += tstep;
			} while (--spancount > 0);

			s = snext;
			t = tnext;

		} while (count > 0);

	} while ((pspan = pspan->pnext) != NULL);
}

