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
//**	Copyright (C) 1999-2010 Jānis Legzdiņš
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

#ifdef _MSC_VER
// Disable some unwanted warnings
#pragma warning(disable : 4097) // typedef-name '' used as synonym for class-name ''
#pragma warning(disable : 4127) // Conditional expression is constant
#pragma warning(disable : 4244) // conversion to float, possible loss of data
#pragma warning(disable : 4284) // return type is not a UDT or reference to a UDT
#pragma warning(disable : 4291) // no matching operator delete found
#pragma warning(disable : 4305) // truncation from 'const double' to 'float'
#pragma warning(disable : 4512) // assignment operator could not be generated
#pragma warning(disable : 4514)	// unreferenced inline function has been removed
#pragma warning(disable : 4702) // unreachable code in inline expanded function
#pragma warning(disable : 4710) // inline function not expanded
#endif

#ifndef __GNUC__
#define __attribute__(whatever)
#endif

#ifndef _WIN32
#define __declspec(whatever)
#endif

//==========================================================================
//
//	Basic types
//
//==========================================================================

#define MIN_VINT8	((vint8)-128)
#define MIN_VINT16	((vint16)-32768)
#define MIN_VINT32	((vint32)-2147483648)

#define MAX_VINT8	((vint8)0x7f)
#define MAX_VINT16	((vint16)0x7fff)
#define MAX_VINT32	((vint32)0x7fffffff)

#define MAX_VUINT8	((vuint8)0xff)
#define MAX_VUINT16	((vuint16)0xffff)
#define MAX_VUINT32	((vuint32)0xffffffff)

typedef unsigned char 		byte;

#ifdef HAVE_INTTYPES_H
#include <inttypes.h>
typedef int8_t				vint8;
typedef uint8_t				vuint8;
typedef int16_t				vint16;
typedef uint16_t			vuint16;
typedef int32_t				vint32;
typedef uint32_t			vuint32;
/* Needs more changes to compile with MSVC
#elif defined _WIN32
typedef __int8				vint8;
typedef unsigned __int8		vuint8;
typedef __int16				vint16;
typedef unsigned __int16	vuint16;
typedef __int32				vint32;
typedef unsigned __int32	vuint32;
*/
#else
typedef char				vint8;
typedef unsigned char		vuint8;
typedef short				vint16;
typedef unsigned short		vuint16;
typedef int					vint32;
typedef unsigned int		vuint32;
#endif

enum ENoInit { E_NoInit };

//==========================================================================
//
//	Standard macros
//
//==========================================================================

//	Number of elements in an array.
#define ARRAY_COUNT(array)				(sizeof(array) / sizeof((array)[0]))

//==========================================================================
//
//	VInterface
//
//==========================================================================

//
//  Base class for abstract classes that need virtual destructor.
//
class VInterface
{
public:
	virtual ~VInterface();
};
