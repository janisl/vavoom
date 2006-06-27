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
//**
//**	x86 assembly-language 8 bpp surface block drawing code.
//**
//**************************************************************************

#include "asm_i386.h"

#ifdef USEASM

//==========================================================================
//
//
//
//==========================================================================

	.data

sb_v:		.long	0

	.text

	Align4
.globl C(D_Surf8Start)
C(D_Surf8Start):

//----------------------------------------------------------------------
// Surface block drawer for mip level 0
//----------------------------------------------------------------------

	Align4
.globl C(D_DrawSurfaceBlock8_mip0)
C(D_DrawSurfaceBlock8_mip0):
	pushl	%ebp				// preserve caller's stack frame
	pushl	%edi
	pushl	%esi				// preserve register variables
	pushl	%ebx

//		for (v=0 ; v<numvblocks ; v++)
//		{
	movl	C(r_lightptr),%ebx
	movl	C(r_numvblocks),%eax

	movl	%eax,sb_v
	movl	C(prowdestbase),%edi

	movl	C(pbasesource),%esi

Lv_loop_mip0:

//			lightleft = lightptr[0];
//			lightright = lightptr[1];
//			lightdelta = (lightleft - lightright) & 0xFFFFF;
	movl	(%ebx),%eax			// lightleft
	movl	4(%ebx),%edx		// lightright

	movl	%eax,%ebp
	movl	C(r_lightwidth),%ecx

	movl	%edx,C(lightright)
	subl	%edx,%ebp

	andl	$0xFFFFF,%ebp
	leal	(%ebx,%ecx,4),%ebx

//			lightptr += lightwidth;
	movl	%ebx,C(r_lightptr)

//			lightleftstep = (lightptr[0] - lightleft) >> blockdivshift;
//			lightrightstep = (lightptr[1] - lightright) >> blockdivshift;
//			lightdeltastep = ((lightleftstep - lightrightstep) & 0xFFFFF) |
//					0xF0000000;
	movl	4(%ebx),%ecx	// lightptr[1]
	movl	(%ebx),%ebx		// lightptr[0]

	subl	%eax,%ebx
	subl	%edx,%ecx

	sarl	$4,%ecx
	orl		$0xF0000000,%ebp

	sarl	$4,%ebx
	movl	%ecx,C(lightrightstep)

	subl	%ecx,%ebx
	andl	$0xFFFFF,%ebx

	orl		$0xF0000000,%ebx
	subl	%ecx,%ecx	// high word must be 0 in loop for addressing

	movl	%ebx,C(lightdeltastep)
	subl	%ebx,%ebx	// high word must be 0 in loop for addressing

Lblockloop8_mip0:
	movl	%ebp,C(lightdelta)
	movb	14(%esi),%cl

	sarl	$4,%ebp
	movb	%dh,%bh

	movb	15(%esi),%bl
	addl	%ebp,%edx

	movb	%dh,%ch
	addl	%ebp,%edx

	movb	0x12345678(%ebx),%ah
LPatch0:
	movb	13(%esi),%bl

	movb	0x12345678(%ecx),%al
LPatch1:
	movb	12(%esi),%cl

	movb	%dh,%bh
	addl	%ebp,%edx

	rorl	$16,%eax
	movb	%dh,%ch

	addl	%ebp,%edx
	movb	0x12345678(%ebx),%ah
LPatch2:

	movb	11(%esi),%bl
	movb	0x12345678(%ecx),%al
LPatch3:

	movb	10(%esi),%cl
	movl	%eax,12(%edi)

	movb	%dh,%bh
	addl	%ebp,%edx

	movb	%dh,%ch
	addl	%ebp,%edx

	movb	0x12345678(%ebx),%ah
LPatch4:
	movb	9(%esi),%bl

	movb	0x12345678(%ecx),%al
LPatch5:
	movb	8(%esi),%cl

	movb	%dh,%bh
	addl	%ebp,%edx

	rorl	$16,%eax
	movb	%dh,%ch

	addl	%ebp,%edx
	movb	0x12345678(%ebx),%ah
LPatch6:

	movb	7(%esi),%bl
	movb	0x12345678(%ecx),%al
LPatch7:

	movb	6(%esi),%cl
	movl	%eax,8(%edi)

	movb	%dh,%bh
	addl	%ebp,%edx

	movb	%dh,%ch
	addl	%ebp,%edx

	movb	0x12345678(%ebx),%ah
LPatch8:
	movb	5(%esi),%bl

	movb	0x12345678(%ecx),%al
LPatch9:
	movb	4(%esi),%cl

	movb	%dh,%bh
	addl	%ebp,%edx

	rorl	$16,%eax
	movb	%dh,%ch

	addl	%ebp,%edx
	movb	0x12345678(%ebx),%ah
LPatch10:

	movb	3(%esi),%bl
	movb	0x12345678(%ecx),%al
LPatch11:

	movb	2(%esi),%cl
	movl	%eax,4(%edi)

	movb	%dh,%bh
	addl	%ebp,%edx

	movb	%dh,%ch
	addl	%ebp,%edx

	movb	0x12345678(%ebx),%ah
LPatch12:
	movb	1(%esi),%bl

	movb	0x12345678(%ecx),%al
LPatch13:
	movb	(%esi),%cl

	movb	%dh,%bh
	addl	%ebp,%edx

	rorl	$16,%eax
	movb	%dh,%ch

	movb	0x12345678(%ebx),%ah
LPatch14:
	movl	C(lightright),%edx

	movb	0x12345678(%ecx),%al
LPatch15:
	movl	C(lightdelta),%ebp

	movl	%eax,(%edi)

	addl	C(sourcetstep),%esi
	addl	C(surfrowbytes),%edi

	addl	C(lightrightstep),%edx
	addl	C(lightdeltastep),%ebp

	movl	%edx,C(lightright)
	jc		Lblockloop8_mip0

//			if (pbasesource >= r_sourcemax)
//				pbasesource -= stepback;

	cmpl	C(r_sourcemax),%esi
	jb		LSkip_mip0
	subl	C(r_stepback),%esi
