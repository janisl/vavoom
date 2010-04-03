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
//	VStream::SerialiseBits
//
//==========================================================================

void VStream::SerialiseBits(void* Data, int Length)
{
	guardSlow(VStream::SerialiseBits);
	Serialise(Data, (Length + 7) >> 3);
	if (IsLoading() && (Length & 7))
		((vuint8*)Data)[Length >> 3] &= (1 << (Length & 7)) - 1;
	unguardSlow;
}

//==========================================================================
//
//	VStream::SerialiseInt
//
//==========================================================================

void VStream::SerialiseInt(vuint32& Value, vuint32)
{
	guardSlow(VStream::SerialiseInt);
	*this << Value;
	unguardSlow;
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
//	VStream::operator<<
//
//==========================================================================

VStream& VStream::operator<<(VObject*&)
{
	return *this;
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
		for (int i = Len - 1; i >= 0; i--)
			Serialise((vuint8*)Val + i, 1);
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
		for (int i = Len - 1; i >= 0; i--)
			Serialise((vuint8*)Val + i, 1);
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
