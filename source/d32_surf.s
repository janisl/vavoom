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
//**
//**	x86 assembly-language 32 bpp surface block drawing code.
//**
//**************************************************************************

#include "asm_i386.h"

#ifdef USEASM

	.data

sb_v:		.long	0

	.text

	Align4
.globl C(D_Surf32Start)
C(D_Surf32Start):

//----------------------------------------------------------------------
// Surface block drawer for mip level 0
//----------------------------------------------------------------------

	Align4
.globl C(D_DrawSurfaceBlock32_mip0)
C(D_DrawSurfaceBlock32_mip0):
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

Lblockloop16_mip0:
	movl	%ebp,C(lightdelta)
	movb	14(%esi),%cl

	sarl	$4,%ebp
	movb	%dh,%bh

	movb	15(%esi),%bl
	addl	%ebp,%edx

	movb	%dh,%ch
	addl	%ebp,%edx

	movl	0x12345678(,%ebx,4),%eax
LPatch0:
	movb	13(%esi),%bl

	movl	%eax,60(%edi)

	movl	0x12345678(,%ecx,4),%eax
LPatch1:
	movb	12(%esi),%cl

	movb	%dh,%bh
	addl	%ebp,%edx

	movl	%eax,56(%edi)
	movb	%dh,%ch

	movl	0x12345678(,%ebx,4),%eax
LPatch2:
	addl	%ebp,%edx

	movl	%eax,52(%edi)

	movl	0x12345678(,%ecx,4),%eax
LPatch3:
	movb	11(%esi),%bl

	movb	10(%esi),%cl
	movl	%eax,48(%edi)

	movb	%dh,%bh
	addl	%ebp,%edx

	movb	%dh,%ch
	addl	%ebp,%edx

	movl	0x12345678(,%ebx,4),%eax
LPatch4:
	movb	9(%esi),%bl

	movl	%eax,44(%edi)

	movl	0x12345678(,%ecx,4),%eax
LPatch5:
	movb	8(%esi),%cl

	movb	%dh,%bh
	addl	%ebp,%edx

	movl	%eax,40(%edi)
	movb	%dh,%ch

	movl	0x12345678(,%ebx,4),%eax
LPatch6:
	addl	%ebp,%edx

	movl	%eax,36(%edi)

	movl	0x12345678(,%ecx,4),%eax
LPatch7:
	movb	7(%esi),%bl

	movb	6(%esi),%cl
	movl	%eax,32(%edi)

	movb	%dh,%bh
	addl	%ebp,%edx

	movb	%dh,%ch
	addl	%ebp,%edx

	movl	0x12345678(,%ebx,4),%eax
LPatch8:
	movb	5(%esi),%bl

	movl	%eax,28(%edi)

	movl	0x12345678(,%ecx,4),%eax
LPatch9:
	movb	4(%esi),%cl

	movb	%dh,%bh
	addl	%ebp,%edx

	movl	%eax,24(%edi)
	movb	%dh,%ch

	movl	0x12345678(,%ebx,4),%eax
LPatch10:
	addl	%ebp,%edx

	movl	%eax,20(%edi)

	movl	0x12345678(,%ecx,4),%eax
LPatch11:
	movb	3(%esi),%bl

	movb	2(%esi),%cl
	movl	%eax,16(%edi)

	movb	%dh,%bh
	addl	%ebp,%edx

	movb	%dh,%ch
	addl	%ebp,%edx

	movl	0x12345678(,%ebx,4),%eax
LPatch12:
	movb	1(%esi),%bl

	movl	%eax,12(%edi)

	movl	0x12345678(,%ecx,4),%eax
LPatch13:
	movb	(%esi),%cl

	movb	%dh,%bh
	addl	%ebp,%edx

	movl	%eax,8(%edi)
	movb	%dh,%ch

	movl	0x12345678(,%ebx,4),%eax
LPatch14:
	movl	C(lightright),%edx

	movl	%eax,4(%edi)

	movl	0x12345678(,%ecx,4),%eax
LPatch15:
	movl	C(lightdelta),%ebp

	movl	%eax,(%edi)

	addl	C(sourcetstep),%esi
	addl	C(surfrowbytes),%edi

	addl	C(lightrightstep),%edx
	addl	C(lightdeltastep),%ebp

	movl	%edx,C(lightright)
	jc		Lblockloop16_mip0

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
.globl C(D_DrawSurfaceBlock32_mip1)
C(D_DrawSurfaceBlock32_mip1):
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

Lblockloop16_mip1:
	movl	%ebp,C(lightdelta)
	movb	6(%esi),%cl

	sarl	$3,%ebp
	movb	%dh,%bh

	movb	7(%esi),%bl
	addl	%ebp,%edx

	movb	%dh,%ch
	addl	%ebp,%edx

	movl	0x12345678(,%ebx,4),%eax
LPatch22:
	movb	5(%esi),%bl

	movl	%eax,28(%edi)

	movl	0x12345678(,%ecx,4),%eax
LPatch23:
	movb	4(%esi),%cl

	movb	%dh,%bh
	addl	%ebp,%edx

	movl	%eax,24(%edi)
	movb	%dh,%ch

	movl	0x12345678(,%ebx,4),%eax
LPatch24:
	addl	%ebp,%edx

	movl	%eax,20(%edi)

	movl	0x12345678(,%ecx,4),%eax
LPatch25:
	movb	3(%esi),%bl

	movb	2(%esi),%cl
	movl	%eax,16(%edi)

	movb	%dh,%bh
	addl	%ebp,%edx

	movb	%dh,%ch
	addl	%ebp,%edx

	movl	0x12345678(,%ebx,4),%eax
LPatch26:
	movb	1(%esi),%bl

	movl	%eax,12(%edi)

	movl	0x12345678(,%ecx,4),%eax
