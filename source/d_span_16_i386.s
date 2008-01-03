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
//**	x86 assembly-language horizontal 16-bpp span-drawing code.
//**
//**************************************************************************

#include "asm_i386.h"

//==========================================================================
//
//	D_DrawSpans8_16
//
//	16-bpp horizontal span drawing code for polygons, with no transparency.
//
//	Assumes there is at least one span in pspans, and that every span
// contains at least one pixel
//
//==========================================================================

	.text

// out-of-line, rarely-needed clamping code

LClampHigh0:
	movl	C(bbextents),%esi
	jmp		LClampReentry0
LClampHighOrLow0:
	jg		LClampHigh0
	xorl	%esi,%esi
	jmp		LClampReentry0

LClampHigh1:
	movl	C(bbextentt),%edx
	jmp		LClampReentry1
LClampHighOrLow1:
	jg		LClampHigh1
	xorl	%edx,%edx
	jmp		LClampReentry1

LClampLow2:
	movl	$2048,%ebp
	jmp		LClampReentry2
LClampHigh2:
	movl	C(bbextents),%ebp
	jmp		LClampReentry2

LClampLow3:
	movl	$2048,%ecx
	jmp		LClampReentry3
LClampHigh3:
	movl	C(bbextentt),%ecx
	jmp		LClampReentry3

LClampLow4:
	movl	$2048,%eax
	jmp		LClampReentry4
LClampHigh4:
	movl	C(bbextents),%eax
	jmp		LClampReentry4

LClampLow5:
	movl	$2048,%ebx
	jmp		LClampReentry5
LClampHigh5:
	movl	C(bbextentt),%ebx
	jmp		LClampReentry5


#define pspans	4+16

	Align4
.globl C(D_DrawSpans8_16)
C(D_DrawSpans8_16):
	pushl	%ebp				// preserve caller's stack frame
	pushl	%edi
	pushl	%esi				// preserve register variables
	pushl	%ebx

//
// set up scaled-by-8 steps, for 8-long segments; also set up cacheblock
// and span list pointers
//
// TODO: any overlap from rearranging?
	flds	C(d_sdivzstepu)
	fmuls	fp_8
	movl	C(cacheblock),%edx
	flds	C(d_tdivzstepu)
	fmuls	fp_8
	movl	pspans(%esp),%ebx	// point to the first span descriptor
	flds	C(d_zistepu)
	fmuls	fp_8
	movl	%edx,pbase			// pbase = cacheblock
	fstps	zi8stepu
	fstps	tdivz8stepu
	fstps	sdivz8stepu

