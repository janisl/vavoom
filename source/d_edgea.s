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
	Align4
Lfp_near_clip:	.single	0.01
Lfp_1m:			.single	1048576.0
Lfp_1m_minus_1:	.single	1048575.0
Lu0:			.long	0
Lv0:			.long	0
Ltemp:			.long	0
Ld0:			.long	0
Ld1:			.long	0

.text

#define pv0			20+0
#define pv1			20+4
#define clip		20+8
#define clipflags	20+12

//==========================================================================
//
//	D_ClipEdge
//
//==========================================================================

.globl C(D_ClipEdge)
C(D_ClipEdge):
	pushl	%ebp
	pushl	%edi
	pushl	%esi
	pushl	%ebx

	movl	pv0(%esp),%esi
	movl	pv1(%esp),%edi
	movl	clip(%esp),%ebx
	movl	clipflags(%esp),%ebp

	testl	%ebx,%ebx
	jz		LEmitEdge

LClipLoop:
	movl	%ebp,%eax
	movl	20(%ebx),%ecx
	testl	%ecx,%eax
	jz		LNoClip

	flds	(%esi)			// pv0[0]
	fmuls	(%ebx)			// d0accum0
	flds	4(%esi)			// pv0[1] | d0accum0
	fmuls	4(%ebx)			// d0accum1 | d0accum0
	flds	8(%esi)			// pv0[2] | d0accum1 | d0accum0
	fmuls	8(%ebx)			// d0accum2 | d0accum1 | d0accum0
	fxch	%st(2)			// d0accum0 | d0accum1 | d0accum2
	faddp	%st(0),%st(1)	// d0accum3 | d0accum2
	flds	(%edi)			// pv1[0] | d0accum3 | d0accum2
	fmuls	(%ebx)			// d1accum0 | d0accum3 | d0accum2
	flds	4(%edi)			// pv1[1] | d1accum0 | d0accum3 | d0accum2
	fmuls	4(%ebx)			// d1accum1 | d1accum0 | d0accum3 | d0accum2
	flds	8(%edi)			// pv1[2] | d1accum1 | d1accum0 | d0accum3 |
							// d0accum2
	fmuls	8(%ebx)			// d1accum2 | d1accum1 | d1accum0 | d0accum3 |
							// d0accum2
	fxch	%st(2)			// d1accum0 | d1accum1 | d1accum2 | d0accum3 |
							// d0accum2
	faddp	%st(0),%st(1)	// d1accum3 | d1accum2 | d0accum3 | d0accum2
	fxch	%st(3)			// d0accum2 | d1accum2 | d0accum3 | d1accum3
	fsubs	12(%ebx)		// d0accum2 - dist | d1accum2 | d0accum3 |
							// d1accum3
	fxch	%st(1)			// d1accum2 | d0accum2 - dist | d0accum3 |
							// d1accum3
	fsubs	12(%ebx)		// d1accum2 - dist | d0accum2 - dist |
							// d0accum3 | d1accum3
	fxch	%st(1)			// d0accum2 - dist | d1accum2 - dist |
							// d0accum3 | d1accum3
	faddp	%st(0),%st(2)	// d1accum2 - dist | d0 | d1accum3
	faddp	%st(0),%st(2)	// d0 | d1
	fstps	Ld0				// d1
	fstps	Ld1				//

	movl	Ld0,%eax
	movl	Ld1,%ecx
	orl		%eax,%ecx
	js		LIsClipped

LNoClip:
	movl	16(%ebx),%ebx
	testl	%ebx,%ebx
	jne		LClipLoop

LEmitEdge:

	fldcw	ceil_cw

	cmpl	$0,C(d_lastvertvalid)
	je		LTransformAndProjectV0

	movl	C(d_u1),%eax
	movl	C(d_v1),%edx
	movl	%eax,Lu0
	movl	%edx,Lv0
	movl	C(d_ceilv1),%ebx
	jmp		LTransformAndProjectV1

	// ----------