LPatch27:
	movb	(%esi),%cl

	movb	%dh,%bh
	addl	%ebp,%edx

	movl	%eax,8(%edi)
	movb	%dh,%ch

	movl	0x12345678(,%ebx,4),%eax
LPatch28:
	movl	C(lightright),%edx

	movl	%eax,4(%edi)

	movl	0x12345678(,%ecx,4),%eax
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

	jc		Lblockloop16_mip1

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
.globl C(D_DrawSurfaceBlock32_mip2)
C(D_DrawSurfaceBlock32_mip2):
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

Lblockloop16_mip2:
	movl	%ebp,C(lightdelta)
	movb	2(%esi),%cl

	sarl	$2,%ebp
	movb	%dh,%bh

	movb	3(%esi),%bl
	addl	%ebp,%edx

	movb	%dh,%ch
	addl	%ebp,%edx

	movl	0x12345678(,%ebx,4),%eax
LPatch18:
	movb	1(%esi),%bl

	movl	%eax,12(%edi)

	movl	0x12345678(,%ecx,4),%eax
LPatch19:
	movb	(%esi),%cl

	movb	%dh,%bh
	addl	%ebp,%edx

	movl	%eax,8(%edi)
	movb	%dh,%ch

	movl	0x12345678(,%ebx,4),%eax
LPatch20:
	movl	C(lightright),%edx

	movl	%eax,4(%edi)

	movl	0x12345678(,%ecx,4),%eax
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

	jc		Lblockloop16_mip2

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
.globl C(D_DrawSurfaceBlock32_mip3)
C(D_DrawSurfaceBlock32_mip3):
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

	movl	0x12345678(,%ebx,4),%eax
LPatch16:
	addl	%ebp,%edx

	movl	%eax,4(%edi)
	movb	%dh,%ch

	movl	0x12345678(,%ecx,4),%eax
LPatch17:
	movl	C(lightright),%edx

	movl	%eax,(%edi)
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

	movl	0x12345678(,%ebx,4),%eax
LPatch30:
	addl	%ebp,%edx

	movl	%eax,4(%edi)
	movb	%dh,%ch

	movl	0x12345678(,%ecx,4),%eax
LPatch31:
	movl	C(sourcetstep),%edx

	movl	%eax,(%edi)
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
.globl C(D_DrawSurfaceBlock32RGB_mip0)
C(D_DrawSurfaceBlock32RGB_mip0):
	pushl	%ebp
	pushl	%edi
	pushl	%esi
	pushl	%ebx

	movl	C(pbasesource),%esi
	movl	C(r_numvblocks),%eax

	movl	C(prowdestbase),%edi
	movl	%eax,sb_v

Lv_loop_RGBmip0:
//FIXME pipeline this
	movl	C(r_lightwidth),%ebx
	movl	C(r_lightptrr),%edx
	movl	(%edx),%eax
	movl	4(%edx),%ecx
	leal	(%edx,%ebx,4),%edx
	movl	%eax,C(lightrleft)
	movl	%ecx,C(lightrright)
	movl	%edx,C(r_lightptrr)
	movl	(%edx),%ebp
	subl	%eax,%ebp
	movl	%ebp,%eax
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
	leal	(%edx,%ebx,4),%edx
	movl	%edx,C(r_lightptrg)
	movl	(%edx),%ebp
	subl	%eax,%ebp
	movl	%ebp,%eax
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
	leal	(%edx,%ebx,4),%edx
	movl	%edx,C(r_lightptrb)
	movl	(%edx),%ebp
	subl	%eax,%ebp
	movl	%ebp,%eax
	shrl	$4,%eax
	movl	%eax,C(lightbleftstep)
	movl	4(%edx),%eax
	subl	%ecx,%eax
	shrl	$4,%eax
	movl	%eax,C(lightbrightstep)

	movl	$16,counttemp
	xorl	%ebx,%ebx
	xorl	%ecx,%ecx

Lblockloop_RGBmip0:

//
// Red component
//
	movl	C(lightrright),%edx
	movl	C(lightrleft),%ebp

	subl	%edx,%ebp
	movb	15(%esi),%bl

	sarl	$4,%ebp
	movb	14(%esi),%cl

	movb	%dh,%bh
	addl	%ebp,%edx

	movb	%dh,%ch
	addl	C(roffs),%edi

	movb	0x12345678(%ebx),%al
LRPatch0:
	movb	13(%esi),%bl

	movb	%al,60(%edi)
	addl	%ebp,%edx

	movb	0x12345678(%ecx),%al
LRPatch1:
	movb	12(%esi),%cl

	movb	%dh,%bh
	addl	%ebp,%edx

	movb	%al,56(%edi)
	movb	%dh,%ch

	movb	0x12345678(%ebx),%al
LRPatch2:
	movb	11(%esi),%bl

	movb	%al,52(%edi)
	addl	%ebp,%edx

	movb	0x12345678(%ecx),%al
LRPatch3:
	movb	10(%esi),%cl

	movb	%dh,%bh
	addl	%ebp,%edx

	movb	%al,48(%edi)
	movb	%dh,%ch

	movb	0x12345678(%ebx),%al
LRPatch4:
	movb	9(%esi),%bl

	movb	%al,44(%edi)
	addl	%ebp,%edx

	movb	0x12345678(%ecx),%al
LRPatch5:
	movb	8(%esi),%cl

	movb	%dh,%bh
	addl	%ebp,%edx

	movb	%al,40(%edi)
	movb	%dh,%ch

	movb	0x12345678(%ebx),%al
LRPatch6:
	movb	7(%esi),%bl

	movb	%al,36(%edi)
	addl	%ebp,%edx

	movb	0x12345678(%ecx),%al
