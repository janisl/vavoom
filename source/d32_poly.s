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

#include "asm_i386.h"

#ifdef USEASM

.data
Ltemp:	.long	0

	.text

	Align4

#define pspans	4+8

.globl C(D_PolysetAff32Start)
C(D_PolysetAff32Start):

//==========================================================================
//
//	D_PolysetDrawSpans_32
//
//	32-bpp horizontal span drawing code for affine polygons, with smooth
// shading and no transparency
//
//==========================================================================

.globl C(D_PolysetDrawSpans_32)
C(D_PolysetDrawSpans_32):
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
	subl	%eax,%edi
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
	movl	0x12345678(,%eax,4),%eax
LPatch8:
	movl	%eax,(%edi)
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
	movl	0x12345678(,%eax,4),%eax
LPatch7:
	movl	%eax,4(%edi)
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
	movl	0x12345678(,%eax,4),%eax
LPatch6:
	movl	%eax,8(%edi)
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
	movl	0x12345678(,%eax,4),%eax
LPatch5:
	movl	%eax,12(%edi)
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
	movl	0x12345678(,%eax,4),%eax
LPatch4:
	movl	%eax,16(%edi)
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
	movl	0x12345678(,%eax,4),%eax
LPatch3:
	movl	%eax,20(%edi)
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
	movl	0x12345678(,%eax,4),%eax
LPatch2:
	movl	%eax,24(%edi)
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
	movl	0x12345678(,%eax,4),%eax
LPatch1:
	movl	%eax,28(%edi)
Lp8:
	addl	tstep,%edx
	sbbl	%eax,%eax
	addl	lzistepx,%ebp
	adcl	$0,%ebp
	addl	C(a_sstepxfrac),%ebx
	adcl	advancetable+4(,%eax,4),%esi

	addl	$32,%edi
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
	movl	0x12345678(,%eax,4),%eax
LPatch9:
	movl	%eax,(%edi)

	jmp		LNextSpanESISet

//==========================================================================
//
//	D_PolysetDrawSpansFuzz_32
//
//	32-bpp horizontal span drawing code for affine polygons, with smooth
// shading and no transparency
//
//==========================================================================

.globl C(D_PolysetDrawSpansFuzz_32)
C(D_PolysetDrawSpansFuzz_32):
	pushl	%esi				// preserve register variables
	pushl	%ebx

	movl	pspans(%esp),%esi	// point to the first span descriptor
	movl	C(r_zistepx),%ecx

	pushl	%ebp				// preserve caller's stack frame
	pushl	%edi

	rorl	$16,%ecx			// put high 16 bits of 1/z step in low word
	movswl	spanpackage_t_count(%esi),%edx

	movl	%ecx,lzistepx

LFuzzSpanLoop:

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
	js		LFuzzNoTurnover

	movl	C(erroradjustdown),%edx
	movl	C(d_countextrastep),%edi
	subl	%edx,%ebx
	movl	C(d_aspancount),%ebp
	movl	%ebx,C(errorterm)
	addl	%edi,%ebp
	movl	%ebp,C(d_aspancount)
	jmp		LFuzzRightEdgeStepped

LFuzzNoTurnover:
	movl	C(d_aspancount),%edi
	movl	C(ubasestep),%edx
	movl	%ebx,C(errorterm)
	addl	%edx,%edi
	movl	%edi,C(d_aspancount)

LFuzzRightEdgeStepped:
	cmpl	$1,%eax

	jl		LFuzzNextSpan
	jz		LFuzzExactlyOneLong

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
	subl	%eax,%edi
	subl	%eax,%edi

	subl	%eax,%ecx
	movl	spanpackage_t_tfrac(%esi),%edx

	movw	spanpackage_t_r(%esi),%dx
	movl	spanpackage_t_zi(%esi),%ebp

	rorl	$16,%ebp	// put high 16 bits of 1/z in low word
	pushl	%esi

	movl	spanpackage_t_ptex(%esi),%esi
	jmp		*LFuzzentryvec_table(,%eax,4)

LFuzzentryvec_table:
	.long	LFuzzDraw8, LFuzzDraw7, LFuzzDraw6, LFuzzDraw5
	.long	LFuzzDraw4, LFuzzDraw3, LFuzzDraw2, LFuzzDraw1

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

LFuzzDrawLoop:

// FIXME: do we need to clamp light? We may need at least a buffer bit to
// keep it from poking into tfrac and causing problems

LFuzzDraw8:
	cmpw	(%ecx),%bp
	jl		LFuzzp1
	xorl	%eax,%eax
	movb	%dh,%ah
	movb	(%esi),%al
	movw	%bp,(%ecx)
	movl	0x12345678(,%eax,4),%eax