LSpanLoop:
//
// set up the initial s/z, t/z, and 1/z on the FP stack, and generate the
// initial s and t values
//
// FIXME: pipeline FILD?
	fildl	espan_t_v(%ebx)
	fildl	espan_t_u(%ebx)

	fld		%st(1)			// dv | du | dv
	fmuls	C(d_sdivzstepv)	// dv*d_sdivzstepv | du | dv
	fld		%st(1)			// du | dv*d_sdivzstepv | du | dv
	fmuls	C(d_sdivzstepu)	// du*d_sdivzstepu | dv*d_sdivzstepv | du | dv
	fld		%st(2)			// du | du*d_sdivzstepu | dv*d_sdivzstepv | du | dv
	fmuls	C(d_tdivzstepu)	// du*d_tdivzstepu | du*d_sdivzstepu |
							//  dv*d_sdivzstepv | du | dv
	fxch	%st(1)			// du*d_sdivzstepu | du*d_tdivzstepu |
							//  dv*d_sdivzstepv | du | dv
	faddp	%st(0),%st(2)	// du*d_tdivzstepu |
							//  du*d_sdivzstepu + dv*d_sdivzstepv | du | dv
	fxch	%st(1)			// du*d_sdivzstepu + dv*d_sdivzstepv |
							//  du*d_tdivzstepu | du | dv
	fld		%st(3)			// dv | du*d_sdivzstepu + dv*d_sdivzstepv |
							//  du*d_tdivzstepu | du | dv
	fmuls	C(d_tdivzstepv)	// dv*d_tdivzstepv |
							//  du*d_sdivzstepu + dv*d_sdivzstepv |
							//  du*d_tdivzstepu | du | dv
	fxch	%st(1)			// du*d_sdivzstepu + dv*d_sdivzstepv |
							//  dv*d_tdivzstepv | du*d_tdivzstepu | du | dv
	fadds	C(d_sdivzorigin)	// sdivz = d_sdivzorigin + dv*d_sdivzstepv +
							//  du*d_sdivzstepu; stays in %st(2) at end
	fxch	%st(4)			// dv | dv*d_tdivzstepv | du*d_tdivzstepu | du |
							//  s/z
	fmuls	C(d_zistepv)		// dv*d_zistepv | dv*d_tdivzstepv |
							//  du*d_tdivzstepu | du | s/z
	fxch	%st(1)			// dv*d_tdivzstepv |  dv*d_zistepv |
							//  du*d_tdivzstepu | du | s/z
	faddp	%st(0),%st(2)	// dv*d_zistepv |
							//  dv*d_tdivzstepv + du*d_tdivzstepu | du | s/z
	fxch	%st(2)			// du | dv*d_tdivzstepv + du*d_tdivzstepu |
							//  dv*d_zistepv | s/z
	fmuls	C(d_zistepu)		// du*d_zistepu |
							//  dv*d_tdivzstepv + du*d_tdivzstepu |
							//  dv*d_zistepv | s/z
	fxch	%st(1)			// dv*d_tdivzstepv + du*d_tdivzstepu |
							//  du*d_zistepu | dv*d_zistepv | s/z
	fadds	C(d_tdivzorigin)	// tdivz = d_tdivzorigin + dv*d_tdivzstepv +
							//  du*d_tdivzstepu; stays in %st(1) at end
	fxch	%st(2)			// dv*d_zistepv | du*d_zistepu | t/z | s/z
	faddp	%st(0),%st(1)	// dv*d_zistepv + du*d_zistepu | t/z | s/z

	flds	fp_64k			// fp_64k | dv*d_zistepv + du*d_zistepu | t/z | s/z
	fxch	%st(1)			// dv*d_zistepv + du*d_zistepu | fp_64k | t/z | s/z
	fadds	C(d_ziorigin)		// zi = d_ziorigin + dv*d_zistepv +
							//  du*d_zistepu; stays in %st(0) at end
							// 1/z | fp_64k | t/z | s/z
//
// calculate and clamp s & t
//
	fdivr	%st(0),%st(1)	// 1/z | z*64k | t/z | s/z

//
// point %edi to the first pixel in the span
//
	movl	C(scrn),%edi
	movl	espan_t_v(%ebx),%eax
	movl	%ebx,pspantemp	// preserve spans pointer

	movl	C(tadjust),%edx
	movl	C(sadjust),%esi
	movl	C(ylookup)(,%eax,4),%ecx	// v * screenwidth
	addl	espan_t_u(%ebx),%ecx
	addl	%ecx,%ecx
	addl	%ecx,%edi				// pdest = &pdestspan[scans->u];
	movl	espan_t_count(%ebx),%ecx

//
// now start the FDIV for the end of the span
//
	cmpl	$8,%ecx
	ja		LSetupNotLast1

	decl	%ecx
	jz		LCleanup1		// if only one pixel, no need to start an FDIV
	movl	%ecx,spancountminus1

// finish up the s and t calcs
	fxch	%st(1)			// z*64k | 1/z | t/z | s/z

	fld		%st(0)			// z*64k | z*64k | 1/z | t/z | s/z
	fmul	%st(4),%st(0)	// s | z*64k | 1/z | t/z | s/z
	fxch	%st(1)			// z*64k | s | 1/z | t/z | s/z
	fmul	%st(3),%st(0)	// t | s | 1/z | t/z | s/z
	fxch	%st(1)			// s | t | 1/z | t/z | s/z
	fistpl	s				// 1/z | t | t/z | s/z
	fistpl	t				// 1/z | t/z | s/z

	fildl	spancountminus1

	flds	C(d_tdivzstepu)	// C(d_tdivzstepu) | spancountminus1
	flds	C(d_zistepu)		// C(d_zistepu) | C(d_tdivzstepu) | spancountminus1
	fmul	%st(2),%st(0)	// C(d_zistepu)*scm1 | C(d_tdivzstepu) | scm1
	fxch	%st(1)			// C(d_tdivzstepu) | C(d_zistepu)*scm1 | scm1
	fmul	%st(2),%st(0)	// C(d_tdivzstepu)*scm1 | C(d_zistepu)*scm1 | scm1
	fxch	%st(2)			// scm1 | C(d_zistepu)*scm1 | C(d_tdivzstepu)*scm1
	fmuls	C(d_sdivzstepu)	// C(d_sdivzstepu)*scm1 | C(d_zistepu)*scm1 |
							//  C(d_tdivzstepu)*scm1
	fxch	%st(1)			// C(d_zistepu)*scm1 | C(d_sdivzstepu)*scm1 |
							//  C(d_tdivzstepu)*scm1
	faddp	%st(0),%st(3)	// C(d_sdivzstepu)*scm1 | C(d_tdivzstepu)*scm1
	fxch	%st(1)			// C(d_tdivzstepu)*scm1 | C(d_sdivzstepu)*scm1
	faddp	%st(0),%st(3)	// C(d_sdivzstepu)*scm1
	faddp	%st(0),%st(3)

	flds	fp_64k
	fdiv	%st(1),%st(0)	// this is what we've gone to all this trouble to
							//  overlap
	jmp		LFDIVInFlight1