LRPatch7:
	movb	6(%esi),%cl

	movb	%dh,%bh
	addl	%ebp,%edx

	movb	%al,32(%edi)
	movb	%dh,%ch

	movb	0x12345678(%ebx),%al
LRPatch8:
	movb	5(%esi),%bl

	movb	%al,28(%edi)
	addl	%ebp,%edx

	movb	0x12345678(%ecx),%al
LRPatch9:
	movb	4(%esi),%cl

	movb	%dh,%bh
	addl	%ebp,%edx

	movb	%al,24(%edi)
	movb	%dh,%ch

	movb	0x12345678(%ebx),%al
LRPatch10:
	movb	3(%esi),%bl

	movb	%al,20(%edi)
	addl	%ebp,%edx

	movb	0x12345678(%ecx),%al
LRPatch11:
	movb	2(%esi),%cl

	movb	%dh,%bh
	addl	%ebp,%edx

	movb	%al,16(%edi)
	movb	%dh,%ch

	movb	0x12345678(%ebx),%al
LRPatch12:
	movb	1(%esi),%bl

	movb	%al,12(%edi)
	addl	%ebp,%edx

	movb	0x12345678(%ecx),%al
LRPatch13:
	movb	(%esi),%cl

	movb	%dh,%bh
	addl	%ebp,%edx

	movb	%al,8(%edi)
	movb	%dh,%ch

	movb	0x12345678(%ebx),%al
LRPatch14:
	movl	C(lightgright),%edx

	movb	%al,4(%edi)
	movl	C(lightgleft),%ebp

	movb	0x12345678(%ecx),%al
LRPatch15:
	subl	%edx,%ebp

	movb	%al,(%edi)
	movb	15(%esi),%bl

	subl	C(roffs),%edi
	addl	C(goffs),%edi

//
// Green component
//

	sarl	$4,%ebp
	movb	14(%esi),%cl

	movb	%dh,%bh
	addl	%ebp,%edx

	movb	%dh,%ch

	movb	0x12345678(%ebx),%al
LGPatch0:
	movb	13(%esi),%bl

	movb	%al,60(%edi)
	addl	%ebp,%edx

	movb	0x12345678(%ecx),%al
LGPatch1:
	movb	12(%esi),%cl

	movb	%dh,%bh
	addl	%ebp,%edx

	movb	%al,56(%edi)
	movb	%dh,%ch

	movb	0x12345678(%ebx),%al
LGPatch2:
	movb	11(%esi),%bl

	movb	%al,52(%edi)
	addl	%ebp,%edx

	movb	0x12345678(%ecx),%al
LGPatch3:
	movb	10(%esi),%cl

	movb	%dh,%bh
	addl	%ebp,%edx

	movb	%al,48(%edi)
	movb	%dh,%ch

	movb	0x12345678(%ebx),%al
LGPatch4:
	movb	9(%esi),%bl

	movb	%al,44(%edi)
	addl	%ebp,%edx

	movb	0x12345678(%ecx),%al
LGPatch5:
	movb	8(%esi),%cl

	movb	%dh,%bh
	addl	%ebp,%edx

	movb	%al,40(%edi)
	movb	%dh,%ch

	movb	0x12345678(%ebx),%al
LGPatch6:
	movb	7(%esi),%bl

	movb	%al,36(%edi)
	addl	%ebp,%edx

	movb	0x12345678(%ecx),%al
LGPatch7:
	movb	6(%esi),%cl

	movb	%dh,%bh
	addl	%ebp,%edx

	movb	%al,32(%edi)
	movb	%dh,%ch

	movb	0x12345678(%ebx),%al
LGPatch8:
	movb	5(%esi),%bl

	movb	%al,28(%edi)
	addl	%ebp,%edx

	movb	0x12345678(%ecx),%al
LGPatch9:
	movb	4(%esi),%cl

	movb	%dh,%bh
	addl	%ebp,%edx

	movb	%al,24(%edi)
	movb	%dh,%ch

	movb	0x12345678(%ebx),%al
LGPatch10:
	movb	3(%esi),%bl

	movb	%al,20(%edi)
	addl	%ebp,%edx

	movb	0x12345678(%ecx),%al
LGPatch11:
	movb	2(%esi),%cl

	movb	%dh,%bh
	addl	%ebp,%edx

	movb	%al,16(%edi)
	movb	%dh,%ch

	movb	0x12345678(%ebx),%al
LGPatch12:
	movb	1(%esi),%bl

	movb	%al,12(%edi)
	addl	%ebp,%edx

	movb	0x12345678(%ecx),%al
LGPatch13:
	movb	(%esi),%cl

	movb	%dh,%bh
	addl	%ebp,%edx

	movb	%al,8(%edi)
	movb	%dh,%ch

	movb	0x12345678(%ebx),%al
LGPatch14:
	movl	C(lightbright),%edx

	movb	%al,4(%edi)
	movl	C(lightbleft),%ebp

	movb	0x12345678(%ecx),%al
LGPatch15:
	subl	%edx,%ebp

	movb	%al,(%edi)

	subl	C(goffs),%edi
	addl	C(boffs),%edi

//
// Blue component
//

	sarl	$4,%ebp
	movb	15(%esi),%bl

	movb	%dh,%bh
	addl	%ebp,%edx

	movb	%dh,%ch
	movb	14(%esi),%cl

	movb	0x12345678(%ebx),%al
LBPatch0:
	movb	13(%esi),%bl

	movb	%al,60(%edi)
	addl	%ebp,%edx

	movb	0x12345678(%ecx),%al
LBPatch1:
	movb	12(%esi),%cl

	movb	%dh,%bh
	addl	%ebp,%edx

	movb	%al,56(%edi)
	movb	%dh,%ch

	movb	0x12345678(%ebx),%al
LBPatch2:
	movb	11(%esi),%bl

	movb	%al,52(%edi)
	addl	%ebp,%edx

	movb	0x12345678(%ecx),%al