LFuzzPatch8:
	pushl	%ebx
	pushl	%ecx
	pushl	%edx
	pushl	%esi
	pushl	%eax
	movl	C(d_srctranstab),%esi
	movl	C(d_dsttranstab),%edx
	andl	$0xff,%eax
	xorl	%ecx,%ecx
	movb	(%edi),%cl
	movl	(%esi,%eax,2),%ebx
	movb	1(%esp),%al
	addl	(%edx,%ecx,2),%ebx
	movb	1(%edi),%cl
	movb	%bh,(%edi)
	movl	(%esi,%eax,2),%ebx
	movb	2(%esp),%al
	addl	(%edx,%ecx,2),%ebx
	movb	2(%edi),%cl
	movb	%bh,1(%edi)
	movl	(%esi,%eax,2),%ebx
	popl	%eax
	popl	%esi
	addl	(%edx,%ecx,2),%ebx
	popl	%edx
	popl	%ecx
	movb	%bh,2(%edi)
	popl	%ebx
LFuzzp1:
	addl	tstep,%edx
	sbbl	%eax,%eax
	addl	lzistepx,%ebp
	adcl	$0,%ebp
	addl	C(a_sstepxfrac),%ebx
	adcl	advancetable+4(,%eax,4),%esi

LFuzzDraw7:
	cmpw	2(%ecx),%bp
	jl		LFuzzp2
	xorl	%eax,%eax
	movb	%dh,%ah
	movb	(%esi),%al
	movw	%bp,2(%ecx)
	movl	0x12345678(,%eax,4),%eax
LFuzzPatch7:
	pushl	%ebx
	pushl	%ecx
	pushl	%edx
	pushl	%esi
	pushl	%eax
	movl	C(d_srctranstab),%esi
	movl	C(d_dsttranstab),%edx
	andl	$0xff,%eax
	xorl	%ecx,%ecx
	movb	4(%edi),%cl
	movl	(%esi,%eax,2),%ebx
	movb	1(%esp),%al
	addl	(%edx,%ecx,2),%ebx
	movb	5(%edi),%cl
	movb	%bh,4(%edi)
	movl	(%esi,%eax,2),%ebx
	movb	2(%esp),%al
	addl	(%edx,%ecx,2),%ebx
	movb	6(%edi),%cl
	movb	%bh,5(%edi)
	movl	(%esi,%eax,2),%ebx
	popl	%eax
	popl	%esi
	addl	(%edx,%ecx,2),%ebx
	popl	%edx
	popl	%ecx
	movb	%bh,6(%edi)
	popl	%ebx
LFuzzp2:
	addl	tstep,%edx
	sbbl	%eax,%eax
	addl	lzistepx,%ebp
	adcl	$0,%ebp
	addl	C(a_sstepxfrac),%ebx
	adcl	advancetable+4(,%eax,4),%esi

LFuzzDraw6:
	cmpw	4(%ecx),%bp
	jl		LFuzzp3
	xorl	%eax,%eax
	movb	%dh,%ah
	movb	(%esi),%al
	movw	%bp,4(%ecx)
	movl	0x12345678(,%eax,4),%eax
LFuzzPatch6:
	pushl	%ebx
	pushl	%ecx
	pushl	%edx
	pushl	%esi
	pushl	%eax
	movl	C(d_srctranstab),%esi
	movl	C(d_dsttranstab),%edx
	andl	$0xff,%eax
	xorl	%ecx,%ecx
	movb	8(%edi),%cl
	movl	(%esi,%eax,2),%ebx
	movb	1(%esp),%al
	addl	(%edx,%ecx,2),%ebx
	movb	9(%edi),%cl
	movb	%bh,8(%edi)
	movl	(%esi,%eax,2),%ebx
	movb	2(%esp),%al
	addl	(%edx,%ecx,2),%ebx
	movb	10(%edi),%cl
	movb	%bh,9(%edi)
	movl	(%esi,%eax,2),%ebx
	popl	%eax
	popl	%esi
	addl	(%edx,%ecx,2),%ebx
	popl	%edx
	popl	%ecx
	movb	%bh,10(%edi)
	popl	%ebx
LFuzzp3:
	addl	tstep,%edx
	sbbl	%eax,%eax
	addl	lzistepx,%ebp
	adcl	$0,%ebp
	addl	C(a_sstepxfrac),%ebx
	adcl	advancetable+4(,%eax,4),%esi

LFuzzDraw5:
	cmpw	6(%ecx),%bp
	jl		LFuzzp4
	xorl	%eax,%eax
	movb	%dh,%ah
	movb	(%esi),%al
	movw	%bp,6(%ecx)
	movl	0x12345678(,%eax,4),%eax
