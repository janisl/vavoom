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

	.data

Ltemp:	.long	0

	.text

#define pspans	4+8

	Align4
.globl C(D_PolysetAff8Start)
C(D_PolysetAff8Start):

//==========================================================================
//
//	D_PolysetDrawSpans_8
//
//	8-bpp horizontal span drawing code for affine polygons, with smooth
// shading and no transparency
//
//==========================================================================

.globl C(D_PolysetDrawSpans_8)
C(D_PolysetDrawSpans_8):
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
	movb	0x12345678(%eax),%al
LPatch8:
	movb	%al,(%edi)
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
	movb	0x12345678(%eax),%al
LPatch7:
	movb	%al,1(%edi)
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
	movb	0x12345678(%eax),%al
LPatch6:
	movb	%al,2(%edi)
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
	movb	0x12345678(%eax),%al
LPatch5:
	movb	%al,3(%edi)
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
	movb	0x12345678(%eax),%al
LPatch4:
	movb	%al,4(%edi)
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
	movb	0x12345678(%eax),%al
LPatch3:
	movb	%al,5(%edi)
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
	movb	0x12345678(%eax),%al
LPatch2:
	movb	%al,6(%edi)
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
	movb	0x12345678(%eax),%al
LPatch1:
	movb	%al,7(%edi)
Lp8:
	addl	tstep,%edx
	sbbl	%eax,%eax
	addl	lzistepx,%ebp
	adcl	$0,%ebp
	addl	C(a_sstepxfrac),%ebx
	adcl	advancetable+4(,%eax,4),%esi

	addl	$8,%edi
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
	movb	0x12345678(%eax),%al
LPatch9:
	movb	%al,(%edi)

	jmp		LNextSpanESISet

//==========================================================================
//
//	D_PolysetDrawSpansFuzz_8
//
//	8-bpp horizontal span drawing code for affine polygons, with smooth
// shading and no transparency
//
//==========================================================================

.globl C(D_PolysetDrawSpansFuzz_8)
C(D_PolysetDrawSpansFuzz_8):
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
	movb	0x12345678(%eax),%ah
LFuzzPatch8:
	movb	(%edi),%al
	addl	C(d_transluc),%eax
	movb	(%eax),%al
	movb	%al,(%edi)
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
	movb	0x12345678(%eax),%ah
LFuzzPatch7:
	movb	1(%edi),%al
	addl	C(d_transluc),%eax
	movb	(%eax),%al
	movb	%al,1(%edi)
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
	movb	0x12345678(%eax),%ah
LFuzzPatch6:
	movb	2(%edi),%al
	addl	C(d_transluc),%eax
	movb	(%eax),%al
	movb	%al,2(%edi)
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
	movb	0x12345678(%eax),%ah
LFuzzPatch5:
	movb	3(%edi),%al
	addl	C(d_transluc),%eax
	movb	(%eax),%al
	movb	%al,3(%edi)
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
	movb	0x12345678(%eax),%ah
LFuzzPatch4:
	movb	4(%edi),%al
	addl	C(d_transluc),%eax
	movb	(%eax),%al
	movb	%al,4(%edi)
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
	movb	0x12345678(%eax),%ah
LFuzzPatch3:
	movb	5(%edi),%al
	addl	C(d_transluc),%eax
	movb	(%eax),%al
	movb	%al,5(%edi)
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
	movb	0x12345678(%eax),%ah
LFuzzPatch2:
	movb	6(%edi),%al
	addl	C(d_transluc),%eax
	movb	(%eax),%al
	movb	%al,6(%edi)
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
	movb	0x12345678(%eax),%ah
LFuzzPatch1:
	movb	7(%edi),%al
	addl	C(d_transluc),%eax
	movb	(%eax),%al
	movb	%al,7(%edi)
LFuzzp8:
	addl	tstep,%edx
	sbbl	%eax,%eax
	addl	lzistepx,%ebp
	adcl	$0,%ebp
	addl	C(a_sstepxfrac),%ebx
	adcl	advancetable+4(,%eax,4),%esi

	addl	$8,%edi
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
	movb	0x12345678(%eax),%ah
LFuzzPatch9:
	movb	(%edi),%al
	addl	C(d_transluc),%eax
	movb	(%eax),%al
	movb	%al,(%edi)

	jmp		LFuzzNextSpanESISet

//==========================================================================
//
//	D_PolysetDrawSpansAltFuzz_8
//
//	8-bpp horizontal span drawing code for affine polygons, with smooth
// shading and no transparency
//
//==========================================================================

.globl C(D_PolysetDrawSpansAltFuzz_8)
C(D_PolysetDrawSpansAltFuzz_8):
	pushl	%esi				// preserve register variables
	pushl	%ebx

	movl	pspans(%esp),%esi	// point to the first span descriptor
	movl	C(r_zistepx),%ecx

	pushl	%ebp				// preserve caller's stack frame
	pushl	%edi

	rorl	$16,%ecx			// put high 16 bits of 1/z step in low word
	movswl	spanpackage_t_count(%esi),%edx

	movl	%ecx,lzistepx

LAltFuzzSpanLoop:

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
	js		LAltFuzzNoTurnover

	movl	C(erroradjustdown),%edx
	movl	C(d_countextrastep),%edi
	subl	%edx,%ebx
	movl	C(d_aspancount),%ebp
	movl	%ebx,C(errorterm)
	addl	%edi,%ebp
	movl	%ebp,C(d_aspancount)
	jmp		LAltFuzzRightEdgeStepped

LAltFuzzNoTurnover:
	movl	C(d_aspancount),%edi
	movl	C(ubasestep),%edx
	movl	%ebx,C(errorterm)
	addl	%edx,%edi
	movl	%edi,C(d_aspancount)

LAltFuzzRightEdgeStepped:
	cmpl	$1,%eax

	jl		LAltFuzzNextSpan
	jz		LAltFuzzExactlyOneLong

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

	subl	%eax,%ecx
	movl	spanpackage_t_tfrac(%esi),%edx

	movw	spanpackage_t_r(%esi),%dx
	movl	spanpackage_t_zi(%esi),%ebp

	rorl	$16,%ebp	// put high 16 bits of 1/z in low word
	pushl	%esi

	movl	spanpackage_t_ptex(%esi),%esi
	jmp		*LAltFuzzentryvec_table(,%eax,4)

