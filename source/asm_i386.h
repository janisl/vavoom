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
//**	$Log$
//**	Revision 1.2  2001/07/27 14:27:53  dj_jl
//**	Update with Id-s and Log-s, some fixes
//**
//**************************************************************************

#if defined __i386__

#define USEASM

#ifdef __linux__
#define C(label)	label		//	ELF format
#else
#define C(label)	_##label	//	COFF format
#endif

#if !defined GAS2TASM && (defined DJGPP || defined __linux__)
#define Align4		.p2align 2
#define Align8		.p2align 3
#define Align16		.p2align 4
#define Align32		.p2align 5
#define Align16max7	.p2align 4,,7
#else
#define Align4		.align 4
#define Align8		.align 4
#define Align16		.align 4
#define Align32		.align 4
#define	Align16max7
#endif

// particle_t structure
// !!! if this is changed, it must be changed in d_iface.h too !!!
// driver-usable fields
#define pt_org				0
#define pt_color			12
// drivers never touch the following fields
#define pt_next				16
#define pt_vel				20
#define pt_ramp				32
#define pt_die				36
#define pt_type				40
#define pt_size				44

#define PARTICLE_Z_CLIP		8.0

// espan_t structure
// !!! if this is changed, it must be changed in r_shared.h too !!!
#define espan_t_u    	0
#define espan_t_v	    4
#define espan_t_count   8
#define espan_t_pnext	12
#define espan_t_size    16

// sspan_t structure
// !!! if this is changed, it must be changed in d_local.h too !!!
#define sspan_t_u    	0
#define sspan_t_v	    4
#define sspan_t_count   8
#define sspan_t_size    12

//	Global rasterizer variables
.extern	C(d_sdivzstepu)
.extern	C(d_tdivzstepu)
.extern	C(d_zistepu)
.extern	C(d_sdivzstepv)
.extern	C(d_tdivzstepv)
.extern	C(d_zistepv)
.extern	C(d_sdivzorigin)
.extern	C(d_tdivzorigin)
.extern	C(d_ziorigin)

.extern	C(sadjust)
.extern	C(tadjust)
.extern	C(bbextents)
.extern	C(bbextentt)

.extern	C(cacheblock)
.extern	C(cachewidth)
.extern	C(ds_transluc)
.extern	C(ds_transluc16)

.extern	C(r_lightptr)
.extern	C(r_lightptrr)
.extern	C(r_lightptrg)
.extern	C(r_lightptrb)
.extern	C(r_lightwidth)
.extern	C(r_numvblocks)
.extern	C(r_sourcemax)
.extern	C(r_stepback)
.extern	C(prowdestbase)
.extern	C(pbasesource)
.extern	C(sourcetstep)
.extern	C(surfrowbytes)
.extern	C(lightright)
.extern	C(lightrightstep)
.extern	C(lightdeltastep)
.extern	C(lightdelta)
.extern	C(lightrleft)
.extern	C(lightrright)
.extern	C(lightrleftstep)
.extern	C(lightrrightstep)
.extern	C(lightgleft)
.extern	C(lightgright)
.extern	C(lightgleftstep)
.extern	C(lightgrightstep)
.extern	C(lightbleft)
.extern	C(lightbright)
.extern	C(lightbleftstep)
.extern	C(lightbrightstep)

//	Asm only variables
.extern	sdivz8stepu
.extern	tdivz8stepu
.extern	zi8stepu
.extern	sdivz16stepu
.extern	tdivz16stepu
.extern	zi16stepu

.extern	s
.extern	t
.extern	snext
.extern	tnext
.extern	sstep
.extern	tstep
.extern	sfracf
.extern	tfracf
.extern	spancountminus1

.extern	izi
.extern	izistep

.extern	advancetable

.extern	pbase
.extern	pz

.extern	reciprocal_table

.extern	pspantemp
.extern	counttemp
.extern	jumptemp

.extern	mmbuf

.extern	fp_64k
.extern	fp_8
.extern	fp_16
.extern	Float2ToThe31nd
.extern	FloatMinus2ToThe31nd
.extern	fp_64kx64k

.extern	float_1
.extern	float_particle_z_clip
.extern	float_point5
.extern	DP_u
.extern	DP_v
.extern	DP_32768
.extern	DP_Color
.extern	DP_Pix

.extern	ceil_cw
.extern	single_cw

//	External rasterizer variables
.extern	C(ylookup)
.extern	C(zbuffer)
.extern	C(scrn)
.extern	C(scrn16)
.extern	C(pal8_to16)
.extern	C(mmx_mask4)
.extern	C(mmx_mask8)
.extern	C(mmx_mask16)
.extern	C(d_rowbytes)
.extern	C(d_zrowbytes)
.extern	C(vieworg)
.extern	C(r_ppn)
.extern	C(r_pup)
.extern	C(r_pright)
.extern	C(centerxfrac)
.extern	C(centeryfrac)
.extern	C(d_particle_right)
.extern	C(d_particle_top)
.extern	C(d_pix_min)
.extern	C(d_pix_max)
.extern	C(d_pix_shift)
.extern	C(d_y_aspect_shift)
.extern	C(d_rgbtable)
.extern	C(rshift)
.extern	C(gshift)
.extern	C(bshift)
.extern	C(fadetable)
.extern	C(fadetable16)
.extern	C(fadetable16r)
.extern	C(fadetable16g)
.extern	C(fadetable16b)
.extern	C(fadetable32)

//	External variables for progs
.extern	C(pr_globals)
.extern	C(pr_stackPtr)
.extern	C(pr_statements)
.extern	C(pr_functions)
.extern	C(pr_globaldefs)
.extern	C(pr_builtins)
.extern	C(current_func)

.extern	C(D_DrawZSpan)
.extern	C(PR_RFInvalidOpcode)

#endif
