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
Lcoords:	.long	0, 0, 0
Ltemp:		.long	0

.text

	Align4
.globl C(D_ConcatTransforms)
C(D_ConcatTransforms):
	movl	4(%esp), %eax	//	In1
	movl	8(%esp), %edx	//	In2
	movl	12(%esp), %ecx	//	Out
	flds	(%edx)		// 2_00
	fmuls	(%eax)		// 1_00 * 2_00
	flds	16(%edx)	// 2_10 | 1_00 * 2_00
	fmuls	4(%eax)		// 1_01 * 2_10 | 1_00 * 2_00
	flds	32(%edx)	// 2_20 | 1_01 * 2_10 | 1_00 * 2_00
	fmuls	8(%eax)		// 1_02 * 2_20 | 1_01 * 2_10 | 1_00 * 2_00
	fxch	%st(1)		// 1_01 * 2_10 | 1_02 * 2_20 | 1_00 * 2_00
	faddp	%st, %st(2)	// 1_02 * 2_20 | 1_00 * 2_00 + 1_01 * 2_10
	flds	4(%edx)		// 2_01 | 1_02 * 2_20 | 1_00 * 2_00 + 1_01 * 2_10
	fmuls	(%eax)		// 1_00 * 2_01 | 1_02 * 2_20 |
 						// 1_00 * 2_00 + 1_01 * 2_10
	flds	20(%edx)	// 2_11 | 1_00 * 2_01 | 1_02 * 2_20 |
 						// 1_00 * 2_00 + 1_01 * 2_10
	fmuls	4(%eax)		// 1_01 * 2_11 | 1_00 * 2_01 | 1_02 * 2_20 |
 						// 1_00 * 2_00 + 1_01 * 2_10
	flds	36(%edx)	// 2_21 | 1_01 * 2_11 | 1_00 * 2_01 | 1_02 * 2_20 |
 						// 1_00 * 2_00 + 1_01 * 2_10
	fmuls	8(%eax)		// 1_02 * 2_21 | 1_01 * 2_11 | 1_00 * 2_01 |
 						// 1_02 * 2_20 | 1_00 * 2_00 + 1_01 * 2_10
	fxch	%st(1)		// 1_01 * 2_11 | 1_02 * 2_21 | 1_00 * 2_01 |
 						// 1_02 * 2_20 | 1_00 * 2_00 + 1_01 * 2_10
	faddp	%st, %st(2)	// 1_02 * 2_21 | 1_00 * 2_01 + 1_01 * 2_11 |
 						// 1_02 * 2_20 | 1_00 * 2_00 + 1_01 * 2_10
	flds	8(%edx)		// 2_02 |
						// 1_02 * 2_21 | 1_00 * 2_01 + 1_01 * 2_11 |
 						// 1_02 * 2_20 | 1_00 * 2_00 + 1_01 * 2_10
	fmuls	(%eax)		// 1_00 * 2_02 |
						// 1_02 * 2_21 | 1_00 * 2_01 + 1_01 * 2_11 |
 						// 1_02 * 2_20 | 1_00 * 2_00 + 1_01 * 2_10
	flds	24(%edx)	// 2_12 | 1_00 * 2_02 |
						// 1_02 * 2_21 | 1_00 * 2_01 + 1_01 * 2_11 |
 						// 1_02 * 2_20 | 1_00 * 2_00 + 1_01 * 2_10
	fmuls	4(%eax)		// 1_01 * 2_12 | 1_00 * 2_02 |
						// 1_02 * 2_21 | 1_00 * 2_01 + 1_01 * 2_11 |
 						// 1_02 * 2_20 | 1_00 * 2_00 + 1_01 * 2_10
	flds	40(%edx)	// 2_22 | 1_01 * 2_12 | 1_00 * 2_02 |
						// 1_02 * 2_21 | 1_00 * 2_01 + 1_01 * 2_11 |
 						// 1_02 * 2_20 | 1_00 * 2_00 + 1_01 * 2_10
	fmuls	8(%eax)		// 1_02 * 2_22 | 1_01 * 2_12 | 1_00 * 2_02 |
						// 1_02 * 2_21 | 1_00 * 2_01 + 1_01 * 2_11 |
 						// 1_02 * 2_20 | 1_00 * 2_00 + 1_01 * 2_10
	fxch	%st(1)		// 1_01 * 2_12 | 1_02 * 2_22 | 1_00 * 2_02 |
						// 1_02 * 2_21 | 1_00 * 2_01 + 1_01 * 2_11 |
 						// 1_02 * 2_20 | 1_00 * 2_00 + 1_01 * 2_10
	faddp	%st, %st(1)	// 1_02 * 2_22 | 1_00 * 2_02 + 1_01 * 2_12 |
						// 1_02 * 2_21 | 1_00 * 2_01 + 1_01 * 2_11 |
 						// 1_02 * 2_20 | 1_00 * 2_00 + 1_01 * 2_10
	fxch	%st(4)		// 1_02 * 2_20 | 1_00 * 2_02 + 1_01 * 2_12 |
						// 1_02 * 2_21 | 1_00 * 2_01 + 1_01 * 2_11 |
 						// 1_02 * 2_22 | 1_00 * 2_00 + 1_01 * 2_10
	faddp	%st, %st(5)	// 1_00 * 2_02 + 1_01 * 2_12 | 1_02 * 2_21 |
						// 1_00 * 2_01 + 1_01 * 2_11 | 1_02 * 2_22 | o_00
	fxch	%st(1)		// 1_02 * 2_21 | 1_00 * 2_02 + 1_01 * 2_12 |
						// 1_00 * 2_01 + 1_01 * 2_11 | 1_02 * 2_22 | o_00
	faddp	%st, %st(2)	// 1_00 * 2_02 + 1_01 * 2_12 | o_01 | 1_02 * 2_22 |
						// o_00
	faddp	%st, %st(2)	// o_01 | o_02 | o_00
	fxch	%st(2)		// o_00 | o_02 | o_01
	fstps	(%ecx)		// o_02 | o_01
	fxch	%st(1)		// o_01 | o_02
	fstps	4(%ecx)		// o_02
	fstps	8(%ecx)		//
	
	flds	12(%edx)
	flds	28(%edx)
	fxch	%st(1)
	fmuls	(%eax)
	fxch	%st(1)
	fmuls	4(%eax)
	faddp	%st, %st(1)
	flds	44(%edx)
	fmuls	8(%eax)
	faddp	%st, %st(1)
	fadds	12(%eax)
	fstps	12(%ecx)
	flds	(%edx)
	flds	16(%edx)
	fxch	%st(1)
	fmuls	16(%eax)
	fxch	%st(1)
	fmuls	20(%eax)
	faddp	%st, %st(1)
	flds	32(%edx)
	fmuls	24(%eax)
	faddp	%st, %st(1)
	fstps	16(%ecx)
	flds	4(%edx)
	flds	20(%edx)
	fxch	%st(1)
	fmuls	16(%eax)
	fxch	%st(1)
	fmuls	20(%eax)
	faddp	%st, %st(1)
	flds	36(%edx)
	fmuls	24(%eax)
	faddp	%st, %st(1)
	fstps	20(%ecx)
	flds	8(%edx)
	flds	24(%edx)
	fxch	%st(1)
	fmuls	16(%eax)
	fxch	%st(1)
	fmuls	20(%eax)
	faddp	%st, %st(1)
	flds	40(%edx)
	fmuls	24(%eax)
	faddp	%st, %st(1)
	fstps	24(%ecx)
	flds	12(%edx)
	flds	28(%edx)
	fxch	%st(1)
	fmuls	16(%eax)
	fxch	%st(1)
	fmuls	20(%eax)
	faddp	%st, %st(1)
	flds	44(%edx)
	fmuls	24(%eax)
	faddp	%st, %st(1)
	fadds	28(%eax)
	fstps	28(%ecx)
	flds	(%edx)
	flds	16(%edx)
	fxch	%st(1)
	fmuls	32(%eax)
	fxch	%st(1)
	fmuls	36(%eax)
	faddp	%st, %st(1)
	flds	32(%edx)
	fmuls	40(%eax)
	faddp	%st, %st(1)
	fstps	32(%ecx)
	flds	4(%edx)
	flds	20(%edx)
	fxch	%st(1)
	fmuls	32(%eax)
	fxch	%st(1)
	fmuls	36(%eax)
	faddp	%st, %st(1)
	flds	36(%edx)
	fmuls	40(%eax)
	faddp	%st, %st(1)
	fstps	36(%ecx)
	flds	8(%edx)
	flds	24(%edx)
	fxch	%st(1)
	fmuls	32(%eax)
	fxch	%st(1)
	fmuls	36(%eax)
	faddp	%st, %st(1)
	flds	40(%edx)
	fmuls	40(%eax)
	faddp	%st, %st(1)
	fstps	40(%ecx)
	flds	12(%edx)
	flds	28(%edx)
	fxch	%st(1)
	fmuls	32(%eax)
	fxch	%st(1)
	fmuls	36(%eax)
	faddp	%st, %st(1)
	flds	44(%edx)
	fmuls	40(%eax)
	faddp	%st, %st(1)
	fadds	44(%eax)
	fstps	44(%ecx)
	ret

