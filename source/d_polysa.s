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

.extern	C(D_PolysetSetEdgeTable)
.extern	C(D_RasterizeAliasPolySmooth)

#define SPAN_SIZE	(1024 + 1 + 1 + 1) * spanpackage_t_size

	.data

	.align	4
p00_minus_p20:	.single		0
p10_minus_p20:	.single		0
p01_minus_p21:	.single		0
p11_minus_p21:	.single		0

Ltemp:	.long	0

	.text

//==========================================================================
//
//	D_PolysetDraw
//
//==========================================================================

#ifndef GAS2TASM

.globl C(D_PolysetDraw)
C(D_PolysetDraw):
	subl	$(SPAN_SIZE),%esp

	movl	%esp,%eax
	addl	$(CACHE_SIZE - 1),%eax
	andl	$(~(CACHE_SIZE - 1)),%eax
	movl	%eax,C(a_spans)

	pushl	%ebp
	pushl	%edi
	pushl	%esi
	pushl	%ebx

	movl	C(d_affinetridesc)+atd_numtriangles,%ebp
	leal	(%ebp,%ebp,2),%ebp
	shll	$2,%ebp

LTriangleLoop:
	movl	C(d_affinetridesc)+atd_ptriangles,%edx
	movl	C(d_affinetridesc)+atd_pfinalverts,%ebx
	movl	C(d_affinetridesc)+atd_pfinalverts,%esi
	movl	C(d_affinetridesc)+atd_pfinalverts,%edi
	movswl	mtriangle_t_vertindex-mtriangle_t_size(%edx,%ebp),%eax
	sall	$5,%eax
	addl	%eax,%ebx
	movswl	mtriangle_t_vertindex+2-mtriangle_t_size(%edx,%ebp),%eax
	sall	$5,%eax
	addl	%eax,%esi
	movswl	mtriangle_t_vertindex+4-mtriangle_t_size(%edx,%ebp),%eax
	sall	$5,%eax
	addl	%eax,%edi

	movl	fv_u(%ebx),%eax
	movl	fv_v(%ebx),%edx
	subl	fv_u(%edi),%eax
	subl	fv_v(%esi),%edx
	imull	%eax,%edx
	movl	fv_u(%ebx),%eax
	movl	fv_v(%ebx),%ecx
	subl	fv_u(%esi),%eax
	subl	fv_v(%edi),%ecx
	imull	%ecx,%eax
	subl	%eax,%edx
	testl	%edx,%edx
	jle		LNextTriangle

	//	Calc 1/denom here where it can overlap
	movl	%edx,C(d_denom)
	fildl	C(d_denom)		// d_denom
	fld1					// 1 | d_denom
	fdivp	%st(0),%st(1)	// denominv

	movl	fv_u(%ebx),%eax
	movl	fv_v(%ebx),%edx
	movl	%eax,C(r_p0)
	movl	%edx,C(r_p0)+4
	movl	fv_zi(%ebx),%eax
	movl	fv_r(%ebx),%edx
	movl	%eax,C(r_p0)+16
	movl	%edx,C(r_p0)+20
	movl	fv_g(%ebx),%eax
	movl	fv_b(%ebx),%edx
	movl	%eax,C(r_p0)+24
	movl	%edx,C(r_p0)+28

	movl	fv_u(%esi),%eax
	movl	fv_v(%esi),%edx
	movl	%eax,C(r_p1)
	movl	%edx,C(r_p1)+4
	movl	fv_zi(%esi),%eax
	movl	fv_r(%esi),%edx
	movl	%eax,C(r_p1)+16
	movl	%edx,C(r_p1)+20
	movl	fv_g(%esi),%eax
	movl	fv_b(%esi),%edx
	movl	%eax,C(r_p1)+24
	movl	%edx,C(r_p1)+28

	movl	C(d_affinetridesc)+atd_ptriangles,%ecx

	movl	fv_u(%edi),%eax
	movl	fv_v(%edi),%edx
	movl	%eax,C(r_p2)
	movl	%edx,C(r_p2)+4
	movl	fv_zi(%edi),%eax
	movl	fv_r(%edi),%edx
	movl	%eax,C(r_p2)+16
	movl	%edx,C(r_p2)+20
	movl	fv_g(%edi),%eax
	movl	fv_b(%edi),%edx
	movl	%eax,C(r_p2)+24
	movl	%edx,C(r_p2)+28

	movl	C(d_affinetridesc)+atd_pstverts,%edx
	movswl	mtriangle_t_stvertindex-mtriangle_t_size(%ecx,%ebp),%ebx
	movswl	mtriangle_t_stvertindex+2-mtriangle_t_size(%ecx,%ebp),%esi
	movswl	mtriangle_t_stvertindex+4-mtriangle_t_size(%ecx,%ebp),%edi
	leal	(%edx,%ebx,8),%ebx
	leal	(%edx,%esi,8),%esi
	leal	(%edx,%edi,8),%edi

	movl	fstv_s(%ebx),%eax
	movl	fstv_t(%ebx),%edx
	movl	%eax,C(r_p0)+8
	movl	%edx,C(r_p0)+12
	movl	fstv_s(%esi),%eax
	movl	fstv_t(%esi),%edx
	movl	%eax,C(r_p1)+8
	movl	%edx,C(r_p1)+12
	movl	fstv_s(%edi),%eax
	movl	fstv_t(%edi),%edx
	movl	%eax,C(r_p2)+8
	movl	%edx,C(r_p2)+12

	fstps	C(d_denom)

	call	C(D_PolysetSetEdgeTable)
	call	C(D_RasterizeAliasPolySmooth)

