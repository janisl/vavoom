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

#include "asm_i386.h"

#ifdef USEASM

.text

//==========================================================================
//
//	D_DrawZSpans
//
//	Horizontal span z drawing code for polygons, with no transparency.
//
//	Assumes there is at least one span in pzspans, and that every span
// contains at least one pixel
//
//==========================================================================

	.text

	Align4
// z-clamp on a non-negative gradient span
LClamp:
	movl	$0x40000000,%edx
	xorl	%ebx,%ebx
	fstp	%st(0)
	jmp		LZDraw

// z-clamp on a negative gradient span
LClampNeg:
	movl	$0x40000000,%edx
	xorl	%ebx,%ebx
	fstp	%st(0)
	jmp		LZDrawNeg


#define pzspans	4+16

	Align4
.globl C(D_DrawZSpans)
C(D_DrawZSpans):
	pushl	%ebp				// preserve caller's stack frame
	pushl	%edi
	pushl	%esi				// preserve register variables
	pushl	%ebx

	flds	C(d_zistepu)
	movl	C(d_zistepu),%eax
	movl	pzspans(%esp),%esi
	testl	%eax,%eax
	jz		LFNegSpan

	fmuls	Float2ToThe31nd
	fistpl	izistep		// note: we are relying on FP exceptions being turned
						// off here to avoid range problems
	movl	izistep,%ebx	// remains loaded for all spans

LFSpanLoop:
// set up the initial 1/z value
	fildl	espan_t_v(%esi)
	fildl	espan_t_u(%esi)
	movl	espan_t_v(%esi),%eax
	movl	C(zbuffer),%edi
	fmuls	C(d_zistepu)
	fxch	%st(1)
	fmuls	C(d_zistepv)
	fxch	%st(1)
	fadds	C(d_ziorigin)
	movl	C(ylookup)(,%eax,4),%eax
	movl	espan_t_count(%esi),%ecx
	faddp	%st(0),%st(1)

// clamp if z is nearer than 2 (1/z > 0.5)
	fcoms	float_point5
	addl	espan_t_u(%esi),%eax
	leal	(%edi,%eax,2),%edi		// pdest = &pdestspan[scans->u];
	pushl	%esi		// preserve spans pointer
	fnstsw	%ax
	testb	$0x45,%ah
	jz		LClamp

	fmuls	Float2ToThe31nd
	fistpl	izi			// note: we are relying on FP exceptions being turned
						// off here to avoid problems when the span is closer
						// than 1/(2**31)
	movl	izi,%edx

// at this point:
// %ebx = izistep
// %ecx = count
// %edx = izi
// %edi = pdest

LZDraw:

// do a single pixel up front, if necessary to dword align the destination
	testl	$2,%edi
	jz		LFMiddle
	movl	%edx,%eax
	addl	%ebx,%edx
	shrl	$16,%eax
	decl	%ecx
	movw	%ax,(%edi)
	addl	$2,%edi

//--
// do middle a pair of aligned dwords at a time
LFMiddle:
//	testl	%ecx,C(mmx_mask16)	// don't bother with MMX with less than 16 pixels
//	jnz		LDrawZSpan_mmx		// use MMX version
	pushl	%ecx
	shrl	$1,%ecx				// count / 2
	jz		LFLast				// no aligned dwords to do
	shrl	$1,%ecx				// (count / 2) / 2
	jnc		LFMiddleLoop		// even number of aligned dwords to do

	movl	%edx,%eax
	addl	%ebx,%edx
	shrl	$16,%eax
	movl	%edx,%esi
	addl	%ebx,%edx
	andl	$0xFFFF0000,%esi
	orl		%esi,%eax
	movl	%eax,(%edi)
	addl	$4,%edi
	andl	%ecx,%ecx
	jz		LFLast

LFMiddleLoop:
	movl	%edx,%eax
	addl	%ebx,%edx
	shrl	$16,%eax
	movl	%edx,%esi
	addl	%ebx,%edx
	andl	$0xFFFF0000,%esi
	orl		%esi,%eax
	movl	%edx,%ebp
	movl	%eax,(%edi)
	addl	%ebx,%edx
	shrl	$16,%ebp
	movl	%edx,%esi
	addl	%ebx,%edx
	andl	$0xFFFF0000,%esi
	orl		%esi,%ebp
	movl	%ebp,4(%edi)	// FIXME: eliminate register contention
	addl	$8,%edi

	decl	%ecx
	jnz		LFMiddleLoop

LFLast:
	popl	%ecx			// retrieve count

// do the last, unaligned pixel, if there is one
	andl	$1,%ecx			// is there an odd pixel left to do?
	jz		LFSpanDone		// no
	shrl	$16,%edx
	movw	%dx,(%edi)		// do the final pixel's z

LFSpanDone:
	popl	%esi			// retrieve span pointer
	movl	espan_t_pnext(%esi),%esi
	testl	%esi,%esi
	jnz		LFSpanLoop

	jmp		LFDone

	//
	//	MMX version
	//
