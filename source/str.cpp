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
//
//	Dynamic string class.
//
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

// CODE --------------------------------------------------------------------

//==========================================================================
//
//	VStr::VStr
//
//==========================================================================

VStr::VStr(const VStr& InStr, int Start, int Len)
: Str(NULL)
{
	check(Start >= 0);
	check(Start <= (int)InStr.Length());
	check(Len >= 0);
	check(Start + Len <= (int)InStr.Length());
	if (Len)
	{
		Resize(Len);
		strncpy(Str, InStr.Str + Start, Len);
	}
}

//==========================================================================
//
//	VStr::Resize
//
//==========================================================================

void VStr::Resize(int NewLen)
{
	guard(VStr::Resize);
	check(NewLen >= 0);
	if (!NewLen)
	{
		//	Free string.
		if (Str)
		{
			Z_Free((int*)Str - 1);
			Str = NULL;
		}
	}
	else
	{
		//	Allocate memory.
		int AllocLen = sizeof(int) + NewLen + 1;
		if (!Str)
		{
			Str = (char*)Z_Malloc(AllocLen, PU_STATIC, NULL) + sizeof(int);
		}
		else
		{
			void* BasePtr = (int*)Str - 1;
			Z_Resize(&BasePtr, AllocLen);
			Str = (char*)((int*)BasePtr + 1);
		}
		//	Set length.
		((int*)Str)[-1] = NewLen;
		//	Set terminator.
		Str[NewLen] = 0;
	}
	unguard;
}

//==========================================================================
//
//	VStr::ExtractFilePath
//
//==========================================================================

VStr VStr::ExtractFilePath() const
{
	guard(FL_ExtractFilePath);
	const char* src = Str + Length() - 1;

	//
	// back up until a \ or the start
	//
	while (src != Str && *(src - 1) != '/' && *(src - 1) != '\\')
		src--;

	return VStr(*this, 0, src - Str);
	unguard;
}

//==========================================================================
//
//	VStr:ExtractFileName
//
//==========================================================================

VStr VStr::ExtractFileName() const
{
	guard(VStr:ExtractFileName);
	const char* src = Str + Length() - 1;

	//
	// back up until a \ or the start
	//
	while (src != Str && *(src - 1) != '/' && *(src - 1) != '\\')
		src--;

	return src;
	unguard;
}

//==========================================================================
//
//	VStr::ExtractFileBase
//
//==========================================================================

VStr VStr::ExtractFileBase() const
{
	guard(VStr::ExtractFileBase);
	int i = Length() - 1;

	// back up until a \ or the start
	while (i && Str[i - 1] != '\\' && Str[i - 1] != '/')
	{
		i--;
	}

	// copy up to eight characters
	int start = i;
	int length = 0;
	while (Str[i] && Str[i] != '.')
	{
		if (++length == 9)
			Sys_Error("Filename base of %s >8 chars", Str);
		i++;
	}
	return VStr(*this, start, length);
	unguard;
}

//==========================================================================
//
//	VStr::ExtractFileExtension
//
//==========================================================================

VStr VStr::ExtractFileExtension() const
{
	guard(VStr::ExtractFileExtension);
	const char* src = Str + Length() - 1;

	//
	// back up until a . or the start
	//
	while (src != Str && *(src - 1) != '.')
		src--;
	if (src == Str)
	{
		return VStr();	// no extension
	}

	return src;
	unguard;
}

//==========================================================================
//
//	VStr::StripExtension
//
//==========================================================================

VStr VStr::StripExtension() const
{
	guard(VStr::StripExtension);
	const char* search = Str + Length() - 1;
	while (*search != '/' && *search != '\\' && search != Str)
	{
		if (*search == '.')
		{
			return VStr(*this, 0, search - Str);
		}
		search--;
	}
	return *this;
	unguard;
}

//==========================================================================
//
//	VStr::DefaultPath
//
//==========================================================================

VStr VStr::DefaultPath(const VStr& basepath) const
{
	guard(VStr::DefaultPath);
	if (Str[0] == '/')
	{
		return *this;	// absolute path location
	}
	return basepath + *this;
	unguard;
}

//==========================================================================
//
//	VStr.DefaultExtension
//
//==========================================================================

VStr VStr::DefaultExtension(const VStr& extension) const
{
	guard(VStr::DefaultExtension);
	//
	// if path doesn't have a .EXT, append extension
	// (extension should include the .)
	//
	const char* src = Str + Length() - 1;

	while (*src != '/' && *src != '\\' && src != Str)
	{
		if (*src == '.')
        {
			return *this;	// it has an extension
		}
		src--;
	}

	return *this + extension;
	unguard;
}

//**************************************************************************
//
//	$Log$
//	Revision 1.3  2006/04/05 17:23:37  dj_jl
//	More dynamic string usage in console command class.
//	Added class for handling command line arguments.
//
//	Revision 1.2  2006/03/04 16:01:34  dj_jl
//	File system API now uses strings.
//	
//	Revision 1.1  2006/02/21 22:31:44  dj_jl
//	Created dynamic string class.
//	
//**************************************************************************