LNextTriangle:
	subl	$12,%ebp
	jnz		LTriangleLoop

	popl	%ebx
	popl	%esi
	popl	%edi
	popl	%ebp
	addl	$(SPAN_SIZE),%esp
	ret

#endif

//==========================================================================
//
//	D_PolysetCalcGradients
//
//	affine triangle gradient calculation code
//
//==========================================================================

#define skinwidth	4+0

.globl C(D_PolysetCalcGradients)
C(D_PolysetCalcGradients):
	fildl	C(r_p0)			// p00
	fildl	C(r_p1)			// p10 | p00
	fildl	C(r_p2)			// p20 | p10 | p00
	fildl	C(r_p0)+4		// p01 | p20 | p10 | p00
	fildl	C(r_p1)+4		// p11 | p01 | p20 | p10 | p00
	fildl	C(r_p2)+4		// p21 | p11 | p01 | p20 | p10 | p00
	fxch	%st(5)			// p00 | p11 | p01 | p20 | p10 | p21
	fsub	%st(3),%st(0)	// p00_minus_p20 | p11 | p01 | p20 | p10 | p21
	fxch	%st(2)			// p01 | p11 | p00_minus_p20 | p20 | p10 | p21
	fsub	%st(5),%st(0)	// p01_minus_p21 | p11 | p00_minus_p20 | p20 |
							// p10 | p21
	fxch	%st(4)			// p10 | p11 | p00_minus_p20 | p20 |
							// p01_minus_p21 | p21
	fsubp	%st(0),%st(3)	// p11 | p00_minus_p20 | p10_minus_p20 |
							// p01_minus_p21 | p21
	fsubp	%st(0),%st(4)	// p00_minus_p20 | p10_minus_p20 |
							// p01_minus_p21 | p11_minus_p21
	fstps	p00_minus_p20	// p10_minus_p20 | p01_minus_p21 | p11_minus_p21
	fxch	%st(1)			// p01_minus_p21 | p10_minus_p20 | p11_minus_p21
	fstps	p01_minus_p21	// p10_minus_p20 | p11_minus_p21
	fstps	p10_minus_p20	// p11_minus_p21
	fstps	p11_minus_p21	//

	flds	C(d_denom)		// denominv

	fildl	C(r_p0)+8		// p02 | denominv
	fildl	C(r_p1)+8		// p12 | p02 | denominv
	fildl	C(r_p2)+8		// p22 | p12 | p02 | denominv
	fsubr	%st(0),%st(1)	// p22 | t1 | p02 | denominv
	fsubrp	%st(0),%st(2)	// t1 | t0 | denominv
