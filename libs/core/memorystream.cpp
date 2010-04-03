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

// HEADER FILES ------------------------------------------------------------

#include "core.h"

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
//	VMemoryStream::VMemoryStream
//
//==========================================================================

VMemoryStream::VMemoryStream()
: Pos(0)
{
	bLoading = false;
}

//==========================================================================
//
//	VMemoryStream::VMemoryStream
//
//==========================================================================

VMemoryStream::VMemoryStream(void* InData, int InLen)
: Pos(0)
{
	guard(VMemoryStream::VMemoryStream);
	bLoading = true;
	Array.SetNum(InLen);
	memcpy(Array.Ptr(), InData, InLen);
	unguard;
}

//==========================================================================
//
//	VMemoryStream::VMemoryStream
//
//==========================================================================

VMemoryStream::VMemoryStream(const TArray<vuint8>& InArray)
: Pos(0)
{
	guard(VMemoryStream::VMemoryStream);
	bLoading = true;
	Array = InArray;
	unguard;
}

//==========================================================================
//
//	VMemoryStream::Serialise
//
//==========================================================================

void VMemoryStream::Serialise(void* Data, int Len)
{
	guard(VMemoryStream::Serialise);
	if (bLoading)
	{
		if (Pos + Len > Array.Num())
		{
			bError = true;
			if (Pos < Array.Num())
			{
				memcpy(Data, &Array[Pos], Array.Num() - Pos);
				Pos = Array.Num();
			}
		}
		else if (Len)
		{
			memcpy(Data, &Array[Pos], Len);
			Pos += Len;
		}
	}
	else
	{
		if (Pos + Len > Array.Num())
			Array.SetNumWithReserve(Pos + Len);
		memcpy(&Array[Pos], Data, Len);
		Pos += Len;
	}
	unguard;
}

//==========================================================================
//
//	VMemoryStream::Seek
//
//==========================================================================

void VMemoryStream::Seek(int InPos)
{
	guard(VMemoryStream::Seek);
	if (InPos < 0 || InPos > Array.Num())
	{
		bError = true;
	}
	else
	{
		Pos = InPos;
	}
	unguard;
}

//==========================================================================
//
//	VMemoryStream::Tell
//
//==========================================================================

int VMemoryStream::Tell()
{
	return Pos;
}

//==========================================================================
//
//	VMemoryStream::TotalSize
//
//==========================================================================

int VMemoryStream::TotalSize()
{
	return Array.Num();
}
