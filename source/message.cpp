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

// CODE --------------------------------------------------------------------

//==========================================================================
//
//  VMessageOut::Alloc
//
//==========================================================================

void VMessageOut::AllocBits(vint32 startsize)
{
	guard(VMessageOut::Alloc);
	ArrData.SetNum((startsize + 7) >> 3);
	MaxSizeBits = startsize;
	CurSizeBits = 0;
	Overflowed = false;
	memset(ArrData.Ptr(), 0, (MaxSizeBits + 7) >> 3);
	unguard;
}

//==========================================================================
//
//  VMessageOut::Free
//
//==========================================================================

void VMessageOut::Free()
{
	guard(VMessageOut::Free);
	ArrData.Clear();
	MaxSizeBits = 0;
	CurSizeBits = 0;
	unguard;
}

//==========================================================================
//
//  VMessageOut::Clear
//
//==========================================================================

void VMessageOut::Clear()
{
	CurSizeBits = 0;
	memset(ArrData.Ptr(), 0, (MaxSizeBits + 7) >> 3);
}

//==========================================================================
//
//  VMessageOut::Serialise
//
//==========================================================================

void VMessageOut::Serialise(void* data, vint32 length)
{
	SerialiseBits(data, length << 3);
}

//==========================================================================
//
//  VMessageOut::SerialiseBits
//
//==========================================================================

void VMessageOut::SerialiseBits(void* Src, vint32 Length)
{
	guard(VMessageOut::SerialiseBits);
	if (!Length)
	{
		return;
	}

	if (CurSizeBits + Length > MaxSizeBits)
	{
		if (!AllowOverflow)
			Sys_Error("TSizeBuf::GetSpace: overflow without allowoverflow set");

		if (Length > MaxSizeBits)
			Sys_Error("TSizeBuf::GetSpace: %i is > full buffer size", Length);

		Overflowed = true;
		GCon->Log("TSizeBuf::GetSpace: overflow");
		Clear();
		return;
	}

	if (Length <= 8)
	{
		int Byte1 = CurSizeBits >> 3;
		int Byte2 = (CurSizeBits + Length - 1) >> 3;

		vuint8 Val = ((vuint8*)Src)[0] & ((1 << Length) - 1);
		int Shift = CurSizeBits & 7;
		if (Byte1 == Byte2)
		{
			ArrData[Byte1] |= Val << Shift;
		}
		else
		{
			ArrData[Byte1] |= Val << Shift;
			ArrData[Byte2] |= Val >> (8 - Shift);
		}
		CurSizeBits += Length;
		return;
	}

	int Bytes = Length >> 3;
	if (Bytes)
	{
		if (CurSizeBits & 7)
		{
			vuint8* pSrc = (vuint8*)Src;
			vuint8* pDst = (vuint8*)ArrData.Ptr() + (CurSizeBits >> 3);
			for (int i = 0; i < Bytes; i++, pSrc++, pDst++)
			{
				pDst[0] |= *pSrc << (CurSizeBits & 7);
				pDst[1] |= *pSrc >> (8 - (CurSizeBits & 7));
			}
		}
		else
		{
			memcpy(ArrData.Ptr() + ((CurSizeBits + 7) >> 3), Src, Length >> 3);
		}
		CurSizeBits += Length & ~7;
	}

	if (Length & 7)
	{
		int Byte1 = CurSizeBits >> 3;
		int Byte2 = (CurSizeBits + (Length & 7) - 1) >> 3;
		vuint8 Val = ((vuint8*)Src)[Length >> 3] & ((1 << (Length & 7)) - 1);
		int Shift = CurSizeBits & 7;
		if (Byte1 == Byte2)
		{
			ArrData[Byte1] |= Val << Shift;
		}
		else
		{
			ArrData[Byte1] |= Val << Shift;
			ArrData[Byte2] |= Val >> (8 - Shift);
		}
		CurSizeBits += Length & 7;
	}
	unguard;
}

//==========================================================================
//
//	VMessageOut::operator << VMessageOut
//
//==========================================================================

VMessageOut& VMessageOut::operator << (const VMessageOut &msg)
{
	guard(VMessageOut::operator << VMessageOut);
	SerialiseBits(const_cast<VMessageOut&>(msg).GetData(), msg.CurSizeBits);
	return *this;
	unguard;
}

//==========================================================================
//
//  VMessageIn::AllocBits
//
//==========================================================================

void VMessageIn::AllocBits(vint32 startsize)
{
	guard(VMessageIn::AllocBits);
	ArrData.SetNum((startsize + 7) >> 3);
	MaxSizeBits = startsize;
	CurSizeBits = 0;
	bLoading = true;
	unguard;
}

//==========================================================================
//
//  VMessageIn::Free
//
//==========================================================================

void VMessageIn::Free()
{
	guard(VMessageIn::Free);
	ArrData.Clear();
	MaxSizeBits = 0;
	CurSizeBits = 0;
	unguard;
}

//==========================================================================
//
//  VMessageIn::SetDataBits
//
//==========================================================================

void VMessageIn::SetDataBits(const void* AData, vint32 ALength)
{
	guard(VMessageIn::SetDataBits);
	if (ALength > MaxSizeBits)
		Sys_Error("TSizeBuf::GetSpace: %i is > full buffer size", ALength);
	memcpy(ArrData.Ptr(), AData, (ALength + 7) >> 3);
	CurSizeBits = ALength;
	unguard;
}

//==========================================================================
//
//  VMessageIn::BeginReading
//
//==========================================================================

void VMessageIn::BeginReading()
{
	ReadCountBits = 0;
	BadRead = false;
}

//==========================================================================
//
//  VMessageIn::Serialise
//
//==========================================================================

void VMessageIn::Serialise(void* AData, int ALen)
{
	SerialiseBits(AData, ALen << 3);
}

//==========================================================================
//
//  VMessageIn::SerialiseBits
//
//==========================================================================

void VMessageIn::SerialiseBits(void* Dst, int Length)
{
	guard(VMessageIn::SerialiseBits);
	if (!Length)
	{
		return;
	}

	if (ReadCountBits + Length > CurSizeBits)
	{
		BadRead = true;
		memset(Dst, 0, (Length + 7) >> 3);
		return;
	}

	if (ReadCountBits & 7)
	{
		int SrcPos = ReadCountBits >> 3;
		int Shift1 = ReadCountBits & 7;
		int Shift2 = 8 - Shift1;
		int Count = Length >> 3;
		for (int i = 0; i < Count; i++, SrcPos++)
		{
			((vuint8*)Dst)[i] = (ArrData[SrcPos] >> Shift1) |
				ArrData[SrcPos + 1] << Shift2;
		}
		if (Length & 7)
		{
			if ((Length & 7) > Shift2)
			{
				((vuint8*)Dst)[Count] = ((ArrData[SrcPos] >> Shift1) |
					ArrData[SrcPos + 1] << Shift2) & ((1 << (Length & 7)) - 1);
			}
			else
			{
				((vuint8*)Dst)[Count] = (ArrData[SrcPos] >> Shift1) &
					((1 << (Length & 7)) - 1);
			}
		}
	}
	else
	{
		int Count = Length >> 3;
		memcpy(Dst, ArrData.Ptr() + (ReadCountBits >> 3), Count);
		if (Length & 7)
		{
			((vuint8*)Dst)[Count] = ArrData[Count] & ((1 << (Length & 7)) - 1);
		}
	}
	ReadCountBits += Length;
	unguard;
}