//STALL
	fld		%st(0)			// t1 | t1 | t0 | denominv
	fmuls	p01_minus_p21	// t1 * p01_minus_p21 | t1 | t0 | denominv
	fld		%st(2)			// t0 | t1 * p01_minus_p21 | t1 | t0 | denominv
	fmuls	p11_minus_p21	// t0 * p11_minus_p21 | t1 * p01_minus_p21 | t1 |
							// t0 | denominv
	fxch	%st(2)			// t1 | t1 * p01_minus_p21 | t0 * p11_minus_p21 |
							// t0 | denominv
	fmuls	p00_minus_p20	// t1 * p00_minus_p20 | t1 * p01_minus_p21 |
							// t0 * p11_minus_p21 | t0 | denominv
	fxch	%st(3)			// t0 | t1 * p01_minus_p21 | t0 * p11_minus_p21 |
							// t1 * p00_minus_p20 | denominv
	fmuls	p10_minus_p20	// t0 * p10_minus_p20 | t1 * p01_minus_p21 |
							// t0 * p11_minus_p21 | t1 * p00_minus_p20 |
							// denominv
	fxch	%st(1)			// t1 * p01_minus_p21 | t0 * p10_minus_p20 |
							// t0 * p11_minus_p21 | t1 * p00_minus_p20 |
							// denominv
	fsubp	%st(0),%st(2)	// t0 * p10_minus_p20 |
							// t1 * p01_minus_p21 - t0 * p11_minus_p21 |
							// t1 * p00_minus_p20 | denominv
//STµLL
	fsubrp	%st(0),%st(2)	// t1 * p01_minus_p21 - t0 * p11_minus_p21 |
							// t1 * p00_minus_p20 - t0 * p10_minus_p20 |
							// denominv
	fmul	%st(2),%st(0)	// r_sstepx |
							// t1 * p00_minus_p20 - t0 * p10_minus_p20 |
							// denominv
	fxch	%st(1)			// t1 * p00_minus_p20 - t0 * p10_minus_p20 |
							// r_sstepx | denominv
//STALL
	fmul	%st(2),%st(0)	// r_sstepy | r_sstepx | denominv
	fxch	%st(1)			// r_sstepx | r_sstepy | denominv
	fistpl	C(r_sstepx)		// r_sstepy | denominv
	fistpl	C(r_sstepy)		// denominv

	fildl	C(r_p0)+12		// p03 | denominv
	fildl	C(r_p1)+12		// p13 | p03 | denominv
	fildl	C(r_p2)+12		// p23 | p13 | p03 | denominv
	fsubr	%st(0),%st(1)	// p23 | t1 | p03 | denominv
	fsubrp	%st(0),%st(2)	// t1 | t0 | denominv
	fld		%st(0)			// t1 | t1 | t0 | denominv
	fmuls	p01_minus_p21	// t1 * p01_minus_p21 | t1 | t0 | denominv
	fld		%st(2)			// t0 | t1 * p01_minus_p21 | t1 | t0 | denominv
	fmuls	p11_minus_p21	// t0 * p11_minus_p21 | t1 * p01_minus_p21 | t1 |
							// t0 | denominv
	fxch	%st(2)			// t1 | t1 * p01_minus_p21 | t0 * p11_minus_p21 |
							// t0 | denominv
	fmuls	p00_minus_p20	// t1 * p00_minus_p20 | t1 * p01_minus_p21 |
							// t0 * p11_minus_p21 | t0 | denominv
	fxch	%st(3)			// t0 | t1 * p01_minus_p21 | t0 * p11_minus_p21 |
							// t1 * p00_minus_p20 | denominv
	fmuls	p10_minus_p20	// t0 * p10_minus_p20 | t1 * p01_minus_p21 |
							// t0 * p11_minus_p21 | t1 * p00_minus_p20 |
							// denominv
	fxch	%st(1)			// t1 * p01_minus_p21 | t0 * p10_minus_p20 |
							// t0 * p11_minus_p21 | t1 * p00_minus_p20 |
							// denominv
	fsubp	%st(0),%st(2)	// t0 * p10_minus_p20 |
							// t1 * p01_minus_p21 - t0 * p11_minus_p21 |
							// t1 * p00_minus_p20 | denominv
//STALL
	fsubrp	%st(0),%st(2)	// t1 * p01_minus_p21 - t0 * p11_minus_p21 |
							// t1 * p00_minus_p20 - t0 * p10_minus_p20 |
							// denominv
	fmul	%st(2),%st(0)	// r_tstepx |
							// t1 * p00_minus_p20 - t0 * p10_minus_p20 |
							// denominv
	fxch	%st(1)			// t1 * p00_minus_p20 - t0 * p10_minus_p20 |
							// r_tstepx | denominv
