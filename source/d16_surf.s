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
//**	x86 assembly-language 16 bpp surface block drawing code.
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

//----------------------------------------------------------------------
// Surface block drawer for mip level 0
//----------------------------------------------------------------------

	Align4
.globl C(D_DrawSurfaceBlock16_mip0)
C(D_DrawSurfaceBlock16_mip0):
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

	movw	C(d_fadetable16)(,%ebx,2),%ax
	movb	13(%esi),%bl

	rorl	$16,%eax

	movw	C(d_fadetable16)(,%ecx,2),%ax
	movb	12(%esi),%cl

	movb	%dh,%bh
	addl	%ebp,%edx

	movl	%eax,28(%edi)
	movb	%dh,%ch

	movw	C(d_fadetable16)(,%ebx,2),%ax
	addl	%ebp,%edx

	rorl	$16,%eax

	movw	C(d_fadetable16)(,%ecx,2),%ax
	movb	11(%esi),%bl

	movb	10(%esi),%cl
	movl	%eax,24(%edi)

	movb	%dh,%bh
	addl	%ebp,%edx

	movb	%dh,%ch
	addl	%ebp,%edx

	movw	C(d_fadetable16)(,%ebx,2),%ax
	movb	9(%esi),%bl

	rorl	$16,%eax

	movw	C(d_fadetable16)(,%ecx,2),%ax
	movb	8(%esi),%cl

	movb	%dh,%bh
	addl	%ebp,%edx

	movl	%eax,20(%edi)
	movb	%dh,%ch

	movw	C(d_fadetable16)(,%ebx,2),%ax
	addl	%ebp,%edx

	rorl	$16,%eax

	movw	C(d_fadetable16)(,%ecx,2),%ax
	movb	7(%esi),%bl

	movb	6(%esi),%cl
	movl	%eax,16(%edi)

	movb	%dh,%bh
	addl	%ebp,%edx

	movb	%dh,%ch
	addl	%ebp,%edx

	movw	C(d_fadetable16)(,%ebx,2),%ax
	movb	5(%esi),%bl

	rorl	$16,%eax

	movw	C(d_fadetable16)(,%ecx,2),%ax
	movb	4(%esi),%cl

	movb	%dh,%bh
	addl	%ebp,%edx

	movl	%eax,12(%edi)
	movb	%dh,%ch

	movw	C(d_fadetable16)(,%ebx,2),%ax
	addl	%ebp,%edx

	rorl	$16,%eax

	movw	C(d_fadetable16)(,%ecx,2),%ax
	movb	3(%esi),%bl

	movb	2(%esi),%cl
	movl	%eax,8(%edi)

	movb	%dh,%bh
	addl	%ebp,%edx

	movb	%dh,%ch
	addl	%ebp,%edx

	movw	C(d_fadetable16)(,%ebx,2),%ax
	movb	1(%esi),%bl

	rorl	$16,%eax

	movw	C(d_fadetable16)(,%ecx,2),%ax
	movb	(%esi),%cl

	movb	%dh,%bh
	addl	%ebp,%edx

	movl	%eax,4(%edi)
	movb	%dh,%ch

	movw	C(d_fadetable16)(,%ebx,2),%ax
	movl	C(lightright),%edx

	rorl	$16,%eax

	movw	C(d_fadetable16)(,%ecx,2),%ax
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
.globl C(D_DrawSurfaceBlock16_mip1)
C(D_DrawSurfaceBlock16_mip1):
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

	movw	C(d_fadetable16)(,%ebx,2),%ax
	movb	5(%esi),%bl

	rorl	$16,%eax

	movw	C(d_fadetable16)(,%ecx,2),%ax
	movb	4(%esi),%cl

	movb	%dh,%bh
	addl	%ebp,%edx

	movl	%eax,12(%edi)
	movb	%dh,%ch

	movw	C(d_fadetable16)(,%ebx,2),%ax
	addl	%ebp,%edx

	rorl	$16,%eax

	movw	C(d_fadetable16)(,%ecx,2),%ax
	movb	3(%esi),%bl

	movb	2(%esi),%cl
	movl	%eax,8(%edi)

	movb	%dh,%bh
	addl	%ebp,%edx

	movb	%dh,%ch
	addl	%ebp,%edx

	movw	C(d_fadetable16)(,%ebx,2),%ax
	movb	1(%esi),%bl

	rorl	$16,%eax

	movw	C(d_fadetable16)(,%ecx,2),%ax
	movb	(%esi),%cl

	movb	%dh,%bh
	addl	%ebp,%edx

	movl	%eax,4(%edi)
	movb	%dh,%ch

	movw	C(d_fadetable16)(,%ebx,2),%ax
	movl	C(lightright),%edx

	rorl	$16,%eax

	movw	C(d_fadetable16)(,%ecx,2),%ax
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
.globl C(D_DrawSurfaceBlock16_mip2)
C(D_DrawSurfaceBlock16_mip2):
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

	movw	C(d_fadetable16)(,%ebx,2),%ax
	movb	1(%esi),%bl

	rorl	$16,%eax

	movw	C(d_fadetable16)(,%ecx,2),%ax
	movb	(%esi),%cl

	movb	%dh,%bh
	addl	%ebp,%edx

	movl	%eax,4(%edi)
	movb	%dh,%ch

	movw	C(d_fadetable16)(,%ebx,2),%ax
	movl	C(lightright),%edx

	rorl	$16,%eax

	movw	C(d_fadetable16)(,%ecx,2),%ax
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
.globl C(D_DrawSurfaceBlock16_mip3)
C(D_DrawSurfaceBlock16_mip3):
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

	movw	C(d_fadetable16)(,%ebx,2),%ax
	addl	%ebp,%edx

	movw	%ax,2(%edi)
	movb	%dh,%ch

	movw	C(d_fadetable16)(,%ecx,2),%ax
	movl	C(lightright),%edx

	movw	%ax,(%edi)
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

	movw	C(d_fadetable16)(,%ebx,2),%ax
	addl	%ebp,%edx

	movw	%ax,2(%edi)
	movb	%dh,%ch

	movw	C(d_fadetable16)(,%ecx,2),%ax
	movl	C(sourcetstep),%edx

	movw	%ax,(%edi)
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
//	Coloured lighting block drawers
//
//==========================================================================