LCleanup1:
// finish up the s and t calcs
	fxch	%st(1)			// z*64k | 1/z | t/z | s/z

	fld		%st(0)			// z*64k | z*64k | 1/z | t/z | s/z
	fmul	%st(4),%st(0)	// s | z*64k | 1/z | t/z | s/z
	fxch	%st(1)			// z*64k | s | 1/z | t/z | s/z
	fmul	%st(3),%st(0)	// t | s | 1/z | t/z | s/z
	fxch	%st(1)			// s | t | 1/z | t/z | s/z
	fistpl	s				// 1/z | t | t/z | s/z
	fistpl	t				// 1/z | t/z | s/z
	jmp		LFDIVInFlight1

	Align4
LSetupNotLast1:
// finish up the s and t calcs
	fxch	%st(1)			// z*64k | 1/z | t/z | s/z

	fld		%st(0)			// z*64k | z*64k | 1/z | t/z | s/z
	fmul	%st(4),%st(0)	// s | z*64k | 1/z | t/z | s/z
	fxch	%st(1)			// z*64k | s | 1/z | t/z | s/z
	fmul	%st(3),%st(0)	// t | s | 1/z | t/z | s/z
	fxch	%st(1)			// s | t | 1/z | t/z | s/z
	fistpl	s				// 1/z | t | t/z | s/z
	fistpl	t				// 1/z | t/z | s/z

	fadds	zi8stepu
	fxch	%st(2)
	fadds	sdivz8stepu
	fxch	%st(2)
	flds	tdivz8stepu
	faddp	%st(0),%st(2)
	flds	fp_64k
	fdiv	%st(1),%st(0)	// z = 1/1/z
							// this is what we've gone to all this trouble to
							//  overlap
LFDIVInFlight1:

	addl	s,%esi
	addl	t,%edx
	movl	C(bbextents),%ebx
	movl	C(bbextentt),%ebp
	cmpl	%ebx,%esi
	ja		LClampHighOrLow0
LClampReentry0:
	movl	%esi,s
	movl	pbase,%ebx
	shll	$16,%esi
	cmpl	%ebp,%edx
	movl	%esi,sfracf
	ja		LClampHighOrLow1
LClampReentry1:
	movl	%edx,t
	movl	s,%esi					// sfrac = scans->sfrac;
	shll	$16,%edx
	movl	t,%eax					// tfrac = scans->tfrac;
	sarl	$16,%esi
	movl	%edx,tfracf

//
// calculate the texture starting address
//
	sarl	$16,%eax
	movl	C(cachewidth),%edx
	imull	%edx,%eax				// (tfrac >> 16) * cachewidth
	addl	%eax,%esi
	leal	(%ebx,%esi,2),%esi		// psource = pbase + (sfrac >> 16) +
									//           ((tfrac >> 16) * cachewidth);

//
// determine whether last span or not
//
	cmpl	$8,%ecx
	jna		LLastSegment

//
// not the last segment; do full 8-wide segment
//
LNotLastSegment:

//
// advance s/z, t/z, and 1/z, and calculate s & t at end of span and steps to
// get there
//

