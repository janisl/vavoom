//**************************************************************************
//**
//**	##   ##    ##    ##   ##   ####     ####   ###     ###
//**	##   ##  ##  ##  ##   ##  ##  ##   ##  ##  ####   ####
//**	 ## ##  ##    ##  ## ##  ##    ## ##    ## ## ## ## ##
//**	 ## ##  ########  ## ##  ##    ## ##    ## ##  ###  ##
//**	  ###   ##    ##   ###    ##  ##   ##  ##  ##       ##
//**	   #    ##    ##    #      ####     ####   ##       ##
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

#endif