//----------------------------------------------------------------------
// Surface block drawer for mip level 0
//----------------------------------------------------------------------

	Align16
.globl C(D_DrawSurfaceBlock16RGB_mip0)
C(D_DrawSurfaceBlock16RGB_mip0):
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

Lblockloop16_RGBmip0:

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

	movw	C(d_fadetable16r)(,%ebx,2),%ax
	movb	13(%esi),%bl

	shll	$16,%eax
	addl	%ebp,%edx

	movw	C(d_fadetable16r)(,%ecx,2),%ax
	movb	12(%esi),%cl

	movb	%dh,%bh
	addl	%ebp,%edx

	movl	%eax,28(%edi)
	movb	%dh,%ch

	movw	C(d_fadetable16r)(,%ebx,2),%ax
	movb	11(%esi),%bl

	shll	$16,%eax
	addl	%ebp,%edx

	movw	C(d_fadetable16r)(,%ecx,2),%ax
	movb	10(%esi),%cl

	movb	%dh,%bh
	addl	%ebp,%edx

	movl	%eax,24(%edi)
	movb	%dh,%ch

	movw	C(d_fadetable16r)(,%ebx,2),%ax
	movb	9(%esi),%bl

	shll	$16,%eax
	addl	%ebp,%edx

	movw	C(d_fadetable16r)(,%ecx,2),%ax
	movb	8(%esi),%cl

	movb	%dh,%bh
	addl	%ebp,%edx

	movl	%eax,20(%edi)
	movb	%dh,%ch

	movw	C(d_fadetable16r)(,%ebx,2),%ax
	movb	7(%esi),%bl

	shll	$16,%eax
	addl	%ebp,%edx

	movw	C(d_fadetable16r)(,%ecx,2),%ax
	movb	6(%esi),%cl

	movb	%dh,%bh
	addl	%ebp,%edx

	movl	%eax,16(%edi)
	movb	%dh,%ch

	movw	C(d_fadetable16r)(,%ebx,2),%ax
	movb	5(%esi),%bl

	shll	$16,%eax
	addl	%ebp,%edx

	movw	C(d_fadetable16r)(,%ecx,2),%ax
	movb	4(%esi),%cl

	movb	%dh,%bh
	addl	%ebp,%edx

	movl	%eax,12(%edi)
	movb	%dh,%ch

	movw	C(d_fadetable16r)(,%ebx,2),%ax
	movb	3(%esi),%bl

	shll	$16,%eax
	addl	%ebp,%edx

	movw	C(d_fadetable16r)(,%ecx,2),%ax
	movb	2(%esi),%cl

	movb	%dh,%bh
	addl	%ebp,%edx

	movl	%eax,8(%edi)
	movb	%dh,%ch

	movw	C(d_fadetable16r)(,%ebx,2),%ax
	movb	1(%esi),%bl

	shll	$16,%eax
	addl	%ebp,%edx

	movw	C(d_fadetable16r)(,%ecx,2),%ax
	movb	(%esi),%cl

	movb	%dh,%bh
	addl	%ebp,%edx

	movl	%eax,4(%edi)
	movb	%dh,%ch

	movw	C(d_fadetable16r)(,%ebx,2),%ax
	movl	C(lightgright),%edx

	shll	$16,%eax
	movl	C(lightgleft),%ebp

	movw	C(d_fadetable16r)(,%ecx,2),%ax
	subl	%edx,%ebp

	movl	%eax,(%edi)
	movb	15(%esi),%bl

