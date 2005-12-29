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

#define VERSION_MAJOR	1
#define VERSION_MINOR	19
#define VERSION_RELEASE	0
#define VERSION_TEXT	"1.19"

#if !defined CLIENT && !defined SERVER
#define CLIENT
#define SERVER
#endif

#if (defined __i386__ || defined _M_IX86) && !defined NOASM
#define USEASM
#endif

#if defined __GNUC__ && defined __i386__
#define INLINE_ASM_GAS		1
#elif (defined _MSC_VER || defined __BORLANDC__) && defined _M_IX86
#define INLINE_ASM_INTEL	1
#endif

// if rangecheck is undefined, most parameter validation debugging code
// will not be compiled
//#define PARANOID		1

//#define DEVELOPER		1

#define DO_GUARD		1

#ifdef PARANOID
#define DO_GUARD_SLOW	1
#endif

//**************************************************************************
//
//	$Log$
//	Revision 1.12  2005/12/29 17:26:01  dj_jl
//	Changed version number.
//
//	Revision 1.11  2005/08/29 19:39:30  dj_jl
//	Increased version number.
//	
//	Revision 1.10  2005/03/28 07:49:56  dj_jl
//	Changed version number.
//	
//	Revision 1.9  2004/12/27 12:23:16  dj_jl
//	Multiple small changes for version 1.16
//	
//	Revision 1.8  2004/10/18 06:37:10  dj_jl
//	Version 1.15.3
//	
//	Revision 1.7  2002/11/16 17:14:22  dj_jl
//	Some changes for release.
//	
//	Revision 1.6  2002/08/24 14:50:30  dj_jl
//	Switched to development.
//	
//	Revision 1.5  2002/08/05 17:19:14  dj_jl
//	Switched to release.
//	
//	Revision 1.4  2002/07/23 16:29:55  dj_jl
//	Replaced console streams with output device class.
//	
//	Revision 1.3  2002/06/22 07:07:21  dj_jl
//	Back to work.
//	
//	Revision 1.2  2002/06/14 15:36:35  dj_jl
//	Changed version number.
//	
//	Revision 1.1  2002/04/11 16:39:21  dj_jl
//	Seperated build setings.
//	
//**************************************************************************