LAltFuzzentryvec_table:
	.long	LAltFuzzDraw8, LAltFuzzDraw7, LAltFuzzDraw6, LAltFuzzDraw5
	.long	LAltFuzzDraw4, LAltFuzzDraw3, LAltFuzzDraw2, LAltFuzzDraw1

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

LAltFuzzDrawLoop:

// FIXME: do we need to clamp light? We may need at least a buffer bit to
// keep it from poking into tfrac and causing problems

LAltFuzzDraw8:
	cmpw	(%ecx),%bp
	jl		LAltFuzzp1
	xorl	%eax,%eax
	movb	%dh,%ah
	movb	(%esi),%al
	movw	%bp,(%ecx)
	movb	0x12345678(%eax),%al
LAltFuzzPatch8:
	movb	(%edi),%ah
	addl	C(d_transluc),%eax
	movb	(%eax),%al
	movb	%al,(%edi)
LAltFuzzp1:
	addl	tstep,%edx
	sbbl	%eax,%eax
	addl	lzistepx,%ebp
	adcl	$0,%ebp
	addl	C(a_sstepxfrac),%ebx
	adcl	advancetable+4(,%eax,4),%esi

LAltFuzzDraw7:
	cmpw	2(%ecx),%bp
	jl		LAltFuzzp2
	xorl	%eax,%eax
	movb	%dh,%ah
	movb	(%esi),%al
	movw	%bp,2(%ecx)
	movb	0x12345678(%eax),%al
LAltFuzzPatch7:
	movb	1(%edi),%ah
	addl	C(d_transluc),%eax
	movb	(%eax),%al
	movb	%al,1(%edi)
LAltFuzzp2:
	addl	tstep,%edx
	sbbl	%eax,%eax
	addl	lzistepx,%ebp
	adcl	$0,%ebp
	addl	C(a_sstepxfrac),%ebx
	adcl	advancetable+4(,%eax,4),%esi

LAltFuzzDraw6:
	cmpw	4(%ecx),%bp
	jl		LAltFuzzp3
	xorl	%eax,%eax
	movb	%dh,%ah
	movb	(%esi),%al
	movw	%bp,4(%ecx)
	movb	0x12345678(%eax),%al
LAltFuzzPatch6:
	movb	2(%edi),%ah
	addl	C(d_transluc),%eax
	movb	(%eax),%al
	movb	%al,2(%edi)
LAltFuzzp3:
	addl	tstep,%edx
	sbbl	%eax,%eax
	addl	lzistepx,%ebp
	adcl	$0,%ebp
	addl	C(a_sstepxfrac),%ebx
	adcl	advancetable+4(,%eax,4),%esi

LAltFuzzDraw5:
	cmpw	6(%ecx),%bp
	jl		LAltFuzzp4
	xorl	%eax,%eax
	movb	%dh,%ah
	movb	(%esi),%al
	movw	%bp,6(%ecx)
	movb	0x12345678(%eax),%al
LAltFuzzPatch5:
	movb	3(%edi),%ah
	addl	C(d_transluc),%eax
	movb	(%eax),%al
	movb	%al,3(%edi)
LAltFuzzp4:
	addl	tstep,%edx
	sbbl	%eax,%eax
	addl	lzistepx,%ebp
	adcl	$0,%ebp
	addl	C(a_sstepxfrac),%ebx
	adcl	advancetable+4(,%eax,4),%esi

LAltFuzzDraw4:
	cmpw	8(%ecx),%bp
	jl		LAltFuzzp5
	xorl	%eax,%eax
	movb	%dh,%ah
	movb	(%esi),%al
	movw	%bp,8(%ecx)
	movb	0x12345678(%eax),%al
LAltFuzzPatch4:
	movb	4(%edi),%ah
	addl	C(d_transluc),%eax
	movb	(%eax),%al
	movb	%al,4(%edi)
LAltFuzzp5:
	addl	tstep,%edx
	sbbl	%eax,%eax
	addl	lzistepx,%ebp
	adcl	$0,%ebp
	addl	C(a_sstepxfrac),%ebx
	adcl	advancetable+4(,%eax,4),%esi

LAltFuzzDraw3:
	cmpw	10(%ecx),%bp
	jl		LAltFuzzp6
	xorl	%eax,%eax
	movb	%dh,%ah
	movb	(%esi),%al
	movw	%bp,10(%ecx)
	movb	0x12345678(%eax),%al
LAltFuzzPatch3:
	movb	5(%edi),%ah
	addl	C(d_transluc),%eax
	movb	(%eax),%al
	movb	%al,5(%edi)
LAltFuzzp6:
	addl	tstep,%edx
	sbbl	%eax,%eax
	addl	lzistepx,%ebp
	adcl	$0,%ebp
	addl	C(a_sstepxfrac),%ebx
	adcl	advancetable+4(,%eax,4),%esi

LAltFuzzDraw2:
	cmpw	12(%ecx),%bp
	jl		LAltFuzzp7
	xorl	%eax,%eax
	movb	%dh,%ah
	movb	(%esi),%al
	movw	%bp,12(%ecx)
	movb	0x12345678(%eax),%al
LAltFuzzPatch2:
	movb	6(%edi),%ah
	addl	C(d_transluc),%eax
	movb	(%eax),%al
	movb	%al,6(%edi)
LAltFuzzp7:
	addl	tstep,%edx
	sbbl	%eax,%eax
	addl	lzistepx,%ebp
	adcl	$0,%ebp
	addl	C(a_sstepxfrac),%ebx
	adcl	advancetable+4(,%eax,4),%esi

LAltFuzzDraw1:
	cmpw	14(%ecx),%bp
	jl		LAltFuzzp8
	xorl	%eax,%eax
	movb	%dh,%ah
	movb	(%esi),%al
	movw	%bp,14(%ecx)
	movb	0x12345678(%eax),%al
LAltFuzzPatch1:
	movb	7(%edi),%ah
	addl	C(d_transluc),%eax
	movb	(%eax),%al
	movb	%al,7(%edi)
LAltFuzzp8:
	addl	tstep,%edx
	sbbl	%eax,%eax
	addl	lzistepx,%ebp
	adcl	$0,%ebp
	addl	C(a_sstepxfrac),%ebx
	adcl	advancetable+4(,%eax,4),%esi

	addl	$8,%edi
	addl	$16,%ecx

	decw	%bx
	jnz		LAltFuzzDrawLoop

	popl	%esi				// restore spans pointer
LAltFuzzNextSpan:
	addl	$(spanpackage_t_size),%esi	// point to next span