//
// Green component
//

	sarl	$4,%ebp
	movb	14(%esi),%cl

	movb	%dh,%bh
	addl	%ebp,%edx

	movb	%dh,%ch

	movw	C(d_fadetable16g)(,%ebx,2),%ax
	movb	13(%esi),%bl

	shll	$16,%eax
	addl	%ebp,%edx

	movw	C(d_fadetable16g)(,%ecx,2),%ax
	movb	12(%esi),%cl

	movb	%dh,%bh
	addl	%ebp,%edx

	orl		%eax,28(%edi)
	movb	%dh,%ch

	movw	C(d_fadetable16g)(,%ebx,2),%ax
	movb	11(%esi),%bl

	shll	$16,%eax
	addl	%ebp,%edx

	movw	C(d_fadetable16g)(,%ecx,2),%ax
	movb	10(%esi),%cl

	movb	%dh,%bh
	addl	%ebp,%edx

	orl		%eax,24(%edi)
	movb	%dh,%ch

	movw	C(d_fadetable16g)(,%ebx,2),%ax
	movb	9(%esi),%bl

	shll	$16,%eax
	addl	%ebp,%edx

	movw	C(d_fadetable16g)(,%ecx,2),%ax
	movb	8(%esi),%cl

	movb	%dh,%bh
	addl	%ebp,%edx

	orl		%eax,20(%edi)
	movb	%dh,%ch

	movw	C(d_fadetable16g)(,%ebx,2),%ax
	movb	7(%esi),%bl

	shll	$16,%eax
	addl	%ebp,%edx

	movw	C(d_fadetable16g)(,%ecx,2),%ax
	movb	6(%esi),%cl

	movb	%dh,%bh
	addl	%ebp,%edx

	orl		%eax,16(%edi)
	movb	%dh,%ch

	movw	C(d_fadetable16g)(,%ebx,2),%ax
	movb	5(%esi),%bl

	shll	$16,%eax
	addl	%ebp,%edx

	movw	C(d_fadetable16g)(,%ecx,2),%ax
	movb	4(%esi),%cl

	movb	%dh,%bh
	addl	%ebp,%edx

	orl		%eax,12(%edi)
	movb	%dh,%ch

	movw	C(d_fadetable16g)(,%ebx,2),%ax
	movb	3(%esi),%bl

	shll	$16,%eax
	addl	%ebp,%edx

	movw	C(d_fadetable16g)(,%ecx,2),%ax
	movb	2(%esi),%cl

	movb	%dh,%bh
	addl	%ebp,%edx

	orl		%eax,8(%edi)
	movb	%dh,%ch

	movw	C(d_fadetable16g)(,%ebx,2),%ax
	movb	1(%esi),%bl

	shll	$16,%eax
	addl	%ebp,%edx

	movw	C(d_fadetable16g)(,%ecx,2),%ax
	movb	(%esi),%cl

	movb	%dh,%bh
	addl	%ebp,%edx

	orl		%eax,4(%edi)
	movb	%dh,%ch

	movw	C(d_fadetable16g)(,%ebx,2),%ax
	movl	C(lightbright),%edx

	shll	$16,%eax
	movl	C(lightbleft),%ebp

	movw	C(d_fadetable16g)(,%ecx,2),%ax
	subl	%edx,%ebp

	orl		%eax,(%edi)