LSkip_mip0:

	movl	C(r_lightptr),%ebx
	decl	sb_v

	jnz		Lv_loop_mip0

	popl	%ebx				// restore register variables
	popl	%esi
	popl	%edi
	popl	%ebp				// restore the caller's stack frame
	ret


//----------------------------------------------------------------------
// Surface block drawer for mip level 1
//----------------------------------------------------------------------

	Align4
.globl C(D_DrawSurfaceBlock8_mip1)
C(D_DrawSurfaceBlock8_mip1):
	pushl	%ebp				// preserve caller's stack frame
	pushl	%edi
	pushl	%esi				// preserve register variables
	pushl	%ebx

//		for (v=0 ; v<numvblocks ; v++)
//		{
	movl	C(r_lightptr),%ebx
	movl	C(r_numvblocks),%eax

	movl	%eax,sb_v
	movl	C(prowdestbase),%edi

	movl	C(pbasesource),%esi

Lv_loop_mip1:

//			lightleft = lightptr[0];
//			lightright = lightptr[1];
//			lightdelta = (lightleft - lightright) & 0xFFFFF;
	movl	(%ebx),%eax			// lightleft
	movl	4(%ebx),%edx		// lightright

	movl	%eax,%ebp
	movl	C(r_lightwidth),%ecx

	movl	%edx,C(lightright)
	subl	%edx,%ebp

	andl	$0xFFFFF,%ebp
	leal	(%ebx,%ecx,4),%ebx

//			lightptr += lightwidth;
	movl	%ebx,C(r_lightptr)

//			lightleftstep = (lightptr[0] - lightleft) >> blockdivshift;
//			lightrightstep = (lightptr[1] - lightright) >> blockdivshift;
//			lightdeltastep = ((lightleftstep - lightrightstep) & 0xFFFFF) |
//					0xF0000000;
	movl	4(%ebx),%ecx	// lightptr[1]
	movl	(%ebx),%ebx		// lightptr[0]

	subl	%eax,%ebx
	subl	%edx,%ecx

	sarl	$3,%ecx
	orl		$0x70000000,%ebp

	sarl	$3,%ebx
	movl	%ecx,C(lightrightstep)

	subl	%ecx,%ebx
	andl	$0xFFFFF,%ebx

	orl		$0xF0000000,%ebx
	subl	%ecx,%ecx	// high word must be 0 in loop for addressing

	movl	%ebx,C(lightdeltastep)
	subl	%ebx,%ebx	// high word must be 0 in loop for addressing

Lblockloop8_mip1:
	movl	%ebp,C(lightdelta)
	movb	6(%esi),%cl

	sarl	$3,%ebp
	movb	%dh,%bh

	movb	7(%esi),%bl
	addl	%ebp,%edx

	movb	%dh,%ch
	addl	%ebp,%edx

	movb	0x12345678(%ebx),%ah
LPatch22:
	movb	5(%esi),%bl

	movb	0x12345678(%ecx),%al
LPatch23:
	movb	4(%esi),%cl

	movb	%dh,%bh
	addl	%ebp,%edx

	rorl	$16,%eax
	movb	%dh,%ch

	addl	%ebp,%edx
	movb	0x12345678(%ebx),%ah
LPatch24:

	movb	3(%esi),%bl
	movb	0x12345678(%ecx),%al
LPatch25:

	movb	2(%esi),%cl
	movl	%eax,4(%edi)

	movb	%dh,%bh
	addl	%ebp,%edx

	movb	%dh,%ch
	addl	%ebp,%edx

	movb	0x12345678(%ebx),%ah
LPatch26:
	movb	1(%esi),%bl

	movb	0x12345678(%ecx),%al
LPatch27:
	movb	(%esi),%cl

	movb	%dh,%bh
	addl	%ebp,%edx

	rorl	$16,%eax
	movb	%dh,%ch

	movb	0x12345678(%ebx),%ah
LPatch28:
	movl	C(lightright),%edx

	movb	0x12345678(%ecx),%al
LPatch29:
	movl	C(lightdelta),%ebp

	movl	%eax,(%edi)
	movl	C(sourcetstep),%eax

	addl	%eax,%esi
	movl	C(surfrowbytes),%eax

	addl	%eax,%edi
	movl	C(lightrightstep),%eax

	addl	%eax,%edx
	movl	C(lightdeltastep),%eax

	addl	%eax,%ebp
	movl	%edx,C(lightright)

	jc		Lblockloop8_mip1

//			if (pbasesource >= r_sourcemax)
//				pbasesource -= stepback;

	cmpl	C(r_sourcemax),%esi
	jb		LSkip_mip1
	subl	C(r_stepback),%esi
LSkip_mip1:

	movl	C(r_lightptr),%ebx
	decl	sb_v

	jnz		Lv_loop_mip1

	popl	%ebx				// restore register variables
	popl	%esi
	popl	%edi
	popl	%ebp				// restore the caller's stack frame
	ret


//----------------------------------------------------------------------
// Surface block drawer for mip level 2
//----------------------------------------------------------------------

	Align4
.globl C(D_DrawSurfaceBlock8_mip2)
C(D_DrawSurfaceBlock8_mip2):
	pushl	%ebp				// preserve caller's stack frame
	pushl	%edi
	pushl	%esi				// preserve register variables
	pushl	%ebx

//		for (v=0 ; v<numvblocks ; v++)
//		{
	movl	C(r_lightptr),%ebx
	movl	C(r_numvblocks),%eax

	movl	%eax,sb_v
	movl	C(prowdestbase),%edi

	movl	C(pbasesource),%esi

Lv_loop_mip2:

//			lightleft = lightptr[0];
//			lightright = lightptr[1];
//			lightdelta = (lightleft - lightright) & 0xFFFFF;
	movl	(%ebx),%eax			// lightleft
	movl	4(%ebx),%edx		// lightright

	movl	%eax,%ebp
	movl	C(r_lightwidth),%ecx

	movl	%edx,C(lightright)
	subl	%edx,%ebp

	andl	$0xFFFFF,%ebp
	leal	(%ebx,%ecx,4),%ebx

//			lightptr += lightwidth;
	movl	%ebx,C(r_lightptr)