// pick up after the FDIV that was left in flight previously

	fld		%st(0)			// duplicate it
	fmul	%st(4),%st(0)	// s = s/z * z
	fxch	%st(1)
	fmul	%st(3),%st(0)	// t = t/z * z
	fxch	%st(1)
	fistpl	snext
	fistpl	tnext
	movl	snext,%eax
	movl	tnext,%edx

	movw	(%esi),%bx	// get first source texel
	subl	$8,%ecx		// count off this segments' pixels
	movl	C(sadjust),%ebp
	movl	%ecx,counttemp	// remember count of remaining pixels

	movw	%bx,(%edi)	// store first dest pixel
	movl	C(tadjust),%ecx

	addl	%eax,%ebp
	addl	%edx,%ecx

	movl	C(bbextents),%eax
	movl	C(bbextentt),%edx

	cmpl	$2048,%ebp
	jl		LClampLow2
	cmpl	%eax,%ebp
	ja		LClampHigh2
LClampReentry2:

	cmpl	$2048,%ecx
	jl		LClampLow3
	cmpl	%edx,%ecx
	ja		LClampHigh3
LClampReentry3:

	movl	%ebp,snext
	movl	%ecx,tnext

	subl	s,%ebp
	subl	t,%ecx
	
//
// set up advancetable
//
	movl	%ecx,%eax
	movl	%ebp,%edx
	sarl	$19,%eax			// tstep >>= 16;
	jz		LZero
	sarl	$19,%edx			// sstep >>= 16;
	movl	C(cachewidth),%ebx
	addl	%ebx,%ebx
	imull	%ebx,%eax
	jmp		LSetUp1

LZero:
	sarl	$19,%edx			// sstep >>= 16;
	movl	C(cachewidth),%ebx
	addl	%ebx,%ebx

LSetUp1:

	addl	%edx,%edx
	addl	%edx,%eax			// add in sstep
								// (tstep >> 16) * cachewidth + (sstep >> 16);
	movl	tfracf,%edx
	movl	%eax,advancetable+8	// advance base in t
	movl	%eax,advancetable+12
	addl	%ebx,%eax			// ((tstep >> 16) + 1) * cachewidth +
								//  (sstep >> 16);
	shll	$13,%ebp			// left-justify sstep fractional part
	movl	sfracf,%ebx
	shll	$13,%ecx			// left-justify tstep fractional part
	movl	%eax,advancetable	// advance extra in t
	movl	%eax,advancetable+4

	addl	$2,advancetable+4
	addl	$2,advancetable+12

	movl	%ecx,tstep
	addl	%ecx,%edx			// advance tfrac fractional part by tstep frac

	sbbl	%ecx,%ecx			// turn tstep carry into -1 (0 if none)
	addl	%ebp,%ebx			// advance sfrac fractional part by sstep frac
	adcl	%ecx,%ecx
	addl	advancetable+8(,%ecx,4),%esi	// point to next source texel

	addl	tstep,%edx
	sbbl	%ecx,%ecx
	addl	%ebp,%ebx
	adcl	%ecx,%ecx
	movb	(%esi),%al
	movb	1(%esi),%ah
	addl	advancetable+8(,%ecx,4),%esi

	addl	tstep,%edx
	sbbl	%ecx,%ecx
	addl	%ebp,%ebx
	adcl	%ecx,%ecx
	movb	%al,2(%edi)
	movb	%ah,3(%edi)
	movb	(%esi),%al
	movb	1(%esi),%ah
	addl	advancetable+8(,%ecx,4),%esi

	addl	tstep,%edx
	sbbl	%ecx,%ecx
	addl	%ebp,%ebx
	adcl	%ecx,%ecx
	movb	%al,4(%edi)
	movb	%ah,5(%edi)
	movb	(%esi),%al
	movb	1(%esi),%ah
	addl	advancetable+8(,%ecx,4),%esi

	addl	tstep,%edx
	sbbl	%ecx,%ecx
	addl	%ebp,%ebx
	adcl	%ecx,%ecx
	movb	%al,6(%edi)
	movb	%ah,7(%edi)
	movb	(%esi),%al
	movb	1(%esi),%ah
	addl	advancetable+8(,%ecx,4),%esi


