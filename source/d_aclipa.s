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

#define pfv0		20+0
#define pfv1		20+4
#define outv		20+8
#define pstv0		20+12
#define pstv1		20+16
#define outstv		20+20

.text

	Align4
.globl C(D_Alias_clip_left)
C(D_Alias_clip_left):
	movl	$fv_u,%eax		// Clip u
	fldz					// against 0
LDoForwardOrBackward:
	// Preserve registers
	pushl	%ebp
	pushl	%edi
	pushl	%esi
	pushl	%ebx
	// Get params
	movl	pfv0(%esp),%edx
	movl	pfv1(%esp),%ecx
	movl	pstv0(%esp),%ebx
	movl	pstv1(%esp),%esi
	// Choose direction
	movl	fv_v(%ecx),%ebp
	cmpl	%ebp,fv_v(%edx)
	jge		LDoForward
	// Swap vertices
	movl	%edx,%ecx
	movl	%ebx,%esi
	movl	pfv1(%esp),%edx
	movl	pstv1(%esp),%ebx
LDoForward:
	// Perform clipping
	movl	outv(%esp),%ebp
	movl	outstv(%esp),%edi
// %eax - offset in finalvert
// %ebx - st0
// %ecx - v1
// %edx - v0
// %esi - st1
// %edi - stout
// %ebp - out
	fildl	(%edx,%eax)		// v0? | ?
	fildl	(%edx,%eax)		// v0? | v0? | ?
	fxch	%st(2)			// ? | v0? | v0?
	fsubp	%st,%st(1)		// ? - v0? | v0?
	fildl	(%ecx,%eax)		// v1? | ? - v0? | v0?
	fsubp	%st,%st(2)		// ? - v0? | v1? - v0?
	fdivp	%st,%st(1)		// scale

	fildl	fv_u(%edx)		// v0u | scale
	fildl	fv_u(%ecx)		// v1u | v0u | scale
	fildl	fv_v(%edx)		// v0v | v1u | v0u | scale
	fildl	fv_v(%ecx)		// v1v | v0v | v1u | v0u | scale
	fildl	fv_zi(%edx)		// v0zi | v1v | v0v | v1u | v0u | scale
	fildl	fv_zi(%ecx)		// v1zi | v0zi | v1v | v0v | v1u | v0u | scale
	fxch	%st(5)			// v0u | v0zi | v1v | v0v | v1u | v1zi | scale
	fsubr	%st(0),%st(4)	// v0u | v0zi | v1v | v0v | v1u - v0u | v1zi |
							// scale
	fxch	%st(3)			// v0v | v0zi | v1v | v0u | v1u - v0u | v1zi |
							// scale
	fsubr	%st(0),%st(2)	// v0v | v0zi | v1v - v0v | v0u | v1u - v0u |
							// v1zi | scale
	fxch	%st(1)			// v0zi | v0v | v1v - v0v | v0u | v1u - v0u |
							// v1zi | scale
	fsubr	%st(0),%st(5)	// v0zi | v0v | v1v - v0v | v0u | v1u - v0u |
							// v1zi - v0zi | scale
	fxch	%st(6)			// scale | v0v | v1v - v0v | v0u | v1u - v0u |
							// v1zi - v0zi | v0zi
	fmul	%st(0),%st(4)	// scale | v0v | v1v - v0v | v0u |
							// (v1u - v0u) * scale | v1zi - v0zi | v0zi
	fmul	%st(0),%st(2)	// scale | v0v | (v1v - v0v) * scale | v0u |
							// (v1u - v0u) * scale | v1zi - v0zi | v0zi
	fmul	%st(0),%st(5)	// scale | v0v | (v1v - v0v) * scale | v0u |
							// (v1u - v0u) * scale | (v1zi - v0zi) * scale |
							// v0zi
	fxch	%st(3)			// v0u | v0v | (v1v - v0v) * scale | scale |
							// (v1u - v0u) * scale | (v1zi - v0zi) * scale |
							// v0zi
	faddp	%st(0),%st(4)	// v0v | (v1v - v0v) * scale | scale |
							// v0u + (v1u - v0u) * scale |
							// (v1zi - v0zi) * scale | v0zi
	faddp	%st(0),%st(1)	// v0v + (v1v - v0v) * scale | scale |
							// v0u + (v1u - v0u) * scale |
							// (v1zi - v0zi) * scale | v0zi
	fxch	%st(4)			// v0zi | scale | v0u + (v1u - v0u) * scale |
							// (v1zi - v0zi) * scale |
							// v0v + (v1v - v0v) * scale
	faddp	%st(0),%st(3)	// scale | v0u + (v1u - v0u) * scale |
							// v0zi + (v1zi - v0zi) * scale |
							// v0v + (v1v - v0v) * scale
	fxch	%st(1)			// v0u + (v1u - v0u) * scale | scale |
							// v0zi + (v1zi - v0zi) * scale |
							// v0v + (v1v - v0v) * scale
	fadds	float_point5	// v0u + (v1u - v0u) * scale + 0.5 | scale |
							// v0zi + (v1zi - v0zi) * scale |
							// v0v + (v1v - v0v) * scale
	fxch	%st(3)			// v0v + (v1v - v0v) * scale | scale |
							// v0zi + (v1zi - v0zi) * scale |
							// v0u + (v1u - v0u) * scale + 0.5
	fadds	float_point5	// v0v + (v1v - v0v) * scale + 0.5 | scale |
							// v0zi + (v1zi - v0zi) * scale |
							// v0u + (v1u - v0u) * scale + 0.5
	fxch	%st(2)			// v0zi + (v1zi - v0zi) * scale | scale |
							// v0v + (v1v - v0v) * scale + 0.5 |
							// v0u + (v1u - v0u) * scale + 0.5
	fadds	float_point5	// v0zi + (v1zi - v0zi) * scale + 0.5 | scale |
							// v0v + (v1v - v0v) * scale + 0.5 |
							// v0u + (v1u - v0u) * scale + 0.5
	fxch	%st(3)			// v0u + (v1u - v0u) * scale + 0.5 | scale |
							// v0v + (v1v - v0v) * scale + 0.5 |
							// v0zi + (v1zi - v0zi) * scale + 0.5
	fistpl	fv_u(%ebp)		// scale | v0v + (v1v - v0v) * scale + 0.5 |
							// v0zi + (v1zi - v0zi) * scale + 0.5
	fxch	%st(1)			// v0v + (v1v - v0v) * scale + 0.5 | scale |
							// v0zi + (v1zi - v0zi) * scale + 0.5
	fistpl	fv_v(%ebp)		// scale | v0zi + (v1zi - v0zi) * scale + 0.5
	fxch	%st(1)			// v0zi + (v1zi - v0zi) * scale + 0.5 | scale
	fistpl	fv_zi(%ebp)		// scale

	fildl	fv_r(%edx)		// v0r | scale
	fildl	fv_r(%ecx)		// v1r | v0r | scale
	fildl	fv_g(%edx)		// v0g | v1r | v0r | scale
	fildl	fv_g(%ecx)		// v1g | v0g | v1r | v0r | scale
	fildl	fv_b(%edx)		// v0b | v1g | v0g | v1r | v0r | scale
	fildl	fv_b(%ecx)		// v1b | v0b | v1g | v0g | v1r | v0r | scale
	fxch	%st(5)			// v0r | v0b | v1g | v0g | v1r | v1b | scale
	fsubr	%st(0),%st(4)	// v0r | v0b | v1g | v0g | v1r - v0r | v1b |
							// scale
	fxch	%st(3)			// v0g | v0b | v1g | v0r | v1r - v0r | v1b |
							// scale
	fsubr	%st(0),%st(2)	// v0g | v0b | v1g - v0g | v0r | v1r - v0r |
							// v1b | scale
	fxch	%st(1)			// v0b | v0g | v1g - v0g | v0r | v1r - v0r |
							// v1b | scale
	fsubr	%st(0),%st(5)	// v0b | v0g | v1g - v0g | v0r | v1r - v0r |
							// v1b - v0b | scale
	fxch	%st(6)			// scale | v0g | v1g - v0g | v0r | v1r - v0r |
							// v1b - v0b | v0b
	fmul	%st(0),%st(4)	// scale | v0g | v1g - v0g | v0r |
							// (v1r - v0r) * scale | v1b - v0b | v0b
	fmul	%st(0),%st(2)	// scale | v0g | (v1g - v0g) * scale | v0r |
							// (v1r - v0r) * scale | v1b - v0b | v0b
	fmul	%st(0),%st(5)	// scale | v0g | (v1g - v0g) * scale | v0r |
							// (v1r - v0r) * scale | (v1b - v0b) * scale |
							// v0b
	fxch	%st(3)			// v0r | v0g | (v1g - v0g) * scale | scale |
							// (v1r - v0r) * scale | (v1b - v0b) * scale |
							// v0b
	faddp	%st(0),%st(4)	// v0g | (v1g - v0g) * scale | scale |
							// v0r + (v1r - v0r) * scale |
							// (v1b - v0b) * scale | v0b
	faddp	%st(0),%st(1)	// v0g + (v1g - v0g) * scale | scale |
							// v0r + (v1r - v0r) * scale |
							// (v1b - v0b) * scale | v0b
	fxch	%st(4)			// v0b | scale | v0r + (v1r - v0r) * scale |
							// (v1b - v0b) * scale |
							// v0g + (v1g - v0g) * scale
	faddp	%st(0),%st(3)	// scale | v0r + (v1r - v0r) * scale |
							// v0b + (v1b - v0b) * scale |
							// v0g + (v1g - v0g) * scale
	fxch	%st(1)			// v0r + (v1r - v0r) * scale | scale |
							// v0b + (v1b - v0b) * scale |
							// v0g + (v1g - v0g) * scale
	fadds	float_point5	// v0r + (v1r - v0r) * scale + 0.5 | scale |
							// v0b + (v1b - v0b) * scale |
							// v0g + (v1g - v0g) * scale
	fxch	%st(3)			// v0g + (v1g - v0g) * scale | scale |
							// v0b + (v1b - v0b) * scale |
							// v0r + (v1r - v0r) * scale + 0.5
	fadds	float_point5	// v0g + (v1g - v0g) * scale + 0.5 | scale |
							// v0b + (v1b - v0b) * scale |
							// v0r + (v1r - v0r) * scale + 0.5
	fxch	%st(2)			// v0b + (v1b - v0b) * scale | scale |
							// v0g + (v1g - v0g) * scale + 0.5 |
							// v0r + (v1r - v0r) * scale + 0.5
	fadds	float_point5	// v0b + (v1b - v0b) * scale + 0.5 | scale |
							// v0g + (v1g - v0g) * scale + 0.5 |
							// v0r + (v1r - v0r) * scale + 0.5
	fxch	%st(3)			// v0r + (v1r - v0r) * scale + 0.5 | scale |
							// v0g + (v1g - v0g) * scale + 0.5 |
							// v0b + (v1b - v0b) * scale + 0.5
	fistpl	fv_r(%ebp)		// scale | v0g + (v1g - v0g) * scale + 0.5 |
							// v0b + (v1b - v0b) * scale + 0.5
	fxch	%st(1)			// v0g + (v1g - v0g) * scale + 0.5 | scale |
							// v0b + (v1b - v0b) * scale + 0.5
	fistpl	fv_g(%ebp)		// scale | v0b + (v1b - v0b) * scale + 0.5
	fxch	%st(1)			// v0b + (v1b - v0b) * scale + 0.5 | scale
	fistpl	fv_b(%ebp)		// scale

	fildl	fstv_s(%ebx)	// st0s | scale
	fildl	fstv_s(%esi)	// st1s | st0s | scale
	fildl	fstv_t(%ebx)	// st0t | st1s | st0s | scale
	fildl	fstv_t(%esi)	// st1t | st0t | st1s | st0s | scale
	fxch	%st(3)			// st0s | st0t | st1s | st1t | scale
	fsubr	%st(0),%st(2)	// st0s | st0t | st1s - st0s | st1t | scale
	fxch	%st(1)			// st0t | st0s | st1s - st0s | st1t | scale
	fsubr	%st(0),%st(3)	// st0t | st0s | st1s - st0s | st1t - st0t |
							// scale
	fxch	%st(4)			// scale | st0s | st1s - st0s | st1t - st0t |
							// st0t