LAltFuzzNextSpanESISet:
	movswl	spanpackage_t_count(%esi),%edx
	cmpl	$DPS_SPAN_LIST_END,%edx		// any more spans?
	jnz		LAltFuzzSpanLoop			// yes

	popl	%edi
	popl	%ebp				// restore the caller's stack frame
	popl	%ebx				// restore register variables
	popl	%esi
	ret


// draw a one-long span

LAltFuzzExactlyOneLong:

	movl	spanpackage_t_pz(%esi),%ecx
	movl	spanpackage_t_zi(%esi),%ebp

	rorl	$16,%ebp	// put high 16 bits of 1/z in low word
	movl	spanpackage_t_ptex(%esi),%ebx

	cmpw	(%ecx),%bp
	jl		LAltFuzzNextSpan
	xorl	%eax,%eax
	movl	spanpackage_t_pdest(%esi),%edi
	movb	spanpackage_t_r+1(%esi),%ah
	addl	$(spanpackage_t_size),%esi	// point to next span
	movb	(%ebx),%al
	movw	%bp,(%ecx)
	movb	0x12345678(%eax),%al
LAltFuzzPatch9:
	movb	(%edi),%ah
	addl	C(d_transluc),%eax
	movb	(%eax),%al
	movb	%al,(%edi)

	jmp		LAltFuzzNextSpanESISet

//==========================================================================
//
//	D_PolysetDrawSpansRGB_8
//
//	8-bpp horizontal span drawing code for affine polygons, with smooth
// shading and no transparency
//
//==========================================================================

.globl C(D_PolysetDrawSpansRGB_8)
C(D_PolysetDrawSpansRGB_8):
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
	movw	%ax,Ltemp
	movb	gb+1,%ah
	movb	(%esi),%al
	movw	0x12345678(,%eax,2),%ax
LGPatch8:
	orw		%ax,Ltemp
	movb	gb+3,%ah
	movb	(%esi),%al
	movw	0x12345678(,%eax,2),%ax
LBPatch8:
	orw		Ltemp,%ax
	movb	0x12345678(,%eax,2),%al
LTablePatch8:
	movb	%al,(%edi)
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
	movw	%ax,Ltemp
	movb	gb+1,%ah
	movb	(%esi),%al
	movw	0x12345678(,%eax,2),%ax
LGPatch7:
	orw		%ax,Ltemp
	movb	gb+3,%ah
	movb	(%esi),%al
	movw	0x12345678(,%eax,2),%ax
LBPatch7:
	orw		Ltemp,%ax
	movb	0x12345678(,%eax,2),%al
LTablePatch7:
	movb	%al,1(%edi)
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
	movw	%ax,Ltemp
	movb	gb+1,%ah
	movb	(%esi),%al
	movw	0x12345678(,%eax,2),%ax
LGPatch6:
	orw		%ax,Ltemp
	movb	gb+3,%ah
	movb	(%esi),%al
	movw	0x12345678(,%eax,2),%ax
LBPatch6:
	orw		Ltemp,%ax
	movb	0x12345678(,%eax,2),%al
LTablePatch6:
	movb	%al,2(%edi)
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
	movw	%ax,Ltemp
	movb	gb+1,%ah
	movb	(%esi),%al
	movw	0x12345678(,%eax,2),%ax
LGPatch5:
	orw		%ax,Ltemp
	movb	gb+3,%ah
	movb	(%esi),%al
	movw	0x12345678(,%eax,2),%ax
LBPatch5:
	orw		Ltemp,%ax
	movb	0x12345678(,%eax,2),%al
LTablePatch5:
	movb	%al,3(%edi)
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
	movw	%ax,Ltemp
	movb	gb+1,%ah
	movb	(%esi),%al
	movw	0x12345678(,%eax,2),%ax
LGPatch4:
	orw		%ax,Ltemp
	movb	gb+3,%ah
	movb	(%esi),%al
	movw	0x12345678(,%eax,2),%ax
LBPatch4:
	orw		Ltemp,%ax
	movb	0x12345678(,%eax,2),%al
LTablePatch4:
	movb	%al,4(%edi)
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
	movw	%ax,Ltemp
	movb	gb+1,%ah
	movb	(%esi),%al
	movw	0x12345678(,%eax,2),%ax
LGPatch3:
	orw		%ax,Ltemp
	movb	gb+3,%ah
	movb	(%esi),%al
	movw	0x12345678(,%eax,2),%ax
LBPatch3:
	orw		Ltemp,%ax
	movb	0x12345678(,%eax,2),%al
LTablePatch3:
	movb	%al,5(%edi)
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
	movw	%ax,Ltemp
	movb	gb+1,%ah
	movb	(%esi),%al
	movw	0x12345678(,%eax,2),%ax
LGPatch2:
	orw		%ax,Ltemp
	movb	gb+3,%ah
	movb	(%esi),%al
	movw	0x12345678(,%eax,2),%ax
LBPatch2:
	orw		Ltemp,%ax
	movb	0x12345678(,%eax,2),%al
LTablePatch2:
	movb	%al,6(%edi)
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
	movw	%ax,Ltemp
	movb	gb+1,%ah
	movb	(%esi),%al
	movw	0x12345678(,%eax,2),%ax
LGPatch1:
	orw		%ax,Ltemp
	movb	gb+3,%ah
	movb	(%esi),%al
	movw	0x12345678(,%eax,2),%ax
LBPatch1:
	orw		Ltemp,%ax
	movb	0x12345678(,%eax,2),%al
LTablePatch1:
	movb	%al,7(%edi)
LRGBp8:
	movl	gbstep,%eax
	addl	%eax,gb
	addl	tstep,%edx
	sbbl	%eax,%eax
	addl	lzistepx,%ebp
	adcl	$0,%ebp
	addl	C(a_sstepxfrac),%ebx
	adcl	advancetable+4(,%eax,4),%esi

	addl	$8,%edi
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
	movw	%ax,Ltemp
	movb	spanpackage_t_g+1(%esi),%ah
	movb	(%ebx),%al
	movw	0x12345678(,%eax,2),%ax
LGPatch9:
	orw		%ax,Ltemp
	movb	spanpackage_t_b+1(%esi),%ah
	addl	$(spanpackage_t_size),%esi	// point to next span
	movb	(%ebx),%al
	movw	0x12345678(,%eax,2),%ax
LBPatch9:
	orw		Ltemp,%ax
	movb	0x12345678(,%eax,2),%al
LTablePatch9:
	movb	%al,(%edi)

	jmp		LRGBNextSpanESISet

