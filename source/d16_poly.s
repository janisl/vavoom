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

#include "asm_i386.h"

#ifdef USEASM

	.text

	Align4

#define pspans	4+8

.globl C(D_PolysetAff16Start)
C(D_PolysetAff16Start):

//==========================================================================
//
//	D_PolysetDrawSpans_16
//
//	16-bpp horizontal span drawing code for affine polygons, with smooth
// shading and no transparency
//
//==========================================================================

.globl C(D_PolysetDrawSpans_16)
C(D_PolysetDrawSpans_16):
	pushl	%esi				// preserve register variables
	pushl	%ebx

	movl	pspans(%esp),%esi	// point to the first span descriptor
	movl	C(r_zistepx),%ecx

	pushl	%ebp				// preserve caller's stack frame
	pushl	%edi

	rorl	$16,%ecx			// put high 16 bits of 1/z step in low word
	movswl	spanpackage_t_count(%esi),%edx

	movl	%ecx,lzistepx

LSpanLoop:

//		lcount = d_aspancount - pspanpackage->count;
//
//		errorterm += erroradjustup;
//		if (errorterm >= 0)
//		{
//			d_aspancount += d_countextrastep;
//			errorterm -= erroradjustdown;
//		}
//		else
//		{
//			d_aspancount += ubasestep;
//		}
	movl	C(d_aspancount),%eax
	subl	%edx,%eax

	movl	C(erroradjustup),%edx
	movl	C(errorterm),%ebx
	addl	%edx,%ebx
	js		LNoTurnover

	movl	C(erroradjustdown),%edx
	movl	C(d_countextrastep),%edi
	subl	%edx,%ebx
	movl	C(d_aspancount),%ebp
	movl	%ebx,C(errorterm)
	addl	%edi,%ebp
	movl	%ebp,C(d_aspancount)
	jmp		LRightEdgeStepped

LNoTurnover:
	movl	C(d_aspancount),%edi
	movl	C(ubasestep),%edx
	movl	%ebx,C(errorterm)
	addl	%edx,%edi
	movl	%edi,C(d_aspancount)

LRightEdgeStepped:
	cmpl	$1,%eax

	jl		LNextSpan
	jz		LExactlyOneLong

//
// set up advancetable
//
	movl	C(a_ststepxwhole),%ecx
	movl	C(d_affinetridesc)+atd_skinwidth,%edx

	movl	%ecx,advancetable+4	// advance base in t
	addl	%edx,%ecx

	movl	%ecx,advancetable	// advance extra in t
	movl	C(a_tstepxfrac),%ecx

	movw	C(r_rstepx),%cx
	movl	%eax,%edx			// count

	movl	%ecx,tstep
	addl	$7,%edx

	shrl	$3,%edx				// count of full and partial loops
	movl	spanpackage_t_sfrac(%esi),%ebx

	movw	%dx,%bx
	movl	spanpackage_t_pz(%esi),%ecx

	negl	%eax

	movl	spanpackage_t_pdest(%esi),%edi
	andl	$7,%eax		// 0->0, 1->7, 2->6, ... , 7->1

	subl	%eax,%edi	// compensate for hardwired offsets
	subl	%eax,%ecx

	subl	%eax,%edi

	subl	%eax,%ecx
	movl	spanpackage_t_tfrac(%esi),%edx

	movw	spanpackage_t_r(%esi),%dx
	movl	spanpackage_t_zi(%esi),%ebp

	rorl	$16,%ebp	// put high 16 bits of 1/z in low word
	pushl	%esi

	movl	spanpackage_t_ptex(%esi),%esi
	jmp		*Lentryvec_table(,%eax,4)

Lentryvec_table:
	.long	LDraw8, LDraw7, LDraw6, LDraw5
	.long	LDraw4, LDraw3, LDraw2, LDraw1