//
// Blue component
//

	sarl	$4,%ebp
	movb	15(%esi),%bl

	movb	%dh,%bh
	addl	%ebp,%edx

	movb	%dh,%ch
	movb	14(%esi),%cl

	movw	C(d_fadetable16b)(,%ebx,2),%ax
	movb	13(%esi),%bl

	shll	$16,%eax
	addl	%ebp,%edx

	movw	C(d_fadetable16b)(,%ecx,2),%ax
	movb	12(%esi),%cl

	movb	%dh,%bh
	addl	%ebp,%edx

	orl		%eax,28(%edi)
	movb	%dh,%ch

	movw	C(d_fadetable16b)(,%ebx,2),%ax
	movb	11(%esi),%bl

	shll	$16,%eax
	addl	%ebp,%edx

	movw	C(d_fadetable16b)(,%ecx,2),%ax
	movb	10(%esi),%cl

	movb	%dh,%bh
	addl	%ebp,%edx

	orl		%eax,24(%edi)
	movb	%dh,%ch

	movw	C(d_fadetable16b)(,%ebx,2),%ax
	movb	9(%esi),%bl

	shll	$16,%eax
	addl	%ebp,%edx

	movw	C(d_fadetable16b)(,%ecx,2),%ax
	movb	8(%esi),%cl

	movb	%dh,%bh
	addl	%ebp,%edx

	orl		%eax,20(%edi)
	movb	%dh,%ch

	movw	C(d_fadetable16b)(,%ebx,2),%ax
	movb	7(%esi),%bl

	shll	$16,%eax
	addl	%ebp,%edx

	movw	C(d_fadetable16b)(,%ecx,2),%ax
	movb	6(%esi),%cl

	movb	%dh,%bh
	addl	%ebp,%edx

	orl		%eax,16(%edi)
	movb	%dh,%ch

	movw	C(d_fadetable16b)(,%ebx,2),%ax
	movb	5(%esi),%bl

	shll	$16,%eax
	addl	%ebp,%edx

	movw	C(d_fadetable16b)(,%ecx,2),%ax
	movb	4(%esi),%cl

	movb	%dh,%bh
	addl	%ebp,%edx

	orl		%eax,12(%edi)
	movb	%dh,%ch

	movw	C(d_fadetable16b)(,%ebx,2),%ax
	movb	3(%esi),%bl

	shll	$16,%eax
	addl	%ebp,%edx

	movw	C(d_fadetable16b)(,%ecx,2),%ax
	movb	2(%esi),%cl

	movb	%dh,%bh
	addl	%ebp,%edx

	orl		%eax,8(%edi)
	movb	%dh,%ch

	movw	C(d_fadetable16b)(,%ebx,2),%ax
	movb	1(%esi),%bl

	shll	$16,%eax
	addl	%ebp,%edx

	movw	C(d_fadetable16b)(,%ecx,2),%ax
	movb	(%esi),%cl

	movb	%dh,%bh
	addl	%ebp,%edx

	orl		%eax,4(%edi)
	movb	%dh,%ch

	movw	C(d_fadetable16b)(,%ebx,2),%ax
	movl	C(lightrrightstep),%edx

	shll	$16,%eax
	addl	%edx,C(lightrright)

	movw	C(d_fadetable16b)(,%ecx,2),%ax
	movl	C(lightrleftstep),%edx

	orl		%eax,(%edi)
	addl	C(surfrowbytes),%edi

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
	jnz		Lblockloop16_RGBmip0

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
.globl C(D_DrawSurfaceBlock16RGB_mip1)
C(D_DrawSurfaceBlock16RGB_mip1):
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

