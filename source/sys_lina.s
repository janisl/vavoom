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
//**
//**	x86 assembly-language LINUX-dependent routines.
//**	
//**************************************************************************

#ifdef __i386__

	.data

	.align	4
single_cw:	.long	0
full_cw:	.long	0
cw:			.long	0

	.text

.globl Sys_SetFPCW
Sys_SetFPCW:
	fnstcw	cw
	movl	cw,%eax

    // round mode, 64-bit precision
	andb	$0xF0,%ah
	orb		$0x03,%ah
	movl	%eax,full_cw

    // chop mode, single precision
	andb	$0xF0,%ah
	orb		$0x0C,%ah
	movl	%eax,single_cw

	ret

.globl Sys_LowFPPrecision
Sys_LowFPPrecision:
	fldcw	single_cw

	ret

.globl Sys_HighFPPrecision
Sys_HighFPPrecision:
	fldcw	full_cw

	ret

#endif