//==========================================================================
//
//	D_PolysetDrawSpansRGBFuzz_8
//
//	8-bpp horizontal span drawing code for affine polygons, with smooth
// shading and no transparency
//
//==========================================================================

.globl C(D_PolysetDrawSpansRGBFuzz_8)
C(D_PolysetDrawSpansRGBFuzz_8):
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
	jmp		*LRGBFuzzentryvec_table(,%eax,4)

LRGBFuzzentryvec_table:
	.long	LRGBFuzzDraw8, LRGBFuzzDraw7, LRGBFuzzDraw6, LRGBFuzzDraw5
	.long	LRGBFuzzDraw4, LRGBFuzzDraw3, LRGBFuzzDraw2, LRGBFuzzDraw1

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

LRGBFuzzDraw8:
	cmpw	(%ecx),%bp
	jl		LRGBFuzzp1
	xorl	%eax,%eax
	movb	%dh,%ah
	movb	(%esi),%al
	movw	%bp,(%ecx)
	movw	0x12345678(,%eax,2),%ax
LRFuzzPatch8:
	movw	%ax,Ltemp
	movb	gb+1,%ah
	movb	(%esi),%al
	movw	0x12345678(,%eax,2),%ax
LGFuzzPatch8:
	orw		%ax,Ltemp
	movb	gb+3,%ah
	movb	(%esi),%al
	movw	0x12345678(,%eax,2),%ax
LBFuzzPatch8:
	orw		Ltemp,%ax
	movb	0x12345678(,%eax,2),%ah
LTableFuzzPatch8:
	movb	(%edi),%al
	addl	C(d_transluc),%eax
	movb	(%eax),%al
	movb	%al,(%edi)
LRGBFuzzp1:
	movl	gbstep,%eax
	addl	%eax,gb
	addl	tstep,%edx
	sbbl	%eax,%eax
	addl	lzistepx,%ebp
	adcl	$0,%ebp
	addl	C(a_sstepxfrac),%ebx
	adcl	advancetable+4(,%eax,4),%esi

LRGBFuzzDraw7:
	cmpw	2(%ecx),%bp
	jl		LRGBFuzzp2
	xorl	%eax,%eax
	movb	%dh,%ah
	movb	(%esi),%al
	movw	%bp,2(%ecx)
	movw	0x12345678(,%eax,2),%ax
LRFuzzPatch7:
	movw	%ax,Ltemp
	movb	gb+1,%ah
	movb	(%esi),%al
	movw	0x12345678(,%eax,2),%ax
LGFuzzPatch7:
	orw		%ax,Ltemp
	movb	gb+3,%ah
	movb	(%esi),%al
	movw	0x12345678(,%eax,2),%ax
LBFuzzPatch7:
	orw		Ltemp,%ax
	movb	0x12345678(,%eax,2),%ah
LTableFuzzPatch7:
	movb	1(%edi),%al
	addl	C(d_transluc),%eax
	movb	(%eax),%al
	movb	%al,1(%edi)
LRGBFuzzp2:
	movl	gbstep,%eax
	addl	%eax,gb
	addl	tstep,%edx
	sbbl	%eax,%eax
	addl	lzistepx,%ebp
	adcl	$0,%ebp
	addl	C(a_sstepxfrac),%ebx
	adcl	advancetable+4(,%eax,4),%esi

LRGBFuzzDraw6:
	cmpw	4(%ecx),%bp
	jl		LRGBFuzzp3
	xorl	%eax,%eax
	movb	%dh,%ah
	movb	(%esi),%al
	movw	%bp,4(%ecx)
	movw	0x12345678(,%eax,2),%ax
LRFuzzPatch6:
	movw	%ax,Ltemp
	movb	gb+1,%ah
	movb	(%esi),%al
	movw	0x12345678(,%eax,2),%ax
LGFuzzPatch6:
	orw		%ax,Ltemp
	movb	gb+3,%ah
	movb	(%esi),%al
	movw	0x12345678(,%eax,2),%ax
LBFuzzPatch6:
	orw		Ltemp,%ax
	movb	0x12345678(,%eax,2),%ah
LTableFuzzPatch6:
	movb	2(%edi),%al
	addl	C(d_transluc),%eax
	movb	(%eax),%al
	movb	%al,2(%edi)
LRGBFuzzp3:
	movl	gbstep,%eax
	addl	%eax,gb
	addl	tstep,%edx
	sbbl	%eax,%eax
	addl	lzistepx,%ebp
	adcl	$0,%ebp
	addl	C(a_sstepxfrac),%ebx
	adcl	advancetable+4(,%eax,4),%esi

LRGBFuzzDraw5:
	cmpw	6(%ecx),%bp
	jl		LRGBFuzzp4
	xorl	%eax,%eax
	movb	%dh,%ah
	movb	(%esi),%al
	movw	%bp,6(%ecx)
	movw	0x12345678(,%eax,2),%ax
LRFuzzPatch5:
	movw	%ax,Ltemp
	movb	gb+1,%ah
	movb	(%esi),%al
	movw	0x12345678(,%eax,2),%ax
LGFuzzPatch5:
	orw		%ax,Ltemp
	movb	gb+3,%ah
	movb	(%esi),%al
	movw	0x12345678(,%eax,2),%ax
LBFuzzPatch5:
	orw		Ltemp,%ax
	movb	0x12345678(,%eax,2),%ah
LTableFuzzPatch5:
	movb	3(%edi),%al
	addl	C(d_transluc),%eax
	movb	(%eax),%al
	movb	%al,3(%edi)
LRGBFuzzp4:
	movl	gbstep,%eax
	addl	%eax,gb
	addl	tstep,%edx
	sbbl	%eax,%eax
	addl	lzistepx,%ebp
	adcl	$0,%ebp
	addl	C(a_sstepxfrac),%ebx
	adcl	advancetable+4(,%eax,4),%esi

LRGBFuzzDraw4:
	cmpw	8(%ecx),%bp
	jl		LRGBFuzzp5
	xorl	%eax,%eax
	movb	%dh,%ah
	movb	(%esi),%al
	movw	%bp,8(%ecx)
	movw	0x12345678(,%eax,2),%ax
LRFuzzPatch4:
	movw	%ax,Ltemp
	movb	gb+1,%ah
	movb	(%esi),%al
	movw	0x12345678(,%eax,2),%ax
LGFuzzPatch4:
	orw		%ax,Ltemp
	movb	gb+3,%ah
	movb	(%esi),%al
	movw	0x12345678(,%eax,2),%ax
