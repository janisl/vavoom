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

.text

//==========================================================================
//
//	D_EmitEdge
//
//==========================================================================

#define ee_pv0	16+0
#define ee_pv1	16+4

.globl C(D_EmitEdge)
C(D_EmitEdge):

	pushl	%edi
	pushl	%esi
	pushl	%ebx

	fldcw	ceil_cw

	movl	ee_pv0(%esp),%ecx
	movl	ee_pv1(%esp),%esi
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
	flds	(%ecx)				// v[0]
	fsubs	C(vieworg)+0		// local[0]
	flds	4(%ecx)				// v[1] | local[0]
	fsubs	C(vieworg)+4		// local[1] | local[0]
	flds	8(%ecx)				// v[2] | local[1] | local[0]
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

	movl	(%ecx),%eax
	movl	%eax,C(firstvert)+0
	movl	4(%ecx),%eax
	movl	%eax,C(firstvert)+4
	movl	8(%ecx),%eax
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

	movl	C(edge_p),%ecx
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
	movw	$0,18(%ecx)
	movw	%ax,16(%ecx)
	movl	%edx,Ltemp
	leal	-1(%ebx),%edi
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
	leal	-1(%eax),%edi
	movl	C(surface_p),%eax
	subl	$C(surfaces),%eax
	sarl	$6,%eax
	movl	%ebx,%edx
	movw	$0,16(%ecx)
	movw	%ax,18(%ecx)
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
	fistpl	4(%ecx)				// u*0x100000+0xfffff
	fistpl	(%ecx)

	movl	(%ecx),%ebx
	movl	4(%ecx),%eax
	cmpw	$0,16(%ecx)
	je		LisLeader
	incl	%ebx
LisLeader:

	sall	$2,%edx
	movl	$C(newedges),%esi
	movl	(%edx,%esi),%eax
	testl	%eax,%eax
	je		LIsFirst
	cmpl	%ebx,(%eax)
	jl		LNotFirst

LIsFirst:
	movl	%eax,12(%ecx)
	movl	%ecx,(%edx,%esi)
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
	movl	%eax,12(%ecx)
	movl	%ecx,12(%edx)

LSetRemove:
	movl	C(removeedges)(,%edi,4),%eax
	movl	%eax,20(%ecx)
	movl	%ecx,C(removeedges)(,%edi,4)

LEmitDone:
	popl	%ebx
	popl	%esi
	popl	%edi
	ret

LPop4AndDone:
	fstp	%st(0)
	fstp	%st(0)
	fstp	%st(0)
	fstp	%st(0)
	jmp		LEmitDone

#endif

//**************************************************************************
//
//	$Log$
//	Revision 1.1  2001/08/15 17:13:05  dj_jl
//	Implemented D_EmitEdge in asm
//
//**************************************************************************