LBPatch3:
	movb	10(%esi),%cl

	movb	%dh,%bh
	addl	%ebp,%edx

	movb	%al,48(%edi)
	movb	%dh,%ch

	movb	0x12345678(%ebx),%al
LBPatch4:
	movb	9(%esi),%bl

	movb	%al,44(%edi)
	addl	%ebp,%edx

	movb	0x12345678(%ecx),%al
LBPatch5:
	movb	8(%esi),%cl

	movb	%dh,%bh
	addl	%ebp,%edx

	movb	%al,40(%edi)
	movb	%dh,%ch

	movb	0x12345678(%ebx),%al
LBPatch6:
	movb	7(%esi),%bl

	movb	%al,36(%edi)
	addl	%ebp,%edx

	movb	0x12345678(%ecx),%al
LBPatch7:
	movb	6(%esi),%cl

	movb	%dh,%bh
	addl	%ebp,%edx

	movb	%al,32(%edi)
	movb	%dh,%ch

	movb	0x12345678(%ebx),%al
LBPatch8:
	movb	5(%esi),%bl

	movb	%al,28(%edi)
	addl	%ebp,%edx

	movb	0x12345678(%ecx),%al
LBPatch9:
	movb	4(%esi),%cl

	movb	%dh,%bh
	addl	%ebp,%edx

	movb	%al,24(%edi)
	movb	%dh,%ch

	movb	0x12345678(%ebx),%al
LBPatch10:
	movb	3(%esi),%bl

	movb	%al,20(%edi)
	addl	%ebp,%edx

	movb	0x12345678(%ecx),%al
LBPatch11:
	movb	2(%esi),%cl

	movb	%dh,%bh
	addl	%ebp,%edx

	movb	%al,16(%edi)
	movb	%dh,%ch

	movb	0x12345678(%ebx),%al
LBPatch12:
	movb	1(%esi),%bl

	movb	%al,12(%edi)
	addl	%ebp,%edx

	movb	0x12345678(%ecx),%al
LBPatch13:
	movb	(%esi),%cl

	movb	%dh,%bh
	addl	%ebp,%edx

	movb	%al,8(%edi)
	movb	%dh,%ch

	movb	0x12345678(%ebx),%al
LBPatch14:
	movl	C(lightrrightstep),%edx

	movb	%al,4(%edi)
	addl	%edx,C(lightrright)

	movb	0x12345678(%ecx),%al
LBPatch15:
	movl	C(lightrleftstep),%edx

	movb	%al,(%edi)
	addl	C(surfrowbytes),%edi

	subl	C(boffs),%edi

	addl	%edx,C(lightrleft)
	movl	C(lightgrightstep),%eax

	addl	%eax,C(lightgright)
	movl	C(lightgleftstep),%eax

	addl	%eax,C(lightgleft)
	movl	C(lightbrightstep),%eax

	addl	%eax,C(lightbright)
	movl	C(lightbleftstep),%eax

	addl	%eax,C(lightbleft)
	addl	C(sourcetstep),%esi

	decl	counttemp
	jnz		Lblockloop_RGBmip0

	cmpl	C(r_sourcemax),%esi
	jb		LSkip_RGBmip0
	subl	C(r_stepback),%esi
LSkip_RGBmip0:
	decl	sb_v
	jnz		Lv_loop_RGBmip0

	popl	%ebx
	popl	%esi
	popl	%edi
	popl	%ebp
	ret

//----------------------------------------------------------------------
// Surface block drawer for mip level 1
//----------------------------------------------------------------------

	Align16
.globl C(D_DrawSurfaceBlock32RGB_mip1)
C(D_DrawSurfaceBlock32RGB_mip1):
	pushl	%ebp
	pushl	%edi
	pushl	%esi
	pushl	%ebx

	movl	C(pbasesource),%esi
	movl	C(r_numvblocks),%eax

	movl	C(prowdestbase),%edi
	movl	%eax,sb_v

Lv_loop_RGBmip1:
//FIXME pipeline this
	movl	C(r_lightwidth),%ebx
	movl	C(r_lightptrr),%edx
	movl	(%edx),%eax
	movl	4(%edx),%ecx
	leal	(%edx,%ebx,4),%edx
	movl	%eax,C(lightrleft)
	movl	%ecx,C(lightrright)
	movl	%edx,C(r_lightptrr)
	movl	(%edx),%ebp
	subl	%eax,%ebp
	movl	%ebp,%eax
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
	leal	(%edx,%ebx,4),%edx
	movl	%edx,C(r_lightptrg)
	movl	(%edx),%ebp
	subl	%eax,%ebp
	movl	%ebp,%eax
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
	leal	(%edx,%ebx,4),%edx
	movl	%edx,C(r_lightptrb)
	movl	(%edx),%ebp
	subl	%eax,%ebp
	movl	%ebp,%eax
	shrl	$3,%eax
	movl	%eax,C(lightbleftstep)
	movl	4(%edx),%eax
	subl	%ecx,%eax
	shrl	$3,%eax
	movl	%eax,C(lightbrightstep)

	movl	$8,counttemp
	xorl	%ebx,%ebx
	xorl	%ecx,%ecx

Lblockloop_RGBmip1:

//
// Red component
//
	movl	C(lightrright),%edx
	movl	C(lightrleft),%ebp

	subl	%edx,%ebp
	movb	7(%esi),%bl

	sarl	$3,%ebp
	movb	6(%esi),%cl

	movb	%dh,%bh
	addl	%ebp,%edx

	movb	%dh,%ch
	addl	C(roffs),%edi

	movb	0x12345678(%ebx),%al
LRPatch16:
	movb	5(%esi),%bl

	movb	%al,28(%edi)
	addl	%ebp,%edx

	movb	0x12345678(%ecx),%al
