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
	Align4

//==========================================================================
//
//	Global rasterizer variables
//
//==========================================================================

.globl	C(d_sdivzstepu)
.globl	C(d_tdivzstepu)
.globl	C(d_zistepu)
.globl	C(d_sdivzstepv)
.globl	C(d_tdivzstepv)
.globl	C(d_zistepv)
.globl	C(d_sdivzorigin)
.globl	C(d_tdivzorigin)
.globl	C(d_ziorigin)
C(d_sdivzstepu):	.single	0
C(d_tdivzstepu):	.single	0
C(d_zistepu):		.single	0
C(d_sdivzstepv):	.single	0
C(d_tdivzstepv):	.single	0
C(d_zistepv):		.single	0
C(d_sdivzorigin):	.single	0
C(d_tdivzorigin):	.single	0
C(d_ziorigin):		.single	0

.globl	C(sadjust)
.globl	C(tadjust)
.globl	C(bbextents)
.globl	C(bbextentt)
C(sadjust):			.long	0
C(tadjust):			.long	0
C(bbextents):		.long	0
C(bbextentt):		.long	0

.globl	C(cacheblock)
.globl	C(cachewidth)
.globl	C(d_transluc)
.globl	C(d_srctranstab)
.globl	C(d_dsttranstab)
C(cacheblock):		.long	0
C(cachewidth):		.long	0
C(d_transluc):		.long	0
C(d_srctranstab):	.long	0
C(d_dsttranstab):	.long	0

.globl	C(d_rowbytes)
.globl	C(d_zrowbytes)
C(d_rowbytes):		.long	0
C(d_zrowbytes):		.long	0

//==========================================================================
//
//	Asm only vars
//
//==========================================================================

.globl	sdivz8stepu
.globl	tdivz8stepu
.globl	zi8stepu
.globl	sdivz16stepu
.globl	tdivz16stepu
.globl	zi16stepu
sdivz8stepu:		.single	0
tdivz8stepu:		.single	0
zi8stepu:			.single	0
sdivz16stepu:		.single	0
tdivz16stepu:		.single	0
zi16stepu:			.single	0

.globl	s
.globl	t
.globl	snext
.globl	tnext
.globl	sstep
.globl	tstep
.globl	sfracf
.globl	tfracf
.globl	spancountminus1
s:					.long	0
t:					.long	0
snext:				.long	0
tnext:				.long	0
sstep:				.long	0
tstep:				.long	0
sfracf:				.long	0
tfracf:				.long	0
spancountminus1:	.long	0

.globl	izi
.globl	izistep
izi:				.long	0
izistep:			.long	0

.globl	advancetable
advancetable:		.long	0, 0, 2, 0

.globl	pbase
.globl	pz
pbase:				.long	0
pz:					.long	0

.globl	reciprocal_table
// 1/2, 1/3, 1/4, 1/5, 1/6, 1/7, 1/8, 1/9, 1/10, 1/11, 1/12, 1/13,
// 1/14, and 1/15 in 0.32 form
reciprocal_table:	.long	0x40000000, 0x2aaaaaaa, 0x20000000
					.long	0x19999999, 0x15555555, 0x12492492
					.long	0x10000000, 0xe38e38e, 0xccccccc, 0xba2e8ba
					.long	0xaaaaaaa, 0x9d89d89, 0x9249249, 0x8888888

.globl	pspantemp
.globl	counttemp
.globl	jumptemp
pspantemp:		.long	0
counttemp:		.long	0
jumptemp:		.long	0

.globl	mmbuf
	Align8
mmbuf:				.long	0
					.long	0

.globl	fp_8
.globl	fp_16
.globl	fp_64k
.globl	Float2ToThe31nd
.globl	FloatMinus2ToThe31nd
.globl	fp_64kx64k
//fp_8:				.long	0x41000000
fp_8:				.single	8.0
fp_16:				.single	16.0
fp_64k:				.long	0x47800000
Float2ToThe31nd:	.long	0x4f000000
FloatMinus2ToThe31nd:	.long	0xcf000000
fp_64kx64k:			.long	0x4f000000

.globl	float_1
.globl	float_particle_z_clip
.globl	float_point5
float_1:				.single	1.0
float_particle_z_clip:	.single	PARTICLE_Z_CLIP
float_point5:			.single	0.5

.globl	DP_u
.globl	DP_v
.globl	DP_32768
.globl	DP_Color
.globl	DP_Pix
DP_u:			.long	0
DP_v:			.long	0
DP_32768:		.single	32768.0
DP_Color:		.long	0
DP_Pix:			.long	0

.globl	lzistepx
lzistepx:		.long	0

.globl	gb
.globl	gbstep
gb:				.long	0
gbstep:			.long	0

#endif