Lblockloop16_RGBmip1:

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

	movw	C(d_fadetable16r)(,%ebx,2),%ax
	movb	5(%esi),%bl

	shll	$16,%eax
	addl	%ebp,%edx

	movw	C(d_fadetable16r)(,%ecx,2),%ax
	movb	4(%esi),%cl

	movb	%dh,%bh
	addl	%ebp,%edx

	movl	%eax,12(%edi)
	movb	%dh,%ch

	movw	C(d_fadetable16r)(,%ebx,2),%ax
	movb	3(%esi),%bl

	shll	$16,%eax
	addl	%ebp,%edx

	movw	C(d_fadetable16r)(,%ecx,2),%ax
	movb	2(%esi),%cl

	movb	%dh,%bh
	addl	%ebp,%edx

	movl	%eax,8(%edi)
	movb	%dh,%ch

	movw	C(d_fadetable16r)(,%ebx,2),%ax
	movb	1(%esi),%bl

	shll	$16,%eax
	addl	%ebp,%edx

	movw	C(d_fadetable16r)(,%ecx,2),%ax
	movb	(%esi),%cl

	movb	%dh,%bh
	addl	%ebp,%edx

	movl	%eax,4(%edi)
	movb	%dh,%ch

	movw	C(d_fadetable16r)(,%ebx,2),%ax
	movl	C(lightgright),%edx

	shll	$16,%eax
	movl	C(lightgleft),%ebp

	movw	C(d_fadetable16r)(,%ecx,2),%ax
	subl	%edx,%ebp

	movl	%eax,(%edi)
	movb	7(%esi),%bl

//
// Green component
//

	sarl	$3,%ebp
	movb	6(%esi),%cl

	movb	%dh,%bh
	addl	%ebp,%edx

	movb	%dh,%ch

	movw	C(d_fadetable16g)(,%ebx,2),%ax
	movb	5(%esi),%bl

	shll	$16,%eax
	addl	%ebp,%edx

	movw	C(d_fadetable16g)(,%ecx,2),%ax
	movb	4(%esi),%cl

	movb	%dh,%bh
	addl	%ebp,%edx

	orl		%eax,12(%edi)
	movb	%dh,%ch

	movw	C(d_fadetable16g)(,%ebx,2),%ax
	movb	3(%esi),%bl

	shll	$16,%eax
	addl	%ebp,%edx

	movw	C(d_fadetable16g)(,%ecx,2),%ax
	movb	2(%esi),%cl

	movb	%dh,%bh
	addl	%ebp,%edx

	orl		%eax,8(%edi)
	movb	%dh,%ch

	movw	C(d_fadetable16g)(,%ebx,2),%ax
	movb	1(%esi),%bl

	shll	$16,%eax
	addl	%ebp,%edx

	movw	C(d_fadetable16g)(,%ecx,2),%ax
	movb	(%esi),%cl

	movb	%dh,%bh
	addl	%ebp,%edx

	orl		%eax,4(%edi)
	movb	%dh,%ch

	movw	C(d_fadetable16g)(,%ebx,2),%ax
	movl	C(lightbright),%edx

	shll	$16,%eax
	movl	C(lightbleft),%ebp

	movw	C(d_fadetable16g)(,%ecx,2),%ax
	subl	%edx,%ebp

	orl		%eax,(%edi)