LTransformAndProjectV0:
	flds	(%esi)				// v[0]
	fsubs	C(vieworg)+0		// local[0]
	flds	4(%esi)				// v[1] | local[0]
	fsubs	C(vieworg)+4		// local[1] | local[0]
	flds	8(%esi)				// v[2] | local[1] | local[0]
	fsubs	C(vieworg)+8		// local[2] | local[1] | local[0]
	fld		%st(2)				// local[0] | local[2] | local[1] | local[0]
	fmuls	C(viewforward)+0	// zm0 | local[2] | local[1] | local[0]
	fld		%st(3)				// local[0] | zm0 | local[2] | local[1] |
								// local[0]
	fmuls	C(viewright)		// xm0 | zm0 | local[2] | local[1] | local[0]
	fxch	%st(4)				// local[0] | zm0 | local[2] | local[1] | xm0
	fmuls	C(viewup)			// ym0 | zm0 | local[2] | local[1] | xm0
	fld		%st(3)				// local[1] | ym0 | zm0 | local[2] |
								// local[1] | xm0
	fmuls	C(viewforward)+4	// zm1 | ym0 | zm0 | local[2] | local[1] |
								// xm0
	fld		%st(4)				// local[1] | zm1 | ym0 | zm0 | local[2] |
								// local[1] | xm0
	fmuls	C(viewright)+4		// xm1 | zm1 | ym0 | zm0 | local[2] |
								// local[1] | xm0
	fxch	%st(5)				// local[1] | zm1 | ym0 | zm0 | local[2] |
								// xm1 | xm0
	fmuls	C(viewup)+4			// ym1 | zm1 | ym0 | zm0 | local[2] | xm1 |
								// xm0
	fxch	%st(3)				// zm0 | zm1 | ym0 | ym1 | local[2] | xm1 |
								// xm0
	faddp	%st(0),%st(1)		// zm3 | ym0 | ym1 | local[2] | xm1 | xm0
	fxch	%st(5)				// xm0 | ym0 | ym1 | local[2] | xm1 | zm3
	faddp	%st(0),%st(4)		// ym0 | ym1 | local[2] | xm3 | zm3
	faddp	%st(0),%st(1)		// ym3 | local[2] | xm3 | zm3
	fld		%st(1)				// local[2] | ym3 | local[2] | xm3 | zm3
	fmuls	C(viewforward)+8	// zm2 | ym3 | local[2] | xm3 | zm3
	fld		%st(2)				// local[2] | zm2 | ym3 | local[2] | xm3 | zm3
	fmuls	C(viewright)+8		// xm2 | zm2 | ym3 | local[2] | xm3 | zm3
	fxch	%st(3)				// local[2] | zm2 | ym3 | xm2 | xm3 | zm3
	fmuls	C(viewup)+8			// ym2 | zm2 | ym3 | xm2 | xm3 | zm3
	fxch	%st(1)				// zm2 | ym2 | ym3 | xm2 | xm3 | zm3
	faddp	%st(0),%st(5)		// ym2 | ym3 | xm2 | xm3 | zm4
	fxch	%st(2)				// xm2 | ym3 | ym2 | xm3 | zm4
	faddp	%st(0),%st(3)		// ym3 | ym2 | xm4 | zm4
	faddp	%st(0),%st(1)		// ym4 | xm4 | zm4
	fxch	%st(2)				// zm4 | xm4 | ym4

	fcoms	Lfp_near_clip
	fnstsw	%ax
	testb	$1,%ah
	jz		LNoClip0
	fstp	%st(0)
	flds	Lfp_near_clip
LNoClip0:

	fld1						// 1 | zm4 | xm4 | ym4
	fdivp	%st(0),%st(1)		// zi | xm4 | ym4

	movl	(%esi),%eax
	movl	%eax,C(firstvert)+0
	movl	4(%esi),%eax
	movl	%eax,C(firstvert)+4
	movl	8(%esi),%eax
	movl	%eax,C(firstvert)+8

	fcoms	C(r_nearzi)
	fnstsw	%ax
	testb	$0x45,%ah
	jnz		LNoSetNearZi0
	fsts	C(r_nearzi)
LNoSetNearZi0:

	fmul	%st(0),%st(1)		// zi | xm4*zi | ym4
	fmulp	%st(0),%st(2)		// xm4*zi | ym4*zi
	fmuls	C(xprojection)		// xm4*zi*xprojection | ym4*zi
	fxch	%st(1)				// ym4*zi | xm4*zi*xprojection
	fmuls	C(yprojection)		// ym4*zi*yprojection | xm4*zi*xprojection
	fxch	%st(1)				// xm4*zi*xprojection | ym4*zi*yprojection
	fadds	C(centerxfrac)		// u0 | ym4*zi*yprojection
	fxch	%st(1)				// ym4*zi*yprojection | u0
	fadds	C(centeryfrac)		// v0 | u0
	fxch	%st(1)				// u0 | v0

	fcoms	C(vrectx_adj)
	fnstsw	%ax
	testb	$1,%ah
	jz		LClampP00
	fstp	%st(0)
	flds	C(vrectx_adj)