//
// start FDIV for end of next segment in flight, so it can overlap
//
	movl	counttemp,%ecx
	cmpl	$8,%ecx			// more than one segment after this?
	ja		LSetupNotLast2	// yes

	decl	%ecx
	jz		LFDIVInFlight2	// if only one pixel, no need to start an FDIV
	movl	%ecx,spancountminus1
	fildl	spancountminus1

	flds	C(d_zistepu)		// C(d_zistepu) | spancountminus1
	fmul	%st(1),%st(0)	// C(d_zistepu)*scm1 | scm1
	flds	C(d_tdivzstepu)	// C(d_tdivzstepu) | C(d_zistepu)*scm1 | scm1
	fmul	%st(2),%st(0)	// C(d_tdivzstepu)*scm1 | C(d_zistepu)*scm1 | scm1
	fxch	%st(1)			// C(d_zistepu)*scm1 | C(d_tdivzstepu)*scm1 | scm1
	faddp	%st(0),%st(3)	// C(d_tdivzstepu)*scm1 | scm1
	fxch	%st(1)			// scm1 | C(d_tdivzstepu)*scm1
	fmuls	C(d_sdivzstepu)	// C(d_sdivzstepu)*scm1 | C(d_tdivzstepu)*scm1
	fxch	%st(1)			// C(d_tdivzstepu)*scm1 | C(d_sdivzstepu)*scm1
	faddp	%st(0),%st(3)	// C(d_sdivzstepu)*scm1
	flds	fp_64k			// 64k | C(d_sdivzstepu)*scm1
	fxch	%st(1)			// C(d_sdivzstepu)*scm1 | 64k
	faddp	%st(0),%st(4)	// 64k

	fdiv	%st(1),%st(0)	// this is what we've gone to all this trouble to
							//  overlap
	jmp		LFDIVInFlight2

	Align4
LSetupNotLast2:
	fadds	zi8stepu
	fxch	%st(2)
	fadds	sdivz8stepu
	fxch	%st(2)
	flds	tdivz8stepu
	faddp	%st(0),%st(2)
	flds	fp_64k
	fdiv	%st(1),%st(0)	// z = 1/1/z
							// this is what we've gone to all this trouble to
							//  overlap
LFDIVInFlight2:
	movl	%ecx,counttemp

	addl	tstep,%edx
	sbbl	%ecx,%ecx
	addl	%ebp,%ebx
	adcl	%ecx,%ecx
	movb	%al,8(%edi)
	movb	%ah,9(%edi)
	movb	(%esi),%al
	movb	1(%esi),%ah
	addl	advancetable+8(,%ecx,4),%esi

	addl	tstep,%edx
	sbbl	%ecx,%ecx
	addl	%ebp,%ebx
	adcl	%ecx,%ecx
	movb	%al,10(%edi)
	movb	%ah,11(%edi)
	movb	(%esi),%al
	movb	1(%esi),%ah
	addl	advancetable+8(,%ecx,4),%esi

	addl	tstep,%edx
	sbbl	%ecx,%ecx
	addl	%ebp,%ebx
	adcl	%ecx,%ecx
	movb	%al,12(%edi)
	movb	%ah,13(%edi)
	movb	(%esi),%al
	movb	1(%esi),%ah
	addl	advancetable+8(,%ecx,4),%esi

	addl	$16,%edi
	movl	%edx,tfracf
	movl	snext,%edx
	movl	%ebx,sfracf
	movl	tnext,%ebx
	movl	%edx,s
	movl	%ebx,t

	movl	counttemp,%ecx		// retrieve count

//
// determine whether last span or not
//
	cmpl	$8,%ecx				// are there multiple segments remaining?
	movb	%al,-2(%edi)
	movb	%ah,-1(%edi)
	ja		LNotLastSegment		// yes

//
// last segment of scan
//
LLastSegment:

//
// advance s/z, t/z, and 1/z, and calculate s & t at end of span and steps to
// get there. The number of pixels left is variable, and we want to land on the
// last pixel, not step one past it, so we can't run into arithmetic problems
//
	testl	%ecx,%ecx
	jz		LNoSteps		// just draw the last pixel and we're done

// pick up after the FDIV that was left in flight previously


	fld		%st(0)			// duplicate it
	fmul	%st(4),%st(0)	// s = s/z * z
	fxch	%st(1)
	fmul	%st(3),%st(0)	// t = t/z * z
	fxch	%st(1)
	fistpl	snext
	fistpl	tnext

	movw	(%esi),%ax		// load first texel in segment
	movl	C(tadjust),%ebx
	movw	%ax,(%edi)		// store first pixel in segment
	movl	C(sadjust),%eax

	addl	snext,%eax
	addl	tnext,%ebx

	movl	C(bbextents),%ebp
	movl	C(bbextentt),%edx

	cmpl	$2048,%eax
	jl		LClampLow4
	cmpl	%ebp,%eax
	ja		LClampHigh4