//			lightleftstep = (lightptr[0] - lightleft) >> blockdivshift;
//			lightrightstep = (lightptr[1] - lightright) >> blockdivshift;
//			lightdeltastep = ((lightleftstep - lightrightstep) & 0xFFFFF) |
//					0xF0000000;
	movl	4(%ebx),%ecx	// lightptr[1]
	movl	(%ebx),%ebx		// lightptr[0]

	subl	%eax,%ebx
	subl	%edx,%ecx

	sarl	$2,%ecx
	orl		$0x30000000,%ebp

	sarl	$2,%ebx
	movl	%ecx,C(lightrightstep)

	subl	%ecx,%ebx

	andl	$0xFFFFF,%ebx

	orl		$0xF0000000,%ebx
	subl	%ecx,%ecx	// high word must be 0 in loop for addressing

	movl	%ebx,C(lightdeltastep)
	subl	%ebx,%ebx	// high word must be 0 in loop for addressing

Lblockloop8_mip2:
	movl	%ebp,C(lightdelta)
	movb	2(%esi),%cl

	sarl	$2,%ebp
	movb	%dh,%bh

	movb	3(%esi),%bl
	addl	%ebp,%edx

	movb	%dh,%ch
	addl	%ebp,%edx

	movb	0x12345678(%ebx),%ah
LPatch18:
	movb	1(%esi),%bl

	movb	0x12345678(%ecx),%al
LPatch19:
	movb	(%esi),%cl

	movb	%dh,%bh
	addl	%ebp,%edx

	rorl	$16,%eax
	movb	%dh,%ch

	movb	0x12345678(%ebx),%ah
LPatch20:
	movl	C(lightright),%edx

	movb	0x12345678(%ecx),%al
LPatch21:
	movl	C(lightdelta),%ebp

	movl	%eax,(%edi)
	movl	C(sourcetstep),%eax

	addl	%eax,%esi
	movl	C(surfrowbytes),%eax

	addl	%eax,%edi
	movl	C(lightrightstep),%eax

	addl	%eax,%edx
	movl	C(lightdeltastep),%eax

	addl	%eax,%ebp
	movl	%edx,C(lightright)

	jc		Lblockloop8_mip2

//			if (pbasesource >= r_sourcemax)
//				pbasesource -= stepback;

	cmpl	C(r_sourcemax),%esi
	jb		LSkip_mip2
	subl	C(r_stepback),%esi
LSkip_mip2:

	movl	C(r_lightptr),%ebx
	decl	sb_v

	jnz		Lv_loop_mip2

	popl	%ebx				// restore register variables
	popl	%esi
	popl	%edi
	popl	%ebp				// restore the caller's stack frame
	ret


//----------------------------------------------------------------------
// Surface block drawer for mip level 3
//----------------------------------------------------------------------

	Align4
.globl C(D_DrawSurfaceBlock8_mip3)
C(D_DrawSurfaceBlock8_mip3):
	pushl	%ebp				// preserve caller's stack frame
	pushl	%edi
	pushl	%esi				// preserve register variables
	pushl	%ebx

//		for (v=0 ; v<numvblocks ; v++)
//		{
	movl	C(r_lightptr),%ebx
	movl	C(r_numvblocks),%eax

	movl	%eax,sb_v
	movl	C(prowdestbase),%edi

	movl	C(pbasesource),%esi

Lv_loop_mip3:

//			lightleft = lightptr[0];
//			lightright = lightptr[1];
//			lightdelta = (lightleft - lightright) & 0xFFFFF;
	movl	(%ebx),%eax			// lightleft
	movl	4(%ebx),%edx		// lightright

	movl	%eax,%ebp
	movl	C(r_lightwidth),%ecx

	movl	%edx,C(lightright)
	subl	%edx,%ebp

	andl	$0xFFFFF,%ebp
	leal	(%ebx,%ecx,4),%ebx

	movl	%ebp,C(lightdelta)
//			lightptr += lightwidth;
	movl	%ebx,C(r_lightptr)

//			lightleftstep = (lightptr[0] - lightleft) >> blockdivshift;
//			lightrightstep = (lightptr[1] - lightright) >> blockdivshift;
//			lightdeltastep = ((lightleftstep - lightrightstep) & 0xFFFFF) |
//					0xF0000000;
	movl	4(%ebx),%ecx	// lightptr[1]
	movl	(%ebx),%ebx		// lightptr[0]

	subl	%eax,%ebx
	subl	%edx,%ecx

	sarl	$1,%ecx

	sarl	$1,%ebx
	movl	%ecx,C(lightrightstep)

	subl	%ecx,%ebx
	andl	$0xFFFFF,%ebx

	sarl	$1,%ebp
	orl		$0xF0000000,%ebx

	movl	%ebx,C(lightdeltastep)
	subl	%ebx,%ebx	// high word must be 0 in loop for addressing

	movb	1(%esi),%bl
	subl	%ecx,%ecx	// high word must be 0 in loop for addressing

	movb	%dh,%bh
	movb	(%esi),%cl

	addl	%ebp,%edx
	movb	%dh,%ch

	movb	0x12345678(%ebx),%al
LPatch16:
	movl	C(lightright),%edx

	movb	%al,1(%edi)
	movb	0x12345678(%ecx),%al
LPatch17:

	movb	%al,(%edi)
	movl	C(sourcetstep),%eax

	addl	%eax,%esi
	movl	C(surfrowbytes),%eax

	addl	%eax,%edi
	movl	C(lightdeltastep),%eax

	movl	C(lightdelta),%ebp
	movb	(%esi),%cl

	addl	%eax,%ebp
	movl	C(lightrightstep),%eax

	sarl	$1,%ebp
	addl	%eax,%edx

	movb	%dh,%bh
	movb	1(%esi),%bl

	addl	%ebp,%edx
	movb	%dh,%ch

	movb	0x12345678(%ebx),%al
LPatch30:
	movl	C(sourcetstep),%edx

	movb	%al,1(%edi)
	movb	0x12345678(%ecx),%al
LPatch31:

	movb	%al,(%edi)
	movl	C(surfrowbytes),%ebp

	addl	%edx,%esi
	addl	%ebp,%edi

//			if (pbasesource >= r_sourcemax)
//				pbasesource -= stepback;

	cmpl	C(r_sourcemax),%esi
	jb		LSkip_mip3
	subl	C(r_stepback),%esi