//STALL
	fmul	%st(2),%st(0)	// r_tstepy | r_tstepx | denominv
	fxch	%st(1)			// r_tstepx | r_tstepy | denominv
	fistpl	C(r_tstepx)		// r_tstepy | denominv
	fistpl	C(r_tstepy)		// denominv

	fildl	C(r_p0)+16		// p04 | denominv
	fildl	C(r_p1)+16		// p14 | p04 | denominv
	fildl	C(r_p2)+16		// p24 | p14 | p04 | denominv
	fsubr	%st(0),%st(1)	// p24 | t1 | p04 | denominv
	fsubrp	%st(0),%st(2)	// t1 | t0 | denominv
	fld		%st(0)			// t1 | t1 | t0 | denominv
	fmuls	p01_minus_p21	// t1 * p01_minus_p21 | t1 | t0 | denominv
	fld		%st(2)			// t0 | t1 * p01_minus_p21 | t1 | t0 | denominv
	fmuls	p11_minus_p21	// t0 * p11_minus_p21 | t1 * p01_minus_p21 | t1 |
							// t0 | denominv
	fxch	%st(2)			// t1 | t1 * p01_minus_p21 | t0 * p11_minus_p21 |
							// t0 | denominv
	fmuls	p00_minus_p20	// t1 * p00_minus_p20 | t1 * p01_minus_p21 |
							// t0 * p11_minus_p21 | t0 | denominv
	fxch	%st(3)			// t0 | t1 * p01_minus_p21 | t0 * p11_minus_p21 |
							// t1 * p00_minus_p20 | denominv
	fmuls	p10_minus_p20	// t0 * p10_minus_p20 | t1 * p01_minus_p21 |
							// t0 * p11_minus_p21 | t1 * p00_minus_p20 |
							// denominv
	fxch	%st(1)			// t1 * p01_minus_p21 | t0 * p10_minus_p20 |
							// t0 * p11_minus_p21 | t1 * p00_minus_p20 |
							// denominv
	fsubp	%st(0),%st(2)	// t0 * p10_minus_p20 |
							// t1 * p01_minus_p21 - t0 * p11_minus_p21 |
							// t1 * p00_minus_p20 | denominv
//STALL
	fsubrp	%st(0),%st(2)	// t1 * p01_minus_p21 - t0 * p11_minus_p21 |
							// t1 * p00_minus_p20 - t0 * p10_minus_p20 |
							// denominv
	fmul	%st(2),%st(0)	// r_zistepx |
							// t1 * p00_minus_p20 - t0 * p10_minus_p20 |
							// denominv
	fxch	%st(1)			// t1 * p00_minus_p20 - t0 * p10_minus_p20 |
							// r_zistepx | denominv
//STALL
	fmul	%st(2),%st(0)	// r_zistepy | r_zistepx | denominv
	fxch	%st(1)			// r_zistepx | r_zistepy | denominv
	fistpl	C(r_zistepx)	// r_zistepy | denominv
	fistpl	C(r_zistepy)	// denominv

	fildl	C(r_p0)+20		// p05 | denominv
	fildl	C(r_p1)+20		// p15 | p05 | denominv
	fildl	C(r_p2)+20		// p25 | p15 | p05 | denominv
	fsubr	%st(0),%st(1)	// p25 | t1 | p05 | denominv
	fsubrp	%st(0),%st(2)	// t1 | t0 | denominv
	fld		%st(0)			// t1 | t1 | t0 | denominv
	fmuls	p01_minus_p21	// t1 * p01_minus_p21 | t1 | t0 | denominv
	fld		%st(2)			// t0 | t1 * p01_minus_p21 | t1 | t0 | denominv
	fmuls	p11_minus_p21	// t0 * p11_minus_p21 | t1 * p01_minus_p21 | t1 |
							// t0 | denominv
	fxch	%st(2)			// t1 | t1 * p01_minus_p21 | t0 * p11_minus_p21 |
							// t0 | denominv
	fmuls	p00_minus_p20	// t1 * p00_minus_p20 | t1 * p01_minus_p21 |
							// t0 * p11_minus_p21 | t0 | denominv
	fxch	%st(3)			// t0 | t1 * p01_minus_p21 | t0 * p11_minus_p21 |
							// t1 * p00_minus_p20 | denominv
	fmuls	p10_minus_p20	// t0 * p10_minus_p20 | t1 * p01_minus_p21 |
							// t0 * p11_minus_p21 | t1 * p00_minus_p20 |
							// denominv
	fxch	%st(1)			// t1 * p01_minus_p21 | t0 * p10_minus_p20 |
							// t0 * p11_minus_p21 | t1 * p00_minus_p20 |
							// denominv
	fsubp	%st(0),%st(2)	// t0 * p10_minus_p20 |
							// t1 * p01_minus_p21 - t0 * p11_minus_p21 |
							// t1 * p00_minus_p20 | denominv