LRPatch17:
	movb	4(%esi),%cl

	movb	%dh,%bh
	addl	%ebp,%edx

	movb	%al,24(%edi)
	movb	%dh,%ch

	movb	0x12345678(%ebx),%al
LRPatch18:
	movb	3(%esi),%bl

	movb	%al,20(%edi)
	addl	%ebp,%edx

	movb	0x12345678(%ecx),%al
LRPatch19:
	movb	2(%esi),%cl

	movb	%dh,%bh
	addl	%ebp,%edx

	movb	%al,16(%edi)
	movb	%dh,%ch

	movb	0x12345678(%ebx),%al
LRPatch20:
	movb	1(%esi),%bl

	movb	%al,12(%edi)
	addl	%ebp,%edx

	movb	0x12345678(%ecx),%al
LRPatch21:
	movb	(%esi),%cl

	movb	%dh,%bh
	addl	%ebp,%edx

	movb	%al,8(%edi)
	movb	%dh,%ch

	movb	0x12345678(%ebx),%al
LRPatch22:
	movl	C(lightgright),%edx

	movb	%al,4(%edi)
	movl	C(lightgleft),%ebp

	movb	0x12345678(%ecx),%al
LRPatch23:
	subl	%edx,%ebp

	movb	%al,(%edi)
	movb	7(%esi),%bl

	subl	C(roffs),%edi
	addl	C(goffs),%edi

//
// Green component
//

	sarl	$3,%ebp
	movb	6(%esi),%cl

	movb	%dh,%bh
	addl	%ebp,%edx

	movb	%dh,%ch

	movb	0x12345678(%ebx),%al
LGPatch16:
	movb	5(%esi),%bl

	movb	%al,28(%edi)
	addl	%ebp,%edx

	movb	0x12345678(%ecx),%al
LGPatch17:
	movb	4(%esi),%cl

	movb	%dh,%bh
	addl	%ebp,%edx

	movb	%al,24(%edi)
	movb	%dh,%ch

	movb	0x12345678(%ebx),%al
LGPatch18:
	movb	3(%esi),%bl

	movb	%al,20(%edi)
	addl	%ebp,%edx

	movb	0x12345678(%ecx),%al
LGPatch19:
	movb	2(%esi),%cl

	movb	%dh,%bh
	addl	%ebp,%edx

	movb	%al,16(%edi)
	movb	%dh,%ch

	movb	0x12345678(%ebx),%al
LGPatch20:
	movb	1(%esi),%bl

	movb	%al,12(%edi)
	addl	%ebp,%edx

	movb	0x12345678(%ecx),%al
LGPatch21:
	movb	(%esi),%cl

	movb	%dh,%bh
	addl	%ebp,%edx

	movb	%al,8(%edi)
	movb	%dh,%ch

	movb	0x12345678(%ebx),%al
LGPatch22:
	movl	C(lightbright),%edx

	movb	%al,4(%edi)
	movl	C(lightbleft),%ebp

	movb	0x12345678(%ecx),%al
LGPatch23:
	subl	%edx,%ebp

	movb	%al,(%edi)

	subl	C(goffs),%edi
	addl	C(boffs),%edi

//
// Blue component
//

	sarl	$3,%ebp
	movb	7(%esi),%bl

	movb	%dh,%bh
	addl	%ebp,%edx

	movb	%dh,%ch
	movb	6(%esi),%cl

	movb	0x12345678(%ebx),%al
LBPatch16:
	movb	5(%esi),%bl

	movb	%al,28(%edi)
	addl	%ebp,%edx

	movb	0x12345678(%ecx),%al
LBPatch17:
	movb	4(%esi),%cl

	movb	%dh,%bh
	addl	%ebp,%edx

	movb	%al,24(%edi)
	movb	%dh,%ch

	movb	0x12345678(%ebx),%al
LBPatch18:
	movb	3(%esi),%bl

	movb	%al,20(%edi)
	addl	%ebp,%edx

	movb	0x12345678(%ecx),%al
LBPatch19:
	movb	2(%esi),%cl

	movb	%dh,%bh
	addl	%ebp,%edx

	movb	%al,16(%edi)
	movb	%dh,%ch

	movb	0x12345678(%ebx),%al
LBPatch20:
	movb	1(%esi),%bl

	movb	%al,12(%edi)
	addl	%ebp,%edx

	movb	0x12345678(%ecx),%al
LBPatch21:
	movb	(%esi),%cl

	movb	%dh,%bh
	addl	%ebp,%edx

	movb	%al,8(%edi)
	movb	%dh,%ch

	movb	0x12345678(%ebx),%al
LBPatch22:
	movl	C(lightrrightstep),%edx

	movb	%al,4(%edi)
	addl	%edx,C(lightrright)

	movb	0x12345678(%ecx),%al
LBPatch23:
	movl	C(lightrleftstep),%edx

	movb	%al,(%edi)
	addl	C(surfrowbytes),%edi

	subl	C(boffs),%edi

	addl	%edx,C(lightrleft)
	movl	C(lightgrightstep),%eax

	addl	%eax,C(lightgright)
	movl	C(lightgleftstep),%eax

	addl	%eax,C(lightgleft)
	movl	C(lightbrightstep),%eax

	addl	%eax,C(lightbright)
	movl	C(lightbleftstep),%eax

	addl	%eax,C(lightbleft)
	addl	C(sourcetstep),%esi

	decl	counttemp
	jnz		Lblockloop_RGBmip1

	cmpl	C(r_sourcemax),%esi
	jb		LSkip_RGBmip1
	subl	C(r_stepback),%esi
LSkip_RGBmip1:
	decl	sb_v
	jnz		Lv_loop_RGBmip1

	popl	%ebx
	popl	%esi
	popl	%edi
	popl	%ebp
	ret