LFuzzPatch5:
	pushl	%ebx
	pushl	%ecx
	pushl	%edx
	pushl	%esi
	pushl	%eax
	movl	C(d_srctranstab),%esi
	movl	C(d_dsttranstab),%edx
	andl	$0xff,%eax
	xorl	%ecx,%ecx
	movb	12(%edi),%cl
	movl	(%esi,%eax,2),%ebx
	movb	1(%esp),%al
	addl	(%edx,%ecx,2),%ebx
	movb	13(%edi),%cl
	movb	%bh,12(%edi)
	movl	(%esi,%eax,2),%ebx
	movb	2(%esp),%al
	addl	(%edx,%ecx,2),%ebx
	movb	14(%edi),%cl
	movb	%bh,13(%edi)
	movl	(%esi,%eax,2),%ebx
	popl	%eax
	popl	%esi
	addl	(%edx,%ecx,2),%ebx
	popl	%edx
	popl	%ecx
	movb	%bh,14(%edi)
	popl	%ebx
LFuzzp4:
	addl	tstep,%edx
	sbbl	%eax,%eax
	addl	lzistepx,%ebp
	adcl	$0,%ebp
	addl	C(a_sstepxfrac),%ebx
	adcl	advancetable+4(,%eax,4),%esi

LFuzzDraw4:
	cmpw	8(%ecx),%bp
	jl		LFuzzp5
	xorl	%eax,%eax
	movb	%dh,%ah
	movb	(%esi),%al
	movw	%bp,8(%ecx)
	movl	0x12345678(,%eax,4),%eax
LFuzzPatch4:
	pushl	%ebx
	pushl	%ecx
	pushl	%edx
	pushl	%esi
	pushl	%eax
	movl	C(d_srctranstab),%esi
	movl	C(d_dsttranstab),%edx
	andl	$0xff,%eax
	xorl	%ecx,%ecx
	movb	16(%edi),%cl
	movl	(%esi,%eax,2),%ebx
	movb	1(%esp),%al
	addl	(%edx,%ecx,2),%ebx
	movb	17(%edi),%cl
	movb	%bh,16(%edi)
	movl	(%esi,%eax,2),%ebx
	movb	2(%esp),%al
	addl	(%edx,%ecx,2),%ebx
	movb	18(%edi),%cl
	movb	%bh,17(%edi)
	movl	(%esi,%eax,2),%ebx
	popl	%eax
	popl	%esi
	addl	(%edx,%ecx,2),%ebx
	popl	%edx
	popl	%ecx
	movb	%bh,18(%edi)
	popl	%ebx
LFuzzp5:
	addl	tstep,%edx
	sbbl	%eax,%eax
	addl	lzistepx,%ebp
	adcl	$0,%ebp
	addl	C(a_sstepxfrac),%ebx
	adcl	advancetable+4(,%eax,4),%esi

LFuzzDraw3:
	cmpw	10(%ecx),%bp
	jl		LFuzzp6
	xorl	%eax,%eax
	movb	%dh,%ah
	movb	(%esi),%al
	movw	%bp,10(%ecx)
	movl	0x12345678(,%eax,4),%eax
LFuzzPatch3:
	pushl	%ebx
	pushl	%ecx
	pushl	%edx
	pushl	%esi
	pushl	%eax
	movl	C(d_srctranstab),%esi
	movl	C(d_dsttranstab),%edx
	andl	$0xff,%eax
	xorl	%ecx,%ecx
	movb	20(%edi),%cl
	movl	(%esi,%eax,2),%ebx
	movb	1(%esp),%al
	addl	(%edx,%ecx,2),%ebx
	movb	21(%edi),%cl
	movb	%bh,20(%edi)
	movl	(%esi,%eax,2),%ebx
	movb	2(%esp),%al
	addl	(%edx,%ecx,2),%ebx
	movb	22(%edi),%cl
	movb	%bh,21(%edi)
	movl	(%esi,%eax,2),%ebx
	popl	%eax
	popl	%esi
	addl	(%edx,%ecx,2),%ebx
	popl	%edx
	popl	%ecx
	movb	%bh,22(%edi)
	popl	%ebx
LFuzzp6:
	addl	tstep,%edx
	sbbl	%eax,%eax
	addl	lzistepx,%ebp
	adcl	$0,%ebp
	addl	C(a_sstepxfrac),%ebx
	adcl	advancetable+4(,%eax,4),%esi

LFuzzDraw2:
	cmpw	12(%ecx),%bp
	jl		LFuzzp7
	xorl	%eax,%eax
	movb	%dh,%ah
	movb	(%esi),%al
	movw	%bp,12(%ecx)
	movl	0x12345678(,%eax,4),%eax