LClampReentry4:
	movl	%eax,snext

	cmpl	$2048,%ebx
	jl		LClampLow5
	cmpl	%edx,%ebx
	ja		LClampHigh5
LClampReentry5:

	cmpl	$1,%ecx			// don't bother 
	je		LOnlyOneStep	// if two pixels in segment, there's only one step,
							//  of the segment length
	subl	s,%eax
	subl	t,%ebx

	addl	%eax,%eax		// convert to 15.17 format so multiply by 1.31
	addl	%ebx,%ebx		//  reciprocal yields 16.48

	imull	reciprocal_table-8(,%ecx,4) // sstep = (snext - s) / (spancount-1)
	movl	%edx,%ebp

	movl	%ebx,%eax
	imull	reciprocal_table-8(,%ecx,4) // tstep = (tnext - t) / (spancount-1)

LSetEntryvec:
//
// set up advancetable
//
	movl	Lentryvec_table(,%ecx,4),%ebx
	movl	%edx,%eax
	movl	%ebx,jumptemp		// entry point into code for RET later
	movl	%ebp,%ecx
	sarl	$16,%edx			// tstep >>= 16;
	movl	C(cachewidth),%ebx
	sarl	$16,%ecx			// sstep >>= 16;
	sall	$1,%ecx
	addl	%ebx,%ebx
	imull	%ebx,%edx

	addl	%ecx,%edx			// add in sstep
								// (tstep >> 16) * cachewidth + (sstep >> 16);
	movl	tfracf,%ecx
	movl	%edx,advancetable+8	// advance base in t
	movl	%edx,advancetable+12
	addl	%ebx,%edx			// ((tstep >> 16) + 1) * cachewidth +
								//  (sstep >> 16);
	shll	$16,%ebp			// left-justify sstep fractional part
	movl	sfracf,%ebx
	shll	$16,%eax			// left-justify tstep fractional part
	movl	%edx,advancetable	// advance extra in t
	movl	%edx,advancetable+4

	addl	$2,advancetable+4
	addl	$2,advancetable+12

	movl	%eax,tstep
	movl	%ecx,%edx
	addl	%eax,%edx
	sbbl	%ecx,%ecx
	addl	%ebp,%ebx
	adcl	%ecx,%ecx
	addl	advancetable+8(,%ecx,4),%esi

	jmp		*jumptemp			// jump to the number-of-pixels handler

	Align4
Lentryvec_table:
	.long	0
	.long	LEntry2_8
	.long	LEntry3_8
	.long	LEntry4_8
	.long	LEntry5_8
	.long	LEntry6_8
	.long	LEntry7_8
	.long	LEntry8_8

//----------------------------------------

LNoSteps:
	movw	(%esi),%ax		// load first texel in segment
	subl	$14,%edi			// adjust for hardwired offset
	jmp		LEndSpan


LOnlyOneStep:
	subl	s,%eax
	subl	t,%ebx
	movl	%eax,%ebp
	movl	%ebx,%edx
	jmp		LSetEntryvec

//----------------------------------------

LEntry2_8:
	subl	$12,%edi		// adjust for hardwired offsets
	movb	(%esi),%al
	movb	1(%esi),%ah
	jmp		LLEntry2_8

//----------------------------------------

LEntry3_8:
	subl	$10,%edi		// adjust for hardwired offsets
	addl	%eax,%edx
	sbbl	%ecx,%ecx
	addl	%ebp,%ebx
	adcl	%ecx,%ecx
	movb	(%esi),%al
	movb	1(%esi),%ah
	addl	advancetable+8(,%ecx,4),%esi
	jmp		LLEntry3_8

//----------------------------------------

LEntry4_8:
	subl	$8,%edi		// adjust for hardwired offsets
	addl	%eax,%edx
	sbbl	%ecx,%ecx
	addl	%ebp,%ebx
	adcl	%ecx,%ecx
	movb	(%esi),%al
	movb	1(%esi),%ah
	addl	advancetable+8(,%ecx,4),%esi
	addl	tstep,%edx
	jmp		LLEntry4_8