//STALL
	fsubrp	%st(0),%st(2)	// t1 * p01_minus_p21 - t0 * p11_minus_p21 |
							// t1 * p00_minus_p20 - t0 * p10_minus_p20 |
							// denominv
	fmul	%st(2),%st(0)	// r_rstepx |
							// t1 * p00_minus_p20 - t0 * p10_minus_p20 |
							// denominv
	fxch	%st(1)			// t1 * p00_minus_p20 - t0 * p10_minus_p20 |
							// r_rstepx | denominv
//STALL
	fmul	%st(2),%st(0)	// r_rstepy | r_rstepx | denominv
	fxch	%st(1)			// r_rstepx | r_rstepy | denominv
	fldcw	ceil_cw
	fistpl	C(r_rstepx)		// r_rstepy | denominv
	fistpl	C(r_rstepy)		// denominv
	fldcw	single_cw

	fildl	C(r_p0)+24		// p06 | denominv
	fildl	C(r_p1)+24		// p16 | p06 | denominv
	fildl	C(r_p2)+24		// p26 | p16 | p06 | denominv
	fsubr	%st(0),%st(1)	// p26 | t1 | p06 | denominv
	fsubrp	%st(0),%st(2)	// t1 | t0 | denominv
	fld		%st(0)			// t1 | t1 | t0 | denominv
	fmuls	p01_minus_p21	// t1 * p01_minus_p21 | t1 | t0 | denominv
	fld		%st(2)			// t0 | t1 * p01_minus_p21 | t1 | t0 | denominv
	fmuls	p11_minus_p21	// t0 * p11_minus_p21 | t1 * p01_minus_p21 | t1 |
							// t0 | denominv
	fxch	%st(2)			// t1 | t1 * p01_minus_p21 | t0 * p11_minus_p21 |
							// t0 | denominv
	fmuls	p00_minus_p20	// t1 * p00_minus_p20 | t1 * p01_minus_p21 |
							// t0 * p11_minus_p21 | t0 | denominv
	fxch	%st(3)			// t0 | t1 * p01_minus_p21 | t0 * p11_minus_p21 |
							// t1 * p00_minus_p20 | denominv
	fmuls	p10_minus_p20	// t0 * p10_minus_p20 | t1 * p01_minus_p21 |
							// t0 * p11_minus_p21 | t1 * p00_minus_p20 |
							// denominv
	fxch	%st(1)			// t1 * p01_minus_p21 | t0 * p10_minus_p20 |
							// t0 * p11_minus_p21 | t1 * p00_minus_p20 |
							// denominv
	fsubp	%st(0),%st(2)	// t0 * p10_minus_p20 |
							// t1 * p01_minus_p21 - t0 * p11_minus_p21 |
							// t1 * p00_minus_p20 | denominv
//STALL
	fsubrp	%st(0),%st(2)	// t1 * p01_minus_p21 - t0 * p11_minus_p21 |
							// t1 * p00_minus_p20 - t0 * p10_minus_p20 |
							// denominv
	fmul	%st(2),%st(0)	// r_gstepx |
							// t1 * p00_minus_p20 - t0 * p10_minus_p20 |
							// denominv
	fxch	%st(1)			// t1 * p00_minus_p20 - t0 * p10_minus_p20 |
							// r_gstepx | denominv