LSkip_mip3:

	movl	C(r_lightptr),%ebx
	decl	sb_v

	jnz		Lv_loop_mip3

	popl	%ebx				// restore register variables
	popl	%esi
	popl	%edi
	popl	%ebp				// restore the caller's stack frame
	ret


//==========================================================================
//
//	Colored lighting block drawers
//
//==========================================================================


//----------------------------------------------------------------------
// Surface block drawer for mip level 0
//----------------------------------------------------------------------

	Align16
.globl C(D_DrawSurfaceBlock8RGB_mip0)
C(D_DrawSurfaceBlock8RGB_mip0):
	movl	C(pbasesource),%eax
	subl	$60,%esp
	movl	C(prowdestbase),%edx
	pushl	%ebp
	pushl	%edi
	pushl	%esi
	pushl	%ebx
	xorl	%esi,%esi
	movl	%eax,36(%esp)
	movl	%edx,32(%esp)
	cmpl	C(r_numvblocks),%esi
	jge		L1013
L1000:
	movl	C(r_lightptrr),%edx
	movl	C(r_lightwidth),%ebx
	sall	$2,%ebx
	incl	%esi
	movl	(%edx),%eax
	movl	4(%edx),%ecx
	movl	%eax,C(lightrleft)
	movl	%ecx,C(lightrright)
	addl	%ebx,%edx
	movl	%edx,C(r_lightptrr)
	movl	(%edx),%edi
	subl	%eax,%edi
	movl	%edi,%eax
	shrl	$4,%eax
	movl	%eax,C(lightrleftstep)
	movl	4(%edx),%eax
	movl	C(r_lightptrg),%edx
	subl	%ecx,%eax
	shrl	$4,%eax
	movl	%eax,C(lightrrightstep)
	movl	(%edx),%eax
	movl	4(%edx),%ecx
	movl	%eax,C(lightgleft)
	movl	%ecx,C(lightgright)
	addl	%ebx,%edx
	movl	%edx,C(r_lightptrg)
	movl	(%edx),%edi
	subl	%eax,%edi
	movl	%edi,%eax
	shrl	$4,%eax
	movl	%eax,C(lightgleftstep)
	movl	4(%edx),%eax
	movl	C(r_lightptrb),%edx
	subl	%ecx,%eax
	shrl	$4,%eax
	movl	%eax,C(lightgrightstep)
	movl	(%edx),%eax
	movl	4(%edx),%ecx
	movl	%eax,C(lightbleft)
	movl	%ecx,C(lightbright)
	addl	%ebx,%edx
	movl	%edx,C(r_lightptrb)
	movl	(%edx),%ebx
	subl	%eax,%ebx
	movl	%ebx,%eax
	shrl	$4,%eax
	movl	%eax,C(lightbleftstep)
	movl	4(%edx),%eax
	subl	%ecx,%eax
	shrl	$4,%eax
	movl	%eax,C(lightbrightstep)
	xorl	%ebx,%ebx
	movl	%esi,28(%esp)
L1004:
	movl	C(lightrright),%ecx
	movl	C(lightrleft),%esi
	movl	C(lightgright),%edx
	movl	C(lightgleft),%edi
	movl	C(lightbright),%eax
	subl	%ecx,%esi
	sarl	$4,%esi
	movl	%esi,56(%esp)
	movl	C(lightbleft),%esi
	subl	%edx,%edi
	sarl	$4,%edi
	movl	%edi,48(%esp)
	subl	%eax,%esi
	sarl	$4,%esi
	movl	%esi,40(%esp)
	movl	%ecx,52(%esp)
	movl	%edx,44(%esp)
	movl	%eax,%ebp
	movl	$15,60(%esp)
	incl	%ebx
	movl	%ebx,24(%esp)

L1008:
	movl	52(%esp),%ecx
	movl	36(%esp),%edi
	movl	60(%esp),%eax
	andl	$65280,%ecx
	xorl	%ebx,%ebx
	movb	(%eax,%edi),%bl
	movl	44(%esp),%eax
	addl	%ebx,%ecx
	andl	$65280,%eax
	addl	%ebx,%eax
	movw	0x12345678(,%ecx,2),%cx
LRPatch0:
	orw		0x12345678(,%eax,2),%cx
LGPatch0:
	movl	%ebp,%eax
	andl	$65280,%eax
	addl	%ebx,%eax
	orw		0x12345678(,%eax,2),%cx
LBPatch0:
	andl	$65535,%ecx
	movl	32(%esp),%edx
	movb	0x12345678(%ecx),%al
LTPatch0:
	movl	60(%esp),%ecx
	movb	%al,(%ecx,%edx)
	movl	56(%esp),%ebx
	movl	48(%esp),%esi
	addl	%ebx,52(%esp)
	movl	%ecx,%edi
	movl	52(%esp),%ecx
	movl	36(%esp),%eax
	addl	%esi,44(%esp)
	addl	40(%esp),%ebp
	decl	%edi
	andl	$65280,%ecx
	xorl	%ebx,%ebx
	movb	(%edi,%eax),%bl
	movl	44(%esp),%eax
	addl	%ebx,%ecx
	andl	$65280,%eax
	addl	%ebx,%eax
	movw	0x12345678(,%ecx,2),%cx
LRPatch1:
	orw		0x12345678(,%eax,2),%cx
LGPatch1:
	movl	%ebp,%eax
	andl	$65280,%eax
	addl	%ebx,%eax
	orw		0x12345678(,%eax,2),%cx
LBPatch1:
	andl	$65535,%ecx
	movl	32(%esp),%edx
	movb	0x12345678(%ecx),%al