LClampP00:

	fcoms	C(vrectw_adj)
	fnstsw	%ax
	testb	$0x45,%ah
	jnz		LClampP10
	fstp	%st(0)
	flds	C(vrectw_adj)
LClampP10:

	fxch	%st(1)				// v0 | u0

	fcoms	C(vrecty_adj)
	fnstsw	%ax
	testb	$1,%ah
	jz		LClampP20
	fstp	%st(0)
	flds	C(vrecty_adj)
LClampP20:

	fcoms	C(vrecth_adj)
	fnstsw	%ax
	testb	$0x45,%ah
	jnz		LClampP30
	fstp	%st(0)
	flds	C(vrecth_adj)
LClampP30:

	fistl	Ltemp
	movl	Ltemp,%ebx
	fstps	Lv0
	fstps	Lu0

	// ----------

LTransformAndProjectV1:
	flds	(%edi)				// v[0]
	fsubs	C(vieworg)+0		// local[0]
	flds	4(%edi)				// v[1] | local[0]
	fsubs	C(vieworg)+4		// local[1] | local[0]
	flds	8(%edi)				// v[2] | local[1] | local[0]
	fsubs	C(vieworg)+8		// local[2] | local[1] | local[0]
	fld		%st(2)				// local[0] | local[2] | local[1] | local[0]
	fmuls	C(viewforward)+0	// zm0 | local[2] | local[1] | local[0]
	fld		%st(3)				// local[0] | zm0 | local[2] | local[1] |
								// local[0]
	fmuls	C(viewright)+0		// xm0 | zm0 | local[2] | local[1] | local[0]
	fxch	%st(4)				// local[0] | zm0 | local[2] | local[1] | xm0
	fmuls	C(viewup)+0			// ym0 | zm0 | local[2] | local[1] | xm0
	fld		%st(3)				// local[1] | ym0 | zm0 | local[2] |
								// local[1] | xm0
	fmuls	C(viewforward)+4	// zm1 | ym0 | zm0 | local[2] | local[1] |
								// xm0
	fld		%st(4)				// local[1] | zm1 | ym0 | zm0 | local[2] |
								// local[1] | xm0
	fmuls	C(viewright)+4		// xm1 | zm1 | ym0 | zm0 | local[2] |
								// local[1] | xm0
	fxch	%st(5)				// local[1] | zm1 | ym0 | zm0 | local[2] |
								// xm1 | xm0
	fmuls	C(viewup)+4			// ym1 | zm1 | ym0 | zm0 | local[2] | xm1 |
								// xm0
	fxch	%st(3)				// zm0 | zm1 | ym0 | ym1 | local[2] | xm1 |
								// xm0
	faddp	%st(0),%st(1)		// zm3 | ym0 | ym1 | local[2] | xm1 | xm0
	fxch	%st(5)				// xm0 | ym0 | ym1 | local[2] | xm1 | zm3
	faddp	%st(0),%st(4)		// ym0 | ym1 | local[2] | xm3 | zm3
	faddp	%st(0),%st(1)		// ym3 | local[2] | xm3 | zm3
	fld		%st(1)				// local[2] | ym3 | local[2] | xm3 | zm3
	fmuls	C(viewforward)+8	// zm2 | ym3 | local[2] | xm3 | zm3
	fld		%st(2)				// local[2] | zm2 | ym3 | local[2] | xm3 | zm3
	fmuls	C(viewright)+8		// xm2 | zm2 | ym3 | local[2] | xm3 | zm3
	fxch	%st(3)				// local[2] | zm2 | ym3 | xm2 | xm3 | zm3
	fmuls	C(viewup)+8			// ym2 | zm2 | ym3 | xm2 | xm3 | zm3
	fxch	%st(1)				// zm2 | ym2 | ym3 | xm2 | xm3 | zm3
	faddp	%st(0),%st(5)		// ym2 | ym3 | xm2 | xm3 | zm4
	fxch	%st(2)				// xm2 | ym3 | ym2 | xm3 | zm4
	faddp	%st(0),%st(3)		// ym3 | ym2 | xm4 | zm4
	faddp	%st(0),%st(1)		// ym4 | xm4 | zm4
	fxch	%st(2)				// zm4 | xm4 | ym4

	fcoms	Lfp_near_clip
	fnstsw	%ax
	testb	$1,%ah
	jz		LNoClip1
	fstp	%st(0)
	flds	Lfp_near_clip