//----------------------------------------------------------------------
// Surface block drawer for mip level 2
//----------------------------------------------------------------------

	Align16
.globl C(D_DrawSurfaceBlock32RGB_mip2)
C(D_DrawSurfaceBlock32RGB_mip2):
	pushl	%ebp
	pushl	%edi
	pushl	%esi
	pushl	%ebx

	movl	C(pbasesource),%esi
	movl	C(r_numvblocks),%eax

	movl	C(prowdestbase),%edi
	movl	%eax,sb_v

Lv_loop_RGBmip2:
//FIXME pipeline this
	movl	C(r_lightwidth),%ebx
	movl	C(r_lightptrr),%edx
	movl	(%edx),%eax
	movl	4(%edx),%ecx
	leal	(%edx,%ebx,4),%edx
	movl	%eax,C(lightrleft)
	movl	%ecx,C(lightrright)
	movl	%edx,C(r_lightptrr)
	movl	(%edx),%ebp
	subl	%eax,%ebp
	movl	%ebp,%eax
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
	leal	(%edx,%ebx,4),%edx
	movl	%edx,C(r_lightptrg)
	movl	(%edx),%ebp
	subl	%eax,%ebp
	movl	%ebp,%eax
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
	leal	(%edx,%ebx,4),%edx
	movl	%edx,C(r_lightptrb)
	movl	(%edx),%ebp
	subl	%eax,%ebp
	movl	%ebp,%eax
	shrl	$2,%eax
	movl	%eax,C(lightbleftstep)
	movl	4(%edx),%eax
	subl	%ecx,%eax
	shrl	$2,%eax
	movl	%eax,C(lightbrightstep)

	movl	$4,counttemp
	xorl	%ebx,%ebx
	xorl	%ecx,%ecx

Lblockloop_RGBmip2:

//
// Red component
//
	movl	C(lightrright),%edx
	movl	C(lightrleft),%ebp

	subl	%edx,%ebp
	movb	3(%esi),%bl

	sarl	$2,%ebp
	movb	2(%esi),%cl

	movb	%dh,%bh
	addl	%ebp,%edx

	movb	%dh,%ch
	addl	C(roffs),%edi

	movb	0x12345678(%ebx),%al
LRPatch24:
	movb	1(%esi),%bl

	movb	%al,12(%edi)
	addl	%ebp,%edx

	movb	0x12345678(%ecx),%al
LRPatch25:
	movb	(%esi),%cl

	movb	%dh,%bh
	addl	%ebp,%edx

	movb	%al,8(%edi)
	movb	%dh,%ch

	movb	0x12345678(%ebx),%al
LRPatch26:
	movl	C(lightgright),%edx

	movb	%al,4(%edi)
	movl	C(lightgleft),%ebp

	movb	0x12345678(%ecx),%al
LRPatch27:
	subl	%edx,%ebp

	movb	%al,(%edi)
	movb	3(%esi),%bl

	subl	C(roffs),%edi
	addl	C(goffs),%edi

//
// Green component
//

	sarl	$2,%ebp
	movb	2(%esi),%cl

	movb	%dh,%bh
	addl	%ebp,%edx

	movb	%dh,%ch

	movb	0x12345678(%ebx),%al
LGPatch24:
	movb	1(%esi),%bl

	movb	%al,12(%edi)
	addl	%ebp,%edx

	movb	0x12345678(%ecx),%al
LGPatch25:
	movb	(%esi),%cl

	movb	%dh,%bh
	addl	%ebp,%edx

	movb	%al,8(%edi)
	movb	%dh,%ch

	movb	0x12345678(%ebx),%al
LGPatch26:
	movl	C(lightbright),%edx

	movb	%al,4(%edi)
	movl	C(lightbleft),%ebp

	movb	0x12345678(%ecx),%al
LGPatch27:
	subl	%edx,%ebp

	movb	%al,(%edi)

	subl	C(goffs),%edi
	addl	C(boffs),%edi

//
// Blue component
//

	sarl	$2,%ebp
	movb	3(%esi),%bl

	movb	%dh,%bh
	addl	%ebp,%edx

	movb	%dh,%ch
	movb	2(%esi),%cl

	movb	0x12345678(%ebx),%al
LBPatch24:
	movb	1(%esi),%bl

	movb	%al,12(%edi)
	addl	%ebp,%edx

	movb	0x12345678(%ecx),%al
LBPatch25:
	movb	(%esi),%cl

	movb	%dh,%bh
	addl	%ebp,%edx

	movb	%al,8(%edi)
	movb	%dh,%ch

	movb	0x12345678(%ebx),%al
LBPatch26:
	movl	C(lightrrightstep),%edx

	movb	%al,4(%edi)
	addl	%edx,C(lightrright)

	movb	0x12345678(%ecx),%al
LBPatch27:
	movl	C(lightrleftstep),%edx

	movb	%al,(%edi)
	addl	C(surfrowbytes),%edi

	subl	C(boffs),%edi

	addl	%edx,C(lightrleft)
	movl	C(lightgrightstep),%eax

	addl	%eax,C(lightgright)
	movl	C(lightgleftstep),%eax

	addl	%eax,C(lightgleft)
	movl	C(lightbrightstep),%eax

	addl	%eax,C(lightbright)
	movl	C(lightbleftstep),%eax

	addl	%eax,C(lightbleft)
	addl	C(sourcetstep),%esi

	decl	counttemp
	jnz		Lblockloop_RGBmip2

	cmpl	C(r_sourcemax),%esi
	jb		LSkip_RGBmip2
	subl	C(r_stepback),%esi
LSkip_RGBmip2:
	decl	sb_v
	jnz		Lv_loop_RGBmip2

	popl	%ebx
	popl	%esi
	popl	%edi
	popl	%ebp
	ret