LBFuzzPatch4:
	orw		Ltemp,%ax
	movb	0x12345678(,%eax,2),%ah
LTableFuzzPatch4:
	movb	4(%edi),%al
	addl	C(d_transluc),%eax
	movb	(%eax),%al
	movb	%al,4(%edi)
LRGBFuzzp5:
	movl	gbstep,%eax
	addl	%eax,gb
	addl	tstep,%edx
	sbbl	%eax,%eax
	addl	lzistepx,%ebp
	adcl	$0,%ebp
	addl	C(a_sstepxfrac),%ebx
	adcl	advancetable+4(,%eax,4),%esi

LRGBFuzzDraw3:
	cmpw	10(%ecx),%bp
	jl		LRGBFuzzp6
	xorl	%eax,%eax
	movb	%dh,%ah
	movb	(%esi),%al
	movw	%bp,10(%ecx)
	movw	0x12345678(,%eax,2),%ax
LRFuzzPatch3:
	movw	%ax,Ltemp
	movb	gb+1,%ah
	movb	(%esi),%al
	movw	0x12345678(,%eax,2),%ax
LGFuzzPatch3:
	orw		%ax,Ltemp
	movb	gb+3,%ah
	movb	(%esi),%al
	movw	0x12345678(,%eax,2),%ax
LBFuzzPatch3:
	orw		Ltemp,%ax
	movb	0x12345678(,%eax,2),%ah
LTableFuzzPatch3:
	movb	5(%edi),%al
	addl	C(d_transluc),%eax
	movb	(%eax),%al
	movb	%al,5(%edi)
LRGBFuzzp6:
	movl	gbstep,%eax
	addl	%eax,gb
	addl	tstep,%edx
	sbbl	%eax,%eax
	addl	lzistepx,%ebp
	adcl	$0,%ebp
	addl	C(a_sstepxfrac),%ebx
	adcl	advancetable+4(,%eax,4),%esi

LRGBFuzzDraw2:
	cmpw	12(%ecx),%bp
	jl		LRGBFuzzp7
	xorl	%eax,%eax
	movb	%dh,%ah
	movb	(%esi),%al
	movw	%bp,12(%ecx)
	movw	0x12345678(,%eax,2),%ax
LRFuzzPatch2:
	movw	%ax,Ltemp
	movb	gb+1,%ah
	movb	(%esi),%al
	movw	0x12345678(,%eax,2),%ax
LGFuzzPatch2:
	orw		%ax,Ltemp
	movb	gb+3,%ah
	movb	(%esi),%al
	movw	0x12345678(,%eax,2),%ax
LBFuzzPatch2:
	orw		Ltemp,%ax
	movb	0x12345678(,%eax,2),%ah
LTableFuzzPatch2:
	movb	6(%edi),%al
	addl	C(d_transluc),%eax
	movb	(%eax),%al
	movb	%al,6(%edi)
LRGBFuzzp7:
	movl	gbstep,%eax
	addl	%eax,gb
	addl	tstep,%edx
	sbbl	%eax,%eax
	addl	lzistepx,%ebp
	adcl	$0,%ebp
	addl	C(a_sstepxfrac),%ebx
	adcl	advancetable+4(,%eax,4),%esi

LRGBFuzzDraw1:
	cmpw	14(%ecx),%bp
	jl		LRGBFuzzp8
	xorl	%eax,%eax
	movb	%dh,%ah
	movb	(%esi),%al
	movw	%bp,14(%ecx)
	movw	0x12345678(,%eax,2),%ax
LRFuzzPatch1:
	movw	%ax,Ltemp
	movb	gb+1,%ah
	movb	(%esi),%al
	movw	0x12345678(,%eax,2),%ax
LGFuzzPatch1:
	orw		%ax,Ltemp
	movb	gb+3,%ah
	movb	(%esi),%al
	movw	0x12345678(,%eax,2),%ax
LBFuzzPatch1:
	orw		Ltemp,%ax
	movb	0x12345678(,%eax,2),%ah
LTableFuzzPatch1:
	movb	7(%edi),%al
	addl	C(d_transluc),%eax
	movb	(%eax),%al
	movb	%al,7(%edi)
LRGBFuzzp8:
	movl	gbstep,%eax
	addl	%eax,gb
	addl	tstep,%edx
	sbbl	%eax,%eax
	addl	lzistepx,%ebp
	adcl	$0,%ebp
	addl	C(a_sstepxfrac),%ebx
	adcl	advancetable+4(,%eax,4),%esi

	addl	$8,%edi
	addl	$16,%ecx

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
	movw	0x12345678(,%eax,2),%ax
LRFuzzPatch9:
	movw	%ax,Ltemp
	movb	spanpackage_t_g+1(%esi),%ah
	movb	(%ebx),%al
	movw	0x12345678(,%eax,2),%ax
LGFuzzPatch9:
	orw		%ax,Ltemp
	movb	spanpackage_t_b+1(%esi),%ah
	addl	$(spanpackage_t_size),%esi	// point to next span
	movb	(%ebx),%al
	movw	0x12345678(,%eax,2),%ax
LBFuzzPatch9:
	orw		Ltemp,%ax
	movb	0x12345678(,%eax,2),%ah
LTableFuzzPatch9:
	movb	(%edi),%al
	addl	C(d_transluc),%eax
	movb	(%eax),%al
	movb	%al,(%edi)

	jmp		LRGBFuzzNextSpanESISet

//==========================================================================
//
//	D_PolysetDrawSpansRGBAltFuzz_8
//
//	8-bpp horizontal span drawing code for affine polygons, with smooth
// shading and no transparency
//
//==========================================================================

.globl C(D_PolysetDrawSpansRGBAltFuzz_8)
C(D_PolysetDrawSpansRGBAltFuzz_8):
	pushl	%esi				// preserve register variables
	pushl	%ebx

	movl	pspans(%esp),%esi	// point to the first span descriptor
	movl	C(r_zistepx),%ecx

	pushl	%ebp				// preserve caller's stack frame
	pushl	%edi

	rorl	$16,%ecx			// put high 16 bits of 1/z step in low word
	movswl	spanpackage_t_count(%esi),%edx

	movl	%ecx,lzistepx

LRGBAltFuzzSpanLoop:

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
	js		LRGBAltFuzzNoTurnover

	movl	C(erroradjustdown),%edx
	movl	C(d_countextrastep),%edi
	subl	%edx,%ebx
	movl	C(d_aspancount),%ebp
	movl	%ebx,C(errorterm)
	addl	%edi,%ebp
	movl	%ebp,C(d_aspancount)
	jmp		LRGBAltFuzzRightEdgeStepped