//STALL
	fmul	%st(2),%st(0)	// r_gstepy | r_gstepx | denominv
	fxch	%st(1)			// r_gstepx | r_gstepy | denominv
	fldcw	ceil_cw
	fistpl	C(r_gstepx)		// r_gstepy | denominv
	fistpl	C(r_gstepy)		// denominv
	fldcw	single_cw

	fildl	C(r_p0)+28		// p07 | denominv
	fildl	C(r_p1)+28		// p17 | p07 | denominv
	fildl	C(r_p2)+28		// p27 | p17 | p07 | denominv
	fsubr	%st(0),%st(1)	// p27 | t1 | p07 | denominv
	fsubrp	%st(0),%st(2)	// t1 | t0 | denominv
	fld		%st(0)			// t1 | t1 | t0 | denominv
	fmuls	p01_minus_p21	// t1 * p01_minus_p21 | t1 | t0 | denominv
	fld		%st(2)			// t0 | t1 * p01_minus_p21 | t1 | t0 | denominv
	fmuls	p11_minus_p21	// t0 * p11_minus_p21 | t1 * p01_minus_p21 | t1 |
							// t0 | denominv
	fxch	%st(2)			// t1 | t1 * p01_minus_p21 | t0 * p11_minus_p21 |
							// t0 | denominv
	fmuls	p00_minus_p20	// t1 * p00_minus_p20 | t1 * p01_minus_p21 |
							// t0 * p11_minus_p21 | t0 | denominv
	fxch	%st(3)			// t0 | t1 * p01_minus_p21 | t0 * p11_minus_p21 |
							// t1 * p00_minus_p20 | denominv
	fmuls	p10_minus_p20	// t0 * p10_minus_p20 | t1 * p01_minus_p21 |
							// t0 * p11_minus_p21 | t1 * p00_minus_p20 |
							// denominv
	fxch	%st(1)			// t1 * p01_minus_p21 | t0 * p10_minus_p20 |
							// t0 * p11_minus_p21 | t1 * p00_minus_p20 |
							// denominv
	fsubp	%st(0),%st(2)	// t0 * p10_minus_p20 |
							// t1 * p01_minus_p21 - t0 * p11_minus_p21 |
							// t1 * p00_minus_p20 | denominv
//STALL
	fsubrp	%st(0),%st(2)	// t1 * p01_minus_p21 - t0 * p11_minus_p21 |
							// t1 * p00_minus_p20 - t0 * p10_minus_p20 |
							// denominv
	fmul	%st(2),%st(0)	// r_bstepx |
							// t1 * p00_minus_p20 - t0 * p10_minus_p20 |
							// denominv
	fxch	%st(1)			// t1 * p00_minus_p20 - t0 * p10_minus_p20 |
							// r_bstepx | denominv
//STALL
	fmulp	%st(0),%st(2)	// r_bstepx | r_bstepy
	fldcw	ceil_cw
	fistpl	C(r_bstepx)		// r_bstepy
	fistpl	C(r_bstepy)		//
	fldcw	single_cw

	movl	C(r_sstepx),%eax
	movl	C(r_tstepx),%edx
	shll	$16,%eax
	sall	$16,%edx
	movl	%eax,C(a_sstepxfrac)
	movl	%edx,C(a_tstepxfrac)

	movl	C(r_sstepx),%ecx
	movl	C(r_tstepx),%eax
	sarl	$16,%ecx
	sarl	$16,%eax
	imull	skinwidth(%esp)
	addl	%ecx,%eax
	movl	%eax,C(a_ststepxwhole)

	ret

//==========================================================================
//
//	D_PolysetSetUpForLineScan
//
//==========================================================================

#define startvertu		12+0
#define startvertv		12+4
#define endvertu		12+8
#define endvertv		12+12

.globl C(D_PolysetSetUpForLineScan)
C(D_PolysetSetUpForLineScan):
	pushl	%esi
	pushl	%ebx
	movl	$-1,C(errorterm)
	movl	endvertu(%esp),%edx
	movl	endvertv(%esp),%eax
	movl	startvertv(%esp),%esi
	subl	startvertu(%esp),%edx
	subl	%eax,%esi

	leal	15(%edx),%eax
	cmpl	$31,%eax
	ja		LFloorDivMod
	leal	15(%esi),%ecx
	cmpl	$31,%ecx
	ja		LFloorDivMod

	sall	$5,%eax
	addl	%ecx,%eax
	sall	$3,%eax
	movl	%esi,C(erroradjustdown)
	movl	C(adivtab)(%eax),%edx
	movl	C(adivtab)+4(%eax),%eax
	movl	%edx,C(ubasestep)
	movl	%eax,C(erroradjustup)
	jmp		LSetupDone