LFuzzPatch2:
	pushl	%ebx
	pushl	%ecx
	pushl	%edx
	pushl	%esi
	pushl	%eax
	movl	C(d_srctranstab),%esi
	movl	C(d_dsttranstab),%edx
	andl	$0xff,%eax
	xorl	%ecx,%ecx
	movb	24(%edi),%cl
	movl	(%esi,%eax,2),%ebx
	movb	1(%esp),%al
	addl	(%edx,%ecx,2),%ebx
	movb	25(%edi),%cl
	movb	%bh,24(%edi)
	movl	(%esi,%eax,2),%ebx
	movb	2(%esp),%al
	addl	(%edx,%ecx,2),%ebx
	movb	26(%edi),%cl
	movb	%bh,25(%edi)
	movl	(%esi,%eax,2),%ebx
	popl	%eax
	popl	%esi
	addl	(%edx,%ecx,2),%ebx
	popl	%edx
	popl	%ecx
	movb	%bh,26(%edi)
	popl	%ebx
LFuzzp7:
	addl	tstep,%edx
	sbbl	%eax,%eax
	addl	lzistepx,%ebp
	adcl	$0,%ebp
	addl	C(a_sstepxfrac),%ebx
	adcl	advancetable+4(,%eax,4),%esi

LFuzzDraw1:
	cmpw	14(%ecx),%bp
	jl		LFuzzp8
	xorl	%eax,%eax
	movb	%dh,%ah
	movb	(%esi),%al
	movw	%bp,14(%ecx)
	movl	0x12345678(,%eax,4),%eax
LFuzzPatch1:
	pushl	%ebx
	pushl	%ecx
	pushl	%edx
	pushl	%esi
	pushl	%eax
	movl	C(d_srctranstab),%esi
	movl	C(d_dsttranstab),%edx
	andl	$0xff,%eax
	xorl	%ecx,%ecx
	movb	28(%edi),%cl
	movl	(%esi,%eax,2),%ebx
	movb	1(%esp),%al
	addl	(%edx,%ecx,2),%ebx
	movb	29(%edi),%cl
	movb	%bh,28(%edi)
	movl	(%esi,%eax,2),%ebx
	movb	2(%esp),%al
	addl	(%edx,%ecx,2),%ebx
	movb	30(%edi),%cl
	movb	%bh,29(%edi)
	movl	(%esi,%eax,2),%ebx
	popl	%eax
	popl	%esi
	addl	(%edx,%ecx,2),%ebx
	popl	%edx
	popl	%ecx
	movb	%bh,30(%edi)
	popl	%ebx
LFuzzp8:
	addl	tstep,%edx
	sbbl	%eax,%eax
	addl	lzistepx,%ebp
	adcl	$0,%ebp
	addl	C(a_sstepxfrac),%ebx
	adcl	advancetable+4(,%eax,4),%esi

	addl	$32,%edi
	addl	$16,%ecx

	decw	%bx
	jnz		LFuzzDrawLoop

	popl	%esi				// restore spans pointer
LFuzzNextSpan:
	addl	$(spanpackage_t_size),%esi	// point to next span
LFuzzNextSpanESISet:
	movswl	spanpackage_t_count(%esi),%edx
	cmpl	$DPS_SPAN_LIST_END,%edx		// any more spans?
	jnz		LFuzzSpanLoop			// yes

	popl	%edi
	popl	%ebp				// restore the caller's stack frame
	popl	%ebx				// restore register variables
	popl	%esi
	ret


// draw a one-long span

LFuzzExactlyOneLong:

	movl	spanpackage_t_pz(%esi),%ecx
	movl	spanpackage_t_zi(%esi),%ebp

	rorl	$16,%ebp	// put high 16 bits of 1/z in low word
	movl	spanpackage_t_ptex(%esi),%ebx

	cmpw	(%ecx),%bp
	jl		LFuzzNextSpan
	xorl	%eax,%eax
	movl	spanpackage_t_pdest(%esi),%edi
	movb	spanpackage_t_r+1(%esi),%ah
	addl	$(spanpackage_t_size),%esi	// point to next span
	movb	(%ebx),%al
	movw	%bp,(%ecx)
	movl	0x12345678(,%eax,4),%eax
LFuzzPatch9:
	pushl	%ebx
	pushl	%ecx
	pushl	%edx
	pushl	%esi
	pushl	%eax
	movl	C(d_srctranstab),%esi
	movl	C(d_dsttranstab),%edx
	andl	$0xff,%eax
	xorl	%ecx,%ecx
	movb	(%edi),%cl
	movl	(%esi,%eax,2),%ebx
	movb	1(%esp),%al
	addl	(%edx,%ecx,2),%ebx
	movb	1(%edi),%cl
	movb	%bh,(%edi)
	movl	(%esi,%eax,2),%ebx
	movb	2(%esp),%al
	addl	(%edx,%ecx,2),%ebx
	movb	2(%edi),%cl
	movb	%bh,1(%edi)
	movl	(%esi,%eax,2),%ebx
	popl	%eax
	popl	%esi
	addl	(%edx,%ecx,2),%ebx
	popl	%edx
	popl	%ecx
	movb	%bh,2(%edi)
	popl	%ebx

	jmp		LFuzzNextSpanESISet

