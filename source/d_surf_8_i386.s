//**************************************************************************
//**
//**	##   ##    ##    ##   ##   ####     ####   ###     ###
//**	##   ##  ##  ##  ##   ##  ##  ##   ##  ##  ####   ####
//**	 ## ##  ##    ##  ## ##  ##    ## ##    ## ## ## ## ##
//**	 ## ##  ########  ## ##  ##    ## ##    ## ##  ###  ##
//**	  ###   ##    ##   ###    ##  ##   ##  ##  ##       ##
//**	   #    ##    ##    #      ####     ####   ##       ##
//**
//**	$Id: d8_surf.s 1954 2007-01-10 18:33:13Z dj_jl $
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

	movb	C(d_fadetable)(%ebx),%ah
	movb	13(%esi),%bl

	movb	C(d_fadetable)(%ecx),%al
	movb	12(%esi),%cl

	movb	%dh,%bh
	addl	%ebp,%edx

	rorl	$16,%eax
	movb	%dh,%ch

	addl	%ebp,%edx
	movb	C(d_fadetable)(%ebx),%ah

	movb	11(%esi),%bl
	movb	C(d_fadetable)(%ecx),%al

	movb	10(%esi),%cl
	movl	%eax,12(%edi)

	movb	%dh,%bh
	addl	%ebp,%edx

	movb	%dh,%ch
	addl	%ebp,%edx

	movb	C(d_fadetable)(%ebx),%ah
	movb	9(%esi),%bl

	movb	C(d_fadetable)(%ecx),%al
	movb	8(%esi),%cl

	movb	%dh,%bh
	addl	%ebp,%edx

	rorl	$16,%eax
	movb	%dh,%ch

	addl	%ebp,%edx
	movb	C(d_fadetable)(%ebx),%ah

	movb	7(%esi),%bl
	movb	C(d_fadetable)(%ecx),%al

	movb	6(%esi),%cl
	movl	%eax,8(%edi)

	movb	%dh,%bh
	addl	%ebp,%edx

	movb	%dh,%ch
	addl	%ebp,%edx

	movb	C(d_fadetable)(%ebx),%ah
	movb	5(%esi),%bl

	movb	C(d_fadetable)(%ecx),%al
	movb	4(%esi),%cl

	movb	%dh,%bh
	addl	%ebp,%edx

	rorl	$16,%eax
	movb	%dh,%ch

	addl	%ebp,%edx
	movb	C(d_fadetable)(%ebx),%ah

	movb	3(%esi),%bl
	movb	C(d_fadetable)(%ecx),%al

	movb	2(%esi),%cl
	movl	%eax,4(%edi)

	movb	%dh,%bh
	addl	%ebp,%edx

	movb	%dh,%ch
	addl	%ebp,%edx

	movb	C(d_fadetable)(%ebx),%ah
	movb	1(%esi),%bl

	movb	C(d_fadetable)(%ecx),%al
	movb	(%esi),%cl

	movb	%dh,%bh
	addl	%ebp,%edx

	rorl	$16,%eax
	movb	%dh,%ch

	movb	C(d_fadetable)(%ebx),%ah
	movl	C(lightright),%edx

	movb	C(d_fadetable)(%ecx),%al
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

	movb	C(d_fadetable)(%ebx),%ah
	movb	5(%esi),%bl

	movb	C(d_fadetable)(%ecx),%al
	movb	4(%esi),%cl

	movb	%dh,%bh
	addl	%ebp,%edx

	rorl	$16,%eax
	movb	%dh,%ch

	addl	%ebp,%edx
	movb	C(d_fadetable)(%ebx),%ah

	movb	3(%esi),%bl
	movb	C(d_fadetable)(%ecx),%al

	movb	2(%esi),%cl
	movl	%eax,4(%edi)

	movb	%dh,%bh
	addl	%ebp,%edx

	movb	%dh,%ch
	addl	%ebp,%edx

	movb	C(d_fadetable)(%ebx),%ah
	movb	1(%esi),%bl

	movb	C(d_fadetable)(%ecx),%al
	movb	(%esi),%cl

	movb	%dh,%bh
	addl	%ebp,%edx

	rorl	$16,%eax
	movb	%dh,%ch

	movb	C(d_fadetable)(%ebx),%ah
	movl	C(lightright),%edx

	movb	C(d_fadetable)(%ecx),%al
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

	movb	C(d_fadetable)(%ebx),%ah
	movb	1(%esi),%bl

	movb	C(d_fadetable)(%ecx),%al
	movb	(%esi),%cl

	movb	%dh,%bh
	addl	%ebp,%edx

	rorl	$16,%eax
	movb	%dh,%ch

	movb	C(d_fadetable)(%ebx),%ah
	movl	C(lightright),%edx

	movb	C(d_fadetable)(%ecx),%al
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

	movb	C(d_fadetable)(%ebx),%al
	movl	C(lightright),%edx

	movb	%al,1(%edi)
	movb	C(d_fadetable)(%ecx),%al

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

	movb	C(d_fadetable)(%ebx),%al
	movl	C(sourcetstep),%edx

	movb	%al,1(%edi)
	movb	C(d_fadetable)(%ecx),%al

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
//	Coloured lighting block drawers
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
	movw	C(d_fadetable16r)(,%ecx,2),%cx
	orw		C(d_fadetable16g)(,%eax,2),%cx
	movl	%ebp,%eax
	andl	$65280,%eax
	addl	%ebx,%eax
	orw		C(d_fadetable16b)(,%eax,2),%cx
	andl	$65535,%ecx
	movl	32(%esp),%edx
	movb	C(r_rgbtable)(%ecx),%al
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
	movw	C(d_fadetable16r)(,%ecx,2),%cx
	orw		C(d_fadetable16g)(,%eax,2),%cx
	movl	%ebp,%eax
	andl	$65280,%eax
	addl	%ebx,%eax
	orw		C(d_fadetable16b)(,%eax,2),%cx
	andl	$65535,%ecx
	movl	32(%esp),%edx
	movb	C(r_rgbtable)(%ecx),%al
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
	movw	C(d_fadetable16r)(,%ecx,2),%cx
	orw		C(d_fadetable16g)(,%eax,2),%cx
	movl	%ebp,%eax
	andl	$65280,%eax
	addl	%ebx,%eax
	orw		C(d_fadetable16b)(,%eax,2),%cx
	andl	$65535,%ecx
	movl	32(%esp),%edx
	movb	C(r_rgbtable)(%ecx),%al
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
	movw	C(d_fadetable16r)(,%ecx,2),%cx
	orw		C(d_fadetable16g)(,%eax,2),%cx
	movl	%ebp,%eax
	andl	$65280,%eax
	addl	%ebx,%eax
	orw		C(d_fadetable16b)(,%eax,2),%cx
	andl	$65535,%ecx
	movl	32(%esp),%edx
	movb	C(r_rgbtable)(%ecx),%al
	movb	%al,(%edi,%edx)
	movl	56(%esp),%ecx
	movl	48(%esp),%ebx
	addl	%ecx,52(%esp)
	addl	%ebx,44(%esp)
	addl	40(%esp),%ebp
	addl	$-2,60(%esp)
	jns		L1030
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
	movw	C(d_fadetable16r)(,%ecx,2),%cx
	orw		C(d_fadetable16g)(,%eax,2),%cx
	movl	%ebp,%eax
	andl	$65280,%eax
	addl	%ebx,%eax
	orw		C(d_fadetable16b)(,%eax,2),%cx
	andl	$65535,%ecx
	movl	32(%esp),%edx
	movb	C(r_rgbtable)(%ecx),%al
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
	movw	C(d_fadetable16r)(,%ecx,2),%cx
	orw		C(d_fadetable16g)(,%eax,2),%cx
	movl	%ebp,%eax
	andl	$65280,%eax
	addl	%ebx,%eax
	orw		C(d_fadetable16b)(,%eax,2),%cx
	andl	$65535,%ecx
	movl	32(%esp),%edx
	movb	C(r_rgbtable)(%ecx),%al
	movb	%al,(%edi,%edx)
	movl	56(%esp),%ecx
	movl	48(%esp),%ebx
	addl	%ecx,52(%esp)
	addl	%ebx,44(%esp)
	addl	40(%esp),%ebp
	addl	$-2,60(%esp)
	jns		L1052
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
	cmpl	$3,%ebx
	jle		L1048
	movl	36(%esp),%eax
	cmpl	C(r_sourcemax),%eax
	jb		L1043
	movl	C(r_stepback),%edx
	subl	%edx,%eax
	movl	%eax,36(%esp)