LNoClip1:

	fld1						// 1 | zm4 | xm4 | ym4
	fdivp	%st(0),%st(1)		// zi | xm4 | ym4

	fcoms	C(r_nearzi)
	fnstsw	%ax
	testb	$0x45,%ah
	jnz		LNoSetNearZi1
	fsts	C(r_nearzi)
LNoSetNearZi1:

	fmul	%st(0),%st(1)		// zi | xm4*zi | ym4
	fmulp	%st(0),%st(2)		// xm4*zi | ym4*zi
	fmuls	C(xprojection)		// xm4*zi*xprojection | ym4*zi
	fxch	%st(1)				// ym4*zi | xm4*zi*xprojection
	fmuls	C(yprojection)		// ym4*zi*yprojection | xm4*zi*xprojection
	fxch	%st(1)				// xm4*zi*xprojection | ym4*zi*yprojection
	fadds	C(centerxfrac)		// u1 | ym4*zi*yprojection
	fxch	%st(1)				// ym4*zi*yprojection | u1
	fadds	C(centeryfrac)		// v1 | u1
	fxch	%st(1)				// u1 | v1

	fcoms	C(vrectx_adj)
	fnstsw	%ax
	testb	$1,%ah
	jz		LClampP01
	fstp	%st(0)
	flds	C(vrectx_adj)
LClampP01:

	fcoms	C(vrectw_adj)
	fnstsw	%ax
	testb	$0x45,%ah
	jnz		LClampP11
	fstp	%st(0)
	flds	C(vrectw_adj)
LClampP11:

	fxch	%st(1)				// v1 | u1

	fcoms	C(vrecty_adj)
	fnstsw	%ax
	testb	$1,%ah
	jz		LClampP21
	fstp	%st(0)
	flds	C(vrecty_adj)
LClampP21:

	fcoms	C(vrecth_adj)
	fnstsw	%ax
	testb	$0x45,%ah
	jnz		LClampP31
	fstp	%st(0)
	flds	C(vrecth_adj)
LClampP31:

	fistl	C(d_ceilv1)			// v1 | u1
	fxch	%st(1)				// u1 | v1
	fsts	C(d_u1)				// u1 | v1
	fxch	%st(1)				// v1 | u1
	fsts	C(d_v1)				// v1 | u1

	flds	Lv0					// v0 | v1 | u1
	flds	Lu0					// u0 | v0 | v1 | u1

	fldcw	single_cw

	// ----------

	movl	C(d_ceilv1),%eax
	movl	$1,C(d_lastvertvalid)
	cmpl	%eax,%ebx
	je		LPop4AndDone

	movl	C(edge_p),%esi
	movl	$1,C(r_emited)
	addl	$24,C(edge_p)
	cmpl	%eax,%ebx
	jle		LEmitLeadingEdge

	fsub	%st(3),%st(0)		// u0 - u1 | v0 | v1 | u1
	fxch	%st(1)				// v0 | u0 - u1 | v1 | u1
	fsub	%st(2),%st(0)		// v0 - v1 | u0 - u1 | v1 | u1
	fdivrp	%st(0),%st(1)		// u_step | v1 | u1
	movl	%eax,%edx
	movl	C(surface_p),%eax
	subl	$C(surfaces),%eax
	sarl	$6,%eax
	movw	$0,18(%esi)
	movw	%ax,16(%esi)
	movl	%edx,Ltemp
	leal	-1(%ebx),%ebp
	fildl	Ltemp				// ceilv0 | u_step | v1 | u1
	fsubp	%st(0),%st(2)		// u_step | ceilv0 - v1 | u1
	fmul	%st(0),%st(1)		// u_step | u_step * (ceilv1 - v1) | u1
	fxch	%st(2)				// u1 | u_step * (ceilv1 - v1) | u_step
	faddp	%st(0),%st(1)		// u | u_step
	jmp		LSideDone

