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

#include "asm_i386.h"

#ifdef USEASM

	.data

Ltemp:	.long	0

	.text

#define pspans	4+8

//==========================================================================
//
//	D_PolysetDrawSpans_8
//
//	8-bpp horizontal span drawing code for affine polygons, with smooth
// shading and no transparency
//
//==========================================================================

	Align4
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
	movb	C(d_fadetable)(%eax),%al
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
	movb	C(d_fadetable)(%eax),%al
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
	movb	C(d_fadetable)(%eax),%al
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
	movb	C(d_fadetable)(%eax),%al
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
	movb	C(d_fadetable)(%eax),%al
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
	movb	C(d_fadetable)(%eax),%al
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
	movb	C(d_fadetable)(%eax),%al
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
	movb	C(d_fadetable)(%eax),%al
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
	movb	C(d_fadetable)(%eax),%al
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
	movb	C(d_fadetable)(%eax),%ah
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
	movb	C(d_fadetable)(%eax),%ah
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
	movb	C(d_fadetable)(%eax),%ah
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
	movb	C(d_fadetable)(%eax),%ah
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
	movb	C(d_fadetable)(%eax),%ah
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
	movb	C(d_fadetable)(%eax),%ah
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
	movb	C(d_fadetable)(%eax),%ah
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
	movb	C(d_fadetable)(%eax),%ah
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
	movb	C(d_fadetable)(%eax),%ah
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
	movb	C(d_fadetable)(%eax),%al
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
	movb	C(d_fadetable)(%eax),%al
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
	movb	C(d_fadetable)(%eax),%al
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
	movb	C(d_fadetable)(%eax),%al
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
	movb	C(d_fadetable)(%eax),%al
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
	movb	C(d_fadetable)(%eax),%al
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
	movb	C(d_fadetable)(%eax),%al
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
	movb	C(d_fadetable)(%eax),%al
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
	movb	C(d_fadetable)(%eax),%al
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
	movw	C(d_fadetable16r)(,%eax,2),%ax
	movw	%ax,Ltemp
	movb	gb+1,%ah
	movb	(%esi),%al
	movw	C(d_fadetable16g)(,%eax,2),%ax
	orw		%ax,Ltemp
	movb	gb+3,%ah
	movb	(%esi),%al
	movw	C(d_fadetable16b)(,%eax,2),%ax
	orw		Ltemp,%ax
	movb	C(r_rgbtable)(,%eax,2),%al
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
	movw	C(d_fadetable16r)(,%eax,2),%ax
	movw	%ax,Ltemp
	movb	gb+1,%ah
	movb	(%esi),%al
	movw	C(d_fadetable16g)(,%eax,2),%ax
	orw		%ax,Ltemp
	movb	gb+3,%ah
	movb	(%esi),%al
	movw	C(d_fadetable16b)(,%eax,2),%ax
	orw		Ltemp,%ax
	movb	C(r_rgbtable)(,%eax,2),%al
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
	movw	C(d_fadetable16r)(,%eax,2),%ax
	movw	%ax,Ltemp
	movb	gb+1,%ah
	movb	(%esi),%al
	movw	C(d_fadetable16g)(,%eax,2),%ax
	orw		%ax,Ltemp
	movb	gb+3,%ah
	movb	(%esi),%al
	movw	C(d_fadetable16b)(,%eax,2),%ax
	orw		Ltemp,%ax
	movb	C(r_rgbtable)(,%eax,2),%al
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
	movw	C(d_fadetable16r)(,%eax,2),%ax
	movw	%ax,Ltemp
	movb	gb+1,%ah
	movb	(%esi),%al
	movw	C(d_fadetable16g)(,%eax,2),%ax
	orw		%ax,Ltemp
	movb	gb+3,%ah
	movb	(%esi),%al
	movw	C(d_fadetable16b)(,%eax,2),%ax
	orw		Ltemp,%ax
	movb	C(r_rgbtable)(,%eax,2),%al
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
	movw	C(d_fadetable16r)(,%eax,2),%ax
	movw	%ax,Ltemp
	movb	gb+1,%ah
	movb	(%esi),%al
	movw	C(d_fadetable16g)(,%eax,2),%ax
	orw		%ax,Ltemp
	movb	gb+3,%ah
	movb	(%esi),%al
	movw	C(d_fadetable16b)(,%eax,2),%ax
	orw		Ltemp,%ax
	movb	C(r_rgbtable)(,%eax,2),%al
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
	movw	C(d_fadetable16r)(,%eax,2),%ax
	movw	%ax,Ltemp
	movb	gb+1,%ah
	movb	(%esi),%al
	movw	C(d_fadetable16g)(,%eax,2),%ax
	orw		%ax,Ltemp
	movb	gb+3,%ah
	movb	(%esi),%al
	movw	C(d_fadetable16b)(,%eax,2),%ax
	orw		Ltemp,%ax
	movb	C(r_rgbtable)(,%eax,2),%al
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
	movw	C(d_fadetable16r)(,%eax,2),%ax
	movw	%ax,Ltemp
	movb	gb+1,%ah
	movb	(%esi),%al
	movw	C(d_fadetable16g)(,%eax,2),%ax
	orw		%ax,Ltemp
	movb	gb+3,%ah
	movb	(%esi),%al
	movw	C(d_fadetable16b)(,%eax,2),%ax
	orw		Ltemp,%ax
	movb	C(r_rgbtable)(,%eax,2),%al
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
	movw	C(d_fadetable16r)(,%eax,2),%ax
	movw	%ax,Ltemp
	movb	gb+1,%ah
	movb	(%esi),%al
	movw	C(d_fadetable16g)(,%eax,2),%ax
	orw		%ax,Ltemp
	movb	gb+3,%ah
	movb	(%esi),%al
	movw	C(d_fadetable16b)(,%eax,2),%ax
	orw		Ltemp,%ax
	movb	C(r_rgbtable)(,%eax,2),%al
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
	movw	C(d_fadetable16r)(,%eax,2),%ax
	movw	%ax,Ltemp
	movb	spanpackage_t_g+1(%esi),%ah
	movb	(%ebx),%al
	movw	C(d_fadetable16g)(,%eax,2),%ax
	orw		%ax,Ltemp
	movb	spanpackage_t_b+1(%esi),%ah
	addl	$(spanpackage_t_size),%esi	// point to next span
	movb	(%ebx),%al
	movw	C(d_fadetable16b)(,%eax,2),%ax
	orw		Ltemp,%ax
	movb	C(r_rgbtable)(,%eax,2),%al
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
	movw	C(d_fadetable16r)(,%eax,2),%ax
	movw	%ax,Ltemp
	movb	gb+1,%ah
	movb	(%esi),%al
	movw	C(d_fadetable16g)(,%eax,2),%ax
	orw		%ax,Ltemp
	movb	gb+3,%ah
	movb	(%esi),%al
	movw	C(d_fadetable16b)(,%eax,2),%ax
	orw		Ltemp,%ax
	movb	C(r_rgbtable)(,%eax,2),%ah
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
	movw	C(d_fadetable16r)(,%eax,2),%ax
	movw	%ax,Ltemp
	movb	gb+1,%ah
	movb	(%esi),%al
	movw	C(d_fadetable16g)(,%eax,2),%ax
	orw		%ax,Ltemp
	movb	gb+3,%ah
	movb	(%esi),%al
	movw	C(d_fadetable16b)(,%eax,2),%ax
	orw		Ltemp,%ax
	movb	C(r_rgbtable)(,%eax,2),%ah
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
	movw	C(d_fadetable16r)(,%eax,2),%ax
	movw	%ax,Ltemp
	movb	gb+1,%ah
	movb	(%esi),%al
	movw	C(d_fadetable16g)(,%eax,2),%ax
	orw		%ax,Ltemp
	movb	gb+3,%ah
	movb	(%esi),%al
	movw	C(d_fadetable16b)(,%eax,2),%ax
	orw		Ltemp,%ax
	movb	C(r_rgbtable)(,%eax,2),%ah
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
	movw	C(d_fadetable16r)(,%eax,2),%ax
	movw	%ax,Ltemp
	movb	gb+1,%ah
	movb	(%esi),%al
	movw	C(d_fadetable16g)(,%eax,2),%ax
	orw		%ax,Ltemp
	movb	gb+3,%ah
	movb	(%esi),%al
	movw	C(d_fadetable16b)(,%eax,2),%ax
	orw		Ltemp,%ax
	movb	C(r_rgbtable)(,%eax,2),%ah
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
	movw	C(d_fadetable16r)(,%eax,2),%ax
	movw	%ax,Ltemp
	movb	gb+1,%ah
	movb	(%esi),%al
	movw	C(d_fadetable16g)(,%eax,2),%ax
	orw		%ax,Ltemp
	movb	gb+3,%ah
	movb	(%esi),%al
	movw	C(d_fadetable16b)(,%eax,2),%ax
	orw		Ltemp,%ax
	movb	C(r_rgbtable)(,%eax,2),%ah
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
	movw	C(d_fadetable16r)(,%eax,2),%ax
	movw	%ax,Ltemp
	movb	gb+1,%ah
	movb	(%esi),%al
	movw	C(d_fadetable16g)(,%eax,2),%ax
	orw		%ax,Ltemp
	movb	gb+3,%ah
	movb	(%esi),%al
	movw	C(d_fadetable16b)(,%eax,2),%ax
	orw		Ltemp,%ax
	movb	C(r_rgbtable)(,%eax,2),%ah
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
	movw	C(d_fadetable16r)(,%eax,2),%ax
	movw	%ax,Ltemp
	movb	gb+1,%ah
	movb	(%esi),%al
	movw	C(d_fadetable16g)(,%eax,2),%ax
	orw		%ax,Ltemp
	movb	gb+3,%ah
	movb	(%esi),%al
	movw	C(d_fadetable16b)(,%eax,2),%ax
	orw		Ltemp,%ax
	movb	C(r_rgbtable)(,%eax,2),%ah
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
	movw	C(d_fadetable16r)(,%eax,2),%ax
	movw	%ax,Ltemp
	movb	gb+1,%ah
	movb	(%esi),%al
	movw	C(d_fadetable16g)(,%eax,2),%ax
	orw		%ax,Ltemp
	movb	gb+3,%ah
	movb	(%esi),%al
	movw	C(d_fadetable16b)(,%eax,2),%ax
	orw		Ltemp,%ax
	movb	C(r_rgbtable)(,%eax,2),%ah
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
	movw	C(d_fadetable16r)(,%eax,2),%ax
	movw	%ax,Ltemp
	movb	spanpackage_t_g+1(%esi),%ah
	movb	(%ebx),%al
	movw	C(d_fadetable16g)(,%eax,2),%ax
	orw		%ax,Ltemp
	movb	spanpackage_t_b+1(%esi),%ah
	addl	$(spanpackage_t_size),%esi	// point to next span
	movb	(%ebx),%al
	movw	C(d_fadetable16b)(,%eax,2),%ax
	orw		Ltemp,%ax
	movb	C(r_rgbtable)(,%eax,2),%ah
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
	movw	C(d_fadetable16r)(,%eax,2),%ax
	movw	%ax,Ltemp
	movb	gb+1,%ah
	movb	(%esi),%al
	movw	C(d_fadetable16g)(,%eax,2),%ax
	orw		%ax,Ltemp
	movb	gb+3,%ah
	movb	(%esi),%al
	movw	C(d_fadetable16b)(,%eax,2),%ax
	orw		Ltemp,%ax
	movb	C(r_rgbtable)(,%eax,2),%al
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
	movw	C(d_fadetable16r)(,%eax,2),%ax
	movw	%ax,Ltemp
	movb	gb+1,%ah
	movb	(%esi),%al
	movw	C(d_fadetable16g)(,%eax,2),%ax
	orw		%ax,Ltemp
	movb	gb+3,%ah
	movb	(%esi),%al
	movw	C(d_fadetable16b)(,%eax,2),%ax
	orw		Ltemp,%ax
	movb	C(r_rgbtable)(,%eax,2),%al
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
	movw	C(d_fadetable16r)(,%eax,2),%ax
	movw	%ax,Ltemp
	movb	gb+1,%ah
	movb	(%esi),%al
	movw	C(d_fadetable16g)(,%eax,2),%ax
	orw		%ax,Ltemp
	movb	gb+3,%ah
	movb	(%esi),%al
	movw	C(d_fadetable16b)(,%eax,2),%ax
	orw		Ltemp,%ax
	movb	C(r_rgbtable)(,%eax,2),%al
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
	movw	C(d_fadetable16r)(,%eax,2),%ax
	movw	%ax,Ltemp
	movb	gb+1,%ah
	movb	(%esi),%al
	movw	C(d_fadetable16g)(,%eax,2),%ax
	orw		%ax,Ltemp
	movb	gb+3,%ah
	movb	(%esi),%al
	movw	C(d_fadetable16b)(,%eax,2),%ax
	orw		Ltemp,%ax
	movb	C(r_rgbtable)(,%eax,2),%al
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
	movw	C(d_fadetable16r)(,%eax,2),%ax
	movw	%ax,Ltemp
	movb	gb+1,%ah
	movb	(%esi),%al
	movw	C(d_fadetable16g)(,%eax,2),%ax
	orw		%ax,Ltemp
	movb	gb+3,%ah
	movb	(%esi),%al
	movw	C(d_fadetable16b)(,%eax,2),%ax
	orw		Ltemp,%ax
	movb	C(r_rgbtable)(,%eax,2),%al
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
	movw	C(d_fadetable16r)(,%eax,2),%ax
	movw	%ax,Ltemp
	movb	gb+1,%ah
	movb	(%esi),%al
	movw	C(d_fadetable16g)(,%eax,2),%ax
	orw		%ax,Ltemp
	movb	gb+3,%ah
	movb	(%esi),%al
	movw	C(d_fadetable16b)(,%eax,2),%ax
	orw		Ltemp,%ax
	movb	C(r_rgbtable)(,%eax,2),%al
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
	movw	C(d_fadetable16r)(,%eax,2),%ax
	movw	%ax,Ltemp
	movb	gb+1,%ah
	movb	(%esi),%al
	movw	C(d_fadetable16g)(,%eax,2),%ax
	orw		%ax,Ltemp
	movb	gb+3,%ah
	movb	(%esi),%al
	movw	C(d_fadetable16b)(,%eax,2),%ax
	orw		Ltemp,%ax
	movb	C(r_rgbtable)(,%eax,2),%al
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
	movw	C(d_fadetable16r)(,%eax,2),%ax
	movw	%ax,Ltemp
	movb	gb+1,%ah
	movb	(%esi),%al
	movw	C(d_fadetable16g)(,%eax,2),%ax
	orw		%ax,Ltemp
	movb	gb+3,%ah
	movb	(%esi),%al
	movw	C(d_fadetable16b)(,%eax,2),%ax
	orw		Ltemp,%ax
	movb	C(r_rgbtable)(,%eax,2),%al
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
	movw	C(d_fadetable16r)(,%eax,2),%ax
	movw	%ax,Ltemp
	movb	spanpackage_t_g+1(%esi),%ah
	movb	(%ebx),%al
	movw	C(d_fadetable16g)(,%eax,2),%ax
	orw		%ax,Ltemp
	movb	spanpackage_t_b+1(%esi),%ah
	addl	$(spanpackage_t_size),%esi	// point to next span
	movb	(%ebx),%al
	movw	C(d_fadetable16b)(,%eax,2),%ax
	orw		Ltemp,%ax
	movb	C(r_rgbtable)(,%eax,2),%al
	movb	(%edi),%ah
	addl	C(d_transluc),%eax
	movb	(%eax),%al
	movb	%al,(%edi)

	jmp		LRGBAltFuzzNextSpanESISet

#endif