//----------------------------------------------------------------------
// Surface block drawer for mip level 3
//----------------------------------------------------------------------

	Align16
.globl C(D_DrawSurfaceBlock32RGB_mip3)
C(D_DrawSurfaceBlock32RGB_mip3):
	pushl	%ebp
	pushl	%edi
	pushl	%esi
	pushl	%ebx

	movl	C(pbasesource),%esi
	movl	C(r_numvblocks),%eax

	movl	C(prowdestbase),%edi
	movl	%eax,sb_v

Lv_loop_RGBmip3:
//FIXME pipeline this
	movl	C(r_lightwidth),%ebx
	movl	C(r_lightptrr),%edx
	movl	(%edx),%eax
	movl	4(%edx),%ecx
	leal	(%edx,%ebx,4),%edx
	movl	%eax,C(lightrleft)
	movl	%ecx,C(lightrright)
	movl	%edx,C(r_lightptrr)
	movl	(%edx),%ebp
	subl	%eax,%ebp
	movl	%ebp,%eax
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
	leal	(%edx,%ebx,4),%edx
	movl	%edx,C(r_lightptrg)
	movl	(%edx),%ebp
	subl	%eax,%ebp
	movl	%ebp,%eax
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
	leal	(%edx,%ebx,4),%edx
	movl	%edx,C(r_lightptrb)
	movl	(%edx),%ebp
	subl	%eax,%ebp
	movl	%ebp,%eax
	shrl	$1,%eax
	movl	%eax,C(lightbleftstep)
	movl	4(%edx),%eax
	subl	%ecx,%eax
	shrl	$1,%eax
	movl	%eax,C(lightbrightstep)

	xorl	%ebx,%ebx
	xorl	%ecx,%ecx

//
// Red component
//
	movl	C(lightrright),%edx
	movl	C(lightrleft),%ebp

	subl	%edx,%ebp
	movb	1(%esi),%bl

	sarl	$1,%ebp
	movb	(%esi),%cl

	movb	%dh,%bh
	addl	%ebp,%edx

	movb	%dh,%ch
	addl	C(roffs),%edi

	movb	0x12345678(%ebx),%al
LRPatch28:
	movl	C(lightgright),%edx

	movb	%al,4(%edi)
	movl	C(lightgleft),%ebp

	movb	0x12345678(%ecx),%al
LRPatch29:
	subl	%edx,%ebp

	movb	%al,(%edi)
	movb	1(%esi),%bl

	subl	C(roffs),%edi
	addl	C(goffs),%edi

//
// Green component
//

	sarl	$1,%ebp
	movb	(%esi),%cl

	movb	%dh,%bh
	addl	%ebp,%edx

	movb	%dh,%ch

	movb	0x12345678(%ebx),%al
LGPatch28:
	movl	C(lightbright),%edx

	movb	%al,4(%edi)
	movl	C(lightbleft),%ebp

	movb	0x12345678(%ecx),%al
LGPatch29:
	subl	%edx,%ebp

	movb	%al,(%edi)

	subl	C(goffs),%edi
	addl	C(boffs),%edi

//
// Blue component
//

	sarl	$1,%ebp
	movb	1(%esi),%bl

	movb	%dh,%bh
	addl	%ebp,%edx

	movb	%dh,%ch
	movb	(%esi),%cl

	movb	0x12345678(%ebx),%al
LBPatch28:
	movl	C(lightrrightstep),%edx

	movb	%al,4(%edi)
	addl	%edx,C(lightrright)

	movb	0x12345678(%ecx),%al
LBPatch29:
	movl	C(lightrleftstep),%edx

	movb	%al,(%edi)
	addl	C(surfrowbytes),%edi

	subl	C(boffs),%edi

	addl	%edx,C(lightrleft)
	movl	C(lightgrightstep),%eax

	addl	%eax,C(lightgright)
	movl	C(lightgleftstep),%eax

	addl	%eax,C(lightgleft)
	movl	C(lightbrightstep),%eax

	addl	%eax,C(lightbright)
	movl	C(lightbleftstep),%eax

	addl	%eax,C(lightbleft)
	addl	C(sourcetstep),%esi

//
// Red component
//
	movl	C(lightrright),%edx
	movl	C(lightrleft),%ebp

	subl	%edx,%ebp
	movb	1(%esi),%bl

	sarl	$1,%ebp
	movb	(%esi),%cl

	movb	%dh,%bh
	addl	%ebp,%edx

	movb	%dh,%ch
	addl	C(roffs),%edi

	movb	0x12345678(%ebx),%al
LRPatch30:
	movl	C(lightgright),%edx

	movb	%al,4(%edi)
	movl	C(lightgleft),%ebp

	movb	0x12345678(%ecx),%al
LRPatch31:
	subl	%edx,%ebp

	movb	%al,(%edi)
	movb	1(%esi),%bl

	subl	C(roffs),%edi
	addl	C(goffs),%edi

//
// Green component
//

	sarl	$1,%ebp
	movb	(%esi),%cl

	movb	%dh,%bh
	addl	%ebp,%edx

	movb	%dh,%ch

	movb	0x12345678(%ebx),%al
LGPatch30:
	movl	C(lightbright),%edx

	movb	%al,4(%edi)
	movl	C(lightbleft),%ebp

	movb	0x12345678(%ecx),%al
LGPatch31:
	subl	%edx,%ebp

	movb	%al,(%edi)

	subl	C(goffs),%edi
	addl	C(boffs),%edi

//
// Blue component
//

	sarl	$1,%ebp
	movb	1(%esi),%bl

	movb	%dh,%bh
	addl	%ebp,%edx

	movb	%dh,%ch
	movb	(%esi),%cl

	movb	0x12345678(%ebx),%al
LBPatch30:

	movb	%al,4(%edi)

	movb	0x12345678(%ecx),%al