LEmitLeadingEdge:
	fsubr	%st(0),%st(3)		// u0 | v0 | v1 | u1 - u0
	fxch	%st(2)				// v1 | v0 | u0 | u1 - u0
	fsub	%st(1),%st(0)		// v1 - v0 | v0 | u0 | u1 - u0
	fdivrp	%st(0),%st(3)		// v0 | u0 | u_step
	leal	-1(%eax),%ebp
	movl	C(surface_p),%eax
	subl	$C(surfaces),%eax
	sarl	$6,%eax
	movl	%ebx,%edx
	movw	$0,16(%esi)
	movw	%ax,18(%esi)
	movl	%edx,Ltemp
	fildl	Ltemp				// ceilv0 | v0 | u0 | u_step
	fsubp	%st(0),%st(1)		// ceilv0 - v0 | u0 | u_step
	fmul	%st(2),%st(0)		// (ceilv0 - v0) * u_step | u0 | u_step
	faddp	%st(0),%st(1)		// u | u_step

LSideDone:
	fmuls	Lfp_1m				// u*0x100000 | u_step
	fxch	%st(1)				// u_step | u*0x100000
	fmuls	Lfp_1m				// u_step*0x100000 | u*0x100000
	fxch	%st(1)				// u*0x100000 | u_step*0x100000
	fadds	Lfp_1m_minus_1		// u*0x100000+0xfffff | u_step*0x100000
	fxch	%st(1)				// u_step*0x100000 | u*0x100000+0xfffff
	fistpl	4(%esi)				// u*0x100000+0xfffff
	fistpl	(%esi)

	movl	(%esi),%ebx
	movl	4(%esi),%eax
	cmpw	$0,16(%esi)
	je		LisLeader
	incl	%ebx
LisLeader:

	sall	$2,%edx
	movl	$C(newedges),%edi
	movl	(%edx,%edi),%eax
	testl	%eax,%eax
	je		LIsFirst
	cmpl	%ebx,(%eax)
	jl		LNotFirst

LIsFirst:
	movl	%eax,12(%esi)
	movl	%esi,(%edx,%edi)
	jmp		LSetRemove

LNotFirst:
LSortLoop:
	movl	%eax,%edx
	movl	12(%edx),%eax
	testl	%eax,%eax
	je		LFoundSpot
	cmpl	%ebx,(%eax)
	jl		LSortLoop

LFoundSpot:
	movl	12(%edx),%eax
	movl	%eax,12(%esi)
	movl	%esi,12(%edx)

LSetRemove:
	movl	C(removeedges)(,%ebp,4),%eax
	movl	%eax,20(%esi)
	movl	%esi,C(removeedges)(,%ebp,4)

LClipDone:
	popl	%ebx
	popl	%esi
	popl	%edi
	popl	%ebp
	ret

LPop4AndDone:
	fstp	%st(0)
	fstp	%st(0)
	fstp	%st(0)
	fstp	%st(0)
	jmp		LClipDone


LIsClipped:
	testl	%eax,%eax
	jns		LPoint1IsClipped

	movl	Ld1,%eax
	testl	%eax,%eax
	jns		LPoint0IsClipped

	jmp		LClipDone

LPoint0IsClipped:
	flds	Ld0				// d0
	flds	Ld1				// d1 | d0
	fld		%st(1)			// d0 | d1 | d0
	fsubp	%st(0),%st(1)	// d0 - d1 | d0
	fdivrp	%st(0),%st(1)	// f
	movl	$1,52(%ebx)
	leal	36(%ebx),%ecx
	flds	(%edi)
	fsubs	(%esi)
	flds	4(%edi)
	fsubs	4(%esi)
	flds	8(%edi)
	fsub	8(%esi)			// z | y | x | f
	fxch	%st(2)			// x | y | z | f
	fmul	%st(3),%st(0)	// x | y | z | f
	fxch	%st(1)			// y | x | z | f
	fmul	%st(3),%st(0)	// y | x | z | f
	fxch	%st(2)			// z | x | y | f
	fmulp	%st(0),%st(3)	// x | y | z
	fadds	(%esi)			// x | y | z
	fxch	%st(1)			// y | x | z
	fadds	4(%esi)			// y | x | z
	fxch	%st(2)			// z | x | y
	fadds	8(%esi)			// z | x | y
	fxch	%st(1)			// x | z | y
	fstps	(%ecx)			// z | y
	fxch	%st(1)			// y | z
	fstps	4(%ecx)			// z
	fstps	8(%ecx)			//
	cmpl	$0,48(%ebx)
	je		LNoEntered
	movl	$0,48(%ebx)
	movl	$0,52(%ebx)
	movl	%ebp,%eax
	xorl	20(%ebx),%eax
	pushl	%eax
	pushl	$C(view_clipplanes)
	pushl	%ecx
	leal	24(%ebx),%eax
	pushl	%eax
	call	C(D_ClipEdge)
	addl	$16,%esp
	leal	36(%ebx),%ecx