LFloorDivMod:
	movl	%esi,C(erroradjustdown)
	fildl	C(erroradjustdown)	// denom
	movl	%edx,Ltemp
	fildl	Ltemp			// numer | denom
	fldz					// 0 | numer | denom
	fcomp	%st(1)			// numer | denom
	fnstsw	%ax
	andb	$69,%ah
	je		LFloorDivModNegative

	fld		%st(0)			// numer | numer | denom
	fdiv	%st(2),%st(0)	// numer / denom | numer | denom
	fldcw	floor_cw
	frndint					// x | numer | denom
	fistl	C(ubasestep)	// x | numer | denom
	fmulp	%st(0),%st(2)	// numer | x * denom
	fsubp	%st(0),%st(1)	// numer - x * denom
	fistpl	C(erroradjustup)
	fldcw	single_cw
	jmp		LSetupDone

//
// perform operations with positive values, and fix mod to make floor-based
//
LFloorDivModNegative:
	fchs					// -numer | denom
	fld		%st(0)			// -numer | -numer | denom
	fdiv	%st(2),%st(0)	// -numer / denom | -numer | denom
	fldcw	floor_cw
	frndint					// x | -numer | denom
	fistl	C(ubasestep)	// x | -numer | denom
	fmulp	%st(0),%st(2)	// -numer | x * denom
	fsubp	%st,%st(1)		// -numer - x * denom
	fistpl	C(erroradjustup)
	fldcw	single_cw
	negl	C(ubasestep)
	movl	C(erroradjustup),%edx
	testl	%edx,%edx
	je		LSetupDone

	decl	C(ubasestep)
	subl	%edx,%esi
	movl	%esi,C(erroradjustup)

LSetupDone:
	popl	%ebx
	popl	%esi
	ret

//==========================================================================
//
//	D_PolysetScanLeftEdge
//
//	Alias model triangle left-edge scanning code
//
//==========================================================================

#define height	4+16

.globl C(D_PolysetScanLeftEdge)
C(D_PolysetScanLeftEdge):
	pushl	%ebp				// preserve caller stack frame pointer
	pushl	%esi				// preserve register variables
	pushl	%edi
	pushl	%ebx

	movl	height(%esp),%eax
	movl	C(d_sfrac),%ecx
	andl	$0xFFFF,%eax
	movl	C(d_ptex),%ebx
	orl		%eax,%ecx
	movl	C(d_pedgespanpackage),%esi
	movl	C(d_tfrac),%edx
	movl	C(d_r),%edi
	movl	C(d_zi),%ebp

// %eax: scratch
// %ebx: d_ptex
// %ecx: d_sfrac in high word, count in low word
// %edx: d_tfrac
// %esi: d_pedgespanpackage, errorterm, scratch alternately
// %edi: d_r
// %ebp: d_zi

//	do
//	{

LScanLoop:

//		d_pedgespanpackage->ptex = ptex;
//		d_pedgespanpackage->pdest = d_pdest;
//		d_pedgespanpackage->pz = d_pz;
//		d_pedgespanpackage->count = d_aspancount;
//		d_pedgespanpackage->light = d_light;
//		d_pedgespanpackage->zi = d_zi;
//		d_pedgespanpackage->sfrac = d_sfrac << 16;
//		d_pedgespanpackage->tfrac = d_tfrac << 16;
	movl	%ebx,spanpackage_t_ptex(%esi)
	movl	C(d_pdest),%eax
	movl	%eax,spanpackage_t_pdest(%esi)
	movl	C(d_pz),%eax
	movl	%eax,spanpackage_t_pz(%esi)
	movl	C(d_aspancount),%eax
	movw	%ax,spanpackage_t_count(%esi)
	movw	%di,spanpackage_t_r(%esi)
	movl	C(d_g),%eax
	movw	%ax,spanpackage_t_g(%esi)
	movl	C(d_b),%eax
	movw	%ax,spanpackage_t_b(%esi)
	movl	%ebp,spanpackage_t_zi(%esi)
	movl	%ecx,spanpackage_t_sfrac(%esi)
	movl	%edx,spanpackage_t_tfrac(%esi)

// pretouch the next cache line
	movb	spanpackage_t_size(%esi),%al

//		d_pedgespanpackage++;
	addl	$(spanpackage_t_size),%esi
	movl	C(erroradjustup),%eax
	movl	%esi,C(d_pedgespanpackage)

//		errorterm += erroradjustup;
	movl	C(errorterm),%esi
	addl	%eax,%esi
	movl	C(d_pdest),%eax