//==========================================================================
//
//	D_AliasTransformFinalVert
//
//==========================================================================

.globl C(D_AliasTransformFinalVert)
C(D_AliasTransformFinalVert):
	pushl	%ebx
	movl	16(%esp),%edx

	movl	8(%esp),%ebx
	movl	12(%esp),%ecx

	movb	(%edx),%al
	movb	%al,Lcoords
	movb	1(%edx),%al
	movb	%al,Lcoords+4
	movb	2(%edx),%al
	movb	%al,Lcoords+8
	xorl	%eax,%eax
	movb	3(%edx),%al
	leal	(%eax,%eax,2),%eax
	leal	C(d_avertexnormals)(,%eax,4),%eax

	fildl	Lcoords					// v[0]
	fildl	Lcoords+4				// v[1] | v[0]
	fildl	Lcoords+8				// v[2] | v[1] | v[0]
	fld		%st(2)					// v[0] | v[2] | v[1] | v[0]
	fmuls	C(aliastransform)+0		// xaccum1 | v[2] | v[1] | v[0]
	fld		%st(2)					// v[1] | xaccum1 | v[2] | v[1] | v[0]
	fmuls	C(aliastransform)+4		// xaccum2 | xaccum1 | v[2] | v[1] | v[0]
	fxch	%st(1)					// xaccum1 | xaccum2 | v[2] | v[1] | v[0]
	fadds	C(aliastransform)+12	// xaccum14 | xaccum2 | v[2] | v[1] |
									// v[0]
	fld		%st(2)					// v[2] | xaccum14 | xaccum2 | v[2] |
									// v[1] | v[0]
	fmuls	C(aliastransform)+8		// xaccum3 | xaccum14 | xaccum2 | v[2] |
									// v[1] | v[0]
	fxch	%st(1)					// xaccum14 | xaccum3 | xaccum2 | v[2] |
									// v[1] | v[0]
	faddp	%st(0),%st(2)			// xaccum3 | xaccum124 | v[2] | v[1] |
									// v[0]
	fld		%st(4)					// v[0] | xaccum3 | xaccum124 | v[2] |
									// v[1] | v[0]
	fmuls	C(aliastransform)+16	// yaccum1 | xaccum3 | xaccum124 | v[2] |
									// v[1] | v[0]
	fld		%st(4)					// v[1] | yaccum1 | xaccum3 | xaccum124 |
									// v[2] | v[1] | v[0]
	fmuls	C(aliastransform)+20	// yaccum2 | yaccum1 | xaccum3 |
									// xaccum124 | v[2] | v[1] | v[0]
	fxch	%st(1)					// yaccum1 | yaccum2 | xaccum3 |
									// xaccum124 | v[2] | v[1] | v[0]
	fadds	C(aliastransform)+28	// yaccum14 | yaccum2 | xaccum3 |
									// xaccum124 | v[2] | v[1] | v[0]
	fld		%st(4)					// v[2] | yaccum14 | yaccum2 | xaccum3 |
									// xaccum124 | v[2] | v[1] | v[0]
	fmuls	C(aliastransform)+24	// yaccum3 | yaccum14 | yaccum2 |
									// xaccum3 | xaccum124 | v[2] | v[1] |
									// v[0]
	fxch	%st(1)					// yaccum14 | yaccum3 | yaccum2 |
									// xaccum3 | xaccum124 | v[2] | v[1] |
									// v[0]
	faddp	%st(0),%st(2)			// yaccum3 | yaccum124 | xaccum3 |
									// xaccum124 | v[2] | v[1] | v[0]
	fxch	%st(6)					// v[0] | yaccum124 | xaccum3 |
									// xaccum124 | v[2] | v[1] | yaccum3
	fmuls	C(aliastransform)+32	// zaccum1 | yaccum124 | xaccum3 |
									// xaccum124 | v[2] | v[1] | yaccum3
	fxch	%st(5)					// v[1] | yaccum124 | xaccum3 |
									// xaccum124 | v[2] | zaccum1 | yaccum3
	fmuls	C(aliastransform)+36	// zaccum2 | yaccum124 | xaccum3 |
									// xaccum124 | v[2] | zaccum1 | yaccum3
	fxch	%st(4)					// v[2] | yaccum124 | xaccum3 |
									// xaccum124 | zaccum2 | zaccum1 |
									// yaccum3
	fmuls	C(aliastransform)+40	// zaccum3 | yaccum124 | xaccum3 |
									// xaccum124 | zaccum2 | zaccum1 |
									// yaccum3
	fxch	%st(5)					// zaccum1 | yaccum124 | xaccum3 |
									// xaccum124 | zaccum2 | zaccum3 |
									// yaccum3
	fadds	C(aliastransform)+44	// zaccum14 | yaccum124 | xaccum3 |
									// xaccum124 | zaccum2 | zaccum3 |
									// yaccum3
	fxch	%st(2)					// xaccum3 | yaccum124 | zaccum14 |
									// xaccum124 | zaccum2 | zaccum3 |
									// yaccum3
	faddp	%st(0),%st(3)			// yaccum124 | zaccum14 | x | zaccum2 |
									// zaccum3 | yaccum3
	fxch	%st(3)					// zaccum2 | zaccum14 | x | yaccum124 |
									// zaccum3 | yaccum3
	faddp	%st(0),%st(4)			// zaccum14 | x | yaccum124 | zaccum23 |
									// yaccum3
	fxch	%st(2)					// yaccum124 | x | zaccum14 | zaccum23 |
									// yaccum3
	faddp	%st(0),%st(4)			// x | zaccum14 | zaccum23 | y
	fstps	(%ecx)					// zaccum14 | zaccum23 | y
	faddp	%st(0),%st(1)			// z | y
	flds	(%eax)					// nx | z | y
	fmuls	C(d_plightvec)			// lx*nx | z | y
	flds	4(%eax)					// ny | lx*nx | z | y
	fmuls	C(d_plightvec)+4		// ly*ny | lx*nx | z | y
	flds	8(%eax)					// nz | ly*ny | lx*nx | z | y
	fmuls	C(d_plightvec)+8		// lz*nz | ly*ny | lx*nx | z | y
	fxch	%st(2)					// lx*nx | ly*ny | lz*nz | z | y
	faddp	%st(0),%st(1)			// lx*nx + ly*ny | lz*nz | z | y
	fxch	%st(3)					// y | lz*nz | z | lx*nx + ly*ny
	fstps	4(%ecx)					// lz*nz | z | lx*nx + ly*ny
	faddp	%st(0),%st(2)			// z | lightcos
	fstps	8(%ecx)					// lightcos

	movl	C(d_ambientlightr),%edx
	movl	C(d_ambientlightg),%eax
	movl	%eax,16(%ebx)
	movl	C(d_ambientlightb),%eax
	movl	%edx,12(%ebx)
	movl	%eax,20(%ebx)
	fsts	Ltemp
	movl	$0,24(%ebx)
	movb	Ltemp+3,%al
	testb	$0x80,%al
	jz		LTFVPopAndDone

	flds	C(d_shadelightr)		// shader | lightcos
	fmul	%st(1),%st(0)			// r | lightcos
	flds	C(d_shadelightg)		// shadeg | r | lightcos
	fmul	%st(2),%st(0)			// g | r | lightcos
	flds	C(d_shadelightb)		// shadeb | g | r | lightcos
	fmulp	%st(0),%st(3)			// g | r | b
	fxch	%st(1)					// r | g | b
	fistpl	Ltemp					// g | b
	movl	Ltemp,%eax
	addl	%edx,%eax
	movl	%eax,12(%ebx)
	jns		LTFVSkip1
	movl	$0,12(%ebx)