//==========================================================================
//
//	D_PolysetDrawSpansRGB_32
//
//	32-bpp horizontal span drawing code for affine polygons, with smooth
// shading and no transparency
//
//==========================================================================

.globl C(D_PolysetDrawSpansRGB_32)
C(D_PolysetDrawSpansRGB_32):
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
	subl	%eax,%edi
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
	movb	0x12345678(%eax),%al
LRPatch8:
	movb	%al,31(%edi)
LROffsPatch8:
	movb	gb+1,%ah
	movb	(%esi),%al
	movb	0x12345678(%eax),%al
LGPatch8:
	movb	%al,31(%edi)
LGOffsPatch8:
	movb	gb+3,%ah
	movb	(%esi),%al
	movb	0x12345678(%eax),%al
LBPatch8:
	movb	%al,31(%edi)
LBOffsPatch8:
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
	movb	0x12345678(%eax),%al
LRPatch7:
	movb	%al,31(%edi)
LROffsPatch7:
	movb	gb+1,%ah
	movb	(%esi),%al
	movb	0x12345678(%eax),%al
LGPatch7:
	movb	%al,31(%edi)
LGOffsPatch7:
	movb	gb+3,%ah
	movb	(%esi),%al
	movb	0x12345678(%eax),%al
LBPatch7:
	movb	%al,31(%edi)
LBOffsPatch7:
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
	movb	0x12345678(%eax),%al
LRPatch6:
	movb	%al,31(%edi)
LROffsPatch6:
	movb	gb+1,%ah
	movb	(%esi),%al
	movb	0x12345678(%eax),%al
LGPatch6:
	movb	%al,31(%edi)
LGOffsPatch6:
	movb	gb+3,%ah
	movb	(%esi),%al
	movb	0x12345678(%eax),%al
LBPatch6:
	movb	%al,31(%edi)
LBOffsPatch6:
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
	movb	0x12345678(%eax),%al
LRPatch5:
	movb	%al,31(%edi)
LROffsPatch5:
	movb	gb+1,%ah
	movb	(%esi),%al
	movb	0x12345678(%eax),%al
LGPatch5:
	movb	%al,31(%edi)
LGOffsPatch5:
	movb	gb+3,%ah
	movb	(%esi),%al
	movb	0x12345678(%eax),%al
LBPatch5:
	movb	%al,31(%edi)
LBOffsPatch5:
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
	movb	0x12345678(%eax),%al
LRPatch4:
	movb	%al,31(%edi)
LROffsPatch4:
	movb	gb+1,%ah
	movb	(%esi),%al
	movb	0x12345678(%eax),%al
LGPatch4:
	movb	%al,31(%edi)
LGOffsPatch4:
	movb	gb+3,%ah
	movb	(%esi),%al
	movb	0x12345678(%eax),%al
LBPatch4:
	movb	%al,31(%edi)
LBOffsPatch4:
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
	movb	0x12345678(%eax),%al
LRPatch3:
	movb	%al,31(%edi)
LROffsPatch3:
	movb	gb+1,%ah
	movb	(%esi),%al
	movb	0x12345678(%eax),%al
LGPatch3:
	movb	%al,31(%edi)
LGOffsPatch3:
	movb	gb+3,%ah
	movb	(%esi),%al
	movb	0x12345678(%eax),%al
LBPatch3:
	movb	%al,31(%edi)
LBOffsPatch3:
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
	movb	0x12345678(%eax),%al
LRPatch2:
	movb	%al,31(%edi)
LROffsPatch2:
	movb	gb+1,%ah
	movb	(%esi),%al
	movb	0x12345678(%eax),%al
LGPatch2:
	movb	%al,31(%edi)
LGOffsPatch2:
	movb	gb+3,%ah
	movb	(%esi),%al
	movb	0x12345678(%eax),%al
LBPatch2:
	movb	%al,31(%edi)
LBOffsPatch2:
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
	movb	0x12345678(%eax),%al
LRPatch1:
	movb	%al,31(%edi)
LROffsPatch1:
	movb	gb+1,%ah
	movb	(%esi),%al
	movb	0x12345678(%eax),%al
LGPatch1:
	movb	%al,31(%edi)
LGOffsPatch1:
	movb	gb+3,%ah
	movb	(%esi),%al
	movb	0x12345678(%eax),%al
LBPatch1:
	movb	%al,31(%edi)
LBOffsPatch1:
LRGBp8:
	movl	gbstep,%eax
	addl	%eax,gb
	addl	tstep,%edx
	sbbl	%eax,%eax
	addl	lzistepx,%ebp
	adcl	$0,%ebp
	addl	C(a_sstepxfrac),%ebx
	adcl	advancetable+4(,%eax,4),%esi

	addl	$32,%edi
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
	movb	0x12345678(%eax),%al
LRPatch9:
	movb	%al,31(%edi)