LRGBAltFuzzNoTurnover:
	movl	C(d_aspancount),%edi
	movl	C(ubasestep),%edx
	movl	%ebx,C(errorterm)
	addl	%edx,%edi
	movl	%edi,C(d_aspancount)

LRGBAltFuzzRightEdgeStepped:
	cmpl	$1,%eax

	jl		LRGBAltFuzzNextSpan
	jz		LRGBAltFuzzExactlyOneLong

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
	jmp		*LRGBAltFuzzentryvec_table(,%eax,4)

LRGBAltFuzzentryvec_table:
	.long	LRGBAltFuzzDraw8, LRGBAltFuzzDraw7, LRGBAltFuzzDraw6, LRGBAltFuzzDraw5
	.long	LRGBAltFuzzDraw4, LRGBAltFuzzDraw3, LRGBAltFuzzDraw2, LRGBAltFuzzDraw1

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

LRGBAltFuzzDrawLoop:

// FIXME: do we need to clamp light? We may need at least a buffer bit to
// keep it from poking into tfrac and causing problems

LRGBAltFuzzDraw8:
	cmpw	(%ecx),%bp
	jl		LRGBAltFuzzp1
	xorl	%eax,%eax
	movb	%dh,%ah
	movb	(%esi),%al
	movw	%bp,(%ecx)
	movw	0x12345678(,%eax,2),%ax
LRAltFuzzPatch8:
	movw	%ax,Ltemp
	movb	gb+1,%ah
	movb	(%esi),%al
	movw	0x12345678(,%eax,2),%ax
LGAltFuzzPatch8:
	orw		%ax,Ltemp
	movb	gb+3,%ah
	movb	(%esi),%al
	movw	0x12345678(,%eax,2),%ax
LBAltFuzzPatch8:
	orw		Ltemp,%ax
	movb	0x12345678(,%eax,2),%al
LTableAltFuzzPatch8:
	movb	(%edi),%ah
	addl	C(d_transluc),%eax
	movb	(%eax),%al
	movb	%al,(%edi)
LRGBAltFuzzp1:
	movl	gbstep,%eax
	addl	%eax,gb
	addl	tstep,%edx
	sbbl	%eax,%eax
	addl	lzistepx,%ebp
	adcl	$0,%ebp
	addl	C(a_sstepxfrac),%ebx
	adcl	advancetable+4(,%eax,4),%esi

LRGBAltFuzzDraw7:
	cmpw	2(%ecx),%bp
	jl		LRGBAltFuzzp2
	xorl	%eax,%eax
	movb	%dh,%ah
	movb	(%esi),%al
	movw	%bp,2(%ecx)
	movw	0x12345678(,%eax,2),%ax
LRAltFuzzPatch7:
	movw	%ax,Ltemp
	movb	gb+1,%ah
	movb	(%esi),%al
	movw	0x12345678(,%eax,2),%ax
LGAltFuzzPatch7:
	orw		%ax,Ltemp
	movb	gb+3,%ah
	movb	(%esi),%al
	movw	0x12345678(,%eax,2),%ax
LBAltFuzzPatch7:
	orw		Ltemp,%ax
	movb	0x12345678(,%eax,2),%al
LTableAltFuzzPatch7:
	movb	1(%edi),%ah
	addl	C(d_transluc),%eax
	movb	(%eax),%al
	movb	%al,1(%edi)
LRGBAltFuzzp2:
	movl	gbstep,%eax
	addl	%eax,gb
	addl	tstep,%edx
	sbbl	%eax,%eax
	addl	lzistepx,%ebp
	adcl	$0,%ebp
	addl	C(a_sstepxfrac),%ebx
	adcl	advancetable+4(,%eax,4),%esi

LRGBAltFuzzDraw6:
	cmpw	4(%ecx),%bp
	jl		LRGBAltFuzzp3
	xorl	%eax,%eax
	movb	%dh,%ah
	movb	(%esi),%al
	movw	%bp,4(%ecx)
	movw	0x12345678(,%eax,2),%ax
LRAltFuzzPatch6:
	movw	%ax,Ltemp
	movb	gb+1,%ah
	movb	(%esi),%al
	movw	0x12345678(,%eax,2),%ax
LGAltFuzzPatch6:
	orw		%ax,Ltemp
	movb	gb+3,%ah
	movb	(%esi),%al
	movw	0x12345678(,%eax,2),%ax
LBAltFuzzPatch6:
	orw		Ltemp,%ax
	movb	0x12345678(,%eax,2),%al
LTableAltFuzzPatch6:
	movb	2(%edi),%ah
	addl	C(d_transluc),%eax
	movb	(%eax),%al
	movb	%al,2(%edi)
LRGBAltFuzzp3:
	movl	gbstep,%eax
	addl	%eax,gb
	addl	tstep,%edx
	sbbl	%eax,%eax
	addl	lzistepx,%ebp
	adcl	$0,%ebp
	addl	C(a_sstepxfrac),%ebx
	adcl	advancetable+4(,%eax,4),%esi

LRGBAltFuzzDraw5:
	cmpw	6(%ecx),%bp
	jl		LRGBAltFuzzp4
	xorl	%eax,%eax
	movb	%dh,%ah
	movb	(%esi),%al
	movw	%bp,6(%ecx)
	movw	0x12345678(,%eax,2),%ax
LRAltFuzzPatch5:
	movw	%ax,Ltemp
	movb	gb+1,%ah
	movb	(%esi),%al
	movw	0x12345678(,%eax,2),%ax
LGAltFuzzPatch5:
	orw		%ax,Ltemp
	movb	gb+3,%ah
	movb	(%esi),%al
	movw	0x12345678(,%eax,2),%ax
LBAltFuzzPatch5:
	orw		Ltemp,%ax
	movb	0x12345678(,%eax,2),%al
LTableAltFuzzPatch5:
	movb	3(%edi),%ah
	addl	C(d_transluc),%eax
	movb	(%eax),%al
	movb	%al,3(%edi)
LRGBAltFuzzp4:
	movl	gbstep,%eax
	addl	%eax,gb
	addl	tstep,%edx
	sbbl	%eax,%eax
	addl	lzistepx,%ebp
	adcl	$0,%ebp
	addl	C(a_sstepxfrac),%ebx
	adcl	advancetable+4(,%eax,4),%esi