LBPatch31:
	addl	C(sourcetstep),%esi

	movb	%al,(%edi)
	addl	C(surfrowbytes),%edi

	subl	C(boffs),%edi

	cmpl	C(r_sourcemax),%esi
	jb		LSkip_RGBmip3
	subl	C(r_stepback),%esi
LSkip_RGBmip3:
	decl	sb_v
	jnz		Lv_loop_RGBmip3

	popl	%ebx
	popl	%esi
	popl	%edi
	popl	%ebp
	ret

.globl C(D_Surf32End)
C(D_Surf32End):

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

LRPatchTable:
	.long	LRPatch0-4
	.long	LRPatch1-4
	.long	LRPatch2-4
	.long	LRPatch3-4
	.long	LRPatch4-4
	.long	LRPatch5-4
	.long	LRPatch6-4
	.long	LRPatch7-4
	.long	LRPatch8-4
	.long	LRPatch9-4
	.long	LRPatch10-4
	.long	LRPatch11-4
	.long	LRPatch12-4
	.long	LRPatch13-4
	.long	LRPatch14-4
	.long	LRPatch15-4
	.long	LRPatch16-4
	.long	LRPatch17-4
	.long	LRPatch18-4
	.long	LRPatch19-4
	.long	LRPatch20-4
	.long	LRPatch21-4
	.long	LRPatch22-4
	.long	LRPatch23-4
	.long	LRPatch24-4
	.long	LRPatch25-4
	.long	LRPatch26-4
	.long	LRPatch27-4
	.long	LRPatch28-4
	.long	LRPatch29-4
	.long	LRPatch30-4
	.long	LRPatch31-4

LGPatchTable:
	.long	LGPatch0-4
	.long	LGPatch1-4
	.long	LGPatch2-4
	.long	LGPatch3-4
	.long	LGPatch4-4
	.long	LGPatch5-4
	.long	LGPatch6-4
	.long	LGPatch7-4
	.long	LGPatch8-4
	.long	LGPatch9-4
	.long	LGPatch10-4
	.long	LGPatch11-4
	.long	LGPatch12-4
	.long	LGPatch13-4
	.long	LGPatch14-4
	.long	LGPatch15-4
	.long	LGPatch16-4
	.long	LGPatch17-4
	.long	LGPatch18-4
	.long	LGPatch19-4
	.long	LGPatch20-4
	.long	LGPatch21-4
	.long	LGPatch22-4
	.long	LGPatch23-4
	.long	LGPatch24-4
	.long	LGPatch25-4
	.long	LGPatch26-4
	.long	LGPatch27-4
	.long	LGPatch28-4
	.long	LGPatch29-4
	.long	LGPatch30-4
	.long	LGPatch31-4

LBPatchTable:
	.long	LBPatch0-4
	.long	LBPatch1-4
	.long	LBPatch2-4
	.long	LBPatch3-4
	.long	LBPatch4-4
	.long	LBPatch5-4
	.long	LBPatch6-4
	.long	LBPatch7-4
	.long	LBPatch8-4
	.long	LBPatch9-4
	.long	LBPatch10-4
	.long	LBPatch11-4
	.long	LBPatch12-4
	.long	LBPatch13-4
	.long	LBPatch14-4
	.long	LBPatch15-4
	.long	LBPatch16-4
	.long	LBPatch17-4
	.long	LBPatch18-4
	.long	LBPatch19-4
	.long	LBPatch20-4
	.long	LBPatch21-4
	.long	LBPatch22-4
	.long	LBPatch23-4
	.long	LBPatch24-4
	.long	LBPatch25-4
	.long	LBPatch26-4
	.long	LBPatch27-4
	.long	LBPatch28-4
	.long	LBPatch29-4
	.long	LBPatch30-4
	.long	LBPatch31-4

	.text

	Align4
.globl C(D_Surf32Patch)
C(D_Surf32Patch):
	pushl	%ebx

	movl	C(fadetable32),%eax
	movl	$LPatchTable,%ebx
	movl	$32,%ecx
LPatchLoop:
	movl	(%ebx),%edx
	addl	$4,%ebx
	movl	%eax,(%edx)
	decl	%ecx
	jnz		LPatchLoop

	movl	C(fadetable32r),%eax
	movl	$LRPatchTable,%ebx
	movl	$32,%ecx
LRPatchLoop:
	movl	(%ebx),%edx
	addl	$4,%ebx
	movl	%eax,(%edx)
	decl	%ecx
	jnz		LRPatchLoop

	movl	C(fadetable32g),%eax
	movl	$LGPatchTable,%ebx
	movl	$32,%ecx
LGPatchLoop:
	movl	(%ebx),%edx
	addl	$4,%ebx
	movl	%eax,(%edx)
	decl	%ecx
	jnz		LGPatchLoop

	movl	C(fadetable32b),%eax
	movl	$LBPatchTable,%ebx
	movl	$32,%ecx
LBPatchLoop:
	movl	(%ebx),%edx
	addl	$4,%ebx
	movl	%eax,(%edx)
	decl	%ecx
	jnz		LBPatchLoop

	popl	%ebx

	ret

#endif

//**************************************************************************
//
//	$Log$
//	Revision 1.6  2002/11/16 17:11:15  dj_jl
//	Improving software driver class.
//
//	Revision 1.5  2002/01/07 12:16:41  dj_jl
//	Changed copyright year
//	
//	Revision 1.4  2001/08/02 17:43:17  dj_jl
//	Added new asm for 32-bits
//	
//	Revision 1.3  2001/07/31 17:16:30  dj_jl
//	Just moved Log to the end of file
//	
//	Revision 1.2  2001/07/27 14:27:54  dj_jl
//	Update with Id-s and Log-s, some fixes
//
//**************************************************************************
