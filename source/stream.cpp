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

VStream& VStream::operator<<(VName&)
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
//	VStream::operator<<
//
//==========================================================================

VStream& VStream::operator<<(VMemberBase*&)
{
	return *this;
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
//	VStreamCompactIndex
//**************************************************************************

//==========================================================================
//
//	operator<<
//
//==========================================================================

VStream& operator<<(VStream& Strm, VStreamCompactIndex& I)
{
	guard(operator VStream << VStreamCompactIndex);
	if (Strm.IsLoading())
	{
		vuint8 B;
		Strm << B;
		bool Neg = !!(B & 0x40);
		vint32 Val = B & 0x3f;
		if (B & 0x80)
		{
			Strm << B;
			Val |= (B & 0x7f) << 6;
			if (B & 0x80)
			{
				Strm << B;
				Val |= (B & 0x7f) << 13;
				if (B & 0x80)
				{
					Strm << B;
					Val |= (B & 0x7f) << 20;
					if (B & 0x80)
					{
						Strm << B;
						Val |= (B & 0x7f) << 27;
					}
				}
			}
		}
		if (Neg)
			Val = -Val;
		I.Val = Val;
	}
	else
	{
		vint32 Val = I.Val;
		if (Val < 0)
			Val = -Val;
		vuint8 B = Val & 0x3f;
		if (I.Val < 0)
			B |= 0x40;
		if (Val & 0xffffffc0)
			B |= 0x80;
		Strm << B;
		if (Val & 0xffffffc0)
		{
			B = (Val >> 6) & 0x7f;
			if (Val & 0xffffe000)
				B |= 0x80;
			Strm << B;
			if (Val & 0xffffe000)
			{
				B = (Val >> 13) & 0x7f;
				if (Val & 0xfff00000)
					B |= 0x80;
				Strm << B;
				if (Val & 0xfff00000)
				{
					B = (Val >> 20) & 0x7f;
					if (Val & 0xf8000000)
						B |= 0x80;
					Strm << B;
					if (Val & 0xf8000000)
					{
						B = (Val >> 27) & 0x7f;
						Strm << B;
					}
				}
			}
		}
	}
	return Strm;
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

//**************************************************************************
//
//	$Log$
//	Revision 1.6  2006/03/29 20:31:59  dj_jl
//	Fixed validity check in seek.
//
//	Revision 1.5  2006/03/10 19:31:25  dj_jl
//	Use serialisation for progs files.
//	
//	Revision 1.4  2006/02/27 20:45:26  dj_jl
//	Rewrote names class.
//	
//	Revision 1.3  2006/02/27 18:44:25  dj_jl
//	Serialisation of indexes in a compact way.
//	
//	Revision 1.2  2006/02/25 17:14:19  dj_jl
//	Implemented proper serialisation of the objects.
//	
//	Revision 1.1  2006/02/22 20:33:51  dj_jl
//	Created stream class.
//	
//**************************************************************************