// %bx = count of full and partial loops
// %ebx high word = sfrac
// %ecx = pz
// %dx = light
// %edx high word = tfrac
// %esi = ptex
// %edi = pdest
// %ebp = 1/z
// tstep low word = C(r_rstepx)
// tstep high word = C(a_tstepxfrac)
// C(a_sstepxfrac) low word = 0
// C(a_sstepxfrac) high word = C(a_sstepxfrac)

LDrawLoop:

// FIXME: do we need to clamp light? We may need at least a buffer bit to
// keep it from poking into tfrac and causing problems

LDraw8:
	cmpw	(%ecx),%bp
	jl		Lp1
	xorl	%eax,%eax
	movb	%dh,%ah
	movb	(%esi),%al
	movw	%bp,(%ecx)
	movw	0x12345678(,%eax,2),%ax
LPatch8:
	movw	%ax,(%edi)
Lp1:
	addl	tstep,%edx
	sbbl	%eax,%eax
	addl	lzistepx,%ebp
	adcl	$0,%ebp
	addl	C(a_sstepxfrac),%ebx
	adcl	advancetable+4(,%eax,4),%esi

LDraw7:
	cmpw	2(%ecx),%bp
	jl		Lp2
	xorl	%eax,%eax
	movb	%dh,%ah
	movb	(%esi),%al
	movw	%bp,2(%ecx)
	movw	0x12345678(,%eax,2),%ax
LPatch7:
	movw	%ax,2(%edi)
Lp2:
	addl	tstep,%edx
	sbbl	%eax,%eax
	addl	lzistepx,%ebp
	adcl	$0,%ebp
	addl	C(a_sstepxfrac),%ebx
	adcl	advancetable+4(,%eax,4),%esi

LDraw6:
	cmpw	4(%ecx),%bp
	jl		Lp3
	xorl	%eax,%eax
	movb	%dh,%ah
	movb	(%esi),%al
	movw	%bp,4(%ecx)
	movw	0x12345678(,%eax,2),%ax
LPatch6:
	movw	%ax,4(%edi)
Lp3:
	addl	tstep,%edx
	sbbl	%eax,%eax
	addl	lzistepx,%ebp
	adcl	$0,%ebp
	addl	C(a_sstepxfrac),%ebx
	adcl	advancetable+4(,%eax,4),%esi

LDraw5:
	cmpw	6(%ecx),%bp
	jl		Lp4
	xorl	%eax,%eax
	movb	%dh,%ah
	movb	(%esi),%al
	movw	%bp,6(%ecx)
	movw	0x12345678(,%eax,2),%ax
LPatch5:
	movw	%ax,6(%edi)
Lp4:
	addl	tstep,%edx
	sbbl	%eax,%eax
	addl	lzistepx,%ebp
	adcl	$0,%ebp
	addl	C(a_sstepxfrac),%ebx
	adcl	advancetable+4(,%eax,4),%esi

LDraw4:
	cmpw	8(%ecx),%bp
	jl		Lp5
	xorl	%eax,%eax
	movb	%dh,%ah
	movb	(%esi),%al
	movw	%bp,8(%ecx)
	movw	0x12345678(,%eax,2),%ax
LPatch4:
	movw	%ax,8(%edi)
Lp5:
	addl	tstep,%edx
	sbbl	%eax,%eax
	addl	lzistepx,%ebp
	adcl	$0,%ebp
	addl	C(a_sstepxfrac),%ebx
	adcl	advancetable+4(,%eax,4),%esi

LDraw3:
	cmpw	10(%ecx),%bp
	jl		Lp6
	xorl	%eax,%eax
	movb	%dh,%ah
	movb	(%esi),%al
	movw	%bp,10(%ecx)
	movw	0x12345678(,%eax,2),%ax
LPatch3:
	movw	%ax,10(%edi)