LROffsPatch9:
	movb	spanpackage_t_g+1(%esi),%ah
	movb	(%ebx),%al
	movb	0x12345678(%eax),%al
LGPatch9:
	movb	%al,31(%edi)
LGOffsPatch9:
	movb	spanpackage_t_b+1(%esi),%ah
	movb	(%ebx),%al
	addl	$(spanpackage_t_size),%esi	// point to next span
	movb	0x12345678(%eax),%al
LBPatch9:
	movb	%al,31(%edi)
LBOffsPatch9:

	jmp		LRGBNextSpanESISet

//==========================================================================
//
//	D_PolysetDrawSpansRGBFuzz_32
//
//	32-bpp horizontal span drawing code for affine polygons, with smooth
// shading and no transparency
//
//==========================================================================

.globl C(D_PolysetDrawSpansRGBFuzz_32)
C(D_PolysetDrawSpansRGBFuzz_32):
	pushl	%esi				// preserve register variables
	pushl	%ebx

	movl	pspans(%esp),%esi	// point to the first span descriptor
	movl	C(r_zistepx),%ecx

	pushl	%ebp				// preserve caller's stack frame
	pushl	%edi

	rorl	$16,%ecx			// put high 16 bits of 1/z step in low word
	movswl	spanpackage_t_count(%esi),%edx

	movl	%ecx,lzistepx

LRGBFuzzSpanLoop:

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
	js		LRGBFuzzNoTurnover

	movl	C(erroradjustdown),%edx
	movl	C(d_countextrastep),%edi
	subl	%edx,%ebx
	movl	C(d_aspancount),%ebp
	movl	%ebx,C(errorterm)
	addl	%edi,%ebp
	movl	%ebp,C(d_aspancount)
	jmp		LRGBFuzzRightEdgeStepped

LRGBFuzzNoTurnover:
	movl	C(d_aspancount),%edi
	movl	C(ubasestep),%edx
	movl	%ebx,C(errorterm)
	addl	%edx,%edi
	movl	%edi,C(d_aspancount)

LRGBFuzzRightEdgeStepped:
	cmpl	$1,%eax

	jl		LRGBFuzzNextSpan
	jz		LRGBFuzzExactlyOneLong

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

	movl	spanpackage_t_sfrac(%esi),%ebx

	movw	%dx,%bx
	movl	spanpackage_t_pz(%esi),%ecx

	movl	spanpackage_t_pdest(%esi),%edi

	movl	spanpackage_t_g(%esi),%edx
	movl	%edx,gb
	movl	C(r_bstepx),%edx
	shrl	$16,%edx
	movw	C(r_gstepx),%dx
	movl	%edx,gbstep

	movl	spanpackage_t_tfrac(%esi),%edx

	movw	spanpackage_t_r(%esi),%dx
	movl	spanpackage_t_zi(%esi),%ebp

	rorl	$16,%ebp	// put high 16 bits of 1/z in low word
	pushl	%esi

	movl	spanpackage_t_ptex(%esi),%esi

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

LRGBFuzzDrawLoop:

// FIXME: do we need to clamp light? We may need at least a buffer bit to
// keep it from poking into tfrac and causing problems

LRGBFuzzDraw1:
	cmpw	14(%ecx),%bp
	jl		LRGBFuzzp1
	xorl	%eax,%eax
	movb	%dh,%ah
	movb	(%esi),%al
	movw	%bp,14(%ecx)
	movb	0x12345678(%eax),%al
LRFuzzPatch1:
	movb	%al,Ltemp
	movb	gb+1,%ah
	movb	(%esi),%al
	movb	0x12345678(%eax),%al
LGFuzzPatch1:
	movb	%al,Ltemp+1
	movb	gb+3,%ah
	movb	(%esi),%al
	movb	0x12345678(%eax),%al
LBFuzzPatch1:
	movb	%al,Ltemp+2
	xorl	%eax,%eax
	pushl	%ebx
	pushl	%ecx
	pushl	%edx
	pushl	%esi
	movl	C(d_srctranstab),%esi
	movl	C(d_dsttranstab),%edx
	movb	Ltemp,%al
	xorl	%ecx,%ecx
	movb	31(%edi),%cl
LRFuzzOffsPatch1:
	movl	(%esi,%eax,2),%ebx
	movb	Ltemp+1,%al
	addl	(%edx,%ecx,2),%ebx
	movb	31(%edi),%cl
LGFuzzOffsPatch1:
	movb	%bh,31(%edi)
LRFuzzOffsPatch2:
	movl	(%esi,%eax,2),%ebx
	movb	Ltemp+2,%al
	addl	(%edx,%ecx,2),%ebx
	movb	31(%edi),%cl
LBFuzzOffsPatch1:
	movb	%bh,31(%edi)