//		if (errorterm >= 0)
//		{
	js		LNoLeftEdgeTurnover

//			errorterm -= erroradjustdown;
//			d_pdest += d_pdestextrastep;
	subl	C(erroradjustdown),%esi
	addl	C(d_pdestextrastep),%eax
	movl	%esi,C(errorterm)
	movl	%eax,C(d_pdest)

//			d_pz += d_pzextrastep;
//			d_aspancount += d_countextrastep;
//			d_ptex += d_ptexextrastep;
//			d_sfrac += d_sfracextrastep;
//			d_ptex += d_sfrac >> 16;
//			d_sfrac &= 0xFFFF;
//			d_tfrac += d_tfracextrastep;
	movl	C(d_pz),%eax
	movl	C(d_aspancount),%esi
	addl	C(d_pzextrastep),%eax
	addl	C(d_sfracextrastep),%ecx
	adcl	C(d_ptexextrastep),%ebx
	addl	C(d_countextrastep),%esi
	movl	%eax,C(d_pz)
	movl	C(d_tfracextrastep),%eax
	movl	%esi,C(d_aspancount)
	addl	%eax,%edx

//			if (d_tfrac & 0x10000)
//			{
	jnc		LSkip1

//				d_ptex += d_affinetridesc.skinwidth;
//				d_tfrac &= 0xFFFF;
	addl	C(d_affinetridesc)+atd_skinwidth,%ebx

//			}

LSkip1:

//			d_light += d_lightextrastep;
//			d_zi += d_ziextrastep;
	addl	C(d_rextrastep),%edi
	movl	C(d_gextrastep),%eax
	addl	%eax,C(d_g)
	movl	C(d_bextrastep),%eax
	addl	%eax,C(d_b)
	addl	C(d_ziextrastep),%ebp

//		}
	movl	C(d_pedgespanpackage),%esi
	decl	%ecx
	testl	$0xFFFF,%ecx
	jnz		LScanLoop

	popl	%ebx
	popl	%edi
	popl	%esi
	popl	%ebp
	ret

//		else
//		{

LNoLeftEdgeTurnover:
	movl	%esi,C(errorterm)

//			d_pdest += d_pdestbasestep;
	addl	C(d_pdestbasestep),%eax
	movl	%eax,C(d_pdest)

//			d_pz += d_pzbasestep;
//			d_aspancount += ubasestep;
//			d_ptex += d_ptexbasestep;
//			d_sfrac += d_sfracbasestep;
//			d_ptex += d_sfrac >> 16;
//			d_sfrac &= 0xFFFF;
	movl	C(d_pz),%eax
	movl	C(d_aspancount),%esi
	addl	C(d_pzbasestep),%eax
	addl	C(d_sfracbasestep),%ecx
	adcl	C(d_ptexbasestep),%ebx
	addl	C(ubasestep),%esi
	movl	%eax,C(d_pz)
	movl	%esi,C(d_aspancount)

//			d_tfrac += d_tfracbasestep;
	movl	C(d_tfracbasestep),%esi
	addl	%esi,%edx

//			if (d_tfrac & 0x10000)
//			{
	jnc		LSkip2

//				d_ptex += d_affinetridesc.skinwidth;
//				d_tfrac &= 0xFFFF;
	addl	C(d_affinetridesc)+atd_skinwidth,%ebx

//			}

LSkip2:

//			d_light += d_lightbasestep;
//			d_zi += d_zibasestep;
	addl	C(d_rbasestep),%edi
	movl	C(d_gbasestep),%eax
	addl	%eax,C(d_g)
	movl	C(d_bbasestep),%eax
	addl	%eax,C(d_b)
	addl	C(d_zibasestep),%ebp

//		}
//	} while (--height);
	movl	C(d_pedgespanpackage),%esi
	decl	%ecx
	testl	$0xFFFF,%ecx
	jnz		LScanLoop

	popl	%ebx
	popl	%edi
	popl	%esi
	popl	%ebp
	ret

#endif

//**************************************************************************
//
//	$Log$
//	Revision 1.3  2001/09/05 12:21:42  dj_jl
//	Release changes
//
//	Revision 1.2  2001/08/15 17:44:41  dj_jl
//	Added missing externs
//	
//	Revision 1.1  2001/08/15 17:12:23  dj_jl
//	Optimized model drawing
//	
//**************************************************************************