Lp6:
	addl	tstep,%edx
	sbbl	%eax,%eax
	addl	lzistepx,%ebp
	adcl	$0,%ebp
	addl	C(a_sstepxfrac),%ebx
	adcl	advancetable+4(,%eax,4),%esi

LDraw2:
	cmpw	12(%ecx),%bp
	jl		Lp7
	xorl	%eax,%eax
	movb	%dh,%ah
	movb	(%esi),%al
	movw	%bp,12(%ecx)
	movw	0x12345678(,%eax,2),%ax
LPatch2:
	movw	%ax,12(%edi)
Lp7:
	addl	tstep,%edx
	sbbl	%eax,%eax
	addl	lzistepx,%ebp
	adcl	$0,%ebp
	addl	C(a_sstepxfrac),%ebx
	adcl	advancetable+4(,%eax,4),%esi

LDraw1:
	cmpw	14(%ecx),%bp
	jl		Lp8
	xorl	%eax,%eax
	movb	%dh,%ah
	movb	(%esi),%al
	movw	%bp,14(%ecx)
	movw	0x12345678(,%eax,2),%ax
LPatch1:
	movw	%ax,14(%edi)
Lp8:
	addl	tstep,%edx
	sbbl	%eax,%eax
	addl	lzistepx,%ebp
	adcl	$0,%ebp
	addl	C(a_sstepxfrac),%ebx
	adcl	advancetable+4(,%eax,4),%esi

	addl	$16,%edi
	addl	$16,%ecx

	decw	%bx
	jnz		LDrawLoop

	popl	%esi				// restore spans pointer
LNextSpan:
	addl	$(spanpackage_t_size),%esi	// point to next span
LNextSpanESISet:
	movswl	spanpackage_t_count(%esi),%edx
	cmpl	$DPS_SPAN_LIST_END,%edx		// any more spans?
	jnz		LSpanLoop			// yes

	popl	%edi
	popl	%ebp				// restore the caller's stack frame
	popl	%ebx				// restore register variables
	popl	%esi
	ret


// draw a one-long span

LExactlyOneLong:

	movl	spanpackage_t_pz(%esi),%ecx
	movl	spanpackage_t_zi(%esi),%ebp

	rorl	$16,%ebp	// put high 16 bits of 1/z in low word
	movl	spanpackage_t_ptex(%esi),%ebx

	cmpw	(%ecx),%bp
	jl		LNextSpan
	xorl	%eax,%eax
	movl	spanpackage_t_pdest(%esi),%edi
	movb	spanpackage_t_r+1(%esi),%ah
	addl	$(spanpackage_t_size),%esi	// point to next span
	movb	(%ebx),%al
	movw	%bp,(%ecx)
	movw	0x12345678(,%eax,2),%ax
LPatch9:
	movw	%ax,(%edi)

	jmp		LNextSpanESISet

//==========================================================================
//
//	D_PolysetDrawSpansRGB_16
//
//	16-bpp horizontal span drawing code for affine polygons, with smooth
// shading and no transparency
//
//==========================================================================

.globl C(D_PolysetDrawSpansRGB_16)
C(D_PolysetDrawSpansRGB_16):
	pushl	%esi				// preserve register variables
	pushl	%ebx

	movl	pspans(%esp),%esi	// point to the first span descriptor
	movl	C(r_zistepx),%ecx

	pushl	%ebp				// preserve caller's stack frame
	pushl	%edi

	rorl	$16,%ecx			// put high 16 bits of 1/z step in low word
	movswl	spanpackage_t_count(%esi),%edx

	movl	%ecx,lzistepx

LRGBSpanLoop:

//		lcount = d_aspancount - pspanpackage->count;
//
//		errorterm += erroradjustup;
//		if (errorterm >= 0)
//		{
//			d_aspancount += d_countextrastep;
//			errorterm -= erroradjustdown;
//		}
//		else
//		{
//			d_aspancount += ubasestep;
//		}
	movl	C(d_aspancount),%eax
	subl	%edx,%eax

	movl	C(erroradjustup),%edx
	movl	C(errorterm),%ebx
	addl	%edx,%ebx
	js		LRGBNoTurnover

	movl	C(erroradjustdown),%edx
	movl	C(d_countextrastep),%edi
	subl	%edx,%ebx
	movl	C(d_aspancount),%ebp
	movl	%ebx,C(errorterm)
	addl	%edi,%ebp
	movl	%ebp,C(d_aspancount)
	jmp		LRGBRightEdgeStepped

LRGBNoTurnover:
	movl	C(d_aspancount),%edi
	movl	C(ubasestep),%edx
	movl	%ebx,C(errorterm)
	addl	%edx,%edi
	movl	%edi,C(d_aspancount)

LRGBRightEdgeStepped:
	cmpl	$1,%eax

	jl		LRGBNextSpan
	jz		LRGBExactlyOneLong

//
// set up advancetable
//
	movl	C(a_ststepxwhole),%ecx
	movl	C(d_affinetridesc)+atd_skinwidth,%edx

	movl	%ecx,advancetable+4	// advance base in t
	addl	%edx,%ecx

	movl	%ecx,advancetable	// advance extra in t
	movl	C(a_tstepxfrac),%ecx

	movw	C(r_rstepx),%cx
	movl	%eax,%edx			// count

	movl	%ecx,tstep
	addl	$7,%edx

	shrl	$3,%edx				// count of full and partial loops
	movl	spanpackage_t_sfrac(%esi),%ebx

	movw	%dx,%bx
	movl	spanpackage_t_pz(%esi),%ecx

	negl	%eax

	movl	spanpackage_t_pdest(%esi),%edi
	andl	$7,%eax		// 0->0, 1->7, 2->6, ... , 7->1

	subl	%eax,%edi	// compensate for hardwired offsets
	subl	%eax,%ecx

	subl	%eax,%edi

	movl	spanpackage_t_g(%esi),%edx
	movl	%edx,gb
	movl	C(r_bstepx),%edx
	shrl	$16,%edx
	movw	C(r_gstepx),%dx
	movl	%edx,gbstep

	subl	%eax,%ecx
	movl	spanpackage_t_tfrac(%esi),%edx

	movw	spanpackage_t_r(%esi),%dx
	movl	spanpackage_t_zi(%esi),%ebp

	rorl	$16,%ebp	// put high 16 bits of 1/z in low word
	pushl	%esi

	movl	spanpackage_t_ptex(%esi),%esi
	jmp		*LRGBentryvec_table(,%eax,4)

LRGBentryvec_table:
	.long	LRGBDraw8, LRGBDraw7, LRGBDraw6, LRGBDraw5
	.long	LRGBDraw4, LRGBDraw3, LRGBDraw2, LRGBDraw1

// %bx = count of full and partial loops
// %ebx high word = sfrac
// %ecx = pz
// %dx = light
// %edx high word = tfrac
// %esi = ptex
// %edi = pdest
// %ebp = 1/z
// tstep low word = C(r_rstepx)
// tstep high word = C(a_tstepxfrac)
// C(a_sstepxfrac) low word = 0
// C(a_sstepxfrac) high word = C(a_sstepxfrac)

LRGBDrawLoop:

// FIXME: do we need to clamp light? We may need at least a buffer bit to
// keep it from poking into tfrac and causing problems

LRGBDraw8:
	cmpw	(%ecx),%bp
	jl		LRGBp1
	xorl	%eax,%eax
	movb	%dh,%ah
	movb	(%esi),%al
	movw	%bp,(%ecx)
	movw	0x12345678(,%eax,2),%ax
LRPatch8:
	movw	%ax,(%edi)
	movb	gb+1,%ah
	movb	(%esi),%al
	movw	0x12345678(,%eax,2),%ax
