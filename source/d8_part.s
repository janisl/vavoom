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
//**	Copyright (C) 1999-2001 J�nis Legzdi��
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
//
// d_parta.s
// x86 assembly-language 8-bpp particle-drawing code.
//

#include "asm_i386.h"

#ifdef USEASM

//==========================================================================
//
//
//
//==========================================================================

//----------------------------------------------------------------------
// 8-bpp particle drawing code.
//----------------------------------------------------------------------

//FIXME: comments, full optimization

//----------------------------------------------------------------------
// 8-bpp particle queueing code.
//----------------------------------------------------------------------

	.text

#define P	12+4

	.align 4
.globl C(D_DrawParticle_8)
C(D_DrawParticle_8):
	pushl	%ebp				// preserve caller's stack frame
	pushl	%edi				// preserve register variables
	pushl	%ebx

	movl	P(%esp),%edi

// FIXME: better FP overlap in general here

// transform point
//	VectorSubtract (p->org, r_origin, local);
	flds	C(vieworg)
	fsubrs	pt_org(%edi)
	flds	pt_org+4(%edi)
	fsubs	C(vieworg)+4
	flds	pt_org+8(%edi)
	fsubs	C(vieworg)+8
	fxch	%st(2)			// local[0] | local[1] | local[2]

//	transformed[2] = DotProduct(local, r_ppn);		
	flds	C(r_ppn)		// r_ppn[0] | local[0] | local[1] | local[2]
	fmul	%st(1),%st(0)	// dot0 | local[0] | local[1] | local[2]
	flds	C(r_ppn)+4	// r_ppn[1] | dot0 | local[0] | local[1] | local[2]
	fmul	%st(3),%st(0)	// dot1 | dot0 | local[0] | local[1] | local[2]
	flds	C(r_ppn)+8	// r_ppn[2] | dot1 | dot0 | local[0] |
						//  local[1] | local[2]
	fmul	%st(5),%st(0)	// dot2 | dot1 | dot0 | local[0] | local[1] | local[2]
	fxch	%st(2)		// dot0 | dot1 | dot2 | local[0] | local[1] | local[2]
	faddp	%st(0),%st(1) // dot0 + dot1 | dot2 | local[0] | local[1] |
						  //  local[2]
	faddp	%st(0),%st(1) // z | local[0] | local[1] | local[2]
	fld		%st(0)		// z | z | local[0] | local[1] |
						//  local[2]
	fdivrs	float_1		// 1/z | z | local[0] | local[1] | local[2]
	fxch	%st(1)		// z | 1/z | local[0] | local[1] | local[2]

//	if (transformed[2] < PARTICLE_Z_CLIP)
//		return;
	fcomps	float_particle_z_clip	// 1/z | local[0] | local[1] | local[2]
	fxch	%st(3)					// local[2] | local[0] | local[1] | 1/z

	flds	C(r_pup)	// r_pup[0] | local[2] | local[0] | local[1] | 1/z
	fmul	%st(2),%st(0)	// dot0 | local[2] | local[0] | local[1] | 1/z 
	flds	C(r_pup)+4	// r_pup[1] | dot0 | local[2] | local[0] |
						//  local[1] | 1/z 

	fnstsw	%ax
	testb	$1,%ah
	jnz		LPop6AndDone

//	transformed[1] = DotProduct(local, r_pup);
	fmul	%st(4),%st(0)	// dot1 | dot0 | local[2] | local[0] | local[1] | 1/z 
	flds	C(r_pup)+8	// r_pup[2] | dot1 | dot0 | local[2] |
						//  local[0] | local[1] | 1/z 
	fmul	%st(3),%st(0)	// dot2 | dot1 | dot0 | local[2] | local[0] |
						//  local[1] | 1/z 
	fxch	%st(2)		// dot0 | dot1 | dot2 | local[2] | local[0] |
						//  local[1] | 1/z 
	faddp	%st(0),%st(1) // dot0 + dot1 | dot2 | local[2] | local[0] |
						//  local[1] | 1/z 
	faddp	%st(0),%st(1) // y | local[2] | local[0] | local[1] | 1/z 
	fxch	%st(3)		// local[1] | local[2] | local[0] | y | 1/z 