//----------------------------------------

LEntry5_8:
	subl	$6,%edi		// adjust for hardwired offsets
	addl	%eax,%edx
	sbbl	%ecx,%ecx
	addl	%ebp,%ebx
	adcl	%ecx,%ecx
	movb	(%esi),%al
	movb	1(%esi),%ah
	addl	advancetable+8(,%ecx,4),%esi
	addl	tstep,%edx
	jmp		LLEntry5_8

//----------------------------------------

LEntry6_8:
	subl	$4,%edi		// adjust for hardwired offsets
	addl	%eax,%edx
	sbbl	%ecx,%ecx
	addl	%ebp,%ebx
	adcl	%ecx,%ecx
	movb	(%esi),%al
	movb	1(%esi),%ah
	addl	advancetable+8(,%ecx,4),%esi
	addl	tstep,%edx
	jmp		LLEntry6_8

//----------------------------------------

LEntry7_8:
	subl	$2,%edi		// adjust for hardwired offsets
	addl	%eax,%edx
	sbbl	%ecx,%ecx
	addl	%ebp,%ebx
	adcl	%ecx,%ecx
	movb	(%esi),%al
	movb	1(%esi),%ah
	addl	advancetable+8(,%ecx,4),%esi
	addl	tstep,%edx
	jmp		LLEntry7_8

//----------------------------------------

LEntry8_8:
	addl	%eax,%edx
	sbbl	%ecx,%ecx
	addl	%ebp,%ebx
	adcl	%ecx,%ecx
	movb	(%esi),%al
	movb	1(%esi),%ah
	addl	advancetable+8(,%ecx,4),%esi

	addl	tstep,%edx
	sbbl	%ecx,%ecx
	addl	%ebp,%ebx
	adcl	%ecx,%ecx
	movb	%al,2(%edi)
	movb	%ah,3(%edi)
	movb	(%esi),%al
	movb	1(%esi),%ah
	addl	advancetable+8(,%ecx,4),%esi
	addl	tstep,%edx
LLEntry7_8:
	sbbl	%ecx,%ecx
	addl	%ebp,%ebx
	adcl	%ecx,%ecx
	movb	%al,4(%edi)
	movb	%ah,5(%edi)
	movb	(%esi),%al
	movb	1(%esi),%ah
	addl	advancetable+8(,%ecx,4),%esi
	addl	tstep,%edx
LLEntry6_8:
	sbbl	%ecx,%ecx
	addl	%ebp,%ebx
	adcl	%ecx,%ecx
	movb	%al,6(%edi)
	movb	%ah,7(%edi)
	movb	(%esi),%al
	movb	1(%esi),%ah
	addl	advancetable+8(,%ecx,4),%esi
	addl	tstep,%edx
LLEntry5_8:
	sbbl	%ecx,%ecx
	addl	%ebp,%ebx
	adcl	%ecx,%ecx
	movb	%al,8(%edi)
	movb	%ah,9(%edi)
	movb	(%esi),%al
	movb	1(%esi),%ah
	addl	advancetable+8(,%ecx,4),%esi
	addl	tstep,%edx
LLEntry4_8:
	sbbl	%ecx,%ecx
	addl	%ebp,%ebx
	adcl	%ecx,%ecx
	movb	%al,10(%edi)
	movb	%ah,11(%edi)
	movb	(%esi),%al
	movb	1(%esi),%ah
	addl	advancetable+8(,%ecx,4),%esi
LLEntry3_8:
	movb	%al,12(%edi)
	movb	%ah,13(%edi)
	movb	(%esi),%al
	movb	1(%esi),%ah
LLEntry2_8:

LEndSpan:

//
// clear s/z, t/z, 1/z from FP stack
//
	fstp %st(0)
	fstp %st(0)
	fstp %st(0)

	movl	pspantemp,%ebx				// restore spans pointer
	movl	espan_t_pnext(%ebx),%ebx	// point to next span
	testl	%ebx,%ebx			// any more spans?
	movb	%al,14(%edi)
	movb	%ah,15(%edi)
	jnz		LSpanLoop			// more spans

	popl	%ebx				// restore register variables
	popl	%esi
	popl	%edi
	popl	%ebp				// restore the caller's stack frame
	ret
