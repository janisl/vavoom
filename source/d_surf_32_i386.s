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
//**	x86 assembly-language 32 bpp surface block drawing code.
//**
//**************************************************************************

#include "asm_i386.h"

	.data

sb_v:		.long	0

	.text

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

	movl	C(d_fadetable32)(,%ebx,4),%eax
	movb	13(%esi),%bl

	movl	%eax,60(%edi)

	movl	C(d_fadetable32)(,%ecx,4),%eax
	movb	12(%esi),%cl

	movb	%dh,%bh
	addl	%ebp,%edx

	movl	%eax,56(%edi)
	movb	%dh,%ch

	movl	C(d_fadetable32)(,%ebx,4),%eax
	addl	%ebp,%edx

	movl	%eax,52(%edi)

	movl	C(d_fadetable32)(,%ecx,4),%eax
	movb	11(%esi),%bl

	movb	10(%esi),%cl
	movl	%eax,48(%edi)

	movb	%dh,%bh
	addl	%ebp,%edx

	movb	%dh,%ch
	addl	%ebp,%edx

	movl	C(d_fadetable32)(,%ebx,4),%eax
	movb	9(%esi),%bl

	movl	%eax,44(%edi)

	movl	C(d_fadetable32)(,%ecx,4),%eax
	movb	8(%esi),%cl

	movb	%dh,%bh
	addl	%ebp,%edx

	movl	%eax,40(%edi)
	movb	%dh,%ch

	movl	C(d_fadetable32)(,%ebx,4),%eax
	addl	%ebp,%edx

	movl	%eax,36(%edi)

	movl	C(d_fadetable32)(,%ecx,4),%eax
	movb	7(%esi),%bl

	movb	6(%esi),%cl
	movl	%eax,32(%edi)

	movb	%dh,%bh
	addl	%ebp,%edx

	movb	%dh,%ch
	addl	%ebp,%edx

	movl	C(d_fadetable32)(,%ebx,4),%eax
	movb	5(%esi),%bl

	movl	%eax,28(%edi)

	movl	C(d_fadetable32)(,%ecx,4),%eax
	movb	4(%esi),%cl

	movb	%dh,%bh
	addl	%ebp,%edx

	movl	%eax,24(%edi)
	movb	%dh,%ch

	movl	C(d_fadetable32)(,%ebx,4),%eax
	addl	%ebp,%edx

	movl	%eax,20(%edi)

	movl	C(d_fadetable32)(,%ecx,4),%eax
	movb	3(%esi),%bl

	movb	2(%esi),%cl
	movl	%eax,16(%edi)

	movb	%dh,%bh
	addl	%ebp,%edx

	movb	%dh,%ch
	addl	%ebp,%edx

	movl	C(d_fadetable32)(,%ebx,4),%eax
	movb	1(%esi),%bl

	movl	%eax,12(%edi)

	movl	C(d_fadetable32)(,%ecx,4),%eax
	movb	(%esi),%cl

	movb	%dh,%bh
	addl	%ebp,%edx

	movl	%eax,8(%edi)
	movb	%dh,%ch

	movl	C(d_fadetable32)(,%ebx,4),%eax
	movl	C(lightright),%edx

	movl	%eax,4(%edi)

	movl	C(d_fadetable32)(,%ecx,4),%eax
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

	movl	C(d_fadetable32)(,%ebx,4),%eax
	movb	5(%esi),%bl

	movl	%eax,28(%edi)

	movl	C(d_fadetable32)(,%ecx,4),%eax
	movb	4(%esi),%cl

	movb	%dh,%bh
	addl	%ebp,%edx

	movl	%eax,24(%edi)
	movb	%dh,%ch

	movl	C(d_fadetable32)(,%ebx,4),%eax
	addl	%ebp,%edx

	movl	%eax,20(%edi)

	movl	C(d_fadetable32)(,%ecx,4),%eax
	movb	3(%esi),%bl

	movb	2(%esi),%cl
	movl	%eax,16(%edi)

	movb	%dh,%bh
	addl	%ebp,%edx

	movb	%dh,%ch
	addl	%ebp,%edx

	movl	C(d_fadetable32)(,%ebx,4),%eax
	movb	1(%esi),%bl

	movl	%eax,12(%edi)

	movl	C(d_fadetable32)(,%ecx,4),%eax
	movb	(%esi),%cl

	movb	%dh,%bh
	addl	%ebp,%edx

	movl	%eax,8(%edi)
	movb	%dh,%ch

	movl	C(d_fadetable32)(,%ebx,4),%eax
	movl	C(lightright),%edx

	movl	%eax,4(%edi)

	movl	C(d_fadetable32)(,%ecx,4),%eax
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

	movl	C(d_fadetable32)(,%ebx,4),%eax
	movb	1(%esi),%bl

	movl	%eax,12(%edi)

	movl	C(d_fadetable32)(,%ecx,4),%eax
	movb	(%esi),%cl

	movb	%dh,%bh
	addl	%ebp,%edx

	movl	%eax,8(%edi)
	movb	%dh,%ch

	movl	C(d_fadetable32)(,%ebx,4),%eax
	movl	C(lightright),%edx

	movl	%eax,4(%edi)

	movl	C(d_fadetable32)(,%ecx,4),%eax
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

	movl	C(d_fadetable32)(,%ebx,4),%eax
	addl	%ebp,%edx

	movl	%eax,4(%edi)
	movb	%dh,%ch

	movl	C(d_fadetable32)(,%ecx,4),%eax
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

	movl	C(d_fadetable32)(,%ebx,4),%eax
	addl	%ebp,%edx

	movl	%eax,4(%edi)
	movb	%dh,%ch

	movl	C(d_fadetable32)(,%ecx,4),%eax
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
//	Coloured lighting block drawers
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

	movb	C(d_fadetable32r)(%ebx),%al
	movb	13(%esi),%bl

	movb	%al,60(%edi)
	addl	%ebp,%edx

	movb	C(d_fadetable32r)(%ecx),%al
	movb	12(%esi),%cl

	movb	%dh,%bh
	addl	%ebp,%edx

	movb	%al,56(%edi)
	movb	%dh,%ch

	movb	C(d_fadetable32r)(%ebx),%al
	movb	11(%esi),%bl

	movb	%al,52(%edi)
	addl	%ebp,%edx

	movb	C(d_fadetable32r)(%ecx),%al
	movb	10(%esi),%cl

	movb	%dh,%bh
	addl	%ebp,%edx

	movb	%al,48(%edi)
	movb	%dh,%ch

	movb	C(d_fadetable32r)(%ebx),%al
	movb	9(%esi),%bl

	movb	%al,44(%edi)
	addl	%ebp,%edx

	movb	C(d_fadetable32r)(%ecx),%al
	movb	8(%esi),%cl

	movb	%dh,%bh
	addl	%ebp,%edx

	movb	%al,40(%edi)
	movb	%dh,%ch

	movb	C(d_fadetable32r)(%ebx),%al
	movb	7(%esi),%bl

	movb	%al,36(%edi)
	addl	%ebp,%edx

	movb	C(d_fadetable32r)(%ecx),%al
	movb	6(%esi),%cl

	movb	%dh,%bh
	addl	%ebp,%edx

	movb	%al,32(%edi)
	movb	%dh,%ch

	movb	C(d_fadetable32r)(%ebx),%al
	movb	5(%esi),%bl

	movb	%al,28(%edi)
	addl	%ebp,%edx

	movb	C(d_fadetable32r)(%ecx),%al
	movb	4(%esi),%cl

	movb	%dh,%bh
	addl	%ebp,%edx

	movb	%al,24(%edi)
	movb	%dh,%ch

	movb	C(d_fadetable32r)(%ebx),%al
	movb	3(%esi),%bl

	movb	%al,20(%edi)
	addl	%ebp,%edx

	movb	C(d_fadetable32r)(%ecx),%al
	movb	2(%esi),%cl

	movb	%dh,%bh
	addl	%ebp,%edx

	movb	%al,16(%edi)
	movb	%dh,%ch

	movb	C(d_fadetable32r)(%ebx),%al
	movb	1(%esi),%bl

	movb	%al,12(%edi)
	addl	%ebp,%edx

	movb	C(d_fadetable32r)(%ecx),%al
	movb	(%esi),%cl

	movb	%dh,%bh
	addl	%ebp,%edx

	movb	%al,8(%edi)
	movb	%dh,%ch

	movb	C(d_fadetable32r)(%ebx),%al
	movl	C(lightgright),%edx

	movb	%al,4(%edi)
	movl	C(lightgleft),%ebp

	movb	C(d_fadetable32r)(%ecx),%al
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

	movb	C(d_fadetable32g)(%ebx),%al
	movb	13(%esi),%bl

	movb	%al,60(%edi)
	addl	%ebp,%edx

	movb	C(d_fadetable32g)(%ecx),%al
	movb	12(%esi),%cl

	movb	%dh,%bh
	addl	%ebp,%edx

	movb	%al,56(%edi)
	movb	%dh,%ch

	movb	C(d_fadetable32g)(%ebx),%al
	movb	11(%esi),%bl

	movb	%al,52(%edi)
	addl	%ebp,%edx

	movb	C(d_fadetable32g)(%ecx),%al
	movb	10(%esi),%cl

	movb	%dh,%bh
	addl	%ebp,%edx

	movb	%al,48(%edi)
	movb	%dh,%ch

	movb	C(d_fadetable32g)(%ebx),%al
	movb	9(%esi),%bl

	movb	%al,44(%edi)
	addl	%ebp,%edx

	movb	C(d_fadetable32g)(%ecx),%al
	movb	8(%esi),%cl

	movb	%dh,%bh
	addl	%ebp,%edx

	movb	%al,40(%edi)
	movb	%dh,%ch

	movb	C(d_fadetable32g)(%ebx),%al
	movb	7(%esi),%bl

	movb	%al,36(%edi)
	addl	%ebp,%edx

	movb	C(d_fadetable32g)(%ecx),%al
	movb	6(%esi),%cl

	movb	%dh,%bh
	addl	%ebp,%edx

	movb	%al,32(%edi)
	movb	%dh,%ch

	movb	C(d_fadetable32g)(%ebx),%al
	movb	5(%esi),%bl

	movb	%al,28(%edi)
	addl	%ebp,%edx

	movb	C(d_fadetable32g)(%ecx),%al
	movb	4(%esi),%cl

	movb	%dh,%bh
	addl	%ebp,%edx

	movb	%al,24(%edi)
	movb	%dh,%ch

	movb	C(d_fadetable32g)(%ebx),%al
	movb	3(%esi),%bl

	movb	%al,20(%edi)
	addl	%ebp,%edx

	movb	C(d_fadetable32g)(%ecx),%al
	movb	2(%esi),%cl

	movb	%dh,%bh
	addl	%ebp,%edx

	movb	%al,16(%edi)
	movb	%dh,%ch

	movb	C(d_fadetable32g)(%ebx),%al
	movb	1(%esi),%bl

	movb	%al,12(%edi)
	addl	%ebp,%edx

	movb	C(d_fadetable32g)(%ecx),%al
	movb	(%esi),%cl

	movb	%dh,%bh
	addl	%ebp,%edx

	movb	%al,8(%edi)
	movb	%dh,%ch

	movb	C(d_fadetable32g)(%ebx),%al
	movl	C(lightbright),%edx

	movb	%al,4(%edi)
	movl	C(lightbleft),%ebp

	movb	C(d_fadetable32g)(%ecx),%al
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

	movb	C(d_fadetable32b)(%ebx),%al
	movb	13(%esi),%bl

	movb	%al,60(%edi)
	addl	%ebp,%edx

	movb	C(d_fadetable32b)(%ecx),%al
	movb	12(%esi),%cl

	movb	%dh,%bh
	addl	%ebp,%edx

	movb	%al,56(%edi)
	movb	%dh,%ch

	movb	C(d_fadetable32b)(%ebx),%al
	movb	11(%esi),%bl

	movb	%al,52(%edi)
	addl	%ebp,%edx

	movb	C(d_fadetable32b)(%ecx),%al
	movb	10(%esi),%cl

	movb	%dh,%bh
	addl	%ebp,%edx

	movb	%al,48(%edi)
	movb	%dh,%ch

	movb	C(d_fadetable32b)(%ebx),%al
	movb	9(%esi),%bl

	movb	%al,44(%edi)
	addl	%ebp,%edx

	movb	C(d_fadetable32b)(%ecx),%al
	movb	8(%esi),%cl

	movb	%dh,%bh
	addl	%ebp,%edx

	movb	%al,40(%edi)
	movb	%dh,%ch

	movb	C(d_fadetable32b)(%ebx),%al
	movb	7(%esi),%bl

	movb	%al,36(%edi)
	addl	%ebp,%edx

	movb	C(d_fadetable32b)(%ecx),%al
	movb	6(%esi),%cl

	movb	%dh,%bh
	addl	%ebp,%edx

	movb	%al,32(%edi)
	movb	%dh,%ch

	movb	C(d_fadetable32b)(%ebx),%al
	movb	5(%esi),%bl

	movb	%al,28(%edi)
	addl	%ebp,%edx

	movb	C(d_fadetable32b)(%ecx),%al
	movb	4(%esi),%cl

	movb	%dh,%bh
	addl	%ebp,%edx

	movb	%al,24(%edi)
	movb	%dh,%ch

	movb	C(d_fadetable32b)(%ebx),%al
	movb	3(%esi),%bl

	movb	%al,20(%edi)
	addl	%ebp,%edx

	movb	C(d_fadetable32b)(%ecx),%al
	movb	2(%esi),%cl

	movb	%dh,%bh
	addl	%ebp,%edx

	movb	%al,16(%edi)
	movb	%dh,%ch

	movb	C(d_fadetable32b)(%ebx),%al
	movb	1(%esi),%bl

	movb	%al,12(%edi)
	addl	%ebp,%edx

	movb	C(d_fadetable32b)(%ecx),%al
	movb	(%esi),%cl

	movb	%dh,%bh
	addl	%ebp,%edx

	movb	%al,8(%edi)
	movb	%dh,%ch

	movb	C(d_fadetable32b)(%ebx),%al
	movl	C(lightrrightstep),%edx

	movb	%al,4(%edi)
	addl	%edx,C(lightrright)

	movb	C(d_fadetable32b)(%ecx),%al
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

	movb	C(d_fadetable32r)(%ebx),%al
	movb	5(%esi),%bl

	movb	%al,28(%edi)
	addl	%ebp,%edx

	movb	C(d_fadetable32r)(%ecx),%al
	movb	4(%esi),%cl

	movb	%dh,%bh
	addl	%ebp,%edx

	movb	%al,24(%edi)
	movb	%dh,%ch

	movb	C(d_fadetable32r)(%ebx),%al
	movb	3(%esi),%bl

	movb	%al,20(%edi)
	addl	%ebp,%edx

	movb	C(d_fadetable32r)(%ecx),%al
	movb	2(%esi),%cl

	movb	%dh,%bh
	addl	%ebp,%edx

	movb	%al,16(%edi)
	movb	%dh,%ch

	movb	C(d_fadetable32r)(%ebx),%al
	movb	1(%esi),%bl

	movb	%al,12(%edi)
	addl	%ebp,%edx

	movb	C(d_fadetable32r)(%ecx),%al
	movb	(%esi),%cl

	movb	%dh,%bh
	addl	%ebp,%edx

	movb	%al,8(%edi)
	movb	%dh,%ch

	movb	C(d_fadetable32r)(%ebx),%al
	movl	C(lightgright),%edx

	movb	%al,4(%edi)
	movl	C(lightgleft),%ebp

	movb	C(d_fadetable32r)(%ecx),%al
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

	movb	C(d_fadetable32g)(%ebx),%al
	movb	5(%esi),%bl

	movb	%al,28(%edi)
	addl	%ebp,%edx

	movb	C(d_fadetable32g)(%ecx),%al
	movb	4(%esi),%cl

	movb	%dh,%bh
	addl	%ebp,%edx

	movb	%al,24(%edi)
	movb	%dh,%ch

	movb	C(d_fadetable32g)(%ebx),%al
	movb	3(%esi),%bl

	movb	%al,20(%edi)
	addl	%ebp,%edx

	movb	C(d_fadetable32g)(%ecx),%al
	movb	2(%esi),%cl

	movb	%dh,%bh
	addl	%ebp,%edx

	movb	%al,16(%edi)
	movb	%dh,%ch

	movb	C(d_fadetable32g)(%ebx),%al
	movb	1(%esi),%bl

	movb	%al,12(%edi)
	addl	%ebp,%edx

	movb	C(d_fadetable32g)(%ecx),%al
	movb	(%esi),%cl

	movb	%dh,%bh
	addl	%ebp,%edx

	movb	%al,8(%edi)
	movb	%dh,%ch

	movb	C(d_fadetable32g)(%ebx),%al
	movl	C(lightbright),%edx

	movb	%al,4(%edi)
	movl	C(lightbleft),%ebp

	movb	C(d_fadetable32g)(%ecx),%al
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

	movb	C(d_fadetable32b)(%ebx),%al
	movb	5(%esi),%bl

	movb	%al,28(%edi)
	addl	%ebp,%edx

	movb	C(d_fadetable32b)(%ecx),%al
	movb	4(%esi),%cl

	movb	%dh,%bh
	addl	%ebp,%edx

	movb	%al,24(%edi)
	movb	%dh,%ch

	movb	C(d_fadetable32b)(%ebx),%al
	movb	3(%esi),%bl

	movb	%al,20(%edi)
	addl	%ebp,%edx

	movb	C(d_fadetable32b)(%ecx),%al
	movb	2(%esi),%cl

	movb	%dh,%bh
	addl	%ebp,%edx

	movb	%al,16(%edi)
	movb	%dh,%ch

	movb	C(d_fadetable32b)(%ebx),%al
	movb	1(%esi),%bl

	movb	%al,12(%edi)
	addl	%ebp,%edx

	movb	C(d_fadetable32b)(%ecx),%al
	movb	(%esi),%cl

	movb	%dh,%bh
	addl	%ebp,%edx

	movb	%al,8(%edi)
	movb	%dh,%ch

	movb	C(d_fadetable32b)(%ebx),%al
	movl	C(lightrrightstep),%edx

	movb	%al,4(%edi)
	addl	%edx,C(lightrright)

	movb	C(d_fadetable32b)(%ecx),%al
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

	movb	C(d_fadetable32r)(%ebx),%al
	movb	1(%esi),%bl

	movb	%al,12(%edi)
	addl	%ebp,%edx

	movb	C(d_fadetable32r)(%ecx),%al
	movb	(%esi),%cl

	movb	%dh,%bh
	addl	%ebp,%edx

	movb	%al,8(%edi)
	movb	%dh,%ch

	movb	C(d_fadetable32r)(%ebx),%al
	movl	C(lightgright),%edx

	movb	%al,4(%edi)
	movl	C(lightgleft),%ebp

	movb	C(d_fadetable32r)(%ecx),%al
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

	movb	C(d_fadetable32g)(%ebx),%al
	movb	1(%esi),%bl

	movb	%al,12(%edi)
	addl	%ebp,%edx

	movb	C(d_fadetable32g)(%ecx),%al
	movb	(%esi),%cl

	movb	%dh,%bh
	addl	%ebp,%edx

	movb	%al,8(%edi)
	movb	%dh,%ch

	movb	C(d_fadetable32g)(%ebx),%al
	movl	C(lightbright),%edx

	movb	%al,4(%edi)
	movl	C(lightbleft),%ebp

	movb	C(d_fadetable32g)(%ecx),%al
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

	movb	C(d_fadetable32b)(%ebx),%al
	movb	1(%esi),%bl

	movb	%al,12(%edi)
	addl	%ebp,%edx

	movb	C(d_fadetable32b)(%ecx),%al
	movb	(%esi),%cl

	movb	%dh,%bh
	addl	%ebp,%edx

	movb	%al,8(%edi)
	movb	%dh,%ch

	movb	C(d_fadetable32b)(%ebx),%al
	movl	C(lightrrightstep),%edx

	movb	%al,4(%edi)
	addl	%edx,C(lightrright)

	movb	C(d_fadetable32b)(%ecx),%al
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

	movb	C(d_fadetable32r)(%ebx),%al
	movl	C(lightgright),%edx

	movb	%al,4(%edi)
	movl	C(lightgleft),%ebp

	movb	C(d_fadetable32r)(%ecx),%al
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

	movb	C(d_fadetable32g)(%ebx),%al
	movl	C(lightbright),%edx

	movb	%al,4(%edi)
	movl	C(lightbleft),%ebp

	movb	C(d_fadetable32g)(%ecx),%al
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

	movb	C(d_fadetable32b)(%ebx),%al
	movl	C(lightrrightstep),%edx

	movb	%al,4(%edi)
	addl	%edx,C(lightrright)

	movb	C(d_fadetable32b)(%ecx),%al
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

	movb	C(d_fadetable32r)(%ebx),%al
	movl	C(lightgright),%edx

	movb	%al,4(%edi)
	movl	C(lightgleft),%ebp

	movb	C(d_fadetable32r)(%ecx),%al
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

	movb	C(d_fadetable32g)(%ebx),%al
	movl	C(lightbright),%edx

	movb	%al,4(%edi)
	movl	C(lightbleft),%ebp

	movb	C(d_fadetable32g)(%ecx),%al
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

	movb	C(d_fadetable32b)(%ebx),%al

	movb	%al,4(%edi)

	movb	C(d_fadetable32b)(%ecx),%al
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