//	transformed[0] = DotProduct(local, r_pright);
	fmuls	C(r_pright)+4	// dot1 | local[2] | local[0] | y | 1/z
	fxch	%st(2)		// local[0] | local[2] | dot1 | y | 1/z
	fmuls	C(r_pright)	// dot0 | local[2] | dot1 | y | 1/z
	fxch	%st(1)		// local[2] | dot0 | dot1 | y | 1/z
	fmuls	C(r_pright)+8	// dot2 | dot0 | dot1 | y | 1/z
	fxch	%st(2)		// dot1 | dot0 | dot2 | y | 1/z
	faddp	%st(0),%st(1) // dot1 + dot0 | dot2 | y | 1/z

	faddp	%st(0),%st(1)	// x | y | 1/z
	fxch	%st(1)			// y | x | 1/z

// project the point
	fmul	%st(2),%st(0)	// y/z | x | 1/z
	fxch	%st(1)			// x | y/z | 1/z
	fmul	%st(2),%st(0)	// x/z | y/z | 1/z
	fxch	%st(1)			// y/z | x/z | 1/z
	fadds	C(centeryfrac)		// v | x/z | 1/z
	fxch	%st(1)			// x/z | v | 1/z
	fadds	C(centerxfrac)		// u | v | 1/z
// FIXME: preadjust xcenter and ycenter
	fxch	%st(1)			// v | u | 1/z
	fadds	float_point5	// v | u | 1/z
	fxch	%st(1)			// u | v | 1/z
	fadds	float_point5	// u | v | 1/z
	fxch	%st(2)			// 1/z | v | u
	fmuls	DP_32768		// 1/z * 0x8000 | v | u
	fxch	%st(2)			// u | v | 1/z * 0x8000

// FIXME: use Terje's fp->int trick here?
// FIXME: check we're getting proper rounding here
	fistpl	DP_u			// v | 1/z * 0x8000
	fistpl	DP_v			// 1/z * 0x8000

	movl	DP_u,%eax
	movl	DP_v,%edx

// if ((v > d_particle_top) || 
// 	(u > d_particle_right) ||
// 	(v < 0) ||
// 	(u < 0))
// {
// 	continue;
// }

	cmpl	$0,%edx
	jl		LPop1AndDone
	cmpl	$0,%eax
	jl		LPop1AndDone
	movl	C(d_particle_top),%ebx
	movl	C(d_particle_right),%ecx
	cmpl	%ebx,%edx
	jg		LPop1AndDone
	cmpl	%ecx,%eax
	jg		LPop1AndDone

	//	Convert color from RBG format into palette index by looking up
	// in RGB table
	movl	pt_color(%edi),%ebx
	movl	%ebx,%ecx
	shrl	$9,%ecx
	movl	%ebx,%ebp
	shrl	$6,%ebp
	andl	$31744,%ecx
	shrl	$3,%ebx
	andl	$992,%ebp
	andl	$31,%ebx
	addl	%ebp,%ecx
	movl	C(d_rgbtable),%ebp
	addl	%ecx,%ebx
	movb	(%ebp,%ebx),%cl
	movb	%cl,DP_Color

	movl	C(scrn),%ebx

	addl	%eax,%ebx
	movl	C(ylookup)(,%edx,4),%edi		// point to the pixel

	leal	(,%edi,2),%edx		// point to the z pixel

	leal	(%edx,%eax,2),%edx
	movl	C(zbuffer),%eax

	fistpl	izi

	addl	%ebx,%edi
	addl	%eax,%edx

// pix = izi >> d_pix_shift;

	movl	izi,%eax
	movl	C(d_pix_shift),%ecx
	shrl	%cl,%eax
	movl	izi,%ebp

// if (pix < d_pix_min)
// 		pix = d_pix_min;
// else if (pix > d_pix_max)
//  	pix = d_pix_max;

	movl	C(d_pix_min),%ebx
	movl	C(d_pix_max),%ecx
	cmpl	%ebx,%eax
	jnl		LTestPixMax
	movl	%ebx,%eax
	jmp		LTestDone

LTestPixMax:
	cmpl	%ecx,%eax
	jng		LTestDone
	movl	%ecx,%eax