LGFuzzOffsPatch2:
	movl	(%esi,%eax,2),%ebx
	popl	%esi
	addl	(%edx,%ecx,2),%ebx
	popl	%edx
	popl	%ecx
	movb	%bh,31(%edi)
LBFuzzOffsPatch2:
	popl	%ebx
LRGBFuzzp1:
	movl	gbstep,%eax
	addl	%eax,gb
	addl	tstep,%edx
	sbbl	%eax,%eax
	addl	lzistepx,%ebp
	adcl	$0,%ebp
	addl	C(a_sstepxfrac),%ebx
	adcl	advancetable+4(,%eax,4),%esi

	addl	$4,%edi
	addl	$2,%ecx

	decw	%bx
	jnz		LRGBFuzzDrawLoop

	popl	%esi				// restore spans pointer
LRGBFuzzNextSpan:
	addl	$(spanpackage_t_size),%esi	// point to next span
LRGBFuzzNextSpanESISet:
	movswl	spanpackage_t_count(%esi),%edx
	cmpl	$DPS_SPAN_LIST_END,%edx		// any more spans?
	jnz		LRGBFuzzSpanLoop			// yes

	popl	%edi
	popl	%ebp				// restore the caller's stack frame
	popl	%ebx				// restore register variables
	popl	%esi
	ret


// draw a one-long span

LRGBFuzzExactlyOneLong:

	movl	spanpackage_t_pz(%esi),%ecx
	movl	spanpackage_t_zi(%esi),%ebp

	rorl	$16,%ebp	// put high 16 bits of 1/z in low word
	movl	spanpackage_t_ptex(%esi),%ebx

	cmpw	(%ecx),%bp
	jl		LRGBFuzzNextSpan
	xorl	%eax,%eax
	movl	spanpackage_t_pdest(%esi),%edi
	movb	spanpackage_t_r+1(%esi),%ah
	movb	(%ebx),%al
	movw	%bp,(%ecx)
	movb	0x12345678(%eax),%al
LRFuzzPatch2:
	movb	%al,Ltemp
	movb	spanpackage_t_g+1(%esi),%ah
	movb	(%ebx),%al
	movb	0x12345678(%eax),%al
LGFuzzPatch2:
	movb	%al,Ltemp+1
	movb	spanpackage_t_b+1(%esi),%ah
	movb	(%ebx),%al
	addl	$(spanpackage_t_size),%esi	// point to next span
	movb	0x12345678(%eax),%al
LBFuzzPatch2:
	movb	%al,Ltemp+2
	xorl	%eax,%eax
	pushl	%ebx
	pushl	%ecx
	pushl	%edx
	pushl	%esi
	movl	C(d_srctranstab),%esi
	movl	C(d_dsttranstab),%edx
	movb	Ltemp,%al
	xorl	%ecx,%ecx
	movb	31(%edi),%cl
LRFuzzOffsPatch3:
	movl	(%esi,%eax,2),%ebx
	movb	Ltemp+1,%al
	addl	(%edx,%ecx,2),%ebx
	movb	31(%edi),%cl
LGFuzzOffsPatch3:
	movb	%bh,31(%edi)
LRFuzzOffsPatch4:
	movl	(%esi,%eax,2),%ebx
	movb	Ltemp+2,%al
	addl	(%edx,%ecx,2),%ebx
	movb	31(%edi),%cl
LBFuzzOffsPatch3:
	movb	%bh,31(%edi)
LGFuzzOffsPatch4:
	movl	(%esi,%eax,2),%ebx
	popl	%esi
	addl	(%edx,%ecx,2),%ebx
	popl	%edx
	popl	%ecx
	movb	%bh,31(%edi)
LBFuzzOffsPatch4:
	popl	%ebx

	jmp		LRGBFuzzNextSpanESISet


.globl C(D_PolysetAff32End)
C(D_PolysetAff32End):

LROffsPatchTable:
	.long	LROffsPatch1-1, 28
	.long	LROffsPatch2-1, 24
	.long	LROffsPatch3-1, 20
	.long	LROffsPatch4-1, 16
	.long	LROffsPatch5-1, 12
	.long	LROffsPatch6-1, 8
	.long	LROffsPatch7-1, 4
	.long	LROffsPatch8-1, 0
	.long	LROffsPatch9-1, 0
	.long	LRFuzzOffsPatch1-1, 0
	.long	LRFuzzOffsPatch2-1, 0
	.long	LRFuzzOffsPatch3-1, 0
	.long	LRFuzzOffsPatch4-1, 0

LGOffsPatchTable:
	.long	LGOffsPatch1-1, 28
	.long	LGOffsPatch2-1, 24
	.long	LGOffsPatch3-1, 20
	.long	LGOffsPatch4-1, 16
	.long	LGOffsPatch5-1, 12
	.long	LGOffsPatch6-1, 8
	.long	LGOffsPatch7-1, 4
	.long	LGOffsPatch8-1, 0
	.long	LGOffsPatch9-1, 0
	.long	LGFuzzOffsPatch1-1, 0
	.long	LGFuzzOffsPatch2-1, 0
	.long	LGFuzzOffsPatch3-1, 0
	.long	LGFuzzOffsPatch4-1, 0