LNoEntered:
	pushl	%ebp
	movl	16(%ebx),%eax
	pushl	%eax
	pushl	%edi
	pushl	%ecx
	call	C(D_ClipEdge)
	addl	$16,%esp
	jmp		LClipDone

LPoint1IsClipped:
	flds	Ld0				// d0
	flds	Ld1				// d1 | d0
	fld		%st(1)			// d0 | d1 | d0
	fsubp	%st(0),%st(1)	// d0 - d1 | d0
	fdivrp	%st(0),%st(1)	// f
	movl	$1,48(%ebx)
	leal	24(%ebx),%ecx
	pushl	%ebp
	movl	16(%ebx),%eax
	pushl	%eax
	pushl	%ecx
	pushl	%esi
	flds	(%edi)
	fsubs	(%esi)
	flds	4(%edi)
	fsubs	4(%esi)
	flds	8(%edi)
	fsub	8(%esi)			// z | y | x | f
	fxch	%st(2)			// x | y | z | f
	fmul	%st(3),%st(0)	// x | y | z | f
	fxch	%st(1)			// y | x | z | f
	fmul	%st(3),%st(0)	// y | x | z | f
	fxch	%st(2)			// z | x | y | f
	fmulp	%st(0),%st(3)	// x | y | z
	fadds	(%esi)			// x | y | z
	fxch	%st(1)			// y | x | z
	fadds	4(%esi)			// y | x | z
	fxch	%st(2)			// z | x | y
	fadds	8(%esi)			// z | x | y
	fxch	%st(1)			// x | z | y
	fstps	(%ecx)			// z | y
	fxch	%st(1)			// y | z
	fstps	4(%ecx)			// z
	fstps	8(%ecx)			//
	call	C(D_ClipEdge)
	addl	$16,%esp
	cmpl	$0,52(%ebx)
	je		LClipDone
	movl	$0,48(%ebx)
	movl	$0,52(%ebx)
	xorl	20(%ebx),%ebp
	pushl	%ebp
	pushl	$C(view_clipplanes)
	leal	36(%ebx),%eax
	leal	24(%ebx),%ecx
	pushl	%eax
	pushl	%ecx
	call	C(D_ClipEdge)
	addl	$16,%esp
	jmp		LClipDone

//==========================================================================
//
//	D_GenerateSpans
//
//==========================================================================

.globl C(D_GenerateSpans)
C(D_GenerateSpans):
	pushl	%ebp
	pushl	%edi
	pushl	%esi
	pushl	%ebx

	// clear active surfaces to just the background surface
	movl	$C(surfaces)+64,%eax
	movl	$0,C(surfaces)+80
	movl	%eax,C(surfaces)+68
	movl	%eax,C(surfaces)+64

	// generate spans
	movl	C(edge_head)+12,%edi
	cmpl	$C(edge_tail),%edi
	je		LCleanupSpan

LEdgesLoop:
	cmpw	$0,16(%edi)
	je		LLeadingEdge
	// it has a left surface, so a surface is going away for this span
	xorl	%ecx,%ecx
	movw	16(%edi),%cx
	sall	$6,%ecx
	leal	C(surfaces)(%ecx),%esi
	//	Don't generate a span if this is an inverted span, with the end
	// edge preceding the start edge (that is, we haven't seen the
	// start edge yet)
	decl	C(surfaces)+20(%ecx)
	jnz		LLeadingEdge
	cmpl	C(surfaces)+64,%esi
	jne		LUnlinkSurf
	movl	(%edi),%ebx
	movl	C(surfaces)+16(%ecx),%eax
	sarl	$20,%ebx
	cmpl	%eax,%ebx
	jle		LTrailingNoSpan
	// emit a span (current top going away)
	movl	C(span_p),%ebp
	movl	%ebx,%edx
	addl	$16,C(span_p)
	subl	%eax,%edx
	movl	%eax,(%ebp)
	movl	%edx,8(%ebp)
	movl	C(current_iv),%eax
	movl	C(surfaces)+8(%ecx),%edx
	movl	%eax,4(%ebp)
	movl	%edx,12(%ebp)
	movl	%ebp,C(surfaces)+8(%ecx)