/*
	Align16
LDrawZSpan_mmx:

	movl	%edx,mmbuf
	addl	%ebx,%edx
	movl	%edx,mmbuf+4
	movq	mmbuf,%mm0

	addl	%ebx,%ebx
	movl	%ebx,mmbuf
	movl	%ebx,mmbuf+4
	movq	mmbuf,%mm1

	//	Do two pixels up front, if necessary to qword align the destination
	testl	$4,%edi
	jz		LMMXDoQwords
	movq	%mm0,%mm2
	paddd	%mm1,%mm0
	psrld	$16,%mm2
	packssdw	%mm0,%mm2
	movd	%mm2,(%edi)
	addl	$4,%edi
	subl	$2,%ecx

LMMXDoQwords:
	movl	%ecx,%ebp

	shrl	$2,%ecx				// count / 4
	shrl	$1,%ecx				// (count / 4) / 2
	jnc		LMMXMiddleLoop		// even number of aligned qwords to do

	movq	%mm0,%mm2
	paddd	%mm1,%mm0
	movq	%mm0,%mm3
	paddd	%mm1,%mm0
	psrld	$16,%mm2
	psrld	$16,%mm3
	packssdw	%mm3,%mm2
	movq	%mm2,(%edi)
	addl	$8,%edi

	Align16max7
LMMXMiddleLoop:
	movq	%mm0,%mm2
	paddd	%mm1,%mm0
	movq	%mm0,%mm3
	paddd	%mm1,%mm0
	psrld	$16,%mm2
	psrld	$16,%mm3
	packssdw	%mm3,%mm2
	movq	%mm2,(%edi)
	movq	%mm0,%mm2
	paddd	%mm1,%mm0
	movq	%mm0,%mm3
	paddd	%mm1,%mm0
	psrld	$16,%mm2
	psrld	$16,%mm3
	packssdw	%mm3,%mm2
	movq	%mm2,8(%edi)
	addl	$16,%edi
	decl	%ecx
	jnz		LMMXMiddleLoop

LMMXLast:
	testl	$2,%ebp
	jz		LMMXLastPixel
	movq	%mm0,%mm2
	paddd	%mm1,%mm0
	psrld	$16,%mm2
	packssdw	%mm0,%mm2
	movd	%mm2,(%edi)
	addl	$4,%edi

	//	Do last unaligned pixel, if there is one
LMMXLastPixel:
	andl	$1,%ebp
	jz		LMMXExit
	movd	%mm0,%eax
	shrl	$16,%eax
	movw	%ax,(%edi)

LMMXExit:
	emms
	jmp		LFSpanDone
*/

LFNegSpan:
	fmuls	FloatMinus2ToThe31nd
	fistpl	izistep		// note: we are relying on FP exceptions being turned
						// off here to avoid range problems
	movl	izistep,%ebx	// remains loaded for all spans

LFNegSpanLoop:
// set up the initial 1/z value
	fildl	espan_t_v(%esi)
	fildl	espan_t_u(%esi)
	movl	espan_t_v(%esi),%ecx
	movl	C(zbuffer),%edi
	fmuls	C(d_zistepu)
	fxch	%st(1)
	fmuls	C(d_zistepv)
	fxch	%st(1)
	fadds	C(d_ziorigin)
	movl	C(ylookup)(,%ecx,4),%ecx
	faddp	%st(0),%st(1)

// clamp if z is nearer than 2 (1/z > 0.5)
	fcoms	float_point5
	leal	(%edi,%ecx,2),%edi
	movl	espan_t_u(%esi),%edx
	addl	%edx,%edx				// word count
	movl	espan_t_count(%esi),%ecx
	addl	%edx,%edi				// pdest = &pdestspan[scans->u];
	pushl	%esi		// preserve spans pointer
	fnstsw	%ax
	testb	$0x45,%ah
	jz		LClampNeg

	fmuls	Float2ToThe31nd
	fistpl	izi			// note: we are relying on FP exceptions being turned
						// off here to avoid problems when the span is closer
						// than 1/(2**31)
	movl	izi,%edx

// at this point:
// %ebx = izistep
// %ecx = count
// %edx = izi
// %edi = pdest

LZDrawNeg:

// do a single pixel up front, if necessary to dword align the destination
	testl	$2,%edi
	jz		LFNegMiddle
	movl	%edx,%eax
	subl	%ebx,%edx
	shrl	$16,%eax
	decl	%ecx
	movw	%ax,(%edi)
	addl	$2,%edi

// do middle a pair of aligned dwords at a time
LFNegMiddle:
	pushl	%ecx
	shrl	$1,%ecx				// count / 2
	jz		LFNegLast			// no aligned dwords to do
	shrl	$1,%ecx				// (count / 2) / 2
	jnc		LFNegMiddleLoop		// even number of aligned dwords to do

	movl	%edx,%eax
	subl	%ebx,%edx
	shrl	$16,%eax
	movl	%edx,%esi
	subl	%ebx,%edx
	andl	$0xFFFF0000,%esi
	orl		%esi,%eax
	movl	%eax,(%edi)
	addl	$4,%edi
	andl	%ecx,%ecx
	jz		LFNegLast

LFNegMiddleLoop:
	movl	%edx,%eax
	subl	%ebx,%edx
	shrl	$16,%eax
	movl	%edx,%esi
	subl	%ebx,%edx
	andl	$0xFFFF0000,%esi
	orl		%esi,%eax
	movl	%edx,%ebp
	movl	%eax,(%edi)
	subl	%ebx,%edx
	shrl	$16,%ebp
	movl	%edx,%esi
	subl	%ebx,%edx
	andl	$0xFFFF0000,%esi
	orl		%esi,%ebp
	movl	%ebp,4(%edi)	// FIXME: eliminate register contention
	addl	$8,%edi

	decl	%ecx
	jnz		LFNegMiddleLoop

LFNegLast:
	popl	%ecx			// retrieve count
	popl	%esi			// retrieve span pointer

// do the last, unaligned pixel, if there is one
	andl	$1,%ecx			// is there an odd pixel left to do?
	jz		LFNegSpanDone	// no
	shrl	$16,%edx
	movw	%dx,(%edi)		// do the final pixel's z

LFNegSpanDone:
	movl	espan_t_pnext(%esi),%esi
	testl	%esi,%esi
	jnz		LFNegSpanLoop

LFDone:
	popl	%ebx				// restore register variables
	popl	%esi
	popl	%edi
	popl	%ebp				// restore the caller's stack frame
	ret

#endif
