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

#define VERSION_MAJOR	1
#define VERSION_MINOR	23
#define VERSION_RELEASE	1
#define VERSION_TEXT	"1.23.1"

#if !defined CLIENT && !defined SERVER
#define CLIENT
#define SERVER
#endif

#if USE_ASM_I386
#if defined __GNUC__
#define INLINE_ASM_I386_GAS		1
#elif (defined _MSC_VER || defined __BORLANDC__)
#define INLINE_ASM_I386_INTEL	1
#endif
#endif

// if rangecheck is undefined, most parameter validation debugging code
// will not be compiled
//#define PARANOID		1

//#define DEVELOPER		1

#define DO_GUARD		1
#define DO_CHECK		1

#ifdef PARANOID
#define DO_GUARD_SLOW	1
#define DO_CHECK_SLOW	1
#endif