LTFVSkip1:
	fistpl	Ltemp					// b
	movl	Ltemp,%eax
	addl	16(%ebx),%eax
	movl	%eax,16(%ebx)
	jns		LTFVSkip2
	movl	$0,16(%ebx)
LTFVSkip2:
	fistpl	Ltemp					//
	movl	Ltemp,%eax
	addl	20(%ebx),%eax
	movl	%eax,20(%ebx)
	jns		LTFVDone
	movl	$0,20(%ebx)
	jmp		LTFVDone
LTFVPopAndDone:
	fstp	%st(0)
LTFVDone:
	popl	%ebx
	ret

//==========================================================================
//
//	D_AliasProjectFinalVert
//
//==========================================================================

.globl C(D_AliasProjectFinalVert)
C(D_AliasProjectFinalVert):
	movl	4(%esp),%ecx
	movl	8(%esp),%edx
	fld1					// 1
	flds	8(%edx)			// z | 1
	fdivrp	%st(0),%st(1)	// zi
	flds	(%edx)			// x | zi
	fmuls	C(xprojection)	// x * xprojection | zi
	flds	4(%edx)			// y | x * xprojection | zi
	fmuls	C(yprojection)	// y * yprojection | x * xprojection | zi
	flds	C(ziscale)		// ziscale | y * yprojection | x * xprojection |
							// zi
	fxch	%st(3)			// zi | y * yprojection | x * xprojection |
							// ziscale
	fmul	%st(0),%st(2)	// zi | y * yprojection | x * xprojection * zi |
							// ziscale
	fmul	%st(0),%st(1)	// zi | y * yprojection * zi |
							// x * xprojection * zi | ziscale
	fmulp	%st(0),%st(3)	// y * yprojection * zi | x * xprojection * zi |
							// ziscale * zi
	fxch	%st(1)			// x * xprojection * zi | y * yprojection * zi |
							// ziscale * zi
	fadds	C(aliasxcentre)	// x * xprojection * zi + aliasxcentre |
							// y * yprojection * zi | ziscale * zi
	fxch	%st(1)			// y * yprojection * zi |
							// x * xprojection * zi + aliasxcentre |
							// ziscale * zi
	fadds	C(aliasycentre)	// y * yprojection * zi + aliasycentre |
							// x * xprojection * zi + aliasxcentre |
							// ziscale * zi
	fxch	%st(2)			// ziscale * zi |
							// x * xprojection * zi + aliasxcentre |
							// y * yprojection * zi + aliasycentre
	fistpl	8(%ecx)			// x * xprojection * zi + aliasxcentre |
							// y * yprojection * zi + aliasycentre
	fistpl	(%ecx)			// y * yprojection * zi + aliasycentre
	fistpl	4(%ecx)			//
	ret

