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
#define VERSION_MINOR	33
#define VERSION_RELEASE	0
#define VERSION_TEXT	"1.33"

// The version as seen in the Windows resource
#define RC_FILEVERSION VERSION_MAJOR,VERSION_MINOR,VERSION_RELEASE,SVN_REVISION_NUMBER
#define RC_PRODUCTVERSION VERSION_MAJOR,VERSION_MINOR,VERSION_RELEASE,0
#define RC_FILEVERSION2 VERSION_TEXT " (r" SVN_REVISION_STRING ")"
#define RC_PRODUCTVERSION2 VERSION_TEXT

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
