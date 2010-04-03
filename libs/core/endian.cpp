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
//**
//**	Endianess handling, swapping 16bit and 32bit.
//**
//**************************************************************************

// HEADER FILES ------------------------------------------------------------

#include "core.h"

// MACROS ------------------------------------------------------------------

// TYPES -------------------------------------------------------------------

// EXTERNAL FUNCTION PROTOTYPES --------------------------------------------

// PUBLIC FUNCTION PROTOTYPES ----------------------------------------------

// PRIVATE FUNCTION PROTOTYPES ---------------------------------------------

// EXTERNAL DATA DECLARATIONS ----------------------------------------------

// PUBLIC DATA DEFINITIONS -------------------------------------------------

bool		GBigEndian;

vint16		(*LittleShort)(vint16);
vint16		(*BigShort)(vint16);
vint32		(*LittleLong)(vint32);
vint32		(*BigLong)(vint32);
float		(*LittleFloat)(float);
float		(*BigFloat)(float);

// PRIVATE DATA DEFINITIONS ------------------------------------------------

// CODE --------------------------------------------------------------------

//==========================================================================
//
//	ShortSwap
//
//==========================================================================

static vint16 ShortSwap(vint16 x)
{
	return  ((vuint16)x >> 8) |
			((vuint16)x << 8);
}

//==========================================================================
//
//	ShortNoSwap
//
//==========================================================================

static vint16 ShortNoSwap(vint16 x)
{
	return x;
}

//==========================================================================
//
//	LongSwap
//
//==========================================================================

static vint32 LongSwap(vint32 x)
{
	return 	((vuint32)x >> 24) |
			(((vuint32)x >> 8) & 0xff00) |
			(((vuint32)x << 8) & 0xff0000) |
			((vuint32)x << 24);
}

//==========================================================================
//
//	LongNoSwap
//
//==========================================================================

static vint32 LongNoSwap(vint32 x)
{
	return x;
}

//==========================================================================
//
//	FloatSwap
//
//==========================================================================

static float FloatSwap(float x)
{
	union { float f; vint32 l; } a;
	a.f = x;
	a.l = LongSwap(a.l);
	return a.f;
}

//==========================================================================
//
//	FloatNoSwap
//
//==========================================================================

static float FloatNoSwap(float x)
{
	return x;
}

//==========================================================================
//
//	M_InitByteOrder
//
//==========================================================================

void M_InitByteOrder()
{
	byte    swaptest[2] = {1, 0};

	// set the byte swapping variables in a portable manner
	if (*(vint16*)swaptest == 1)
	{
		GBigEndian = false;
		BigShort = ShortSwap;
		LittleShort = ShortNoSwap;
		BigLong = LongSwap;
		LittleLong = LongNoSwap;
		BigFloat = FloatSwap;
		LittleFloat = FloatNoSwap;
	}
	else
	{
		GBigEndian = true;
		BigShort = ShortNoSwap;
		LittleShort = ShortSwap;
		BigLong = LongNoSwap;
		LittleLong = LongSwap;
		BigFloat = FloatNoSwap;
		LittleFloat = FloatSwap;
	}
}
