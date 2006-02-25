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

//**************************************************************************
//	VStream
//**************************************************************************

//==========================================================================
//
//	VStream::~VStream
//
//==========================================================================

VStream::~VStream()
{
}

//==========================================================================
//
//	VStream::Serialise
//
//==========================================================================

void VStream::Serialise(void*, int)
{
}

//==========================================================================
//
//	VStream::Seek
//
//==========================================================================

void VStream::Seek(int)
{
}

//==========================================================================
//
//	VStream::Tell
//
//==========================================================================

int VStream::Tell()
{
	return -1;
}

//==========================================================================
//
//	VStream::TotalSize
//
//==========================================================================

int VStream::TotalSize()
{
	return -1;
}

//==========================================================================
//
//	VStream::AtEnd
//
//==========================================================================

bool VStream::AtEnd()
{
	int Pos = Tell();
	return Pos != -1 && Pos >= TotalSize();
}

//==========================================================================
//
//	VStream::Flush
//
//==========================================================================

void VStream::Flush()
{
}

//==========================================================================
//
//	VStream::Close
//
//==========================================================================

bool VStream::Close()
{
	return !bError;
}

//==========================================================================
//
//	VStream::operator<<
//
//==========================================================================

VStream& VStream::operator<<(FName&)
{
	return *this;
}

//==========================================================================
//
//	VStream::SerialiseReference
//
//==========================================================================

void VStream::SerialiseReference(VObject*&, VClass*)
{
}

//==========================================================================
//
//	VStream::SerialiseStructPointer
//
//==========================================================================

void VStream::SerialiseStructPointer(void*&, VStruct*)
{
}

//==========================================================================
//
//	VStream::SerialiseLittleEndian
//
//==========================================================================

void VStream::SerialiseLittleEndian(void* Val, int Len)
{
	guard(VStream::SerialiseLittleEndian);
	if (GBigEndian)
	{
		//	Swap byte order.
		for (int i = Len - 1; i >= 0; i++)
			Serialise((byte*)Val + i, 1);
	}
	else
	{
		//	Already in correct byte order.
		Serialise(Val, Len);
	}
	unguard;
}

//==========================================================================
//
//	VStream::SerialiseBigEndian
//
//==========================================================================

void VStream::SerialiseBigEndian(void* Val, int Len)
{
	guard(VStream::SerialiseBigEndian);
	if (!GBigEndian)
	{
		//	Swap byte order.
		for (int i = Len - 1; i >= 0; i++)
			Serialise((byte*)Val + i, 1);
	}
	else
	{
		//	Already in correct byte order.
		Serialise(Val, Len);
	}
	unguard;
}

//**************************************************************************
//	VMemoryStream
//**************************************************************************

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
	Array.Add(InLen);
	memcpy(&Array[0], InData, InLen);
	unguard;
}

//==========================================================================
//
//	VMemoryStream::VMemoryStream
//
//==========================================================================

VMemoryStream::VMemoryStream(const TArray<byte, PU_STATIC>& InArray)
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
			memcpy(Data, &Array[Pos], Array.Num() - Pos);
			Pos = Array.Num();
		}
		else
		{
			memcpy(Data, &Array[Pos], Len);
			Pos += Len;
		}
	}
	else
	{
		if (Pos + Len > Array.Num())
			Array.Add(Pos + Len - Array.Num());
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
	if (InPos < 0 || Pos > Array.Num())
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

//**************************************************************************
//
//	$Log$
//	Revision 1.2  2006/02/25 17:14:19  dj_jl
//	Implemented proper serialisation of the objects.
//
//	Revision 1.1  2006/02/22 20:33:51  dj_jl
//	Created stream class.
//	
//**************************************************************************