LTPatch1:
	movb	%al,(%edi,%edx)
	movl	56(%esp),%ecx
	movl	48(%esp),%ebx
	addl	%ecx,52(%esp)
	addl	%ebx,44(%esp)
	addl	40(%esp),%ebp
	addl	$-2,60(%esp)
	jns		L1008

	movl	C(sourcetstep),%esi
	movl	C(lightrrightstep),%eax
	addl	%eax,C(lightrright)
	movl	C(lightrleftstep),%eax
	addl	%eax,C(lightrleft)
	movl	C(lightgrightstep),%eax
	addl	%eax,C(lightgright)
	movl	C(lightgleftstep),%eax
	addl	%eax,C(lightgleft)
	movl	C(lightbrightstep),%eax
	addl	%eax,C(lightbright)
	movl	C(lightbleftstep),%eax
	movl	C(surfrowbytes),%edi
	movl	24(%esp),%ebx
	addl	%esi,36(%esp)
	addl	%eax,C(lightbleft)
	addl	%edi,%edx
	movl	%edx,32(%esp)
	cmpl	$15,%ebx
	jle		L1004
	movl	36(%esp),%eax
	cmpl	C(r_sourcemax),%eax
	jb		L999
	movl	C(r_stepback),%edx
	subl	%edx,%eax
	movl	%eax,36(%esp)
L999:
	movl	28(%esp),%esi
	cmpl	C(r_numvblocks),%esi
	jl		L1000
L1013:
	popl	%ebx
	popl	%esi
	popl	%edi
	popl	%ebp
	addl	$60,%esp
	ret

//----------------------------------------------------------------------
// Surface block drawer for mip level 1
//----------------------------------------------------------------------

	Align16
.globl C(D_DrawSurfaceBlock8RGB_mip1)
C(D_DrawSurfaceBlock8RGB_mip1):
	movl	C(pbasesource),%eax
	subl	$60,%esp
	movl	C(prowdestbase),%edx
	pushl	%ebp
	pushl	%edi
	pushl	%esi
	pushl	%ebx
	xorl	%esi,%esi
	movl	%eax,36(%esp)
	movl	%edx,32(%esp)
	cmpl	C(r_numvblocks),%esi
	jge		L1035
L1022:
	movl	C(r_lightptrr),%edx
	movl	C(r_lightwidth),%ebx
	sall	$2,%ebx
	incl	%esi
	movl	(%edx),%eax
	movl	4(%edx),%ecx
	movl	%eax,C(lightrleft)
	movl	%ecx,C(lightrright)
	addl	%ebx,%edx
	movl	%edx,C(r_lightptrr)
	movl	(%edx),%edi
	subl	%eax,%edi
	movl	%edi,%eax
	shrl	$3,%eax
	movl	%eax,C(lightrleftstep)
	movl	4(%edx),%eax
	movl	C(r_lightptrg),%edx
	subl	%ecx,%eax
	shrl	$3,%eax
	movl	%eax,C(lightrrightstep)
	movl	(%edx),%eax
	movl	4(%edx),%ecx
	movl	%eax,C(lightgleft)
	movl	%ecx,C(lightgright)
	addl	%ebx,%edx
	movl	%edx,C(r_lightptrg)
	movl	(%edx),%edi
	subl	%eax,%edi
	movl	%edi,%eax
	shrl	$3,%eax
	movl	%eax,C(lightgleftstep)
	movl	4(%edx),%eax
	movl	C(r_lightptrb),%edx
	subl	%ecx,%eax
	shrl	$3,%eax
	movl	%eax,C(lightgrightstep)
	movl	(%edx),%eax
	movl	4(%edx),%ecx
	movl	%eax,C(lightbleft)
	movl	%ecx,C(lightbright)
	addl	%ebx,%edx
	movl	%edx,C(r_lightptrb)
	movl	(%edx),%ebx
	subl	%eax,%ebx
	movl	%ebx,%eax
	shrl	$3,%eax
	movl	%eax,C(lightbleftstep)
	movl	4(%edx),%eax
	subl	%ecx,%eax
	shrl	$3,%eax
	movl	%eax,C(lightbrightstep)
	xorl	%ebx,%ebx
	movl	%esi,28(%esp)
L1026:
	movl	C(lightrright),%ecx
	movl	C(lightrleft),%esi
	movl	C(lightgright),%edx
	movl	C(lightgleft),%edi
	movl	C(lightbright),%eax
	subl	%ecx,%esi
	sarl	$3,%esi
	movl	%esi,56(%esp)
	movl	C(lightbleft),%esi
	subl	%edx,%edi
	sarl	$3,%edi
	movl	%edi,48(%esp)
	subl	%eax,%esi
	sarl	$3,%esi
	movl	%esi,40(%esp)
	movl	%ecx,52(%esp)
	movl	%edx,44(%esp)
	movl	%eax,%ebp
	movl	$7,60(%esp)
	incl	%ebx
	movl	%ebx,24(%esp)
L1030:
	movl 52(%esp),%ecx
	movl 36(%esp),%edi
	movl 60(%esp),%eax
	andl	$65280,%ecx
	xorl	%ebx,%ebx
	movb	(%eax,%edi),%bl
	movl	44(%esp),%eax
	addl	%ebx,%ecx
	andl	$65280,%eax
	addl	%ebx,%eax
	movw	0x12345678(,%ecx,2),%cx
LRPatch2:
	orw		0x12345678(,%eax,2),%cx
LGPatch2:
	movl	%ebp,%eax
	andl	$65280,%eax
	addl	%ebx,%eax
	orw		0x12345678(,%eax,2),%cx
LBPatch2:
	andl	$65535,%ecx
	movl 32(%esp),%edx
	movb	0x12345678(%ecx),%al
LTPatch2:
	movl 60(%esp),%ecx
	movb	%al,(%ecx,%edx)
	movl 56(%esp),%ebx
	movl	48(%esp),%esi
	addl	%ebx,52(%esp)
	movl	%ecx,%edi
	movl 52(%esp),%ecx
	movl 36(%esp),%eax
	addl	%esi,44(%esp)
	addl	40(%esp),%ebp
	decl	%edi
	andl	$65280,%ecx
	xorl	%ebx,%ebx
	movb	(%edi,%eax),%bl
	movl	44(%esp),%eax
	addl	%ebx,%ecx
	andl	$65280,%eax
	addl	%ebx,%eax
	movw	0x12345678(,%ecx,2),%cx
LRPatch3:
	orw		0x12345678(,%eax,2),%cx
LGPatch3:
	movl	%ebp,%eax
	andl	$65280,%eax
	addl	%ebx,%eax
	orw		(,%eax,2),%cx