LTestDone:

	movb	DP_Color,%ch

	movl	C(d_y_aspect_shift),%ebx
	testl	%ebx,%ebx
	jnz		LDefault

	cmpl	$4,%eax
	ja		LDefault

	jmp		*DP_EntryTable-4(,%eax,4)

DP_EntryTable:
	.long	LDP_1x1
	.long	LDP_2x2
	.long	LDP_3x3
	.long	LDP_4x4

// 1x1
LDP_1x1:
	cmpw	%bp,(%edx)		// just one pixel to do
	jg		LDone
	movw	%bp,(%edx)
	movb	%ch,(%edi)
	jmp		LDone

// 2x2
LDP_2x2:
	pushl	%esi
	movl	C(d_rowbytes),%ebx
	movl	C(d_zrowbytes),%esi

	cmpw	%bp,(%edx)
	jg		L2x2_1
	movw	%bp,(%edx)
	movb	%ch,(%edi)
L2x2_1:
	cmpw	%bp,2(%edx)
	jg		L2x2_2
	movw	%bp,2(%edx)
	movb	%ch,1(%edi)
L2x2_2:
	cmpw	%bp,(%edx,%esi,1)
	jg		L2x2_3
	movw	%bp,(%edx,%esi,1)
	movb	%ch,(%edi,%ebx,1)
L2x2_3:
	cmpw	%bp,2(%edx,%esi,1)
	jg		L2x2_4
	movw	%bp,2(%edx,%esi,1)
	movb	%ch,1(%edi,%ebx,1)
L2x2_4:

	popl	%esi
	jmp		LDone

// 3x3
LDP_3x3:
	pushl	%esi
	movl	C(d_rowbytes),%ebx
	movl	C(d_zrowbytes),%esi

	cmpw	%bp,(%edx)
	jg		L3x3_1
	movw	%bp,(%edx)
	movb	%ch,(%edi)
L3x3_1:
	cmpw	%bp,2(%edx)
	jg		L3x3_2
	movw	%bp,2(%edx)
	movb	%ch,1(%edi)
L3x3_2:
	cmpw	%bp,4(%edx)
	jg		L3x3_3
	movw	%bp,4(%edx)
	movb	%ch,2(%edi)
L3x3_3:

	cmpw	%bp,(%edx,%esi,1)
	jg		L3x3_4
	movw	%bp,(%edx,%esi,1)
	movb	%ch,(%edi,%ebx,1)
L3x3_4:
	cmpw	%bp,2(%edx,%esi,1)
	jg		L3x3_5
	movw	%bp,2(%edx,%esi,1)
	movb	%ch,1(%edi,%ebx,1)
L3x3_5:
	cmpw	%bp,4(%edx,%esi,1)
	jg		L3x3_6
	movw	%bp,4(%edx,%esi,1)
	movb	%ch,2(%edi,%ebx,1)
L3x3_6:

	cmpw	%bp,(%edx,%esi,2)
	jg		L3x3_7
	movw	%bp,(%edx,%esi,2)
	movb	%ch,(%edi,%ebx,2)
L3x3_7:
	cmpw	%bp,2(%edx,%esi,2)
	jg		L3x3_8
	movw	%bp,2(%edx,%esi,2)
	movb	%ch,1(%edi,%ebx,2)
L3x3_8:
	cmpw	%bp,4(%edx,%esi,2)
	jg		L3x3_9
	movw	%bp,4(%edx,%esi,2)
	movb	%ch,2(%edi,%ebx,2)
L3x3_9:

	popl	%esi
	jmp		LDone


// 4x4
LDP_4x4:
	pushl	%esi
	movl	C(d_rowbytes),%ebx
	movl	C(d_zrowbytes),%esi

	cmpw	%bp,(%edx)
	jg		L4x4_1
	movw	%bp,(%edx)
	movb	%ch,(%edi)
L4x4_1:
	cmpw	%bp,2(%edx)
	jg		L4x4_2
	movw	%bp,2(%edx)
	movb	%ch,1(%edi)
L4x4_2:
	cmpw	%bp,4(%edx)
	jg		L4x4_3
	movw	%bp,4(%edx)
	movb	%ch,2(%edi)
L4x4_3:
	cmpw	%bp,6(%edx)
	jg		L4x4_4
	movw	%bp,6(%edx)
	movb	%ch,3(%edi)