LRGBAltFuzzDraw4:
	cmpw	8(%ecx),%bp
	jl		LRGBAltFuzzp5
	xorl	%eax,%eax
	movb	%dh,%ah
	movb	(%esi),%al
	movw	%bp,8(%ecx)
	movw	0x12345678(,%eax,2),%ax
LRAltFuzzPatch4:
	movw	%ax,Ltemp
	movb	gb+1,%ah
	movb	(%esi),%al
	movw	0x12345678(,%eax,2),%ax
LGAltFuzzPatch4:
	orw		%ax,Ltemp
	movb	gb+3,%ah
	movb	(%esi),%al
	movw	0x12345678(,%eax,2),%ax
LBAltFuzzPatch4:
	orw		Ltemp,%ax
	movb	0x12345678(,%eax,2),%al
LTableAltFuzzPatch4:
	movb	4(%edi),%ah
	addl	C(d_transluc),%eax
	movb	(%eax),%al
	movb	%al,4(%edi)
LRGBAltFuzzp5:
	movl	gbstep,%eax
	addl	%eax,gb
	addl	tstep,%edx
	sbbl	%eax,%eax
	addl	lzistepx,%ebp
	adcl	$0,%ebp
	addl	C(a_sstepxfrac),%ebx
	adcl	advancetable+4(,%eax,4),%esi

LRGBAltFuzzDraw3:
	cmpw	10(%ecx),%bp
	jl		LRGBAltFuzzp6
	xorl	%eax,%eax
	movb	%dh,%ah
	movb	(%esi),%al
	movw	%bp,10(%ecx)
	movw	0x12345678(,%eax,2),%ax
LRAltFuzzPatch3:
	movw	%ax,Ltemp
	movb	gb+1,%ah
	movb	(%esi),%al
	movw	0x12345678(,%eax,2),%ax
LGAltFuzzPatch3:
	orw		%ax,Ltemp
	movb	gb+3,%ah
	movb	(%esi),%al
	movw	0x12345678(,%eax,2),%ax
LBAltFuzzPatch3:
	orw		Ltemp,%ax
	movb	0x12345678(,%eax,2),%al
LTableAltFuzzPatch3:
	movb	5(%edi),%ah
	addl	C(d_transluc),%eax
	movb	(%eax),%al
	movb	%al,5(%edi)
LRGBAltFuzzp6:
	movl	gbstep,%eax
	addl	%eax,gb
	addl	tstep,%edx
	sbbl	%eax,%eax
	addl	lzistepx,%ebp
	adcl	$0,%ebp
	addl	C(a_sstepxfrac),%ebx
	adcl	advancetable+4(,%eax,4),%esi

LRGBAltFuzzDraw2:
	cmpw	12(%ecx),%bp
	jl		LRGBAltFuzzp7
	xorl	%eax,%eax
	movb	%dh,%ah
	movb	(%esi),%al
	movw	%bp,12(%ecx)
	movw	0x12345678(,%eax,2),%ax
LRAltFuzzPatch2:
	movw	%ax,Ltemp
	movb	gb+1,%ah
	movb	(%esi),%al
	movw	0x12345678(,%eax,2),%ax
LGAltFuzzPatch2:
	orw		%ax,Ltemp
	movb	gb+3,%ah
	movb	(%esi),%al
	movw	0x12345678(,%eax,2),%ax
LBAltFuzzPatch2:
	orw		Ltemp,%ax
	movb	0x12345678(,%eax,2),%al
LTableAltFuzzPatch2:
	movb	6(%edi),%ah
	addl	C(d_transluc),%eax
	movb	(%eax),%al
	movb	%al,6(%edi)
LRGBAltFuzzp7:
	movl	gbstep,%eax
	addl	%eax,gb
	addl	tstep,%edx
	sbbl	%eax,%eax
	addl	lzistepx,%ebp
	adcl	$0,%ebp
	addl	C(a_sstepxfrac),%ebx
	adcl	advancetable+4(,%eax,4),%esi

LRGBAltFuzzDraw1:
	cmpw	14(%ecx),%bp
	jl		LRGBAltFuzzp8
	xorl	%eax,%eax
	movb	%dh,%ah
	movb	(%esi),%al
	movw	%bp,14(%ecx)
	movw	0x12345678(,%eax,2),%ax
LRAltFuzzPatch1:
	movw	%ax,Ltemp
	movb	gb+1,%ah
	movb	(%esi),%al
	movw	0x12345678(,%eax,2),%ax
LGAltFuzzPatch1:
	orw		%ax,Ltemp
	movb	gb+3,%ah
	movb	(%esi),%al
	movw	0x12345678(,%eax,2),%ax
LBAltFuzzPatch1:
	orw		Ltemp,%ax
	movb	0x12345678(,%eax,2),%al
LTableAltFuzzPatch1:
	movb	7(%edi),%ah
	addl	C(d_transluc),%eax
	movb	(%eax),%al
	movb	%al,7(%edi)
LRGBAltFuzzp8:
	movl	gbstep,%eax
	addl	%eax,gb
	addl	tstep,%edx
	sbbl	%eax,%eax
	addl	lzistepx,%ebp
	adcl	$0,%ebp
	addl	C(a_sstepxfrac),%ebx
	adcl	advancetable+4(,%eax,4),%esi

	addl	$8,%edi
	addl	$16,%ecx

	decw	%bx
	jnz		LRGBAltFuzzDrawLoop

	popl	%esi				// restore spans pointer
LRGBAltFuzzNextSpan:
	addl	$(spanpackage_t_size),%esi	// point to next span
LRGBAltFuzzNextSpanESISet:
	movswl	spanpackage_t_count(%esi),%edx
	cmpl	$DPS_SPAN_LIST_END,%edx		// any more spans?
	jnz		LRGBAltFuzzSpanLoop			// yes

	popl	%edi
	popl	%ebp				// restore the caller's stack frame
	popl	%ebx				// restore register variables
	popl	%esi
	ret


// draw a one-long span

LRGBAltFuzzExactlyOneLong:

	movl	spanpackage_t_pz(%esi),%ecx
	movl	spanpackage_t_zi(%esi),%ebp

	rorl	$16,%ebp	// put high 16 bits of 1/z in low word
	movl	spanpackage_t_ptex(%esi),%ebx

	cmpw	(%ecx),%bp
	jl		LRGBAltFuzzNextSpan
	xorl	%eax,%eax
	movl	spanpackage_t_pdest(%esi),%edi
	movb	spanpackage_t_r+1(%esi),%ah
	movb	(%ebx),%al
	movw	%bp,(%ecx)
	movw	0x12345678(,%eax,2),%ax