LBPatch3:
	andl	$65535,%ecx
	movl 32(%esp),%edx
	movb	0x12345678(%ecx),%al
LTPatch3:
	movb	%al,(%edi,%edx)
	movl 56(%esp),%ecx
	movl	48(%esp),%ebx
	addl	%ecx,52(%esp)
	addl	%ebx,44(%esp)
	addl	40(%esp),%ebp
	addl	$-2,60(%esp)
	jns L1030
	movl	C(sourcetstep),%esi
	movl	C(lightrrightstep),%eax
	addl	%eax,C(lightrright)
	movl	C(lightrleftstep),%eax
	addl	%eax,C(lightrleft)
	movl	C(lightgrightstep),%eax
	addl	%eax,C(lightgright)
	movl	C(lightgleftstep),%eax
	addl	%eax,C(lightgleft)
	movl	C(lightbrightstep),%eax
	addl	%eax,C(lightbright)
	movl	C(lightbleftstep),%eax
	movl	C(surfrowbytes),%edi
	movl 24(%esp),%ebx
	addl	%esi,36(%esp)
	addl	%eax,C(lightbleft)
	addl	%edi,%edx
	movl	%edx,32(%esp)
	cmpl	$7,%ebx
	jle L1026
	movl 36(%esp),%eax
	cmpl	C(r_sourcemax),%eax
	jb L1021
	movl	C(r_stepback),%edx
	subl	%edx,%eax
	movl	%eax,36(%esp)
L1021:
	movl 28(%esp),%esi
	cmpl	C(r_numvblocks),%esi
	jl L1022
L1035:
	popl	%ebx
	popl	%esi
	popl	%edi
	popl	%ebp
	addl	$60,%esp
	ret

//----------------------------------------------------------------------
// Surface block drawer for mip level 2
//----------------------------------------------------------------------

	Align16
.globl C(D_DrawSurfaceBlock8RGB_mip2)
C(D_DrawSurfaceBlock8RGB_mip2):
	movl	C(pbasesource),%eax
	subl	$60,%esp
	movl	C(prowdestbase),%edx
	pushl	%ebp
	pushl	%edi
	pushl	%esi
	pushl	%ebx
	xorl	%esi,%esi
	movl	%eax,36(%esp)
	movl	%edx,32(%esp)
	cmpl	C(r_numvblocks),%esi
	jge L1057
L1044:
	movl	C(r_lightptrr),%edx
	movl	C(r_lightwidth),%ebx
	sall	$2,%ebx
	incl	%esi
	movl	(%edx),%eax
	movl	4(%edx),%ecx
	movl	%eax,C(lightrleft)
	movl	%ecx,C(lightrright)
	addl	%ebx,%edx
	movl	%edx,C(r_lightptrr)
	movl	(%edx),%edi
	subl	%eax,%edi
	movl	%edi,%eax
	shrl	$2,%eax
	movl	%eax,C(lightrleftstep)
	movl	4(%edx),%eax
	movl	C(r_lightptrg),%edx
	subl	%ecx,%eax
	shrl	$2,%eax
	movl	%eax,C(lightrrightstep)
	movl	(%edx),%eax
	movl	4(%edx),%ecx
	movl	%eax,C(lightgleft)
	movl	%ecx,C(lightgright)
	addl	%ebx,%edx
	movl	%edx,C(r_lightptrg)
	movl	(%edx),%edi
	subl	%eax,%edi
	movl	%edi,%eax
	shrl	$2,%eax
	movl	%eax,C(lightgleftstep)
	movl	4(%edx),%eax
	movl	C(r_lightptrb),%edx
	subl	%ecx,%eax
	shrl	$2,%eax
	movl	%eax,C(lightgrightstep)
	movl	(%edx),%eax
	movl	4(%edx),%ecx
	movl	%eax,C(lightbleft)
	movl	%ecx,C(lightbright)
	addl	%ebx,%edx
	movl	%edx,C(r_lightptrb)
	movl	(%edx),%ebx
	subl	%eax,%ebx
	movl	%ebx,%eax
	shrl	$2,%eax
	movl	%eax,C(lightbleftstep)
	movl	4(%edx),%eax
	subl	%ecx,%eax
	shrl	$2,%eax
	movl	%eax,C(lightbrightstep)
	xorl	%ebx,%ebx
	movl	%esi,28(%esp)
L1048:
	movl	C(lightrright),%ecx
	movl	C(lightrleft),%esi
	movl	C(lightgright),%edx
	movl	C(lightgleft),%edi
	movl	C(lightbright),%eax
	subl	%ecx,%esi
	sarl	$2,%esi
	movl	%esi,56(%esp)
	movl	C(lightbleft),%esi
	subl	%edx,%edi
	sarl	$2,%edi
	movl	%edi,48(%esp)
	subl	%eax,%esi
	sarl	$2,%esi
	movl	%esi,40(%esp)
	movl	%ecx,52(%esp)
	movl	%edx,44(%esp)
	movl	%eax,%ebp
	movl	$3,60(%esp)
	incl	%ebx
	movl	%ebx,24(%esp)
L1052:
	movl 52(%esp),%ecx
	movl 36(%esp),%edi
	movl 60(%esp),%eax
	andl	$65280,%ecx
	xorl	%ebx,%ebx
	movb	(%eax,%edi),%bl
	movl	44(%esp),%eax
	addl	%ebx,%ecx
	andl	$65280,%eax
	addl	%ebx,%eax
	movw	0x12345678(,%ecx,2),%cx
LRPatch4:
	orw		0x12345678(,%eax,2),%cx
LGPatch4:
	movl	%ebp,%eax
	andl	$65280,%eax
	addl	%ebx,%eax
	orw		(,%eax,2),%cx
LBPatch4:
	andl	$65535,%ecx
	movl 32(%esp),%edx
	movb	0x12345678(%ecx),%al
