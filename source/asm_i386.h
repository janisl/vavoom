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
// !!! if this is changed, it must be changed in drawer.h too !!!
// driver-usable fields
#define pt_org				0
#define pt_color			12
// drivers never touch the following fields
#define pt_next				16
#define pt_vel				20
#define pt_die				32
#define pt_user_fields		36
#define pt_size				64

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

// spanpackage_t structure
// !!! if this is changed, it must be changed in d_polyse.cpp too !!!
#define spanpackage_t_pdest				0
#define spanpackage_t_pz				4
#define spanpackage_t_ptex				8
#define spanpackage_t_sfrac				12
#define spanpackage_t_tfrac				16
#define spanpackage_t_zi				20
#define spanpackage_t_count				24
#define spanpackage_t_r					26
#define spanpackage_t_g					28
#define spanpackage_t_b					30
#define spanpackage_t_size				32

// finalvert_t structure
// !!! if this is changed, it must be changed in d_local.h too !!!
#define fv_u			0
#define fv_v			4
#define fv_zi			8
#define fv_r			12
#define fv_g			16
#define fv_b			20
#define fv_flags		24
#define fv_reserved		28
#define fv_size			32

// finalstvert_t structure
// !!! if this is changed, it must be changed in d_local.h too !!!
#define fstv_s			0
#define fstv_t			4
#define fstv_size		8

// affinetridesc_t structure
// !!! if this is changed, it must be changed in d_local.h too !!!
#define atd_pskin			0
#define atd_skinwidth		4
#define atd_skinheight		8
#define atd_ptriangles		12
#define atd_pfinalverts		16
#define atd_pstverts		20
#define atd_numtriangles	24
#define atd_coloredlight	28
#define atd_size			32

// mtriangle_t structure
#define mtriangle_t_vertindex			0
#define mtriangle_t_stvertindex			6
#define	mtriangle_t_size				12

#define PARTICLE_Z_CLIP		8.0

#define DPS_SPAN_LIST_END	-9999

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
.extern	C(d_transluc)
.extern	C(d_srctranstab)
.extern	C(d_dsttranstab)

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

.extern	lzistepx
.extern	gb
.extern	gbstep

.extern	full_cw
.extern	single_cw
.extern	floor_cw
.extern	ceil_cw

//	External rasterizer variables
.extern	C(ylookup)
.extern	C(zbuffer)
.extern	C(scrn)
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
.extern	C(roffs)
.extern	C(goffs)
.extern	C(boffs)
.extern	C(fadetable)
.extern	C(fadetable16)
.extern	C(fadetable16r)
.extern	C(fadetable16g)
.extern	C(fadetable16b)
.extern	C(fadetable32)
.extern	C(fadetable32r)
.extern	C(fadetable32g)
.extern	C(fadetable32b)
.extern	C(viewwidth)
.extern	C(viewheight)

.extern	C(view_clipplanes)
.extern	C(viewforward)
.extern	C(viewright)
.extern	C(viewup)
.extern	C(vrectx_adj)
.extern	C(vrecty_adj)
.extern	C(vrectw_adj)
.extern	C(vrecth_adj)
.extern	C(r_nearzi)
.extern	C(r_emited)
.extern	C(d_u1)
.extern	C(d_v1)
.extern	C(d_ceilv1)
.extern	C(d_lastvertvalid)
.extern	C(firstvert)
.extern	C(edge_p)
.extern	C(edge_head)
.extern	C(edge_tail)
.extern	C(surfaces)
.extern	C(surface_p)
.extern	C(newedges)
.extern	C(removeedges)
.extern	C(span_p)
.extern	C(current_iv)

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

.extern	C(d_affinetridesc)
.extern	C(d_apverts)
.extern	C(d_anumverts)
.extern	C(aliastransform)
.extern	C(xprojection)
.extern	C(yprojection)
.extern	C(aliasxcenter)
.extern	C(aliasycenter)
.extern	C(ziscale)
.extern	C(d_plightvec)
.extern	C(d_avertexnormals)
.extern	C(d_ambientlightr)
.extern	C(d_ambientlightg)
.extern	C(d_ambientlightb)
.extern	C(d_shadelightr)
.extern	C(d_shadelightg)
.extern	C(d_shadelightb)

.extern	C(ubasestep)
.extern	C(errorterm)
.extern	C(erroradjustup)
.extern	C(erroradjustdown)
.extern	C(r_p0)
.extern	C(r_p1)
.extern	C(r_p2)
.extern	C(d_denom)
.extern	C(a_sstepxfrac)
.extern	C(a_tstepxfrac)
.extern	C(r_rstepx)
.extern	C(r_gstepx)
.extern	C(r_bstepx)
.extern	C(a_ststepxwhole)
.extern	C(r_sstepx)
.extern	C(r_tstepx)
.extern	C(r_rstepy)
.extern	C(r_gstepy)
.extern	C(r_bstepy)
.extern	C(r_sstepy)
.extern	C(r_tstepy)
.extern	C(r_zistepx)
.extern	C(r_zistepy)
.extern	C(d_aspancount)
.extern	C(d_countextrastep)
.extern	C(d_pedgespanpackage)
.extern	C(d_pdest)
.extern	C(d_ptex)
.extern	C(d_pz)
.extern	C(d_sfrac)
.extern	C(d_tfrac)
.extern	C(d_r)
.extern	C(d_g)
.extern	C(d_b)
.extern	C(d_zi)
.extern	C(d_ptexextrastep)
.extern	C(d_ptexbasestep)
.extern	C(d_pdestextrastep)
.extern	C(d_pdestbasestep)
.extern	C(d_sfracextrastep)
.extern	C(d_sfracbasestep)
.extern	C(d_tfracextrastep)
.extern	C(d_tfracbasestep)
.extern	C(d_rextrastep)
.extern	C(d_rbasestep)
.extern	C(d_gextrastep)
.extern	C(d_gbasestep)
.extern	C(d_bextrastep)
.extern	C(d_bbasestep)
.extern	C(d_ziextrastep)
.extern	C(d_zibasestep)
.extern	C(d_pzextrastep)
.extern	C(d_pzbasestep)
.extern	C(a_spans)
.extern	C(adivtab)

.extern	C(D_DrawZSpan)

#endif

//**************************************************************************
//
//	$Log$
//	Revision 1.13  2002/11/16 17:11:14  dj_jl
//	Improving software driver class.
//
//	Revision 1.12  2002/01/07 12:16:41  dj_jl
//	Changed copyright year
//	
//	Revision 1.11  2001/12/18 18:54:43  dj_jl
//	Found a workaround for calling a C++ function, progs code patching
//	
//	Revision 1.10  2001/11/02 18:35:54  dj_jl
//	Sky optimizations
//	
//	Revision 1.9  2001/10/18 17:36:31  dj_jl
//	A lots of changes for Alpha 2
//	
//	Revision 1.8  2001/08/21 17:22:28  dj_jl
//	Optimized rendering with some asm
//	
//	Revision 1.7  2001/08/15 17:44:41  dj_jl
//	Added missing externs
//	
//	Revision 1.6  2001/08/15 17:12:23  dj_jl
//	Optimized model drawing
//	
//	Revision 1.5  2001/08/04 17:32:39  dj_jl
//	Beautification
//	
//	Revision 1.4  2001/08/02 17:41:19  dj_jl
//	Added new asm for 32-bits
//	
//	Revision 1.3  2001/07/31 17:16:30  dj_jl
//	Just moved Log to the end of file
//	
//	Revision 1.2  2001/07/27 14:27:54  dj_jl
//	Update with Id-s and Log-s, some fixes
//
//**************************************************************************