//
// Blue component
//

	sarl	$3,%ebp
	movb	7(%esi),%bl

	movb	%dh,%bh
	addl	%ebp,%edx

	movb	%dh,%ch
	movb	6(%esi),%cl

	movw	C(d_fadetable16b)(,%ebx,2),%ax
	movb	5(%esi),%bl

	shll	$16,%eax
	addl	%ebp,%edx

	movw	C(d_fadetable16b)(,%ecx,2),%ax
	movb	4(%esi),%cl

	movb	%dh,%bh
	addl	%ebp,%edx

	orl		%eax,12(%edi)
	movb	%dh,%ch

	movw	C(d_fadetable16b)(,%ebx,2),%ax
	movb	3(%esi),%bl

	shll	$16,%eax
	addl	%ebp,%edx

	movw	C(d_fadetable16b)(,%ecx,2),%ax
	movb	2(%esi),%cl

	movb	%dh,%bh
	addl	%ebp,%edx

	orl		%eax,8(%edi)
	movb	%dh,%ch

	movw	C(d_fadetable16b)(,%ebx,2),%ax
	movb	1(%esi),%bl

	shll	$16,%eax
	addl	%ebp,%edx

	movw	C(d_fadetable16b)(,%ecx,2),%ax
	movb	(%esi),%cl

	movb	%dh,%bh
	addl	%ebp,%edx

	orl		%eax,4(%edi)
	movb	%dh,%ch

	movw	C(d_fadetable16b)(,%ebx,2),%ax
	movl	C(lightrrightstep),%edx

	shll	$16,%eax
	addl	%edx,C(lightrright)

	movw	C(d_fadetable16b)(,%ecx,2),%ax
	movl	C(lightrleftstep),%edx

	orl		%eax,(%edi)
	addl	C(surfrowbytes),%edi

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
	jnz		Lblockloop16_RGBmip1

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
.globl C(D_DrawSurfaceBlock16RGB_mip2)
C(D_DrawSurfaceBlock16RGB_mip2):
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

Lblockloop16_RGBmip2:

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

	movw	C(d_fadetable16r)(,%ebx,2),%ax
	movb	1(%esi),%bl

	shll	$16,%eax
	addl	%ebp,%edx

	movw	C(d_fadetable16r)(,%ecx,2),%ax
	movb	(%esi),%cl

	movb	%dh,%bh
	addl	%ebp,%edx

	movl	%eax,4(%edi)
	movb	%dh,%ch

	movw	C(d_fadetable16r)(,%ebx,2),%ax
	movl	C(lightgright),%edx

	shll	$16,%eax
	movl	C(lightgleft),%ebp

	movw	C(d_fadetable16r)(,%ecx,2),%ax
	subl	%edx,%ebp

	movl	%eax,(%edi)
	movb	3(%esi),%bl

//
// Green component
//

	sarl	$2,%ebp
	movb	2(%esi),%cl

	movb	%dh,%bh
	addl	%ebp,%edx

	movb	%dh,%ch

	movw	C(d_fadetable16g)(,%ebx,2),%ax
	movb	1(%esi),%bl

	shll	$16,%eax
	addl	%ebp,%edx

	movw	C(d_fadetable16g)(,%ecx,2),%ax
	movb	(%esi),%cl

	movb	%dh,%bh
	addl	%ebp,%edx

	orl		%eax,4(%edi)
	movb	%dh,%ch

	movw	C(d_fadetable16g)(,%ebx,2),%ax
	movl	C(lightbright),%edx

	shll	$16,%eax
	movl	C(lightbleft),%ebp

	movw	C(d_fadetable16g)(,%ecx,2),%ax
	subl	%edx,%ebp

	orl		%eax,(%edi)