LTPatch4:
	movl 60(%esp),%ecx
	movb	%al,(%ecx,%edx)
	movl 56(%esp),%ebx
	movl	48(%esp),%esi
	addl	%ebx,52(%esp)
	movl	%ecx,%edi
	movl 52(%esp),%ecx
	movl 36(%esp),%eax
	addl	%esi,44(%esp)
	addl	40(%esp),%ebp
	decl	%edi
	andl	$65280,%ecx
	xorl	%ebx,%ebx
	movb	(%edi,%eax),%bl
	movl	44(%esp),%eax
	addl	%ebx,%ecx
	andl	$65280,%eax
	addl	%ebx,%eax
	movw	0x12345678(,%ecx,2),%cx
LRPatch5:
	orw		0x12345678(,%eax,2),%cx
LGPatch5:
	movl	%ebp,%eax
	andl	$65280,%eax
	addl	%ebx,%eax
	orw		0x12345678(,%eax,2),%cx
LBPatch5:
	andl	$65535,%ecx
	movl 32(%esp),%edx
	movb	0x12345678(%ecx),%al
LTPatch5:
	movb	%al,(%edi,%edx)
	movl 56(%esp),%ecx
	movl	48(%esp),%ebx
	addl	%ecx,52(%esp)
	addl	%ebx,44(%esp)
	addl	40(%esp),%ebp
	addl	$-2,60(%esp)
	jns L1052
	movl	C(sourcetstep),%esi
	movl	C(lightrrightstep),%eax
	addl	%eax,C(lightrright)
	movl	C(lightrleftstep),%eax
	addl	%eax,C(lightrleft)
	movl	C(lightgrightstep),%eax
	addl	%eax,C(lightgright)
	movl	C(lightgleftstep),%eax
	addl	%eax,C(lightgleft)
	movl	C(lightbrightstep),%eax
	addl	%eax,C(lightbright)
	movl	C(lightbleftstep),%eax
	movl	C(surfrowbytes),%edi
	movl 24(%esp),%ebx
	addl	%esi,36(%esp)
	addl	%eax,C(lightbleft)
	addl	%edi,%edx
	movl	%edx,32(%esp)
	cmpl	$3,%ebx
	jle L1048
	movl 36(%esp),%eax
	cmpl	C(r_sourcemax),%eax
	jb L1043
	movl	C(r_stepback),%edx
	subl	%edx,%eax
	movl	%eax,36(%esp)
L1043:
	movl 28(%esp),%esi
	cmpl	C(r_numvblocks),%esi
	jl L1044
L1057:
	popl	%ebx
	popl	%esi
	popl	%edi
	popl	%ebp
	addl	$60,%esp
	ret

//----------------------------------------------------------------------
// Surface block drawer for mip level 3
//----------------------------------------------------------------------

	Align16
.globl C(D_DrawSurfaceBlock8RGB_mip3)
C(D_DrawSurfaceBlock8RGB_mip3):
	movl	C(pbasesource),%eax
	subl	$44,%esp
	movl	C(prowdestbase),%edx
	pushl	%ebp
	pushl	%edi
	pushl	%esi
	pushl	%ebx
	xorl	%esi,%esi
	movl	%eax,28(%esp)
	movl	%edx,24(%esp)
	cmpl	C(r_numvblocks),%esi
	jge L1079
L1066:
	movl	C(r_lightptrr),%edx
	movl	C(r_lightwidth),%ebx
	sall	$2,%ebx
	incl	%esi
	movl	(%edx),%eax
	movl	4(%edx),%ecx
	movl	%eax,C(lightrleft)
	movl	%ecx,C(lightrright)
	addl	%ebx,%edx
	movl	%edx,C(r_lightptrr)
	movl	(%edx),%edi
	subl	%eax,%edi
	movl	%edi,%eax
	shrl	$1,%eax
	movl	%eax,C(lightrleftstep)
	movl	4(%edx),%eax
	movl	C(r_lightptrg),%edx
	subl	%ecx,%eax
	shrl	$1,%eax
	movl	%eax,C(lightrrightstep)
	movl	(%edx),%eax
	movl	4(%edx),%ecx
	movl	%eax,C(lightgleft)
	movl	%ecx,C(lightgright)
	addl	%ebx,%edx
	movl	%edx,C(r_lightptrg)
	movl	(%edx),%edi
	subl	%eax,%edi
	movl	%edi,%eax
	shrl	$1,%eax
	movl	%eax,C(lightgleftstep)
	movl	4(%edx),%eax
	movl	C(r_lightptrb),%edx
	subl	%ecx,%eax
	shrl	$1,%eax
	movl	%eax,C(lightgrightstep)
	movl	(%edx),%eax
	movl	4(%edx),%ecx
	movl	%eax,C(lightbleft)
	movl	%ecx,C(lightbright)
	addl	%ebx,%edx
	movl	%edx,C(r_lightptrb)
	movl	(%edx),%ebx
	subl	%eax,%ebx
	movl	%ebx,%eax
	shrl	$1,%eax
	movl	%eax,C(lightbleftstep)
	movl	4(%edx),%eax
	subl	%ecx,%eax
	shrl	$1,%eax
	movl	%eax,C(lightbrightstep)
	movl	$0,44(%esp)
	movl	%esi,16(%esp)
L1070:
	movl	C(lightrright),%edi
	movl	C(lightrleft),%eax
	movl	C(lightgright),%esi
	movl	C(lightgleft),%edx
	movl	%edi,20(%esp)
	subl	%edi,%eax
	movl	C(lightbright),%edi
	movl	C(lightbleft),%ebx
	sarl	$1,%eax
	movl	%eax,40(%esp)
	subl	%esi,%edx
	sarl	$1,%edx
	movl	%edx,36(%esp)
	subl	%edi,%ebx
	sarl	$1,%ebx
	movl	%ebx,32(%esp)
	movl 20(%esp),%ecx
	movl 28(%esp),%eax
	andl	$65280,%ecx
	xorl	%ebx,%ebx
	movb 1(%eax),%bl
	addl	%ebx,%ecx
	movl	%esi,%eax
	andl	$65280,%eax
	addl	%ebx,%eax
	movw	0x12345678(,%ecx,2),%cx
LRPatch6:
	orw		0x12345678(,%eax,2),%cx
LGPatch6:
	movl	%edi,%eax
	andl	$65280,%eax
	addl	%ebx,%eax
	orw		0x12345678(,%eax,2),%cx
LBPatch6:
	andl	$65535,%ecx
	movl	24(%esp),%edx
	movb	0x12345678(%ecx),%al