LGPatch8:
	orw		%ax,(%edi)
	movb	gb+3,%ah
	movb	(%esi),%al
	movw	0x12345678(,%eax,2),%ax
LBPatch8:
	orw		%ax,(%edi)
LRGBp1:
	movl	gbstep,%eax
	addl	%eax,gb
	addl	tstep,%edx
	sbbl	%eax,%eax
	addl	lzistepx,%ebp
	adcl	$0,%ebp
	addl	C(a_sstepxfrac),%ebx
	adcl	advancetable+4(,%eax,4),%esi

LRGBDraw7:
	cmpw	2(%ecx),%bp
	jl		LRGBp2
	xorl	%eax,%eax
	movb	%dh,%ah
	movb	(%esi),%al
	movw	%bp,2(%ecx)
	movw	0x12345678(,%eax,2),%ax
LRPatch7:
	movw	%ax,2(%edi)
	movb	gb+1,%ah
	movb	(%esi),%al
	movw	0x12345678(,%eax,2),%ax
LGPatch7:
	orw		%ax,2(%edi)
	movb	gb+3,%ah
	movb	(%esi),%al
	movw	0x12345678(,%eax,2),%ax
LBPatch7:
	orw		%ax,2(%edi)
LRGBp2:
	movl	gbstep,%eax
	addl	%eax,gb
	addl	tstep,%edx
	sbbl	%eax,%eax
	addl	lzistepx,%ebp
	adcl	$0,%ebp
	addl	C(a_sstepxfrac),%ebx
	adcl	advancetable+4(,%eax,4),%esi

LRGBDraw6:
	cmpw	4(%ecx),%bp
	jl		LRGBp3
	xorl	%eax,%eax
	movb	%dh,%ah
	movb	(%esi),%al
	movw	%bp,4(%ecx)
	movw	0x12345678(,%eax,2),%ax
LRPatch6:
	movw	%ax,4(%edi)
	movb	gb+1,%ah
	movb	(%esi),%al
	movw	0x12345678(,%eax,2),%ax
LGPatch6:
	orw		%ax,4(%edi)
	movb	gb+3,%ah
	movb	(%esi),%al
	movw	0x12345678(,%eax,2),%ax
LBPatch6:
	orw		%ax,4(%edi)
LRGBp3:
	movl	gbstep,%eax
	addl	%eax,gb
	addl	tstep,%edx
	sbbl	%eax,%eax
	addl	lzistepx,%ebp
	adcl	$0,%ebp
	addl	C(a_sstepxfrac),%ebx
	adcl	advancetable+4(,%eax,4),%esi

LRGBDraw5:
	cmpw	6(%ecx),%bp
	jl		LRGBp4
	xorl	%eax,%eax
	movb	%dh,%ah
	movb	(%esi),%al
	movw	%bp,6(%ecx)
	movw	0x12345678(,%eax,2),%ax
LRPatch5:
	movw	%ax,6(%edi)
	movb	gb+1,%ah
	movb	(%esi),%al
	movw	0x12345678(,%eax,2),%ax
LGPatch5:
	orw		%ax,6(%edi)
	movb	gb+3,%ah
	movb	(%esi),%al
	movw	0x12345678(,%eax,2),%ax
LBPatch5:
	orw		%ax,6(%edi)
LRGBp4:
	movl	gbstep,%eax
	addl	%eax,gb
	addl	tstep,%edx
	sbbl	%eax,%eax
	addl	lzistepx,%ebp
	adcl	$0,%ebp
	addl	C(a_sstepxfrac),%ebx
	adcl	advancetable+4(,%eax,4),%esi

LRGBDraw4:
	cmpw	8(%ecx),%bp
	jl		LRGBp5
	xorl	%eax,%eax
	movb	%dh,%ah
	movb	(%esi),%al
	movw	%bp,8(%ecx)
	movw	0x12345678(,%eax,2),%ax