//STALL
	fmul	%st(0),%st(2)	// scale | st0s | (st1s - st0s) * scale |
							// st1t - st0t | st0t
	fmulp	%st(0),%st(3)	// st0s | (st1s - st0s) * scale |
							// (st1t - st0t) * scale | st0t
//STALL
	faddp	%st(0),%st(1)	// st0s + (st1s - st0s) * scale |
							// (st1t - st0t) * scale | st0t
	fxch	%st(2)			// st0t | (st1t - st0t) * scale |
							// st0s + (st1s - st0s) * scale
	faddp	%st(0),%st(1)	// st0t + (st1t - st0t) * scale |
							// st0s + (st1s - st0s) * scale
	fxch	%st(1)			// st0s + (st1s - st0s) * scale |
							// st0t + (st1t - st0t) * scale
//STALL
	fadds	float_point5	// st0s + (st1s - st0s) * scale + 0.5 |
							// st0t + (st1t - st0t) * scale
	fxch	%st(1)			// st0t + (st1t - st0t) * scale |
							// st0s + (st1s - st0s) * scale + 0.5
	fadds	float_point5	// st0t + (st1t - st0t) * scale + 0.5 |
							// st0s + (st1s - st0s) * scale + 0.5
	fxch	%st(1)			// st0s + (st1s - st0s) * scale + 0.5 |
							// st0t + (st1t - st0t) * scale + 0.5
//STALL
	fistpl	fstv_s(%edi)	// st0t + (st1t - st0t) * scale + 0.5
	fistpl	fstv_t(%edi)	//

	// Restore registers
	popl	%ebx
	popl	%esi
	popl	%edi
	popl	%ebp
	ret

	Align4
.globl C(D_Alias_clip_right)
C(D_Alias_clip_right):
	movl	$fv_u,%eax		// Clip u
	fildl	C(viewwidth)	// against viewwidth
	jmp		LDoForwardOrBackward

	Align4
.globl C(D_Alias_clip_top)
C(D_Alias_clip_top):
	movl	$fv_v,%eax		// Clip v
	fldz					// against 0
	jmp		LDoForwardOrBackward

	Align4
.globl C(D_Alias_clip_bottom)
C(D_Alias_clip_bottom):
	movl	$fv_v,%eax		// Clip v
	fildl	C(viewheight)	// against viewheight
	jmp		LDoForwardOrBackward

#endif

//**************************************************************************
//
//	$Log$
//	Revision 1.1  2001/08/15 17:12:23  dj_jl
//	Optimized model drawing
//
//**************************************************************************