LTPatch6:
	movb	%al,1(%edx)
	movl	20(%esp),%eax
	movl	28(%esp),%ebx
	addl	40(%esp),%eax
	addl	36(%esp),%esi
	addl	32(%esp),%edi
	andl	$65280,%eax
	xorl	%ecx,%ecx
	andl	$65280,%esi
	andl	$65280,%edi
	movb	(%ebx),%cl
	leal	(%ecx,%eax),%ebx
	addl	%ecx,%esi
	addl	%ecx,%edi
	movw	0x12345678(,%ebx,2),%dx
LRPatch7:
	orw		0x12345678(,%esi,2),%dx
LGPatch7:
	orw		0x12345678(,%edi,2),%dx
LBPatch7:
	andl	$65535,%edx
	movl	24(%esp),%edi
	movb	0x12345678(%edx),%al
LTPatch7:
	movb	%al,(%edi)
	movl	C(sourcetstep),%eax
	addl	%eax,28(%esp)
	movl	C(lightrrightstep),%eax
	addl	%eax,C(lightrright)
	movl	C(lightrleftstep),%eax
	addl	%eax,C(lightrleft)
	movl	C(lightgrightstep),%eax
	addl	%eax,C(lightgright)
	movl	C(lightgleftstep),%eax
	addl	%eax,C(lightgleft)
	movl	C(lightbrightstep),%eax
	addl	%eax,C(lightbright)
	movl	C(lightbleftstep),%eax
	movl	C(surfrowbytes),%edx
	addl	%eax,C(lightbleft)
	addl	%edx,%edi
	movl	%edi,24(%esp)
	incl	44(%esp)
	cmpl	$1,44(%esp)
	jle L1070
	movl 28(%esp),%ebx
	cmpl	C(r_sourcemax),%ebx
	jb L1065
	movl	C(r_stepback),%edi
	subl	%edi,%ebx
	movl	%ebx,28(%esp)
L1065:
	movl 16(%esp),%esi
	cmpl	C(r_numvblocks),%esi
	jl L1066
L1079:
	popl	%ebx
	popl	%esi
	popl	%edi
	popl	%ebp
	addl	$44,%esp
	ret


.globl C(D_Surf8End)
C(D_Surf8End):

//----------------------------------------------------------------------
// Code patching routines
//----------------------------------------------------------------------
	.data

	Align4
LPatchTable:
	.long	LPatch0-4
	.long	LPatch1-4
	.long	LPatch2-4
	.long	LPatch3-4
	.long	LPatch4-4
	.long	LPatch5-4
	.long	LPatch6-4
	.long	LPatch7-4
	.long	LPatch8-4
	.long	LPatch9-4
	.long	LPatch10-4
	.long	LPatch11-4
	.long	LPatch12-4
	.long	LPatch13-4
	.long	LPatch14-4
	.long	LPatch15-4
	.long	LPatch16-4
	.long	LPatch17-4
	.long	LPatch18-4
	.long	LPatch19-4
	.long	LPatch20-4
	.long	LPatch21-4
	.long	LPatch22-4
	.long	LPatch23-4
	.long	LPatch24-4
	.long	LPatch25-4
	.long	LPatch26-4
	.long	LPatch27-4
	.long	LPatch28-4
	.long	LPatch29-4
	.long	LPatch30-4
	.long	LPatch31-4

	Align4
LTPatchTable:
	.long	LTPatch0-4
	.long	LTPatch1-4
	.long	LTPatch2-4
	.long	LTPatch3-4
	.long	LTPatch4-4
	.long	LTPatch5-4
	.long	LTPatch6-4
	.long	LTPatch7-4

	Align4
LRPatchTable:
	.long	LRPatch0-4
	.long	LRPatch1-4
	.long	LRPatch2-4
	.long	LRPatch3-4
	.long	LRPatch4-4
	.long	LRPatch5-4
	.long	LRPatch6-4
	.long	LRPatch7-4

	Align4
LGPatchTable:
	.long	LGPatch0-4
	.long	LGPatch1-4
	.long	LGPatch2-4
	.long	LGPatch3-4
	.long	LGPatch4-4
	.long	LGPatch5-4
	.long	LGPatch6-4
	.long	LGPatch7-4

	Align4
LBPatchTable:
	.long	LBPatch0-4
	.long	LBPatch1-4
	.long	LBPatch2-4
	.long	LBPatch3-4
	.long	LBPatch4-4
	.long	LBPatch5-4
	.long	LBPatch6-4
	.long	LBPatch7-4

	.text

	Align4
.globl C(D_Surf8Patch)
C(D_Surf8Patch):
	pushl	%ebx

	movl	C(fadetable),%eax
	movl	$LPatchTable,%ebx
	movl	$32,%ecx
LPatchLoop:
	movl	(%ebx),%edx
	addl	$4,%ebx
	movl	%eax,(%edx)
	decl	%ecx
	jnz		LPatchLoop

	movl	C(d_rgbtable),%eax
	movl	$LTPatchTable,%ebx
	movl	$8,%ecx
LTPatchLoop:
	movl	(%ebx),%edx
	addl	$4,%ebx
	movl	%eax,(%edx)
	decl	%ecx
	jnz		LTPatchLoop

	movl	C(fadetable16r),%eax
	movl	$LRPatchTable,%ebx
	movl	$8,%ecx
LRPatchLoop:
	movl	(%ebx),%edx
	addl	$4,%ebx
	movl	%eax,(%edx)
	decl	%ecx
	jnz		LRPatchLoop

	movl	C(fadetable16g),%eax
	movl	$LGPatchTable,%ebx
	movl	$8,%ecx
LGPatchLoop:
	movl	(%ebx),%edx
	addl	$4,%ebx
	movl	%eax,(%edx)
	decl	%ecx
	jnz		LGPatchLoop

	movl	C(fadetable16b),%eax
	movl	$LBPatchTable,%ebx
	movl	$8,%ecx
LBPatchLoop:
	movl	(%ebx),%edx
	addl	$4,%ebx
	movl	%eax,(%edx)
	decl	%ecx
	jnz		LBPatchLoop

	popl	%ebx

	ret

#endif