LRPatch4:
	movw	%ax,8(%edi)
	movb	gb+1,%ah
	movb	(%esi),%al
	movw	0x12345678(,%eax,2),%ax
LGPatch4:
	orw		%ax,8(%edi)
	movb	gb+3,%ah
	movb	(%esi),%al
	movw	0x12345678(,%eax,2),%ax
LBPatch4:
	orw		%ax,8(%edi)
LRGBp5:
	movl	gbstep,%eax
	addl	%eax,gb
	addl	tstep,%edx
	sbbl	%eax,%eax
	addl	lzistepx,%ebp
	adcl	$0,%ebp
	addl	C(a_sstepxfrac),%ebx
	adcl	advancetable+4(,%eax,4),%esi

LRGBDraw3:
	cmpw	10(%ecx),%bp
	jl		LRGBp6
	xorl	%eax,%eax
	movb	%dh,%ah
	movb	(%esi),%al
	movw	%bp,10(%ecx)
	movw	0x12345678(,%eax,2),%ax
LRPatch3:
	movw	%ax,10(%edi)
	movb	gb+1,%ah
	movb	(%esi),%al
	movw	0x12345678(,%eax,2),%ax
LGPatch3:
	orw		%ax,10(%edi)
	movb	gb+3,%ah
	movb	(%esi),%al
	movw	0x12345678(,%eax,2),%ax
LBPatch3:
	orw		%ax,10(%edi)
LRGBp6:
	movl	gbstep,%eax
	addl	%eax,gb
	addl	tstep,%edx
	sbbl	%eax,%eax
	addl	lzistepx,%ebp
	adcl	$0,%ebp
	addl	C(a_sstepxfrac),%ebx
	adcl	advancetable+4(,%eax,4),%esi

LRGBDraw2:
	cmpw	12(%ecx),%bp
	jl		LRGBp7
	xorl	%eax,%eax
	movb	%dh,%ah
	movb	(%esi),%al
	movw	%bp,12(%ecx)
	movw	0x12345678(,%eax,2),%ax
LRPatch2:
	movw	%ax,12(%edi)
	movb	gb+1,%ah
	movb	(%esi),%al
	movw	0x12345678(,%eax,2),%ax
LGPatch2:
	orw		%ax,12(%edi)
	movb	gb+3,%ah
	movb	(%esi),%al
	movw	0x12345678(,%eax,2),%ax
LBPatch2:
	orw		%ax,12(%edi)
LRGBp7:
	movl	gbstep,%eax
	addl	%eax,gb
	addl	tstep,%edx
	sbbl	%eax,%eax
	addl	lzistepx,%ebp
	adcl	$0,%ebp
	addl	C(a_sstepxfrac),%ebx
	adcl	advancetable+4(,%eax,4),%esi

LRGBDraw1:
	cmpw	14(%ecx),%bp
	jl		LRGBp8
	xorl	%eax,%eax
	movb	%dh,%ah
	movb	(%esi),%al
	movw	%bp,14(%ecx)
	movw	0x12345678(,%eax,2),%ax
LRPatch1:
	movw	%ax,14(%edi)
	movb	gb+1,%ah
	movb	(%esi),%al
	movw	0x12345678(,%eax,2),%ax
LGPatch1:
	orw		%ax,14(%edi)
	movb	gb+3,%ah
	movb	(%esi),%al
	movw	0x12345678(,%eax,2),%ax
LBPatch1:
	orw		%ax,14(%edi)
LRGBp8:
	movl	gbstep,%eax
	addl	%eax,gb
	addl	tstep,%edx
	sbbl	%eax,%eax
	addl	lzistepx,%ebp
	adcl	$0,%ebp
	addl	C(a_sstepxfrac),%ebx
	adcl	advancetable+4(,%eax,4),%esi

	addl	$16,%edi
	addl	$16,%ecx

	decw	%bx
	jnz		LRGBDrawLoop

	popl	%esi				// restore spans pointer