L4x4_4:

	cmpw	%bp,(%edx,%esi,1)
	jg		L4x4_5
	movw	%bp,(%edx,%esi,1)
	movb	%ch,(%edi,%ebx,1)
L4x4_5:
	cmpw	%bp,2(%edx,%esi,1)
	jg		L4x4_6
	movw	%bp,2(%edx,%esi,1)
	movb	%ch,1(%edi,%ebx,1)
L4x4_6:
	cmpw	%bp,4(%edx,%esi,1)
	jg		L4x4_7
	movw	%bp,4(%edx,%esi,1)
	movb	%ch,2(%edi,%ebx,1)
L4x4_7:
	cmpw	%bp,6(%edx,%esi,1)
	jg		L4x4_8
	movw	%bp,6(%edx,%esi,1)
	movb	%ch,3(%edi,%ebx,1)
L4x4_8:

	leal	(%edx,%esi,2),%edx
	leal	(%edi,%ebx,2),%edi

	cmpw	%bp,(%edx)
	jg		L4x4_9
	movw	%bp,(%edx)
	movb	%ch,(%edi)
L4x4_9:
	cmpw	%bp,2(%edx)
	jg		L4x4_10
	movw	%bp,2(%edx)
	movb	%ch,1(%edi)
L4x4_10:
	cmpw	%bp,4(%edx)
	jg		L4x4_11
	movw	%bp,4(%edx)
	movb	%ch,2(%edi)
L4x4_11:
	cmpw	%bp,6(%edx)
	jg		L4x4_12
	movw	%bp,6(%edx)
	movb	%ch,3(%edi)
L4x4_12:

	cmpw	%bp,(%edx,%esi,1)
	jg		L4x4_13
	movw	%bp,(%edx,%esi,1)
	movb	%ch,(%edi,%ebx,1)
L4x4_13:
	cmpw	%bp,2(%edx,%esi,1)
	jg		L4x4_14
	movw	%bp,2(%edx,%esi,1)
	movb	%ch,1(%edi,%ebx,1)
L4x4_14:
	cmpw	%bp,4(%edx,%esi,1)
	jg		L4x4_15
	movw	%bp,4(%edx,%esi,1)
	movb	%ch,2(%edi,%ebx,1)
L4x4_15:
	cmpw	%bp,6(%edx,%esi,1)
	jg		L4x4_16
	movw	%bp,6(%edx,%esi,1)
	movb	%ch,3(%edi,%ebx,1)
L4x4_16:

	popl	%esi
	jmp		LDone

// default case, handling any size particle
LDefault:

// count = pix << d_y_aspect_shift;

	movl	%eax,%ebx
	movl	%eax,DP_Pix
	movb	C(d_y_aspect_shift),%cl
	shll	%cl,%ebx

// for ( ; count ; count--, pz += d_zwidth, pdest += screenwidth)
// {
// 	for (i=0 ; i<pix ; i++)
// 	{
// 		if (pz[i] <= izi)
// 		{
// 			pz[i] = izi;
// 			pdest[i] = color;
// 		}
// 	}
// }

LGenRowLoop:
	movl	DP_Pix,%eax

LGenColLoop:
	cmpw	%bp,-2(%edx,%eax,2)
	jg		LGSkip
	movw	%bp,-2(%edx,%eax,2)
	movb	%ch,-1(%edi,%eax,1)
LGSkip:
	decl	%eax			// --pix
	jnz		LGenColLoop

	addl	C(d_zrowbytes),%edx
	addl	C(d_rowbytes),%edi

	decl	%ebx			// --count
	jnz		LGenRowLoop

LDone:
	popl	%ebx				// restore register variables
	popl	%edi
	popl	%ebp				// restore the caller's stack frame
	ret

LPop6AndDone:
	fstp	%st(0)
	fstp	%st(0)
	fstp	%st(0)
	fstp	%st(0)
	fstp	%st(0)
LPop1AndDone:
	fstp	%st(0)
	jmp		LDone

#endif

//**************************************************************************
//
//	$Log$
//	Revision 1.3  2001/07/31 17:16:30  dj_jl
//	Just moved Log to the end of file
//
//	Revision 1.2  2001/07/27 14:27:54  dj_jl
//	Update with Id-s and Log-s, some fixes
//
//**************************************************************************