LBOffsPatchTable:
	.long	LBOffsPatch1-1, 28
	.long	LBOffsPatch2-1, 24
	.long	LBOffsPatch3-1, 20
	.long	LBOffsPatch4-1, 16
	.long	LBOffsPatch5-1, 12
	.long	LBOffsPatch6-1, 8
	.long	LBOffsPatch7-1, 4
	.long	LBOffsPatch8-1, 0
	.long	LBOffsPatch9-1, 0
	.long	LBFuzzOffsPatch1-1, 0
	.long	LBFuzzOffsPatch2-1, 0
	.long	LBFuzzOffsPatch3-1, 0
	.long	LBFuzzOffsPatch4-1, 0

.globl C(D_Aff32Patch)
C(D_Aff32Patch):
	movl	C(fadetable32),%eax
	movl	%eax,LPatch1-4
	movl	%eax,LPatch2-4
	movl	%eax,LPatch3-4
	movl	%eax,LPatch4-4
	movl	%eax,LPatch5-4
	movl	%eax,LPatch6-4
	movl	%eax,LPatch7-4
	movl	%eax,LPatch8-4
	movl	%eax,LPatch9-4

	movl	%eax,LFuzzPatch1-4
	movl	%eax,LFuzzPatch2-4
	movl	%eax,LFuzzPatch3-4
	movl	%eax,LFuzzPatch4-4
	movl	%eax,LFuzzPatch5-4
	movl	%eax,LFuzzPatch6-4
	movl	%eax,LFuzzPatch7-4
	movl	%eax,LFuzzPatch8-4
	movl	%eax,LFuzzPatch9-4

	movl	C(fadetable32r),%eax
	movl	%eax,LRPatch1-4
	movl	%eax,LRPatch2-4
	movl	%eax,LRPatch3-4
	movl	%eax,LRPatch4-4
	movl	%eax,LRPatch5-4
	movl	%eax,LRPatch6-4
	movl	%eax,LRPatch7-4
	movl	%eax,LRPatch8-4
	movl	%eax,LRPatch9-4

	movl	%eax,LRFuzzPatch1-4
	movl	%eax,LRFuzzPatch2-4

	movl	C(fadetable32g),%eax
	movl	%eax,LGPatch1-4
	movl	%eax,LGPatch2-4
	movl	%eax,LGPatch3-4
	movl	%eax,LGPatch4-4
	movl	%eax,LGPatch5-4
	movl	%eax,LGPatch6-4
	movl	%eax,LGPatch7-4
	movl	%eax,LGPatch8-4
	movl	%eax,LGPatch9-4

	movl	%eax,LGFuzzPatch1-4
	movl	%eax,LGFuzzPatch2-4

	movl	C(fadetable32b),%eax
	movl	%eax,LBPatch1-4
	movl	%eax,LBPatch2-4
	movl	%eax,LBPatch3-4
	movl	%eax,LBPatch4-4
	movl	%eax,LBPatch5-4
	movl	%eax,LBPatch6-4
	movl	%eax,LBPatch7-4
	movl	%eax,LBPatch8-4
	movl	%eax,LBPatch9-4

	movl	%eax,LBFuzzPatch1-4
	movl	%eax,LBFuzzPatch2-4

	pushl	%ebx

	movl	$LROffsPatchTable,%ebx
	movl	$13,%ecx
LROffsPatchLoop:
	movl	(%ebx),%edx
	movl	C(roffs),%eax
	addl	4(%ebx),%eax
	addl	$8,%ebx
	movb	%al,(%edx)
	decl	%ecx
	jnz		LROffsPatchLoop

	movl	$LGOffsPatchTable,%ebx
	movl	$13,%ecx
LGOffsPatchLoop:
	movl	(%ebx),%edx
	movl	C(goffs),%eax
	addl	4(%ebx),%eax
	addl	$8,%ebx
	movb	%al,(%edx)
	decl	%ecx
	jnz		LGOffsPatchLoop

	movl	$LBOffsPatchTable,%ebx
	movl	$13,%ecx
LBOffsPatchLoop:
	movl	(%ebx),%edx
	movl	C(boffs),%eax
	addl	4(%ebx),%eax
	addl	$8,%ebx
	movb	%al,(%edx)
	decl	%ecx
	jnz		LBOffsPatchLoop

	popl	%ebx

	ret

#endif

//**************************************************************************
//
//	$Log$
//	Revision 1.1  2001/08/15 17:12:23  dj_jl
//	Optimized model drawing
//
//**************************************************************************