LRGBNextSpan:
	addl	$(spanpackage_t_size),%esi	// point to next span
LRGBNextSpanESISet:
	movswl	spanpackage_t_count(%esi),%edx
	cmpl	$DPS_SPAN_LIST_END,%edx		// any more spans?
	jnz		LRGBSpanLoop			// yes

	popl	%edi
	popl	%ebp				// restore the caller's stack frame
	popl	%ebx				// restore register variables
	popl	%esi
	ret


// draw a one-long span

LRGBExactlyOneLong:

	movl	spanpackage_t_pz(%esi),%ecx
	movl	spanpackage_t_zi(%esi),%ebp

	rorl	$16,%ebp	// put high 16 bits of 1/z in low word
	movl	spanpackage_t_ptex(%esi),%ebx

	cmpw	(%ecx),%bp
	jl		LRGBNextSpan
	xorl	%eax,%eax
	movl	spanpackage_t_pdest(%esi),%edi
	movb	spanpackage_t_r+1(%esi),%ah
	movb	(%ebx),%al
	movw	%bp,(%ecx)
	movw	0x12345678(,%eax,2),%ax
LRPatch9:
	movw	%ax,(%edi)
	movb	spanpackage_t_g+1(%esi),%ah
	movb	(%ebx),%al
	movw	0x12345678(,%eax,2),%ax
LGPatch9:
	orw		%ax,(%edi)
	movb	spanpackage_t_b+1(%esi),%ah
	addl	$(spanpackage_t_size),%esi	// point to next span
	movb	(%ebx),%al
	movw	0x12345678(,%eax,2),%ax
LBPatch9:
	orw		%ax,(%edi)

	jmp		LRGBNextSpanESISet


.globl C(D_PolysetAff16End)
C(D_PolysetAff16End):


.globl C(D_Aff16Patch)
C(D_Aff16Patch):
	movl	C(fadetable16),%eax
	movl	%eax,LPatch1-4
	movl	%eax,LPatch2-4
	movl	%eax,LPatch3-4
	movl	%eax,LPatch4-4
	movl	%eax,LPatch5-4
	movl	%eax,LPatch6-4
	movl	%eax,LPatch7-4
	movl	%eax,LPatch8-4
	movl	%eax,LPatch9-4

	movl	C(fadetable16r),%eax
	movl	%eax,LRPatch1-4
	movl	%eax,LRPatch2-4
	movl	%eax,LRPatch3-4
	movl	%eax,LRPatch4-4
	movl	%eax,LRPatch5-4
	movl	%eax,LRPatch6-4
	movl	%eax,LRPatch7-4
	movl	%eax,LRPatch8-4
	movl	%eax,LRPatch9-4

	movl	C(fadetable16g),%eax
	movl	%eax,LGPatch1-4
	movl	%eax,LGPatch2-4
	movl	%eax,LGPatch3-4
	movl	%eax,LGPatch4-4
	movl	%eax,LGPatch5-4
	movl	%eax,LGPatch6-4
	movl	%eax,LGPatch7-4
	movl	%eax,LGPatch8-4
	movl	%eax,LGPatch9-4

	movl	C(fadetable16b),%eax
	movl	%eax,LBPatch1-4
	movl	%eax,LBPatch2-4
	movl	%eax,LBPatch3-4
	movl	%eax,LBPatch4-4
	movl	%eax,LBPatch5-4
	movl	%eax,LBPatch6-4
	movl	%eax,LBPatch7-4
	movl	%eax,LBPatch8-4
	movl	%eax,LBPatch9-4

	ret

#endif

//**************************************************************************
//
//	$Log$
//	Revision 1.2  2002/01/07 12:16:41  dj_jl
//	Changed copyright year
//
//	Revision 1.1  2001/08/15 17:12:23  dj_jl
//	Optimized model drawing
//	
//**************************************************************************
