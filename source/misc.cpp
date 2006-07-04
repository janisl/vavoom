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

// HEADER FILES ------------------------------------------------------------

#include "gamedefs.h"

// MACROS ------------------------------------------------------------------

// TYPES -------------------------------------------------------------------

// EXTERNAL FUNCTION PROTOTYPES --------------------------------------------

// PUBLIC FUNCTION PROTOTYPES ----------------------------------------------

// PRIVATE FUNCTION PROTOTYPES ---------------------------------------------

// EXTERNAL DATA DECLARATIONS ----------------------------------------------

// PUBLIC DATA DEFINITIONS -------------------------------------------------

// PRIVATE DATA DEFINITIONS ------------------------------------------------

static char va_buffer[4][1024];
static int va_bufnum;

// CODE --------------------------------------------------------------------

//==========================================================================
//
//	Endianess handling, swapping 16bit and 32bit.
//	WAD files are stored little endian.
//
//==========================================================================

bool GBigEndian;
short (*LittleShort)(short);
short (*BigShort)(short);
int (*LittleLong)(int);
int (*BigLong)(int);
float (*LittleFloat)(float);
float (*BigFloat)(float);

//	Swaping
static short ShortSwap(short x)
{
	return  ((word)x >> 8) |
			((word)x << 8);
}
static short ShortNoSwap(short x)
{
	return x;
}
static int LongSwap(int x)
{
	return 	((vuint32)x >> 24) |
			(((vuint32)x >> 8) & 0xff00) |
			(((vuint32)x << 8) & 0xff0000) |
			((vuint32)x << 24);
}
static int LongNoSwap(int x)
{
	return x;
}
static float FloatSwap(float x)
{
	union { float f; long l; } a;
	a.f = x;
	a.l = LongSwap(a.l);
	return a.f;
}
static float FloatNoSwap(float x)
{
	return x;
}

void M_InitByteOrder()
{
	byte    swaptest[2] = {1, 0};

	// set the byte swapping variables in a portable manner
	if (*(short *)swaptest == 1)
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

//==========================================================================
//
//	M_ReadFile
//
//==========================================================================

int M_ReadFile(const char* name, byte** buffer)
{
	int			handle;
	int			count;
	int			length;
	byte		*buf;
	
	handle = Sys_FileOpenRead(name);
	if (handle == -1)
	{
		Sys_Error("Couldn't open file %s", name);
	}
	length = Sys_FileSize(handle);
	buf = (byte*)Z_Malloc(length + 1);
	count = Sys_FileRead(handle, buf, length);
	buf[length] = 0;
	Sys_FileClose(handle);
	
	if (count < length)
	{
		Sys_Error("Couldn't read file %s", name);
	}

	*buffer = buf;
	return length;
}

//==========================================================================
//
//	M_WriteFile
//
//==========================================================================

boolean M_WriteFile(const char* name, const void* source, int length)
{
	int		handle;
	int		count;
	
	handle = Sys_FileOpenWrite(name);
	if (handle == -1)
	{
		return false;
	}

	count = Sys_FileWrite(handle, source, length);
	Sys_FileClose(handle);
	
	if (count < length)
	{
		return false;
	}
		
	return true;
}

//==========================================================================
//
//  superatoi
//
//==========================================================================

int superatoi(const char *s)
{
	int n=0, r=10, x, mul=1;
	const char *c=s;

	for (; *c; c++)
	{
		x = (*c & 223) - 16;

		if (x == -3)
		{
			mul = -mul;
		}
		else if (x == 72 && r == 10)
		{
			n -= (r=n);
			if (!r) r=16;
			if (r<2 || r>36) return -1;
		}
		else
		{
			if (x>10) x-=39;
			if (x >= r) return -1;
			n = (n*r) + x;
		}
	}
	return(mul*n);
}

//==========================================================================
//
//	va
//
//	Very usefull function from QUAKE
//	Does a varargs printf into a temp buffer, so I don't need to have
// varargs versions of all text functions.
//	FIXME: make this buffer size safe someday
//
//==========================================================================

char *va(const char *text, ...)
{
	va_list args;

	va_bufnum = (va_bufnum + 1) & 3;
	va_start(args, text);
	vsprintf(va_buffer[va_bufnum], text, args);
	va_end(args);

	return va_buffer[va_bufnum];
}

//==========================================================================
//
//	PassFloat
//
//==========================================================================

int PassFloat(float f)
{
	union
	{
		float	f;
		int		i;
	} v;

	v.f = f;
	return v.i;
}