//==========================================================================
//
//	D_AliasTransformAndProjectFinalVerts
//
//==========================================================================

.globl C(D_AliasTransformAndProjectFinalVerts)
C(D_AliasTransformAndProjectFinalVerts):
	pushl	%ebp
	pushl	%edi
	pushl	%esi
	pushl	%ebx
	movl	C(d_anumverts),%ebp
	movl	C(d_apverts),%edi
	movl	20(%esp),%esi

LTLoop:
	xorl	%eax,%eax
	movb	(%edi),%al
	movb	%al,Lcoords
	movb	1(%edi),%al
	movb	%al,Lcoords+4
	movb	2(%edi),%al
	movb	%al,Lcoords+8
	movb	3(%edi),%al

	fildl	Lcoords					// v[0]
	fildl	Lcoords+4				// v[1] | v[0]
	leal	(%eax,%eax,2),%eax
	fildl	Lcoords+8				// v[2] | v[1] | v[0]
	sall	$2,%eax
	fld		%st(2)					// v[0] | v[2] | v[1] | v[0]
	fmuls	C(aliastransform)+32	// zaccum1 | v[2] | v[1] | v[0]
	fld		%st(2)					// v[1] | zaccum1 | v[2] | v[1] | v[0]
	fmuls	C(aliastransform)+36	// zaccum2 | zaccum1 | v[2] | v[1] | v[0]
	fxch	%st(1)					// zaccum1 | zaccum2 | v[2] | v[1] | v[0]
	fadds	C(aliastransform)+44	// zaccum14 | zaccum2 | v[2] | v[1] |
									// v[0]
	flds	C(d_avertexnormals)+0(%eax)	// avn[0] | zaccum14 | zaccum2 |
									// v[2] | v[1] | v[0]
	fmuls	C(d_plightvec)+0		// laccum1 | zaccum14 | zaccum2 | v[2] |
									// v[1] | v[0]
	flds	C(d_avertexnormals)+4(%eax)	// avn[1] | laccum1 | zaccum14 |
									// zaccum2 | v[2] | v[1] | v[0]
	fmuls	C(d_plightvec)+4		// laccum2 | laccum1 | zaccum14 |
									// zaccum2 | v[2] | v[1] | v[0]
	flds	C(d_avertexnormals)+8(%eax)	// avn[2] | laccum2 | laccum1 |
									// zaccum14 | zaccum2 | v[2] | v[1] |
									// v[0]
	fmuls	C(d_plightvec)+8		// laccum3 | laccum2 | laccum1 |
									// zaccum14 | zaccum2 | v[2] | v[1] |
									// v[0]
	fxch	%st(2)					// laccum1 | laccum2 | laccum3 |
									// zaccum14 | zaccum2 | v[2] | v[1] |
									// v[0]
	faddp	%st(0),%st(1)			// laccum12 | laccum3 | zaccum14 |
									// zaccum2 | v[2] | v[1] | v[0]
	fxch	%st(2)					// zaccum14 | laccum3 | laccum12 |
									// zaccum2 | v[2] | v[1] | v[0]
	faddp	%st(0),%st(3)			// laccum3 | laccum12 | zaccum124 |
									// v[2] | v[1] | v[0]
	fld		%st(3)					// v[2] | laccum3 | laccum12 |
									// zaccum124 | v[2] | v[1] | v[0]
	fmuls	C(aliastransform)+40	// zaccum3 | laccum3 | laccum12 |
									// zaccum124 | v[2] | v[1] | v[0]
	fxch	%st(2)					// laccum12 | laccum3 | zaccum3 |
									// zaccum124 | v[2] | v[1] | v[0]
	faddp	%st(0),%st(1)			// lightcos | zaccum3 | zaccum124 |
									// v[2] | v[1] | v[0]
	fld1							// 1 | lightcos | zaccum3 | zaccum124 |
									// v[2] | v[1] | v[0]
	fxch	%st(2)					// zaccum3 | lightcos | 1 | zaccum124 |
									// v[2] | v[1] | v[0]
	faddp	%st(0),%st(3)			// lightcos | 1 | z | v[2] | v[1] | v[0]
	fsts	Ltemp					// lightcos | 1 | z | v[2] | v[1] | v[0]
	fxch	%st(2)					// z | 1 | lightcos | v[2] | v[1] | v[0]
	fdivrp	%st(0),%st(1)			// zi | lightcos | v[2] | v[1] | v[0]
	movl	C(d_ambientlightr),%ecx
	movl	C(d_ambientlightg),%edx
	movl	C(d_ambientlightb),%ebx
	movb	Ltemp+3,%al
	testb	$0x80,%al
	jz		Lp4

	flds	C(d_shadelightr)		// shadelr | zi | lightcos | v[2] |
									// v[1] | v[0]
	fmul	%st(2),%st(0)			// r | zi | lightcos | v[2] | v[1] |
									// v[0]
	flds	C(d_shadelightg)		// shadelg | r | zi | lightcos | v[2] |
									// v[1] | v[0]
	fmul	%st(3),%st(0)			// g | r | zi | lightcos | v[2] |
									// v[1] | v[0]
	flds	C(d_shadelightb)		// shadelb | g | r | zi | lightcos |
									// v[2] | v[1] | v[0]
	fmulp	%st(0),%st(4)			// g | r | zi | b | v[2] | v[1] | v[0]
	fxch	%st(1)					// r | g | zi | b | v[2] | v[1] | v[0]
	fistpl	Ltemp					// g | zi | b | v[2] | v[1] | v[0]
	movl	Ltemp,%eax
	addl	%eax,%ecx
	jns		Lp1
	xorl	%ecx,%ecx
