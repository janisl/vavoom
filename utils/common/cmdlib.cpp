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
//**	$Log$
//**	Revision 1.2  2001/07/27 14:27:54  dj_jl
//**	Update with Id-s and Log-s, some fixes
//**
//**************************************************************************

// HEADER FILES ------------------------------------------------------------

#include "cmdlib.h"

// MACROS ------------------------------------------------------------------

// TYPES -------------------------------------------------------------------

// EXTERNAL FUNCTION PROTOTYPES --------------------------------------------

// PUBLIC FUNCTION PROTOTYPES ----------------------------------------------

// PRIVATE FUNCTION PROTOTYPES ---------------------------------------------

// EXTERNAL DATA DECLARATIONS ----------------------------------------------

// PUBLIC DATA DEFINITIONS -------------------------------------------------

// PRIVATE DATA DEFINITIONS ------------------------------------------------

static int		empty_ptr;

// CODE --------------------------------------------------------------------

//==========================================================================
//
//	Malloc
//
//==========================================================================

void *Malloc(size_t size)
{
	if (!size)
	{
		return &empty_ptr;
	}

	void *ptr = malloc(size);
	if (!ptr)
	{
		Error("Couldn't alloc %ld bytes", size);
	}
	memset(ptr, 0, size);
	return ptr;
}

//==========================================================================
//
//	Free
//
//==========================================================================

void Free(void *ptr)
{
	if (ptr != &empty_ptr)
	{
		free(ptr);
	}
}

//==========================================================================
//
//	C++ operators
//
//==========================================================================

void * operator new(size_t size)
{
	return Malloc(size);
}

void * operator new[](size_t size)
{
	return ::operator new(size);
}

void operator delete(void *ptr)
{
	Free(ptr);
}

void operator delete[](void *ptr)
{
	::operator delete(ptr);
}

//==========================================================================
//
//	Error
//
//==========================================================================

void Error(const char *error, ...)
{
    va_list		argptr;

    va_start(argptr, error);
    vfprintf(stderr, error, argptr);
    va_end(argptr);

	exit(1);
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
	byte *bv = (byte*)&val;
	return bv[0] + (bv[1] << 8);
}

//==========================================================================
//
//	LittleLong
//
//==========================================================================

int LittleLong(int val)
{
	byte *bv = (byte*)&val;
	return bv[0] + (bv[1] << 8) + (bv[2] << 16) + (bv[3] << 24);
}

//==========================================================================
//
//	DefaultPath
//
//==========================================================================

void DefaultPath(char *path, const char *basepath)
{
	char    temp[128];

	if (path[0] == '/')
		return;                   // absolute path location
	strcpy(temp, path);
	strcpy(path, basepath);
	strcat(path, temp);
}

//==========================================================================
//
//	DefaultExtension
//
//==========================================================================

void DefaultExtension(char *path, const char *extension)
{
	char    *src;

	//
	// if path doesn't have a .EXT, append extension
	// (extension should include the .)
	//
	src = path + strlen(path) - 1;

	while (*src != '/' && src != path)
	{
		if (*src == '.')
        {
			return;                 // it has an extension
		}
		src--;
	}

	strcat(path, extension);
}

//==========================================================================
//
//	StripFilename
//
//==========================================================================

void StripFilename(char *path)
{
	int             length;

	length = strlen(path)-1;
	while (length > 0 && path[length] != '/')
		length--;
	path[length] = 0;
}

//==========================================================================
//
//	StripExtension
//
//==========================================================================

void StripExtension(char *path)
{
	char *search;

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
//	ExtractFilePath
//
//==========================================================================

void ExtractFilePath(const char *path, char *dest)
{
	const char    *src;

	src = path + strlen(path) - 1;

	//
	// back up until a \ or the start
	//
	while (src != path && *(src-1) != '/')
		src--;

	memcpy(dest, path, src - path);
	dest[src - path] = 0;
}

//==========================================================================
//
//	ExtractFileBase
//
//==========================================================================

void ExtractFileBase(const char *path, char *dest)
{
	const char    *src;

	src = path + strlen(path) - 1;

	//
	// back up until a \ or the start
	//
	while (src != path && *(src-1) != '/')
		src--;

	while (*src && *src != '.')
	{
		*dest++ = *src++;
	}
	*dest = 0;
}

//==========================================================================
//
//	ExtractFileExtension
//
//==========================================================================

void ExtractFileExtension(const char *path, char *dest)
{
	const char    *src;

	src = path + strlen(path) - 1;

	//
	// back up until a . or the start
	//
	while (src != path && *(src-1) != '.')
		src--;
	if (src == path)
	{
		*dest = 0;	// no extension
		return;
	}

	strcpy(dest, src);
}

//==========================================================================
//
//	LoadFile
//
//==========================================================================

int LoadFile(const char *name, void **bufferptr)
{
	FILE		*f;
	int			length;
	int			count;
	void    	*buffer;

	f = fopen(name, "rb");
	if (!f)
	{
		Error("Couldn't open file \"%s\".", name);
	}

	fseek(f, 0, SEEK_END);
	length = ftell(f);
	fseek(f, 0, SEEK_SET);

	buffer = Malloc(length);

	count = fread(buffer, 1, length, f);
	fclose (f);

	if (count != length)
	{
		Error("Couldn't read file \"%s\".", name);
	}

	*bufferptr = buffer;
	return length;
}