//
// Blue component
//

	sarl	$2,%ebp
	movb	3(%esi),%bl

	movb	%dh,%bh
	addl	%ebp,%edx

	movb	%dh,%ch
	movb	2(%esi),%cl

	movw	C(d_fadetable16b)(,%ebx,2),%ax
	movb	1(%esi),%bl

	shll	$16,%eax
	addl	%ebp,%edx

	movw	C(d_fadetable16b)(,%ecx,2),%ax
	movb	(%esi),%cl

	movb	%dh,%bh
	addl	%ebp,%edx

	orl		%eax,4(%edi)
	movb	%dh,%ch

	movw	C(d_fadetable16b)(,%ebx,2),%ax
	movl	C(lightrrightstep),%edx

	shll	$16,%eax
	addl	%edx,C(lightrright)

	movw	C(d_fadetable16b)(,%ecx,2),%ax
	movl	C(lightrleftstep),%edx

	orl		%eax,(%edi)
	addl	C(surfrowbytes),%edi

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
	jnz		Lblockloop16_RGBmip2

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
.globl C(D_DrawSurfaceBlock16RGB_mip3)
C(D_DrawSurfaceBlock16RGB_mip3):
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

	movw	C(d_fadetable16r)(,%ebx,2),%ax
	movl	C(lightgright),%edx

	shll	$16,%eax
	movl	C(lightgleft),%ebp

	movw	C(d_fadetable16r)(,%ecx,2),%ax
	subl	%edx,%ebp

	movl	%eax,(%edi)
	movb	1(%esi),%bl

//
// Green component
//

	sarl	$1,%ebp
	movb	(%esi),%cl

	movb	%dh,%bh
	addl	%ebp,%edx

	movb	%dh,%ch

	movw	C(d_fadetable16g)(,%ebx,2),%ax
	movl	C(lightbright),%edx

	shll	$16,%eax
	movl	C(lightbleft),%ebp

	movw	C(d_fadetable16g)(,%ecx,2),%ax
	subl	%edx,%ebp

	orl		%eax,(%edi)

//
// Blue component
//

	sarl	$1,%ebp
	movb	1(%esi),%bl

	movb	%dh,%bh
	addl	%ebp,%edx

	movb	%dh,%ch
	movb	(%esi),%cl

	movw	C(d_fadetable16b)(,%ebx,2),%ax
	movl	C(lightrrightstep),%edx

	shll	$16,%eax
	addl	%edx,C(lightrright)

	movw	C(d_fadetable16b)(,%ecx,2),%ax
	movl	C(lightrleftstep),%edx

	orl		%eax,(%edi)
	addl	C(surfrowbytes),%edi

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

	movw	C(d_fadetable16r)(,%ebx,2),%ax
	movl	C(lightgright),%edx

	shll	$16,%eax
	movl	C(lightgleft),%ebp

	movw	C(d_fadetable16r)(,%ecx,2),%ax
	subl	%edx,%ebp

	movl	%eax,(%edi)
	movb	1(%esi),%bl

//
// Green component
//

	sarl	$1,%ebp
	movb	(%esi),%cl

	movb	%dh,%bh
	addl	%ebp,%edx

	movb	%dh,%ch

	movw	C(d_fadetable16g)(,%ebx,2),%ax
	movl	C(lightbright),%edx

	shll	$16,%eax
	movl	C(lightbleft),%ebp

	movw	C(d_fadetable16g)(,%ecx,2),%ax
	subl	%edx,%ebp

	orl		%eax,(%edi)

//
// Blue component
//

	sarl	$1,%ebp
	movb	1(%esi),%bl

	movb	%dh,%bh
	addl	%ebp,%edx

	movb	%dh,%ch
	movb	(%esi),%cl

	movw	C(d_fadetable16b)(,%ebx,2),%ax

	shll	$16,%eax

	movw	C(d_fadetable16b)(,%ecx,2),%ax
	addl	C(sourcetstep),%esi

	orl		%eax,(%edi)
	addl	C(surfrowbytes),%edi

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

#endif