LTrailingNoSpan:
	// set last_u on the surface below
	movl	C(surfaces)(%ecx),%eax
	movl	%ebx,16(%eax)
LUnlinkSurf:
	movl	4(%esi),%edx
	movl	(%esi),%eax
	movl	%eax,(%edx)
	movl	(%esi),%edx
	movl	4(%esi),%eax
	movl	%eax,4(%edx)

LLeadingEdge:
	cmpw	$0,18(%edi)
	je		LNextEdge
	// it's adding a new surface in, so find the correct place
	xorl	%ebx,%ebx
	movw	18(%edi),%bx
	sall	$6,%ebx
	leal	C(surfaces)(%ebx),%ebp
	// don't start a span if this is an inverted span, with the end edge
	// preceding the start edge (that is, we've already seen the end edge)
	movl	C(surfaces)+20(%ebx),%eax
	incl	C(surfaces)+20(%ebx)
	testl	%eax,%eax
	jnz		LNextEdge
	movl	C(surfaces)+64,%ecx
	movl	C(surfaces)+12(%ebx),%eax
	cmpl	12(%ecx),%eax
	jl		LNewTop
LSurfSearchLoop:
	movl	(%ecx),%ecx
	cmpl	12(%ecx),%eax
	jge		LSurfSearchLoop
	jmp		LGotPosition
LNewTop:
	movl	(%edi),%esi
	sarl	$20,%esi
	// set last_u on the new span
	movl	%esi,C(surfaces)+16(%ebx)
	movl	16(%ecx),%eax
	cmpl	%eax,%esi
	jle		LGotPosition
	// emit a span (obscures current top)
	movl	C(span_p),%edx
	addl	$16,C(span_p)
	subl	%eax,%esi
	movl	%eax,(%edx)
	movl	%esi,8(%edx)
	movl	C(current_iv),%eax
	movl	%eax,4(%edx)
	movl	8(%ecx),%eax
	movl	%eax,12(%edx)
	movl	%edx,8(%ecx)
LGotPosition:
	// insert before surf2
	movl	%ecx,(%ebp)
	movl	4(%ecx),%eax
	movl	%eax,4(%ebp)
	movl	4(%ecx),%eax
	movl	%ebp,(%eax)
	movl	%ebp,4(%ecx)

LNextEdge:
	movl	12(%edi),%edi
	cmpl	$C(edge_tail),%edi
	jne		LEdgesLoop

	//	Now that we've reached the right edge of the screen, we're done with
	// any unfinished surfaces, so emit a span for whatever's on top
LCleanupSpan:
	movl	C(surfaces)+64,%ecx
	movl	C(viewwidth),%ebx
	movl	16(%ecx),%edx
	cmpl	%edx,%ebx
	jle		LResetSurfacesLoop
	movl	C(span_p),%eax
	addl	$16,C(span_p)
	movl	%edx,(%eax)
	subl	%edx,%ebx
	movl	C(current_iv),%edx
	movl	%ebx,8(%eax)
	movl	%edx,4(%eax)
	movl	8(%ecx),%edx
	movl	%edx,12(%eax)
	movl	%eax,8(%ecx)

	// reset spanstate for all surfaces in the surface stack
LResetSurfacesLoop:
	movl	$0,20(%ecx)
	movl	(%ecx),%ecx
	cmpl	$C(surfaces)+64,%ecx
	jne		LResetSurfacesLoop

	popl	%ebx
	popl	%esi
	popl	%edi
	popl	%ebp
	ret

#endif

//**************************************************************************
//
//	$Log$
//	Revision 1.2  2001/08/21 17:22:28  dj_jl
//	Optimized rendering with some asm
//
//	Revision 1.1  2001/08/15 17:13:05  dj_jl
//	Implemented D_EmitEdge in asm
//	
//**************************************************************************
