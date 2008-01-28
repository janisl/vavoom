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

#include "vcc.h"

// MACROS ------------------------------------------------------------------

// TYPES -------------------------------------------------------------------

// EXTERNAL FUNCTION PROTOTYPES --------------------------------------------

// PUBLIC FUNCTION PROTOTYPES ----------------------------------------------

// PRIVATE FUNCTION PROTOTYPES ---------------------------------------------

// EXTERNAL DATA DECLARATIONS ----------------------------------------------

// PUBLIC DATA DEFINITIONS -------------------------------------------------

// PRIVATE DATA DEFINITIONS ------------------------------------------------

// CODE --------------------------------------------------------------------

//==========================================================================
//
//	Malloc
//
//==========================================================================

void* Malloc(size_t size)
{
	if (!size)
	{
		return NULL;
	}

	void *ptr = malloc(size);
	if (!ptr)
	{
		FatalError("Couldn't alloc %d bytes", (int)size);
	}
	memset(ptr, 0, size);
	return ptr;
}

//==========================================================================
//
//	Free
//
//==========================================================================

void Free(void* ptr)
{
	if (ptr)
	{
		free(ptr);
	}
}

//==========================================================================
//
//	va
//
//	Very usefull function from QUAKE
//
//==========================================================================

char *va(const char *text, ...)
{
	va_list		args;
	static char	va_buffer[1024];

	va_start(args, text);
	vsprintf(va_buffer, text, args);
	va_end(args);

	return va_buffer;
}

//==========================================================================
//
//	LittleShort
//
//==========================================================================

short LittleShort(short val)
{
	vuint8* bv = (vuint8*)&val;
	return bv[0] + (bv[1] << 8);
}

//==========================================================================
//
//	LittleLong
//
//==========================================================================

int LittleLong(int val)
{
	vuint8* bv = (vuint8*)&val;
	return bv[0] + (bv[1] << 8) + (bv[2] << 16) + (bv[3] << 24);
}

//==========================================================================
//
//	DefaultExtension
//
//==========================================================================

void DefaultExtension(char *path, const char *extension)
{
	char	*src;

	//
	// if path doesn't have a .EXT, append extension
	// (extension should include the .)
	//
	src = path + strlen(path) - 1;

	while (*src != '/' && src != path)
	{
		if (*src == '.')
		{
			return;			// it has an extension
		}
		src--;
	}

	strcat(path, extension);
}

//==========================================================================
//
//	StripExtension
//
//==========================================================================

void StripExtension(char *path)
{
	char	*search;

	search = path + strlen(path) - 1;
	while (*search != '/' && search != path)
	{
		if (*search == '.')
		{
			*search = 0;
			return;
		}
		search--;
	}
}

//==========================================================================
//
//	FixFileSlashes
//
//==========================================================================

void FixFileSlashes(char *path)
{
	while (*path)
	{
		if (*path == '\\')
			*path = '/';
		path++;
	}
}