L1043:
	movl	28(%esp),%esi
	cmpl	C(r_numvblocks),%esi
	jl		L1044
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
	jge		L1079
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
	movl	20(%esp),%ecx
	movl	28(%esp),%eax
	andl	$65280,%ecx
	xorl	%ebx,%ebx
	movb	1(%eax),%bl
	addl	%ebx,%ecx
	movl	%esi,%eax
	andl	$65280,%eax
	addl	%ebx,%eax
	movw	C(d_fadetable16r)(,%ecx,2),%cx
	orw		C(d_fadetable16g)(,%eax,2),%cx
	movl	%edi,%eax
	andl	$65280,%eax
	addl	%ebx,%eax
	orw		C(d_fadetable16b)(,%eax,2),%cx
	andl	$65535,%ecx
	movl	24(%esp),%edx
	movb	C(r_rgbtable)(%ecx),%al
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
	movw	C(d_fadetable16r)(,%ebx,2),%dx
	orw		C(d_fadetable16g)(,%esi,2),%dx
	orw		C(d_fadetable16b)(,%edi,2),%dx
	andl	$65535,%edx
	movl	24(%esp),%edi
	movb	C(r_rgbtable)(%edx),%al
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
	jle		L1070
	movl	28(%esp),%ebx
	cmpl	C(r_sourcemax),%ebx
	jb		L1065
	movl	C(r_stepback),%edi
	subl	%edi,%ebx
	movl	%ebx,28(%esp)
L1065:
	movl	16(%esp),%esi
	cmpl	C(r_numvblocks),%esi
	jl		L1066
L1079:
	popl	%ebx
	popl	%esi
	popl	%edi
	popl	%ebp
	addl	$44,%esp
	ret