Lp1:
	fistpl	Ltemp					// zi | b | v[2] | v[1] | v[0]
	movl	Ltemp,%eax
	addl	%eax,%edx
	jns		Lp2
	xorl	%edx,%edx
Lp2:
	fxch	%st(1)					// b | zi | v[2] | v[1] | v[0]
	fistpl	Ltemp					// zi | v[2] | v[1] | v[0]
	movl	Ltemp,%eax
	addl	%eax,%ebx
	jns		Lp3
	xorl	%ebx,%ebx
	jmp		Lp3

Lp4:
	fstp	%st(1)					// zi | v[2] | v[1] | v[0]

Lp3:
	fld		%st(3)					// v[0] | zi | v[2] | v[1] | v[0]
	fmuls	C(aliastransform)+0		// xaccum1 | zi | v[2] | v[1] | v[0]
	fld		%st(3)					// v[1] | xaccum1 | zi | v[2] | v[1] |
									// v[0]
	fmuls	C(aliastransform)+4		// xaccum2 | xaccum1 | zi | v[2] |
									// v[1] | v[0]
	fxch	%st(1)					// xaccum1 | xaccum2 | zi | v[2] |
									// v[1] | v[0]
	fadds	C(aliastransform)+12	// xaccum14 | xaccum2 | zi | v[2] |
									// v[1] | v[0]
	fld		%st(3)					// v[2] | xaccum14 | xaccum2 | zi |
									// v[2] | v[1] | v[0]
	fmuls	C(aliastransform)+8		// xaccum3 | xaccum14 | xaccum2 | zi |
									// v[2] | v[1] | v[0]
	fxch	%st(1)					// xaccum14 | xaccum3 | xaccum2 | zi |
									// v[2] | v[1] | v[0]
	faddp	%st(0),%st(2)			// xaccum3 | xaccum124 | zi | v[2] |
									// v[1] | v[0]
	fxch	%st(5)					// v[0] | xaccum124 | zi | v[2] | v[1] |
									// xaccum3
	fmuls	C(aliastransform)+16	// yaccum1 | xaccum124 | zi | v[2] |
									// v[1] | xaccum3
	fxch	%st(4)					// v[1] | xaccum124 | zi | v[2] |
									// yaccum1 | xaccum3
	fmuls	C(aliastransform)+20	// yaccum2 | xaccum124 | zi | v[2] |
									// yaccum1 | xaccum3
	fxch	%st(1)					// xaccum124 | yaccum2 | zi | v[2] |
									// yaccum1 | xaccum3
	faddp	%st(0),%st(5)			// yaccum2 | zi | v[2] | yaccum1 | x
	fxch	%st(3)					// yaccum1 | zi | v[2] | yaccum2 | x
	fadds	C(aliastransform)+28	// yaccum14 | zi | v[2] | yaccum2 | x
	fxch	%st(2)					// v[2] | zi | yaccum14 | yaccum2 | x
	fmuls	C(aliastransform)+24	// yaccum3 | zi | yaccum14 | yaccum2 | x
	fxch	%st(4)					// x | zi | yaccum14 | yaccum2 | yaccum3
	fmul	%st(1),%st(0)			// x * zi | zi | yaccum14 | yaccum2 |
									// yaccum3
	fxch	%st(2)					// yaccum14 | zi | x * zi | yaccum2 |
									// yaccum3
	faddp	%st(0),%st(3)			// zi | x * zi | yaccum124 | yaccum3
	movl	%ecx,12(%esi)
	movl	%edx,16(%esi)
	fxch	%st(1)					// x * zi | zi | yaccum124 | yaccum3
	fadds	C(aliasxcentre)			// x * zi + aliasxcentre | zi |
									// yaccum124 | yaccum3
	fxch	%st(2)					// yaccum124 | zi |
									// x * zi + aliasxcentre | yaccum3
	faddp	%st(0),%st(3)			// zi | x * zi + aliasxcentre | y
	fistl	8(%esi)					// zi | x * zi + aliasxcentre | y
	fmulp	%st(0),%st(2)			// x * zi + aliasxcentre | y * zi
	fistpl	(%esi)					// y * zi
	fadds	C(aliasycentre)			// y * zi + aliasycentre
	movl	%ebx,20(%esi)
	movl	$0,24(%esi)
	fistpl	4(%esi)					//

	addl	$4,%edi
	addl	$32,%esi
	decl	%ebp
	jnz		LTLoop

	popl	%ebx
	popl	%esi
	popl	%edi
	popl	%ebp
	ret

#endif