LRAltFuzzPatch9:
	movw	%ax,Ltemp
	movb	spanpackage_t_g+1(%esi),%ah
	movb	(%ebx),%al
	movw	0x12345678(,%eax,2),%ax
LGAltFuzzPatch9:
	orw		%ax,Ltemp
	movb	spanpackage_t_b+1(%esi),%ah
	addl	$(spanpackage_t_size),%esi	// point to next span
	movb	(%ebx),%al
	movw	0x12345678(,%eax,2),%ax
LBAltFuzzPatch9:
	orw		Ltemp,%ax
	movb	0x12345678(,%eax,2),%al
LTableAltFuzzPatch9:
	movb	(%edi),%ah
	addl	C(d_transluc),%eax
	movb	(%eax),%al
	movb	%al,(%edi)

	jmp		LRGBAltFuzzNextSpanESISet



.globl C(D_PolysetAff8End)
C(D_PolysetAff8End):


.globl C(D_Aff8Patch)
C(D_Aff8Patch):
	movl	C(fadetable),%eax
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

	movl	%eax,LAltFuzzPatch1-4
	movl	%eax,LAltFuzzPatch2-4
	movl	%eax,LAltFuzzPatch3-4
	movl	%eax,LAltFuzzPatch4-4
	movl	%eax,LAltFuzzPatch5-4
	movl	%eax,LAltFuzzPatch6-4
	movl	%eax,LAltFuzzPatch7-4
	movl	%eax,LAltFuzzPatch8-4
	movl	%eax,LAltFuzzPatch9-4

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

	movl	%eax,LRFuzzPatch1-4
	movl	%eax,LRFuzzPatch2-4
	movl	%eax,LRFuzzPatch3-4
	movl	%eax,LRFuzzPatch4-4
	movl	%eax,LRFuzzPatch5-4
	movl	%eax,LRFuzzPatch6-4
	movl	%eax,LRFuzzPatch7-4
	movl	%eax,LRFuzzPatch8-4
	movl	%eax,LRFuzzPatch9-4

	movl	%eax,LRAltFuzzPatch1-4
	movl	%eax,LRAltFuzzPatch2-4
	movl	%eax,LRAltFuzzPatch3-4
	movl	%eax,LRAltFuzzPatch4-4
	movl	%eax,LRAltFuzzPatch5-4
	movl	%eax,LRAltFuzzPatch6-4
	movl	%eax,LRAltFuzzPatch7-4
	movl	%eax,LRAltFuzzPatch8-4
	movl	%eax,LRAltFuzzPatch9-4

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

	movl	%eax,LGFuzzPatch1-4
	movl	%eax,LGFuzzPatch2-4
	movl	%eax,LGFuzzPatch3-4
	movl	%eax,LGFuzzPatch4-4
	movl	%eax,LGFuzzPatch5-4
	movl	%eax,LGFuzzPatch6-4
	movl	%eax,LGFuzzPatch7-4
	movl	%eax,LGFuzzPatch8-4
	movl	%eax,LGFuzzPatch9-4

	movl	%eax,LGAltFuzzPatch1-4
	movl	%eax,LGAltFuzzPatch2-4
	movl	%eax,LGAltFuzzPatch3-4
	movl	%eax,LGAltFuzzPatch4-4
	movl	%eax,LGAltFuzzPatch5-4
	movl	%eax,LGAltFuzzPatch6-4
	movl	%eax,LGAltFuzzPatch7-4
	movl	%eax,LGAltFuzzPatch8-4
	movl	%eax,LGAltFuzzPatch9-4

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

	movl	%eax,LBFuzzPatch1-4
	movl	%eax,LBFuzzPatch2-4
	movl	%eax,LBFuzzPatch3-4
	movl	%eax,LBFuzzPatch4-4
	movl	%eax,LBFuzzPatch5-4
	movl	%eax,LBFuzzPatch6-4
	movl	%eax,LBFuzzPatch7-4
	movl	%eax,LBFuzzPatch8-4
	movl	%eax,LBFuzzPatch9-4

	movl	%eax,LBAltFuzzPatch1-4
	movl	%eax,LBAltFuzzPatch2-4
	movl	%eax,LBAltFuzzPatch3-4
	movl	%eax,LBAltFuzzPatch4-4
	movl	%eax,LBAltFuzzPatch5-4
	movl	%eax,LBAltFuzzPatch6-4
	movl	%eax,LBAltFuzzPatch7-4
	movl	%eax,LBAltFuzzPatch8-4
	movl	%eax,LBAltFuzzPatch9-4

	movl	C(d_rgbtable),%eax
	movl	%eax,LTablePatch1-4
	movl	%eax,LTablePatch2-4
	movl	%eax,LTablePatch3-4
	movl	%eax,LTablePatch4-4
	movl	%eax,LTablePatch5-4
	movl	%eax,LTablePatch6-4
	movl	%eax,LTablePatch7-4
	movl	%eax,LTablePatch8-4
	movl	%eax,LTablePatch9-4

	movl	%eax,LTableFuzzPatch1-4
	movl	%eax,LTableFuzzPatch2-4
	movl	%eax,LTableFuzzPatch3-4
	movl	%eax,LTableFuzzPatch4-4
	movl	%eax,LTableFuzzPatch5-4
	movl	%eax,LTableFuzzPatch6-4
	movl	%eax,LTableFuzzPatch7-4
	movl	%eax,LTableFuzzPatch8-4
	movl	%eax,LTableFuzzPatch9-4

	movl	%eax,LTableAltFuzzPatch1-4
	movl	%eax,LTableAltFuzzPatch2-4
	movl	%eax,LTableAltFuzzPatch3-4
	movl	%eax,LTableAltFuzzPatch4-4
	movl	%eax,LTableAltFuzzPatch5-4
	movl	%eax,LTableAltFuzzPatch6-4
	movl	%eax,LTableAltFuzzPatch7-4
	movl	%eax,LTableAltFuzzPatch8-4
	movl	%eax,LTableAltFuzzPatch9-4

	ret

#endif

//**************************************************************************
//
//	$Log$
//	Revision 1.2  2002/01/07 12:16:42  dj_jl
//	Changed copyright year
//
//	Revision 1.1  2001/08/15 17:12:23  dj_jl
//	Optimized model drawing
//	
//**************************************************************************
